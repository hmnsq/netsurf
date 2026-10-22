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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <libnsbmp.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image.h"
#অন্তর্ভাব "image/ico.h"

প্রকারলেখ কাঠামো nsico_content {
	কাঠামো content base;

	কাঠামো ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
স্থির শূন্য *nsico_bitmap_create(পূর্ণসংখ্যা width, পূর্ণসংখ্যা height, unsigned পূর্ণসংখ্যা bmp_state)
{
	unsigned পূর্ণসংখ্যা bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ফেরত guit->bitmap->create(width, height, bitmap_state);
}

স্থির nserror nsico_create_ico_তথ্য(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_বাফার = guit->bitmap->get_বাফার,
	};

	c->ico = calloc(1, আকার(ico_collection));
	যদি (c->ico == ফাঁকা) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ফেরত NSERROR_OK;
}


স্থির nserror nsico_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks, কাঠামো content **c)
{
	nsico_content *ফলাফল;
	nserror error;

	ফলাফল = calloc(1, আকার(nsico_content));
	যদি (ফলাফল == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&ফলাফল->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(ফলাফল);
		ফেরত error;
	}

	error = nsico_create_ico_তথ্য(ফলাফল);
	যদি (error != NSERROR_OK) {
		মুক্ত(ফলাফল);
		ফেরত error;
	}

	*c = (কাঠামো content *) ফলাফল;

	ফেরত NSERROR_OK;
}



স্থির bool nsico_convert(কাঠামো content *c)
{
	nsico_content *ico = (nsico_content *) c;
	কাঠামো bmp_image *bmp;
	bmp_ফলাফল res;
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;
	অক্ষর *title;

	/* set the ico data */
	তথ্য = content_get_source_তথ্য_2(c, &আকার_2);

	/* analyse the ico */
	res = ico_analyse(ico->ico, আকার_2, (unsigned অক্ষর *) তথ্য);

	নির্বাচন (res) {
	ক্ষেত্র BMP_OK:
		বিরতি;
	ক্ষেত্র BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
	ক্ষেত্র BMP_INSUFFICIENT_তথ্য:
	ক্ষেত্র BMP_তথ্য_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, ফাঁকা);
		ফেরত false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->আকার_2 += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != ফাঁকা) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	যদি (bmp == ফাঁকা) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ফেরত false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ফেরত true;
}

স্থির bool nsico__decode(কাঠামো bmp_image *ico)
{
	যদি (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		যদি (bmp_decode(ico) != BMP_OK) {
			ফেরত false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ফেরত true;
}

স্থির bool nsico_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	কাঠামো bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, তথ্য->width, তথ্য->height);
	যদি (bmp == ফাঁকা) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ফেরত false;
	}

	/* ensure its decided */
	যদি (!nsico__decode(bmp)) {
		ফেরত false;
	}

	ফেরত image_bitmap_plot(bmp->bitmap, তথ্য, clip, ctx);
}


স্থির শূন্য nsico_destroy(কাঠামো content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	মুক্ত(ico->ico);
}

স্থির nserror nsico_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, আকার(nsico_content));
	যদি (ico == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ফেরত error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_তথ্য(ico);
	যদি (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ফেরত error;
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*newc = (কাঠামো content *) ico;

	ফেরত NSERROR_OK;
}

স্থির শূন্য *nsico_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	কাঠামো bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	যদি (bmp == ফাঁকা) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ফেরত ফাঁকা;
	}

	যদি (!nsico__decode(bmp)) {
		ফেরত ফাঁকা;
	}

	ফেরত bmp->bitmap;
}

স্থির content_type nsico_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}

স্থির bool nsico_is_opaque(কাঠামো content *c)
{
	nsico_content *ico = (nsico_content *) c;
	কাঠামো bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	যদি (bmp == ফাঁকা) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ফেরত false;
	}

	যদি (!nsico__decode(bmp)) {
		ফেরত false;
	}

	ফেরত guit->bitmap->get_opaque(bmp->bitmap);
}

স্থির ধ্রুবক content_handler nsico_content_handler = {
	.create = nsico_create,
	.তথ্য_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_internal = nsico_get_internal,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
