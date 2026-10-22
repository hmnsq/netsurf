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

#شامل <stdbool.h>
#شامل <string.h>
#شامل <stdlib.h>
#شامل <png.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/bitmap.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image_cache.h"
#شامل "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#اگر PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

تعریفنوع ساختار nspng_content {
	ساختار content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	صحیح interlace;
	ساختار bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ایستا unsigned صحیح interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
ایستا unsigned صحیح interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
ایستا unsigned صحیح interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
ایستا unsigned صحیح interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
شمارش nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ایستا تهی nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
ایستا تهی nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ایستا تهی nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	صحیح bit_depth, color_type, intent;
	دوگانه gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	اگر (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	اگر ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	اگر (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	اگر (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	اگر (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	انتخاب (bitmap_fmt.layout) {
	حالت BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	حالت BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		بشکن;
	پیشفرض:
		/* RGB is the default. */
		بشکن;
	}

	اگر (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		انتخاب (bitmap_fmt.layout) {
		حالت BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالت BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			بشکن;

		پیشفرض:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			بشکن;
		}
	} وگرنه {
		انتخاب (bitmap_fmt.layout) {
		حالت BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالت BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			بشکن;
		پیشفرض:
			/* Alpha as final component is the default. */
			بشکن;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	اگر (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} وگرنه {
		اگر (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} وگرنه {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ایستا تهی info_callback(png_structp png_s, png_infop info)
{
	صحیح interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	اگر (image_cache_speculate((ساختار content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	اگر (png_c->bitmap == پوچ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = اندازه(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

ایستا تهی row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_رقم, صحیح pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned نویسه *buffer, *row;

	/* Give up if there's no bitmap */
	اگر (png_c->bitmap == پوچ)
		بازگشت;

	/* Abort if we've not got any data */
	اگر (new_row == پوچ)
		بازگشت;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	اگر (buffer == پوچ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_رقم);

	/* Handle interlaced sprites using the Adam7 algorithm */
	اگر (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned صحیح start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_رقم = interlace_row_start[pass] +
			interlace_row_step[pass] * row_رقم;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_رقم);

		برای (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} وگرنه {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


ایستا تهی end_callback(png_structp png_s, png_infop info)
{
}

ایستا nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = پوچ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	اگر (png_c->png == پوچ) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, پوچ);
		بازگشت NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, پوچ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	اگر (png_c->info == پوچ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, پوچ);
		بازگشت NSERROR_NOMEM;
	}

	اگر (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = پوچ;
		png_c->info = پوچ;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, پوچ);
		بازگشت NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	بازگشت NSERROR_OK;
}

ایستا nserror
nspng_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت ساختار http_parameter *params,
	     ساختار llcache_handle *llcache,
	     ثابت نویسه *fallback_charset,
	     bool quirks,
	     ساختار content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, اندازه(nspng_content));
	اگر (png_c == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	اگر (error != NSERROR_OK) {
		آزاد(png_c);
		بازگشت error;
	}

	error = nspng_create_png_data(png_c);
	اگر (error != NSERROR_OK) {
		آزاد(png_c);
		بازگشت error;
	}

	*c = (ساختار content *)png_c;

	بازگشت NSERROR_OK;
}


ایستا bool nspng_process_data(ساختار content *c, ثابت نویسه *data,
			       unsigned صحیح size)
{
	nspng_content *png_c = (nspng_content *)c;
	فرار bool ret = true;

	اگر (png_c->no_process_data) {
		بازگشت ret;
	}

	انتخاب (setjmp(png_jmpbuf(png_c->png))) {
	حالت CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		بشکن;

	حالت CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		بشکن;

	پیشفرض: /* fatal error from library processing png */
		اگر (png_c->bitmap != پوچ) {
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
		} وگرنه {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = پوچ;
			png_c->info = پوچ;

			content_broadcast_error(c, NSERROR_PNG_ERROR, پوچ);

			ret = false;

		}
		بشکن;
	}

	بازگشت ret;
}

ساختار png_cache_read_data_s {
	ثابت uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
ایستا تهی 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ساختار png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	اگر (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	اگر (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
ایستا png_bytep *calc_row_pointers(ساختار bitmap *bitmap)
{
	صحیح height = guit->bitmap->get_height(bitmap);
	unsigned نویسه *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	صحیح hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	اگر (buffer == پوچ) {
		بازگشت پوچ;
	}

	row_ptrs = malloc(اندازه(png_bytep) * height);

	اگر (row_ptrs != پوچ) {
		برای (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	بازگشت row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ایستا ساختار bitmap *
png_cache_convert(ساختار content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	فرار ساختار bitmap * فرار bitmap = پوچ;
	ساختار png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	فرار png_bytep * فرار row_pointers = پوچ;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	اگر ((png_cache_read_data.data == پوچ) || 
	    (png_cache_read_data.size <= 8)) {
		بازگشت پوچ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, پوچ,
			nspng_error, nspng_warning);
	اگر (png_ptr == پوچ) {
		بازگشت پوچ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	اگر (info_ptr == پوچ) {
		png_destroy_read_struct(&png_ptr, پوچ, پوچ);
		بازگشت پوچ;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	اگر (end_info_ptr == پوچ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, پوچ);
		بازگشت پوچ;
	}

	/* setup error exit path */
	اگر (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		برو png_cache_convert_error;
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
	اگر (bitmap == پوچ) {
		/* cleanup and bail */
		برو png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((ساختار bitmap *) bitmap);

	اگر (row_pointers != پوچ) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} وگرنه {
		guit->bitmap->destroy((ساختار bitmap *)bitmap);
		bitmap = پوچ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	اگر (row_pointers != پوچ) {
		آزاد((png_bytep *) row_pointers);
	}

	اگر (bitmap != پوچ) {
		bool opaque = bitmap_test_opaque((تهی *)bitmap);
		guit->bitmap->set_opaque((تهی *)bitmap, opaque);
		bitmap_format_to_client((تهی *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((تهی *)bitmap);
	}

	بازگشت (ساختار bitmap *)bitmap;
}

ایستا bool nspng_convert(ساختار content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	نویسه *title;

	أكد(png_c->png != پوچ);
	أكد(png_c->info != پوچ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	اگر (title != پوچ) {
		content__set_title(c, title);
		آزاد(title);
	}

	اگر (png_c->bitmap != پوچ) {
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

	بازگشت true;
}


ایستا nserror nspng_clone(ثابت ساختار content *old_c, ساختار content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ثابت uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, اندازه(nspng_content));
	اگر (clone_png_c == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		بازگشت error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	اگر (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		بازگشت error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	اگر (size > 0) {
		اگر (nspng_process_data(&clone_png_c->base, (ثابت نویسه *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			بازگشت NSERROR_NOMEM;
		}
	}

	اگر ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		اگر (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ساختار content *)clone_png_c;

	بازگشت NSERROR_OK;
}

ایستا ثابت content_handler nspng_content_handler = {
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

ایستا ثابت نویسه *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
