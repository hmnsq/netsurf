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

प्रकारव्याख्या गणना {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

संरचना mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	अक्षर *target;			/**< target frame (if any) */
	संघ {
		संरचना {
			पूर्णांक x;		/**< x coordinate of centre */
			पूर्णांक y;		/**< y coordinate of center */
			पूर्णांक r;		/**< radius of circle */
		} circle;
		संरचना {
			पूर्णांक x0;		/**< left hand edge */
			पूर्णांक y0;		/**< top edge */
			पूर्णांक x1;		/**< right hand edge */
			पूर्णांक y1;		/**< bottom edge */
		} rect;
		संरचना {
			पूर्णांक अंक;	/**< number of points */
			दशांश *xcoords;	/**< x coordinates */
			दशांश *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	संरचना mapentry *next;		/**< next entry in list */
};

संरचना imagemap {
	अक्षर *किल्ली;		/**< key for this entry */
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

	जर (c->imagemaps == रिक्त) {
		c->imagemaps = सुस्मृति(HASH_SIZE, आकार(संरचना imagemap *));
		जर (c->imagemaps == रिक्त) {
			परत false;
		}
	}

	परत true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
स्थिर अचिन्हित पूर्णांक imagemap_hash(स्थिरांक अक्षर *किल्ली)
{
	अचिन्हित पूर्णांक z = 0;

	जर (किल्ली == 0) परत 0;

	साठी (; *किल्ली != 0; किल्ली++) {
		z += *किल्ली & 0x1f;
	}

	परत (z % (HASH_SIZE - 1)) + 1;
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
imagemap_add(html_content *c, dom_string *किल्ली, संरचना mapentry *list)
{
	संरचना imagemap *map;
	अचिन्हित पूर्णांक slot;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(किल्ली != रिक्त);
	प्रतिज्ञा(list != रिक्त);

	जर (imagemap_create(c) == false)
		परत false;

	map = सुस्मृति(1, आकार(*map));
	जर (map == रिक्त)
		परत false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->किल्ली = strdup(dom_string_data(किल्ली));
	जर (map->किल्ली == रिक्त) {
		मुक्त(map);
		परत false;
	}

	map->list = list;

	slot = imagemap_hash(map->किल्ली);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	परत true;
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

	पर्यंत (entry != रिक्त) {
		prev = entry;

		nsurl_unref(entry->url);

		जर (entry->target)
			मुक्त(entry->target);

		जर (entry->type == IMAGEMAP_POLY) {
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
	अचिन्हित पूर्णांक i;

	प्रतिज्ञा(c != रिक्त);

	/* no imagemaps -> return */
	जर (c->imagemaps == रिक्त)
		परत;

	साठी (i = 0; i != HASH_SIZE; i++) {
		संरचना imagemap *map, *next;

		map = c->imagemaps[i];
		पर्यंत (map != रिक्त) {
			next = map->next;
			imagemap_freelist(map->list);
			मुक्त(map->किल्ली);
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
	अचिन्हित पूर्णांक i;

	पूर्णांक j;

	प्रतिज्ञा(c != रिक्त);

	जर (c->imagemaps == रिक्त)
		परत;

	साठी (i = 0; i != HASH_SIZE; i++) {
		संरचना imagemap *map;
		संरचना mapentry *entry;

		map = c->imagemaps[i];
		पर्यंत (map != रिक्त) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->किल्ली);

			साठी (entry = map->list; entry; entry = entry->next) {
				निवड (entry->type) {
				स्थिती IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					थांबा;
				स्थिती IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					थांबा;
				स्थिती IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					थांबा;
				स्थिती IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					साठी (j = 0; j != entry->bounds.poly.अंक;
							j++) {
						ख_लिखो(मानक_त्रुटि, "(%d,%d) ",
							(पूर्णांक)entry->bounds.poly.xcoords[j],
							(पूर्णांक)entry->bounds.poly.ycoords[j]);
					}
					ख_लिखो(मानक_त्रुटि,"\n");
					थांबा;
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
imagemap_addtolist(स्थिरांक संरचना html_content *c,
		   dom_गाठ *n,
		   nsurl *base_url,
		   संरचना mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = रिक्त, *target = रिक्त, *shape = रिक्त;
	dom_string *coords = रिक्त;
	संरचना mapentry *new_map, *temp;
	bool ret = true;

	जर (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		जर ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			जा ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	जर (exc != DOM_NO_ERR || href == रिक्त) {
		/* No href="" attribute, skip this element */
		जा ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	जर (exc != DOM_NO_ERR) {
		जा ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	जर (exc != DOM_NO_ERR) {
		जा ok_out;
	}

	/* If there's no shape, we default to rectangles */
	जर (shape == रिक्त)
		shape = dom_string_ref(corestring_dom_rect);

	जर (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		जर (exc != DOM_NO_ERR || coords == रिक्त) {
			जा ok_out;
		}
	}

	new_map = सुस्मृति(1, आकार(*new_map));
	जर (new_map == रिक्त) {
		जा bad_out;
	}

	जर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	अन्यथा जर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	अन्यथा जर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	अन्यथा जर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	अन्यथा
		जा bad_out;

	जर (box_extract_link(c, href, base_url, &new_map->url) == false)
		जा bad_out;

	जर (new_map->url == रिक्त) {
		/* non-fatal error -> ignore this */
		जा ok_free_map_out;
	}

	जर (target != रिक्त) {
		/* Copy target dom string into the map data */
		new_map->target = दो_स्मृति(dom_string_byte_length(target) + 1);
		जर (new_map->target == रिक्त)
			जा bad_out;

		स_नकल(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	जर (new_map->type != IMAGEMAP_DEFAULT) {
		पूर्णांक x, y;
		दशांश *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		अक्षर *val = म_मोहर((अक्षर *)dom_string_data(coords), ",");
		पूर्णांक अंक = 1;

		निवड (new_map->type) {
		स्थिती IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			पर्यंत (val != रिक्त && अंक <= 4) {
				निवड (अंक) {
				स्थिती 1:
					new_map->bounds.rect.x0 = म_से_प(val);
					थांबा;
				स्थिती 2:
					new_map->bounds.rect.y0 = म_से_प(val);
					थांबा;
				स्थिती 3:
					new_map->bounds.rect.x1 = म_से_प(val);
					थांबा;
				स्थिती 4:
					new_map->bounds.rect.y1 = म_से_प(val);
					थांबा;
				}

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}
			थांबा;
		स्थिती IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			पर्यंत (val != रिक्त && अंक <= 3) {
				निवड (अंक) {
				स्थिती 1:
					new_map->bounds.circle.x = म_से_प(val);
					थांबा;
				स्थिती 2:
					new_map->bounds.circle.y = म_से_प(val);
					थांबा;
				स्थिती 3:
					new_map->bounds.circle.r = म_से_प(val);
					थांबा;
				}

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}
			थांबा;
		स्थिती IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = रिक्त;
			new_map->bounds.poly.ycoords = रिक्त;

			पर्यंत (val != रिक्त) {
				x = म_से_प(val);

				val = म_मोहर(रिक्त, ",");
				जर (val == रिक्त)
					थांबा;

				y = म_से_प(val);

				xcoords = पुनः_स्मृति(new_map->bounds.poly.xcoords,
						अंक * आकार(दशांश));
				जर (xcoords == रिक्त) {
					जा bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = पुनः_स्मृति(new_map->bounds.poly.ycoords,
					अंक * आकार(दशांश));
				जर (ycoords == रिक्त) {
					जा bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[अंक - 1] = x;
				new_map->bounds.poly.ycoords[अंक - 1] = y;

				अंक++;
				val = म_मोहर(रिक्त, ",");
			}

			new_map->bounds.poly.अंक = अंक - 1;

			थांबा;
		पूर्वनिर्धारित:
			थांबा;
		}
	}

	new_map->next = रिक्त;

	जर (*entry) {
		/* add to END of list */
		साठी (temp = (*entry); temp->next != रिक्त; temp = temp->next)
			;
		temp->next = new_map;
	} अन्यथा {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	जा ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	जर (new_map != रिक्त) {
		जर (new_map->url != रिक्त)
			nsurl_unref(new_map->url);
		जर (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != रिक्त)
			मुक्त(new_map->bounds.poly.ycoords);
		जर (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != रिक्त)
			मुक्त(new_map->bounds.poly.xcoords);
		जर (new_map->target != रिक्त)
			मुक्त(new_map->target);

		मुक्त(new_map);
	}
ok_out:
	जर (href != रिक्त)
		dom_string_unref(href);
	जर (target != रिक्त)
		dom_string_unref(target);
	जर (shape != रिक्त)
		dom_string_unref(shape);
	जर (coords != रिक्त)
		dom_string_unref(coords);

	परत ret;
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
imagemap_extract_map_entries(dom_गाठ *गाठ, html_content *c,
			     संरचना mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	अचिन्हित दीर्घ ent;
	uint32_t tag_मोजणी;

	exc = dom_element_get_elements_by_tag_name(गाठ, tname, &nlist);
	जर (exc != DOM_NO_ERR) {
		परत false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_मोजणी);
	जर (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		परत false;
	}

	साठी (ent = 0; ent < tag_मोजणी; ++ent) {
		dom_गाठ *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		जर (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			परत false;
		}
		जर (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_गाठ_unref(subnode);
			dom_nodelist_unref(nlist);
			परत false;
		}
		dom_गाठ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	परत true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
स्थिर bool imagemap_extract_map(dom_गाठ *गाठ, html_content *c,
		संरचना mapentry **entry)
{
	जर (imagemap_extract_map_entries(गाठ, c, entry,
			corestring_dom_area) == false)
		परत false;
	परत imagemap_extract_map_entries(गाठ, c, entry,
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
	जर (exc != DOM_NO_ERR) {
		परत NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	जर (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		जा out_nlist;
	}

	साठी (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_गाठ *गाठ;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &गाठ);
		जर (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			जा out_nlist;
		}

		exc = dom_element_get_attribute(गाठ, corestring_dom_id,
						&name);
		जर (exc != DOM_NO_ERR) {
			dom_गाठ_unref(गाठ);
			ret = NSERROR_DOM;
			जा out_nlist;
		}

		जर (name == रिक्त) {
			exc = dom_element_get_attribute(गाठ,
							corestring_dom_name,
							&name);
			जर (exc != DOM_NO_ERR) {
				dom_गाठ_unref(गाठ);
				ret = NSERROR_DOM;
				जा out_nlist;
			}
		}

		जर (name != रिक्त) {
			संरचना mapentry *entry = रिक्त;
			जर (imagemap_extract_map(गाठ, c, &entry) == false) {
				जर (entry != रिक्त) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_गाठ_unref(गाठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				जा out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			जर ((entry != रिक्त) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_गाठ_unref(गाठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				जा out_nlist;
			}
		}

		dom_string_unref(name);
		dom_गाठ_unref(गाठ);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	परत ret;
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
स्थिर पूर्णांक
imagemap_point_in_poly(पूर्णांक अंक, दशांश *xpt, दशांश *ypt, अचिन्हित दीर्घ x,
		अचिन्हित दीर्घ y, अचिन्हित दीर्घ click_x,	अचिन्हित दीर्घ click_y)
{
	पूर्णांक i, j, c = 0;

	प्रतिज्ञा(xpt != रिक्त);
	प्रतिज्ञा(ypt != रिक्त);

	साठी (i = 0, j = अंक - 1; i < अंक; j = i++) {
		जर ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	परत c;
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
nsurl *imagemap_get(संरचना html_content *c, स्थिरांक अक्षर *किल्ली,
		अचिन्हित दीर्घ x, अचिन्हित दीर्घ y,
		अचिन्हित दीर्घ click_x, अचिन्हित दीर्घ click_y,
		स्थिरांक अक्षर **target)
{
	अचिन्हित पूर्णांक slot = 0;
	संरचना imagemap *map;
	संरचना mapentry *entry;
	अचिन्हित दीर्घ cx, cy;

	प्रतिज्ञा(c != रिक्त);

	जर (किल्ली == रिक्त)
		परत रिक्त;

	जर (c->imagemaps == रिक्त)
		परत रिक्त;

	slot = imagemap_hash(किल्ली);

	साठी (map = c->imagemaps[slot]; map != रिक्त; map = map->next) {
		जर (map->किल्ली != रिक्त && strcasecmp(map->किल्ली, किल्ली) == 0)
			थांबा;
	}

	जर (map == रिक्त || map->list == रिक्त)
		परत रिक्त;

	साठी (entry = map->list; entry; entry = entry->next) {
		निवड (entry->type) {
		स्थिती IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			जर (target)
				*target = entry->target;
			परत entry->url;
			थांबा;
		स्थिती IMAGEMAP_RECT:
			जर (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				जर (target)
					*target = entry->target;
				परत entry->url;
			}
			थांबा;
		स्थिती IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			जर ((cx * cx + cy * cy) <=
				(अचिन्हित दीर्घ) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				जर (target)
					*target = entry->target;
				परत entry->url;
			}
			थांबा;
		स्थिती IMAGEMAP_POLY:
			जर (imagemap_point_in_poly(entry->bounds.poly.अंक,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				जर (target)
					*target = entry->target;
				परत entry->url;
			}
			थांबा;
		}
	}

	जर (target)
		*target = रिक्त;

	परत रिक्त;
}
