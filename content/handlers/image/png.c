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

#यदि_अपरिभाषितम् png_jmpbuf
#warning you have an antique libpng
#परिभाषय png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#यदि_अन्तः

#यदि PNG_LIBPNG_VER < 10209
#परिभाषय png_समुच्चयः_expand_gray_1_2_4_गन्तव्यम्_8(png) png_समुच्चयः_gray_1_2_4_गन्तव्यम्_8(png)
#यदि_अन्तः

प्रकारनिर्देश संरचना nspng_content {
	संरचना content आधारः; /**< base content type */

	bool no_process_दत्त; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	पूर्णाङ्क interlace;
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

स्थिर अचिह्नित पूर्णाङ्क interlace_start[8] = {0, 16, 0, 8, 0, 4, 0};
स्थिर अचिह्नित पूर्णाङ्क interlace_चरणः[8] = {28, 28, 12, 12, 4, 4, 0};
स्थिर अचिह्नित पूर्णाङ्क interlace_row_start[8] = {0, 0, 4, 0, 2, 0, 1};
स्थिर अचिह्नित पूर्णाङ्क interlace_row_चरणः[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
गणना nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
स्थिर शून्य nspng_warning(png_structp png_ptr, png_const_charp warning_सन्देशः)
{
	NSLOG(netsurf, INFO, "%s", warning_सन्देशः);
}

/**
 * nspng_error -- callback for libpng errors
 */
स्थिर शून्य nspng_error(png_structp png_ptr, png_const_charp error_सन्देशः)
{
	NSLOG(netsurf, INFO, "%s", error_सन्देशः);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

स्थिर शून्य nspng_सज्जता_transforms(png_structp png_ptr, png_infop info_ptr)
{
	पूर्णाङ्क bit_गभीरता, color_type, intent;
	द्विगुण gamma;

	bit_गभीरता = png_get_bit_गभीरता(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	यदि (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_समुच्चयः_palette_गन्तव्यम्_rgb(png_ptr);
	}

	यदि ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_गभीरता < 8)) {
		png_समुच्चयः_expand_gray_1_2_4_गन्तव्यम्_8(png_ptr);
	}

	यदि (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_समुच्चयः_t_RNS_गन्तव्यम्_alpha(png_ptr);
	}

	यदि (bit_गभीरता == 16) {
		png_समुच्चयः_strip_16(png_ptr);
	}

	यदि (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_समुच्चयः_gray_गन्तव्यम्_rgb(png_ptr);
	}

	विकल्प (bitmap_fmt.layout) {
	पक्ष BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	पक्ष BITMAP_LAYOUT_A8B8G8R8:
		png_समुच्चयः_bgr(png_ptr);
		भङ्ग;
	स्वभाव:
		/* RGB is the default. */
		भङ्ग;
	}

	यदि (!(color_type & PNG_COLOR_आवरणम्_ALPHA)) {
		विकल्प (bitmap_fmt.layout) {
		पक्ष BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		पक्ष BITMAP_LAYOUT_A8B8G8R8:
			png_समुच्चयः_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			भङ्ग;

		स्वभाव:
			png_समुच्चयः_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			भङ्ग;
		}
	} अन्यथा {
		विकल्प (bitmap_fmt.layout) {
		पक्ष BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		पक्ष BITMAP_LAYOUT_A8B8G8R8:
			png_समुच्चयः_swap_alpha(png_ptr);
			भङ्ग;
		स्वभाव:
			/* Alpha as final component is the default. */
			भङ्ग;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	यदि (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_समुच्चयः_gamma(png_ptr, 2.2, 0.45455);
	} अन्यथा {
		यदि (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_समुच्चयः_gamma(png_ptr, 2.2, gamma);
		} अन्यथा {
			png_समुच्चयः_gamma(png_ptr, 2.2, 0.45455);
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
	पूर्णाङ्क interlace;
	png_uint_32 विस्तारः, height;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	विस्तारः = png_get_image_विस्तारः(png_s, info);
	height = png_get_image_height(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->आधारः.विस्तारः = विस्तारः;
	png_c->आधारः.height = height;
	png_c->आधारः.आकार += विस्तारः * height * 4;

	/* see if progressive-conversion should continue */
	यदि (image_cache_speculate((संरचना content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(विस्तारः, height, BITMAP_NONE);
	यदि (png_c->bitmap == रिक्त) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = परिमाण(uint32_t);

	nspng_सज्जता_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (अचिह्नित दीर्घ)विस्तारः, (अचिह्नित दीर्घ)height, png_c->rowbytes);
}

स्थिर शून्य row_callback(png_structp png_s, png_bytep new_row,
			 png_uint_32 row_अङ्कः, पूर्णाङ्क pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	अचिह्नित दीर्घ rowbytes = png_c->rowbytes;
	अचिह्नित वर्ण *बफर, *row;

	/* Give up if there's no bitmap */
	यदि (png_c->bitmap == रिक्त)
		प्रत्यागम;

	/* Abort if we've not got any data */
	यदि (new_row == रिक्त)
		प्रत्यागम;

	/* Get bitmap buffer */
	बफर = guit->bitmap->get_बफर(png_c->bitmap);
	यदि (बफर == रिक्त) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	row = बफर + (png_c->rowstride * row_अङ्कः);

	/* Handle interlaced sprites using the Adam7 algorithm */
	यदि (png_c->interlace) {
		अचिह्नित दीर्घ dst_off;
		अचिह्नित दीर्घ src_off = 0;
		अचिह्नित पूर्णाङ्क start, चरणः;

		start = interlace_start[pass];
		चरणः = interlace_चरणः[pass];
		row_अङ्कः = interlace_row_start[pass] +
			interlace_row_चरणः[pass] * row_अङ्कः;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		row = बफर + (png_c->rowstride * row_अङ्कः);

		प्रति (dst_off = start; dst_off < rowbytes; dst_off += चरणः) {
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
			row[dst_off++] = new_row[src_off++];
		}
	} अन्यथा {
		/* Do a fast memcpy of the row data */
		स्मृतिं_प्रतिलिख(row, new_row, rowbytes);
	}
}


स्थिर शून्य अन्तः_callback(png_structp png_s, png_infop info)
{
}

स्थिर nserror nspng_create_png_दत्त(nspng_content *png_c)
{
	png_c->bitmap = रिक्त;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	यदि (png_c->png == रिक्त) {
		content_प्रसारणम्_error(&png_c->आधारः, NSERROR_NOMEM, रिक्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	png_समुच्चयः_error_fn(png_c->png, रिक्त, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	यदि (png_c->info == रिक्त) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_प्रसारणम्_error(&png_c->आधारः, NSERROR_NOMEM, रिक्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	यदि (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = रिक्त;
		png_c->info = रिक्त;

		content_प्रसारणम्_error(&png_c->आधारः, NSERROR_PNG_ERROR, रिक्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	png_समुच्चयः_progressive_read_fn(png_c->png, png_c,
			info_callback, row_callback, अन्तः_callback);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
nspng_create(ध्रुव content_handler *handler,
	     lwc_string *imime_type,
	     ध्रुव संरचना http_parameter *params,
	     संरचना llcache_handle *llcache,
	     ध्रुव वर्ण *fallback_charset,
	     bool quirks,
	     संरचना content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = शून्यस्मृतिं_देहि(1, परिमाण(nspng_content));
	यदि (png_c == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&png_c->आधारः, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(png_c);
		प्रत्यागम error;
	}

	error = nspng_create_png_दत्त(png_c);
	यदि (error != NSERROR_OK) {
		मुक्त(png_c);
		प्रत्यागम error;
	}

	*c = (संरचना content *)png_c;

	प्रत्यागम NSERROR_OK;
}


स्थिर bool nspng_process_दत्त(संरचना content *c, ध्रुव वर्ण *दत्त,
			       अचिह्नित पूर्णाङ्क आकार)
{
	nspng_content *png_c = (nspng_content *)c;
	चञ्चल bool ret = true;

	यदि (png_c->no_process_दत्त) {
		प्रत्यागम ret;
	}

	विकल्प (setjmp(png_jmpbuf(png_c->png))) {
	पक्ष CBERR_NONE: /* direct return */	
		png_process_दत्त(png_c->png, png_c->info, (uint8_t *)दत्त, आकार);
		भङ्ग;

	पक्ष CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_दत्त = true;
		भङ्ग;

	स्वभाव: /* fatal error from library processing png */
		यदि (png_c->bitmap != रिक्त) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_दत्त = true;
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

			content_प्रसारणम्_error(c, NSERROR_PNG_ERROR, रिक्त);

			ret = false;

		}
		भङ्ग;
	}

	प्रत्यागम ret;
}

संरचना png_cache_read_दत्त_s {
	ध्रुव uint8_t *दत्त;
	size_t आकार;
};

/** PNG library read fucntion to read data from a memory array 
 */
स्थिर शून्य 
png_cache_read_fn(png_structp png_ptr, png_bytep दत्त, png_आकार_t दीर्घता)
{
	संरचना png_cache_read_दत्त_s *png_cache_read_दत्त;
	png_cache_read_दत्त = png_get_io_ptr(png_ptr);

	यदि (दीर्घता > png_cache_read_दत्त->आकार) {
		दीर्घता = png_cache_read_दत्त->आकार;
	}

	यदि (दीर्घता == 0) {
		png_error(png_ptr, "Read Error");
	}

	स्मृतिं_प्रतिलिख(दत्त, png_cache_read_दत्त->दत्त, दीर्घता);

	png_cache_read_दत्त->दत्त += दीर्घता;
	png_cache_read_दत्त->आकार -= दीर्घता;
}

/** calculate an array of row pointers into a bitmap data area
 */
स्थिर png_bytep *calc_row_सूचकाः(संरचना bitmap *bitmap)
{
	पूर्णाङ्क height = guit->bitmap->get_height(bitmap);
	अचिह्नित वर्ण *बफर= guit->bitmap->get_बफर(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *row_ptrs;
	पूर्णाङ्क hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	यदि (बफर == रिक्त) {
		प्रत्यागम रिक्त;
	}

	row_ptrs = स्मृतिं_देहि(परिमाण(png_bytep) * height);

	यदि (row_ptrs != रिक्त) {
		प्रति (hloop = 0; hloop < height; hloop++) {
			row_ptrs[hloop] = बफर + (rowstride * hloop);
		}
	}

	प्रत्यागम row_ptrs;
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
	png_infop अन्तः_info_ptr;
	चञ्चल संरचना bitmap * चञ्चल bitmap = रिक्त;
	संरचना png_cache_read_दत्त_s png_cache_read_दत्त;
	png_uint_32 विस्तारः, height;
	चञ्चल png_bytep * चञ्चल row_सूचकाः = रिक्त;

	png_cache_read_दत्त.दत्त = 
		content_get_source_दत्त_2(c, &png_cache_read_दत्त.आकार);

	यदि ((png_cache_read_दत्त.दत्त == रिक्त) || 
	    (png_cache_read_दत्त.आकार <= 8)) {
		प्रत्यागम रिक्त;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, रिक्त,
			nspng_error, nspng_warning);
	यदि (png_ptr == रिक्त) {
		प्रत्यागम रिक्त;
	}

	info_ptr = png_create_info_struct(png_ptr);
	यदि (info_ptr == रिक्त) {
		png_destroy_read_struct(&png_ptr, रिक्त, रिक्त);
		प्रत्यागम रिक्त;
	}

	अन्तः_info_ptr = png_create_info_struct(png_ptr);
	यदि (अन्तः_info_ptr == रिक्त) {
		png_destroy_read_struct(&png_ptr, &info_ptr, रिक्त);
		प्रत्यागम रिक्त;
	}

	/* setup error exit path */
	यदि (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		गच्छ png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_समुच्चयः_read_fn(png_ptr, &png_cache_read_दत्त, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_सज्जता_transforms(png_ptr, info_ptr);

	विस्तारः = png_get_image_विस्तारः(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(विस्तारः, height, BITMAP_NONE);
	यदि (bitmap == रिक्त) {
		/* cleanup and bail */
		गच्छ png_cache_convert_error;
	}

	row_सूचकाः = calc_row_सूचकाः((संरचना bitmap *) bitmap);

	यदि (row_सूचकाः != रिक्त) {
		png_read_image(png_ptr, (png_bytep *) row_सूचकाः);
	} अन्यथा {
		guit->bitmap->destroy((संरचना bitmap *)bitmap);
		bitmap = रिक्त;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &अन्तः_info_ptr);

	यदि (row_सूचकाः != रिक्त) {
		मुक्त((png_bytep *) row_सूचकाः);
	}

	यदि (bitmap != रिक्त) {
		bool opaque = bitmap_test_opaque((शून्य *)bitmap);
		guit->bitmap->समुच्चयः_opaque((शून्य *)bitmap, opaque);
		bitmap_format_गन्तव्यम्_client((शून्य *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((शून्य *)bitmap);
	}

	प्रत्यागम (संरचना bitmap *)bitmap;
}

स्थिर bool nspng_convert(संरचना content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	वर्ण *title;

	प्रतिज्ञा(png_c->png != रिक्त);
	प्रतिज्ञा(png_c->info != रिक्त);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_पर्णम्(llcache_handle_get_url(c->llcache)),
			c->विस्तारः, c->height);
	यदि (title != रिक्त) {
		content_समुच्चयः_title(c, title);
		मुक्त(title);
	}

	यदि (png_c->bitmap != रिक्त) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->समुच्चयः_opaque(png_c->bitmap, opaque);
		bitmap_format_गन्तव्यम्_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_सङ्कलय(c, png_c->bitmap, png_cache_convert);

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);
	content_समुच्चयः_स्थितिः(c, "");

	प्रत्यागम true;
}


स्थिर nserror nspng_clone(ध्रुव संरचना content *old_c, संरचना content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ध्रुव uint8_t *दत्त;
	size_t आकार;

	clone_png_c = शून्यस्मृतिं_देहि(1, परिमाण(nspng_content));
	यदि (clone_png_c == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&clone_png_c->आधारः);
		प्रत्यागम error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_दत्त(clone_png_c);
	यदि (error != NSERROR_OK) {
		content_destroy(&clone_png_c->आधारः);
		प्रत्यागम error;
	}

	दत्त = content_get_source_दत्त_2(&clone_png_c->आधारः, &आकार);
	यदि (आकार > 0) {
		यदि (nspng_process_दत्त(&clone_png_c->आधारः, (ध्रुव वर्ण *)दत्त, आकार) == false) {
			content_destroy(&clone_png_c->आधारः);
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	यदि ((old_c->स्थितिः == CONTENT_स्थितिः_READY) ||
	    (old_c->स्थितिः == CONTENT_स्थितिः_समाप्तम्)) {
		यदि (nspng_convert(&clone_png_c->आधारः) == false) {
			content_destroy(&clone_png_c->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (संरचना content *)clone_png_c;

	प्रत्यागम NSERROR_OK;
}

स्थिर ध्रुव content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_दत्त = nspng_process_दत्त,
	.दत्त_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_आन्तरिकम् = image_cache_get_आन्तरिकम्,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nspng, nspng_types, nspng_content_handler);
