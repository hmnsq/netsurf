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
قسمتعریف غیرعلامتی صحیح cache_age;

/**
 * Image cache entry
 */
ساخت image_cache_اندراج_s {
	ساخت image_cache_اندراج_s *اگلا; /**< next cache entry in list */
	ساخت image_cache_اندراج_s *پچھلا; /**< previous cache entry in list */

	/** content is used as a key */
	ساخت content *content;
	/** associated bitmap entry */
	ساخت bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	غیرعلامتی صحیح redraw_گنتی; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_سائز; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	صحیح conversion_گنتی; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
ساخت image_cache_s {
	/** Cache parameters */
	ساخت image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	ساخت image_cache_اندراج_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t کل_bitmap_سائز;

	/** Total count of bitmaps currently allocated */
	صحیح bitmap_گنتی;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_سائز;
	/** The number of objects when maximum bitmap usage occurred */
	صحیح max_bitmap_سائز_گنتی;

	/** Maximum count of bitmaps allocated at any one time */
	صحیح max_bitmap_گنتی;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_گنتی_سائز;

	/** Bitmap was not available at plot time required conversion */
	صحیح ناکامی_گنتی;
	uint64_t ناکامی_سائز;
	/** Bitmap was available at plot time required no conversion */
	صحیح کامیابی_گنتی;
	uint64_t کامیابی_سائز;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	صحیح fail_گنتی;
	uint64_t fail_سائز;

	/* Cache entry freed without ever being redrawn */
	صحیح کل_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	صحیح specultive_ناکامی_گنتی;

	/** Total number of additional (after the first) conversions */
	صحیح کل_extra_conversions;
	/** counts total number of images with more than one conversion */
	صحیح کل_extra_conversions_گنتی;

	/** Bitmap with most conversions was converted this many times */
	صحیح peak_conversions;
	/** Size of bitmap with most conversions */
	غیرعلامتی صحیح peak_conversions_سائز;
};

/** image cache state */
ساکن ساخت image_cache_s *image_cache = تہی;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ساکن ساخت image_cache_اندراج_s *image_cache__findn(صحیح entryn)
{
	ساخت image_cache_اندراج_s *ملا;

	ملا = image_cache->entries;
	جب ((ملا != تہی) && (entryn > 0)) {
		entryn--;
		ملا = ملا->اگلا;
	}
	واپس ملا;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ساکن ساخت image_cache_اندراج_s *image_cache_ڈھونڈو(ثابت ساخت content *c)
{
	ساخت image_cache_اندراج_s *ملا;

	ملا = image_cache->entries;
	جب ((ملا != تہی) && (ملا->content != c)) {
		ملا = ملا->اگلا;
	}
	واپس ملا;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ساکن خالی image_cache_stats_bitmap_جوڑو(ساخت image_cache_اندراج_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_گنتی++;

	image_cache->کل_bitmap_سائز += centry->bitmap_سائز;
	image_cache->bitmap_گنتی++;

	اگر (image_cache->کل_bitmap_سائز > image_cache->max_bitmap_سائز) {
		image_cache->max_bitmap_سائز = image_cache->کل_bitmap_سائز;
		image_cache->max_bitmap_سائز_گنتی = image_cache->bitmap_گنتی;

	}

	اگر (image_cache->bitmap_گنتی > image_cache->max_bitmap_گنتی) {
		image_cache->max_bitmap_گنتی = image_cache->bitmap_گنتی;
		image_cache->max_bitmap_گنتی_سائز = image_cache->کل_bitmap_سائز;
	}

	اگر (centry->conversion_گنتی == 2) {
		image_cache->کل_extra_conversions_گنتی++;
	}

	اگر (centry->conversion_گنتی > 1) {
		image_cache->کل_extra_conversions++;
	}

	اگر ((centry->conversion_گنتی > image_cache->peak_conversions) ||
	    (centry->conversion_گنتی == image_cache->peak_conversions &&
	     centry->bitmap_سائز > image_cache->peak_conversions_سائز)) {
		image_cache->peak_conversions = centry->conversion_گنتی;
		image_cache->peak_conversions_سائز = centry->bitmap_سائز;
	}
}

ساکن خالی image_cache__link(ساخت image_cache_اندراج_s *centry)
{
	centry->اگلا = image_cache->entries;
	centry->پچھلا = تہی;
	اگر (centry->اگلا != تہی) {
		centry->اگلا->پچھلا = centry;
	}
	image_cache->entries = centry;
}

ساکن خالی image_cache__unlink(ساخت image_cache_اندراج_s *centry)
{
	/* unlink entry */
	اگر (centry->پچھلا == تہی) {
		/* first in list */
		اگر (centry->اگلا != تہی) {
			centry->اگلا->پچھلا = centry->پچھلا;
			image_cache->entries = centry->اگلا;
		} ورنہ {
			/* empty list */
			image_cache->entries = تہی;
		}
	} ورنہ {
		centry->پچھلا->اگلا = centry->اگلا;

		اگر (centry->اگلا != تہی) {
			centry->اگلا->پچھلا = centry->پچھلا;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ساکن خالی image_cache__free_bitmap(ساخت image_cache_اندراج_s *centry)
{
	اگر (centry->bitmap != تہی) {
#اگر_تعریف_شدہ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_سائز,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_گنتی);
#اگر_ختم
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = تہی;
		image_cache->کل_bitmap_سائز -= centry->bitmap_سائز;
		image_cache->bitmap_گنتی--;
		اگر (centry->redraw_گنتی == 0) {
			image_cache->specultive_ناکامی_گنتی++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ساکن خالی image_cache_free_اندراج(ساخت image_cache_اندراج_s *centry)
{
#اگر_تعریف_شدہ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#اگر_ختم

	اگر (centry->redraw_گنتی == 0) {
		image_cache->کل_unrendered++;
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
ساکن خالی image_cache__clean(ساخت image_cache_s *icache)
{
	ساخت image_cache_اندراج_s *centry = icache->entries;

	جب (centry != تہی) {
		اگر ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_وقت) {
			/* only consider older entries, avoids active entries */
			اگر ((icache->کل_bitmap_سائز >
			     (icache->params.حد_بندی - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->اگلا;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
ساکن خالی image_cache__background_update(خالی *p)
{
	ساخت image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_وقت;

#اگر_تعریف_شدہ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#اگر_ختم

	image_cache__clean(icache);

	guit->misc->جدول_وقت(icache->params.bg_clean_وقت,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
ساخت bitmap *image_cache_get_bitmap(ثابت ساخت content *c)
{
	ساخت image_cache_اندراج_s *centry;

	centry = image_cache_ڈھونڈو(c);
	اگر (centry == تہی) {
		واپس تہی;
	}

	اگر (centry->bitmap == تہی) {
		اگر (centry->convert != تہی) {
			centry->bitmap = centry->convert(centry->content);
		}

		اگر (centry->bitmap != تہی) {
			image_cache_stats_bitmap_جوڑو(centry);
			image_cache->ناکامی_گنتی++;
			image_cache->ناکامی_سائز += centry->bitmap_سائز;
		} ورنہ {
			image_cache->fail_گنتی++;
			image_cache->fail_سائز += centry->bitmap_سائز;
		}
	} ورنہ {
		image_cache->کامیابی_گنتی++;
		image_cache->کامیابی_سائز += centry->bitmap_سائز;
	}

	واپس centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(ساخت content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	اگر ((image_cache->کل_bitmap_سائز < image_cache->params.حد_بندی) &&
	    (c->سائز <= image_cache->params.speculative_small)) {
#اگر_تعریف_شدہ IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->سائز,
		      SPECULATE_SMALL);
#اگر_ختم
		decision = true;
	}

#اگر_تعریف_شدہ IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#اگر_ختم
	واپس decision;
}

/* exported interface documented in image_cache.h */
ساخت bitmap *image_cache_ڈھونڈو_bitmap(ساخت content *c)
{
	ساخت image_cache_اندراج_s *centry;

	centry = image_cache_ڈھونڈو(c);
	اگر (centry == تہی) {
		واپس تہی;
	}

	واپس centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ثابت ساخت image_cache_parameters *image_cache_parameters)
{
	image_cache = صفر_حافظہ_دو(1, حجم(ساخت image_cache_s));
	اگر (image_cache == تہی) {
		واپس NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->جدول_وقت(image_cache->params.bg_clean_وقت,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.حد_بندی,
	      image_cache->params.hysteresis);

	واپس NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(خالی)
{
	غیرعلامتی صحیح op_گنتی;
	uint64_t op_سائز;

	guit->misc->جدول_وقت(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->کل_bitmap_سائز, image_cache->bitmap_گنتی);

	جب (image_cache->entries != تہی) {
		image_cache_free_اندراج(image_cache->entries);
	}

	op_گنتی = image_cache->کامیابی_گنتی +
		image_cache->ناکامی_گنتی +
		image_cache->fail_گنتی;

	op_سائز = image_cache->کامیابی_سائز +
		image_cache->ناکامی_سائز +
		image_cache->fail_سائز;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_سائز,
	      image_cache->max_bitmap_سائز_گنتی);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_گنتی,
	      image_cache->max_bitmap_گنتی_سائز);

	اگر ((op_گنتی > 0) && (op_سائز >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_گنتی,
		      image_cache->کامیابی_گنتی,
		      image_cache->ناکامی_گنتی,
		      image_cache->fail_گنتی,
		      (image_cache->کامیابی_گنتی * 100) / op_گنتی,
		      (image_cache->ناکامی_گنتی * 100) / op_گنتی,
		      (image_cache->fail_گنتی * 100) / op_گنتی);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_سائز,
		      image_cache->کامیابی_سائز,
		      image_cache->ناکامی_سائز,
		      image_cache->fail_سائز,
		      (image_cache->کامیابی_سائز * 100) / op_سائز,
		      (image_cache->ناکامی_سائز * 100) / op_سائز,
		      (image_cache->fail_سائز * 100) / op_سائز);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->کل_unrendered,
	      image_cache->specultive_ناکامی_گنتی);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->کل_extra_conversions,
	      image_cache->کل_extra_conversions_گنتی);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_سائز,
	      image_cache->peak_conversions);

	آزاد(image_cache);

	واپس NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_جوڑو(ساخت content *content,
			ساخت bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	ساخت image_cache_اندراج_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache_ڈھونڈو(content);
	اگر (centry == تہی) {
		/* new cache entry, content not previously added */
		centry = صفر_حافظہ_دو(1, حجم(ساخت image_cache_اندراج_s));
		اگر (centry == تہی) {
			واپس NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_سائز = content->چوڑائی * content->اونچائی * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	اگر (bitmap != تہی) {
		اگر (centry->bitmap != تہی) {
			guit->bitmap->destroy(centry->bitmap);
		} ورنہ {
			image_cache_stats_bitmap_جوڑو(centry);
		}
		centry->bitmap = bitmap;
	} ورنہ {
		/* no bitmap, check to see if we should speculatively convert */
		اگر ((centry->convert != تہی) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			اگر (centry->bitmap != تہی) {
				image_cache_stats_bitmap_جوڑو(centry);
			} ورنہ {
				image_cache->fail_گنتی++;
			}
		}
	}



	واپس NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(ساخت content *content)
{
	ساخت image_cache_اندراج_s *centry;

	/* get the cache entry */
	centry = image_cache_ڈھونڈو(content);
	اگر (centry == تہی) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		واپس NSERROR_NOT_ملا;
	}

	image_cache_free_اندراج(centry);

	واپس NSERROR_OK;
}

/* exported interface documented in image_cache.h */
صحیح image_cache_snsummaryf(حرف *string, size_t سائز, ثابت حرف *fmt)
{
	size_t slen = 0; /* current output string length */
	صحیح fmtc = 0; /* current index into format string */
	bool pct;
	غیرعلامتی صحیح op_گنتی;
	uint64_t op_سائز;

	op_گنتی = image_cache->کامیابی_گنتی +
		image_cache->ناکامی_گنتی +
		image_cache->fail_گنتی;

	op_سائز = image_cache->کامیابی_سائز +
		image_cache->ناکامی_سائز +
		image_cache->fail_سائز;

	جب((slen < سائز) && (fmt[fmtc] != 0)) {
		اگر (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			اگر (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} ورنہ {
				pct = false;
			}

#تعریف_کرو FMTCHR(chr,fmt,var) حالت chr : \
slen += snprintf(string + slen, سائز - slen, "%"fmt, image_cache->var); توڑو

#تعریف_کرو FMTPCHR(chr,fmt,var,div) \
حالت chr :					\
	اگر (pct) {							\
		اگر (div > 0) {						\
			slen += snprintf(string + slen, سائز - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} ورنہ {						\
			slen += snprintf(string + slen, سائز - slen, "100"); \
		}							\
	} ورنہ {							\
		slen += snprintf(string + slen, سائز - slen, "%"fmt, image_cache->var); \
	} توڑو


			انتخاب (fmt[fmtc]) {
			حالت '%':
				string[slen] = '%';
				slen++;
				توڑو;

			FMTCHR('a', PRIsizet, params.حد_بندی);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, کل_bitmap_سائز);
			FMTCHR('d', "d", bitmap_گنتی);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_سائز);
			FMTCHR('g', "d", max_bitmap_سائز_گنتی);
			FMTCHR('h', "d", max_bitmap_گنتی);
			FMTCHR('i', PRIsizet, max_bitmap_گنتی_سائز);


			حالت 'j':
				slen += snprintf(string + slen, سائز - slen,
						 "%u", pct?100:op_گنتی);
				توڑو;

			FMTPCHR('k', "d", کامیابی_گنتی, op_گنتی);
			FMTPCHR('l', "d", ناکامی_گنتی, op_گنتی);
			FMTPCHR('m', "d", fail_گنتی, op_گنتی);

			حالت 'n':
				slen += snprintf(string + slen, سائز - slen,
						 "%"PRId64, pct?100:op_سائز);
				توڑو;

			FMTPCHR('o', PRId64, کامیابی_سائز, op_سائز);
			FMTPCHR('q', PRId64, ناکامی_سائز, op_سائز);
			FMTPCHR('r', PRId64, fail_سائز, op_سائز);

			FMTCHR('s', "d", کل_unrendered);
			FMTCHR('t', "d", specultive_ناکامی_گنتی);
			FMTCHR('u', "d", کل_extra_conversions);
			FMTCHR('v', "d", کل_extra_conversions_گنتی);
			FMTCHR('w', "u", peak_conversions_سائز);
			FMTCHR('x', "d", peak_conversions);


			}
#تعریف_منسوخ FMTCHR
#تعریف_منسوخ FMTPCHR

			fmtc++;
		} ورنہ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, سائز - 1)] = '\0';

	واپس slen;
}

/* exported interface documented in image_cache.h */
صحیح
image_cache_snentryf(حرف *string,
		     size_t سائز,
		     غیرعلامتی صحیح entryn,
		     ثابت حرف *fmt)
{
	ساخت image_cache_اندراج_s *centry;
	size_t slen = 0; /* current output string length */
	صحیح fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	اگر (centry == تہی)
		واپس -1;

	جب((slen < سائز) && (fmt[fmtc] != 0)) {
		اگر (fmt[fmtc] == '%') {
			fmtc++;
			انتخاب (fmt[fmtc]) {
			حالت 'e':
				slen += snprintf(string + slen, سائز - slen,
						"%u", entryn);
				توڑو;

			حالت 'r':
				slen += snprintf(string + slen, سائز - slen,
						"%u", centry->redraw_گنتی);
				توڑو;

			حالت 'a':
				slen += snprintf(string + slen, سائز - slen,
						 "%.2f", (اعشاری)((image_cache->current_age -  centry->redraw_age)) / 1000);
				توڑو;


			حالت 'c':
				slen += snprintf(string + slen, سائز - slen,
						"%d", centry->conversion_گنتی);
				توڑو;

			حالت 'g':
				slen += snprintf(string + slen, سائز - slen,
						"%.2f", (اعشاری)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				توڑو;

			حالت 'k':
				slen += snprintf(string + slen, سائز - slen,
						"%p", centry->content);
				توڑو;

			حالت 'U':
				slen += snprintf(string + slen, سائز - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				توڑو;

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
							سائز - slen, "%s",
							lwc_string_ڈیٹا(
								origin));

					lwc_string_unref(origin);
				} ورنہ {
					slen += snprintf(string + slen,
							سائز - slen, "%s",
							"localhost");
				}
				توڑو;

			حالت 's':
				اگر (centry->bitmap != تہی) {
					slen += snprintf(string + slen,
							 سائز - slen,
							 "%" PRIsizet,
							 centry->bitmap_سائز);
				} ورنہ {
					slen += snprintf(string + slen,
							 سائز - slen,
							 "0");
				}
				توڑو;
			}
			fmtc++;
		} ورنہ {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, سائز - 1)] = '\0';

	واپس slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(ساخت content *c,
			ساخت content_redraw_ڈیٹا *ڈیٹا,
			ثابت ساخت rect *clip,
			ثابت ساخت redraw_context *ctx)
{
	ساخت image_cache_اندراج_s *centry;

	/* get the cache entry */
	centry = image_cache_ڈھونڈو(c);
	اگر (centry == تہی) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		واپس false;
	}

	اگر (centry->bitmap == تہی) {
		اگر (centry->convert != تہی) {
			centry->bitmap = centry->convert(centry->content);
		}

		اگر (centry->bitmap != تہی) {
			image_cache_stats_bitmap_جوڑو(centry);
			image_cache->ناکامی_گنتی++;
			image_cache->ناکامی_سائز += centry->bitmap_سائز;
		} ورنہ {
			image_cache->fail_گنتی++;
			image_cache->fail_سائز += centry->bitmap_سائز;
			واپس false;
		}
	} ورنہ {
		image_cache->کامیابی_گنتی++;
		image_cache->کامیابی_سائز += centry->bitmap_سائز;
	}


	/* update statistics */
	centry->redraw_گنتی++;
	centry->redraw_age = image_cache->current_age;

	واپس image_bitmap_plot(centry->bitmap, ڈیٹا, clip, ctx);
}

/* exported interface documented in image_cache.h */
خالی image_cache_destroy(ساخت content *content)
{
	ساخت image_cache_اندراج_s *centry;

	/* get the cache entry */
	centry = image_cache_ڈھونڈو(content);
	اگر (centry == تہی) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} ورنہ {
		image_cache_free_اندراج(centry);
	}
}

/* exported interface documented in image_cache.h */
خالی *image_cache_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	واپس image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(ساخت content *c)
{
	ساخت bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	اگر (bmp != تہی) {
		واپس guit->bitmap->get_opaque(bmp);
	}
	واپس false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}
