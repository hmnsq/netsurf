/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
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
 * Implementation of HTML image maps
 *
 * \todo should this should use the general hashmap instead of its own
 */

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/imagemap.h"

#ವ್ಯಾಖ್ಯಾನಿಸು HASH_ಗಾತ್ರ 31 /* fixed size hash table */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_ನಮೂದು_type;

ರಚನೆ mapentry {
	imagemap_ನಮೂದು_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	ಅಕ್ಷರ *ಗುರಿ;			/**< target frame (if any) */
	ಒಕ್ಕೂಟ {
		ರಚನೆ {
			ಪೂರ್ಣಾಂಕ x;		/**< x coordinate of centre */
			ಪೂರ್ಣಾಂಕ y;		/**< y coordinate of center */
			ಪೂರ್ಣಾಂಕ r;		/**< radius of circle */
		} circle;
		ರಚನೆ {
			ಪೂರ್ಣಾಂಕ x0;		/**< left hand edge */
			ಪೂರ್ಣಾಂಕ y0;		/**< top edge */
			ಪೂರ್ಣಾಂಕ x1;		/**< right hand edge */
			ಪೂರ್ಣಾಂಕ y1;		/**< bottom edge */
		} rect;
		ರಚನೆ {
			ಪೂರ್ಣಾಂಕ ಅಂಕೆ;	/**< number of points */
			ದಶಮಾಂಶ *xcoords;	/**< x coordinates */
			ದಶಮಾಂಶ *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ರಚನೆ mapentry *ಮುಂದಿನ;		/**< next entry in list */
};

ರಚನೆ imagemap {
	ಅಕ್ಷರ *ಕೀಲಿ;		/**< key for this entry */
	ರಚನೆ mapentry *list;	/**< pointer to linked list of entries */
	ರಚನೆ imagemap *ಮುಂದಿನ;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ಸ್ಥಿರ bool imagemap_create(html_content *c)
{
	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->imagemaps == ಖಾಲಿ) {
		c->imagemaps = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(HASH_ಗಾತ್ರ, ಗಾತ್ರ(ರಚನೆ imagemap *));
		ಆದರೆ (c->imagemaps == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ imagemap_hash(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z = 0;

	ಆದರೆ (ಕೀಲಿ == 0) ಹಿಂತಿರುಗಿ 0;

	ಗಾಗಿ (; *ಕೀಲಿ != 0; ಕೀಲಿ++) {
		z += *ಕೀಲಿ & 0x1f;
	}

	ಹಿಂತಿರುಗಿ (z % (HASH_ಗಾತ್ರ - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ಸ್ಥಿರ bool
imagemap_ಕೂಡಿಸು(html_content *c, dom_string *ಕೀಲಿ, ರಚನೆ mapentry *list)
{
	ರಚನೆ imagemap *map;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಕೀಲಿ != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(list != ಖಾಲಿ);

	ಆದರೆ (imagemap_create(c) == false)
		ಹಿಂತಿರುಗಿ false;

	map = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*map));
	ಆದರೆ (map == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ಕೀಲಿ = strdup(dom_string_ದತ್ತಾಂಶ(ಕೀಲಿ));
	ಆದರೆ (map->ಕೀಲಿ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(map);
		ಹಿಂತಿರುಗಿ false;
	}

	map->list = list;

	ಸ್ಥಾನ = imagemap_hash(map->ಕೀಲಿ);

	map->ಮುಂದಿನ = c->imagemaps[ಸ್ಥಾನ];
	c->imagemaps[ಸ್ಥಾನ] = map;

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ಸ್ಥಿರ ಶೂನ್ಯ imagemap_freelist(ರಚನೆ mapentry *list)
{
	ರಚನೆ mapentry *ನಮೂದು, *prev;

	ಪ್ರತಿಜ್ಞಾ(list != ಖಾಲಿ);

	ನಮೂದು = list;

	ಆಗ (ನಮೂದು != ಖಾಲಿ) {
		prev = ನಮೂದು;

		nsurl_unref(ನಮೂದು->url);

		ಆದರೆ (ನಮೂದು->ಗುರಿ)
			ಬಿಡುಗಡೆ(ನಮೂದು->ಗುರಿ);

		ಆದರೆ (ನಮೂದು->type == IMAGEMAP_POLY) {
			ಬಿಡುಗಡೆ(ನಮೂದು->bounds.poly.xcoords);
			ಬಿಡುಗಡೆ(ನಮೂದು->bounds.poly.ycoords);
		}

		ನಮೂದು = ನಮೂದು->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
ಶೂನ್ಯ imagemap_destroy(html_content *c)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	/* no imagemaps -> return */
	ಆದರೆ (c->imagemaps == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (i = 0; i != HASH_ಗಾತ್ರ; i++) {
		ರಚನೆ imagemap *map, *ಮುಂದಿನ;

		map = c->imagemaps[i];
		ಆಗ (map != ಖಾಲಿ) {
			ಮುಂದಿನ = map->ಮುಂದಿನ;
			imagemap_freelist(map->list);
			ಬಿಡುಗಡೆ(map->ಕೀಲಿ);
			ಬಿಡುಗಡೆ(map);
			map = ಮುಂದಿನ;
		}
	}

	ಬಿಡುಗಡೆ(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
ಶೂನ್ಯ imagemap_dump(html_content *c)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಪೂರ್ಣಾಂಕ j;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->imagemaps == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (i = 0; i != HASH_ಗಾತ್ರ; i++) {
		ರಚನೆ imagemap *map;
		ರಚನೆ mapentry *ನಮೂದು;

		map = c->imagemaps[i];
		ಆಗ (map != ಖಾಲಿ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ಕೀಲಿ);

			ಗಾಗಿ (ನಮೂದು = map->list; ನಮೂದು; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
				ಆಯ್ಕೆ (ನಮೂದು->type) {
				ಸ್ಥಿತಿ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(ನಮೂದು->url));
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(ನಮೂದು->url),
					      ನಮೂದು->bounds.rect.x0,
					      ನಮೂದು->bounds.rect.y0,
					      ನಮೂದು->bounds.rect.x1,
					      ನಮೂದು->bounds.rect.y1);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(ನಮೂದು->url),
					      ನಮೂದು->bounds.circle.x,
					      ನಮೂದು->bounds.circle.y,
					      ನಮೂದು->bounds.circle.r);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(ನಮೂದು->url));
					ಗಾಗಿ (j = 0; j != ನಮೂದು->bounds.poly.ಅಂಕೆ;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(ಪೂರ್ಣಾಂಕ)ನಮೂದು->bounds.poly.xcoords[j],
							(ಪೂರ್ಣಾಂಕ)ನಮೂದು->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ನಿಲ್ಲಿಸು;
				}
			}
			map = map->ಮುಂದಿನ;
		}
	}
}

/**
 * Adds an imagemap entry to the list
 *
 * \param c  The html content that the imagemap belongs to
 * \param n  The xmlNode representing the entry to add
 * \param base_url  Base URL for resolving relative URLs
 * \param entry  Pointer to list of entries
 * \param tagtype  The type of tag
 * \return false on memory exhaustion, true otherwise
 */
ಸ್ಥಿರ bool
imagemap_addtolist(ಸ್ಥಿರಾಂಕ ರಚನೆ html_content *c,
		   dom_ಗಂಟು *n,
		   nsurl *ತಳ_url,
		   ರಚನೆ mapentry **ನಮೂದು,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ಖಾಲಿ, *ಗುರಿ = ಖಾಲಿ, *shape = ಖಾಲಿ;
	dom_string *coords = ಖಾಲಿ;
	ರಚನೆ mapentry *new_map, *ತಾತ್ಕಾಲಿಕ;
	bool ret = true;

	ಆದರೆ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ಆದರೆ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			ಹೋಗು ok_ಹೊರಹೋಗು;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ಆದರೆ (exc != DOM_NO_ERR || href == ಖಾಲಿ) {
		/* No href="" attribute, skip this element */
		ಹೋಗು ok_ಹೊರಹೋಗು;
	}

	exc = dom_element_get_attribute(n, corestring_dom_ಗುರಿ, &ಗುರಿ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹೋಗು ok_ಹೊರಹೋಗು;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹೋಗು ok_ಹೊರಹೋಗು;
	}

	/* If there's no shape, we default to rectangles */
	ಆದರೆ (shape == ಖಾಲಿ)
		shape = dom_string_ref(corestring_dom_rect);

	ಆದರೆ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ಆದರೆ (exc != DOM_NO_ERR || coords == ಖಾಲಿ) {
			ಹೋಗು ok_ಹೊರಹೋಗು;
		}
	}

	new_map = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*new_map));
	ಆದರೆ (new_map == ಖಾಲಿ) {
		ಹೋಗು bad_ಹೊರಹೋಗು;
	}

	ಆದರೆ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹೋಗು bad_ಹೊರಹೋಗು;

	ಆದರೆ (box_ಹೊರತೆಗೆ_link(c, href, ತಳ_url, &new_map->url) == false)
		ಹೋಗು bad_ಹೊರಹೋಗು;

	ಆದರೆ (new_map->url == ಖಾಲಿ) {
		/* non-fatal error -> ignore this */
		ಹೋಗು ok_free_map_ಹೊರಹೋಗು;
	}

	ಆದರೆ (ಗುರಿ != ಖಾಲಿ) {
		/* Copy target dom string into the map data */
		new_map->ಗುರಿ = ಸ್ಮರಣೆ_ಕೊಡು(dom_string_byte_ಉದ್ದಳತೆ(ಗುರಿ) + 1);
		ಆದರೆ (new_map->ಗುರಿ == ಖಾಲಿ)
			ಹೋಗು bad_ಹೊರಹೋಗು;

		ಸ್ಮರಣೆ_ನಕಲು(new_map->ಗುರಿ,
		       dom_string_ದತ್ತಾಂಶ(ಗುರಿ),
		       dom_string_byte_ಉದ್ದಳತೆ(ಗುರಿ));

		new_map->ಗುರಿ[dom_string_byte_ಉದ್ದಳತೆ(ಗುರಿ)] = 0;
	}

	ಆದರೆ (new_map->type != IMAGEMAP_DEFAULT) {
		ಪೂರ್ಣಾಂಕ x, y;
		ದಶಮಾಂಶ *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		ಅಕ್ಷರ *val = strtok((ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(coords), ",");
		ಪೂರ್ಣಾಂಕ ಅಂಕೆ = 1;

		ಆಯ್ಕೆ (new_map->type) {
		ಸ್ಥಿತಿ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ಆಗ (val != ಖಾಲಿ && ಅಂಕೆ <= 4) {
				ಆಯ್ಕೆ (ಅಂಕೆ) {
				ಸ್ಥಿತಿ 1:
					new_map->bounds.rect.x0 = atoi(val);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 2:
					new_map->bounds.rect.y0 = atoi(val);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 3:
					new_map->bounds.rect.x1 = atoi(val);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 4:
					new_map->bounds.rect.y1 = atoi(val);
					ನಿಲ್ಲಿಸು;
				}

				ಅಂಕೆ++;
				val = strtok(ಖಾಲಿ, ",");
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ಆಗ (val != ಖಾಲಿ && ಅಂಕೆ <= 3) {
				ಆಯ್ಕೆ (ಅಂಕೆ) {
				ಸ್ಥಿತಿ 1:
					new_map->bounds.circle.x = atoi(val);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 2:
					new_map->bounds.circle.y = atoi(val);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 3:
					new_map->bounds.circle.r = atoi(val);
					ನಿಲ್ಲಿಸು;
				}

				ಅಂಕೆ++;
				val = strtok(ಖಾಲಿ, ",");
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ಖಾಲಿ;
			new_map->bounds.poly.ycoords = ಖಾಲಿ;

			ಆಗ (val != ಖಾಲಿ) {
				x = atoi(val);

				val = strtok(ಖಾಲಿ, ",");
				ಆದರೆ (val == ಖಾಲಿ)
					ನಿಲ್ಲಿಸು;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						ಅಂಕೆ * ಗಾತ್ರ(ದಶಮಾಂಶ));
				ಆದರೆ (xcoords == ಖಾಲಿ) {
					ಹೋಗು bad_ಹೊರಹೋಗು;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					ಅಂಕೆ * ಗಾತ್ರ(ದಶಮಾಂಶ));
				ಆದರೆ (ycoords == ಖಾಲಿ) {
					ಹೋಗು bad_ಹೊರಹೋಗು;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[ಅಂಕೆ - 1] = x;
				new_map->bounds.poly.ycoords[ಅಂಕೆ - 1] = y;

				ಅಂಕೆ++;
				val = strtok(ಖಾಲಿ, ",");
			}

			new_map->bounds.poly.ಅಂಕೆ = ಅಂಕೆ - 1;

			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}
	}

	new_map->ಮುಂದಿನ = ಖಾಲಿ;

	ಆದರೆ (*ನಮೂದು) {
		/* add to END of list */
		ಗಾಗಿ (ತಾತ್ಕಾಲಿಕ = (*ನಮೂದು); ತಾತ್ಕಾಲಿಕ->ಮುಂದಿನ != ಖಾಲಿ; ತಾತ್ಕಾಲಿಕ = ತಾತ್ಕಾಲಿಕ->ಮುಂದಿನ)
			;
		ತಾತ್ಕಾಲಿಕ->ಮುಂದಿನ = new_map;
	} ಇಲ್ಲದಿದ್ದರೆ {
		(*ನಮೂದು) = new_map;
	}

	/* All good, linked in, let's clean up */
	ಹೋಗು ok_ಹೊರಹೋಗು;

bad_ಹೊರಹೋಗು:
	ret = false;
ok_free_map_ಹೊರಹೋಗು:
	ಆದರೆ (new_map != ಖಾಲಿ) {
		ಆದರೆ (new_map->url != ಖಾಲಿ)
			nsurl_unref(new_map->url);
		ಆದರೆ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ಖಾಲಿ)
			ಬಿಡುಗಡೆ(new_map->bounds.poly.ycoords);
		ಆದರೆ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ಖಾಲಿ)
			ಬಿಡುಗಡೆ(new_map->bounds.poly.xcoords);
		ಆದರೆ (new_map->ಗುರಿ != ಖಾಲಿ)
			ಬಿಡುಗಡೆ(new_map->ಗುರಿ);

		ಬಿಡುಗಡೆ(new_map);
	}
ok_ಹೊರಹೋಗು:
	ಆದರೆ (href != ಖಾಲಿ)
		dom_string_unref(href);
	ಆದರೆ (ಗುರಿ != ಖಾಲಿ)
		dom_string_unref(ಗುರಿ);
	ಆದರೆ (shape != ಖಾಲಿ)
		dom_string_unref(shape);
	ಆದರೆ (coords != ಖಾಲಿ)
		dom_string_unref(coords);

	ಹಿಂತಿರುಗಿ ret;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \param tname The sub-tags to consider on this pass
 * \return false on memory exhaustion, true otherwise
 */
ಸ್ಥಿರ bool
imagemap_ಹೊರತೆಗೆ_map_entries(dom_ಗಂಟು *ಗಂಟು, html_content *c,
			     ರಚನೆ mapentry **ನಮೂದು, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ent;
	uint32_t ಟ್ಯಾಗ್_ಎಣಿಕೆ;

	exc = dom_element_get_elements_by_ಟ್ಯಾಗ್_name(ಗಂಟು, tname, &nlist);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_nodelist_get_ಉದ್ದಳತೆ(nlist, &ಟ್ಯಾಗ್_ಎಣಿಕೆ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ಹಿಂತಿರುಗಿ false;
	}

	ಗಾಗಿ (ent = 0; ent < ಟ್ಯಾಗ್_ಎಣಿಕೆ; ++ent) {
		dom_ಗಂಟು *subnode;

		exc = dom_nodelist_ಅಂಶ(nlist, ent, &subnode);
		ಆದರೆ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ಹಿಂತಿರುಗಿ false;
		}
		ಆದರೆ (imagemap_addtolist(c, subnode, c->ತಳ_url,
				       ನಮೂದು, tname) == false) {
			dom_ಗಂಟು_unref(subnode);
			dom_nodelist_unref(nlist);
			ಹಿಂತಿರುಗಿ false;
		}
		dom_ಗಂಟು_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ಸ್ಥಿರ bool imagemap_ಹೊರತೆಗೆ_map(dom_ಗಂಟು *ಗಂಟು, html_content *c,
		ರಚನೆ mapentry **ನಮೂದು)
{
	ಆದರೆ (imagemap_ಹೊರತೆಗೆ_map_entries(ಗಂಟು, c, ನಮೂದು,
			corestring_dom_area) == false)
		ಹಿಂತಿರುಗಿ false;
	ಹಿಂತಿರುಗಿ imagemap_ಹೊರತೆಗೆ_map_entries(ಗಂಟು, c, ನಮೂದು,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_ಹೊರತೆಗೆ(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_ಟ್ಯಾಗ್_name(c->document,
						    corestring_dom_map,
						    &nlist);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	exc = dom_nodelist_get_ಉದ್ದಳತೆ(nlist, &maybe_maps);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		ಹೋಗು ಹೊರಹೋಗು_nlist;
	}

	ಗಾಗಿ (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ಗಂಟು *ಗಂಟು;
		dom_string *name;
		exc = dom_nodelist_ಅಂಶ(nlist, mapnr, &ಗಂಟು);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			ಹೋಗು ಹೊರಹೋಗು_nlist;
		}

		exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_id,
						&name);
		ಆದರೆ (exc != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(ಗಂಟು);
			ret = NSERROR_DOM;
			ಹೋಗು ಹೊರಹೋಗು_nlist;
		}

		ಆದರೆ (name == ಖಾಲಿ) {
			exc = dom_element_get_attribute(ಗಂಟು,
							corestring_dom_name,
							&name);
			ಆದರೆ (exc != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_DOM;
				ಹೋಗು ಹೊರಹೋಗು_nlist;
			}
		}

		ಆದರೆ (name != ಖಾಲಿ) {
			ರಚನೆ mapentry *ನಮೂದು = ಖಾಲಿ;
			ಆದರೆ (imagemap_ಹೊರತೆಗೆ_map(ಗಂಟು, c, &ನಮೂದು) == false) {
				ಆದರೆ (ನಮೂದು != ಖಾಲಿ) {
					imagemap_freelist(ನಮೂದು);
				}

				dom_string_unref(name);
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_NOMEM; /** @todo check this */
				ಹೋಗು ಹೊರಹೋಗು_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ಆದರೆ ((ನಮೂದು != ಖಾಲಿ) &&
			    (imagemap_ಕೂಡಿಸು(c, name, ನಮೂದು) == false)) {
				imagemap_freelist(ನಮೂದು);

				dom_string_unref(name);
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_NOMEM; /** @todo check this */
				ಹೋಗು ಹೊರಹೋಗು_nlist;
			}
		}

		dom_string_unref(name);
		dom_ಗಂಟು_unref(ಗಂಟು);
	}

ಹೊರಹೋಗು_nlist:

	dom_nodelist_unref(nlist);

	ಹಿಂತಿರುಗಿ ret;
}

/**
 * Test if a point lies within an arbitrary polygon
 * Modified from comp.graphics.algorithms FAQ 2.03
 *
 * \param num Number of vertices
 * \param xpt Array of x coordinates
 * \param ypt Array of y coordinates
 * \param x Left hand edge of containing box
 * \param y Top edge of containing box
 * \param click_x X coordinate of click
 * \param click_y Y coordinate of click
 * \return 1 if point is in polygon, 0 if outside. 0 or 1 if on boundary
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
imagemap_ಬಿಂದು_ಒಳ_poly(ಪೂರ್ಣಾಂಕ ಅಂಕೆ, ದಶಮಾಂಶ *xpt, ದಶಮಾಂಶ *ypt, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ x,
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ y, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_x,	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_y)
{
	ಪೂರ್ಣಾಂಕ i, j, c = 0;

	ಪ್ರತಿಜ್ಞಾ(xpt != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ypt != ಖಾಲಿ);

	ಗಾಗಿ (i = 0, j = ಅಂಕೆ - 1; i < ಅಂಕೆ; j = i++) {
		ಆದರೆ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ಹಿಂತಿರುಗಿ c;
}

/**
 * Retrieve url associated with imagemap entry
 *
 * \param c        The containing content
 * \param key      The map name to search for
 * \param x        The left edge of the containing box
 * \param y        The top edge of the containing box
 * \param click_x  The horizontal location of the click
 * \param click_y  The vertical location of the click
 * \param target   Pointer to location to receive target pointer (if any)
 * \return The url associated with this area, or NULL if not found
 */
nsurl *imagemap_get(ರಚನೆ html_content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ,
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ x, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ y,
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_x, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_y,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಗುರಿ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ = 0;
	ರಚನೆ imagemap *map;
	ರಚನೆ mapentry *ನಮೂದು;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cx, cy;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (ಕೀಲಿ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (c->imagemaps == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಸ್ಥಾನ = imagemap_hash(ಕೀಲಿ);

	ಗಾಗಿ (map = c->imagemaps[ಸ್ಥಾನ]; map != ಖಾಲಿ; map = map->ಮುಂದಿನ) {
		ಆದರೆ (map->ಕೀಲಿ != ಖಾಲಿ && strcasecmp(map->ಕೀಲಿ, ಕೀಲಿ) == 0)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (map == ಖಾಲಿ || map->list == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಗಾಗಿ (ನಮೂದು = map->list; ನಮೂದು; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		ಆಯ್ಕೆ (ನಮೂದು->type) {
		ಸ್ಥಿತಿ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ಆದರೆ (ಗುರಿ)
				*ಗುರಿ = ನಮೂದು->ಗುರಿ;
			ಹಿಂತಿರುಗಿ ನಮೂದು->url;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IMAGEMAP_RECT:
			ಆದರೆ (click_x >= x + ನಮೂದು->bounds.rect.x0 &&
				    click_x <= x + ನಮೂದು->bounds.rect.x1 &&
				    click_y >= y + ನಮೂದು->bounds.rect.y0 &&
				    click_y <= y + ನಮೂದು->bounds.rect.y1) {
				ಆದರೆ (ಗುರಿ)
					*ಗುರಿ = ನಮೂದು->ಗುರಿ;
				ಹಿಂತಿರುಗಿ ನಮೂದು->url;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IMAGEMAP_CIRCLE:
			cx = x + ನಮೂದು->bounds.circle.x - click_x;
			cy = y + ನಮೂದು->bounds.circle.y - click_y;
			ಆದರೆ ((cx * cx + cy * cy) <=
				(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ನಮೂದು->bounds.circle.r *
					ನಮೂದು->bounds.circle.r)) {
				ಆದರೆ (ಗುರಿ)
					*ಗುರಿ = ನಮೂದು->ಗುರಿ;
				ಹಿಂತಿರುಗಿ ನಮೂದು->url;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IMAGEMAP_POLY:
			ಆದರೆ (imagemap_ಬಿಂದು_ಒಳ_poly(ನಮೂದು->bounds.poly.ಅಂಕೆ,
					ನಮೂದು->bounds.poly.xcoords,
					ನಮೂದು->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ಆದರೆ (ಗುರಿ)
					*ಗುರಿ = ನಮೂದು->ಗುರಿ;
				ಹಿಂತಿರುಗಿ ನಮೂದು->url;
			}
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (ಗುರಿ)
		*ಗುರಿ = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
