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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <setjmp.h>
#ضمن <string.h>

#ضمن <jxl/decode.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "content/llcache.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image_cache.h"

#ضمن "image/jpegxl.h"


/**
 * output image format
 */
ساكن ثابت JxlPixelFormat jxl_output_format = {
	.num_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ساكن nserror
nsjpegxl_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	بنية content *jpeg;
	nserror error;

	jpeg = calloc(1, حجم(بنية content));
	𐤀𐤌 (jpeg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	𐤀𐤌 (error != NSERROR_OK) {
		حرر(jpeg);
		أرجع error;
	}

	*c = jpeg;

	أرجع NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
ساكن بنية bitmap *
jpegxl_cache_convert(بنية content *c)
{
	بنية bitmap * bitmap = عدم;
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

	jxldec = JxlDecoderCreate(عدم);
	𐤀𐤌 (jxldec == عدم) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		أرجع عدم;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		أرجع عدم;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = JxlDecoderSetInput(jxldec, src_data, src_size);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		أرجع عدم;
	}

	decstatus = JxlDecoderProcessInput(jxldec);
	𐤀𐤌 (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	/* create bitmap with appropriate opacity */
	𐤀𐤌 (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} وإلا {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	𐤀𐤌 (bitmap == عدم) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	𐤀𐤌 (output == عدم) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus = JxlDecoderProcessInput(jxldec);
	𐤀𐤌 (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	أرجع bitmap;
}

/**
 * report failiure
 */
ساكن bool jxl_report_fail(بنية content *c, JxlDecoderStatus decstatus, ثابت حرف *msg)
{
	اتحاد content_msg_data msg_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", msg, decstatus);
	msg_data.errordata.errorcode = NSERROR_UNKNOWN;
	msg_data.errordata.errormsg = msg;
	content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
	أرجع false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ساكن bool nsjpegxl_convert(بنية content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	اتحاد content_msg_data msg_data;
	ثابت uint8_t *data;
	size_t size;
	حرف *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	𐤀𐤌 ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		msg_data.errordata.errorcode = NSERROR_UNKNOWN;
		msg_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
		أرجع false;
	}

	jxldec = JxlDecoderCreate(عدم);
	𐤀𐤌 (jxldec == عدم) {
		أرجع jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = JxlDecoderSetInput(jxldec, data,size);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = JxlDecoderProcessInput(jxldec);
	𐤀𐤌 (decstatus != JXL_DEC_BASIC_INFO) {
		أرجع jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	𐤀𐤌 (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, عدم, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	𐤀𐤌 (title != عدم) {
		content__set_title(c, title);
		حرر(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	أرجع true;
}


/**
 * Clone content.
 */
ساكن nserror nsjpegxl_clone(ثابت بنية content *old, بنية content **newc)
{
	بنية content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, حجم(بنية content));
	𐤀𐤌 (jpegxl_c == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		أرجع error;
	}

	/* re-convert if the content is ready */
	𐤀𐤌 ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		𐤀𐤌 (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	أرجع NSERROR_OK;
}

ساكن ثابت content_handler nsjpegxl_content_handler = {
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

ساكن ثابت حرف *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
