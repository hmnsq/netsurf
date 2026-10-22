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

#تعریف_کړه HASH_اندازه 31 /* fixed size hash table */

تعريفنوع تعداد {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_ثبت_type;

بنية mapentry {
	imagemap_ثبت_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	حرف *موخه;			/**< target frame (if any) */
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
			صحيح عدد;	/**< number of points */
			عائم *xcoords;	/**< x coordinates */
			عائم *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	بنية mapentry *بل;		/**< next entry in list */
};

بنية imagemap {
	حرف *کلی;		/**< key for this entry */
	بنية mapentry *list;	/**< pointer to linked list of entries */
	بنية imagemap *بل;	/**< next entry in this hash chain */
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
		c->imagemaps = حافظه_صفر_ورکړه(HASH_اندازه, حجم(بنية imagemap *));
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
ساكن بینښه صحيح imagemap_hash(ثابت حرف *کلی)
{
	بینښه صحيح z = 0;

	إذا (کلی == 0) أرجع 0;

	لكل (; *کلی != 0; کلی++) {
		z += *کلی & 0x1f;
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
imagemap_جمع_کړه(html_content *c, dom_string *کلی, بنية mapentry *list)
{
	بنية imagemap *map;
	بینښه صحيح خونه;

	أكد(c != عدم);
	أكد(کلی != عدم);
	أكد(list != عدم);

	إذا (imagemap_create(c) == false)
		أرجع false;

	map = حافظه_صفر_ورکړه(1, حجم(*map));
	إذا (map == عدم)
		أرجع false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->کلی = strdup(dom_string_معلومات(کلی));
	إذا (map->کلی == عدم) {
		حرر(map);
		أرجع false;
	}

	map->list = list;

	خونه = imagemap_hash(map->کلی);

	map->بل = c->imagemaps[خونه];
	c->imagemaps[خونه] = map;

	أرجع true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساكن فراغ imagemap_freelist(بنية mapentry *list)
{
	بنية mapentry *ثبت, *prev;

	أكد(list != عدم);

	ثبت = list;

	طالما (ثبت != عدم) {
		prev = ثبت;

		nsurl_unref(ثبت->url);

		إذا (ثبت->موخه)
			حرر(ثبت->موخه);

		إذا (ثبت->type == IMAGEMAP_POLY) {
			حرر(ثبت->bounds.poly.xcoords);
			حرر(ثبت->bounds.poly.ycoords);
		}

		ثبت = ثبت->بل;
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
	بینښه صحيح i;

	أكد(c != عدم);

	/* no imagemaps -> return */
	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_اندازه; i++) {
		بنية imagemap *map, *بل;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			بل = map->بل;
			imagemap_freelist(map->list);
			حرر(map->کلی);
			حرر(map);
			map = بل;
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
	بینښه صحيح i;

	صحيح j;

	أكد(c != عدم);

	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_اندازه; i++) {
		بنية imagemap *map;
		بنية mapentry *ثبت;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->کلی);

			لكل (ثبت = map->list; ثبت; ثبت = ثبت->بل) {
				بدل (ثبت->type) {
				حالة IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(ثبت->url));
					اكسر;
				حالة IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(ثبت->url),
					      ثبت->bounds.rect.x0,
					      ثبت->bounds.rect.y0,
					      ثبت->bounds.rect.x1,
					      ثبت->bounds.rect.y1);
					اكسر;
				حالة IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(ثبت->url),
					      ثبت->bounds.circle.x,
					      ثبت->bounds.circle.y,
					      ثبت->bounds.circle.r);
					اكسر;
				حالة IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(ثبت->url));
					لكل (j = 0; j != ثبت->bounds.poly.عدد;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحيح)ثبت->bounds.poly.xcoords[j],
							(صحيح)ثبت->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					اكسر;
				}
			}
			map = map->بل;
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
		   dom_غوټه *n,
		   nsurl *بنسټ_url,
		   بنية mapentry **ثبت,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = عدم, *موخه = عدم, *shape = عدم;
	dom_string *coords = عدم;
	بنية mapentry *new_map, *لنډمهاله;
	bool ret = true;

	إذا (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		إذا ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			اذهب ok_بهر;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	إذا (exc != DOM_NO_ERR || href == عدم) {
		/* No href="" attribute, skip this element */
		اذهب ok_بهر;
	}

	exc = dom_element_get_attribute(n, corestring_dom_موخه, &موخه);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_بهر;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_بهر;
	}

	/* If there's no shape, we default to rectangles */
	إذا (shape == عدم)
		shape = dom_string_ref(corestring_dom_rect);

	إذا (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		إذا (exc != DOM_NO_ERR || coords == عدم) {
			اذهب ok_بهر;
		}
	}

	new_map = حافظه_صفر_ورکړه(1, حجم(*new_map));
	إذا (new_map == عدم) {
		اذهب bad_بهر;
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
		اذهب bad_بهر;

	إذا (box_ایستل_link(c, href, بنسټ_url, &new_map->url) == false)
		اذهب bad_بهر;

	إذا (new_map->url == عدم) {
		/* non-fatal error -> ignore this */
		اذهب ok_free_map_بهر;
	}

	إذا (موخه != عدم) {
		/* Copy target dom string into the map data */
		new_map->موخه = حافظه_ورکړه(dom_string_byte_اوږدتیا(موخه) + 1);
		إذا (new_map->موخه == عدم)
			اذهب bad_بهر;

		حافظه_نقل(new_map->موخه,
		       dom_string_معلومات(موخه),
		       dom_string_byte_اوږدتیا(موخه));

		new_map->موخه[dom_string_byte_اوږدتیا(موخه)] = 0;
	}

	إذا (new_map->type != IMAGEMAP_DEFAULT) {
		صحيح x, y;
		عائم *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_معلومات(coords), ",");
		صحيح عدد = 1;

		بدل (new_map->type) {
		حالة IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			طالما (val != عدم && عدد <= 4) {
				بدل (عدد) {
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

				عدد++;
				val = strtok(عدم, ",");
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			طالما (val != عدم && عدد <= 3) {
				بدل (عدد) {
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

				عدد++;
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
						عدد * حجم(عائم));
				إذا (xcoords == عدم) {
					اذهب bad_بهر;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					عدد * حجم(عائم));
				إذا (ycoords == عدم) {
					اذهب bad_بهر;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[عدد - 1] = x;
				new_map->bounds.poly.ycoords[عدد - 1] = y;

				عدد++;
				val = strtok(عدم, ",");
			}

			new_map->bounds.poly.عدد = عدد - 1;

			اكسر;
		افتراضي:
			اكسر;
		}
	}

	new_map->بل = عدم;

	إذا (*ثبت) {
		/* add to END of list */
		لكل (لنډمهاله = (*ثبت); لنډمهاله->بل != عدم; لنډمهاله = لنډمهاله->بل)
			;
		لنډمهاله->بل = new_map;
	} وإلا {
		(*ثبت) = new_map;
	}

	/* All good, linked in, let's clean up */
	اذهب ok_بهر;

bad_بهر:
	ret = false;
ok_free_map_بهر:
	إذا (new_map != عدم) {
		إذا (new_map->url != عدم)
			nsurl_unref(new_map->url);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != عدم)
			حرر(new_map->bounds.poly.ycoords);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != عدم)
			حرر(new_map->bounds.poly.xcoords);
		إذا (new_map->موخه != عدم)
			حرر(new_map->موخه);

		حرر(new_map);
	}
ok_بهر:
	إذا (href != عدم)
		dom_string_unref(href);
	إذا (موخه != عدم)
		dom_string_unref(موخه);
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
imagemap_ایستل_map_entries(dom_غوټه *غوټه, html_content *c,
			     بنية mapentry **ثبت, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بینښه اوږد ent;
	uint32_t نښان_شمېر;

	exc = dom_element_get_elements_by_نښان_name(غوټه, tname, &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	exc = dom_nodelist_get_اوږدتیا(nlist, &نښان_شمېر);
	إذا (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		أرجع false;
	}

	لكل (ent = 0; ent < نښان_شمېر; ++ent) {
		dom_غوټه *subnode;

		exc = dom_nodelist_توکی(nlist, ent, &subnode);
		إذا (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		إذا (imagemap_addtolist(c, subnode, c->بنسټ_url,
				       ثبت, tname) == false) {
			dom_غوټه_unref(subnode);
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		dom_غوټه_unref(subnode);
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
ساكن bool imagemap_ایستل_map(dom_غوټه *غوټه, html_content *c,
		بنية mapentry **ثبت)
{
	إذا (imagemap_ایستل_map_entries(غوټه, c, ثبت,
			corestring_dom_area) == false)
		أرجع false;
	أرجع imagemap_ایستل_map_entries(غوټه, c, ثبت,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_ایستل(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بینښه اوږد mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_نښان_name(c->document,
						    corestring_dom_map,
						    &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	exc = dom_nodelist_get_اوږدتیا(nlist, &maybe_maps);
	إذا (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		اذهب بهر_nlist;
	}

	لكل (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_غوټه *غوټه;
		dom_string *name;
		exc = dom_nodelist_توکی(nlist, mapnr, &غوټه);
		إذا (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			اذهب بهر_nlist;
		}

		exc = dom_element_get_attribute(غوټه, corestring_dom_id,
						&name);
		إذا (exc != DOM_NO_ERR) {
			dom_غوټه_unref(غوټه);
			ret = NSERROR_DOM;
			اذهب بهر_nlist;
		}

		إذا (name == عدم) {
			exc = dom_element_get_attribute(غوټه,
							corestring_dom_name,
							&name);
			إذا (exc != DOM_NO_ERR) {
				dom_غوټه_unref(غوټه);
				ret = NSERROR_DOM;
				اذهب بهر_nlist;
			}
		}

		إذا (name != عدم) {
			بنية mapentry *ثبت = عدم;
			إذا (imagemap_ایستل_map(غوټه, c, &ثبت) == false) {
				إذا (ثبت != عدم) {
					imagemap_freelist(ثبت);
				}

				dom_string_unref(name);
				dom_غوټه_unref(غوټه);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب بهر_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			إذا ((ثبت != عدم) &&
			    (imagemap_جمع_کړه(c, name, ثبت) == false)) {
				imagemap_freelist(ثبت);

				dom_string_unref(name);
				dom_غوټه_unref(غوټه);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب بهر_nlist;
			}
		}

		dom_string_unref(name);
		dom_غوټه_unref(غوټه);
	}

بهر_nlist:

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
imagemap_ټکی_دننه_poly(صحيح عدد, عائم *xpt, عائم *ypt, بینښه اوږد x,
		بینښه اوږد y, بینښه اوږد click_x,	بینښه اوږد click_y)
{
	صحيح i, j, c = 0;

	أكد(xpt != عدم);
	أكد(ypt != عدم);

	لكل (i = 0, j = عدد - 1; i < عدد; j = i++) {
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
nsurl *imagemap_get(بنية html_content *c, ثابت حرف *کلی,
		بینښه اوږد x, بینښه اوږد y,
		بینښه اوږد click_x, بینښه اوږد click_y,
		ثابت حرف **موخه)
{
	بینښه صحيح خونه = 0;
	بنية imagemap *map;
	بنية mapentry *ثبت;
	بینښه اوږد cx, cy;

	أكد(c != عدم);

	إذا (کلی == عدم)
		أرجع عدم;

	إذا (c->imagemaps == عدم)
		أرجع عدم;

	خونه = imagemap_hash(کلی);

	لكل (map = c->imagemaps[خونه]; map != عدم; map = map->بل) {
		إذا (map->کلی != عدم && strcasecmp(map->کلی, کلی) == 0)
			اكسر;
	}

	إذا (map == عدم || map->list == عدم)
		أرجع عدم;

	لكل (ثبت = map->list; ثبت; ثبت = ثبت->بل) {
		بدل (ثبت->type) {
		حالة IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			إذا (موخه)
				*موخه = ثبت->موخه;
			أرجع ثبت->url;
			اكسر;
		حالة IMAGEMAP_RECT:
			إذا (click_x >= x + ثبت->bounds.rect.x0 &&
				    click_x <= x + ثبت->bounds.rect.x1 &&
				    click_y >= y + ثبت->bounds.rect.y0 &&
				    click_y <= y + ثبت->bounds.rect.y1) {
				إذا (موخه)
					*موخه = ثبت->موخه;
				أرجع ثبت->url;
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			cx = x + ثبت->bounds.circle.x - click_x;
			cy = y + ثبت->bounds.circle.y - click_y;
			إذا ((cx * cx + cy * cy) <=
				(بینښه اوږد) (ثبت->bounds.circle.r *
					ثبت->bounds.circle.r)) {
				إذا (موخه)
					*موخه = ثبت->موخه;
				أرجع ثبت->url;
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			إذا (imagemap_ټکی_دننه_poly(ثبت->bounds.poly.عدد,
					ثبت->bounds.poly.xcoords,
					ثبت->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				إذا (موخه)
					*موخه = ثبت->موخه;
				أرجع ثبت->url;
			}
			اكسر;
		}
	}

	إذا (موخه)
		*موخه = عدم;

	أرجع عدم;
}
