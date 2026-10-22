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

#ಅನ್ತರ್ಭಾವ <gst/gst.h>

#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"

#ಅನ್ತರ್ಭಾವ "image/video.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nsvideo_content {
	ಸಂರಚನಾ content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ಸ್ಥಿರ gboolean nsvideo_bus_call(GstBus *bus, GstMessage *ಸಂದೇಶ, 
		nsvideo_content *video)
{
	ವಿಕಲ್ಪ (GST_MESSAGE_TYPE(ಸಂದೇಶ)) {
	ಪಕ್ಷ GST_MESSAGE_ERROR:
		ಭಙ್ಗ;
	ಪಕ್ಷ GST_MESSAGE_EOS:
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ TRUE;
}

ಸ್ಥಿರ ಶೂನ್ಯ nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

ಸ್ಥಿರ ಶೂನ್ಯ nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ಸ್ಥಿರ ಶೂನ್ಯ nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ರಿಕ್ತ);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

ಸ್ಥಿರ nserror nsvideo_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ http_parameter *params,
		llcache_handle *llcache,
		ಧ್ರುವ ವರ್ಣ *fallback_charset, bool quirks,
		ಸಂರಚನಾ content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, ಪರಿಮಾಣ(nsvideo_content));
	ಯದಿ (video == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(video);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = llcache_handle_force_stream(llcache);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(video);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	video->playbin = gst_element_factory_make("playbin2", ರಿಕ್ತ);
	ಯದಿ (video->playbin == ರಿಕ್ತ) {
		ಮುಕ್ತ(video);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", ರಿಕ್ತ);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ಸಂರಚನಾ content *) video;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ bool nsvideo_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data, 
		unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	ಪ್ರತ್ಯಾಗಮ ret == GST_FLOW_OK;
}

ಸ್ಥಿರ bool nsvideo_convert(ಸಂರಚನಾ content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	ಪ್ರತ್ಯಾಗಮ ret == GST_FLOW_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ nsvideo_destroy(ಸಂರಚನಾ content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ಸ್ಥಿರ bool nsvideo_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	/** \todo Implement */
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ nserror nsvideo_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	/** \todo Implement */
	ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
}

ಸ್ಥಿರ content_type nsvideo_type(ಶೂನ್ಯ)
{
	/** \todo Lies */
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nsvideo_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nsvideo_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

