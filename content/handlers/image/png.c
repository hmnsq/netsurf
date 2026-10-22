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

#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <stdlib.h>
#ضمن <png.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image_cache.h"
#ضمن "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#جے_بنا_تعریف png_jmpbuf
#warning you have an antique libpng
#تعریف_کرو png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#جے_انت

#إذا PNG_LIBPNG_VER < 10209
#تعریف_کرو png_سیٹ_expand_سلیٹی_1_2_4_منزل_8(png) png_سیٹ_سلیٹی_1_2_4_منزل_8(png)
#جے_انت

تعريفنوع بنية nspng_content {
	بنية content ادھار; /**< base content type */

	bool no_process_انکڑے; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	صحيح interlace;
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

ساكن بنانشانی صحيح interlace_شروع[8] = {0, 16, 0, 8, 0, 4, 0};
ساكن بنانشانی صحيح interlace_قدم[8] = {28, 28, 12, 12, 4, 4, 0};
ساكن بنانشانی صحيح interlace_صف_شروع[8] = {0, 0, 4, 0, 2, 0, 1};
ساكن بنانشانی صحيح interlace_صف_قدم[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
تعداد nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
ساكن فراغ nspng_warning(png_structp png_ptr, png_const_charp warning_سنیہا)
{
	NSLOG(netsurf, INFO, "%s", warning_سنیہا);
}

/**
 * nspng_error -- callback for libpng errors
 */
ساكن فراغ nspng_error(png_structp png_ptr, png_const_charp error_سنیہا)
{
	NSLOG(netsurf, INFO, "%s", error_سنیہا);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

ساكن فراغ nspng_تیاری_transforms(png_structp png_ptr, png_infop info_ptr)
{
	صحيح ثنائی_ہندسہ_ڈونگھائی, color_type, intent;
	مزدوج gamma;

	ثنائی_ہندسہ_ڈونگھائی = png_get_ثنائی_ہندسہ_ڈونگھائی(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	إذا (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_سیٹ_palette_منزل_rgb(png_ptr);
	}

	إذا ((color_type == PNG_COLOR_TYPE_سلیٹی) && (ثنائی_ہندسہ_ڈونگھائی < 8)) {
		png_سیٹ_expand_سلیٹی_1_2_4_منزل_8(png_ptr);
	}

	إذا (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_سیٹ_t_RNS_منزل_alpha(png_ptr);
	}

	إذا (ثنائی_ہندسہ_ڈونگھائی == 16) {
		png_سیٹ_strip_16(png_ptr);
	}

	إذا (color_type == PNG_COLOR_TYPE_سلیٹی ||
	    color_type == PNG_COLOR_TYPE_سلیٹی_ALPHA) {
		png_سیٹ_سلیٹی_منزل_rgb(png_ptr);
	}

	بدل (bitmap_fmt.layout) {
	حالة BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	حالة BITMAP_LAYOUT_A8B8G8R8:
		png_سیٹ_bgr(png_ptr);
		اكسر;
	افتراضي:
		/* RGB is the default. */
		اكسر;
	}

	إذا (!(color_type & PNG_COLOR_نقاب_ALPHA)) {
		بدل (bitmap_fmt.layout) {
		حالة BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالة BITMAP_LAYOUT_A8B8G8R8:
			png_سیٹ_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			اكسر;

		افتراضي:
			png_سیٹ_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			اكسر;
		}
	} وإلا {
		بدل (bitmap_fmt.layout) {
		حالة BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		حالة BITMAP_LAYOUT_A8B8G8R8:
			png_سیٹ_وٹاندرا_alpha(png_ptr);
			اكسر;
		افتراضي:
			/* Alpha as final component is the default. */
			اكسر;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	إذا (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_سیٹ_gamma(png_ptr, 2.2, 0.45455);
	} وإلا {
		إذا (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_سیٹ_gamma(png_ptr, 2.2, gamma);
		} وإلا {
			png_سیٹ_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
ساكن فراغ info_callback(png_structp png_s, png_infop info)
{
	صحيح interlace;
	png_uint_32 چوڑائی, اچائی;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	چوڑائی = png_get_image_چوڑائی(png_s, info);
	اچائی = png_get_image_اچائی(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->ادھار.چوڑائی = چوڑائی;
	png_c->ادھار.اچائی = اچائی;
	png_c->ادھار.ناپ += چوڑائی * اچائی * 4;

	/* see if progressive-conversion should continue */
	إذا (image_cache_speculate((بنية content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(چوڑائی, اچائی, BITMAP_NONE);
	إذا (png_c->bitmap == عدم) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = حجم(uint32_t);

	nspng_تیاری_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (بنانشانی لما)چوڑائی, (بنانشانی لما)اچائی, png_c->rowbytes);
}

ساكن فراغ صف_callback(png_structp png_s, png_bytep new_صف,
			 png_uint_32 صف_عدد, صحيح pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	بنانشانی لما rowbytes = png_c->rowbytes;
	بنانشانی حرف *بفر, *صف;

	/* Give up if there's no bitmap */
	إذا (png_c->bitmap == عدم)
		أرجع;

	/* Abort if we've not got any data */
	إذا (new_صف == عدم)
		أرجع;

	/* Get bitmap buffer */
	بفر = guit->bitmap->get_بفر(png_c->bitmap);
	إذا (بفر == عدم) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	صف = بفر + (png_c->rowstride * صف_عدد);

	/* Handle interlaced sprites using the Adam7 algorithm */
	إذا (png_c->interlace) {
		بنانشانی لما dst_off;
		بنانشانی لما src_off = 0;
		بنانشانی صحيح شروع, قدم;

		شروع = interlace_شروع[pass];
		قدم = interlace_قدم[pass];
		صف_عدد = interlace_صف_شروع[pass] +
			interlace_صف_قدم[pass] * صف_عدد;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		صف = بفر + (png_c->rowstride * صف_عدد);

		لكل (dst_off = شروع; dst_off < rowbytes; dst_off += قدم) {
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
			صف[dst_off++] = new_صف[src_off++];
		}
	} وإلا {
		/* Do a fast memcpy of the row data */
		حافظہ_نقل(صف, new_صف, rowbytes);
	}
}


ساكن فراغ سرا_callback(png_structp png_s, png_infop info)
{
}

ساكن nserror nspng_create_png_انکڑے(nspng_content *png_c)
{
	png_c->bitmap = عدم;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	إذا (png_c->png == عدم) {
		content_نشر_error(&png_c->ادھار, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}

	png_سیٹ_error_fn(png_c->png, عدم, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	إذا (png_c->info == عدم) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_نشر_error(&png_c->ادھار, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}

	إذا (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = عدم;
		png_c->info = عدم;

		content_نشر_error(&png_c->ادھار, NSERROR_PNG_ERROR, عدم);
		أرجع NSERROR_NOMEM;
	}

	png_سیٹ_progressive_read_fn(png_c->png, png_c,
			info_callback, صف_callback, سرا_callback);

	أرجع NSERROR_OK;
}

ساكن nserror
nspng_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت بنية http_parameter *params,
	     بنية llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     بنية content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = صفر_حافظہ_دیو(1, حجم(nspng_content));
	إذا (png_c == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&png_c->ادھار, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	إذا (error != NSERROR_OK) {
		حرر(png_c);
		أرجع error;
	}

	error = nspng_create_png_انکڑے(png_c);
	إذا (error != NSERROR_OK) {
		حرر(png_c);
		أرجع error;
	}

	*c = (بنية content *)png_c;

	أرجع NSERROR_OK;
}


ساكن bool nspng_process_انکڑے(بنية content *c, ثابت حرف *انکڑے,
			       بنانشانی صحيح ناپ)
{
	nspng_content *png_c = (nspng_content *)c;
	متطاير bool ret = true;

	إذا (png_c->no_process_انکڑے) {
		أرجع ret;
	}

	بدل (setjmp(png_jmpbuf(png_c->png))) {
	حالة CBERR_NONE: /* direct return */	
		png_process_انکڑے(png_c->png, png_c->info, (uint8_t *)انکڑے, ناپ);
		اكسر;

	حالة CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_انکڑے = true;
		اكسر;

	افتراضي: /* fatal error from library processing png */
		إذا (png_c->bitmap != عدم) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_انکڑے = true;
		} وإلا {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = عدم;
			png_c->info = عدم;

			content_نشر_error(c, NSERROR_PNG_ERROR, عدم);

			ret = false;

		}
		اكسر;
	}

	أرجع ret;
}

بنية png_cache_read_انکڑے_s {
	ثابت uint8_t *انکڑے;
	size_t ناپ;
};

/** PNG library read fucntion to read data from a memory array 
 */
ساكن فراغ 
png_cache_read_fn(png_structp png_ptr, png_bytep انکڑے, png_ناپ_t لمائی)
{
	بنية png_cache_read_انکڑے_s *png_cache_read_انکڑے;
	png_cache_read_انکڑے = png_get_io_ptr(png_ptr);

	إذا (لمائی > png_cache_read_انکڑے->ناپ) {
		لمائی = png_cache_read_انکڑے->ناپ;
	}

	إذا (لمائی == 0) {
		png_error(png_ptr, "Read Error");
	}

	حافظہ_نقل(انکڑے, png_cache_read_انکڑے->انکڑے, لمائی);

	png_cache_read_انکڑے->انکڑے += لمائی;
	png_cache_read_انکڑے->ناپ -= لمائی;
}

/** calculate an array of row pointers into a bitmap data area
 */
ساكن png_bytep *calc_صف_سوچکاں(بنية bitmap *bitmap)
{
	صحيح اچائی = guit->bitmap->get_اچائی(bitmap);
	بنانشانی حرف *بفر= guit->bitmap->get_بفر(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *صف_ptrs;
	صحيح hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	إذا (بفر == عدم) {
		أرجع عدم;
	}

	صف_ptrs = حافظہ_دیو(حجم(png_bytep) * اچائی);

	إذا (صف_ptrs != عدم) {
		لكل (hloop = 0; hloop < اچائی; hloop++) {
			صف_ptrs[hloop] = بفر + (rowstride * hloop);
		}
	}

	أرجع صف_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
ساكن بنية bitmap *
png_cache_convert(بنية content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop سرا_info_ptr;
	متطاير بنية bitmap * متطاير bitmap = عدم;
	بنية png_cache_read_انکڑے_s png_cache_read_انکڑے;
	png_uint_32 چوڑائی, اچائی;
	متطاير png_bytep * متطاير صف_سوچکاں = عدم;

	png_cache_read_انکڑے.انکڑے = 
		content_get_سروت_انکڑے_2(c, &png_cache_read_انکڑے.ناپ);

	إذا ((png_cache_read_انکڑے.انکڑے == عدم) || 
	    (png_cache_read_انکڑے.ناپ <= 8)) {
		أرجع عدم;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, عدم,
			nspng_error, nspng_warning);
	إذا (png_ptr == عدم) {
		أرجع عدم;
	}

	info_ptr = png_create_info_struct(png_ptr);
	إذا (info_ptr == عدم) {
		png_destroy_read_struct(&png_ptr, عدم, عدم);
		أرجع عدم;
	}

	سرا_info_ptr = png_create_info_struct(png_ptr);
	إذا (سرا_info_ptr == عدم) {
		png_destroy_read_struct(&png_ptr, &info_ptr, عدم);
		أرجع عدم;
	}

	/* setup error exit path */
	إذا (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		اذهب png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_سیٹ_read_fn(png_ptr, &png_cache_read_انکڑے, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_تیاری_transforms(png_ptr, info_ptr);

	چوڑائی = png_get_image_چوڑائی(png_ptr, info_ptr);
	اچائی = png_get_image_اچائی(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(چوڑائی, اچائی, BITMAP_NONE);
	إذا (bitmap == عدم) {
		/* cleanup and bail */
		اذهب png_cache_convert_error;
	}

	صف_سوچکاں = calc_صف_سوچکاں((بنية bitmap *) bitmap);

	إذا (صف_سوچکاں != عدم) {
		png_read_image(png_ptr, (png_bytep *) صف_سوچکاں);
	} وإلا {
		guit->bitmap->destroy((بنية bitmap *)bitmap);
		bitmap = عدم;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &سرا_info_ptr);

	إذا (صف_سوچکاں != عدم) {
		حرر((png_bytep *) صف_سوچکاں);
	}

	إذا (bitmap != عدم) {
		bool opaque = bitmap_test_opaque((فراغ *)bitmap);
		guit->bitmap->سیٹ_opaque((فراغ *)bitmap, opaque);
		bitmap_format_منزل_client((فراغ *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((فراغ *)bitmap);
	}

	أرجع (بنية bitmap *)bitmap;
}

ساكن bool nspng_convert(بنية content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	حرف *title;

	أكد(png_c->png != عدم);
	أكد(png_c->info != عدم);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_پتی(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اچائی);
	إذا (title != عدم) {
		content_سیٹ_title(c, title);
		حرر(title);
	}

	إذا (png_c->bitmap != عدم) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->سیٹ_opaque(png_c->bitmap, opaque);
		bitmap_format_منزل_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_جوڑو(c, png_c->bitmap, png_cache_convert);

	content_سیٹ_ready(c);
	content_سیٹ_پورا(c);
	content_سیٹ_حالت(c, "");

	أرجع true;
}


ساكن nserror nspng_clone(ثابت بنية content *old_c, بنية content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	ثابت uint8_t *انکڑے;
	size_t ناپ;

	clone_png_c = صفر_حافظہ_دیو(1, حجم(nspng_content));
	إذا (clone_png_c == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->ادھار);
	إذا (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ادھار);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_انکڑے(clone_png_c);
	إذا (error != NSERROR_OK) {
		content_destroy(&clone_png_c->ادھار);
		أرجع error;
	}

	انکڑے = content_get_سروت_انکڑے_2(&clone_png_c->ادھار, &ناپ);
	إذا (ناپ > 0) {
		إذا (nspng_process_انکڑے(&clone_png_c->ادھار, (ثابت حرف *)انکڑے, ناپ) == false) {
			content_destroy(&clone_png_c->ادھار);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا ((old_c->حالت == CONTENT_حالت_READY) ||
	    (old_c->حالت == CONTENT_حالت_پورا)) {
		إذا (nspng_convert(&clone_png_c->ادھار) == false) {
			content_destroy(&clone_png_c->ادھار);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (بنية content *)clone_png_c;

	أرجع NSERROR_OK;
}

ساكن ثابت content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_انکڑے = nspng_process_انکڑے,
	.انکڑے_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_اندرونی = image_cache_get_اندرونی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_رجسٹر_TYPES(nspng, nspng_types, nspng_content_handler);
