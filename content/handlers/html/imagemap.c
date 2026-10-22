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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_construct.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/imagemap.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HASH_ਮਾਪ 31 /* fixed size hash table */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_ਇੰਦਰਾਜ_type;

ਬਣਤਰ mapentry {
	imagemap_ਇੰਦਰਾਜ_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;			/**< target frame (if any) */
	ਸੰਘ {
		ਬਣਤਰ {
			ਪੂਰਨ x;		/**< x coordinate of centre */
			ਪੂਰਨ y;		/**< y coordinate of center */
			ਪੂਰਨ r;		/**< radius of circle */
		} circle;
		ਬਣਤਰ {
			ਪੂਰਨ x0;		/**< left hand edge */
			ਪੂਰਨ y0;		/**< top edge */
			ਪੂਰਨ x1;		/**< right hand edge */
			ਪੂਰਨ y1;		/**< bottom edge */
		} rect;
		ਬਣਤਰ {
			ਪੂਰਨ ਅੰਕ;	/**< number of points */
			ਦਸ਼ਮਲਵ *xcoords;	/**< x coordinates */
			ਦਸ਼ਮਲਵ *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ਬਣਤਰ mapentry *ਅਗਲਾ;		/**< next entry in list */
};

ਬਣਤਰ imagemap {
	ਅੱਖਰ *ਕੁੰਜੀ;		/**< key for this entry */
	ਬਣਤਰ mapentry *list;	/**< pointer to linked list of entries */
	ਬਣਤਰ imagemap *ਅਗਲਾ;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ਸਥਿਰ bool imagemap_create(html_content *c)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->imagemaps == ਨਿਰਾ) {
		c->imagemaps = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(HASH_ਮਾਪ, ਆਕਾਰ(ਬਣਤਰ imagemap *));
		ਜੇ (c->imagemaps == ਨਿਰਾ) {
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ imagemap_hash(ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ z = 0;

	ਜੇ (ਕੁੰਜੀ == 0) ਵਾਪਸ 0;

	ਲਈ (; *ਕੁੰਜੀ != 0; ਕੁੰਜੀ++) {
		z += *ਕੁੰਜੀ & 0x1f;
	}

	ਵਾਪਸ (z % (HASH_ਮਾਪ - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ਸਥਿਰ bool
imagemap_ਜੋੜੋ(html_content *c, dom_string *ਕੁੰਜੀ, ਬਣਤਰ mapentry *list)
{
	ਬਣਤਰ imagemap *map;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਕੁੰਜੀ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(list != ਨਿਰਾ);

	ਜੇ (imagemap_create(c) == false)
		ਵਾਪਸ false;

	map = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*map));
	ਜੇ (map == ਨਿਰਾ)
		ਵਾਪਸ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ਕੁੰਜੀ = strdup(dom_string_ਅੰਕੜੇ(ਕੁੰਜੀ));
	ਜੇ (map->ਕੁੰਜੀ == ਨਿਰਾ) {
		ਮੁਕਤ(map);
		ਵਾਪਸ false;
	}

	map->list = list;

	ਥਾਂ = imagemap_hash(map->ਕੁੰਜੀ);

	map->ਅਗਲਾ = c->imagemaps[ਥਾਂ];
	c->imagemaps[ਥਾਂ] = map;

	ਵਾਪਸ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ਸਥਿਰ ਖਾਲੀ imagemap_freelist(ਬਣਤਰ mapentry *list)
{
	ਬਣਤਰ mapentry *ਇੰਦਰਾਜ, *prev;

	ਪ੍ਰਤਿਜ੍ਞਾ(list != ਨਿਰਾ);

	ਇੰਦਰਾਜ = list;

	ਜਦੋਂ (ਇੰਦਰਾਜ != ਨਿਰਾ) {
		prev = ਇੰਦਰਾਜ;

		nsurl_unref(ਇੰਦਰਾਜ->url);

		ਜੇ (ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ)
			ਮੁਕਤ(ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ);

		ਜੇ (ਇੰਦਰਾਜ->type == IMAGEMAP_POLY) {
			ਮੁਕਤ(ਇੰਦਰਾਜ->bounds.poly.xcoords);
			ਮੁਕਤ(ਇੰਦਰਾਜ->bounds.poly.ycoords);
		}

		ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ;
		ਮੁਕਤ(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
ਖਾਲੀ imagemap_destroy(html_content *c)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	/* no imagemaps -> return */
	ਜੇ (c->imagemaps == ਨਿਰਾ)
		ਵਾਪਸ;

	ਲਈ (i = 0; i != HASH_ਮਾਪ; i++) {
		ਬਣਤਰ imagemap *map, *ਅਗਲਾ;

		map = c->imagemaps[i];
		ਜਦੋਂ (map != ਨਿਰਾ) {
			ਅਗਲਾ = map->ਅਗਲਾ;
			imagemap_freelist(map->list);
			ਮੁਕਤ(map->ਕੁੰਜੀ);
			ਮੁਕਤ(map);
			map = ਅਗਲਾ;
		}
	}

	ਮੁਕਤ(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
ਖਾਲੀ imagemap_dump(html_content *c)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਪੂਰਨ j;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->imagemaps == ਨਿਰਾ)
		ਵਾਪਸ;

	ਲਈ (i = 0; i != HASH_ਮਾਪ; i++) {
		ਬਣਤਰ imagemap *map;
		ਬਣਤਰ mapentry *ਇੰਦਰਾਜ;

		map = c->imagemaps[i];
		ਜਦੋਂ (map != ਨਿਰਾ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ਕੁੰਜੀ);

			ਲਈ (ਇੰਦਰਾਜ = map->list; ਇੰਦਰਾਜ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
				ਚੋਣ (ਇੰਦਰਾਜ->type) {
				ਹਾਲਤ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(ਇੰਦਰਾਜ->url));
					ਰੋਕੋ;
				ਹਾਲਤ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(ਇੰਦਰਾਜ->url),
					      ਇੰਦਰਾਜ->bounds.rect.x0,
					      ਇੰਦਰਾਜ->bounds.rect.y0,
					      ਇੰਦਰਾਜ->bounds.rect.x1,
					      ਇੰਦਰਾਜ->bounds.rect.y1);
					ਰੋਕੋ;
				ਹਾਲਤ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(ਇੰਦਰਾਜ->url),
					      ਇੰਦਰਾਜ->bounds.circle.x,
					      ਇੰਦਰਾਜ->bounds.circle.y,
					      ਇੰਦਰਾਜ->bounds.circle.r);
					ਰੋਕੋ;
				ਹਾਲਤ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(ਇੰਦਰਾਜ->url));
					ਲਈ (j = 0; j != ਇੰਦਰਾਜ->bounds.poly.ਅੰਕ;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(ਪੂਰਨ)ਇੰਦਰਾਜ->bounds.poly.xcoords[j],
							(ਪੂਰਨ)ਇੰਦਰਾਜ->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ਰੋਕੋ;
				}
			}
			map = map->ਅਗਲਾ;
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
ਸਥਿਰ bool
imagemap_addtolist(ਅਚਲ ਬਣਤਰ html_content *c,
		   dom_ਗੰਢ *n,
		   nsurl *ਅਧਾਰ_url,
		   ਬਣਤਰ mapentry **ਇੰਦਰਾਜ,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ਨਿਰਾ, *ਨਿਸ਼ਾਨਾ = ਨਿਰਾ, *shape = ਨਿਰਾ;
	dom_string *coords = ਨਿਰਾ;
	ਬਣਤਰ mapentry *new_map, *ਆਰਜ਼ੀ;
	bool ret = true;

	ਜੇ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ਜੇ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			ਜਾਓ ok_ਬਾਹਰਲਾ;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ਜੇ (exc != DOM_NO_ERR || href == ਨਿਰਾ) {
		/* No href="" attribute, skip this element */
		ਜਾਓ ok_ਬਾਹਰਲਾ;
	}

	exc = dom_element_get_attribute(n, corestring_dom_ਨਿਸ਼ਾਨਾ, &ਨਿਸ਼ਾਨਾ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਜਾਓ ok_ਬਾਹਰਲਾ;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ਜੇ (exc != DOM_NO_ERR) {
		ਜਾਓ ok_ਬਾਹਰਲਾ;
	}

	/* If there's no shape, we default to rectangles */
	ਜੇ (shape == ਨਿਰਾ)
		shape = dom_string_ref(corestring_dom_rect);

	ਜੇ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ਜੇ (exc != DOM_NO_ERR || coords == ਨਿਰਾ) {
			ਜਾਓ ok_ਬਾਹਰਲਾ;
		}
	}

	new_map = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*new_map));
	ਜੇ (new_map == ਨਿਰਾ) {
		ਜਾਓ bad_ਬਾਹਰਲਾ;
	}

	ਜੇ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ਨਹੀਂਤਾਂ
		ਜਾਓ bad_ਬਾਹਰਲਾ;

	ਜੇ (box_ਕੱਢੋ_link(c, href, ਅਧਾਰ_url, &new_map->url) == false)
		ਜਾਓ bad_ਬਾਹਰਲਾ;

	ਜੇ (new_map->url == ਨਿਰਾ) {
		/* non-fatal error -> ignore this */
		ਜਾਓ ok_free_map_ਬਾਹਰਲਾ;
	}

	ਜੇ (ਨਿਸ਼ਾਨਾ != ਨਿਰਾ) {
		/* Copy target dom string into the map data */
		new_map->ਨਿਸ਼ਾਨਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(dom_string_byte_ਲੰਮਾਈ(ਨਿਸ਼ਾਨਾ) + 1);
		ਜੇ (new_map->ਨਿਸ਼ਾਨਾ == ਨਿਰਾ)
			ਜਾਓ bad_ਬਾਹਰਲਾ;

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(new_map->ਨਿਸ਼ਾਨਾ,
		       dom_string_ਅੰਕੜੇ(ਨਿਸ਼ਾਨਾ),
		       dom_string_byte_ਲੰਮਾਈ(ਨਿਸ਼ਾਨਾ));

		new_map->ਨਿਸ਼ਾਨਾ[dom_string_byte_ਲੰਮਾਈ(ਨਿਸ਼ਾਨਾ)] = 0;
	}

	ਜੇ (new_map->type != IMAGEMAP_DEFAULT) {
		ਪੂਰਨ x, y;
		ਦਸ਼ਮਲਵ *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		ਅੱਖਰ *val = strtok((ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(coords), ",");
		ਪੂਰਨ ਅੰਕ = 1;

		ਚੋਣ (new_map->type) {
		ਹਾਲਤ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ਜਦੋਂ (val != ਨਿਰਾ && ਅੰਕ <= 4) {
				ਚੋਣ (ਅੰਕ) {
				ਹਾਲਤ 1:
					new_map->bounds.rect.x0 = atoi(val);
					ਰੋਕੋ;
				ਹਾਲਤ 2:
					new_map->bounds.rect.y0 = atoi(val);
					ਰੋਕੋ;
				ਹਾਲਤ 3:
					new_map->bounds.rect.x1 = atoi(val);
					ਰੋਕੋ;
				ਹਾਲਤ 4:
					new_map->bounds.rect.y1 = atoi(val);
					ਰੋਕੋ;
				}

				ਅੰਕ++;
				val = strtok(ਨਿਰਾ, ",");
			}
			ਰੋਕੋ;
		ਹਾਲਤ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ਜਦੋਂ (val != ਨਿਰਾ && ਅੰਕ <= 3) {
				ਚੋਣ (ਅੰਕ) {
				ਹਾਲਤ 1:
					new_map->bounds.circle.x = atoi(val);
					ਰੋਕੋ;
				ਹਾਲਤ 2:
					new_map->bounds.circle.y = atoi(val);
					ਰੋਕੋ;
				ਹਾਲਤ 3:
					new_map->bounds.circle.r = atoi(val);
					ਰੋਕੋ;
				}

				ਅੰਕ++;
				val = strtok(ਨਿਰਾ, ",");
			}
			ਰੋਕੋ;
		ਹਾਲਤ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ਨਿਰਾ;
			new_map->bounds.poly.ycoords = ਨਿਰਾ;

			ਜਦੋਂ (val != ਨਿਰਾ) {
				x = atoi(val);

				val = strtok(ਨਿਰਾ, ",");
				ਜੇ (val == ਨਿਰਾ)
					ਰੋਕੋ;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						ਅੰਕ * ਆਕਾਰ(ਦਸ਼ਮਲਵ));
				ਜੇ (xcoords == ਨਿਰਾ) {
					ਜਾਓ bad_ਬਾਹਰਲਾ;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					ਅੰਕ * ਆਕਾਰ(ਦਸ਼ਮਲਵ));
				ਜੇ (ycoords == ਨਿਰਾ) {
					ਜਾਓ bad_ਬਾਹਰਲਾ;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[ਅੰਕ - 1] = x;
				new_map->bounds.poly.ycoords[ਅੰਕ - 1] = y;

				ਅੰਕ++;
				val = strtok(ਨਿਰਾ, ",");
			}

			new_map->bounds.poly.ਅੰਕ = ਅੰਕ - 1;

			ਰੋਕੋ;
		ਮੂਲ:
			ਰੋਕੋ;
		}
	}

	new_map->ਅਗਲਾ = ਨਿਰਾ;

	ਜੇ (*ਇੰਦਰਾਜ) {
		/* add to END of list */
		ਲਈ (ਆਰਜ਼ੀ = (*ਇੰਦਰਾਜ); ਆਰਜ਼ੀ->ਅਗਲਾ != ਨਿਰਾ; ਆਰਜ਼ੀ = ਆਰਜ਼ੀ->ਅਗਲਾ)
			;
		ਆਰਜ਼ੀ->ਅਗਲਾ = new_map;
	} ਨਹੀਂਤਾਂ {
		(*ਇੰਦਰਾਜ) = new_map;
	}

	/* All good, linked in, let's clean up */
	ਜਾਓ ok_ਬਾਹਰਲਾ;

bad_ਬਾਹਰਲਾ:
	ret = false;
ok_free_map_ਬਾਹਰਲਾ:
	ਜੇ (new_map != ਨਿਰਾ) {
		ਜੇ (new_map->url != ਨਿਰਾ)
			nsurl_unref(new_map->url);
		ਜੇ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ਨਿਰਾ)
			ਮੁਕਤ(new_map->bounds.poly.ycoords);
		ਜੇ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ਨਿਰਾ)
			ਮੁਕਤ(new_map->bounds.poly.xcoords);
		ਜੇ (new_map->ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
			ਮੁਕਤ(new_map->ਨਿਸ਼ਾਨਾ);

		ਮੁਕਤ(new_map);
	}
ok_ਬਾਹਰਲਾ:
	ਜੇ (href != ਨਿਰਾ)
		dom_string_unref(href);
	ਜੇ (ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
		dom_string_unref(ਨਿਸ਼ਾਨਾ);
	ਜੇ (shape != ਨਿਰਾ)
		dom_string_unref(shape);
	ਜੇ (coords != ਨਿਰਾ)
		dom_string_unref(coords);

	ਵਾਪਸ ret;
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
ਸਥਿਰ bool
imagemap_ਕੱਢੋ_map_entries(dom_ਗੰਢ *ਗੰਢ, html_content *c,
			     ਬਣਤਰ mapentry **ਇੰਦਰਾਜ, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ent;
	uint32_t ਟੈਗ_ਗਿਣਤੀ;

	exc = dom_element_get_elements_by_ਟੈਗ_name(ਗੰਢ, tname, &nlist);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	exc = dom_nodelist_get_ਲੰਮਾਈ(nlist, &ਟੈਗ_ਗਿਣਤੀ);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ਵਾਪਸ false;
	}

	ਲਈ (ent = 0; ent < ਟੈਗ_ਗਿਣਤੀ; ++ent) {
		dom_ਗੰਢ *subnode;

		exc = dom_nodelist_ਵਸਤੂ(nlist, ent, &subnode);
		ਜੇ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ਵਾਪਸ false;
		}
		ਜੇ (imagemap_addtolist(c, subnode, c->ਅਧਾਰ_url,
				       ਇੰਦਰਾਜ, tname) == false) {
			dom_ਗੰਢ_unref(subnode);
			dom_nodelist_unref(nlist);
			ਵਾਪਸ false;
		}
		dom_ਗੰਢ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ਵਾਪਸ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ਸਥਿਰ bool imagemap_ਕੱਢੋ_map(dom_ਗੰਢ *ਗੰਢ, html_content *c,
		ਬਣਤਰ mapentry **ਇੰਦਰਾਜ)
{
	ਜੇ (imagemap_ਕੱਢੋ_map_entries(ਗੰਢ, c, ਇੰਦਰਾਜ,
			corestring_dom_area) == false)
		ਵਾਪਸ false;
	ਵਾਪਸ imagemap_ਕੱਢੋ_map_entries(ਗੰਢ, c, ਇੰਦਰਾਜ,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_ਕੱਢੋ(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_ਟੈਗ_name(c->document,
						    corestring_dom_map,
						    &nlist);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ NSERROR_DOM;
	}

	exc = dom_nodelist_get_ਲੰਮਾਈ(nlist, &maybe_maps);
	ਜੇ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		ਜਾਓ ਬਾਹਰਲਾ_nlist;
	}

	ਲਈ (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ਗੰਢ *ਗੰਢ;
		dom_string *name;
		exc = dom_nodelist_ਵਸਤੂ(nlist, mapnr, &ਗੰਢ);
		ਜੇ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			ਜਾਓ ਬਾਹਰਲਾ_nlist;
		}

		exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_id,
						&name);
		ਜੇ (exc != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(ਗੰਢ);
			ret = NSERROR_DOM;
			ਜਾਓ ਬਾਹਰਲਾ_nlist;
		}

		ਜੇ (name == ਨਿਰਾ) {
			exc = dom_element_get_attribute(ਗੰਢ,
							corestring_dom_name,
							&name);
			ਜੇ (exc != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(ਗੰਢ);
				ret = NSERROR_DOM;
				ਜਾਓ ਬਾਹਰਲਾ_nlist;
			}
		}

		ਜੇ (name != ਨਿਰਾ) {
			ਬਣਤਰ mapentry *ਇੰਦਰਾਜ = ਨਿਰਾ;
			ਜੇ (imagemap_ਕੱਢੋ_map(ਗੰਢ, c, &ਇੰਦਰਾਜ) == false) {
				ਜੇ (ਇੰਦਰਾਜ != ਨਿਰਾ) {
					imagemap_freelist(ਇੰਦਰਾਜ);
				}

				dom_string_unref(name);
				dom_ਗੰਢ_unref(ਗੰਢ);
				ret = NSERROR_NOMEM; /** @todo check this */
				ਜਾਓ ਬਾਹਰਲਾ_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ਜੇ ((ਇੰਦਰਾਜ != ਨਿਰਾ) &&
			    (imagemap_ਜੋੜੋ(c, name, ਇੰਦਰਾਜ) == false)) {
				imagemap_freelist(ਇੰਦਰਾਜ);

				dom_string_unref(name);
				dom_ਗੰਢ_unref(ਗੰਢ);
				ret = NSERROR_NOMEM; /** @todo check this */
				ਜਾਓ ਬਾਹਰਲਾ_nlist;
			}
		}

		dom_string_unref(name);
		dom_ਗੰਢ_unref(ਗੰਢ);
	}

ਬਾਹਰਲਾ_nlist:

	dom_nodelist_unref(nlist);

	ਵਾਪਸ ret;
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
ਸਥਿਰ ਪੂਰਨ
imagemap_ਬਿੰਦੂ_ਵਿੱਚ_poly(ਪੂਰਨ ਅੰਕ, ਦਸ਼ਮਲਵ *xpt, ਦਸ਼ਮਲਵ *ypt, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ x,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ y, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_x,	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_y)
{
	ਪੂਰਨ i, j, c = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(xpt != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ypt != ਨਿਰਾ);

	ਲਈ (i = 0, j = ਅੰਕ - 1; i < ਅੰਕ; j = i++) {
		ਜੇ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ਵਾਪਸ c;
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
nsurl *imagemap_get(ਬਣਤਰ html_content *c, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ x, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ y,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_x, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_y,
		ਅਚਲ ਅੱਖਰ **ਨਿਸ਼ਾਨਾ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ = 0;
	ਬਣਤਰ imagemap *map;
	ਬਣਤਰ mapentry *ਇੰਦਰਾਜ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cx, cy;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (ਕੁੰਜੀ == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (c->imagemaps == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਥਾਂ = imagemap_hash(ਕੁੰਜੀ);

	ਲਈ (map = c->imagemaps[ਥਾਂ]; map != ਨਿਰਾ; map = map->ਅਗਲਾ) {
		ਜੇ (map->ਕੁੰਜੀ != ਨਿਰਾ && strcasecmp(map->ਕੁੰਜੀ, ਕੁੰਜੀ) == 0)
			ਰੋਕੋ;
	}

	ਜੇ (map == ਨਿਰਾ || map->list == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਲਈ (ਇੰਦਰਾਜ = map->list; ਇੰਦਰਾਜ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		ਚੋਣ (ਇੰਦਰਾਜ->type) {
		ਹਾਲਤ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ਜੇ (ਨਿਸ਼ਾਨਾ)
				*ਨਿਸ਼ਾਨਾ = ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ;
			ਵਾਪਸ ਇੰਦਰਾਜ->url;
			ਰੋਕੋ;
		ਹਾਲਤ IMAGEMAP_RECT:
			ਜੇ (click_x >= x + ਇੰਦਰਾਜ->bounds.rect.x0 &&
				    click_x <= x + ਇੰਦਰਾਜ->bounds.rect.x1 &&
				    click_y >= y + ਇੰਦਰਾਜ->bounds.rect.y0 &&
				    click_y <= y + ਇੰਦਰਾਜ->bounds.rect.y1) {
				ਜੇ (ਨਿਸ਼ਾਨਾ)
					*ਨਿਸ਼ਾਨਾ = ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ;
				ਵਾਪਸ ਇੰਦਰਾਜ->url;
			}
			ਰੋਕੋ;
		ਹਾਲਤ IMAGEMAP_CIRCLE:
			cx = x + ਇੰਦਰਾਜ->bounds.circle.x - click_x;
			cy = y + ਇੰਦਰਾਜ->bounds.circle.y - click_y;
			ਜੇ ((cx * cx + cy * cy) <=
				(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਇੰਦਰਾਜ->bounds.circle.r *
					ਇੰਦਰਾਜ->bounds.circle.r)) {
				ਜੇ (ਨਿਸ਼ਾਨਾ)
					*ਨਿਸ਼ਾਨਾ = ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ;
				ਵਾਪਸ ਇੰਦਰਾਜ->url;
			}
			ਰੋਕੋ;
		ਹਾਲਤ IMAGEMAP_POLY:
			ਜੇ (imagemap_ਬਿੰਦੂ_ਵਿੱਚ_poly(ਇੰਦਰਾਜ->bounds.poly.ਅੰਕ,
					ਇੰਦਰਾਜ->bounds.poly.xcoords,
					ਇੰਦਰਾਜ->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ਜੇ (ਨਿਸ਼ਾਨਾ)
					*ਨਿਸ਼ਾਨਾ = ਇੰਦਰਾਜ->ਨਿਸ਼ਾਨਾ;
				ਵਾਪਸ ਇੰਦਰਾਜ->url;
			}
			ਰੋਕੋ;
		}
	}

	ਜੇ (ਨਿਸ਼ਾਨਾ)
		*ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;

	ਵਾਪਸ ਨਿਰਾ;
}
