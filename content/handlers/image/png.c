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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <png.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"
#ಅನ್ತರ್ಭಾವ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ png_jmpbuf
#warning you have an antique libpng
#ವ್ಯಾಖ್ಯಾನಿಸು png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ PNG_LIBPNG_VER < 10209
#ವ್ಯಾಖ್ಯಾನಿಸು png_ಗಣ_expand_ಬೂದು_1_2_4_ಗಮ್ಯ_8(png) png_ಗಣ_ಬೂದು_1_2_4_ಗಮ್ಯ_8(png)
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nspng_content {
	ರಚನೆ content ತಳ; /**< base content type */

	bool no_process_ದತ್ತಾಂಶ; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	ಪೂರ್ಣಾಂಕ interlace;
	ರಚನೆ bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ interlace_ಆರಂಭ[8] = {0, 16, 0, 8, 0, 4, 0};
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ interlace_ಹೆಜ್ಜೆ[8] = {28, 28, 12, 12, 4, 4, 0};
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ interlace_ಅಡ್ಡಸಾಲು_ಆರಂಭ[8] = {0, 0, 4, 0, 2, 0, 1};
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ interlace_ಅಡ್ಡಸಾಲು_ಹೆಜ್ಜೆ[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
ಎಣಿಕೆ nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ಸ್ಥಿರ ಶೂನ್ಯ nspng_warning(png_structp png_ptr, png_const_charp warning_ಸಂದೇಶ)
{
	NSLOG(netsurf, INFO, "%s", warning_ಸಂದೇಶ);
}

/**
 * nspng_error -- callback for libpng errors
 */
ಸ್ಥಿರ ಶೂನ್ಯ nspng_error(png_structp png_ptr, png_const_charp error_ಸಂದೇಶ)
{
	NSLOG(netsurf, INFO, "%s", error_ಸಂದೇಶ);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ಸ್ಥಿರ ಶೂನ್ಯ nspng_ಸಿದ್ಧತೆ_transforms(png_structp png_ptr, png_infop info_ptr)
{
	ಪೂರ್ಣಾಂಕ ಬಿಟ್_ಆಳ, color_type, intent;
	ದ್ವಿಗುಣ gamma;

	ಬಿಟ್_ಆಳ = png_get_ಬಿಟ್_ಆಳ(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	ಆದರೆ (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_ಗಣ_palette_ಗಮ್ಯ_rgb(png_ptr);
	}

	ಆದರೆ ((color_type == PNG_COLOR_TYPE_ಬೂದು) && (ಬಿಟ್_ಆಳ < 8)) {
		png_ಗಣ_expand_ಬೂದು_1_2_4_ಗಮ್ಯ_8(png_ptr);
	}

	ಆದರೆ (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_ಗಣ_t_RNS_ಗಮ್ಯ_alpha(png_ptr);
	}

	ಆದರೆ (ಬಿಟ್_ಆಳ == 16) {
		png_ಗಣ_strip_16(png_ptr);
	}

	ಆದರೆ (color_type == PNG_COLOR_TYPE_ಬೂದು ||
	    color_type == PNG_COLOR_TYPE_ಬೂದು_ALPHA) {
		png_ಗಣ_ಬೂದು_ಗಮ್ಯ_rgb(png_ptr);
	}

	ಆಯ್ಕೆ (bitmap_fmt.layout) {
	ಸ್ಥಿತಿ BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	ಸ್ಥಿತಿ BITMAP_LAYOUT_A8B8G8R8:
		png_ಗಣ_bgr(png_ptr);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* RGB is the default. */
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (!(color_type & PNG_COLOR_ಮುಖವಾಡ_ALPHA)) {
		ಆಯ್ಕೆ (bitmap_fmt.layout) {
		ಸ್ಥಿತಿ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ಸ್ಥಿತಿ BITMAP_LAYOUT_A8B8G8R8:
			png_ಗಣ_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			png_ಗಣ_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			ನಿಲ್ಲಿಸು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆಯ್ಕೆ (bitmap_fmt.layout) {
		ಸ್ಥಿತಿ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ಸ್ಥಿತಿ BITMAP_LAYOUT_A8B8G8R8:
			png_ಗಣ_ಅದಲುಬದಲು_alpha(png_ptr);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			/* Alpha as final component is the default. */
			ನಿಲ್ಲಿಸು;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	ಆದರೆ (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_ಗಣ_gamma(png_ptr, 2.2, 0.45455);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_ಗಣ_gamma(png_ptr, 2.2, gamma);
		} ಇಲ್ಲದಿದ್ದರೆ {
			png_ಗಣ_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ಸ್ಥಿರ ಶೂನ್ಯ info_callback(png_structp png_s, png_infop info)
{
	ಪೂರ್ಣಾಂಕ interlace;
	png_uint_32 ಅಗಲ, ಎತ್ತರ;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	ಅಗಲ = png_get_image_ಅಗಲ(png_s, info);
	ಎತ್ತರ = png_get_image_ಎತ್ತರ(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->ತಳ.ಅಗಲ = ಅಗಲ;
	png_c->ತಳ.ಎತ್ತರ = ಎತ್ತರ;
	png_c->ತಳ.ಗಾತ್ರ_2 += ಅಗಲ * ಎತ್ತರ * 4;

	/* see if progressive-conversion should continue */
	ಆದರೆ (image_cache_speculate((ರಚನೆ content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(ಅಗಲ, ಎತ್ತರ, BITMAP_NONE);
	ಆದರೆ (png_c->bitmap == ಖಾಲಿ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = ಗಾತ್ರ(uint32_t);

	nspng_ಸಿದ್ಧತೆ_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಅಗಲ, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಎತ್ತರ, png_c->rowbytes);
}

ಸ್ಥಿರ ಶೂನ್ಯ ಅಡ್ಡಸಾಲು_callback(png_structp png_s, png_bytep new_ಅಡ್ಡಸಾಲು,
			 png_uint_32 ಅಡ್ಡಸಾಲು_ಅಂಕೆ, ಪೂರ್ಣಾಂಕ pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ rowbytes = png_c->rowbytes;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಫರ್, *ಅಡ್ಡಸಾಲು;

	/* Give up if there's no bitmap */
	ಆದರೆ (png_c->bitmap == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	/* Abort if we've not got any data */
	ಆದರೆ (new_ಅಡ್ಡಸಾಲು == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	/* Get bitmap buffer */
	ಬಫರ್ = guit->bitmap->get_ಬಫರ್(png_c->bitmap);
	ಆದರೆ (ಬಫರ್ == ಖಾಲಿ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	ಅಡ್ಡಸಾಲು = ಬಫರ್ + (png_c->rowstride * ಅಡ್ಡಸಾಲು_ಅಂಕೆ);

	/* Handle interlaced sprites using the Adam7 algorithm */
	ಆದರೆ (png_c->interlace) {
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ dst_off;
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ src_off = 0;
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆರಂಭ, ಹೆಜ್ಜೆ;

		ಆರಂಭ = interlace_ಆರಂಭ[pass];
		ಹೆಜ್ಜೆ = interlace_ಹೆಜ್ಜೆ[pass];
		ಅಡ್ಡಸಾಲು_ಅಂಕೆ = interlace_ಅಡ್ಡಸಾಲು_ಆರಂಭ[pass] +
			interlace_ಅಡ್ಡಸಾಲು_ಹೆಜ್ಜೆ[pass] * ಅಡ್ಡಸಾಲು_ಅಂಕೆ;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		ಅಡ್ಡಸಾಲು = ಬಫರ್ + (png_c->rowstride * ಅಡ್ಡಸಾಲು_ಅಂಕೆ);

		ಗಾಗಿ (dst_off = ಆರಂಭ; dst_off < rowbytes; dst_off += ಹೆಜ್ಜೆ) {
			ಅಡ್ಡಸಾಲು[dst_off++] = new_ಅಡ್ಡಸಾಲು[src_off++];
			ಅಡ್ಡಸಾಲು[dst_off++] = new_ಅಡ್ಡಸಾಲು[src_off++];
			ಅಡ್ಡಸಾಲು[dst_off++] = new_ಅಡ್ಡಸಾಲು[src_off++];
			ಅಡ್ಡಸಾಲು[dst_off++] = new_ಅಡ್ಡಸಾಲು[src_off++];
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Do a fast memcpy of the row data */
		ಸ್ಮರಣೆ_ನಕಲು(ಅಡ್ಡಸಾಲು, new_ಅಡ್ಡಸಾಲು, rowbytes);
	}
}


ಸ್ಥಿರ ಶೂನ್ಯ ಕೊನೆ_callback(png_structp png_s, png_infop info)
{
}

ಸ್ಥಿರ nserror nspng_create_png_ದತ್ತಾಂಶ(nspng_content *png_c)
{
	png_c->bitmap = ಖಾಲಿ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	ಆದರೆ (png_c->png == ಖಾಲಿ) {
		content_ಪ್ರಸಾರ_error(&png_c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	png_ಗಣ_error_fn(png_c->png, ಖಾಲಿ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	ಆದರೆ (png_c->info == ಖಾಲಿ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_ಪ್ರಸಾರ_error(&png_c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ಖಾಲಿ;
		png_c->info = ಖಾಲಿ;

		content_ಪ್ರಸಾರ_error(&png_c->ತಳ, NSERROR_PNG_ERROR, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	png_ಗಣ_progressive_read_fn(png_c->png, png_c,
			info_callback, ಅಡ್ಡಸಾಲು_callback, ಕೊನೆ_callback);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror
nspng_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
	     lwc_string *imime_type,
	     ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
	     ರಚನೆ llcache_handle *llcache,
	     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	     bool quirks,
	     ರಚನೆ content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nspng_content));
	ಆದರೆ (png_c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&png_c->ತಳ, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(png_c);
		ಹಿಂತಿರುಗಿ error;
	}

	error = nspng_create_png_ದತ್ತಾಂಶ(png_c);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(png_c);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *)png_c;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ bool nspng_process_ದತ್ತಾಂಶ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ,
			       ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	nspng_content *png_c = (nspng_content *)c;
	ಅಸ್ಥಿರ bool ret = true;

	ಆದರೆ (png_c->no_process_ದತ್ತಾಂಶ) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆಯ್ಕೆ (setjmp(png_jmpbuf(png_c->png))) {
	ಸ್ಥಿತಿ CBERR_NONE: /* direct return */	
		png_process_ದತ್ತಾಂಶ(png_c->png, png_c->info, (uint8_t *)ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_ದತ್ತಾಂಶ = true;
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ: /* fatal error from library processing png */
		ಆದರೆ (png_c->bitmap != ಖಾಲಿ) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_ದತ್ತಾಂಶ = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ಖಾಲಿ;
			png_c->info = ಖಾಲಿ;

			content_ಪ್ರಸಾರ_error(c, NSERROR_PNG_ERROR, ಖಾಲಿ);

			ret = false;

		}
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ret;
}

ರಚನೆ png_cache_read_ದತ್ತಾಂಶ_s {
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
};

/** PNG library read fucntion to read data from a memory array 
 */
ಸ್ಥಿರ ಶೂನ್ಯ 
png_cache_read_fn(png_structp png_ptr, png_bytep ದತ್ತಾಂಶ, png_ಗಾತ್ರ_t ಉದ್ದಳತೆ)
{
	ರಚನೆ png_cache_read_ದತ್ತಾಂಶ_s *png_cache_read_ದತ್ತಾಂಶ;
	png_cache_read_ದತ್ತಾಂಶ = png_get_io_ptr(png_ptr);

	ಆದರೆ (ಉದ್ದಳತೆ > png_cache_read_ದತ್ತಾಂಶ->ಗಾತ್ರ_2) {
		ಉದ್ದಳತೆ = png_cache_read_ದತ್ತಾಂಶ->ಗಾತ್ರ_2;
	}

	ಆದರೆ (ಉದ್ದಳತೆ == 0) {
		png_error(png_ptr, "Read Error");
	}

	ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, png_cache_read_ದತ್ತಾಂಶ->ದತ್ತಾಂಶ, ಉದ್ದಳತೆ);

	png_cache_read_ದತ್ತಾಂಶ->ದತ್ತಾಂಶ += ಉದ್ದಳತೆ;
	png_cache_read_ದತ್ತಾಂಶ->ಗಾತ್ರ_2 -= ಉದ್ದಳತೆ;
}

/** calculate an array of row pointers into a bitmap data area
 */
ಸ್ಥಿರ png_bytep *calc_ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು(ರಚನೆ bitmap *bitmap)
{
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ = guit->bitmap->get_ಎತ್ತರ(bitmap);
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಫರ್= guit->bitmap->get_ಬಫರ್(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *ಅಡ್ಡಸಾಲು_ptrs;
	ಪೂರ್ಣಾಂಕ hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	ಆದರೆ (ಬಫರ್ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಅಡ್ಡಸಾಲು_ptrs = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(png_bytep) * ಎತ್ತರ);

	ಆದರೆ (ಅಡ್ಡಸಾಲು_ptrs != ಖಾಲಿ) {
		ಗಾಗಿ (hloop = 0; hloop < ಎತ್ತರ; hloop++) {
			ಅಡ್ಡಸಾಲು_ptrs[hloop] = ಬಫರ್ + (rowstride * hloop);
		}
	}

	ಹಿಂತಿರುಗಿ ಅಡ್ಡಸಾಲು_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ಸ್ಥಿರ ರಚನೆ bitmap *
png_cache_convert(ರಚನೆ content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop ಕೊನೆ_info_ptr;
	ಅಸ್ಥಿರ ರಚನೆ bitmap * ಅಸ್ಥಿರ bitmap = ಖಾಲಿ;
	ರಚನೆ png_cache_read_ದತ್ತಾಂಶ_s png_cache_read_ದತ್ತಾಂಶ;
	png_uint_32 ಅಗಲ, ಎತ್ತರ;
	ಅಸ್ಥಿರ png_bytep * ಅಸ್ಥಿರ ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು = ಖಾಲಿ;

	png_cache_read_ದತ್ತಾಂಶ.ದತ್ತಾಂಶ = 
		content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &png_cache_read_ದತ್ತಾಂಶ.ಗಾತ್ರ_2);

	ಆದರೆ ((png_cache_read_ದತ್ತಾಂಶ.ದತ್ತಾಂಶ == ಖಾಲಿ) || 
	    (png_cache_read_ದತ್ತಾಂಶ.ಗಾತ್ರ_2 <= 8)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ಖಾಲಿ,
			nspng_error, nspng_warning);
	ಆದರೆ (png_ptr == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	ಆದರೆ (info_ptr == ಖಾಲಿ) {
		png_destroy_read_struct(&png_ptr, ಖಾಲಿ, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಕೊನೆ_info_ptr = png_create_info_struct(png_ptr);
	ಆದರೆ (ಕೊನೆ_info_ptr == ಖಾಲಿ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* setup error exit path */
	ಆದರೆ (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		ಹೋಗು png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_ಗಣ_read_fn(png_ptr, &png_cache_read_ದತ್ತಾಂಶ, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_ಸಿದ್ಧತೆ_transforms(png_ptr, info_ptr);

	ಅಗಲ = png_get_image_ಅಗಲ(png_ptr, info_ptr);
	ಎತ್ತರ = png_get_image_ಎತ್ತರ(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(ಅಗಲ, ಎತ್ತರ, BITMAP_NONE);
	ಆದರೆ (bitmap == ಖಾಲಿ) {
		/* cleanup and bail */
		ಹೋಗು png_cache_convert_error;
	}

	ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು = calc_ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು((ರಚನೆ bitmap *) bitmap);

	ಆದರೆ (ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು != ಖಾಲಿ) {
		png_read_image(png_ptr, (png_bytep *) ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು);
	} ಇಲ್ಲದಿದ್ದರೆ {
		guit->bitmap->destroy((ರಚನೆ bitmap *)bitmap);
		bitmap = ಖಾಲಿ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &ಕೊನೆ_info_ptr);

	ಆದರೆ (ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ((png_bytep *) ಅಡ್ಡಸಾಲು_ಸೂಚಕಗಳು);
	}

	ಆದರೆ (bitmap != ಖಾಲಿ) {
		bool opaque = bitmap_test_opaque((ಶೂನ್ಯ *)bitmap);
		guit->bitmap->ಗಣ_opaque((ಶೂನ್ಯ *)bitmap, opaque);
		bitmap_format_ಗಮ್ಯ_client((ಶೂನ್ಯ *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((ಶೂನ್ಯ *)bitmap);
	}

	ಹಿಂತಿರುಗಿ (ರಚನೆ bitmap *)bitmap;
}

ಸ್ಥಿರ bool nspng_convert(ರಚನೆ content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	ಅಕ್ಷರ *title;

	ಪ್ರತಿಜ್ಞಾ(png_c->png != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(png_c->info != ಖಾಲಿ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_ಎಲೆ(llcache_handle_get_url(c->llcache)),
			c->ಅಗಲ, c->ಎತ್ತರ);
	ಆದರೆ (title != ಖಾಲಿ) {
		content_ಗಣ_title(c, title);
		ಬಿಡುಗಡೆ(title);
	}

	ಆದರೆ (png_c->bitmap != ಖಾಲಿ) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->ಗಣ_opaque(png_c->bitmap, opaque);
		bitmap_format_ಗಮ್ಯ_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_ಕೂಡಿಸು(c, png_c->bitmap, png_cache_convert);

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
	content_ಗಣ_ಸ್ಥಿತಿ(c, "");

	ಹಿಂತಿರುಗಿ true;
}


ಸ್ಥಿರ nserror nspng_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old_c, ರಚನೆ content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;

	clone_png_c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nspng_content));
	ಆದರೆ (clone_png_c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_ದತ್ತಾಂಶ(clone_png_c);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(&clone_png_c->ತಳ, &ಗಾತ್ರ_2);
	ಆದರೆ (ಗಾತ್ರ_2 > 0) {
		ಆದರೆ (nspng_process_ದತ್ತಾಂಶ(&clone_png_c->ತಳ, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ದತ್ತಾಂಶ, ಗಾತ್ರ_2) == false) {
			content_destroy(&clone_png_c->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ಆದರೆ ((old_c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) ||
	    (old_c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		ಆದರೆ (nspng_convert(&clone_png_c->ತಳ) == false) {
			content_destroy(&clone_png_c->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ರಚನೆ content *)clone_png_c;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_ದತ್ತಾಂಶ = nspng_process_ದತ್ತಾಂಶ,
	.ದತ್ತಾಂಶ_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_ಆಂತರಿಕ = image_cache_get_ಆಂತರಿಕ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nspng, nspng_types, nspng_content_handler);
