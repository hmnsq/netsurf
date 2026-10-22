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
	.अङ्कः_channels = 4,
	.दत्त_type    = JXL_TYPE_UINT8,
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

	jpeg = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content));
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
	Jxl_Decoder_स्थितिः decstatus;
	JxlBasicInfo binfo;
	ध्रुव uint8_t *src_दत्त;
	size_t src_आकार;
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

	decstatus = Jxl_Decoder_समुच्चयः_Unpremultiply_Alpha(jxldec, !bitmap_fmt.pma);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_पूर्णम्_IMAGE);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		प्रत्यागम रिक्त;
	}
	src_दत्त = content_get_source_दत्त_2(c, &src_आकार);

	decstatus = Jxl_Decoder_समुच्चयः_निवेशः(jxldec, src_दत्त, src_आकार);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		प्रत्यागम रिक्त;
	}

	decstatus = Jxl_Decoder_Process_निवेशः(jxldec);
	यदि (decstatus != JXL_DEC_NEED_IMAGE_बहिर्गमनम्_बफर) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_बहिर्गमनम्_बफर,
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
	यदि (binfo.alpha_द्वयङ्काः > 0) {
		bitmap = guit->bitmap->create(c->विस्तारः, c->height, BITMAP_OPAQUE);
	} अन्यथा {
		bitmap = guit->bitmap->create(c->विस्तारः, c->height, BITMAP_NONE);
	}
	यदि (bitmap == रिक्त) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_बफर(bitmap);
	यदि (output == रिक्त) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}
	decstatus = Jxl_Decoder_समुच्चयः_Image_बहिर्गमनम्_बफर(jxldec, &jxl_output_format, output, c->आकार);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}

	decstatus = Jxl_Decoder_Process_निवेशः(jxldec);
	यदि (decstatus != JXL_DEC_पूर्णम्_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		प्रत्यागम रिक्त;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_गन्तव्यम्_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	प्रत्यागम bitmap;
}

/**
 * report failiure
 */
स्थिर bool jxl_report_fail(संरचना content *c, Jxl_Decoder_स्थितिः decstatus, ध्रुव वर्ण *सन्देशः)
{
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", सन्देशः, decstatus);
	सन्देशः_दत्त.errordata.errorcode = NSERROR_UNKNOWN;
	सन्देशः_दत्त.errordata.errormsg = सन्देशः;
	content_प्रसारणम्(c, CONTENT_सन्देशः_ERROR, &सन्देशः_दत्त);
	प्रत्यागम false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
स्थिर bool nsjpegxl_convert(संरचना content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	Jxl_Decoder_स्थितिः decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	ध्रुव uint8_t *दत्त;
	size_t आकार;
	वर्ण *title;
	size_t image_आकार;

	/* check image header is valid and get width/height */
	दत्त = content_get_source_दत्त_2(c, &आकार);

	decsig = JxlSignatureCheck(दत्त,आकार);
	यदि ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		सन्देशः_दत्त.errordata.errorcode = NSERROR_UNKNOWN;
		सन्देशः_दत्त.errordata.errormsg = "Signature failed";
		content_प्रसारणम्(c, CONTENT_सन्देशः_ERROR, &सन्देशः_दत्त);
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
	decstatus = Jxl_Decoder_समुच्चयः_निवेशः(jxldec, दत्त,आकार);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_निवेशः(jxldec);
	यदि (decstatus != JXL_DEC_BASIC_INFO) {
		प्रत्यागम jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = Jxl_Decoder_Image_बहिर्गमनम्_बफर_आकार(jxldec, &jxl_output_format, &image_आकार);
	यदि (decstatus != JXL_DEC_SUCCESS) {
		प्रत्यागम jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_आकार, binfo.xsize, binfo.ysize);

	c->विस्तारः = binfo.xsize;
	c->height = binfo.ysize;
	c->आकार = image_आकार;

	image_cache_सङ्कलय(c, रिक्त, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_पर्णम्(llcache_handle_get_url(c->llcache)),
			c->विस्तारः, c->height);
	यदि (title != रिक्त) {
		content_समुच्चयः_title(c, title);
		मुक्त(title);
	}

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);	
	content_समुच्चयः_स्थितिः(c, ""); /* Done: update status bar */

	प्रत्यागम true;
}


/**
 * Clone content.
 */
स्थिर nserror nsjpegxl_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	संरचना content *jpegxl_c;
	nserror error;

	jpegxl_c = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content));
	यदि (jpegxl_c == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	यदि (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		प्रत्यागम error;
	}

	/* re-convert if the content is ready */
	यदि ((old->स्थितिः == CONTENT_स्थितिः_READY) ||
	    (old->स्थितिः == CONTENT_स्थितिः_समाप्तम्)) {
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
	.दत्त_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_आन्तरिकम् = image_cache_get_आन्तरिकम्,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
