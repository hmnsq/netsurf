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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "image/image_cache.h"
#অন্তর্ভাব "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
প্রকারলেখ unsigned পূর্ণসংখ্যা cache_age;

/**
 * Image cache entry
 */
কাঠামো image_cache_entry_s {
	কাঠামো image_cache_entry_s *পরবর্তী; /**< next cache entry in list */
	কাঠামো image_cache_entry_s *পূর্ব; /**< previous cache entry in list */

	/** content is used as a key */
	কাঠামো content *content;
	/** associated bitmap entry */
	কাঠামো bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned পূর্ণসংখ্যা redraw_গুনতি; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_আকার; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	পূর্ণসংখ্যা conversion_গুনতি; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
কাঠামো image_cache_s {
	/** Cache parameters */
	কাঠামো image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	কাঠামো image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t যোগ_bitmap_আকার;

	/** Total count of bitmaps currently allocated */
	পূর্ণসংখ্যা bitmap_গুনতি;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_আকার;
	/** The number of objects when maximum bitmap usage occurred */
	পূর্ণসংখ্যা max_bitmap_আকার_গুনতি;

	/** Maximum count of bitmaps allocated at any one time */
	পূর্ণসংখ্যা max_bitmap_গুনতি;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_গুনতি_আকার;

	/** Bitmap was not available at plot time required conversion */
	পূর্ণসংখ্যা miss_গুনতি;
	uint64_t miss_আকার;
	/** Bitmap was available at plot time required no conversion */
	পূর্ণসংখ্যা hit_গুনতি;
	uint64_t hit_আকার;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	পূর্ণসংখ্যা fail_গুনতি;
	uint64_t fail_আকার;

	/* Cache entry freed without ever being redrawn */
	পূর্ণসংখ্যা যোগ_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	পূর্ণসংখ্যা specultive_miss_গুনতি;

	/** Total number of additional (after the first) conversions */
	পূর্ণসংখ্যা যোগ_extra_conversions;
	/** counts total number of images with more than one conversion */
	পূর্ণসংখ্যা যোগ_extra_conversions_গুনতি;

	/** Bitmap with most conversions was converted this many times */
	পূর্ণসংখ্যা peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned পূর্ণসংখ্যা peak_conversions_আকার;
};

/** image cache state */
স্থির কাঠামো image_cache_s *image_cache = ফাঁকা;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
স্থির কাঠামো image_cache_entry_s *image_cache__findn(পূর্ণসংখ্যা entryn)
{
	কাঠামো image_cache_entry_s *found;

	found = image_cache->entries;
	যখন ((found != ফাঁকা) && (entryn > 0)) {
		entryn--;
		found = found->পরবর্তী;
	}
	ফেরত found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
স্থির কাঠামো image_cache_entry_s *image_cache__find(ধ্রুবক কাঠামো content *c)
{
	কাঠামো image_cache_entry_s *found;

	found = image_cache->entries;
	যখন ((found != ফাঁকা) && (found->content != c)) {
		found = found->পরবর্তী;
	}
	ফেরত found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
স্থির শূন্য image_cache_stats_bitmap_add(কাঠামো image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_গুনতি++;

	image_cache->যোগ_bitmap_আকার += centry->bitmap_আকার;
	image_cache->bitmap_গুনতি++;

	যদি (image_cache->যোগ_bitmap_আকার > image_cache->max_bitmap_আকার) {
		image_cache->max_bitmap_আকার = image_cache->যোগ_bitmap_আকার;
		image_cache->max_bitmap_আকার_গুনতি = image_cache->bitmap_গুনতি;

	}

	যদি (image_cache->bitmap_গুনতি > image_cache->max_bitmap_গুনতি) {
		image_cache->max_bitmap_গুনতি = image_cache->bitmap_গুনতি;
		image_cache->max_bitmap_গুনতি_আকার = image_cache->যোগ_bitmap_আকার;
	}

	যদি (centry->conversion_গুনতি == 2) {
		image_cache->যোগ_extra_conversions_গুনতি++;
	}

	যদি (centry->conversion_গুনতি > 1) {
		image_cache->যোগ_extra_conversions++;
	}

	যদি ((centry->conversion_গুনতি > image_cache->peak_conversions) ||
	    (centry->conversion_গুনতি == image_cache->peak_conversions &&
	     centry->bitmap_আকার > image_cache->peak_conversions_আকার)) {
		image_cache->peak_conversions = centry->conversion_গুনতি;
		image_cache->peak_conversions_আকার = centry->bitmap_আকার;
	}
}

স্থির শূন্য image_cache__link(কাঠামো image_cache_entry_s *centry)
{
	centry->পরবর্তী = image_cache->entries;
	centry->পূর্ব = ফাঁকা;
	যদি (centry->পরবর্তী != ফাঁকা) {
		centry->পরবর্তী->পূর্ব = centry;
	}
	image_cache->entries = centry;
}

স্থির শূন্য image_cache__unlink(কাঠামো image_cache_entry_s *centry)
{
	/* unlink entry */
	যদি (centry->পূর্ব == ফাঁকা) {
		/* first in list */
		যদি (centry->পরবর্তী != ফাঁকা) {
			centry->পরবর্তী->পূর্ব = centry->পূর্ব;
			image_cache->entries = centry->পরবর্তী;
		} অন্যথা {
			/* empty list */
			image_cache->entries = ফাঁকা;
		}
	} অন্যথা {
		centry->পূর্ব->পরবর্তী = centry->পরবর্তী;

		যদি (centry->পরবর্তী != ফাঁকা) {
			centry->পরবর্তী->পূর্ব = centry->পূর্ব;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
স্থির শূন্য image_cache__free_bitmap(কাঠামো image_cache_entry_s *centry)
{
	যদি (centry->bitmap != ফাঁকা) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_আকার,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_গুনতি);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ফাঁকা;
		image_cache->যোগ_bitmap_আকার -= centry->bitmap_আকার;
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
স্থির শূন্য image_cache__free_entry(কাঠামো image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	যদি (centry->redraw_গুনতি == 0) {
		image_cache->যোগ_unrendered++;
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
স্থির শূন্য image_cache__clean(কাঠামো image_cache_s *icache)
{
	কাঠামো image_cache_entry_s *centry = icache->entries;

	যখন (centry != ফাঁকা) {
		যদি ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			যদি ((icache->যোগ_bitmap_আকার >
			     (icache->params.limit - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->পরবর্তী;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
স্থির শূন্য image_cache__background_update(শূন্য *p)
{
	কাঠামো image_cache_s *icache = p;

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
কাঠামো bitmap *image_cache_get_bitmap(ধ্রুবক কাঠামো content *c)
{
	কাঠামো image_cache_entry_s *centry;

	centry = image_cache__find(c);
	যদি (centry == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	যদি (centry->bitmap == ফাঁকা) {
		যদি (centry->convert != ফাঁকা) {
			centry->bitmap = centry->convert(centry->content);
		}

		যদি (centry->bitmap != ফাঁকা) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_গুনতি++;
			image_cache->miss_আকার += centry->bitmap_আকার;
		} অন্যথা {
			image_cache->fail_গুনতি++;
			image_cache->fail_আকার += centry->bitmap_আকার;
		}
	} অন্যথা {
		image_cache->hit_গুনতি++;
		image_cache->hit_আকার += centry->bitmap_আকার;
	}

	ফেরত centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(কাঠামো content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	যদি ((image_cache->যোগ_bitmap_আকার < image_cache->params.limit) &&
	    (c->আকার_2 <= image_cache->params.speculative_small)) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->আকার_2,
		      SPECULATE_SMALL);
#endif
		decision = true;
	}

#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#endif
	ফেরত decision;
}

/* exported interface documented in image_cache.h */
কাঠামো bitmap *image_cache_find_bitmap(কাঠামো content *c)
{
	কাঠামো image_cache_entry_s *centry;

	centry = image_cache__find(c);
	যদি (centry == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	ফেরত centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ধ্রুবক কাঠামো image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, আকার(কাঠামো image_cache_s));
	যদি (image_cache == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	ফেরত NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(শূন্য)
{
	unsigned পূর্ণসংখ্যা op_গুনতি;
	uint64_t op_আকার;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->যোগ_bitmap_আকার, image_cache->bitmap_গুনতি);

	যখন (image_cache->entries != ফাঁকা) {
		image_cache__free_entry(image_cache->entries);
	}

	op_গুনতি = image_cache->hit_গুনতি +
		image_cache->miss_গুনতি +
		image_cache->fail_গুনতি;

	op_আকার = image_cache->hit_আকার +
		image_cache->miss_আকার +
		image_cache->fail_আকার;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_আকার,
	      image_cache->max_bitmap_আকার_গুনতি);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_গুনতি,
	      image_cache->max_bitmap_গুনতি_আকার);

	যদি ((op_গুনতি > 0) && (op_আকার >0)) {

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
		      op_আকার,
		      image_cache->hit_আকার,
		      image_cache->miss_আকার,
		      image_cache->fail_আকার,
		      (image_cache->hit_আকার * 100) / op_আকার,
		      (image_cache->miss_আকার * 100) / op_আকার,
		      (image_cache->fail_আকার * 100) / op_আকার);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->যোগ_unrendered,
	      image_cache->specultive_miss_গুনতি);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->যোগ_extra_conversions,
	      image_cache->যোগ_extra_conversions_গুনতি);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_আকার,
	      image_cache->peak_conversions);

	মুক্ত(image_cache);

	ফেরত NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(কাঠামো content *content,
			কাঠামো bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	কাঠামো image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	যদি (centry == ফাঁকা) {
		/* new cache entry, content not previously added */
		centry = calloc(1, আকার(কাঠামো image_cache_entry_s));
		যদি (centry == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_আকার = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	যদি (bitmap != ফাঁকা) {
		যদি (centry->bitmap != ফাঁকা) {
			guit->bitmap->destroy(centry->bitmap);
		} অন্যথা {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} অন্যথা {
		/* no bitmap, check to see if we should speculatively convert */
		যদি ((centry->convert != ফাঁকা) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			যদি (centry->bitmap != ফাঁকা) {
				image_cache_stats_bitmap_add(centry);
			} অন্যথা {
				image_cache->fail_গুনতি++;
			}
		}
	}



	ফেরত NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(কাঠামো content *content)
{
	কাঠামো image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	যদি (centry == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ফেরত NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	ফেরত NSERROR_OK;
}

/* exported interface documented in image_cache.h */
পূর্ণসংখ্যা image_cache_snsummaryf(অক্ষর *string, size_t আকার_2, ধ্রুবক অক্ষর *fmt)
{
	size_t slen = 0; /* current output string length */
	পূর্ণসংখ্যা fmtc = 0; /* current index into format string */
	bool pct;
	unsigned পূর্ণসংখ্যা op_গুনতি;
	uint64_t op_আকার;

	op_গুনতি = image_cache->hit_গুনতি +
		image_cache->miss_গুনতি +
		image_cache->fail_গুনতি;

	op_আকার = image_cache->hit_আকার +
		image_cache->miss_আকার +
		image_cache->fail_আকার;

	যখন((slen < আকার_2) && (fmt[fmtc] != 0)) {
		যদি (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			যদি (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} অন্যথা {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) ক্ষেত্র chr : \
slen += snprintf(string + slen, আকার_2 - slen, "%"fmt, image_cache->var); বিরতি

#define FMTPCHR(chr,fmt,var,div) \
ক্ষেত্র chr :					\
	যদি (pct) {							\
		যদি (div > 0) {						\
			slen += snprintf(string + slen, আকার_2 - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} অন্যথা {						\
			slen += snprintf(string + slen, আকার_2 - slen, "100"); \
		}							\
	} অন্যথা {							\
		slen += snprintf(string + slen, আকার_2 - slen, "%"fmt, image_cache->var); \
	} বিরতি


			নির্বাচন (fmt[fmtc]) {
			ক্ষেত্র '%':
				string[slen] = '%';
				slen++;
				বিরতি;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, যোগ_bitmap_আকার);
			FMTCHR('d', "d", bitmap_গুনতি);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_আকার);
			FMTCHR('g', "d", max_bitmap_আকার_গুনতি);
			FMTCHR('h', "d", max_bitmap_গুনতি);
			FMTCHR('i', PRIsizet, max_bitmap_গুনতি_আকার);


			ক্ষেত্র 'j':
				slen += snprintf(string + slen, আকার_2 - slen,
						 "%u", pct?100:op_গুনতি);
				বিরতি;

			FMTPCHR('k', "d", hit_গুনতি, op_গুনতি);
			FMTPCHR('l', "d", miss_গুনতি, op_গুনতি);
			FMTPCHR('m', "d", fail_গুনতি, op_গুনতি);

			ক্ষেত্র 'n':
				slen += snprintf(string + slen, আকার_2 - slen,
						 "%"PRId64, pct?100:op_আকার);
				বিরতি;

			FMTPCHR('o', PRId64, hit_আকার, op_আকার);
			FMTPCHR('q', PRId64, miss_আকার, op_আকার);
			FMTPCHR('r', PRId64, fail_আকার, op_আকার);

			FMTCHR('s', "d", যোগ_unrendered);
			FMTCHR('t', "d", specultive_miss_গুনতি);
			FMTCHR('u', "d", যোগ_extra_conversions);
			FMTCHR('v', "d", যোগ_extra_conversions_গুনতি);
			FMTCHR('w', "u", peak_conversions_আকার);
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
	string[min(slen, আকার_2 - 1)] = '\0';

	ফেরত slen;
}

/* exported interface documented in image_cache.h */
পূর্ণসংখ্যা
image_cache_snentryf(অক্ষর *string,
		     size_t আকার_2,
		     unsigned পূর্ণসংখ্যা entryn,
		     ধ্রুবক অক্ষর *fmt)
{
	কাঠামো image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	পূর্ণসংখ্যা fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	যদি (centry == ফাঁকা)
		ফেরত -1;

	যখন((slen < আকার_2) && (fmt[fmtc] != 0)) {
		যদি (fmt[fmtc] == '%') {
			fmtc++;
			নির্বাচন (fmt[fmtc]) {
			ক্ষেত্র 'e':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%u", entryn);
				বিরতি;

			ক্ষেত্র 'r':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%u", centry->redraw_গুনতি);
				বিরতি;

			ক্ষেত্র 'a':
				slen += snprintf(string + slen, আকার_2 - slen,
						 "%.2f", (দশমিক)((image_cache->current_age -  centry->redraw_age)) / 1000);
				বিরতি;


			ক্ষেত্র 'c':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%d", centry->conversion_গুনতি);
				বিরতি;

			ক্ষেত্র 'g':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%.2f", (দশমিক)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				বিরতি;

			ক্ষেত্র 'k':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%p", centry->content);
				বিরতি;

			ক্ষেত্র 'U':
				slen += snprintf(string + slen, আকার_2 - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				বিরতি;

			ক্ষেত্র 'o':
				যদি (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							আকার_2 - slen, "%s",
							lwc_string_তথ্য(
								origin));

					lwc_string_unref(origin);
				} অন্যথা {
					slen += snprintf(string + slen,
							আকার_2 - slen, "%s",
							"localhost");
				}
				বিরতি;

			ক্ষেত্র 's':
				যদি (centry->bitmap != ফাঁকা) {
					slen += snprintf(string + slen,
							 আকার_2 - slen,
							 "%" PRIsizet,
							 centry->bitmap_আকার);
				} অন্যথা {
					slen += snprintf(string + slen,
							 আকার_2 - slen,
							 "0");
				}
				বিরতি;
			}
			fmtc++;
		} অন্যথা {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, আকার_2 - 1)] = '\0';

	ফেরত slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(কাঠামো content *c,
			কাঠামো content_redraw_তথ্য *তথ্য,
			ধ্রুবক কাঠামো rect *clip,
			ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	যদি (centry == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ফেরত false;
	}

	যদি (centry->bitmap == ফাঁকা) {
		যদি (centry->convert != ফাঁকা) {
			centry->bitmap = centry->convert(centry->content);
		}

		যদি (centry->bitmap != ফাঁকা) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_গুনতি++;
			image_cache->miss_আকার += centry->bitmap_আকার;
		} অন্যথা {
			image_cache->fail_গুনতি++;
			image_cache->fail_আকার += centry->bitmap_আকার;
			ফেরত false;
		}
	} অন্যথা {
		image_cache->hit_গুনতি++;
		image_cache->hit_আকার += centry->bitmap_আকার;
	}


	/* update statistics */
	centry->redraw_গুনতি++;
	centry->redraw_age = image_cache->current_age;

	ফেরত image_bitmap_plot(centry->bitmap, তথ্য, clip, ctx);
}

/* exported interface documented in image_cache.h */
শূন্য image_cache_destroy(কাঠামো content *content)
{
	কাঠামো image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	যদি (centry == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} অন্যথা {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
শূন্য *image_cache_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	ফেরত image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(কাঠামো content *c)
{
	কাঠামো bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	যদি (bmp != ফাঁকা) {
		ফেরত guit->bitmap->get_opaque(bmp);
	}
	ফেরত false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}
