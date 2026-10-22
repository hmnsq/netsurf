/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Sean Fox <dyntryx@gmail.com>
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
 * implementation of content handler for BMP images.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libnsbmp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/bmp.h"

/** bmp context. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nsbmp_content {
	ਬਣਤਰ content ਅਧਾਰ;

	bmp_image *bmp;	/** BMP image data */

	ਬਣਤਰ bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ਸਥਿਰ ਖਾਲੀ *nsbmp_bitmap_create(ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bmp_state)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_ਸਮ੍ਰਿਤੀ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ਵਾਪਸ guit->bitmap->create(ਚੌੜਾਈ, ਉਚਾਈ, bitmap_state);
}

ਸਥਿਰ nserror nsbmp_create_bmp_ਅੰਕੜੇ(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_ਬਫਰ = guit->bitmap->get_ਬਫਰ,
	};

	bmp->bmp = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ bmp_image));
	ਜੇ (bmp->bmp == ਨਿਰਾ) {
		content_ਪ੍ਰਸਾਰਣ_error(&bmp->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror
nsbmp_create(ਅਚਲ ਬਣਤਰ content_handler *handler,
	     lwc_string *imime_type,
	     ਅਚਲ ਬਣਤਰ http_parameter *params,
	     llcache_handle *llcache,
	     ਅਚਲ ਅੱਖਰ *fallback_charset,
	     bool quirks,
	     ਬਣਤਰ content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nsbmp_content));
	ਜੇ (bmp == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&bmp->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(bmp);
		ਵਾਪਸ error;
	}

	error = nsbmp_create_bmp_ਅੰਕੜੇ(bmp);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(bmp);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) bmp;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ bool nsbmp_convert(ਬਣਤਰ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_ਨਤੀਜਾ res;
	uint32_t swidth;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;

	/* set the bmp data */
	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, ਮਾਪ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਅੰਕੜੇ);
	ਚੋਣ (res) {
		ਹਾਲਤ BMP_OK:
			ਰੋਕੋ;
		ਹਾਲਤ BMP_INSUFFICIENT_ਸਮ੍ਰਿਤੀ:
			content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
			ਵਾਪਸ false;
		ਹਾਲਤ BMP_INSUFFICIENT_ਅੰਕੜੇ:
		ਹਾਲਤ BMP_ਅੰਕੜੇ_ERROR:
			content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_BMP_ERROR, ਨਿਰਾ);
			ਵਾਪਸ false;
	}

	/* Store our content width and description */
	c->ਚੌੜਾਈ = bmp->bmp->ਚੌੜਾਈ;
	c->ਉਚਾਈ = bmp->bmp->ਉਚਾਈ;
	swidth = ਆਕਾਰ(uint32_t) * bmp->bmp->ਚੌੜਾਈ;
	c->ਮਾਪ += (swidth * bmp->bmp->ਉਚਾਈ) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);

	/* Done: update status bar */
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, "");
	ਵਾਪਸ true;
}

ਸਥਿਰ bool nsbmp_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ਜੇ (bmp->bmp->decoded == false) {
		bmp_ਨਤੀਜਾ res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ਜੇ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_ਅੰਕੜੇ) &&
		    (res != BMP_ਅੰਕੜੇ_ERROR)) {
			ਵਾਪਸ false;
		}

		bitmap_format_ਮੰਜ਼ਿਲ_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ਜੇ (ਅੰਕੜੇ->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ਜੇ (ਅੰਕੜੇ->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ਵਾਪਸ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  ਅੰਕੜੇ->x, ਅੰਕੜੇ->y,
				  ਅੰਕੜੇ->ਚੌੜਾਈ, ਅੰਕੜੇ->ਉਚਾਈ,
				  ਅੰਕੜੇ->background_ਰੰਗ,
				  flags) == NSERROR_OK);
}


ਸਥਿਰ ਖਾਲੀ nsbmp_destroy(ਬਣਤਰ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	ਮੁਕਤ(bmp->bmp);
}


ਸਥਿਰ nserror nsbmp_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nsbmp_content));
	ਜੇ (new_bmp == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&new_bmp->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_ਅੰਕੜੇ(new_bmp);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&new_bmp->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY || 
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (nsbmp_convert(&new_bmp->ਅਧਾਰ) == false) {
			content_destroy(&new_bmp->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) new_bmp;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ *nsbmp_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ਵਾਪਸ bmp->bitmap;
}

ਸਥਿਰ content_type nsbmp_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}

ਸਥਿਰ bool nsbmp_content_is_opaque(ਬਣਤਰ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ਜੇ (bmp->bitmap != ਨਿਰਾ) {
		ਵਾਪਸ guit->bitmap->get_opaque(bmp->bitmap);
	}

	ਵਾਪਸ false;
}

ਸਥਿਰ ਅਚਲ content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.ਅੰਕੜੇ_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_ਅੰਦਰੂਨੀ = nsbmp_get_ਅੰਦਰੂਨੀ,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nsbmp_types[] = {
	"application/bmp",
	"application/preview",
	"application/x-bmp",
	"application/x-win-bitmap",
	"image/bmp",
	"image/ms-bmp",
	"image/x-bitmap",
	"image/x-bmp",
	"image/x-ms-bmp",
	"image/x-win-bitmap",
	"image/x-windows-bmp",
	"image/x-xbitmap"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
