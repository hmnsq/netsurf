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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <png.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"
#अन्तर्भाव "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#ifndef png_jmpbuf
#warning you have an antique libpng
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#जर PNG_LIBPNG_VER < 10209
#define png_set_expand_gray_1_2_4_to_8(png) png_set_gray_1_2_4_to_8(png)
#endif

प्रकारव्याख्या संरचना nspng_content {
	संरचना content base; /**< base content type */

	bool no_process_data; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	पूर्णांक interlace;
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */
	माप_प्रकार rowstride, bpp; /**< Bitmap rowstride and bpp */
	माप_प्रकार rowbytes; /**< Number of bytes per row */
} nspng_content;

स्थिर अचिन्हित पूर्णांक interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
स्थिर अचिन्हित पूर्णांक interlace_step[8] = {28, 28, 12, 12, 4, 4, 0};
स्थिर अचिन्हित पूर्णांक interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
स्थिर अचिन्हित पूर्णांक interlace_row_step[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
गणना nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
स्थिर शून्य nspng_warning(png_structp png_ptr, png_const_charp warning_message)
{
	NSLOG(netsurf, INFO, "%s", warning_message);
}

/**
 * nspng_error -- callback for libpng errors
 */
स्थिर शून्य nspng_error(png_structp png_ptr, png_const_charp error_message)
{
	NSLOG(netsurf, INFO, "%s", error_message);
	दीर्घ_लाँघ(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

स्थिर शून्य nspng_setup_transforms(png_structp png_ptr, png_infop info_ptr)
{
	पूर्णांक bit_depth, color_type, intent;
	द्विगुण gamma;

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	जर (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	जर ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	जर (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	जर (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	जर (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	निवड (bitmap_fmt.layout) {
	स्थिती BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	स्थिती BITMAP_LAYOUT_A8B8G8R8:
		png_set_bgr(png_ptr);
		थांबा;
	पूर्वनिर्धारित:
		/* RGB is the default. */
		थांबा;
	}

	जर (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		निवड (bitmap_fmt.layout) {
		स्थिती BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		स्थिती BITMAP_LAYOUT_A8B8G8R8:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			थांबा;

		पूर्वनिर्धारित:
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			थांबा;
		}
	} अन्यथा {
		निवड (bitmap_fmt.layout) {
		स्थिती BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		स्थिती BITMAP_LAYOUT_A8B8G8R8:
			png_set_swap_alpha(png_ptr);
			थांबा;
		पूर्वनिर्धारित:
			/* Alpha as final component is the default. */
			थांबा;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	जर (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, 2.2, 0.45455);
	} अन्यथा {
		जर (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_set_gamma(png_ptr, 2.2, gamma);
		} अन्यथा {
			png_set_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
स्थिर शून्य info_callback(png_structp png_s, png_infop info)
{
	पूर्णांक interlace;
	png_uint_32 width, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	width = png_get_image_width(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->base.width = width;
	png_c->base.height = height;
	png_c->base.size += width * height * 4;

	/* see if progressive-conversion should continue */
	जर (image_cache_speculate((संरचना content *)png_c) == false) {
		दीर्घ_लाँघ(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(width, height, BITMAP_NONE);
	जर (png_c->bitmap == रिक्त) {
		/* Failed to create bitmap skip pre-conversion */
		दीर्घ_लाँघ(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = आकार(uint32_t);

	nspng_setup_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (अचिन्हित दीर्घ)width, (अचिन्हित दीर्घ)height, png_c->rowbytes);
}

स्थिर शून्य row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_अंक, पूर्णांक pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	अचिन्हित दीर्घ rowbytes = png_c->rowbytes;
	अचिन्हित अक्षर *buffer, *row;

	/* Give up if there's no bitmap */
	जर (png_c->bitmap == रिक्त)
		परत;

	/* Abort if we've not got any data */
	जर (new_row == रिक्त)
		परत;

	/* Get bitmap buffer */
	buffer = guit->bitmap->get_buffer(png_c->bitmap);
	जर (buffer == रिक्त) {
		/* No buffer, bail out */
		दीर्घ_लाँघ(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = buffer + (png_c->rowstride * row_अंक);

	/* Handle interlaced sprites using the Adam7 algorithm */
	जर (png_c->interlace) {
		अचिन्हित दीर्घ dst_off;
		अचिन्हित दीर्घ src_off = 0;
		अचिन्हित पूर्णांक start, step;

		start = interlace_start[pass];
		step = interlace_step[pass];
		row_अंक = interlace_row_start[pass] +
			interlace_row_step[pass] * row_अंक;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = buffer + (png_c->rowstride * row_अंक);

		साठी (dst_off = start; dst_off < rowbytes; dst_off += step) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} अन्यथा {
		/* Do a fast memcpy of the row data */
		स_नकल(row, new_row, rowbytes);
	}
}


स्थिर शून्य end_callback(png_structp png_s, png_infop info)
{
}

स्थिर nserror nspng_create_png_data(nspng_content *png_c)
{
	png_c->bitmap = रिक्त;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	जर (png_c->png == रिक्त) {
		content_broadcast_error(&png_c->base, NSERROR_NOMEM, रिक्त);
		परत NSERROR_NOMEM;
	}

	png_set_error_fn(png_c->png, रिक्त, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	जर (png_c->info == रिक्त) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_broadcast_error(&png_c->base, NSERROR_NOMEM, रिक्त);
		परत NSERROR_NOMEM;
	}

	जर (बनाओ_लाँघ(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = रिक्त;
		png_c->info = रिक्त;

		content_broadcast_error(&png_c->base, NSERROR_PNG_ERROR, रिक्त);
		परत NSERROR_NOMEM;
	}

	png_set_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, end_callback);

	परत NSERROR_OK;
}

स्थिर nserror
nspng_create(स्थिरांक content_handler *handler,
	     lwc_string *imime_type,
	     स्थिरांक संरचना http_parameter *params,
	     संरचना llcache_handle *llcache,
	     स्थिरांक अक्षर *fallback_charset,
	     bool quirks,
	     संरचना content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = सुस्मृति(1, आकार(nspng_content));
	जर (png_c == रिक्त)
		परत NSERROR_NOMEM;

	error = content__init(&png_c->base, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	जर (error != NSERROR_OK) {
		मुक्त(png_c);
		परत error;
	}

	error = nspng_create_png_data(png_c);
	जर (error != NSERROR_OK) {
		मुक्त(png_c);
		परत error;
	}

	*c = (संरचना content *)png_c;

	परत NSERROR_OK;
}


स्थिर bool nspng_process_data(संरचना content *c, स्थिरांक अक्षर *data,
			       अचिन्हित पूर्णांक size)
{
	nspng_content *png_c = (nspng_content *)c;
	अस्थिर bool ret = true;

	जर (png_c->no_process_data) {
		परत ret;
	}

	निवड (बनाओ_लाँघ(png_jmpbuf(png_c->png))) {
	स्थिती CBERR_NONE: /* direct return */	
		png_process_data(png_c->png, png_c->info, (uint8_t *)data, size);
		थांबा;

	स्थिती CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_data = true;
		थांबा;

	पूर्वनिर्धारित: /* fatal error from library processing png */
		जर (png_c->bitmap != रिक्त) {
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
		} अन्यथा {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = रिक्त;
			png_c->info = रिक्त;

			content_broadcast_error(c, NSERROR_PNG_ERROR, रिक्त);

			ret = false;

		}
		थांबा;
	}

	परत ret;
}

संरचना png_cache_read_data_s {
	स्थिरांक uint8_t *data;
	माप_प्रकार size;
};

/** PNG library read fucntion to read data from a memory array 
 */
स्थिर शून्य 
png_cache_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	संरचना png_cache_read_data_s *png_cache_read_data;
	png_cache_read_data = png_get_io_ptr(png_ptr);

	जर (length > png_cache_read_data->size) {
		length = png_cache_read_data->size;
	}

	जर (length == 0) {
		png_error(png_ptr, "Read Error");
	}

	स_नकल(data, png_cache_read_data->data, length);

	png_cache_read_data->data += length;
	png_cache_read_data->size -= length;
}

/** calculate an array of row pointers into a bitmap data area
 */
स्थिर png_bytep *calc_row_pointers(संरचना bitmap *bitmap)
{
	पूर्णांक height = guit->bitmap->get_height(bitmap);
	अचिन्हित अक्षर *buffer= guit->bitmap->get_buffer(bitmap);
	माप_प्रकार rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	पूर्णांक hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	जर (buffer == रिक्त) {
		परत रिक्त;
	}

	row_ptrs = दो_स्मृति(आकार(png_bytep) * height);

	जर (row_ptrs != रिक्त) {
		साठी (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = buffer + (rowstride * hloop);
		}
	}

	परत row_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
स्थिर संरचना bitmap *
png_cache_convert(संरचना content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	अस्थिर संरचना bitmap * अस्थिर bitmap = रिक्त;
	संरचना png_cache_read_data_s png_cache_read_data;
	png_uint_32 width, height;
	अस्थिर png_bytep * अस्थिर row_pointers = रिक्त;

	png_cache_read_data.data = 
		content__get_source_data(c, &png_cache_read_data.size);

	जर ((png_cache_read_data.data == रिक्त) || 
	    (png_cache_read_data.size <= 8)) {
		परत रिक्त;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, रिक्त,
			nspng_error, nspng_warning);
	जर (png_ptr == रिक्त) {
		परत रिक्त;
	}

	info_ptr = png_create_info_struct(png_ptr);
	जर (info_ptr == रिक्त) {
		png_destroy_read_struct(&png_ptr, रिक्त, रिक्त);
		परत रिक्त;
	}

	end_info_ptr = png_create_info_struct(png_ptr);
	जर (end_info_ptr == रिक्त) {
		png_destroy_read_struct(&png_ptr, &info_ptr, रिक्त);
		परत रिक्त;
	}

	/* setup error exit path */
	जर (बनाओ_लाँघ(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		जा png_cache_convert_error;
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
	जर (bitmap == रिक्त) {
		/* cleanup and bail */
		जा png_cache_convert_error;
	}

	row_pointers = calc_row_pointers((संरचना bitmap *) bitmap);

	जर (row_pointers != रिक्त) {
		png_read_image(png_ptr, (png_bytep *) row_pointers);
	} अन्यथा {
		guit->bitmap->destroy((संरचना bitmap *)bitmap);
		bitmap = रिक्त;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	जर (row_pointers != रिक्त) {
		मुक्त((png_bytep *) row_pointers);
	}

	जर (bitmap != रिक्त) {
		bool opaque = bitmap_test_opaque((शून्य *)bitmap);
		guit->bitmap->set_opaque((शून्य *)bitmap, opaque);
		bitmap_format_to_client((शून्य *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((शून्य *)bitmap);
	}

	परत (संरचना bitmap *)bitmap;
}

स्थिर bool nspng_convert(संरचना content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	अक्षर *title;

	प्रतिज्ञा(png_c->png != रिक्त);
	प्रतिज्ञा(png_c->info != रिक्त);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	जर (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	जर (png_c->bitmap != रिक्त) {
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

	परत true;
}


स्थिर nserror nspng_clone(स्थिरांक संरचना content *old_c, संरचना content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	स्थिरांक uint8_t *data;
	माप_प्रकार size;

	clone_png_c = सुस्मृति(1, आकार(nspng_content));
	जर (clone_png_c == रिक्त)
		परत NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->base);
	जर (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		परत error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_data(clone_png_c);
	जर (error != NSERROR_OK) {
		content_destroy(&clone_png_c->base);
		परत error;
	}

	data = content__get_source_data(&clone_png_c->base, &size);
	जर (size > 0) {
		जर (nspng_process_data(&clone_png_c->base, (स्थिरांक अक्षर *)data, size) == false) {
			content_destroy(&clone_png_c->base);
			परत NSERROR_NOMEM;
		}
	}

	जर ((old_c->status == CONTENT_STATUS_READY) ||
	    (old_c->status == CONTENT_STATUS_DONE)) {
		जर (nspng_convert(&clone_png_c->base) == false) {
			content_destroy(&clone_png_c->base);
			परत NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (संरचना content *)clone_png_c;

	परत NSERROR_OK;
}

स्थिर स्थिरांक content_handler nspng_content_handler = {
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

स्थिर स्थिरांक अक्षर *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nspng, nspng_types, nspng_content_handler);
