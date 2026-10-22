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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>

#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/hlcache.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_construct.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

પ્રકારવ્યાખ્યા ગણના {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

સંરચના mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	અક્ષર *target;			/**< target frame (if any) */
	સંઘ {
		સંરચના {
			પૂર્ણાંક x;		/**< x coordinate of centre */
			પૂર્ણાંક y;		/**< y coordinate of center */
			પૂર્ણાંક r;		/**< radius of circle */
		} circle;
		સંરચના {
			પૂર્ણાંક x0;		/**< left hand edge */
			પૂર્ણાંક y0;		/**< top edge */
			પૂર્ણાંક x1;		/**< right hand edge */
			પૂર્ણાંક y1;		/**< bottom edge */
		} rect;
		સંરચના {
			પૂર્ણાંક અંક;	/**< number of points */
			દશાંશ *xcoords;	/**< x coordinates */
			દશાંશ *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	સંરચના mapentry *next;		/**< next entry in list */
};

સંરચના imagemap {
	અક્ષર *ચાવી;		/**< key for this entry */
	સંરચના mapentry *list;	/**< pointer to linked list of entries */
	સંરચના imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
સ્થિર bool imagemap_create(html_content *c)
{
	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->imagemaps == ખાલી) {
		c->imagemaps = calloc(HASH_SIZE, કદ(સંરચના imagemap *));
		જો (c->imagemaps == ખાલી) {
			પાછા false;
		}
	}

	પાછા true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
સ્થિર unsigned પૂર્ણાંક imagemap_hash(અચલ અક્ષર *ચાવી)
{
	unsigned પૂર્ણાંક z = 0;

	જો (ચાવી == 0) પાછા 0;

	માટે (; *ચાવી != 0; ચાવી++) {
		z += *ચાવી & 0x1f;
	}

	પાછા (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
સ્થિર bool
imagemap_add(html_content *c, dom_string *ચાવી, સંરચના mapentry *list)
{
	સંરચના imagemap *map;
	unsigned પૂર્ણાંક slot;

	પ્રતિજ્ઞા(c != ખાલી);
	પ્રતિજ્ઞા(ચાવી != ખાલી);
	પ્રતિજ્ઞા(list != ખાલી);

	જો (imagemap_create(c) == false)
		પાછા false;

	map = calloc(1, કદ(*map));
	જો (map == ખાલી)
		પાછા false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ચાવી = strdup(dom_string_data(ચાવી));
	જો (map->ચાવી == ખાલી) {
		મુક્ત(map);
		પાછા false;
	}

	map->list = list;

	slot = imagemap_hash(map->ચાવી);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	પાછા true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
સ્થિર શૂન્ય imagemap_freelist(સંરચના mapentry *list)
{
	સંરચના mapentry *entry, *prev;

	પ્રતિજ્ઞા(list != ખાલી);

	entry = list;

	જ્યારે (entry != ખાલી) {
		prev = entry;

		nsurl_unref(entry->url);

		જો (entry->target)
			મુક્ત(entry->target);

		જો (entry->type == IMAGEMAP_POLY) {
			મુક્ત(entry->bounds.poly.xcoords);
			મુક્ત(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		મુક્ત(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
શૂન્ય imagemap_destroy(html_content *c)
{
	unsigned પૂર્ણાંક i;

	પ્રતિજ્ઞા(c != ખાલી);

	/* no imagemaps -> return */
	જો (c->imagemaps == ખાલી)
		પાછા;

	માટે (i = 0; i != HASH_SIZE; i++) {
		સંરચના imagemap *map, *next;

		map = c->imagemaps[i];
		જ્યારે (map != ખાલી) {
			next = map->next;
			imagemap_freelist(map->list);
			મુક્ત(map->ચાવી);
			મુક્ત(map);
			map = next;
		}
	}

	મુક્ત(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
શૂન્ય imagemap_dump(html_content *c)
{
	unsigned પૂર્ણાંક i;

	પૂર્ણાંક j;

	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->imagemaps == ખાલી)
		પાછા;

	માટે (i = 0; i != HASH_SIZE; i++) {
		સંરચના imagemap *map;
		સંરચના mapentry *entry;

		map = c->imagemaps[i];
		જ્યારે (map != ખાલી) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ચાવી);

			માટે (entry = map->list; entry; entry = entry->next) {
				પસંદગી (entry->type) {
				સ્થિતિ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					તોડો;
				સ્થિતિ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					તોડો;
				સ્થિતિ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					તોડો;
				સ્થિતિ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					માટે (j = 0; j != entry->bounds.poly.અંક;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(પૂર્ણાંક)entry->bounds.poly.xcoords[j],
							(પૂર્ણાંક)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					તોડો;
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
સ્થિર bool
imagemap_addtolist(અચલ સંરચના html_content *c,
		   dom_ગાંઠ *n,
		   nsurl *base_url,
		   સંરચના mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ખાલી, *target = ખાલી, *shape = ખાલી;
	dom_string *coords = ખાલી;
	સંરચના mapentry *new_map, *temp;
	bool ret = true;

	જો (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		જો ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			જાઓ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	જો (exc != DOM_NO_ERR || href == ખાલી) {
		/* No href="" attribute, skip this element */
		જાઓ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	જો (exc != DOM_NO_ERR) {
		જાઓ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	જો (exc != DOM_NO_ERR) {
		જાઓ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	જો (shape == ખાલી)
		shape = dom_string_ref(corestring_dom_rect);

	જો (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		જો (exc != DOM_NO_ERR || coords == ખાલી) {
			જાઓ ok_out;
		}
	}

	new_map = calloc(1, કદ(*new_map));
	જો (new_map == ખાલી) {
		જાઓ bad_out;
	}

	જો (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	નહીંતર જો (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	નહીંતર જો (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	નહીંતર જો (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	નહીંતર
		જાઓ bad_out;

	જો (box_extract_link(c, href, base_url, &new_map->url) == false)
		જાઓ bad_out;

	જો (new_map->url == ખાલી) {
		/* non-fatal error -> ignore this */
		જાઓ ok_free_map_out;
	}

	જો (target != ખાલી) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		જો (new_map->target == ખાલી)
			જાઓ bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	જો (new_map->type != IMAGEMAP_DEFAULT) {
		પૂર્ણાંક x, y;
		દશાંશ *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		અક્ષર *val = strtok((અક્ષર *)dom_string_data(coords), ",");
		પૂર્ણાંક અંક = 1;

		પસંદગી (new_map->type) {
		સ્થિતિ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			જ્યારે (val != ખાલી && અંક <= 4) {
				પસંદગી (અંક) {
				સ્થિતિ 1:
					new_map->bounds.rect.x0 = atoi(val);
					તોડો;
				સ્થિતિ 2:
					new_map->bounds.rect.y0 = atoi(val);
					તોડો;
				સ્થિતિ 3:
					new_map->bounds.rect.x1 = atoi(val);
					તોડો;
				સ્થિતિ 4:
					new_map->bounds.rect.y1 = atoi(val);
					તોડો;
				}

				અંક++;
				val = strtok(ખાલી, ",");
			}
			તોડો;
		સ્થિતિ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			જ્યારે (val != ખાલી && અંક <= 3) {
				પસંદગી (અંક) {
				સ્થિતિ 1:
					new_map->bounds.circle.x = atoi(val);
					તોડો;
				સ્થિતિ 2:
					new_map->bounds.circle.y = atoi(val);
					તોડો;
				સ્થિતિ 3:
					new_map->bounds.circle.r = atoi(val);
					તોડો;
				}

				અંક++;
				val = strtok(ખાલી, ",");
			}
			તોડો;
		સ્થિતિ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ખાલી;
			new_map->bounds.poly.ycoords = ખાલી;

			જ્યારે (val != ખાલી) {
				x = atoi(val);

				val = strtok(ખાલી, ",");
				જો (val == ખાલી)
					તોડો;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						અંક * કદ(દશાંશ));
				જો (xcoords == ખાલી) {
					જાઓ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					અંક * કદ(દશાંશ));
				જો (ycoords == ખાલી) {
					જાઓ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[અંક - 1] = x;
				new_map->bounds.poly.ycoords[અંક - 1] = y;

				અંક++;
				val = strtok(ખાલી, ",");
			}

			new_map->bounds.poly.અંક = અંક - 1;

			તોડો;
		મૂળભૂત:
			તોડો;
		}
	}

	new_map->next = ખાલી;

	જો (*entry) {
		/* add to END of list */
		માટે (temp = (*entry); temp->next != ખાલી; temp = temp->next)
			;
		temp->next = new_map;
	} નહીંતર {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	જાઓ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	જો (new_map != ખાલી) {
		જો (new_map->url != ખાલી)
			nsurl_unref(new_map->url);
		જો (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ખાલી)
			મુક્ત(new_map->bounds.poly.ycoords);
		જો (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ખાલી)
			મુક્ત(new_map->bounds.poly.xcoords);
		જો (new_map->target != ખાલી)
			મુક્ત(new_map->target);

		મુક્ત(new_map);
	}
ok_out:
	જો (href != ખાલી)
		dom_string_unref(href);
	જો (target != ખાલી)
		dom_string_unref(target);
	જો (shape != ખાલી)
		dom_string_unref(shape);
	જો (coords != ખાલી)
		dom_string_unref(coords);

	પાછા ret;
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
સ્થિર bool
imagemap_extract_map_entries(dom_ગાંઠ *ગાંઠ, html_content *c,
			     સંરચના mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_ગણતરી;

	exc = dom_element_get_elements_by_tag_name(ગાંઠ, tname, &nlist);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_ગણતરી);
	જો (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		પાછા false;
	}

	માટે (ent = 0; ent < tag_ગણતરી; ++ent) {
		dom_ગાંઠ *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		જો (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			પાછા false;
		}
		જો (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_ગાંઠ_unref(subnode);
			dom_nodelist_unref(nlist);
			પાછા false;
		}
		dom_ગાંઠ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	પાછા true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
સ્થિર bool imagemap_extract_map(dom_ગાંઠ *ગાંઠ, html_content *c,
		સંરચના mapentry **entry)
{
	જો (imagemap_extract_map_entries(ગાંઠ, c, entry,
			corestring_dom_area) == false)
		પાછા false;
	પાછા imagemap_extract_map_entries(ગાંઠ, c, entry,
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
	જો (exc != DOM_NO_ERR) {
		પાછા NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	જો (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		જાઓ out_nlist;
	}

	માટે (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ગાંઠ *ગાંઠ;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &ગાંઠ);
		જો (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			જાઓ out_nlist;
		}

		exc = dom_element_get_attribute(ગાંઠ, corestring_dom_id,
						&name);
		જો (exc != DOM_NO_ERR) {
			dom_ગાંઠ_unref(ગાંઠ);
			ret = NSERROR_DOM;
			જાઓ out_nlist;
		}

		જો (name == ખાલી) {
			exc = dom_element_get_attribute(ગાંઠ,
							corestring_dom_name,
							&name);
			જો (exc != DOM_NO_ERR) {
				dom_ગાંઠ_unref(ગાંઠ);
				ret = NSERROR_DOM;
				જાઓ out_nlist;
			}
		}

		જો (name != ખાલી) {
			સંરચના mapentry *entry = ખાલી;
			જો (imagemap_extract_map(ગાંઠ, c, &entry) == false) {
				જો (entry != ખાલી) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_ગાંઠ_unref(ગાંઠ);
				ret = NSERROR_NOMEM; /** @todo check this */
				જાઓ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			જો ((entry != ખાલી) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_ગાંઠ_unref(ગાંઠ);
				ret = NSERROR_NOMEM; /** @todo check this */
				જાઓ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_ગાંઠ_unref(ગાંઠ);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	પાછા ret;
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
સ્થિર પૂર્ણાંક
imagemap_point_in_poly(પૂર્ણાંક અંક, દશાંશ *xpt, દશાંશ *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	પૂર્ણાંક i, j, c = 0;

	પ્રતિજ્ઞા(xpt != ખાલી);
	પ્રતિજ્ઞા(ypt != ખાલી);

	માટે (i = 0, j = અંક - 1; i < અંક; j = i++) {
		જો ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	પાછા c;
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
nsurl *imagemap_get(સંરચના html_content *c, અચલ અક્ષર *ચાવી,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		અચલ અક્ષર **target)
{
	unsigned પૂર્ણાંક slot = 0;
	સંરચના imagemap *map;
	સંરચના mapentry *entry;
	unsigned long cx, cy;

	પ્રતિજ્ઞા(c != ખાલી);

	જો (ચાવી == ખાલી)
		પાછા ખાલી;

	જો (c->imagemaps == ખાલી)
		પાછા ખાલી;

	slot = imagemap_hash(ચાવી);

	માટે (map = c->imagemaps[slot]; map != ખાલી; map = map->next) {
		જો (map->ચાવી != ખાલી && strcasecmp(map->ચાવી, ચાવી) == 0)
			તોડો;
	}

	જો (map == ખાલી || map->list == ખાલી)
		પાછા ખાલી;

	માટે (entry = map->list; entry; entry = entry->next) {
		પસંદગી (entry->type) {
		સ્થિતિ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			જો (target)
				*target = entry->target;
			પાછા entry->url;
			તોડો;
		સ્થિતિ IMAGEMAP_RECT:
			જો (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				જો (target)
					*target = entry->target;
				પાછા entry->url;
			}
			તોડો;
		સ્થિતિ IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			જો ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				જો (target)
					*target = entry->target;
				પાછા entry->url;
			}
			તોડો;
		સ્થિતિ IMAGEMAP_POLY:
			જો (imagemap_point_in_poly(entry->bounds.poly.અંક,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				જો (target)
					*target = entry->target;
				પાછા entry->url;
			}
			તોડો;
		}
	}

	જો (target)
		*target = ખાલી;

	પાછા ખાલી;
}
