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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libnsbmp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/ico.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nsico_content {
	ਬਣਤਰ content ਅਧਾਰ;

	ਬਣਤਰ ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ਸਥਿਰ ਖਾਲੀ *nsico_bitmap_create(ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bmp_state)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_ਸਮ੍ਰਿਤੀ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ਵਾਪਸ guit->bitmap->create(ਚੌੜਾਈ, ਉਚਾਈ, bitmap_state);
}

ਸਥਿਰ nserror nsico_create_ico_ਅੰਕੜੇ(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_ਬਫਰ = guit->bitmap->get_ਬਫਰ,
	};

	c->ico = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ico_collection));
	ਜੇ (c->ico == ਨਿਰਾ) {
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ nserror nsico_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	nsico_content *ਨਤੀਜਾ;
	nserror error;

	ਨਤੀਜਾ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nsico_content));
	ਜੇ (ਨਤੀਜਾ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&ਨਤੀਜਾ->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	error = nsico_create_ico_ਅੰਕੜੇ(ਨਤੀਜਾ);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) ਨਤੀਜਾ;

	ਵਾਪਸ NSERROR_OK;
}



ਸਥਿਰ bool nsico_convert(ਬਣਤਰ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ਬਣਤਰ bmp_image *bmp;
	bmp_ਨਤੀਜਾ res;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;

	/* set the ico data */
	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	/* analyse the ico */
	res = ico_analyse(ico->ico, ਮਾਪ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਅੰਕੜੇ);

	ਚੋਣ (res) {
	ਹਾਲਤ BMP_OK:
		ਰੋਕੋ;
	ਹਾਲਤ BMP_INSUFFICIENT_ਸਮ੍ਰਿਤੀ:
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	ਹਾਲਤ BMP_INSUFFICIENT_ਅੰਕੜੇ:
	ਹਾਲਤ BMP_ਅੰਕੜੇ_ERROR:
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_ICO_ERROR, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	/* Store our content width, height and calculate size */
	c->ਚੌੜਾਈ = ico->ico->ਚੌੜਾਈ;
	c->ਉਚਾਈ = ico->ico->ਉਚਾਈ;
	c->ਮਾਪ += (ico->ico->ਚੌੜਾਈ * ico->ico->ਉਚਾਈ * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_ਲੱਭੋ(ico->ico, 255, 255);
	ਜੇ (bmp == ਨਿਰਾ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ਵਾਪਸ false;
	}

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);

	/* Done: update status bar */
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, "");
	ਵਾਪਸ true;
}

ਸਥਿਰ bool nsico__decode(ਬਣਤਰ bmp_image *ico)
{
	ਜੇ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ਜੇ (bmp_decode(ico) != BMP_OK) {
			ਵਾਪਸ false;
		}

		bitmap_format_ਮੰਜ਼ਿਲ_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ਵਾਪਸ true;
}

ਸਥਿਰ bool nsico_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ਬਣਤਰ bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_ਲੱਭੋ(ico->ico, ਅੰਕੜੇ->ਚੌੜਾਈ, ਅੰਕੜੇ->ਉਚਾਈ);
	ਜੇ (bmp == ਨਿਰਾ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ਵਾਪਸ false;
	}

	/* ensure its decided */
	ਜੇ (!nsico__decode(bmp)) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ image_bitmap_plot(bmp->bitmap, ਅੰਕੜੇ, clip, ctx);
}


ਸਥਿਰ ਖਾਲੀ nsico_destroy(ਬਣਤਰ content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	ਮੁਕਤ(ico->ico);
}

ਸਥਿਰ nserror nsico_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nsico_content));
	ਜੇ (ico == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &ico->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&ico->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_ਅੰਕੜੇ(ico);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&ico->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (nsico_convert(&ico->ਅਧਾਰ) == false) {
			content_destroy(&ico->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) ico;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ *nsico_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ਬਣਤਰ bmp_image *bmp;

	bmp = ico_ਲੱਭੋ(ico->ico, 16, 16);
	ਜੇ (bmp == ਨਿਰਾ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (!nsico__decode(bmp)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ bmp->bitmap;
}

ਸਥਿਰ content_type nsico_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}

ਸਥਿਰ bool nsico_is_opaque(ਬਣਤਰ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ਬਣਤਰ bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_ਲੱਭੋ(ico->ico, 16, 16);
	ਜੇ (bmp == ਨਿਰਾ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ਵਾਪਸ false;
	}

	ਜੇ (!nsico__decode(bmp)) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ guit->bitmap->get_opaque(bmp->bitmap);
}

ਸਥਿਰ ਅਚਲ content_handler nsico_content_handler = {
	.create = nsico_create,
	.ਅੰਕੜੇ_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_ਅੰਦਰੂਨੀ = nsico_get_ਅੰਦਰੂਨੀ,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsico, nsico_types, nsico_content_handler);
