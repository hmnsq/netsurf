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

#اگر_غیر_تعریف_شدہ png_jmpbuf
#warning you have an antique libpng
#تعریف_کرو png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#اگر_ختم

#اگر PNG_LIBPNG_VER < 10209
#تعریف_کرو png_سیٹ_expand_سرمئی_1_2_4_مقصد_8(png) png_سیٹ_سرمئی_1_2_4_مقصد_8(png)
#اگر_ختم

قسمتعریف ساخت nspng_content {
	ساخت content بنیاد; /**< base content type */

	bool no_process_ڈیٹا; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	صحیح interlace;
	ساخت bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ساکن غیرعلامتی صحیح interlace_آغاز[8] = {0, 16, 0, 8, 0, 4, 0};
ساکن غیرعلامتی صحیح interlace_قدم[8] = {28, 28, 12, 12, 4, 4, 0};
ساکن غیرعلامتی صحیح interlace_صف_آغاز[8] = {0, 0, 4, 0, 2, 0, 1};
ساکن غیرعلامتی صحیح interlace_صف_قدم[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
شمار nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ساکن خالی nspng_warning(png_structp png_ptr, png_const_charp warning_پیغام)
{
	NSLOG(netsurf, INFO, "%s", warning_پیغام);
}

/**
 * nspng_error -- callback for libpng errors
 */
ساکن خالی nspng_error(png_structp png_ptr, png_const_charp error_پیغام)
{
	NSLOG(netsurf, INFO, "%s", error_پیغام);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ساکن خالی nspng_تیاری_transforms(png_structp png_ptr, png_infop info_ptr)
{
	صحیح ثنائی_ہندسہ_گہرائی, color_type, intent;
	دوہرا gamma;

	ثنائی_ہندسہ_گہرائی = png_get_ثنائی_ہندسہ_گہرائی(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	اگر (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_سیٹ_palette_مقصد_rgb(png_ptr);
	}

	اگر ((color_type == PNG_COLOR_TYPE_سرمئی) && (ثنائی_ہندسہ_گہرائی < 8)) {
		png_سیٹ_expand_سرمئی_1_2_4_مقصد_8(png_ptr);
	}

	اگر (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_سیٹ_t_RNS_مقصد_alpha(png_ptr);
	}

	اگر (ثنائی_ہندسہ_گہرائی == 16) {
		png_سیٹ_strip_16(png_ptr);
	}

	اگر (color_type == PNG_COLOR_TYPE_سرمئی ||
	    color_type == PNG_COLOR_TYPE_سرمئی_ALPHA) {
		png_سیٹ_سرمئی_مقصد_rgb(png_ptr);
	}

	انتخاب (bitmap_fmt.layout) {
	حالت BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	حالت BITMAP_LAYOUT_A8B8G8R8:
		png_سیٹ_bgr(png_ptr);
		توڑو;
	طےشدہ:
		/* RGB is the default. */
		توڑو;
	}

	اگر (!(color_type & PNG_COLOR_نقاب_ALPHA)) {
		انتخاب (bitmap_fmt.layout) {
		حالت BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالت BITMAP_LAYOUT_A8B8G8R8:
			png_سیٹ_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			توڑو;

		طےشدہ:
			png_سیٹ_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			توڑو;
		}
	} ورنہ {
		انتخاب (bitmap_fmt.layout) {
		حالت BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالت BITMAP_LAYOUT_A8B8G8R8:
			png_سیٹ_تبادلہ_alpha(png_ptr);
			توڑو;
		طےشدہ:
			/* Alpha as final component is the default. */
			توڑو;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	اگر (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_سیٹ_gamma(png_ptr, 2.2, 0.45455);
	} ورنہ {
		اگر (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_سیٹ_gamma(png_ptr, 2.2, gamma);
		} ورنہ {
			png_سیٹ_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ساکن خالی info_callback(png_structp png_s, png_infop info)
{
	صحیح interlace;
	png_uint_32 چوڑائی, اونچائی;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	چوڑائی = png_get_image_چوڑائی(png_s, info);
	اونچائی = png_get_image_اونچائی(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->بنیاد.چوڑائی = چوڑائی;
	png_c->بنیاد.اونچائی = اونچائی;
	png_c->بنیاد.سائز += چوڑائی * اونچائی * 4;

	/* see if progressive-conversion should continue */
	اگر (image_cache_speculate((ساخت content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(چوڑائی, اونچائی, BITMAP_NONE);
	اگر (png_c->bitmap == تہی) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = حجم(uint32_t);

	nspng_تیاری_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (غیرعلامتی طویل)چوڑائی, (غیرعلامتی طویل)اونچائی, png_c->rowbytes);
}

ساکن خالی صف_callback(png_structp png_s, png_bytep new_صف,
			 png_uint_32 صف_عدد, صحیح pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	غیرعلامتی طویل rowbytes = png_c->rowbytes;
	غیرعلامتی حرف *بفر, *صف;

	/* Give up if there's no bitmap */
	اگر (png_c->bitmap == تہی)
		واپس;

	/* Abort if we've not got any data */
	اگر (new_صف == تہی)
		واپس;

	/* Get bitmap buffer */
	بفر = guit->bitmap->get_بفر(png_c->bitmap);
	اگر (بفر == تہی) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	صف = بفر + (png_c->rowstride * صف_عدد);

	/* Handle interlaced sprites using the Adam7 algorithm */
	اگر (png_c->interlace) {
		غیرعلامتی طویل dst_off;
		غیرعلامتی طویل src_off = 0;
		غیرعلامتی صحیح آغاز, قدم;

		آغاز = interlace_آغاز[pass];
		قدم = interlace_قدم[pass];
		صف_عدد = interlace_صف_آغاز[pass] +
			interlace_صف_قدم[pass] * صف_عدد;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		صف = بفر + (png_c->rowstride * صف_عدد);

		برائے (dst_off = آغاز; dst_off < rowbytes; dst_off += قدم) {
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
		}
	} ورنہ {
		/* Do a fast memcpy of the row data */
		حافظہ_نقل(صف, new_صف, rowbytes);
	}
}


ساکن خالی انتہا_callback(png_structp png_s, png_infop info)
{
}

ساکن nserror nspng_create_png_ڈیٹا(nspng_content *png_c)
{
	png_c->bitmap = تہی;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	اگر (png_c->png == تہی) {
		content_نشر_error(&png_c->بنیاد, NSERROR_NOMEM, تہی);
		واپس NSERROR_NOMEM;
	}

	png_سیٹ_error_fn(png_c->png, تہی, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	اگر (png_c->info == تہی) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_نشر_error(&png_c->بنیاد, NSERROR_NOMEM, تہی);
		واپس NSERROR_NOMEM;
	}

	اگر (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = تہی;
		png_c->info = تہی;

		content_نشر_error(&png_c->بنیاد, NSERROR_PNG_ERROR, تہی);
		واپس NSERROR_NOMEM;
	}

	png_سیٹ_progressive_read_fn(png_c->png, png_c,
			info_callback, صف_callback, انتہا_callback);

	واپس NSERROR_OK;
}

ساکن nserror
nspng_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت ساخت http_parameter *params,
	     ساخت llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     ساخت content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = صفر_حافظہ_دو(1, حجم(nspng_content));
	اگر (png_c == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&png_c->بنیاد, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	اگر (error != NSERROR_OK) {
		آزاد(png_c);
		واپس error;
	}

	error = nspng_create_png_ڈیٹا(png_c);
	اگر (error != NSERROR_OK) {
		آزاد(png_c);
		واپس error;
	}

	*c = (ساخت content *)png_c;

	واپس NSERROR_OK;
}


ساکن bool nspng_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا,
			       غیرعلامتی صحیح سائز)
{
	nspng_content *png_c = (nspng_content *)c;
	متلون bool ret = true;

	اگر (png_c->no_process_ڈیٹا) {
		واپس ret;
	}

	انتخاب (setjmp(png_jmpbuf(png_c->png))) {
	حالت CBERR_NONE: /* direct return */	
		png_process_ڈیٹا(png_c->png, png_c->info, (uint8_t *)ڈیٹا, سائز);
		توڑو;

	حالت CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_ڈیٹا = true;
		توڑو;

	طےشدہ: /* fatal error from library processing png */
		اگر (png_c->bitmap != تہی) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_ڈیٹا = true;
		} ورنہ {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = تہی;
			png_c->info = تہی;

			content_نشر_error(c, NSERROR_PNG_ERROR, تہی);

			ret = false;

		}
		توڑو;
	}

	واپس ret;
}

ساخت png_cache_read_ڈیٹا_s {
	ثابت uint8_t *ڈیٹا;
	size_t سائز;
};

/** PNG library read fucntion to read data from a memory array 
 */
ساکن خالی 
png_cache_read_fn(png_structp png_ptr, png_bytep ڈیٹا, png_سائز_t لمبائی)
{
	ساخت png_cache_read_ڈیٹا_s *png_cache_read_ڈیٹا;
	png_cache_read_ڈیٹا = png_get_io_ptr(png_ptr);

	اگر (لمبائی > png_cache_read_ڈیٹا->سائز) {
		لمبائی = png_cache_read_ڈیٹا->سائز;
	}

	اگر (لمبائی == 0) {
		png_error(png_ptr, "Read Error");
	}

	حافظہ_نقل(ڈیٹا, png_cache_read_ڈیٹا->ڈیٹا, لمبائی);

	png_cache_read_ڈیٹا->ڈیٹا += لمبائی;
	png_cache_read_ڈیٹا->سائز -= لمبائی;
}

/** calculate an array of row pointers into a bitmap data area
 */
ساکن png_bytep *calc_صف_اشارے(ساخت bitmap *bitmap)
{
	صحیح اونچائی = guit->bitmap->get_اونچائی(bitmap);
	غیرعلامتی حرف *بفر= guit->bitmap->get_بفر(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *صف_ptrs;
	صحیح hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	اگر (بفر == تہی) {
		واپس تہی;
	}

	صف_ptrs = حافظہ_دو(حجم(png_bytep) * اونچائی);

	اگر (صف_ptrs != تہی) {
		برائے (hloop = 0; hloop < اونچائی; hloop++) {
			صف_ptrs[hloop] = بفر + (rowstride * hloop);
		}
	}

	واپس صف_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ساکن ساخت bitmap *
png_cache_convert(ساخت content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop انتہا_info_ptr;
	متلون ساخت bitmap * متلون bitmap = تہی;
	ساخت png_cache_read_ڈیٹا_s png_cache_read_ڈیٹا;
	png_uint_32 چوڑائی, اونچائی;
	متلون png_bytep * متلون صف_اشارے = تہی;

	png_cache_read_ڈیٹا.ڈیٹا = 
		content_get_ماخذ_ڈیٹا_2(c, &png_cache_read_ڈیٹا.سائز);

	اگر ((png_cache_read_ڈیٹا.ڈیٹا == تہی) || 
	    (png_cache_read_ڈیٹا.سائز <= 8)) {
		واپس تہی;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, تہی,
			nspng_error, nspng_warning);
	اگر (png_ptr == تہی) {
		واپس تہی;
	}

	info_ptr = png_create_info_struct(png_ptr);
	اگر (info_ptr == تہی) {
		png_destroy_read_struct(&png_ptr, تہی, تہی);
		واپس تہی;
	}

	انتہا_info_ptr = png_create_info_struct(png_ptr);
	اگر (انتہا_info_ptr == تہی) {
		png_destroy_read_struct(&png_ptr, &info_ptr, تہی);
		واپس تہی;
	}

	/* setup error exit path */
	اگر (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		جاؤ png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_سیٹ_read_fn(png_ptr, &png_cache_read_ڈیٹا, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_تیاری_transforms(png_ptr, info_ptr);

	چوڑائی = png_get_image_چوڑائی(png_ptr, info_ptr);
	اونچائی = png_get_image_اونچائی(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(چوڑائی, اونچائی, BITMAP_NONE);
	اگر (bitmap == تہی) {
		/* cleanup and bail */
		جاؤ png_cache_convert_error;
	}

	صف_اشارے = calc_صف_اشارے((ساخت bitmap *) bitmap);

	اگر (صف_اشارے != تہی) {
		png_read_image(png_ptr, (png_bytep *) صف_اشارے);
	} ورنہ {
		guit->bitmap->destroy((ساخت bitmap *)bitmap);
		bitmap = تہی;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &انتہا_info_ptr);

	اگر (صف_اشارے != تہی) {
		آزاد((png_bytep *) صف_اشارے);
	}

	اگر (bitmap != تہی) {
		bool opaque = bitmap_test_opaque((خالی *)bitmap);
		guit->bitmap->سیٹ_opaque((خالی *)bitmap, opaque);
		bitmap_format_مقصد_client((خالی *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((خالی *)bitmap);
	}

	واپس (ساخت bitmap *)bitmap;
}

ساکن bool nspng_convert(ساخت content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	حرف *title;

	أكد(png_c->png != تہی);
	أكد(png_c->info != تہی);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اونچائی);
	اگر (title != تہی) {
		content_سیٹ_title(c, title);
		آزاد(title);
	}

	اگر (png_c->bitmap != تہی) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->سیٹ_opaque(png_c->bitmap, opaque);
		bitmap_format_مقصد_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_جوڑو(c, png_c->bitmap, png_cache_convert);

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
	content_سیٹ_حالت(c, "");

	واپس true;
}


ساکن nserror nspng_clone(ثابت ساخت content *old_c, ساخت content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;

	clone_png_c = صفر_حافظہ_دو(1, حجم(nspng_content));
	اگر (clone_png_c == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&clone_png_c->بنیاد);
		واپس error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_ڈیٹا(clone_png_c);
	اگر (error != NSERROR_OK) {
		content_destroy(&clone_png_c->بنیاد);
		واپس error;
	}

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(&clone_png_c->بنیاد, &سائز);
	اگر (سائز > 0) {
		اگر (nspng_process_ڈیٹا(&clone_png_c->بنیاد, (ثابت حرف *)ڈیٹا, سائز) == false) {
			content_destroy(&clone_png_c->بنیاد);
			واپس NSERROR_NOMEM;
		}
	}

	اگر ((old_c->حالت_2 == CONTENT_حالت_READY) ||
	    (old_c->حالت_2 == CONTENT_حالت_مکمل)) {
		اگر (nspng_convert(&clone_png_c->بنیاد) == false) {
			content_destroy(&clone_png_c->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (ساخت content *)clone_png_c;

	واپس NSERROR_OK;
}

ساکن ثابت content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_ڈیٹا = nspng_process_ڈیٹا,
	.ڈیٹا_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_اندرونی = image_cache_get_اندرونی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_رجسٹر_TYPES(nspng, nspng_types, nspng_content_handler);
