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

#परिभाषय HASH_आकार 31 /* fixed size hash table */

प्रकारनिर्देश गणना {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_प्रविष्टिः_type;

संरचना mapentry {
	imagemap_प्रविष्टिः_type type;	/**< type of shape */
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
			पूर्णाङ्क अङ्कः;	/**< number of points */
			भिन्न *xcoords;	/**< x coordinates */
			भिन्न *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	संरचना mapentry *अग्र;		/**< next entry in list */
};

संरचना imagemap {
	वर्ण *कुञ्चिका;		/**< key for this entry */
	संरचना mapentry *list;	/**< pointer to linked list of entries */
	संरचना imagemap *अग्र;	/**< next entry in this hash chain */
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
		c->imagemaps = शून्यस्मृतिं_देहि(HASH_आकार, परिमाण(संरचना imagemap *));
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
स्थिर अचिह्नित पूर्णाङ्क imagemap_hash(ध्रुव वर्ण *कुञ्चिका)
{
	अचिह्नित पूर्णाङ्क z = 0;

	यदि (कुञ्चिका == 0) प्रत्यागम 0;

	प्रति (; *कुञ्चिका != 0; कुञ्चिका++) {
		z += *कुञ्चिका & 0x1f;
	}

	प्रत्यागम (z % (HASH_आकार - 1)) + 1;
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
imagemap_सङ्कलय(html_content *c, dom_string *कुञ्चिका, संरचना mapentry *list)
{
	संरचना imagemap *map;
	अचिह्नित पूर्णाङ्क slot;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(कुञ्चिका != रिक्त);
	प्रतिज्ञा(list != रिक्त);

	यदि (imagemap_create(c) == false)
		प्रत्यागम false;

	map = शून्यस्मृतिं_देहि(1, परिमाण(*map));
	यदि (map == रिक्त)
		प्रत्यागम false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->कुञ्चिका = strdup(dom_string_दत्त(कुञ्चिका));
	यदि (map->कुञ्चिका == रिक्त) {
		मुक्त(map);
		प्रत्यागम false;
	}

	map->list = list;

	slot = imagemap_hash(map->कुञ्चिका);

	map->अग्र = c->imagemaps[slot];
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
	संरचना mapentry *प्रविष्टिः, *पूर्व;

	प्रतिज्ञा(list != रिक्त);

	प्रविष्टिः = list;

	यावत् (प्रविष्टिः != रिक्त) {
		पूर्व = प्रविष्टिः;

		nsurl_unref(प्रविष्टिः->url);

		यदि (प्रविष्टिः->target)
			मुक्त(प्रविष्टिः->target);

		यदि (प्रविष्टिः->type == IMAGEMAP_POLY) {
			मुक्त(प्रविष्टिः->bounds.poly.xcoords);
			मुक्त(प्रविष्टिः->bounds.poly.ycoords);
		}

		प्रविष्टिः = प्रविष्टिः->अग्र;
		मुक्त(पूर्व);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
शून्य imagemap_destroy(html_content *c)
{
	अचिह्नित पूर्णाङ्क i;

	प्रतिज्ञा(c != रिक्त);

	/* no imagemaps -> return */
	यदि (c->imagemaps == रिक्त)
		प्रत्यागम;

	प्रति (i = 0; i != HASH_आकार; i++) {
		संरचना imagemap *map, *अग्र;

		map = c->imagemaps[i];
		यावत् (map != रिक्त) {
			अग्र = map->अग्र;
			imagemap_freelist(map->list);
			मुक्त(map->कुञ्चिका);
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
शून्य imagemap_dump(html_content *c)
{
	अचिह्नित पूर्णाङ्क i;

	पूर्णाङ्क j;

	प्रतिज्ञा(c != रिक्त);

	यदि (c->imagemaps == रिक्त)
		प्रत्यागम;

	प्रति (i = 0; i != HASH_आकार; i++) {
		संरचना imagemap *map;
		संरचना mapentry *प्रविष्टिः;

		map = c->imagemaps[i];
		यावत् (map != रिक्त) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->कुञ्चिका);

			प्रति (प्रविष्टिः = map->list; प्रविष्टिः; प्रविष्टिः = प्रविष्टिः->अग्र) {
				विकल्प (प्रविष्टिः->type) {
				पक्ष IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(प्रविष्टिः->url));
					भङ्ग;
				पक्ष IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(प्रविष्टिः->url),
					      प्रविष्टिः->bounds.rect.x0,
					      प्रविष्टिः->bounds.rect.y0,
					      प्रविष्टिः->bounds.rect.x1,
					      प्रविष्टिः->bounds.rect.y1);
					भङ्ग;
				पक्ष IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(प्रविष्टिः->url),
					      प्रविष्टिः->bounds.circle.x,
					      प्रविष्टिः->bounds.circle.y,
					      प्रविष्टिः->bounds.circle.r);
					भङ्ग;
				पक्ष IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(प्रविष्टिः->url));
					प्रति (j = 0; j != प्रविष्टिः->bounds.poly.अङ्कः;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(पूर्णाङ्क)प्रविष्टिः->bounds.poly.xcoords[j],
							(पूर्णाङ्क)प्रविष्टिः->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					भङ्ग;
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
स्थिर bool
imagemap_addtolist(ध्रुव संरचना html_content *c,
		   dom_ग्रन्थिः *n,
		   nsurl *आधारः_url,
		   संरचना mapentry **प्रविष्टिः,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = रिक्त, *target = रिक्त, *shape = रिक्त;
	dom_string *coords = रिक्त;
	संरचना mapentry *new_map, *अस्थायी;
	bool ret = true;

	यदि (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		यदि ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			गच्छ ok_बहिर्गमनम्;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	यदि (exc != DOM_NO_ERR || href == रिक्त) {
		/* No href="" attribute, skip this element */
		गच्छ ok_बहिर्गमनम्;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	यदि (exc != DOM_NO_ERR) {
		गच्छ ok_बहिर्गमनम्;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	यदि (exc != DOM_NO_ERR) {
		गच्छ ok_बहिर्गमनम्;
	}

	/* If there's no shape, we default to rectangles */
	यदि (shape == रिक्त)
		shape = dom_string_ref(corestring_dom_rect);

	यदि (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		यदि (exc != DOM_NO_ERR || coords == रिक्त) {
			गच्छ ok_बहिर्गमनम्;
		}
	}

	new_map = शून्यस्मृतिं_देहि(1, परिमाण(*new_map));
	यदि (new_map == रिक्त) {
		गच्छ bad_बहिर्गमनम्;
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
		गच्छ bad_बहिर्गमनम्;

	यदि (box_extract_link(c, href, आधारः_url, &new_map->url) == false)
		गच्छ bad_बहिर्गमनम्;

	यदि (new_map->url == रिक्त) {
		/* non-fatal error -> ignore this */
		गच्छ ok_free_map_बहिर्गमनम्;
	}

	यदि (target != रिक्त) {
		/* Copy target dom string into the map data */
		new_map->target = स्मृतिं_देहि(dom_string_byte_दीर्घता(target) + 1);
		यदि (new_map->target == रिक्त)
			गच्छ bad_बहिर्गमनम्;

		स्मृतिं_प्रतिलिख(new_map->target,
		       dom_string_दत्त(target),
		       dom_string_byte_दीर्घता(target));

		new_map->target[dom_string_byte_दीर्घता(target)] = 0;
	}

	यदि (new_map->type != IMAGEMAP_DEFAULT) {
		पूर्णाङ्क x, y;
		भिन्न *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		वर्ण *val = strtok((वर्ण *)dom_string_दत्त(coords), ",");
		पूर्णाङ्क अङ्कः = 1;

		विकल्प (new_map->type) {
		पक्ष IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			यावत् (val != रिक्त && अङ्कः <= 4) {
				विकल्प (अङ्कः) {
				पक्ष 1:
					new_map->bounds.rect.x0 = atoi(val);
					भङ्ग;
				पक्ष 2:
					new_map->bounds.rect.y0 = atoi(val);
					भङ्ग;
				पक्ष 3:
					new_map->bounds.rect.x1 = atoi(val);
					भङ्ग;
				पक्ष 4:
					new_map->bounds.rect.y1 = atoi(val);
					भङ्ग;
				}

				अङ्कः++;
				val = strtok(रिक्त, ",");
			}
			भङ्ग;
		पक्ष IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			यावत् (val != रिक्त && अङ्कः <= 3) {
				विकल्प (अङ्कः) {
				पक्ष 1:
					new_map->bounds.circle.x = atoi(val);
					भङ्ग;
				पक्ष 2:
					new_map->bounds.circle.y = atoi(val);
					भङ्ग;
				पक्ष 3:
					new_map->bounds.circle.r = atoi(val);
					भङ्ग;
				}

				अङ्कः++;
				val = strtok(रिक्त, ",");
			}
			भङ्ग;
		पक्ष IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = रिक्त;
			new_map->bounds.poly.ycoords = रिक्त;

			यावत् (val != रिक्त) {
				x = atoi(val);

				val = strtok(रिक्त, ",");
				यदि (val == रिक्त)
					भङ्ग;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						अङ्कः * परिमाण(भिन्न));
				यदि (xcoords == रिक्त) {
					गच्छ bad_बहिर्गमनम्;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					अङ्कः * परिमाण(भिन्न));
				यदि (ycoords == रिक्त) {
					गच्छ bad_बहिर्गमनम्;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[अङ्कः - 1] = x;
				new_map->bounds.poly.ycoords[अङ्कः - 1] = y;

				अङ्कः++;
				val = strtok(रिक्त, ",");
			}

			new_map->bounds.poly.अङ्कः = अङ्कः - 1;

			भङ्ग;
		स्वभाव:
			भङ्ग;
		}
	}

	new_map->अग्र = रिक्त;

	यदि (*प्रविष्टिः) {
		/* add to END of list */
		प्रति (अस्थायी = (*प्रविष्टिः); अस्थायी->अग्र != रिक्त; अस्थायी = अस्थायी->अग्र)
			;
		अस्थायी->अग्र = new_map;
	} अन्यथा {
		(*प्रविष्टिः) = new_map;
	}

	/* All good, linked in, let's clean up */
	गच्छ ok_बहिर्गमनम्;

bad_बहिर्गमनम्:
	ret = false;
ok_free_map_बहिर्गमनम्:
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
ok_बहिर्गमनम्:
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
imagemap_extract_map_entries(dom_ग्रन्थिः *ग्रन्थिः, html_content *c,
			     संरचना mapentry **प्रविष्टिः, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	अचिह्नित दीर्घ ent;
	uint32_t चिह्नकम्_गणनम्;

	exc = dom_element_get_elements_by_चिह्नकम्_name(ग्रन्थिः, tname, &nlist);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	exc = dom_nodelist_get_दीर्घता(nlist, &चिह्नकम्_गणनम्);
	यदि (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		प्रत्यागम false;
	}

	प्रति (ent = 0; ent < चिह्नकम्_गणनम्; ++ent) {
		dom_ग्रन्थिः *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		यदि (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			प्रत्यागम false;
		}
		यदि (imagemap_addtolist(c, subnode, c->आधारः_url,
				       प्रविष्टिः, tname) == false) {
			dom_ग्रन्थिः_unref(subnode);
			dom_nodelist_unref(nlist);
			प्रत्यागम false;
		}
		dom_ग्रन्थिः_unref(subnode);
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
स्थिर bool imagemap_extract_map(dom_ग्रन्थिः *ग्रन्थिः, html_content *c,
		संरचना mapentry **प्रविष्टिः)
{
	यदि (imagemap_extract_map_entries(ग्रन्थिः, c, प्रविष्टिः,
			corestring_dom_area) == false)
		प्रत्यागम false;
	प्रत्यागम imagemap_extract_map_entries(ग्रन्थिः, c, प्रविष्टिः,
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
	अचिह्नित दीर्घ mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_चिह्नकम्_name(c->document,
						    corestring_dom_map,
						    &nlist);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम NSERROR_DOM;
	}

	exc = dom_nodelist_get_दीर्घता(nlist, &maybe_maps);
	यदि (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		गच्छ बहिर्गमनम्_nlist;
	}

	प्रति (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ग्रन्थिः *ग्रन्थिः;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &ग्रन्थिः);
		यदि (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			गच्छ बहिर्गमनम्_nlist;
		}

		exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_id,
						&name);
		यदि (exc != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(ग्रन्थिः);
			ret = NSERROR_DOM;
			गच्छ बहिर्गमनम्_nlist;
		}

		यदि (name == रिक्त) {
			exc = dom_element_get_attribute(ग्रन्थिः,
							corestring_dom_name,
							&name);
			यदि (exc != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(ग्रन्थिः);
				ret = NSERROR_DOM;
				गच्छ बहिर्गमनम्_nlist;
			}
		}

		यदि (name != रिक्त) {
			संरचना mapentry *प्रविष्टिः = रिक्त;
			यदि (imagemap_extract_map(ग्रन्थिः, c, &प्रविष्टिः) == false) {
				यदि (प्रविष्टिः != रिक्त) {
					imagemap_freelist(प्रविष्टिः);
				}

				dom_string_unref(name);
				dom_ग्रन्थिः_unref(ग्रन्थिः);
				ret = NSERROR_NOMEM; /** @todo check this */
				गच्छ बहिर्गमनम्_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			यदि ((प्रविष्टिः != रिक्त) &&
			    (imagemap_सङ्कलय(c, name, प्रविष्टिः) == false)) {
				imagemap_freelist(प्रविष्टिः);

				dom_string_unref(name);
				dom_ग्रन्थिः_unref(ग्रन्थिः);
				ret = NSERROR_NOMEM; /** @todo check this */
				गच्छ बहिर्गमनम्_nlist;
			}
		}

		dom_string_unref(name);
		dom_ग्रन्थिः_unref(ग्रन्थिः);
	}

बहिर्गमनम्_nlist:

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
imagemap_point_अन्तर्_poly(पूर्णाङ्क अङ्कः, भिन्न *xpt, भिन्न *ypt, अचिह्नित दीर्घ x,
		अचिह्नित दीर्घ y, अचिह्नित दीर्घ click_x,	अचिह्नित दीर्घ click_y)
{
	पूर्णाङ्क i, j, c = 0;

	प्रतिज्ञा(xpt != रिक्त);
	प्रतिज्ञा(ypt != रिक्त);

	प्रति (i = 0, j = अङ्कः - 1; i < अङ्कः; j = i++) {
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
nsurl *imagemap_get(संरचना html_content *c, ध्रुव वर्ण *कुञ्चिका,
		अचिह्नित दीर्घ x, अचिह्नित दीर्घ y,
		अचिह्नित दीर्घ click_x, अचिह्नित दीर्घ click_y,
		ध्रुव वर्ण **target)
{
	अचिह्नित पूर्णाङ्क slot = 0;
	संरचना imagemap *map;
	संरचना mapentry *प्रविष्टिः;
	अचिह्नित दीर्घ cx, cy;

	प्रतिज्ञा(c != रिक्त);

	यदि (कुञ्चिका == रिक्त)
		प्रत्यागम रिक्त;

	यदि (c->imagemaps == रिक्त)
		प्रत्यागम रिक्त;

	slot = imagemap_hash(कुञ्चिका);

	प्रति (map = c->imagemaps[slot]; map != रिक्त; map = map->अग्र) {
		यदि (map->कुञ्चिका != रिक्त && strcasecmp(map->कुञ्चिका, कुञ्चिका) == 0)
			भङ्ग;
	}

	यदि (map == रिक्त || map->list == रिक्त)
		प्रत्यागम रिक्त;

	प्रति (प्रविष्टिः = map->list; प्रविष्टिः; प्रविष्टिः = प्रविष्टिः->अग्र) {
		विकल्प (प्रविष्टिः->type) {
		पक्ष IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			यदि (target)
				*target = प्रविष्टिः->target;
			प्रत्यागम प्रविष्टिः->url;
			भङ्ग;
		पक्ष IMAGEMAP_RECT:
			यदि (click_x >= x + प्रविष्टिः->bounds.rect.x0 &&
				    click_x <= x + प्रविष्टिः->bounds.rect.x1 &&
				    click_y >= y + प्रविष्टिः->bounds.rect.y0 &&
				    click_y <= y + प्रविष्टिः->bounds.rect.y1) {
				यदि (target)
					*target = प्रविष्टिः->target;
				प्रत्यागम प्रविष्टिः->url;
			}
			भङ्ग;
		पक्ष IMAGEMAP_CIRCLE:
			cx = x + प्रविष्टिः->bounds.circle.x - click_x;
			cy = y + प्रविष्टिः->bounds.circle.y - click_y;
			यदि ((cx * cx + cy * cy) <=
				(अचिह्नित दीर्घ) (प्रविष्टिः->bounds.circle.r *
					प्रविष्टिः->bounds.circle.r)) {
				यदि (target)
					*target = प्रविष्टिः->target;
				प्रत्यागम प्रविष्टिः->url;
			}
			भङ्ग;
		पक्ष IMAGEMAP_POLY:
			यदि (imagemap_point_अन्तर्_poly(प्रविष्टिः->bounds.poly.अङ्कः,
					प्रविष्टिः->bounds.poly.xcoords,
					प्रविष्टिः->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				यदि (target)
					*target = प्रविष्टिः->target;
				प्रत्यागम प्रविष्टिः->url;
			}
			भङ्ग;
		}
	}

	यदि (target)
		*target = रिक्त;

	प्रत्यागम रिक्त;
}
