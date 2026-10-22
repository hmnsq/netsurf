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

#അന്തര്ഭാവ <gst/gst.h>

#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/content_protected.h"

#അന്തര്ഭാവ "image/video.h"

തരനിർവചനം ഘടന nsvideo_content {
	ഘടന content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

സ്ഥിതം gboolean nsvideo_bus_call(GstBus *bus, GstMessage *സന്ദേശം, 
		nsvideo_content *video)
{
	തിരഞ്ഞെടുപ്പ് (GST_MESSAGE_TYPE(സന്ദേശം)) {
	അവസ്ഥ GST_MESSAGE_ERROR:
		നിർത്തുക;
	അവസ്ഥ GST_MESSAGE_EOS:
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ TRUE;
}

സ്ഥിതം ശൂന്യം nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

സ്ഥിതം ശൂന്യം nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

സ്ഥിതം ശൂന്യം nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ശൂന്യ);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

സ്ഥിതം nserror nsvideo_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം http_parameter *params,
		llcache_handle *llcache,
		സ്ഥിരം അക്ഷരം *fallback_charset, bool quirks,
		ഘടന content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, വലുപ്പം(nsvideo_content));
	എങ്കിൽ (video == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(video);
		തിരികെ error;
	}

	error = llcache_handle_force_stream(llcache);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(video);
		തിരികെ error;
	}

	video->playbin = gst_element_factory_make("playbin2", ശൂന്യ);
	എങ്കിൽ (video->playbin == ശൂന്യ) {
		സ്വതന്ത്രം(video);
		തിരികെ NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", ശൂന്യ);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ഘടന content *) video;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം bool nsvideo_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data, 
		unsigned പൂർണ്ണസംഖ്യ size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	തിരികെ ret == GST_FLOW_OK;
}

സ്ഥിതം bool nsvideo_convert(ഘടന content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	തിരികെ ret == GST_FLOW_OK;
}

സ്ഥിതം ശൂന്യം nsvideo_destroy(ഘടന content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

സ്ഥിതം bool nsvideo_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	/** \todo Implement */
	തിരികെ true;
}

സ്ഥിതം nserror nsvideo_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	/** \todo Implement */
	തിരികെ NSERROR_CLONE_FAILED;
}

സ്ഥിതം content_type nsvideo_type(ശൂന്യം)
{
	/** \todo Lies */
	തിരികെ CONTENT_IMAGE;
}

സ്ഥിതം ശൂന്യം *nsvideo_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	തിരികെ ശൂന്യ;
}

സ്ഥിതം സ്ഥിരം content_handler nsvideo_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

