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

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>

#ضمن <dom/dom.h>

#ضمن "utils/log.h"
#ضمن "utils/corestrings.h"
#ضمن "content/content_protected.h"
#ضمن "content/hlcache.h"

#ضمن "html/box.h"
#ضمن "html/box_construct.h"
#ضمن "html/private.h"
#ضمن "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

تعريفنوع تعداد {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

بنية mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	حرف *target;			/**< target frame (if any) */
	اتحاد {
		بنية {
			صحيح x;		/**< x coordinate of centre */
			صحيح y;		/**< y coordinate of center */
			صحيح r;		/**< radius of circle */
		} circle;
		بنية {
			صحيح x0;		/**< left hand edge */
			صحيح y0;		/**< top edge */
			صحيح x1;		/**< right hand edge */
			صحيح y1;		/**< bottom edge */
		} rect;
		بنية {
			صحيح num;	/**< number of points */
			عائم *xcoords;	/**< x coordinates */
			عائم *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	بنية mapentry *next;		/**< next entry in list */
};

بنية imagemap {
	حرف *key;		/**< key for this entry */
	بنية mapentry *list;	/**< pointer to linked list of entries */
	بنية imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ساكن bool imagemap_create(html_content *c)
{
	أكد(c != عدم);

	ܐܢ (c->imagemaps == عدم) {
		c->imagemaps = calloc(HASH_SIZE, حجم(بنية imagemap *));
		ܐܢ (c->imagemaps == عدم) {
			ܦܢܐ false;
		}
	}

	ܦܢܐ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ساكن unsigned صحيح imagemap_hash(ثابت حرف *key)
{
	unsigned صحيح z = 0;

	ܐܢ (key == 0) ܦܢܐ 0;

	لكل (; *key != 0; key++) {
		z += *key & 0x1f;
	}

	ܦܢܐ (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ساكن bool
imagemap_add(html_content *c, dom_string *key, بنية mapentry *list)
{
	بنية imagemap *map;
	unsigned صحيح slot;

	أكد(c != عدم);
	أكد(key != عدم);
	أكد(list != عدم);

	ܐܢ (imagemap_create(c) == false)
		ܦܢܐ false;

	map = calloc(1, حجم(*map));
	ܐܢ (map == عدم)
		ܦܢܐ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->key = strdup(dom_string_data(key));
	ܐܢ (map->key == عدم) {
		حرر(map);
		ܦܢܐ false;
	}

	map->list = list;

	slot = imagemap_hash(map->key);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ܦܢܐ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساكن فراغ imagemap_freelist(بنية mapentry *list)
{
	بنية mapentry *entry, *prev;

	أكد(list != عدم);

	entry = list;

	ܟܡܐ (entry != عدم) {
		prev = entry;

		nsurl_unref(entry->url);

		ܐܢ (entry->target)
			حرر(entry->target);

		ܐܢ (entry->type == IMAGEMAP_POLY) {
			حرر(entry->bounds.poly.xcoords);
			حرر(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		حرر(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
فراغ imagemap_destroy(html_content *c)
{
	unsigned صحيح i;

	أكد(c != عدم);

	/* no imagemaps -> return */
	ܐܢ (c->imagemaps == عدم)
		ܦܢܐ;

	لكل (i = 0; i != HASH_SIZE; i++) {
		بنية imagemap *map, *next;

		map = c->imagemaps[i];
		ܟܡܐ (map != عدم) {
			next = map->next;
			imagemap_freelist(map->list);
			حرر(map->key);
			حرر(map);
			map = next;
		}
	}

	حرر(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
فراغ imagemap_dump(html_content *c)
{
	unsigned صحيح i;

	صحيح j;

	أكد(c != عدم);

	ܐܢ (c->imagemaps == عدم)
		ܦܢܐ;

	لكل (i = 0; i != HASH_SIZE; i++) {
		بنية imagemap *map;
		بنية mapentry *entry;

		map = c->imagemaps[i];
		ܟܡܐ (map != عدم) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->key);

			لكل (entry = map->list; entry; entry = entry->next) {
				بدل (entry->type) {
				حالة IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					اكسر;
				حالة IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					اكسر;
				حالة IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					اكسر;
				حالة IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					لكل (j = 0; j != entry->bounds.poly.num;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحيح)entry->bounds.poly.xcoords[j],
							(صحيح)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					اكسر;
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
ساكن bool
imagemap_addtolist(ثابت بنية html_content *c,
		   dom_node *n,
		   nsurl *base_url,
		   بنية mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = عدم, *target = عدم, *shape = عدم;
	dom_string *coords = عدم;
	بنية mapentry *new_map, *temp;
	bool ret = true;

	ܐܢ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ܐܢ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			اذهب ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ܐܢ (exc != DOM_NO_ERR || href == عدم) {
		/* No href="" attribute, skip this element */
		اذهب ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	ܐܢ (exc != DOM_NO_ERR) {
		اذهب ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ܐܢ (exc != DOM_NO_ERR) {
		اذهب ok_out;
	}

	/* If there's no shape, we default to rectangles */
	ܐܢ (shape == عدم)
		shape = dom_string_ref(corestring_dom_rect);

	ܐܢ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ܐܢ (exc != DOM_NO_ERR || coords == عدم) {
			اذهب ok_out;
		}
	}

	new_map = calloc(1, حجم(*new_map));
	ܐܢ (new_map == عدم) {
		اذهب bad_out;
	}

	ܐܢ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ܐܚܪܢܐ
		اذهب bad_out;

	ܐܢ (box_extract_link(c, href, base_url, &new_map->url) == false)
		اذهب bad_out;

	ܐܢ (new_map->url == عدم) {
		/* non-fatal error -> ignore this */
		اذهب ok_free_map_out;
	}

	ܐܢ (target != عدم) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		ܐܢ (new_map->target == عدم)
			اذهب bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	ܐܢ (new_map->type != IMAGEMAP_DEFAULT) {
		صحيح x, y;
		عائم *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_data(coords), ",");
		صحيح num = 1;

		بدل (new_map->type) {
		حالة IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ܟܡܐ (val != عدم && num <= 4) {
				بدل (num) {
				حالة 1:
					new_map->bounds.rect.x0 = atoi(val);
					اكسر;
				حالة 2:
					new_map->bounds.rect.y0 = atoi(val);
					اكسر;
				حالة 3:
					new_map->bounds.rect.x1 = atoi(val);
					اكسر;
				حالة 4:
					new_map->bounds.rect.y1 = atoi(val);
					اكسر;
				}

				num++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ܟܡܐ (val != عدم && num <= 3) {
				بدل (num) {
				حالة 1:
					new_map->bounds.circle.x = atoi(val);
					اكسر;
				حالة 2:
					new_map->bounds.circle.y = atoi(val);
					اكسر;
				حالة 3:
					new_map->bounds.circle.r = atoi(val);
					اكسر;
				}

				num++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = عدم;
			new_map->bounds.poly.ycoords = عدم;

			ܟܡܐ (val != عدم) {
				x = atoi(val);

				val = strtok(عدم, ",");
				ܐܢ (val == عدم)
					اكسر;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						num * حجم(عائم));
				ܐܢ (xcoords == عدم) {
					اذهب bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					num * حجم(عائم));
				ܐܢ (ycoords == عدم) {
					اذهب bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[num - 1] = x;
				new_map->bounds.poly.ycoords[num - 1] = y;

				num++;
				val = strtok(عدم, ",");
			}

			new_map->bounds.poly.num = num - 1;

			اكسر;
		افتراضي:
			اكسر;
		}
	}

	new_map->next = عدم;

	ܐܢ (*entry) {
		/* add to END of list */
		لكل (temp = (*entry); temp->next != عدم; temp = temp->next)
			;
		temp->next = new_map;
	} ܐܚܪܢܐ {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	اذهب ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	ܐܢ (new_map != عدم) {
		ܐܢ (new_map->url != عدم)
			nsurl_unref(new_map->url);
		ܐܢ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != عدم)
			حرر(new_map->bounds.poly.ycoords);
		ܐܢ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != عدم)
			حرر(new_map->bounds.poly.xcoords);
		ܐܢ (new_map->target != عدم)
			حرر(new_map->target);

		حرر(new_map);
	}
ok_out:
	ܐܢ (href != عدم)
		dom_string_unref(href);
	ܐܢ (target != عدم)
		dom_string_unref(target);
	ܐܢ (shape != عدم)
		dom_string_unref(shape);
	ܐܢ (coords != عدم)
		dom_string_unref(coords);

	ܦܢܐ ret;
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
ساكن bool
imagemap_extract_map_entries(dom_node *node, html_content *c,
			     بنية mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_count;

	exc = dom_element_get_elements_by_tag_name(node, tname, &nlist);
	ܐܢ (exc != DOM_NO_ERR) {
		ܦܢܐ false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_count);
	ܐܢ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ܦܢܐ false;
	}

	لكل (ent = 0; ent < tag_count; ++ent) {
		dom_node *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		ܐܢ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ܦܢܐ false;
		}
		ܐܢ (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_node_unref(subnode);
			dom_nodelist_unref(nlist);
			ܦܢܐ false;
		}
		dom_node_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ܦܢܐ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ساكن bool imagemap_extract_map(dom_node *node, html_content *c,
		بنية mapentry **entry)
{
	ܐܢ (imagemap_extract_map_entries(node, c, entry,
			corestring_dom_area) == false)
		ܦܢܐ false;
	ܦܢܐ imagemap_extract_map_entries(node, c, entry,
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
	ܐܢ (exc != DOM_NO_ERR) {
		ܦܢܐ NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	ܐܢ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		اذهب out_nlist;
	}

	لكل (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_node *node;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &node);
		ܐܢ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			اذهب out_nlist;
		}

		exc = dom_element_get_attribute(node, corestring_dom_id,
						&name);
		ܐܢ (exc != DOM_NO_ERR) {
			dom_node_unref(node);
			ret = NSERROR_DOM;
			اذهب out_nlist;
		}

		ܐܢ (name == عدم) {
			exc = dom_element_get_attribute(node,
							corestring_dom_name,
							&name);
			ܐܢ (exc != DOM_NO_ERR) {
				dom_node_unref(node);
				ret = NSERROR_DOM;
				اذهب out_nlist;
			}
		}

		ܐܢ (name != عدم) {
			بنية mapentry *entry = عدم;
			ܐܢ (imagemap_extract_map(node, c, &entry) == false) {
				ܐܢ (entry != عدم) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_node_unref(node);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ܐܢ ((entry != عدم) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_node_unref(node);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب out_nlist;
			}
		}

		dom_string_unref(name);
		dom_node_unref(node);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ܦܢܐ ret;
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
ساكن صحيح
imagemap_point_in_poly(صحيح num, عائم *xpt, عائم *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	صحيح i, j, c = 0;

	أكد(xpt != عدم);
	أكد(ypt != عدم);

	لكل (i = 0, j = num - 1; i < num; j = i++) {
		ܐܢ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ܦܢܐ c;
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
nsurl *imagemap_get(بنية html_content *c, ثابت حرف *key,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ثابت حرف **target)
{
	unsigned صحيح slot = 0;
	بنية imagemap *map;
	بنية mapentry *entry;
	unsigned long cx, cy;

	أكد(c != عدم);

	ܐܢ (key == عدم)
		ܦܢܐ عدم;

	ܐܢ (c->imagemaps == عدم)
		ܦܢܐ عدم;

	slot = imagemap_hash(key);

	لكل (map = c->imagemaps[slot]; map != عدم; map = map->next) {
		ܐܢ (map->key != عدم && strcasecmp(map->key, key) == 0)
			اكسر;
	}

	ܐܢ (map == عدم || map->list == عدم)
		ܦܢܐ عدم;

	لكل (entry = map->list; entry; entry = entry->next) {
		بدل (entry->type) {
		حالة IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ܐܢ (target)
				*target = entry->target;
			ܦܢܐ entry->url;
			اكسر;
		حالة IMAGEMAP_RECT:
			ܐܢ (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				ܐܢ (target)
					*target = entry->target;
				ܦܢܐ entry->url;
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			ܐܢ ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				ܐܢ (target)
					*target = entry->target;
				ܦܢܐ entry->url;
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			ܐܢ (imagemap_point_in_poly(entry->bounds.poly.num,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ܐܢ (target)
					*target = entry->target;
				ܦܢܐ entry->url;
			}
			اكسر;
		}
	}

	ܐܢ (target)
		*target = عدم;

	ܦܢܐ عدم;
}
