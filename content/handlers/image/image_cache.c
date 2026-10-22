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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "image/image_cache.h"
#અન્તર્ભાવ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
પ્રકારવ્યાખ્યા unsigned પૂર્ણાંક cache_age;

/**
 * Image cache entry
 */
સંરચના image_cache_entry_s {
	સંરચના image_cache_entry_s *next; /**< next cache entry in list */
	સંરચના image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	સંરચના content *content;
	/** associated bitmap entry */
	સંરચના bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned પૂર્ણાંક redraw_ગણતરી; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	પૂર્ણાંક conversion_ગણતરી; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
સંરચના image_cache_s {
	/** Cache parameters */
	સંરચના image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	સંરચના image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	પૂર્ણાંક bitmap_ગણતરી;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	પૂર્ણાંક max_bitmap_size_ગણતરી;

	/** Maximum count of bitmaps allocated at any one time */
	પૂર્ણાંક max_bitmap_ગણતરી;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_ગણતરી_size;

	/** Bitmap was not available at plot time required conversion */
	પૂર્ણાંક miss_ગણતરી;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	પૂર્ણાંક hit_ગણતરી;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	પૂર્ણાંક fail_ગણતરી;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	પૂર્ણાંક total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	પૂર્ણાંક specultive_miss_ગણતરી;

	/** Total number of additional (after the first) conversions */
	પૂર્ણાંક total_extra_conversions;
	/** counts total number of images with more than one conversion */
	પૂર્ણાંક total_extra_conversions_ગણતરી;

	/** Bitmap with most conversions was converted this many times */
	પૂર્ણાંક peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned પૂર્ણાંક peak_conversions_size;
};

/** image cache state */
સ્થિર સંરચના image_cache_s *image_cache = ખાલી;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
સ્થિર સંરચના image_cache_entry_s *image_cache__findn(પૂર્ણાંક entryn)
{
	સંરચના image_cache_entry_s *found;

	found = image_cache->entries;
	જ્યારે ((found != ખાલી) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	પાછા found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
સ્થિર સંરચના image_cache_entry_s *image_cache__find(અચલ સંરચના content *c)
{
	સંરચના image_cache_entry_s *found;

	found = image_cache->entries;
	જ્યારે ((found != ખાલી) && (found->content != c)) {
		found = found->next;
	}
	પાછા found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
સ્થિર શૂન્ય image_cache_stats_bitmap_add(સંરચના image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_ગણતરી++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_ગણતરી++;

	જો (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_ગણતરી = image_cache->bitmap_ગણતરી;

	}

	જો (image_cache->bitmap_ગણતરી > image_cache->max_bitmap_ગણતરી) {
		image_cache->max_bitmap_ગણતરી = image_cache->bitmap_ગણતરી;
		image_cache->max_bitmap_ગણતરી_size = image_cache->total_bitmap_size;
	}

	જો (centry->conversion_ગણતરી == 2) {
		image_cache->total_extra_conversions_ગણતરી++;
	}

	જો (centry->conversion_ગણતરી > 1) {
		image_cache->total_extra_conversions++;
	}

	જો ((centry->conversion_ગણતરી > image_cache->peak_conversions) ||
	    (centry->conversion_ગણતરી == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_ગણતરી;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

સ્થિર શૂન્ય image_cache__link(સંરચના image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = ખાલી;
	જો (centry->next != ખાલી) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

સ્થિર શૂન્ય image_cache__unlink(સંરચના image_cache_entry_s *centry)
{
	/* unlink entry */
	જો (centry->prev == ખાલી) {
		/* first in list */
		જો (centry->next != ખાલી) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} નહીંતર {
			/* empty list */
			image_cache->entries = ખાલી;
		}
	} નહીંતર {
		centry->prev->next = centry->next;

		જો (centry->next != ખાલી) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
સ્થિર શૂન્ય image_cache__free_bitmap(સંરચના image_cache_entry_s *centry)
{
	જો (centry->bitmap != ખાલી) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_ગણતરી);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ખાલી;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_ગણતરી--;
		જો (centry->redraw_ગણતરી == 0) {
			image_cache->specultive_miss_ગણતરી++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
સ્થિર શૂન્ય image_cache__free_entry(સંરચના image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	જો (centry->redraw_ગણતરી == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	મુક્ત(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
સ્થિર શૂન્ય image_cache__clean(સંરચના image_cache_s *icache)
{
	સંરચના image_cache_entry_s *centry = icache->entries;

	જ્યારે (centry != ખાલી) {
		જો ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			જો ((icache->total_bitmap_size >
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
સ્થિર શૂન્ય image_cache__background_update(શૂન્ય *p)
{
	સંરચના image_cache_s *icache = p;

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
સંરચના bitmap *image_cache_get_bitmap(અચલ સંરચના content *c)
{
	સંરચના image_cache_entry_s *centry;

	centry = image_cache__find(c);
	જો (centry == ખાલી) {
		પાછા ખાલી;
	}

	જો (centry->bitmap == ખાલી) {
		જો (centry->convert != ખાલી) {
			centry->bitmap = centry->convert(centry->content);
		}

		જો (centry->bitmap != ખાલી) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ગણતરી++;
			image_cache->miss_size += centry->bitmap_size;
		} નહીંતર {
			image_cache->fail_ગણતરી++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} નહીંતર {
		image_cache->hit_ગણતરી++;
		image_cache->hit_size += centry->bitmap_size;
	}

	પાછા centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(સંરચના content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	જો ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	પાછા decision;
}

/* exported interface documented in image_cache.h */
સંરચના bitmap *image_cache_find_bitmap(સંરચના content *c)
{
	સંરચના image_cache_entry_s *centry;

	centry = image_cache__find(c);
	જો (centry == ખાલી) {
		પાછા ખાલી;
	}

	પાછા centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(અચલ સંરચના image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, કદ(સંરચના image_cache_s));
	જો (image_cache == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	પાછા NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(શૂન્ય)
{
	unsigned પૂર્ણાંક op_ગણતરી;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_ગણતરી);

	જ્યારે (image_cache->entries != ખાલી) {
		image_cache__free_entry(image_cache->entries);
	}

	op_ગણતરી = image_cache->hit_ગણતરી +
		image_cache->miss_ગણતરી +
		image_cache->fail_ગણતરી;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_ગણતરી);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_ગણતરી,
	      image_cache->max_bitmap_ગણતરી_size);

	જો ((op_ગણતરી > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_ગણતરી,
		      image_cache->hit_ગણતરી,
		      image_cache->miss_ગણતરી,
		      image_cache->fail_ગણતરી,
		      (image_cache->hit_ગણતરી * 100) / op_ગણતરી,
		      (image_cache->miss_ગણતરી * 100) / op_ગણતરી,
		      (image_cache->fail_ગણતરી * 100) / op_ગણતરી);
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
	      image_cache->specultive_miss_ગણતરી);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_ગણતરી);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	મુક્ત(image_cache);

	પાછા NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(સંરચના content *content,
			સંરચના bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	સંરચના image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	જો (centry == ખાલી) {
		/* new cache entry, content not previously added */
		centry = calloc(1, કદ(સંરચના image_cache_entry_s));
		જો (centry == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	જો (bitmap != ખાલી) {
		જો (centry->bitmap != ખાલી) {
			guit->bitmap->destroy(centry->bitmap);
		} નહીંતર {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} નહીંતર {
		/* no bitmap, check to see if we should speculatively convert */
		જો ((centry->convert != ખાલી) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			જો (centry->bitmap != ખાલી) {
				image_cache_stats_bitmap_add(centry);
			} નહીંતર {
				image_cache->fail_ગણતરી++;
			}
		}
	}



	પાછા NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(સંરચના content *content)
{
	સંરચના image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	જો (centry == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		પાછા NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	પાછા NSERROR_OK;
}

/* exported interface documented in image_cache.h */
પૂર્ણાંક image_cache_snsummaryf(અક્ષર *string, size_t size, અચલ અક્ષર *fmt)
{
	size_t slen = 0; /* current output string length */
	પૂર્ણાંક fmtc = 0; /* current index into format string */
	bool pct;
	unsigned પૂર્ણાંક op_ગણતરી;
	uint64_t op_size;

	op_ગણતરી = image_cache->hit_ગણતરી +
		image_cache->miss_ગણતરી +
		image_cache->fail_ગણતરી;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	જ્યારે((slen < size) && (fmt[fmtc] != 0)) {
		જો (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			જો (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} નહીંતર {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) સ્થિતિ chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); તોડો

#define FMTPCHR(chr,fmt,var,div) \
સ્થિતિ chr :					\
	જો (pct) {							\
		જો (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} નહીંતર {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} નહીંતર {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} તોડો


			પસંદગી (fmt[fmtc]) {
			સ્થિતિ '%':
				string[slen] = '%';
				slen++;
				તોડો;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_ગણતરી);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_ગણતરી);
			FMTCHR('h', "d", max_bitmap_ગણતરી);
			FMTCHR('i', PRIsizet, max_bitmap_ગણતરી_size);


			સ્થિતિ 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_ગણતરી);
				તોડો;

			FMTPCHR('k', "d", hit_ગણતરી, op_ગણતરી);
			FMTPCHR('l', "d", miss_ગણતરી, op_ગણતરી);
			FMTPCHR('m', "d", fail_ગણતરી, op_ગણતરી);

			સ્થિતિ 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				તોડો;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_ગણતરી);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_ગણતરી);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} નહીંતર {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	પાછા slen;
}

/* exported interface documented in image_cache.h */
પૂર્ણાંક
image_cache_snentryf(અક્ષર *string,
		     size_t size,
		     unsigned પૂર્ણાંક entryn,
		     અચલ અક્ષર *fmt)
{
	સંરચના image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	પૂર્ણાંક fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	જો (centry == ખાલી)
		પાછા -1;

	જ્યારે((slen < size) && (fmt[fmtc] != 0)) {
		જો (fmt[fmtc] == '%') {
			fmtc++;
			પસંદગી (fmt[fmtc]) {
			સ્થિતિ 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				તોડો;

			સ્થિતિ 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_ગણતરી);
				તોડો;

			સ્થિતિ 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (દશાંશ)((image_cache->current_age -  centry->redraw_age)) / 1000);
				તોડો;


			સ્થિતિ 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_ગણતરી);
				તોડો;

			સ્થિતિ 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (દશાંશ)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				તોડો;

			સ્થિતિ 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				તોડો;

			સ્થિતિ 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				તોડો;

			સ્થિતિ 'o':
				જો (nsurl_has_component(llcache_handle_get_url(
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
				} નહીંતર {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				તોડો;

			સ્થિતિ 's':
				જો (centry->bitmap != ખાલી) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} નહીંતર {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				તોડો;
			}
			fmtc++;
		} નહીંતર {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	પાછા slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(સંરચના content *c,
			સંરચના content_redraw_data *data,
			અચલ સંરચના rect *clip,
			અચલ સંરચના redraw_context *ctx)
{
	સંરચના image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	જો (centry == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		પાછા false;
	}

	જો (centry->bitmap == ખાલી) {
		જો (centry->convert != ખાલી) {
			centry->bitmap = centry->convert(centry->content);
		}

		જો (centry->bitmap != ખાલી) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_ગણતરી++;
			image_cache->miss_size += centry->bitmap_size;
		} નહીંતર {
			image_cache->fail_ગણતરી++;
			image_cache->fail_size += centry->bitmap_size;
			પાછા false;
		}
	} નહીંતર {
		image_cache->hit_ગણતરી++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_ગણતરી++;
	centry->redraw_age = image_cache->current_age;

	પાછા image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
શૂન્ય image_cache_destroy(સંરચના content *content)
{
	સંરચના image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	જો (centry == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} નહીંતર {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
શૂન્ય *image_cache_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	પાછા image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(સંરચના content *c)
{
	સંરચના bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	જો (bmp != ખાલી) {
		પાછા guit->bitmap->get_opaque(bmp);
	}
	પાછા false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}
