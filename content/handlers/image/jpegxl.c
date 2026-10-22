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
	.عدد_channels = 4,
	.معلومات_type    = JXL_TYPE_UINT8,
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

	jpeg = حافظه_صفر_ورکړه(1, حجم(بنية content));
	إذا (jpeg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
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
	Jxl_Decoder_حالت decstatus;
	JxlBasicInfo binfo;
	ثابت uint8_t *src_معلومات;
	size_t src_اندازه;
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
	إذا (jxldec == عدم) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		أرجع عدم;
	}

	decstatus = Jxl_Decoder_سېټ_Unpremultiply_Alpha(jxldec, !bitmap_fmt.pma);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_ډک_IMAGE);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		أرجع عدم;
	}
	src_معلومات = content_get_سرچینه_معلومات_2(c, &src_اندازه);

	decstatus = Jxl_Decoder_سېټ_ننوت(jxldec, src_معلومات, src_اندازه);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		أرجع عدم;
	}

	decstatus = Jxl_Decoder_Process_ننوت(jxldec);
	إذا (decstatus != JXL_DEC_NEED_IMAGE_بهر_بفر) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_بهر_بفر,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	/* create bitmap with appropriate opacity */
	إذا (binfo.alpha_بټونه > 0) {
		bitmap = guit->bitmap->create(c->پلنوالی, c->لوړوالی, BITMAP_OPAQUE);
	} وإلا {
		bitmap = guit->bitmap->create(c->پلنوالی, c->لوړوالی, BITMAP_NONE);
	}
	إذا (bitmap == عدم) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_بفر(bitmap);
	إذا (output == عدم) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}
	decstatus = Jxl_Decoder_سېټ_Image_بهر_بفر(jxldec, &jxl_output_format, output, c->اندازه);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}

	decstatus = Jxl_Decoder_Process_ننوت(jxldec);
	إذا (decstatus != JXL_DEC_ډک_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		أرجع عدم;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_منزل_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	أرجع bitmap;
}

/**
 * report failiure
 */
ساكن bool jxl_report_fail(بنية content *c, Jxl_Decoder_حالت decstatus, ثابت حرف *پیغام)
{
	اتحاد content_پیغام_معلومات پیغام_معلومات;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", پیغام, decstatus);
	پیغام_معلومات.errordata.errorcode = NSERROR_UNKNOWN;
	پیغام_معلومات.errordata.errormsg = پیغام;
	content_خپرونه(c, CONTENT_پیغام_ERROR, &پیغام_معلومات);
	أرجع false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ساكن bool nsjpegxl_convert(بنية content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	Jxl_Decoder_حالت decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	اتحاد content_پیغام_معلومات پیغام_معلومات;
	ثابت uint8_t *معلومات;
	size_t اندازه;
	حرف *title;
	size_t image_اندازه;

	/* check image header is valid and get width/height */
	معلومات = content_get_سرچینه_معلومات_2(c, &اندازه);

	decsig = JxlSignatureCheck(معلومات,اندازه);
	إذا ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		پیغام_معلومات.errordata.errorcode = NSERROR_UNKNOWN;
		پیغام_معلومات.errordata.errormsg = "Signature failed";
		content_خپرونه(c, CONTENT_پیغام_ERROR, &پیغام_معلومات);
		أرجع false;
	}

	jxldec = JxlDecoderCreate(عدم);
	إذا (jxldec == عدم) {
		أرجع jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_سېټ_ننوت(jxldec, معلومات,اندازه);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ننوت(jxldec);
	إذا (decstatus != JXL_DEC_BASIC_INFO) {
		أرجع jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = Jxl_Decoder_Image_بهر_بفر_اندازه(jxldec, &jxl_output_format, &image_اندازه);
	إذا (decstatus != JXL_DEC_SUCCESS) {
		أرجع jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_اندازه, binfo.xsize, binfo.ysize);

	c->پلنوالی = binfo.xsize;
	c->لوړوالی = binfo.ysize;
	c->اندازه = image_اندازه;

	image_cache_جمع_کړه(c, عدم, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_ونې_پاڼه(llcache_handle_get_url(c->llcache)),
			c->پلنوالی, c->لوړوالی);
	إذا (title != عدم) {
		content_سېټ_title(c, title);
		حرر(title);
	}

	content_سېټ_ready(c);
	content_سېټ_بشپړ(c);	
	content_سېټ_حالت(c, ""); /* Done: update status bar */

	أرجع true;
}


/**
 * Clone content.
 */
ساكن nserror nsjpegxl_clone(ثابت بنية content *old, بنية content **newc)
{
	بنية content *jpegxl_c;
	nserror error;

	jpegxl_c = حافظه_صفر_ورکړه(1, حجم(بنية content));
	إذا (jpegxl_c == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	إذا (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		أرجع error;
	}

	/* re-convert if the content is ready */
	إذا ((old->حالت == CONTENT_حالت_READY) ||
	    (old->حالت == CONTENT_حالت_بشپړ)) {
		إذا (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	أرجع NSERROR_OK;
}

ساكن ثابت content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.معلومات_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_دننی = image_cache_get_دننی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_ثبتوونکی_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
