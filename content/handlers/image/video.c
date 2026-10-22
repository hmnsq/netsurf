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

#अन्तर्भाव <gst/gst.h>

#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/content_protected.h"

#अन्तर्भाव "image/video.h"

प्रकारव्याख्या संरचना nsvideo_content {
	संरचना content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

स्थिर gboolean nsvideo_bus_call(GstBus *bus, GstMessage *संदेश, 
		nsvideo_content *video)
{
	निवड (GST_MESSAGE_TYPE(संदेश)) {
	स्थिती GST_MESSAGE_ERROR:
		थांबा;
	स्थिती GST_MESSAGE_EOS:
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	परत TRUE;
}

स्थिर शून्य nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

स्थिर शून्य nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

स्थिर शून्य nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, रिक्त);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

स्थिर nserror nsvideo_create(स्थिरांक content_handler *handler,
		lwc_string *imime_type, स्थिरांक http_parameter *params,
		llcache_handle *llcache,
		स्थिरांक अक्षर *fallback_charset, bool quirks,
		संरचना content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = सुस्मृति(1, आकार(nsvideo_content));
	जर (video == रिक्त)
		परत NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	जर (error != NSERROR_OK) {
		मुक्त(video);
		परत error;
	}

	error = llcache_handle_force_stream(llcache);
	जर (error != NSERROR_OK) {
		मुक्त(video);
		परत error;
	}

	video->playbin = gst_element_factory_make("playbin2", रिक्त);
	जर (video->playbin == रिक्त) {
		मुक्त(video);
		परत NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", रिक्त);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (संरचना content *) video;

	परत NSERROR_OK;
}

स्थिर bool nsvideo_process_data(संरचना content *c, स्थिरांक अक्षर *data, 
		अचिन्हित पूर्णांक size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	परत ret == GST_FLOW_OK;
}

स्थिर bool nsvideo_convert(संरचना content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	परत ret == GST_FLOW_OK;
}

स्थिर शून्य nsvideo_destroy(संरचना content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

स्थिर bool nsvideo_redraw(संरचना content *c, संरचना content_redraw_data *data,
		स्थिरांक संरचना rect *clip, स्थिरांक संरचना redraw_context *ctx)
{
	/** \todo Implement */
	परत true;
}

स्थिर nserror nsvideo_clone(स्थिरांक संरचना content *old, संरचना content **newc)
{
	/** \todo Implement */
	परत NSERROR_CLONE_FAILED;
}

स्थिर content_type nsvideo_type(शून्य)
{
	/** \todo Lies */
	परत CONTENT_IMAGE;
}

स्थिर शून्य *nsvideo_get_internal(स्थिरांक संरचना content *c, शून्य *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	परत रिक्त;
}

स्थिर स्थिरांक content_handler nsvideo_content_handler = {
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

स्थिर स्थिरांक अक्षर *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

