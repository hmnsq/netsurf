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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>

#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/hlcache.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_construct.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/imagemap.h"

#నిర్వచించు HASH_కొలత 31 /* fixed size hash table */

రకనిర్వచనం గణన {
	IMAGEMAP_DEFAULT,
	IMAGEMAP_RECT,
	IMAGEMAP_CIRCLE,
	IMAGEMAP_POLY
} imagemap_నమోదు_type;

నిర్మాణం mapentry {
	imagemap_నమోదు_type type;	/**< type of shape */
	nsurl *url;			/**< absolute url to go to */
	అక్షరం *లక్ష్యం;			/**< target frame (if any) */
	సంఘం {
		నిర్మాణం {
			పూర్ణాంకం x;		/**< x coordinate of centre */
			పూర్ణాంకం y;		/**< y coordinate of center */
			పూర్ణాంకం r;		/**< radius of circle */
		} circle;
		నిర్మాణం {
			పూర్ణాంకం x0;		/**< left hand edge */
			పూర్ణాంకం y0;		/**< top edge */
			పూర్ణాంకం x1;		/**< right hand edge */
			పూర్ణాంకం y1;		/**< bottom edge */
		} rect;
		నిర్మాణం {
			పూర్ణాంకం అంకె;	/**< number of points */
			దశాంశం *xcoords;	/**< x coordinates */
			దశాంశం *ycoords;	/**< y coordinates */
		} poly;
	} bounds;
	నిర్మాణం mapentry *తదుపరి;		/**< next entry in list */
};

నిర్మాణం imagemap {
	అక్షరం *కీలకం;		/**< key for this entry */
	నిర్మాణం mapentry *list;	/**< pointer to linked list of entries */
	నిర్మాణం imagemap *తదుపరి;	/**< next entry in this hash chain */
};

/**
 * Create hashtable of imagemaps
 *
 * \param c The containing content
 * \return true on success, false otherwise
 */
స్థిర bool imagemap_create(html_content *c)
{
	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->imagemaps == ఖాళీ) {
		c->imagemaps = సున్నా_జ్ఞాపకం_ఇవ్వు(HASH_కొలత, పరిమాణం(నిర్మాణం imagemap *));
		ఒకవేళ (c->imagemaps == ఖాళీ) {
			తిరిగి false;
		}
	}

	తిరిగి true;
}

/**
 * Hash function.
 *
 * \param key The key to hash.
 * \return The hashed value.
 */
స్థిర గుర్తులేని పూర్ణాంకం imagemap_hash(స్థిరరాశి అక్షరం *కీలకం)
{
	గుర్తులేని పూర్ణాంకం z = 0;

	ఒకవేళ (కీలకం == 0) తిరిగి 0;

	కొరకు (; *కీలకం != 0; కీలకం++) {
		z += *కీలకం & 0x1f;
	}

	తిరిగి (z % (HASH_కొలత - 1)) + 1;
}

/**
 * Add an imagemap to the hashtable, creating it if it doesn't exist
 *
 * \param c The containing content
 * \param key The name of the imagemap
 * \param list List of map regions
 * \return true on succes, false otherwise
 */
స్థిర bool
imagemap_కూడు(html_content *c, dom_string *కీలకం, నిర్మాణం mapentry *list)
{
	నిర్మాణం imagemap *map;
	గుర్తులేని పూర్ణాంకం స్థానం;

	ప్రతిజ్ఞా(c != ఖాళీ);
	ప్రతిజ్ఞా(కీలకం != ఖాళీ);
	ప్రతిజ్ఞా(list != ఖాళీ);

	ఒకవేళ (imagemap_create(c) == false)
		తిరిగి false;

	map = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*map));
	ఒకవేళ (map == ఖాళీ)
		తిరిగి false;

	/* \todo Stop relying on NULL termination of dom_string */
	map->కీలకం = strdup(dom_string_దత్తాంశం(కీలకం));
	ఒకవేళ (map->కీలకం == ఖాళీ) {
		విడుదల(map);
		తిరిగి false;
	}

	map->list = list;

	స్థానం = imagemap_hash(map->కీలకం);

	map->తదుపరి = c->imagemaps[స్థానం];
	c->imagemaps[స్థానం] = map;

	తిరిగి true;
}

/**
 * Free list of imagemap entries
 *
 * \param list Pointer to head of list
 */
స్థిర శూన్య imagemap_freelist(నిర్మాణం mapentry *list)
{
	నిర్మాణం mapentry *నమోదు_2, *prev;

	ప్రతిజ్ఞా(list != ఖాళీ);

	నమోదు_2 = list;

	అయితే (నమోదు_2 != ఖాళీ) {
		prev = నమోదు_2;

		nsurl_unref(నమోదు_2->url);

		ఒకవేళ (నమోదు_2->లక్ష్యం)
			విడుదల(నమోదు_2->లక్ష్యం);

		ఒకవేళ (నమోదు_2->type == IMAGEMAP_POLY) {
			విడుదల(నమోదు_2->bounds.poly.xcoords);
			విడుదల(నమోదు_2->bounds.poly.ycoords);
		}

		నమోదు_2 = నమోదు_2->తదుపరి;
		విడుదల(prev);
	}
}

/**
 * Destroy hashtable of imagemaps
 *
 * \param c The containing content
 */
శూన్య imagemap_destroy(html_content *c)
{
	గుర్తులేని పూర్ణాంకం i;

	ప్రతిజ్ఞా(c != ఖాళీ);

	/* no imagemaps -> return */
	ఒకవేళ (c->imagemaps == ఖాళీ)
		తిరిగి;

	కొరకు (i = 0; i != HASH_కొలత; i++) {
		నిర్మాణం imagemap *map, *తదుపరి;

		map = c->imagemaps[i];
		అయితే (map != ఖాళీ) {
			తదుపరి = map->తదుపరి;
			imagemap_freelist(map->list);
			విడుదల(map->కీలకం);
			విడుదల(map);
			map = తదుపరి;
		}
	}

	విడుదల(c->imagemaps);
}

/**
 * Dump imagemap data to the log
 *
 * \param c The containing content
 */
శూన్య imagemap_dump(html_content *c)
{
	గుర్తులేని పూర్ణాంకం i;

	పూర్ణాంకం j;

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->imagemaps == ఖాళీ)
		తిరిగి;

	కొరకు (i = 0; i != HASH_కొలత; i++) {
		నిర్మాణం imagemap *map;
		నిర్మాణం mapentry *నమోదు_2;

		map = c->imagemaps[i];
		అయితే (map != ఖాళీ) {
			NSLOG(netsurf, INFO, "Imagemap: %s", map->కీలకం);

			కొరకు (నమోదు_2 = map->list; నమోదు_2; నమోదు_2 = నమోదు_2->తదుపరి) {
				ఎంపిక (నమోదు_2->type) {
				స్థితి IMAGEMAP_DEFAULT:
					NSLOG(netsurf, INFO, "\tDefault: %s",
					      nsurl_access(నమోదు_2->url));
					ఆపు;
				స్థితి IMAGEMAP_RECT:
					NSLOG(netsurf, INFO,
					      "\tRectangle: %s: [(%d,%d),(%d,%d)]",
					      nsurl_access(నమోదు_2->url),
					      నమోదు_2->bounds.rect.x0,
					      నమోదు_2->bounds.rect.y0,
					      నమోదు_2->bounds.rect.x1,
					      నమోదు_2->bounds.rect.y1);
					ఆపు;
				స్థితి IMAGEMAP_CIRCLE:
					NSLOG(netsurf, INFO,
					      "\tCircle: %s: [(%d,%d),%d]",
					      nsurl_access(నమోదు_2->url),
					      నమోదు_2->bounds.circle.x,
					      నమోదు_2->bounds.circle.y,
					      నమోదు_2->bounds.circle.r);
					ఆపు;
				స్థితి IMAGEMAP_POLY:
					NSLOG(netsurf, INFO,
					      "\tPolygon: %s:",
					      nsurl_access(నమోదు_2->url));
					కొరకు (j = 0; j != నమోదు_2->bounds.poly.అంకె;
							j++) {
						fprintf(stderr, "(%d,%d) ",
							(పూర్ణాంకం)నమోదు_2->bounds.poly.xcoords[j],
							(పూర్ణాంకం)నమోదు_2->bounds.poly.ycoords[j]);
					}
					fprintf(stderr,"\n");
					ఆపు;
				}
			}
			map = map->తదుపరి;
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
స్థిర bool
imagemap_addtolist(స్థిరరాశి నిర్మాణం html_content *c,
		   dom_కణుపు *n,
		   nsurl *పునాది_url,
		   నిర్మాణం mapentry **నమోదు_2,
		   dom_string *tagtype)
{
	dom_exception exc;
	dom_string *href = ఖాళీ, *లక్ష్యం = ఖాళీ, *shape = ఖాళీ;
	dom_string *coords = ఖాళీ;
	నిర్మాణం mapentry *new_map, *తాత్కాలిక;
	bool ret = true;

	ఒకవేళ (dom_string_caseless_isequal(tagtype, corestring_dom_area)) {
		bool nohref = false;
		exc = dom_element_has_attribute(n,
				corestring_dom_nohref, &nohref);
		ఒకవేళ ((exc != DOM_NO_ERR) || nohref)
			/* Skip <area nohref="anything" /> */
			వెళ్ళు ok_బహిర్గతం;
	}

	exc = dom_element_get_attribute(n, corestring_dom_href, &href);
	ఒకవేళ (exc != DOM_NO_ERR || href == ఖాళీ) {
		/* No href="" attribute, skip this element */
		వెళ్ళు ok_బహిర్గతం;
	}

	exc = dom_element_get_attribute(n, corestring_dom_లక్ష్యం, &లక్ష్యం);
	ఒకవేళ (exc != DOM_NO_ERR) {
		వెళ్ళు ok_బహిర్గతం;
	}

	exc = dom_element_get_attribute(n, corestring_dom_shape, &shape);
	ఒకవేళ (exc != DOM_NO_ERR) {
		వెళ్ళు ok_బహిర్గతం;
	}

	/* If there's no shape, we default to rectangles */
	ఒకవేళ (shape == ఖాళీ)
		shape = dom_string_ref(corestring_dom_rect);

	ఒకవేళ (!dom_string_caseless_lwc_isequal(shape, corestring_lwc_default)) {
		/* If not 'default' and there's no 'coords' give up */
		exc = dom_element_get_attribute(n, corestring_dom_coords,
						&coords);
		ఒకవేళ (exc != DOM_NO_ERR || coords == ఖాళీ) {
			వెళ్ళు ok_బహిర్గతం;
		}
	}

	new_map = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*new_map));
	ఒకవేళ (new_map == ఖాళీ) {
		వెళ్ళు bad_బహిర్గతం;
	}

	ఒకవేళ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_rect) ||
	    dom_string_caseless_lwc_isequal(shape, corestring_lwc_rectangle))
		new_map->type = IMAGEMAP_RECT;
	లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_circle))
		new_map->type = IMAGEMAP_CIRCLE;
	లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_poly) ||
		 dom_string_caseless_lwc_isequal(shape, corestring_lwc_polygon))
		new_map->type = IMAGEMAP_POLY;
	లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(shape, corestring_lwc_default))
		new_map->type = IMAGEMAP_DEFAULT;
	లేకపోతే
		వెళ్ళు bad_బహిర్గతం;

	ఒకవేళ (box_వెలికితీయి_link(c, href, పునాది_url, &new_map->url) == false)
		వెళ్ళు bad_బహిర్గతం;

	ఒకవేళ (new_map->url == ఖాళీ) {
		/* non-fatal error -> ignore this */
		వెళ్ళు ok_free_map_బహిర్గతం;
	}

	ఒకవేళ (లక్ష్యం != ఖాళీ) {
		/* Copy target dom string into the map data */
		new_map->లక్ష్యం = జ్ఞాపకం_ఇవ్వు(dom_string_byte_నిడివి(లక్ష్యం) + 1);
		ఒకవేళ (new_map->లక్ష్యం == ఖాళీ)
			వెళ్ళు bad_బహిర్గతం;

		జ్ఞాపకం_నకలు(new_map->లక్ష్యం,
		       dom_string_దత్తాంశం(లక్ష్యం),
		       dom_string_byte_నిడివి(లక్ష్యం));

		new_map->లక్ష్యం[dom_string_byte_నిడివి(లక్ష్యం)] = 0;
	}

	ఒకవేళ (new_map->type != IMAGEMAP_DEFAULT) {
		పూర్ణాంకం x, y;
		దశాంశం *xcoords, *ycoords;
		/* coordinates are a comma-separated list of values */
		అక్షరం *val = strtok((అక్షరం *)dom_string_దత్తాంశం(coords), ",");
		పూర్ణాంకం అంకె = 1;

		ఎంపిక (new_map->type) {
		స్థితి IMAGEMAP_RECT:
			/* (left, top, right, bottom) */
			అయితే (val != ఖాళీ && అంకె <= 4) {
				ఎంపిక (అంకె) {
				స్థితి 1:
					new_map->bounds.rect.x0 = atoi(val);
					ఆపు;
				స్థితి 2:
					new_map->bounds.rect.y0 = atoi(val);
					ఆపు;
				స్థితి 3:
					new_map->bounds.rect.x1 = atoi(val);
					ఆపు;
				స్థితి 4:
					new_map->bounds.rect.y1 = atoi(val);
					ఆపు;
				}

				అంకె++;
				val = strtok(ఖాళీ, ",");
			}
			ఆపు;
		స్థితి IMAGEMAP_CIRCLE:
			/* (x, y, radius ) */
			అయితే (val != ఖాళీ && అంకె <= 3) {
				ఎంపిక (అంకె) {
				స్థితి 1:
					new_map->bounds.circle.x = atoi(val);
					ఆపు;
				స్థితి 2:
					new_map->bounds.circle.y = atoi(val);
					ఆపు;
				స్థితి 3:
					new_map->bounds.circle.r = atoi(val);
					ఆపు;
				}

				అంకె++;
				val = strtok(ఖాళీ, ",");
			}
			ఆపు;
		స్థితి IMAGEMAP_POLY:
			new_map->bounds.poly.xcoords = ఖాళీ;
			new_map->bounds.poly.ycoords = ఖాళీ;

			అయితే (val != ఖాళీ) {
				x = atoi(val);

				val = strtok(ఖాళీ, ",");
				ఒకవేళ (val == ఖాళీ)
					ఆపు;

				y = atoi(val);

				xcoords = realloc(new_map->bounds.poly.xcoords,
						అంకె * పరిమాణం(దశాంశం));
				ఒకవేళ (xcoords == ఖాళీ) {
					వెళ్ళు bad_బహిర్గతం;
				}
				new_map->bounds.poly.xcoords = xcoords;

				ycoords = realloc(new_map->bounds.poly.ycoords,
					అంకె * పరిమాణం(దశాంశం));
				ఒకవేళ (ycoords == ఖాళీ) {
					వెళ్ళు bad_బహిర్గతం;
				}
				new_map->bounds.poly.ycoords = ycoords;

				new_map->bounds.poly.xcoords[అంకె - 1] = x;
				new_map->bounds.poly.ycoords[అంకె - 1] = y;

				అంకె++;
				val = strtok(ఖాళీ, ",");
			}

			new_map->bounds.poly.అంకె = అంకె - 1;

			ఆపు;
		అప్రమేయ:
			ఆపు;
		}
	}

	new_map->తదుపరి = ఖాళీ;

	ఒకవేళ (*నమోదు_2) {
		/* add to END of list */
		కొరకు (తాత్కాలిక = (*నమోదు_2); తాత్కాలిక->తదుపరి != ఖాళీ; తాత్కాలిక = తాత్కాలిక->తదుపరి)
			;
		తాత్కాలిక->తదుపరి = new_map;
	} లేకపోతే {
		(*నమోదు_2) = new_map;
	}

	/* All good, linked in, let's clean up */
	వెళ్ళు ok_బహిర్గతం;

bad_బహిర్గతం:
	ret = false;
ok_free_map_బహిర్గతం:
	ఒకవేళ (new_map != ఖాళీ) {
		ఒకవేళ (new_map->url != ఖాళీ)
			nsurl_unref(new_map->url);
		ఒకవేళ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.ycoords != ఖాళీ)
			విడుదల(new_map->bounds.poly.ycoords);
		ఒకవేళ (new_map->type == IMAGEMAP_POLY &&
				new_map->bounds.poly.xcoords != ఖాళీ)
			విడుదల(new_map->bounds.poly.xcoords);
		ఒకవేళ (new_map->లక్ష్యం != ఖాళీ)
			విడుదల(new_map->లక్ష్యం);

		విడుదల(new_map);
	}
ok_బహిర్గతం:
	ఒకవేళ (href != ఖాళీ)
		dom_string_unref(href);
	ఒకవేళ (లక్ష్యం != ఖాళీ)
		dom_string_unref(లక్ష్యం);
	ఒకవేళ (shape != ఖాళీ)
		dom_string_unref(shape);
	ఒకవేళ (coords != ఖాళీ)
		dom_string_unref(coords);

	తిరిగి ret;
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
స్థిర bool
imagemap_వెలికితీయి_map_entries(dom_కణుపు *కణుపు, html_content *c,
			     నిర్మాణం mapentry **నమోదు_2, dom_string *tname)
{
	dom_nodelist *nlist;
	dom_exception exc;
	గుర్తులేని దీర్ఘ ent;
	uint32_t ట్యాగ్_లెక్క;

	exc = dom_element_get_elements_by_ట్యాగ్_name(కణుపు, tname, &nlist);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	exc = dom_nodelist_get_నిడివి(nlist, &ట్యాగ్_లెక్క);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_nodelist_unref(nlist);
		తిరిగి false;
	}

	కొరకు (ent = 0; ent < ట్యాగ్_లెక్క; ++ent) {
		dom_కణుపు *subnode;

		exc = dom_nodelist_అంశం(nlist, ent, &subnode);
		ఒకవేళ (exc != DOM_NO_ERR) {
			dom_nodelist_unref(nlist);
			తిరిగి false;
		}
		ఒకవేళ (imagemap_addtolist(c, subnode, c->పునాది_url,
				       నమోదు_2, tname) == false) {
			dom_కణుపు_unref(subnode);
			dom_nodelist_unref(nlist);
			తిరిగి false;
		}
		dom_కణుపు_unref(subnode);
	}

	dom_nodelist_unref(nlist);

	తిరిగి true;
}

/**
 * Extract an imagemap from html source
 *
 * \param node  XML node containing map
 * \param c     Content containing document
 * \param entry List of map entries
 * \return false on memory exhaustion, true otherwise
 */
స్థిర bool imagemap_వెలికితీయి_map(dom_కణుపు *కణుపు, html_content *c,
		నిర్మాణం mapentry **నమోదు_2)
{
	ఒకవేళ (imagemap_వెలికితీయి_map_entries(కణుపు, c, నమోదు_2,
			corestring_dom_area) == false)
		తిరిగి false;
	తిరిగి imagemap_వెలికితీయి_map_entries(కణుపు, c, నమోదు_2,
			corestring_dom_a);
}

/**
 * Extract all imagemaps from a document tree
 *
 * \param c The content to extract imagemaps from.
 * \return false on memory exhaustion, true otherwise
 */
nserror
imagemap_వెలికితీయి(html_content *c)
{
	dom_nodelist *nlist;
	dom_exception exc;
	గుర్తులేని దీర్ఘ mapnr;
	uint32_t maybe_maps;
	nserror ret = NSERROR_OK;

	exc = dom_document_get_elements_by_ట్యాగ్_name(c->document,
						    corestring_dom_map,
						    &nlist);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి NSERROR_DOM;
	}

	exc = dom_nodelist_get_నిడివి(nlist, &maybe_maps);
	ఒకవేళ (exc != DOM_NO_ERR) {
		ret = NSERROR_DOM;
		వెళ్ళు బహిర్గతం_nlist;
	}

	కొరకు (mapnr = 0; mapnr < maybe_maps; ++mapnr) {
		dom_కణుపు *కణుపు;
		dom_string *name;
		exc = dom_nodelist_అంశం(nlist, mapnr, &కణుపు);
		ఒకవేళ (exc != DOM_NO_ERR) {
			ret = NSERROR_DOM;
			వెళ్ళు బహిర్గతం_nlist;
		}

		exc = dom_element_get_attribute(కణుపు, corestring_dom_id,
						&name);
		ఒకవేళ (exc != DOM_NO_ERR) {
			dom_కణుపు_unref(కణుపు);
			ret = NSERROR_DOM;
			వెళ్ళు బహిర్గతం_nlist;
		}

		ఒకవేళ (name == ఖాళీ) {
			exc = dom_element_get_attribute(కణుపు,
							corestring_dom_name,
							&name);
			ఒకవేళ (exc != DOM_NO_ERR) {
				dom_కణుపు_unref(కణుపు);
				ret = NSERROR_DOM;
				వెళ్ళు బహిర్గతం_nlist;
			}
		}

		ఒకవేళ (name != ఖాళీ) {
			నిర్మాణం mapentry *నమోదు_2 = ఖాళీ;
			ఒకవేళ (imagemap_వెలికితీయి_map(కణుపు, c, &నమోదు_2) == false) {
				ఒకవేళ (నమోదు_2 != ఖాళీ) {
					imagemap_freelist(నమోదు_2);
				}

				dom_string_unref(name);
				dom_కణుపు_unref(కణుపు);
				ret = NSERROR_NOMEM; /** @todo check this */
				వెళ్ళు బహిర్గతం_nlist;
			}

			/* imagemap_extract_map may not extract anything,
			 * so entry can still be NULL here. This isn't an
			 * error as it just means that we've encountered
			 * an incorrectly defined <map>...</map> block
			 */
			ఒకవేళ ((నమోదు_2 != ఖాళీ) &&
			    (imagemap_కూడు(c, name, నమోదు_2) == false)) {
				imagemap_freelist(నమోదు_2);

				dom_string_unref(name);
				dom_కణుపు_unref(కణుపు);
				ret = NSERROR_NOMEM; /** @todo check this */
				వెళ్ళు బహిర్గతం_nlist;
			}
		}

		dom_string_unref(name);
		dom_కణుపు_unref(కణుపు);
	}

బహిర్గతం_nlist:

	dom_nodelist_unref(nlist);

	తిరిగి ret;
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
స్థిర పూర్ణాంకం
imagemap_బిందువు_లోపల_poly(పూర్ణాంకం అంకె, దశాంశం *xpt, దశాంశం *ypt, గుర్తులేని దీర్ఘ x,
		గుర్తులేని దీర్ఘ y, గుర్తులేని దీర్ఘ click_x,	గుర్తులేని దీర్ఘ click_y)
{
	పూర్ణాంకం i, j, c = 0;

	ప్రతిజ్ఞా(xpt != ఖాళీ);
	ప్రతిజ్ఞా(ypt != ఖాళీ);

	కొరకు (i = 0, j = అంకె - 1; i < అంకె; j = i++) {
		ఒకవేళ ((((ypt[i] + y <= click_y) && (click_y < ypt[j] + y)) ||
		     ((ypt[j] + y <= click_y) && (click_y < ypt[i] + y))) &&
		     (click_x < (xpt[j] - xpt[i]) *
		     (click_y - (ypt[i] + y)) / (ypt[j] - ypt[i]) + xpt[i] + x))
			c = !c;
	}

	తిరిగి c;
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
nsurl *imagemap_get(నిర్మాణం html_content *c, స్థిరరాశి అక్షరం *కీలకం,
		గుర్తులేని దీర్ఘ x, గుర్తులేని దీర్ఘ y,
		గుర్తులేని దీర్ఘ click_x, గుర్తులేని దీర్ఘ click_y,
		స్థిరరాశి అక్షరం **లక్ష్యం)
{
	గుర్తులేని పూర్ణాంకం స్థానం = 0;
	నిర్మాణం imagemap *map;
	నిర్మాణం mapentry *నమోదు_2;
	గుర్తులేని దీర్ఘ cx, cy;

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (కీలకం == ఖాళీ)
		తిరిగి ఖాళీ;

	ఒకవేళ (c->imagemaps == ఖాళీ)
		తిరిగి ఖాళీ;

	స్థానం = imagemap_hash(కీలకం);

	కొరకు (map = c->imagemaps[స్థానం]; map != ఖాళీ; map = map->తదుపరి) {
		ఒకవేళ (map->కీలకం != ఖాళీ && strcasecmp(map->కీలకం, కీలకం) == 0)
			ఆపు;
	}

	ఒకవేళ (map == ఖాళీ || map->list == ఖాళీ)
		తిరిగి ఖాళీ;

	కొరకు (నమోదు_2 = map->list; నమోదు_2; నమోదు_2 = నమోదు_2->తదుపరి) {
		ఎంపిక (నమోదు_2->type) {
		స్థితి IMAGEMAP_DEFAULT:
			/* just return the URL. no checks required */
			ఒకవేళ (లక్ష్యం)
				*లక్ష్యం = నమోదు_2->లక్ష్యం;
			తిరిగి నమోదు_2->url;
			ఆపు;
		స్థితి IMAGEMAP_RECT:
			ఒకవేళ (click_x >= x + నమోదు_2->bounds.rect.x0 &&
				    click_x <= x + నమోదు_2->bounds.rect.x1 &&
				    click_y >= y + నమోదు_2->bounds.rect.y0 &&
				    click_y <= y + నమోదు_2->bounds.rect.y1) {
				ఒకవేళ (లక్ష్యం)
					*లక్ష్యం = నమోదు_2->లక్ష్యం;
				తిరిగి నమోదు_2->url;
			}
			ఆపు;
		స్థితి IMAGEMAP_CIRCLE:
			cx = x + నమోదు_2->bounds.circle.x - click_x;
			cy = y + నమోదు_2->bounds.circle.y - click_y;
			ఒకవేళ ((cx * cx + cy * cy) <=
				(గుర్తులేని దీర్ఘ) (నమోదు_2->bounds.circle.r *
					నమోదు_2->bounds.circle.r)) {
				ఒకవేళ (లక్ష్యం)
					*లక్ష్యం = నమోదు_2->లక్ష్యం;
				తిరిగి నమోదు_2->url;
			}
			ఆపు;
		స్థితి IMAGEMAP_POLY:
			ఒకవేళ (imagemap_బిందువు_లోపల_poly(నమోదు_2->bounds.poly.అంకె,
					నమోదు_2->bounds.poly.xcoords,
					నమోదు_2->bounds.poly.ycoords, x, y,
					click_x, click_y)) {
				ఒకవేళ (లక్ష్యం)
					*లక్ష్యం = నమోదు_2->లక్ష్యం;
				తిరిగి నమోదు_2->url;
			}
			ఆపు;
		}
	}

	ఒకవేళ (లక్ష్యం)
		*లక్ష్యం = ఖాళీ;

	తిరిగి ఖాళీ;
}
