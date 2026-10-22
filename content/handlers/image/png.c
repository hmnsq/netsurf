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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <png.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"
#ଅନ୍ତର୍ଭାଵ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#ଯଦି PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nspng_content {
	ସଂରଚନା content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace;
	ସଂରଚନା bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
ଗଣନା nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ସ୍ଥିର ଶୂନ୍ୟ nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
ସ୍ଥିର ଶୂନ୍ୟ nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ସ୍ଥିର ଶୂନ୍ୟ nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit_depth, color_type, intent;
	ଦ୍ୱିଗୁଣ gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	ଯଦି (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	ଯଦି ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	ଯଦି (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	ଯଦି (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	ଯଦି (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	ବାଛ (bitmap_fmt.layout) {
	ସ୍ଥିତି BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	ସ୍ଥିତି BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* RGB is the default. */
		ଭାଙ୍ଗ;
	}

	ଯଦି (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		ବାଛ (bitmap_fmt.layout) {
		ସ୍ଥିତି BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ସ୍ଥିତି BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			ଭାଙ୍ଗ;
		}
	} ଅନ୍ୟଥା {
		ବାଛ (bitmap_fmt.layout) {
		ସ୍ଥିତି BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ସ୍ଥିତି BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			/* Alpha as final component is the default. */
			ଭାଙ୍ଗ;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	ଯଦି (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} ଅନ୍ୟଥା {
		ଯଦି (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} ଅନ୍ୟଥା {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ସ୍ଥିର ଶୂନ୍ୟ info_callback(png_structp png_s, png_infop info)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	ଯଦି (image_cache_speculate((ସଂରଚନା content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	ଯଦି (png_c->bitmap == ଖାଲି) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = ଆକାର(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

ସ୍ଥିର ଶୂନ୍ୟ row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned ଅକ୍ଷର *buffer, *row;

	/* Give up if there's no bitmap */
	ଯଦି (png_c->bitmap == ଖାଲି)
		ଫେରନ୍ତୁ;

	/* Abort if we've not got any data */
	ଯଦି (new_row == ଖାଲି)
		ଫେରନ୍ତୁ;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	ଯଦି (buffer == ଖାଲି) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_ଅଙ୍କ);

	/* Handle interlaced sprites using the Adam7 algorithm */
	ଯଦି (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_ଅଙ୍କ = interlace_row_start[pass] +
			interlace_row_step[pass] * row_ଅଙ୍କ;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_ଅଙ୍କ);

		ପାଇଁ (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} ଅନ୍ୟଥା {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


ସ୍ଥିର ଶୂନ୍ୟ end_callback(png_structp png_s, png_infop info)
{
}

ସ୍ଥିର nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = ଖାଲି;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	ଯଦି (png_c->png == ଖାଲି) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, ଖାଲି, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	ଯଦି (png_c->info == ଖାଲି) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ଖାଲି;
		png_c->info = ଖାଲି;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror
nspng_create(ଅଚଳ content_handler *handler,
	     lwc_string *imime_type,
	     ଅଚଳ ସଂରଚନା http_parameter *params,
	     ସଂରଚନା llcache_handle *llcache,
	     ଅଚଳ ଅକ୍ଷର *fallback_charset,
	     bool quirks,
	     ସଂରଚନା content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, ଆକାର(nspng_content));
	ଯଦି (png_c == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(png_c);
		ଫେରନ୍ତୁ error;
	}

	error = nspng_create_png_data(png_c);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(png_c);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *)png_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର bool nspng_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data,
			       unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	nspng_content *png_c = (nspng_content *)c;
	ଅସ୍ଥିର bool ret = true;

	ଯଦି (png_c->no_process_data) {
		ଫେରନ୍ତୁ ret;
	}

	ବାଛ (setjmp(png_jmpbuf(png_c->png))) {
	ସ୍ଥିତି CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ: /* fatal error from library processing png */
		ଯଦି (png_c->bitmap != ଖାଲି) {
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
		} ଅନ୍ୟଥା {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ଖାଲି;
			png_c->info = ଖାଲି;

			content_broadcast_error(c, NSERROR_PNG_ERROR, ଖାଲି);

			ret = false;

		}
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ret;
}

ସଂରଚନା png_cache_read_data_s {
	ଅଚଳ uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
ସ୍ଥିର ଶୂନ୍ୟ 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ସଂରଚନା png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	ଯଦି (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	ଯଦି (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
ସ୍ଥିର png_bytep *calc_row_pointers(ସଂରଚନା bitmap *bitmap)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height = guit->bitmap->get_height(bitmap);
	unsigned ଅକ୍ଷର *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	ଯଦି (buffer == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	row_ptrs = malloc(ଆକାର(png_bytep) * height);

	ଯଦି (row_ptrs != ଖାଲି) {
		ପାଇଁ (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	ଫେରନ୍ତୁ row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ସ୍ଥିର ସଂରଚନା bitmap *
png_cache_convert(ସଂରଚନା content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	ଅସ୍ଥିର ସଂରଚନା bitmap * ଅସ୍ଥିର bitmap = ଖାଲି;
	ସଂରଚନା png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	ଅସ୍ଥିର png_bytep * ଅସ୍ଥିର row_pointers = ଖାଲି;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	ଯଦି ((png_cache_read_data.data == ଖାଲି) || 
	    (png_cache_read_data.size <= 8)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ଖାଲି,
			nspng_error, nspng_warning);
	ଯଦି (png_ptr == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	info_ptr = png_create_info_struct(png_ptr);
	ଯଦି (info_ptr == ଖାଲି) {
		png_destroy_read_struct(&png_ptr, ଖାଲି, ଖାଲି);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	ଯଦି (end_info_ptr == ଖାଲି) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ଖାଲି);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* setup error exit path */
	ଯଦି (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		ଯାଅ png_cache_convert_error;
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
	ଯଦି (bitmap == ଖାଲି) {
		/* cleanup and bail */
		ଯାଅ png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((ସଂରଚନା bitmap *) bitmap);

	ଯଦି (row_pointers != ଖାଲି) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} ଅନ୍ୟଥା {
		guit->bitmap->destroy((ସଂରଚନା bitmap *)bitmap);
		bitmap = ଖାଲି;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	ଯଦି (row_pointers != ଖାଲି) {
		ମୁକ୍ତ((png_bytep *) row_pointers);
	}

	ଯଦି (bitmap != ଖାଲି) {
		bool opaque = bitmap_test_opaque((ଶୂନ୍ୟ *)bitmap);
		guit->bitmap->set_opaque((ଶୂନ୍ୟ *)bitmap, opaque);
		bitmap_format_to_client((ଶୂନ୍ୟ *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((ଶୂନ୍ୟ *)bitmap);
	}

	ଫେରନ୍ତୁ (ସଂରଚନା bitmap *)bitmap;
}

ସ୍ଥିର bool nspng_convert(ସଂରଚନା content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	ଅକ୍ଷର *title;

	ପ୍ରତିଜ୍ଞା(png_c->png != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(png_c->info != ଖାଲି);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	ଯଦି (png_c->bitmap != ଖାଲି) {
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

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର nserror nspng_clone(ଅଚଳ ସଂରଚନା content *old_c, ସଂରଚନା content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ଅଚଳ uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, ଆକାର(nspng_content));
	ଯଦି (clone_png_c == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ଫେରନ୍ତୁ error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	ଯଦି (size > 0) {
		ଯଦି (nspng_process_data(&clone_png_c->base, (ଅଚଳ ଅକ୍ଷର *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ଯଦି ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		ଯଦି (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ସଂରଚନା content *)clone_png_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଅଚଳ content_handler nspng_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
