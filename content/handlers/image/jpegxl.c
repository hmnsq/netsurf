/*
 * Copyright 2023 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handling for image/jpegxl
 *
 * This implementation uses the JXL library.
 */

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ <jxl/decode.h>

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

#ଅନ୍ତର୍ଭାଵ "image/jpegxl.h"


/**
 * output image format
 */
ସ୍ଥିର ଅଚଳ JxlPixelFormat jxl_output_format = {
	.ଅଙ୍କ_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
ସ୍ଥିର nserror
nsjpegxl_create(ଅଚଳ content_handler *handler,
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
 * create a bitmap from jpeg xl content.
 */
ସ୍ଥିର ସଂରଚନା bitmap *
jpegxl_cache_convert(ସଂରଚନା content *c)
{
	ସଂରଚନା bitmap * bitmap = ଖାଲି;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	ଅଚଳ uint8_t *src_data;
	size_t src_size;
	uint8_t * output;
	bitmap_fmt_t jxl_fmt = {
		/** TODO: At the moment we have to set the layout to the only
		 *        pixel layout that libjxl supports. It looks like they
		 *        plan to add support for decoding to other layouts
		 *        in the future, as shown by the TODO in the docs:
		 *
		 * https://libjxl.readthedocs.io/en/latest/api_common.html#_CPPv414JxlPixelFormat
		 */
		.layout = BITMAP_LAYOUT_R8G8B8A8,
		.pma    = bitmap_fmt.pma,
	};

	jxldec = JxlDecoderCreate(ଖାଲି);
	ଯଦି (jxldec == ଖାଲି) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		ଫେରନ୍ତୁ ଖାଲି;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_ନିବେଶ(jxldec, src_data, src_size);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	decstatus = Jxl_Decoder_Process_ନିବେଶ(jxldec);
	ଯଦି (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* create bitmap with appropriate opacity */
	ଯଦି (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} ଅନ୍ୟଥା {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	ଯଦି (bitmap == ଖାଲି) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	ଯଦି (output == ଖାଲି) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	decstatus = Jxl_Decoder_Process_ନିବେଶ(jxldec);
	ଯଦି (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	ଫେରନ୍ତୁ bitmap;
}

/**
 * report failiure
 */
ସ୍ଥିର bool jxl_report_fail(ସଂରଚନା content *c, JxlDecoderStatus decstatus, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", ସନ୍ଦେଶ, decstatus);
	ସନ୍ଦେଶ_data.errordata.errorcode = NSERROR_UNKNOWN;
	ସନ୍ଦେଶ_data.errordata.errormsg = ସନ୍ଦେଶ;
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_ERROR, &ସନ୍ଦେଶ_data);
	ଫେରନ୍ତୁ false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
ସ୍ଥିର bool nsjpegxl_convert(ସଂରଚନା content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ଅଚଳ uint8_t *data;
	size_t size;
	ଅକ୍ଷର *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	ଯଦି ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		ସନ୍ଦେଶ_data.errordata.errorcode = NSERROR_UNKNOWN;
		ସନ୍ଦେଶ_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_ERROR, &ସନ୍ଦେଶ_data);
		ଫେରନ୍ତୁ false;
	}

	jxldec = JxlDecoderCreate(ଖାଲି);
	ଯଦି (jxldec == ଖାଲି) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_ନିବେଶ(jxldec, data,size);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ନିବେଶ(jxldec);
	ଯଦି (decstatus != JXL_DEC_BASIC_INFO) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	ଯଦି (decstatus != JXL_DEC_SUCCESS) {
		ଫେରନ୍ତୁ jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, ଖାଲି, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
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
ସ୍ଥିର nserror nsjpegxl_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	ସଂରଚନା content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, ଆକାର(ସଂରଚନା content));
	ଯଦି (jpegxl_c == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		ଫେରନ୍ତୁ error;
	}

	/* re-convert if the content is ready */
	ଯଦି ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ଯଦି (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଅଚଳ content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.data_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
