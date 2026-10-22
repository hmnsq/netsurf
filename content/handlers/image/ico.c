/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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

/**
 * \file
 * implementation for image/ico content handler
 */

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <libnsbmp.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image.h"
#अन्तर्भाव "image/ico.h"

प्रकारनिर्देश संरचना nsico_content {
	संरचना content आधारः;

	संरचना ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
स्थिर शून्य *nsico_bitmap_create(पूर्णाङ्क विस्तारः, पूर्णाङ्क height, अचिह्नित पूर्णाङ्क bmp_state)
{
	अचिह्नित पूर्णाङ्क bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_स्मृतिः) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	प्रत्यागम guit->bitmap->create(विस्तारः, height, bitmap_state);
}

स्थिर nserror nsico_create_ico_दत्त(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_बफर = guit->bitmap->get_बफर,
	};

	c->ico = शून्यस्मृतिं_देहि(1, परिमाण(ico_collection));
	यदि (c->ico == रिक्त) {
		content_प्रसारणम्_error(&c->आधारः, NSERROR_NOMEM, रिक्त);
		प्रत्यागम NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	प्रत्यागम NSERROR_OK;
}


स्थिर nserror nsico_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	nsico_content *परिणाम;
	nserror error;

	परिणाम = शून्यस्मृतिं_देहि(1, परिमाण(nsico_content));
	यदि (परिणाम == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&परिणाम->आधारः, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(परिणाम);
		प्रत्यागम error;
	}

	error = nsico_create_ico_दत्त(परिणाम);
	यदि (error != NSERROR_OK) {
		मुक्त(परिणाम);
		प्रत्यागम error;
	}

	*c = (संरचना content *) परिणाम;

	प्रत्यागम NSERROR_OK;
}



स्थिर bool nsico_convert(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;
	संरचना bmp_image *bmp;
	bmp_परिणाम res;
	ध्रुव uint8_t *दत्त;
	size_t आकार;
	वर्ण *title;

	/* set the ico data */
	दत्त = content_get_source_दत्त_2(c, &आकार);

	/* analyse the ico */
	res = ico_analyse(ico->ico, आकार, (अचिह्नित वर्ण *) दत्त);

	विकल्प (res) {
	पक्ष BMP_OK:
		भङ्ग;
	पक्ष BMP_INSUFFICIENT_स्मृतिः:
		content_प्रसारणम्_error(c, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;
	पक्ष BMP_INSUFFICIENT_दत्त:
	पक्ष BMP_दत्त_ERROR:
		content_प्रसारणम्_error(c, NSERROR_ICO_ERROR, रिक्त);
		प्रत्यागम false;
	}

	/* Store our content width, height and calculate size */
	c->विस्तारः = ico->ico->विस्तारः;
	c->height = ico->ico->height;
	c->आकार += (ico->ico->विस्तारः * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_पर्णम्(llcache_handle_get_url(c->llcache)),
			c->विस्तारः, c->height);
	यदि (title != रिक्त) {
		content_समुच्चयः_title(c, title);
		मुक्त(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	यदि (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		प्रत्यागम false;
	}

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);

	/* Done: update status bar */
	content_समुच्चयः_स्थितिः(c, "");
	प्रत्यागम true;
}

स्थिर bool nsico__decode(संरचना bmp_image *ico)
{
	यदि (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		यदि (bmp_decode(ico) != BMP_OK) {
			प्रत्यागम false;
		}

		bitmap_format_गन्तव्यम्_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	प्रत्यागम true;
}

स्थिर bool nsico_redraw(संरचना content *c, संरचना content_redraw_दत्त *दत्त,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	संरचना bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, दत्त->विस्तारः, दत्त->height);
	यदि (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		प्रत्यागम false;
	}

	/* ensure its decided */
	यदि (!nsico__decode(bmp)) {
		प्रत्यागम false;
	}

	प्रत्यागम image_bitmap_plot(bmp->bitmap, दत्त, clip, ctx);
}


स्थिर शून्य nsico_destroy(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	मुक्त(ico->ico);
}

स्थिर nserror nsico_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = शून्यस्मृतिं_देहि(1, परिमाण(nsico_content));
	यदि (ico == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &ico->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&ico->आधारः);
		प्रत्यागम error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_दत्त(ico);
	यदि (error != NSERROR_OK) {
		content_destroy(&ico->आधारः);
		प्रत्यागम error;
	}

	यदि (old->स्थितिः == CONTENT_स्थितिः_READY ||
			old->स्थितिः == CONTENT_स्थितिः_समाप्तम्) {
		यदि (nsico_convert(&ico->आधारः) == false) {
			content_destroy(&ico->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) ico;

	प्रत्यागम NSERROR_OK;
}

स्थिर शून्य *nsico_get_आन्तरिकम्(ध्रुव संरचना content *c, शून्य *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	संरचना bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	यदि (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		प्रत्यागम रिक्त;
	}

	यदि (!nsico__decode(bmp)) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम bmp->bitmap;
}

स्थिर content_type nsico_content_type(शून्य)
{
	प्रत्यागम CONTENT_IMAGE;
}

स्थिर bool nsico_is_opaque(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;
	संरचना bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	यदि (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		प्रत्यागम false;
	}

	यदि (!nsico__decode(bmp)) {
		प्रत्यागम false;
	}

	प्रत्यागम guit->bitmap->get_opaque(bmp->bitmap);
}

स्थिर ध्रुव content_handler nsico_content_handler = {
	.create = nsico_create,
	.दत्त_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_आन्तरिकम् = nsico_get_आन्तरिकम्,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nsico, nsico_types, nsico_content_handler);
