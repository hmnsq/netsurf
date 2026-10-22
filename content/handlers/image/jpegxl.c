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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <setjmp.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ <jxl/decode.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image_cache.h"

#അന്തര്ഭാവ "image/jpegxl.h"


/**
 * output image format
 */
സ്ഥിതം സ്ഥിരം JxlPixelFormat jxl_output_format = {
	.അക്കം_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
സ്ഥിതം nserror
nsjpegxl_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	ഘടന content *jpeg;
	nserror error;

	jpeg = calloc(1, വലുപ്പം(ഘടന content));
	എങ്കിൽ (jpeg == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(jpeg);
		തിരികെ error;
	}

	*c = jpeg;

	തിരികെ NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
സ്ഥിതം ഘടന bitmap *
jpegxl_cache_convert(ഘടന content *c)
{
	ഘടന bitmap * bitmap = ശൂന്യ;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	സ്ഥിരം uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(ശൂന്യ);
	എങ്കിൽ (jxldec == ശൂന്യ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		തിരികെ ശൂന്യ;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		തിരികെ ശൂന്യ;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_നിവേശം(jxldec, src_data, src_size);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		തിരികെ ശൂന്യ;
	}

	decstatus = Jxl_Decoder_Process_നിവേശം(jxldec);
	എങ്കിൽ (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}

	/* create bitmap with appropriate opacity */
	എങ്കിൽ (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} അല്ലെങ്കിൽ {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	എങ്കിൽ (bitmap == ശൂന്യ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	എങ്കിൽ (output == ശൂന്യ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}

	decstatus = Jxl_Decoder_Process_നിവേശം(jxldec);
	എങ്കിൽ (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		തിരികെ ശൂന്യ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	തിരികെ bitmap;
}

/**
 * report failiure
 */
സ്ഥിതം bool jxl_report_fail(ഘടന content *c, JxlDecoderStatus decstatus, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", സന്ദേശം, decstatus);
	സന്ദേശം_data.errordata.errorcode = NSERROR_UNKNOWN;
	സന്ദേശം_data.errordata.errormsg = സന്ദേശം;
	content_broadcast(c, CONTENT_സന്ദേശം_ERROR, &സന്ദേശം_data);
	തിരികെ false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
സ്ഥിതം bool nsjpegxl_convert(ഘടന content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	സ്ഥിരം uint8_t *data;
	size_t size;
	അക്ഷരം *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	എങ്കിൽ ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		സന്ദേശം_data.errordata.errorcode = NSERROR_UNKNOWN;
		സന്ദേശം_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_സന്ദേശം_ERROR, &സന്ദേശം_data);
		തിരികെ false;
	}

	jxldec = JxlDecoderCreate(ശൂന്യ);
	എങ്കിൽ (jxldec == ശൂന്യ) {
		തിരികെ jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		തിരികെ jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_നിവേശം(jxldec, data,size);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		തിരികെ jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_നിവേശം(jxldec);
	എങ്കിൽ (decstatus != JXL_DEC_BASIC_INFO) {
		തിരികെ jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		തിരികെ jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	എങ്കിൽ (decstatus != JXL_DEC_SUCCESS) {
		തിരികെ jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, ശൂന്യ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	തിരികെ true;
}


/**
 * Clone content.
 */
സ്ഥിതം nserror nsjpegxl_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	ഘടന content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, വലുപ്പം(ഘടന content));
	എങ്കിൽ (jpegxl_c == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		തിരികെ error;
	}

	/* re-convert if the content is ready */
	എങ്കിൽ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		എങ്കിൽ (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം സ്ഥിരം content_handler nsjpegxl_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
