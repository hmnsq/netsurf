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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/hlcache.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_construct.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

प्रकारनिर्देश गणना {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

संरचना mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	वर्ण *target;			/**< target frame (if any) */
	सङ्घ {
		संरचना {
			पूर्णाङ्क x;		/**< x coordinate of centre */
			पूर्णाङ्क y;		/**< y coordinate of center */
			पूर्णाङ्क r;		/**< radius of circle */
		} circle;
		संरचना {
			पूर्णाङ्क x0;		/**< left hand edge */
			पूर्णाङ्क y0;		/**< top edge */
			पूर्णाङ्क x1;		/**< right hand edge */
			पूर्णाङ्क y1;		/**< bottom edge */
		} rect;
		संरचना {
			पूर्णाङ्क अंक;	/**< number of points */
			भिन्न *xcoords;	/**< x coordinates */
			भिन्न *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	संरचना mapentry *next;		/**< next entry in list */
};

संरचना imagemap {
	वर्ण *कुंजी;		/**< key for this entry */
	संरचना mapentry *list;	/**< pointer to linked list of entries */
	संरचना imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
स्थिर bool imagemap_create(html_content *c)
{
	प्रतिज्ञा(c != रिक्त);

	यदि (c->imagemaps == रिक्त) {
		c->imagemaps = सुस्मृति(HASH_SIZE, परिमाण(संरचना imagemap *));
		यदि (c->imagemaps == रिक्त) {
			प्रत्यागम false;
		}
	}

	प्रत्यागम true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
स्थिर अचिन्हित पूर्णाङ्क imagemap_hash(ध्रुव वर्ण *कुंजी)
{
	अचिन्हित पूर्णाङ्क z = 0;

	यदि (कुंजी == 0) प्रत्यागम 0;

	प्रति (; *कुंजी != 0; कुंजी++) {
		z += *कुंजी & 0x1f;
	}

	प्रत्यागम (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
स्थिर bool
imagemap_add(html_content *c, dom_string *कुंजी, संरचना mapentry *list)
{
	संरचना imagemap *map;
	अचिन्हित पूर्णाङ्क slot;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(कुंजी != रिक्त);
	प्रतिज्ञा(list != रिक्त);

	यदि (imagemap_create(c) == false)
		प्रत्यागम false;

	map = सुस्मृति(1, परिमाण(*map));
	यदि (map == रिक्त)
		प्रत्यागम false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->कुंजी = strdup(dom_string_data(कुंजी));
	यदि (map->कुंजी == रिक्त) {
		मुक्त(map);
		प्रत्यागम false;
	}

	map->list = list;

	slot = imagemap_hash(map->कुंजी);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	प्रत्यागम true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
स्थिर शून्य imagemap_freelist(संरचना mapentry *list)
{
	संरचना mapentry *entry, *prev;

	प्रतिज्ञा(list != रिक्त);

	entry = list;

	यावत् (entry != रिक्त) {
		prev = entry;

		nsurl_unref(entry->url);

		यदि (entry->target)
			मुक्त(entry->target);

		यदि (entry->type == IMAGEMAP_POLY) {
			मुक्त(entry->bounds.poly.xcoords);
			मुक्त(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		मुक्त(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
शून्य imagemap_destroy(html_content *c)
{
	अचिन्हित पूर्णाङ्क i;

	प्रतिज्ञा(c != रिक्त);

	/* no imagemaps -> return */
	यदि (c->imagemaps == रिक्त)
		प्रत्यागम;

	प्रति (i = 0; i != HASH_SIZE; i++) {
		संरचना imagemap *map, *next;

		map = c->imagemaps[i];
		यावत् (map != रिक्त) {
			next = map->next;
			imagemap_freelist(map->list);
			मुक्त(map->कुंजी);
			मुक्त(map);
			map = next;
		}
	}

	मुक्त(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
शून्य imagemap_dump(html_content *c)
{
	अचिन्हित पूर्णाङ्क i;

	पूर्णाङ्क j;

	प्रतिज्ञा(c != रिक्त);

	यदि (c->imagemaps == रिक्त)
		प्रत्यागम;

	प्रति (i = 0; i != HASH_SIZE; i++) {
		संरचना imagemap *map;
		संरचना mapentry *entry;

		map = c->imagemaps[i];
		यावत् (map != रिक्त) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->कुंजी);

			प्रति (entry = map->list; entry; entry = entry->next) {
				विकल्प (entry->type) {
				पक्ष IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					भङ्ग;
				पक्ष IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					भङ्ग;
				पक्ष IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					भङ्ग;
				पक्ष IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					प्रति (j = 0; j != entry->bounds.poly.अंक;
							j++) {
						ख_लिखो(मानक_त्रुटि, "(%d,%d) ",
							(पूर्णाङ्क)entry->bounds.poly.xcoords[j],
							(पूर्णाङ्क)entry->bounds.poly.ycoords[j]);
					}
					ख_लिखो(मानक_त्रुटि,"\n");
					भङ्ग;
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
स्थिर bool
imagemap_addtolist(ध्रुव संरचना html_content *c,
		   dom_गाँठ *n,
		   nsurl *base_url,
		   संरचना mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = रिक्त, *target = रिक्त, *shape = रिक्त;
	dom_string *coords = रिक्त;
	संरचना mapentry *new_map, *temp;
	bool ret = true;

	यदि (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		यदि ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			गच्छ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	यदि (exc != DOM_NO_ERR || href == रिक्त) {
		/* No href="" attribute, skip this element */
		गच्छ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	यदि (exc != DOM_NO_ERR) {
		गच्छ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	यदि (exc != DOM_NO_ERR) {
		गच्छ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	यदि (shape == रिक्त)
		shape = dom_string_ref(corestring_dom_rect);

	यदि (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		यदि (exc != DOM_NO_ERR || coords == रिक्त) {
			गच्छ ok_out;
		}
	}

	new_map = सुस्मृति(1, परिमाण(*new_map));
	यदि (new_map == रिक्त) {
		गच्छ bad_out;
	}

	यदि (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	अन्यथा यदि (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	अन्यथा यदि (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	अन्यथा यदि (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	अन्यथा
		गच्छ bad_out;

	यदि (box_extract_link(c, href, base_url, &new_map->url) == false)
		गच्छ bad_out;

	यदि (new_map->url == रिक्त) {
		/* non-fatal error -> ignore this */
		गच्छ ok_free_map_out;
	}

	यदि (target != रिक्त) {
		/* Copy target dom string into the map data */
		new_map->target = दो_स्मृति(dom_string_byte_length(target) + 1);
		यदि (new_map->target == रिक्त)
			गच्छ bad_out;

		स_नकल(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	यदि (new_map->type != IMAGEMAP_DEFAULT) {
		पूर्णाङ्क x, y;
		भिन्न *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		वर्ण *val = म_मोहर((वर्ण *)dom_string_data(coords), ",");
		पूर्णाङ्क अंक = 1;

		विकल्प (new_map->type) {
		पक्ष IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			यावत् (val != रिक्त && अंक <= 4) {
				विकल्प (अंक) {
				पक्ष 1:
					new_map->bounds.rect.x0 = म_से_प(val);
					भङ्ग;
				पक्ष 2:
					new_map->bounds.rect.y0 = म_से_प(val);
					भङ्ग;
				पक्ष 3:
					new_map->bounds.rect.x1 = म_से_प(val);
					भङ्ग;
				पक्ष 4:
					new_map->bounds.rect.y1 = म_से_प(val);
					भङ्ग;
				}

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}
			भङ्ग;
		पक्ष IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			यावत् (val != रिक्त && अंक <= 3) {
				विकल्प (अंक) {
				पक्ष 1:
					new_map->bounds.circle.x = म_से_प(val);
					भङ्ग;
				पक्ष 2:
					new_map->bounds.circle.y = म_से_प(val);
					भङ्ग;
				पक्ष 3:
					new_map->bounds.circle.r = म_से_प(val);
					भङ्ग;
				}

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}
			भङ्ग;
		पक्ष IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = रिक्त;
			new_map->bounds.poly.ycoords = रिक्त;

			यावत् (val != रिक्त) {
				x = म_से_प(val);

				val = म_मोहर(रिक्त, ",");
				यदि (val == रिक्त)
					भङ्ग;

				y = म_से_प(val);

				xcoords = पुनः_स्मृति(new_map->bounds.poly.xcoords,
						अंक * परिमाण(भिन्न));
				यदि (xcoords == रिक्त) {
					गच्छ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = पुनः_स्मृति(new_map->bounds.poly.ycoords,
					अंक * परिमाण(भिन्न));
				यदि (ycoords == रिक्त) {
					गच्छ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[अंक - 1] = x;
				new_map->bounds.poly.ycoords[अंक - 1] = y;

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}

			new_map->bounds.poly.अंक = अंक - 1;

			भङ्ग;
		स्वभाव:
			भङ्ग;
		}
	}

	new_map->next = रिक्त;

	यदि (*entry) {
		/* add to END of list */
		प्रति (temp = (*entry); temp->next != रिक्त; temp = temp->next)
			;
		temp->next = new_map;
	} अन्यथा {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	गच्छ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	यदि (new_map != रिक्त) {
		यदि (new_map->url != रिक्त)
			nsurl_unref(new_map->url);
		यदि (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != रिक्त)
			मुक्त(new_map->bounds.poly.ycoords);
		यदि (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != रिक्त)
			मुक्त(new_map->bounds.poly.xcoords);
		यदि (new_map->target != रिक्त)
			मुक्त(new_map->target);

		मुक्त(new_map);
	}
ok_out:
	यदि (href != रिक्त)
		dom_string_unref(href);
	यदि (target != रिक्त)
		dom_string_unref(target);
	यदि (shape != रिक्त)
		dom_string_unref(shape);
	यदि (coords != रिक्त)
		dom_string_unref(coords);

	प्रत्यागम ret;
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
स्थिर bool
imagemap_extract_map_entries(dom_गाँठ *गाँठ, html_content *c,
			     संरचना mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	अचिन्हित दीर्घ ent;
	uint32_t tag_गिनती;

	exc = dom_element_get_elements_by_tag_name(गाँठ, tname, &nlist);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_गिनती);
	यदि (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		प्रत्यागम false;
	}

	प्रति (ent = 0; ent < tag_गिनती; ++ent) {
		dom_गाँठ *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		यदि (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			प्रत्यागम false;
		}
		यदि (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_गाँठ_unref(subnode);
			dom_nodelist_unref(nlist);
			प्रत्यागम false;
		}
		dom_गाँठ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	प्रत्यागम true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
स्थिर bool imagemap_extract_map(dom_गाँठ *गाँठ, html_content *c,
		संरचना mapentry **entry)
{
	यदि (imagemap_extract_map_entries(गाँठ, c, entry,
			corestring_dom_area) == false)
		प्रत्यागम false;
	प्रत्यागम imagemap_extract_map_entries(गाँठ, c, entry,
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
	अचिन्हित दीर्घ mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_tag_name(c->document,
						    corestring_dom_map,
						    &nlist);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	यदि (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		गच्छ out_nlist;
	}

	प्रति (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_गाँठ *गाँठ;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &गाँठ);
		यदि (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			गच्छ out_nlist;
		}

		exc = dom_element_get_attribute(गाँठ, corestring_dom_id,
						&name);
		यदि (exc != DOM_NO_ERR) {
			dom_गाँठ_unref(गाँठ);
			ret = NSERROR_DOM;
			गच्छ out_nlist;
		}

		यदि (name == रिक्त) {
			exc = dom_element_get_attribute(गाँठ,
							corestring_dom_name,
							&name);
			यदि (exc != DOM_NO_ERR) {
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_DOM;
				गच्छ out_nlist;
			}
		}

		यदि (name != रिक्त) {
			संरचना mapentry *entry = रिक्त;
			यदि (imagemap_extract_map(गाँठ, c, &entry) == false) {
				यदि (entry != रिक्त) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				गच्छ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			यदि ((entry != रिक्त) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				गच्छ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_गाँठ_unref(गाँठ);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	प्रत्यागम ret;
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
स्थिर पूर्णाङ्क
imagemap_point_in_poly(पूर्णाङ्क अंक, भिन्न *xpt, भिन्न *ypt, अचिन्हित दीर्घ x,
		अचिन्हित दीर्घ y, अचिन्हित दीर्घ click_x,	अचिन्हित दीर्घ click_y)
{
	पूर्णाङ्क i, j, c = 0;

	प्रतिज्ञा(xpt != रिक्त);
	प्रतिज्ञा(ypt != रिक्त);

	प्रति (i = 0, j = अंक - 1; i < अंक; j = i++) {
		यदि ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	प्रत्यागम c;
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
nsurl *imagemap_get(संरचना html_content *c, ध्रुव वर्ण *कुंजी,
		अचिन्हित दीर्घ x, अचिन्हित दीर्घ y,
		अचिन्हित दीर्घ click_x, अचिन्हित दीर्घ click_y,
		ध्रुव वर्ण **target)
{
	अचिन्हित पूर्णाङ्क slot = 0;
	संरचना imagemap *map;
	संरचना mapentry *entry;
	अचिन्हित दीर्घ cx, cy;

	प्रतिज्ञा(c != रिक्त);

	यदि (कुंजी == रिक्त)
		प्रत्यागम रिक्त;

	यदि (c->imagemaps == रिक्त)
		प्रत्यागम रिक्त;

	slot = imagemap_hash(कुंजी);

	प्रति (map = c->imagemaps[slot]; map != रिक्त; map = map->next) {
		यदि (map->कुंजी != रिक्त && strcasecmp(map->कुंजी, कुंजी) == 0)
			भङ्ग;
	}

	यदि (map == रिक्त || map->list == रिक्त)
		प्रत्यागम रिक्त;

	प्रति (entry = map->list; entry; entry = entry->next) {
		विकल्प (entry->type) {
		पक्ष IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			यदि (target)
				*target = entry->target;
			प्रत्यागम entry->url;
			भङ्ग;
		पक्ष IMAGEMAP_RECT:
			यदि (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				यदि (target)
					*target = entry->target;
				प्रत्यागम entry->url;
			}
			भङ्ग;
		पक्ष IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			यदि ((cx * cx + cy * cy) <=
				(अचिन्हित दीर्घ) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				यदि (target)
					*target = entry->target;
				प्रत्यागम entry->url;
			}
			भङ्ग;
		पक्ष IMAGEMAP_POLY:
			यदि (imagemap_point_in_poly(entry->bounds.poly.अंक,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				यदि (target)
					*target = entry->target;
				प्रत्यागम entry->url;
			}
			भङ्ग;
		}
	}

	यदि (target)
		*target = रिक्त;

	प्रत्यागम रिक्त;
}
