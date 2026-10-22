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

#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/bitmap.h"
#include "content/llcache.h"
#include "content/content.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/image_cache.h"

#define JPEG_INTERNAL_OPTIONS
#include "jpeglib.h"
#include "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

static var_na nsjpeg_error_buffer[JMSG_LENGTH_MAX];

static unsigned var_na nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
static nserror nsjpeg_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	sa_nracanaa content *jpeg;
	nserror error;

	jpeg = calloc(1, parimaa_na(sa_nracanaa content));
	ᤀᤡ᤺ (jpeg == NULL)
		ᤀᤡᤴ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ᤀᤡ᤺ (error != NSERROR_OK) {
		free(jpeg);
		ᤀᤡᤴ error;
	}

	*c = jpeg;

	ᤀᤡᤴ NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
static void nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
static boolean nsjpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_input_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	ᤀᤡᤴ TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

static void nsjpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	ᤀᤡ᤺ ((long) cinfo->src->bytes_in_buffer < num_bytes) {
		cinfo->src->next_input_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} ᤀᤡ᤺ᤆᤡ {
		cinfo->src->next_input_byte += num_bytes;
		cinfo->src->bytes_in_buffer -= num_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
static void nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
static void nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
static void nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	longjmp(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
static anta_hstha void nsjpeg__decode_cmyk(
		sa_nracanaa jpeg_decompress_struct *cinfo,
		uint8_t * ca_ncala pixels,
		size_t rowstride)
{
	puur_naa_nka width = cinfo->output_width * 4;

	do {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		for (puur_naa_nka i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			const puur_naa_nka c = scanlines[0][i + 0];
			const puur_naa_nka m = scanlines[0][i + 1];
			const puur_naa_nka y = scanlines[0][i + 2];
			const puur_naa_nka k = scanlines[0][i + 3];

			const puur_naa_nka ck = c * k;
			const puur_naa_nka mk = m * k;
			const puur_naa_nka yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} ᤀᤠ᤺ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
static anta_hstha void nsjpeg__decode_rgb(
		sa_nracanaa jpeg_decompress_struct *cinfo,
		uint8_t * ca_ncala pixels,
		size_t rowstride)
{
#ᤀᤡ᤺ RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	puur_naa_nka width = cinfo->output_width;
#endif

	do {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#ᤀᤡ᤺ RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		for (puur_naa_nka i = width - 1; 0 <= i; i--) {
			puur_naa_nka r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			puur_naa_nka g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			puur_naa_nka b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} ᤀᤠ᤺ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
static anta_hstha void nsjpeg__decode_client_fmt(
		sa_nracanaa jpeg_decompress_struct *cinfo,
		uint8_t * ca_ncala pixels,
		size_t rowstride)
{
	do {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} ᤀᤠ᤺ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
static sa_nracanaa bitmap *
jpeg_cache_convert(sa_nracanaa content *c)
{
	const uint8_t *source_data; /* Jpeg source data */
	size_t source_size; /* length of Jpeg source data */
	sa_nracanaa jpeg_decompress_struct cinfo;
	sa_nracanaa jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	sa_nracanaa bitmap * ca_ncala bitmap = NULL;
	uint8_t * ca_ncala pixels = NULL;
	size_t rowstride;
	sa_nracanaa jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_input_buffer,
		nsjpeg_skip_input_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	ᤀᤡ᤺ ((source_data == NULL) ||
	    (source_size < MIN_JPEG_SIZE)) {
		ᤀᤡᤴ NULL;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	ᤀᤡ᤺ (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		ᤀᤡᤴ bitmap;
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
	ᤀᤡ᤺ (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} ᤀᤡ᤺ᤆᤡ {
#ifdef JCS_ALPHA_EXTENSIONS
		switch (bitmap_fmt.layout) {
		pak_sa BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			bha_nga;
		pak_sa BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			bha_nga;
		pak_sa BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			bha_nga;
		pak_sa BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			bha_nga;
		default:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			ᤀᤡᤴ NULL;
		}
#ᤀᤡ᤺ᤆᤡ
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
	ᤀᤡ᤺ (bitmap == NULL) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		ᤀᤡᤴ NULL;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	ᤀᤡ᤺ (pixels == NULL) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		ᤀᤡᤴ NULL;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	switch (cinfo.out_color_space) {
	pak_sa JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		bha_nga;

	pak_sa JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		bha_nga;

	default:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		bha_nga;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	ᤀᤡᤴ bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
static bool nsjpeg_convert(sa_nracanaa content *c)
{
	sa_nracanaa jpeg_decompress_struct cinfo;
	sa_nracanaa jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	sa_nracanaa jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_input_buffer,
		nsjpeg_skip_input_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	sa_ngha content_msg_data msg_data;
	const uint8_t *data;
	size_t size;
	var_na *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	ᤀᤡ᤺ (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		msg_data.errordata.errorcode = NSERROR_UNKNOWN;
		msg_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
		ᤀᤡᤴ false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_input_byte = (unsigned var_na *) data;
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

	image_cache_add(c, NULL, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ᤀᤡ᤺ (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	ᤀᤡᤴ true;
}



/**
 * Clone content.
 */
static nserror nsjpeg_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	sa_nracanaa content *jpeg_c;
	nserror error;

	jpeg_c = calloc(1, parimaa_na(sa_nracanaa content));
	ᤀᤡ᤺ (jpeg_c == NULL)
		ᤀᤡᤴ NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	ᤀᤡ᤺ (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		ᤀᤡᤴ error;
	}

	/* re-convert if the content is ready */
	ᤀᤡ᤺ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ᤀᤡ᤺ (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			ᤀᤡᤴ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	ᤀᤡᤴ NSERROR_OK;
}

static const content_handler nsjpeg_content_handler = {
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

static const var_na *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
