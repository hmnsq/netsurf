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

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <setjmp.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/bitmap.h"
#समावेश "content/llcache.h"
#समावेश "content/content.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/image_cache.h"

#घोषणा JPEG_INTERNAL_OPTIONS
#समावेश "jpeglib.h"
#समावेश "jpeg.h"

/** absolute minimum size of a jpeg below which it is not even worth
 * trying to read header data
 */
#घोषणा MIN_JPEG_आकार 20

#अगर_अघोषित LIBJPEG_TURBO_VERSION
#warning Using libjpeg (libjpeg-turbo is recommended)
#पूर्ण_अगर

अटल अक्षर nsjpeg_error_बफर[JMSG_लंबाई_MAX];

अटल अचिन्हित अक्षर nsjpeg_eoi[] = { 0xff, JPEG_EOI };

/**
 * Content create entry point.
 */
अटल nserror nsjpeg_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	काष्ठा content *jpeg;
	nserror error;

	jpeg = सुस्मृति(1, माप(काष्ठा content));
	अगर (jpeg == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(jpeg);
		वापस error;
	}

	*c = jpeg;

	वापस NSERROR_OK;
}

/**
 * JPEG data source manager: initialize source.
 */
अटल व्योम nsjpeg_init_source(j_decompress_ptr cinfo)
{
}


/**
 * JPEG data source manager: fill the input buffer.
 *
 * This can only occur if the JPEG data was truncated or corrupted. Insert a
 * fake EOI marker to allow the decompressor to output as much as possible.
 */
अटल boolean nsjpeg_fill_निवेश_बफर(j_decompress_ptr cinfo)
{
	cinfo->src->अग्र_निवेश_byte = nsjpeg_eoi;
	cinfo->src->bytes_in_बफर = 2;
 	वापस TRUE;
}


/**
 * JPEG data source manager: skip num_bytes worth of data.
 */

अटल व्योम nsjpeg_skip_निवेश_तथ्य(j_decompress_ptr cinfo, दीर्घ अंक_bytes)
{
	अगर ((दीर्घ) cinfo->src->bytes_in_बफर < अंक_bytes) {
		cinfo->src->अग्र_निवेश_byte = 0;
		cinfo->src->bytes_in_बफर = 0;
	} अन्यथा {
		cinfo->src->अग्र_निवेश_byte += अंक_bytes;
		cinfo->src->bytes_in_बफर -= अंक_bytes;
	}
}


/**
 * JPEG data source manager: terminate source.
 */
अटल व्योम nsjpeg_term_source(j_decompress_ptr cinfo)
{
}


/**
 * Error output handler for JPEG library.
 *
 * This logs to NetSurf log instead of stderr.
 * Warnings only - fatal errors are trapped by nsjpeg_error_exit
 *                 and do not call the output handler.
 */
अटल व्योम nsjpeg_error_log(j_common_ptr cinfo)
{
	cinfo->err->format_message(cinfo, nsjpeg_error_बफर);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_बफर);
}


/**
 * Fatal error handler for JPEG library.
 *
 * This prevents jpeglib calling exit() on a fatal error.
 */
अटल व्योम nsjpeg_error_exit(j_common_ptr cinfo)
{
	लाँघ_बफ *setjmp_बफर = (लाँघ_बफ *) cinfo->client_तथ्य;

	cinfo->err->format_message(cinfo, nsjpeg_error_बफर);
	NSLOG(netsurf, INFO, "%s", nsjpeg_error_बफर);

	दीर्घ_लाँघ(*setjmp_बफर, 1);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
अटल अंतरभूत व्योम nsjpeg__decode_cmyk(
		काष्ठा jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
	पूर्णांक width = cinfo->output_width * 4;

	करो {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

		क्रम (पूर्णांक i = width - 4; 0 <= i; i -= 4) {
			/* Trivial inverse CMYK -> RGBA */
			स्थिर पूर्णांक c = scanlines[0][i + 0];
			स्थिर पूर्णांक m = scanlines[0][i + 1];
			स्थिर पूर्णांक y = scanlines[0][i + 2];
			स्थिर पूर्णांक k = scanlines[0][i + 3];

			स्थिर पूर्णांक ck = c * k;
			स्थिर पूर्णांक mk = m * k;
			स्थिर पूर्णांक yk = y * k;

#घोषणा DIV255(x) ((x) + 1 + ((x) >> 8)) >> 8
			scanlines[0][i + bitmap_layout.r] = DIV255(ck);
			scanlines[0][i + bitmap_layout.g] = DIV255(mk);
			scanlines[0][i + bitmap_layout.b] = DIV255(yk);
			scanlines[0][i + bitmap_layout.a] = 0xff;
#अघोषित DIV255
		}
	} जबतक (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
अटल अंतरभूत व्योम nsjpeg__decode_rgb(
		काष्ठा jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
#अगर RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
	पूर्णांक width = cinfo->output_width;
#पूर्ण_अगर

	करो {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);

#अगर RGB_RED != 0 || RGB_GREEN != 1 || RGB_BLUE != 2 || RGB_PIXELSIZE != 4
		/* Missmatch between configured libjpeg pixel format and
		 * NetSurf pixel format.  Convert to RGBA */
		क्रम (पूर्णांक i = width - 1; 0 <= i; i--) {
			पूर्णांक r = scanlines[0][i * RGB_PIXELSIZE + RGB_RED];
			पूर्णांक g = scanlines[0][i * RGB_PIXELSIZE + RGB_GREEN];
			पूर्णांक b = scanlines[0][i * RGB_PIXELSIZE + RGB_BLUE];
			scanlines[0][i * 4 + bitmap_layout.r] = r;
			scanlines[0][i * 4 + bitmap_layout.g] = g;
			scanlines[0][i * 4 + bitmap_layout.b] = b;
			scanlines[0][i * 4 + bitmap_layout.a] = 0xff;
		}
#पूर्ण_अगर
	} जबतक (cinfo->output_scanline != cinfo->output_height);
}

/**
 * Convert scan lines from CMYK to core client bitmap layout.
 */
अटल अंतरभूत व्योम nsjpeg__decode_client_fmt(
		काष्ठा jpeg_decompress_struct *cinfo,
		uint8_t * अस्थिर pixels,
		माप_प्रकार rowstride)
{
	करो {
		JSAMPROW scanlines[1] = {
			[0] = (JSAMPROW)
				(pixels + rowstride * cinfo->output_scanline),
		};
		jpeg_read_scanlines(cinfo, scanlines, 1);
	} जबतक (cinfo->output_scanline != cinfo->output_height);
}

/**
 * create a bitmap from jpeg content.
 */
अटल काष्ठा bitmap *
jpeg_cache_convert(काष्ठा content *c)
{
	स्थिर uint8_t *source_तथ्य; /* Jpeg source data */
	माप_प्रकार source_आकार; /* length of Jpeg source data */
	काष्ठा jpeg_decompress_struct cinfo;
	काष्ठा jpeg_error_mgr jerr;
	लाँघ_बफ setjmp_बफर;
	काष्ठा bitmap * अस्थिर bitmap = शून्य;
	uint8_t * अस्थिर pixels = शून्य;
	माप_प्रकार rowstride;
	काष्ठा jpeg_source_mgr source_mgr = {
		0,
		0,
		nsjpeg_init_source,
		nsjpeg_fill_निवेश_बफर,
		nsjpeg_skip_निवेश_तथ्य,
		jpeg_resync_to_restart,
		nsjpeg_term_source };

	/* obtain jpeg source data and perfom minimal sanity checks */
	source_तथ्य = content_get_source_तथ्य_2(c, &source_आकार);

	अगर ((source_तथ्य == शून्य) ||
	    (source_आकार < MIN_JPEG_आकार)) {
		वापस शून्य;
	}

	/* setup a JPEG library error handler */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	/* handler for fatal errors during decompression */
	अगर (बनाओ_लाँघ(setjmp_बफर)) {
		jpeg_destroy_decompress(&cinfo);
		वापस bitmap;
	}

	cinfo.client_तथ्य = &setjmp_बफर;
	jpeg_create_decompress(&cinfo);

	/* setup data source */
	source_mgr.अग्र_निवेश_byte = source_तथ्य;
	source_mgr.bytes_in_बफर = source_आकार;
	cinfo.src = &source_mgr;

	/* read JPEG header information */
	jpeg_read_header(&cinfo, TRUE);

	/* set output processing parameters */
	अगर (cinfo.jpeg_color_space == JCS_CMYK ||
	    cinfo.jpeg_color_space == JCS_YCCK) {
		cinfo.out_color_space = JCS_CMYK;
	} अन्यथा {
#अगर_घोषित JCS_ALPHA_EXTENSIONS
		चयन (bitmap_fmt.layout) {
		हाल BITMAP_LAYOUT_R8G8B8A8:
			cinfo.out_color_space = JCS_EXT_RGBA;
			अवरोध;
		हाल BITMAP_LAYOUT_B8G8R8A8:
			cinfo.out_color_space = JCS_EXT_BGRA;
			अवरोध;
		हाल BITMAP_LAYOUT_A8R8G8B8:
			cinfo.out_color_space = JCS_EXT_ARGB;
			अवरोध;
		हाल BITMAP_LAYOUT_A8B8G8R8:
			cinfo.out_color_space = JCS_EXT_ABGR;
			अवरोध;
		शेष:
			NSLOG(netsurf, ERROR, "Unexpected bitmap format: %u",
					bitmap_fmt.layout);
			jpeg_destroy_decompress(&cinfo);
			वापस शून्य;
		}
#अन्यथा
		cinfo.out_color_space = JCS_RGB;
#पूर्ण_अगर
	}
	cinfo.dct_method = JDCT_ISLOW;

	/* commence the decompression, output parameters now valid */
	jpeg_start_decompress(&cinfo);

	/* create opaque bitmap (jpegs cannot be transparent) */
	bitmap = guit->bitmap->create(
			cinfo.output_width,
			cinfo.output_height, BITMAP_OPAQUE);
	अगर (bitmap == शून्य) {
		/* empty bitmap could not be created */
		jpeg_destroy_decompress(&cinfo);
		वापस शून्य;
	}

	pixels = guit->bitmap->get_बफर(bitmap);
	अगर (pixels == शून्य) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		jpeg_destroy_decompress(&cinfo);
		वापस शून्य;
	}

	/* Convert scanlines from jpeg into bitmap */
	rowstride = guit->bitmap->get_rowstride(bitmap);

	चयन (cinfo.out_color_space) {
	हाल JCS_CMYK:
		nsjpeg__decode_cmyk(&cinfo, pixels, rowstride);
		अवरोध;

	हाल JCS_RGB:
		nsjpeg__decode_rgb(&cinfo, pixels, rowstride);
		अवरोध;

	शेष:
		nsjpeg__decode_client_fmt(&cinfo, pixels, rowstride);
		अवरोध;
	}

	guit->bitmap->modified(bitmap);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	वापस bitmap;
}

/**
 * Convert a CONTENT_JPEG for display.
 */
अटल bool nsjpeg_convert(काष्ठा content *c)
{
	काष्ठा jpeg_decompress_struct cinfo;
	काष्ठा jpeg_error_mgr jerr;
	लाँघ_बफ setjmp_बफर;
	काष्ठा jpeg_source_mgr source_mgr = { 0, 0,
		nsjpeg_init_source, nsjpeg_fill_निवेश_बफर,
		nsjpeg_skip_निवेश_तथ्य, jpeg_resync_to_restart,
		nsjpeg_term_source };
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;
	अक्षर *title;

	/* check image header is valid and get width/height */
	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = nsjpeg_error_exit;
	jerr.output_message = nsjpeg_error_log;

	अगर (बनाओ_लाँघ(setjmp_बफर)) {
		jpeg_destroy_decompress(&cinfo);

		संदेश_तथ्य.errordata.errorcode = NSERROR_UNKNOWN;
		संदेश_तथ्य.errordata.errormsg = nsjpeg_error_बफर;
		content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_तथ्य);
		वापस false;
	}

	cinfo.client_तथ्य = &setjmp_बफर;
	jpeg_create_decompress(&cinfo);
	source_mgr.अग्र_निवेश_byte = (अचिन्हित अक्षर *) तथ्य;
	source_mgr.bytes_in_बफर = आकार;
	cinfo.src = &source_mgr;
	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	cinfo.dct_method = JDCT_ISLOW;

	jpeg_calc_output_dimensions(&cinfo);

	c->width = cinfo.output_width;
	c->height = cinfo.output_height;
	c->आकार = c->width * c->height * 4;

	jpeg_destroy_decompress(&cinfo);

	image_cache_add(c, शून्य, jpeg_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	अगर (title != शून्य) {
		content__set_title(c, title);
		मुक्त(title);
	}

	content_set_ready(c);
	content_set_done(c);	
	content_set_status(c, ""); /* Done: update status bar */

	वापस true;
}



/**
 * Clone content.
 */
अटल nserror nsjpeg_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	काष्ठा content *jpeg_c;
	nserror error;

	jpeg_c = सुस्मृति(1, माप(काष्ठा content));
	अगर (jpeg_c == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, jpeg_c);
	अगर (error != NSERROR_OK) {
		content_destroy(jpeg_c);
		वापस error;
	}

	/* re-convert if the content is ready */
	अगर ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		अगर (nsjpeg_convert(jpeg_c) == false) {
			content_destroy(jpeg_c);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpeg_c;

	वापस NSERROR_OK;
}

अटल स्थिर content_handler nsjpeg_content_handler = {
	.create = nsjpeg_create,
	.तथ्य_complete = nsjpeg_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpeg_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *nsjpeg_types[] = {
	"image/jpeg",
	"image/jpg",
	"image/pjpeg"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpeg, nsjpeg_types, nsjpeg_content_handler);
