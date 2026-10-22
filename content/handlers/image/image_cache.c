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
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ಪೂರ್ಣಾಙ್ಕ cache_age;

/**
 * Image cache entry
 */
ಸಂರಚನಾ image_cache_entry_s {
	ಸಂರಚನಾ image_cache_entry_s *next; /**< next cache entry in list */
	ಸಂರಚನಾ image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ಸಂರಚನಾ content *content;
	/** associated bitmap entry */
	ಸಂರಚನಾ bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned ಪೂರ್ಣಾಙ್ಕ redraw_ಎಣಿಕೆ; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	ಪೂರ್ಣಾಙ್ಕ conversion_ಎಣಿಕೆ; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ಸಂರಚನಾ image_cache_s {
	/** Cache parameters */
	ಸಂರಚನಾ image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ಸಂರಚನಾ image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	ಪೂರ್ಣಾಙ್ಕ bitmap_ಎಣಿಕೆ;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	ಪೂರ್ಣಾಙ್ಕ max_bitmap_size_ಎಣಿಕೆ;

	/** Maximum count of bitmaps allocated at any one time */
	ಪೂರ್ಣಾಙ್ಕ max_bitmap_ಎಣಿಕೆ;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_ಎಣಿಕೆ_size;

	/** Bitmap was not available at plot time required conversion */
	ಪೂರ್ಣಾಙ್ಕ miss_ಎಣಿಕೆ;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	ಪೂರ್ಣಾಙ್ಕ hit_ಎಣಿಕೆ;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	ಪೂರ್ಣಾಙ್ಕ fail_ಎಣಿಕೆ;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	ಪೂರ್ಣಾಙ್ಕ total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	ಪೂರ್ಣಾಙ್ಕ specultive_miss_ಎಣಿಕೆ;

	/** Total number of additional (after the first) conversions */
	ಪೂರ್ಣಾಙ್ಕ total_extra_conversions;
	/** counts total number of images with more than one conversion */
	ಪೂರ್ಣಾಙ್ಕ total_extra_conversions_ಎಣಿಕೆ;

	/** Bitmap with most conversions was converted this many times */
	ಪೂರ್ಣಾಙ್ಕ peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned ಪೂರ್ಣಾಙ್ಕ peak_conversions_size;
};

/** image cache state */
ಸ್ಥಿರ ಸಂರಚನಾ image_cache_s *image_cache = ರಿಕ್ತ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ಸ್ಥಿರ ಸಂರಚನಾ image_cache_entry_s *image_cache__findn(ಪೂರ್ಣಾಙ್ಕ entryn)
{
	ಸಂರಚನಾ image_cache_entry_s *found;

	found = image_cache->entries;
	ಯಾವತ್ ((found != ರಿಕ್ತ) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	ಪ್ರತ್ಯಾಗಮ found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ಸ್ಥಿರ ಸಂರಚನಾ image_cache_entry_s *image_cache__find(ಧ್ರುವ ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ image_cache_entry_s *found;

	found = image_cache->entries;
	ಯಾವತ್ ((found != ರಿಕ್ತ) && (found->content != c)) {
		found = found->next;
	}
	ಪ್ರತ್ಯಾಗಮ found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache_stats_bitmap_add(ಸಂರಚನಾ image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_ಎಣಿಕೆ++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_ಎಣಿಕೆ++;

	ಯದಿ (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_ಎಣಿಕೆ = image_cache->bitmap_ಎಣಿಕೆ;

	}

	ಯದಿ (image_cache->bitmap_ಎಣಿಕೆ > image_cache->max_bitmap_ಎಣಿಕೆ) {
		image_cache->max_bitmap_ಎಣಿಕೆ = image_cache->bitmap_ಎಣಿಕೆ;
		image_cache->max_bitmap_ಎಣಿಕೆ_size = image_cache->total_bitmap_size;
	}

	ಯದಿ (centry->conversion_ಎಣಿಕೆ == 2) {
		image_cache->total_extra_conversions_ಎಣಿಕೆ++;
	}

	ಯದಿ (centry->conversion_ಎಣಿಕೆ > 1) {
		image_cache->total_extra_conversions++;
	}

	ಯದಿ ((centry->conversion_ಎಣಿಕೆ > image_cache->peak_conversions) ||
	    (centry->conversion_ಎಣಿಕೆ == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_ಎಣಿಕೆ;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ image_cache__link(ಸಂರಚನಾ image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = ರಿಕ್ತ;
	ಯದಿ (centry->next != ರಿಕ್ತ) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

ಸ್ಥಿರ ಶೂನ್ಯ image_cache__unlink(ಸಂರಚನಾ image_cache_entry_s *centry)
{
	/* unlink entry */
	ಯದಿ (centry->prev == ರಿಕ್ತ) {
		/* first in list */
		ಯದಿ (centry->next != ರಿಕ್ತ) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} ಅನ್ಯಥಾ {
			/* empty list */
			image_cache->entries = ರಿಕ್ತ;
		}
	} ಅನ್ಯಥಾ {
		centry->prev->next = centry->next;

		ಯದಿ (centry->next != ರಿಕ್ತ) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__free_bitmap(ಸಂರಚನಾ image_cache_entry_s *centry)
{
	ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_ಎಣಿಕೆ);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ರಿಕ್ತ;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_ಎಣಿಕೆ--;
		ಯದಿ (centry->redraw_ಎಣಿಕೆ == 0) {
			image_cache->specultive_miss_ಎಣಿಕೆ++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__free_entry(ಸಂರಚನಾ image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	ಯದಿ (centry->redraw_ಎಣಿಕೆ == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	ಮುಕ್ತ(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__clean(ಸಂರಚನಾ image_cache_s *icache)
{
	ಸಂರಚನಾ image_cache_entry_s *centry = icache->entries;

	ಯಾವತ್ (centry != ರಿಕ್ತ) {
		ಯದಿ ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			ಯದಿ ((icache->total_bitmap_size >
			     (icache->params.limit - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->next;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ image_cache__background_update(ಶೂನ್ಯ *p)
{
	ಸಂರಚನಾ image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_time;

#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#endif

	image_cache__clean(icache);

	guit->misc->schedule(icache->params.bg_clean_time,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
ಸಂರಚನಾ bitmap *image_cache_get_bitmap(ಧ್ರುವ ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	centry = image_cache__find(c);
	ಯದಿ (centry == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (centry->bitmap == ರಿಕ್ತ) {
		ಯದಿ (centry->convert != ರಿಕ್ತ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ಎಣಿಕೆ++;
			image_cache->miss_size += centry->bitmap_size;
		} ಅನ್ಯಥಾ {
			image_cache->fail_ಎಣಿಕೆ++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} ಅನ್ಯಥಾ {
		image_cache->hit_ಎಣಿಕೆ++;
		image_cache->hit_size += centry->bitmap_size;
	}

	ಪ್ರತ್ಯಾಗಮ centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ಸಂರಚನಾ content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	ಯದಿ ((image_cache->total_bitmap_size < image_cache->params.limit) &&
	    (c->size <= image_cache->params.speculative_small)) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->size,
		      SPECULATE_SMALL);
#endif
		decision = true;
	}

#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#endif
	ಪ್ರತ್ಯಾಗಮ decision;
}

/* exported interface documented in image_cache.h */
ಸಂರಚನಾ bitmap *image_cache_find_bitmap(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	centry = image_cache__find(c);
	ಯದಿ (centry == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ಧ್ರುವ ಸಂರಚನಾ image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ image_cache_s));
	ಯದಿ (image_cache == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(ಶೂನ್ಯ)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ op_ಎಣಿಕೆ;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_ಎಣಿಕೆ);

	ಯಾವತ್ (image_cache->entries != ರಿಕ್ತ) {
		image_cache__free_entry(image_cache->entries);
	}

	op_ಎಣಿಕೆ = image_cache->hit_ಎಣಿಕೆ +
		image_cache->miss_ಎಣಿಕೆ +
		image_cache->fail_ಎಣಿಕೆ;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_ಎಣಿಕೆ);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_ಎಣಿಕೆ,
	      image_cache->max_bitmap_ಎಣಿಕೆ_size);

	ಯದಿ ((op_ಎಣಿಕೆ > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_ಎಣಿಕೆ,
		      image_cache->hit_ಎಣಿಕೆ,
		      image_cache->miss_ಎಣಿಕೆ,
		      image_cache->fail_ಎಣಿಕೆ,
		      (image_cache->hit_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
		      (image_cache->miss_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
		      (image_cache->fail_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_size,
		      image_cache->hit_size,
		      image_cache->miss_size,
		      image_cache->fail_size,
		      (image_cache->hit_size * 100) / op_size,
		      (image_cache->miss_size * 100) / op_size,
		      (image_cache->fail_size * 100) / op_size);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->total_unrendered,
	      image_cache->specultive_miss_ಎಣಿಕೆ);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_ಎಣಿಕೆ);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	ಮುಕ್ತ(image_cache);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(ಸಂರಚನಾ content *content,
			ಸಂರಚನಾ bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	ಯದಿ (centry == ರಿಕ್ತ) {
		/* new cache entry, content not previously added */
		centry = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ image_cache_entry_s));
		ಯದಿ (centry == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	ಯದಿ (bitmap != ರಿಕ್ತ) {
		ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
			guit->bitmap->destroy(centry->bitmap);
		} ಅನ್ಯಥಾ {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} ಅನ್ಯಥಾ {
		/* no bitmap, check to see if we should speculatively convert */
		ಯದಿ ((centry->convert != ರಿಕ್ತ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
				image_cache_stats_bitmap_add(centry);
			} ಅನ್ಯಥಾ {
				image_cache->fail_ಎಣಿಕೆ++;
			}
		}
	}



	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ಸಂರಚನಾ content *content)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	ಯದಿ (centry == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
ಪೂರ್ಣಾಙ್ಕ image_cache_snsummaryf(ವರ್ಣ *string, size_t size, ಧ್ರುವ ವರ್ಣ *fmt)
{
	size_t slen = 0; /* current output string length */
	ಪೂರ್ಣಾಙ್ಕ fmtc = 0; /* current index into format string */
	bool pct;
	unsigned ಪೂರ್ಣಾಙ್ಕ op_ಎಣಿಕೆ;
	uint64_t op_size;

	op_ಎಣಿಕೆ = image_cache->hit_ಎಣಿಕೆ +
		image_cache->miss_ಎಣಿಕೆ +
		image_cache->fail_ಎಣಿಕೆ;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	ಯಾವತ್((slen < size) && (fmt[fmtc] != 0)) {
		ಯದಿ (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			ಯದಿ (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} ಅನ್ಯಥಾ {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) ಪಕ್ಷ chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); ಭಙ್ಗ

#define FMTPCHR(chr,fmt,var,div) \
ಪಕ್ಷ chr :					\
	ಯದಿ (pct) {							\
		ಯದಿ (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} ಅನ್ಯಥಾ {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} ಅನ್ಯಥಾ {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} ಭಙ್ಗ


			ವಿಕಲ್ಪ (fmt[fmtc]) {
			ಪಕ್ಷ '%':
				string[slen] = '%';
				slen++;
				ಭಙ್ಗ;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_ಎಣಿಕೆ);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_ಎಣಿಕೆ);
			FMTCHR('h', "d", max_bitmap_ಎಣಿಕೆ);
			FMTCHR('i', PRIsizet, max_bitmap_ಎಣಿಕೆ_size);


			ಪಕ್ಷ 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_ಎಣಿಕೆ);
				ಭಙ್ಗ;

			FMTPCHR('k', "d", hit_ಎಣಿಕೆ, op_ಎಣಿಕೆ);
			FMTPCHR('l', "d", miss_ಎಣಿಕೆ, op_ಎಣಿಕೆ);
			FMTPCHR('m', "d", fail_ಎಣಿಕೆ, op_ಎಣಿಕೆ);

			ಪಕ್ಷ 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				ಭಙ್ಗ;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_ಎಣಿಕೆ);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_ಎಣಿಕೆ);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} ಅನ್ಯಥಾ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ಪ್ರತ್ಯಾಗಮ slen;
}

/* exported interface documented in image_cache.h */
ಪೂರ್ಣಾಙ್ಕ
image_cache_snentryf(ವರ್ಣ *string,
		     size_t size,
		     unsigned ಪೂರ್ಣಾಙ್ಕ entryn,
		     ಧ್ರುವ ವರ್ಣ *fmt)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	ಪೂರ್ಣಾಙ್ಕ fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	ಯದಿ (centry == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯಾವತ್((slen < size) && (fmt[fmtc] != 0)) {
		ಯದಿ (fmt[fmtc] == '%') {
			fmtc++;
			ವಿಕಲ್ಪ (fmt[fmtc]) {
			ಪಕ್ಷ 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				ಭಙ್ಗ;

			ಪಕ್ಷ 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_ಎಣಿಕೆ);
				ಭಙ್ಗ;

			ಪಕ್ಷ 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (ಭಿನ್ನ)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ಭಙ್ಗ;


			ಪಕ್ಷ 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_ಎಣಿಕೆ);
				ಭಙ್ಗ;

			ಪಕ್ಷ 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (ಭಿನ್ನ)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ಭಙ್ಗ;

			ಪಕ್ಷ 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				ಭಙ್ಗ;

			ಪಕ್ಷ 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ಭಙ್ಗ;

			ಪಕ್ಷ 'o':
				ಯದಿ (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							size - slen, "%s",
							lwc_string_data(
								origin));

					lwc_string_unref(origin);
				} ಅನ್ಯಥಾ {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				ಭಙ್ಗ;

			ಪಕ್ಷ 's':
				ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} ಅನ್ಯಥಾ {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				ಭಙ್ಗ;
			}
			fmtc++;
		} ಅನ್ಯಥಾ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ಪ್ರತ್ಯಾಗಮ slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ಸಂರಚನಾ content *c,
			ಸಂರಚನಾ content_redraw_data *data,
			ಧ್ರುವ ಸಂರಚನಾ rect *clip,
			ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	ಯದಿ (centry == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (centry->bitmap == ರಿಕ್ತ) {
		ಯದಿ (centry->convert != ರಿಕ್ತ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ಯದಿ (centry->bitmap != ರಿಕ್ತ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ಎಣಿಕೆ++;
			image_cache->miss_size += centry->bitmap_size;
		} ಅನ್ಯಥಾ {
			image_cache->fail_ಎಣಿಕೆ++;
			image_cache->fail_size += centry->bitmap_size;
			ಪ್ರತ್ಯಾಗಮ false;
		}
	} ಅನ್ಯಥಾ {
		image_cache->hit_ಎಣಿಕೆ++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_ಎಣಿಕೆ++;
	centry->redraw_age = image_cache->current_age;

	ಪ್ರತ್ಯಾಗಮ image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
ಶೂನ್ಯ image_cache_destroy(ಸಂರಚನಾ content *content)
{
	ಸಂರಚನಾ image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	ಯದಿ (centry == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} ಅನ್ಯಥಾ {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
ಶೂನ್ಯ *image_cache_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	ಪ್ರತ್ಯಾಗಮ image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	ಯದಿ (bmp != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(bmp);
	}
	ಪ್ರತ್ಯಾಗಮ false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}
