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

#અન્તર્ભાવ <gst/gst.h>

#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/content_protected.h"

#અન્તર્ભાવ "image/video.h"

પ્રકારવ્યાખ્યા સંરચના nsvideo_content {
	સંરચના content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

સ્થિર gboolean nsvideo_bus_call(GstBus *bus, GstMessage *સંદેશ, 
		nsvideo_content *video)
{
	પસંદગી (GST_MESSAGE_TYPE(સંદેશ)) {
	સ્થિતિ GST_MESSAGE_ERROR:
		તોડો;
	સ્થિતિ GST_MESSAGE_EOS:
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	પાછા TRUE;
}

સ્થિર શૂન્ય nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

સ્થિર શૂન્ય nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

સ્થિર શૂન્ય nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ખાલી);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

સ્થિર nserror nsvideo_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ http_parameter *params,
		llcache_handle *llcache,
		અચલ અક્ષર *fallback_charset, bool quirks,
		સંરચના content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, કદ(nsvideo_content));
	જો (video == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(video);
		પાછા error;
	}

	error = llcache_handle_force_stream(llcache);
	જો (error != NSERROR_OK) {
		મુક્ત(video);
		પાછા error;
	}

	video->playbin = gst_element_factory_make("playbin2", ખાલી);
	જો (video->playbin == ખાલી) {
		મુક્ત(video);
		પાછા NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", ખાલી);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (સંરચના content *) video;

	પાછા NSERROR_OK;
}

સ્થિર bool nsvideo_process_data(સંરચના content *c, અચલ અક્ષર *data, 
		unsigned પૂર્ણાંક size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	પાછા ret == GST_FLOW_OK;
}

સ્થિર bool nsvideo_convert(સંરચના content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	પાછા ret == GST_FLOW_OK;
}

સ્થિર શૂન્ય nsvideo_destroy(સંરચના content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

સ્થિર bool nsvideo_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	/** \todo Implement */
	પાછા true;
}

સ્થિર nserror nsvideo_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	/** \todo Implement */
	પાછા NSERROR_CLONE_FAILED;
}

સ્થિર content_type nsvideo_type(શૂન્ય)
{
	/** \todo Lies */
	પાછા CONTENT_IMAGE;
}

સ્થિર શૂન્ય *nsvideo_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	પાછા ખાલી;
}

સ્થિર અચલ content_handler nsvideo_content_handler = {
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

સ્થિર અચલ અક્ષર *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

