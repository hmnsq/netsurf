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

#समावेश <assert.h>
#समावेश <stdint.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <stdlib.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "netsurf/misc.h"
#समावेश "netsurf/bitmap.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "desktop/gui_internal.h"

#समावेश "image/image_cache.h"
#समावेश "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
प्रकार अचिन्हित पूर्णांक cache_age;

/**
 * Image cache entry
 */
काष्ठा image_cache_entry_s {
	काष्ठा image_cache_entry_s *अग्र; /**< next cache entry in list */
	काष्ठा image_cache_entry_s *पूर्व; /**< previous cache entry in list */

	/** content is used as a key */
	काष्ठा content *content;
	/** associated bitmap entry */
	काष्ठा bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	अचिन्हित पूर्णांक redraw_गिनती; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	माप_प्रकार bitmap_आकार; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	पूर्णांक conversion_गिनती; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
काष्ठा image_cache_s {
	/** Cache parameters */
	काष्ठा image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	काष्ठा image_cache_entry_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	माप_प्रकार योग_bitmap_आकार;

	/** Total count of bitmaps currently allocated */
	पूर्णांक bitmap_गिनती;

	/** Maximum size of bitmaps allocated at any one time */
	माप_प्रकार max_bitmap_आकार;
	/** The number of objects when maximum bitmap usage occurred */
	पूर्णांक max_bitmap_आकार_गिनती;

	/** Maximum count of bitmaps allocated at any one time */
	पूर्णांक max_bitmap_गिनती;
	/** The size of the bitmaps when the max count occurred */
	माप_प्रकार max_bitmap_गिनती_आकार;

	/** Bitmap was not available at plot time required conversion */
	पूर्णांक miss_गिनती;
	uint64_t miss_आकार;
	/** Bitmap was available at plot time required no conversion */
	पूर्णांक hit_गिनती;
	uint64_t hit_आकार;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	पूर्णांक fail_गिनती;
	uint64_t fail_आकार;

	/* Cache entry freed without ever being redrawn */
	पूर्णांक योग_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	पूर्णांक specultive_miss_गिनती;

	/** Total number of additional (after the first) conversions */
	पूर्णांक योग_extra_conversions;
	/** counts total number of images with more than one conversion */
	पूर्णांक योग_extra_conversions_गिनती;

	/** Bitmap with most conversions was converted this many times */
	पूर्णांक peak_conversions;
	/** Size of bitmap with most conversions */
	अचिन्हित पूर्णांक peak_conversions_आकार;
};

/** image cache state */
अटल काष्ठा image_cache_s *image_cache = शून्य;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
अटल काष्ठा image_cache_entry_s *image_cache__findn(पूर्णांक entryn)
{
	काष्ठा image_cache_entry_s *found;

	found = image_cache->entries;
	जबतक ((found != शून्य) && (entryn > 0)) {
		entryn--;
		found = found->अग्र;
	}
	वापस found;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
अटल काष्ठा image_cache_entry_s *image_cache__find(स्थिर काष्ठा content *c)
{
	काष्ठा image_cache_entry_s *found;

	found = image_cache->entries;
	जबतक ((found != शून्य) && (found->content != c)) {
		found = found->अग्र;
	}
	वापस found;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
अटल व्योम image_cache_stats_bitmap_add(काष्ठा image_cache_entry_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_गिनती++;

	image_cache->योग_bitmap_आकार += centry->bitmap_आकार;
	image_cache->bitmap_गिनती++;

	अगर (image_cache->योग_bitmap_आकार > image_cache->max_bitmap_आकार) {
		image_cache->max_bitmap_आकार = image_cache->योग_bitmap_आकार;
		image_cache->max_bitmap_आकार_गिनती = image_cache->bitmap_गिनती;

	}

	अगर (image_cache->bitmap_गिनती > image_cache->max_bitmap_गिनती) {
		image_cache->max_bitmap_गिनती = image_cache->bitmap_गिनती;
		image_cache->max_bitmap_गिनती_आकार = image_cache->योग_bitmap_आकार;
	}

	अगर (centry->conversion_गिनती == 2) {
		image_cache->योग_extra_conversions_गिनती++;
	}

	अगर (centry->conversion_गिनती > 1) {
		image_cache->योग_extra_conversions++;
	}

	अगर ((centry->conversion_गिनती > image_cache->peak_conversions) ||
	    (centry->conversion_गिनती == image_cache->peak_conversions &&
	     centry->bitmap_आकार > image_cache->peak_conversions_आकार)) {
		image_cache->peak_conversions = centry->conversion_गिनती;
		image_cache->peak_conversions_आकार = centry->bitmap_आकार;
	}
}

अटल व्योम image_cache__link(काष्ठा image_cache_entry_s *centry)
{
	centry->अग्र = image_cache->entries;
	centry->पूर्व = शून्य;
	अगर (centry->अग्र != शून्य) {
		centry->अग्र->पूर्व = centry;
	}
	image_cache->entries = centry;
}

अटल व्योम image_cache__unlink(काष्ठा image_cache_entry_s *centry)
{
	/* unlink entry */
	अगर (centry->पूर्व == शून्य) {
		/* first in list */
		अगर (centry->अग्र != शून्य) {
			centry->अग्र->पूर्व = centry->पूर्व;
			image_cache->entries = centry->अग्र;
		} अन्यथा {
			/* empty list */
			image_cache->entries = शून्य;
		}
	} अन्यथा {
		centry->पूर्व->अग्र = centry->अग्र;

		अगर (centry->अग्र != शून्य) {
			centry->अग्र->पूर्व = centry->पूर्व;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
अटल व्योम image_cache__free_bitmap(काष्ठा image_cache_entry_s *centry)
{
	अगर (centry->bitmap != शून्य) {
#अगर_घोषित IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_आकार,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_गिनती);
#पूर्ण_अगर
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = शून्य;
		image_cache->योग_bitmap_आकार -= centry->bitmap_आकार;
		image_cache->bitmap_गिनती--;
		अगर (centry->redraw_गिनती == 0) {
			image_cache->specultive_miss_गिनती++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
अटल व्योम image_cache__free_entry(काष्ठा image_cache_entry_s *centry)
{
#अगर_घोषित IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#पूर्ण_अगर

	अगर (centry->redraw_गिनती == 0) {
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
अटल व्योम image_cache__clean(काष्ठा image_cache_s *icache)
{
	काष्ठा image_cache_entry_s *centry = icache->entries;

	जबतक (centry != शून्य) {
		अगर ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_time) {
			/* only consider older entries, avoids active entries */
			अगर ((icache->योग_bitmap_आकार >
			     (icache->params.limit - icache->params.hysteresis)) &&
			    (अक्रम() > (अक्रम_उच्च / 2))) {
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
अटल व्योम image_cache__background_update(व्योम *p)
{
	काष्ठा image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_time;

#अगर_घोषित IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#पूर्ण_अगर

	image_cache__clean(icache);

	guit->misc->schedule(icache->params.bg_clean_time,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
काष्ठा bitmap *image_cache_get_bitmap(स्थिर काष्ठा content *c)
{
	काष्ठा image_cache_entry_s *centry;

	centry = image_cache__find(c);
	अगर (centry == शून्य) {
		वापस शून्य;
	}

	अगर (centry->bitmap == शून्य) {
		अगर (centry->convert != शून्य) {
			centry->bitmap = centry->convert(centry->content);
		}

		अगर (centry->bitmap != शून्य) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_गिनती++;
			image_cache->miss_आकार += centry->bitmap_आकार;
		} अन्यथा {
			image_cache->fail_गिनती++;
			image_cache->fail_आकार += centry->bitmap_आकार;
		}
	} अन्यथा {
		image_cache->hit_गिनती++;
		image_cache->hit_आकार += centry->bitmap_आकार;
	}

	वापस centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(काष्ठा content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	अगर ((image_cache->योग_bitmap_आकार < image_cache->params.limit) &&
	    (c->आकार <= image_cache->params.speculative_small)) {
#अगर_घोषित IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->आकार,
		      SPECULATE_SMALL);
#पूर्ण_अगर
		decision = true;
	}

#अगर_घोषित IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#पूर्ण_अगर
	वापस decision;
}

/* exported interface documented in image_cache.h */
काष्ठा bitmap *image_cache_find_bitmap(काष्ठा content *c)
{
	काष्ठा image_cache_entry_s *centry;

	centry = image_cache__find(c);
	अगर (centry == शून्य) {
		वापस शून्य;
	}

	वापस centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(स्थिर काष्ठा image_cache_parameters *image_cache_parameters)
{
	image_cache = सुस्मृति(1, माप(काष्ठा image_cache_s));
	अगर (image_cache == शून्य) {
		वापस NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->schedule(image_cache->params.bg_clean_time,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.limit,
	      image_cache->params.hysteresis);

	वापस NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(व्योम)
{
	अचिन्हित पूर्णांक op_गिनती;
	uint64_t op_आकार;

	guit->misc->schedule(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->योग_bitmap_आकार, image_cache->bitmap_गिनती);

	जबतक (image_cache->entries != शून्य) {
		image_cache__free_entry(image_cache->entries);
	}

	op_गिनती = image_cache->hit_गिनती +
		image_cache->miss_गिनती +
		image_cache->fail_गिनती;

	op_आकार = image_cache->hit_आकार +
		image_cache->miss_आकार +
		image_cache->fail_आकार;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_आकार,
	      image_cache->max_bitmap_आकार_गिनती);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_गिनती,
	      image_cache->max_bitmap_गिनती_आकार);

	अगर ((op_गिनती > 0) && (op_आकार >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_गिनती,
		      image_cache->hit_गिनती,
		      image_cache->miss_गिनती,
		      image_cache->fail_गिनती,
		      (image_cache->hit_गिनती * 100) / op_गिनती,
		      (image_cache->miss_गिनती * 100) / op_गिनती,
		      (image_cache->fail_गिनती * 100) / op_गिनती);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_आकार,
		      image_cache->hit_आकार,
		      image_cache->miss_आकार,
		      image_cache->fail_आकार,
		      (image_cache->hit_आकार * 100) / op_आकार,
		      (image_cache->miss_आकार * 100) / op_आकार,
		      (image_cache->fail_आकार * 100) / op_आकार);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->योग_unrendered,
	      image_cache->specultive_miss_गिनती);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->योग_extra_conversions,
	      image_cache->योग_extra_conversions_गिनती);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_आकार,
	      image_cache->peak_conversions);

	मुक्त(image_cache);

	वापस NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_add(काष्ठा content *content,
			काष्ठा bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	काष्ठा image_cache_entry_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache__find(content);
	अगर (centry == शून्य) {
		/* new cache entry, content not previously added */
		centry = सुस्मृति(1, माप(काष्ठा image_cache_entry_s));
		अगर (centry == शून्य) {
			वापस NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_आकार = content->width * content->height * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	अगर (bitmap != शून्य) {
		अगर (centry->bitmap != शून्य) {
			guit->bitmap->destroy(centry->bitmap);
		} अन्यथा {
			image_cache_stats_bitmap_add(centry);
		}
		centry->bitmap = bitmap;
	} अन्यथा {
		/* no bitmap, check to see if we should speculatively convert */
		अगर ((centry->convert != शून्य) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			अगर (centry->bitmap != शून्य) {
				image_cache_stats_bitmap_add(centry);
			} अन्यथा {
				image_cache->fail_गिनती++;
			}
		}
	}



	वापस NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(काष्ठा content *content)
{
	काष्ठा image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	अगर (centry == शून्य) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		वापस NSERROR_NOT_FOUND;
	}

	image_cache__free_entry(centry);

	वापस NSERROR_OK;
}

/* exported interface documented in image_cache.h */
पूर्णांक image_cache_snsummaryf(अक्षर *string, माप_प्रकार आकार, स्थिर अक्षर *fmt)
{
	माप_प्रकार slen = 0; /* current output string length */
	पूर्णांक fmtc = 0; /* current index into format string */
	bool pct;
	अचिन्हित पूर्णांक op_गिनती;
	uint64_t op_आकार;

	op_गिनती = image_cache->hit_गिनती +
		image_cache->miss_गिनती +
		image_cache->fail_गिनती;

	op_आकार = image_cache->hit_आकार +
		image_cache->miss_आकार +
		image_cache->fail_आकार;

	जबतक((slen < आकार) && (fmt[fmtc] != 0)) {
		अगर (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			अगर (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} अन्यथा {
				pct = false;
			}

#घोषणा FMTCHR(chr,fmt,var) हाल chr : \
slen += snprintf(string + slen, आकार - slen, "%"fmt, image_cache->var); अवरोध

#घोषणा FMTPCHR(chr,fmt,var,भाग) \
हाल chr :					\
	अगर (pct) {							\
		अगर (भाग > 0) {						\
			slen += snprintf(string + slen, आकार - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / भाग)); \
		} अन्यथा {						\
			slen += snprintf(string + slen, आकार - slen, "100"); \
		}							\
	} अन्यथा {							\
		slen += snprintf(string + slen, आकार - slen, "%"fmt, image_cache->var); \
	} अवरोध


			चयन (fmt[fmtc]) {
			हाल '%':
				string[slen] = '%';
				slen++;
				अवरोध;

			FMTCHR('a', PRIsizet, params.limit);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, योग_bitmap_आकार);
			FMTCHR('d', "d", bitmap_गिनती);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_आकार);
			FMTCHR('g', "d", max_bitmap_आकार_गिनती);
			FMTCHR('h', "d", max_bitmap_गिनती);
			FMTCHR('i', PRIsizet, max_bitmap_गिनती_आकार);


			हाल 'j':
				slen += snprintf(string + slen, आकार - slen,
						 "%u", pct?100:op_गिनती);
				अवरोध;

			FMTPCHR('k', "d", hit_गिनती, op_गिनती);
			FMTPCHR('l', "d", miss_गिनती, op_गिनती);
			FMTPCHR('m', "d", fail_गिनती, op_गिनती);

			हाल 'n':
				slen += snprintf(string + slen, आकार - slen,
						 "%"PRId64, pct?100:op_आकार);
				अवरोध;

			FMTPCHR('o', PRId64, hit_आकार, op_आकार);
			FMTPCHR('q', PRId64, miss_आकार, op_आकार);
			FMTPCHR('r', PRId64, fail_आकार, op_आकार);

			FMTCHR('s', "d", योग_unrendered);
			FMTCHR('t', "d", specultive_miss_गिनती);
			FMTCHR('u', "d", योग_extra_conversions);
			FMTCHR('v', "d", योग_extra_conversions_गिनती);
			FMTCHR('w', "u", peak_conversions_आकार);
			FMTCHR('x', "d", peak_conversions);


			}
#अघोषित FMTCHR
#अघोषित FMTPCHR

			fmtc++;
		} अन्यथा {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, आकार - 1)] = '\0';

	वापस slen;
}

/* exported interface documented in image_cache.h */
पूर्णांक
image_cache_snentryf(अक्षर *string,
		     माप_प्रकार आकार,
		     अचिन्हित पूर्णांक entryn,
		     स्थिर अक्षर *fmt)
{
	काष्ठा image_cache_entry_s *centry;
	माप_प्रकार slen = 0; /* current output string length */
	पूर्णांक fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	अगर (centry == शून्य)
		वापस -1;

	जबतक((slen < आकार) && (fmt[fmtc] != 0)) {
		अगर (fmt[fmtc] == '%') {
			fmtc++;
			चयन (fmt[fmtc]) {
			हाल 'e':
				slen += snprintf(string + slen, आकार - slen,
						"%u", entryn);
				अवरोध;

			हाल 'r':
				slen += snprintf(string + slen, आकार - slen,
						"%u", centry->redraw_गिनती);
				अवरोध;

			हाल 'a':
				slen += snprintf(string + slen, आकार - slen,
						 "%.2f", (भग्न)((image_cache->current_age -  centry->redraw_age)) / 1000);
				अवरोध;


			हाल 'c':
				slen += snprintf(string + slen, आकार - slen,
						"%d", centry->conversion_गिनती);
				अवरोध;

			हाल 'g':
				slen += snprintf(string + slen, आकार - slen,
						"%.2f", (भग्न)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				अवरोध;

			हाल 'k':
				slen += snprintf(string + slen, आकार - slen,
						"%p", centry->content);
				अवरोध;

			हाल 'U':
				slen += snprintf(string + slen, आकार - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				अवरोध;

			हाल 'o':
				अगर (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							आकार - slen, "%s",
							lwc_string_तथ्य(
								origin));

					lwc_string_unref(origin);
				} अन्यथा {
					slen += snprintf(string + slen,
							आकार - slen, "%s",
							"localhost");
				}
				अवरोध;

			हाल 's':
				अगर (centry->bitmap != शून्य) {
					slen += snprintf(string + slen,
							 आकार - slen,
							 "%" PRIsizet,
							 centry->bitmap_आकार);
				} अन्यथा {
					slen += snprintf(string + slen,
							 आकार - slen,
							 "0");
				}
				अवरोध;
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

	वापस slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(काष्ठा content *c,
			काष्ठा content_redraw_तथ्य *तथ्य,
			स्थिर काष्ठा rect *clip,
			स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(c);
	अगर (centry == शून्य) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		वापस false;
	}

	अगर (centry->bitmap == शून्य) {
		अगर (centry->convert != शून्य) {
			centry->bitmap = centry->convert(centry->content);
		}

		अगर (centry->bitmap != शून्य) {
			image_cache_stats_bitmap_add(centry);
			image_cache->miss_गिनती++;
			image_cache->miss_आकार += centry->bitmap_आकार;
		} अन्यथा {
			image_cache->fail_गिनती++;
			image_cache->fail_आकार += centry->bitmap_आकार;
			वापस false;
		}
	} अन्यथा {
		image_cache->hit_गिनती++;
		image_cache->hit_आकार += centry->bitmap_आकार;
	}


	/* update statistics */
	centry->redraw_गिनती++;
	centry->redraw_age = image_cache->current_age;

	वापस image_bitmap_plot(centry->bitmap, तथ्य, clip, ctx);
}

/* exported interface documented in image_cache.h */
व्योम image_cache_destroy(काष्ठा content *content)
{
	काष्ठा image_cache_entry_s *centry;

	/* get the cache entry */
	centry = image_cache__find(content);
	अगर (centry == शून्य) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} अन्यथा {
		image_cache__free_entry(centry);
	}
}

/* exported interface documented in image_cache.h */
व्योम *image_cache_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	वापस image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(काष्ठा content *c)
{
	काष्ठा bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	अगर (bmp != शून्य) {
		वापस guit->bitmap->get_opaque(bmp);
	}
	वापस false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}
