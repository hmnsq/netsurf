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

#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>

#समावेश <dom/dom.h>

#समावेश "utils/log.h"
#समावेश "utils/corestrings.h"
#समावेश "content/content_protected.h"
#समावेश "content/hlcache.h"

#समावेश "html/box.h"
#समावेश "html/box_construct.h"
#समावेश "html/private.h"
#समावेश "html/imagemap.h"

#घोषणा HASH_आकार 31 /* fixed size hash table */

प्रकार क्रमागत {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

काष्ठा mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	अक्षर *target;			/**< target frame (if any) */
	जोड़ {
		काष्ठा {
			पूर्णांक x;		/**< x coordinate of centre */
			पूर्णांक y;		/**< y coordinate of center */
			पूर्णांक r;		/**< radius of circle */
		} circle;
		काष्ठा {
			पूर्णांक x0;		/**< left hand edge */
			पूर्णांक y0;		/**< top edge */
			पूर्णांक x1;		/**< right hand edge */
			पूर्णांक y1;		/**< bottom edge */
		} rect;
		काष्ठा {
			पूर्णांक अंक;	/**< number of points */
			भग्न *xcoords;	/**< x coordinates */
			भग्न *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	काष्ठा mapentry *अग्र;		/**< next entry in list */
};

काष्ठा imagemap {
	अक्षर *कुंजी;		/**< key for this entry */
	काष्ठा mapentry *list;	/**< pointer to linked list of entries */
	काष्ठा imagemap *अग्र;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
अटल bool imagemap_create(html_content *c)
{
	निश्चित(c != शून्य);

	अगर (c->imagemaps == शून्य) {
		c->imagemaps = सुस्मृति(HASH_आकार, माप(काष्ठा imagemap *));
		अगर (c->imagemaps == शून्य) {
			वापस false;
		}
	}

	वापस true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
अटल अचिन्हित पूर्णांक imagemap_hash(स्थिर अक्षर *कुंजी)
{
	अचिन्हित पूर्णांक z = 0;

	अगर (कुंजी == 0) वापस 0;

	क्रम (; *कुंजी != 0; कुंजी++) {
		z += *कुंजी & 0x1f;
	}

	वापस (z % (HASH_आकार - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
अटल bool
imagemap_add(html_content *c, dom_string *कुंजी, काष्ठा mapentry *list)
{
	काष्ठा imagemap *map;
	अचिन्हित पूर्णांक slot;

	निश्चित(c != शून्य);
	निश्चित(कुंजी != शून्य);
	निश्चित(list != शून्य);

	अगर (imagemap_create(c) == false)
		वापस false;

	map = सुस्मृति(1, माप(*map));
	अगर (map == शून्य)
		वापस false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->कुंजी = strdup(dom_string_तथ्य(कुंजी));
	अगर (map->कुंजी == शून्य) {
		मुक्त(map);
		वापस false;
	}

	map->list = list;

	slot = imagemap_hash(map->कुंजी);

	map->अग्र = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	वापस true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
अटल व्योम imagemap_freelist(काष्ठा mapentry *list)
{
	काष्ठा mapentry *entry, *पूर्व;

	निश्चित(list != शून्य);

	entry = list;

	जबतक (entry != शून्य) {
		पूर्व = entry;

		nsurl_unref(entry->url);

		अगर (entry->target)
			मुक्त(entry->target);

		अगर (entry->type == IMAGEMAP_POLY) {
			मुक्त(entry->bounds.poly.xcoords);
			मुक्त(entry->bounds.poly.ycoords);
		}

		entry = entry->अग्र;
		मुक्त(पूर्व);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
व्योम imagemap_destroy(html_content *c)
{
	अचिन्हित पूर्णांक i;

	निश्चित(c != शून्य);

	/* no imagemaps -> return */
	अगर (c->imagemaps == शून्य)
		वापस;

	क्रम (i = 0; i != HASH_आकार; i++) {
		काष्ठा imagemap *map, *अग्र;

		map = c->imagemaps[i];
		जबतक (map != शून्य) {
			अग्र = map->अग्र;
			imagemap_freelist(map->list);
			मुक्त(map->कुंजी);
			मुक्त(map);
			map = अग्र;
		}
	}

	मुक्त(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
व्योम imagemap_dump(html_content *c)
{
	अचिन्हित पूर्णांक i;

	पूर्णांक j;

	निश्चित(c != शून्य);

	अगर (c->imagemaps == शून्य)
		वापस;

	क्रम (i = 0; i != HASH_आकार; i++) {
		काष्ठा imagemap *map;
		काष्ठा mapentry *entry;

		map = c->imagemaps[i];
		जबतक (map != शून्य) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->कुंजी);

			क्रम (entry = map->list; entry; entry = entry->अग्र) {
				चयन (entry->type) {
				हाल IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					अवरोध;
				हाल IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					अवरोध;
				हाल IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					अवरोध;
				हाल IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					क्रम (j = 0; j != entry->bounds.poly.अंक;
							j++) {
						ख_लिखो(मानक_त्रुटि, "(%d,%d) ",
							(पूर्णांक)entry->bounds.poly.xcoords[j],
							(पूर्णांक)entry->bounds.poly.ycoords[j]);
					}
					ख_लिखो(मानक_त्रुटि,"\n");
					अवरोध;
				}
			}
			map = map->अग्र;
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
अटल bool
imagemap_addtolist(स्थिर काष्ठा html_content *c,
		   dom_गाँठ *n,
		   nsurl *base_url,
		   काष्ठा mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = शून्य, *target = शून्य, *shape = शून्य;
	dom_string *coords = शून्य;
	काष्ठा mapentry *new_map, *अस्थायी;
	bool ret = true;

	अगर (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		अगर ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			जाओ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	अगर (exc != DOM_NO_ERR || href == शून्य) {
		/* No href="" attribute, skip this element */
		जाओ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	अगर (exc != DOM_NO_ERR) {
		जाओ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	अगर (exc != DOM_NO_ERR) {
		जाओ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	अगर (shape == शून्य)
		shape = dom_string_ref(corestring_dom_rect);

	अगर (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		अगर (exc != DOM_NO_ERR || coords == शून्य) {
			जाओ ok_out;
		}
	}

	new_map = सुस्मृति(1, माप(*new_map));
	अगर (new_map == शून्य) {
		जाओ bad_out;
	}

	अगर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	अन्यथा अगर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	अन्यथा अगर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	अन्यथा अगर (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	अन्यथा
		जाओ bad_out;

	अगर (box_extract_link(c, href, base_url, &new_map->url) == false)
		जाओ bad_out;

	अगर (new_map->url == शून्य) {
		/* non-fatal error -> ignore this */
		जाओ ok_free_map_out;
	}

	अगर (target != शून्य) {
		/* Copy target dom string into the map data */
		new_map->target = दो_स्मृति(dom_string_byte_लंबाई(target) + 1);
		अगर (new_map->target == शून्य)
			जाओ bad_out;

		स_नकल(new_map->target,
		       dom_string_तथ्य(target),
		       dom_string_byte_लंबाई(target));

		new_map->target[dom_string_byte_लंबाई(target)] = 0;
	}

	अगर (new_map->type != IMAGEMAP_DEFAULT) {
		पूर्णांक x, y;
		भग्न *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		अक्षर *val = म_मोहर((अक्षर *)dom_string_तथ्य(coords), ",");
		पूर्णांक अंक = 1;

		चयन (new_map->type) {
		हाल IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			जबतक (val != शून्य && अंक <= 4) {
				चयन (अंक) {
				हाल 1:
					new_map->bounds.rect.x0 = म_से_प(val);
					अवरोध;
				हाल 2:
					new_map->bounds.rect.y0 = म_से_प(val);
					अवरोध;
				हाल 3:
					new_map->bounds.rect.x1 = म_से_प(val);
					अवरोध;
				हाल 4:
					new_map->bounds.rect.y1 = म_से_प(val);
					अवरोध;
				}

				अंक++;
				val = म_मोहर(शून्य, ",");
			}
			अवरोध;
		हाल IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			जबतक (val != शून्य && अंक <= 3) {
				चयन (अंक) {
				हाल 1:
					new_map->bounds.circle.x = म_से_प(val);
					अवरोध;
				हाल 2:
					new_map->bounds.circle.y = म_से_प(val);
					अवरोध;
				हाल 3:
					new_map->bounds.circle.r = म_से_प(val);
					अवरोध;
				}

				अंक++;
				val = म_मोहर(शून्य, ",");
			}
			अवरोध;
		हाल IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = शून्य;
			new_map->bounds.poly.ycoords = शून्य;

			जबतक (val != शून्य) {
				x = म_से_प(val);

				val = म_मोहर(शून्य, ",");
				अगर (val == शून्य)
					अवरोध;

				y = म_से_प(val);

				xcoords = पुनः_स्मृति(new_map->bounds.poly.xcoords,
						अंक * माप(भग्न));
				अगर (xcoords == शून्य) {
					जाओ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = पुनः_स्मृति(new_map->bounds.poly.ycoords,
					अंक * माप(भग्न));
				अगर (ycoords == शून्य) {
					जाओ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[अंक - 1] = x;
				new_map->bounds.poly.ycoords[अंक - 1] = y;

				अंक++;
				val = म_मोहर(शून्य, ",");
			}

			new_map->bounds.poly.अंक = अंक - 1;

			अवरोध;
		शेष:
			अवरोध;
		}
	}

	new_map->अग्र = शून्य;

	अगर (*entry) {
		/* add to END of list */
		क्रम (अस्थायी = (*entry); अस्थायी->अग्र != शून्य; अस्थायी = अस्थायी->अग्र)
			;
		अस्थायी->अग्र = new_map;
	} अन्यथा {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	जाओ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	अगर (new_map != शून्य) {
		अगर (new_map->url != शून्य)
			nsurl_unref(new_map->url);
		अगर (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != शून्य)
			मुक्त(new_map->bounds.poly.ycoords);
		अगर (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != शून्य)
			मुक्त(new_map->bounds.poly.xcoords);
		अगर (new_map->target != शून्य)
			मुक्त(new_map->target);

		मुक्त(new_map);
	}
ok_out:
	अगर (href != शून्य)
		dom_string_unref(href);
	अगर (target != शून्य)
		dom_string_unref(target);
	अगर (shape != शून्य)
		dom_string_unref(shape);
	अगर (coords != शून्य)
		dom_string_unref(coords);

	वापस ret;
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
अटल bool
imagemap_extract_map_entries(dom_गाँठ *गाँठ, html_content *c,
			     काष्ठा mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	अचिन्हित दीर्घ ent;
	uint32_t tag_गिनती;

	exc = dom_element_get_elements_by_tag_name(गाँठ, tname, &nlist);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	exc = dom_nodelist_get_लंबाई(nlist, &tag_गिनती);
	अगर (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		वापस false;
	}

	क्रम (ent = 0; ent < tag_गिनती; ++ent) {
		dom_गाँठ *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		अगर (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			वापस false;
		}
		अगर (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_गाँठ_unref(subnode);
			dom_nodelist_unref(nlist);
			वापस false;
		}
		dom_गाँठ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	वापस true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
अटल bool imagemap_extract_map(dom_गाँठ *गाँठ, html_content *c,
		काष्ठा mapentry **entry)
{
	अगर (imagemap_extract_map_entries(गाँठ, c, entry,
			corestring_dom_area) == false)
		वापस false;
	वापस imagemap_extract_map_entries(गाँठ, c, entry,
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
	अगर (exc != DOM_NO_ERR) {
		वापस NSERROR_DOM;
	}

	exc = dom_nodelist_get_लंबाई(nlist, &maybe_maps);
	अगर (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		जाओ out_nlist;
	}

	क्रम (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_गाँठ *गाँठ;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &गाँठ);
		अगर (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			जाओ out_nlist;
		}

		exc = dom_element_get_attribute(गाँठ, corestring_dom_id,
						&name);
		अगर (exc != DOM_NO_ERR) {
			dom_गाँठ_unref(गाँठ);
			ret = NSERROR_DOM;
			जाओ out_nlist;
		}

		अगर (name == शून्य) {
			exc = dom_element_get_attribute(गाँठ,
							corestring_dom_name,
							&name);
			अगर (exc != DOM_NO_ERR) {
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_DOM;
				जाओ out_nlist;
			}
		}

		अगर (name != शून्य) {
			काष्ठा mapentry *entry = शून्य;
			अगर (imagemap_extract_map(गाँठ, c, &entry) == false) {
				अगर (entry != शून्य) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				जाओ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			अगर ((entry != शून्य) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_गाँठ_unref(गाँठ);
				ret = NSERROR_NOMEM; /** @todo check this */
				जाओ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_गाँठ_unref(गाँठ);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	वापस ret;
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
अटल पूर्णांक
imagemap_point_in_poly(पूर्णांक अंक, भग्न *xpt, भग्न *ypt, अचिन्हित दीर्घ x,
		अचिन्हित दीर्घ y, अचिन्हित दीर्घ click_x,	अचिन्हित दीर्घ click_y)
{
	पूर्णांक i, j, c = 0;

	निश्चित(xpt != शून्य);
	निश्चित(ypt != शून्य);

	क्रम (i = 0, j = अंक - 1; i < अंक; j = i++) {
		अगर ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	वापस c;
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
nsurl *imagemap_get(काष्ठा html_content *c, स्थिर अक्षर *कुंजी,
		अचिन्हित दीर्घ x, अचिन्हित दीर्घ y,
		अचिन्हित दीर्घ click_x, अचिन्हित दीर्घ click_y,
		स्थिर अक्षर **target)
{
	अचिन्हित पूर्णांक slot = 0;
	काष्ठा imagemap *map;
	काष्ठा mapentry *entry;
	अचिन्हित दीर्घ cx, cy;

	निश्चित(c != शून्य);

	अगर (कुंजी == शून्य)
		वापस शून्य;

	अगर (c->imagemaps == शून्य)
		वापस शून्य;

	slot = imagemap_hash(कुंजी);

	क्रम (map = c->imagemaps[slot]; map != शून्य; map = map->अग्र) {
		अगर (map->कुंजी != शून्य && strcasecmp(map->कुंजी, कुंजी) == 0)
			अवरोध;
	}

	अगर (map == शून्य || map->list == शून्य)
		वापस शून्य;

	क्रम (entry = map->list; entry; entry = entry->अग्र) {
		चयन (entry->type) {
		हाल IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			अगर (target)
				*target = entry->target;
			वापस entry->url;
			अवरोध;
		हाल IMAGEMAP_RECT:
			अगर (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				अगर (target)
					*target = entry->target;
				वापस entry->url;
			}
			अवरोध;
		हाल IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			अगर ((cx * cx + cy * cy) <=
				(अचिन्हित दीर्घ) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				अगर (target)
					*target = entry->target;
				वापस entry->url;
			}
			अवरोध;
		हाल IMAGEMAP_POLY:
			अगर (imagemap_point_in_poly(entry->bounds.poly.अंक,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				अगर (target)
					*target = entry->target;
				वापस entry->url;
			}
			अवरोध;
		}
	}

	अगर (target)
		*target = शून्य;

	वापस शून्य;
}
