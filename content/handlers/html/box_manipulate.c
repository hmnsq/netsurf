/*
 * Copyright 2005-2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of box tree manipulation.
 */


#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "netsurf/mouse.h"
#అన్తర్భావ "desktop/scrollbar.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/interaction.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
స్థిర పూర్ణాంకం box_talloc_destructor(నిర్మాణం box *b)
{
	నిర్మాణం html_scrollbar_దత్తాంశం *దత్తాంశం;

	ఒకవేళ ((b->flags & STYLE_OWNED) && b->style != ఖాళీ) {
		css_computed_style_destroy(b->style);
		b->style = ఖాళీ;
	}

	ఒకవేళ (b->styles != ఖాళీ) {
		css_select_results_destroy(b->styles);
		b->styles = ఖాళీ;
	}

	ఒకవేళ (b->href != ఖాళీ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ఒకవేళ (b->కణుపు != ఖాళీ) {
		dom_కణుపు_unref(b->కణుపు);
	}

	ఒకవేళ (b->scroll_x != ఖాళీ) {
		దత్తాంశం = scrollbar_get_దత్తాంశం(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		విడుదల(దత్తాంశం);
	}

	ఒకవేళ (b->scroll_y != ఖాళీ) {
		దత్తాంశం = scrollbar_get_దత్తాంశం(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		విడుదల(దత్తాంశం);
	}

	తిరిగి 0;
}


/* Exported function documented in html/box.h */
నిర్మాణం box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   స్థిరరాశి అక్షరం *లక్ష్యం,
	   స్థిరరాశి అక్షరం *title,
	   lwc_string *id,
	   శూన్య *context)
{
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం box *box;

	box = talloc(context, నిర్మాణం box);
	ఒకవేళ (!box) {
		తిరిగి 0;
	}

	talloc_సమితి_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->వెడల్పు = UNKNOWN_వెడల్పు;
	box->ఎత్తు = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	కొరకు (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].వెడల్పు = 0;
	box->scroll_x = box->scroll_y = ఖాళీ;
	box->min_వెడల్పు = 0;
	box->max_వెడల్పు = UNKNOWN_MAX_వెడల్పు;
	box->byte_స్థానభ్రంశం = 0;
	box->పాఠ్యం = ఖాళీ;
	box->నిడివి = 0;
	box->space = 0;
	box->href = (href == ఖాళీ) ? ఖాళీ : nsurl_ref(href);
	box->లక్ష్యం = లక్ష్యం;
	box->title = title;
	box->columns = 1;
	box->అడ్డువరుసలు = 1;
	box->ప్రారంభం_స్తంభం = 0;
	box->తదుపరి = ఖాళీ;
	box->prev = ఖాళీ;
	box->children = ఖాళీ;
	box->చివరి = ఖాళీ;
	box->మాతృ = ఖాళీ;
	box->inline_చివర = ఖాళీ;
	box->float_children = ఖాళీ;
	box->float_container = ఖాళీ;
	box->తదుపరి_float = ఖాళీ;
	box->cached_ఉంచు_క్రింద_level = 0;
	box->list_విలువ = 1;
	box->list_marker = ఖాళీ;
	box->నిలువు = ఖాళీ;
	box->gadget = ఖాళీ;
	box->usemap = ఖాళీ;
	box->id = id;
	box->background = ఖాళీ;
	box->object = ఖాళీ;
	box->object_params = ఖాళీ;
	box->iframe = ఖాళీ;
	box->కణుపు = ఖాళీ;

	తిరిగి box;
}


/* Exported function documented in html/box.h */
శూన్య box_కూడు_బిడ్డ(నిర్మాణం box *మాతృ, నిర్మాణం box *బిడ్డ)
{
	ప్రతిజ్ఞా(మాతృ);
	ప్రతిజ్ఞా(బిడ్డ);

	ఒకవేళ (మాతృ->children != 0) {	/* has children already */
		మాతృ->చివరి->తదుపరి = బిడ్డ;
		బిడ్డ->prev = మాతృ->చివరి;
	} లేకపోతే {			/* this is the first child */
		మాతృ->children = బిడ్డ;
		బిడ్డ->prev = 0;
	}

	మాతృ->చివరి = బిడ్డ;
	బిడ్డ->మాతృ = మాతృ;
}


/* Exported function documented in html/box.h */
శూన్య box_చొప్పించు_sibling(నిర్మాణం box *box, నిర్మాణం box *new_box)
{
	new_box->మాతృ = box->మాతృ;
	new_box->prev = box;
	new_box->తదుపరి = box->తదుపరి;
	box->తదుపరి = new_box;
	ఒకవేళ (new_box->తదుపరి)
		new_box->తదుపరి->prev = new_box;
	లేకపోతే ఒకవేళ (new_box->మాతృ)
		new_box->మాతృ->చివరి = new_box;
}


/* Exported function documented in html/box.h */
శూన్య box_unlink_and_free(నిర్మాణం box *box)
{
	నిర్మాణం box *మాతృ = box->మాతృ;
	నిర్మాణం box *తదుపరి = box->తదుపరి;
	నిర్మాణం box *prev = box->prev;

	ఒకవేళ (మాతృ) {
		ఒకవేళ (మాతృ->children == box)
			మాతృ->children = తదుపరి;
		ఒకవేళ (మాతృ->చివరి == box)
			మాతృ->చివరి = తదుపరి ? తదుపరి : prev;
	}

	ఒకవేళ (prev)
		prev->తదుపరి = తదుపరి;
	ఒకవేళ (తదుపరి)
		తదుపరి->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
శూన్య box_free(నిర్మాణం box *box)
{
	నిర్మాణం box *బిడ్డ, *తదుపరి;

	/* free children first */
	కొరకు (బిడ్డ = box->children; బిడ్డ; బిడ్డ = తదుపరి) {
		తదుపరి = బిడ్డ->తదుపరి;
		box_free(బిడ్డ);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
శూన్య box_free_box(నిర్మాణం box *box)
{
	ఒకవేళ (!(box->flags & CLONE)) {
		ఒకవేళ (box->gadget)
			form_free_control(box->gadget);
		ఒకవేళ (box->scroll_x != ఖాళీ)
			scrollbar_destroy(box->scroll_x);
		ఒకవేళ (box->scroll_y != ఖాళీ)
			scrollbar_destroy(box->scroll_y);
		ఒకవేళ (box->styles != ఖాళీ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(నిర్మాణం content *c,
		      నిర్మాణం box *box,
		      bool bottom,
		      bool కుడి)
{
	నిర్మాణం html_scrollbar_దత్తాంశం *దత్తాంశం;
	పూర్ణాంకం visible_వెడల్పు, visible_ఎత్తు;
	పూర్ణాంకం నిండిన_వెడల్పు, నిండిన_ఎత్తు;
	nserror res;

	ఒకవేళ (!bottom && box->scroll_x != ఖాళీ) {
		దత్తాంశం = scrollbar_get_దత్తాంశం(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		విడుదల(దత్తాంశం);
		box->scroll_x = ఖాళీ;
	}

	ఒకవేళ (!కుడి && box->scroll_y != ఖాళీ) {
		దత్తాంశం = scrollbar_get_దత్తాంశం(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		విడుదల(దత్తాంశం);
		box->scroll_y = ఖాళీ;
	}

	ఒకవేళ (!bottom && !కుడి) {
		తిరిగి NSERROR_OK;
	}

	visible_వెడల్పు = box->వెడల్పు + box->padding[కుడి_2] + box->padding[ఎడమ_2];
	visible_ఎత్తు = box->ఎత్తు + box->padding[శిఖరం_2] + box->padding[BOTTOM];

	నిండిన_వెడల్పు = ((box->descendant_x1 - box->border[కుడి_2].వెడల్పు) >
			visible_వెడల్పు) ?
			box->descendant_x1 + box->padding[కుడి_2] :
			visible_వెడల్పు;
	నిండిన_ఎత్తు = ((box->descendant_y1 - box->border[BOTTOM].వెడల్పు) >
			visible_ఎత్తు) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_ఎత్తు;

	ఒకవేళ (కుడి) {
		ఒకవేళ (box->scroll_y == ఖాళీ) {
			దత్తాంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం html_scrollbar_దత్తాంశం));
			ఒకవేళ (దత్తాంశం == ఖాళీ) {
				తిరిగి NSERROR_NOMEM;
			}
			దత్తాంశం->c = c;
			దత్తాంశం->box = box;
			res = scrollbar_create(false,
					       visible_ఎత్తు,
					       నిండిన_ఎత్తు,
					       visible_ఎత్తు,
					       దత్తాంశం,
					       html_అధికప్రవాహం_scroll_callback,
					       &(box->scroll_y));
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}
		} లేకపోతే  {
			scrollbar_సమితి_extents(box->scroll_y,
					      visible_ఎత్తు,
					      visible_ఎత్తు,
					      నిండిన_ఎత్తు);
		}
	}
	ఒకవేళ (bottom) {
		ఒకవేళ (box->scroll_x == ఖాళీ) {
			దత్తాంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం html_scrollbar_దత్తాంశం));
			ఒకవేళ (దత్తాంశం == ఖాళీ) {
				తిరిగి NSERROR_OK;
			}
			దత్తాంశం->c = c;
			దత్తాంశం->box = box;
			res = scrollbar_create(true,
					       visible_వెడల్పు - (కుడి ? SCROLLBAR_వెడల్పు : 0),
					       నిండిన_వెడల్పు,
					       visible_వెడల్పు,
					       దత్తాంశం,
					       html_అధికప్రవాహం_scroll_callback,
					       &box->scroll_x);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}
		} లేకపోతే {
			scrollbar_సమితి_extents(box->scroll_x,
					visible_వెడల్పు -
					(కుడి ? SCROLLBAR_వెడల్పు : 0),
					visible_వెడల్పు, నిండిన_వెడల్పు);
		}
	}

	ఒకవేళ (కుడి && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	తిరిగి NSERROR_OK;
}


