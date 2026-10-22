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

#অন্তর্ভাৱ <gst/gst.h>

#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "content/content_protected.h"

#অন্তর্ভাৱ "image/video.h"

প্রকারনির্দেশ সংরচনা nsvideo_content {
	সংরচনা content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

স্থির gboolean nsvideo_bus_call(GstBus *bus, GstMessage *বাৰ্তা, 
		nsvideo_content *video)
{
	ৱিকল্প (GST_MESSAGE_TYPE(বাৰ্তা)) {
	পক্ষ GST_MESSAGE_ERROR:
		ভঙ্গ;
	পক্ষ GST_MESSAGE_EOS:
		ভঙ্গ;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	ঘূৰাই TRUE;
}

স্থির শূন্য nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

স্থির শূন্য nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

স্থির শূন্য nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, রিক্ত);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

স্থির nserror nsvideo_create(ধ্রুৱ content_handler *handler,
		lwc_string *imime_type, ধ্রুৱ http_parameter *params,
		llcache_handle *llcache,
		ধ্রুৱ ৱর্ণ *fallback_charset, bool quirks,
		সংরচনা content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, পরিমাণ(nsvideo_content));
	যদি (video == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(video);
		ঘূৰাই error;
	}

	error = llcache_handle_force_stream(llcache);
	যদি (error != NSERROR_OK) {
		মুক্ত(video);
		ঘূৰাই error;
	}

	video->playbin = gst_element_factory_make("playbin2", রিক্ত);
	যদি (video->playbin == রিক্ত) {
		মুক্ত(video);
		ঘূৰাই NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", রিক্ত);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (সংরচনা content *) video;

	ঘূৰাই NSERROR_OK;
}

স্থির bool nsvideo_process_data(সংরচনা content *c, ধ্রুৱ ৱর্ণ *data, 
		unsigned পূর্ণাঙ্ক size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	ঘূৰাই ret == GST_FLOW_OK;
}

স্থির bool nsvideo_convert(সংরচনা content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	ঘূৰাই ret == GST_FLOW_OK;
}

স্থির শূন্য nsvideo_destroy(সংরচনা content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

স্থির bool nsvideo_redraw(সংরচনা content *c, সংরচনা content_redraw_data *data,
		ধ্রুৱ সংরচনা rect *clip, ধ্রুৱ সংরচনা redraw_context *ctx)
{
	/** \todo Implement */
	ঘূৰাই true;
}

স্থির nserror nsvideo_clone(ধ্রুৱ সংরচনা content *old, সংরচনা content **newc)
{
	/** \todo Implement */
	ঘূৰাই NSERROR_CLONE_FAILED;
}

স্থির content_type nsvideo_type(শূন্য)
{
	/** \todo Lies */
	ঘূৰাই CONTENT_IMAGE;
}

স্থির শূন্য *nsvideo_get_internal(ধ্রুৱ সংরচনা content *c, শূন্য *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	ঘূৰাই রিক্ত;
}

স্থির ধ্রুৱ content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_data = nsvideo_process_data,
	.data_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_internal = nsvideo_get_internal,
	/* Can't share videos because we stream them */
	.no_share = true
};

স্থির ধ্রুৱ ৱর্ণ *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

