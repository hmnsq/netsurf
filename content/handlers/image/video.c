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

प्रकारनिर्देश संरचना nsvideo_content {
	संरचना content आधारः;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

स्थिर gboolean nsvideo_bus_call(GstBus *bus, Gst_सन्देशः *सन्देशः, 
		nsvideo_content *video)
{
	विकल्प (GST_सन्देशः_TYPE(सन्देशः)) {
	पक्ष GST_सन्देशः_ERROR:
		भङ्ग;
	पक्ष GST_सन्देशः_EOS:
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	प्रत्यागम TRUE;
}

स्थिर शून्य nsvideo_need_दत्त_event(GstElement *playbin, guint आकार,
		nsvideo_content *video)
{
}

स्थिर शून्य nsvideo_enough_दत्त_event(GstElement *playbin,
		nsvideo_content *video)
{
}

स्थिर शून्य nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, रिक्त);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_दत्त_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_दत्त_event), video);
}

स्थिर nserror nsvideo_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव http_parameter *params,
		llcache_handle *llcache,
		ध्रुव वर्ण *fallback_charset, bool quirks,
		संरचना content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = शून्यस्मृतिं_देहि(1, परिमाण(nsvideo_content));
	यदि (video == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&video->आधारः, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(video);
		प्रत्यागम error;
	}

	error = llcache_handle_force_प्रवाहः(llcache);
	यदि (error != NSERROR_OK) {
		मुक्त(video);
		प्रत्यागम error;
	}

	video->playbin = gst_element_factory_make("playbin2", रिक्त);
	यदि (video->playbin == रिक्त) {
		मुक्त(video);
		प्रत्यागम NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_सङ्कलय_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_समुच्चयः(video->playbin, "uri", "appsrc://", रिक्त);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_समुच्चयः_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (संरचना content *) video;

	प्रत्यागम NSERROR_OK;
}

स्थिर bool nsvideo_process_दत्त(संरचना content *c, ध्रुव वर्ण *दत्त, 
		अचिह्नित पूर्णाङ्क आकार)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_बफर *बफर;
	GstFlowReturn ret;

	बफर = gst_बफर_new();
	GST_बफर_दत्त(बफर) = (guint8 *) दत्त;
	GST_बफर_आकार(बफर) = (gsize) आकार;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", बफर, &ret);

	प्रत्यागम ret == GST_FLOW_OK;
}

स्थिर bool nsvideo_convert(संरचना content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	प्रत्यागम ret == GST_FLOW_OK;
}

स्थिर शून्य nsvideo_destroy(संरचना content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_समुच्चयः_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

स्थिर bool nsvideo_redraw(संरचना content *c, संरचना content_redraw_दत्त *दत्त,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx)
{
	/** \todo Implement */
	प्रत्यागम true;
}

स्थिर nserror nsvideo_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	/** \todo Implement */
	प्रत्यागम NSERROR_CLONE_FAILED;
}

स्थिर content_type nsvideo_type(शून्य)
{
	/** \todo Lies */
	प्रत्यागम CONTENT_IMAGE;
}

स्थिर शून्य *nsvideo_get_आन्तरिकम्(ध्रुव संरचना content *c, शून्य *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	प्रत्यागम रिक्त;
}

स्थिर ध्रुव content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_दत्त = nsvideo_process_दत्त,
	.दत्त_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_आन्तरिकम् = nsvideo_get_आन्तरिकम्,
	/* Can't share videos because we stream them */
	.no_share = true
};

स्थिर ध्रुव वर्ण *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

