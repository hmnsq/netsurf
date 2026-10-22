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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <setjmp.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ <jxl/decode.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"

#ಅನ್ತರ್ಭಾವ "image/jpegxl.h"


/**
 * output image format
 */
ಸ್ಥಿರ ಧ್ರುವ JxlPixelFormat jxl_output_format = {
	.ಅಂಕೆ_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ಸ್ಥಿರ nserror
nsjpegxl_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	ಸಂರಚನಾ content *jpeg;
	nserror error;

	jpeg = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content));
	ಯದಿ (jpeg == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(jpeg);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = jpeg;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
ಸ್ಥಿರ ಸಂರಚನಾ bitmap *
jpegxl_cache_convert(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ bitmap * bitmap = ರಿಕ್ತ;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	ಧ್ರುವ uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(ರಿಕ್ತ);
	ಯದಿ (jxldec == ರಿಕ್ತ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_ಒಳಹರಿವು(jxldec, src_data, src_size);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಯದಿ (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* create bitmap with appropriate opacity */
	ಯದಿ (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} ಅನ್ಯಥಾ {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	ಯದಿ (bitmap == ರಿಕ್ತ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	ಯದಿ (output == ರಿಕ್ತ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಯದಿ (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	ಪ್ರತ್ಯಾಗಮ bitmap;
}

/**
 * report failiure
 */
ಸ್ಥಿರ bool jxl_report_fail(ಸಂರಚನಾ content *c, JxlDecoderStatus decstatus, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", ಸಂದೇಶ, decstatus);
	ಸಂದೇಶ_data.errordata.errorcode = NSERROR_UNKNOWN;
	ಸಂದೇಶ_data.errordata.errormsg = ಸಂದೇಶ;
	content_broadcast(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_data);
	ಪ್ರತ್ಯಾಗಮ false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ಸ್ಥಿರ bool nsjpegxl_convert(ಸಂರಚನಾ content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಧ್ರುವ uint8_t *data;
	size_t size;
	ವರ್ಣ *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	ಯದಿ ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		ಸಂದೇಶ_data.errordata.errorcode = NSERROR_UNKNOWN;
		ಸಂದೇಶ_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_data);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	jxldec = JxlDecoderCreate(ರಿಕ್ತ);
	ಯದಿ (jxldec == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_ಒಳಹರಿವು(jxldec, data,size);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಯದಿ (decstatus != JXL_DEC_BASIC_INFO) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	ಯದಿ (decstatus != JXL_DEC_SUCCESS) {
		ಪ್ರತ್ಯಾಗಮ jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, ರಿಕ್ತ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Clone content.
 */
ಸ್ಥಿರ nserror nsjpegxl_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	ಸಂರಚನಾ content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content));
	ಯದಿ (jpegxl_c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* re-convert if the content is ready */
	ಯದಿ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ಯದಿ (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nsjpegxl_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
