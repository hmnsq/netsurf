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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include <dom/dom.h>

#include "utils/log.h"
#include "utils/corestrings.h"
#include "content/content_protected.h"
#include "content/hlcache.h"

#include "html/box.h"
#include "html/box_construct.h"
#include "html/private.h"
#include "html/imagemap.h"

#define HASH_SIZE 31 /* fixed size hash table */

typedef ga_nanaa {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_entry_type;

sa_nracanaa mapentry {
	imagemap_entry_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	var_na *target;			/**< target frame (if any) */
	sa_ngha {
		sa_nracanaa {
			ຈຳນວນເຕັມ x;		/**< x coordinate of centre */
			ຈຳນວນເຕັມ y;		/**< y coordinate of center */
			ຈຳນວນເຕັມ r;		/**< radius of circle */
		} circle;
		sa_nracanaa {
			ຈຳນວນເຕັມ x0;		/**< left hand edge */
			ຈຳນວນເຕັມ y0;		/**< top edge */
			ຈຳນວນເຕັມ x1;		/**< right hand edge */
			ຈຳນວນເຕັມ y1;		/**< bottom edge */
		} rect;
		sa_nracanaa {
			ຈຳນວນເຕັມ num;	/**< number of points */
			float *xcoords;	/**< x coordinates */
			float *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	sa_nracanaa mapentry *next;		/**< next entry in list */
};

sa_nracanaa imagemap {
	var_na *key;		/**< key for this entry */
	sa_nracanaa mapentry *list;	/**< pointer to linked list of entries */
	sa_nracanaa imagemap *next;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
static bool imagemap_create(html_content *c)
{
	assert(c != NULL);

	ຖ້າ (c->imagemaps == NULL) {
		c->imagemaps = calloc(HASH_SIZE, parimaa_na(sa_nracanaa imagemap *));
		ຖ້າ (c->imagemaps == NULL) {
			ສົ່ງຄືນ false;
		}
	}

	ສົ່ງຄືນ true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
static unsigned ຈຳນວນເຕັມ imagemap_hash(const var_na *key)
{
	unsigned ຈຳນວນເຕັມ z = 0;

	ຖ້າ (key == 0) ສົ່ງຄືນ 0;

	ສຳລັບ (; *key != 0; key++) {
		z += *key & 0x1f;
	}

	ສົ່ງຄືນ (z % (HASH_SIZE - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
static bool
imagemap_add(html_content *c, dom_string *key, sa_nracanaa mapentry *list)
{
	sa_nracanaa imagemap *map;
	unsigned ຈຳນວນເຕັມ slot;

	assert(c != NULL);
	assert(key != NULL);
	assert(list != NULL);

	ຖ້າ (imagemap_create(c) == false)
		ສົ່ງຄືນ false;

	map = calloc(1, parimaa_na(*map));
	ຖ້າ (map == NULL)
		ສົ່ງຄືນ false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->key = strdup(dom_string_data(key));
	ຖ້າ (map->key == NULL) {
		free(map);
		ສົ່ງຄືນ false;
	}

	map->list = list;

	slot = imagemap_hash(map->key);

	map->next = c->imagemaps[slot];
	c->imagemaps[slot] = map;

	ສົ່ງຄືນ true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
static void imagemap_freelist(sa_nracanaa mapentry *list)
{
	sa_nracanaa mapentry *entry, *prev;

	assert(list != NULL);

	entry = list;

	ໃນຂະນະທີ່ (entry != NULL) {
		prev = entry;

		nsurl_unref(entry->url);

		ຖ້າ (entry->target)
			free(entry->target);

		ຖ້າ (entry->type == IMAGEMAP_POLY) {
			free(entry->bounds.poly.xcoords);
			free(entry->bounds.poly.ycoords);
		}

		entry = entry->next;
		free(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
void imagemap_destroy(html_content *c)
{
	unsigned ຈຳນວນເຕັມ i;

	assert(c != NULL);

	/* no imagemaps -> return */
	ຖ້າ (c->imagemaps == NULL)
		ສົ່ງຄືນ;

	ສຳລັບ (i = 0; i != HASH_SIZE; i++) {
		sa_nracanaa imagemap *map, *next;

		map = c->imagemaps[i];
		ໃນຂະນະທີ່ (map != NULL) {
			next = map->next;
			imagemap_freelist(map->list);
			free(map->key);
			free(map);
			map = next;
		}
	}

	free(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
void imagemap_dump(html_content *c)
{
	unsigned ຈຳນວນເຕັມ i;

	ຈຳນວນເຕັມ j;

	assert(c != NULL);

	ຖ້າ (c->imagemaps == NULL)
		ສົ່ງຄືນ;

	ສຳລັບ (i = 0; i != HASH_SIZE; i++) {
		sa_nracanaa imagemap *map;
		sa_nracanaa mapentry *entry;

		map = c->imagemaps[i];
		ໃນຂະນະທີ່ (map != NULL) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->key);

			ສຳລັບ (entry = map->list; entry; entry = entry->next) {
				switch (entry->type) {
				pak_sa IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(entry->url));
					bha_nga;
				pak_sa IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(entry->url),
					      entry->bounds.rect.x0,
					      entry->bounds.rect.y0,
					      entry->bounds.rect.x1,
					      entry->bounds.rect.y1);
					bha_nga;
				pak_sa IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(entry->url),
					      entry->bounds.circle.x,
					      entry->bounds.circle.y,
					      entry->bounds.circle.r);
					bha_nga;
				pak_sa IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(entry->url));
					ສຳລັບ (j = 0; j != entry->bounds.poly.num;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(ຈຳນວນເຕັມ)entry->bounds.poly.xcoords[j],
							(ຈຳນວນເຕັມ)entry->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					bha_nga;
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
static bool
imagemap_addtolist(const sa_nracanaa html_content *c,
		   dom_node *n,
		   nsurl *base_url,
		   sa_nracanaa mapentry **entry,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = NULL, *target = NULL, *shape = NULL;
	dom_string *coords = NULL;
	sa_nracanaa mapentry *new_map, *temp;
	bool ret = true;

	ຖ້າ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ຖ້າ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			goto ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ຖ້າ (exc != DOM_NO_ERR || href == NULL) {
		/* No href="" attribute, skip this element */
		goto ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_target, &target);
	ຖ້າ (exc != DOM_NO_ERR) {
		goto ok_out;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ຖ້າ (exc != DOM_NO_ERR) {
		goto ok_out;
	}

	/* If there's no shape, we default to rectangles */
	ຖ້າ (shape == NULL)
		shape = dom_string_ref(corestring_dom_rect);

	ຖ້າ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ຖ້າ (exc != DOM_NO_ERR || coords == NULL) {
			goto ok_out;
		}
	}

	new_map = calloc(1, parimaa_na(*new_map));
	ຖ້າ (new_map == NULL) {
		goto bad_out;
	}

	ຖ້າ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ບໍ່ດັ່ງນັ້ນ
		goto bad_out;

	ຖ້າ (box_extract_link(c, href, base_url, &new_map->url) == false)
		goto bad_out;

	ຖ້າ (new_map->url == NULL) {
		/* non-fatal error -> ignore this */
		goto ok_free_map_out;
	}

	ຖ້າ (target != NULL) {
		/* Copy target dom string into the map data */
		new_map->target = malloc(dom_string_byte_length(target) + 1);
		ຖ້າ (new_map->target == NULL)
			goto bad_out;

		memcpy(new_map->target,
		       dom_string_data(target),
		       dom_string_byte_length(target));

		new_map->target[dom_string_byte_length(target)] = 0;
	}

	ຖ້າ (new_map->type != IMAGEMAP_DEFAULT) {
		ຈຳນວນເຕັມ x, y;
		float *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		var_na *val = strtok((var_na *)dom_string_data(coords), ",");
		ຈຳນວນເຕັມ num = 1;

		switch (new_map->type) {
		pak_sa IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			ໃນຂະນະທີ່ (val != NULL && num <= 4) {
				switch (num) {
				pak_sa 1:
					new_map->bounds.rect.x0 = atoi(val);
					bha_nga;
				pak_sa 2:
					new_map->bounds.rect.y0 = atoi(val);
					bha_nga;
				pak_sa 3:
					new_map->bounds.rect.x1 = atoi(val);
					bha_nga;
				pak_sa 4:
					new_map->bounds.rect.y1 = atoi(val);
					bha_nga;
				}

				num++;
				val = strtok(NULL, ",");
			}
			bha_nga;
		pak_sa IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			ໃນຂະນະທີ່ (val != NULL && num <= 3) {
				switch (num) {
				pak_sa 1:
					new_map->bounds.circle.x = atoi(val);
					bha_nga;
				pak_sa 2:
					new_map->bounds.circle.y = atoi(val);
					bha_nga;
				pak_sa 3:
					new_map->bounds.circle.r = atoi(val);
					bha_nga;
				}

				num++;
				val = strtok(NULL, ",");
			}
			bha_nga;
		pak_sa IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = NULL;
			new_map->bounds.poly.ycoords = NULL;

			ໃນຂະນະທີ່ (val != NULL) {
				x = atoi(val);

				val = strtok(NULL, ",");
				ຖ້າ (val == NULL)
					bha_nga;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						num * parimaa_na(float));
				ຖ້າ (xcoords == NULL) {
					goto bad_out;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					num * parimaa_na(float));
				ຖ້າ (ycoords == NULL) {
					goto bad_out;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[num - 1] = x;
				new_map->bounds.poly.ycoords[num - 1] = y;

				num++;
				val = strtok(NULL, ",");
			}

			new_map->bounds.poly.num = num - 1;

			bha_nga;
		default:
			bha_nga;
		}
	}

	new_map->next = NULL;

	ຖ້າ (*entry) {
		/* add to END of list */
		ສຳລັບ (temp = (*entry); temp->next != NULL; temp = temp->next)
			;
		temp->next = new_map;
	} ບໍ່ດັ່ງນັ້ນ {
		(*entry) = new_map;
	}

	/* All good, linked in, let's clean up */
	goto ok_out;

bad_out:
	ret = false;
ok_free_map_out:
	ຖ້າ (new_map != NULL) {
		ຖ້າ (new_map->url != NULL)
			nsurl_unref(new_map->url);
		ຖ້າ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != NULL)
			free(new_map->bounds.poly.ycoords);
		ຖ້າ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != NULL)
			free(new_map->bounds.poly.xcoords);
		ຖ້າ (new_map->target != NULL)
			free(new_map->target);

		free(new_map);
	}
ok_out:
	ຖ້າ (href != NULL)
		dom_string_unref(href);
	ຖ້າ (target != NULL)
		dom_string_unref(target);
	ຖ້າ (shape != NULL)
		dom_string_unref(shape);
	ຖ້າ (coords != NULL)
		dom_string_unref(coords);

	ສົ່ງຄືນ ret;
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
static bool
imagemap_extract_map_entries(dom_node *node, html_content *c,
			     sa_nracanaa mapentry **entry, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	unsigned long ent;
	uint32_t tag_count;

	exc = dom_element_get_elements_by_tag_name(node, tname, &nlist);
	ຖ້າ (exc != DOM_NO_ERR) {
		ສົ່ງຄືນ false;
	}

	exc = dom_nodelist_get_length(nlist, &tag_count);
	ຖ້າ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		ສົ່ງຄືນ false;
	}

	ສຳລັບ (ent = 0; ent < tag_count; ++ent) {
		dom_node *subnode;

		exc = dom_nodelist_item(nlist, ent, &subnode);
		ຖ້າ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			ສົ່ງຄືນ false;
		}
		ຖ້າ (imagemap_addtolist(c, subnode, c->base_url,
				       entry, tname) == false) {
			dom_node_unref(subnode);
			dom_nodelist_unref(nlist);
			ສົ່ງຄືນ false;
		}
		dom_node_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	ສົ່ງຄືນ true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
static bool imagemap_extract_map(dom_node *node, html_content *c,
		sa_nracanaa mapentry **entry)
{
	ຖ້າ (imagemap_extract_map_entries(node, c, entry,
			corestring_dom_area) == false)
		ສົ່ງຄືນ false;
	ສົ່ງຄືນ imagemap_extract_map_entries(node, c, entry,
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
	ຖ້າ (exc != DOM_NO_ERR) {
		ສົ່ງຄືນ NSERROR_DOM;
	}

	exc = dom_nodelist_get_length(nlist, &maybe_maps);
	ຖ້າ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		goto out_nlist;
	}

	ສຳລັບ (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_node *node;
		dom_string *name;
		exc = dom_nodelist_item(nlist, mapnr, &node);
		ຖ້າ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			goto out_nlist;
		}

		exc = dom_element_get_attribute(node, corestring_dom_id,
						&name);
		ຖ້າ (exc != DOM_NO_ERR) {
			dom_node_unref(node);
			ret = NSERROR_DOM;
			goto out_nlist;
		}

		ຖ້າ (name == NULL) {
			exc = dom_element_get_attribute(node,
							corestring_dom_name,
							&name);
			ຖ້າ (exc != DOM_NO_ERR) {
				dom_node_unref(node);
				ret = NSERROR_DOM;
				goto out_nlist;
			}
		}

		ຖ້າ (name != NULL) {
			sa_nracanaa mapentry *entry = NULL;
			ຖ້າ (imagemap_extract_map(node, c, &entry) == false) {
				ຖ້າ (entry != NULL) {
					imagemap_freelist(entry);
				}

				dom_string_unref(name);
				dom_node_unref(node);
				ret = NSERROR_NOMEM; /** @todo check this */
				goto out_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ຖ້າ ((entry != NULL) &&
			    (imagemap_add(c, name, entry) == false)) {
				imagemap_freelist(entry);

				dom_string_unref(name);
				dom_node_unref(node);
				ret = NSERROR_NOMEM; /** @todo check this */
				goto out_nlist;
			}
		}

		dom_string_unref(name);
		dom_node_unref(node);
	}

out_nlist:

	dom_nodelist_unref(nlist);

	ສົ່ງຄືນ ret;
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
static ຈຳນວນເຕັມ
imagemap_point_in_poly(ຈຳນວນເຕັມ num, float *xpt, float *ypt, unsigned long x,
		unsigned long y, unsigned long click_x,	unsigned long click_y)
{
	ຈຳນວນເຕັມ i, j, c = 0;

	assert(xpt != NULL);
	assert(ypt != NULL);

	ສຳລັບ (i = 0, j = num - 1; i < num; j = i++) {
		ຖ້າ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	ສົ່ງຄືນ c;
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
nsurl *imagemap_get(sa_nracanaa html_content *c, const var_na *key,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		const var_na **target)
{
	unsigned ຈຳນວນເຕັມ slot = 0;
	sa_nracanaa imagemap *map;
	sa_nracanaa mapentry *entry;
	unsigned long cx, cy;

	assert(c != NULL);

	ຖ້າ (key == NULL)
		ສົ່ງຄືນ NULL;

	ຖ້າ (c->imagemaps == NULL)
		ສົ່ງຄືນ NULL;

	slot = imagemap_hash(key);

	ສຳລັບ (map = c->imagemaps[slot]; map != NULL; map = map->next) {
		ຖ້າ (map->key != NULL && strcasecmp(map->key, key) == 0)
			bha_nga;
	}

	ຖ້າ (map == NULL || map->list == NULL)
		ສົ່ງຄືນ NULL;

	ສຳລັບ (entry = map->list; entry; entry = entry->next) {
		switch (entry->type) {
		pak_sa IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ຖ້າ (target)
				*target = entry->target;
			ສົ່ງຄືນ entry->url;
			bha_nga;
		pak_sa IMAGEMAP_RECT:
			ຖ້າ (click_x >= x + entry->bounds.rect.x0 &&
				    click_x <= x + entry->bounds.rect.x1 &&
				    click_y >= y + entry->bounds.rect.y0 &&
				    click_y <= y + entry->bounds.rect.y1) {
				ຖ້າ (target)
					*target = entry->target;
				ສົ່ງຄືນ entry->url;
			}
			bha_nga;
		pak_sa IMAGEMAP_CIRCLE:
			cx = x + entry->bounds.circle.x - click_x;
			cy = y + entry->bounds.circle.y - click_y;
			ຖ້າ ((cx * cx + cy * cy) <=
				(unsigned long) (entry->bounds.circle.r *
					entry->bounds.circle.r)) {
				ຖ້າ (target)
					*target = entry->target;
				ສົ່ງຄືນ entry->url;
			}
			bha_nga;
		pak_sa IMAGEMAP_POLY:
			ຖ້າ (imagemap_point_in_poly(entry->bounds.poly.num,
					entry->bounds.poly.xcoords,
					entry->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ຖ້າ (target)
					*target = entry->target;
				ສົ່ງຄືນ entry->url;
			}
			bha_nga;
		}
	}

	ຖ້າ (target)
		*target = NULL;

	ສົ່ງຄືນ NULL;
}
