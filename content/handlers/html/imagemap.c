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

#define HASH_SIZE 31 /* fixed size hash table */

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

ಸಂರಚನಾ mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	ವರ್ಣ *target;			/**< target frame (if any) */
	ಸಙ್ಘ {
		ಸಂರಚನಾ {
			ಪೂರ್ಣಾಙ್ಕ x;		/**< x coordinate of centre */
			ಪೂರ್ಣಾಙ್ಕ y;		/**< y coordinate of center */
			ಪೂರ್ಣಾಙ್ಕ r;		/**< radius of circle */
		} circle;
		ಸಂರಚನಾ {
			ಪೂರ್ಣಾಙ್ಕ x0;		/**< left hand edge */
			ಪೂರ್ಣಾಙ್ಕ y0;		/**< top edge */
			ಪೂರ್ಣಾಙ್ಕ x1;		/**< right hand edge */
			ಪೂರ್ಣಾಙ್ಕ y1;		/**< bottom edge */
		} rect;
		ಸಂರಚನಾ {
			ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ;	/**< number of points */
			ಭಿನ್ನ *xcoords;	/**< x coordinates */
			ಭಿನ್ನ *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ಸಂರಚನಾ mapentry *next;		/**< next entry in list */
};

ಸಂರಚನಾ imagemap {
	ವರ್ಣ *ಕೀಲಿ;		/**< key for this entry */
	ಸಂರಚನಾ mapentry *list;	/**< pointer to linked list of entries */
	ಸಂರಚನಾ imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ಸ್ಥಿರ bool imagemap_create(html_content *c)
{
	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->imagemaps == ರಿಕ್ತ) {
		c->imagemaps = calloc(HASH_SIZE, ಪರಿಮಾಣ(ಸಂರಚನಾ imagemap *));
		ಯದಿ (c->imagemaps == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ imagemap_hash(ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ z = 0;

	ಯದಿ (ಕೀಲಿ == 0) ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತಿ (; *ಕೀಲಿ != 0; ಕೀಲಿ++) {
		z += *ಕೀಲಿ & 0x1f;
	}

	ಪ್ರತ್ಯಾಗಮ (z % (HASH_SIZE - 1)) + 1;
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
imagemap_add(html_content *c, dom_string *ಕೀಲಿ, ಸಂರಚನಾ mapentry *list)
{
	ಸಂರಚನಾ imagemap *map;
	unsigned ಪೂರ್ಣಾಙ್ಕ slot;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(ಕೀಲಿ != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(list != ರಿಕ್ತ);

	ಯದಿ (imagemap_create(c) == false)
		ಪ್ರತ್ಯಾಗಮ false;

	map = calloc(1, ಪರಿಮಾಣ(*map));
	ಯದಿ (map == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ಕೀಲಿ = strdup(dom_string_data(ಕೀಲಿ));
	ಯದಿ (map->ಕೀಲಿ == ರಿಕ್ತ) {
		ಮುಕ್ತ(map);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	map->list = list;

	slot = imagemap_hash(map->ಕೀಲಿ);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ಸ್ಥಿರ ಶೂನ್ಯ imagemap_freelist(ಸಂರಚನಾ mapentry *list)
{
	ಸಂರಚನಾ mapentry *entry, *prev;

	ಪ್ರತಿಜ್ಞಾ(list != ರಿಕ್ತ);

	entry = list;

	ಯಾವತ್ (entry != ರಿಕ್ತ) {
		prev = entry;

		nsurl_unref(entry->url);

		ಯದಿ (entry->target)
			ಮುಕ್ತ(entry->target);

		ಯದಿ (entry->type == IMAGEMAP_POLY) {
			ಮುಕ್ತ(entry->bounds.poly.xcoords);
			ಮುಕ್ತ(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		ಮುಕ್ತ(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
ಶೂನ್ಯ imagemap_destroy(html_content *c)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	/* no imagemaps -> return */
	ಯದಿ (c->imagemaps == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	ಪ್ರತಿ (i = 0; i != HASH_SIZE; i++) {
		ಸಂರಚನಾ imagemap *map, *next;

		map = c->imagemaps[i];
		ಯಾವತ್ (map != ರಿಕ್ತ) {
			next = map->next;
			imagemap_freelist(map->list);
			ಮುಕ್ತ(map->ಕೀಲಿ);
			ಮುಕ್ತ(map);
			map = next;
		}
	}

	ಮುಕ್ತ(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
ಶೂನ್ಯ imagemap_dump(html_content *c)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪೂರ್ಣಾಙ್ಕ j;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->imagemaps == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	ಪ್ರತಿ (i = 0; i != HASH_SIZE; i++) {
		ಸಂರಚನಾ imagemap *map;
		ಸಂರಚನಾ mapentry *entry;

		map = c->imagemaps[i];
		ಯಾವತ್ (map != ರಿಕ್ತ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ಕೀಲಿ);

			ಪ್ರತಿ (entry = map->list; entry; entry = entry->next) {
				ವಿಕಲ್ಪ (entry->type) {
				ಪಕ್ಷ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					ಭಙ್ಗ;
				ಪಕ್ಷ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					ಭಙ್ಗ;
				ಪಕ್ಷ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					ಭಙ್ಗ;
				ಪಕ್ಷ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					ಪ್ರತಿ (j = 0; j != entry->bounds.poly.ಅಂಕೆ;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(ಪೂರ್ಣಾಙ್ಕ)entry->bounds.poly.xcoords[j],
							(ಪೂರ್ಣಾಙ್ಕ)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ಭಙ್ಗ;
				}
			}
			map = map->next;
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
imagemap_addtolist(ಧ್ರುವ ಸಂರಚನಾ html_content *c,
		   dom_ಗಂಟು *n,
		   nsurl *base_url,
		   ಸಂರಚನಾ mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ರಿಕ್ತ, *target = ರಿಕ್ತ, *shape = ರಿಕ್ತ;
	dom_string *coords = ರಿಕ್ತ;
	ಸಂರಚನಾ mapentry *new_map, *temp;
	bool ret = true;

	ಯದಿ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ಯದಿ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			ಗಚ್ಛ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ಯದಿ (exc != DOM_NO_ERR || href == ರಿಕ್ತ) {
		/* No href="" attribute, skip this element */
		ಗಚ್ಛ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಗಚ್ಛ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಗಚ್ಛ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	ಯದಿ (shape == ರಿಕ್ತ)
		shape = dom_string_ref(corestring_dom_rect);

	ಯದಿ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ಯದಿ (exc != DOM_NO_ERR || coords == ರಿಕ್ತ) {
			ಗಚ್ಛ ok_out;
		}
	}

	new_map = calloc(1, ಪರಿಮಾಣ(*new_map));
	ಯದಿ (new_map == ರಿಕ್ತ) {
		ಗಚ್ಛ bad_out;
	}

	ಯದಿ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ಅನ್ಯಥಾ
		ಗಚ್ಛ bad_out;

	ಯದಿ (box_extract_link(c, href, base_url, &new_map->url) == false)
		ಗಚ್ಛ bad_out;

	ಯದಿ (new_map->url == ರಿಕ್ತ) {
		/* non-fatal error -> ignore this */
		ಗಚ್ಛ ok_free_map_out;
	}

	ಯದಿ (target != ರಿಕ್ತ) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		ಯದಿ (new_map->target == ರಿಕ್ತ)
			ಗಚ್ಛ bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	ಯದಿ (new_map->type != IMAGEMAP_DEFAULT) {
		ಪೂರ್ಣಾಙ್ಕ x, y;
		ಭಿನ್ನ *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		ವರ್ಣ *val = strtok((ವರ್ಣ *)dom_string_data(coords), ",");
		ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ = 1;

		ವಿಕಲ್ಪ (new_map->type) {
		ಪಕ್ಷ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ಯಾವತ್ (val != ರಿಕ್ತ && ಅಂಕೆ <= 4) {
				ವಿಕಲ್ಪ (ಅಂಕೆ) {
				ಪಕ್ಷ 1:
					new_map->bounds.rect.x0 = atoi(val);
					ಭಙ್ಗ;
				ಪಕ್ಷ 2:
					new_map->bounds.rect.y0 = atoi(val);
					ಭಙ್ಗ;
				ಪಕ್ಷ 3:
					new_map->bounds.rect.x1 = atoi(val);
					ಭಙ್ಗ;
				ಪಕ್ಷ 4:
					new_map->bounds.rect.y1 = atoi(val);
					ಭಙ್ಗ;
				}

				ಅಂಕೆ++;
				val = strtok(ರಿಕ್ತ, ",");
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ಯಾವತ್ (val != ರಿಕ್ತ && ಅಂಕೆ <= 3) {
				ವಿಕಲ್ಪ (ಅಂಕೆ) {
				ಪಕ್ಷ 1:
					new_map->bounds.circle.x = atoi(val);
					ಭಙ್ಗ;
				ಪಕ್ಷ 2:
					new_map->bounds.circle.y = atoi(val);
					ಭಙ್ಗ;
				ಪಕ್ಷ 3:
					new_map->bounds.circle.r = atoi(val);
					ಭಙ್ಗ;
				}

				ಅಂಕೆ++;
				val = strtok(ರಿಕ್ತ, ",");
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ರಿಕ್ತ;
			new_map->bounds.poly.ycoords = ರಿಕ್ತ;

			ಯಾವತ್ (val != ರಿಕ್ತ) {
				x = atoi(val);

				val = strtok(ರಿಕ್ತ, ",");
				ಯದಿ (val == ರಿಕ್ತ)
					ಭಙ್ಗ;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						ಅಂಕೆ * ಪರಿಮಾಣ(ಭಿನ್ನ));
				ಯದಿ (xcoords == ರಿಕ್ತ) {
					ಗಚ್ಛ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					ಅಂಕೆ * ಪರಿಮಾಣ(ಭಿನ್ನ));
				ಯದಿ (ycoords == ರಿಕ್ತ) {
					ಗಚ್ಛ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[ಅಂಕೆ - 1] = x;
				new_map->bounds.poly.ycoords[ಅಂಕೆ - 1] = y;

				ಅಂಕೆ++;
				val = strtok(ರಿಕ್ತ, ",");
			}

			new_map->bounds.poly.ಅಂಕೆ = ಅಂಕೆ - 1;

			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}
	}

	new_map->next = ರಿಕ್ತ;

	ಯದಿ (*entry) {
		/* add to END of list */
		ಪ್ರತಿ (temp = (*entry); temp->next != ರಿಕ್ತ; temp = temp->next)
			;
		temp->next = new_map;
	} ಅನ್ಯಥಾ {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	ಗಚ್ಛ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	ಯದಿ (new_map != ರಿಕ್ತ) {
		ಯದಿ (new_map->url != ರಿಕ್ತ)
			nsurl_unref(new_map->url);
		ಯದಿ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ರಿಕ್ತ)
			ಮುಕ್ತ(new_map->bounds.poly.ycoords);
		ಯದಿ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ರಿಕ್ತ)
			ಮುಕ್ತ(new_map->bounds.poly.xcoords);
		ಯದಿ (new_map->target != ರಿಕ್ತ)
			ಮುಕ್ತ(new_map->target);

		ಮುಕ್ತ(new_map);
	}
ok_out:
	ಯದಿ (href != ರಿಕ್ತ)
		dom_string_unref(href);
	ಯದಿ (target != ರಿಕ್ತ)
		dom_string_unref(target);
	ಯದಿ (shape != ರಿಕ್ತ)
		dom_string_unref(shape);
	ಯದಿ (coords != ರಿಕ್ತ)
		dom_string_unref(coords);

	ಪ್ರತ್ಯಾಗಮ ret;
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
imagemap_extract_map_entries(dom_ಗಂಟು *ಗಂಟು, html_content *c,
			     ಸಂರಚನಾ mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_ಎಣಿಕೆ;

	exc = dom_element_get_elements_by_tag_name(ಗಂಟು, tname, &nlist);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_ಎಣಿಕೆ);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತಿ (ent = 0; ent < tag_ಎಣಿಕೆ; ++ent) {
		dom_ಗಂಟು *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		ಯದಿ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		ಯದಿ (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_ಗಂಟು_unref(subnode);
			dom_nodelist_unref(nlist);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		dom_ಗಂಟು_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ಸ್ಥಿರ bool imagemap_extract_map(dom_ಗಂಟು *ಗಂಟು, html_content *c,
		ಸಂರಚನಾ mapentry **entry)
{
	ಯದಿ (imagemap_extract_map_entries(ಗಂಟು, c, entry,
			corestring_dom_area) == false)
		ಪ್ರತ್ಯಾಗಮ false;
	ಪ್ರತ್ಯಾಗಮ imagemap_extract_map_entries(ಗಂಟು, c, entry,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_extract(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_tag_name(c->document,
						    corestring_dom_map,
						    &nlist);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	ಯದಿ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		ಗಚ್ಛ out_nlist;
	}

	ಪ್ರತಿ (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ಗಂಟು *ಗಂಟು;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &ಗಂಟು);
		ಯದಿ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			ಗಚ್ಛ out_nlist;
		}

		exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_id,
						&name);
		ಯದಿ (exc != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(ಗಂಟು);
			ret = NSERROR_DOM;
			ಗಚ್ಛ out_nlist;
		}

		ಯದಿ (name == ರಿಕ್ತ) {
			exc = dom_element_get_attribute(ಗಂಟು,
							corestring_dom_name,
							&name);
			ಯದಿ (exc != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_DOM;
				ಗಚ್ಛ out_nlist;
			}
		}

		ಯದಿ (name != ರಿಕ್ತ) {
			ಸಂರಚನಾ mapentry *entry = ರಿಕ್ತ;
			ಯದಿ (imagemap_extract_map(ಗಂಟು, c, &entry) == false) {
				ಯದಿ (entry != ರಿಕ್ತ) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_NOMEM; /** @todo check this */
				ಗಚ್ಛ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ಯದಿ ((entry != ರಿಕ್ತ) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_ಗಂಟು_unref(ಗಂಟು);
				ret = NSERROR_NOMEM; /** @todo check this */
				ಗಚ್ಛ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_ಗಂಟು_unref(ಗಂಟು);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ಪ್ರತ್ಯಾಗಮ ret;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
imagemap_point_in_poly(ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ, ಭಿನ್ನ *xpt, ಭಿನ್ನ *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	ಪೂರ್ಣಾಙ್ಕ i, j, c = 0;

	ಪ್ರತಿಜ್ಞಾ(xpt != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(ypt != ರಿಕ್ತ);

	ಪ್ರತಿ (i = 0, j = ಅಂಕೆ - 1; i < ಅಂಕೆ; j = i++) {
		ಯದಿ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ಪ್ರತ್ಯಾಗಮ c;
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
nsurl *imagemap_get(ಸಂರಚನಾ html_content *c, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ಧ್ರುವ ವರ್ಣ **target)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ slot = 0;
	ಸಂರಚನಾ imagemap *map;
	ಸಂರಚನಾ mapentry *entry;
	unsigned long cx, cy;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (ಕೀಲಿ == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (c->imagemaps == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	slot = imagemap_hash(ಕೀಲಿ);

	ಪ್ರತಿ (map = c->imagemaps[slot]; map != ರಿಕ್ತ; map = map->next) {
		ಯದಿ (map->ಕೀಲಿ != ರಿಕ್ತ && strcasecmp(map->ಕೀಲಿ, ಕೀಲಿ) == 0)
			ಭಙ್ಗ;
	}

	ಯದಿ (map == ರಿಕ್ತ || map->list == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತಿ (entry = map->list; entry; entry = entry->next) {
		ವಿಕಲ್ಪ (entry->type) {
		ಪಕ್ಷ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ಯದಿ (target)
				*target = entry->target;
			ಪ್ರತ್ಯಾಗಮ entry->url;
			ಭಙ್ಗ;
		ಪಕ್ಷ IMAGEMAP_RECT:
			ಯದಿ (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				ಯದಿ (target)
					*target = entry->target;
				ಪ್ರತ್ಯಾಗಮ entry->url;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			ಯದಿ ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				ಯದಿ (target)
					*target = entry->target;
				ಪ್ರತ್ಯಾಗಮ entry->url;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ IMAGEMAP_POLY:
			ಯದಿ (imagemap_point_in_poly(entry->bounds.poly.ಅಂಕೆ,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ಯದಿ (target)
					*target = entry->target;
				ಪ್ರತ್ಯಾಗಮ entry->url;
			}
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (target)
		*target = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
