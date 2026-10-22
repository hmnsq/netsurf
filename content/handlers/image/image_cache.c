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

#ضمن <assert.h>
#ضمن <stdint.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <stdlib.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "netsurf/misc.h"
#ضمن "netsurf/bitmap.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "desktop/gui_internal.h"

#ضمن "image/image_cache.h"
#ضمن "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
تعريفنوع unsigned صحيح cache_age;

/**
 * Image cache entry
 */
بنية image_cache_entry_s {
	بنية image_cache_entry_s *next; /**< next cache entry in list */
	بنية image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	بنية content *content;
	/** associated bitmap entry */
	بنية bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned صحيح redraw_count; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	صحيح conversion_count; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
بنية image_cache_s {
	/** Cache parameters */
	بنية image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	بنية image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	صحيح bitmap_count;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	صحيح max_bitmap_size_count;

	/** Maximum count of bitmaps allocated at any one time */
	صحيح max_bitmap_count;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_count_size;

	/** Bitmap was not available at plot time required conversion */
	صحيح miss_count;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	صحيح hit_count;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	صحيح fail_count;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	صحيح total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	صحيح specultive_miss_count;

	/** Total number of additional (after the first) conversions */
	صحيح total_extra_conversions;
	/** counts total number of images with more than one conversion */
	صحيح total_extra_conversions_count;

	/** Bitmap with most conversions was converted this many times */
	صحيح peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned صحيح peak_conversions_size;
};

/** image cache state */
ساكن بنية image_cache_s *image_cache = عدم;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ساكن بنية image_cache_entry_s *image_cache__findn(صحيح entryn)
{
	بنية image_cache_entry_s *found;

	found = image_cache->entries;
	طالما ((found != عدم) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	أرجع found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ساكن بنية image_cache_entry_s *image_cache__find(ثابت بنية content *c)
{
	بنية image_cache_entry_s *found;

	found = image_cache->entries;
	طالما ((found != عدم) && (found->content != c)) {
		found = found->next;
	}
	أرجع found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ساكن فراغ image_cache_stats_bitmap_add(بنية image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_count++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_count++;

	𐤀𐤌 (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_count = image_cache->bitmap_count;

	}

	𐤀𐤌 (image_cache->bitmap_count > image_cache->max_bitmap_count) {
		image_cache->max_bitmap_count = image_cache->bitmap_count;
		image_cache->max_bitmap_count_size = image_cache->total_bitmap_size;
	}

	𐤀𐤌 (centry->conversion_count == 2) {
		image_cache->total_extra_conversions_count++;
	}

	𐤀𐤌 (centry->conversion_count > 1) {
		image_cache->total_extra_conversions++;
	}

	𐤀𐤌 ((centry->conversion_count > image_cache->peak_conversions) ||
	    (centry->conversion_count == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_count;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

ساكن فراغ image_cache__link(بنية image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = عدم;
	𐤀𐤌 (centry->next != عدم) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

ساكن فراغ image_cache__unlink(بنية image_cache_entry_s *centry)
{
	/* unlink entry */
	𐤀𐤌 (centry->prev == عدم) {
		/* first in list */
		𐤀𐤌 (centry->next != عدم) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} وإلا {
			/* empty list */
			image_cache->entries = عدم;
		}
	} وإلا {
		centry->prev->next = centry->next;

		𐤀𐤌 (centry->next != عدم) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ساكن فراغ image_cache__free_bitmap(بنية image_cache_entry_s *centry)
{
	𐤀𐤌 (centry->bitmap != عدم) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_count);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = عدم;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_count--;
		𐤀𐤌 (centry->redraw_count == 0) {
			image_cache->specultive_miss_count++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ساكن فراغ image_cache__free_entry(بنية image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	𐤀𐤌 (centry->redraw_count == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	حرر(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
ساكن فراغ image_cache__clean(بنية image_cache_s *icache)
{
	بنية image_cache_entry_s *centry = icache->entries;

	طالما (centry != عدم) {
		𐤀𐤌 ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			𐤀𐤌 ((icache->total_bitmap_size >
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
ساكن فراغ image_cache__background_update(فراغ *p)
{
	بنية image_cache_s *icache = p;

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
بنية bitmap *image_cache_get_bitmap(ثابت بنية content *c)
{
	بنية image_cache_entry_s *centry;

	centry = image_cache__find(c);
	𐤀𐤌 (centry == عدم) {
		أرجع عدم;
	}

	𐤀𐤌 (centry->bitmap == عدم) {
		𐤀𐤌 (centry->convert != عدم) {
			centry->bitmap = centry->convert(centry->content);
		}

		𐤀𐤌 (centry->bitmap != عدم) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_count++;
			image_cache->miss_size += centry->bitmap_size;
		} وإلا {
			image_cache->fail_count++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} وإلا {
		image_cache->hit_count++;
		image_cache->hit_size += centry->bitmap_size;
	}

	أرجع centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(بنية content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	𐤀𐤌 ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	أرجع decision;
}

/* exported interface documented in image_cache.h */
بنية bitmap *image_cache_find_bitmap(بنية content *c)
{
	بنية image_cache_entry_s *centry;

	centry = image_cache__find(c);
	𐤀𐤌 (centry == عدم) {
		أرجع عدم;
	}

	أرجع centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ثابت بنية image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, حجم(بنية image_cache_s));
	𐤀𐤌 (image_cache == عدم) {
		أرجع NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(فراغ)
{
	unsigned صحيح op_count;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_count);

	طالما (image_cache->entries != عدم) {
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

	𐤀𐤌 ((op_count > 0) && (op_size >0)) {

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

	حرر(image_cache);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(بنية content *content,
			بنية bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	بنية image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	𐤀𐤌 (centry == عدم) {
		/* new cache entry, content not previously added */
		centry = calloc(1, حجم(بنية image_cache_entry_s));
		𐤀𐤌 (centry == عدم) {
			أرجع NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	𐤀𐤌 (bitmap != عدم) {
		𐤀𐤌 (centry->bitmap != عدم) {
			guit->bitmap->destroy(centry->bitmap);
		} وإلا {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} وإلا {
		/* no bitmap, check to see if we should speculatively convert */
		𐤀𐤌 ((centry->convert != عدم) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			𐤀𐤌 (centry->bitmap != عدم) {
				image_cache_stats_bitmap_add(centry);
			} وإلا {
				image_cache->fail_count++;
			}
		}
	}



	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(بنية content *content)
{
	بنية image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	𐤀𐤌 (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		أرجع NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
صحيح image_cache_snsummaryf(حرف *string, size_t size, ثابت حرف *fmt)
{
	size_t slen = 0; /* current output string length */
	صحيح fmtc = 0; /* current index into format string */
	bool pct;
	unsigned صحيح op_count;
	uint64_t op_size;

	op_count = image_cache->hit_count +
		image_cache->miss_count +
		image_cache->fail_count;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	طالما((slen < size) && (fmt[fmtc] != 0)) {
		𐤀𐤌 (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			𐤀𐤌 (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} وإلا {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) حالة chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); اكسر

#define FMTPCHR(chr,fmt,var,div) \
حالة chr :					\
	𐤀𐤌 (pct) {							\
		𐤀𐤌 (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} وإلا {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} وإلا {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} اكسر


			بدل (fmt[fmtc]) {
			حالة '%':
				string[slen] = '%';
				slen++;
				اكسر;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_count);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_count);
			FMTCHR('h', "d", max_bitmap_count);
			FMTCHR('i', PRIsizet, max_bitmap_count_size);


			حالة 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_count);
				اكسر;

			FMTPCHR('k', "d", hit_count, op_count);
			FMTPCHR('l', "d", miss_count, op_count);
			FMTPCHR('m', "d", fail_count, op_count);

			حالة 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				اكسر;

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
		} وإلا {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	أرجع slen;
}

/* exported interface documented in image_cache.h */
صحيح
image_cache_snentryf(حرف *string,
		     size_t size,
		     unsigned صحيح entryn,
		     ثابت حرف *fmt)
{
	بنية image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	صحيح fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	𐤀𐤌 (centry == عدم)
		أرجع -1;

	طالما((slen < size) && (fmt[fmtc] != 0)) {
		𐤀𐤌 (fmt[fmtc] == '%') {
			fmtc++;
			بدل (fmt[fmtc]) {
			حالة 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				اكسر;

			حالة 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_count);
				اكسر;

			حالة 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (عائم)((image_cache->current_age -  centry->redraw_age)) / 1000);
				اكسر;


			حالة 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_count);
				اكسر;

			حالة 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (عائم)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				اكسر;

			حالة 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				اكسر;

			حالة 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				اكسر;

			حالة 'o':
				𐤀𐤌 (nsurl_has_component(llcache_handle_get_url(
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
				} وإلا {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				اكسر;

			حالة 's':
				𐤀𐤌 (centry->bitmap != عدم) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} وإلا {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				اكسر;
			}
			fmtc++;
		} وإلا {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	أرجع slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(بنية content *c,
			بنية content_redraw_data *data,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx)
{
	بنية image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	𐤀𐤌 (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		أرجع false;
	}

	𐤀𐤌 (centry->bitmap == عدم) {
		𐤀𐤌 (centry->convert != عدم) {
			centry->bitmap = centry->convert(centry->content);
		}

		𐤀𐤌 (centry->bitmap != عدم) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_count++;
			image_cache->miss_size += centry->bitmap_size;
		} وإلا {
			image_cache->fail_count++;
			image_cache->fail_size += centry->bitmap_size;
			أرجع false;
		}
	} وإلا {
		image_cache->hit_count++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_count++;
	centry->redraw_age = image_cache->current_age;

	أرجع image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
فراغ image_cache_destroy(بنية content *content)
{
	بنية image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	𐤀𐤌 (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} وإلا {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
فراغ *image_cache_get_internal(ثابت بنية content *c, فراغ *context)
{
	أرجع image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(بنية content *c)
{
	بنية bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	𐤀𐤌 (bmp != عدم) {
		أرجع guit->bitmap->get_opaque(bmp);
	}
	أرجع false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}
