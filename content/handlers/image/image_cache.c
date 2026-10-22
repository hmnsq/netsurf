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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"
#ଅନ୍ତର୍ଭାଵ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cache_age;

/**
 * Image cache entry
 */
ସଂରଚନା image_cache_entry_s {
	ସଂରଚନା image_cache_entry_s *next; /**< next cache entry in list */
	ସଂରଚନା image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ସଂରଚନା content *content;
	/** associated bitmap entry */
	ସଂରଚନା bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା redraw_ଗଣନା; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା conversion_ଗଣନା; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ସଂରଚନା image_cache_s {
	/** Cache parameters */
	ସଂରଚନା image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ସଂରଚନା image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bitmap_ଗଣନା;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_bitmap_size_ଗଣନା;

	/** Maximum count of bitmaps allocated at any one time */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_bitmap_ଗଣନା;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_ଗଣନା_size;

	/** Bitmap was not available at plot time required conversion */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା miss_ଗଣନା;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hit_ଗଣନା;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fail_ଗଣନା;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା specultive_miss_ଗଣନା;

	/** Total number of additional (after the first) conversions */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_extra_conversions;
	/** counts total number of images with more than one conversion */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_extra_conversions_ଗଣନା;

	/** Bitmap with most conversions was converted this many times */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା peak_conversions_size;
};

/** image cache state */
ସ୍ଥିର ସଂରଚନା image_cache_s *image_cache = ଖାଲି;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ସ୍ଥିର ସଂରଚନା image_cache_entry_s *image_cache__findn(ପୂର୍ଣ୍ଣସଂଖ୍ୟା entryn)
{
	ସଂରଚନା image_cache_entry_s *found;

	found = image_cache->entries;
	ଯେତେବେଳେ ((found != ଖାଲି) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	ଫେରନ୍ତୁ found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ସ୍ଥିର ସଂରଚନା image_cache_entry_s *image_cache__find(ଅଚଳ ସଂରଚନା content *c)
{
	ସଂରଚନା image_cache_entry_s *found;

	found = image_cache->entries;
	ଯେତେବେଳେ ((found != ଖାଲି) && (found->content != c)) {
		found = found->next;
	}
	ଫେରନ୍ତୁ found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ସ୍ଥିର ଶୂନ୍ୟ image_cache_stats_bitmap_add(ସଂରଚନା image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_ଗଣନା++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_ଗଣନା++;

	ଯଦି (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_ଗଣନା = image_cache->bitmap_ଗଣନା;

	}

	ଯଦି (image_cache->bitmap_ଗଣନା > image_cache->max_bitmap_ଗଣନା) {
		image_cache->max_bitmap_ଗଣନା = image_cache->bitmap_ଗଣନା;
		image_cache->max_bitmap_ଗଣନା_size = image_cache->total_bitmap_size;
	}

	ଯଦି (centry->conversion_ଗଣନା == 2) {
		image_cache->total_extra_conversions_ଗଣନା++;
	}

	ଯଦି (centry->conversion_ଗଣନା > 1) {
		image_cache->total_extra_conversions++;
	}

	ଯଦି ((centry->conversion_ଗଣନା > image_cache->peak_conversions) ||
	    (centry->conversion_ଗଣନା == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_ଗଣନା;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ image_cache__link(ସଂରଚନା image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = ଖାଲି;
	ଯଦି (centry->next != ଖାଲି) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

ସ୍ଥିର ଶୂନ୍ୟ image_cache__unlink(ସଂରଚନା image_cache_entry_s *centry)
{
	/* unlink entry */
	ଯଦି (centry->prev == ଖାଲି) {
		/* first in list */
		ଯଦି (centry->next != ଖାଲି) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} ଅନ୍ୟଥା {
			/* empty list */
			image_cache->entries = ଖାଲି;
		}
	} ଅନ୍ୟଥା {
		centry->prev->next = centry->next;

		ଯଦି (centry->next != ଖାଲି) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ସ୍ଥିର ଶୂନ୍ୟ image_cache__free_bitmap(ସଂରଚନା image_cache_entry_s *centry)
{
	ଯଦି (centry->bitmap != ଖାଲି) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_ଗଣନା);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ଖାଲି;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_ଗଣନା--;
		ଯଦି (centry->redraw_ଗଣନା == 0) {
			image_cache->specultive_miss_ଗଣନା++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ସ୍ଥିର ଶୂନ୍ୟ image_cache__free_entry(ସଂରଚନା image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	ଯଦି (centry->redraw_ଗଣନା == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	ମୁକ୍ତ(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ସ୍ଥିର ଶୂନ୍ୟ image_cache__clean(ସଂରଚନା image_cache_s *icache)
{
	ସଂରଚନା image_cache_entry_s *centry = icache->entries;

	ଯେତେବେଳେ (centry != ଖାଲି) {
		ଯଦି ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			ଯଦି ((icache->total_bitmap_size >
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
ସ୍ଥିର ଶୂନ୍ୟ image_cache__background_update(ଶୂନ୍ୟ *p)
{
	ସଂରଚନା image_cache_s *icache = p;

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
ସଂରଚନା bitmap *image_cache_get_bitmap(ଅଚଳ ସଂରଚନା content *c)
{
	ସଂରଚନା image_cache_entry_s *centry;

	centry = image_cache__find(c);
	ଯଦି (centry == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (centry->bitmap == ଖାଲି) {
		ଯଦି (centry->convert != ଖାଲି) {
			centry->bitmap = centry->convert(centry->content);
		}

		ଯଦି (centry->bitmap != ଖାଲି) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ଗଣନା++;
			image_cache->miss_size += centry->bitmap_size;
		} ଅନ୍ୟଥା {
			image_cache->fail_ଗଣନା++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} ଅନ୍ୟଥା {
		image_cache->hit_ଗଣନା++;
		image_cache->hit_size += centry->bitmap_size;
	}

	ଫେରନ୍ତୁ centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ସଂରଚନା content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	ଯଦି ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	ଫେରନ୍ତୁ decision;
}

/* exported interface documented in image_cache.h */
ସଂରଚନା bitmap *image_cache_find_bitmap(ସଂରଚନା content *c)
{
	ସଂରଚନା image_cache_entry_s *centry;

	centry = image_cache__find(c);
	ଯଦି (centry == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ଅଚଳ ସଂରଚନା image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, ଆକାର(ସଂରଚନା image_cache_s));
	ଯଦି (image_cache == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(ଶୂନ୍ୟ)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା op_ଗଣନା;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_ଗଣନା);

	ଯେତେବେଳେ (image_cache->entries != ଖାଲି) {
		image_cache__free_entry(image_cache->entries);
	}

	op_ଗଣନା = image_cache->hit_ଗଣନା +
		image_cache->miss_ଗଣନା +
		image_cache->fail_ଗଣନା;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_ଗଣନା);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_ଗଣନା,
	      image_cache->max_bitmap_ଗଣନା_size);

	ଯଦି ((op_ଗଣନା > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_ଗଣନା,
		      image_cache->hit_ଗଣନା,
		      image_cache->miss_ଗଣନା,
		      image_cache->fail_ଗଣନା,
		      (image_cache->hit_ଗଣନା * 100) / op_ଗଣନା,
		      (image_cache->miss_ଗଣନା * 100) / op_ଗଣନା,
		      (image_cache->fail_ଗଣନା * 100) / op_ଗଣନା);
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
	      image_cache->specultive_miss_ଗଣନା);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_ଗଣନା);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	ମୁକ୍ତ(image_cache);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(ସଂରଚନା content *content,
			ସଂରଚନା bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ସଂରଚନା image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	ଯଦି (centry == ଖାଲି) {
		/* new cache entry, content not previously added */
		centry = calloc(1, ଆକାର(ସଂରଚନା image_cache_entry_s));
		ଯଦି (centry == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	ଯଦି (bitmap != ଖାଲି) {
		ଯଦି (centry->bitmap != ଖାଲି) {
			guit->bitmap->destroy(centry->bitmap);
		} ଅନ୍ୟଥା {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} ଅନ୍ୟଥା {
		/* no bitmap, check to see if we should speculatively convert */
		ଯଦି ((centry->convert != ଖାଲି) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			ଯଦି (centry->bitmap != ଖାଲି) {
				image_cache_stats_bitmap_add(centry);
			} ଅନ୍ୟଥା {
				image_cache->fail_ଗଣନା++;
			}
		}
	}



	ଫେରନ୍ତୁ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ସଂରଚନା content *content)
{
	ସଂରଚନା image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	ଯଦି (centry == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା image_cache_snsummaryf(ଅକ୍ଷର *string, size_t size, ଅଚଳ ଅକ୍ଷର *fmt)
{
	size_t slen = 0; /* current output string length */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fmtc = 0; /* current index into format string */
	bool pct;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା op_ଗଣନା;
	uint64_t op_size;

	op_ଗଣନା = image_cache->hit_ଗଣନା +
		image_cache->miss_ଗଣନା +
		image_cache->fail_ଗଣନା;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	ଯେତେବେଳେ((slen < size) && (fmt[fmtc] != 0)) {
		ଯଦି (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			ଯଦି (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} ଅନ୍ୟଥା {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) ସ୍ଥିତି chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); ଭାଙ୍ଗ

#define FMTPCHR(chr,fmt,var,div) \
ସ୍ଥିତି chr :					\
	ଯଦି (pct) {							\
		ଯଦି (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} ଅନ୍ୟଥା {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} ଅନ୍ୟଥା {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} ଭାଙ୍ଗ


			ବାଛ (fmt[fmtc]) {
			ସ୍ଥିତି '%':
				string[slen] = '%';
				slen++;
				ଭାଙ୍ଗ;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_ଗଣନା);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_ଗଣନା);
			FMTCHR('h', "d", max_bitmap_ଗଣନା);
			FMTCHR('i', PRIsizet, max_bitmap_ଗଣନା_size);


			ସ୍ଥିତି 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_ଗଣନା);
				ଭାଙ୍ଗ;

			FMTPCHR('k', "d", hit_ଗଣନା, op_ଗଣନା);
			FMTPCHR('l', "d", miss_ଗଣନା, op_ଗଣନା);
			FMTPCHR('m', "d", fail_ଗଣନା, op_ଗଣନା);

			ସ୍ଥିତି 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				ଭାଙ୍ଗ;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_ଗଣନା);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_ଗଣନା);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} ଅନ୍ୟଥା {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ଫେରନ୍ତୁ slen;
}

/* exported interface documented in image_cache.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
image_cache_snentryf(ଅକ୍ଷର *string,
		     size_t size,
		     unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା entryn,
		     ଅଚଳ ଅକ୍ଷର *fmt)
{
	ସଂରଚନା image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	ଯଦି (centry == ଖାଲି)
		ଫେରନ୍ତୁ -1;

	ଯେତେବେଳେ((slen < size) && (fmt[fmtc] != 0)) {
		ଯଦି (fmt[fmtc] == '%') {
			fmtc++;
			ବାଛ (fmt[fmtc]) {
			ସ୍ଥିତି 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_ଗଣନା);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (ଦଶମିକ)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ଭାଙ୍ଗ;


			ସ୍ଥିତି 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_ଗଣନା);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (ଦଶମିକ)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 'o':
				ଯଦି (nsurl_has_component(llcache_handle_get_url(
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
				} ଅନ୍ୟଥା {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				ଭାଙ୍ଗ;

			ସ୍ଥିତି 's':
				ଯଦି (centry->bitmap != ଖାଲି) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} ଅନ୍ୟଥା {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				ଭାଙ୍ଗ;
			}
			fmtc++;
		} ଅନ୍ୟଥା {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ଫେରନ୍ତୁ slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ସଂରଚନା content *c,
			ସଂରଚନା content_redraw_data *data,
			ଅଚଳ ସଂରଚନା rect *clip,
			ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	ଯଦି (centry == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (centry->bitmap == ଖାଲି) {
		ଯଦି (centry->convert != ଖାଲି) {
			centry->bitmap = centry->convert(centry->content);
		}

		ଯଦି (centry->bitmap != ଖାଲି) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ଗଣନା++;
			image_cache->miss_size += centry->bitmap_size;
		} ଅନ୍ୟଥା {
			image_cache->fail_ଗଣନା++;
			image_cache->fail_size += centry->bitmap_size;
			ଫେରନ୍ତୁ false;
		}
	} ଅନ୍ୟଥା {
		image_cache->hit_ଗଣନା++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_ଗଣନା++;
	centry->redraw_age = image_cache->current_age;

	ଫେରନ୍ତୁ image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
ଶୂନ୍ୟ image_cache_destroy(ସଂରଚନା content *content)
{
	ସଂରଚନା image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	ଯଦି (centry == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} ଅନ୍ୟଥା {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
ଶୂନ୍ୟ *image_cache_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	ଫେରନ୍ତୁ image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ସଂରଚନା content *c)
{
	ସଂରଚନା bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	ଯଦି (bmp != ଖାଲି) {
		ଫେରନ୍ତୁ guit->bitmap->get_opaque(bmp);
	}
	ଫେରନ୍ତୁ false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}
