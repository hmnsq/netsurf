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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "image/image_cache.h"
#அந்தர்பாவ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
வகைவரை unsigned முழு cache_age;

/**
 * Image cache entry
 */
கட்டமைப்பு image_cache_entry_s {
	கட்டமைப்பு image_cache_entry_s *next; /**< next cache entry in list */
	கட்டமைப்பு image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	கட்டமைப்பு content *content;
	/** associated bitmap entry */
	கட்டமைப்பு bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned முழு redraw_மொத்தம்; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	முழு conversion_மொத்தம்; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
கட்டமைப்பு image_cache_s {
	/** Cache parameters */
	கட்டமைப்பு image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	கட்டமைப்பு image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	முழு bitmap_மொத்தம்;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	முழு max_bitmap_size_மொத்தம்;

	/** Maximum count of bitmaps allocated at any one time */
	முழு max_bitmap_மொத்தம்;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_மொத்தம்_size;

	/** Bitmap was not available at plot time required conversion */
	முழு miss_மொத்தம்;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	முழு hit_மொத்தம்;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	முழு fail_மொத்தம்;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	முழு total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	முழு specultive_miss_மொத்தம்;

	/** Total number of additional (after the first) conversions */
	முழு total_extra_conversions;
	/** counts total number of images with more than one conversion */
	முழு total_extra_conversions_மொத்தம்;

	/** Bitmap with most conversions was converted this many times */
	முழு peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned முழு peak_conversions_size;
};

/** image cache state */
நிலையான கட்டமைப்பு image_cache_s *image_cache = வெற்று;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
நிலையான கட்டமைப்பு image_cache_entry_s *image_cache__findn(முழு entryn)
{
	கட்டமைப்பு image_cache_entry_s *found;

	found = image_cache->entries;
	போது ((found != வெற்று) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	திரும்பு found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
நிலையான கட்டமைப்பு image_cache_entry_s *image_cache__find(மாறிலி கட்டமைப்பு content *c)
{
	கட்டமைப்பு image_cache_entry_s *found;

	found = image_cache->entries;
	போது ((found != வெற்று) && (found->content != c)) {
		found = found->next;
	}
	திரும்பு found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
நிலையான வெறுமை image_cache_stats_bitmap_add(கட்டமைப்பு image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_மொத்தம்++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_மொத்தம்++;

	எனில் (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_மொத்தம் = image_cache->bitmap_மொத்தம்;

	}

	எனில் (image_cache->bitmap_மொத்தம் > image_cache->max_bitmap_மொத்தம்) {
		image_cache->max_bitmap_மொத்தம் = image_cache->bitmap_மொத்தம்;
		image_cache->max_bitmap_மொத்தம்_size = image_cache->total_bitmap_size;
	}

	எனில் (centry->conversion_மொத்தம் == 2) {
		image_cache->total_extra_conversions_மொத்தம்++;
	}

	எனில் (centry->conversion_மொத்தம் > 1) {
		image_cache->total_extra_conversions++;
	}

	எனில் ((centry->conversion_மொத்தம் > image_cache->peak_conversions) ||
	    (centry->conversion_மொத்தம் == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_மொத்தம்;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

நிலையான வெறுமை image_cache__link(கட்டமைப்பு image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = வெற்று;
	எனில் (centry->next != வெற்று) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

நிலையான வெறுமை image_cache__unlink(கட்டமைப்பு image_cache_entry_s *centry)
{
	/* unlink entry */
	எனில் (centry->prev == வெற்று) {
		/* first in list */
		எனில் (centry->next != வெற்று) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} இல்லையெனில் {
			/* empty list */
			image_cache->entries = வெற்று;
		}
	} இல்லையெனில் {
		centry->prev->next = centry->next;

		எனில் (centry->next != வெற்று) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
நிலையான வெறுமை image_cache__free_bitmap(கட்டமைப்பு image_cache_entry_s *centry)
{
	எனில் (centry->bitmap != வெற்று) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_மொத்தம்);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = வெற்று;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_மொத்தம்--;
		எனில் (centry->redraw_மொத்தம் == 0) {
			image_cache->specultive_miss_மொத்தம்++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
நிலையான வெறுமை image_cache__free_entry(கட்டமைப்பு image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	எனில் (centry->redraw_மொத்தம் == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	விடுவி(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
நிலையான வெறுமை image_cache__clean(கட்டமைப்பு image_cache_s *icache)
{
	கட்டமைப்பு image_cache_entry_s *centry = icache->entries;

	போது (centry != வெற்று) {
		எனில் ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			எனில் ((icache->total_bitmap_size >
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
நிலையான வெறுமை image_cache__background_update(வெறுமை *p)
{
	கட்டமைப்பு image_cache_s *icache = p;

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
கட்டமைப்பு bitmap *image_cache_get_bitmap(மாறிலி கட்டமைப்பு content *c)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	centry = image_cache__find(c);
	எனில் (centry == வெற்று) {
		திரும்பு வெற்று;
	}

	எனில் (centry->bitmap == வெற்று) {
		எனில் (centry->convert != வெற்று) {
			centry->bitmap = centry->convert(centry->content);
		}

		எனில் (centry->bitmap != வெற்று) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_மொத்தம்++;
			image_cache->miss_size += centry->bitmap_size;
		} இல்லையெனில் {
			image_cache->fail_மொத்தம்++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} இல்லையெனில் {
		image_cache->hit_மொத்தம்++;
		image_cache->hit_size += centry->bitmap_size;
	}

	திரும்பு centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(கட்டமைப்பு content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	எனில் ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	திரும்பு decision;
}

/* exported interface documented in image_cache.h */
கட்டமைப்பு bitmap *image_cache_find_bitmap(கட்டமைப்பு content *c)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	centry = image_cache__find(c);
	எனில் (centry == வெற்று) {
		திரும்பு வெற்று;
	}

	திரும்பு centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(மாறிலி கட்டமைப்பு image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, அளவு(கட்டமைப்பு image_cache_s));
	எனில் (image_cache == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	திரும்பு NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(வெறுமை)
{
	unsigned முழு op_மொத்தம்;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_மொத்தம்);

	போது (image_cache->entries != வெற்று) {
		image_cache__free_entry(image_cache->entries);
	}

	op_மொத்தம் = image_cache->hit_மொத்தம் +
		image_cache->miss_மொத்தம் +
		image_cache->fail_மொத்தம்;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_மொத்தம்);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_மொத்தம்,
	      image_cache->max_bitmap_மொத்தம்_size);

	எனில் ((op_மொத்தம் > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_மொத்தம்,
		      image_cache->hit_மொத்தம்,
		      image_cache->miss_மொத்தம்,
		      image_cache->fail_மொத்தம்,
		      (image_cache->hit_மொத்தம் * 100) / op_மொத்தம்,
		      (image_cache->miss_மொத்தம் * 100) / op_மொத்தம்,
		      (image_cache->fail_மொத்தம் * 100) / op_மொத்தம்);
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
	      image_cache->specultive_miss_மொத்தம்);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_மொத்தம்);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	விடுவி(image_cache);

	திரும்பு NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(கட்டமைப்பு content *content,
			கட்டமைப்பு bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	எனில் (centry == வெற்று) {
		/* new cache entry, content not previously added */
		centry = calloc(1, அளவு(கட்டமைப்பு image_cache_entry_s));
		எனில் (centry == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	எனில் (bitmap != வெற்று) {
		எனில் (centry->bitmap != வெற்று) {
			guit->bitmap->destroy(centry->bitmap);
		} இல்லையெனில் {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} இல்லையெனில் {
		/* no bitmap, check to see if we should speculatively convert */
		எனில் ((centry->convert != வெற்று) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			எனில் (centry->bitmap != வெற்று) {
				image_cache_stats_bitmap_add(centry);
			} இல்லையெனில் {
				image_cache->fail_மொத்தம்++;
			}
		}
	}



	திரும்பு NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(கட்டமைப்பு content *content)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	எனில் (centry == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		திரும்பு NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	திரும்பு NSERROR_OK;
}

/* exported interface documented in image_cache.h */
முழு image_cache_snsummaryf(எழுத்து *string, size_t size, மாறிலி எழுத்து *fmt)
{
	size_t slen = 0; /* current output string length */
	முழு fmtc = 0; /* current index into format string */
	bool pct;
	unsigned முழு op_மொத்தம்;
	uint64_t op_size;

	op_மொத்தம் = image_cache->hit_மொத்தம் +
		image_cache->miss_மொத்தம் +
		image_cache->fail_மொத்தம்;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	போது((slen < size) && (fmt[fmtc] != 0)) {
		எனில் (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			எனில் (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} இல்லையெனில் {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) நிலை chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); நிறுத்து

#define FMTPCHR(chr,fmt,var,div) \
நிலை chr :					\
	எனில் (pct) {							\
		எனில் (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} இல்லையெனில் {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} இல்லையெனில் {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} நிறுத்து


			தேர்வு (fmt[fmtc]) {
			நிலை '%':
				string[slen] = '%';
				slen++;
				நிறுத்து;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_மொத்தம்);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_மொத்தம்);
			FMTCHR('h', "d", max_bitmap_மொத்தம்);
			FMTCHR('i', PRIsizet, max_bitmap_மொத்தம்_size);


			நிலை 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_மொத்தம்);
				நிறுத்து;

			FMTPCHR('k', "d", hit_மொத்தம், op_மொத்தம்);
			FMTPCHR('l', "d", miss_மொத்தம், op_மொத்தம்);
			FMTPCHR('m', "d", fail_மொத்தம், op_மொத்தம்);

			நிலை 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				நிறுத்து;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_மொத்தம்);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_மொத்தம்);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} இல்லையெனில் {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	திரும்பு slen;
}

/* exported interface documented in image_cache.h */
முழு
image_cache_snentryf(எழுத்து *string,
		     size_t size,
		     unsigned முழு entryn,
		     மாறிலி எழுத்து *fmt)
{
	கட்டமைப்பு image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	முழு fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	எனில் (centry == வெற்று)
		திரும்பு -1;

	போது((slen < size) && (fmt[fmtc] != 0)) {
		எனில் (fmt[fmtc] == '%') {
			fmtc++;
			தேர்வு (fmt[fmtc]) {
			நிலை 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				நிறுத்து;

			நிலை 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_மொத்தம்);
				நிறுத்து;

			நிலை 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (தசம)((image_cache->current_age -  centry->redraw_age)) / 1000);
				நிறுத்து;


			நிலை 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_மொத்தம்);
				நிறுத்து;

			நிலை 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (தசம)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				நிறுத்து;

			நிலை 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				நிறுத்து;

			நிலை 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				நிறுத்து;

			நிலை 'o':
				எனில் (nsurl_has_component(llcache_handle_get_url(
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
				} இல்லையெனில் {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				நிறுத்து;

			நிலை 's':
				எனில் (centry->bitmap != வெற்று) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} இல்லையெனில் {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				நிறுத்து;
			}
			fmtc++;
		} இல்லையெனில் {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	திரும்பு slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(கட்டமைப்பு content *c,
			கட்டமைப்பு content_redraw_data *data,
			மாறிலி கட்டமைப்பு rect *clip,
			மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	எனில் (centry == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		திரும்பு false;
	}

	எனில் (centry->bitmap == வெற்று) {
		எனில் (centry->convert != வெற்று) {
			centry->bitmap = centry->convert(centry->content);
		}

		எனில் (centry->bitmap != வெற்று) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_மொத்தம்++;
			image_cache->miss_size += centry->bitmap_size;
		} இல்லையெனில் {
			image_cache->fail_மொத்தம்++;
			image_cache->fail_size += centry->bitmap_size;
			திரும்பு false;
		}
	} இல்லையெனில் {
		image_cache->hit_மொத்தம்++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_மொத்தம்++;
	centry->redraw_age = image_cache->current_age;

	திரும்பு image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
வெறுமை image_cache_destroy(கட்டமைப்பு content *content)
{
	கட்டமைப்பு image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	எனில் (centry == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} இல்லையெனில் {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
வெறுமை *image_cache_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	திரும்பு image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(கட்டமைப்பு content *c)
{
	கட்டமைப்பு bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	எனில் (bmp != வெற்று) {
		திரும்பு guit->bitmap->get_opaque(bmp);
	}
	திரும்பு false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}
