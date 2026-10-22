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

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#ಯದಿ PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nspng_content {
	ಸಂರಚನಾ content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	ಪೂರ್ಣಾಙ್ಕ interlace;
	ಸಂರಚನಾ bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
ಗಣನಾ nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ಸ್ಥಿರ ಶೂನ್ಯ nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
ಸ್ಥಿರ ಶೂನ್ಯ nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ಸ್ಥಿರ ಶೂನ್ಯ nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	ಪೂರ್ಣಾಙ್ಕ bit_depth, color_type, intent;
	ದ್ವಿಗುಣ gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	ಯದಿ (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	ಯದಿ ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	ಯದಿ (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	ಯದಿ (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	ಯದಿ (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	ವಿಕಲ್ಪ (bitmap_fmt.layout) {
	ಪಕ್ಷ BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	ಪಕ್ಷ BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* RGB is the default. */
		ಭಙ್ಗ;
	}

	ಯದಿ (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		ವಿಕಲ್ಪ (bitmap_fmt.layout) {
		ಪಕ್ಷ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ಪಕ್ಷ BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			ಭಙ್ಗ;
		}
	} ಅನ್ಯಥಾ {
		ವಿಕಲ್ಪ (bitmap_fmt.layout) {
		ಪಕ್ಷ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ಪಕ್ಷ BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			/* Alpha as final component is the default. */
			ಭಙ್ಗ;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	ಯದಿ (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} ಅನ್ಯಥಾ {
		ಯದಿ (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} ಅನ್ಯಥಾ {
			png_set_gamma(png_ptr, 2.2, 0.45455);
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
	ಪೂರ್ಣಾಙ್ಕ interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	ಯದಿ (image_cache_speculate((ಸಂರಚನಾ content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	ಯದಿ (png_c->bitmap == ರಿಕ್ತ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = ಪರಿಮಾಣ(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

ಸ್ಥಿರ ಶೂನ್ಯ row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned ವರ್ಣ *buffer, *row;

	/* Give up if there's no bitmap */
	ಯದಿ (png_c->bitmap == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	/* Abort if we've not got any data */
	ಯದಿ (new_row == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	ಯದಿ (buffer == ರಿಕ್ತ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_ಅಂಕೆ);

	/* Handle interlaced sprites using the Adam7 algorithm */
	ಯದಿ (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned ಪೂರ್ಣಾಙ್ಕ start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_ಅಂಕೆ = interlace_row_start[pass] +
			interlace_row_step[pass] * row_ಅಂಕೆ;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_ಅಂಕೆ);

		ಪ್ರತಿ (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} ಅನ್ಯಥಾ {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


ಸ್ಥಿರ ಶೂನ್ಯ end_callback(png_structp png_s, png_infop info)
{
}

ಸ್ಥಿರ nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = ರಿಕ್ತ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	ಯದಿ (png_c->png == ರಿಕ್ತ) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, ರಿಕ್ತ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	ಯದಿ (png_c->info == ರಿಕ್ತ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ರಿಕ್ತ;
		png_c->info = ರಿಕ್ತ;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ nserror
nspng_create(ಧ್ರುವ content_handler *handler,
	     lwc_string *imime_type,
	     ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
	     ಸಂರಚನಾ llcache_handle *llcache,
	     ಧ್ರುವ ವರ್ಣ *fallback_charset,
	     bool quirks,
	     ಸಂರಚನಾ content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, ಪರಿಮಾಣ(nspng_content));
	ಯದಿ (png_c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(png_c);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = nspng_create_png_data(png_c);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(png_c);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *)png_c;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ bool nspng_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data,
			       unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	nspng_content *png_c = (nspng_content *)c;
	ಚಞ್ಚಲ bool ret = true;

	ಯದಿ (png_c->no_process_data) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ವಿಕಲ್ಪ (setjmp(png_jmpbuf(png_c->png))) {
	ಪಕ್ಷ CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		ಭಙ್ಗ;

	ಪಕ್ಷ CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		ಭಙ್ಗ;

	ಸ್ವಭಾವ: /* fatal error from library processing png */
		ಯದಿ (png_c->bitmap != ರಿಕ್ತ) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_data = true;
		} ಅನ್ಯಥಾ {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ರಿಕ್ತ;
			png_c->info = ರಿಕ್ತ;

			content_broadcast_error(c, NSERROR_PNG_ERROR, ರಿಕ್ತ);

			ret = false;

		}
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸಂರಚನಾ png_cache_read_data_s {
	ಧ್ರುವ uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
ಸ್ಥಿರ ಶೂನ್ಯ 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ಸಂರಚನಾ png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	ಯದಿ (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	ಯದಿ (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
ಸ್ಥಿರ png_bytep *calc_row_pointers(ಸಂರಚನಾ bitmap *bitmap)
{
	ಪೂರ್ಣಾಙ್ಕ height = guit->bitmap->get_height(bitmap);
	unsigned ವರ್ಣ *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	ಪೂರ್ಣಾಙ್ಕ hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	ಯದಿ (buffer == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	row_ptrs = malloc(ಪರಿಮಾಣ(png_bytep) * height);

	ಯದಿ (row_ptrs != ರಿಕ್ತ) {
		ಪ್ರತಿ (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	ಪ್ರತ್ಯಾಗಮ row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ಸ್ಥಿರ ಸಂರಚನಾ bitmap *
png_cache_convert(ಸಂರಚನಾ content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	ಚಞ್ಚಲ ಸಂರಚನಾ bitmap * ಚಞ್ಚಲ bitmap = ರಿಕ್ತ;
	ಸಂರಚನಾ png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	ಚಞ್ಚಲ png_bytep * ಚಞ್ಚಲ row_pointers = ರಿಕ್ತ;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	ಯದಿ ((png_cache_read_data.data == ರಿಕ್ತ) || 
	    (png_cache_read_data.size <= 8)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ರಿಕ್ತ,
			nspng_error, nspng_warning);
	ಯದಿ (png_ptr == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	ಯದಿ (info_ptr == ರಿಕ್ತ) {
		png_destroy_read_struct(&png_ptr, ರಿಕ್ತ, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	ಯದಿ (end_info_ptr == ರಿಕ್ತ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* setup error exit path */
	ಯದಿ (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		ಗಚ್ಛ png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_set_read_fn(png_ptr, &png_cache_read_data, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_setup_transforms(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	ಯದಿ (bitmap == ರಿಕ್ತ) {
		/* cleanup and bail */
		ಗಚ್ಛ png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((ಸಂರಚನಾ bitmap *) bitmap);

	ಯದಿ (row_pointers != ರಿಕ್ತ) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} ಅನ್ಯಥಾ {
		guit->bitmap->destroy((ಸಂರಚನಾ bitmap *)bitmap);
		bitmap = ರಿಕ್ತ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	ಯದಿ (row_pointers != ರಿಕ್ತ) {
		ಮುಕ್ತ((png_bytep *) row_pointers);
	}

	ಯದಿ (bitmap != ರಿಕ್ತ) {
		bool opaque = bitmap_test_opaque((ಶೂನ್ಯ *)bitmap);
		guit->bitmap->set_opaque((ಶೂನ್ಯ *)bitmap, opaque);
		bitmap_format_to_client((ಶೂನ್ಯ *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((ಶೂನ್ಯ *)bitmap);
	}

	ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ bitmap *)bitmap;
}

ಸ್ಥಿರ bool nspng_convert(ಸಂರಚನಾ content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	ವರ್ಣ *title;

	ಪ್ರತಿಜ್ಞಾ(png_c->png != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(png_c->info != ರಿಕ್ತ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	ಯದಿ (png_c->bitmap != ರಿಕ್ತ) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->set_opaque(png_c->bitmap, opaque);
		bitmap_format_to_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_add(c, png_c->bitmap, png_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, "");

	ಪ್ರತ್ಯಾಗಮ true;
}


ಸ್ಥಿರ nserror nspng_clone(ಧ್ರುವ ಸಂರಚನಾ content *old_c, ಸಂರಚನಾ content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ಧ್ರುವ uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, ಪರಿಮಾಣ(nspng_content));
	ಯದಿ (clone_png_c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	ಯದಿ (size > 0) {
		ಯದಿ (nspng_process_data(&clone_png_c->base, (ಧ್ರುವ ವರ್ಣ *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	ಯದಿ ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		ಯದಿ (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ಸಂರಚನಾ content *)clone_png_c;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_data = nspng_process_data,
	.data_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
