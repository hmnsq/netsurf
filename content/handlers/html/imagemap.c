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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>

#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/corestrings.h"
#شامل "content/content_protected.h"
#شامل "content/hlcache.h"

#شامل "html/box.h"
#شامل "html/box_construct.h"
#شامل "html/private.h"
#شامل "html/imagemap.h"

#تعریف_کرو HASH_سائز 31 /* fixed size hash table */

قسمتعریف شمار {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_اندراج_type;

ساخت mapentry {
	imagemap_اندراج_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	حرف *ہدف;			/**< target frame (if any) */
	اتحاد {
		ساخت {
			صحیح x;		/**< x coordinate of centre */
			صحیح y;		/**< y coordinate of center */
			صحیح r;		/**< radius of circle */
		} circle;
		ساخت {
			صحیح x0;		/**< left hand edge */
			صحیح y0;		/**< top edge */
			صحیح x1;		/**< right hand edge */
			صحیح y1;		/**< bottom edge */
		} rect;
		ساخت {
			صحیح عدد;	/**< number of points */
			اعشاری *xcoords;	/**< x coordinates */
			اعشاری *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	ساخت mapentry *اگلا;		/**< next entry in list */
};

ساخت imagemap {
	حرف *کلید;		/**< key for this entry */
	ساخت mapentry *list;	/**< pointer to linked list of entries */
	ساخت imagemap *اگلا;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
ساکن bool imagemap_create(html_content *c)
{
	أكد(c != تہی);

	اگر (c->imagemaps == تہی) {
		c->imagemaps = صفر_حافظہ_دو(HASH_سائز, حجم(ساخت imagemap *));
		اگر (c->imagemaps == تہی) {
			واپس false;
		}
	}

	واپس true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
ساکن غیرعلامتی صحیح imagemap_hash(ثابت حرف *کلید)
{
	غیرعلامتی صحیح z = 0;

	اگر (کلید == 0) واپس 0;

	برائے (; *کلید != 0; کلید++) {
		z += *کلید & 0x1f;
	}

	واپس (z % (HASH_سائز - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
ساکن bool
imagemap_جوڑو(html_content *c, dom_string *کلید, ساخت mapentry *list)
{
	ساخت imagemap *map;
	غیرعلامتی صحیح خانہ;

	أكد(c != تہی);
	أكد(کلید != تہی);
	أكد(list != تہی);

	اگر (imagemap_create(c) == false)
		واپس false;

	map = صفر_حافظہ_دو(1, حجم(*map));
	اگر (map == تہی)
		واپس false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->کلید = strdup(dom_string_ڈیٹا(کلید));
	اگر (map->کلید == تہی) {
		آزاد(map);
		واپس false;
	}

	map->list = list;

	خانہ = imagemap_hash(map->کلید);

	map->اگلا = c->imagemaps[خانہ];
	c->imagemaps[خانہ] = map;

	واپس true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
ساکن خالی imagemap_freelist(ساخت mapentry *list)
{
	ساخت mapentry *اندراج_2, *پچھلا;

	أكد(list != تہی);

	اندراج_2 = list;

	جب (اندراج_2 != تہی) {
		پچھلا = اندراج_2;

		nsurl_unref(اندراج_2->url);

		اگر (اندراج_2->ہدف)
			آزاد(اندراج_2->ہدف);

		اگر (اندراج_2->type == IMAGEMAP_POLY) {
			آزاد(اندراج_2->bounds.poly.xcoords);
			آزاد(اندراج_2->bounds.poly.ycoords);
		}

		اندراج_2 = اندراج_2->اگلا;
		آزاد(پچھلا);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
خالی imagemap_destroy(html_content *c)
{
	غیرعلامتی صحیح i;

	أكد(c != تہی);

	/* no imagemaps -> return */
	اگر (c->imagemaps == تہی)
		واپس;

	برائے (i = 0; i != HASH_سائز; i++) {
		ساخت imagemap *map, *اگلا;

		map = c->imagemaps[i];
		جب (map != تہی) {
			اگلا = map->اگلا;
			imagemap_freelist(map->list);
			آزاد(map->کلید);
			آزاد(map);
			map = اگلا;
		}
	}

	آزاد(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
خالی imagemap_dump(html_content *c)
{
	غیرعلامتی صحیح i;

	صحیح j;

	أكد(c != تہی);

	اگر (c->imagemaps == تہی)
		واپس;

	برائے (i = 0; i != HASH_سائز; i++) {
		ساخت imagemap *map;
		ساخت mapentry *اندراج_2;

		map = c->imagemaps[i];
		جب (map != تہی) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->کلید);

			برائے (اندراج_2 = map->list; اندراج_2; اندراج_2 = اندراج_2->اگلا) {
				انتخاب (اندراج_2->type) {
				حالت IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(اندراج_2->url));
					توڑو;
				حالت IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(اندراج_2->url),
					      اندراج_2->bounds.rect.x0,
					      اندراج_2->bounds.rect.y0,
					      اندراج_2->bounds.rect.x1,
					      اندراج_2->bounds.rect.y1);
					توڑو;
				حالت IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(اندراج_2->url),
					      اندراج_2->bounds.circle.x,
					      اندراج_2->bounds.circle.y,
					      اندراج_2->bounds.circle.r);
					توڑو;
				حالت IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(اندراج_2->url));
					برائے (j = 0; j != اندراج_2->bounds.poly.عدد;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(صحیح)اندراج_2->bounds.poly.xcoords[j],
							(صحیح)اندراج_2->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					توڑو;
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
ساکن bool
imagemap_addtolist(ثابت ساخت html_content *c,
		   dom_گرہ *n,
		   nsurl *بنیاد_url,
		   ساخت mapentry **اندراج_2,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = تہی, *ہدف = تہی, *shape = تہی;
	dom_string *coords = تہی;
	ساخت mapentry *new_map, *عارضی;
	bool ret = true;

	اگر (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		اگر ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			جاؤ ok_برآمد;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	اگر (exc != DOM_NO_ERR || href == تہی) {
		/* No href="" attribute, skip this element */
		جاؤ ok_برآمد;
	}

	exc = dom_element_get_attribute(n, corestring_dom_ہدف, &ہدف);
	اگر (exc != DOM_NO_ERR) {
		جاؤ ok_برآمد;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	اگر (exc != DOM_NO_ERR) {
		جاؤ ok_برآمد;
	}

	/* If there's no shape, we default to rectangles */
	اگر (shape == تہی)
		shape = dom_string_ref(corestring_dom_rect);

	اگر (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		اگر (exc != DOM_NO_ERR || coords == تہی) {
			جاؤ ok_برآمد;
		}
	}

	new_map = صفر_حافظہ_دو(1, حجم(*new_map));
	اگر (new_map == تہی) {
		جاؤ bad_برآمد;
	}

	اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	ورنہ اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	ورنہ اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	ورنہ اگر (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	ورنہ
		جاؤ bad_برآمد;

	اگر (box_نکالو_link(c, href, بنیاد_url, &new_map->url) == false)
		جاؤ bad_برآمد;

	اگر (new_map->url == تہی) {
		/* non-fatal error -> ignore this */
		جاؤ ok_free_map_برآمد;
	}

	اگر (ہدف != تہی) {
		/* Copy target dom string into the map data */
		new_map->ہدف = حافظہ_دو(dom_string_byte_لمبائی(ہدف) + 1);
		اگر (new_map->ہدف == تہی)
			جاؤ bad_برآمد;

		حافظہ_نقل(new_map->ہدف,
		       dom_string_ڈیٹا(ہدف),
		       dom_string_byte_لمبائی(ہدف));

		new_map->ہدف[dom_string_byte_لمبائی(ہدف)] = 0;
	}

	اگر (new_map->type != IMAGEMAP_DEFAULT) {
		صحیح x, y;
		اعشاری *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		حرف *val = strtok((حرف *)dom_string_ڈیٹا(coords), ",");
		صحیح عدد = 1;

		انتخاب (new_map->type) {
		حالت IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			جب (val != تہی && عدد <= 4) {
				انتخاب (عدد) {
				حالت 1:
					new_map->bounds.rect.x0 = atoi(val);
					توڑو;
				حالت 2:
					new_map->bounds.rect.y0 = atoi(val);
					توڑو;
				حالت 3:
					new_map->bounds.rect.x1 = atoi(val);
					توڑو;
				حالت 4:
					new_map->bounds.rect.y1 = atoi(val);
					توڑو;
				}

				عدد++;
				val = strtok(تہی, ",");
			}
			توڑو;
		حالت IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			جب (val != تہی && عدد <= 3) {
				انتخاب (عدد) {
				حالت 1:
					new_map->bounds.circle.x = atoi(val);
					توڑو;
				حالت 2:
					new_map->bounds.circle.y = atoi(val);
					توڑو;
				حالت 3:
					new_map->bounds.circle.r = atoi(val);
					توڑو;
				}

				عدد++;
				val = strtok(تہی, ",");
			}
			توڑو;
		حالت IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = تہی;
			new_map->bounds.poly.ycoords = تہی;

			جب (val != تہی) {
				x = atoi(val);

				val = strtok(تہی, ",");
				اگر (val == تہی)
					توڑو;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						عدد * حجم(اعشاری));
				اگر (xcoords == تہی) {
					جاؤ bad_برآمد;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					عدد * حجم(اعشاری));
				اگر (ycoords == تہی) {
					جاؤ bad_برآمد;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[عدد - 1] = x;
				new_map->bounds.poly.ycoords[عدد - 1] = y;

				عدد++;
				val = strtok(تہی, ",");
			}

			new_map->bounds.poly.عدد = عدد - 1;

			توڑو;
		طےشدہ:
			توڑو;
		}
	}

	new_map->اگلا = تہی;

	اگر (*اندراج_2) {
		/* add to END of list */
		برائے (عارضی = (*اندراج_2); عارضی->اگلا != تہی; عارضی = عارضی->اگلا)
			;
		عارضی->اگلا = new_map;
	} ورنہ {
		(*اندراج_2) = new_map;
	}

	/* All good, linked in, let's clean up */
	جاؤ ok_برآمد;

bad_برآمد:
	ret = false;
ok_free_map_برآمد:
	اگر (new_map != تہی) {
		اگر (new_map->url != تہی)
			nsurl_unref(new_map->url);
		اگر (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != تہی)
			آزاد(new_map->bounds.poly.ycoords);
		اگر (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != تہی)
			آزاد(new_map->bounds.poly.xcoords);
		اگر (new_map->ہدف != تہی)
			آزاد(new_map->ہدف);

		آزاد(new_map);
	}
ok_برآمد:
	اگر (href != تہی)
		dom_string_unref(href);
	اگر (ہدف != تہی)
		dom_string_unref(ہدف);
	اگر (shape != تہی)
		dom_string_unref(shape);
	اگر (coords != تہی)
		dom_string_unref(coords);

	واپس ret;
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
ساکن bool
imagemap_نکالو_map_entries(dom_گرہ *گرہ, html_content *c,
			     ساخت mapentry **اندراج_2, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	غیرعلامتی طویل ent;
	uint32_t لیبل_گنتی;

	exc = dom_element_get_elements_by_لیبل_name(گرہ, tname, &nlist);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	exc = dom_nodelist_get_لمبائی(nlist, &لیبل_گنتی);
	اگر (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		واپس false;
	}

	برائے (ent = 0; ent < لیبل_گنتی; ++ent) {
		dom_گرہ *subnode;

		exc = dom_nodelist_شے(nlist, ent, &subnode);
		اگر (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			واپس false;
		}
		اگر (imagemap_addtolist(c, subnode, c->بنیاد_url,
				       اندراج_2, tname) == false) {
			dom_گرہ_unref(subnode);
			dom_nodelist_unref(nlist);
			واپس false;
		}
		dom_گرہ_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	واپس true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
ساکن bool imagemap_نکالو_map(dom_گرہ *گرہ, html_content *c,
		ساخت mapentry **اندراج_2)
{
	اگر (imagemap_نکالو_map_entries(گرہ, c, اندراج_2,
			corestring_dom_area) == false)
		واپس false;
	واپس imagemap_نکالو_map_entries(گرہ, c, اندراج_2,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_نکالو(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	غیرعلامتی طویل mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_لیبل_name(c->document,
						    corestring_dom_map,
						    &nlist);
	اگر (exc != DOM_NO_ERR) {
		واپس NSERROR_DOM;
	}

	exc = dom_nodelist_get_لمبائی(nlist, &maybe_maps);
	اگر (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		جاؤ برآمد_nlist;
	}

	برائے (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_گرہ *گرہ;
		dom_string *name;
		exc = dom_nodelist_شے(nlist, mapnr, &گرہ);
		اگر (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			جاؤ برآمد_nlist;
		}

		exc = dom_element_get_attribute(گرہ, corestring_dom_id,
						&name);
		اگر (exc != DOM_NO_ERR) {
			dom_گرہ_unref(گرہ);
			ret = NSERROR_DOM;
			جاؤ برآمد_nlist;
		}

		اگر (name == تہی) {
			exc = dom_element_get_attribute(گرہ,
							corestring_dom_name,
							&name);
			اگر (exc != DOM_NO_ERR) {
				dom_گرہ_unref(گرہ);
				ret = NSERROR_DOM;
				جاؤ برآمد_nlist;
			}
		}

		اگر (name != تہی) {
			ساخت mapentry *اندراج_2 = تہی;
			اگر (imagemap_نکالو_map(گرہ, c, &اندراج_2) == false) {
				اگر (اندراج_2 != تہی) {
					imagemap_freelist(اندراج_2);
				}

				dom_string_unref(name);
				dom_گرہ_unref(گرہ);
				ret = NSERROR_NOMEM; /** @todo check this */
				جاؤ برآمد_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			اگر ((اندراج_2 != تہی) &&
			    (imagemap_جوڑو(c, name, اندراج_2) == false)) {
				imagemap_freelist(اندراج_2);

				dom_string_unref(name);
				dom_گرہ_unref(گرہ);
				ret = NSERROR_NOMEM; /** @todo check this */
				جاؤ برآمد_nlist;
			}
		}

		dom_string_unref(name);
		dom_گرہ_unref(گرہ);
	}

برآمد_nlist:

	dom_nodelist_unref(nlist);

	واپس ret;
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
ساکن صحیح
imagemap_نقطہ_اندر_poly(صحیح عدد, اعشاری *xpt, اعشاری *ypt, غیرعلامتی طویل x,
		غیرعلامتی طویل y, غیرعلامتی طویل click_x,	غیرعلامتی طویل click_y)
{
	صحیح i, j, c = 0;

	أكد(xpt != تہی);
	أكد(ypt != تہی);

	برائے (i = 0, j = عدد - 1; i < عدد; j = i++) {
		اگر ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	واپس c;
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
nsurl *imagemap_get(ساخت html_content *c, ثابت حرف *کلید,
		غیرعلامتی طویل x, غیرعلامتی طویل y,
		غیرعلامتی طویل click_x, غیرعلامتی طویل click_y,
		ثابت حرف **ہدف)
{
	غیرعلامتی صحیح خانہ = 0;
	ساخت imagemap *map;
	ساخت mapentry *اندراج_2;
	غیرعلامتی طویل cx, cy;

	أكد(c != تہی);

	اگر (کلید == تہی)
		واپس تہی;

	اگر (c->imagemaps == تہی)
		واپس تہی;

	خانہ = imagemap_hash(کلید);

	برائے (map = c->imagemaps[خانہ]; map != تہی; map = map->اگلا) {
		اگر (map->کلید != تہی && strcasecmp(map->کلید, کلید) == 0)
			توڑو;
	}

	اگر (map == تہی || map->list == تہی)
		واپس تہی;

	برائے (اندراج_2 = map->list; اندراج_2; اندراج_2 = اندراج_2->اگلا) {
		انتخاب (اندراج_2->type) {
		حالت IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			اگر (ہدف)
				*ہدف = اندراج_2->ہدف;
			واپس اندراج_2->url;
			توڑو;
		حالت IMAGEMAP_RECT:
			اگر (click_x >= x + اندراج_2->bounds.rect.x0 &&
				    click_x <= x + اندراج_2->bounds.rect.x1 &&
				    click_y >= y + اندراج_2->bounds.rect.y0 &&
				    click_y <= y + اندراج_2->bounds.rect.y1) {
				اگر (ہدف)
					*ہدف = اندراج_2->ہدف;
				واپس اندراج_2->url;
			}
			توڑو;
		حالت IMAGEMAP_CIRCLE:
			cx = x + اندراج_2->bounds.circle.x - click_x;
			cy = y + اندراج_2->bounds.circle.y - click_y;
			اگر ((cx * cx + cy * cy) <=
				(غیرعلامتی طویل) (اندراج_2->bounds.circle.r *
					اندراج_2->bounds.circle.r)) {
				اگر (ہدف)
					*ہدف = اندراج_2->ہدف;
				واپس اندراج_2->url;
			}
			توڑو;
		حالت IMAGEMAP_POLY:
			اگر (imagemap_نقطہ_اندر_poly(اندراج_2->bounds.poly.عدد,
					اندراج_2->bounds.poly.xcoords,
					اندراج_2->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				اگر (ہدف)
					*ہدف = اندراج_2->ہدف;
				واپس اندراج_2->url;
			}
			توڑو;
		}
	}

	اگر (ہدف)
		*ہدف = تہی;

	واپس تہی;
}
