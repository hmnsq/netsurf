/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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

#অন্তর্ভাব <gst/gst.h>

#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/content_protected.h"

#অন্তর্ভাব "image/video.h"

প্রকারলেখ কাঠামো nsvideo_content {
	কাঠামো content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

স্থির gboolean nsvideo_bus_call(GstBus *bus, GstMessage *বার্তা, 
		nsvideo_content *video)
{
	নির্বাচন (GST_MESSAGE_TYPE(বার্তা)) {
	ক্ষেত্র GST_MESSAGE_ERROR:
		বিরতি;
	ক্ষেত্র GST_MESSAGE_EOS:
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	ফেরত TRUE;
}

স্থির শূন্য nsvideo_need_তথ্য_event(GstElement *playbin, guint আকার_2,
		nsvideo_content *video)
{
}

স্থির শূন্য nsvideo_enough_তথ্য_event(GstElement *playbin,
		nsvideo_content *video)
{
}

স্থির শূন্য nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ফাঁকা);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_তথ্য_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_তথ্য_event), video);
}

স্থির nserror nsvideo_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক http_parameter *params,
		llcache_handle *llcache,
		ধ্রুবক অক্ষর *fallback_charset, bool quirks,
		কাঠামো content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, আকার(nsvideo_content));
	যদি (video == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(video);
		ফেরত error;
	}

	error = llcache_handle_force_stream(llcache);
	যদি (error != NSERROR_OK) {
		মুক্ত(video);
		ফেরত error;
	}

	video->playbin = gst_element_factory_make("playbin2", ফাঁকা);
	যদি (video->playbin == ফাঁকা) {
		মুক্ত(video);
		ফেরত NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", ফাঁকা);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (কাঠামো content *) video;

	ফেরত NSERROR_OK;
}

স্থির bool nsvideo_process_তথ্য(কাঠামো content *c, ধ্রুবক অক্ষর *তথ্য, 
		unsigned পূর্ণসংখ্যা আকার_2)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_বাফার *বাফার;
	GstFlowReturn ret;

	বাফার = gst_বাফার_new();
	GST_বাফার_তথ্য(বাফার) = (guint8 *) তথ্য;
	GST_বাফার_আকার(বাফার) = (gsize) আকার_2;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", বাফার, &ret);

	ফেরত ret == GST_FLOW_OK;
}

স্থির bool nsvideo_convert(কাঠামো content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	ফেরত ret == GST_FLOW_OK;
}

স্থির শূন্য nsvideo_destroy(কাঠামো content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

স্থির bool nsvideo_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	/** \todo Implement */
	ফেরত true;
}

স্থির nserror nsvideo_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	/** \todo Implement */
	ফেরত NSERROR_CLONE_FAILED;
}

স্থির content_type nsvideo_type(শূন্য)
{
	/** \todo Lies */
	ফেরত CONTENT_IMAGE;
}

স্থির শূন্য *nsvideo_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	ফেরত ফাঁকা;
}

স্থির ধ্রুবক content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_তথ্য = nsvideo_process_তথ্য,
	.তথ্য_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_internal = nsvideo_get_internal,
	/* Can't share videos because we stream them */
	.no_share = true
};

স্থির ধ্রুবক অক্ষর *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

