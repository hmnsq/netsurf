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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <setjmp.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ <jxl/decode.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/image_cache.h"

#అన్తర్భావ "image/jpegxl.h"


/**
 * output image format
 */
స్థిర స్థిరరాశి JxlPixelFormat jxl_output_format = {
	.అంకె_channels = 4,
	.దత్తాంశం_type    = JXL_TYPE_UINT8,
	.endianness   = JXL_LITTLE_ENDIAN,
	.align        = 0,
};

/**
 * Content create entry point.
 */
స్థిర nserror
nsjpegxl_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	నిర్మాణం content *jpeg;
	nserror error;

	jpeg = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content));
	ఒకవేళ (jpeg == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(jpeg, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(jpeg);
		తిరిగి error;
	}

	*c = jpeg;

	తిరిగి NSERROR_OK;
}

/**
 * create a bitmap from jpeg xl content.
 */
స్థిర నిర్మాణం bitmap *
jpegxl_cache_convert(నిర్మాణం content *c)
{
	నిర్మాణం bitmap * bitmap = ఖాళీ;
	JxlDecoder *jxldec;
	Jxl_Decoder_స్థితి decstatus;
	JxlBasicInfo binfo;
	స్థిరరాశి uint8_t *src_దత్తాంశం;
	size_t src_కొలత;
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

	jxldec = JxlDecoderCreate(ఖాళీ);
	ఒకవేళ (jxldec == ఖాళీ) {
		NSLOG(netsurf, ERROR, "Unable to allocate decoder");
		తిరిగి ఖాళీ;
	}

	decstatus = Jxl_Decoder_సమితి_Unpremultiply_Alpha(jxldec, !bitmap_fmt.pma);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set premultiplied alpha status: %d",
				decstatus);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}

	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_నిండిన_IMAGE);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "Unable to subscribe");
		తిరిగి ఖాళీ;
	}
	src_దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &src_కొలత);

	decstatus = Jxl_Decoder_సమితి_ఆదానం(jxldec, src_దత్తాంశం, src_కొలత);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set input");
		తిరిగి ఖాళీ;
	}

	decstatus = Jxl_Decoder_Process_ఆదానం(jxldec);
	ఒకవేళ (decstatus != JXL_DEC_NEED_IMAGE_బహిర్గతం_బఫరు) {
		NSLOG(netsurf, ERROR,
		      "expected status JXL_DEC_NEED_IMAGE_OUT_BUFFER(%d) got %d",
		      JXL_DEC_NEED_IMAGE_బహిర్గతం_బఫరు,
		      decstatus);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}

	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to get basic info status:%d",decstatus);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}

	/* create bitmap with appropriate opacity */
	ఒకవేళ (binfo.alpha_బిట్లు > 0) {
		bitmap = guit->bitmap->create(c->వెడల్పు, c->ఎత్తు, BITMAP_OPAQUE);
	} లేకపోతే {
		bitmap = guit->bitmap->create(c->వెడల్పు, c->ఎత్తు, BITMAP_NONE);
	}
	ఒకవేళ (bitmap == ఖాళీ) {
		/* empty bitmap could not be created */
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}

	/* ensure buffer was allocated */
	output = guit->bitmap->get_బఫరు(bitmap);
	ఒకవేళ (output == ఖాళీ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}
	decstatus = Jxl_Decoder_సమితి_Image_బహిర్గతం_బఫరు(jxldec, &jxl_output_format, output, c->కొలత);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		NSLOG(netsurf, ERROR, "unable to set output buffer callback status:%d",decstatus);
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}

	decstatus = Jxl_Decoder_Process_ఆదానం(jxldec);
	ఒకవేళ (decstatus != JXL_DEC_నిండిన_IMAGE) {
		NSLOG(netsurf, ERROR, "did not get decode event");
		guit->bitmap->destroy(bitmap);
		JxlDecoderDestroy(jxldec);
		తిరిగి ఖాళీ;
	}
	
	JxlDecoderDestroy(jxldec);

	bitmap_format_గమ్యం_client(bitmap, &jxl_fmt);
	guit->bitmap->modified(bitmap);

	తిరిగి bitmap;
}

/**
 * report failiure
 */
స్థిర bool jxl_report_fail(నిర్మాణం content *c, Jxl_Decoder_స్థితి decstatus, స్థిరరాశి అక్షరం *సందేశం)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	NSLOG(netsurf, ERROR, "%s decoder status:%d", సందేశం, decstatus);
	సందేశం_దత్తాంశం.errordata.errorcode = NSERROR_UNKNOWN;
	సందేశం_దత్తాంశం.errordata.errormsg = సందేశం;
	content_ప్రసారం(c, CONTENT_సందేశం_ERROR, &సందేశం_దత్తాంశం);
	తిరిగి false;
}

/**
 * Convert a CONTENT_JPEGXL for display.
 */
స్థిర bool nsjpegxl_convert(నిర్మాణం content *c)
{
	JxlDecoder *jxldec;
	JxlSignature decsig;
	Jxl_Decoder_స్థితి decstatus = JXL_DEC_ERROR;
	JxlBasicInfo binfo;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
	అక్షరం *title;
	size_t image_కొలత;

	/* check image header is valid and get width/height */
	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	decsig = JxlSignatureCheck(దత్తాంశం,కొలత);
	ఒకవేళ ((decsig != JXL_SIG_CODESTREAM) && (decsig != JXL_SIG_CONTAINER)) {
		NSLOG(netsurf, ERROR, "signature failed");
		సందేశం_దత్తాంశం.errordata.errorcode = NSERROR_UNKNOWN;
		సందేశం_దత్తాంశం.errordata.errormsg = "Signature failed";
		content_ప్రసారం(c, CONTENT_సందేశం_ERROR, &సందేశం_దత్తాంశం);
		తిరిగి false;
	}

	jxldec = JxlDecoderCreate(ఖాళీ);
	ఒకవేళ (jxldec == ఖాళీ) {
		తిరిగి jxl_report_fail(c, decstatus, "Unable to allocate decoder");
	}
	decstatus= JxlDecoderSubscribeEvents(jxldec, JXL_DEC_BASIC_INFO);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		తిరిగి jxl_report_fail(c, decstatus, "Unable to subscribe");
	}
	decstatus = Jxl_Decoder_సమితి_ఆదానం(jxldec, దత్తాంశం,కొలత);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		తిరిగి jxl_report_fail(c, decstatus, "unable to set input");
	}
	decstatus = Jxl_Decoder_Process_ఆదానం(jxldec);
	ఒకవేళ (decstatus != JXL_DEC_BASIC_INFO) {
		తిరిగి jxl_report_fail(c, decstatus, "did not get basic info event");
	}
	decstatus = JxlDecoderGetBasicInfo(jxldec, &binfo);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		తిరిగి jxl_report_fail(c, decstatus, "unable to get basic info");
	}
	decstatus = Jxl_Decoder_Image_బహిర్గతం_బఫరు_కొలత(jxldec, &jxl_output_format, &image_కొలత);
	ఒకవేళ (decstatus != JXL_DEC_SUCCESS) {
		తిరిగి jxl_report_fail(c, decstatus, "unable get image size");
	}
	
	JxlDecoderDestroy(jxldec);

	NSLOG(netsurf, INFO, "got basic info size:%ld x:%d y:%d", image_కొలత, binfo.xsize, binfo.ysize);

	c->వెడల్పు = binfo.xsize;
	c->ఎత్తు = binfo.ysize;
	c->కొలత = image_కొలత;

	image_cache_కూడు(c, ఖాళీ, jpegxl_cache_convert);

	/* set title text */
	title = messages_get_buff("JPEGXLTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);	
	content_సమితి_స్థితి(c, ""); /* Done: update status bar */

	తిరిగి true;
}


/**
 * Clone content.
 */
స్థిర nserror nsjpegxl_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	నిర్మాణం content *jpegxl_c;
	nserror error;

	jpegxl_c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content));
	ఒకవేళ (jpegxl_c == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, jpegxl_c);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(jpegxl_c);
		తిరిగి error;
	}

	/* re-convert if the content is ready */
	ఒకవేళ ((old->స్థితి_2 == CONTENT_స్థితి_READY) ||
	    (old->స్థితి_2 == CONTENT_స్థితి_పూర్తి)) {
		ఒకవేళ (nsjpegxl_convert(jpegxl_c) == false) {
			content_destroy(jpegxl_c);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = jpegxl_c;

	తిరిగి NSERROR_OK;
}

స్థిర స్థిరరాశి content_handler nsjpegxl_content_handler = {
	.create = nsjpegxl_create,
	.దత్తాంశం_complete = nsjpegxl_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = nsjpegxl_clone,
	.get_అంతర్గత = image_cache_get_అంతర్గత,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *nsjpegxl_types[] = {
	"image/jxl",
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsjpegxl, nsjpegxl_types, nsjpegxl_content_handler);
