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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ JxlPixelFormat jxl_output_format = {
	.ಅಂಕೆ_channels = 4,
	.ದತ್ತಾಂಶ_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ಸ್ಥಿರ nserror
nsjpegxl_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks, ರಚನೆ content **c)
{
	ರಚನೆ content *jpeg;
	nserror error;

	jpeg = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content));
	ಆದರೆ (jpeg == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(jpeg);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = jpeg;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
ಸ್ಥಿರ ರಚನೆ bitmap *
jpegxl_cache_convert(ರಚನೆ content *c)
{
	ರಚನೆ bitmap * bitmap = ಖಾಲಿ;
	JxlDecoder *jxldec;
	Jxl_Decoder_ಸ್ಥಿತಿ decstatus;
	JxlBasicInfo binfo;
	ಸ್ಥಿರಾಂಕ uint8_t *src_ದತ್ತಾಂಶ;
	size_t src_ಗಾತ್ರ;
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

	jxldec = JxlDecoderCreate(ಖಾಲಿ);
	ಆದರೆ (jxldec == ಖಾಲಿ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	decstatus = Jxl_Decoder_ಗಣ_Unpremultiply_Alpha(jxldec, !bitmap_fmt.pma);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_ತುಂಬಿದ_IMAGE);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	src_ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &src_ಗಾತ್ರ);

	decstatus = Jxl_Decoder_ಗಣ_ಒಳಹರಿವು(jxldec, src_ದತ್ತಾಂಶ, src_ಗಾತ್ರ);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಆದರೆ (decstatus != JXL_DEC_NEED_IMAGE_ಹೊರಹೋಗು_ಬಫರ್) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_ಹೊರಹೋಗು_ಬಫರ್,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* create bitmap with appropriate opacity */
	ಆದರೆ (binfo.alpha_ಬಿಟ್ಟುಗಳು > 0) {
		bitmap = guit->bitmap->create(c->ಅಗಲ, c->ಎತ್ತರ, BITMAP_OPAQUE);
	} ಇಲ್ಲದಿದ್ದರೆ {
		bitmap = guit->bitmap->create(c->ಅಗಲ, c->ಎತ್ತರ, BITMAP_NONE);
	}
	ಆದರೆ (bitmap == ಖಾಲಿ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_ಬಫರ್(bitmap);
	ಆದರೆ (output == ಖಾಲಿ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	decstatus = Jxl_Decoder_ಗಣ_Image_ಹೊರಹೋಗು_ಬಫರ್(jxldec, &jxl_output_format, output, c->ಗಾತ್ರ_2);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಆದರೆ (decstatus != JXL_DEC_ತುಂಬಿದ_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_ಗಮ್ಯ_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	ಹಿಂತಿರುಗಿ bitmap;
}

/**
 * report failiure
 */
ಸ್ಥಿರ bool jxl_report_fail(ರಚನೆ content *c, Jxl_Decoder_ಸ್ಥಿತಿ decstatus, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", ಸಂದೇಶ, decstatus);
	ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errorcode = NSERROR_UNKNOWN;
	ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errormsg = ಸಂದೇಶ;
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	ಹಿಂತಿರುಗಿ false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ಸ್ಥಿರ bool nsjpegxl_convert(ರಚನೆ content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	Jxl_Decoder_ಸ್ಥಿತಿ decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *title;
	size_t image_ಗಾತ್ರ;

	/* check image header is valid and get width/height */
	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಗಾತ್ರ_2);

	decsig = JxlSignatureCheck(ದತ್ತಾಂಶ,ಗಾತ್ರ_2);
	ಆದರೆ ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errorcode = NSERROR_UNKNOWN;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errormsg = "Signature failed";
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ಹಿಂತಿರುಗಿ false;
	}

	jxldec = JxlDecoderCreate(ಖಾಲಿ);
	ಆದರೆ (jxldec == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_ಗಣ_ಒಳಹರಿವು(jxldec, ದತ್ತಾಂಶ,ಗಾತ್ರ_2);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ಒಳಹರಿವು(jxldec);
	ಆದರೆ (decstatus != JXL_DEC_BASIC_INFO) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = Jxl_Decoder_Image_ಹೊರಹೋಗು_ಬಫರ್_ಗಾತ್ರ(jxldec, &jxl_output_format, &image_ಗಾತ್ರ);
	ಆದರೆ (decstatus != JXL_DEC_SUCCESS) {
		ಹಿಂತಿರುಗಿ jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_ಗಾತ್ರ, binfo.xsize, binfo.ysize);

	c->ಅಗಲ = binfo.xsize;
	c->ಎತ್ತರ = binfo.ysize;
	c->ಗಾತ್ರ_2 = image_ಗಾತ್ರ;

	image_cache_ಕೂಡಿಸು(c, ಖಾಲಿ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_ಎಲೆ(llcache_handle_get_url(c->llcache)),
			c->ಅಗಲ, c->ಎತ್ತರ);
	ಆದರೆ (title != ಖಾಲಿ) {
		content_ಗಣ_title(c, title);
		ಬಿಡುಗಡೆ(title);
	}

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);	
	content_ಗಣ_ಸ್ಥಿತಿ(c, ""); /* Done: update status bar */

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Clone content.
 */
ಸ್ಥಿರ nserror nsjpegxl_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	ರಚನೆ content *jpegxl_c;
	nserror error;

	jpegxl_c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content));
	ಆದರೆ (jpegxl_c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		ಹಿಂತಿರುಗಿ error;
	}

	/* re-convert if the content is ready */
	ಆದರೆ ((old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) ||
	    (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		ಆದರೆ (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.ದತ್ತಾಂಶ_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_ಆಂತರಿಕ = image_cache_get_ಆಂತರಿಕ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
