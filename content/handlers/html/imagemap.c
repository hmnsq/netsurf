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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>

#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/corestrings.h"
#شامل "content/content_protected.h"
#شامل "content/hlcache.h"

#شامل "html/box.h"
#شامل "html/box_construct.h"
#شامل "html/private.h"
#شامل "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

تعریفنوع شمارش {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

ساختار mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	نویسه *target;			/**< target frame (if any) */
	اجتماع {
		ساختار {
			صحیح x;		/**< x coordinate of centre */
			صحیح y;		/**< y coordinate of center */
			صحیح r;		/**< radius of circle */
		} circle;
		ساختار {
			صحیح x0;		/**< left hand edge */
			صحیح y0;		/**< top edge */
			صحیح x1;		/**< right hand edge */
			صحیح y1;		/**< bottom edge */
		} rect;
		ساختار {
			صحیح رقم;	/**< number of points */
			اعشاری *xcoords;	/**< x coordinates */
			اعشاری *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ساختار mapentry *next;		/**< next entry in list */
};

ساختار imagemap {
	نویسه *کلید;		/**< key for this entry */
	ساختار mapentry *list;	/**< pointer to linked list of entries */
	ساختار imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ایستا bool imagemap_create(html_content *c)
{
	أكد(c != پوچ);

	اگر (c->imagemaps == پوچ) {
		c->imagemaps = calloc(HASH_SIZE, اندازه(ساختار imagemap *));
		اگر (c->imagemaps == پوچ) {
			بازگشت false;
		}
	}

	بازگشت true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ایستا unsigned صحیح imagemap_hash(ثابت نویسه *کلید)
{
	unsigned صحیح z = 0;

	اگر (کلید == 0) بازگشت 0;

	برای (; *کلید != 0; کلید++) {
		z += *کلید & 0x1f;
	}

	بازگشت (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ایستا bool
imagemap_add(html_content *c, dom_string *کلید, ساختار mapentry *list)
{
	ساختار imagemap *map;
	unsigned صحیح slot;

	أكد(c != پوچ);
	أكد(کلید != پوچ);
	أكد(list != پوچ);

	اگر (imagemap_create(c) == false)
		بازگشت false;

	map = calloc(1, اندازه(*map));
	اگر (map == پوچ)
		بازگشت false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->کلید = strdup(dom_string_data(کلید));
	اگر (map->کلید == پوچ) {
		آزاد(map);
		بازگشت false;
	}

	map->list = list;

	slot = imagemap_hash(map->کلید);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	بازگشت true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ایستا تهی imagemap_freelist(ساختار mapentry *list)
{
	ساختار mapentry *entry, *prev;

	أكد(list != پوچ);

	entry = list;

	وقتی (entry != پوچ) {
		prev = entry;

		nsurl_unref(entry->url);

		اگر (entry->target)
			آزاد(entry->target);

		اگر (entry->type == IMAGEMAP_POLY) {
			آزاد(entry->bounds.poly.xcoords);
			آزاد(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		آزاد(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
تهی imagemap_destroy(html_content *c)
{
	unsigned صحیح i;

	أكد(c != پوچ);

	/* no imagemaps -> return */
	اگر (c->imagemaps == پوچ)
		بازگشت;

	برای (i = 0; i != HASH_SIZE; i++) {
		ساختار imagemap *map, *next;

		map = c->imagemaps[i];
		وقتی (map != پوچ) {
			next = map->next;
			imagemap_freelist(map->list);
			آزاد(map->کلید);
			آزاد(map);
			map = next;
		}
	}

	آزاد(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
تهی imagemap_dump(html_content *c)
{
	unsigned صحیح i;

	صحیح j;

	أكد(c != پوچ);

	اگر (c->imagemaps == پوچ)
		بازگشت;

	برای (i = 0; i != HASH_SIZE; i++) {
		ساختار imagemap *map;
		ساختار mapentry *entry;

		map = c->imagemaps[i];
		وقتی (map != پوچ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->کلید);

			برای (entry = map->list; entry; entry = entry->next) {
				انتخاب (entry->type) {
				حالت IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					بشکن;
				حالت IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					بشکن;
				حالت IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					بشکن;
				حالت IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					برای (j = 0; j != entry->bounds.poly.رقم;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحیح)entry->bounds.poly.xcoords[j],
							(صحیح)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					بشکن;
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
ایستا bool
imagemap_addtolist(ثابت ساختار html_content *c,
		   dom_گره *n,
		   nsurl *base_url,
		   ساختار mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = پوچ, *target = پوچ, *shape = پوچ;
	dom_string *coords = پوچ;
	ساختار mapentry *new_map, *temp;
	bool ret = true;

	اگر (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		اگر ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			برو ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	اگر (exc != DOM_NO_ERR || href == پوچ) {
		/* No href="" attribute, skip this element */
		برو ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	اگر (exc != DOM_NO_ERR) {
		برو ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	اگر (exc != DOM_NO_ERR) {
		برو ok_out;
	}

	/* If there's no shape, we default to rectangles */
	اگر (shape == پوچ)
		shape = dom_string_ref(corestring_dom_rect);

	اگر (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		اگر (exc != DOM_NO_ERR || coords == پوچ) {
			برو ok_out;
		}
	}

	new_map = calloc(1, اندازه(*new_map));
	اگر (new_map == پوچ) {
		برو bad_out;
	}

	اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	وگرنه اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	وگرنه اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	وگرنه اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	وگرنه
		برو bad_out;

	اگر (box_extract_link(c, href, base_url, &new_map->url) == false)
		برو bad_out;

	اگر (new_map->url == پوچ) {
		/* non-fatal error -> ignore this */
		برو ok_free_map_out;
	}

	اگر (target != پوچ) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		اگر (new_map->target == پوچ)
			برو bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	اگر (new_map->type != IMAGEMAP_DEFAULT) {
		صحیح x, y;
		اعشاری *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		نویسه *val = strtok((نویسه *)dom_string_data(coords), ",");
		صحیح رقم = 1;

		انتخاب (new_map->type) {
		حالت IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			وقتی (val != پوچ && رقم <= 4) {
				انتخاب (رقم) {
				حالت 1:
					new_map->bounds.rect.x0 = atoi(val);
					بشکن;
				حالت 2:
					new_map->bounds.rect.y0 = atoi(val);
					بشکن;
				حالت 3:
					new_map->bounds.rect.x1 = atoi(val);
					بشکن;
				حالت 4:
					new_map->bounds.rect.y1 = atoi(val);
					بشکن;
				}

				رقم++;
				val = strtok(پوچ, ",");
			}
			بشکن;
		حالت IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			وقتی (val != پوچ && رقم <= 3) {
				انتخاب (رقم) {
				حالت 1:
					new_map->bounds.circle.x = atoi(val);
					بشکن;
				حالت 2:
					new_map->bounds.circle.y = atoi(val);
					بشکن;
				حالت 3:
					new_map->bounds.circle.r = atoi(val);
					بشکن;
				}

				رقم++;
				val = strtok(پوچ, ",");
			}
			بشکن;
		حالت IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = پوچ;
			new_map->bounds.poly.ycoords = پوچ;

			وقتی (val != پوچ) {
				x = atoi(val);

				val = strtok(پوچ, ",");
				اگر (val == پوچ)
					بشکن;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						رقم * اندازه(اعشاری));
				اگر (xcoords == پوچ) {
					برو bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					رقم * اندازه(اعشاری));
				اگر (ycoords == پوچ) {
					برو bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[رقم - 1] = x;
				new_map->bounds.poly.ycoords[رقم - 1] = y;

				رقم++;
				val = strtok(پوچ, ",");
			}

			new_map->bounds.poly.رقم = رقم - 1;

			بشکن;
		پیشفرض:
			بشکن;
		}
	}

	new_map->next = پوچ;

	اگر (*entry) {
		/* add to END of list */
		برای (temp = (*entry); temp->next != پوچ; temp = temp->next)
			;
		temp->next = new_map;
	} وگرنه {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	برو ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	اگر (new_map != پوچ) {
		اگر (new_map->url != پوچ)
			nsurl_unref(new_map->url);
		اگر (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != پوچ)
			آزاد(new_map->bounds.poly.ycoords);
		اگر (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != پوچ)
			آزاد(new_map->bounds.poly.xcoords);
		اگر (new_map->target != پوچ)
			آزاد(new_map->target);

		آزاد(new_map);
	}
ok_out:
	اگر (href != پوچ)
		dom_string_unref(href);
	اگر (target != پوچ)
		dom_string_unref(target);
	اگر (shape != پوچ)
		dom_string_unref(shape);
	اگر (coords != پوچ)
		dom_string_unref(coords);

	بازگشت ret;
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
ایستا bool
imagemap_extract_map_entries(dom_گره *گره, html_content *c,
			     ساختار mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_شمار;

	exc = dom_element_get_elements_by_tag_name(گره, tname, &nlist);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_شمار);
	اگر (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		بازگشت false;
	}

	برای (ent = 0; ent < tag_شمار; ++ent) {
		dom_گره *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		اگر (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			بازگشت false;
		}
		اگر (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_گره_unref(subnode);
			dom_nodelist_unref(nlist);
			بازگشت false;
		}
		dom_گره_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	بازگشت true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ایستا bool imagemap_extract_map(dom_گره *گره, html_content *c,
		ساختار mapentry **entry)
{
	اگر (imagemap_extract_map_entries(گره, c, entry,
			corestring_dom_area) == false)
		بازگشت false;
	بازگشت imagemap_extract_map_entries(گره, c, entry,
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
	اگر (exc != DOM_NO_ERR) {
		بازگشت NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	اگر (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		برو out_nlist;
	}

	برای (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_گره *گره;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &گره);
		اگر (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			برو out_nlist;
		}

		exc = dom_element_get_attribute(گره, corestring_dom_id,
						&name);
		اگر (exc != DOM_NO_ERR) {
			dom_گره_unref(گره);
			ret = NSERROR_DOM;
			برو out_nlist;
		}

		اگر (name == پوچ) {
			exc = dom_element_get_attribute(گره,
							corestring_dom_name,
							&name);
			اگر (exc != DOM_NO_ERR) {
				dom_گره_unref(گره);
				ret = NSERROR_DOM;
				برو out_nlist;
			}
		}

		اگر (name != پوچ) {
			ساختار mapentry *entry = پوچ;
			اگر (imagemap_extract_map(گره, c, &entry) == false) {
				اگر (entry != پوچ) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_گره_unref(گره);
				ret = NSERROR_NOMEM; /** @todo check this */
				برو out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			اگر ((entry != پوچ) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_گره_unref(گره);
				ret = NSERROR_NOMEM; /** @todo check this */
				برو out_nlist;
			}
		}

		dom_string_unref(name);
		dom_گره_unref(گره);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	بازگشت ret;
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
ایستا صحیح
imagemap_point_in_poly(صحیح رقم, اعشاری *xpt, اعشاری *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	صحیح i, j, c = 0;

	أكد(xpt != پوچ);
	أكد(ypt != پوچ);

	برای (i = 0, j = رقم - 1; i < رقم; j = i++) {
		اگر ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	بازگشت c;
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
nsurl *imagemap_get(ساختار html_content *c, ثابت نویسه *کلید,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ثابت نویسه **target)
{
	unsigned صحیح slot = 0;
	ساختار imagemap *map;
	ساختار mapentry *entry;
	unsigned long cx, cy;

	أكد(c != پوچ);

	اگر (کلید == پوچ)
		بازگشت پوچ;

	اگر (c->imagemaps == پوچ)
		بازگشت پوچ;

	slot = imagemap_hash(کلید);

	برای (map = c->imagemaps[slot]; map != پوچ; map = map->next) {
		اگر (map->کلید != پوچ && strcasecmp(map->کلید, کلید) == 0)
			بشکن;
	}

	اگر (map == پوچ || map->list == پوچ)
		بازگشت پوچ;

	برای (entry = map->list; entry; entry = entry->next) {
		انتخاب (entry->type) {
		حالت IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			اگر (target)
				*target = entry->target;
			بازگشت entry->url;
			بشکن;
		حالت IMAGEMAP_RECT:
			اگر (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				اگر (target)
					*target = entry->target;
				بازگشت entry->url;
			}
			بشکن;
		حالت IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			اگر ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				اگر (target)
					*target = entry->target;
				بازگشت entry->url;
			}
			بشکن;
		حالت IMAGEMAP_POLY:
			اگر (imagemap_point_in_poly(entry->bounds.poly.رقم,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				اگر (target)
					*target = entry->target;
				بازگشت entry->url;
			}
			بشکن;
		}
	}

	اگر (target)
		*target = پوچ;

	بازگشت پوچ;
}
