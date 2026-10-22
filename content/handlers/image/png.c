/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Richard Wilson <not_ginger_matt@hotmail.com>
 * Copyright 2008 Daniel Silverstone <dsilvers@netsurf-browser.org>
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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <png.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image_cache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ png_jmpbuf
#warning you have an antique libpng
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#ਜੇ_ਅੰਤ

#ਜੇ PNG_LIBPNG_VER < 10209
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ png_ਸੈੱਟ_expand_ਸਲੇਟੀ_1_2_4_ਮੰਜ਼ਿਲ_8(png) png_ਸੈੱਟ_ਸਲੇਟੀ_1_2_4_ਮੰਜ਼ਿਲ_8(png)
#ਜੇ_ਅੰਤ

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nspng_content {
	ਬਣਤਰ content ਅਧਾਰ; /**< base content type */

	bool no_process_ਅੰਕੜੇ; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	ਪੂਰਨ interlace;
	ਬਣਤਰ bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ interlace_ਸ਼ੁਰੂ[8] = {0, 16, 0, 8, 0, 4, 0};
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ interlace_ਕਦਮ[8] = {28, 28, 12, 12, 4, 4, 0};
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ interlace_ਪੰਕਤੀ_ਸ਼ੁਰੂ[8] = {0, 0, 4, 0, 2, 0, 1};
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ interlace_ਪੰਕਤੀ_ਕਦਮ[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
ਗਿਣਤੀ nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ਸਥਿਰ ਖਾਲੀ nspng_warning(png_structp png_ptr, png_const_charp warning_ਸੁਨੇਹਾ)
{
	NSLOG(netsurf, INFO, "%s", warning_ਸੁਨੇਹਾ);
}

/**
 * nspng_error -- callback for libpng errors
 */
ਸਥਿਰ ਖਾਲੀ nspng_error(png_structp png_ptr, png_const_charp error_ਸੁਨੇਹਾ)
{
	NSLOG(netsurf, INFO, "%s", error_ਸੁਨੇਹਾ);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ਸਥਿਰ ਖਾਲੀ nspng_ਤਿਆਰੀ_transforms(png_structp png_ptr, png_infop info_ptr)
{
	ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ_ਡੂੰਘਾਈ, color_type, intent;
	ਦੁੱਗਣਾ gamma;

	ਦੋਆਧਾਰੀ_ਅੰਕ_ਡੂੰਘਾਈ = png_get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਡੂੰਘਾਈ(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	ਜੇ (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_ਸੈੱਟ_palette_ਮੰਜ਼ਿਲ_rgb(png_ptr);
	}

	ਜੇ ((color_type == PNG_COLOR_TYPE_ਸਲੇਟੀ) && (ਦੋਆਧਾਰੀ_ਅੰਕ_ਡੂੰਘਾਈ < 8)) {
		png_ਸੈੱਟ_expand_ਸਲੇਟੀ_1_2_4_ਮੰਜ਼ਿਲ_8(png_ptr);
	}

	ਜੇ (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_ਸੈੱਟ_t_RNS_ਮੰਜ਼ਿਲ_alpha(png_ptr);
	}

	ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਡੂੰਘਾਈ == 16) {
		png_ਸੈੱਟ_strip_16(png_ptr);
	}

	ਜੇ (color_type == PNG_COLOR_TYPE_ਸਲੇਟੀ ||
	    color_type == PNG_COLOR_TYPE_ਸਲੇਟੀ_ALPHA) {
		png_ਸੈੱਟ_ਸਲੇਟੀ_ਮੰਜ਼ਿਲ_rgb(png_ptr);
	}

	ਚੋਣ (bitmap_fmt.layout) {
	ਹਾਲਤ BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	ਹਾਲਤ BITMAP_LAYOUT_A8B8G8R8:
		png_ਸੈੱਟ_bgr(png_ptr);
		ਰੋਕੋ;
	ਮੂਲ:
		/* RGB is the default. */
		ਰੋਕੋ;
	}

	ਜੇ (!(color_type & PNG_COLOR_ਨਕਾਬ_ALPHA)) {
		ਚੋਣ (bitmap_fmt.layout) {
		ਹਾਲਤ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ਹਾਲਤ BITMAP_LAYOUT_A8B8G8R8:
			png_ਸੈੱਟ_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			ਰੋਕੋ;

		ਮੂਲ:
			png_ਸੈੱਟ_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			ਰੋਕੋ;
		}
	} ਨਹੀਂਤਾਂ {
		ਚੋਣ (bitmap_fmt.layout) {
		ਹਾਲਤ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ਹਾਲਤ BITMAP_LAYOUT_A8B8G8R8:
			png_ਸੈੱਟ_ਅਦਲਾ_ਬਦਲੀ_alpha(png_ptr);
			ਰੋਕੋ;
		ਮੂਲ:
			/* Alpha as final component is the default. */
			ਰੋਕੋ;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	ਜੇ (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_ਸੈੱਟ_gamma(png_ptr, 2.2, 0.45455);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_ਸੈੱਟ_gamma(png_ptr, 2.2, gamma);
		} ਨਹੀਂਤਾਂ {
			png_ਸੈੱਟ_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ਸਥਿਰ ਖਾਲੀ info_callback(png_structp png_s, png_infop info)
{
	ਪੂਰਨ interlace;
	png_uint_32 ਚੌੜਾਈ, ਉਚਾਈ;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	ਚੌੜਾਈ = png_get_image_ਚੌੜਾਈ(png_s, info);
	ਉਚਾਈ = png_get_image_ਉਚਾਈ(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->ਅਧਾਰ.ਚੌੜਾਈ = ਚੌੜਾਈ;
	png_c->ਅਧਾਰ.ਉਚਾਈ = ਉਚਾਈ;
	png_c->ਅਧਾਰ.ਮਾਪ += ਚੌੜਾਈ * ਉਚਾਈ * 4;

	/* see if progressive-conversion should continue */
	ਜੇ (image_cache_speculate((ਬਣਤਰ content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(ਚੌੜਾਈ, ਉਚਾਈ, BITMAP_NONE);
	ਜੇ (png_c->bitmap == ਨਿਰਾ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = ਆਕਾਰ(uint32_t);

	nspng_ਤਿਆਰੀ_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਚੌੜਾਈ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਉਚਾਈ, png_c->rowbytes);
}

ਸਥਿਰ ਖਾਲੀ ਪੰਕਤੀ_callback(png_structp png_s, png_bytep new_ਪੰਕਤੀ,
			 png_uint_32 ਪੰਕਤੀ_ਅੰਕ, ਪੂਰਨ pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ rowbytes = png_c->rowbytes;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਫਰ, *ਪੰਕਤੀ;

	/* Give up if there's no bitmap */
	ਜੇ (png_c->bitmap == ਨਿਰਾ)
		ਵਾਪਸ;

	/* Abort if we've not got any data */
	ਜੇ (new_ਪੰਕਤੀ == ਨਿਰਾ)
		ਵਾਪਸ;

	/* Get bitmap buffer */
	ਬਫਰ = guit->bitmap->get_ਬਫਰ(png_c->bitmap);
	ਜੇ (ਬਫਰ == ਨਿਰਾ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	ਪੰਕਤੀ = ਬਫਰ + (png_c->rowstride * ਪੰਕਤੀ_ਅੰਕ);

	/* Handle interlaced sprites using the Adam7 algorithm */
	ਜੇ (png_c->interlace) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ dst_off;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ src_off = 0;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ੁਰੂ, ਕਦਮ;

		ਸ਼ੁਰੂ = interlace_ਸ਼ੁਰੂ[pass];
		ਕਦਮ = interlace_ਕਦਮ[pass];
		ਪੰਕਤੀ_ਅੰਕ = interlace_ਪੰਕਤੀ_ਸ਼ੁਰੂ[pass] +
			interlace_ਪੰਕਤੀ_ਕਦਮ[pass] * ਪੰਕਤੀ_ਅੰਕ;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		ਪੰਕਤੀ = ਬਫਰ + (png_c->rowstride * ਪੰਕਤੀ_ਅੰਕ);

		ਲਈ (dst_off = ਸ਼ੁਰੂ; dst_off < rowbytes; dst_off += ਕਦਮ) {
			ਪੰਕਤੀ[dst_off++] = new_ਪੰਕਤੀ[src_off++];
			ਪੰਕਤੀ[dst_off++] = new_ਪੰਕਤੀ[src_off++];
			ਪੰਕਤੀ[dst_off++] = new_ਪੰਕਤੀ[src_off++];
			ਪੰਕਤੀ[dst_off++] = new_ਪੰਕਤੀ[src_off++];
		}
	} ਨਹੀਂਤਾਂ {
		/* Do a fast memcpy of the row data */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਪੰਕਤੀ, new_ਪੰਕਤੀ, rowbytes);
	}
}


ਸਥਿਰ ਖਾਲੀ ਸਿਰਾ_callback(png_structp png_s, png_infop info)
{
}

ਸਥਿਰ nserror nspng_create_png_ਅੰਕੜੇ(nspng_content *png_c)
{
	png_c->bitmap = ਨਿਰਾ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	ਜੇ (png_c->png == ਨਿਰਾ) {
		content_ਪ੍ਰਸਾਰਣ_error(&png_c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	png_ਸੈੱਟ_error_fn(png_c->png, ਨਿਰਾ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	ਜੇ (png_c->info == ਨਿਰਾ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_ਪ੍ਰਸਾਰਣ_error(&png_c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ਨਿਰਾ;
		png_c->info = ਨਿਰਾ;

		content_ਪ੍ਰਸਾਰਣ_error(&png_c->ਅਧਾਰ, NSERROR_PNG_ERROR, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	png_ਸੈੱਟ_progressive_read_fn(png_c->png, png_c,
			info_callback, ਪੰਕਤੀ_callback, ਸਿਰਾ_callback);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror
nspng_create(ਅਚਲ content_handler *handler,
	     lwc_string *imime_type,
	     ਅਚਲ ਬਣਤਰ http_parameter *params,
	     ਬਣਤਰ llcache_handle *llcache,
	     ਅਚਲ ਅੱਖਰ *fallback_charset,
	     bool quirks,
	     ਬਣਤਰ content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nspng_content));
	ਜੇ (png_c == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&png_c->ਅਧਾਰ, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(png_c);
		ਵਾਪਸ error;
	}

	error = nspng_create_png_ਅੰਕੜੇ(png_c);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(png_c);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *)png_c;

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ bool nspng_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	nspng_content *png_c = (nspng_content *)c;
	ਅਸਥਿਰ bool ret = true;

	ਜੇ (png_c->no_process_ਅੰਕੜੇ) {
		ਵਾਪਸ ret;
	}

	ਚੋਣ (setjmp(png_jmpbuf(png_c->png))) {
	ਹਾਲਤ CBERR_NONE: /* direct return */	
		png_process_ਅੰਕੜੇ(png_c->png, png_c->info, (uint8_t *)ਅੰਕੜੇ, ਮਾਪ);
		ਰੋਕੋ;

	ਹਾਲਤ CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_ਅੰਕੜੇ = true;
		ਰੋਕੋ;

	ਮੂਲ: /* fatal error from library processing png */
		ਜੇ (png_c->bitmap != ਨਿਰਾ) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_ਅੰਕੜੇ = true;
		} ਨਹੀਂਤਾਂ {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ਨਿਰਾ;
			png_c->info = ਨਿਰਾ;

			content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_PNG_ERROR, ਨਿਰਾ);

			ret = false;

		}
		ਰੋਕੋ;
	}

	ਵਾਪਸ ret;
}

ਬਣਤਰ png_cache_read_ਅੰਕੜੇ_s {
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
};

/** PNG library read fucntion to read data from a memory array 
 */
ਸਥਿਰ ਖਾਲੀ 
png_cache_read_fn(png_structp png_ptr, png_bytep ਅੰਕੜੇ, png_ਮਾਪ_t ਲੰਮਾਈ)
{
	ਬਣਤਰ png_cache_read_ਅੰਕੜੇ_s *png_cache_read_ਅੰਕੜੇ;
	png_cache_read_ਅੰਕੜੇ = png_get_io_ptr(png_ptr);

	ਜੇ (ਲੰਮਾਈ > png_cache_read_ਅੰਕੜੇ->ਮਾਪ) {
		ਲੰਮਾਈ = png_cache_read_ਅੰਕੜੇ->ਮਾਪ;
	}

	ਜੇ (ਲੰਮਾਈ == 0) {
		png_error(png_ptr, "Read Error");
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, png_cache_read_ਅੰਕੜੇ->ਅੰਕੜੇ, ਲੰਮਾਈ);

	png_cache_read_ਅੰਕੜੇ->ਅੰਕੜੇ += ਲੰਮਾਈ;
	png_cache_read_ਅੰਕੜੇ->ਮਾਪ -= ਲੰਮਾਈ;
}

/** calculate an array of row pointers into a bitmap data area
 */
ਸਥਿਰ png_bytep *calc_ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ(ਬਣਤਰ bitmap *bitmap)
{
	ਪੂਰਨ ਉਚਾਈ = guit->bitmap->get_ਉਚਾਈ(bitmap);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਫਰ= guit->bitmap->get_ਬਫਰ(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *ਪੰਕਤੀ_ptrs;
	ਪੂਰਨ hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	ਜੇ (ਬਫਰ == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਪੰਕਤੀ_ptrs = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(png_bytep) * ਉਚਾਈ);

	ਜੇ (ਪੰਕਤੀ_ptrs != ਨਿਰਾ) {
		ਲਈ (hloop = 0; hloop < ਉਚਾਈ; hloop++) {
			ਪੰਕਤੀ_ptrs[hloop] = ਬਫਰ + (rowstride * hloop);
		}
	}

	ਵਾਪਸ ਪੰਕਤੀ_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ਸਥਿਰ ਬਣਤਰ bitmap *
png_cache_convert(ਬਣਤਰ content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop ਸਿਰਾ_info_ptr;
	ਅਸਥਿਰ ਬਣਤਰ bitmap * ਅਸਥਿਰ bitmap = ਨਿਰਾ;
	ਬਣਤਰ png_cache_read_ਅੰਕੜੇ_s png_cache_read_ਅੰਕੜੇ;
	png_uint_32 ਚੌੜਾਈ, ਉਚਾਈ;
	ਅਸਥਿਰ png_bytep * ਅਸਥਿਰ ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ = ਨਿਰਾ;

	png_cache_read_ਅੰਕੜੇ.ਅੰਕੜੇ = 
		content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &png_cache_read_ਅੰਕੜੇ.ਮਾਪ);

	ਜੇ ((png_cache_read_ਅੰਕੜੇ.ਅੰਕੜੇ == ਨਿਰਾ) || 
	    (png_cache_read_ਅੰਕੜੇ.ਮਾਪ <= 8)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ਨਿਰਾ,
			nspng_error, nspng_warning);
	ਜੇ (png_ptr == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	ਜੇ (info_ptr == ਨਿਰਾ) {
		png_destroy_read_struct(&png_ptr, ਨਿਰਾ, ਨਿਰਾ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਸਿਰਾ_info_ptr = png_create_info_struct(png_ptr);
	ਜੇ (ਸਿਰਾ_info_ptr == ਨਿਰਾ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ਨਿਰਾ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* setup error exit path */
	ਜੇ (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		ਜਾਓ png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_ਸੈੱਟ_read_fn(png_ptr, &png_cache_read_ਅੰਕੜੇ, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_ਤਿਆਰੀ_transforms(png_ptr, info_ptr);

	ਚੌੜਾਈ = png_get_image_ਚੌੜਾਈ(png_ptr, info_ptr);
	ਉਚਾਈ = png_get_image_ਉਚਾਈ(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(ਚੌੜਾਈ, ਉਚਾਈ, BITMAP_NONE);
	ਜੇ (bitmap == ਨਿਰਾ) {
		/* cleanup and bail */
		ਜਾਓ png_cache_convert_error;
	}

	ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ = calc_ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ((ਬਣਤਰ bitmap *) bitmap);

	ਜੇ (ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ != ਨਿਰਾ) {
		png_read_image(png_ptr, (png_bytep *) ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ);
	} ਨਹੀਂਤਾਂ {
		guit->bitmap->destroy((ਬਣਤਰ bitmap *)bitmap);
		bitmap = ਨਿਰਾ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &ਸਿਰਾ_info_ptr);

	ਜੇ (ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ != ਨਿਰਾ) {
		ਮੁਕਤ((png_bytep *) ਪੰਕਤੀ_ਸੂਚਕ_ਸਾਰੇ);
	}

	ਜੇ (bitmap != ਨਿਰਾ) {
		bool opaque = bitmap_test_opaque((ਖਾਲੀ *)bitmap);
		guit->bitmap->ਸੈੱਟ_opaque((ਖਾਲੀ *)bitmap, opaque);
		bitmap_format_ਮੰਜ਼ਿਲ_client((ਖਾਲੀ *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((ਖਾਲੀ *)bitmap);
	}

	ਵਾਪਸ (ਬਣਤਰ bitmap *)bitmap;
}

ਸਥਿਰ bool nspng_convert(ਬਣਤਰ content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	ਅੱਖਰ *title;

	ਪ੍ਰਤਿਜ੍ਞਾ(png_c->png != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(png_c->info != ਨਿਰਾ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	ਜੇ (png_c->bitmap != ਨਿਰਾ) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->ਸੈੱਟ_opaque(png_c->bitmap, opaque);
		bitmap_format_ਮੰਜ਼ਿਲ_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_ਜੋੜੋ(c, png_c->bitmap, png_cache_convert);

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, "");

	ਵਾਪਸ true;
}


ਸਥਿਰ nserror nspng_clone(ਅਚਲ ਬਣਤਰ content *old_c, ਬਣਤਰ content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;

	clone_png_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nspng_content));
	ਜੇ (clone_png_c == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_ਅੰਕੜੇ(clone_png_c);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(&clone_png_c->ਅਧਾਰ, &ਮਾਪ);
	ਜੇ (ਮਾਪ > 0) {
		ਜੇ (nspng_process_ਅੰਕੜੇ(&clone_png_c->ਅਧਾਰ, (ਅਚਲ ਅੱਖਰ *)ਅੰਕੜੇ, ਮਾਪ) == false) {
			content_destroy(&clone_png_c->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ਜੇ ((old_c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) ||
	    (old_c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		ਜੇ (nspng_convert(&clone_png_c->ਅਧਾਰ) == false) {
			content_destroy(&clone_png_c->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ਬਣਤਰ content *)clone_png_c;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਅਚਲ content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_ਅੰਕੜੇ = nspng_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_ਅੰਦਰੂਨੀ = image_cache_get_ਅੰਦਰੂਨੀ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nspng, nspng_types, nspng_content_handler);
