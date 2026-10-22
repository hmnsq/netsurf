/*
 * Copyright 2011 Vincent Sanders <vince@netsurf-browser.org>
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
 * Cache implementation for bitmap images decoded into frontend format.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image_cache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cache_age;

/**
 * Image cache entry
 */
ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s {
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *ਅਗਲਾ; /**< next cache entry in list */
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ਬਣਤਰ content *content;
	/** associated bitmap entry */
	ਬਣਤਰ bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ redraw_ਗਿਣਤੀ; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_ਮਾਪ; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	ਪੂਰਨ conversion_ਗਿਣਤੀ; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ਬਣਤਰ image_cache_s {
	/** Cache parameters */
	ਬਣਤਰ image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t ਕੁੱਲ_bitmap_ਮਾਪ;

	/** Total count of bitmaps currently allocated */
	ਪੂਰਨ bitmap_ਗਿਣਤੀ;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_ਮਾਪ;
	/** The number of objects when maximum bitmap usage occurred */
	ਪੂਰਨ max_bitmap_ਮਾਪ_ਗਿਣਤੀ;

	/** Maximum count of bitmaps allocated at any one time */
	ਪੂਰਨ max_bitmap_ਗਿਣਤੀ;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_ਗਿਣਤੀ_ਮਾਪ;

	/** Bitmap was not available at plot time required conversion */
	ਪੂਰਨ ਖੁੰਝ_ਗਿਣਤੀ;
	uint64_t ਖੁੰਝ_ਮਾਪ;
	/** Bitmap was available at plot time required no conversion */
	ਪੂਰਨ ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ;
	uint64_t ਪ੍ਰਾਪਤੀ_ਮਾਪ;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	ਪੂਰਨ fail_ਗਿਣਤੀ;
	uint64_t fail_ਮਾਪ;

	/* Cache entry freed without ever being redrawn */
	ਪੂਰਨ ਕੁੱਲ_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	ਪੂਰਨ specultive_ਖੁੰਝ_ਗਿਣਤੀ;

	/** Total number of additional (after the first) conversions */
	ਪੂਰਨ ਕੁੱਲ_extra_conversions;
	/** counts total number of images with more than one conversion */
	ਪੂਰਨ ਕੁੱਲ_extra_conversions_ਗਿਣਤੀ;

	/** Bitmap with most conversions was converted this many times */
	ਪੂਰਨ peak_conversions;
	/** Size of bitmap with most conversions */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ peak_conversions_ਮਾਪ;
};

/** image cache state */
ਸਥਿਰ ਬਣਤਰ image_cache_s *image_cache = ਨਿਰਾ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ਸਥਿਰ ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *image_cache__findn(ਪੂਰਨ entryn)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *ਮਿਲਿਆ;

	ਮਿਲਿਆ = image_cache->entries;
	ਜਦੋਂ ((ਮਿਲਿਆ != ਨਿਰਾ) && (entryn > 0)) {
		entryn--;
		ਮਿਲਿਆ = ਮਿਲਿਆ->ਅਗਲਾ;
	}
	ਵਾਪਸ ਮਿਲਿਆ;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ਸਥਿਰ ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *image_cache_ਲੱਭੋ(ਅਚਲ ਬਣਤਰ content *c)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *ਮਿਲਿਆ;

	ਮਿਲਿਆ = image_cache->entries;
	ਜਦੋਂ ((ਮਿਲਿਆ != ਨਿਰਾ) && (ਮਿਲਿਆ->content != c)) {
		ਮਿਲਿਆ = ਮਿਲਿਆ->ਅਗਲਾ;
	}
	ਵਾਪਸ ਮਿਲਿਆ;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ਸਥਿਰ ਖਾਲੀ image_cache_stats_bitmap_ਜੋੜੋ(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_ਗਿਣਤੀ++;

	image_cache->ਕੁੱਲ_bitmap_ਮਾਪ += centry->bitmap_ਮਾਪ;
	image_cache->bitmap_ਗਿਣਤੀ++;

	ਜੇ (image_cache->ਕੁੱਲ_bitmap_ਮਾਪ > image_cache->max_bitmap_ਮਾਪ) {
		image_cache->max_bitmap_ਮਾਪ = image_cache->ਕੁੱਲ_bitmap_ਮਾਪ;
		image_cache->max_bitmap_ਮਾਪ_ਗਿਣਤੀ = image_cache->bitmap_ਗਿਣਤੀ;

	}

	ਜੇ (image_cache->bitmap_ਗਿਣਤੀ > image_cache->max_bitmap_ਗਿਣਤੀ) {
		image_cache->max_bitmap_ਗਿਣਤੀ = image_cache->bitmap_ਗਿਣਤੀ;
		image_cache->max_bitmap_ਗਿਣਤੀ_ਮਾਪ = image_cache->ਕੁੱਲ_bitmap_ਮਾਪ;
	}

	ਜੇ (centry->conversion_ਗਿਣਤੀ == 2) {
		image_cache->ਕੁੱਲ_extra_conversions_ਗਿਣਤੀ++;
	}

	ਜੇ (centry->conversion_ਗਿਣਤੀ > 1) {
		image_cache->ਕੁੱਲ_extra_conversions++;
	}

	ਜੇ ((centry->conversion_ਗਿਣਤੀ > image_cache->peak_conversions) ||
	    (centry->conversion_ਗਿਣਤੀ == image_cache->peak_conversions &&
	     centry->bitmap_ਮਾਪ > image_cache->peak_conversions_ਮਾਪ)) {
		image_cache->peak_conversions = centry->conversion_ਗਿਣਤੀ;
		image_cache->peak_conversions_ਮਾਪ = centry->bitmap_ਮਾਪ;
	}
}

ਸਥਿਰ ਖਾਲੀ image_cache__link(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry)
{
	centry->ਅਗਲਾ = image_cache->entries;
	centry->prev = ਨਿਰਾ;
	ਜੇ (centry->ਅਗਲਾ != ਨਿਰਾ) {
		centry->ਅਗਲਾ->prev = centry;
	}
	image_cache->entries = centry;
}

ਸਥਿਰ ਖਾਲੀ image_cache__unlink(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry)
{
	/* unlink entry */
	ਜੇ (centry->prev == ਨਿਰਾ) {
		/* first in list */
		ਜੇ (centry->ਅਗਲਾ != ਨਿਰਾ) {
			centry->ਅਗਲਾ->prev = centry->prev;
			image_cache->entries = centry->ਅਗਲਾ;
		} ਨਹੀਂਤਾਂ {
			/* empty list */
			image_cache->entries = ਨਿਰਾ;
		}
	} ਨਹੀਂਤਾਂ {
		centry->prev->ਅਗਲਾ = centry->ਅਗਲਾ;

		ਜੇ (centry->ਅਗਲਾ != ਨਿਰਾ) {
			centry->ਅਗਲਾ->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ਸਥਿਰ ਖਾਲੀ image_cache__free_bitmap(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry)
{
	ਜੇ (centry->bitmap != ਨਿਰਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_ਮਾਪ,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_ਗਿਣਤੀ);
#ਜੇ_ਅੰਤ
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ਨਿਰਾ;
		image_cache->ਕੁੱਲ_bitmap_ਮਾਪ -= centry->bitmap_ਮਾਪ;
		image_cache->bitmap_ਗਿਣਤੀ--;
		ਜੇ (centry->redraw_ਗਿਣਤੀ == 0) {
			image_cache->specultive_ਖੁੰਝ_ਗਿਣਤੀ++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ਸਥਿਰ ਖਾਲੀ image_cache_free_ਇੰਦਰਾਜ(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#ਜੇ_ਅੰਤ

	ਜੇ (centry->redraw_ਗਿਣਤੀ == 0) {
		image_cache->ਕੁੱਲ_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	ਮੁਕਤ(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ਸਥਿਰ ਖਾਲੀ image_cache__clean(ਬਣਤਰ image_cache_s *icache)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry = icache->entries;

	ਜਦੋਂ (centry != ਨਿਰਾ) {
		ਜੇ ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_ਸਮਾਂ) {
			/* only consider older entries, avoids active entries */
			ਜੇ ((icache->ਕੁੱਲ_bitmap_ਮਾਪ >
			     (icache->params.ਹੱਦਬੰਦੀ - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->ਅਗਲਾ;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
ਸਥਿਰ ਖਾਲੀ image_cache__background_update(ਖਾਲੀ *p)
{
	ਬਣਤਰ image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_ਸਮਾਂ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#ਜੇ_ਅੰਤ

	image_cache__clean(icache);

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(icache->params.bg_clean_ਸਮਾਂ,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
ਬਣਤਰ bitmap *image_cache_get_bitmap(ਅਚਲ ਬਣਤਰ content *c)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	centry = image_cache_ਲੱਭੋ(c);
	ਜੇ (centry == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (centry->bitmap == ਨਿਰਾ) {
		ਜੇ (centry->convert != ਨਿਰਾ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ਜੇ (centry->bitmap != ਨਿਰਾ) {
			image_cache_stats_bitmap_ਜੋੜੋ(centry);
			image_cache->ਖੁੰਝ_ਗਿਣਤੀ++;
			image_cache->ਖੁੰਝ_ਮਾਪ += centry->bitmap_ਮਾਪ;
		} ਨਹੀਂਤਾਂ {
			image_cache->fail_ਗਿਣਤੀ++;
			image_cache->fail_ਮਾਪ += centry->bitmap_ਮਾਪ;
		}
	} ਨਹੀਂਤਾਂ {
		image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ++;
		image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ += centry->bitmap_ਮਾਪ;
	}

	ਵਾਪਸ centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ਬਣਤਰ content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	ਜੇ ((image_cache->ਕੁੱਲ_bitmap_ਮਾਪ < image_cache->params.ਹੱਦਬੰਦੀ) &&
	    (c->ਮਾਪ <= image_cache->params.speculative_small)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->ਮਾਪ,
		      SPECULATE_SMALL);
#ਜੇ_ਅੰਤ
		decision = true;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ decision;
}

/* exported interface documented in image_cache.h */
ਬਣਤਰ bitmap *image_cache_ਲੱਭੋ_bitmap(ਬਣਤਰ content *c)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	centry = image_cache_ਲੱਭੋ(c);
	ਜੇ (centry == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ਅਚਲ ਬਣਤਰ image_cache_parameters *image_cache_parameters)
{
	image_cache = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ image_cache_s));
	ਜੇ (image_cache == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(image_cache->params.bg_clean_ਸਮਾਂ,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.ਹੱਦਬੰਦੀ,
	      image_cache->params.hysteresis);

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ op_ਗਿਣਤੀ;
	uint64_t op_ਮਾਪ;

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->ਕੁੱਲ_bitmap_ਮਾਪ, image_cache->bitmap_ਗਿਣਤੀ);

	ਜਦੋਂ (image_cache->entries != ਨਿਰਾ) {
		image_cache_free_ਇੰਦਰਾਜ(image_cache->entries);
	}

	op_ਗਿਣਤੀ = image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ +
		image_cache->ਖੁੰਝ_ਗਿਣਤੀ +
		image_cache->fail_ਗਿਣਤੀ;

	op_ਮਾਪ = image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ +
		image_cache->ਖੁੰਝ_ਮਾਪ +
		image_cache->fail_ਮਾਪ;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_ਮਾਪ,
	      image_cache->max_bitmap_ਮਾਪ_ਗਿਣਤੀ);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_ਗਿਣਤੀ,
	      image_cache->max_bitmap_ਗਿਣਤੀ_ਮਾਪ);

	ਜੇ ((op_ਗਿਣਤੀ > 0) && (op_ਮਾਪ >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_ਗਿਣਤੀ,
		      image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ,
		      image_cache->ਖੁੰਝ_ਗਿਣਤੀ,
		      image_cache->fail_ਗਿਣਤੀ,
		      (image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ * 100) / op_ਗਿਣਤੀ,
		      (image_cache->ਖੁੰਝ_ਗਿਣਤੀ * 100) / op_ਗਿਣਤੀ,
		      (image_cache->fail_ਗਿਣਤੀ * 100) / op_ਗਿਣਤੀ);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_ਮਾਪ,
		      image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ,
		      image_cache->ਖੁੰਝ_ਮਾਪ,
		      image_cache->fail_ਮਾਪ,
		      (image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ * 100) / op_ਮਾਪ,
		      (image_cache->ਖੁੰਝ_ਮਾਪ * 100) / op_ਮਾਪ,
		      (image_cache->fail_ਮਾਪ * 100) / op_ਮਾਪ);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->ਕੁੱਲ_unrendered,
	      image_cache->specultive_ਖੁੰਝ_ਗਿਣਤੀ);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->ਕੁੱਲ_extra_conversions,
	      image_cache->ਕੁੱਲ_extra_conversions_ਗਿਣਤੀ);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_ਮਾਪ,
	      image_cache->peak_conversions);

	ਮੁਕਤ(image_cache);

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_ਜੋੜੋ(ਬਣਤਰ content *content,
			ਬਣਤਰ bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache_ਲੱਭੋ(content);
	ਜੇ (centry == ਨਿਰਾ) {
		/* new cache entry, content not previously added */
		centry = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s));
		ਜੇ (centry == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_ਮਾਪ = content->ਚੌੜਾਈ * content->ਉਚਾਈ * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	ਜੇ (bitmap != ਨਿਰਾ) {
		ਜੇ (centry->bitmap != ਨਿਰਾ) {
			guit->bitmap->destroy(centry->bitmap);
		} ਨਹੀਂਤਾਂ {
			image_cache_stats_bitmap_ਜੋੜੋ(centry);
		}
		centry->bitmap = bitmap;
	} ਨਹੀਂਤਾਂ {
		/* no bitmap, check to see if we should speculatively convert */
		ਜੇ ((centry->convert != ਨਿਰਾ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			ਜੇ (centry->bitmap != ਨਿਰਾ) {
				image_cache_stats_bitmap_ਜੋੜੋ(centry);
			} ਨਹੀਂਤਾਂ {
				image_cache->fail_ਗਿਣਤੀ++;
			}
		}
	}



	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ਬਣਤਰ content *content)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	/* get the cache entry */
	centry = image_cache_ਲੱਭੋ(content);
	ਜੇ (centry == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	image_cache_free_ਇੰਦਰਾਜ(centry);

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
ਪੂਰਨ image_cache_snsummaryf(ਅੱਖਰ *string, size_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *fmt)
{
	size_t slen = 0; /* current output string length */
	ਪੂਰਨ fmtc = 0; /* current index into format string */
	bool pct;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ op_ਗਿਣਤੀ;
	uint64_t op_ਮਾਪ;

	op_ਗਿਣਤੀ = image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ +
		image_cache->ਖੁੰਝ_ਗਿਣਤੀ +
		image_cache->fail_ਗਿਣਤੀ;

	op_ਮਾਪ = image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ +
		image_cache->ਖੁੰਝ_ਮਾਪ +
		image_cache->fail_ਮਾਪ;

	ਜਦੋਂ((slen < ਮਾਪ) && (fmt[fmtc] != 0)) {
		ਜੇ (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			ਜੇ (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} ਨਹੀਂਤਾਂ {
				pct = false;
			}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMTCHR(chr,fmt,var) ਹਾਲਤ chr : \
slen += snprintf(string + slen, ਮਾਪ - slen, "%"fmt, image_cache->var); ਰੋਕੋ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMTPCHR(chr,fmt,var,div) \
ਹਾਲਤ chr :					\
	ਜੇ (pct) {							\
		ਜੇ (div > 0) {						\
			slen += snprintf(string + slen, ਮਾਪ - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} ਨਹੀਂਤਾਂ {						\
			slen += snprintf(string + slen, ਮਾਪ - slen, "100"); \
		}							\
	} ਨਹੀਂਤਾਂ {							\
		slen += snprintf(string + slen, ਮਾਪ - slen, "%"fmt, image_cache->var); \
	} ਰੋਕੋ


			ਚੋਣ (fmt[fmtc]) {
			ਹਾਲਤ '%':
				string[slen] = '%';
				slen++;
				ਰੋਕੋ;

			FMTCHR('a', PRIsizet, params.ਹੱਦਬੰਦੀ);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, ਕੁੱਲ_bitmap_ਮਾਪ);
			FMTCHR('d', "d", bitmap_ਗਿਣਤੀ);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_ਮਾਪ);
			FMTCHR('g', "d", max_bitmap_ਮਾਪ_ਗਿਣਤੀ);
			FMTCHR('h', "d", max_bitmap_ਗਿਣਤੀ);
			FMTCHR('i', PRIsizet, max_bitmap_ਗਿਣਤੀ_ਮਾਪ);


			ਹਾਲਤ 'j':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						 "%u", pct?100:op_ਗਿਣਤੀ);
				ਰੋਕੋ;

			FMTPCHR('k', "d", ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ, op_ਗਿਣਤੀ);
			FMTPCHR('l', "d", ਖੁੰਝ_ਗਿਣਤੀ, op_ਗਿਣਤੀ);
			FMTPCHR('m', "d", fail_ਗਿਣਤੀ, op_ਗਿਣਤੀ);

			ਹਾਲਤ 'n':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						 "%"PRId64, pct?100:op_ਮਾਪ);
				ਰੋਕੋ;

			FMTPCHR('o', PRId64, ਪ੍ਰਾਪਤੀ_ਮਾਪ, op_ਮਾਪ);
			FMTPCHR('q', PRId64, ਖੁੰਝ_ਮਾਪ, op_ਮਾਪ);
			FMTPCHR('r', PRId64, fail_ਮਾਪ, op_ਮਾਪ);

			FMTCHR('s', "d", ਕੁੱਲ_unrendered);
			FMTCHR('t', "d", specultive_ਖੁੰਝ_ਗਿਣਤੀ);
			FMTCHR('u', "d", ਕੁੱਲ_extra_conversions);
			FMTCHR('v', "d", ਕੁੱਲ_extra_conversions_ਗਿਣਤੀ);
			FMTCHR('w', "u", peak_conversions_ਮਾਪ);
			FMTCHR('x', "d", peak_conversions);


			}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ FMTCHR
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ FMTPCHR

			fmtc++;
		} ਨਹੀਂਤਾਂ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, ਮਾਪ - 1)] = '\0';

	ਵਾਪਸ slen;
}

/* exported interface documented in image_cache.h */
ਪੂਰਨ
image_cache_snentryf(ਅੱਖਰ *string,
		     size_t ਮਾਪ,
		     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ entryn,
		     ਅਚਲ ਅੱਖਰ *fmt)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;
	size_t slen = 0; /* current output string length */
	ਪੂਰਨ fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	ਜੇ (centry == ਨਿਰਾ)
		ਵਾਪਸ -1;

	ਜਦੋਂ((slen < ਮਾਪ) && (fmt[fmtc] != 0)) {
		ਜੇ (fmt[fmtc] == '%') {
			fmtc++;
			ਚੋਣ (fmt[fmtc]) {
			ਹਾਲਤ 'e':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%u", entryn);
				ਰੋਕੋ;

			ਹਾਲਤ 'r':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%u", centry->redraw_ਗਿਣਤੀ);
				ਰੋਕੋ;

			ਹਾਲਤ 'a':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						 "%.2f", (ਦਸ਼ਮਲਵ)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ਰੋਕੋ;


			ਹਾਲਤ 'c':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%d", centry->conversion_ਗਿਣਤੀ);
				ਰੋਕੋ;

			ਹਾਲਤ 'g':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%.2f", (ਦਸ਼ਮਲਵ)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ਰੋਕੋ;

			ਹਾਲਤ 'k':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%p", centry->content);
				ਰੋਕੋ;

			ਹਾਲਤ 'U':
				slen += snprintf(string + slen, ਮਾਪ - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ਰੋਕੋ;

			ਹਾਲਤ 'o':
				ਜੇ (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							ਮਾਪ - slen, "%s",
							lwc_string_ਅੰਕੜੇ(
								origin));

					lwc_string_unref(origin);
				} ਨਹੀਂਤਾਂ {
					slen += snprintf(string + slen,
							ਮਾਪ - slen, "%s",
							"localhost");
				}
				ਰੋਕੋ;

			ਹਾਲਤ 's':
				ਜੇ (centry->bitmap != ਨਿਰਾ) {
					slen += snprintf(string + slen,
							 ਮਾਪ - slen,
							 "%" PRIsizet,
							 centry->bitmap_ਮਾਪ);
				} ਨਹੀਂਤਾਂ {
					slen += snprintf(string + slen,
							 ਮਾਪ - slen,
							 "0");
				}
				ਰੋਕੋ;
			}
			fmtc++;
		} ਨਹੀਂਤਾਂ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, ਮਾਪ - 1)] = '\0';

	ਵਾਪਸ slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ਬਣਤਰ content *c,
			ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
			ਅਚਲ ਬਣਤਰ rect *clip,
			ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	/* get the cache entry */
	centry = image_cache_ਲੱਭੋ(c);
	ਜੇ (centry == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ਵਾਪਸ false;
	}

	ਜੇ (centry->bitmap == ਨਿਰਾ) {
		ਜੇ (centry->convert != ਨਿਰਾ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ਜੇ (centry->bitmap != ਨਿਰਾ) {
			image_cache_stats_bitmap_ਜੋੜੋ(centry);
			image_cache->ਖੁੰਝ_ਗਿਣਤੀ++;
			image_cache->ਖੁੰਝ_ਮਾਪ += centry->bitmap_ਮਾਪ;
		} ਨਹੀਂਤਾਂ {
			image_cache->fail_ਗਿਣਤੀ++;
			image_cache->fail_ਮਾਪ += centry->bitmap_ਮਾਪ;
			ਵਾਪਸ false;
		}
	} ਨਹੀਂਤਾਂ {
		image_cache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ++;
		image_cache->ਪ੍ਰਾਪਤੀ_ਮਾਪ += centry->bitmap_ਮਾਪ;
	}


	/* update statistics */
	centry->redraw_ਗਿਣਤੀ++;
	centry->redraw_age = image_cache->current_age;

	ਵਾਪਸ image_bitmap_plot(centry->bitmap, ਅੰਕੜੇ, clip, ctx);
}

/* exported interface documented in image_cache.h */
ਖਾਲੀ image_cache_destroy(ਬਣਤਰ content *content)
{
	ਬਣਤਰ image_cache_ਇੰਦਰਾਜ_s *centry;

	/* get the cache entry */
	centry = image_cache_ਲੱਭੋ(content);
	ਜੇ (centry == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} ਨਹੀਂਤਾਂ {
		image_cache_free_ਇੰਦਰਾਜ(centry);
	}
}

/* exported interface documented in image_cache.h */
ਖਾਲੀ *image_cache_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	ਵਾਪਸ image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ਬਣਤਰ content *c)
{
	ਬਣਤਰ bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	ਜੇ (bmp != ਨਿਰਾ) {
		ਵਾਪਸ guit->bitmap->get_opaque(bmp);
	}
	ਵਾਪਸ false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}
