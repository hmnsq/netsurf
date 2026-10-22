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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "content/llcache.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "image/image_cache.h"
#অন্তর্ভাৱ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
প্রকারনির্দেশ unsigned পূর্ণাঙ্ক cache_age;

/**
 * Image cache entry
 */
সংরচনা image_cache_entry_s {
	সংরচনা image_cache_entry_s *next; /**< next cache entry in list */
	সংরচনা image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	সংরচনা content *content;
	/** associated bitmap entry */
	সংরচনা bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned পূর্ণাঙ্ক redraw_গুনতি; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	পূর্ণাঙ্ক conversion_গুনতি; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
সংরচনা image_cache_s {
	/** Cache parameters */
	সংরচনা image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	সংরচনা image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	পূর্ণাঙ্ক bitmap_গুনতি;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	পূর্ণাঙ্ক max_bitmap_size_গুনতি;

	/** Maximum count of bitmaps allocated at any one time */
	পূর্ণাঙ্ক max_bitmap_গুনতি;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_গুনতি_size;

	/** Bitmap was not available at plot time required conversion */
	পূর্ণাঙ্ক miss_গুনতি;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	পূর্ণাঙ্ক hit_গুনতি;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	পূর্ণাঙ্ক fail_গুনতি;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	পূর্ণাঙ্ক total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	পূর্ণাঙ্ক specultive_miss_গুনতি;

	/** Total number of additional (after the first) conversions */
	পূর্ণাঙ্ক total_extra_conversions;
	/** counts total number of images with more than one conversion */
	পূর্ণাঙ্ক total_extra_conversions_গুনতি;

	/** Bitmap with most conversions was converted this many times */
	পূর্ণাঙ্ক peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned পূর্ণাঙ্ক peak_conversions_size;
};

/** image cache state */
স্থির সংরচনা image_cache_s *image_cache = রিক্ত;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
স্থির সংরচনা image_cache_entry_s *image_cache__findn(পূর্ণাঙ্ক entryn)
{
	সংরচনা image_cache_entry_s *found;

	found = image_cache->entries;
	যেতিয়ালৈ ((found != রিক্ত) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	ঘূৰাই found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
স্থির সংরচনা image_cache_entry_s *image_cache__find(ধ্রুৱ সংরচনা content *c)
{
	সংরচনা image_cache_entry_s *found;

	found = image_cache->entries;
	যেতিয়ালৈ ((found != রিক্ত) && (found->content != c)) {
		found = found->next;
	}
	ঘূৰাই found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
স্থির শূন্য image_cache_stats_bitmap_add(সংরচনা image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_গুনতি++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_গুনতি++;

	যদি (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_গুনতি = image_cache->bitmap_গুনতি;

	}

	যদি (image_cache->bitmap_গুনতি > image_cache->max_bitmap_গুনতি) {
		image_cache->max_bitmap_গুনতি = image_cache->bitmap_গুনতি;
		image_cache->max_bitmap_গুনতি_size = image_cache->total_bitmap_size;
	}

	যদি (centry->conversion_গুনতি == 2) {
		image_cache->total_extra_conversions_গুনতি++;
	}

	যদি (centry->conversion_গুনতি > 1) {
		image_cache->total_extra_conversions++;
	}

	যদি ((centry->conversion_গুনতি > image_cache->peak_conversions) ||
	    (centry->conversion_গুনতি == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_গুনতি;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

স্থির শূন্য image_cache__link(সংরচনা image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = রিক্ত;
	যদি (centry->next != রিক্ত) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

স্থির শূন্য image_cache__unlink(সংরচনা image_cache_entry_s *centry)
{
	/* unlink entry */
	যদি (centry->prev == রিক্ত) {
		/* first in list */
		যদি (centry->next != রিক্ত) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} অন্যথা {
			/* empty list */
			image_cache->entries = রিক্ত;
		}
	} অন্যথা {
		centry->prev->next = centry->next;

		যদি (centry->next != রিক্ত) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
স্থির শূন্য image_cache__free_bitmap(সংরচনা image_cache_entry_s *centry)
{
	যদি (centry->bitmap != রিক্ত) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_গুনতি);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = রিক্ত;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_গুনতি--;
		যদি (centry->redraw_গুনতি == 0) {
			image_cache->specultive_miss_গুনতি++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
স্থির শূন্য image_cache__free_entry(সংরচনা image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	যদি (centry->redraw_গুনতি == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	মুক্ত(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
স্থির শূন্য image_cache__clean(সংরচনা image_cache_s *icache)
{
	সংরচনা image_cache_entry_s *centry = icache->entries;

	যেতিয়ালৈ (centry != রিক্ত) {
		যদি ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			যদি ((icache->total_bitmap_size >
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
স্থির শূন্য image_cache__background_update(শূন্য *p)
{
	সংরচনা image_cache_s *icache = p;

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
সংরচনা bitmap *image_cache_get_bitmap(ধ্রুৱ সংরচনা content *c)
{
	সংরচনা image_cache_entry_s *centry;

	centry = image_cache__find(c);
	যদি (centry == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	যদি (centry->bitmap == রিক্ত) {
		যদি (centry->convert != রিক্ত) {
			centry->bitmap = centry->convert(centry->content);
		}

		যদি (centry->bitmap != রিক্ত) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_গুনতি++;
			image_cache->miss_size += centry->bitmap_size;
		} অন্যথা {
			image_cache->fail_গুনতি++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} অন্যথা {
		image_cache->hit_গুনতি++;
		image_cache->hit_size += centry->bitmap_size;
	}

	ঘূৰাই centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(সংরচনা content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	যদি ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	ঘূৰাই decision;
}

/* exported interface documented in image_cache.h */
সংরচনা bitmap *image_cache_find_bitmap(সংরচনা content *c)
{
	সংরচনা image_cache_entry_s *centry;

	centry = image_cache__find(c);
	যদি (centry == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ধ্রুৱ সংরচনা image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, পরিমাণ(সংরচনা image_cache_s));
	যদি (image_cache == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(শূন্য)
{
	unsigned পূর্ণাঙ্ক op_গুনতি;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_গুনতি);

	যেতিয়ালৈ (image_cache->entries != রিক্ত) {
		image_cache__free_entry(image_cache->entries);
	}

	op_গুনতি = image_cache->hit_গুনতি +
		image_cache->miss_গুনতি +
		image_cache->fail_গুনতি;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_গুনতি);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_গুনতি,
	      image_cache->max_bitmap_গুনতি_size);

	যদি ((op_গুনতি > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_গুনতি,
		      image_cache->hit_গুনতি,
		      image_cache->miss_গুনতি,
		      image_cache->fail_গুনতি,
		      (image_cache->hit_গুনতি * 100) / op_গুনতি,
		      (image_cache->miss_গুনতি * 100) / op_গুনতি,
		      (image_cache->fail_গুনতি * 100) / op_গুনতি);
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
	      image_cache->specultive_miss_গুনতি);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_গুনতি);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	মুক্ত(image_cache);

	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(সংরচনা content *content,
			সংরচনা bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	সংরচনা image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	যদি (centry == রিক্ত) {
		/* new cache entry, content not previously added */
		centry = calloc(1, পরিমাণ(সংরচনা image_cache_entry_s));
		যদি (centry == রিক্ত) {
			ঘূৰাই NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	যদি (bitmap != রিক্ত) {
		যদি (centry->bitmap != রিক্ত) {
			guit->bitmap->destroy(centry->bitmap);
		} অন্যথা {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} অন্যথা {
		/* no bitmap, check to see if we should speculatively convert */
		যদি ((centry->convert != রিক্ত) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			যদি (centry->bitmap != রিক্ত) {
				image_cache_stats_bitmap_add(centry);
			} অন্যথা {
				image_cache->fail_গুনতি++;
			}
		}
	}



	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(সংরচনা content *content)
{
	সংরচনা image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	যদি (centry == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ঘূৰাই NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in image_cache.h */
পূর্ণাঙ্ক image_cache_snsummaryf(ৱর্ণ *string, size_t size, ধ্রুৱ ৱর্ণ *fmt)
{
	size_t slen = 0; /* current output string length */
	পূর্ণাঙ্ক fmtc = 0; /* current index into format string */
	bool pct;
	unsigned পূর্ণাঙ্ক op_গুনতি;
	uint64_t op_size;

	op_গুনতি = image_cache->hit_গুনতি +
		image_cache->miss_গুনতি +
		image_cache->fail_গুনতি;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	যেতিয়ালৈ((slen < size) && (fmt[fmtc] != 0)) {
		যদি (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			যদি (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} অন্যথা {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) পক্ষ chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); ভঙ্গ

#define FMTPCHR(chr,fmt,var,div) \
পক্ষ chr :					\
	যদি (pct) {							\
		যদি (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} অন্যথা {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} অন্যথা {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} ভঙ্গ


			ৱিকল্প (fmt[fmtc]) {
			পক্ষ '%':
				string[slen] = '%';
				slen++;
				ভঙ্গ;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_গুনতি);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_গুনতি);
			FMTCHR('h', "d", max_bitmap_গুনতি);
			FMTCHR('i', PRIsizet, max_bitmap_গুনতি_size);


			পক্ষ 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_গুনতি);
				ভঙ্গ;

			FMTPCHR('k', "d", hit_গুনতি, op_গুনতি);
			FMTPCHR('l', "d", miss_গুনতি, op_গুনতি);
			FMTPCHR('m', "d", fail_গুনতি, op_গুনতি);

			পক্ষ 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				ভঙ্গ;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_গুনতি);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_গুনতি);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} অন্যথা {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ঘূৰাই slen;
}

/* exported interface documented in image_cache.h */
পূর্ণাঙ্ক
image_cache_snentryf(ৱর্ণ *string,
		     size_t size,
		     unsigned পূর্ণাঙ্ক entryn,
		     ধ্রুৱ ৱর্ণ *fmt)
{
	সংরচনা image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	পূর্ণাঙ্ক fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	যদি (centry == রিক্ত)
		ঘূৰাই -1;

	যেতিয়ালৈ((slen < size) && (fmt[fmtc] != 0)) {
		যদি (fmt[fmtc] == '%') {
			fmtc++;
			ৱিকল্প (fmt[fmtc]) {
			পক্ষ 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				ভঙ্গ;

			পক্ষ 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_গুনতি);
				ভঙ্গ;

			পক্ষ 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (ভিন্ন)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ভঙ্গ;


			পক্ষ 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_গুনতি);
				ভঙ্গ;

			পক্ষ 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (ভিন্ন)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ভঙ্গ;

			পক্ষ 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				ভঙ্গ;

			পক্ষ 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ভঙ্গ;

			পক্ষ 'o':
				যদি (nsurl_has_component(llcache_handle_get_url(
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
				} অন্যথা {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				ভঙ্গ;

			পক্ষ 's':
				যদি (centry->bitmap != রিক্ত) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} অন্যথা {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				ভঙ্গ;
			}
			fmtc++;
		} অন্যথা {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ঘূৰাই slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(সংরচনা content *c,
			সংরচনা content_redraw_data *data,
			ধ্রুৱ সংরচনা rect *clip,
			ধ্রুৱ সংরচনা redraw_context *ctx)
{
	সংরচনা image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	যদি (centry == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ঘূৰাই false;
	}

	যদি (centry->bitmap == রিক্ত) {
		যদি (centry->convert != রিক্ত) {
			centry->bitmap = centry->convert(centry->content);
		}

		যদি (centry->bitmap != রিক্ত) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_গুনতি++;
			image_cache->miss_size += centry->bitmap_size;
		} অন্যথা {
			image_cache->fail_গুনতি++;
			image_cache->fail_size += centry->bitmap_size;
			ঘূৰাই false;
		}
	} অন্যথা {
		image_cache->hit_গুনতি++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_গুনতি++;
	centry->redraw_age = image_cache->current_age;

	ঘূৰাই image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
শূন্য image_cache_destroy(সংরচনা content *content)
{
	সংরচনা image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	যদি (centry == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} অন্যথা {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
শূন্য *image_cache_get_internal(ধ্রুৱ সংরচনা content *c, শূন্য *context)
{
	ঘূৰাই image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(সংরচনা content *c)
{
	সংরচনা bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	যদি (bmp != রিক্ত) {
		ঘূৰাই guit->bitmap->get_opaque(bmp);
	}
	ঘূৰাই false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(শূন্য)
{
	ঘূৰাই CONTENT_IMAGE;
}
