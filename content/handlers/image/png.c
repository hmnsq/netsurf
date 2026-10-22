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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <png.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/image_cache.h"
#અન્તર્ભાવ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#જો PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

પ્રકારવ્યાખ્યા સંરચના nspng_content {
	સંરચના content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	પૂર્ણાંક interlace;
	સંરચના bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

સ્થિર unsigned પૂર્ણાંક interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
સ્થિર unsigned પૂર્ણાંક interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
સ્થિર unsigned પૂર્ણાંક interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
સ્થિર unsigned પૂર્ણાંક interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
ગણના nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
સ્થિર શૂન્ય nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
સ્થિર શૂન્ય nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

સ્થિર શૂન્ય nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	પૂર્ણાંક bit_depth, color_type, intent;
	દ્વિગુણ gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	જો (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	જો ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	જો (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	જો (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	જો (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	પસંદગી (bitmap_fmt.layout) {
	સ્થિતિ BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	સ્થિતિ BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		તોડો;
	મૂળભૂત:
		/* RGB is the default. */
		તોડો;
	}

	જો (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		પસંદગી (bitmap_fmt.layout) {
		સ્થિતિ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		સ્થિતિ BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			તોડો;

		મૂળભૂત:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			તોડો;
		}
	} નહીંતર {
		પસંદગી (bitmap_fmt.layout) {
		સ્થિતિ BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		સ્થિતિ BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			તોડો;
		મૂળભૂત:
			/* Alpha as final component is the default. */
			તોડો;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	જો (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} નહીંતર {
		જો (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} નહીંતર {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
સ્થિર શૂન્ય info_callback(png_structp png_s, png_infop info)
{
	પૂર્ણાંક interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	જો (image_cache_speculate((સંરચના content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	જો (png_c->bitmap == ખાલી) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = કદ(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (unsigned long)width, (unsigned long)height, png_c->rowbytes);
}

સ્થિર શૂન્ય row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_અંક, પૂર્ણાંક pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	unsigned long rowbytes = png_c->rowbytes;
	unsigned અક્ષર *buffer, *row;

	/* Give up if there's no bitmap */
	જો (png_c->bitmap == ખાલી)
		પાછા;

	/* Abort if we've not got any data */
	જો (new_row == ખાલી)
		પાછા;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	જો (buffer == ખાલી) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_અંક);

	/* Handle interlaced sprites using the Adam7 algorithm */
	જો (png_c->interlace) {
		unsigned long dst_off;
		unsigned long src_off = 0;
		unsigned પૂર્ણાંક start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_અંક = interlace_row_start[pass] +
			interlace_row_step[pass] * row_અંક;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_અંક);

		માટે (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} નહીંતર {
		/* Do a fast memcpy of the row data */
		memcpy(row, new_row, rowbytes);
	}
}


સ્થિર શૂન્ય end_callback(png_structp png_s, png_infop info)
{
}

સ્થિર nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = ખાલી;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	જો (png_c->png == ખાલી) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ખાલી);
		પાછા NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, ખાલી, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	જો (png_c->info == ખાલી) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, ખાલી);
		પાછા NSERROR_NOMEM;
	}

	જો (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ખાલી;
		png_c->info = ખાલી;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, ખાલી);
		પાછા NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	પાછા NSERROR_OK;
}

સ્થિર nserror
nspng_create(અચલ content_handler *handler,
	     lwc_string *imime_type,
	     અચલ સંરચના http_parameter *params,
	     સંરચના llcache_handle *llcache,
	     અચલ અક્ષર *fallback_charset,
	     bool quirks,
	     સંરચના content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = calloc(1, કદ(nspng_content));
	જો (png_c == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(png_c);
		પાછા error;
	}

	error = nspng_create_png_data(png_c);
	જો (error != NSERROR_OK) {
		મુક્ત(png_c);
		પાછા error;
	}

	*c = (સંરચના content *)png_c;

	પાછા NSERROR_OK;
}


સ્થિર bool nspng_process_data(સંરચના content *c, અચલ અક્ષર *data,
			       unsigned પૂર્ણાંક size)
{
	nspng_content *png_c = (nspng_content *)c;
	અસ્થિર bool ret = true;

	જો (png_c->no_process_data) {
		પાછા ret;
	}

	પસંદગી (setjmp(png_jmpbuf(png_c->png))) {
	સ્થિતિ CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		તોડો;

	સ્થિતિ CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		તોડો;

	મૂળભૂત: /* fatal error from library processing png */
		જો (png_c->bitmap != ખાલી) {
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
		} નહીંતર {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ખાલી;
			png_c->info = ખાલી;

			content_broadcast_error(c, NSERROR_PNG_ERROR, ખાલી);

			ret = false;

		}
		તોડો;
	}

	પાછા ret;
}

સંરચના png_cache_read_data_s {
	અચલ uint8_t *data;
	size_t size;
};

/** PNG library read fucntion to read data from a memory array 
 */
સ્થિર શૂન્ય 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	સંરચના png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	જો (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	જો (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	memcpy(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
સ્થિર png_bytep *calc_row_pointers(સંરચના bitmap *bitmap)
{
	પૂર્ણાંક height = guit->bitmap->get_height(bitmap);
	unsigned અક્ષર *buffer= guit->bitmap->get_buffer(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	પૂર્ણાંક hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	જો (buffer == ખાલી) {
		પાછા ખાલી;
	}

	row_ptrs = malloc(કદ(png_bytep) * height);

	જો (row_ptrs != ખાલી) {
		માટે (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	પાછા row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
સ્થિર સંરચના bitmap *
png_cache_convert(સંરચના content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	અસ્થિર સંરચના bitmap * અસ્થિર bitmap = ખાલી;
	સંરચના png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	અસ્થિર png_bytep * અસ્થિર row_pointers = ખાલી;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	જો ((png_cache_read_data.data == ખાલી) || 
	    (png_cache_read_data.size <= 8)) {
		પાછા ખાલી;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ખાલી,
			nspng_error, nspng_warning);
	જો (png_ptr == ખાલી) {
		પાછા ખાલી;
	}

	info_ptr = png_create_info_struct(png_ptr);
	જો (info_ptr == ખાલી) {
		png_destroy_read_struct(&png_ptr, ખાલી, ખાલી);
		પાછા ખાલી;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	જો (end_info_ptr == ખાલી) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ખાલી);
		પાછા ખાલી;
	}

	/* setup error exit path */
	જો (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		જાઓ png_cache_convert_error;
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
	જો (bitmap == ખાલી) {
		/* cleanup and bail */
		જાઓ png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((સંરચના bitmap *) bitmap);

	જો (row_pointers != ખાલી) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} નહીંતર {
		guit->bitmap->destroy((સંરચના bitmap *)bitmap);
		bitmap = ખાલી;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	જો (row_pointers != ખાલી) {
		મુક્ત((png_bytep *) row_pointers);
	}

	જો (bitmap != ખાલી) {
		bool opaque = bitmap_test_opaque((શૂન્ય *)bitmap);
		guit->bitmap->set_opaque((શૂન્ય *)bitmap, opaque);
		bitmap_format_to_client((શૂન્ય *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((શૂન્ય *)bitmap);
	}

	પાછા (સંરચના bitmap *)bitmap;
}

સ્થિર bool nspng_convert(સંરચના content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	અક્ષર *title;

	પ્રતિજ્ઞા(png_c->png != ખાલી);
	પ્રતિજ્ઞા(png_c->info != ખાલી);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	જો (png_c->bitmap != ખાલી) {
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

	પાછા true;
}


સ્થિર nserror nspng_clone(અચલ સંરચના content *old_c, સંરચના content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	અચલ uint8_t *data;
	size_t size;

	clone_png_c = calloc(1, કદ(nspng_content));
	જો (clone_png_c == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	જો (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		પાછા error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	જો (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		પાછા error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	જો (size > 0) {
		જો (nspng_process_data(&clone_png_c->base, (અચલ અક્ષર *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			પાછા NSERROR_NOMEM;
		}
	}

	જો ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		જો (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (સંરચના content *)clone_png_c;

	પાછા NSERROR_OK;
}

સ્થિર અચલ content_handler nspng_content_handler = {
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

સ્થિર અચલ અક્ષર *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
