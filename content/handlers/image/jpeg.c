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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <setjmp.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"

#define JPEG_INTERNAL_OPTIONS
#अन्तर्भाव "jpeglib.h"
#अन्तर्भाव "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#define MIN_JPEG_SIZE 20

#ifndef LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#endif

स्थिर अक्षर nsjpeg_error_buffer[JMSG_LENGTH_MAX];

स्थिर अचिन्हित अक्षर nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
स्थिर nserror nsjpeg_create(स्थिरांक content_handler *handler,
		lwc_string *imime_type, स्थिरांक संरचना http_parameter *params,
		llcache_handle *llcache, स्थिरांक अक्षर *fallback_charset,
		bool quirks, संरचना content **c)
{
	संरचना content *jpeg;
	nserror error;

	jpeg = सुस्मृति(1, आकार(संरचना content));
	जर (jpeg == रिक्त)
		परत NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	जर (error != NSERROR_OK) {
		मुक्त(jpeg);
		परत error;
	}

	*c = jpeg;

	परत NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
स्थिर शून्य nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
स्थिर boolean nsjpeg_fill_आदान_buffer(j_decompress_ptr cinfo)
{
	cinfo->src->next_आदान_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_buffer = 2;
 	परत TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

स्थिर शून्य nsjpeg_skip_आदान_data(j_decompress_ptr cinfo, दीर्घ अंक_bytes)
{
	जर ((दीर्घ) cinfo->src->bytes_in_buffer < अंक_bytes) {
		cinfo->src->next_आदान_byte = 0;
		cinfo->src->bytes_in_buffer = 0;
	} अन्यथा {
		cinfo->src->next_आदान_byte += अंक_bytes;
		cinfo->src->bytes_in_buffer -= अंक_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
स्थिर शून्य nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
स्थिर शून्य nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
स्थिर शून्य nsjpeg_error_exit(j_common_ptr cinfo)
{
	लाँघ_बफ *setjmp_buffer = (लाँघ_बफ *) cinfo->client_data;

	cinfo->err->format_message(cinfo, nsjpeg_error_buffer);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_buffer);

	दीर्घ_लाँघ(*setjmp_buffer, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
स्थिर ओळीत शून्य nsjpeg__decode_cmyk(
		संरचना jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
	पूर्णांक width = cinfo->output_width * 4;

	करा {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		साठी (पूर्णांक i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			स्थिरांक पूर्णांक c = scanlines[0][i + 0];
			स्थिरांक पूर्णांक m = scanlines[0][i + 1];
			स्थिरांक पूर्णांक y = scanlines[0][i + 2];
			स्थिरांक पूर्णांक k = scanlines[0][i + 3];

			स्थिरांक पूर्णांक ck = c * k;
			स्थिरांक पूर्णांक mk = m * k;
			स्थिरांक पूर्णांक yk = y * k;

#define DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#undef DIV255
		}
	} पर्यंत (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
स्थिर ओळीत शून्य nsjpeg__decode_rgb(
		संरचना jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
#जर RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	पूर्णांक width = cinfo->output_width;
#endif

	करा {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#जर RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		साठी (पूर्णांक i = width - 1; 0 <= i; i--) {
			पूर्णांक r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			पूर्णांक g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			पूर्णांक b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#endif
	} पर्यंत (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
स्थिर ओळीत शून्य nsjpeg__decode_client_fmt(
		संरचना jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
	करा {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} पर्यंत (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
स्थिर संरचना bitmap *
jpeg_cache_convert(संरचना content *c)
{
	स्थिरांक uint8_t *source_data; /* Jpeg source data */
	माप_प्रकार source_size; /* length of Jpeg source data */
	संरचना jpeg_decompress_struct cinfo;
	संरचना jpeg_error_mgr jerr;
	लाँघ_बफ setjmp_buffer;
	संरचना bitmap * अस्थिर bitmap = रिक्त;
	uint8_t * अस्थिर pixels = रिक्त;
	माप_प्रकार rowstride;
	संरचना jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_आदान_buffer,
		nsjpeg_skip_आदान_data,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_data = content__get_source_data(c, &source_size);

	जर ((source_data == रिक्त) ||
	    (source_size < MIN_JPEG_SIZE)) {
		परत रिक्त;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	जर (बनाओ_लाँघ(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		परत bitmap;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.next_आदान_byte = source_data;
	source_mgr.bytes_in_buffer = source_size;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	जर (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} अन्यथा {
#ifdef JCS_ALPHA_EXTENSIONS
		निवड (bitmap_fmt.layout) {
		स्थिती BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			थांबा;
		स्थिती BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			थांबा;
		स्थिती BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			थांबा;
		स्थिती BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			थांबा;
		पूर्वनिर्धारित:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			परत रिक्त;
		}
#अन्यथा
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
	जर (bitmap == रिक्त) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		परत रिक्त;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	जर (pixels == रिक्त) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		परत रिक्त;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	निवड (cinfo.out_color_space) {
	स्थिती JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		थांबा;

	स्थिती JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		थांबा;

	पूर्वनिर्धारित:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		थांबा;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	परत bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
स्थिर bool nsjpeg_convert(संरचना content *c)
{
	संरचना jpeg_decompress_struct cinfo;
	संरचना jpeg_error_mgr jerr;
	लाँघ_बफ setjmp_buffer;
	संरचना jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_आदान_buffer,
		nsjpeg_skip_आदान_data, jpeg_resync_to_restart,
		nsjpeg_term_source };
	संघ content_संदेश_data संदेश_data;
	स्थिरांक uint8_t *data;
	माप_प्रकार size;
	अक्षर *title;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	जर (बनाओ_लाँघ(setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		संदेश_data.errordata.errorcode = NSERROR_UNKNOWN;
		संदेश_data.errordata.errormsg = nsjpeg_error_buffer;
		content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_data);
		परत false;
	}

	cinfo.client_data = &setjmp_buffer;
	jpeg_create_decompress(&cinfo);
	source_mgr.next_आदान_byte = (अचिन्हित अक्षर *) data;
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

	image_cache_add(c, रिक्त, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	जर (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	परत true;
}



/**
 * Clone content.
 */
स्थिर nserror nsjpeg_clone(स्थिरांक संरचना content *old, संरचना content **newc)
{
	संरचना content *jpeg_c;
	nserror error;

	jpeg_c = सुस्मृति(1, आकार(संरचना content));
	जर (jpeg_c == रिक्त)
		परत NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	जर (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		परत error;
	}

	/* re-convert if the content is ready */
	जर ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		जर (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			परत NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	परत NSERROR_OK;
}

स्थिर स्थिरांक content_handler nsjpeg_content_handler = {
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

स्थिर स्थिरांक अक्षर *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
