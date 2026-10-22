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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "image/image_cache.h"
#അന്തര്ഭാവ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
തരനിർവചനം unsigned പൂർണ്ണസംഖ്യ cache_age;

/**
 * Image cache entry
 */
ഘടന image_cache_entry_s {
	ഘടന image_cache_entry_s *next; /**< next cache entry in list */
	ഘടന image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ഘടന content *content;
	/** associated bitmap entry */
	ഘടന bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned പൂർണ്ണസംഖ്യ redraw_എണ്ണം; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	പൂർണ്ണസംഖ്യ conversion_എണ്ണം; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ഘടന image_cache_s {
	/** Cache parameters */
	ഘടന image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ഘടന image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	പൂർണ്ണസംഖ്യ bitmap_എണ്ണം;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	പൂർണ്ണസംഖ്യ max_bitmap_size_എണ്ണം;

	/** Maximum count of bitmaps allocated at any one time */
	പൂർണ്ണസംഖ്യ max_bitmap_എണ്ണം;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_എണ്ണം_size;

	/** Bitmap was not available at plot time required conversion */
	പൂർണ്ണസംഖ്യ miss_എണ്ണം;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	പൂർണ്ണസംഖ്യ hit_എണ്ണം;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	പൂർണ്ണസംഖ്യ fail_എണ്ണം;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	പൂർണ്ണസംഖ്യ total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	പൂർണ്ണസംഖ്യ specultive_miss_എണ്ണം;

	/** Total number of additional (after the first) conversions */
	പൂർണ്ണസംഖ്യ total_extra_conversions;
	/** counts total number of images with more than one conversion */
	പൂർണ്ണസംഖ്യ total_extra_conversions_എണ്ണം;

	/** Bitmap with most conversions was converted this many times */
	പൂർണ്ണസംഖ്യ peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned പൂർണ്ണസംഖ്യ peak_conversions_size;
};

/** image cache state */
സ്ഥിതം ഘടന image_cache_s *image_cache = ശൂന്യ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
സ്ഥിതം ഘടന image_cache_entry_s *image_cache__findn(പൂർണ്ണസംഖ്യ entryn)
{
	ഘടന image_cache_entry_s *found;

	found = image_cache->entries;
	ആകുമ്പോൾ ((found != ശൂന്യ) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	തിരികെ found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
സ്ഥിതം ഘടന image_cache_entry_s *image_cache__find(സ്ഥിരം ഘടന content *c)
{
	ഘടന image_cache_entry_s *found;

	found = image_cache->entries;
	ആകുമ്പോൾ ((found != ശൂന്യ) && (found->content != c)) {
		found = found->next;
	}
	തിരികെ found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
സ്ഥിതം ശൂന്യം image_cache_stats_bitmap_add(ഘടന image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_എണ്ണം++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_എണ്ണം++;

	എങ്കിൽ (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_എണ്ണം = image_cache->bitmap_എണ്ണം;

	}

	എങ്കിൽ (image_cache->bitmap_എണ്ണം > image_cache->max_bitmap_എണ്ണം) {
		image_cache->max_bitmap_എണ്ണം = image_cache->bitmap_എണ്ണം;
		image_cache->max_bitmap_എണ്ണം_size = image_cache->total_bitmap_size;
	}

	എങ്കിൽ (centry->conversion_എണ്ണം == 2) {
		image_cache->total_extra_conversions_എണ്ണം++;
	}

	എങ്കിൽ (centry->conversion_എണ്ണം > 1) {
		image_cache->total_extra_conversions++;
	}

	എങ്കിൽ ((centry->conversion_എണ്ണം > image_cache->peak_conversions) ||
	    (centry->conversion_എണ്ണം == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_എണ്ണം;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

സ്ഥിതം ശൂന്യം image_cache__link(ഘടന image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = ശൂന്യ;
	എങ്കിൽ (centry->next != ശൂന്യ) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

സ്ഥിതം ശൂന്യം image_cache__unlink(ഘടന image_cache_entry_s *centry)
{
	/* unlink entry */
	എങ്കിൽ (centry->prev == ശൂന്യ) {
		/* first in list */
		എങ്കിൽ (centry->next != ശൂന്യ) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} അല്ലെങ്കിൽ {
			/* empty list */
			image_cache->entries = ശൂന്യ;
		}
	} അല്ലെങ്കിൽ {
		centry->prev->next = centry->next;

		എങ്കിൽ (centry->next != ശൂന്യ) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
സ്ഥിതം ശൂന്യം image_cache__free_bitmap(ഘടന image_cache_entry_s *centry)
{
	എങ്കിൽ (centry->bitmap != ശൂന്യ) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_എണ്ണം);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ശൂന്യ;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_എണ്ണം--;
		എങ്കിൽ (centry->redraw_എണ്ണം == 0) {
			image_cache->specultive_miss_എണ്ണം++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
സ്ഥിതം ശൂന്യം image_cache__free_entry(ഘടന image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	എങ്കിൽ (centry->redraw_എണ്ണം == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	സ്വതന്ത്രം(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
സ്ഥിതം ശൂന്യം image_cache__clean(ഘടന image_cache_s *icache)
{
	ഘടന image_cache_entry_s *centry = icache->entries;

	ആകുമ്പോൾ (centry != ശൂന്യ) {
		എങ്കിൽ ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			എങ്കിൽ ((icache->total_bitmap_size >
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
സ്ഥിതം ശൂന്യം image_cache__background_update(ശൂന്യം *p)
{
	ഘടന image_cache_s *icache = p;

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
ഘടന bitmap *image_cache_get_bitmap(സ്ഥിരം ഘടന content *c)
{
	ഘടന image_cache_entry_s *centry;

	centry = image_cache__find(c);
	എങ്കിൽ (centry == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (centry->bitmap == ശൂന്യ) {
		എങ്കിൽ (centry->convert != ശൂന്യ) {
			centry->bitmap = centry->convert(centry->content);
		}

		എങ്കിൽ (centry->bitmap != ശൂന്യ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_എണ്ണം++;
			image_cache->miss_size += centry->bitmap_size;
		} അല്ലെങ്കിൽ {
			image_cache->fail_എണ്ണം++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} അല്ലെങ്കിൽ {
		image_cache->hit_എണ്ണം++;
		image_cache->hit_size += centry->bitmap_size;
	}

	തിരികെ centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ഘടന content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	എങ്കിൽ ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	തിരികെ decision;
}

/* exported interface documented in image_cache.h */
ഘടന bitmap *image_cache_find_bitmap(ഘടന content *c)
{
	ഘടന image_cache_entry_s *centry;

	centry = image_cache__find(c);
	എങ്കിൽ (centry == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(സ്ഥിരം ഘടന image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, വലുപ്പം(ഘടന image_cache_s));
	എങ്കിൽ (image_cache == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	തിരികെ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(ശൂന്യം)
{
	unsigned പൂർണ്ണസംഖ്യ op_എണ്ണം;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_എണ്ണം);

	ആകുമ്പോൾ (image_cache->entries != ശൂന്യ) {
		image_cache__free_entry(image_cache->entries);
	}

	op_എണ്ണം = image_cache->hit_എണ്ണം +
		image_cache->miss_എണ്ണം +
		image_cache->fail_എണ്ണം;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_എണ്ണം);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_എണ്ണം,
	      image_cache->max_bitmap_എണ്ണം_size);

	എങ്കിൽ ((op_എണ്ണം > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_എണ്ണം,
		      image_cache->hit_എണ്ണം,
		      image_cache->miss_എണ്ണം,
		      image_cache->fail_എണ്ണം,
		      (image_cache->hit_എണ്ണം * 100) / op_എണ്ണം,
		      (image_cache->miss_എണ്ണം * 100) / op_എണ്ണം,
		      (image_cache->fail_എണ്ണം * 100) / op_എണ്ണം);
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
	      image_cache->specultive_miss_എണ്ണം);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_എണ്ണം);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	സ്വതന്ത്രം(image_cache);

	തിരികെ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(ഘടന content *content,
			ഘടന bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ഘടന image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	എങ്കിൽ (centry == ശൂന്യ) {
		/* new cache entry, content not previously added */
		centry = calloc(1, വലുപ്പം(ഘടന image_cache_entry_s));
		എങ്കിൽ (centry == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	എങ്കിൽ (bitmap != ശൂന്യ) {
		എങ്കിൽ (centry->bitmap != ശൂന്യ) {
			guit->bitmap->destroy(centry->bitmap);
		} അല്ലെങ്കിൽ {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} അല്ലെങ്കിൽ {
		/* no bitmap, check to see if we should speculatively convert */
		എങ്കിൽ ((centry->convert != ശൂന്യ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			എങ്കിൽ (centry->bitmap != ശൂന്യ) {
				image_cache_stats_bitmap_add(centry);
			} അല്ലെങ്കിൽ {
				image_cache->fail_എണ്ണം++;
			}
		}
	}



	തിരികെ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ഘടന content *content)
{
	ഘടന image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	എങ്കിൽ (centry == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		തിരികെ NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	തിരികെ NSERROR_OK;
}

/* exported interface documented in image_cache.h */
പൂർണ്ണസംഖ്യ image_cache_snsummaryf(അക്ഷരം *string, size_t size, സ്ഥിരം അക്ഷരം *fmt)
{
	size_t slen = 0; /* current output string length */
	പൂർണ്ണസംഖ്യ fmtc = 0; /* current index into format string */
	bool pct;
	unsigned പൂർണ്ണസംഖ്യ op_എണ്ണം;
	uint64_t op_size;

	op_എണ്ണം = image_cache->hit_എണ്ണം +
		image_cache->miss_എണ്ണം +
		image_cache->fail_എണ്ണം;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	ആകുമ്പോൾ((slen < size) && (fmt[fmtc] != 0)) {
		എങ്കിൽ (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			എങ്കിൽ (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} അല്ലെങ്കിൽ {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) അവസ്ഥ chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); നിർത്തുക

#define FMTPCHR(chr,fmt,var,div) \
അവസ്ഥ chr :					\
	എങ്കിൽ (pct) {							\
		എങ്കിൽ (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} അല്ലെങ്കിൽ {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} അല്ലെങ്കിൽ {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} നിർത്തുക


			തിരഞ്ഞെടുപ്പ് (fmt[fmtc]) {
			അവസ്ഥ '%':
				string[slen] = '%';
				slen++;
				നിർത്തുക;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_എണ്ണം);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_എണ്ണം);
			FMTCHR('h', "d", max_bitmap_എണ്ണം);
			FMTCHR('i', PRIsizet, max_bitmap_എണ്ണം_size);


			അവസ്ഥ 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_എണ്ണം);
				നിർത്തുക;

			FMTPCHR('k', "d", hit_എണ്ണം, op_എണ്ണം);
			FMTPCHR('l', "d", miss_എണ്ണം, op_എണ്ണം);
			FMTPCHR('m', "d", fail_എണ്ണം, op_എണ്ണം);

			അവസ്ഥ 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				നിർത്തുക;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_എണ്ണം);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_എണ്ണം);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} അല്ലെങ്കിൽ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	തിരികെ slen;
}

/* exported interface documented in image_cache.h */
പൂർണ്ണസംഖ്യ
image_cache_snentryf(അക്ഷരം *string,
		     size_t size,
		     unsigned പൂർണ്ണസംഖ്യ entryn,
		     സ്ഥിരം അക്ഷരം *fmt)
{
	ഘടന image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	പൂർണ്ണസംഖ്യ fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	എങ്കിൽ (centry == ശൂന്യ)
		തിരികെ -1;

	ആകുമ്പോൾ((slen < size) && (fmt[fmtc] != 0)) {
		എങ്കിൽ (fmt[fmtc] == '%') {
			fmtc++;
			തിരഞ്ഞെടുപ്പ് (fmt[fmtc]) {
			അവസ്ഥ 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				നിർത്തുക;

			അവസ്ഥ 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_എണ്ണം);
				നിർത്തുക;

			അവസ്ഥ 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (ദശാംശം)((image_cache->current_age -  centry->redraw_age)) / 1000);
				നിർത്തുക;


			അവസ്ഥ 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_എണ്ണം);
				നിർത്തുക;

			അവസ്ഥ 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (ദശാംശം)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				നിർത്തുക;

			അവസ്ഥ 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				നിർത്തുക;

			അവസ്ഥ 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				നിർത്തുക;

			അവസ്ഥ 'o':
				എങ്കിൽ (nsurl_has_component(llcache_handle_get_url(
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
				} അല്ലെങ്കിൽ {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				നിർത്തുക;

			അവസ്ഥ 's':
				എങ്കിൽ (centry->bitmap != ശൂന്യ) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} അല്ലെങ്കിൽ {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				നിർത്തുക;
			}
			fmtc++;
		} അല്ലെങ്കിൽ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	തിരികെ slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ഘടന content *c,
			ഘടന content_redraw_data *data,
			സ്ഥിരം ഘടന rect *clip,
			സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	എങ്കിൽ (centry == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		തിരികെ false;
	}

	എങ്കിൽ (centry->bitmap == ശൂന്യ) {
		എങ്കിൽ (centry->convert != ശൂന്യ) {
			centry->bitmap = centry->convert(centry->content);
		}

		എങ്കിൽ (centry->bitmap != ശൂന്യ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_എണ്ണം++;
			image_cache->miss_size += centry->bitmap_size;
		} അല്ലെങ്കിൽ {
			image_cache->fail_എണ്ണം++;
			image_cache->fail_size += centry->bitmap_size;
			തിരികെ false;
		}
	} അല്ലെങ്കിൽ {
		image_cache->hit_എണ്ണം++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_എണ്ണം++;
	centry->redraw_age = image_cache->current_age;

	തിരികെ image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
ശൂന്യം image_cache_destroy(ഘടന content *content)
{
	ഘടന image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	എങ്കിൽ (centry == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} അല്ലെങ്കിൽ {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
ശൂന്യം *image_cache_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	തിരികെ image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ഘടന content *c)
{
	ഘടന bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	എങ്കിൽ (bmp != ശൂന്യ) {
		തിരികെ guit->bitmap->get_opaque(bmp);
	}
	തിരികെ false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}
