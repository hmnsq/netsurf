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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <setjmp.h>
#شامل <string.h>

#شامل <jxl/decode.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/bitmap.h"
#شامل "content/llcache.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image_cache.h"

#شامل "image/jpegxl.h"


/**
 * output image format
 */
ایستا ثابت JxlPixelFormat jxl_output_format = {
	.رقم_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ایستا nserror
nsjpegxl_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	ساختار content *jpeg;
	nserror error;

	jpeg = calloc(1, اندازه(ساختار content));
	اگر (jpeg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(jpeg);
		بازگشت error;
	}

	*c = jpeg;

	بازگشت NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
ایستا ساختار bitmap *
jpegxl_cache_convert(ساختار content *c)
{
	ساختار bitmap * bitmap = پوچ;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	ثابت uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(پوچ);
	اگر (jxldec == پوچ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		بازگشت پوچ;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		بازگشت پوچ;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_درون_داد(jxldec, src_data, src_size);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		بازگشت پوچ;
	}

	decstatus = Jxl_Decoder_Process_درون_داد(jxldec);
	اگر (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}

	/* create bitmap with appropriate opacity */
	اگر (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} وگرنه {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	اگر (bitmap == پوچ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	اگر (output == پوچ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}

	decstatus = Jxl_Decoder_Process_درون_داد(jxldec);
	اگر (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		بازگشت پوچ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	بازگشت bitmap;
}

/**
 * report failiure
 */
ایستا bool jxl_report_fail(ساختار content *c, JxlDecoderStatus decstatus, ثابت نویسه *پیام)
{
	اجتماع content_پیام_data پیام_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", پیام, decstatus);
	پیام_data.errordata.errorcode = NSERROR_UNKNOWN;
	پیام_data.errordata.errormsg = پیام;
	content_broadcast(c, CONTENT_پیام_ERROR, &پیام_data);
	بازگشت false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ایستا bool nsjpegxl_convert(ساختار content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	اجتماع content_پیام_data پیام_data;
	ثابت uint8_t *data;
	size_t size;
	نویسه *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	اگر ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		پیام_data.errordata.errorcode = NSERROR_UNKNOWN;
		پیام_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_پیام_ERROR, &پیام_data);
		بازگشت false;
	}

	jxldec = JxlDecoderCreate(پوچ);
	اگر (jxldec == پوچ) {
		بازگشت jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		بازگشت jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_درون_داد(jxldec, data,size);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		بازگشت jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_درون_داد(jxldec);
	اگر (decstatus != JXL_DEC_BASIC_INFO) {
		بازگشت jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		بازگشت jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	اگر (decstatus != JXL_DEC_SUCCESS) {
		بازگشت jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, پوچ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	اگر (title != پوچ) {
		content__set_title(c, title);
		آزاد(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	بازگشت true;
}


/**
 * Clone content.
 */
ایستا nserror nsjpegxl_clone(ثابت ساختار content *old, ساختار content **newc)
{
	ساختار content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, اندازه(ساختار content));
	اگر (jpegxl_c == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	اگر (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		بازگشت error;
	}

	/* re-convert if the content is ready */
	اگر ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		اگر (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	بازگشت NSERROR_OK;
}

ایستا ثابت content_handler nsjpegxl_content_handler = {
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

ایستا ثابت نویسه *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
