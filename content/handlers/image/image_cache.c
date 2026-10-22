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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "image/image_cache.h"
#अन्तर्भाव "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
प्रकारनिर्देश unsigned पूर्णाङ्क cache_age;

/**
 * Image cache entry
 */
संरचना image_cache_entry_s {
	संरचना image_cache_entry_s *next; /**< next cache entry in list */
	संरचना image_cache_entry_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	संरचना content *content;
	/** associated bitmap entry */
	संरचना bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	unsigned पूर्णाङ्क redraw_गणनम्; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_size; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	पूर्णाङ्क conversion_गणनम्; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
संरचना image_cache_s {
	/** Cache parameters */
	संरचना image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	संरचना image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t total_bitmap_size;

	/** Total count of bitmaps currently allocated */
	पूर्णाङ्क bitmap_गणनम्;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_size;
	/** The number of objects when maximum bitmap usage occurred */
	पूर्णाङ्क max_bitmap_size_गणनम्;

	/** Maximum count of bitmaps allocated at any one time */
	पूर्णाङ्क max_bitmap_गणनम्;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_गणनम्_size;

	/** Bitmap was not available at plot time required conversion */
	पूर्णाङ्क miss_गणनम्;
	uint64_t miss_size;
	/** Bitmap was available at plot time required no conversion */
	पूर्णाङ्क hit_गणनम्;
	uint64_t hit_size;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	पूर्णाङ्क fail_गणनम्;
	uint64_t fail_size;

	/* Cache entry freed without ever being redrawn */
	पूर्णाङ्क total_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	पूर्णाङ्क specultive_miss_गणनम्;

	/** Total number of additional (after the first) conversions */
	पूर्णाङ्क total_extra_conversions;
	/** counts total number of images with more than one conversion */
	पूर्णाङ्क total_extra_conversions_गणनम्;

	/** Bitmap with most conversions was converted this many times */
	पूर्णाङ्क peak_conversions;
	/** Size of bitmap with most conversions */
	unsigned पूर्णाङ्क peak_conversions_size;
};

/** image cache state */
स्थिर संरचना image_cache_s *image_cache = रिक्त;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
स्थिर संरचना image_cache_entry_s *image_cache__findn(पूर्णाङ्क entryn)
{
	संरचना image_cache_entry_s *found;

	found = image_cache->entries;
	याव ((found != रिक्त) && (entryn > 0)) {
		entryn--;
		found = found->next;
	}
	पटिनिवत्त found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
स्थिर संरचना image_cache_entry_s *image_cache__find(ध्रुव संरचना content *c)
{
	संरचना image_cache_entry_s *found;

	found = image_cache->entries;
	याव ((found != रिक्त) && (found->content != c)) {
		found = found->next;
	}
	पटिनिवत्त found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
स्थिर शून्य image_cache_stats_bitmap_add(संरचना image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_गणनम्++;

	image_cache->total_bitmap_size += centry->bitmap_size;
	image_cache->bitmap_गणनम्++;

	सचे (image_cache->total_bitmap_size > image_cache->max_bitmap_size) {
		image_cache->max_bitmap_size = image_cache->total_bitmap_size;
		image_cache->max_bitmap_size_गणनम् = image_cache->bitmap_गणनम्;

	}

	सचे (image_cache->bitmap_गणनम् > image_cache->max_bitmap_गणनम्) {
		image_cache->max_bitmap_गणनम् = image_cache->bitmap_गणनम्;
		image_cache->max_bitmap_गणनम्_size = image_cache->total_bitmap_size;
	}

	सचे (centry->conversion_गणनम् == 2) {
		image_cache->total_extra_conversions_गणनम्++;
	}

	सचे (centry->conversion_गणनम् > 1) {
		image_cache->total_extra_conversions++;
	}

	सचे ((centry->conversion_गणनम् > image_cache->peak_conversions) ||
	    (centry->conversion_गणनम् == image_cache->peak_conversions &&
	     centry->bitmap_size > image_cache->peak_conversions_size)) {
		image_cache->peak_conversions = centry->conversion_गणनम्;
		image_cache->peak_conversions_size = centry->bitmap_size;
	}
}

स्थिर शून्य image_cache__link(संरचना image_cache_entry_s *centry)
{
	centry->next = image_cache->entries;
	centry->prev = रिक्त;
	सचे (centry->next != रिक्त) {
		centry->next->prev = centry;
	}
	image_cache->entries = centry;
}

स्थिर शून्य image_cache__unlink(संरचना image_cache_entry_s *centry)
{
	/* unlink entry */
	सचे (centry->prev == रिक्त) {
		/* first in list */
		सचे (centry->next != रिक्त) {
			centry->next->prev = centry->prev;
			image_cache->entries = centry->next;
		} अञ्ञथा {
			/* empty list */
			image_cache->entries = रिक्त;
		}
	} अञ्ञथा {
		centry->prev->next = centry->next;

		सचे (centry->next != रिक्त) {
			centry->next->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
स्थिर शून्य image_cache__free_bitmap(संरचना image_cache_entry_s *centry)
{
	सचे (centry->bitmap != रिक्त) {
#ifdef IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_size,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_गणनम्);
#endif
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = रिक्त;
		image_cache->total_bitmap_size -= centry->bitmap_size;
		image_cache->bitmap_गणनम्--;
		सचे (centry->redraw_गणनम् == 0) {
			image_cache->specultive_miss_गणनम्++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
स्थिर शून्य image_cache__free_entry(संरचना image_cache_entry_s *centry)
{
#ifdef IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#endif

	सचे (centry->redraw_गणनम् == 0) {
		image_cache->total_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	मुक्त(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
स्थिर शून्य image_cache__clean(संरचना image_cache_s *icache)
{
	संरचना image_cache_entry_s *centry = icache->entries;

	याव (centry != रिक्त) {
		सचे ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			सचे ((icache->total_bitmap_size >
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
स्थिर शून्य image_cache__background_update(शून्य *p)
{
	संरचना image_cache_s *icache = p;

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
संरचना bitmap *image_cache_get_bitmap(ध्रुव संरचना content *c)
{
	संरचना image_cache_entry_s *centry;

	centry = image_cache__find(c);
	सचे (centry == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	सचे (centry->bitmap == रिक्त) {
		सचे (centry->convert != रिक्त) {
			centry->bitmap = centry->convert(centry->content);
		}

		सचे (centry->bitmap != रिक्त) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_गणनम्++;
			image_cache->miss_size += centry->bitmap_size;
		} अञ्ञथा {
			image_cache->fail_गणनम्++;
			image_cache->fail_size += centry->bitmap_size;
		}
	} अञ्ञथा {
		image_cache->hit_गणनम्++;
		image_cache->hit_size += centry->bitmap_size;
	}

	पटिनिवत्त centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(संरचना content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	सचे ((image_cache->total_bitmap_size < image_cache->params.limit) &&
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
	पटिनिवत्त decision;
}

/* exported interface documented in image_cache.h */
संरचना bitmap *image_cache_find_bitmap(संरचना content *c)
{
	संरचना image_cache_entry_s *centry;

	centry = image_cache__find(c);
	सचे (centry == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ध्रुव संरचना image_cache_parameters *image_cache_parameters)
{
	image_cache = calloc(1, परिमाण(संरचना image_cache_s));
	सचे (image_cache == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	पटिनिवत्त NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(शून्य)
{
	unsigned पूर्णाङ्क op_गणनम्;
	uint64_t op_size;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->total_bitmap_size, image_cache->bitmap_गणनम्);

	याव (image_cache->entries != रिक्त) {
		image_cache__free_entry(image_cache->entries);
	}

	op_गणनम् = image_cache->hit_गणनम् +
		image_cache->miss_गणनम् +
		image_cache->fail_गणनम्;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_size,
	      image_cache->max_bitmap_size_गणनम्);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_गणनम्,
	      image_cache->max_bitmap_गणनम्_size);

	सचे ((op_गणनम् > 0) && (op_size >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_गणनम्,
		      image_cache->hit_गणनम्,
		      image_cache->miss_गणनम्,
		      image_cache->fail_गणनम्,
		      (image_cache->hit_गणनम् * 100) / op_गणनम्,
		      (image_cache->miss_गणनम् * 100) / op_गणनम्,
		      (image_cache->fail_गणनम् * 100) / op_गणनम्);
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
	      image_cache->specultive_miss_गणनम्);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->total_extra_conversions,
	      image_cache->total_extra_conversions_गणनम्);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_size,
	      image_cache->peak_conversions);

	मुक्त(image_cache);

	पटिनिवत्त NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(संरचना content *content,
			संरचना bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	संरचना image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	सचे (centry == रिक्त) {
		/* new cache entry, content not previously added */
		centry = calloc(1, परिमाण(संरचना image_cache_entry_s));
		सचे (centry == रिक्त) {
			पटिनिवत्त NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_size = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	सचे (bitmap != रिक्त) {
		सचे (centry->bitmap != रिक्त) {
			guit->bitmap->destroy(centry->bitmap);
		} अञ्ञथा {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} अञ्ञथा {
		/* no bitmap, check to see if we should speculatively convert */
		सचे ((centry->convert != रिक्त) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			सचे (centry->bitmap != रिक्त) {
				image_cache_stats_bitmap_add(centry);
			} अञ्ञथा {
				image_cache->fail_गणनम्++;
			}
		}
	}



	पटिनिवत्त NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(संरचना content *content)
{
	संरचना image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	सचे (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		पटिनिवत्त NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	पटिनिवत्त NSERROR_OK;
}

/* exported interface documented in image_cache.h */
पूर्णाङ्क image_cache_snsummaryf(वर्ण *string, size_t size, ध्रुव वर्ण *fmt)
{
	size_t slen = 0; /* current output string length */
	पूर्णाङ्क fmtc = 0; /* current index into format string */
	bool pct;
	unsigned पूर्णाङ्क op_गणनम्;
	uint64_t op_size;

	op_गणनम् = image_cache->hit_गणनम् +
		image_cache->miss_गणनम् +
		image_cache->fail_गणनम्;

	op_size = image_cache->hit_size +
		image_cache->miss_size +
		image_cache->fail_size;

	याव((slen < size) && (fmt[fmtc] != 0)) {
		सचे (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			सचे (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} अञ्ञथा {
				pct = false;
			}

#define FMTCHR(chr,fmt,var) पक्ष chr : \
slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); भङ्ग

#define FMTPCHR(chr,fmt,var,div) \
पक्ष chr :					\
	सचे (pct) {							\
		सचे (div > 0) {						\
			slen += snprintf(string + slen, size - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} अञ्ञथा {						\
			slen += snprintf(string + slen, size - slen, "100"); \
		}							\
	} अञ्ञथा {							\
		slen += snprintf(string + slen, size - slen, "%"fmt, image_cache->var); \
	} भङ्ग


			विकल्प (fmt[fmtc]) {
			पक्ष '%':
				string[slen] = '%';
				slen++;
				भङ्ग;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, total_bitmap_size);
			FMTCHR('d', "d", bitmap_गणनम्);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_size);
			FMTCHR('g', "d", max_bitmap_size_गणनम्);
			FMTCHR('h', "d", max_bitmap_गणनम्);
			FMTCHR('i', PRIsizet, max_bitmap_गणनम्_size);


			पक्ष 'j':
				slen += snprintf(string + slen, size - slen,
						 "%u", pct?100:op_गणनम्);
				भङ्ग;

			FMTPCHR('k', "d", hit_गणनम्, op_गणनम्);
			FMTPCHR('l', "d", miss_गणनम्, op_गणनम्);
			FMTPCHR('m', "d", fail_गणनम्, op_गणनम्);

			पक्ष 'n':
				slen += snprintf(string + slen, size - slen,
						 "%"PRId64, pct?100:op_size);
				भङ्ग;

			FMTPCHR('o', PRId64, hit_size, op_size);
			FMTPCHR('q', PRId64, miss_size, op_size);
			FMTPCHR('r', PRId64, fail_size, op_size);

			FMTCHR('s', "d", total_unrendered);
			FMTCHR('t', "d", specultive_miss_गणनम्);
			FMTCHR('u', "d", total_extra_conversions);
			FMTCHR('v', "d", total_extra_conversions_गणनम्);
			FMTCHR('w', "u", peak_conversions_size);
			FMTCHR('x', "d", peak_conversions);


			}
#undef FMTCHR
#undef FMTPCHR

			fmtc++;
		} अञ्ञथा {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	पटिनिवत्त slen;
}

/* exported interface documented in image_cache.h */
पूर्णाङ्क
image_cache_snentryf(वर्ण *string,
		     size_t size,
		     unsigned पूर्णाङ्क entryn,
		     ध्रुव वर्ण *fmt)
{
	संरचना image_cache_entry_s *centry;
	size_t slen = 0; /* current output string length */
	पूर्णाङ्क fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	सचे (centry == रिक्त)
		पटिनिवत्त -1;

	याव((slen < size) && (fmt[fmtc] != 0)) {
		सचे (fmt[fmtc] == '%') {
			fmtc++;
			विकल्प (fmt[fmtc]) {
			पक्ष 'e':
				slen += snprintf(string + slen, size - slen,
						"%u", entryn);
				भङ्ग;

			पक्ष 'r':
				slen += snprintf(string + slen, size - slen,
						"%u", centry->redraw_गणनम्);
				भङ्ग;

			पक्ष 'a':
				slen += snprintf(string + slen, size - slen,
						 "%.2f", (भिन्न)((image_cache->current_age -  centry->redraw_age)) / 1000);
				भङ्ग;


			पक्ष 'c':
				slen += snprintf(string + slen, size - slen,
						"%d", centry->conversion_गणनम्);
				भङ्ग;

			पक्ष 'g':
				slen += snprintf(string + slen, size - slen,
						"%.2f", (भिन्न)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				भङ्ग;

			पक्ष 'k':
				slen += snprintf(string + slen, size - slen,
						"%p", centry->content);
				भङ्ग;

			पक्ष 'U':
				slen += snprintf(string + slen, size - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				भङ्ग;

			पक्ष 'o':
				सचे (nsurl_has_component(llcache_handle_get_url(
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
				} अञ्ञथा {
					slen += snprintf(string + slen,
							size - slen, "%s",
							"localhost");
				}
				भङ्ग;

			पक्ष 's':
				सचे (centry->bitmap != रिक्त) {
					slen += snprintf(string + slen,
							 size - slen,
							 "%" PRIsizet,
							 centry->bitmap_size);
				} अञ्ञथा {
					slen += snprintf(string + slen,
							 size - slen,
							 "0");
				}
				भङ्ग;
			}
			fmtc++;
		} अञ्ञथा {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	पटिनिवत्त slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(संरचना content *c,
			संरचना content_redraw_data *data,
			ध्रुव संरचना rect *clip,
			ध्रुव संरचना redraw_context *ctx)
{
	संरचना image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	सचे (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		पटिनिवत्त false;
	}

	सचे (centry->bitmap == रिक्त) {
		सचे (centry->convert != रिक्त) {
			centry->bitmap = centry->convert(centry->content);
		}

		सचे (centry->bitmap != रिक्त) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_गणनम्++;
			image_cache->miss_size += centry->bitmap_size;
		} अञ्ञथा {
			image_cache->fail_गणनम्++;
			image_cache->fail_size += centry->bitmap_size;
			पटिनिवत्त false;
		}
	} अञ्ञथा {
		image_cache->hit_गणनम्++;
		image_cache->hit_size += centry->bitmap_size;
	}


	/* update statistics */
	centry->redraw_गणनम्++;
	centry->redraw_age = image_cache->current_age;

	पटिनिवत्त image_bitmap_plot(centry->bitmap, data, clip, ctx);
}

/* exported interface documented in image_cache.h */
शून्य image_cache_destroy(संरचना content *content)
{
	संरचना image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	सचे (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} अञ्ञथा {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
शून्य *image_cache_get_internal(ध्रुव संरचना content *c, शून्य *context)
{
	पटिनिवत्त image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(संरचना content *c)
{
	संरचना bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	सचे (bmp != रिक्त) {
		पटिनिवत्त guit->bitmap->get_opaque(bmp);
	}
	पटिनिवत्त false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(शून्य)
{
	पटिनिवत्त CONTENT_IMAGE;
}
