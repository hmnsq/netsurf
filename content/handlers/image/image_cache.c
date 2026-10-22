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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"
#ಅನ್ತರ್ಭಾವ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cache_age;

/**
 * Image cache entry
 */
ರಚನೆ image_cache_ನಮೂದು_s {
	ರಚನೆ image_cache_ನಮೂದು_s *ಮುಂದಿನ; /**< next cache entry in list */
	ರಚನೆ image_cache_ನಮೂದು_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ರಚನೆ content *content;
	/** associated bitmap entry */
	ರಚನೆ bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ redraw_ಎಣಿಕೆ; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_ಗಾತ್ರ; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	ಪೂರ್ಣಾಂಕ conversion_ಎಣಿಕೆ; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ರಚನೆ image_cache_s {
	/** Cache parameters */
	ರಚನೆ image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ರಚನೆ image_cache_ನಮೂದು_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t ಒಟ್ಟು_bitmap_ಗಾತ್ರ;

	/** Total count of bitmaps currently allocated */
	ಪೂರ್ಣಾಂಕ bitmap_ಎಣಿಕೆ;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_ಗಾತ್ರ;
	/** The number of objects when maximum bitmap usage occurred */
	ಪೂರ್ಣಾಂಕ max_bitmap_ಗಾತ್ರ_ಎಣಿಕೆ;

	/** Maximum count of bitmaps allocated at any one time */
	ಪೂರ್ಣಾಂಕ max_bitmap_ಎಣಿಕೆ;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_ಎಣಿಕೆ_ಗಾತ್ರ;

	/** Bitmap was not available at plot time required conversion */
	ಪೂರ್ಣಾಂಕ ತಪ್ಪಿದ_ಎಣಿಕೆ;
	uint64_t ತಪ್ಪಿದ_ಗಾತ್ರ;
	/** Bitmap was available at plot time required no conversion */
	ಪೂರ್ಣಾಂಕ ದೊರಕಿತು_ಎಣಿಕೆ;
	uint64_t ದೊರಕಿತು_ಗಾತ್ರ;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	ಪೂರ್ಣಾಂಕ fail_ಎಣಿಕೆ;
	uint64_t fail_ಗಾತ್ರ;

	/* Cache entry freed without ever being redrawn */
	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	ಪೂರ್ಣಾಂಕ specultive_ತಪ್ಪಿದ_ಎಣಿಕೆ;

	/** Total number of additional (after the first) conversions */
	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_extra_conversions;
	/** counts total number of images with more than one conversion */
	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_extra_conversions_ಎಣಿಕೆ;

	/** Bitmap with most conversions was converted this many times */
	ಪೂರ್ಣಾಂಕ peak_conversions;
	/** Size of bitmap with most conversions */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ peak_conversions_ಗಾತ್ರ;
};

/** image cache state */
ಸ್ಥಿರ ರಚನೆ image_cache_s *image_cache = ಖಾಲಿ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ಸ್ಥಿರ ರಚನೆ image_cache_ನಮೂದು_s *image_cache__findn(ಪೂರ್ಣಾಂಕ entryn)
{
	ರಚನೆ image_cache_ನಮೂದು_s *ಸಿಕ್ಕಿತು;

	ಸಿಕ್ಕಿತು = image_cache->entries;
	ಆಗ ((ಸಿಕ್ಕಿತು != ಖಾಲಿ) && (entryn > 0)) {
		entryn--;
		ಸಿಕ್ಕಿತು = ಸಿಕ್ಕಿತು->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ಸ್ಥಿರ ರಚನೆ image_cache_ನಮೂದು_s *image_cache_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c)
{
	ರಚನೆ image_cache_ನಮೂದು_s *ಸಿಕ್ಕಿತು;

	ಸಿಕ್ಕಿತು = image_cache->entries;
	ಆಗ ((ಸಿಕ್ಕಿತು != ಖಾಲಿ) && (ಸಿಕ್ಕಿತು->content != c)) {
		ಸಿಕ್ಕಿತು = ಸಿಕ್ಕಿತು->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache_stats_bitmap_ಕೂಡಿಸು(ರಚನೆ image_cache_ನಮೂದು_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_ಎಣಿಕೆ++;

	image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
	image_cache->bitmap_ಎಣಿಕೆ++;

	ಆದರೆ (image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ > image_cache->max_bitmap_ಗಾತ್ರ) {
		image_cache->max_bitmap_ಗಾತ್ರ = image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ;
		image_cache->max_bitmap_ಗಾತ್ರ_ಎಣಿಕೆ = image_cache->bitmap_ಎಣಿಕೆ;

	}

	ಆದರೆ (image_cache->bitmap_ಎಣಿಕೆ > image_cache->max_bitmap_ಎಣಿಕೆ) {
		image_cache->max_bitmap_ಎಣಿಕೆ = image_cache->bitmap_ಎಣಿಕೆ;
		image_cache->max_bitmap_ಎಣಿಕೆ_ಗಾತ್ರ = image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ;
	}

	ಆದರೆ (centry->conversion_ಎಣಿಕೆ == 2) {
		image_cache->ಒಟ್ಟು_extra_conversions_ಎಣಿಕೆ++;
	}

	ಆದರೆ (centry->conversion_ಎಣಿಕೆ > 1) {
		image_cache->ಒಟ್ಟು_extra_conversions++;
	}

	ಆದರೆ ((centry->conversion_ಎಣಿಕೆ > image_cache->peak_conversions) ||
	    (centry->conversion_ಎಣಿಕೆ == image_cache->peak_conversions &&
	     centry->bitmap_ಗಾತ್ರ > image_cache->peak_conversions_ಗಾತ್ರ)) {
		image_cache->peak_conversions = centry->conversion_ಎಣಿಕೆ;
		image_cache->peak_conversions_ಗಾತ್ರ = centry->bitmap_ಗಾತ್ರ;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ image_cache__link(ರಚನೆ image_cache_ನಮೂದು_s *centry)
{
	centry->ಮುಂದಿನ = image_cache->entries;
	centry->prev = ಖಾಲಿ;
	ಆದರೆ (centry->ಮುಂದಿನ != ಖಾಲಿ) {
		centry->ಮುಂದಿನ->prev = centry;
	}
	image_cache->entries = centry;
}

ಸ್ಥಿರ ಶೂನ್ಯ image_cache__unlink(ರಚನೆ image_cache_ನಮೂದು_s *centry)
{
	/* unlink entry */
	ಆದರೆ (centry->prev == ಖಾಲಿ) {
		/* first in list */
		ಆದರೆ (centry->ಮುಂದಿನ != ಖಾಲಿ) {
			centry->ಮುಂದಿನ->prev = centry->prev;
			image_cache->entries = centry->ಮುಂದಿನ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* empty list */
			image_cache->entries = ಖಾಲಿ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		centry->prev->ಮುಂದಿನ = centry->ಮುಂದಿನ;

		ಆದರೆ (centry->ಮುಂದಿನ != ಖಾಲಿ) {
			centry->ಮುಂದಿನ->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__free_bitmap(ರಚನೆ image_cache_ನಮೂದು_s *centry)
{
	ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_ಗಾತ್ರ,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_ಎಣಿಕೆ);
#ಆದರೆ_ಅಂತ್ಯ
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ಖಾಲಿ;
		image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ -= centry->bitmap_ಗಾತ್ರ;
		image_cache->bitmap_ಎಣಿಕೆ--;
		ಆದರೆ (centry->redraw_ಎಣಿಕೆ == 0) {
			image_cache->specultive_ತಪ್ಪಿದ_ಎಣಿಕೆ++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache_free_ನಮೂದು(ರಚನೆ image_cache_ನಮೂದು_s *centry)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (centry->redraw_ಎಣಿಕೆ == 0) {
		image_cache->ಒಟ್ಟು_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	ಬಿಡುಗಡೆ(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__clean(ರಚನೆ image_cache_s *icache)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry = icache->entries;

	ಆಗ (centry != ಖಾಲಿ) {
		ಆದರೆ ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_ಸಮಯ) {
			/* only consider older entries, avoids active entries */
			ಆದರೆ ((icache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ >
			     (icache->params.ಎಲ್ಲೆ - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->ಮುಂದಿನ;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__background_update(ಶೂನ್ಯ *p)
{
	ರಚನೆ image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_ಸಮಯ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#ಆದರೆ_ಅಂತ್ಯ

	image_cache__clean(icache);

	guit->misc->ವೇಳಾಪಟ್ಟಿ(icache->params.bg_clean_ಸಮಯ,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
ರಚನೆ bitmap *image_cache_get_bitmap(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	centry = image_cache_ಕಂಡುಹಿಡಿ(c);
	ಆದರೆ (centry == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (centry->bitmap == ಖಾಲಿ) {
		ಆದರೆ (centry->convert != ಖಾಲಿ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
			image_cache_stats_bitmap_ಕೂಡಿಸು(centry);
			image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ++;
			image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			image_cache->fail_ಎಣಿಕೆ++;
			image_cache->fail_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		image_cache->ದೊರಕಿತು_ಎಣಿಕೆ++;
		image_cache->ದೊರಕಿತು_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
	}

	ಹಿಂತಿರುಗಿ centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ರಚನೆ content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	ಆದರೆ ((image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ < image_cache->params.ಎಲ್ಲೆ) &&
	    (c->ಗಾತ್ರ_2 <= image_cache->params.speculative_small)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->ಗಾತ್ರ_2,
		      SPECULATE_SMALL);
#ಆದರೆ_ಅಂತ್ಯ
		decision = true;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ decision;
}

/* exported interface documented in image_cache.h */
ರಚನೆ bitmap *image_cache_ಕಂಡುಹಿಡಿ_bitmap(ರಚನೆ content *c)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	centry = image_cache_ಕಂಡುಹಿಡಿ(c);
	ಆದರೆ (centry == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ಸ್ಥಿರಾಂಕ ರಚನೆ image_cache_parameters *image_cache_parameters)
{
	image_cache = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ image_cache_s));
	ಆದರೆ (image_cache == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->ವೇಳಾಪಟ್ಟಿ(image_cache->params.bg_clean_ಸಮಯ,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.ಎಲ್ಲೆ,
	      image_cache->params.hysteresis);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ op_ಎಣಿಕೆ;
	uint64_t op_ಗಾತ್ರ;

	guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->ಒಟ್ಟು_bitmap_ಗಾತ್ರ, image_cache->bitmap_ಎಣಿಕೆ);

	ಆಗ (image_cache->entries != ಖಾಲಿ) {
		image_cache_free_ನಮೂದು(image_cache->entries);
	}

	op_ಎಣಿಕೆ = image_cache->ದೊರಕಿತು_ಎಣಿಕೆ +
		image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ +
		image_cache->fail_ಎಣಿಕೆ;

	op_ಗಾತ್ರ = image_cache->ದೊರಕಿತು_ಗಾತ್ರ +
		image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ +
		image_cache->fail_ಗಾತ್ರ;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_ಗಾತ್ರ,
	      image_cache->max_bitmap_ಗಾತ್ರ_ಎಣಿಕೆ);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_ಎಣಿಕೆ,
	      image_cache->max_bitmap_ಎಣಿಕೆ_ಗಾತ್ರ);

	ಆದರೆ ((op_ಎಣಿಕೆ > 0) && (op_ಗಾತ್ರ >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_ಎಣಿಕೆ,
		      image_cache->ದೊರಕಿತು_ಎಣಿಕೆ,
		      image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ,
		      image_cache->fail_ಎಣಿಕೆ,
		      (image_cache->ದೊರಕಿತು_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
		      (image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
		      (image_cache->fail_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_ಗಾತ್ರ,
		      image_cache->ದೊರಕಿತು_ಗಾತ್ರ,
		      image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ,
		      image_cache->fail_ಗಾತ್ರ,
		      (image_cache->ದೊರಕಿತು_ಗಾತ್ರ * 100) / op_ಗಾತ್ರ,
		      (image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ * 100) / op_ಗಾತ್ರ,
		      (image_cache->fail_ಗಾತ್ರ * 100) / op_ಗಾತ್ರ);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->ಒಟ್ಟು_unrendered,
	      image_cache->specultive_ತಪ್ಪಿದ_ಎಣಿಕೆ);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->ಒಟ್ಟು_extra_conversions,
	      image_cache->ಒಟ್ಟು_extra_conversions_ಎಣಿಕೆ);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_ಗಾತ್ರ,
	      image_cache->peak_conversions);

	ಬಿಡುಗಡೆ(image_cache);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_ಕೂಡಿಸು(ರಚನೆ content *content,
			ರಚನೆ bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache_ಕಂಡುಹಿಡಿ(content);
	ಆದರೆ (centry == ಖಾಲಿ) {
		/* new cache entry, content not previously added */
		centry = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ image_cache_ನಮೂದು_s));
		ಆದರೆ (centry == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_ಗಾತ್ರ = content->ಅಗಲ * content->ಎತ್ತರ * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	ಆದರೆ (bitmap != ಖಾಲಿ) {
		ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
			guit->bitmap->destroy(centry->bitmap);
		} ಇಲ್ಲದಿದ್ದರೆ {
			image_cache_stats_bitmap_ಕೂಡಿಸು(centry);
		}
		centry->bitmap = bitmap;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* no bitmap, check to see if we should speculatively convert */
		ಆದರೆ ((centry->convert != ಖಾಲಿ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
				image_cache_stats_bitmap_ಕೂಡಿಸು(centry);
			} ಇಲ್ಲದಿದ್ದರೆ {
				image_cache->fail_ಎಣಿಕೆ++;
			}
		}
	}



	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ರಚನೆ content *content)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	/* get the cache entry */
	centry = image_cache_ಕಂಡುಹಿಡಿ(content);
	ಆದರೆ (centry == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	image_cache_free_ನಮೂದು(centry);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
ಪೂರ್ಣಾಂಕ image_cache_snsummaryf(ಅಕ್ಷರ *string, size_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt)
{
	size_t slen = 0; /* current output string length */
	ಪೂರ್ಣಾಂಕ fmtc = 0; /* current index into format string */
	bool pct;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ op_ಎಣಿಕೆ;
	uint64_t op_ಗಾತ್ರ;

	op_ಎಣಿಕೆ = image_cache->ದೊರಕಿತು_ಎಣಿಕೆ +
		image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ +
		image_cache->fail_ಎಣಿಕೆ;

	op_ಗಾತ್ರ = image_cache->ದೊರಕಿತು_ಗಾತ್ರ +
		image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ +
		image_cache->fail_ಗಾತ್ರ;

	ಆಗ((slen < ಗಾತ್ರ_2) && (fmt[fmtc] != 0)) {
		ಆದರೆ (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			ಆದರೆ (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} ಇಲ್ಲದಿದ್ದರೆ {
				pct = false;
			}

#ವ್ಯಾಖ್ಯಾನಿಸು FMTCHR(chr,fmt,var) ಸ್ಥಿತಿ chr : \
slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen, "%"fmt, image_cache->var); ನಿಲ್ಲಿಸು

#ವ್ಯಾಖ್ಯಾನಿಸು FMTPCHR(chr,fmt,var,div) \
ಸ್ಥಿತಿ chr :					\
	ಆದರೆ (pct) {							\
		ಆದರೆ (div > 0) {						\
			slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} ಇಲ್ಲದಿದ್ದರೆ {						\
			slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen, "100"); \
		}							\
	} ಇಲ್ಲದಿದ್ದರೆ {							\
		slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen, "%"fmt, image_cache->var); \
	} ನಿಲ್ಲಿಸು


			ಆಯ್ಕೆ (fmt[fmtc]) {
			ಸ್ಥಿತಿ '%':
				string[slen] = '%';
				slen++;
				ನಿಲ್ಲಿಸು;

			FMTCHR('a', PRIsizet, params.ಎಲ್ಲೆ);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, ಒಟ್ಟು_bitmap_ಗಾತ್ರ);
			FMTCHR('d', "d", bitmap_ಎಣಿಕೆ);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_ಗಾತ್ರ);
			FMTCHR('g', "d", max_bitmap_ಗಾತ್ರ_ಎಣಿಕೆ);
			FMTCHR('h', "d", max_bitmap_ಎಣಿಕೆ);
			FMTCHR('i', PRIsizet, max_bitmap_ಎಣಿಕೆ_ಗಾತ್ರ);


			ಸ್ಥಿತಿ 'j':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						 "%u", pct?100:op_ಎಣಿಕೆ);
				ನಿಲ್ಲಿಸು;

			FMTPCHR('k', "d", ದೊರಕಿತು_ಎಣಿಕೆ, op_ಎಣಿಕೆ);
			FMTPCHR('l', "d", ತಪ್ಪಿದ_ಎಣಿಕೆ, op_ಎಣಿಕೆ);
			FMTPCHR('m', "d", fail_ಎಣಿಕೆ, op_ಎಣಿಕೆ);

			ಸ್ಥಿತಿ 'n':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						 "%"PRId64, pct?100:op_ಗಾತ್ರ);
				ನಿಲ್ಲಿಸು;

			FMTPCHR('o', PRId64, ದೊರಕಿತು_ಗಾತ್ರ, op_ಗಾತ್ರ);
			FMTPCHR('q', PRId64, ತಪ್ಪಿದ_ಗಾತ್ರ, op_ಗಾತ್ರ);
			FMTPCHR('r', PRId64, fail_ಗಾತ್ರ, op_ಗಾತ್ರ);

			FMTCHR('s', "d", ಒಟ್ಟು_unrendered);
			FMTCHR('t', "d", specultive_ತಪ್ಪಿದ_ಎಣಿಕೆ);
			FMTCHR('u', "d", ಒಟ್ಟು_extra_conversions);
			FMTCHR('v', "d", ಒಟ್ಟು_extra_conversions_ಎಣಿಕೆ);
			FMTCHR('w', "u", peak_conversions_ಗಾತ್ರ);
			FMTCHR('x', "d", peak_conversions);


			}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು FMTCHR
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು FMTPCHR

			fmtc++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, ಗಾತ್ರ_2 - 1)] = '\0';

	ಹಿಂತಿರುಗಿ slen;
}

/* exported interface documented in image_cache.h */
ಪೂರ್ಣಾಂಕ
image_cache_snentryf(ಅಕ್ಷರ *string,
		     size_t ಗಾತ್ರ_2,
		     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ entryn,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;
	size_t slen = 0; /* current output string length */
	ಪೂರ್ಣಾಂಕ fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	ಆದರೆ (centry == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -1;

	ಆಗ((slen < ಗಾತ್ರ_2) && (fmt[fmtc] != 0)) {
		ಆದರೆ (fmt[fmtc] == '%') {
			fmtc++;
			ಆಯ್ಕೆ (fmt[fmtc]) {
			ಸ್ಥಿತಿ 'e':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%u", entryn);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'r':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%u", centry->redraw_ಎಣಿಕೆ);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'a':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						 "%.2f", (ದಶಮಾಂಶ)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ನಿಲ್ಲಿಸು;


			ಸ್ಥಿತಿ 'c':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%d", centry->conversion_ಎಣಿಕೆ);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'g':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%.2f", (ದಶಮಾಂಶ)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'k':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%p", centry->content);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'U':
				slen += snprintf(string + slen, ಗಾತ್ರ_2 - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 'o':
				ಆದರೆ (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							ಗಾತ್ರ_2 - slen, "%s",
							lwc_string_ದತ್ತಾಂಶ(
								origin));

					lwc_string_unref(origin);
				} ಇಲ್ಲದಿದ್ದರೆ {
					slen += snprintf(string + slen,
							ಗಾತ್ರ_2 - slen, "%s",
							"localhost");
				}
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ 's':
				ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
					slen += snprintf(string + slen,
							 ಗಾತ್ರ_2 - slen,
							 "%" PRIsizet,
							 centry->bitmap_ಗಾತ್ರ);
				} ಇಲ್ಲದಿದ್ದರೆ {
					slen += snprintf(string + slen,
							 ಗಾತ್ರ_2 - slen,
							 "0");
				}
				ನಿಲ್ಲಿಸು;
			}
			fmtc++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, ಗಾತ್ರ_2 - 1)] = '\0';

	ಹಿಂತಿರುಗಿ slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ರಚನೆ content *c,
			ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
			ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
			ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	/* get the cache entry */
	centry = image_cache_ಕಂಡುಹಿಡಿ(c);
	ಆದರೆ (centry == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (centry->bitmap == ಖಾಲಿ) {
		ಆದರೆ (centry->convert != ಖಾಲಿ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ಆದರೆ (centry->bitmap != ಖಾಲಿ) {
			image_cache_stats_bitmap_ಕೂಡಿಸು(centry);
			image_cache->ತಪ್ಪಿದ_ಎಣಿಕೆ++;
			image_cache->ತಪ್ಪಿದ_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			image_cache->fail_ಎಣಿಕೆ++;
			image_cache->fail_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
			ಹಿಂತಿರುಗಿ false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		image_cache->ದೊರಕಿತು_ಎಣಿಕೆ++;
		image_cache->ದೊರಕಿತು_ಗಾತ್ರ += centry->bitmap_ಗಾತ್ರ;
	}


	/* update statistics */
	centry->redraw_ಎಣಿಕೆ++;
	centry->redraw_age = image_cache->current_age;

	ಹಿಂತಿರುಗಿ image_bitmap_plot(centry->bitmap, ದತ್ತಾಂಶ, clip, ctx);
}

/* exported interface documented in image_cache.h */
ಶೂನ್ಯ image_cache_destroy(ರಚನೆ content *content)
{
	ರಚನೆ image_cache_ನಮೂದು_s *centry;

	/* get the cache entry */
	centry = image_cache_ಕಂಡುಹಿಡಿ(content);
	ಆದರೆ (centry == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} ಇಲ್ಲದಿದ್ದರೆ {
		image_cache_free_ನಮೂದು(centry);
	}
}

/* exported interface documented in image_cache.h */
ಶೂನ್ಯ *image_cache_get_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context)
{
	ಹಿಂತಿರುಗಿ image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ರಚನೆ content *c)
{
	ರಚನೆ bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	ಆದರೆ (bmp != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ guit->bitmap->get_opaque(bmp);
	}
	ಹಿಂತಿರುಗಿ false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_IMAGE;
}
