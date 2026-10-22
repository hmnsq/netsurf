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
प्रकारनिर्देश अचिह्नित पूर्णाङ्क cache_age;

/**
 * Image cache entry
 */
संरचना image_cache_प्रविष्टिः_s {
	संरचना image_cache_प्रविष्टिः_s *अग्र; /**< next cache entry in list */
	संरचना image_cache_प्रविष्टिः_s *पूर्व; /**< previous cache entry in list */

	/** content is used as a key */
	संरचना content *content;
	/** associated bitmap entry */
	संरचना bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	अचिह्नित पूर्णाङ्क redraw_गणनम्; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_आकार; /**< size if storage occupied by bitmap */
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
	संरचना image_cache_प्रविष्टिः_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t योग_bitmap_आकार;

	/** Total count of bitmaps currently allocated */
	पूर्णाङ्क bitmap_गणनम्;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_आकार;
	/** The number of objects when maximum bitmap usage occurred */
	पूर्णाङ्क max_bitmap_आकार_गणनम्;

	/** Maximum count of bitmaps allocated at any one time */
	पूर्णाङ्क max_bitmap_गणनम्;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_गणनम्_आकार;

	/** Bitmap was not available at plot time required conversion */
	पूर्णाङ्क अलब्धिः_गणनम्;
	uint64_t अलब्धिः_आकार;
	/** Bitmap was available at plot time required no conversion */
	पूर्णाङ्क लब्धम्_गणनम्;
	uint64_t लब्धम्_आकार;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	पूर्णाङ्क fail_गणनम्;
	uint64_t fail_आकार;

	/* Cache entry freed without ever being redrawn */
	पूर्णाङ्क योग_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	पूर्णाङ्क specultive_अलब्धिः_गणनम्;

	/** Total number of additional (after the first) conversions */
	पूर्णाङ्क योग_extra_conversions;
	/** counts total number of images with more than one conversion */
	पूर्णाङ्क योग_extra_conversions_गणनम्;

	/** Bitmap with most conversions was converted this many times */
	पूर्णाङ्क peak_conversions;
	/** Size of bitmap with most conversions */
	अचिह्नित पूर्णाङ्क peak_conversions_आकार;
};

/** image cache state */
स्थिर संरचना image_cache_s *image_cache = रिक्त;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
स्थिर संरचना image_cache_प्रविष्टिः_s *image_cache__findn(पूर्णाङ्क entryn)
{
	संरचना image_cache_प्रविष्टिः_s *प्राप्तम्;

	प्राप्तम् = image_cache->entries;
	यावत् ((प्राप्तम् != रिक्त) && (entryn > 0)) {
		entryn--;
		प्राप्तम् = प्राप्तम्->अग्र;
	}
	प्रत्यागम प्राप्तम्;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
स्थिर संरचना image_cache_प्रविष्टिः_s *image_cache__find(ध्रुव संरचना content *c)
{
	संरचना image_cache_प्रविष्टिः_s *प्राप्तम्;

	प्राप्तम् = image_cache->entries;
	यावत् ((प्राप्तम् != रिक्त) && (प्राप्तम्->content != c)) {
		प्राप्तम् = प्राप्तम्->अग्र;
	}
	प्रत्यागम प्राप्तम्;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
स्थिर शून्य image_cache_stats_bitmap_सङ्कलय(संरचना image_cache_प्रविष्टिः_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_गणनम्++;

	image_cache->योग_bitmap_आकार += centry->bitmap_आकार;
	image_cache->bitmap_गणनम्++;

	यदि (image_cache->योग_bitmap_आकार > image_cache->max_bitmap_आकार) {
		image_cache->max_bitmap_आकार = image_cache->योग_bitmap_आकार;
		image_cache->max_bitmap_आकार_गणनम् = image_cache->bitmap_गणनम्;

	}

	यदि (image_cache->bitmap_गणनम् > image_cache->max_bitmap_गणनम्) {
		image_cache->max_bitmap_गणनम् = image_cache->bitmap_गणनम्;
		image_cache->max_bitmap_गणनम्_आकार = image_cache->योग_bitmap_आकार;
	}

	यदि (centry->conversion_गणनम् == 2) {
		image_cache->योग_extra_conversions_गणनम्++;
	}

	यदि (centry->conversion_गणनम् > 1) {
		image_cache->योग_extra_conversions++;
	}

	यदि ((centry->conversion_गणनम् > image_cache->peak_conversions) ||
	    (centry->conversion_गणनम् == image_cache->peak_conversions &&
	     centry->bitmap_आकार > image_cache->peak_conversions_आकार)) {
		image_cache->peak_conversions = centry->conversion_गणनम्;
		image_cache->peak_conversions_आकार = centry->bitmap_आकार;
	}
}

स्थिर शून्य image_cache__link(संरचना image_cache_प्रविष्टिः_s *centry)
{
	centry->अग्र = image_cache->entries;
	centry->पूर्व = रिक्त;
	यदि (centry->अग्र != रिक्त) {
		centry->अग्र->पूर्व = centry;
	}
	image_cache->entries = centry;
}

स्थिर शून्य image_cache__unlink(संरचना image_cache_प्रविष्टिः_s *centry)
{
	/* unlink entry */
	यदि (centry->पूर्व == रिक्त) {
		/* first in list */
		यदि (centry->अग्र != रिक्त) {
			centry->अग्र->पूर्व = centry->पूर्व;
			image_cache->entries = centry->अग्र;
		} अन्यथा {
			/* empty list */
			image_cache->entries = रिक्त;
		}
	} अन्यथा {
		centry->पूर्व->अग्र = centry->अग्र;

		यदि (centry->अग्र != रिक्त) {
			centry->अग्र->पूर्व = centry->पूर्व;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
स्थिर शून्य image_cache__free_bitmap(संरचना image_cache_प्रविष्टिः_s *centry)
{
	यदि (centry->bitmap != रिक्त) {
#यदि_परिभाषितम् IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_आकार,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_गणनम्);
#यदि_अन्तः
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = रिक्त;
		image_cache->योग_bitmap_आकार -= centry->bitmap_आकार;
		image_cache->bitmap_गणनम्--;
		यदि (centry->redraw_गणनम् == 0) {
			image_cache->specultive_अलब्धिः_गणनम्++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
स्थिर शून्य image_cache_free_प्रविष्टिः(संरचना image_cache_प्रविष्टिः_s *centry)
{
#यदि_परिभाषितम् IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#यदि_अन्तः

	यदि (centry->redraw_गणनम् == 0) {
		image_cache->योग_unrendered++;
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
	संरचना image_cache_प्रविष्टिः_s *centry = icache->entries;

	यावत् (centry != रिक्त) {
		यदि ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			यदि ((icache->योग_bitmap_आकार >
			     (icache->params.सीमा - icache->params.hysteresis)) &&
			    (यादृच्छिकम्() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->अग्र;
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

#यदि_परिभाषितम् IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#यदि_अन्तः

	image_cache__clean(icache);

	guit->misc->कालसारणी(icache->params.bg_clean_time,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
संरचना bitmap *image_cache_get_bitmap(ध्रुव संरचना content *c)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	centry = image_cache__find(c);
	यदि (centry == रिक्त) {
		प्रत्यागम रिक्त;
	}

	यदि (centry->bitmap == रिक्त) {
		यदि (centry->convert != रिक्त) {
			centry->bitmap = centry->convert(centry->content);
		}

		यदि (centry->bitmap != रिक्त) {
			image_cache_stats_bitmap_सङ्कलय(centry);
			image_cache->अलब्धिः_गणनम्++;
			image_cache->अलब्धिः_आकार += centry->bitmap_आकार;
		} अन्यथा {
			image_cache->fail_गणनम्++;
			image_cache->fail_आकार += centry->bitmap_आकार;
		}
	} अन्यथा {
		image_cache->लब्धम्_गणनम्++;
		image_cache->लब्धम्_आकार += centry->bitmap_आकार;
	}

	प्रत्यागम centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(संरचना content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	यदि ((image_cache->योग_bitmap_आकार < image_cache->params.सीमा) &&
	    (c->आकार <= image_cache->params.speculative_small)) {
#यदि_परिभाषितम् IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->आकार,
		      SPECULATE_SMALL);
#यदि_अन्तः
		decision = true;
	}

#यदि_परिभाषितम् IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#यदि_अन्तः
	प्रत्यागम decision;
}

/* exported interface documented in image_cache.h */
संरचना bitmap *image_cache_find_bitmap(संरचना content *c)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	centry = image_cache__find(c);
	यदि (centry == रिक्त) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(ध्रुव संरचना image_cache_parameters *image_cache_parameters)
{
	image_cache = शून्यस्मृतिं_देहि(1, परिमाण(संरचना image_cache_s));
	यदि (image_cache == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->कालसारणी(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.सीमा,
	      image_cache->params.hysteresis);

	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(शून्य)
{
	अचिह्नित पूर्णाङ्क op_गणनम्;
	uint64_t op_आकार;

	guit->misc->कालसारणी(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->योग_bitmap_आकार, image_cache->bitmap_गणनम्);

	यावत् (image_cache->entries != रिक्त) {
		image_cache_free_प्रविष्टिः(image_cache->entries);
	}

	op_गणनम् = image_cache->लब्धम्_गणनम् +
		image_cache->अलब्धिः_गणनम् +
		image_cache->fail_गणनम्;

	op_आकार = image_cache->लब्धम्_आकार +
		image_cache->अलब्धिः_आकार +
		image_cache->fail_आकार;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_आकार,
	      image_cache->max_bitmap_आकार_गणनम्);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_गणनम्,
	      image_cache->max_bitmap_गणनम्_आकार);

	यदि ((op_गणनम् > 0) && (op_आकार >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_गणनम्,
		      image_cache->लब्धम्_गणनम्,
		      image_cache->अलब्धिः_गणनम्,
		      image_cache->fail_गणनम्,
		      (image_cache->लब्धम्_गणनम् * 100) / op_गणनम्,
		      (image_cache->अलब्धिः_गणनम् * 100) / op_गणनम्,
		      (image_cache->fail_गणनम् * 100) / op_गणनम्);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_आकार,
		      image_cache->लब्धम्_आकार,
		      image_cache->अलब्धिः_आकार,
		      image_cache->fail_आकार,
		      (image_cache->लब्धम्_आकार * 100) / op_आकार,
		      (image_cache->अलब्धिः_आकार * 100) / op_आकार,
		      (image_cache->fail_आकार * 100) / op_आकार);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->योग_unrendered,
	      image_cache->specultive_अलब्धिः_गणनम्);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->योग_extra_conversions,
	      image_cache->योग_extra_conversions_गणनम्);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_आकार,
	      image_cache->peak_conversions);

	मुक्त(image_cache);

	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_सङ्कलय(संरचना content *content,
			संरचना bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	यदि (centry == रिक्त) {
		/* new cache entry, content not previously added */
		centry = शून्यस्मृतिं_देहि(1, परिमाण(संरचना image_cache_प्रविष्टिः_s));
		यदि (centry == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_आकार = content->विस्तारः * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	यदि (bitmap != रिक्त) {
		यदि (centry->bitmap != रिक्त) {
			guit->bitmap->destroy(centry->bitmap);
		} अन्यथा {
			image_cache_stats_bitmap_सङ्कलय(centry);
		}
		centry->bitmap = bitmap;
	} अन्यथा {
		/* no bitmap, check to see if we should speculatively convert */
		यदि ((centry->convert != रिक्त) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			यदि (centry->bitmap != रिक्त) {
				image_cache_stats_bitmap_सङ्कलय(centry);
			} अन्यथा {
				image_cache->fail_गणनम्++;
			}
		}
	}



	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(संरचना content *content)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	यदि (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	image_cache_free_प्रविष्टिः(centry);

	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in image_cache.h */
पूर्णाङ्क image_cache_snsummaryf(वर्ण *string, size_t आकार, ध्रुव वर्ण *fmt)
{
	size_t slen = 0; /* current output string length */
	पूर्णाङ्क fmtc = 0; /* current index into format string */
	bool pct;
	अचिह्नित पूर्णाङ्क op_गणनम्;
	uint64_t op_आकार;

	op_गणनम् = image_cache->लब्धम्_गणनम् +
		image_cache->अलब्धिः_गणनम् +
		image_cache->fail_गणनम्;

	op_आकार = image_cache->लब्धम्_आकार +
		image_cache->अलब्धिः_आकार +
		image_cache->fail_आकार;

	यावत्((slen < आकार) && (fmt[fmtc] != 0)) {
		यदि (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			यदि (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} अन्यथा {
				pct = false;
			}

#परिभाषय FMTCHR(chr,fmt,var) पक्ष chr : \
slen += snprintf(string + slen, आकार - slen, "%"fmt, image_cache->var); भङ्ग

#परिभाषय FMTPCHR(chr,fmt,var,भाजय) \
पक्ष chr :					\
	यदि (pct) {							\
		यदि (भाजय > 0) {						\
			slen += snprintf(string + slen, आकार - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / भाजय)); \
		} अन्यथा {						\
			slen += snprintf(string + slen, आकार - slen, "100"); \
		}							\
	} अन्यथा {							\
		slen += snprintf(string + slen, आकार - slen, "%"fmt, image_cache->var); \
	} भङ्ग


			विकल्प (fmt[fmtc]) {
			पक्ष '%':
				string[slen] = '%';
				slen++;
				भङ्ग;

			FMTCHR('a', PRIsizet, params.सीमा);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, योग_bitmap_आकार);
			FMTCHR('d', "d", bitmap_गणनम्);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_आकार);
			FMTCHR('g', "d", max_bitmap_आकार_गणनम्);
			FMTCHR('h', "d", max_bitmap_गणनम्);
			FMTCHR('i', PRIsizet, max_bitmap_गणनम्_आकार);


			पक्ष 'j':
				slen += snprintf(string + slen, आकार - slen,
						 "%u", pct?100:op_गणनम्);
				भङ्ग;

			FMTPCHR('k', "d", लब्धम्_गणनम्, op_गणनम्);
			FMTPCHR('l', "d", अलब्धिः_गणनम्, op_गणनम्);
			FMTPCHR('m', "d", fail_गणनम्, op_गणनम्);

			पक्ष 'n':
				slen += snprintf(string + slen, आकार - slen,
						 "%"PRId64, pct?100:op_आकार);
				भङ्ग;

			FMTPCHR('o', PRId64, लब्धम्_आकार, op_आकार);
			FMTPCHR('q', PRId64, अलब्धिः_आकार, op_आकार);
			FMTPCHR('r', PRId64, fail_आकार, op_आकार);

			FMTCHR('s', "d", योग_unrendered);
			FMTCHR('t', "d", specultive_अलब्धिः_गणनम्);
			FMTCHR('u', "d", योग_extra_conversions);
			FMTCHR('v', "d", योग_extra_conversions_गणनम्);
			FMTCHR('w', "u", peak_conversions_आकार);
			FMTCHR('x', "d", peak_conversions);


			}
#अपरिभाषय FMTCHR
#अपरिभाषय FMTPCHR

			fmtc++;
		} अन्यथा {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, आकार - 1)] = '\0';

	प्रत्यागम slen;
}

/* exported interface documented in image_cache.h */
पूर्णाङ्क
image_cache_snentryf(वर्ण *string,
		     size_t आकार,
		     अचिह्नित पूर्णाङ्क entryn,
		     ध्रुव वर्ण *fmt)
{
	संरचना image_cache_प्रविष्टिः_s *centry;
	size_t slen = 0; /* current output string length */
	पूर्णाङ्क fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	यदि (centry == रिक्त)
		प्रत्यागम -1;

	यावत्((slen < आकार) && (fmt[fmtc] != 0)) {
		यदि (fmt[fmtc] == '%') {
			fmtc++;
			विकल्प (fmt[fmtc]) {
			पक्ष 'e':
				slen += snprintf(string + slen, आकार - slen,
						"%u", entryn);
				भङ्ग;

			पक्ष 'r':
				slen += snprintf(string + slen, आकार - slen,
						"%u", centry->redraw_गणनम्);
				भङ्ग;

			पक्ष 'a':
				slen += snprintf(string + slen, आकार - slen,
						 "%.2f", (भिन्न)((image_cache->current_age -  centry->redraw_age)) / 1000);
				भङ्ग;


			पक्ष 'c':
				slen += snprintf(string + slen, आकार - slen,
						"%d", centry->conversion_गणनम्);
				भङ्ग;

			पक्ष 'g':
				slen += snprintf(string + slen, आकार - slen,
						"%.2f", (भिन्न)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				भङ्ग;

			पक्ष 'k':
				slen += snprintf(string + slen, आकार - slen,
						"%p", centry->content);
				भङ्ग;

			पक्ष 'U':
				slen += snprintf(string + slen, आकार - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				भङ्ग;

			पक्ष 'o':
				यदि (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							आकार - slen, "%s",
							lwc_string_दत्त(
								origin));

					lwc_string_unref(origin);
				} अन्यथा {
					slen += snprintf(string + slen,
							आकार - slen, "%s",
							"localhost");
				}
				भङ्ग;

			पक्ष 's':
				यदि (centry->bitmap != रिक्त) {
					slen += snprintf(string + slen,
							 आकार - slen,
							 "%" PRIsizet,
							 centry->bitmap_आकार);
				} अन्यथा {
					slen += snprintf(string + slen,
							 आकार - slen,
							 "0");
				}
				भङ्ग;
			}
			fmtc++;
		} अन्यथा {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, आकार - 1)] = '\0';

	प्रत्यागम slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(संरचना content *c,
			संरचना content_redraw_दत्त *दत्त,
			ध्रुव संरचना rect *clip,
			ध्रुव संरचना redraw_context *ctx)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	यदि (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		प्रत्यागम false;
	}

	यदि (centry->bitmap == रिक्त) {
		यदि (centry->convert != रिक्त) {
			centry->bitmap = centry->convert(centry->content);
		}

		यदि (centry->bitmap != रिक्त) {
			image_cache_stats_bitmap_सङ्कलय(centry);
			image_cache->अलब्धिः_गणनम्++;
			image_cache->अलब्धिः_आकार += centry->bitmap_आकार;
		} अन्यथा {
			image_cache->fail_गणनम्++;
			image_cache->fail_आकार += centry->bitmap_आकार;
			प्रत्यागम false;
		}
	} अन्यथा {
		image_cache->लब्धम्_गणनम्++;
		image_cache->लब्धम्_आकार += centry->bitmap_आकार;
	}


	/* update statistics */
	centry->redraw_गणनम्++;
	centry->redraw_age = image_cache->current_age;

	प्रत्यागम image_bitmap_plot(centry->bitmap, दत्त, clip, ctx);
}

/* exported interface documented in image_cache.h */
शून्य image_cache_destroy(संरचना content *content)
{
	संरचना image_cache_प्रविष्टिः_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	यदि (centry == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} अन्यथा {
		image_cache_free_प्रविष्टिः(centry);
	}
}

/* exported interface documented in image_cache.h */
शून्य *image_cache_get_आन्तरिकम्(ध्रुव संरचना content *c, शून्य *context)
{
	प्रत्यागम image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(संरचना content *c)
{
	संरचना bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	यदि (bmp != रिक्त) {
		प्रत्यागम guit->bitmap->get_opaque(bmp);
	}
	प्रत्यागम false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(शून्य)
{
	प्रत्यागम CONTENT_IMAGE;
}
