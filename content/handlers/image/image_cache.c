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

#شامل <assert.h>
#شامل <stdint.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <stdlib.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/bitmap.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "desktop/gui_internal.h"

#شامل "image/image_cache.h"
#شامل "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
تعریفنوع unsigned صحیح cache_age;

/**
 * Image cache entry
 */
ساختار image_cache_entry_s {
	ساختار image_cache_entry_s *next; /**< next cache entry in list */
	ساختار image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	ساختار content *content;
	/** associated bitmap entry */
	ساختار bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned صحیح redraw_شمار; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	صحیح conversion_شمار; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ساختار image_cache_s {
	/** Cache parameters */
	ساختار image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ساختار image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	صحیح bitmap_شمار;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	صحیح max_bitmap_size_شمار;

	/** Maximum count of bitmaps allocated at any one time */
	صحیح max_bitmap_شمار;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_شمار_size;

	/** Bitmap was not available at plot time required conversion */
	صحیح miss_شمار;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	صحیح hit_شمار;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	صحیح fail_شمار;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	صحیح total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	صحیح specultive_miss_شمار;

	/** Total number of additional (after the first) conversions */
	صحیح total_extra_conversions;
	/** counts total number of images with more than one conversion */
	صحیح total_extra_conversions_شمار;

	/** Bitmap with most conversions was converted this many times */
	صحیح peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned صحیح peak_conversions_size;
};

/** image cache state */
ایستا ساختار image_cache_s *image_cache = پوچ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ایستا ساختار image_cache_entry_s *image_cache__findn(صحیح entryn)
{
	ساختار image_cache_entry_s *found;

	found = image_cache->entries;
	وقتی ((found != پوچ) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	بازگشت found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ایستا ساختار image_cache_entry_s *image_cache__find(ثابت ساختار content *c)
{
	ساختار image_cache_entry_s *found;

	found = image_cache->entries;
	وقتی ((found != پوچ) && (found->content != c)) {
		found = found->next;
	}
	بازگشت found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ایستا تهی image_cache_stats_bitmap_add(ساختار image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_شمار++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_شمار++;

	اگر (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_شمار = image_cache->bitmap_شمار;

	}

	اگر (image_cache->bitmap_شمار > image_cache->max_bitmap_شمار) {
		image_cache->max_bitmap_شمار = image_cache->bitmap_شمار;
		image_cache->max_bitmap_شمار_size = image_cache->total_bitmap_size;
	}

	اگر (centry->conversion_شمار == 2) {
		image_cache->total_extra_conversions_شمار++;
	}

	اگر (centry->conversion_شمار > 1) {
		image_cache->total_extra_conversions++;
	}

	اگر ((centry->conversion_شمار > image_cache->peak_conversions) ||
	    (centry->conversion_شمار == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_شمار;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

ایستا تهی image_cache__link(ساختار image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = پوچ;
	اگر (centry->next != پوچ) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

ایستا تهی image_cache__unlink(ساختار image_cache_entry_s *centry)
{
	/* unlink entry */
	اگر (centry->prev == پوچ) {
		/* first in list */
		اگر (centry->next != پوچ) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} وگرنه {
			/* empty list */
			image_cache->entries = پوچ;
		}
	} وگرنه {
		centry->prev->next = centry->next;

		اگر (centry->next != پوچ) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ایستا تهی image_cache__free_bitmap(ساختار image_cache_entry_s *centry)
{
	اگر (centry->bitmap != پوچ) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_شمار);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = پوچ;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_شمار--;
		اگر (centry->redraw_شمار == 0) {
			image_cache->specultive_miss_شمار++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ایستا تهی image_cache__free_entry(ساختار image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	اگر (centry->redraw_شمار == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	آزاد(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ایستا تهی image_cache__clean(ساختار image_cache_s *icache)
{
	ساختار image_cache_entry_s *centry = icache->entries;

	وقتی (centry != پوچ) {
		اگر ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			اگر ((icache->total_bitmap_size >
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
ایستا تهی image_cache__background_update(تهی *p)
{
	ساختار image_cache_s *icache = p;

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
ساختار bitmap *image_cache_get_bitmap(ثابت ساختار content *c)
{
	ساختار image_cache_entry_s *centry;

	centry = image_cache__find(c);
	اگر (centry == پوچ) {
		بازگشت پوچ;
	}

	اگر (centry->bitmap == پوچ) {
		اگر (centry->convert != پوچ) {
			centry->bitmap = centry->convert(centry->content);
		}

		اگر (centry->bitmap != پوچ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_شمار++;
			image_cache->miss_size += centry->bitmap_size;
		} وگرنه {
			image_cache->fail_شمار++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} وگرنه {
		image_cache->hit_شمار++;
		image_cache->hit_size += centry->bitmap_size;
	}

	بازگشت centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ساختار content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	اگر ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	بازگشت decision;
}

/* exported interface documented in image_cache.h */
ساختار bitmap *image_cache_find_bitmap(ساختار content *c)
{
	ساختار image_cache_entry_s *centry;

	centry = image_cache__find(c);
	اگر (centry == پوچ) {
		بازگشت پوچ;
	}

	بازگشت centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ثابت ساختار image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, اندازه(ساختار image_cache_s));
	اگر (image_cache == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	بازگشت NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(تهی)
{
	unsigned صحیح op_شمار;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_شمار);

	وقتی (image_cache->entries != پوچ) {
		image_cache__free_entry(image_cache->entries);
	}

	op_شمار = image_cache->hit_شمار +
		image_cache->miss_شمار +
		image_cache->fail_شمار;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_شمار);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_شمار,
	      image_cache->max_bitmap_شمار_size);

	اگر ((op_شمار > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_شمار,
		      image_cache->hit_شمار,
		      image_cache->miss_شمار,
		      image_cache->fail_شمار,
		      (image_cache->hit_شمار * 100) / op_شمار,
		      (image_cache->miss_شمار * 100) / op_شمار,
		      (image_cache->fail_شمار * 100) / op_شمار);
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
	      image_cache->specultive_miss_شمار);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_شمار);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	آزاد(image_cache);

	بازگشت NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(ساختار content *content,
			ساختار bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ساختار image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	اگر (centry == پوچ) {
		/* new cache entry, content not previously added */
		centry = calloc(1, اندازه(ساختار image_cache_entry_s));
		اگر (centry == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	اگر (bitmap != پوچ) {
		اگر (centry->bitmap != پوچ) {
			guit->bitmap->destroy(centry->bitmap);
		} وگرنه {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} وگرنه {
		/* no bitmap, check to see if we should speculatively convert */
		اگر ((centry->convert != پوچ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			اگر (centry->bitmap != پوچ) {
				image_cache_stats_bitmap_add(centry);
			} وگرنه {
				image_cache->fail_شمار++;
			}
		}
	}



	بازگشت NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ساختار content *content)
{
	ساختار image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	اگر (centry == پوچ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		بازگشت NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	بازگشت NSERROR_OK;
}

/* exported interface documented in image_cache.h */
صحیح image_cache_snsummaryf(نویسه *string, size_t size, ثابت نویسه *fmt)
{
	size_t slen = 0; /* current output string length */
	صحیح fmtc = 0; /* current index into format string */
	bool pct;
	unsigned صحیح op_شمار;
	uint64_t op_size;

	op_شمار = image_cache->hit_شمار +
		image_cache->miss_شمار +
		image_cache->fail_شمار;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	وقتی((slen < size) && (fmt[fmtc] != 0)) {
		اگر (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			اگر (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} وگرنه {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) حالت chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); بشکن

#define FMTPCHR(chr,fmt,var,div) \
حالت chr :					\
	اگر (pct) {							\
		اگر (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} وگرنه {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} وگرنه {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} بشکن


			انتخاب (fmt[fmtc]) {
			حالت '%':
				string[slen] = '%';
				slen++;
				بشکن;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_شمار);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_شمار);
			FMTCHR('h', "d", max_bitmap_شمار);
			FMTCHR('i', PRIsizet, max_bitmap_شمار_size);


			حالت 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_شمار);
				بشکن;

			FMTPCHR('k', "d", hit_شمار, op_شمار);
			FMTPCHR('l', "d", miss_شمار, op_شمار);
			FMTPCHR('m', "d", fail_شمار, op_شمار);

			حالت 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				بشکن;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_شمار);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_شمار);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} وگرنه {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	بازگشت slen;
}

/* exported interface documented in image_cache.h */
صحیح
image_cache_snentryf(نویسه *string,
		     size_t size,
		     unsigned صحیح entryn,
		     ثابت نویسه *fmt)
{
	ساختار image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	صحیح fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	اگر (centry == پوچ)
		بازگشت -1;

	وقتی((slen < size) && (fmt[fmtc] != 0)) {
		اگر (fmt[fmtc] == '%') {
			fmtc++;
			انتخاب (fmt[fmtc]) {
			حالت 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				بشکن;

			حالت 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_شمار);
				بشکن;

			حالت 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (اعشاری)((image_cache->current_age -  centry->redraw_age)) / 1000);
				بشکن;


			حالت 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_شمار);
				بشکن;

			حالت 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (اعشاری)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				بشکن;

			حالت 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				بشکن;

			حالت 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				بشکن;

			حالت 'o':
				اگر (nsurl_has_component(llcache_handle_get_url(
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
				} وگرنه {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				بشکن;

			حالت 's':
				اگر (centry->bitmap != پوچ) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} وگرنه {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				بشکن;
			}
			fmtc++;
		} وگرنه {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	بازگشت slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ساختار content *c,
			ساختار content_redraw_data *data,
			ثابت ساختار rect *clip,
			ثابت ساختار redraw_context *ctx)
{
	ساختار image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	اگر (centry == پوچ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		بازگشت false;
	}

	اگر (centry->bitmap == پوچ) {
		اگر (centry->convert != پوچ) {
			centry->bitmap = centry->convert(centry->content);
		}

		اگر (centry->bitmap != پوچ) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_شمار++;
			image_cache->miss_size += centry->bitmap_size;
		} وگرنه {
			image_cache->fail_شمار++;
			image_cache->fail_size += centry->bitmap_size;
			بازگشت false;
		}
	} وگرنه {
		image_cache->hit_شمار++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_شمار++;
	centry->redraw_age = image_cache->current_age;

	بازگشت image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
تهی image_cache_destroy(ساختار content *content)
{
	ساختار image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	اگر (centry == پوچ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} وگرنه {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
تهی *image_cache_get_internal(ثابت ساختار content *c, تهی *context)
{
	بازگشت image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ساختار content *c)
{
	ساختار bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	اگر (bmp != پوچ) {
		بازگشت guit->bitmap->get_opaque(bmp);
	}
	بازگشت false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(تهی)
{
	بازگشت CONTENT_IMAGE;
}
