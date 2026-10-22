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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <setjmp.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ <jxl/decode.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image_cache.h"

#அந்தர்பாவ "image/jpegxl.h"


/**
 * output image format
 */
நிலையான மாறிலி JxlPixelFormat jxl_output_format = {
	.இலக்கம்_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
நிலையான nserror
nsjpegxl_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	கட்டமைப்பு content *jpeg;
	nserror error;

	jpeg = calloc(1, அளவு(கட்டமைப்பு content));
	எனில் (jpeg == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(jpeg);
		திரும்பு error;
	}

	*c = jpeg;

	திரும்பு NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
நிலையான கட்டமைப்பு bitmap *
jpegxl_cache_convert(கட்டமைப்பு content *c)
{
	கட்டமைப்பு bitmap * bitmap = வெற்று;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	மாறிலி uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(வெற்று);
	எனில் (jxldec == வெற்று) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		திரும்பு வெற்று;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		திரும்பு வெற்று;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_உள்ளீடு(jxldec, src_data, src_size);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		திரும்பு வெற்று;
	}

	decstatus = Jxl_Decoder_Process_உள்ளீடு(jxldec);
	எனில் (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}

	/* create bitmap with appropriate opacity */
	எனில் (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} இல்லையெனில் {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	எனில் (bitmap == வெற்று) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	எனில் (output == வெற்று) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}

	decstatus = Jxl_Decoder_Process_உள்ளீடு(jxldec);
	எனில் (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		திரும்பு வெற்று;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	திரும்பு bitmap;
}

/**
 * report failiure
 */
நிலையான bool jxl_report_fail(கட்டமைப்பு content *c, JxlDecoderStatus decstatus, மாறிலி எழுத்து *செய்தி)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", செய்தி, decstatus);
	செய்தி_data.errordata.errorcode = NSERROR_UNKNOWN;
	செய்தி_data.errordata.errormsg = செய்தி;
	content_broadcast(c, CONTENT_செய்தி_ERROR, &செய்தி_data);
	திரும்பு false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
நிலையான bool nsjpegxl_convert(கட்டமைப்பு content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	மாறிலி uint8_t *data;
	size_t size;
	எழுத்து *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	எனில் ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		செய்தி_data.errordata.errorcode = NSERROR_UNKNOWN;
		செய்தி_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_செய்தி_ERROR, &செய்தி_data);
		திரும்பு false;
	}

	jxldec = JxlDecoderCreate(வெற்று);
	எனில் (jxldec == வெற்று) {
		திரும்பு jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		திரும்பு jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_உள்ளீடு(jxldec, data,size);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		திரும்பு jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_உள்ளீடு(jxldec);
	எனில் (decstatus != JXL_DEC_BASIC_INFO) {
		திரும்பு jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		திரும்பு jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	எனில் (decstatus != JXL_DEC_SUCCESS) {
		திரும்பு jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, வெற்று, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	திரும்பு true;
}


/**
 * Clone content.
 */
நிலையான nserror nsjpegxl_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	கட்டமைப்பு content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, அளவு(கட்டமைப்பு content));
	எனில் (jpegxl_c == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	எனில் (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		திரும்பு error;
	}

	/* re-convert if the content is ready */
	எனில் ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		எனில் (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	திரும்பு NSERROR_OK;
}

நிலையான மாறிலி content_handler nsjpegxl_content_handler = {
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

நிலையான மாறிலி எழுத்து *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
