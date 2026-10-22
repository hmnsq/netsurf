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

قسمتعریف ساخت nsvideo_content {
	ساخت content بنیاد;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

ساکن gboolean nsvideo_bus_call(GstBus *bus, Gst_پیغام *پیغام, 
		nsvideo_content *video)
{
	انتخاب (GST_پیغام_TYPE(پیغام)) {
	حالت GST_پیغام_ERROR:
		توڑو;
	حالت GST_پیغام_EOS:
		توڑو;
	طےشدہ:
		توڑو;
	}

	واپس TRUE;
}

ساکن خالی nsvideo_need_ڈیٹا_event(GstElement *playbin, guint سائز,
		nsvideo_content *video)
{
}

ساکن خالی nsvideo_enough_ڈیٹا_event(GstElement *playbin,
		nsvideo_content *video)
{
}

ساکن خالی nsvideo_ماخذ_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, تہی);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_ڈیٹا_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_ڈیٹا_event), video);
}

ساکن nserror nsvideo_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت http_parameter *params,
		llcache_handle *llcache,
		ثابت حرف *fallback_charset, bool quirks,
		ساخت content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = صفر_حافظہ_دو(1, حجم(nsvideo_content));
	اگر (video == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&video->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(video);
		واپس error;
	}

	error = llcache_handle_force_دھارا(llcache);
	اگر (error != NSERROR_OK) {
		آزاد(video);
		واپس error;
	}

	video->playbin = gst_element_factory_make("playbin2", تہی);
	اگر (video->playbin == تہی) {
		آزاد(video);
		واپس NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_جوڑو_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_سیٹ(video->playbin, "uri", "appsrc://", تہی);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_ماخذ_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_سیٹ_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (ساخت content *) video;

	واپس NSERROR_OK;
}

ساکن bool nsvideo_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا, 
		غیرعلامتی صحیح سائز)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_بفر *بفر;
	GstFlowReturn ret;

	بفر = gst_بفر_new();
	GST_بفر_ڈیٹا(بفر) = (guint8 *) ڈیٹا;
	GST_بفر_سائز(بفر) = (gsize) سائز;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", بفر, &ret);

	واپس ret == GST_FLOW_OK;
}

ساکن bool nsvideo_convert(ساخت content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	واپس ret == GST_FLOW_OK;
}

ساکن خالی nsvideo_destroy(ساخت content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_سیٹ_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

ساکن bool nsvideo_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	/** \todo Implement */
	واپس true;
}

ساکن nserror nsvideo_clone(ثابت ساخت content *old, ساخت content **newc)
{
	/** \todo Implement */
	واپس NSERROR_CLONE_FAILED;
}

ساکن content_type nsvideo_type(خالی)
{
	/** \todo Lies */
	واپس CONTENT_IMAGE;
}

ساکن خالی *nsvideo_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	واپس تہی;
}

ساکن ثابت content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_ڈیٹا = nsvideo_process_ڈیٹا,
	.ڈیٹا_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_اندرونی = nsvideo_get_اندرونی,
	/* Can't share videos because we stream them */
	.no_share = true
};

ساکن ثابت حرف *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

