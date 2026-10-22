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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <setjmp.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/bitmap.h"
#شامل "content/llcache.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image_cache.h"

#define JPEG_INTERNAL_OPTIONS
#شامل "jpeglib.h"
#شامل "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

ایستا نویسه nsjpeg_error_buffer[JMSG_LENGTH_MAX];

ایستا unsigned نویسه nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
ایستا nserror nsjpeg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	ساختار content *jpeg;
	nserror error;

	jpeg = calloc(1, اندازه(ساختار content));
	اگر (jpeg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(jpeg);
		بازگشت error;
	}

	*c = jpeg;

	بازگشت NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
ایستا تهی nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
ایستا boolean nsjpeg_fill_درون_داد_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_درون_داد_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	بازگشت TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

ایستا تهی nsjpeg_skip_درون_داد_data(j_decompress_ptr cinfo, long رقم_bytes)
{
	اگر ((long) cinfo->src->bytes_in_buffer < رقم_bytes) {
		cinfo->src->next_درون_داد_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} وگرنه {
		cinfo->src->next_درون_داد_byte += رقم_bytes;
		cinfo->src->bytes_in_buffer -= رقم_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
ایستا تهی nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
ایستا تهی nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
ایستا تهی nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	longjmp(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ایستا درونخطی تهی nsjpeg__decode_cmyk(
		ساختار jpeg_decompress_struct *cinfo,
		uint8_t * فرار pixels,
		size_t rowstride)
{
	صحیح width = cinfo->output_width * 4;

	انجامبده {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		برای (صحیح i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			ثابت صحیح c = scanlines[0][i + 0];
			ثابت صحیح m = scanlines[0][i + 1];
			ثابت صحیح y = scanlines[0][i + 2];
			ثابت صحیح k = scanlines[0][i + 3];

			ثابت صحیح ck = c * k;
			ثابت صحیح mk = m * k;
			ثابت صحیح yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} وقتی (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ایستا درونخطی تهی nsjpeg__decode_rgb(
		ساختار jpeg_decompress_struct *cinfo,
		uint8_t * فرار pixels,
		size_t rowstride)
{
#اگر RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	صحیح width = cinfo->output_width;
#endif

	انجامبده {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#اگر RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		برای (صحیح i = width - 1; 0 <= i; i--) {
			صحیح r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			صحیح g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			صحیح b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} وقتی (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ایستا درونخطی تهی nsjpeg__decode_client_fmt(
		ساختار jpeg_decompress_struct *cinfo,
		uint8_t * فرار pixels,
		size_t rowstride)
{
	انجامبده {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} وقتی (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
ایستا ساختار bitmap *
jpeg_cache_convert(ساختار content *c)
{
	ثابت uint8_t *source_data; /* Jpeg source data */
	size_t source_size; /* length of Jpeg source data */
	ساختار jpeg_decompress_struct cinfo;
	ساختار jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	ساختار bitmap * فرار bitmap = پوچ;
	uint8_t * فرار pixels = پوچ;
	size_t rowstride;
	ساختار jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_درون_داد_buffer,
		nsjpeg_skip_درون_داد_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	اگر ((source_data == پوچ) ||
	    (source_size < MIN_JPEG_SIZE)) {
		بازگشت پوچ;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	اگر (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		بازگشت bitmap;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.next_درون_داد_byte = source_data;
	source_mgr.bytes_in_buffer = source_size;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	اگر (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} وگرنه {
#ifdef JCS_ALPHA_EXTENSIONS
		انتخاب (bitmap_fmt.layout) {
		حالت BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			بشکن;
		حالت BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			بشکن;
		حالت BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			بشکن;
		حالت BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			بشکن;
		پیشفرض:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			بازگشت پوچ;
		}
#وگرنه
		cinfo.out_color_space = JCS_RGB;
#endif
	}
	cinfo.dct_method = JDCT_ISLOW;

	/* commence the decompression, output parameters now valid */
	jpeg_start_decompress(&cinfo);

	/* create opaque bitmap (jpegs cannot be transparent) */
	bitmap = guit->bitmap->create(
			cinfo.output_width,
			cinfo.output_height, BITMAP_OPAQUE);
	اگر (bitmap == پوچ) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		بازگشت پوچ;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	اگر (pixels == پوچ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		بازگشت پوچ;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	انتخاب (cinfo.out_color_space) {
	حالت JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		بشکن;

	حالت JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		بشکن;

	پیشفرض:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		بشکن;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	بازگشت bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
ایستا bool nsjpeg_convert(ساختار content *c)
{
	ساختار jpeg_decompress_struct cinfo;
	ساختار jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	ساختار jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_درون_داد_buffer,
		nsjpeg_skip_درون_داد_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	اجتماع content_پیام_data پیام_data;
	ثابت uint8_t *data;
	size_t size;
	نویسه *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	اگر (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		پیام_data.errordata.errorcode = NSERROR_UNKNOWN;
		پیام_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_پیام_ERROR, &پیام_data);
		بازگشت false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_درون_داد_byte = (unsigned نویسه *) data;
	source_mgr.bytes_in_buffer = size;
	cinfo.src = &source_mgr;
	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	cinfo.dct_method = JDCT_ISLOW;

	jpeg_calc_output_dimensions(&cinfo);

	c->width = cinfo.output_width;
	c->height = cinfo.output_height;
	c->size = c->width * c->height * 4;

	jpeg_destroy_decompress(&cinfo);

	image_cache_add(c, پوچ, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	اگر (title != پوچ) {
		content__set_title(c, title);
		آزاد(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	بازگشت true;
}



/**
 * Clone content.
 */
ایستا nserror nsjpeg_clone(ثابت ساختار content *old, ساختار content **newc)
{
	ساختار content *jpeg_c;
	nserror error;

	jpeg_c = calloc(1, اندازه(ساختار content));
	اگر (jpeg_c == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	اگر (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		بازگشت error;
	}

	/* re-convert if the content is ready */
	اگر ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		اگر (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	بازگشت NSERROR_OK;
}

ایستا ثابت content_handler nsjpeg_content_handler = {
	.create = nsjpeg_create,
	.data_complete = nsjpeg_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpeg_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ایستا ثابت نویسه *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
