/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
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

/**
 * \file
 * implementation of content handling for image/jpeg
 *
 * This implementation uses the IJG JPEG library.
 */

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <setjmp.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "content/llcache.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image_cache.h"

#تعریف_کن JPEG_داخلی_OPTIONS
#ضمن "jpeglib.h"
#ضمن "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#تعریف_کن MIN_JPEG_اندازه 20

#اگر_تعریف_نشده LIBJPEG_TURBO_نسخه
#warning Using libjpeg (libjpeg-turbo is recommended)
#پایان_اگر

ساكن حرف nsjpeg_error_میانگیر[JMSG_درازی_MAX];

ساكن بیعلامت حرف nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
ساكن nserror nsjpeg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	بنية content *jpeg;
	nserror error;

	jpeg = حافظه_صفر_بده(1, حجم(بنية content));
	إذا (jpeg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(jpeg);
		أرجع error;
	}

	*c = jpeg;

	أرجع NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
ساكن فراغ nsjpeg_init_منبع(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
ساكن boolean nsjpeg_fill_درون_داد_میانگیر(j_decompress_ptr cinfo)
{
	cinfo->src->بعدی_درون_داد_byte = nsjpeg_eoi;
	cinfo->src->بایتها_درون_میانگیر = 2;
 	أرجع TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

ساكن فراغ nsjpeg_skip_درون_داد_داده(j_decompress_ptr cinfo, دراز رقم_بایتها)
{
	إذا ((دراز) cinfo->src->بایتها_درون_میانگیر < رقم_بایتها) {
		cinfo->src->بعدی_درون_داد_byte = 0;
		cinfo->src->بایتها_درون_میانگیر = 0;
	} وإلا {
		cinfo->src->بعدی_درون_داد_byte += رقم_بایتها;
		cinfo->src->بایتها_درون_میانگیر -= رقم_بایتها;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
ساكن فراغ nsjpeg_جمله_منبع(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
ساكن فراغ nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_پیام(cinfo, nsjpeg_error_میانگیر);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_میانگیر);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
ساكن فراغ nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_میانگیر = (jmp_buf *) cinfo->client_داده;

	cinfo->err->format_پیام(cinfo, nsjpeg_error_میانگیر);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_میانگیر);

	longjmp(*setjmp_میانگیر, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_cmyk(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
	صحيح عرض = cinfo->output_عرض * 4;

	افعل {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		لكل (صحيح i = عرض - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			ثابت صحيح c = scanlines[0][i + 0];
			ثابت صحيح m = scanlines[0][i + 1];
			ثابت صحيح y = scanlines[0][i + 2];
			ثابت صحيح k = scanlines[0][i + 3];

			ثابت صحيح ck = c * k;
			ثابت صحيح mk = m * k;
			ثابت صحيح yk = y * k;

#تعریف_کن DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#لغو_تعریف DIV255
		}
	} طالما (cinfo->output_scanline != cinfo->output_ارتفاع);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_rgb(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
#إذا RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	صحيح عرض = cinfo->output_عرض;
#پایان_اگر

	افعل {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#إذا RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		لكل (صحيح i = عرض - 1; 0 <= i; i--) {
			صحيح r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			صحيح g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			صحيح b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#پایان_اگر
	} طالما (cinfo->output_scanline != cinfo->output_ارتفاع);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_client_fmt(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
	افعل {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} طالما (cinfo->output_scanline != cinfo->output_ارتفاع);
}

/**
 * create a bitmap from jpeg content.
 */
ساكن بنية bitmap *
jpeg_cache_convert(بنية content *c)
{
	ثابت uint8_t *منبع_داده; /* Jpeg source data */
	size_t منبع_اندازه; /* length of Jpeg source data */
	بنية jpeg_decompress_struct cinfo;
	بنية jpeg_error_mgr jerr;
	jmp_buf setjmp_میانگیر;
	بنية bitmap * متطاير bitmap = عدم;
	uint8_t * متطاير pixels = عدم;
	size_t rowstride;
	بنية jpeg_منبع_mgr منبع_mgr = {
		0,
		0,
		nsjpeg_init_منبع,
		nsjpeg_fill_درون_داد_میانگیر,
		nsjpeg_skip_درون_داد_داده,
		jpeg_resync_مقصد_restart,
		nsjpeg_جمله_منبع };

	/* obtain jpeg source data and perfom minimal sanity checks */
	منبع_داده = content_get_منبع_داده_2(c, &منبع_اندازه);

	إذا ((منبع_داده == عدم) ||
	    (منبع_اندازه < MIN_JPEG_اندازه)) {
		أرجع عدم;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_پیام = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	إذا (setjmp(setjmp_میانگیر)) {
		jpeg_destroy_decompress(&cinfo);
		أرجع bitmap;
	}

	cinfo.client_داده = &setjmp_میانگیر;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	منبع_mgr.بعدی_درون_داد_byte = منبع_داده;
	منبع_mgr.بایتها_درون_میانگیر = منبع_اندازه;
	cinfo.src = &منبع_mgr;

	/* read JPEG header information */
	jpeg_read_سرآیند(&cinfo, TRUE);

	/* set output processing parameters */
	إذا (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.بیرون_color_space = JCS_CMYK;
	} وإلا {
#اگر_تعریف_شده JCS_ALPHA_EXTENSIONS
		بدل (bitmap_fmt.layout) {
		حالة BITMAP_LAYOUT_R8G8B8A8:
			cinfo.بیرون_color_space = JCS_EXT_RGBA;
			اكسر;
		حالة BITMAP_LAYOUT_B8G8R8A8:
			cinfo.بیرون_color_space = JCS_EXT_BGRA;
			اكسر;
		حالة BITMAP_LAYOUT_A8R8G8B8:
			cinfo.بیرون_color_space = JCS_EXT_ARGB;
			اكسر;
		حالة BITMAP_LAYOUT_A8B8G8R8:
			cinfo.بیرون_color_space = JCS_EXT_ABGR;
			اكسر;
		افتراضي:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			أرجع عدم;
		}
#وإلا
		cinfo.بیرون_color_space = JCS_RGB;
#پایان_اگر
	}
	cinfo.dct_روش = JDCT_ISLOW;

	/* commence the decompression, output parameters now valid */
	jpeg_شروع_decompress(&cinfo);

	/* create opaque bitmap (jpegs cannot be transparent) */
	bitmap = guit->bitmap->create(
			cinfo.output_عرض,
			cinfo.output_ارتفاع, BITMAP_OPAQUE);
	إذا (bitmap == عدم) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		أرجع عدم;
	}

	pixels = guit->bitmap->get_میانگیر(bitmap);
	إذا (pixels == عدم) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		أرجع عدم;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	بدل (cinfo.بیرون_color_space) {
	حالة JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		اكسر;

	حالة JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		اكسر;

	افتراضي:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		اكسر;
	}

	guit->bitmap->modified(bitmap);

	jpeg_پایان_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	أرجع bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
ساكن bool nsjpeg_convert(بنية content *c)
{
	بنية jpeg_decompress_struct cinfo;
	بنية jpeg_error_mgr jerr;
	jmp_buf setjmp_میانگیر;
	بنية jpeg_منبع_mgr منبع_mgr = { 0, 0,
		nsjpeg_init_منبع, nsjpeg_fill_درون_داد_میانگیر,
		nsjpeg_skip_درون_داد_داده, jpeg_resync_مقصد_restart,
		nsjpeg_جمله_منبع };
	اتحاد content_پیام_داده پیام_داده;
	ثابت uint8_t *داده;
	size_t اندازه;
	حرف *title;

	/* check image header is valid and get width/height */
	داده = content_get_منبع_داده_2(c, &اندازه);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_پیام = nsjpeg_error_log;

	إذا (setjmp(setjmp_میانگیر)) {
		jpeg_destroy_decompress(&cinfo);

		پیام_داده.errordata.errorcode = NSERROR_UNKNOWN;
		پیام_داده.errordata.errormsg = nsjpeg_error_میانگیر;
		content_پخش(c, CONTENT_پیام_ERROR, &پیام_داده);
		أرجع false;
	}

	cinfo.client_داده = &setjmp_میانگیر;
	jpeg_create_decompress(&cinfo);
	منبع_mgr.بعدی_درون_داد_byte = (بیعلامت حرف *) داده;
	منبع_mgr.بایتها_درون_میانگیر = اندازه;
	cinfo.src = &منبع_mgr;
	jpeg_read_سرآیند(&cinfo, TRUE);
	cinfo.بیرون_color_space = JCS_RGB;
	cinfo.dct_روش = JDCT_ISLOW;

	jpeg_calc_output_dimensions(&cinfo);

	c->عرض = cinfo.output_عرض;
	c->ارتفاع = cinfo.output_ارتفاع;
	c->اندازه = c->عرض * c->ارتفاع * 4;

	jpeg_destroy_decompress(&cinfo);

	image_cache_جمع_کن(c, عدم, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->عرض, c->ارتفاع);
	إذا (title != عدم) {
		content_مجموعه_title(c, title);
		حرر(title);
	}

	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);	
	content_مجموعه_وضعیت(c, ""); /* Done: update status bar */

	أرجع true;
}



/**
 * Clone content.
 */
ساكن nserror nsjpeg_clone(ثابت بنية content *old, بنية content **newc)
{
	بنية content *jpeg_c;
	nserror error;

	jpeg_c = حافظه_صفر_بده(1, حجم(بنية content));
	إذا (jpeg_c == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	إذا (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		أرجع error;
	}

	/* re-convert if the content is ready */
	إذا ((old->وضعیت == CONTENT_وضعیت_READY) ||
	    (old->وضعیت == CONTENT_وضعیت_انجامشده)) {
		إذا (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	أرجع NSERROR_OK;
}

ساكن ثابت content_handler nsjpeg_content_handler = {
	.create = nsjpeg_create,
	.داده_complete = nsjpeg_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpeg_clone,
	.get_داخلی = image_cache_get_داخلی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_ثبات_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
