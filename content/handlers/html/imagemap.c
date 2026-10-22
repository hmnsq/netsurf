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

#وصف_ڪريو HASH_سائيز 31 /* fixed size hash table */

تعريفنوع تعداد {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_داخلا_type;

بنية mapentry {
	imagemap_داخلا_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	حرف *هدف;			/**< target frame (if any) */
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
			صحيح انگ;	/**< number of points */
			عائم *xcoords;	/**< x coordinates */
			عائم *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	بنية mapentry *ايندڙ;		/**< next entry in list */
};

بنية imagemap {
	حرف *ڪنجي;		/**< key for this entry */
	بنية mapentry *list;	/**< pointer to linked list of entries */
	بنية imagemap *ايندڙ;	/**< next entry in this hash chain */
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

	إذا (c->imagemaps == عدم) {
		c->imagemaps = صفر_يادگيري_ڏيو(HASH_سائيز, حجم(بنية imagemap *));
		إذا (c->imagemaps == عدم) {
			أرجع false;
		}
	}

	أرجع true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ساكن بنانشان صحيح imagemap_hash(ثابت حرف *ڪنجي)
{
	بنانشان صحيح z = 0;

	إذا (ڪنجي == 0) أرجع 0;

	لكل (; *ڪنجي != 0; ڪنجي++) {
		z += *ڪنجي & 0x1f;
	}

	أرجع (z % (HASH_سائيز - 1)) + 1;
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
imagemap_جوڙيو(html_content *c, dom_string *ڪنجي, بنية mapentry *list)
{
	بنية imagemap *map;
	بنانشان صحيح جڳهه;

	أكد(c != عدم);
	أكد(ڪنجي != عدم);
	أكد(list != عدم);

	إذا (imagemap_create(c) == false)
		أرجع false;

	map = صفر_يادگيري_ڏيو(1, حجم(*map));
	إذا (map == عدم)
		أرجع false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->ڪنجي = strdup(dom_string_معلومات(ڪنجي));
	إذا (map->ڪنجي == عدم) {
		حرر(map);
		أرجع false;
	}

	map->list = list;

	جڳهه = imagemap_hash(map->ڪنجي);

	map->ايندڙ = c->imagemaps[جڳهه];
	c->imagemaps[جڳهه] = map;

	أرجع true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساكن فراغ imagemap_freelist(بنية mapentry *list)
{
	بنية mapentry *داخلا, *prev;

	أكد(list != عدم);

	داخلا = list;

	طالما (داخلا != عدم) {
		prev = داخلا;

		nsurl_unref(داخلا->url);

		إذا (داخلا->هدف)
			حرر(داخلا->هدف);

		إذا (داخلا->type == IMAGEMAP_POLY) {
			حرر(داخلا->bounds.poly.xcoords);
			حرر(داخلا->bounds.poly.ycoords);
		}

		داخلا = داخلا->ايندڙ;
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
	بنانشان صحيح i;

	أكد(c != عدم);

	/* no imagemaps -> return */
	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_سائيز; i++) {
		بنية imagemap *map, *ايندڙ;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			ايندڙ = map->ايندڙ;
			imagemap_freelist(map->list);
			حرر(map->ڪنجي);
			حرر(map);
			map = ايندڙ;
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
	بنانشان صحيح i;

	صحيح j;

	أكد(c != عدم);

	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_سائيز; i++) {
		بنية imagemap *map;
		بنية mapentry *داخلا;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->ڪنجي);

			لكل (داخلا = map->list; داخلا; داخلا = داخلا->ايندڙ) {
				بدل (داخلا->type) {
				حالة IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(داخلا->url));
					اكسر;
				حالة IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(داخلا->url),
					      داخلا->bounds.rect.x0,
					      داخلا->bounds.rect.y0,
					      داخلا->bounds.rect.x1,
					      داخلا->bounds.rect.y1);
					اكسر;
				حالة IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(داخلا->url),
					      داخلا->bounds.circle.x,
					      داخلا->bounds.circle.y,
					      داخلا->bounds.circle.r);
					اكسر;
				حالة IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(داخلا->url));
					لكل (j = 0; j != داخلا->bounds.poly.انگ;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحيح)داخلا->bounds.poly.xcoords[j],
							(صحيح)داخلا->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					اكسر;
				}
			}
			map = map->ايندڙ;
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
		   dom_ڳنڍ *n,
		   nsurl *بنياد_url,
		   بنية mapentry **داخلا,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = عدم, *هدف = عدم, *shape = عدم;
	dom_string *coords = عدم;
	بنية mapentry *new_map, *عارضي;
	bool ret = true;

	إذا (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		إذا ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			اذهب ok_ٻاهريون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	إذا (exc != DOM_NO_ERR || href == عدم) {
		/* No href="" attribute, skip this element */
		اذهب ok_ٻاهريون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_هدف, &هدف);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_ٻاهريون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_ٻاهريون;
	}

	/* If there's no shape, we default to rectangles */
	إذا (shape == عدم)
		shape = dom_string_ref(corestring_dom_rect);

	إذا (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		إذا (exc != DOM_NO_ERR || coords == عدم) {
			اذهب ok_ٻاهريون;
		}
	}

	new_map = صفر_يادگيري_ڏيو(1, حجم(*new_map));
	إذا (new_map == عدم) {
		اذهب bad_ٻاهريون;
	}

	إذا (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	وإلا إذا (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	وإلا إذا (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	وإلا إذا (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	وإلا
		اذهب bad_ٻاهريون;

	إذا (box_ڪڍو_link(c, href, بنياد_url, &new_map->url) == false)
		اذهب bad_ٻاهريون;

	إذا (new_map->url == عدم) {
		/* non-fatal error -> ignore this */
		اذهب ok_free_map_ٻاهريون;
	}

	إذا (هدف != عدم) {
		/* Copy target dom string into the map data */
		new_map->هدف = يادگيري_ڏيو(dom_string_byte_ڊيگهائي(هدف) + 1);
		إذا (new_map->هدف == عدم)
			اذهب bad_ٻاهريون;

		يادگيري_نقل(new_map->هدف,
		       dom_string_معلومات(هدف),
		       dom_string_byte_ڊيگهائي(هدف));

		new_map->هدف[dom_string_byte_ڊيگهائي(هدف)] = 0;
	}

	إذا (new_map->type != IMAGEMAP_DEFAULT) {
		صحيح x, y;
		عائم *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_معلومات(coords), ",");
		صحيح انگ = 1;

		بدل (new_map->type) {
		حالة IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			طالما (val != عدم && انگ <= 4) {
				بدل (انگ) {
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

				انگ++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			طالما (val != عدم && انگ <= 3) {
				بدل (انگ) {
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

				انگ++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = عدم;
			new_map->bounds.poly.ycoords = عدم;

			طالما (val != عدم) {
				x = atoi(val);

				val = strtok(عدم, ",");
				إذا (val == عدم)
					اكسر;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						انگ * حجم(عائم));
				إذا (xcoords == عدم) {
					اذهب bad_ٻاهريون;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					انگ * حجم(عائم));
				إذا (ycoords == عدم) {
					اذهب bad_ٻاهريون;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[انگ - 1] = x;
				new_map->bounds.poly.ycoords[انگ - 1] = y;

				انگ++;
				val = strtok(عدم, ",");
			}

			new_map->bounds.poly.انگ = انگ - 1;

			اكسر;
		افتراضي:
			اكسر;
		}
	}

	new_map->ايندڙ = عدم;

	إذا (*داخلا) {
		/* add to END of list */
		لكل (عارضي = (*داخلا); عارضي->ايندڙ != عدم; عارضي = عارضي->ايندڙ)
			;
		عارضي->ايندڙ = new_map;
	} وإلا {
		(*داخلا) = new_map;
	}

	/* All good, linked in, let's clean up */
	اذهب ok_ٻاهريون;

bad_ٻاهريون:
	ret = false;
ok_free_map_ٻاهريون:
	إذا (new_map != عدم) {
		إذا (new_map->url != عدم)
			nsurl_unref(new_map->url);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != عدم)
			حرر(new_map->bounds.poly.ycoords);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != عدم)
			حرر(new_map->bounds.poly.xcoords);
		إذا (new_map->هدف != عدم)
			حرر(new_map->هدف);

		حرر(new_map);
	}
ok_ٻاهريون:
	إذا (href != عدم)
		dom_string_unref(href);
	إذا (هدف != عدم)
		dom_string_unref(هدف);
	إذا (shape != عدم)
		dom_string_unref(shape);
	إذا (coords != عدم)
		dom_string_unref(coords);

	أرجع ret;
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
imagemap_ڪڍو_map_entries(dom_ڳنڍ *ڳنڍ, html_content *c,
			     بنية mapentry **داخلا, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بنانشان ڊگهو ent;
	uint32_t ٽيگ_ڳڻپ;

	exc = dom_element_get_elements_by_ٽيگ_name(ڳنڍ, tname, &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	exc = dom_nodelist_get_ڊيگهائي(nlist, &ٽيگ_ڳڻپ);
	إذا (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		أرجع false;
	}

	لكل (ent = 0; ent < ٽيگ_ڳڻپ; ++ent) {
		dom_ڳنڍ *subnode;

		exc = dom_nodelist_شي(nlist, ent, &subnode);
		إذا (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		إذا (imagemap_addtolist(c, subnode, c->بنياد_url,
				       داخلا, tname) == false) {
			dom_ڳنڍ_unref(subnode);
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		dom_ڳنڍ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	أرجع true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ساكن bool imagemap_ڪڍو_map(dom_ڳنڍ *ڳنڍ, html_content *c,
		بنية mapentry **داخلا)
{
	إذا (imagemap_ڪڍو_map_entries(ڳنڍ, c, داخلا,
			corestring_dom_area) == false)
		أرجع false;
	أرجع imagemap_ڪڍو_map_entries(ڳنڍ, c, داخلا,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_ڪڍو(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بنانشان ڊگهو mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_ٽيگ_name(c->document,
						    corestring_dom_map,
						    &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	exc = dom_nodelist_get_ڊيگهائي(nlist, &maybe_maps);
	إذا (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		اذهب ٻاهريون_nlist;
	}

	لكل (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_ڳنڍ *ڳنڍ;
		dom_string *name;
		exc = dom_nodelist_شي(nlist, mapnr, &ڳنڍ);
		إذا (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			اذهب ٻاهريون_nlist;
		}

		exc = dom_element_get_attribute(ڳنڍ, corestring_dom_id,
						&name);
		إذا (exc != DOM_NO_ERR) {
			dom_ڳنڍ_unref(ڳنڍ);
			ret = NSERROR_DOM;
			اذهب ٻاهريون_nlist;
		}

		إذا (name == عدم) {
			exc = dom_element_get_attribute(ڳنڍ,
							corestring_dom_name,
							&name);
			إذا (exc != DOM_NO_ERR) {
				dom_ڳنڍ_unref(ڳنڍ);
				ret = NSERROR_DOM;
				اذهب ٻاهريون_nlist;
			}
		}

		إذا (name != عدم) {
			بنية mapentry *داخلا = عدم;
			إذا (imagemap_ڪڍو_map(ڳنڍ, c, &داخلا) == false) {
				إذا (داخلا != عدم) {
					imagemap_freelist(داخلا);
				}

				dom_string_unref(name);
				dom_ڳنڍ_unref(ڳنڍ);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب ٻاهريون_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			إذا ((داخلا != عدم) &&
			    (imagemap_جوڙيو(c, name, داخلا) == false)) {
				imagemap_freelist(داخلا);

				dom_string_unref(name);
				dom_ڳنڍ_unref(ڳنڍ);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب ٻاهريون_nlist;
			}
		}

		dom_string_unref(name);
		dom_ڳنڍ_unref(ڳنڍ);
	}

ٻاهريون_nlist:

	dom_nodelist_unref(nlist);

	أرجع ret;
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
imagemap_نقطو_اندريون_poly(صحيح انگ, عائم *xpt, عائم *ypt, بنانشان ڊگهو x,
		بنانشان ڊگهو y, بنانشان ڊگهو click_x,	بنانشان ڊگهو click_y)
{
	صحيح i, j, c = 0;

	أكد(xpt != عدم);
	أكد(ypt != عدم);

	لكل (i = 0, j = انگ - 1; i < انگ; j = i++) {
		إذا ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	أرجع c;
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
nsurl *imagemap_get(بنية html_content *c, ثابت حرف *ڪنجي,
		بنانشان ڊگهو x, بنانشان ڊگهو y,
		بنانشان ڊگهو click_x, بنانشان ڊگهو click_y,
		ثابت حرف **هدف)
{
	بنانشان صحيح جڳهه = 0;
	بنية imagemap *map;
	بنية mapentry *داخلا;
	بنانشان ڊگهو cx, cy;

	أكد(c != عدم);

	إذا (ڪنجي == عدم)
		أرجع عدم;

	إذا (c->imagemaps == عدم)
		أرجع عدم;

	جڳهه = imagemap_hash(ڪنجي);

	لكل (map = c->imagemaps[جڳهه]; map != عدم; map = map->ايندڙ) {
		إذا (map->ڪنجي != عدم && strcasecmp(map->ڪنجي, ڪنجي) == 0)
			اكسر;
	}

	إذا (map == عدم || map->list == عدم)
		أرجع عدم;

	لكل (داخلا = map->list; داخلا; داخلا = داخلا->ايندڙ) {
		بدل (داخلا->type) {
		حالة IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			إذا (هدف)
				*هدف = داخلا->هدف;
			أرجع داخلا->url;
			اكسر;
		حالة IMAGEMAP_RECT:
			إذا (click_x >= x + داخلا->bounds.rect.x0 &&
				    click_x <= x + داخلا->bounds.rect.x1 &&
				    click_y >= y + داخلا->bounds.rect.y0 &&
				    click_y <= y + داخلا->bounds.rect.y1) {
				إذا (هدف)
					*هدف = داخلا->هدف;
				أرجع داخلا->url;
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			cx = x + داخلا->bounds.circle.x - click_x;
			cy = y + داخلا->bounds.circle.y - click_y;
			إذا ((cx * cx + cy * cy) <=
				(بنانشان ڊگهو) (داخلا->bounds.circle.r *
					داخلا->bounds.circle.r)) {
				إذا (هدف)
					*هدف = داخلا->هدف;
				أرجع داخلا->url;
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			إذا (imagemap_نقطو_اندريون_poly(داخلا->bounds.poly.انگ,
					داخلا->bounds.poly.xcoords,
					داخلا->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				إذا (هدف)
					*هدف = داخلا->هدف;
				أرجع داخلا->url;
			}
			اكسر;
		}
	}

	إذا (هدف)
		*هدف = عدم;

	أرجع عدم;
}
