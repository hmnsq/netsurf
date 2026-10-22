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

#define JPEG_INTERNAL_OPTIONS
#ضمن "jpeglib.h"
#ضمن "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

ساكن حرف nsjpeg_error_buffer[JMSG_LENGTH_MAX];

ساكن unsigned حرف nsjpeg_eoi[] = { 0xff, JPEG_EOI };

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

	jpeg = calloc(1, حجم(بنية content));
	אם (jpeg == عدم)
		החזר NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	אם (error != NSERROR_OK) {
		حرر(jpeg);
		החזר error;
	}

	*c = jpeg;

	החזר NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
ساكن فراغ nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
ساكن boolean nsjpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_input_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	החזר TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

ساكن فراغ nsjpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	אם ((long) cinfo->src->bytes_in_buffer < num_bytes) {
		cinfo->src->next_input_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} אחרת {
		cinfo->src->next_input_byte += num_bytes;
		cinfo->src->bytes_in_buffer -= num_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
ساكن فراغ nsjpeg_term_source(j_decompress_ptr cinfo)
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
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
ساكن فراغ nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	longjmp(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_cmyk(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
	صحيح width = cinfo->output_width * 4;

	עשה {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		עבור (صحيح i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			ثابت صحيح c = scanlines[0][i + 0];
			ثابت صحيح m = scanlines[0][i + 1];
			ثابت صحيح y = scanlines[0][i + 2];
			ثابت صحيح k = scanlines[0][i + 3];

			ثابت صحيح ck = c * k;
			ثابت صحيح mk = m * k;
			ثابت صحيح yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} כלעוד (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_rgb(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
#אם RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	صحيح width = cinfo->output_width;
#endif

	עשה {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#אם RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		עבור (صحيح i = width - 1; 0 <= i; i--) {
			صحيح r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			صحيح g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			صحيح b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} כלעוד (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ساكن مضمن فراغ nsjpeg__decode_client_fmt(
		بنية jpeg_decompress_struct *cinfo,
		uint8_t * متطاير pixels,
		size_t rowstride)
{
	עשה {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} כלעוד (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
ساكن بنية bitmap *
jpeg_cache_convert(بنية content *c)
{
	ثابت uint8_t *source_data; /* Jpeg source data */
	size_t source_size; /* length of Jpeg source data */
	بنية jpeg_decompress_struct cinfo;
	بنية jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	بنية bitmap * متطاير bitmap = عدم;
	uint8_t * متطاير pixels = عدم;
	size_t rowstride;
	بنية jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_input_buffer,
		nsjpeg_skip_input_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	אם ((source_data == عدم) ||
	    (source_size < MIN_JPEG_SIZE)) {
		החזר عدم;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	אם (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		החזר bitmap;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.next_input_byte = source_data;
	source_mgr.bytes_in_buffer = source_size;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	אם (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} אחרת {
#ifdef JCS_ALPHA_EXTENSIONS
		بدل (bitmap_fmt.layout) {
		حالة BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			اكسر;
		حالة BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			اكسر;
		حالة BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			اكسر;
		حالة BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			اكسر;
		افتراضي:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			החזר عدم;
		}
#אחרת
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
	אם (bitmap == عدم) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		החזר عدم;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	אם (pixels == عدم) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		החזר عدم;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	بدل (cinfo.out_color_space) {
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

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	החזר bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
ساكن bool nsjpeg_convert(بنية content *c)
{
	بنية jpeg_decompress_struct cinfo;
	بنية jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	بنية jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_input_buffer,
		nsjpeg_skip_input_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	اتحاد content_msg_data msg_data;
	ثابت uint8_t *data;
	size_t size;
	حرف *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	אם (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		msg_data.errordata.errorcode = NSERROR_UNKNOWN;
		msg_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
		החזר false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_input_byte = (unsigned حرف *) data;
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

	image_cache_add(c, عدم, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	אם (title != عدم) {
		content__set_title(c, title);
		حرر(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	החזר true;
}



/**
 * Clone content.
 */
ساكن nserror nsjpeg_clone(ثابت بنية content *old, بنية content **newc)
{
	بنية content *jpeg_c;
	nserror error;

	jpeg_c = calloc(1, حجم(بنية content));
	אם (jpeg_c == عدم)
		החזר NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	אם (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		החזר error;
	}

	/* re-convert if the content is ready */
	אם ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		אם (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			החזר NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	החזר NSERROR_OK;
}

ساكن ثابت content_handler nsjpeg_content_handler = {
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

ساكن ثابت حرف *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
