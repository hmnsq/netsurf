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

#அந்தர்பாவ <gst/gst.h>

#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/content_protected.h"

#அந்தர்பாவ "image/video.h"

வகைவரை கட்டமைப்பு nsvideo_content {
	கட்டமைப்பு content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

நிலையான gboolean nsvideo_bus_call(GstBus *bus, GstMessage *செய்தி, 
		nsvideo_content *video)
{
	தேர்வு (GST_MESSAGE_TYPE(செய்தி)) {
	நிலை GST_MESSAGE_ERROR:
		நிறுத்து;
	நிலை GST_MESSAGE_EOS:
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	திரும்பு TRUE;
}

நிலையான வெறுமை nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

நிலையான வெறுமை nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

நிலையான வெறுமை nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, வெற்று);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

நிலையான nserror nsvideo_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி http_parameter *params,
		llcache_handle *llcache,
		மாறிலி எழுத்து *fallback_charset, bool quirks,
		கட்டமைப்பு content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, அளவு(nsvideo_content));
	எனில் (video == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(video);
		திரும்பு error;
	}

	error = llcache_handle_force_stream(llcache);
	எனில் (error != NSERROR_OK) {
		விடுவி(video);
		திரும்பு error;
	}

	video->playbin = gst_element_factory_make("playbin2", வெற்று);
	எனில் (video->playbin == வெற்று) {
		விடுவி(video);
		திரும்பு NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", வெற்று);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (கட்டமைப்பு content *) video;

	திரும்பு NSERROR_OK;
}

நிலையான bool nsvideo_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data, 
		unsigned முழு size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	திரும்பு ret == GST_FLOW_OK;
}

நிலையான bool nsvideo_convert(கட்டமைப்பு content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	திரும்பு ret == GST_FLOW_OK;
}

நிலையான வெறுமை nsvideo_destroy(கட்டமைப்பு content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

நிலையான bool nsvideo_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	/** \todo Implement */
	திரும்பு true;
}

நிலையான nserror nsvideo_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	/** \todo Implement */
	திரும்பு NSERROR_CLONE_FAILED;
}

நிலையான content_type nsvideo_type(வெறுமை)
{
	/** \todo Lies */
	திரும்பு CONTENT_IMAGE;
}

நிலையான வெறுமை *nsvideo_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	திரும்பு வெற்று;
}

நிலையான மாறிலி content_handler nsvideo_content_handler = {
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

நிலையான மாறிலி எழுத்து *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

