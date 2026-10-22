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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <setjmp.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव <jxl/decode.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"

#अन्तर्भाव "image/jpegxl.h"


/**
 * output image format
 */
स्थिर ध्रुव JxlPixelFormat jxl_output_format = {
	.अंक_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
स्थिर nserror
nsjpegxl_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	संरचना content *jpeg;
	nserror error;

	jpeg = calloc(1, परिमाण(संरचना content));
	यदि (jpeg == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(jpeg);
		प्रत्यागम error;
	}

	*c = jpeg;

	प्रत्यागम NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
स्थिर संरचना bitmap *
jpegxl_cache_convert(संरचना content *c)
{
	संरचना bitmap * bitmap = रिक्त;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	ध्रुव uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(रिक्त);
	यदि (jxldec == रिक्त) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		प्रत्यागम रिक्त;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		प्रत्यागम रिक्त;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_निवेश(jxldec, src_data, src_size);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		प्रत्यागम रिक्त;
	}

	decstatus = Jxl_Decoder_Process_निवेश(jxldec);
	यदि (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	/* create bitmap with appropriate opacity */
	यदि (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} अन्यथा {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	यदि (bitmap == रिक्त) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	यदि (output == रिक्त) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	decstatus = Jxl_Decoder_Process_निवेश(jxldec);
	यदि (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	प्रत्यागम bitmap;
}

/**
 * report failiure
 */
स्थिर bool jxl_report_fail(संरचना content *c, JxlDecoderStatus decstatus, ध्रुव वर्ण *संदेश)
{
	सङ्घ content_संदेश_data संदेश_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", संदेश, decstatus);
	संदेश_data.errordata.errorcode = NSERROR_UNKNOWN;
	संदेश_data.errordata.errormsg = संदेश;
	content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_data);
	प्रत्यागम false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
स्थिर bool nsjpegxl_convert(संरचना content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	सङ्घ content_संदेश_data संदेश_data;
	ध्रुव uint8_t *data;
	size_t size;
	वर्ण *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	यदि ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		संदेश_data.errordata.errorcode = NSERROR_UNKNOWN;
		संदेश_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_data);
		प्रत्यागम false;
	}

	jxldec = JxlDecoderCreate(रिक्त);
	यदि (jxldec == रिक्त) {
		प्रत्यागम jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_निवेश(jxldec, data,size);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_निवेश(jxldec);
	यदि (decstatus != JXL_DEC_BASIC_INFO) {
		प्रत्यागम jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, रिक्त, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	यदि (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	प्रत्यागम true;
}


/**
 * Clone content.
 */
स्थिर nserror nsjpegxl_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	संरचना content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, परिमाण(संरचना content));
	यदि (jpegxl_c == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	यदि (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		प्रत्यागम error;
	}

	/* re-convert if the content is ready */
	यदि ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		यदि (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	प्रत्यागम NSERROR_OK;
}

स्थिर ध्रुव content_handler nsjpegxl_content_handler = {
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

स्थिर ध्रुव वर्ण *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
