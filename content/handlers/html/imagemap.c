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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>

#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/hlcache.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_construct.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

வகைவரை எண்ணிக்கை {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

கட்டமைப்பு mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	எழுத்து *target;			/**< target frame (if any) */
	ஒன்றிப்பு {
		கட்டமைப்பு {
			முழு x;		/**< x coordinate of centre */
			முழு y;		/**< y coordinate of center */
			முழு r;		/**< radius of circle */
		} circle;
		கட்டமைப்பு {
			முழு x0;		/**< left hand edge */
			முழு y0;		/**< top edge */
			முழு x1;		/**< right hand edge */
			முழு y1;		/**< bottom edge */
		} rect;
		கட்டமைப்பு {
			முழு இலக்கம்;	/**< number of points */
			தசம *xcoords;	/**< x coordinates */
			தசம *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	கட்டமைப்பு mapentry *next;		/**< next entry in list */
};

கட்டமைப்பு imagemap {
	எழுத்து *சாவி;		/**< key for this entry */
	கட்டமைப்பு mapentry *list;	/**< pointer to linked list of entries */
	கட்டமைப்பு imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
நிலையான bool imagemap_create(html_content *c)
{
	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->imagemaps == வெற்று) {
		c->imagemaps = calloc(HASH_SIZE, அளவு(கட்டமைப்பு imagemap *));
		எனில் (c->imagemaps == வெற்று) {
			திரும்பு false;
		}
	}

	திரும்பு true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
நிலையான unsigned முழு imagemap_hash(மாறிலி எழுத்து *சாவி)
{
	unsigned முழு z = 0;

	எனில் (சாவி == 0) திரும்பு 0;

	க்கு (; *சாவி != 0; சாவி++) {
		z += *சாவி & 0x1f;
	}

	திரும்பு (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
நிலையான bool
imagemap_add(html_content *c, dom_string *சாவி, கட்டமைப்பு mapentry *list)
{
	கட்டமைப்பு imagemap *map;
	unsigned முழு slot;

	ப்ரதிஜ்ஞா(c != வெற்று);
	ப்ரதிஜ்ஞா(சாவி != வெற்று);
	ப்ரதிஜ்ஞா(list != வெற்று);

	எனில் (imagemap_create(c) == false)
		திரும்பு false;

	map = calloc(1, அளவு(*map));
	எனில் (map == வெற்று)
		திரும்பு false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->சாவி = strdup(dom_string_data(சாவி));
	எனில் (map->சாவி == வெற்று) {
		விடுவி(map);
		திரும்பு false;
	}

	map->list = list;

	slot = imagemap_hash(map->சாவி);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	திரும்பு true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
நிலையான வெறுமை imagemap_freelist(கட்டமைப்பு mapentry *list)
{
	கட்டமைப்பு mapentry *entry, *prev;

	ப்ரதிஜ்ஞா(list != வெற்று);

	entry = list;

	போது (entry != வெற்று) {
		prev = entry;

		nsurl_unref(entry->url);

		எனில் (entry->target)
			விடுவி(entry->target);

		எனில் (entry->type == IMAGEMAP_POLY) {
			விடுவி(entry->bounds.poly.xcoords);
			விடுவி(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		விடுவி(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
வெறுமை imagemap_destroy(html_content *c)
{
	unsigned முழு i;

	ப்ரதிஜ்ஞா(c != வெற்று);

	/* no imagemaps -> return */
	எனில் (c->imagemaps == வெற்று)
		திரும்பு;

	க்கு (i = 0; i != HASH_SIZE; i++) {
		கட்டமைப்பு imagemap *map, *next;

		map = c->imagemaps[i];
		போது (map != வெற்று) {
			next = map->next;
			imagemap_freelist(map->list);
			விடுவி(map->சாவி);
			விடுவி(map);
			map = next;
		}
	}

	விடுவி(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
வெறுமை imagemap_dump(html_content *c)
{
	unsigned முழு i;

	முழு j;

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->imagemaps == வெற்று)
		திரும்பு;

	க்கு (i = 0; i != HASH_SIZE; i++) {
		கட்டமைப்பு imagemap *map;
		கட்டமைப்பு mapentry *entry;

		map = c->imagemaps[i];
		போது (map != வெற்று) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->சாவி);

			க்கு (entry = map->list; entry; entry = entry->next) {
				தேர்வு (entry->type) {
				நிலை IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					நிறுத்து;
				நிலை IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					நிறுத்து;
				நிலை IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					நிறுத்து;
				நிலை IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					க்கு (j = 0; j != entry->bounds.poly.இலக்கம்;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(முழு)entry->bounds.poly.xcoords[j],
							(முழு)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					நிறுத்து;
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
நிலையான bool
imagemap_addtolist(மாறிலி கட்டமைப்பு html_content *c,
		   dom_முனை *n,
		   nsurl *base_url,
		   கட்டமைப்பு mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = வெற்று, *target = வெற்று, *shape = வெற்று;
	dom_string *coords = வெற்று;
	கட்டமைப்பு mapentry *new_map, *temp;
	bool ret = true;

	எனில் (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		எனில் ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			செல் ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	எனில் (exc != DOM_NO_ERR || href == வெற்று) {
		/* No href="" attribute, skip this element */
		செல் ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	எனில் (exc != DOM_NO_ERR) {
		செல் ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	எனில் (exc != DOM_NO_ERR) {
		செல் ok_out;
	}

	/* If there's no shape, we default to rectangles */
	எனில் (shape == வெற்று)
		shape = dom_string_ref(corestring_dom_rect);

	எனில் (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		எனில் (exc != DOM_NO_ERR || coords == வெற்று) {
			செல் ok_out;
		}
	}

	new_map = calloc(1, அளவு(*new_map));
	எனில் (new_map == வெற்று) {
		செல் bad_out;
	}

	எனில் (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	இல்லையெனில்
		செல் bad_out;

	எனில் (box_extract_link(c, href, base_url, &new_map->url) == false)
		செல் bad_out;

	எனில் (new_map->url == வெற்று) {
		/* non-fatal error -> ignore this */
		செல் ok_free_map_out;
	}

	எனில் (target != வெற்று) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		எனில் (new_map->target == வெற்று)
			செல் bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	எனில் (new_map->type != IMAGEMAP_DEFAULT) {
		முழு x, y;
		தசம *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		எழுத்து *val = strtok((எழுத்து *)dom_string_data(coords), ",");
		முழு இலக்கம் = 1;

		தேர்வு (new_map->type) {
		நிலை IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			போது (val != வெற்று && இலக்கம் <= 4) {
				தேர்வு (இலக்கம்) {
				நிலை 1:
					new_map->bounds.rect.x0 = atoi(val);
					நிறுத்து;
				நிலை 2:
					new_map->bounds.rect.y0 = atoi(val);
					நிறுத்து;
				நிலை 3:
					new_map->bounds.rect.x1 = atoi(val);
					நிறுத்து;
				நிலை 4:
					new_map->bounds.rect.y1 = atoi(val);
					நிறுத்து;
				}

				இலக்கம்++;
				val = strtok(வெற்று, ",");
			}
			நிறுத்து;
		நிலை IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			போது (val != வெற்று && இலக்கம் <= 3) {
				தேர்வு (இலக்கம்) {
				நிலை 1:
					new_map->bounds.circle.x = atoi(val);
					நிறுத்து;
				நிலை 2:
					new_map->bounds.circle.y = atoi(val);
					நிறுத்து;
				நிலை 3:
					new_map->bounds.circle.r = atoi(val);
					நிறுத்து;
				}

				இலக்கம்++;
				val = strtok(வெற்று, ",");
			}
			நிறுத்து;
		நிலை IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = வெற்று;
			new_map->bounds.poly.ycoords = வெற்று;

			போது (val != வெற்று) {
				x = atoi(val);

				val = strtok(வெற்று, ",");
				எனில் (val == வெற்று)
					நிறுத்து;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						இலக்கம் * அளவு(தசம));
				எனில் (xcoords == வெற்று) {
					செல் bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					இலக்கம் * அளவு(தசம));
				எனில் (ycoords == வெற்று) {
					செல் bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[இலக்கம் - 1] = x;
				new_map->bounds.poly.ycoords[இலக்கம் - 1] = y;

				இலக்கம்++;
				val = strtok(வெற்று, ",");
			}

			new_map->bounds.poly.இலக்கம் = இலக்கம் - 1;

			நிறுத்து;
		இயல்பு:
			நிறுத்து;
		}
	}

	new_map->next = வெற்று;

	எனில் (*entry) {
		/* add to END of list */
		க்கு (temp = (*entry); temp->next != வெற்று; temp = temp->next)
			;
		temp->next = new_map;
	} இல்லையெனில் {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	செல் ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	எனில் (new_map != வெற்று) {
		எனில் (new_map->url != வெற்று)
			nsurl_unref(new_map->url);
		எனில் (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != வெற்று)
			விடுவி(new_map->bounds.poly.ycoords);
		எனில் (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != வெற்று)
			விடுவி(new_map->bounds.poly.xcoords);
		எனில் (new_map->target != வெற்று)
			விடுவி(new_map->target);

		விடுவி(new_map);
	}
ok_out:
	எனில் (href != வெற்று)
		dom_string_unref(href);
	எனில் (target != வெற்று)
		dom_string_unref(target);
	எனில் (shape != வெற்று)
		dom_string_unref(shape);
	எனில் (coords != வெற்று)
		dom_string_unref(coords);

	திரும்பு ret;
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
நிலையான bool
imagemap_extract_map_entries(dom_முனை *முனை, html_content *c,
			     கட்டமைப்பு mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_மொத்தம்;

	exc = dom_element_get_elements_by_tag_name(முனை, tname, &nlist);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_மொத்தம்);
	எனில் (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		திரும்பு false;
	}

	க்கு (ent = 0; ent < tag_மொத்தம்; ++ent) {
		dom_முனை *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		எனில் (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			திரும்பு false;
		}
		எனில் (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_முனை_unref(subnode);
			dom_nodelist_unref(nlist);
			திரும்பு false;
		}
		dom_முனை_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	திரும்பு true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
நிலையான bool imagemap_extract_map(dom_முனை *முனை, html_content *c,
		கட்டமைப்பு mapentry **entry)
{
	எனில் (imagemap_extract_map_entries(முனை, c, entry,
			corestring_dom_area) == false)
		திரும்பு false;
	திரும்பு imagemap_extract_map_entries(முனை, c, entry,
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
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	எனில் (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		செல் out_nlist;
	}

	க்கு (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_முனை *முனை;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &முனை);
		எனில் (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			செல் out_nlist;
		}

		exc = dom_element_get_attribute(முனை, corestring_dom_id,
						&name);
		எனில் (exc != DOM_NO_ERR) {
			dom_முனை_unref(முனை);
			ret = NSERROR_DOM;
			செல் out_nlist;
		}

		எனில் (name == வெற்று) {
			exc = dom_element_get_attribute(முனை,
							corestring_dom_name,
							&name);
			எனில் (exc != DOM_NO_ERR) {
				dom_முனை_unref(முனை);
				ret = NSERROR_DOM;
				செல் out_nlist;
			}
		}

		எனில் (name != வெற்று) {
			கட்டமைப்பு mapentry *entry = வெற்று;
			எனில் (imagemap_extract_map(முனை, c, &entry) == false) {
				எனில் (entry != வெற்று) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_முனை_unref(முனை);
				ret = NSERROR_NOMEM; /** @todo check this */
				செல் out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			எனில் ((entry != வெற்று) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_முனை_unref(முனை);
				ret = NSERROR_NOMEM; /** @todo check this */
				செல் out_nlist;
			}
		}

		dom_string_unref(name);
		dom_முனை_unref(முனை);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	திரும்பு ret;
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
நிலையான முழு
imagemap_point_in_poly(முழு இலக்கம், தசம *xpt, தசம *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	முழு i, j, c = 0;

	ப்ரதிஜ்ஞா(xpt != வெற்று);
	ப்ரதிஜ்ஞா(ypt != வெற்று);

	க்கு (i = 0, j = இலக்கம் - 1; i < இலக்கம்; j = i++) {
		எனில் ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	திரும்பு c;
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
nsurl *imagemap_get(கட்டமைப்பு html_content *c, மாறிலி எழுத்து *சாவி,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		மாறிலி எழுத்து **target)
{
	unsigned முழு slot = 0;
	கட்டமைப்பு imagemap *map;
	கட்டமைப்பு mapentry *entry;
	unsigned long cx, cy;

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (சாவி == வெற்று)
		திரும்பு வெற்று;

	எனில் (c->imagemaps == வெற்று)
		திரும்பு வெற்று;

	slot = imagemap_hash(சாவி);

	க்கு (map = c->imagemaps[slot]; map != வெற்று; map = map->next) {
		எனில் (map->சாவி != வெற்று && strcasecmp(map->சாவி, சாவி) == 0)
			நிறுத்து;
	}

	எனில் (map == வெற்று || map->list == வெற்று)
		திரும்பு வெற்று;

	க்கு (entry = map->list; entry; entry = entry->next) {
		தேர்வு (entry->type) {
		நிலை IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			எனில் (target)
				*target = entry->target;
			திரும்பு entry->url;
			நிறுத்து;
		நிலை IMAGEMAP_RECT:
			எனில் (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				எனில் (target)
					*target = entry->target;
				திரும்பு entry->url;
			}
			நிறுத்து;
		நிலை IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			எனில் ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				எனில் (target)
					*target = entry->target;
				திரும்பு entry->url;
			}
			நிறுத்து;
		நிலை IMAGEMAP_POLY:
			எனில் (imagemap_point_in_poly(entry->bounds.poly.இலக்கம்,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				எனில் (target)
					*target = entry->target;
				திரும்பு entry->url;
			}
			நிறுத்து;
		}
	}

	எனில் (target)
		*target = வெற்று;

	திரும்பு வெற்று;
}
