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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>

#অন্তর্ভাৱ <dom/dom.h>

#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/hlcache.h"

#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/box_construct.h"
#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

প্রকারনির্দেশ গণনা {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

সংরচনা mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	ৱর্ণ *target;			/**< target frame (if any) */
	সঙ্ঘ {
		সংরচনা {
			পূর্ণাঙ্ক x;		/**< x coordinate of centre */
			পূর্ণাঙ্ক y;		/**< y coordinate of center */
			পূর্ণাঙ্ক r;		/**< radius of circle */
		} circle;
		সংরচনা {
			পূর্ণাঙ্ক x0;		/**< left hand edge */
			পূর্ণাঙ্ক y0;		/**< top edge */
			পূর্ণাঙ্ক x1;		/**< right hand edge */
			পূর্ণাঙ্ক y1;		/**< bottom edge */
		} rect;
		সংরচনা {
			পূর্ণাঙ্ক অঙ্ক;	/**< number of points */
			ভিন্ন *xcoords;	/**< x coordinates */
			ভিন্ন *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	সংরচনা mapentry *next;		/**< next entry in list */
};

সংরচনা imagemap {
	ৱর্ণ *চাবি;		/**< key for this entry */
	সংরচনা mapentry *list;	/**< pointer to linked list of entries */
	সংরচনা imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
স্থির bool imagemap_create(html_content *c)
{
	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->imagemaps == রিক্ত) {
		c->imagemaps = calloc(HASH_SIZE, পরিমাণ(সংরচনা imagemap *));
		যদি (c->imagemaps == রিক্ত) {
			ঘূৰাই false;
		}
	}

	ঘূৰাই true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
স্থির unsigned পূর্ণাঙ্ক imagemap_hash(ধ্রুৱ ৱর্ণ *চাবি)
{
	unsigned পূর্ণাঙ্ক z = 0;

	যদি (চাবি == 0) ঘূৰাই 0;

	বাবে (; *চাবি != 0; চাবি++) {
		z += *চাবি & 0x1f;
	}

	ঘূৰাই (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
স্থির bool
imagemap_add(html_content *c, dom_string *চাবি, সংরচনা mapentry *list)
{
	সংরচনা imagemap *map;
	unsigned পূর্ণাঙ্ক slot;

	প্রতিজ্ঞা(c != রিক্ত);
	প্রতিজ্ঞা(চাবি != রিক্ত);
	প্রতিজ্ঞা(list != রিক্ত);

	যদি (imagemap_create(c) == false)
		ঘূৰাই false;

	map = calloc(1, পরিমাণ(*map));
	যদি (map == রিক্ত)
		ঘূৰাই false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->চাবি = strdup(dom_string_data(চাবি));
	যদি (map->চাবি == রিক্ত) {
		মুক্ত(map);
		ঘূৰাই false;
	}

	map->list = list;

	slot = imagemap_hash(map->চাবি);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ঘূৰাই true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
স্থির শূন্য imagemap_freelist(সংরচনা mapentry *list)
{
	সংরচনা mapentry *entry, *prev;

	প্রতিজ্ঞা(list != রিক্ত);

	entry = list;

	যেতিয়ালৈ (entry != রিক্ত) {
		prev = entry;

		nsurl_unref(entry->url);

		যদি (entry->target)
			মুক্ত(entry->target);

		যদি (entry->type == IMAGEMAP_POLY) {
			মুক্ত(entry->bounds.poly.xcoords);
			মুক্ত(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		মুক্ত(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
শূন্য imagemap_destroy(html_content *c)
{
	unsigned পূর্ণাঙ্ক i;

	প্রতিজ্ঞা(c != রিক্ত);

	/* no imagemaps -> return */
	যদি (c->imagemaps == রিক্ত)
		ঘূৰাই;

	বাবে (i = 0; i != HASH_SIZE; i++) {
		সংরচনা imagemap *map, *next;

		map = c->imagemaps[i];
		যেতিয়ালৈ (map != রিক্ত) {
			next = map->next;
			imagemap_freelist(map->list);
			মুক্ত(map->চাবি);
			মুক্ত(map);
			map = next;
		}
	}

	মুক্ত(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
শূন্য imagemap_dump(html_content *c)
{
	unsigned পূর্ণাঙ্ক i;

	পূর্ণাঙ্ক j;

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->imagemaps == রিক্ত)
		ঘূৰাই;

	বাবে (i = 0; i != HASH_SIZE; i++) {
		সংরচনা imagemap *map;
		সংরচনা mapentry *entry;

		map = c->imagemaps[i];
		যেতিয়ালৈ (map != রিক্ত) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->চাবি);

			বাবে (entry = map->list; entry; entry = entry->next) {
				ৱিকল্প (entry->type) {
				পক্ষ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					ভঙ্গ;
				পক্ষ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					ভঙ্গ;
				পক্ষ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					ভঙ্গ;
				পক্ষ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					বাবে (j = 0; j != entry->bounds.poly.অঙ্ক;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(পূর্ণাঙ্ক)entry->bounds.poly.xcoords[j],
							(পূর্ণাঙ্ক)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ভঙ্গ;
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
স্থির bool
imagemap_addtolist(ধ্রুৱ সংরচনা html_content *c,
		   dom_গ্ৰন্থি *n,
		   nsurl *base_url,
		   সংরচনা mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = রিক্ত, *target = রিক্ত, *shape = রিক্ত;
	dom_string *coords = রিক্ত;
	সংরচনা mapentry *new_map, *temp;
	bool ret = true;

	যদি (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		যদি ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			গচ্ছ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	যদি (exc != DOM_NO_ERR || href == রিক্ত) {
		/* No href="" attribute, skip this element */
		গচ্ছ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	যদি (exc != DOM_NO_ERR) {
		গচ্ছ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	যদি (exc != DOM_NO_ERR) {
		গচ্ছ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	যদি (shape == রিক্ত)
		shape = dom_string_ref(corestring_dom_rect);

	যদি (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		যদি (exc != DOM_NO_ERR || coords == রিক্ত) {
			গচ্ছ ok_out;
		}
	}

	new_map = calloc(1, পরিমাণ(*new_map));
	যদি (new_map == রিক্ত) {
		গচ্ছ bad_out;
	}

	যদি (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	অন্যথা যদি (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	অন্যথা যদি (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	অন্যথা যদি (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	অন্যথা
		গচ্ছ bad_out;

	যদি (box_extract_link(c, href, base_url, &new_map->url) == false)
		গচ্ছ bad_out;

	যদি (new_map->url == রিক্ত) {
		/* non-fatal error -> ignore this */
		গচ্ছ ok_free_map_out;
	}

	যদি (target != রিক্ত) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		যদি (new_map->target == রিক্ত)
			গচ্ছ bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	যদি (new_map->type != IMAGEMAP_DEFAULT) {
		পূর্ণাঙ্ক x, y;
		ভিন্ন *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		ৱর্ণ *val = strtok((ৱর্ণ *)dom_string_data(coords), ",");
		পূর্ণাঙ্ক অঙ্ক = 1;

		ৱিকল্প (new_map->type) {
		পক্ষ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			যেতিয়ালৈ (val != রিক্ত && অঙ্ক <= 4) {
				ৱিকল্প (অঙ্ক) {
				পক্ষ 1:
					new_map->bounds.rect.x0 = atoi(val);
					ভঙ্গ;
				পক্ষ 2:
					new_map->bounds.rect.y0 = atoi(val);
					ভঙ্গ;
				পক্ষ 3:
					new_map->bounds.rect.x1 = atoi(val);
					ভঙ্গ;
				পক্ষ 4:
					new_map->bounds.rect.y1 = atoi(val);
					ভঙ্গ;
				}

				অঙ্ক++;
				val = strtok(রিক্ত, ",");
			}
			ভঙ্গ;
		পক্ষ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			যেতিয়ালৈ (val != রিক্ত && অঙ্ক <= 3) {
				ৱিকল্প (অঙ্ক) {
				পক্ষ 1:
					new_map->bounds.circle.x = atoi(val);
					ভঙ্গ;
				পক্ষ 2:
					new_map->bounds.circle.y = atoi(val);
					ভঙ্গ;
				পক্ষ 3:
					new_map->bounds.circle.r = atoi(val);
					ভঙ্গ;
				}

				অঙ্ক++;
				val = strtok(রিক্ত, ",");
			}
			ভঙ্গ;
		পক্ষ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = রিক্ত;
			new_map->bounds.poly.ycoords = রিক্ত;

			যেতিয়ালৈ (val != রিক্ত) {
				x = atoi(val);

				val = strtok(রিক্ত, ",");
				যদি (val == রিক্ত)
					ভঙ্গ;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						অঙ্ক * পরিমাণ(ভিন্ন));
				যদি (xcoords == রিক্ত) {
					গচ্ছ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					অঙ্ক * পরিমাণ(ভিন্ন));
				যদি (ycoords == রিক্ত) {
					গচ্ছ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[অঙ্ক - 1] = x;
				new_map->bounds.poly.ycoords[অঙ্ক - 1] = y;

				অঙ্ক++;
				val = strtok(রিক্ত, ",");
			}

			new_map->bounds.poly.অঙ্ক = অঙ্ক - 1;

			ভঙ্গ;
		স্ৱভাৱ:
			ভঙ্গ;
		}
	}

	new_map->next = রিক্ত;

	যদি (*entry) {
		/* add to END of list */
		বাবে (temp = (*entry); temp->next != রিক্ত; temp = temp->next)
			;
		temp->next = new_map;
	} অন্যথা {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	গচ্ছ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	যদি (new_map != রিক্ত) {
		যদি (new_map->url != রিক্ত)
			nsurl_unref(new_map->url);
		যদি (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != রিক্ত)
			মুক্ত(new_map->bounds.poly.ycoords);
		যদি (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != রিক্ত)
			মুক্ত(new_map->bounds.poly.xcoords);
		যদি (new_map->target != রিক্ত)
			মুক্ত(new_map->target);

		মুক্ত(new_map);
	}
ok_out:
	যদি (href != রিক্ত)
		dom_string_unref(href);
	যদি (target != রিক্ত)
		dom_string_unref(target);
	যদি (shape != রিক্ত)
		dom_string_unref(shape);
	যদি (coords != রিক্ত)
		dom_string_unref(coords);

	ঘূৰাই ret;
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
স্থির bool
imagemap_extract_map_entries(dom_গ্ৰন্থি *গ্ৰন্থি, html_content *c,
			     সংরচনা mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_গুনতি;

	exc = dom_element_get_elements_by_tag_name(গ্ৰন্থি, tname, &nlist);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_গুনতি);
	যদি (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ঘূৰাই false;
	}

	বাবে (ent = 0; ent < tag_গুনতি; ++ent) {
		dom_গ্ৰন্থি *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		যদি (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ঘূৰাই false;
		}
		যদি (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_গ্ৰন্থি_unref(subnode);
			dom_nodelist_unref(nlist);
			ঘূৰাই false;
		}
		dom_গ্ৰন্থি_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ঘূৰাই true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
স্থির bool imagemap_extract_map(dom_গ্ৰন্থি *গ্ৰন্থি, html_content *c,
		সংরচনা mapentry **entry)
{
	যদি (imagemap_extract_map_entries(গ্ৰন্থি, c, entry,
			corestring_dom_area) == false)
		ঘূৰাই false;
	ঘূৰাই imagemap_extract_map_entries(গ্ৰন্থি, c, entry,
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
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	যদি (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		গচ্ছ out_nlist;
	}

	বাবে (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_গ্ৰন্থি *গ্ৰন্থি;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &গ্ৰন্থি);
		যদি (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			গচ্ছ out_nlist;
		}

		exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_id,
						&name);
		যদি (exc != DOM_NO_ERR) {
			dom_গ্ৰন্থি_unref(গ্ৰন্থি);
			ret = NSERROR_DOM;
			গচ্ছ out_nlist;
		}

		যদি (name == রিক্ত) {
			exc = dom_element_get_attribute(গ্ৰন্থি,
							corestring_dom_name,
							&name);
			যদি (exc != DOM_NO_ERR) {
				dom_গ্ৰন্থি_unref(গ্ৰন্থি);
				ret = NSERROR_DOM;
				গচ্ছ out_nlist;
			}
		}

		যদি (name != রিক্ত) {
			সংরচনা mapentry *entry = রিক্ত;
			যদি (imagemap_extract_map(গ্ৰন্থি, c, &entry) == false) {
				যদি (entry != রিক্ত) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_গ্ৰন্থি_unref(গ্ৰন্থি);
				ret = NSERROR_NOMEM; /** @todo check this */
				গচ্ছ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			যদি ((entry != রিক্ত) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_গ্ৰন্থি_unref(গ্ৰন্থি);
				ret = NSERROR_NOMEM; /** @todo check this */
				গচ্ছ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_গ্ৰন্থি_unref(গ্ৰন্থি);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ঘূৰাই ret;
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
স্থির পূর্ণাঙ্ক
imagemap_point_in_poly(পূর্ণাঙ্ক অঙ্ক, ভিন্ন *xpt, ভিন্ন *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	পূর্ণাঙ্ক i, j, c = 0;

	প্রতিজ্ঞা(xpt != রিক্ত);
	প্রতিজ্ঞা(ypt != রিক্ত);

	বাবে (i = 0, j = অঙ্ক - 1; i < অঙ্ক; j = i++) {
		যদি ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ঘূৰাই c;
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
nsurl *imagemap_get(সংরচনা html_content *c, ধ্রুৱ ৱর্ণ *চাবি,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ধ্রুৱ ৱর্ণ **target)
{
	unsigned পূর্ণাঙ্ক slot = 0;
	সংরচনা imagemap *map;
	সংরচনা mapentry *entry;
	unsigned long cx, cy;

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (চাবি == রিক্ত)
		ঘূৰাই রিক্ত;

	যদি (c->imagemaps == রিক্ত)
		ঘূৰাই রিক্ত;

	slot = imagemap_hash(চাবি);

	বাবে (map = c->imagemaps[slot]; map != রিক্ত; map = map->next) {
		যদি (map->চাবি != রিক্ত && strcasecmp(map->চাবি, চাবি) == 0)
			ভঙ্গ;
	}

	যদি (map == রিক্ত || map->list == রিক্ত)
		ঘূৰাই রিক্ত;

	বাবে (entry = map->list; entry; entry = entry->next) {
		ৱিকল্প (entry->type) {
		পক্ষ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			যদি (target)
				*target = entry->target;
			ঘূৰাই entry->url;
			ভঙ্গ;
		পক্ষ IMAGEMAP_RECT:
			যদি (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				যদি (target)
					*target = entry->target;
				ঘূৰাই entry->url;
			}
			ভঙ্গ;
		পক্ষ IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			যদি ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				যদি (target)
					*target = entry->target;
				ঘূৰাই entry->url;
			}
			ভঙ্গ;
		পক্ষ IMAGEMAP_POLY:
			যদি (imagemap_point_in_poly(entry->bounds.poly.অঙ্ক,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				যদি (target)
					*target = entry->target;
				ঘূৰাই entry->url;
			}
			ভঙ্গ;
		}
	}

	যদি (target)
		*target = রিক্ত;

	ঘূৰাই রিক্ত;
}
