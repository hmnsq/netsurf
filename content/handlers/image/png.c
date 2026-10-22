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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <png.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image_cache.h"
#അന്തര്ഭാവ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#എങ്കിൽ PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

തരനിർവചനം ഘടന nspng_content {
	ഘടന content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	പൂർണ്ണസംഖ്യ interlace;
	ഘടന bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
എണ്ണൽ nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
സ്ഥിതം ശൂന്യം nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
സ്ഥിതം ശൂന്യം nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

സ്ഥിതം ശൂന്യം nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	പൂർണ്ണസംഖ്യ bit_depth, color_type, intent;
	ഇരട്ട gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	എങ്കിൽ (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	എങ്കിൽ ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	എങ്കിൽ (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	എങ്കിൽ (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	എങ്കിൽ (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	തിരഞ്ഞെടുപ്പ് (bitmap_fmt.layout) {
	അവസ്ഥ BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	അവസ്ഥ BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* RGB is the default. */
		നിർത്തുക;
	}

	എങ്കിൽ (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		തിരഞ്ഞെടുപ്പ് (bitmap_fmt.layout) {
		അവസ്ഥ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		അവസ്ഥ BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			നിർത്തുക;
		}
	} അല്ലെങ്കിൽ {
		തിരഞ്ഞെടുപ്പ് (bitmap_fmt.layout) {
		അവസ്ഥ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		അവസ്ഥ BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			/* Alpha as final component is the default. */
			നിർത്തുക;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	എങ്കിൽ (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} അല്ലെങ്കിൽ {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
സ്ഥിതം ശൂന്യം info_callback(png_structp png_s, png_infop info)
{
	പൂർണ്ണസംഖ്യ interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	എങ്കിൽ (image_cache_speculate((ഘടന content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	എങ്കിൽ (png_c->bitmap == ശൂന്യ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = വലുപ്പം(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

സ്ഥിതം ശൂന്യം row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_അക്കം, പൂർണ്ണസംഖ്യ pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned അക്ഷരം *buffer, *row;

	/* Give up if there's no bitmap */
	എങ്കിൽ (png_c->bitmap == ശൂന്യ)
		തിരികെ;

	/* Abort if we've not got any data */
	എങ്കിൽ (new_row == ശൂന്യ)
		തിരികെ;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	എങ്കിൽ (buffer == ശൂന്യ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_അക്കം);

	/* Handle interlaced sprites using the Adam7 algorithm */
	എങ്കിൽ (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned പൂർണ്ണസംഖ്യ start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_അക്കം = interlace_row_start[pass] +
			interlace_row_step[pass] * row_അക്കം;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_അക്കം);

		വേണ്ടി (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} അല്ലെങ്കിൽ {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


സ്ഥിതം ശൂന്യം end_callback(png_structp png_s, png_infop info)
{
}

സ്ഥിതം nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = ശൂന്യ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	എങ്കിൽ (png_c->png == ശൂന്യ) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, ശൂന്യ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	എങ്കിൽ (png_c->info == ശൂന്യ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ശൂന്യ;
		png_c->info = ശൂന്യ;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror
nspng_create(സ്ഥിരം content_handler *handler,
	     lwc_string *imime_type,
	     സ്ഥിരം ഘടന http_parameter *params,
	     ഘടന llcache_handle *llcache,
	     സ്ഥിരം അക്ഷരം *fallback_charset,
	     bool quirks,
	     ഘടന content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, വലുപ്പം(nspng_content));
	എങ്കിൽ (png_c == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(png_c);
		തിരികെ error;
	}

	error = nspng_create_png_data(png_c);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(png_c);
		തിരികെ error;
	}

	*c = (ഘടന content *)png_c;

	തിരികെ NSERROR_OK;
}


സ്ഥിതം bool nspng_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data,
			       unsigned പൂർണ്ണസംഖ്യ size)
{
	nspng_content *png_c = (nspng_content *)c;
	അസ്ഥിര bool ret = true;

	എങ്കിൽ (png_c->no_process_data) {
		തിരികെ ret;
	}

	തിരഞ്ഞെടുപ്പ് (setjmp(png_jmpbuf(png_c->png))) {
	അവസ്ഥ CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		നിർത്തുക;

	അവസ്ഥ CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി: /* fatal error from library processing png */
		എങ്കിൽ (png_c->bitmap != ശൂന്യ) {
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
		} അല്ലെങ്കിൽ {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ശൂന്യ;
			png_c->info = ശൂന്യ;

			content_broadcast_error(c, NSERROR_PNG_ERROR, ശൂന്യ);

			ret = false;

		}
		നിർത്തുക;
	}

	തിരികെ ret;
}

ഘടന png_cache_read_data_s {
	സ്ഥിരം uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
സ്ഥിതം ശൂന്യം 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ഘടന png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	എങ്കിൽ (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	എങ്കിൽ (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
സ്ഥിതം png_bytep *calc_row_pointers(ഘടന bitmap *bitmap)
{
	പൂർണ്ണസംഖ്യ height = guit->bitmap->get_height(bitmap);
	unsigned അക്ഷരം *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	പൂർണ്ണസംഖ്യ hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	എങ്കിൽ (buffer == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	row_ptrs = malloc(വലുപ്പം(png_bytep) * height);

	എങ്കിൽ (row_ptrs != ശൂന്യ) {
		വേണ്ടി (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	തിരികെ row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
സ്ഥിതം ഘടന bitmap *
png_cache_convert(ഘടന content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	അസ്ഥിര ഘടന bitmap * അസ്ഥിര bitmap = ശൂന്യ;
	ഘടന png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	അസ്ഥിര png_bytep * അസ്ഥിര row_pointers = ശൂന്യ;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	എങ്കിൽ ((png_cache_read_data.data == ശൂന്യ) || 
	    (png_cache_read_data.size <= 8)) {
		തിരികെ ശൂന്യ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ശൂന്യ,
			nspng_error, nspng_warning);
	എങ്കിൽ (png_ptr == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	എങ്കിൽ (info_ptr == ശൂന്യ) {
		png_destroy_read_struct(&png_ptr, ശൂന്യ, ശൂന്യ);
		തിരികെ ശൂന്യ;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	എങ്കിൽ (end_info_ptr == ശൂന്യ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ശൂന്യ);
		തിരികെ ശൂന്യ;
	}

	/* setup error exit path */
	എങ്കിൽ (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		പോകുക png_cache_convert_error;
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
	എങ്കിൽ (bitmap == ശൂന്യ) {
		/* cleanup and bail */
		പോകുക png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((ഘടന bitmap *) bitmap);

	എങ്കിൽ (row_pointers != ശൂന്യ) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} അല്ലെങ്കിൽ {
		guit->bitmap->destroy((ഘടന bitmap *)bitmap);
		bitmap = ശൂന്യ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	എങ്കിൽ (row_pointers != ശൂന്യ) {
		സ്വതന്ത്രം((png_bytep *) row_pointers);
	}

	എങ്കിൽ (bitmap != ശൂന്യ) {
		bool opaque = bitmap_test_opaque((ശൂന്യം *)bitmap);
		guit->bitmap->set_opaque((ശൂന്യം *)bitmap, opaque);
		bitmap_format_to_client((ശൂന്യം *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((ശൂന്യം *)bitmap);
	}

	തിരികെ (ഘടന bitmap *)bitmap;
}

സ്ഥിതം bool nspng_convert(ഘടന content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	അക്ഷരം *title;

	പ്രതിജ്ഞാ(png_c->png != ശൂന്യ);
	പ്രതിജ്ഞാ(png_c->info != ശൂന്യ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	എങ്കിൽ (png_c->bitmap != ശൂന്യ) {
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

	തിരികെ true;
}


സ്ഥിതം nserror nspng_clone(സ്ഥിരം ഘടന content *old_c, ഘടന content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	സ്ഥിരം uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, വലുപ്പം(nspng_content));
	എങ്കിൽ (clone_png_c == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		തിരികെ error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		തിരികെ error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	എങ്കിൽ (size > 0) {
		എങ്കിൽ (nspng_process_data(&clone_png_c->base, (സ്ഥിരം അക്ഷരം *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			തിരികെ NSERROR_NOMEM;
		}
	}

	എങ്കിൽ ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		എങ്കിൽ (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ഘടന content *)clone_png_c;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം സ്ഥിരം content_handler nspng_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
