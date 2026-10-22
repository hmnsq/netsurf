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

#ਅਨ੍ਤਰ੍ਭਾਵ <gst/gst.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/video.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nsvideo_content {
	ਬਣਤਰ content ਅਧਾਰ;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ਸਥਿਰ gboolean nsvideo_bus_call(GstBus *bus, Gst_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, 
		nsvideo_content *video)
{
	ਚੋਣ (GST_ਸੁਨੇਹਾ_TYPE(ਸੁਨੇਹਾ)) {
	ਹਾਲਤ GST_ਸੁਨੇਹਾ_ERROR:
		ਰੋਕੋ;
	ਹਾਲਤ GST_ਸੁਨੇਹਾ_EOS:
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ TRUE;
}

ਸਥਿਰ ਖਾਲੀ nsvideo_need_ਅੰਕੜੇ_event(GstElement *playbin, guint ਮਾਪ,
		nsvideo_content *video)
{
}

ਸਥਿਰ ਖਾਲੀ nsvideo_enough_ਅੰਕੜੇ_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ਸਥਿਰ ਖਾਲੀ nsvideo_ਸਰੋਤ_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ਨਿਰਾ);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_ਅੰਕੜੇ_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_ਅੰਕੜੇ_event), video);
}

ਸਥਿਰ nserror nsvideo_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ http_parameter *params,
		llcache_handle *llcache,
		ਅਚਲ ਅੱਖਰ *fallback_charset, bool quirks,
		ਬਣਤਰ content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nsvideo_content));
	ਜੇ (video == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&video->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(video);
		ਵਾਪਸ error;
	}

	error = llcache_handle_force_ਧਾਰਾ(llcache);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(video);
		ਵਾਪਸ error;
	}

	video->playbin = gst_element_factory_make("playbin2", ਨਿਰਾ);
	ਜੇ (video->playbin == ਨਿਰਾ) {
		ਮੁਕਤ(video);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_ਜੋੜੋ_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_ਸੈੱਟ(video->playbin, "uri", "appsrc://", ਨਿਰਾ);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_ਸਰੋਤ_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_ਸੈੱਟ_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ਬਣਤਰ content *) video;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ bool nsvideo_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, 
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_ਬਫਰ *ਬਫਰ;
	GstFlowReturn ret;

	ਬਫਰ = gst_ਬਫਰ_new();
	GST_ਬਫਰ_ਅੰਕੜੇ(ਬਫਰ) = (guint8 *) ਅੰਕੜੇ;
	GST_ਬਫਰ_ਮਾਪ(ਬਫਰ) = (gsize) ਮਾਪ;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", ਬਫਰ, &ret);

	ਵਾਪਸ ret == GST_FLOW_OK;
}

ਸਥਿਰ bool nsvideo_convert(ਬਣਤਰ content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	ਵਾਪਸ ret == GST_FLOW_OK;
}

ਸਥਿਰ ਖਾਲੀ nsvideo_destroy(ਬਣਤਰ content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_ਸੈੱਟ_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ਸਥਿਰ bool nsvideo_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	/** \todo Implement */
	ਵਾਪਸ true;
}

ਸਥਿਰ nserror nsvideo_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	/** \todo Implement */
	ਵਾਪਸ NSERROR_CLONE_FAILED;
}

ਸਥਿਰ content_type nsvideo_type(ਖਾਲੀ)
{
	/** \todo Lies */
	ਵਾਪਸ CONTENT_IMAGE;
}

ਸਥਿਰ ਖਾਲੀ *nsvideo_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਅਚਲ content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_ਅੰਕੜੇ = nsvideo_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_ਅੰਦਰੂਨੀ = nsvideo_get_ਅੰਦਰੂਨੀ,
	/* Can't share videos because we stream them */
	.no_share = true
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

