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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <setjmp.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ <jxl/decode.h>

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

#અન્તર્ભાવ "image/jpegxl.h"


/**
 * output image format
 */
સ્થિર અચલ JxlPixelFormat jxl_output_format = {
	.અંક_channels = 4,
	.data_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
સ્થિર nserror
nsjpegxl_create(અચલ content_handler *handler,
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
 * create a bitmap from jpeg xl content.
 */
સ્થિર સંરચના bitmap *
jpegxl_cache_convert(સંરચના content *c)
{
	સંરચના bitmap * bitmap = ખાલી;
	JxlDecoder *jxldec;
	JxlDecoderStatus decstatus;
	JxlBasicInfo binfo;
	અચલ uint8_t *src_data;
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

	jxldec = JxlDecoderCreate(ખાલી);
	જો (jxldec == ખાલી) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		પાછા ખાલી;
	}

	decstatus = JxlDecoderSetUnpremultiplyAlpha(jxldec, !bitmap_fmt.pma);
	જો (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_FULL_IMAGE);
	જો (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		પાછા ખાલી;
	}
	src_data = content__get_source_data(c, &src_size);

	decstatus = Jxl_Decoder_Set_આગત(jxldec, src_data, src_size);
	જો (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		પાછા ખાલી;
	}

	decstatus = Jxl_Decoder_Process_આગત(jxldec);
	જો (decstatus != JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_OUT_BUFFER,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	જો (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}

	/* create bitmap with appropriate opacity */
	જો (binfo.alpha_bits > 0) {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_OPAQUE);
	} નહીંતર {
		bitmap = guit->bitmap->create(c->width, c->height, BITMAP_NONE);
	}
	જો (bitmap == ખાલી) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_buffer(bitmap);
	જો (output == ખાલી) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}
	decstatus = JxlDecoderSetImageOutBuffer(jxldec, &jxl_output_format, output, c->size);
	જો (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}

	decstatus = Jxl_Decoder_Process_આગત(jxldec);
	જો (decstatus != JXL_DEC_FULL_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		પાછા ખાલી;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_to_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	પાછા bitmap;
}

/**
 * report failiure
 */
સ્થિર bool jxl_report_fail(સંરચના content *c, JxlDecoderStatus decstatus, અચલ અક્ષર *સંદેશ)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", સંદેશ, decstatus);
	સંદેશ_data.errordata.errorcode = NSERROR_UNKNOWN;
	સંદેશ_data.errordata.errormsg = સંદેશ;
	content_broadcast(c, CONTENT_સંદેશ_ERROR, &સંદેશ_data);
	પાછા false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
સ્થિર bool nsjpegxl_convert(સંરચના content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	JxlDecoderStatus decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	સંઘ content_સંદેશ_data સંદેશ_data;
	અચલ uint8_t *data;
	size_t size;
	અક્ષર *title;
	size_t image_size;

	/* check image header is valid and get width/height */
	data = content__get_source_data(c, &size);

	decsig = JxlSignatureCheck(data,size);
	જો ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		સંદેશ_data.errordata.errorcode = NSERROR_UNKNOWN;
		સંદેશ_data.errordata.errormsg = "Signature failed";
		content_broadcast(c, CONTENT_સંદેશ_ERROR, &સંદેશ_data);
		પાછા false;
	}

	jxldec = JxlDecoderCreate(ખાલી);
	જો (jxldec == ખાલી) {
		પાછા jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	જો (decstatus != JXL_DEC_SUCCESS) {
		પાછા jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_Set_આગત(jxldec, data,size);
	જો (decstatus != JXL_DEC_SUCCESS) {
		પાછા jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_આગત(jxldec);
	જો (decstatus != JXL_DEC_BASIC_INFO) {
		પાછા jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	જો (decstatus != JXL_DEC_SUCCESS) {
		પાછા jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = JxlDecoderImageOutBufferSize(jxldec, &jxl_output_format, &image_size);
	જો (decstatus != JXL_DEC_SUCCESS) {
		પાછા jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_size, binfo.xsize, binfo.ysize);

	c->width = binfo.xsize;
	c->height = binfo.ysize;
	c->size = image_size;

	image_cache_add(c, ખાલી, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
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
સ્થિર nserror nsjpegxl_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	સંરચના content *jpegxl_c;
	nserror error;

	jpegxl_c = calloc(1, કદ(સંરચના content));
	જો (jpegxl_c == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	જો (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		પાછા error;
	}

	/* re-convert if the content is ready */
	જો ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		જો (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	પાછા NSERROR_OK;
}

સ્થિર અચલ content_handler nsjpegxl_content_handler = {
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

સ્થિર અચલ અક્ષર *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
