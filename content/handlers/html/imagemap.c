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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>

#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_construct.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

ସଂରଚନା mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	ଅକ୍ଷର *target;			/**< target frame (if any) */
	ସଂଘ {
		ସଂରଚନା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;		/**< x coordinate of centre */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା y;		/**< y coordinate of center */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;		/**< radius of circle */
		} circle;
		ସଂରଚନା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0;		/**< left hand edge */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା y0;		/**< top edge */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x1;		/**< right hand edge */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା y1;		/**< bottom edge */
		} rect;
		ସଂରଚନା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ;	/**< number of points */
			ଦଶମିକ *xcoords;	/**< x coordinates */
			ଦଶମିକ *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ସଂରଚନା mapentry *next;		/**< next entry in list */
};

ସଂରଚନା imagemap {
	ଅକ୍ଷର *ଚାବି;		/**< key for this entry */
	ସଂରଚନା mapentry *list;	/**< pointer to linked list of entries */
	ସଂରଚନା imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ସ୍ଥିର bool imagemap_create(html_content *c)
{
	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->imagemaps == ଖାଲି) {
		c->imagemaps = calloc(HASH_SIZE, ଆକାର(ସଂରଚନା imagemap *));
		ଯଦି (c->imagemaps == ଖାଲି) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା imagemap_hash(ଅଚଳ ଅକ୍ଷର *ଚାବି)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା z = 0;

	ଯଦି (ଚାବି == 0) ଫେରନ୍ତୁ 0;

	ପାଇଁ (; *ଚାବି != 0; ଚାବି++) {
		z += *ଚାବି & 0x1f;
	}

	ଫେରନ୍ତୁ (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ସ୍ଥିର bool
imagemap_add(html_content *c, dom_string *ଚାବି, ସଂରଚନା mapentry *list)
{
	ସଂରଚନା imagemap *map;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା slot;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(ଚାବି != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(list != ଖାଲି);

	ଯଦି (imagemap_create(c) == false)
		ଫେରନ୍ତୁ false;

	map = calloc(1, ଆକାର(*map));
	ଯଦି (map == ଖାଲି)
		ଫେରନ୍ତୁ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ଚାବି = strdup(dom_string_data(ଚାବି));
	ଯଦି (map->ଚାବି == ଖାଲି) {
		ମୁକ୍ତ(map);
		ଫେରନ୍ତୁ false;
	}

	map->list = list;

	slot = imagemap_hash(map->ଚାବି);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ଫେରନ୍ତୁ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ସ୍ଥିର ଶୂନ୍ୟ imagemap_freelist(ସଂରଚନା mapentry *list)
{
	ସଂରଚନା mapentry *entry, *prev;

	ପ୍ରତିଜ୍ଞା(list != ଖାଲି);

	entry = list;

	ଯେତେବେଳେ (entry != ଖାଲି) {
		prev = entry;

		nsurl_unref(entry->url);

		ଯଦି (entry->target)
			ମୁକ୍ତ(entry->target);

		ଯଦି (entry->type == IMAGEMAP_POLY) {
			ମୁକ୍ତ(entry->bounds.poly.xcoords);
			ମୁକ୍ତ(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		ମୁକ୍ତ(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
ଶୂନ୍ୟ imagemap_destroy(html_content *c)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	/* no imagemaps -> return */
	ଯଦି (c->imagemaps == ଖାଲି)
		ଫେରନ୍ତୁ;

	ପାଇଁ (i = 0; i != HASH_SIZE; i++) {
		ସଂରଚନା imagemap *map, *next;

		map = c->imagemaps[i];
		ଯେତେବେଳେ (map != ଖାଲି) {
			next = map->next;
			imagemap_freelist(map->list);
			ମୁକ୍ତ(map->ଚାବି);
			ମୁକ୍ତ(map);
			map = next;
		}
	}

	ମୁକ୍ତ(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
ଶୂନ୍ୟ imagemap_dump(html_content *c)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->imagemaps == ଖାଲି)
		ଫେରନ୍ତୁ;

	ପାଇଁ (i = 0; i != HASH_SIZE; i++) {
		ସଂରଚନା imagemap *map;
		ସଂରଚନା mapentry *entry;

		map = c->imagemaps[i];
		ଯେତେବେଳେ (map != ଖାଲି) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ଚାବି);

			ପାଇଁ (entry = map->list; entry; entry = entry->next) {
				ବାଛ (entry->type) {
				ସ୍ଥିତି IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					ଭାଙ୍ଗ;
				ସ୍ଥିତି IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					ପାଇଁ (j = 0; j != entry->bounds.poly.ଅଙ୍କ;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)entry->bounds.poly.xcoords[j],
							(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ଭାଙ୍ଗ;
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
ସ୍ଥିର bool
imagemap_addtolist(ଅଚଳ ସଂରଚନା html_content *c,
		   dom_ଗଣ୍ଠି *n,
		   nsurl *base_url,
		   ସଂରଚନା mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ଖାଲି, *target = ଖାଲି, *shape = ଖାଲି;
	dom_string *coords = ଖାଲି;
	ସଂରଚନା mapentry *new_map, *temp;
	bool ret = true;

	ଯଦି (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ଯଦି ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			ଯାଅ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ଯଦି (exc != DOM_NO_ERR || href == ଖାଲି) {
		/* No href="" attribute, skip this element */
		ଯାଅ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	ଯଦି (exc != DOM_NO_ERR) {
		ଯାଅ ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ଯଦି (exc != DOM_NO_ERR) {
		ଯାଅ ok_out;
	}

	/* If there's no shape, we default to rectangles */
	ଯଦି (shape == ଖାଲି)
		shape = dom_string_ref(corestring_dom_rect);

	ଯଦି (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ଯଦି (exc != DOM_NO_ERR || coords == ଖାଲି) {
			ଯାଅ ok_out;
		}
	}

	new_map = calloc(1, ଆକାର(*new_map));
	ଯଦି (new_map == ଖାଲି) {
		ଯାଅ bad_out;
	}

	ଯଦି (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ଅନ୍ୟଥା
		ଯାଅ bad_out;

	ଯଦି (box_extract_link(c, href, base_url, &new_map->url) == false)
		ଯାଅ bad_out;

	ଯଦି (new_map->url == ଖାଲି) {
		/* non-fatal error -> ignore this */
		ଯାଅ ok_free_map_out;
	}

	ଯଦି (target != ଖାଲି) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		ଯଦି (new_map->target == ଖାଲି)
			ଯାଅ bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	ଯଦି (new_map->type != IMAGEMAP_DEFAULT) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;
		ଦଶମିକ *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		ଅକ୍ଷର *val = strtok((ଅକ୍ଷର *)dom_string_data(coords), ",");
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ = 1;

		ବାଛ (new_map->type) {
		ସ୍ଥିତି IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ଯେତେବେଳେ (val != ଖାଲି && ଅଙ୍କ <= 4) {
				ବାଛ (ଅଙ୍କ) {
				ସ୍ଥିତି 1:
					new_map->bounds.rect.x0 = atoi(val);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 2:
					new_map->bounds.rect.y0 = atoi(val);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 3:
					new_map->bounds.rect.x1 = atoi(val);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 4:
					new_map->bounds.rect.y1 = atoi(val);
					ଭାଙ୍ଗ;
				}

				ଅଙ୍କ++;
				val = strtok(ଖାଲି, ",");
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ଯେତେବେଳେ (val != ଖାଲି && ଅଙ୍କ <= 3) {
				ବାଛ (ଅଙ୍କ) {
				ସ୍ଥିତି 1:
					new_map->bounds.circle.x = atoi(val);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 2:
					new_map->bounds.circle.y = atoi(val);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 3:
					new_map->bounds.circle.r = atoi(val);
					ଭାଙ୍ଗ;
				}

				ଅଙ୍କ++;
				val = strtok(ଖାଲି, ",");
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ଖାଲି;
			new_map->bounds.poly.ycoords = ଖାଲି;

			ଯେତେବେଳେ (val != ଖାଲି) {
				x = atoi(val);

				val = strtok(ଖାଲି, ",");
				ଯଦି (val == ଖାଲି)
					ଭାଙ୍ଗ;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						ଅଙ୍କ * ଆକାର(ଦଶମିକ));
				ଯଦି (xcoords == ଖାଲି) {
					ଯାଅ bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					ଅଙ୍କ * ଆକାର(ଦଶମିକ));
				ଯଦି (ycoords == ଖାଲି) {
					ଯାଅ bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[ଅଙ୍କ - 1] = x;
				new_map->bounds.poly.ycoords[ଅଙ୍କ - 1] = y;

				ଅଙ୍କ++;
				val = strtok(ଖାଲି, ",");
			}

			new_map->bounds.poly.ଅଙ୍କ = ଅଙ୍କ - 1;

			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}
	}

	new_map->next = ଖାଲି;

	ଯଦି (*entry) {
		/* add to END of list */
		ପାଇଁ (temp = (*entry); temp->next != ଖାଲି; temp = temp->next)
			;
		temp->next = new_map;
	} ଅନ୍ୟଥା {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	ଯାଅ ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	ଯଦି (new_map != ଖାଲି) {
		ଯଦି (new_map->url != ଖାଲି)
			nsurl_unref(new_map->url);
		ଯଦି (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ଖାଲି)
			ମୁକ୍ତ(new_map->bounds.poly.ycoords);
		ଯଦି (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ଖାଲି)
			ମୁକ୍ତ(new_map->bounds.poly.xcoords);
		ଯଦି (new_map->target != ଖାଲି)
			ମୁକ୍ତ(new_map->target);

		ମୁକ୍ତ(new_map);
	}
ok_out:
	ଯଦି (href != ଖାଲି)
		dom_string_unref(href);
	ଯଦି (target != ଖାଲି)
		dom_string_unref(target);
	ଯଦି (shape != ଖାଲି)
		dom_string_unref(shape);
	ଯଦି (coords != ଖାଲି)
		dom_string_unref(coords);

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର bool
imagemap_extract_map_entries(dom_ଗଣ୍ଠି *ଗଣ୍ଠି, html_content *c,
			     ସଂରଚନା mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_ଗଣନା;

	exc = dom_element_get_elements_by_tag_name(ଗଣ୍ଠି, tname, &nlist);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_ଗଣନା);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ଫେରନ୍ତୁ false;
	}

	ପାଇଁ (ent = 0; ent < tag_ଗଣନା; ++ent) {
		dom_ଗଣ୍ଠି *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		ଯଦି (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ଫେରନ୍ତୁ false;
		}
		ଯଦି (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_ଗଣ୍ଠି_unref(subnode);
			dom_nodelist_unref(nlist);
			ଫେରନ୍ତୁ false;
		}
		dom_ଗଣ୍ଠି_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ଫେରନ୍ତୁ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ସ୍ଥିର bool imagemap_extract_map(dom_ଗଣ୍ଠି *ଗଣ୍ଠି, html_content *c,
		ସଂରଚନା mapentry **entry)
{
	ଯଦି (imagemap_extract_map_entries(ଗଣ୍ଠି, c, entry,
			corestring_dom_area) == false)
		ଫେରନ୍ତୁ false;
	ଫେରନ୍ତୁ imagemap_extract_map_entries(ଗଣ୍ଠି, c, entry,
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
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	ଯଦି (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		ଯାଅ out_nlist;
	}

	ପାଇଁ (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &ଗଣ୍ଠି);
		ଯଦି (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			ଯାଅ out_nlist;
		}

		exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_id,
						&name);
		ଯଦି (exc != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
			ret = NSERROR_DOM;
			ଯାଅ out_nlist;
		}

		ଯଦି (name == ଖାଲି) {
			exc = dom_element_get_attribute(ଗଣ୍ଠି,
							corestring_dom_name,
							&name);
			ଯଦି (exc != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
				ret = NSERROR_DOM;
				ଯାଅ out_nlist;
			}
		}

		ଯଦି (name != ଖାଲି) {
			ସଂରଚନା mapentry *entry = ଖାଲି;
			ଯଦି (imagemap_extract_map(ଗଣ୍ଠି, c, &entry) == false) {
				ଯଦି (entry != ଖାଲି) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
				ret = NSERROR_NOMEM; /** @todo check this */
				ଯାଅ out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ଯଦି ((entry != ଖାଲି) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
				ret = NSERROR_NOMEM; /** @todo check this */
				ଯାଅ out_nlist;
			}
		}

		dom_string_unref(name);
		dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
imagemap_point_in_poly(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ, ଦଶମିକ *xpt, ଦଶମିକ *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, c = 0;

	ପ୍ରତିଜ୍ଞା(xpt != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(ypt != ଖାଲି);

	ପାଇଁ (i = 0, j = ଅଙ୍କ - 1; i < ଅଙ୍କ; j = i++) {
		ଯଦି ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ଫେରନ୍ତୁ c;
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
nsurl *imagemap_get(ସଂରଚନା html_content *c, ଅଚଳ ଅକ୍ଷର *ଚାବି,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ଅଚଳ ଅକ୍ଷର **target)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା slot = 0;
	ସଂରଚନା imagemap *map;
	ସଂରଚନା mapentry *entry;
	unsigned long cx, cy;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (ଚାବି == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (c->imagemaps == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	slot = imagemap_hash(ଚାବି);

	ପାଇଁ (map = c->imagemaps[slot]; map != ଖାଲି; map = map->next) {
		ଯଦି (map->ଚାବି != ଖାଲି && strcasecmp(map->ଚାବି, ଚାବି) == 0)
			ଭାଙ୍ଗ;
	}

	ଯଦି (map == ଖାଲି || map->list == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ପାଇଁ (entry = map->list; entry; entry = entry->next) {
		ବାଛ (entry->type) {
		ସ୍ଥିତି IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ଯଦି (target)
				*target = entry->target;
			ଫେରନ୍ତୁ entry->url;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IMAGEMAP_RECT:
			ଯଦି (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				ଯଦି (target)
					*target = entry->target;
				ଫେରନ୍ତୁ entry->url;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			ଯଦି ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				ଯଦି (target)
					*target = entry->target;
				ଫେରନ୍ତୁ entry->url;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IMAGEMAP_POLY:
			ଯଦି (imagemap_point_in_poly(entry->bounds.poly.ଅଙ୍କ,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ଯଦି (target)
					*target = entry->target;
				ଫେରନ୍ତୁ entry->url;
			}
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (target)
		*target = ଖାଲି;

	ଫେରନ୍ତୁ ଖାଲି;
}
