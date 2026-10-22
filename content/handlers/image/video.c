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

#ଅନ୍ତର୍ଭାଵ <gst/gst.h>

#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"

#ଅନ୍ତର୍ଭାଵ "image/video.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nsvideo_content {
	ସଂରଚନା content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ସ୍ଥିର gboolean nsvideo_bus_call(GstBus *bus, GstMessage *ସନ୍ଦେଶ, 
		nsvideo_content *video)
{
	ବାଛ (GST_MESSAGE_TYPE(ସନ୍ଦେଶ)) {
	ସ୍ଥିତି GST_MESSAGE_ERROR:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି GST_MESSAGE_EOS:
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ TRUE;
}

ସ୍ଥିର ଶୂନ୍ୟ nsvideo_need_data_event(GstElement *playbin, guint size,
		nsvideo_content *video)
{
}

ସ୍ଥିର ଶୂନ୍ୟ nsvideo_enough_data_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ସ୍ଥିର ଶୂନ୍ୟ nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ଖାଲି);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_data_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_data_event), video);
}

ସ୍ଥିର nserror nsvideo_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ http_parameter *params,
		llcache_handle *llcache,
		ଅଚଳ ଅକ୍ଷର *fallback_charset, bool quirks,
		ସଂରଚନା content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = calloc(1, ଆକାର(nsvideo_content));
	ଯଦି (video == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(video);
		ଫେରନ୍ତୁ error;
	}

	error = llcache_handle_force_stream(llcache);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(video);
		ଫେରନ୍ତୁ error;
	}

	video->playbin = gst_element_factory_make("playbin2", ଖାଲି);
	ଯଦି (video->playbin == ଖାଲି) {
		ମୁକ୍ତ(video);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", ଖାଲି);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ସଂରଚନା content *) video;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର bool nsvideo_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data, 
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new();
	GST_BUFFER_DATA(buffer) = (guint8 *) data;
	GST_BUFFER_SIZE(buffer) = (gsize) size;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", buffer, &ret);

	ଫେରନ୍ତୁ ret == GST_FLOW_OK;
}

ସ୍ଥିର bool nsvideo_convert(ସଂରଚନା content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	ଫେରନ୍ତୁ ret == GST_FLOW_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ nsvideo_destroy(ସଂରଚନା content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ସ୍ଥିର bool nsvideo_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	/** \todo Implement */
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର nserror nsvideo_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	/** \todo Implement */
	ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
}

ସ୍ଥିର content_type nsvideo_type(ଶୂନ୍ୟ)
{
	/** \todo Lies */
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}

ସ୍ଥିର ଶୂନ୍ୟ *nsvideo_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ଅଚଳ content_handler nsvideo_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

