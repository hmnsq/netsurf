/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
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
 * implementation of content handling for image/jpeg
 *
 * This implementation uses the IJG JPEG library.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JPEG_ਅੰਦਰੂਨੀ_OPTIONS
#ਅਨ੍ਤਰ੍ਭਾਵ "jpeglib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_JPEG_ਮਾਪ 20

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBJPEG_TURBO_ਸੰਸਕਰਣ
#warning Using libjpeg (libjpeg-turbo is recommended)
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅੱਖਰ nsjpeg_error_ਬਫਰ[JMSG_ਲੰਮਾਈ_MAX];

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
ਸਥਿਰ nserror nsjpeg_create(ਅਚਲ content_handler *handler,
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
 * JPEG data source manager: initialize source.
 */
ਸਥਿਰ ਖਾਲੀ nsjpeg_init_ਸਰੋਤ(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
ਸਥਿਰ boolean nsjpeg_fill_ਨਿਵੇਸ਼_ਬਫਰ(j_decompress_ptr cinfo)
{
	cinfo->src->ਅਗਲਾ_ਨਿਵੇਸ਼_byte = nsjpeg_eoi;
	cinfo->src->ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ = 2;
 	ਵਾਪਸ TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

ਸਥਿਰ ਖਾਲੀ nsjpeg_skip_ਨਿਵੇਸ਼_ਅੰਕੜੇ(j_decompress_ptr cinfo, ਲੰਮਾ ਅੰਕ_ਬਾਈਟਾਂ)
{
	ਜੇ ((ਲੰਮਾ) cinfo->src->ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ < ਅੰਕ_ਬਾਈਟਾਂ) {
		cinfo->src->ਅਗਲਾ_ਨਿਵੇਸ਼_byte = 0;
		cinfo->src->ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ = 0;
	} ਨਹੀਂਤਾਂ {
		cinfo->src->ਅਗਲਾ_ਨਿਵੇਸ਼_byte += ਅੰਕ_ਬਾਈਟਾਂ;
		cinfo->src->ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ -= ਅੰਕ_ਬਾਈਟਾਂ;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
ਸਥਿਰ ਖਾਲੀ nsjpeg_ਪਦ_ਸਰੋਤ(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
ਸਥਿਰ ਖਾਲੀ nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_ਸੁਨੇਹਾ(cinfo, nsjpeg_error_ਬਫਰ);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_ਬਫਰ);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
ਸਥਿਰ ਖਾਲੀ nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_ਬਫਰ = (jmp_buf *) cinfo->client_ਅੰਕੜੇ;

	cinfo->err->format_ਸੁਨੇਹਾ(cinfo, nsjpeg_error_ਬਫਰ);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_ਬਫਰ);

	longjmp(*setjmp_ਬਫਰ, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ nsjpeg__decode_cmyk(
		ਬਣਤਰ jpeg_decompress_struct *cinfo,
		uint8_t * ਅਸਥਿਰ pixels,
		size_t rowstride)
{
	ਪੂਰਨ ਚੌੜਾਈ = cinfo->output_ਚੌੜਾਈ * 4;

	ਕਰੋ {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		ਲਈ (ਪੂਰਨ i = ਚੌੜਾਈ - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			ਅਚਲ ਪੂਰਨ c = scanlines[0][i + 0];
			ਅਚਲ ਪੂਰਨ m = scanlines[0][i + 1];
			ਅਚਲ ਪੂਰਨ y = scanlines[0][i + 2];
			ਅਚਲ ਪੂਰਨ k = scanlines[0][i + 3];

			ਅਚਲ ਪੂਰਨ ck = c * k;
			ਅਚਲ ਪੂਰਨ mk = m * k;
			ਅਚਲ ਪੂਰਨ yk = y * k;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ DIV255
		}
	} ਜਦੋਂ (cinfo->output_scanline != cinfo->output_ਉਚਾਈ);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ nsjpeg__decode_rgb(
		ਬਣਤਰ jpeg_decompress_struct *cinfo,
		uint8_t * ਅਸਥਿਰ pixels,
		size_t rowstride)
{
#ਜੇ RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	ਪੂਰਨ ਚੌੜਾਈ = cinfo->output_ਚੌੜਾਈ;
#ਜੇ_ਅੰਤ

	ਕਰੋ {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#ਜੇ RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		ਲਈ (ਪੂਰਨ i = ਚੌੜਾਈ - 1; 0 <= i; i--) {
			ਪੂਰਨ r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			ਪੂਰਨ g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			ਪੂਰਨ b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#ਜੇ_ਅੰਤ
	} ਜਦੋਂ (cinfo->output_scanline != cinfo->output_ਉਚਾਈ);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ nsjpeg__decode_client_fmt(
		ਬਣਤਰ jpeg_decompress_struct *cinfo,
		uint8_t * ਅਸਥਿਰ pixels,
		size_t rowstride)
{
	ਕਰੋ {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} ਜਦੋਂ (cinfo->output_scanline != cinfo->output_ਉਚਾਈ);
}

/**
 * create a bitmap from jpeg content.
 */
ਸਥਿਰ ਬਣਤਰ bitmap *
jpeg_cache_convert(ਬਣਤਰ content *c)
{
	ਅਚਲ uint8_t *ਸਰੋਤ_ਅੰਕੜੇ; /* Jpeg source data */
	size_t ਸਰੋਤ_ਮਾਪ; /* length of Jpeg source data */
	ਬਣਤਰ jpeg_decompress_struct cinfo;
	ਬਣਤਰ jpeg_error_mgr jerr;
	jmp_buf setjmp_ਬਫਰ;
	ਬਣਤਰ bitmap * ਅਸਥਿਰ bitmap = ਨਿਰਾ;
	uint8_t * ਅਸਥਿਰ pixels = ਨਿਰਾ;
	size_t rowstride;
	ਬਣਤਰ jpeg_ਸਰੋਤ_mgr ਸਰੋਤ_mgr = {
		0,
		0,
		nsjpeg_init_ਸਰੋਤ,
		nsjpeg_fill_ਨਿਵੇਸ਼_ਬਫਰ,
		nsjpeg_skip_ਨਿਵੇਸ਼_ਅੰਕੜੇ,
		jpeg_resync_ਮੰਜ਼ਿਲ_restart,
		nsjpeg_ਪਦ_ਸਰੋਤ };

	/* obtain jpeg source data and perfom minimal sanity checks */
	ਸਰੋਤ_ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਸਰੋਤ_ਮਾਪ);

	ਜੇ ((ਸਰੋਤ_ਅੰਕੜੇ == ਨਿਰਾ) ||
	    (ਸਰੋਤ_ਮਾਪ < MIN_JPEG_ਮਾਪ)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_ਸੁਨੇਹਾ = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	ਜੇ (setjmp(setjmp_ਬਫਰ)) {
		jpeg_destroy_decompress(&cinfo);
		ਵਾਪਸ bitmap;
	}

	cinfo.client_ਅੰਕੜੇ = &setjmp_ਬਫਰ;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	ਸਰੋਤ_mgr.ਅਗਲਾ_ਨਿਵੇਸ਼_byte = ਸਰੋਤ_ਅੰਕੜੇ;
	ਸਰੋਤ_mgr.ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ = ਸਰੋਤ_ਮਾਪ;
	cinfo.src = &ਸਰੋਤ_mgr;

	/* read JPEG header information */
	jpeg_read_ਸਿਰਲੇਖ(&cinfo, TRUE);

	/* set output processing parameters */
	ਜੇ (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.ਬਾਹਰਲਾ_color_space = JCS_CMYK;
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ JCS_ALPHA_EXTENSIONS
		ਚੋਣ (bitmap_fmt.layout) {
		ਹਾਲਤ BITMAP_LAYOUT_R8G8B8A8:
			cinfo.ਬਾਹਰਲਾ_color_space = JCS_EXT_RGBA;
			ਰੋਕੋ;
		ਹਾਲਤ BITMAP_LAYOUT_B8G8R8A8:
			cinfo.ਬਾਹਰਲਾ_color_space = JCS_EXT_BGRA;
			ਰੋਕੋ;
		ਹਾਲਤ BITMAP_LAYOUT_A8R8G8B8:
			cinfo.ਬਾਹਰਲਾ_color_space = JCS_EXT_ARGB;
			ਰੋਕੋ;
		ਹਾਲਤ BITMAP_LAYOUT_A8B8G8R8:
			cinfo.ਬਾਹਰਲਾ_color_space = JCS_EXT_ABGR;
			ਰੋਕੋ;
		ਮੂਲ:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			ਵਾਪਸ ਨਿਰਾ;
		}
#ਨਹੀਂਤਾਂ
		cinfo.ਬਾਹਰਲਾ_color_space = JCS_RGB;
#ਜੇ_ਅੰਤ
	}
	cinfo.dct_ਵਿਧੀ = JDCT_ISLOW;

	/* commence the decompression, output parameters now valid */
	jpeg_ਸ਼ੁਰੂ_decompress(&cinfo);

	/* create opaque bitmap (jpegs cannot be transparent) */
	bitmap = guit->bitmap->create(
			cinfo.output_ਚੌੜਾਈ,
			cinfo.output_ਉਚਾਈ, BITMAP_OPAQUE);
	ਜੇ (bitmap == ਨਿਰਾ) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		ਵਾਪਸ ਨਿਰਾ;
	}

	pixels = guit->bitmap->get_ਬਫਰ(bitmap);
	ਜੇ (pixels == ਨਿਰਾ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	ਚੋਣ (cinfo.ਬਾਹਰਲਾ_color_space) {
	ਹਾਲਤ JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		ਰੋਕੋ;

	ਹਾਲਤ JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		ਰੋਕੋ;

	ਮੂਲ:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		ਰੋਕੋ;
	}

	guit->bitmap->modified(bitmap);

	jpeg_ਅੰਤ_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	ਵਾਪਸ bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
ਸਥਿਰ bool nsjpeg_convert(ਬਣਤਰ content *c)
{
	ਬਣਤਰ jpeg_decompress_struct cinfo;
	ਬਣਤਰ jpeg_error_mgr jerr;
	jmp_buf setjmp_ਬਫਰ;
	ਬਣਤਰ jpeg_ਸਰੋਤ_mgr ਸਰੋਤ_mgr = { 0, 0,
		nsjpeg_init_ਸਰੋਤ, nsjpeg_fill_ਨਿਵੇਸ਼_ਬਫਰ,
		nsjpeg_skip_ਨਿਵੇਸ਼_ਅੰਕੜੇ, jpeg_resync_ਮੰਜ਼ਿਲ_restart,
		nsjpeg_ਪਦ_ਸਰੋਤ };
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;

	/* check image header is valid and get width/height */
	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_ਸੁਨੇਹਾ = nsjpeg_error_log;

	ਜੇ (setjmp(setjmp_ਬਫਰ)) {
		jpeg_destroy_decompress(&cinfo);

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errorcode = NSERROR_UNKNOWN;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errormsg = nsjpeg_error_ਬਫਰ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ERROR, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਵਾਪਸ false;
	}

	cinfo.client_ਅੰਕੜੇ = &setjmp_ਬਫਰ;
	jpeg_create_decompress(&cinfo);
	ਸਰੋਤ_mgr.ਅਗਲਾ_ਨਿਵੇਸ਼_byte = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਅੰਕੜੇ;
	ਸਰੋਤ_mgr.ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ = ਮਾਪ;
	cinfo.src = &ਸਰੋਤ_mgr;
	jpeg_read_ਸਿਰਲੇਖ(&cinfo, TRUE);
	cinfo.ਬਾਹਰਲਾ_color_space = JCS_RGB;
	cinfo.dct_ਵਿਧੀ = JDCT_ISLOW;

	jpeg_calc_output_dimensions(&cinfo);

	c->ਚੌੜਾਈ = cinfo.output_ਚੌੜਾਈ;
	c->ਉਚਾਈ = cinfo.output_ਉਚਾਈ;
	c->ਮਾਪ = c->ਚੌੜਾਈ * c->ਉਚਾਈ * 4;

	jpeg_destroy_decompress(&cinfo);

	image_cache_ਜੋੜੋ(c, ਨਿਰਾ, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
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
ਸਥਿਰ nserror nsjpeg_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	ਬਣਤਰ content *jpeg_c;
	nserror error;

	jpeg_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content));
	ਜੇ (jpeg_c == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		ਵਾਪਸ error;
	}

	/* re-convert if the content is ready */
	ਜੇ ((old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) ||
	    (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		ਜੇ (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਅਚਲ content_handler nsjpeg_content_handler = {
	.create = nsjpeg_create,
	.ਅੰਕੜੇ_complete = nsjpeg_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpeg_clone,
	.get_ਅੰਦਰੂਨੀ = image_cache_get_ਅੰਦਰੂਨੀ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
