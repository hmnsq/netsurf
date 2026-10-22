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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"

#define JPEG_INTERNAL_OPTIONS
#ଅନ୍ତର୍ଭାଵ "jpeglib.h"
#ଅନ୍ତର୍ଭାଵ "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

ସ୍ଥିର ଅକ୍ଷର nsjpeg_error_buffer[JMSG_LENGTH_MAX];

ସ୍ଥିର unsigned ଅକ୍ଷର nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
ସ୍ଥିର nserror nsjpeg_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	ସଂରଚନା content *jpeg;
	nserror error;

	jpeg = calloc(1, ଆକାର(ସଂରଚନା content));
	ଯଦି (jpeg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(jpeg);
		ଫେରନ୍ତୁ error;
	}

	*c = jpeg;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
ସ୍ଥିର ଶୂନ୍ୟ nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
ସ୍ଥିର boolean nsjpeg_fill_ନିବେଶ_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_ନିବେଶ_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	ଫେରନ୍ତୁ TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

ସ୍ଥିର ଶୂନ୍ୟ nsjpeg_skip_ନିବେଶ_data(j_decompress_ptr cinfo, long ଅଙ୍କ_bytes)
{
	ଯଦି ((long) cinfo->src->bytes_in_buffer < ଅଙ୍କ_bytes) {
		cinfo->src->next_ନିବେଶ_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} ଅନ୍ୟଥା {
		cinfo->src->next_ନିବେଶ_byte += ଅଙ୍କ_bytes;
		cinfo->src->bytes_in_buffer -= ଅଙ୍କ_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
ସ୍ଥିର ଶୂନ୍ୟ nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
ସ୍ଥିର ଶୂନ୍ୟ nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
ସ୍ଥିର ଶୂନ୍ୟ nsjpeg_error_exit(j_common_ptr cinfo)
{
	jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	longjmp(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ nsjpeg__decode_cmyk(
		ସଂରଚନା jpeg_decompress_struct *cinfo,
		uint8_t * ଅସ୍ଥିର pixels,
		size_t rowstride)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = cinfo->output_width * 4;

	କର {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = scanlines[0][i + 0];
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = scanlines[0][i + 1];
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା y = scanlines[0][i + 2];
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = scanlines[0][i + 3];

			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ck = c * k;
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା mk = m * k;
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} ଯେତେବେଳେ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ nsjpeg__decode_rgb(
		ସଂରଚନା jpeg_decompress_struct *cinfo,
		uint8_t * ଅସ୍ଥିର pixels,
		size_t rowstride)
{
#ଯଦି RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = cinfo->output_width;
#endif

	କର {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#ଯଦି RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = width - 1; 0 <= i; i--) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} ଯେତେବେଳେ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ nsjpeg__decode_client_fmt(
		ସଂରଚନା jpeg_decompress_struct *cinfo,
		uint8_t * ଅସ୍ଥିର pixels,
		size_t rowstride)
{
	କର {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} ଯେତେବେଳେ (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
ସ୍ଥିର ସଂରଚନା bitmap *
jpeg_cache_convert(ସଂରଚନା content *c)
{
	ଅଚଳ uint8_t *source_data; /* Jpeg source data */
	size_t source_size; /* length of Jpeg source data */
	ସଂରଚନା jpeg_decompress_struct cinfo;
	ସଂରଚନା jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	ସଂରଚନା bitmap * ଅସ୍ଥିର bitmap = ଖାଲି;
	uint8_t * ଅସ୍ଥିର pixels = ଖାଲି;
	size_t rowstride;
	ସଂରଚନା jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_ନିବେଶ_buffer,
		nsjpeg_skip_ନିବେଶ_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	ଯଦି ((source_data == ଖାଲି) ||
	    (source_size < MIN_JPEG_SIZE)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	ଯଦି (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		ଫେରନ୍ତୁ bitmap;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.next_ନିବେଶ_byte = source_data;
	source_mgr.bytes_in_buffer = source_size;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	ଯଦି (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} ଅନ୍ୟଥା {
#ifdef JCS_ALPHA_EXTENSIONS
		ବାଛ (bitmap_fmt.layout) {
		ସ୍ଥିତି BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			ଫେରନ୍ତୁ ଖାଲି;
		}
#ଅନ୍ୟଥା
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
	ଯଦି (bitmap == ଖାଲି) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	ଯଦି (pixels == ଖାଲି) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	ବାଛ (cinfo.out_color_space) {
	ସ୍ଥିତି JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		ଭାଙ୍ଗ;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	ଫେରନ୍ତୁ bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
ସ୍ଥିର bool nsjpeg_convert(ସଂରଚନା content *c)
{
	ସଂରଚନା jpeg_decompress_struct cinfo;
	ସଂରଚନା jpeg_error_mgr jerr;
	jmp_buf setjmp_buffer;
	ସଂରଚନା jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_ନିବେଶ_buffer,
		nsjpeg_skip_ନିବେଶ_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ଅଚଳ uint8_t *data;
	size_t size;
	ଅକ୍ଷର *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	ଯଦି (setjmp(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		ସନ୍ଦେଶ_data.errordata.errorcode = NSERROR_UNKNOWN;
		ସନ୍ଦେଶ_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_ERROR, &ସନ୍ଦେଶ_data);
		ଫେରନ୍ତୁ false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_ନିବେଶ_byte = (unsigned ଅକ୍ଷର *) data;
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

	image_cache_add(c, ଖାଲି, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	ଫେରନ୍ତୁ true;
}



/**
 * Clone content.
 */
ସ୍ଥିର nserror nsjpeg_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	ସଂରଚନା content *jpeg_c;
	nserror error;

	jpeg_c = calloc(1, ଆକାର(ସଂରଚନା content));
	ଯଦି (jpeg_c == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		ଫେରନ୍ତୁ error;
	}

	/* re-convert if the content is ready */
	ଯଦି ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ଯଦି (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଅଚଳ content_handler nsjpeg_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
