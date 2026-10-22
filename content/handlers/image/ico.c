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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <libnsbmp.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image.h"
#ضمن "image/ico.h"

تعريفنوع بنية nsico_content {
	بنية content بنياد;

	بنية ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ساكن فراغ *nsico_bitmap_create(صحيح ويڪر, صحيح اوچائي, بنانشان صحيح bmp_state)
{
	بنانشان صحيح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_يادگيري) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	أرجع guit->bitmap->create(ويڪر, اوچائي, bitmap_state);
}

ساكن nserror nsico_create_ico_معلومات(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_بفر = guit->bitmap->get_بفر,
	};

	c->ico = صفر_يادگيري_ڏيو(1, حجم(ico_collection));
	إذا (c->ico == عدم) {
		content_نشر_error(&c->بنياد, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	أرجع NSERROR_OK;
}


ساكن nserror nsico_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	nsico_content *نتيجو;
	nserror error;

	نتيجو = صفر_يادگيري_ڏيو(1, حجم(nsico_content));
	إذا (نتيجو == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&نتيجو->بنياد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(نتيجو);
		أرجع error;
	}

	error = nsico_create_ico_معلومات(نتيجو);
	إذا (error != NSERROR_OK) {
		حرر(نتيجو);
		أرجع error;
	}

	*c = (بنية content *) نتيجو;

	أرجع NSERROR_OK;
}



ساكن bool nsico_convert(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;
	بنية bmp_image *bmp;
	bmp_نتيجو res;
	ثابت uint8_t *معلومات;
	size_t سائيز;
	حرف *title;

	/* set the ico data */
	معلومات = content_get_ذريعو_معلومات_2(c, &سائيز);

	/* analyse the ico */
	res = ico_analyse(ico->ico, سائيز, (بنانشان حرف *) معلومات);

	بدل (res) {
	حالة BMP_OK:
		اكسر;
	حالة BMP_INSUFFICIENT_يادگيري:
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	حالة BMP_INSUFFICIENT_معلومات:
	حالة BMP_معلومات_ERROR:
		content_نشر_error(c, NSERROR_ICO_ERROR, عدم);
		أرجع false;
	}

	/* Store our content width, height and calculate size */
	c->ويڪر = ico->ico->ويڪر;
	c->اوچائي = ico->ico->اوچائي;
	c->سائيز += (ico->ico->ويڪر * ico->ico->اوچائي * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_پن(llcache_handle_get_url(c->llcache)),
			c->ويڪر, c->اوچائي);
	إذا (title != عدم) {
		content_سيٽ_title(c, title);
		حرر(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_ڳوليو(ico->ico, 255, 255);
	إذا (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	content_سيٽ_ready(c);
	content_سيٽ_پورو(c);

	/* Done: update status bar */
	content_سيٽ_حالت(c, "");
	أرجع true;
}

ساكن bool nsico__decode(بنية bmp_image *ico)
{
	إذا (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		إذا (bmp_decode(ico) != BMP_OK) {
			أرجع false;
		}

		bitmap_format_منزل_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	أرجع true;
}

ساكن bool nsico_redraw(بنية content *c, بنية content_redraw_معلومات *معلومات,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	بنية bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_ڳوليو(ico->ico, معلومات->ويڪر, معلومات->اوچائي);
	إذا (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	/* ensure its decided */
	إذا (!nsico__decode(bmp)) {
		أرجع false;
	}

	أرجع image_bitmap_plot(bmp->bitmap, معلومات, clip, ctx);
}


ساكن فراغ nsico_destroy(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	حرر(ico->ico);
}

ساكن nserror nsico_clone(ثابت بنية content *old, بنية content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = صفر_يادگيري_ڏيو(1, حجم(nsico_content));
	إذا (ico == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &ico->بنياد);
	إذا (error != NSERROR_OK) {
		content_destroy(&ico->بنياد);
		أرجع error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_معلومات(ico);
	إذا (error != NSERROR_OK) {
		content_destroy(&ico->بنياد);
		أرجع error;
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
			old->حالت == CONTENT_حالت_پورو) {
		إذا (nsico_convert(&ico->بنياد) == false) {
			content_destroy(&ico->بنياد);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) ico;

	أرجع NSERROR_OK;
}

ساكن فراغ *nsico_get_اندروني(ثابت بنية content *c, فراغ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	بنية bmp_image *bmp;

	bmp = ico_ڳوليو(ico->ico, 16, 16);
	إذا (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع عدم;
	}

	إذا (!nsico__decode(bmp)) {
		أرجع عدم;
	}

	أرجع bmp->bitmap;
}

ساكن content_type nsico_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}

ساكن bool nsico_is_opaque(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;
	بنية bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_ڳوليو(ico->ico, 16, 16);
	إذا (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	إذا (!nsico__decode(bmp)) {
		أرجع false;
	}

	أرجع guit->bitmap->get_opaque(bmp->bitmap);
}

ساكن ثابت content_handler nsico_content_handler = {
	.create = nsico_create,
	.معلومات_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_اندروني = nsico_get_اندروني,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_رجسٽر_TYPES(nsico, nsico_types, nsico_content_handler);
