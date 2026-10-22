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

#تعریف_کرو HASH_ناپ 31 /* fixed size hash table */

تعريفنوع تعداد {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_اندراج_type;

بنية mapentry {
	imagemap_اندراج_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	حرف *نشانہ;			/**< target frame (if any) */
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
	بنية mapentry *اگلا;		/**< next entry in list */
};

بنية imagemap {
	حرف *کلید;		/**< key for this entry */
	بنية mapentry *list;	/**< pointer to linked list of entries */
	بنية imagemap *اگلا;	/**< next entry in this hash chain */
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
		c->imagemaps = صفر_حافظہ_دیو(HASH_ناپ, حجم(بنية imagemap *));
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
ساكن بنانشانی صحيح imagemap_hash(ثابت حرف *کلید)
{
	بنانشانی صحيح z = 0;

	إذا (کلید == 0) أرجع 0;

	لكل (; *کلید != 0; کلید++) {
		z += *کلید & 0x1f;
	}

	أرجع (z % (HASH_ناپ - 1)) + 1;
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
imagemap_جوڑو(html_content *c, dom_string *کلید, بنية mapentry *list)
{
	بنية imagemap *map;
	بنانشانی صحيح تھاں;

	أكد(c != عدم);
	أكد(کلید != عدم);
	أكد(list != عدم);

	إذا (imagemap_create(c) == false)
		أرجع false;

	map = صفر_حافظہ_دیو(1, حجم(*map));
	إذا (map == عدم)
		أرجع false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->کلید = strdup(dom_string_انکڑے(کلید));
	إذا (map->کلید == عدم) {
		حرر(map);
		أرجع false;
	}

	map->list = list;

	تھاں = imagemap_hash(map->کلید);

	map->اگلا = c->imagemaps[تھاں];
	c->imagemaps[تھاں] = map;

	أرجع true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساكن فراغ imagemap_freelist(بنية mapentry *list)
{
	بنية mapentry *اندراج, *prev;

	أكد(list != عدم);

	اندراج = list;

	طالما (اندراج != عدم) {
		prev = اندراج;

		nsurl_unref(اندراج->url);

		إذا (اندراج->نشانہ)
			حرر(اندراج->نشانہ);

		إذا (اندراج->type == IMAGEMAP_POLY) {
			حرر(اندراج->bounds.poly.xcoords);
			حرر(اندراج->bounds.poly.ycoords);
		}

		اندراج = اندراج->اگلا;
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
	بنانشانی صحيح i;

	أكد(c != عدم);

	/* no imagemaps -> return */
	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_ناپ; i++) {
		بنية imagemap *map, *اگلا;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			اگلا = map->اگلا;
			imagemap_freelist(map->list);
			حرر(map->کلید);
			حرر(map);
			map = اگلا;
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
	بنانشانی صحيح i;

	صحيح j;

	أكد(c != عدم);

	إذا (c->imagemaps == عدم)
		أرجع;

	لكل (i = 0; i != HASH_ناپ; i++) {
		بنية imagemap *map;
		بنية mapentry *اندراج;

		map = c->imagemaps[i];
		طالما (map != عدم) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->کلید);

			لكل (اندراج = map->list; اندراج; اندراج = اندراج->اگلا) {
				بدل (اندراج->type) {
				حالة IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(اندراج->url));
					اكسر;
				حالة IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(اندراج->url),
					      اندراج->bounds.rect.x0,
					      اندراج->bounds.rect.y0,
					      اندراج->bounds.rect.x1,
					      اندراج->bounds.rect.y1);
					اكسر;
				حالة IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(اندراج->url),
					      اندراج->bounds.circle.x,
					      اندراج->bounds.circle.y,
					      اندراج->bounds.circle.r);
					اكسر;
				حالة IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(اندراج->url));
					لكل (j = 0; j != اندراج->bounds.poly.عدد;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحيح)اندراج->bounds.poly.xcoords[j],
							(صحيح)اندراج->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					اكسر;
				}
			}
			map = map->اگلا;
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
		   dom_گرہ *n,
		   nsurl *ادھار_url,
		   بنية mapentry **اندراج,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = عدم, *نشانہ = عدم, *shape = عدم;
	dom_string *coords = عدم;
	بنية mapentry *new_map, *عارضی;
	bool ret = true;

	إذا (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		إذا ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			اذهب ok_باہرلا;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	إذا (exc != DOM_NO_ERR || href == عدم) {
		/* No href="" attribute, skip this element */
		اذهب ok_باہرلا;
	}

	exc = dom_element_get_attribute(n, corestring_dom_نشانہ, &نشانہ);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_باہرلا;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	إذا (exc != DOM_NO_ERR) {
		اذهب ok_باہرلا;
	}

	/* If there's no shape, we default to rectangles */
	إذا (shape == عدم)
		shape = dom_string_ref(corestring_dom_rect);

	إذا (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		إذا (exc != DOM_NO_ERR || coords == عدم) {
			اذهب ok_باہرلا;
		}
	}

	new_map = صفر_حافظہ_دیو(1, حجم(*new_map));
	إذا (new_map == عدم) {
		اذهب bad_باہرلا;
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
		اذهب bad_باہرلا;

	إذا (box_کڈھو_link(c, href, ادھار_url, &new_map->url) == false)
		اذهب bad_باہرلا;

	إذا (new_map->url == عدم) {
		/* non-fatal error -> ignore this */
		اذهب ok_free_map_باہرلا;
	}

	إذا (نشانہ != عدم) {
		/* Copy target dom string into the map data */
		new_map->نشانہ = حافظہ_دیو(dom_string_byte_لمائی(نشانہ) + 1);
		إذا (new_map->نشانہ == عدم)
			اذهب bad_باہرلا;

		حافظہ_نقل(new_map->نشانہ,
		       dom_string_انکڑے(نشانہ),
		       dom_string_byte_لمائی(نشانہ));

		new_map->نشانہ[dom_string_byte_لمائی(نشانہ)] = 0;
	}

	إذا (new_map->type != IMAGEMAP_DEFAULT) {
		صحيح x, y;
		عائم *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_انکڑے(coords), ",");
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
					اذهب bad_باہرلا;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					عدد * حجم(عائم));
				إذا (ycoords == عدم) {
					اذهب bad_باہرلا;
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

	new_map->اگلا = عدم;

	إذا (*اندراج) {
		/* add to END of list */
		لكل (عارضی = (*اندراج); عارضی->اگلا != عدم; عارضی = عارضی->اگلا)
			;
		عارضی->اگلا = new_map;
	} وإلا {
		(*اندراج) = new_map;
	}

	/* All good, linked in, let's clean up */
	اذهب ok_باہرلا;

bad_باہرلا:
	ret = false;
ok_free_map_باہرلا:
	إذا (new_map != عدم) {
		إذا (new_map->url != عدم)
			nsurl_unref(new_map->url);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != عدم)
			حرر(new_map->bounds.poly.ycoords);
		إذا (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != عدم)
			حرر(new_map->bounds.poly.xcoords);
		إذا (new_map->نشانہ != عدم)
			حرر(new_map->نشانہ);

		حرر(new_map);
	}
ok_باہرلا:
	إذا (href != عدم)
		dom_string_unref(href);
	إذا (نشانہ != عدم)
		dom_string_unref(نشانہ);
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
imagemap_کڈھو_map_entries(dom_گرہ *گرہ, html_content *c,
			     بنية mapentry **اندراج, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بنانشانی لما ent;
	uint32_t ٹیگ_گنتی;

	exc = dom_element_get_elements_by_ٹیگ_name(گرہ, tname, &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	exc = dom_nodelist_get_لمائی(nlist, &ٹیگ_گنتی);
	إذا (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		أرجع false;
	}

	لكل (ent = 0; ent < ٹیگ_گنتی; ++ent) {
		dom_گرہ *subnode;

		exc = dom_nodelist_شے(nlist, ent, &subnode);
		إذا (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		إذا (imagemap_addtolist(c, subnode, c->ادھار_url,
				       اندراج, tname) == false) {
			dom_گرہ_unref(subnode);
			dom_nodelist_unref(nlist);
			أرجع false;
		}
		dom_گرہ_unref(subnode);
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
ساكن bool imagemap_کڈھو_map(dom_گرہ *گرہ, html_content *c,
		بنية mapentry **اندراج)
{
	إذا (imagemap_کڈھو_map_entries(گرہ, c, اندراج,
			corestring_dom_area) == false)
		أرجع false;
	أرجع imagemap_کڈھو_map_entries(گرہ, c, اندراج,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_کڈھو(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	بنانشانی لما mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_ٹیگ_name(c->document,
						    corestring_dom_map,
						    &nlist);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	exc = dom_nodelist_get_لمائی(nlist, &maybe_maps);
	إذا (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		اذهب باہرلا_nlist;
	}

	لكل (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_گرہ *گرہ;
		dom_string *name;
		exc = dom_nodelist_شے(nlist, mapnr, &گرہ);
		إذا (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			اذهب باہرلا_nlist;
		}

		exc = dom_element_get_attribute(گرہ, corestring_dom_id,
						&name);
		إذا (exc != DOM_NO_ERR) {
			dom_گرہ_unref(گرہ);
			ret = NSERROR_DOM;
			اذهب باہرلا_nlist;
		}

		إذا (name == عدم) {
			exc = dom_element_get_attribute(گرہ,
							corestring_dom_name,
							&name);
			إذا (exc != DOM_NO_ERR) {
				dom_گرہ_unref(گرہ);
				ret = NSERROR_DOM;
				اذهب باہرلا_nlist;
			}
		}

		إذا (name != عدم) {
			بنية mapentry *اندراج = عدم;
			إذا (imagemap_کڈھو_map(گرہ, c, &اندراج) == false) {
				إذا (اندراج != عدم) {
					imagemap_freelist(اندراج);
				}

				dom_string_unref(name);
				dom_گرہ_unref(گرہ);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب باہرلا_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			إذا ((اندراج != عدم) &&
			    (imagemap_جوڑو(c, name, اندراج) == false)) {
				imagemap_freelist(اندراج);

				dom_string_unref(name);
				dom_گرہ_unref(گرہ);
				ret = NSERROR_NOMEM; /** @todo check this */
				اذهب باہرلا_nlist;
			}
		}

		dom_string_unref(name);
		dom_گرہ_unref(گرہ);
	}

باہرلا_nlist:

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
imagemap_نقطہ_وچ_poly(صحيح عدد, عائم *xpt, عائم *ypt, بنانشانی لما x,
		بنانشانی لما y, بنانشانی لما click_x,	بنانشانی لما click_y)
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
nsurl *imagemap_get(بنية html_content *c, ثابت حرف *کلید,
		بنانشانی لما x, بنانشانی لما y,
		بنانشانی لما click_x, بنانشانی لما click_y,
		ثابت حرف **نشانہ)
{
	بنانشانی صحيح تھاں = 0;
	بنية imagemap *map;
	بنية mapentry *اندراج;
	بنانشانی لما cx, cy;

	أكد(c != عدم);

	إذا (کلید == عدم)
		أرجع عدم;

	إذا (c->imagemaps == عدم)
		أرجع عدم;

	تھاں = imagemap_hash(کلید);

	لكل (map = c->imagemaps[تھاں]; map != عدم; map = map->اگلا) {
		إذا (map->کلید != عدم && strcasecmp(map->کلید, کلید) == 0)
			اكسر;
	}

	إذا (map == عدم || map->list == عدم)
		أرجع عدم;

	لكل (اندراج = map->list; اندراج; اندراج = اندراج->اگلا) {
		بدل (اندراج->type) {
		حالة IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			إذا (نشانہ)
				*نشانہ = اندراج->نشانہ;
			أرجع اندراج->url;
			اكسر;
		حالة IMAGEMAP_RECT:
			إذا (click_x >= x + اندراج->bounds.rect.x0 &&
				    click_x <= x + اندراج->bounds.rect.x1 &&
				    click_y >= y + اندراج->bounds.rect.y0 &&
				    click_y <= y + اندراج->bounds.rect.y1) {
				إذا (نشانہ)
					*نشانہ = اندراج->نشانہ;
				أرجع اندراج->url;
			}
			اكسر;
		حالة IMAGEMAP_CIRCLE:
			cx = x + اندراج->bounds.circle.x - click_x;
			cy = y + اندراج->bounds.circle.y - click_y;
			إذا ((cx * cx + cy * cy) <=
				(بنانشانی لما) (اندراج->bounds.circle.r *
					اندراج->bounds.circle.r)) {
				إذا (نشانہ)
					*نشانہ = اندراج->نشانہ;
				أرجع اندراج->url;
			}
			اكسر;
		حالة IMAGEMAP_POLY:
			إذا (imagemap_نقطہ_وچ_poly(اندراج->bounds.poly.عدد,
					اندراج->bounds.poly.xcoords,
					اندراج->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				إذا (نشانہ)
					*نشانہ = اندراج->نشانہ;
				أرجع اندراج->url;
			}
			اكسر;
		}
	}

	إذا (نشانہ)
		*نشانہ = عدم;

	أرجع عدم;
}
