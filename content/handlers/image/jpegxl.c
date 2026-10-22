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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <jxl/decode.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image_cache.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/jpegxl.h"


/**
 * output image format
 */
ਸਥਿਰ ਅਚਲ JxlPixelFormat jxl_output_format = {
	.ਅੰਕ_channels = 4,
	.ਅੰਕੜੇ_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ਸਥਿਰ nserror
nsjpegxl_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	ਬਣਤਰ content *jpeg;
	nserror error;

	jpeg = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content));
	ਜੇ (jpeg == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(jpeg);
		ਵਾਪਸ error;
	}

	*c = jpeg;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
ਸਥਿਰ ਬਣਤਰ bitmap *
jpegxl_cache_convert(ਬਣਤਰ content *c)
{
	ਬਣਤਰ bitmap * bitmap = ਨਿਰਾ;
	JxlDecoder *jxldec;
	Jxl_Decoder_ਸਥਿਤੀ decstatus;
	JxlBasicInfo binfo;
	ਅਚਲ uint8_t *src_ਅੰਕੜੇ;
	size_t src_ਮਾਪ;
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

	jxldec = JxlDecoderCreate(ਨਿਰਾ);
	ਜੇ (jxldec == ਨਿਰਾ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		ਵਾਪਸ ਨਿਰਾ;
	}

	decstatus = Jxl_Decoder_ਸੈੱਟ_Unpremultiply_Alpha(jxldec, !bitmap_fmt.pma);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_ਭਰਿਆ_IMAGE);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		ਵਾਪਸ ਨਿਰਾ;
	}
	src_ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &src_ਮਾਪ);

	decstatus = Jxl_Decoder_ਸੈੱਟ_ਨਿਵੇਸ਼(jxldec, src_ਅੰਕੜੇ, src_ਮਾਪ);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		ਵਾਪਸ ਨਿਰਾ;
	}

	decstatus = Jxl_Decoder_Process_ਨਿਵੇਸ਼(jxldec);
	ਜੇ (decstatus != JXL_DEC_NEED_IMAGE_ਬਾਹਰਲਾ_ਬਫਰ) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_ਬਾਹਰਲਾ_ਬਫਰ,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* create bitmap with appropriate opacity */
	ਜੇ (binfo.alpha_ਬਿੱਟ > 0) {
		bitmap = guit->bitmap->create(c->ਚੌੜਾਈ, c->ਉਚਾਈ, BITMAP_OPAQUE);
	} ਨਹੀਂਤਾਂ {
		bitmap = guit->bitmap->create(c->ਚੌੜਾਈ, c->ਉਚਾਈ, BITMAP_NONE);
	}
	ਜੇ (bitmap == ਨਿਰਾ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_ਬਫਰ(bitmap);
	ਜੇ (output == ਨਿਰਾ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}
	decstatus = Jxl_Decoder_ਸੈੱਟ_Image_ਬਾਹਰਲਾ_ਬਫਰ(jxldec, &jxl_output_format, output, c->ਮਾਪ);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}

	decstatus = Jxl_Decoder_Process_ਨਿਵੇਸ਼(jxldec);
	ਜੇ (decstatus != JXL_DEC_ਭਰਿਆ_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ਵਾਪਸ ਨਿਰਾ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_ਮੰਜ਼ਿਲ_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	ਵਾਪਸ bitmap;
}

/**
 * report failiure
 */
ਸਥਿਰ bool jxl_report_fail(ਬਣਤਰ content *c, Jxl_Decoder_ਸਥਿਤੀ decstatus, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", ਸੁਨੇਹਾ, decstatus);
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errorcode = NSERROR_UNKNOWN;
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errormsg = ਸੁਨੇਹਾ;
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ERROR, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	ਵਾਪਸ false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ਸਥਿਰ bool nsjpegxl_convert(ਬਣਤਰ content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	Jxl_Decoder_ਸਥਿਤੀ decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;
	size_t image_ਮਾਪ;

	/* check image header is valid and get width/height */
	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	decsig = JxlSignatureCheck(ਅੰਕੜੇ,ਮਾਪ);
	ਜੇ ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errorcode = NSERROR_UNKNOWN;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errormsg = "Signature failed";
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ERROR, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਵਾਪਸ false;
	}

	jxldec = JxlDecoderCreate(ਨਿਰਾ);
	ਜੇ (jxldec == ਨਿਰਾ) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_ਸੈੱਟ_ਨਿਵੇਸ਼(jxldec, ਅੰਕੜੇ,ਮਾਪ);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ਨਿਵੇਸ਼(jxldec);
	ਜੇ (decstatus != JXL_DEC_BASIC_INFO) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = Jxl_Decoder_Image_ਬਾਹਰਲਾ_ਬਫਰ_ਮਾਪ(jxldec, &jxl_output_format, &image_ਮਾਪ);
	ਜੇ (decstatus != JXL_DEC_SUCCESS) {
		ਵਾਪਸ jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_ਮਾਪ, binfo.xsize, binfo.ysize);

	c->ਚੌੜਾਈ = binfo.xsize;
	c->ਉਚਾਈ = binfo.ysize;
	c->ਮਾਪ = image_ਮਾਪ;

	image_cache_ਜੋੜੋ(c, ਨਿਰਾ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);	
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, ""); /* Done: update status bar */

	ਵਾਪਸ true;
}


/**
 * Clone content.
 */
ਸਥਿਰ nserror nsjpegxl_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	ਬਣਤਰ content *jpegxl_c;
	nserror error;

	jpegxl_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content));
	ਜੇ (jpegxl_c == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		ਵਾਪਸ error;
	}

	/* re-convert if the content is ready */
	ਜੇ ((old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) ||
	    (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		ਜੇ (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਅਚਲ content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.ਅੰਕੜੇ_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_ਅੰਦਰੂਨੀ = image_cache_get_ਅੰਦਰੂਨੀ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
