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

#అన్తర్భావ <gst/gst.h>

#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/content_protected.h"

#అన్తర్భావ "image/video.h"

రకనిర్వచనం నిర్మాణం nsvideo_content {
	నిర్మాణం content పునాది;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

స్థిర gboolean nsvideo_bus_call(GstBus *bus, Gst_సందేశం *సందేశం, 
		nsvideo_content *video)
{
	ఎంపిక (GST_సందేశం_TYPE(సందేశం)) {
	స్థితి GST_సందేశం_ERROR:
		ఆపు;
	స్థితి GST_సందేశం_EOS:
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	తిరిగి TRUE;
}

స్థిర శూన్య nsvideo_need_దత్తాంశం_event(GstElement *playbin, guint కొలత,
		nsvideo_content *video)
{
}

స్థిర శూన్య nsvideo_enough_దత్తాంశం_event(GstElement *playbin,
		nsvideo_content *video)
{
}

స్థిర శూన్య nsvideo_ఆధారం_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, ఖాళీ);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_దత్తాంశం_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_దత్తాంశం_event), video);
}

స్థిర nserror nsvideo_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి http_parameter *params,
		llcache_handle *llcache,
		స్థిరరాశి అక్షరం *fallback_charset, bool quirks,
		నిర్మాణం content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nsvideo_content));
	ఒకవేళ (video == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&video->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(video);
		తిరిగి error;
	}

	error = llcache_handle_force_ప్రవాహం(llcache);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(video);
		తిరిగి error;
	}

	video->playbin = gst_element_factory_make("playbin2", ఖాళీ);
	ఒకవేళ (video->playbin == ఖాళీ) {
		విడుదల(video);
		తిరిగి NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_కూడు_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_సమితి(video->playbin, "uri", "appsrc://", ఖాళీ);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_ఆధారం_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_సమితి_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (నిర్మాణం content *) video;

	తిరిగి NSERROR_OK;
}

స్థిర bool nsvideo_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం, 
		గుర్తులేని పూర్ణాంకం కొలత)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_బఫరు *బఫరు;
	GstFlowReturn ret;

	బఫరు = gst_బఫరు_new();
	GST_బఫరు_దత్తాంశం(బఫరు) = (guint8 *) దత్తాంశం;
	GST_బఫరు_కొలత(బఫరు) = (gsize) కొలత;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", బఫరు, &ret);

	తిరిగి ret == GST_FLOW_OK;
}

స్థిర bool nsvideo_convert(నిర్మాణం content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	తిరిగి ret == GST_FLOW_OK;
}

స్థిర శూన్య nsvideo_destroy(నిర్మాణం content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_సమితి_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

స్థిర bool nsvideo_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	/** \todo Implement */
	తిరిగి true;
}

స్థిర nserror nsvideo_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	/** \todo Implement */
	తిరిగి NSERROR_CLONE_FAILED;
}

స్థిర content_type nsvideo_type(శూన్య)
{
	/** \todo Lies */
	తిరిగి CONTENT_IMAGE;
}

స్థిర శూన్య *nsvideo_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	తిరిగి ఖాళీ;
}

స్థిర స్థిరరాశి content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_దత్తాంశం = nsvideo_process_దత్తాంశం,
	.దత్తాంశం_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_అంతర్గత = nsvideo_get_అంతర్గత,
	/* Can't share videos because we stream them */
	.no_share = true
};

స్థిర స్థిరరాశి అక్షరం *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

