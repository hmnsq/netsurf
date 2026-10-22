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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <png.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/image_cache.h"
#అన్తర్భావ "image/png.h"

/* accommodate for old versions of libpng (beware security holes!) */

#నిర్వచితం_కాకపోతే png_jmpbuf
#warning you have an antique libpng
#నిర్వచించు png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#అయితే_ముగింపు

#ఒకవేళ PNG_LIBPNG_VER < 10209
#నిర్వచించు png_సమితి_expand_బూడిద_1_2_4_గమ్యం_8(png) png_సమితి_బూడిద_1_2_4_గమ్యం_8(png)
#అయితే_ముగింపు

రకనిర్వచనం నిర్మాణం nspng_content {
	నిర్మాణం content పునాది; /**< base content type */

	bool no_process_దత్తాంశం; /**< Do not continue to process data as it arrives */
	png_structp png;
	png_infop info;
	పూర్ణాంకం interlace;
	నిర్మాణం bitmap *bitmap;	/**< Created NetSurf bitmap */
	size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
	size_t rowbytes; /**< Number of bytes per row */
} nspng_content;

స్థిర గుర్తులేని పూర్ణాంకం interlace_ప్రారంభం[8] = {0, 16, 0, 8, 0, 4, 0};
స్థిర గుర్తులేని పూర్ణాంకం interlace_అడుగు[8] = {28, 28, 12, 12, 4, 4, 0};
స్థిర గుర్తులేని పూర్ణాంకం interlace_అడ్డువరుస_ప్రారంభం[8] = {0, 0, 4, 0, 2, 0, 1};
స్థిర గుర్తులేని పూర్ణాంకం interlace_అడ్డువరుస_అడుగు[8] = {8, 8, 8, 4, 4, 2, 2};

/** Callbak error numbers*/
గణన nspng_cberr {
	CBERR_NONE = 0, /* no error */
	CBERR_LIBPNG, /* error from png library */
	CBERR_NOPRE, /* no pre-conversion performed */
}; 

/**
 * nspng_warning -- callback for libpng warnings
 */
స్థిర శూన్య nspng_warning(png_structp png_ptr, png_const_charp warning_సందేశం)
{
	NSLOG(netsurf, INFO, "%s", warning_సందేశం);
}

/**
 * nspng_error -- callback for libpng errors
 */
స్థిర శూన్య nspng_error(png_structp png_ptr, png_const_charp error_సందేశం)
{
	NSLOG(netsurf, INFO, "%s", error_సందేశం);
	longjmp(png_jmpbuf(png_ptr), CBERR_LIBPNG);
}

స్థిర శూన్య nspng_అమరిక_transforms(png_structp png_ptr, png_infop info_ptr)
{
	పూర్ణాంకం బిట్టు_లోతు, color_type, intent;
	ద్విగుణ gamma;

	బిట్టు_లోతు = png_get_బిట్టు_లోతు(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Set up our transformations */
	ఒకవేళ (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_సమితి_palette_గమ్యం_rgb(png_ptr);
	}

	ఒకవేళ ((color_type == PNG_COLOR_TYPE_బూడిద) && (బిట్టు_లోతు < 8)) {
		png_సమితి_expand_బూడిద_1_2_4_గమ్యం_8(png_ptr);
	}

	ఒకవేళ (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_సమితి_t_RNS_గమ్యం_alpha(png_ptr);
	}

	ఒకవేళ (బిట్టు_లోతు == 16) {
		png_సమితి_strip_16(png_ptr);
	}

	ఒకవేళ (color_type == PNG_COLOR_TYPE_బూడిద ||
	    color_type == PNG_COLOR_TYPE_బూడిద_ALPHA) {
		png_సమితి_బూడిద_గమ్యం_rgb(png_ptr);
	}

	ఎంపిక (bitmap_fmt.layout) {
	స్థితి BITMAP_LAYOUT_B8G8R8A8: /* Fall through. */
	స్థితి BITMAP_LAYOUT_A8B8G8R8:
		png_సమితి_bgr(png_ptr);
		ఆపు;
	అప్రమేయ:
		/* RGB is the default. */
		ఆపు;
	}

	ఒకవేళ (!(color_type & PNG_COLOR_ముసుగు_ALPHA)) {
		ఎంపిక (bitmap_fmt.layout) {
		స్థితి BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		స్థితి BITMAP_LAYOUT_A8B8G8R8:
			png_సమితి_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
			ఆపు;

		అప్రమేయ:
			png_సమితి_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			ఆపు;
		}
	} లేకపోతే {
		ఎంపిక (bitmap_fmt.layout) {
		స్థితి BITMAP_LAYOUT_A8R8G8B8: /* Fall through. */
		స్థితి BITMAP_LAYOUT_A8B8G8R8:
			png_సమితి_మార్పిడి_alpha(png_ptr);
			ఆపు;
		అప్రమేయ:
			/* Alpha as final component is the default. */
			ఆపు;
		}
	}

	/* gamma correction - we use 2.2 as our screen gamma
	 * this appears to be correct (at least in respect to !Browse)
	 * see http://www.w3.org/Graphics/PNG/all_seven.html for a test case
	 */
	ఒకవేళ (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_సమితి_gamma(png_ptr, 2.2, 0.45455);
	} లేకపోతే {
		ఒకవేళ (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
			png_సమితి_gamma(png_ptr, 2.2, gamma);
		} లేకపోతే {
			png_సమితి_gamma(png_ptr, 2.2, 0.45455);
		}
	}

	png_read_update_info(png_ptr, info_ptr);
}

/**
 * info_callback -- PNG header has been completely received, prepare to process
 * image data
 */
స్థిర శూన్య info_callback(png_structp png_s, png_infop info)
{
	పూర్ణాంకం interlace;
	png_uint_32 వెడల్పు, ఎత్తు;
	nspng_content *png_c = png_get_progressive_ptr(png_s);

	వెడల్పు = png_get_image_వెడల్పు(png_s, info);
	ఎత్తు = png_get_image_ఎత్తు(png_s, info);
	interlace = png_get_interlace_type(png_s, info);

	png_c->పునాది.వెడల్పు = వెడల్పు;
	png_c->పునాది.ఎత్తు = ఎత్తు;
	png_c->పునాది.కొలత += వెడల్పు * ఎత్తు * 4;

	/* see if progressive-conversion should continue */
	ఒకవేళ (image_cache_speculate((నిర్మాణం content *)png_c) == false) {
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	/* Claim the required memory for the converted PNG */
	png_c->bitmap = guit->bitmap->create(వెడల్పు, ఎత్తు, BITMAP_NONE);
	ఒకవేళ (png_c->bitmap == ఖాళీ) {
		/* Failed to create bitmap skip pre-conversion */
		longjmp(png_jmpbuf(png_s), CBERR_NOPRE);
	}

	png_c->rowstride = guit->bitmap->get_rowstride(png_c->bitmap);
	png_c->bpp = పరిమాణం(uint32_t);

	nspng_అమరిక_transforms(png_s, info);

	png_c->rowbytes = png_get_rowbytes(png_s, info);
	png_c->interlace = (interlace == PNG_INTERLACE_ADAM7);

	NSLOG(netsurf, INFO, "size %li * %li, rowbytes %"PRIsizet,
	      (గుర్తులేని దీర్ఘ)వెడల్పు, (గుర్తులేని దీర్ఘ)ఎత్తు, png_c->rowbytes);
}

స్థిర శూన్య అడ్డువరుస_callback(png_structp png_s, png_bytep new_అడ్డువరుస,
			 png_uint_32 అడ్డువరుస_అంకె, పూర్ణాంకం pass)
{
	nspng_content *png_c = png_get_progressive_ptr(png_s);
	గుర్తులేని దీర్ఘ rowbytes = png_c->rowbytes;
	గుర్తులేని అక్షరం *బఫరు, *అడ్డువరుస;

	/* Give up if there's no bitmap */
	ఒకవేళ (png_c->bitmap == ఖాళీ)
		తిరిగి;

	/* Abort if we've not got any data */
	ఒకవేళ (new_అడ్డువరుస == ఖాళీ)
		తిరిగి;

	/* Get bitmap buffer */
	బఫరు = guit->bitmap->get_బఫరు(png_c->bitmap);
	ఒకవేళ (బఫరు == ఖాళీ) {
		/* No buffer, bail out */
		longjmp(png_jmpbuf(png_s), 1);
	}

	/* Calculate address of row start */
	అడ్డువరుస = బఫరు + (png_c->rowstride * అడ్డువరుస_అంకె);

	/* Handle interlaced sprites using the Adam7 algorithm */
	ఒకవేళ (png_c->interlace) {
		గుర్తులేని దీర్ఘ dst_off;
		గుర్తులేని దీర్ఘ src_off = 0;
		గుర్తులేని పూర్ణాంకం ప్రారంభం, అడుగు;

		ప్రారంభం = interlace_ప్రారంభం[pass];
		అడుగు = interlace_అడుగు[pass];
		అడ్డువరుస_అంకె = interlace_అడ్డువరుస_ప్రారంభం[pass] +
			interlace_అడ్డువరుస_అడుగు[pass] * అడ్డువరుస_అంకె;

		/* Copy the data to our current row taking interlacing
		 * into consideration */
		అడ్డువరుస = బఫరు + (png_c->rowstride * అడ్డువరుస_అంకె);

		కొరకు (dst_off = ప్రారంభం; dst_off < rowbytes; dst_off += అడుగు) {
			అడ్డువరుస[dst_off++] = new_అడ్డువరుస[src_off++];
			అడ్డువరుస[dst_off++] = new_అడ్డువరుస[src_off++];
			అడ్డువరుస[dst_off++] = new_అడ్డువరుస[src_off++];
			అడ్డువరుస[dst_off++] = new_అడ్డువరుస[src_off++];
		}
	} లేకపోతే {
		/* Do a fast memcpy of the row data */
		జ్ఞాపకం_నకలు(అడ్డువరుస, new_అడ్డువరుస, rowbytes);
	}
}


స్థిర శూన్య చివర_callback(png_structp png_s, png_infop info)
{
}

స్థిర nserror nspng_create_png_దత్తాంశం(nspng_content *png_c)
{
	png_c->bitmap = ఖాళీ;

	png_c->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	ఒకవేళ (png_c->png == ఖాళీ) {
		content_ప్రసారం_error(&png_c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}

	png_సమితి_error_fn(png_c->png, ఖాళీ, nspng_error, nspng_warning);

	png_c->info = png_create_info_struct(png_c->png);
	ఒకవేళ (png_c->info == ఖాళీ) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);

		content_ప్రసారం_error(&png_c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (setjmp(png_jmpbuf(png_c->png))) {
		png_destroy_read_struct(&png_c->png, &png_c->info, 0);
		NSLOG(netsurf, INFO, "Failed to set callbacks");
		png_c->png = ఖాళీ;
		png_c->info = ఖాళీ;

		content_ప్రసారం_error(&png_c->పునాది, NSERROR_PNG_ERROR, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}

	png_సమితి_progressive_read_fn(png_c->png, png_c,
			info_callback, అడ్డువరుస_callback, చివర_callback);

	తిరిగి NSERROR_OK;
}

స్థిర nserror
nspng_create(స్థిరరాశి content_handler *handler,
	     lwc_string *imime_type,
	     స్థిరరాశి నిర్మాణం http_parameter *params,
	     నిర్మాణం llcache_handle *llcache,
	     స్థిరరాశి అక్షరం *fallback_charset,
	     bool quirks,
	     నిర్మాణం content **c)
{
	nspng_content *png_c;
	nserror error;

	png_c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nspng_content));
	ఒకవేళ (png_c == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&png_c->పునాది, 
			      handler, 
			      imime_type, 
			      params,
			      llcache, 
			      fallback_charset, 
			      quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(png_c);
		తిరిగి error;
	}

	error = nspng_create_png_దత్తాంశం(png_c);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(png_c);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *)png_c;

	తిరిగి NSERROR_OK;
}


స్థిర bool nspng_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం,
			       గుర్తులేని పూర్ణాంకం కొలత)
{
	nspng_content *png_c = (nspng_content *)c;
	అస్థిర bool ret = true;

	ఒకవేళ (png_c->no_process_దత్తాంశం) {
		తిరిగి ret;
	}

	ఎంపిక (setjmp(png_jmpbuf(png_c->png))) {
	స్థితి CBERR_NONE: /* direct return */	
		png_process_దత్తాంశం(png_c->png, png_c->info, (uint8_t *)దత్తాంశం, కొలత);
		ఆపు;

	స్థితి CBERR_NOPRE: /* not going to progressive convert */
		png_c->no_process_దత్తాంశం = true;
		ఆపు;

	అప్రమేయ: /* fatal error from library processing png */
		ఒకవేళ (png_c->bitmap != ఖాళీ) {
			/* A bitmap managed to get created so
			 * operation is past header and possibly some
			 * conversion happened before faliure. 
			 *
			 * In this case keep the partial
			 * conversion. This is usually seen if a png
			 * has been truncated (often jsut lost its
			 * last byte and hence end of image marker)
			 */
			png_c->no_process_దత్తాంశం = true;
		} లేకపోతే {
			/* not managed to progress past header, clean
			 * up png conversion and signal the content
			 * error 
			 */
			NSLOG(netsurf, INFO,
			      "Fatal PNG error during header, error content");

			png_destroy_read_struct(&png_c->png, &png_c->info, 0);
			png_c->png = ఖాళీ;
			png_c->info = ఖాళీ;

			content_ప్రసారం_error(c, NSERROR_PNG_ERROR, ఖాళీ);

			ret = false;

		}
		ఆపు;
	}

	తిరిగి ret;
}

నిర్మాణం png_cache_read_దత్తాంశం_s {
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
};

/** PNG library read fucntion to read data from a memory array 
 */
స్థిర శూన్య 
png_cache_read_fn(png_structp png_ptr, png_bytep దత్తాంశం, png_కొలత_t నిడివి)
{
	నిర్మాణం png_cache_read_దత్తాంశం_s *png_cache_read_దత్తాంశం;
	png_cache_read_దత్తాంశం = png_get_io_ptr(png_ptr);

	ఒకవేళ (నిడివి > png_cache_read_దత్తాంశం->కొలత) {
		నిడివి = png_cache_read_దత్తాంశం->కొలత;
	}

	ఒకవేళ (నిడివి == 0) {
		png_error(png_ptr, "Read Error");
	}

	జ్ఞాపకం_నకలు(దత్తాంశం, png_cache_read_దత్తాంశం->దత్తాంశం, నిడివి);

	png_cache_read_దత్తాంశం->దత్తాంశం += నిడివి;
	png_cache_read_దత్తాంశం->కొలత -= నిడివి;
}

/** calculate an array of row pointers into a bitmap data area
 */
స్థిర png_bytep *calc_అడ్డువరుస_సూచికలు(నిర్మాణం bitmap *bitmap)
{
	పూర్ణాంకం ఎత్తు = guit->bitmap->get_ఎత్తు(bitmap);
	గుర్తులేని అక్షరం *బఫరు= guit->bitmap->get_బఫరు(bitmap);
	size_t rowstride = guit->bitmap->get_rowstride(bitmap);
	png_bytep *అడ్డువరుస_ptrs;
	పూర్ణాంకం hloop;

	/* The buffer allocation may occour when the buffer is aquired
	 * and therefore may fail.
	 */
	ఒకవేళ (బఫరు == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	అడ్డువరుస_ptrs = జ్ఞాపకం_ఇవ్వు(పరిమాణం(png_bytep) * ఎత్తు);

	ఒకవేళ (అడ్డువరుస_ptrs != ఖాళీ) {
		కొరకు (hloop = 0; hloop < ఎత్తు; hloop++) {
			అడ్డువరుస_ptrs[hloop] = బఫరు + (rowstride * hloop);
		}
	}

	తిరిగి అడ్డువరుస_ptrs;
}

/** PNG content to bitmap conversion.
 *
 * This routine generates a bitmap object from a PNG image content
 */
స్థిర నిర్మాణం bitmap *
png_cache_convert(నిర్మాణం content *c)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop చివర_info_ptr;
	అస్థిర నిర్మాణం bitmap * అస్థిర bitmap = ఖాళీ;
	నిర్మాణం png_cache_read_దత్తాంశం_s png_cache_read_దత్తాంశం;
	png_uint_32 వెడల్పు, ఎత్తు;
	అస్థిర png_bytep * అస్థిర అడ్డువరుస_సూచికలు = ఖాళీ;

	png_cache_read_దత్తాంశం.దత్తాంశం = 
		content_get_ఆధారం_దత్తాంశం_2(c, &png_cache_read_దత్తాంశం.కొలత);

	ఒకవేళ ((png_cache_read_దత్తాంశం.దత్తాంశం == ఖాళీ) || 
	    (png_cache_read_దత్తాంశం.కొలత <= 8)) {
		తిరిగి ఖాళీ;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, ఖాళీ,
			nspng_error, nspng_warning);
	ఒకవేళ (png_ptr == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	info_ptr = png_create_info_struct(png_ptr);
	ఒకవేళ (info_ptr == ఖాళీ) {
		png_destroy_read_struct(&png_ptr, ఖాళీ, ఖాళీ);
		తిరిగి ఖాళీ;
	}

	చివర_info_ptr = png_create_info_struct(png_ptr);
	ఒకవేళ (చివర_info_ptr == ఖాళీ) {
		png_destroy_read_struct(&png_ptr, &info_ptr, ఖాళీ);
		తిరిగి ఖాళీ;
	}

	/* setup error exit path */
	ఒకవేళ (setjmp(png_jmpbuf(png_ptr))) {
		/* cleanup and bail */
		వెళ్ళు png_cache_convert_error;
	}

	/* read from a buffer instead of stdio */
	png_సమితి_read_fn(png_ptr, &png_cache_read_దత్తాంశం, png_cache_read_fn);

	/* ensure the png info structure is populated */
	png_read_info(png_ptr, info_ptr);

	/* setup output transforms */
	nspng_అమరిక_transforms(png_ptr, info_ptr);

	వెడల్పు = png_get_image_వెడల్పు(png_ptr, info_ptr);
	ఎత్తు = png_get_image_ఎత్తు(png_ptr, info_ptr);

	/* Claim the required memory for the converted PNG */
	bitmap = guit->bitmap->create(వెడల్పు, ఎత్తు, BITMAP_NONE);
	ఒకవేళ (bitmap == ఖాళీ) {
		/* cleanup and bail */
		వెళ్ళు png_cache_convert_error;
	}

	అడ్డువరుస_సూచికలు = calc_అడ్డువరుస_సూచికలు((నిర్మాణం bitmap *) bitmap);

	ఒకవేళ (అడ్డువరుస_సూచికలు != ఖాళీ) {
		png_read_image(png_ptr, (png_bytep *) అడ్డువరుస_సూచికలు);
	} లేకపోతే {
		guit->bitmap->destroy((నిర్మాణం bitmap *)bitmap);
		bitmap = ఖాళీ;
	}

png_cache_convert_error:

	/* cleanup png read */
	png_destroy_read_struct(&png_ptr, &info_ptr, &చివర_info_ptr);

	ఒకవేళ (అడ్డువరుస_సూచికలు != ఖాళీ) {
		విడుదల((png_bytep *) అడ్డువరుస_సూచికలు);
	}

	ఒకవేళ (bitmap != ఖాళీ) {
		bool opaque = bitmap_test_opaque((శూన్య *)bitmap);
		guit->bitmap->సమితి_opaque((శూన్య *)bitmap, opaque);
		bitmap_format_గమ్యం_client((శూన్య *)bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified((శూన్య *)bitmap);
	}

	తిరిగి (నిర్మాణం bitmap *)bitmap;
}

స్థిర bool nspng_convert(నిర్మాణం content *c)
{
	nspng_content *png_c = (nspng_content *) c;
	అక్షరం *title;

	ప్రతిజ్ఞా(png_c->png != ఖాళీ);
	ప్రతిజ్ఞా(png_c->info != ఖాళీ);

	/* clean up png structures */
	png_destroy_read_struct(&png_c->png, &png_c->info, 0);

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	ఒకవేళ (png_c->bitmap != ఖాళీ) {
		bool opaque = bitmap_test_opaque(png_c->bitmap);
		guit->bitmap->సమితి_opaque(png_c->bitmap, opaque);
		bitmap_format_గమ్యం_client(png_c->bitmap, &(bitmap_fmt_t) {
			.layout = bitmap_fmt.layout,
			.pma = opaque ? bitmap_fmt.pma : false,
		});
		guit->bitmap->modified(png_c->bitmap);
	}

	image_cache_కూడు(c, png_c->bitmap, png_cache_convert);

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
	content_సమితి_స్థితి(c, "");

	తిరిగి true;
}


స్థిర nserror nspng_clone(స్థిరరాశి నిర్మాణం content *old_c, నిర్మాణం content **new_c)
{
	nspng_content *clone_png_c;
	nserror error;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;

	clone_png_c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nspng_content));
	ఒకవేళ (clone_png_c == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old_c, &clone_png_c->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->పునాది);
		తిరిగి error;
	}

	/* Simply replay create/process/convert */
	error = nspng_create_png_దత్తాంశం(clone_png_c);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&clone_png_c->పునాది);
		తిరిగి error;
	}

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(&clone_png_c->పునాది, &కొలత);
	ఒకవేళ (కొలత > 0) {
		ఒకవేళ (nspng_process_దత్తాంశం(&clone_png_c->పునాది, (స్థిరరాశి అక్షరం *)దత్తాంశం, కొలత) == false) {
			content_destroy(&clone_png_c->పునాది);
			తిరిగి NSERROR_NOMEM;
		}
	}

	ఒకవేళ ((old_c->స్థితి_2 == CONTENT_స్థితి_READY) ||
	    (old_c->స్థితి_2 == CONTENT_స్థితి_పూర్తి)) {
		ఒకవేళ (nspng_convert(&clone_png_c->పునాది) == false) {
			content_destroy(&clone_png_c->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*new_c = (నిర్మాణం content *)clone_png_c;

	తిరిగి NSERROR_OK;
}

స్థిర స్థిరరాశి content_handler nspng_content_handler = {
	.create = nspng_create,
	.process_దత్తాంశం = nspng_process_దత్తాంశం,
	.దత్తాంశం_complete = nspng_convert,
	.clone = nspng_clone,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.get_అంతర్గత = image_cache_get_అంతర్గత,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *nspng_types[] = {
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nspng, nspng_types, nspng_content_handler);
