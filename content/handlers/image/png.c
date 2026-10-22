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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <png.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image_cache.h"
#অন্তর্ভাব "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#যদি PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

প্রকারলেখ কাঠামো nspng_content {
	কাঠামো content base; /**< base content type */

	bool no_process_তথ্য; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	পূর্ণসংখ্যা interlace;
	কাঠামো bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

স্থির unsigned পূর্ণসংখ্যা interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
স্থির unsigned পূর্ণসংখ্যা interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
স্থির unsigned পূর্ণসংখ্যা interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
স্থির unsigned পূর্ণসংখ্যা interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
গণনা nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
স্থির শূন্য nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
স্থির শূন্য nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

স্থির শূন্য nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	পূর্ণসংখ্যা bit_depth, color_type, intent;
	দ্বিগুণ gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	যদি (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	যদি ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	যদি (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	যদি (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	যদি (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	নির্বাচন (bitmap_fmt.layout) {
	ক্ষেত্র BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	ক্ষেত্র BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		বিরতি;
	পূর্বনির্ধারিত:
		/* RGB is the default. */
		বিরতি;
	}

	যদি (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		নির্বাচন (bitmap_fmt.layout) {
		ক্ষেত্র BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ক্ষেত্র BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			বিরতি;

		পূর্বনির্ধারিত:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			বিরতি;
		}
	} অন্যথা {
		নির্বাচন (bitmap_fmt.layout) {
		ক্ষেত্র BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		ক্ষেত্র BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			বিরতি;
		পূর্বনির্ধারিত:
			/* Alpha as final component is the default. */
			বিরতি;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	যদি (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} অন্যথা {
		যদি (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} অন্যথা {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
স্থির শূন্য info_callback(png_structp png_s, png_infop info)
{
	পূর্ণসংখ্যা interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.আকার_2 += width * height * 4;

	/* see if progressive-conversion should continue */
	যদি (image_cache_speculate((কাঠামো content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	যদি (png_c->bitmap == ফাঁকা) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = আকার(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

স্থির শূন্য row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_অঙ্ক, পূর্ণসংখ্যা pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned অক্ষর *বাফার, *row;

	/* Give up if there's no bitmap */
	যদি (png_c->bitmap == ফাঁকা)
		ফেরত;

	/* Abort if we've not got any data */
	যদি (new_row == ফাঁকা)
		ফেরত;

	/* Get bitmap buffer */
	বাফার = guit->bitmap->get_বাফার(png_c->bitmap);
	যদি (বাফার == ফাঁকা) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = বাফার + (png_c->rowstride * row_অঙ্ক);

	/* Handle interlaced sprites using the Adam7 algorithm */
	যদি (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned পূর্ণসংখ্যা start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_অঙ্ক = interlace_row_start[pass] +
			interlace_row_step[pass] * row_অঙ্ক;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = বাফার + (png_c->rowstride * row_অঙ্ক);

		জন্য (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} অন্যথা {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


স্থির শূন্য end_callback(png_structp png_s, png_infop info)
{
}

স্থির nserror nspng_create_png_তথ্য(nspng_content *png_c)
{
	png_c->bitmap = ফাঁকা;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	যদি (png_c->png == ফাঁকা) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, ফাঁকা, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	যদি (png_c->info == ফাঁকা) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}

	যদি (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ফাঁকা;
		png_c->info = ফাঁকা;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	ফেরত NSERROR_OK;
}

স্থির nserror
nspng_create(ধ্রুবক content_handler *handler,
	     lwc_string *imime_type,
	     ধ্রুবক কাঠামো http_parameter *params,
	     কাঠামো llcache_handle *llcache,
	     ধ্রুবক অক্ষর *fallback_charset,
	     bool quirks,
	     কাঠামো content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, আকার(nspng_content));
	যদি (png_c == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(png_c);
		ফেরত error;
	}

	error = nspng_create_png_তথ্য(png_c);
	যদি (error != NSERROR_OK) {
		মুক্ত(png_c);
		ফেরত error;
	}

	*c = (কাঠামো content *)png_c;

	ফেরত NSERROR_OK;
}


স্থির bool nspng_process_তথ্য(কাঠামো content *c, ধ্রুবক অক্ষর *তথ্য,
			       unsigned পূর্ণসংখ্যা আকার_2)
{
	nspng_content *png_c = (nspng_content *)c;
	অস্থির bool ret = true;

	যদি (png_c->no_process_তথ্য) {
		ফেরত ret;
	}

	নির্বাচন (setjmp(png_jmpbuf(png_c->png))) {
	ক্ষেত্র CBERR_NONE: /* direct return */	
		png_process_তথ্য(png_c->png, png_c->info, (uint8_t *)তথ্য, আকার_2);
		বিরতি;

	ক্ষেত্র CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_তথ্য = true;
		বিরতি;

	পূর্বনির্ধারিত: /* fatal error from library processing png */
		যদি (png_c->bitmap != ফাঁকা) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_তথ্য = true;
		} অন্যথা {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ফাঁকা;
			png_c->info = ফাঁকা;

			content_broadcast_error(c, NSERROR_PNG_ERROR, ফাঁকা);

			ret = false;

		}
		বিরতি;
	}

	ফেরত ret;
}

কাঠামো png_cache_read_তথ্য_s {
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;
};

/** PNG library read fucntion to read data from a memory array 
 */
স্থির শূন্য 
png_cache_read_fn(png_structp png_ptr, png_bytep তথ্য, png_আকার_t দৈর্ঘ্য)
{
	কাঠামো png_cache_read_তথ্য_s *png_cache_read_তথ্য;
	png_cache_read_তথ্য = png_get_io_ptr(png_ptr);

	যদি (দৈর্ঘ্য > png_cache_read_তথ্য->আকার_2) {
		দৈর্ঘ্য = png_cache_read_তথ্য->আকার_2;
	}

	যদি (দৈর্ঘ্য == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(তথ্য, png_cache_read_তথ্য->তথ্য, দৈর্ঘ্য);

	png_cache_read_তথ্য->তথ্য += দৈর্ঘ্য;
	png_cache_read_তথ্য->আকার_2 -= দৈর্ঘ্য;
}

/** calculate an array of row pointers into a bitmap data area
 */
স্থির png_bytep *calc_row_pointers(কাঠামো bitmap *bitmap)
{
	পূর্ণসংখ্যা height = guit->bitmap->get_height(bitmap);
	unsigned অক্ষর *বাফার= guit->bitmap->get_বাফার(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	পূর্ণসংখ্যা hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	যদি (বাফার == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	row_ptrs = malloc(আকার(png_bytep) * height);

	যদি (row_ptrs != ফাঁকা) {
		জন্য (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = বাফার + (rowstride * hloop);
		}
	}

	ফেরত row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
স্থির কাঠামো bitmap *
png_cache_convert(কাঠামো content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	অস্থির কাঠামো bitmap * অস্থির bitmap = ফাঁকা;
	কাঠামো png_cache_read_তথ্য_s png_cache_read_তথ্য;
	png_uint_32 width, height;
	অস্থির png_bytep * অস্থির row_pointers = ফাঁকা;

	png_cache_read_তথ্য.তথ্য = 
		content_get_source_তথ্য_2(c, &png_cache_read_তথ্য.আকার_2);

	যদি ((png_cache_read_তথ্য.তথ্য == ফাঁকা) || 
	    (png_cache_read_তথ্য.আকার_2 <= 8)) {
		ফেরত ফাঁকা;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ফাঁকা,
			nspng_error, nspng_warning);
	যদি (png_ptr == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	info_ptr = png_create_info_struct(png_ptr);
	যদি (info_ptr == ফাঁকা) {
		png_destroy_read_struct(&png_ptr, ফাঁকা, ফাঁকা);
		ফেরত ফাঁকা;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	যদি (end_info_ptr == ফাঁকা) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ফাঁকা);
		ফেরত ফাঁকা;
	}

	/* setup error exit path */
	যদি (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		যাও png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_set_read_fn(png_ptr, &png_cache_read_তথ্য, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_setup_transforms(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	যদি (bitmap == ফাঁকা) {
		/* cleanup and bail */
		যাও png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((কাঠামো bitmap *) bitmap);

	যদি (row_pointers != ফাঁকা) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} অন্যথা {
		guit->bitmap->destroy((কাঠামো bitmap *)bitmap);
		bitmap = ফাঁকা;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	যদি (row_pointers != ফাঁকা) {
		মুক্ত((png_bytep *) row_pointers);
	}

	যদি (bitmap != ফাঁকা) {
		bool opaque = bitmap_test_opaque((শূন্য *)bitmap);
		guit->bitmap->set_opaque((শূন্য *)bitmap, opaque);
		bitmap_format_to_client((শূন্য *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((শূন্য *)bitmap);
	}

	ফেরত (কাঠামো bitmap *)bitmap;
}

স্থির bool nspng_convert(কাঠামো content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	অক্ষর *title;

	প্রতিজ্ঞা(png_c->png != ফাঁকা);
	প্রতিজ্ঞা(png_c->info != ফাঁকা);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != ফাঁকা) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	যদি (png_c->bitmap != ফাঁকা) {
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

	ফেরত true;
}


স্থির nserror nspng_clone(ধ্রুবক কাঠামো content *old_c, কাঠামো content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;

	clone_png_c = calloc(1, আকার(nspng_content));
	যদি (clone_png_c == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ফেরত error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_তথ্য(clone_png_c);
	যদি (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		ফেরত error;
	}

	তথ্য = content_get_source_তথ্য_2(&clone_png_c->base, &আকার_2);
	যদি (আকার_2 > 0) {
		যদি (nspng_process_তথ্য(&clone_png_c->base, (ধ্রুবক অক্ষর *)তথ্য, আকার_2) == false) {
			content_destroy(&clone_png_c->base);
			ফেরত NSERROR_NOMEM;
		}
	}

	যদি ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		যদি (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (কাঠামো content *)clone_png_c;

	ফেরত NSERROR_OK;
}

স্থির ধ্রুবক content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_তথ্য = nspng_process_তথ্য,
	.তথ্য_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
