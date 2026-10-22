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
تعريفنوع بیعلامت صحيح cache_age;

/**
 * Image cache entry
 */
بنية image_cache_مدخل_s {
	بنية image_cache_مدخل_s *بعدی; /**< next cache entry in list */
	بنية image_cache_مدخل_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	بنية content *content;
	/** associated bitmap entry */
	بنية bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	بیعلامت صحيح redraw_شمار; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_اندازه; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	صحيح conversion_شمار; /**< Number of times image has been converted */
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
	بنية image_cache_مدخل_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t مجموع_bitmap_اندازه;

	/** Total count of bitmaps currently allocated */
	صحيح bitmap_شمار;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_اندازه;
	/** The number of objects when maximum bitmap usage occurred */
	صحيح max_bitmap_اندازه_شمار;

	/** Maximum count of bitmaps allocated at any one time */
	صحيح max_bitmap_شمار;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_شمار_اندازه;

	/** Bitmap was not available at plot time required conversion */
	صحيح نیافتن_شمار;
	uint64_t نیافتن_اندازه;
	/** Bitmap was available at plot time required no conversion */
	صحيح اصابت_شمار;
	uint64_t اصابت_اندازه;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	صحيح fail_شمار;
	uint64_t fail_اندازه;

	/* Cache entry freed without ever being redrawn */
	صحيح مجموع_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	صحيح specultive_نیافتن_شمار;

	/** Total number of additional (after the first) conversions */
	صحيح مجموع_extra_conversions;
	/** counts total number of images with more than one conversion */
	صحيح مجموع_extra_conversions_شمار;

	/** Bitmap with most conversions was converted this many times */
	صحيح peak_conversions;
	/** Size of bitmap with most conversions */
	بیعلامت صحيح peak_conversions_اندازه;
};

/** image cache state */
ساكن بنية image_cache_s *image_cache = عدم;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
ساكن بنية image_cache_مدخل_s *image_cache__findn(صحيح entryn)
{
	بنية image_cache_مدخل_s *یافت;

	یافت = image_cache->entries;
	طالما ((یافت != عدم) && (entryn > 0)) {
		entryn--;
		یافت = یافت->بعدی;
	}
	أرجع یافت;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
ساكن بنية image_cache_مدخل_s *image_cache_پیدا_کن(ثابت بنية content *c)
{
	بنية image_cache_مدخل_s *یافت;

	یافت = image_cache->entries;
	طالما ((یافت != عدم) && (یافت->content != c)) {
		یافت = یافت->بعدی;
	}
	أرجع یافت;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
ساكن فراغ image_cache_stats_bitmap_جمع_کن(بنية image_cache_مدخل_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_شمار++;

	image_cache->مجموع_bitmap_اندازه += centry->bitmap_اندازه;
	image_cache->bitmap_شمار++;

	إذا (image_cache->مجموع_bitmap_اندازه > image_cache->max_bitmap_اندازه) {
		image_cache->max_bitmap_اندازه = image_cache->مجموع_bitmap_اندازه;
		image_cache->max_bitmap_اندازه_شمار = image_cache->bitmap_شمار;

	}

	إذا (image_cache->bitmap_شمار > image_cache->max_bitmap_شمار) {
		image_cache->max_bitmap_شمار = image_cache->bitmap_شمار;
		image_cache->max_bitmap_شمار_اندازه = image_cache->مجموع_bitmap_اندازه;
	}

	إذا (centry->conversion_شمار == 2) {
		image_cache->مجموع_extra_conversions_شمار++;
	}

	إذا (centry->conversion_شمار > 1) {
		image_cache->مجموع_extra_conversions++;
	}

	إذا ((centry->conversion_شمار > image_cache->peak_conversions) ||
	    (centry->conversion_شمار == image_cache->peak_conversions &&
	     centry->bitmap_اندازه > image_cache->peak_conversions_اندازه)) {
		image_cache->peak_conversions = centry->conversion_شمار;
		image_cache->peak_conversions_اندازه = centry->bitmap_اندازه;
	}
}

ساكن فراغ image_cache__link(بنية image_cache_مدخل_s *centry)
{
	centry->بعدی = image_cache->entries;
	centry->prev = عدم;
	إذا (centry->بعدی != عدم) {
		centry->بعدی->prev = centry;
	}
	image_cache->entries = centry;
}

ساكن فراغ image_cache__unlink(بنية image_cache_مدخل_s *centry)
{
	/* unlink entry */
	إذا (centry->prev == عدم) {
		/* first in list */
		إذا (centry->بعدی != عدم) {
			centry->بعدی->prev = centry->prev;
			image_cache->entries = centry->بعدی;
		} وإلا {
			/* empty list */
			image_cache->entries = عدم;
		}
	} وإلا {
		centry->prev->بعدی = centry->بعدی;

		إذا (centry->بعدی != عدم) {
			centry->بعدی->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
ساكن فراغ image_cache__free_bitmap(بنية image_cache_مدخل_s *centry)
{
	إذا (centry->bitmap != عدم) {
#اگر_تعریف_شده IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_اندازه,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_شمار);
#پایان_اگر
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = عدم;
		image_cache->مجموع_bitmap_اندازه -= centry->bitmap_اندازه;
		image_cache->bitmap_شمار--;
		إذا (centry->redraw_شمار == 0) {
			image_cache->specultive_نیافتن_شمار++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
ساكن فراغ image_cache_free_مدخل(بنية image_cache_مدخل_s *centry)
{
#اگر_تعریف_شده IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#پایان_اگر

	إذا (centry->redraw_شمار == 0) {
		image_cache->مجموع_unrendered++;
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
	بنية image_cache_مدخل_s *centry = icache->entries;

	طالما (centry != عدم) {
		إذا ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_زمان) {
			/* only consider older entries, avoids active entries */
			إذا ((icache->مجموع_bitmap_اندازه >
			     (icache->params.حد - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->بعدی;
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
	icache->current_age += icache->params.bg_clean_زمان;

#اگر_تعریف_شده IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#پایان_اگر

	image_cache__clean(icache);

	guit->misc->زمانبندی(icache->params.bg_clean_زمان,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
بنية bitmap *image_cache_get_bitmap(ثابت بنية content *c)
{
	بنية image_cache_مدخل_s *centry;

	centry = image_cache_پیدا_کن(c);
	إذا (centry == عدم) {
		أرجع عدم;
	}

	إذا (centry->bitmap == عدم) {
		إذا (centry->convert != عدم) {
			centry->bitmap = centry->convert(centry->content);
		}

		إذا (centry->bitmap != عدم) {
			image_cache_stats_bitmap_جمع_کن(centry);
			image_cache->نیافتن_شمار++;
			image_cache->نیافتن_اندازه += centry->bitmap_اندازه;
		} وإلا {
			image_cache->fail_شمار++;
			image_cache->fail_اندازه += centry->bitmap_اندازه;
		}
	} وإلا {
		image_cache->اصابت_شمار++;
		image_cache->اصابت_اندازه += centry->bitmap_اندازه;
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
	إذا ((image_cache->مجموع_bitmap_اندازه < image_cache->params.حد) &&
	    (c->اندازه <= image_cache->params.speculative_small)) {
#اگر_تعریف_شده IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->اندازه,
		      SPECULATE_SMALL);
#پایان_اگر
		decision = true;
	}

#اگر_تعریف_شده IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#پایان_اگر
	أرجع decision;
}

/* exported interface documented in image_cache.h */
بنية bitmap *image_cache_پیدا_کن_bitmap(بنية content *c)
{
	بنية image_cache_مدخل_s *centry;

	centry = image_cache_پیدا_کن(c);
	إذا (centry == عدم) {
		أرجع عدم;
	}

	أرجع centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ثابت بنية image_cache_parameters *image_cache_parameters)
{
	image_cache = حافظه_صفر_بده(1, حجم(بنية image_cache_s));
	إذا (image_cache == عدم) {
		أرجع NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->زمانبندی(image_cache->params.bg_clean_زمان,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.حد,
	      image_cache->params.hysteresis);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(فراغ)
{
	بیعلامت صحيح op_شمار;
	uint64_t op_اندازه;

	guit->misc->زمانبندی(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->مجموع_bitmap_اندازه, image_cache->bitmap_شمار);

	طالما (image_cache->entries != عدم) {
		image_cache_free_مدخل(image_cache->entries);
	}

	op_شمار = image_cache->اصابت_شمار +
		image_cache->نیافتن_شمار +
		image_cache->fail_شمار;

	op_اندازه = image_cache->اصابت_اندازه +
		image_cache->نیافتن_اندازه +
		image_cache->fail_اندازه;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_اندازه,
	      image_cache->max_bitmap_اندازه_شمار);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_شمار,
	      image_cache->max_bitmap_شمار_اندازه);

	إذا ((op_شمار > 0) && (op_اندازه >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_شمار,
		      image_cache->اصابت_شمار,
		      image_cache->نیافتن_شمار,
		      image_cache->fail_شمار,
		      (image_cache->اصابت_شمار * 100) / op_شمار,
		      (image_cache->نیافتن_شمار * 100) / op_شمار,
		      (image_cache->fail_شمار * 100) / op_شمار);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_اندازه,
		      image_cache->اصابت_اندازه,
		      image_cache->نیافتن_اندازه,
		      image_cache->fail_اندازه,
		      (image_cache->اصابت_اندازه * 100) / op_اندازه,
		      (image_cache->نیافتن_اندازه * 100) / op_اندازه,
		      (image_cache->fail_اندازه * 100) / op_اندازه);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->مجموع_unrendered,
	      image_cache->specultive_نیافتن_شمار);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->مجموع_extra_conversions,
	      image_cache->مجموع_extra_conversions_شمار);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_اندازه,
	      image_cache->peak_conversions);

	حرر(image_cache);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_جمع_کن(بنية content *content,
			بنية bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	بنية image_cache_مدخل_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache_پیدا_کن(content);
	إذا (centry == عدم) {
		/* new cache entry, content not previously added */
		centry = حافظه_صفر_بده(1, حجم(بنية image_cache_مدخل_s));
		إذا (centry == عدم) {
			أرجع NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_اندازه = content->عرض * content->ارتفاع * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	إذا (bitmap != عدم) {
		إذا (centry->bitmap != عدم) {
			guit->bitmap->destroy(centry->bitmap);
		} وإلا {
			image_cache_stats_bitmap_جمع_کن(centry);
		}
		centry->bitmap = bitmap;
	} وإلا {
		/* no bitmap, check to see if we should speculatively convert */
		إذا ((centry->convert != عدم) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			إذا (centry->bitmap != عدم) {
				image_cache_stats_bitmap_جمع_کن(centry);
			} وإلا {
				image_cache->fail_شمار++;
			}
		}
	}



	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(بنية content *content)
{
	بنية image_cache_مدخل_s *centry;

	/* get the cache entry */
	centry = image_cache_پیدا_کن(content);
	إذا (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		أرجع NSERROR_NOT_یافت;
	}

	image_cache_free_مدخل(centry);

	أرجع NSERROR_OK;
}

/* exported interface documented in image_cache.h */
صحيح image_cache_snsummaryf(حرف *string, size_t اندازه, ثابت حرف *fmt)
{
	size_t slen = 0; /* current output string length */
	صحيح fmtc = 0; /* current index into format string */
	bool pct;
	بیعلامت صحيح op_شمار;
	uint64_t op_اندازه;

	op_شمار = image_cache->اصابت_شمار +
		image_cache->نیافتن_شمار +
		image_cache->fail_شمار;

	op_اندازه = image_cache->اصابت_اندازه +
		image_cache->نیافتن_اندازه +
		image_cache->fail_اندازه;

	طالما((slen < اندازه) && (fmt[fmtc] != 0)) {
		إذا (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			إذا (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} وإلا {
				pct = false;
			}

#تعریف_کن FMTCHR(chr,fmt,var) حالة chr : \
slen += snprintf(string + slen, اندازه - slen, "%"fmt, image_cache->var); اكسر

#تعریف_کن FMTPCHR(chr,fmt,var,div) \
حالة chr :					\
	إذا (pct) {							\
		إذا (div > 0) {						\
			slen += snprintf(string + slen, اندازه - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} وإلا {						\
			slen += snprintf(string + slen, اندازه - slen, "100"); \
		}							\
	} وإلا {							\
		slen += snprintf(string + slen, اندازه - slen, "%"fmt, image_cache->var); \
	} اكسر


			بدل (fmt[fmtc]) {
			حالة '%':
				string[slen] = '%';
				slen++;
				اكسر;

			FMTCHR('a', PRIsizet, params.حد);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, مجموع_bitmap_اندازه);
			FMTCHR('d', "d", bitmap_شمار);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_اندازه);
			FMTCHR('g', "d", max_bitmap_اندازه_شمار);
			FMTCHR('h', "d", max_bitmap_شمار);
			FMTCHR('i', PRIsizet, max_bitmap_شمار_اندازه);


			حالة 'j':
				slen += snprintf(string + slen, اندازه - slen,
						 "%u", pct?100:op_شمار);
				اكسر;

			FMTPCHR('k', "d", اصابت_شمار, op_شمار);
			FMTPCHR('l', "d", نیافتن_شمار, op_شمار);
			FMTPCHR('m', "d", fail_شمار, op_شمار);

			حالة 'n':
				slen += snprintf(string + slen, اندازه - slen,
						 "%"PRId64, pct?100:op_اندازه);
				اكسر;

			FMTPCHR('o', PRId64, اصابت_اندازه, op_اندازه);
			FMTPCHR('q', PRId64, نیافتن_اندازه, op_اندازه);
			FMTPCHR('r', PRId64, fail_اندازه, op_اندازه);

			FMTCHR('s', "d", مجموع_unrendered);
			FMTCHR('t', "d", specultive_نیافتن_شمار);
			FMTCHR('u', "d", مجموع_extra_conversions);
			FMTCHR('v', "d", مجموع_extra_conversions_شمار);
			FMTCHR('w', "u", peak_conversions_اندازه);
			FMTCHR('x', "d", peak_conversions);


			}
#لغو_تعریف FMTCHR
#لغو_تعریف FMTPCHR

			fmtc++;
		} وإلا {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, اندازه - 1)] = '\0';

	أرجع slen;
}

/* exported interface documented in image_cache.h */
صحيح
image_cache_snentryf(حرف *string,
		     size_t اندازه,
		     بیعلامت صحيح entryn,
		     ثابت حرف *fmt)
{
	بنية image_cache_مدخل_s *centry;
	size_t slen = 0; /* current output string length */
	صحيح fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	إذا (centry == عدم)
		أرجع -1;

	طالما((slen < اندازه) && (fmt[fmtc] != 0)) {
		إذا (fmt[fmtc] == '%') {
			fmtc++;
			بدل (fmt[fmtc]) {
			حالة 'e':
				slen += snprintf(string + slen, اندازه - slen,
						"%u", entryn);
				اكسر;

			حالة 'r':
				slen += snprintf(string + slen, اندازه - slen,
						"%u", centry->redraw_شمار);
				اكسر;

			حالة 'a':
				slen += snprintf(string + slen, اندازه - slen,
						 "%.2f", (عائم)((image_cache->current_age -  centry->redraw_age)) / 1000);
				اكسر;


			حالة 'c':
				slen += snprintf(string + slen, اندازه - slen,
						"%d", centry->conversion_شمار);
				اكسر;

			حالة 'g':
				slen += snprintf(string + slen, اندازه - slen,
						"%.2f", (عائم)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				اكسر;

			حالة 'k':
				slen += snprintf(string + slen, اندازه - slen,
						"%p", centry->content);
				اكسر;

			حالة 'U':
				slen += snprintf(string + slen, اندازه - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				اكسر;

			حالة 'o':
				إذا (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							اندازه - slen, "%s",
							lwc_string_داده(
								origin));

					lwc_string_unref(origin);
				} وإلا {
					slen += snprintf(string + slen,
							اندازه - slen, "%s",
							"localhost");
				}
				اكسر;

			حالة 's':
				إذا (centry->bitmap != عدم) {
					slen += snprintf(string + slen,
							 اندازه - slen,
							 "%" PRIsizet,
							 centry->bitmap_اندازه);
				} وإلا {
					slen += snprintf(string + slen,
							 اندازه - slen,
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
	string[min(slen, اندازه - 1)] = '\0';

	أرجع slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(بنية content *c,
			بنية content_redraw_داده *داده,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx)
{
	بنية image_cache_مدخل_s *centry;

	/* get the cache entry */
	centry = image_cache_پیدا_کن(c);
	إذا (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		أرجع false;
	}

	إذا (centry->bitmap == عدم) {
		إذا (centry->convert != عدم) {
			centry->bitmap = centry->convert(centry->content);
		}

		إذا (centry->bitmap != عدم) {
			image_cache_stats_bitmap_جمع_کن(centry);
			image_cache->نیافتن_شمار++;
			image_cache->نیافتن_اندازه += centry->bitmap_اندازه;
		} وإلا {
			image_cache->fail_شمار++;
			image_cache->fail_اندازه += centry->bitmap_اندازه;
			أرجع false;
		}
	} وإلا {
		image_cache->اصابت_شمار++;
		image_cache->اصابت_اندازه += centry->bitmap_اندازه;
	}


	/* update statistics */
	centry->redraw_شمار++;
	centry->redraw_age = image_cache->current_age;

	أرجع image_bitmap_plot(centry->bitmap, داده, clip, ctx);
}

/* exported interface documented in image_cache.h */
فراغ image_cache_destroy(بنية content *content)
{
	بنية image_cache_مدخل_s *centry;

	/* get the cache entry */
	centry = image_cache_پیدا_کن(content);
	إذا (centry == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} وإلا {
		image_cache_free_مدخل(centry);
	}
}

/* exported interface documented in image_cache.h */
فراغ *image_cache_get_داخلی(ثابت بنية content *c, فراغ *context)
{
	أرجع image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(بنية content *c)
{
	بنية bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	إذا (bmp != عدم) {
		أرجع guit->bitmap->get_opaque(bmp);
	}
	أرجع false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}
