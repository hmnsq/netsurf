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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <png.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image_cache.h"
#அந்தர்பாவ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#எனில் PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

வகைவரை கட்டமைப்பு nspng_content {
	கட்டமைப்பு content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	முழு interlace;
	கட்டமைப்பு bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

நிலையான unsigned முழு interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
நிலையான unsigned முழு interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
நிலையான unsigned முழு interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
நிலையான unsigned முழு interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
எண்ணிக்கை nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
நிலையான வெறுமை nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
நிலையான வெறுமை nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

நிலையான வெறுமை nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	முழு bit_depth, color_type, intent;
	இரட்டை gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	எனில் (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	எனில் ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	எனில் (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	எனில் (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	எனில் (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	தேர்வு (bitmap_fmt.layout) {
	நிலை BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	நிலை BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		நிறுத்து;
	இயல்பு:
		/* RGB is the default. */
		நிறுத்து;
	}

	எனில் (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		தேர்வு (bitmap_fmt.layout) {
		நிலை BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		நிலை BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			நிறுத்து;

		இயல்பு:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			நிறுத்து;
		}
	} இல்லையெனில் {
		தேர்வு (bitmap_fmt.layout) {
		நிலை BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		நிலை BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			நிறுத்து;
		இயல்பு:
			/* Alpha as final component is the default. */
			நிறுத்து;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	எனில் (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} இல்லையெனில் {
		எனில் (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} இல்லையெனில் {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
நிலையான வெறுமை info_callback(png_structp png_s, png_infop info)
{
	முழு interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	எனில் (image_cache_speculate((கட்டமைப்பு content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	எனில் (png_c->bitmap == வெற்று) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = அளவு(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

நிலையான வெறுமை row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_இலக்கம், முழு pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned எழுத்து *buffer, *row;

	/* Give up if there's no bitmap */
	எனில் (png_c->bitmap == வெற்று)
		திரும்பு;

	/* Abort if we've not got any data */
	எனில் (new_row == வெற்று)
		திரும்பு;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	எனில் (buffer == வெற்று) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_இலக்கம்);

	/* Handle interlaced sprites using the Adam7 algorithm */
	எனில் (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned முழு start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_இலக்கம் = interlace_row_start[pass] +
			interlace_row_step[pass] * row_இலக்கம்;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_இலக்கம்);

		க்கு (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} இல்லையெனில் {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


நிலையான வெறுமை end_callback(png_structp png_s, png_infop info)
{
}

நிலையான nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = வெற்று;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	எனில் (png_c->png == வெற்று) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, வெற்று, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	எனில் (png_c->info == வெற்று) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = வெற்று;
		png_c->info = வெற்று;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	திரும்பு NSERROR_OK;
}

நிலையான nserror
nspng_create(மாறிலி content_handler *handler,
	     lwc_string *imime_type,
	     மாறிலி கட்டமைப்பு http_parameter *params,
	     கட்டமைப்பு llcache_handle *llcache,
	     மாறிலி எழுத்து *fallback_charset,
	     bool quirks,
	     கட்டமைப்பு content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, அளவு(nspng_content));
	எனில் (png_c == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(png_c);
		திரும்பு error;
	}

	error = nspng_create_png_data(png_c);
	எனில் (error != NSERROR_OK) {
		விடுவி(png_c);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *)png_c;

	திரும்பு NSERROR_OK;
}


நிலையான bool nspng_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data,
			       unsigned முழு size)
{
	nspng_content *png_c = (nspng_content *)c;
	நிலையற்ற bool ret = true;

	எனில் (png_c->no_process_data) {
		திரும்பு ret;
	}

	தேர்வு (setjmp(png_jmpbuf(png_c->png))) {
	நிலை CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		நிறுத்து;

	நிலை CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		நிறுத்து;

	இயல்பு: /* fatal error from library processing png */
		எனில் (png_c->bitmap != வெற்று) {
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
		} இல்லையெனில் {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = வெற்று;
			png_c->info = வெற்று;

			content_broadcast_error(c, NSERROR_PNG_ERROR, வெற்று);

			ret = false;

		}
		நிறுத்து;
	}

	திரும்பு ret;
}

கட்டமைப்பு png_cache_read_data_s {
	மாறிலி uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
நிலையான வெறுமை 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	கட்டமைப்பு png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	எனில் (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	எனில் (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
நிலையான png_bytep *calc_row_pointers(கட்டமைப்பு bitmap *bitmap)
{
	முழு height = guit->bitmap->get_height(bitmap);
	unsigned எழுத்து *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	முழு hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	எனில் (buffer == வெற்று) {
		திரும்பு வெற்று;
	}

	row_ptrs = malloc(அளவு(png_bytep) * height);

	எனில் (row_ptrs != வெற்று) {
		க்கு (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	திரும்பு row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
நிலையான கட்டமைப்பு bitmap *
png_cache_convert(கட்டமைப்பு content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	நிலையற்ற கட்டமைப்பு bitmap * நிலையற்ற bitmap = வெற்று;
	கட்டமைப்பு png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	நிலையற்ற png_bytep * நிலையற்ற row_pointers = வெற்று;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	எனில் ((png_cache_read_data.data == வெற்று) || 
	    (png_cache_read_data.size <= 8)) {
		திரும்பு வெற்று;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, வெற்று,
			nspng_error, nspng_warning);
	எனில் (png_ptr == வெற்று) {
		திரும்பு வெற்று;
	}

	info_ptr = png_create_info_struct(png_ptr);
	எனில் (info_ptr == வெற்று) {
		png_destroy_read_struct(&png_ptr, வெற்று, வெற்று);
		திரும்பு வெற்று;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	எனில் (end_info_ptr == வெற்று) {
		png_destroy_read_struct(&png_ptr, &info_ptr, வெற்று);
		திரும்பு வெற்று;
	}

	/* setup error exit path */
	எனில் (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		செல் png_cache_convert_error;
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
	எனில் (bitmap == வெற்று) {
		/* cleanup and bail */
		செல் png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((கட்டமைப்பு bitmap *) bitmap);

	எனில் (row_pointers != வெற்று) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} இல்லையெனில் {
		guit->bitmap->destroy((கட்டமைப்பு bitmap *)bitmap);
		bitmap = வெற்று;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	எனில் (row_pointers != வெற்று) {
		விடுவி((png_bytep *) row_pointers);
	}

	எனில் (bitmap != வெற்று) {
		bool opaque = bitmap_test_opaque((வெறுமை *)bitmap);
		guit->bitmap->set_opaque((வெறுமை *)bitmap, opaque);
		bitmap_format_to_client((வெறுமை *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((வெறுமை *)bitmap);
	}

	திரும்பு (கட்டமைப்பு bitmap *)bitmap;
}

நிலையான bool nspng_convert(கட்டமைப்பு content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	எழுத்து *title;

	ப்ரதிஜ்ஞா(png_c->png != வெற்று);
	ப்ரதிஜ்ஞா(png_c->info != வெற்று);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	எனில் (png_c->bitmap != வெற்று) {
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

	திரும்பு true;
}


நிலையான nserror nspng_clone(மாறிலி கட்டமைப்பு content *old_c, கட்டமைப்பு content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	மாறிலி uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, அளவு(nspng_content));
	எனில் (clone_png_c == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		திரும்பு error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	எனில் (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		திரும்பு error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	எனில் (size > 0) {
		எனில் (nspng_process_data(&clone_png_c->base, (மாறிலி எழுத்து *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			திரும்பு NSERROR_NOMEM;
		}
	}

	எனில் ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		எனில் (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (கட்டமைப்பு content *)clone_png_c;

	திரும்பு NSERROR_OK;
}

நிலையான மாறிலி content_handler nspng_content_handler = {
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

நிலையான மாறிலி எழுத்து *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
