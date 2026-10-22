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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <libnsbmp.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image.h"
#شامل "image/ico.h"

قسمتعریف ساخت nsico_content {
	ساخت content بنیاد;

	ساخت ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ساکن خالی *nsico_bitmap_create(صحیح چوڑائی, صحیح اونچائی, غیرعلامتی صحیح bmp_state)
{
	غیرعلامتی صحیح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_حافظہ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	واپس guit->bitmap->create(چوڑائی, اونچائی, bitmap_state);
}

ساکن nserror nsico_create_ico_ڈیٹا(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_بفر = guit->bitmap->get_بفر,
	};

	c->ico = صفر_حافظہ_دو(1, حجم(ico_collection));
	اگر (c->ico == تہی) {
		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	واپس NSERROR_OK;
}


ساکن nserror nsico_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, ساخت content **c)
{
	nsico_content *نتیجہ;
	nserror error;

	نتیجہ = صفر_حافظہ_دو(1, حجم(nsico_content));
	اگر (نتیجہ == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&نتیجہ->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(نتیجہ);
		واپس error;
	}

	error = nsico_create_ico_ڈیٹا(نتیجہ);
	اگر (error != NSERROR_OK) {
		آزاد(نتیجہ);
		واپس error;
	}

	*c = (ساخت content *) نتیجہ;

	واپس NSERROR_OK;
}



ساکن bool nsico_convert(ساخت content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ساخت bmp_image *bmp;
	bmp_نتیجہ res;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;
	حرف *title;

	/* set the ico data */
	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &سائز);

	/* analyse the ico */
	res = ico_analyse(ico->ico, سائز, (غیرعلامتی حرف *) ڈیٹا);

	انتخاب (res) {
	حالت BMP_OK:
		توڑو;
	حالت BMP_INSUFFICIENT_حافظہ:
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	حالت BMP_INSUFFICIENT_ڈیٹا:
	حالت BMP_ڈیٹا_ERROR:
		content_نشر_error(c, NSERROR_ICO_ERROR, تہی);
		واپس false;
	}

	/* Store our content width, height and calculate size */
	c->چوڑائی = ico->ico->چوڑائی;
	c->اونچائی = ico->ico->اونچائی;
	c->سائز += (ico->ico->چوڑائی * ico->ico->اونچائی * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اونچائی);
	اگر (title != تہی) {
		content_سیٹ_title(c, title);
		آزاد(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_ڈھونڈو(ico->ico, 255, 255);
	اگر (bmp == تہی) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		واپس false;
	}

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);

	/* Done: update status bar */
	content_سیٹ_حالت(c, "");
	واپس true;
}

ساکن bool nsico__decode(ساخت bmp_image *ico)
{
	اگر (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		اگر (bmp_decode(ico) != BMP_OK) {
			واپس false;
		}

		bitmap_format_مقصد_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	واپس true;
}

ساکن bool nsico_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ساخت bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_ڈھونڈو(ico->ico, ڈیٹا->چوڑائی, ڈیٹا->اونچائی);
	اگر (bmp == تہی) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		واپس false;
	}

	/* ensure its decided */
	اگر (!nsico__decode(bmp)) {
		واپس false;
	}

	واپس image_bitmap_plot(bmp->bitmap, ڈیٹا, clip, ctx);
}


ساکن خالی nsico_destroy(ساخت content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	آزاد(ico->ico);
}

ساکن nserror nsico_clone(ثابت ساخت content *old, ساخت content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = صفر_حافظہ_دو(1, حجم(nsico_content));
	اگر (ico == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &ico->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&ico->بنیاد);
		واپس error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_ڈیٹا(ico);
	اگر (error != NSERROR_OK) {
		content_destroy(&ico->بنیاد);
		واپس error;
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY ||
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (nsico_convert(&ico->بنیاد) == false) {
			content_destroy(&ico->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) ico;

	واپس NSERROR_OK;
}

ساکن خالی *nsico_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ساخت bmp_image *bmp;

	bmp = ico_ڈھونڈو(ico->ico, 16, 16);
	اگر (bmp == تہی) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		واپس تہی;
	}

	اگر (!nsico__decode(bmp)) {
		واپس تہی;
	}

	واپس bmp->bitmap;
}

ساکن content_type nsico_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}

ساکن bool nsico_is_opaque(ساخت content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ساخت bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_ڈھونڈو(ico->ico, 16, 16);
	اگر (bmp == تہی) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		واپس false;
	}

	اگر (!nsico__decode(bmp)) {
		واپس false;
	}

	واپس guit->bitmap->get_opaque(bmp->bitmap);
}

ساکن ثابت content_handler nsico_content_handler = {
	.create = nsico_create,
	.ڈیٹا_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_اندرونی = nsico_get_اندرونی,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsico, nsico_types, nsico_content_handler);
