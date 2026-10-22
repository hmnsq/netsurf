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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>

#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/hlcache.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_construct.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/imagemap.h"

#define HASH_আকার 31 /* fixed size hash table */

প্রকারলেখ গণনা {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

কাঠামো mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	অক্ষর *target;			/**< target frame (if any) */
	সংঘ {
		কাঠামো {
			পূর্ণসংখ্যা x;		/**< x coordinate of centre */
			পূর্ণসংখ্যা y;		/**< y coordinate of center */
			পূর্ণসংখ্যা r;		/**< radius of circle */
		} circle;
		কাঠামো {
			পূর্ণসংখ্যা x0;		/**< left hand edge */
			পূর্ণসংখ্যা y0;		/**< top edge */
			পূর্ণসংখ্যা x1;		/**< right hand edge */
			পূর্ণসংখ্যা y1;		/**< bottom edge */
		} rect;
		কাঠামো {
			পূর্ণসংখ্যা অঙ্ক;	/**< number of points */
			দশমিক *xcoords;	/**< x coordinates */
			দশমিক *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	কাঠামো mapentry *পরবর্তী;		/**< next entry in list */
};

কাঠামো imagemap {
	অক্ষর *চাবি;		/**< key for this entry */
	কাঠামো mapentry *list;	/**< pointer to linked list of entries */
	কাঠামো imagemap *পরবর্তী;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
স্থির bool imagemap_create(html_content *c)
{
	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->imagemaps == ফাঁকা) {
		c->imagemaps = calloc(HASH_আকার, আকার(কাঠামো imagemap *));
		যদি (c->imagemaps == ফাঁকা) {
			ফেরত false;
		}
	}

	ফেরত true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
স্থির unsigned পূর্ণসংখ্যা imagemap_hash(ধ্রুবক অক্ষর *চাবি)
{
	unsigned পূর্ণসংখ্যা z = 0;

	যদি (চাবি == 0) ফেরত 0;

	জন্য (; *চাবি != 0; চাবি++) {
		z += *চাবি & 0x1f;
	}

	ফেরত (z % (HASH_আকার - 1)) + 1;
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
imagemap_add(html_content *c, dom_string *চাবি, কাঠামো mapentry *list)
{
	কাঠামো imagemap *map;
	unsigned পূর্ণসংখ্যা slot;

	প্রতিজ্ঞা(c != ফাঁকা);
	প্রতিজ্ঞা(চাবি != ফাঁকা);
	প্রতিজ্ঞা(list != ফাঁকা);

	যদি (imagemap_create(c) == false)
		ফেরত false;

	map = calloc(1, আকার(*map));
	যদি (map == ফাঁকা)
		ফেরত false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->চাবি = strdup(dom_string_তথ্য(চাবি));
	যদি (map->চাবি == ফাঁকা) {
		মুক্ত(map);
		ফেরত false;
	}

	map->list = list;

	slot = imagemap_hash(map->চাবি);

	map->পরবর্তী = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ফেরত true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
স্থির শূন্য imagemap_freelist(কাঠামো mapentry *list)
{
	কাঠামো mapentry *entry, *পূর্ব;

	প্রতিজ্ঞা(list != ফাঁকা);

	entry = list;

	যখন (entry != ফাঁকা) {
		পূর্ব = entry;

		nsurl_unref(entry->url);

		যদি (entry->target)
			মুক্ত(entry->target);

		যদি (entry->type == IMAGEMAP_POLY) {
			মুক্ত(entry->bounds.poly.xcoords);
			মুক্ত(entry->bounds.poly.ycoords);
		}

		entry = entry->পরবর্তী;
		মুক্ত(পূর্ব);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
শূন্য imagemap_destroy(html_content *c)
{
	unsigned পূর্ণসংখ্যা i;

	প্রতিজ্ঞা(c != ফাঁকা);

	/* no imagemaps -> return */
	যদি (c->imagemaps == ফাঁকা)
		ফেরত;

	জন্য (i = 0; i != HASH_আকার; i++) {
		কাঠামো imagemap *map, *পরবর্তী;

		map = c->imagemaps[i];
		যখন (map != ফাঁকা) {
			পরবর্তী = map->পরবর্তী;
			imagemap_freelist(map->list);
			মুক্ত(map->চাবি);
			মুক্ত(map);
			map = পরবর্তী;
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
	unsigned পূর্ণসংখ্যা i;

	পূর্ণসংখ্যা j;

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->imagemaps == ফাঁকা)
		ফেরত;

	জন্য (i = 0; i != HASH_আকার; i++) {
		কাঠামো imagemap *map;
		কাঠামো mapentry *entry;

		map = c->imagemaps[i];
		যখন (map != ফাঁকা) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->চাবি);

			জন্য (entry = map->list; entry; entry = entry->পরবর্তী) {
				নির্বাচন (entry->type) {
				ক্ষেত্র IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					বিরতি;
				ক্ষেত্র IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					বিরতি;
				ক্ষেত্র IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					বিরতি;
				ক্ষেত্র IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					জন্য (j = 0; j != entry->bounds.poly.অঙ্ক;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(পূর্ণসংখ্যা)entry->bounds.poly.xcoords[j],
							(পূর্ণসংখ্যা)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					বিরতি;
				}
			}
			map = map->পরবর্তী;
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
imagemap_addtolist(ধ্রুবক কাঠামো html_content *c,
		   dom_গ্রন্থি *n,
		   nsurl *base_url,
		   কাঠামো mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ফাঁকা, *target = ফাঁকা, *shape = ফাঁকা;
	dom_string *coords = ফাঁকা;
	কাঠামো mapentry *new_map, *অস্থায়ী;
	bool ret = true;

	যদি (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		যদি ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			যাও ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	যদি (exc != DOM_NO_ERR || href == ফাঁকা) {
		/* No href="" attribute, skip this element */
		যাও ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	যদি (exc != DOM_NO_ERR) {
		যাও ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	যদি (exc != DOM_NO_ERR) {
		যাও ok_out;
	}

	/* If there's no shape, we default to rectangles */
	যদি (shape == ফাঁকা)
		shape = dom_string_ref(corestring_dom_rect);

	যদি (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		যদি (exc != DOM_NO_ERR || coords == ফাঁকা) {
			যাও ok_out;
		}
	}

	new_map = calloc(1, আকার(*new_map));
	যদি (new_map == ফাঁকা) {
		যাও bad_out;
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
		যাও bad_out;

	যদি (box_extract_link(c, href, base_url, &new_map->url) == false)
		যাও bad_out;

	যদি (new_map->url == ফাঁকা) {
		/* non-fatal error -> ignore this */
		যাও ok_free_map_out;
	}

	যদি (target != ফাঁকা) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_দৈর্ঘ্য(target) + 1);
		যদি (new_map->target == ফাঁকা)
			যাও bad_out;

		memcpy(new_map->target,
		       dom_string_তথ্য(target),
		       dom_string_byte_দৈর্ঘ্য(target));

		new_map->target[dom_string_byte_দৈর্ঘ্য(target)] = 0;
	}

	যদি (new_map->type != IMAGEMAP_DEFAULT) {
		পূর্ণসংখ্যা x, y;
		দশমিক *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		অক্ষর *val = strtok((অক্ষর *)dom_string_তথ্য(coords), ",");
		পূর্ণসংখ্যা অঙ্ক = 1;

		নির্বাচন (new_map->type) {
		ক্ষেত্র IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			যখন (val != ফাঁকা && অঙ্ক <= 4) {
				নির্বাচন (অঙ্ক) {
				ক্ষেত্র 1:
					new_map->bounds.rect.x0 = atoi(val);
					বিরতি;
				ক্ষেত্র 2:
					new_map->bounds.rect.y0 = atoi(val);
					বিরতি;
				ক্ষেত্র 3:
					new_map->bounds.rect.x1 = atoi(val);
					বিরতি;
				ক্ষেত্র 4:
					new_map->bounds.rect.y1 = atoi(val);
					বিরতি;
				}

				অঙ্ক++;
				val = strtok(ফাঁকা, ",");
			}
			বিরতি;
		ক্ষেত্র IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			যখন (val != ফাঁকা && অঙ্ক <= 3) {
				নির্বাচন (অঙ্ক) {
				ক্ষেত্র 1:
					new_map->bounds.circle.x = atoi(val);
					বিরতি;
				ক্ষেত্র 2:
					new_map->bounds.circle.y = atoi(val);
					বিরতি;
				ক্ষেত্র 3:
					new_map->bounds.circle.r = atoi(val);
					বিরতি;
				}

				অঙ্ক++;
				val = strtok(ফাঁকা, ",");
			}
			বিরতি;
		ক্ষেত্র IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ফাঁকা;
			new_map->bounds.poly.ycoords = ফাঁকা;

			যখন (val != ফাঁকা) {
				x = atoi(val);

				val = strtok(ফাঁকা, ",");
				যদি (val == ফাঁকা)
					বিরতি;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						অঙ্ক * আকার(দশমিক));
				যদি (xcoords == ফাঁকা) {
					যাও bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					অঙ্ক * আকার(দশমিক));
				যদি (ycoords == ফাঁকা) {
					যাও bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[অঙ্ক - 1] = x;
				new_map->bounds.poly.ycoords[অঙ্ক - 1] = y;

				অঙ্ক++;
				val = strtok(ফাঁকা, ",");
			}

			new_map->bounds.poly.অঙ্ক = অঙ্ক - 1;

			বিরতি;
		পূর্বনির্ধারিত:
			বিরতি;
		}
	}

	new_map->পরবর্তী = ফাঁকা;

	যদি (*entry) {
		/* add to END of list */
		জন্য (অস্থায়ী = (*entry); অস্থায়ী->পরবর্তী != ফাঁকা; অস্থায়ী = অস্থায়ী->পরবর্তী)
			;
		অস্থায়ী->পরবর্তী = new_map;
	} অন্যথা {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	যাও ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	যদি (new_map != ফাঁকা) {
		যদি (new_map->url != ফাঁকা)
			nsurl_unref(new_map->url);
		যদি (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ফাঁকা)
			মুক্ত(new_map->bounds.poly.ycoords);
		যদি (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ফাঁকা)
			মুক্ত(new_map->bounds.poly.xcoords);
		যদি (new_map->target != ফাঁকা)
			মুক্ত(new_map->target);

		মুক্ত(new_map);
	}
ok_out:
	যদি (href != ফাঁকা)
		dom_string_unref(href);
	যদি (target != ফাঁকা)
		dom_string_unref(target);
	যদি (shape != ফাঁকা)
		dom_string_unref(shape);
	যদি (coords != ফাঁকা)
		dom_string_unref(coords);

	ফেরত ret;
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
imagemap_extract_map_entries(dom_গ্রন্থি *গ্রন্থি, html_content *c,
			     কাঠামো mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_গুনতি;

	exc = dom_element_get_elements_by_tag_name(গ্রন্থি, tname, &nlist);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	exc = dom_nodelist_get_দৈর্ঘ্য(nlist, &tag_গুনতি);
	যদি (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ফেরত false;
	}

	জন্য (ent = 0; ent < tag_গুনতি; ++ent) {
		dom_গ্রন্থি *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		যদি (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ফেরত false;
		}
		যদি (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_গ্রন্থি_unref(subnode);
			dom_nodelist_unref(nlist);
			ফেরত false;
		}
		dom_গ্রন্থি_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ফেরত true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
স্থির bool imagemap_extract_map(dom_গ্রন্থি *গ্রন্থি, html_content *c,
		কাঠামো mapentry **entry)
{
	যদি (imagemap_extract_map_entries(গ্রন্থি, c, entry,
			corestring_dom_area) == false)
		ফেরত false;
	ফেরত imagemap_extract_map_entries(গ্রন্থি, c, entry,
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
		ফেরত NSERROR_DOM;
	}

	exc = dom_nodelist_get_দৈর্ঘ্য(nlist, &maybe_maps);
	যদি (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		যাও out_nlist;
	}

	জন্য (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_গ্রন্থি *গ্রন্থি;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &গ্রন্থি);
		যদি (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			যাও out_nlist;
		}

		exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_id,
						&name);
		যদি (exc != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(গ্রন্থি);
			ret = NSERROR_DOM;
			যাও out_nlist;
		}

		যদি (name == ফাঁকা) {
			exc = dom_element_get_attribute(গ্রন্থি,
							corestring_dom_name,
							&name);
			যদি (exc != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(গ্রন্থি);
				ret = NSERROR_DOM;
				যাও out_nlist;
			}
		}

		যদি (name != ফাঁকা) {
			কাঠামো mapentry *entry = ফাঁকা;
			যদি (imagemap_extract_map(গ্রন্থি, c, &entry) == false) {
				যদি (entry != ফাঁকা) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_গ্রন্থি_unref(গ্রন্থি);
				ret = NSERROR_NOMEM; /** @todo check this */
				যাও out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			যদি ((entry != ফাঁকা) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_গ্রন্থি_unref(গ্রন্থি);
				ret = NSERROR_NOMEM; /** @todo check this */
				যাও out_nlist;
			}
		}

		dom_string_unref(name);
		dom_গ্রন্থি_unref(গ্রন্থি);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ফেরত ret;
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
স্থির পূর্ণসংখ্যা
imagemap_point_in_poly(পূর্ণসংখ্যা অঙ্ক, দশমিক *xpt, দশমিক *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	পূর্ণসংখ্যা i, j, c = 0;

	প্রতিজ্ঞা(xpt != ফাঁকা);
	প্রতিজ্ঞা(ypt != ফাঁকা);

	জন্য (i = 0, j = অঙ্ক - 1; i < অঙ্ক; j = i++) {
		যদি ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ফেরত c;
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
nsurl *imagemap_get(কাঠামো html_content *c, ধ্রুবক অক্ষর *চাবি,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ধ্রুবক অক্ষর **target)
{
	unsigned পূর্ণসংখ্যা slot = 0;
	কাঠামো imagemap *map;
	কাঠামো mapentry *entry;
	unsigned long cx, cy;

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (চাবি == ফাঁকা)
		ফেরত ফাঁকা;

	যদি (c->imagemaps == ফাঁকা)
		ফেরত ফাঁকা;

	slot = imagemap_hash(চাবি);

	জন্য (map = c->imagemaps[slot]; map != ফাঁকা; map = map->পরবর্তী) {
		যদি (map->চাবি != ফাঁকা && strcasecmp(map->চাবি, চাবি) == 0)
			বিরতি;
	}

	যদি (map == ফাঁকা || map->list == ফাঁকা)
		ফেরত ফাঁকা;

	জন্য (entry = map->list; entry; entry = entry->পরবর্তী) {
		নির্বাচন (entry->type) {
		ক্ষেত্র IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			যদি (target)
				*target = entry->target;
			ফেরত entry->url;
			বিরতি;
		ক্ষেত্র IMAGEMAP_RECT:
			যদি (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				যদি (target)
					*target = entry->target;
				ফেরত entry->url;
			}
			বিরতি;
		ক্ষেত্র IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			যদি ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				যদি (target)
					*target = entry->target;
				ফেরত entry->url;
			}
			বিরতি;
		ক্ষেত্র IMAGEMAP_POLY:
			যদি (imagemap_point_in_poly(entry->bounds.poly.অঙ্ক,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				যদি (target)
					*target = entry->target;
				ফেরত entry->url;
			}
			বিরতি;
		}
	}

	যদি (target)
		*target = ফাঁকা;

	ফেরত ফাঁকা;
}
