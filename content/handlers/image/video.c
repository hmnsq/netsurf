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

#ضمن <gst/gst.h>

#ضمن "content/content_factory.h"
#ضمن "content/content_protected.h"

#ضمن "image/video.h"

تعريفنوع بنية nsvideo_content {
	بنية content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ساكن gboolean nsvideo_bus_call(GstBus *bus, GstMessage *سكو, 
		nsvideo_content *video)
{
	بدل (GST_MESSAGE_TYPE(سكو)) {
	حالة GST_MESSAGE_ERROR:
		اكسر;
	حالة GST_MESSAGE_EOS:
		اكسر;
	افتراضي:
		اكسر;
	}

	أرجع TRUE;
}

ساكن فراغ nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

ساكن فراغ nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ساكن فراغ nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, عدم);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

ساكن nserror nsvideo_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت http_parameter *params,
		llcache_handle *llcache,
		ثابت حرف *fallback_charset, bool quirks,
		بنية content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, حجم(nsvideo_content));
	إذا (video == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(video);
		أرجع error;
	}

	error = llcache_handle_force_stream(llcache);
	إذا (error != NSERROR_OK) {
		حرر(video);
		أرجع error;
	}

	video->playbin = gst_element_factory_make("playbin2", عدم);
	إذا (video->playbin == عدم) {
		حرر(video);
		أرجع NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", عدم);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (بنية content *) video;

	أرجع NSERROR_OK;
}

ساكن bool nsvideo_process_data(بنية content *c, ثابت حرف *data, 
		unsigned صحيح size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	أرجع ret == GST_FLOW_OK;
}

ساكن bool nsvideo_convert(بنية content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	أرجع ret == GST_FLOW_OK;
}

ساكن فراغ nsvideo_destroy(بنية content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ساكن bool nsvideo_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	/** \todo Implement */
	أرجع true;
}

ساكن nserror nsvideo_clone(ثابت بنية content *old, بنية content **newc)
{
	/** \todo Implement */
	أرجع NSERROR_CLONE_FAILED;
}

ساكن content_type nsvideo_type(فراغ)
{
	/** \todo Lies */
	أرجع CONTENT_IMAGE;
}

ساكن فراغ *nsvideo_get_internal(ثابت بنية content *c, فراغ *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	أرجع عدم;
}

ساكن ثابت content_handler nsvideo_content_handler = {
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

ساكن ثابت حرف *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

