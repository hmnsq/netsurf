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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>

#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/hlcache.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_construct.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

തരനിർവചനം എണ്ണൽ {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

ഘടന mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	അക്ഷരം *target;			/**< target frame (if any) */
	സംഘം {
		ഘടന {
			പൂർണ്ണസംഖ്യ x;		/**< x coordinate of centre */
			പൂർണ്ണസംഖ്യ y;		/**< y coordinate of center */
			പൂർണ്ണസംഖ്യ r;		/**< radius of circle */
		} circle;
		ഘടന {
			പൂർണ്ണസംഖ്യ x0;		/**< left hand edge */
			പൂർണ്ണസംഖ്യ y0;		/**< top edge */
			പൂർണ്ണസംഖ്യ x1;		/**< right hand edge */
			പൂർണ്ണസംഖ്യ y1;		/**< bottom edge */
		} rect;
		ഘടന {
			പൂർണ്ണസംഖ്യ അക്കം;	/**< number of points */
			ദശാംശം *xcoords;	/**< x coordinates */
			ദശാംശം *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ഘടന mapentry *next;		/**< next entry in list */
};

ഘടന imagemap {
	അക്ഷരം *താക്കോൽ;		/**< key for this entry */
	ഘടന mapentry *list;	/**< pointer to linked list of entries */
	ഘടന imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
സ്ഥിതം bool imagemap_create(html_content *c)
{
	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->imagemaps == ശൂന്യ) {
		c->imagemaps = calloc(HASH_SIZE, വലുപ്പം(ഘടന imagemap *));
		എങ്കിൽ (c->imagemaps == ശൂന്യ) {
			തിരികെ false;
		}
	}

	തിരികെ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ imagemap_hash(സ്ഥിരം അക്ഷരം *താക്കോൽ)
{
	unsigned പൂർണ്ണസംഖ്യ z = 0;

	എങ്കിൽ (താക്കോൽ == 0) തിരികെ 0;

	വേണ്ടി (; *താക്കോൽ != 0; താക്കോൽ++) {
		z += *താക്കോൽ & 0x1f;
	}

	തിരികെ (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
സ്ഥിതം bool
imagemap_add(html_content *c, dom_string *താക്കോൽ, ഘടന mapentry *list)
{
	ഘടന imagemap *map;
	unsigned പൂർണ്ണസംഖ്യ slot;

	പ്രതിജ്ഞാ(c != ശൂന്യ);
	പ്രതിജ്ഞാ(താക്കോൽ != ശൂന്യ);
	പ്രതിജ്ഞാ(list != ശൂന്യ);

	എങ്കിൽ (imagemap_create(c) == false)
		തിരികെ false;

	map = calloc(1, വലുപ്പം(*map));
	എങ്കിൽ (map == ശൂന്യ)
		തിരികെ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->താക്കോൽ = strdup(dom_string_data(താക്കോൽ));
	എങ്കിൽ (map->താക്കോൽ == ശൂന്യ) {
		സ്വതന്ത്രം(map);
		തിരികെ false;
	}

	map->list = list;

	slot = imagemap_hash(map->താക്കോൽ);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	തിരികെ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
സ്ഥിതം ശൂന്യം imagemap_freelist(ഘടന mapentry *list)
{
	ഘടന mapentry *entry, *prev;

	പ്രതിജ്ഞാ(list != ശൂന്യ);

	entry = list;

	ആകുമ്പോൾ (entry != ശൂന്യ) {
		prev = entry;

		nsurl_unref(entry->url);

		എങ്കിൽ (entry->target)
			സ്വതന്ത്രം(entry->target);

		എങ്കിൽ (entry->type == IMAGEMAP_POLY) {
			സ്വതന്ത്രം(entry->bounds.poly.xcoords);
			സ്വതന്ത്രം(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		സ്വതന്ത്രം(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
ശൂന്യം imagemap_destroy(html_content *c)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	/* no imagemaps -> return */
	എങ്കിൽ (c->imagemaps == ശൂന്യ)
		തിരികെ;

	വേണ്ടി (i = 0; i != HASH_SIZE; i++) {
		ഘടന imagemap *map, *next;

		map = c->imagemaps[i];
		ആകുമ്പോൾ (map != ശൂന്യ) {
			next = map->next;
			imagemap_freelist(map->list);
			സ്വതന്ത്രം(map->താക്കോൽ);
			സ്വതന്ത്രം(map);
			map = next;
		}
	}

	സ്വതന്ത്രം(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
ശൂന്യം imagemap_dump(html_content *c)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	പൂർണ്ണസംഖ്യ j;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->imagemaps == ശൂന്യ)
		തിരികെ;

	വേണ്ടി (i = 0; i != HASH_SIZE; i++) {
		ഘടന imagemap *map;
		ഘടന mapentry *entry;

		map = c->imagemaps[i];
		ആകുമ്പോൾ (map != ശൂന്യ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->താക്കോൽ);

			വേണ്ടി (entry = map->list; entry; entry = entry->next) {
				തിരഞ്ഞെടുപ്പ് (entry->type) {
				അവസ്ഥ IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					നിർത്തുക;
				അവസ്ഥ IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					നിർത്തുക;
				അവസ്ഥ IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					നിർത്തുക;
				അവസ്ഥ IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					വേണ്ടി (j = 0; j != entry->bounds.poly.അക്കം;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(പൂർണ്ണസംഖ്യ)entry->bounds.poly.xcoords[j],
							(പൂർണ്ണസംഖ്യ)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					നിർത്തുക;
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
സ്ഥിതം bool
imagemap_addtolist(സ്ഥിരം ഘടന html_content *c,
		   dom_കണ്ണി *n,
		   nsurl *base_url,
		   ഘടന mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ശൂന്യ, *target = ശൂന്യ, *shape = ശൂന്യ;
	dom_string *coords = ശൂന്യ;
	ഘടന mapentry *new_map, *temp;
	bool ret = true;

	എങ്കിൽ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		എങ്കിൽ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			പോകുക ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	എങ്കിൽ (exc != DOM_NO_ERR || href == ശൂന്യ) {
		/* No href="" attribute, skip this element */
		പോകുക ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		പോകുക ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		പോകുക ok_out;
	}

	/* If there's no shape, we default to rectangles */
	എങ്കിൽ (shape == ശൂന്യ)
		shape = dom_string_ref(corestring_dom_rect);

	എങ്കിൽ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		എങ്കിൽ (exc != DOM_NO_ERR || coords == ശൂന്യ) {
			പോകുക ok_out;
		}
	}

	new_map = calloc(1, വലുപ്പം(*new_map));
	എങ്കിൽ (new_map == ശൂന്യ) {
		പോകുക bad_out;
	}

	എങ്കിൽ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	അല്ലെങ്കിൽ
		പോകുക bad_out;

	എങ്കിൽ (box_extract_link(c, href, base_url, &new_map->url) == false)
		പോകുക bad_out;

	എങ്കിൽ (new_map->url == ശൂന്യ) {
		/* non-fatal error -> ignore this */
		പോകുക ok_free_map_out;
	}

	എങ്കിൽ (target != ശൂന്യ) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		എങ്കിൽ (new_map->target == ശൂന്യ)
			പോകുക bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	എങ്കിൽ (new_map->type != IMAGEMAP_DEFAULT) {
		പൂർണ്ണസംഖ്യ x, y;
		ദശാംശം *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		അക്ഷരം *val = strtok((അക്ഷരം *)dom_string_data(coords), ",");
		പൂർണ്ണസംഖ്യ അക്കം = 1;

		തിരഞ്ഞെടുപ്പ് (new_map->type) {
		അവസ്ഥ IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ആകുമ്പോൾ (val != ശൂന്യ && അക്കം <= 4) {
				തിരഞ്ഞെടുപ്പ് (അക്കം) {
				അവസ്ഥ 1:
					new_map->bounds.rect.x0 = atoi(val);
					നിർത്തുക;
				അവസ്ഥ 2:
					new_map->bounds.rect.y0 = atoi(val);
					നിർത്തുക;
				അവസ്ഥ 3:
					new_map->bounds.rect.x1 = atoi(val);
					നിർത്തുക;
				അവസ്ഥ 4:
					new_map->bounds.rect.y1 = atoi(val);
					നിർത്തുക;
				}

				അക്കം++;
				val = strtok(ശൂന്യ, ",");
			}
			നിർത്തുക;
		അവസ്ഥ IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ആകുമ്പോൾ (val != ശൂന്യ && അക്കം <= 3) {
				തിരഞ്ഞെടുപ്പ് (അക്കം) {
				അവസ്ഥ 1:
					new_map->bounds.circle.x = atoi(val);
					നിർത്തുക;
				അവസ്ഥ 2:
					new_map->bounds.circle.y = atoi(val);
					നിർത്തുക;
				അവസ്ഥ 3:
					new_map->bounds.circle.r = atoi(val);
					നിർത്തുക;
				}

				അക്കം++;
				val = strtok(ശൂന്യ, ",");
			}
			നിർത്തുക;
		അവസ്ഥ IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ശൂന്യ;
			new_map->bounds.poly.ycoords = ശൂന്യ;

			ആകുമ്പോൾ (val != ശൂന്യ) {
				x = atoi(val);

				val = strtok(ശൂന്യ, ",");
				എങ്കിൽ (val == ശൂന്യ)
					നിർത്തുക;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						അക്കം * വലുപ്പം(ദശാംശം));
				എങ്കിൽ (xcoords == ശൂന്യ) {
					പോകുക bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					അക്കം * വലുപ്പം(ദശാംശം));
				എങ്കിൽ (ycoords == ശൂന്യ) {
					പോകുക bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[അക്കം - 1] = x;
				new_map->bounds.poly.ycoords[അക്കം - 1] = y;

				അക്കം++;
				val = strtok(ശൂന്യ, ",");
			}

			new_map->bounds.poly.അക്കം = അക്കം - 1;

			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}
	}

	new_map->next = ശൂന്യ;

	എങ്കിൽ (*entry) {
		/* add to END of list */
		വേണ്ടി (temp = (*entry); temp->next != ശൂന്യ; temp = temp->next)
			;
		temp->next = new_map;
	} അല്ലെങ്കിൽ {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	പോകുക ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	എങ്കിൽ (new_map != ശൂന്യ) {
		എങ്കിൽ (new_map->url != ശൂന്യ)
			nsurl_unref(new_map->url);
		എങ്കിൽ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ശൂന്യ)
			സ്വതന്ത്രം(new_map->bounds.poly.ycoords);
		എങ്കിൽ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ശൂന്യ)
			സ്വതന്ത്രം(new_map->bounds.poly.xcoords);
		എങ്കിൽ (new_map->target != ശൂന്യ)
			സ്വതന്ത്രം(new_map->target);

		സ്വതന്ത്രം(new_map);
	}
ok_out:
	എങ്കിൽ (href != ശൂന്യ)
		dom_string_unref(href);
	എങ്കിൽ (target != ശൂന്യ)
		dom_string_unref(target);
	എങ്കിൽ (shape != ശൂന്യ)
		dom_string_unref(shape);
	എങ്കിൽ (coords != ശൂന്യ)
		dom_string_unref(coords);

	തിരികെ ret;
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
സ്ഥിതം bool
imagemap_extract_map_entries(dom_കണ്ണി *കണ്ണി, html_content *c,
			     ഘടന mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_എണ്ണം;

	exc = dom_element_get_elements_by_tag_name(കണ്ണി, tname, &nlist);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_എണ്ണം);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		തിരികെ false;
	}

	വേണ്ടി (ent = 0; ent < tag_എണ്ണം; ++ent) {
		dom_കണ്ണി *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			തിരികെ false;
		}
		എങ്കിൽ (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_കണ്ണി_unref(subnode);
			dom_nodelist_unref(nlist);
			തിരികെ false;
		}
		dom_കണ്ണി_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	തിരികെ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
സ്ഥിതം bool imagemap_extract_map(dom_കണ്ണി *കണ്ണി, html_content *c,
		ഘടന mapentry **entry)
{
	എങ്കിൽ (imagemap_extract_map_entries(കണ്ണി, c, entry,
			corestring_dom_area) == false)
		തിരികെ false;
	തിരികെ imagemap_extract_map_entries(കണ്ണി, c, entry,
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
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		പോകുക out_nlist;
	}

	വേണ്ടി (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_കണ്ണി *കണ്ണി;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &കണ്ണി);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			പോകുക out_nlist;
		}

		exc = dom_element_get_attribute(കണ്ണി, corestring_dom_id,
						&name);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			dom_കണ്ണി_unref(കണ്ണി);
			ret = NSERROR_DOM;
			പോകുക out_nlist;
		}

		എങ്കിൽ (name == ശൂന്യ) {
			exc = dom_element_get_attribute(കണ്ണി,
							corestring_dom_name,
							&name);
			എങ്കിൽ (exc != DOM_NO_ERR) {
				dom_കണ്ണി_unref(കണ്ണി);
				ret = NSERROR_DOM;
				പോകുക out_nlist;
			}
		}

		എങ്കിൽ (name != ശൂന്യ) {
			ഘടന mapentry *entry = ശൂന്യ;
			എങ്കിൽ (imagemap_extract_map(കണ്ണി, c, &entry) == false) {
				എങ്കിൽ (entry != ശൂന്യ) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_കണ്ണി_unref(കണ്ണി);
				ret = NSERROR_NOMEM; /** @todo check this */
				പോകുക out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			എങ്കിൽ ((entry != ശൂന്യ) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_കണ്ണി_unref(കണ്ണി);
				ret = NSERROR_NOMEM; /** @todo check this */
				പോകുക out_nlist;
			}
		}

		dom_string_unref(name);
		dom_കണ്ണി_unref(കണ്ണി);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	തിരികെ ret;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
imagemap_point_in_poly(പൂർണ്ണസംഖ്യ അക്കം, ദശാംശം *xpt, ദശാംശം *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	പൂർണ്ണസംഖ്യ i, j, c = 0;

	പ്രതിജ്ഞാ(xpt != ശൂന്യ);
	പ്രതിജ്ഞാ(ypt != ശൂന്യ);

	വേണ്ടി (i = 0, j = അക്കം - 1; i < അക്കം; j = i++) {
		എങ്കിൽ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	തിരികെ c;
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
nsurl *imagemap_get(ഘടന html_content *c, സ്ഥിരം അക്ഷരം *താക്കോൽ,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		സ്ഥിരം അക്ഷരം **target)
{
	unsigned പൂർണ്ണസംഖ്യ slot = 0;
	ഘടന imagemap *map;
	ഘടന mapentry *entry;
	unsigned long cx, cy;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (താക്കോൽ == ശൂന്യ)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (c->imagemaps == ശൂന്യ)
		തിരികെ ശൂന്യ;

	slot = imagemap_hash(താക്കോൽ);

	വേണ്ടി (map = c->imagemaps[slot]; map != ശൂന്യ; map = map->next) {
		എങ്കിൽ (map->താക്കോൽ != ശൂന്യ && strcasecmp(map->താക്കോൽ, താക്കോൽ) == 0)
			നിർത്തുക;
	}

	എങ്കിൽ (map == ശൂന്യ || map->list == ശൂന്യ)
		തിരികെ ശൂന്യ;

	വേണ്ടി (entry = map->list; entry; entry = entry->next) {
		തിരഞ്ഞെടുപ്പ് (entry->type) {
		അവസ്ഥ IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			എങ്കിൽ (target)
				*target = entry->target;
			തിരികെ entry->url;
			നിർത്തുക;
		അവസ്ഥ IMAGEMAP_RECT:
			എങ്കിൽ (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				എങ്കിൽ (target)
					*target = entry->target;
				തിരികെ entry->url;
			}
			നിർത്തുക;
		അവസ്ഥ IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			എങ്കിൽ ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				എങ്കിൽ (target)
					*target = entry->target;
				തിരികെ entry->url;
			}
			നിർത്തുക;
		അവസ്ഥ IMAGEMAP_POLY:
			എങ്കിൽ (imagemap_point_in_poly(entry->bounds.poly.അക്കം,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				എങ്കിൽ (target)
					*target = entry->target;
				തിരികെ entry->url;
			}
			നിർത്തുക;
		}
	}

	എങ്കിൽ (target)
		*target = ശൂന്യ;

	തിരികെ ശൂന്യ;
}
