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

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "netsurf/inttypes.h"
#include "utils/utils.h"
#include "utils/log.h"
#include "netsurf/misc.h"
#include "netsurf/bitmap.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "desktop/gui_internal.h"

#include "image/image_cache.h"
#include "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
typedef unsigned පූර්ණ_සංඛ්‍යා cache_age;

/**
 * Image cache entry
 */
sa_nracanaa image_cache_entry_s {
	sa_nracanaa image_cache_entry_s *next; /**< next cache entry in list */
	sa_nracanaa image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	sa_nracanaa content *content;
	/** associated bitmap entry */
	sa_nracanaa bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned පූර්ණ_සංඛ්‍යා redraw_count; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	පූර්ණ_සංඛ්‍යා conversion_count; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
sa_nracanaa image_cache_s {
	/** Cache parameters */
	sa_nracanaa image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	sa_nracanaa image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	පූර්ණ_සංඛ්‍යා bitmap_count;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	පූර්ණ_සංඛ්‍යා max_bitmap_size_count;

	/** Maximum count of bitmaps allocated at any one time */
	පූර්ණ_සංඛ්‍යා max_bitmap_count;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_count_size;

	/** Bitmap was not available at plot time required conversion */
	පූර්ණ_සංඛ්‍යා miss_count;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	පූර්ණ_සංඛ්‍යා hit_count;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	පූර්ණ_සංඛ්‍යා fail_count;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	පූර්ණ_සංඛ්‍යා total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	පූර්ණ_සංඛ්‍යා specultive_miss_count;

	/** Total number of additional (after the first) conversions */
	පූර්ණ_සංඛ්‍යා total_extra_conversions;
	/** counts total number of images with more than one conversion */
	පූර්ණ_සංඛ්‍යා total_extra_conversions_count;

	/** Bitmap with most conversions was converted this many times */
	පූර්ණ_සංඛ්‍යා peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned පූර්ණ_සංඛ්‍යා peak_conversions_size;
};

/** image cache state */
static sa_nracanaa image_cache_s *image_cache = NULL;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
static sa_nracanaa image_cache_entry_s *image_cache__findn(පූර්ණ_සංඛ්‍යා entryn)
{
	sa_nracanaa image_cache_entry_s *found;

	found = image_cache->entries;
	අතරතුර ((found != NULL) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	ආපසු found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
static sa_nracanaa image_cache_entry_s *image_cache__find(const sa_nracanaa content *c)
{
	sa_nracanaa image_cache_entry_s *found;

	found = image_cache->entries;
	අතරතුර ((found != NULL) && (found->content != c)) {
		found = found->next;
	}
	ආපසු found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
static void image_cache_stats_bitmap_add(sa_nracanaa image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_count++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_count++;

	නම් (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_count = image_cache->bitmap_count;

	}

	නම් (image_cache->bitmap_count > image_cache->max_bitmap_count) {
		image_cache->max_bitmap_count = image_cache->bitmap_count;
		image_cache->max_bitmap_count_size = image_cache->total_bitmap_size;
	}

	නම් (centry->conversion_count == 2) {
		image_cache->total_extra_conversions_count++;
	}

	නම් (centry->conversion_count > 1) {
		image_cache->total_extra_conversions++;
	}

	නම් ((centry->conversion_count > image_cache->peak_conversions) ||
	    (centry->conversion_count == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_count;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

static void image_cache__link(sa_nracanaa image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = NULL;
	නම් (centry->next != NULL) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

static void image_cache__unlink(sa_nracanaa image_cache_entry_s *centry)
{
	/* unlink entry */
	නම් (centry->prev == NULL) {
		/* first in list */
		නම් (centry->next != NULL) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} නැතහොත් {
			/* empty list */
			image_cache->entries = NULL;
		}
	} නැතහොත් {
		centry->prev->next = centry->next;

		නම් (centry->next != NULL) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
static void image_cache__free_bitmap(sa_nracanaa image_cache_entry_s *centry)
{
	නම් (centry->bitmap != NULL) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_count);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = NULL;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_count--;
		නම් (centry->redraw_count == 0) {
			image_cache->specultive_miss_count++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
static void image_cache__free_entry(sa_nracanaa image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	නම් (centry->redraw_count == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	free(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
static void image_cache__clean(sa_nracanaa image_cache_s *icache)
{
	sa_nracanaa image_cache_entry_s *centry = icache->entries;

	අතරතුර (centry != NULL) {
		නම් ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			නම් ((icache->total_bitmap_size >
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
static void image_cache__background_update(void *p)
{
	sa_nracanaa image_cache_s *icache = p;

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
sa_nracanaa bitmap *image_cache_get_bitmap(const sa_nracanaa content *c)
{
	sa_nracanaa image_cache_entry_s *centry;

	centry = image_cache__find(c);
	නම් (centry == NULL) {
		ආපසු NULL;
	}

	නම් (centry->bitmap == NULL) {
		නම් (centry->convert != NULL) {
			centry->bitmap = centry->convert(centry->content);
		}

		නම් (centry->bitmap != NULL) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_count++;
			image_cache->miss_size += centry->bitmap_size;
		} නැතහොත් {
			image_cache->fail_count++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} නැතහොත් {
		image_cache->hit_count++;
		image_cache->hit_size += centry->bitmap_size;
	}

	ආපසු centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(sa_nracanaa content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	නම් ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	ආපසු decision;
}

/* exported interface documented in image_cache.h */
sa_nracanaa bitmap *image_cache_find_bitmap(sa_nracanaa content *c)
{
	sa_nracanaa image_cache_entry_s *centry;

	centry = image_cache__find(c);
	නම් (centry == NULL) {
		ආපසු NULL;
	}

	ආපසු centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(const sa_nracanaa image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, parimaa_na(sa_nracanaa image_cache_s));
	නම් (image_cache == NULL) {
		ආපසු NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	ආපසු NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(void)
{
	unsigned පූර්ණ_සංඛ්‍යා op_count;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_count);

	අතරතුර (image_cache->entries != NULL) {
		image_cache__free_entry(image_cache->entries);
	}

	op_count = image_cache->hit_count +
		image_cache->miss_count +
		image_cache->fail_count;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_count);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_count,
	      image_cache->max_bitmap_count_size);

	නම් ((op_count > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_count,
		      image_cache->hit_count,
		      image_cache->miss_count,
		      image_cache->fail_count,
		      (image_cache->hit_count * 100) / op_count,
		      (image_cache->miss_count * 100) / op_count,
		      (image_cache->fail_count * 100) / op_count);
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
	      image_cache->specultive_miss_count);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_count);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	free(image_cache);

	ආපසු NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(sa_nracanaa content *content,
			sa_nracanaa bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	sa_nracanaa image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	නම් (centry == NULL) {
		/* new cache entry, content not previously added */
		centry = calloc(1, parimaa_na(sa_nracanaa image_cache_entry_s));
		නම් (centry == NULL) {
			ආපසු NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	නම් (bitmap != NULL) {
		නම් (centry->bitmap != NULL) {
			guit->bitmap->destroy(centry->bitmap);
		} නැතහොත් {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} නැතහොත් {
		/* no bitmap, check to see if we should speculatively convert */
		නම් ((centry->convert != NULL) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			නම් (centry->bitmap != NULL) {
				image_cache_stats_bitmap_add(centry);
			} නැතහොත් {
				image_cache->fail_count++;
			}
		}
	}



	ආපසු NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(sa_nracanaa content *content)
{
	sa_nracanaa image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	නම් (centry == NULL) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		ආපසු NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	ආපසු NSERROR_OK;
}

/* exported interface documented in image_cache.h */
පූර්ණ_සංඛ්‍යා image_cache_snsummaryf(var_na *string, size_t size, const var_na *fmt)
{
	size_t slen = 0; /* current output string length */
	පූර්ණ_සංඛ්‍යා fmtc = 0; /* current index into format string */
	bool pct;
	unsigned පූර්ණ_සංඛ්‍යා op_count;
	uint64_t op_size;

	op_count = image_cache->hit_count +
		image_cache->miss_count +
		image_cache->fail_count;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	අතරතුර((slen < size) && (fmt[fmtc] != 0)) {
		නම් (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			නම් (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} නැතහොත් {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) pak_sa chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); bha_nga

#define FMTPCHR(chr,fmt,var,div) \
pak_sa chr :					\
	නම් (pct) {							\
		නම් (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} නැතහොත් {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} නැතහොත් {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} bha_nga


			switch (fmt[fmtc]) {
			pak_sa '%':
				string[slen] = '%';
				slen++;
				bha_nga;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_count);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_count);
			FMTCHR('h', "d", max_bitmap_count);
			FMTCHR('i', PRIsizet, max_bitmap_count_size);


			pak_sa 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_count);
				bha_nga;

			FMTPCHR('k', "d", hit_count, op_count);
			FMTPCHR('l', "d", miss_count, op_count);
			FMTPCHR('m', "d", fail_count, op_count);

			pak_sa 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				bha_nga;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_count);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_count);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} නැතහොත් {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ආපසු slen;
}

/* exported interface documented in image_cache.h */
පූර්ණ_සංඛ්‍යා
image_cache_snentryf(var_na *string,
		     size_t size,
		     unsigned පූර්ණ_සංඛ්‍යා entryn,
		     const var_na *fmt)
{
	sa_nracanaa image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	පූර්ණ_සංඛ්‍යා fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	නම් (centry == NULL)
		ආපසු -1;

	අතරතුර((slen < size) && (fmt[fmtc] != 0)) {
		නම් (fmt[fmtc] == '%') {
			fmtc++;
			switch (fmt[fmtc]) {
			pak_sa 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				bha_nga;

			pak_sa 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_count);
				bha_nga;

			pak_sa 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (float)((image_cache->current_age -  centry->redraw_age)) / 1000);
				bha_nga;


			pak_sa 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_count);
				bha_nga;

			pak_sa 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (float)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				bha_nga;

			pak_sa 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				bha_nga;

			pak_sa 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				bha_nga;

			pak_sa 'o':
				නම් (nsurl_has_component(llcache_handle_get_url(
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
				} නැතහොත් {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				bha_nga;

			pak_sa 's':
				නම් (centry->bitmap != NULL) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} නැතහොත් {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				bha_nga;
			}
			fmtc++;
		} නැතහොත් {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	ආපසු slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(sa_nracanaa content *c,
			sa_nracanaa content_redraw_data *data,
			const sa_nracanaa rect *clip,
			const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	නම් (centry == NULL) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		ආපසු false;
	}

	නම් (centry->bitmap == NULL) {
		නම් (centry->convert != NULL) {
			centry->bitmap = centry->convert(centry->content);
		}

		නම් (centry->bitmap != NULL) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_count++;
			image_cache->miss_size += centry->bitmap_size;
		} නැතහොත් {
			image_cache->fail_count++;
			image_cache->fail_size += centry->bitmap_size;
			ආපසු false;
		}
	} නැතහොත් {
		image_cache->hit_count++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_count++;
	centry->redraw_age = image_cache->current_age;

	ආපසු image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
void image_cache_destroy(sa_nracanaa content *content)
{
	sa_nracanaa image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	නම් (centry == NULL) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} නැතහොත් {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
void *image_cache_get_internal(const sa_nracanaa content *c, void *context)
{
	ආපසු image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(sa_nracanaa content *c)
{
	sa_nracanaa bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	නම් (bmp != NULL) {
		ආපසු guit->bitmap->get_opaque(bmp);
	}
	ආපසු false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(void)
{
	ආපසු CONTENT_IMAGE;
}
