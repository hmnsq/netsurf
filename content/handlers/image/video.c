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

#شامل <gst/gst.h>

#شامل "content/content_factory.h"
#شامل "content/content_protected.h"

#شامل "image/video.h"

تعریفنوع ساختار nsvideo_content {
	ساختار content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ایستا gboolean nsvideo_bus_call(GstBus *bus, GstMessage *پیام, 
		nsvideo_content *video)
{
	انتخاب (GST_MESSAGE_TYPE(پیام)) {
	حالت GST_MESSAGE_ERROR:
		بشکن;
	حالت GST_MESSAGE_EOS:
		بشکن;
	پیشفرض:
		بشکن;
	}

	بازگشت TRUE;
}

ایستا تهی nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

ایستا تهی nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ایستا تهی nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, پوچ);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

ایستا nserror nsvideo_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت http_parameter *params,
		llcache_handle *llcache,
		ثابت نویسه *fallback_charset, bool quirks,
		ساختار content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, اندازه(nsvideo_content));
	اگر (video == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(video);
		بازگشت error;
	}

	error = llcache_handle_force_stream(llcache);
	اگر (error != NSERROR_OK) {
		آزاد(video);
		بازگشت error;
	}

	video->playbin = gst_element_factory_make("playbin2", پوچ);
	اگر (video->playbin == پوچ) {
		آزاد(video);
		بازگشت NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", پوچ);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ساختار content *) video;

	بازگشت NSERROR_OK;
}

ایستا bool nsvideo_process_data(ساختار content *c, ثابت نویسه *data, 
		unsigned صحیح size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	بازگشت ret == GST_FLOW_OK;
}

ایستا bool nsvideo_convert(ساختار content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	بازگشت ret == GST_FLOW_OK;
}

ایستا تهی nsvideo_destroy(ساختار content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ایستا bool nsvideo_redraw(ساختار content *c, ساختار content_redraw_data *data,
		ثابت ساختار rect *clip, ثابت ساختار redraw_context *ctx)
{
	/** \todo Implement */
	بازگشت true;
}

ایستا nserror nsvideo_clone(ثابت ساختار content *old, ساختار content **newc)
{
	/** \todo Implement */
	بازگشت NSERROR_CLONE_FAILED;
}

ایستا content_type nsvideo_type(تهی)
{
	/** \todo Lies */
	بازگشت CONTENT_IMAGE;
}

ایستا تهی *nsvideo_get_internal(ثابت ساختار content *c, تهی *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	بازگشت پوچ;
}

ایستا ثابت content_handler nsvideo_content_handler = {
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

ایستا ثابت نویسه *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

