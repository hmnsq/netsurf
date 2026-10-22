/*
 * Copyright 2023 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * implementation of content handling for image/jpegxl
 *
 * This implementation uses the JXL library.
 */

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <setjmp.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব <jxl/decode.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image_cache.h"

#অন্তর্ভাব "image/jpegxl.h"


/**
 * output image format
 */
স্থির ধ্রুব JxlPixelFormat jxl_output_format = {
	.অঙ্ক_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
স্থির nserror
nsjpegxl_create(ধ্রুব content_handler *handler,
		lwc_string *imime_type, ধ্রুব সংরচনা http_parameter *params,
		llcache_handle *llcache, ধ্রুব বর্ণ *fallback_charset,
		bool quirks, সংরচনা content **c)
{
	সংরচনা content *jpeg;
	nserror error;

	jpeg = calloc(1, পরিমাণ(সংরচনা content));
	যদি (jpeg == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(jpeg);
		প্রত্যাগম error;
	}

	*c = jpeg;

	প্রত্যাগম NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
স্থির সংরচনা bitmap *
jpegxl_cache_convert(সংরচনা content *c)
{
	সংরচনা bitmap * bitmap = রিক্ত;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	ধ্রুব uint8_t *src_data;
	size_t src_size;
	uint8_t * output;
	bitmap_fmt_t jxl_fmt = {
		/** TODO: At the moment we have to set the layout to the only
		 *        pixel layout that libjxl supports. It looks like they
		 *        plan to add support for decoding to other layouts
		 *        in the future, as shown by the TODO in the docs:
		 *
		 * https://libjxl.readthedocs.io/en/latest/api_common.html#_CPPv414JxlPixelFormat
		 */
		.layout = BITMAP_LAYOUT_R8G8B8A8,
		.pma    = bitmap_fmt.pma,
	};

	jxldec = JxlDecoderCreate(রিক্ত);
	যদি (jxldec == রিক্ত) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		প্রত্যাগম রিক্ত;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		প্রত্যাগম রিক্ত;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_প্রবেশ(jxldec, src_data, src_size);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		প্রত্যাগম রিক্ত;
	}

	decstatus = Jxl_Decoder_Process_প্রবেশ(jxldec);
	যদি (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}

	/* create bitmap with appropriate opacity */
	যদি (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} অন্যথা {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	যদি (bitmap == রিক্ত) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	যদি (output == রিক্ত) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}

	decstatus = Jxl_Decoder_Process_প্রবেশ(jxldec);
	যদি (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		প্রত্যাগম রিক্ত;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	প্রত্যাগম bitmap;
}

/**
 * report failiure
 */
স্থির bool jxl_report_fail(সংরচনা content *c, JxlDecoderStatus decstatus, ধ্রুব বর্ণ *বার্তা)
{
	সঙ্ঘ content_বার্তা_data বার্তা_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", বার্তা, decstatus);
	বার্তা_data.errordata.errorcode = NSERROR_UNKNOWN;
	বার্তা_data.errordata.errormsg = বার্তা;
	content_broadcast(c, CONTENT_বার্তা_ERROR, &বার্তা_data);
	প্রত্যাগম false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
স্থির bool nsjpegxl_convert(সংরচনা content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	সঙ্ঘ content_বার্তা_data বার্তা_data;
	ধ্রুব uint8_t *data;
	size_t size;
	বর্ণ *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	যদি ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		বার্তা_data.errordata.errorcode = NSERROR_UNKNOWN;
		বার্তা_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_বার্তা_ERROR, &বার্তা_data);
		প্রত্যাগম false;
	}

	jxldec = JxlDecoderCreate(রিক্ত);
	যদি (jxldec == রিক্ত) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_প্রবেশ(jxldec, data,size);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_প্রবেশ(jxldec);
	যদি (decstatus != JXL_DEC_BASIC_INFO) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	যদি (decstatus != JXL_DEC_SUCCESS) {
		প্রত্যাগম jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, রিক্ত, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != রিক্ত) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	প্রত্যাগম true;
}


/**
 * Clone content.
 */
স্থির nserror nsjpegxl_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc)
{
	সংরচনা content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, পরিমাণ(সংরচনা content));
	যদি (jpegxl_c == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	যদি (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		প্রত্যাগম error;
	}

	/* re-convert if the content is ready */
	যদি ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		যদি (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			প্রত্যাগম NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	প্রত্যাগম NSERROR_OK;
}

স্থির ধ্রুব content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.data_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

স্থির ধ্রুব বর্ণ *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
