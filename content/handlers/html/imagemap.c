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

#تعریف_کن HASH_اندازه 31 /* fixed size hash table */

تعريفنوع تعداد {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_مدخل_type;

بنية mapentry {
	imagemap_مدخل_type type;	/**< type of shape */
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
			صحيح رقم;	/**< number of points */
			عائم *xcoords;	/**< x coordinates */
			عائم *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	بنية mapentry *بعدی;		/**< next entry in list */
};

بنية imagemap {
	حرف *کلید;		/**< key for this entry */
	بنية mapentry *list;	/**< pointer to linked list of entries */
	بنية imagemap *بعدی;	/**< next entry in this hash chain */
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
		c->imagemaps = حافظه_صفر_بده(HASH_اندازه, حجم(بنية imagemap *));
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
ساكن بیعلامت صحيح imagemap_hash(ثابت حرف *کلید)
{
	بیعلامت صحيح z = 0;

	إذا (کلید == 0) أرجع 0;

	لكل (; *کلید != 0; کلید++) {
		z += *کلید & 0x1f;
	}

	أرجع (z % (HASH_اندازه - 1)) + 1;
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
imagemap_جمع_کن(html_content *c, dom_string *کلید, بنية mapentry *list)
{
	بنية imagemap *map;
	بیعلامت صحيح جایگاه;

	أكد(c != عدم);
	أكد(کلید != عدم);
	أكد(list != عدم);

	إذا (imagemap_create(c) == false)
		أرجع false;

	map = حافظه_صفر_بده(1, حجم(*map));
	إذا (map == عدم)
		أرجع false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->کلید = strdup(dom_string_داده(کلید));
	إذا (map->کلید == عدم) {
		حرر(map);
		أرجع false;
	}

	map->list = list;

	جایگاه = imagemap_hash(map->کلید);

	map->بعدی = c->imagemaps[جایگاه];
	c->imagemaps[جایگاه] = map;

	أرجع true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساكن فراغ imagemap_freelist(بنية mapentry *list)
{
	بنية mapentry *مدخل, *prev;

	أكد(list != عدم);

	مدخل = list;

	طالما (مدخل != عدم) {
		prev = مدخل;

		nsurl_unref(مدخل->url);

		إذا (مدخل->هدف)
			حرر(مدخل->هدف);

		إذا (مدخل->type == IMAGEMAP_POLY) {
			حرر(مدخل->bounds.poly.xcoords);
			حرر(مدخل->bounds.poly.ycoords);
		}

		مدخل = مدخل->بعدی;
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
	بیعلامت صحيح i;

	أكد(c != عدم);

	/* no imagemaps -> return */
	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_اندازه; i++) {
		بنية imagemap *map, *بعدی;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			بعدی = map->بعدی;
			imagemap_freelist(map->list);
			حرر(map->کلید);
			حرر(map);
			map = بعدی;
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
	بیعلامت صحيح i;

	صحيح j;

	أكد(c != عدم);

	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_اندازه; i++) {
		بنية imagemap *map;
		بنية mapentry *مدخل;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->کلید);

			لكل (مدخل = map->list; مدخل; مدخل = مدخل->بعدی) {
				بدل (مدخل->type) {
				حالة IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(مدخل->url));
					اكسر;
				حالة IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(مدخل->url),
					      مدخل->bounds.rect.x0,
					      مدخل->bounds.rect.y0,
					      مدخل->bounds.rect.x1,
					      مدخل->bounds.rect.y1);
					اكسر;
				حالة IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(مدخل->url),
					      مدخل->bounds.circle.x,
					      مدخل->bounds.circle.y,
					      مدخل->bounds.circle.r);
					اكسر;
				حالة IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(مدخل->url));
					لكل (j = 0; j != مدخل->bounds.poly.رقم;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحيح)مدخل->bounds.poly.xcoords[j],
							(صحيح)مدخل->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					اكسر;
				}
			}
			map = map->بعدی;
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
		   dom_گره *n,
		   nsurl *پایه_url,
		   بنية mapentry **مدخل,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = عدم, *هدف = عدم, *shape = عدم;
	dom_string *coords = عدم;
	بنية mapentry *new_map, *موقت;
	bool ret = true;

	إذا (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		إذا ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			اذهب ok_بیرون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	إذا (exc != DOM_NO_ERR || href == عدم) {
		/* No href="" attribute, skip this element */
		اذهب ok_بیرون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_هدف, &هدف);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_بیرون;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_بیرون;
	}

	/* If there's no shape, we default to rectangles */
	إذا (shape == عدم)
		shape = dom_string_ref(corestring_dom_rect);

	إذا (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		إذا (exc != DOM_NO_ERR || coords == عدم) {
			اذهب ok_بیرون;
		}
	}

	new_map = حافظه_صفر_بده(1, حجم(*new_map));
	إذا (new_map == عدم) {
		اذهب bad_بیرون;
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
		اذهب bad_بیرون;

	إذا (box_استخراج_link(c, href, پایه_url, &new_map->url) == false)
		اذهب bad_بیرون;

	إذا (new_map->url == عدم) {
		/* non-fatal error -> ignore this */
		اذهب ok_free_map_بیرون;
	}

	إذا (هدف != عدم) {
		/* Copy target dom string into the map data */
		new_map->هدف = حافظه_بده(dom_string_byte_درازی(هدف) + 1);
		إذا (new_map->هدف == عدم)
			اذهب bad_بیرون;

		کپی_حافظه(new_map->هدف,
		       dom_string_داده(هدف),
		       dom_string_byte_درازی(هدف));

		new_map->هدف[dom_string_byte_درازی(هدف)] = 0;
	}

	إذا (new_map->type != IMAGEMAP_DEFAULT) {
		صحيح x, y;
		عائم *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_داده(coords), ",");
		صحيح رقم = 1;

		بدل (new_map->type) {
		حالة IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			طالما (val != عدم && رقم <= 4) {
				بدل (رقم) {
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

				رقم++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			طالما (val != عدم && رقم <= 3) {
				بدل (رقم) {
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

				رقم++;
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
						رقم * حجم(عائم));
				إذا (xcoords == عدم) {
					اذهب bad_بیرون;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					رقم * حجم(عائم));
				إذا (ycoords == عدم) {
					اذهب bad_بیرون;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[رقم - 1] = x;
				new_map->bounds.poly.ycoords[رقم - 1] = y;

				رقم++;
				val = strtok(عدم, ",");
			}

			new_map->bounds.poly.رقم = رقم - 1;

			اكسر;
		افتراضي:
			اكسر;
		}
	}

	new_map->بعدی = عدم;

	إذا (*مدخل) {
		/* add to END of list */
		لكل (موقت = (*مدخل); موقت->بعدی != عدم; موقت = موقت->بعدی)
			;
		موقت->بعدی = new_map;
	} وإلا {
		(*مدخل) = new_map;
	}

	/* All good, linked in, let's clean up */
	اذهب ok_بیرون;

bad_بیرون:
	ret = false;
ok_free_map_بیرون:
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
ok_بیرون:
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
imagemap_استخراج_map_entries(dom_گره *گره, html_content *c,
			     بنية mapentry **مدخل, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بیعلامت دراز ent;
	uint32_t برچسب_شمار;

	exc = dom_element_get_elements_by_برچسب_name(گره, tname, &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	exc = dom_nodelist_get_درازی(nlist, &برچسب_شمار);
	إذا (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		أرجع false;
	}

	لكل (ent = 0; ent < برچسب_شمار; ++ent) {
		dom_گره *subnode;

		exc = dom_nodelist_قلم(nlist, ent, &subnode);
		إذا (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		إذا (imagemap_addtolist(c, subnode, c->پایه_url,
				       مدخل, tname) == false) {
			dom_گره_unref(subnode);
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		dom_گره_unref(subnode);
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
ساكن bool imagemap_استخراج_map(dom_گره *گره, html_content *c,
		بنية mapentry **مدخل)
{
	إذا (imagemap_استخراج_map_entries(گره, c, مدخل,
			corestring_dom_area) == false)
		أرجع false;
	أرجع imagemap_استخراج_map_entries(گره, c, مدخل,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_استخراج(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بیعلامت دراز mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_برچسب_name(c->document,
						    corestring_dom_map,
						    &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	exc = dom_nodelist_get_درازی(nlist, &maybe_maps);
	إذا (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		اذهب بیرون_nlist;
	}

	لكل (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_گره *گره;
		dom_string *name;
		exc = dom_nodelist_قلم(nlist, mapnr, &گره);
		إذا (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			اذهب بیرون_nlist;
		}

		exc = dom_element_get_attribute(گره, corestring_dom_id,
						&name);
		إذا (exc != DOM_NO_ERR) {
			dom_گره_unref(گره);
			ret = NSERROR_DOM;
			اذهب بیرون_nlist;
		}

		إذا (name == عدم) {
			exc = dom_element_get_attribute(گره,
							corestring_dom_name,
							&name);
			إذا (exc != DOM_NO_ERR) {
				dom_گره_unref(گره);
				ret = NSERROR_DOM;
				اذهب بیرون_nlist;
			}
		}

		إذا (name != عدم) {
			بنية mapentry *مدخل = عدم;
			إذا (imagemap_استخراج_map(گره, c, &مدخل) == false) {
				إذا (مدخل != عدم) {
					imagemap_freelist(مدخل);
				}

				dom_string_unref(name);
				dom_گره_unref(گره);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب بیرون_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			إذا ((مدخل != عدم) &&
			    (imagemap_جمع_کن(c, name, مدخل) == false)) {
				imagemap_freelist(مدخل);

				dom_string_unref(name);
				dom_گره_unref(گره);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب بیرون_nlist;
			}
		}

		dom_string_unref(name);
		dom_گره_unref(گره);
	}

بیرون_nlist:

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
imagemap_نقطه_درون_poly(صحيح رقم, عائم *xpt, عائم *ypt, بیعلامت دراز x,
		بیعلامت دراز y, بیعلامت دراز click_x,	بیعلامت دراز click_y)
{
	صحيح i, j, c = 0;

	أكد(xpt != عدم);
	أكد(ypt != عدم);

	لكل (i = 0, j = رقم - 1; i < رقم; j = i++) {
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
nsurl *imagemap_get(بنية html_content *c, ثابت حرف *کلید,
		بیعلامت دراز x, بیعلامت دراز y,
		بیعلامت دراز click_x, بیعلامت دراز click_y,
		ثابت حرف **هدف)
{
	بیعلامت صحيح جایگاه = 0;
	بنية imagemap *map;
	بنية mapentry *مدخل;
	بیعلامت دراز cx, cy;

	أكد(c != عدم);

	إذا (کلید == عدم)
		أرجع عدم;

	إذا (c->imagemaps == عدم)
		أرجع عدم;

	جایگاه = imagemap_hash(کلید);

	لكل (map = c->imagemaps[جایگاه]; map != عدم; map = map->بعدی) {
		إذا (map->کلید != عدم && strcasecmp(map->کلید, کلید) == 0)
			اكسر;
	}

	إذا (map == عدم || map->list == عدم)
		أرجع عدم;

	لكل (مدخل = map->list; مدخل; مدخل = مدخل->بعدی) {
		بدل (مدخل->type) {
		حالة IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			إذا (هدف)
				*هدف = مدخل->هدف;
			أرجع مدخل->url;
			اكسر;
		حالة IMAGEMAP_RECT:
			إذا (click_x >= x + مدخل->bounds.rect.x0 &&
				    click_x <= x + مدخل->bounds.rect.x1 &&
				    click_y >= y + مدخل->bounds.rect.y0 &&
				    click_y <= y + مدخل->bounds.rect.y1) {
				إذا (هدف)
					*هدف = مدخل->هدف;
				أرجع مدخل->url;
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			cx = x + مدخل->bounds.circle.x - click_x;
			cy = y + مدخل->bounds.circle.y - click_y;
			إذا ((cx * cx + cy * cy) <=
				(بیعلامت دراز) (مدخل->bounds.circle.r *
					مدخل->bounds.circle.r)) {
				إذا (هدف)
					*هدف = مدخل->هدف;
				أرجع مدخل->url;
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			إذا (imagemap_نقطه_درون_poly(مدخل->bounds.poly.رقم,
					مدخل->bounds.poly.xcoords,
					مدخل->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				إذا (هدف)
					*هدف = مدخل->هدف;
				أرجع مدخل->url;
			}
			اكسر;
		}
	}

	إذا (هدف)
		*هدف = عدم;

	أرجع عدم;
}
