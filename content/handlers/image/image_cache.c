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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "image/image_cache.h"
#అన్తర్భావ "image/image.h"

/**
 * Age of an entry within the cache
 *
 * type deffed away so it can be readily changed later perhaps to a
 * wallclock time structure.
 */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం cache_age;

/**
 * Image cache entry
 */
నిర్మాణం image_cache_నమోదు_s {
	నిర్మాణం image_cache_నమోదు_s *తదుపరి; /**< next cache entry in list */
	నిర్మాణం image_cache_నమోదు_s *prev; /**< previous cache entry in list */

	/** content is used as a key */
	నిర్మాణం content *content;
	/** associated bitmap entry */
	నిర్మాణం bitmap *bitmap;
	/** routine to convert content into bitmap */
	image_cache_convert_fn *convert;

	/* Statistics for replacement algorithm */

	గుర్తులేని పూర్ణాంకం redraw_లెక్క; /**< number of times object has been drawn */
	cache_age redraw_age; /**< Age of last redraw */
	size_t bitmap_కొలత; /**< size if storage occupied by bitmap */
	cache_age bitmap_age; /**< Age of last conversion to a bitmap by cache*/

	పూర్ణాంకం conversion_లెక్క; /**< Number of times image has been converted */
};

/**
 * Current state of the cache.
 *
 * Global state of the cache. entries "age" is determined based on a
 * monotonically incrementing operation count. This avoids issues with
 * using wall clock time while allowing the LRU algorithm to work
 * sensibly.
 */
నిర్మాణం image_cache_s {
	/** Cache parameters */
	నిర్మాణం image_cache_parameters params;

	/** The "age" of the current operation */
	cache_age current_age;

	/* The objects the cache holds */
	నిర్మాణం image_cache_నమోదు_s *entries;


	/* Statistics for management algorithm */

	/** total size of bitmaps currently allocated */
	size_t మొత్తం_bitmap_కొలత;

	/** Total count of bitmaps currently allocated */
	పూర్ణాంకం bitmap_లెక్క;

	/** Maximum size of bitmaps allocated at any one time */
	size_t max_bitmap_కొలత;
	/** The number of objects when maximum bitmap usage occurred */
	పూర్ణాంకం max_bitmap_కొలత_లెక్క;

	/** Maximum count of bitmaps allocated at any one time */
	పూర్ణాంకం max_bitmap_లెక్క;
	/** The size of the bitmaps when the max count occurred */
	size_t max_bitmap_లెక్క_కొలత;

	/** Bitmap was not available at plot time required conversion */
	పూర్ణాంకం తప్పిపోవు_లెక్క;
	uint64_t తప్పిపోవు_కొలత;
	/** Bitmap was available at plot time required no conversion */
	పూర్ణాంకం తగిలింది_లెక్క;
	uint64_t తగిలింది_కొలత;
	/** Bitmap was not available at plot time and required
	 * conversion which failed.
	 */
	పూర్ణాంకం fail_లెక్క;
	uint64_t fail_కొలత;

	/* Cache entry freed without ever being redrawn */
	పూర్ణాంకం మొత్తం_unrendered;
	/** Bitmap was available but never required - wasted conversions */
	పూర్ణాంకం specultive_తప్పిపోవు_లెక్క;

	/** Total number of additional (after the first) conversions */
	పూర్ణాంకం మొత్తం_extra_conversions;
	/** counts total number of images with more than one conversion */
	పూర్ణాంకం మొత్తం_extra_conversions_లెక్క;

	/** Bitmap with most conversions was converted this many times */
	పూర్ణాంకం peak_conversions;
	/** Size of bitmap with most conversions */
	గుర్తులేని పూర్ణాంకం peak_conversions_కొలత;
};

/** image cache state */
స్థిర నిర్మాణం image_cache_s *image_cache = ఖాళీ;


/**
 * Find a cache entry by index.
 *
 * \param entryn index of cache entry
 * \return cache entry at index or NULL if not found.
 */
స్థిర నిర్మాణం image_cache_నమోదు_s *image_cache__findn(పూర్ణాంకం entryn)
{
	నిర్మాణం image_cache_నమోదు_s *దొరికింది;

	దొరికింది = image_cache->entries;
	అయితే ((దొరికింది != ఖాళీ) && (entryn > 0)) {
		entryn--;
		దొరికింది = దొరికింది->తదుపరి;
	}
	తిరిగి దొరికింది;
}


/**
 * Find the cache entry for a content
 *
 * \param c The content to get an entry for
 * \return The image cache entry or NULL if not found.
 */
స్థిర నిర్మాణం image_cache_నమోదు_s *image_cache_కనుగొను(స్థిరరాశి నిర్మాణం content *c)
{
	నిర్మాణం image_cache_నమోదు_s *దొరికింది;

	దొరికింది = image_cache->entries;
	అయితే ((దొరికింది != ఖాళీ) && (దొరికింది->content != c)) {
		దొరికింది = దొరికింది->తదుపరి;
	}
	తిరిగి దొరికింది;
}

/**
 * Update the image cache statistics with an entry.
 *
 * \param centry The image cache entry to update the stats with.
 */
స్థిర శూన్య image_cache_stats_bitmap_కూడు(నిర్మాణం image_cache_నమోదు_s *centry)
{
	centry->bitmap_age = image_cache->current_age;
	centry->conversion_లెక్క++;

	image_cache->మొత్తం_bitmap_కొలత += centry->bitmap_కొలత;
	image_cache->bitmap_లెక్క++;

	ఒకవేళ (image_cache->మొత్తం_bitmap_కొలత > image_cache->max_bitmap_కొలత) {
		image_cache->max_bitmap_కొలత = image_cache->మొత్తం_bitmap_కొలత;
		image_cache->max_bitmap_కొలత_లెక్క = image_cache->bitmap_లెక్క;

	}

	ఒకవేళ (image_cache->bitmap_లెక్క > image_cache->max_bitmap_లెక్క) {
		image_cache->max_bitmap_లెక్క = image_cache->bitmap_లెక్క;
		image_cache->max_bitmap_లెక్క_కొలత = image_cache->మొత్తం_bitmap_కొలత;
	}

	ఒకవేళ (centry->conversion_లెక్క == 2) {
		image_cache->మొత్తం_extra_conversions_లెక్క++;
	}

	ఒకవేళ (centry->conversion_లెక్క > 1) {
		image_cache->మొత్తం_extra_conversions++;
	}

	ఒకవేళ ((centry->conversion_లెక్క > image_cache->peak_conversions) ||
	    (centry->conversion_లెక్క == image_cache->peak_conversions &&
	     centry->bitmap_కొలత > image_cache->peak_conversions_కొలత)) {
		image_cache->peak_conversions = centry->conversion_లెక్క;
		image_cache->peak_conversions_కొలత = centry->bitmap_కొలత;
	}
}

స్థిర శూన్య image_cache__link(నిర్మాణం image_cache_నమోదు_s *centry)
{
	centry->తదుపరి = image_cache->entries;
	centry->prev = ఖాళీ;
	ఒకవేళ (centry->తదుపరి != ఖాళీ) {
		centry->తదుపరి->prev = centry;
	}
	image_cache->entries = centry;
}

స్థిర శూన్య image_cache__unlink(నిర్మాణం image_cache_నమోదు_s *centry)
{
	/* unlink entry */
	ఒకవేళ (centry->prev == ఖాళీ) {
		/* first in list */
		ఒకవేళ (centry->తదుపరి != ఖాళీ) {
			centry->తదుపరి->prev = centry->prev;
			image_cache->entries = centry->తదుపరి;
		} లేకపోతే {
			/* empty list */
			image_cache->entries = ఖాళీ;
		}
	} లేకపోతే {
		centry->prev->తదుపరి = centry->తదుపరి;

		ఒకవేళ (centry->తదుపరి != ఖాళీ) {
			centry->తదుపరి->prev = centry->prev;
		}
	}
}

/**
 * free bitmap from an image cache entry
 *
 * \param centry The image cache entry to free bitmap from.
 */
స్థిర శూన్య image_cache__free_bitmap(నిర్మాణం image_cache_నమోదు_s *centry)
{
	ఒకవేళ (centry->bitmap != ఖాళీ) {
#నిర్వచితమైతే IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "Freeing bitmap %p size %d age %d redraw count %d",
		      centry->bitmap,
		      centry->bitmap_కొలత,
		      image_cache->current_age - centry->bitmap_age,
		      centry->redraw_లెక్క);
#అయితే_ముగింపు
		guit->bitmap->destroy(centry->bitmap);
		centry->bitmap = ఖాళీ;
		image_cache->మొత్తం_bitmap_కొలత -= centry->bitmap_కొలత;
		image_cache->bitmap_లెక్క--;
		ఒకవేళ (centry->redraw_లెక్క == 0) {
			image_cache->specultive_తప్పిపోవు_లెక్క++;
		}
	}

}

/**
 * free image cache entry
 *
 * \param centry The image cache entry to free.
 */
స్థిర శూన్య image_cache_free_నమోదు(నిర్మాణం image_cache_నమోదు_s *centry)
{
#నిర్వచితమైతే IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "freeing %p ", centry);
#అయితే_ముగింపు

	ఒకవేళ (centry->redraw_లెక్క == 0) {
		image_cache->మొత్తం_unrendered++;
	}

	image_cache__free_bitmap(centry);

	image_cache__unlink(centry);

	విడుదల(centry);
}

/**
 * Image cache cleaner
 *
 * \param icache The image cache context.
 */
స్థిర శూన్య image_cache__clean(నిర్మాణం image_cache_s *icache)
{
	నిర్మాణం image_cache_నమోదు_s *centry = icache->entries;

	అయితే (centry != ఖాళీ) {
		ఒకవేళ ((icache->current_age - centry->redraw_age) >
		    icache->params.bg_clean_సమయం) {
			/* only consider older entries, avoids active entries */
			ఒకవేళ ((icache->మొత్తం_bitmap_కొలత >
			     (icache->params.హద్దు - icache->params.hysteresis)) &&
			    (rand() > (RAND_MAX / 2))) {
				image_cache__free_bitmap(centry);
			}
		}
		centry=centry->తదుపరి;
	}
}

/**
 * Cache background scheduled callback.
 *
 * \param p The image cache context.
 */
స్థిర శూన్య image_cache__background_update(శూన్య *p)
{
	నిర్మాణం image_cache_s *icache = p;

	/* increment current cache age */
	icache->current_age += icache->params.bg_clean_సమయం;

#నిర్వచితమైతే IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "Cache age %ds", icache->current_age / 1000);
#అయితే_ముగింపు

	image_cache__clean(icache);

	guit->misc->కాలపట్టిక(icache->params.bg_clean_సమయం,
				image_cache__background_update,
				icache);
}

/* exported interface documented in image_cache.h */
నిర్మాణం bitmap *image_cache_get_bitmap(స్థిరరాశి నిర్మాణం content *c)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	centry = image_cache_కనుగొను(c);
	ఒకవేళ (centry == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (centry->bitmap == ఖాళీ) {
		ఒకవేళ (centry->convert != ఖాళీ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ఒకవేళ (centry->bitmap != ఖాళీ) {
			image_cache_stats_bitmap_కూడు(centry);
			image_cache->తప్పిపోవు_లెక్క++;
			image_cache->తప్పిపోవు_కొలత += centry->bitmap_కొలత;
		} లేకపోతే {
			image_cache->fail_లెక్క++;
			image_cache->fail_కొలత += centry->bitmap_కొలత;
		}
	} లేకపోతే {
		image_cache->తగిలింది_లెక్క++;
		image_cache->తగిలింది_కొలత += centry->bitmap_కొలత;
	}

	తిరిగి centry->bitmap;
}

/* exported interface documented in image_cache.h */
bool image_cache_speculate(నిర్మాణం content *c)
{
	bool decision = false;

	/* If the cache is below its target usage and the bitmap is
	 * small enough speculate.
	 */
	ఒకవేళ ((image_cache->మొత్తం_bitmap_కొలత < image_cache->params.హద్దు) &&
	    (c->కొలత <= image_cache->params.speculative_small)) {
#నిర్వచితమైతే IMAGE_CACHE_VERBOSE
		NSLOG(netsurf, INFO,
		      "content size (%d) is smaller than minimum (%d)",
		      c->కొలత,
		      SPECULATE_SMALL);
#అయితే_ముగింపు
		decision = true;
	}

#నిర్వచితమైతే IMAGE_CACHE_VERBOSE
	NSLOG(netsurf, INFO, "returning %d", decision);
#అయితే_ముగింపు
	తిరిగి decision;
}

/* exported interface documented in image_cache.h */
నిర్మాణం bitmap *image_cache_కనుగొను_bitmap(నిర్మాణం content *c)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	centry = image_cache_కనుగొను(c);
	ఒకవేళ (centry == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	తిరిగి centry->bitmap;
}

/* exported interface documented in image_cache.h */
nserror
image_cache_init(స్థిరరాశి నిర్మాణం image_cache_parameters *image_cache_parameters)
{
	image_cache = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం image_cache_s));
	ఒకవేళ (image_cache == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	image_cache->params = *image_cache_parameters;

	guit->misc->కాలపట్టిక(image_cache->params.bg_clean_సమయం,
				image_cache__background_update,
				image_cache);

	NSLOG(netsurf, INFO,
	      "Image cache initialised with a limit of %"PRIsizet" hysteresis of %"PRIsizet,
	      image_cache->params.హద్దు,
	      image_cache->params.hysteresis);

	తిరిగి NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_fini(శూన్య)
{
	గుర్తులేని పూర్ణాంకం op_లెక్క;
	uint64_t op_కొలత;

	guit->misc->కాలపట్టిక(-1, image_cache__background_update, image_cache);

	NSLOG(netsurf, INFO, "Size at finish %"PRIsizet" (in %d)",
	      image_cache->మొత్తం_bitmap_కొలత, image_cache->bitmap_లెక్క);

	అయితే (image_cache->entries != ఖాళీ) {
		image_cache_free_నమోదు(image_cache->entries);
	}

	op_లెక్క = image_cache->తగిలింది_లెక్క +
		image_cache->తప్పిపోవు_లెక్క +
		image_cache->fail_లెక్క;

	op_కొలత = image_cache->తగిలింది_కొలత +
		image_cache->తప్పిపోవు_కొలత +
		image_cache->fail_కొలత;

	NSLOG(netsurf, INFO, "Age %ds", image_cache->current_age / 1000);
	NSLOG(netsurf, INFO, "Peak size %"PRIsizet" (in %d)",
	      image_cache->max_bitmap_కొలత,
	      image_cache->max_bitmap_కొలత_లెక్క);
	NSLOG(netsurf, INFO, "Peak image count %d (size %"PRIsizet")",
	      image_cache->max_bitmap_లెక్క,
	      image_cache->max_bitmap_లెక్క_కొలత);

	ఒకవేళ ((op_లెక్క > 0) && (op_కొలత >0)) {

		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (counts) %d/%d/%d/%d (100%%/%d%%/%d%%/%d%%)",
		      op_లెక్క,
		      image_cache->తగిలింది_లెక్క,
		      image_cache->తప్పిపోవు_లెక్క,
		      image_cache->fail_లెక్క,
		      (image_cache->తగిలింది_లెక్క * 100) / op_లెక్క,
		      (image_cache->తప్పిపోవు_లెక్క * 100) / op_లెక్క,
		      (image_cache->fail_లెక్క * 100) / op_లెక్క);
		NSLOG(netsurf, INFO,
		      "Cache total/hit/miss/fail (size) %"PRIu64"/%"PRIu64"/%"PRIu64"/%"PRIu64" (100%%/%"PRId64"%%/%"PRId64"%%/%"PRId64"%%)",
		      op_కొలత,
		      image_cache->తగిలింది_కొలత,
		      image_cache->తప్పిపోవు_కొలత,
		      image_cache->fail_కొలత,
		      (image_cache->తగిలింది_కొలత * 100) / op_కొలత,
		      (image_cache->తప్పిపోవు_కొలత * 100) / op_కొలత,
		      (image_cache->fail_కొలత * 100) / op_కొలత);
	}

	NSLOG(netsurf, INFO,
	      "Total images never rendered: %d (includes %d that were converted)",
	      image_cache->మొత్తం_unrendered,
	      image_cache->specultive_తప్పిపోవు_లెక్క);

	NSLOG(netsurf, INFO,
	      "Total number of excessive conversions: %d (from %d images converted more than once)",
	      image_cache->మొత్తం_extra_conversions,
	      image_cache->మొత్తం_extra_conversions_లెక్క);

	NSLOG(netsurf, INFO, "Bitmap of size %d had most (%d) conversions",
	      image_cache->peak_conversions_కొలత,
	      image_cache->peak_conversions);

	విడుదల(image_cache);

	తిరిగి NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_కూడు(నిర్మాణం content *content,
			నిర్మాణం bitmap *bitmap,
			image_cache_convert_fn *convert)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	/* bump the cache age by a ms to ensure multiple items are not
	 * added at exactly the same time
	 */
	image_cache->current_age++;

	centry = image_cache_కనుగొను(content);
	ఒకవేళ (centry == ఖాళీ) {
		/* new cache entry, content not previously added */
		centry = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం image_cache_నమోదు_s));
		ఒకవేళ (centry == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
		image_cache__link(centry);
		centry->content = content;

		centry->bitmap_కొలత = content->వెడల్పు * content->ఎత్తు * 4llu;
	}

	NSLOG(netsurf, INFO, "centry %p, content %p, bitmap %p", centry,
	      content, bitmap);

	centry->convert = convert;

	/* set bitmap entry if one is passed, free extant one if present */
	ఒకవేళ (bitmap != ఖాళీ) {
		ఒకవేళ (centry->bitmap != ఖాళీ) {
			guit->bitmap->destroy(centry->bitmap);
		} లేకపోతే {
			image_cache_stats_bitmap_కూడు(centry);
		}
		centry->bitmap = bitmap;
	} లేకపోతే {
		/* no bitmap, check to see if we should speculatively convert */
		ఒకవేళ ((centry->convert != ఖాళీ) &&
		    (image_cache_speculate(content) == true)) {
			centry->bitmap = centry->convert(centry->content);

			ఒకవేళ (centry->bitmap != ఖాళీ) {
				image_cache_stats_bitmap_కూడు(centry);
			} లేకపోతే {
				image_cache->fail_లెక్క++;
			}
		}
	}



	తిరిగి NSERROR_OK;
}

/* exported interface documented in image_cache.h */
nserror image_cache_remove(నిర్మాణం content *content)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	/* get the cache entry */
	centry = image_cache_కనుగొను(content);
	ఒకవేళ (centry == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	image_cache_free_నమోదు(centry);

	తిరిగి NSERROR_OK;
}

/* exported interface documented in image_cache.h */
పూర్ణాంకం image_cache_snsummaryf(అక్షరం *string, size_t కొలత, స్థిరరాశి అక్షరం *fmt)
{
	size_t slen = 0; /* current output string length */
	పూర్ణాంకం fmtc = 0; /* current index into format string */
	bool pct;
	గుర్తులేని పూర్ణాంకం op_లెక్క;
	uint64_t op_కొలత;

	op_లెక్క = image_cache->తగిలింది_లెక్క +
		image_cache->తప్పిపోవు_లెక్క +
		image_cache->fail_లెక్క;

	op_కొలత = image_cache->తగిలింది_కొలత +
		image_cache->తప్పిపోవు_కొలత +
		image_cache->fail_కొలత;

	అయితే((slen < కొలత) && (fmt[fmtc] != 0)) {
		ఒకవేళ (fmt[fmtc] == '%') {
			fmtc++;

			/* check for percentage modifier */
			ఒకవేళ (fmt[fmtc] == 'p') {
				fmtc++;
				pct = true;
			} లేకపోతే {
				pct = false;
			}

#నిర్వచించు FMTCHR(chr,fmt,var) స్థితి chr : \
slen += snprintf(string + slen, కొలత - slen, "%"fmt, image_cache->var); ఆపు

#నిర్వచించు FMTPCHR(chr,fmt,var,div) \
స్థితి chr :					\
	ఒకవేళ (pct) {							\
		ఒకవేళ (div > 0) {						\
			slen += snprintf(string + slen, కొలత - slen, "%"PRId64, (uint64_t)((image_cache->var * 100) / div)); \
		} లేకపోతే {						\
			slen += snprintf(string + slen, కొలత - slen, "100"); \
		}							\
	} లేకపోతే {							\
		slen += snprintf(string + slen, కొలత - slen, "%"fmt, image_cache->var); \
	} ఆపు


			ఎంపిక (fmt[fmtc]) {
			స్థితి '%':
				string[slen] = '%';
				slen++;
				ఆపు;

			FMTCHR('a', PRIsizet, params.హద్దు);
			FMTCHR('b', PRIsizet, params.hysteresis);
			FMTCHR('c', PRIsizet, మొత్తం_bitmap_కొలత);
			FMTCHR('d', "d", bitmap_లెక్క);
			FMTCHR('e', "u", current_age / 1000);
			FMTCHR('f', PRIsizet, max_bitmap_కొలత);
			FMTCHR('g', "d", max_bitmap_కొలత_లెక్క);
			FMTCHR('h', "d", max_bitmap_లెక్క);
			FMTCHR('i', PRIsizet, max_bitmap_లెక్క_కొలత);


			స్థితి 'j':
				slen += snprintf(string + slen, కొలత - slen,
						 "%u", pct?100:op_లెక్క);
				ఆపు;

			FMTPCHR('k', "d", తగిలింది_లెక్క, op_లెక్క);
			FMTPCHR('l', "d", తప్పిపోవు_లెక్క, op_లెక్క);
			FMTPCHR('m', "d", fail_లెక్క, op_లెక్క);

			స్థితి 'n':
				slen += snprintf(string + slen, కొలత - slen,
						 "%"PRId64, pct?100:op_కొలత);
				ఆపు;

			FMTPCHR('o', PRId64, తగిలింది_కొలత, op_కొలత);
			FMTPCHR('q', PRId64, తప్పిపోవు_కొలత, op_కొలత);
			FMTPCHR('r', PRId64, fail_కొలత, op_కొలత);

			FMTCHR('s', "d", మొత్తం_unrendered);
			FMTCHR('t', "d", specultive_తప్పిపోవు_లెక్క);
			FMTCHR('u', "d", మొత్తం_extra_conversions);
			FMTCHR('v', "d", మొత్తం_extra_conversions_లెక్క);
			FMTCHR('w', "u", peak_conversions_కొలత);
			FMTCHR('x', "d", peak_conversions);


			}
#నిర్వచనం_రద్దు FMTCHR
#నిర్వచనం_రద్దు FMTPCHR

			fmtc++;
		} లేకపోతే {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, కొలత - 1)] = '\0';

	తిరిగి slen;
}

/* exported interface documented in image_cache.h */
పూర్ణాంకం
image_cache_snentryf(అక్షరం *string,
		     size_t కొలత,
		     గుర్తులేని పూర్ణాంకం entryn,
		     స్థిరరాశి అక్షరం *fmt)
{
	నిర్మాణం image_cache_నమోదు_s *centry;
	size_t slen = 0; /* current output string length */
	పూర్ణాంకం fmtc = 0; /* current index into format string */
	lwc_string *origin; /* current entry's origin */

	centry = image_cache__findn(entryn);
	ఒకవేళ (centry == ఖాళీ)
		తిరిగి -1;

	అయితే((slen < కొలత) && (fmt[fmtc] != 0)) {
		ఒకవేళ (fmt[fmtc] == '%') {
			fmtc++;
			ఎంపిక (fmt[fmtc]) {
			స్థితి 'e':
				slen += snprintf(string + slen, కొలత - slen,
						"%u", entryn);
				ఆపు;

			స్థితి 'r':
				slen += snprintf(string + slen, కొలత - slen,
						"%u", centry->redraw_లెక్క);
				ఆపు;

			స్థితి 'a':
				slen += snprintf(string + slen, కొలత - slen,
						 "%.2f", (దశాంశం)((image_cache->current_age -  centry->redraw_age)) / 1000);
				ఆపు;


			స్థితి 'c':
				slen += snprintf(string + slen, కొలత - slen,
						"%d", centry->conversion_లెక్క);
				ఆపు;

			స్థితి 'g':
				slen += snprintf(string + slen, కొలత - slen,
						"%.2f", (దశాంశం)((image_cache->current_age -  centry->bitmap_age)) / 1000);
				ఆపు;

			స్థితి 'k':
				slen += snprintf(string + slen, కొలత - slen,
						"%p", centry->content);
				ఆపు;

			స్థితి 'U':
				slen += snprintf(string + slen, కొలత - slen,
						"%s", nsurl_access(llcache_handle_get_url(centry->content->llcache)));
				ఆపు;

			స్థితి 'o':
				ఒకవేళ (nsurl_has_component(llcache_handle_get_url(
						centry->content->llcache),
						NSURL_HOST)) {
					origin = nsurl_get_component(
							llcache_handle_get_url(
							centry->content->
								llcache),
							NSURL_HOST);

					slen += snprintf(string + slen,
							కొలత - slen, "%s",
							lwc_string_దత్తాంశం(
								origin));

					lwc_string_unref(origin);
				} లేకపోతే {
					slen += snprintf(string + slen,
							కొలత - slen, "%s",
							"localhost");
				}
				ఆపు;

			స్థితి 's':
				ఒకవేళ (centry->bitmap != ఖాళీ) {
					slen += snprintf(string + slen,
							 కొలత - slen,
							 "%" PRIsizet,
							 centry->bitmap_కొలత);
				} లేకపోతే {
					slen += snprintf(string + slen,
							 కొలత - slen,
							 "0");
				}
				ఆపు;
			}
			fmtc++;
		} లేకపోతే {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, కొలత - 1)] = '\0';

	తిరిగి slen;
}


/* exported interface documented in image_cache.h */
bool image_cache_redraw(నిర్మాణం content *c,
			నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
			స్థిరరాశి నిర్మాణం rect *clip,
			స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	/* get the cache entry */
	centry = image_cache_కనుగొను(c);
	ఒకవేళ (centry == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", c);
		తిరిగి false;
	}

	ఒకవేళ (centry->bitmap == ఖాళీ) {
		ఒకవేళ (centry->convert != ఖాళీ) {
			centry->bitmap = centry->convert(centry->content);
		}

		ఒకవేళ (centry->bitmap != ఖాళీ) {
			image_cache_stats_bitmap_కూడు(centry);
			image_cache->తప్పిపోవు_లెక్క++;
			image_cache->తప్పిపోవు_కొలత += centry->bitmap_కొలత;
		} లేకపోతే {
			image_cache->fail_లెక్క++;
			image_cache->fail_కొలత += centry->bitmap_కొలత;
			తిరిగి false;
		}
	} లేకపోతే {
		image_cache->తగిలింది_లెక్క++;
		image_cache->తగిలింది_కొలత += centry->bitmap_కొలత;
	}


	/* update statistics */
	centry->redraw_లెక్క++;
	centry->redraw_age = image_cache->current_age;

	తిరిగి image_bitmap_plot(centry->bitmap, దత్తాంశం, clip, ctx);
}

/* exported interface documented in image_cache.h */
శూన్య image_cache_destroy(నిర్మాణం content *content)
{
	నిర్మాణం image_cache_నమోదు_s *centry;

	/* get the cache entry */
	centry = image_cache_కనుగొను(content);
	ఒకవేళ (centry == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Could not find cache entry for content (%p)", content);
	} లేకపోతే {
		image_cache_free_నమోదు(centry);
	}
}

/* exported interface documented in image_cache.h */
శూన్య *image_cache_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	తిరిగి image_cache_get_bitmap(c);
}

/* exported interface documented in image_cache.h */
bool image_cache_is_opaque(నిర్మాణం content *c)
{
	నిర్మాణం bitmap *bmp;
	bmp = image_cache_get_bitmap(c);
	ఒకవేళ (bmp != ఖాళీ) {
		తిరిగి guit->bitmap->get_opaque(bmp);
	}
	తిరిగి false;
}

/* exported interface documented in image_cache.h */
content_type image_cache_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}
