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

#समावेश <gst/gst.h>

#समावेश "content/content_factory.h"
#समावेश "content/content_protected.h"

#समावेश "image/video.h"

प्रकार काष्ठा nsvideo_content {
	काष्ठा content base;

	GstElement *playbin;
	GstElement *appsrc;
} nsvideo_content;

अटल gboolean nsvideo_bus_call(GstBus *bus, GstMessage *संदेश, 
		nsvideo_content *video)
{
	चयन (GST_MESSAGE_TYPE(संदेश)) {
	हाल GST_MESSAGE_ERROR:
		अवरोध;
	हाल GST_MESSAGE_EOS:
		अवरोध;
	शेष:
		अवरोध;
	}

	वापस TRUE;
}

अटल व्योम nsvideo_need_तथ्य_event(GstElement *playbin, guint आकार,
		nsvideo_content *video)
{
}

अटल व्योम nsvideo_enough_तथ्य_event(GstElement *playbin,
		nsvideo_content *video)
{
}

अटल व्योम nsvideo_source_event(GObject *object, GObject *orig, 
		GParamSpec *pspec, nsvideo_content *video)
{
	g_object_get(orig, pspec->name, &video->appsrc, शून्य);

	g_signal_connect(video->appsrc, "need-data", 
			G_CALLBACK(nsvideo_need_तथ्य_event), video);
	g_signal_connect(video->appsrc, "enough-data",
			G_CALLBACK(nsvideo_enough_तथ्य_event), video);
}

अटल nserror nsvideo_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर http_parameter *params,
		llcache_handle *llcache,
		स्थिर अक्षर *fallback_charset, bool quirks,
		काष्ठा content **c)
{
	nsvideo_content *video;
	nserror error;
	GstBus *bus;

	video = सुस्मृति(1, माप(nsvideo_content));
	अगर (video == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&video->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(video);
		वापस error;
	}

	error = llcache_handle_force_stream(llcache);
	अगर (error != NSERROR_OK) {
		मुक्त(video);
		वापस error;
	}

	video->playbin = gst_element_factory_make("playbin2", शून्य);
	अगर (video->playbin == शून्य) {
		मुक्त(video);
		वापस NSERROR_NOMEM;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(video->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) nsvideo_bus_call, video);
	gst_object_unref(bus);

	g_object_set(video->playbin, "uri", "appsrc://", शून्य);
	g_signal_connect(video->playbin, "deep-notify::source",
			G_CALLBACK(nsvideo_source_event), video);

	/** \todo Create appsink & register with playbin */

	gst_element_set_state(video->playbin, GST_STATE_PLAYING);
	
	*c = (काष्ठा content *) video;

	वापस NSERROR_OK;
}

अटल bool nsvideo_process_तथ्य(काष्ठा content *c, स्थिर अक्षर *तथ्य, 
		अचिन्हित पूर्णांक आकार)
{
	nsvideo_content *video = (nsvideo_content *) c;
	Gst_बफर *बफर;
	GstFlowReturn ret;

	बफर = gst_बफर_new();
	GST_बफर_तथ्य(बफर) = (guint8 *) तथ्य;
	GST_बफर_आकार(बफर) = (gsize) आकार;

	/* Send data to appsrc */
	g_signal_emit_by_name(video->appsrc, "push-buffer", बफर, &ret);

	वापस ret == GST_FLOW_OK;
}

अटल bool nsvideo_convert(काष्ठा content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;
	GstFlowReturn ret;

	/* Tell appsrc we're done */
	g_signal_emit_by_name(video->appsrc, "end-of-stream", &ret);

	/* Appsink will flag DONE on receipt of first frame */

	वापस ret == GST_FLOW_OK;
}

अटल व्योम nsvideo_destroy(काष्ठा content *c)
{
	nsvideo_content *video = (nsvideo_content *) c;

	gst_element_set_state(video->playbin, GST_STATE_NULL);
	gst_object_unref(video->playbin);
}

अटल bool nsvideo_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	/** \todo Implement */
	वापस true;
}

अटल nserror nsvideo_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	/** \todo Implement */
	वापस NSERROR_CLONE_FAILED;
}

अटल content_type nsvideo_type(व्योम)
{
	/** \todo Lies */
	वापस CONTENT_IMAGE;
}

अटल व्योम *nsvideo_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	/** \todo Return pointer to bitmap containing current frame, if any? */
	वापस शून्य;
}

अटल स्थिर content_handler nsvideo_content_handler = {
	.create = nsvideo_create,
	.process_तथ्य = nsvideo_process_तथ्य,
	.तथ्य_complete = nsvideo_convert,
	.destroy = nsvideo_destroy,
	.redraw = nsvideo_redraw,
	.clone = nsvideo_clone,
	.type = nsvideo_type,
	.get_internal = nsvideo_get_internal,
	/* Can't share videos because we stream them */
	.no_share = true
};

अटल स्थिर अक्षर *nsvideo_types[] = {
	"video/mp4",
	"video/webm"
};

CONTENT_FACTORY_REGISTER_TYPES(nsvideo, nsvideo_types, 
		nsvideo_content_handler);

