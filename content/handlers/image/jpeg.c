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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <setjmp.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/image_cache.h"

#define JPEG_INTERNAL_OPTIONS
#અન્તર્ભાવ "jpeglib.h"
#અન્તર્ભાવ "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

સ્થિર અક્ષર nsjpeg_error_buffer[JMSG_LENGTH_MAX];

સ્થિર unsigned અક્ષર nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
સ્થિર nserror nsjpeg_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	સંરચના content *jpeg;
	nserror error;

	jpeg = calloc(1, કદ(સંરચના content));
	જો (jpeg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(jpeg);
		પાછા error;
	}

	*c = jpeg;

	પાછા NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
સ્થિર શૂન્ય nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
સ્થિર boolean nsjpeg_fill_આગત_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_આગત_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	પાછા TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

સ્થિર શૂન્ય nsjpeg_skip_આગત_data(j_decompress_ptr cinfo, long અંક_bytes)
{
	જો ((long) cinfo->src->bytes_in_buffer < અંક_bytes) {
		cinfo->src->next_આગત_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} નહીંતર {
		cinfo->src->next_આગત_byte += અંક_bytes;
		cinfo->src->bytes_in_buffer -= અંક_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
સ્થિર શૂન્ય nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
સ્થિર શૂન્ય nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
સ્થિર શૂન્ય nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	longjmp(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
સ્થિર અંતઃરેખ શૂન્ય nsjpeg__decode_cmyk(
		સંરચના jpeg_decompress_struct *cinfo,
		uint8_t * અસ્થિર pixels,
		size_t rowstride)
{
	પૂર્ણાંક width = cinfo->output_width * 4;

	કરો {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		માટે (પૂર્ણાંક i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			અચલ પૂર્ણાંક c = scanlines[0][i + 0];
			અચલ પૂર્ણાંક m = scanlines[0][i + 1];
			અચલ પૂર્ણાંક y = scanlines[0][i + 2];
			અચલ પૂર્ણાંક k = scanlines[0][i + 3];

			અચલ પૂર્ણાંક ck = c * k;
			અચલ પૂર્ણાંક mk = m * k;
			અચલ પૂર્ણાંક yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} જ્યારે (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
સ્થિર અંતઃરેખ શૂન્ય nsjpeg__decode_rgb(
		સંરચના jpeg_decompress_struct *cinfo,
		uint8_t * અસ્થિર pixels,
		size_t rowstride)
{
#જો RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	પૂર્ણાંક width = cinfo->output_width;
#endif

	કરો {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#જો RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		માટે (પૂર્ણાંક i = width - 1; 0 <= i; i--) {
			પૂર્ણાંક r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			પૂર્ણાંક g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			પૂર્ણાંક b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} જ્યારે (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
સ્થિર અંતઃરેખ શૂન્ય nsjpeg__decode_client_fmt(
		સંરચના jpeg_decompress_struct *cinfo,
		uint8_t * અસ્થિર pixels,
		size_t rowstride)
{
	કરો {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} જ્યારે (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
સ્થિર સંરચના bitmap *
jpeg_cache_convert(સંરચના content *c)
{
	અચલ uint8_t *source_data; /* Jpeg source data */
	size_t source_size; /* length of Jpeg source data */
	સંરચના jpeg_decompress_struct cinfo;
	સંરચના jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	સંરચના bitmap * અસ્થિર bitmap = ખાલી;
	uint8_t * અસ્થિર pixels = ખાલી;
	size_t rowstride;
	સંરચના jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_આગત_buffer,
		nsjpeg_skip_આગત_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	જો ((source_data == ખાલી) ||
	    (source_size < MIN_JPEG_SIZE)) {
		પાછા ખાલી;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	જો (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		પાછા bitmap;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.next_આગત_byte = source_data;
	source_mgr.bytes_in_buffer = source_size;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	જો (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} નહીંતર {
#ifdef JCS_ALPHA_EXTENSIONS
		પસંદગી (bitmap_fmt.layout) {
		સ્થિતિ BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			તોડો;
		સ્થિતિ BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			તોડો;
		સ્થિતિ BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			તોડો;
		સ્થિતિ BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			તોડો;
		મૂળભૂત:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			પાછા ખાલી;
		}
#નહીંતર
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
	જો (bitmap == ખાલી) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		પાછા ખાલી;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	જો (pixels == ખાલી) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		પાછા ખાલી;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	પસંદગી (cinfo.out_color_space) {
	સ્થિતિ JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		તોડો;

	સ્થિતિ JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		તોડો;

	મૂળભૂત:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		તોડો;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	પાછા bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
સ્થિર bool nsjpeg_convert(સંરચના content *c)
{
	સંરચના jpeg_decompress_struct cinfo;
	સંરચના jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	સંરચના jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_આગત_buffer,
		nsjpeg_skip_આગત_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	સંઘ content_સંદેશ_data સંદેશ_data;
	અચલ uint8_t *data;
	size_t size;
	અક્ષર *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	જો (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		સંદેશ_data.errordata.errorcode = NSERROR_UNKNOWN;
		સંદેશ_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_સંદેશ_ERROR, &સંદેશ_data);
		પાછા false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_આગત_byte = (unsigned અક્ષર *) data;
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

	image_cache_add(c, ખાલી, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	પાછા true;
}



/**
 * Clone content.
 */
સ્થિર nserror nsjpeg_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	સંરચના content *jpeg_c;
	nserror error;

	jpeg_c = calloc(1, કદ(સંરચના content));
	જો (jpeg_c == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	જો (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		પાછા error;
	}

	/* re-convert if the content is ready */
	જો ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		જો (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	પાછા NSERROR_OK;
}

સ્થિર અચલ content_handler nsjpeg_content_handler = {
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

સ્થિર અચલ અક્ષર *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
