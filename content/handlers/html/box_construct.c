/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
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
 * Implementation of conversion from DOM tree to box tree.
 */

#అన్తర్భావ <string.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/string.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "css/select.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/object.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_manipulate.h"
#అన్తర్భావ "html/box_construct.h"
#అన్తర్భావ "html/box_special.h"
#అన్తర్భావ "html/box_normalise.h"
#అన్తర్భావ "html/form_internal.h"

/**
 * Context for box tree construction
 */
నిర్మాణం box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_కణుపు *n;			/**< Current node to process */

	నిర్మాణం box *మూలం_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	పూర్ణాంకం *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
నిర్మాణం box_construct_props {
	/** Style from which to inherit, or NULL if none */
	స్థిరరాశి css_computed_style *మాతృ_style;
	/** Current link target, or NULL if none */
	నిర్మాణం nsurl *href;
	/** Current frame target, or NULL if none */
	స్థిరరాశి అక్షరం *లక్ష్యం;
	/** Current title attribute, or NULL if none */
	స్థిరరాశి అక్షరం *title;
	/** Identity of the current block-level container */
	నిర్మాణం box *containing_ఖండం;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	నిర్మాణం box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool కణుపు_is_మూలం;
};

స్థిర స్థిరరాశి content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
స్థిర స్థిరరాశి box_type box_map[] = {
	BOX_ఖండం,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_ఖండం,           /* CSS_DISPLAY_BLOCK */
	BOX_ఖండం,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_ఖండం,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_పట్టిక,           /* CSS_DISPLAY_TABLE */
	BOX_పట్టిక,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_పట్టిక_అడ్డువరుస_సమూహం, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_పట్టిక_అడ్డువరుస_సమూహం, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_పట్టిక_అడ్డువరుస_సమూహం, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_పట్టిక_అడ్డువరుస,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_పట్టిక_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_ఖండం,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_ఖండం,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
స్థిర అంతఃవరుస bool box_is_మూలం(dom_కణుపు *n)
{
	dom_కణుపు *మాతృ;
	dom_కణుపు_type type;
	dom_exception err;

	err = dom_కణుపు_get_మాతృ_కణుపు(n, &మాతృ);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (మాతృ != ఖాళీ) {
		err = dom_కణుపు_get_కణుపు_type(మాతృ, &type);

		dom_కణుపు_unref(మాతృ);

		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		ఒకవేళ (type != DOM_DOCUMENT_కణుపు)
			తిరిగి false;
	}

	తిరిగి true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
స్థిర శూన్య
box_వెలికితీయి_properties(dom_కణుపు *n, నిర్మాణం box_construct_props *props)
{
	memset(props, 0, పరిమాణం(*props));

	props->కణుపు_is_మూలం = box_is_మూలం(n);

	/* Extract properties from containing DOM node */
	ఒకవేళ (props->కణుపు_is_మూలం == false) {
		dom_కణుపు *current_కణుపు = n;
		dom_కణుపు *మాతృ_కణుపు = ఖాళీ;
		నిర్మాణం box *మాతృ_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		అయితే (true) {
			err = dom_కణుపు_get_మాతృ_కణుపు(current_కణుపు,
					&మాతృ_కణుపు);
			ఒకవేళ (err != DOM_NO_ERR || మాతృ_కణుపు == ఖాళీ)
				ఆపు;

			మాతృ_box = box_for_కణుపు(మాతృ_కణుపు);

			ఒకవేళ (మాతృ_box != ఖాళీ) {
				props->మాతృ_style = మాతృ_box->style;
				props->href = మాతృ_box->href;
				props->లక్ష్యం = మాతృ_box->లక్ష్యం;
				props->title = మాతృ_box->title;

				dom_కణుపు_unref(మాతృ_కణుపు);
				ఆపు;
			} లేకపోతే {
				ఒకవేళ (current_కణుపు != n)
					dom_కణుపు_unref(current_కణుపు);
				current_కణుపు = మాతృ_కణుపు;
				మాతృ_కణుపు = ఖాళీ;
			}
		}

		/* Find containing block (may be parent) */
		అయితే (true) {
			నిర్మాణం box *b;

			err = dom_కణుపు_get_మాతృ_కణుపు(current_కణుపు,
					&మాతృ_కణుపు);
			ఒకవేళ (err != DOM_NO_ERR || మాతృ_కణుపు == ఖాళీ) {
				ఒకవేళ (current_కణుపు != n)
					dom_కణుపు_unref(current_కణుపు);
				ఆపు;
			}

			ఒకవేళ (current_కణుపు != n)
				dom_కణుపు_unref(current_కణుపు);

			b = box_for_కణుపు(మాతృ_కణుపు);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ఒకవేళ (b != ఖాళీ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_ఖండం = b;

				dom_కణుపు_unref(మాతృ_కణుపు);
				ఆపు;
			} లేకపోతే {
				current_కణుపు = మాతృ_కణుపు;
				మాతృ_కణుపు = ఖాళీ;
			}
		}
	}

	/* Compute current inline container, if any */
	ఒకవేళ (props->containing_ఖండం != ఖాళీ &&
			props->containing_ఖండం->చివరి != ఖాళీ &&
			props->containing_ఖండం->చివరి->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_ఖండం->చివరి;
}


/**
 * Get the style for an element.
 *
 * \param  c               content of type CONTENT_HTML that is being processed
 * \param  parent_style    style at this point in xml tree, or NULL for root
 * \param  root_style      root node's style, or NULL for root
 * \param  n               node in xml tree
 * \return  the new style, or NULL on memory exhaustion
 */
స్థిర css_select_results *
box_get_style(html_content *c,
	      స్థిరరాశి css_computed_style *మాతృ_style,
	      స్థిరరాశి css_computed_style *మూలం_style,
	      dom_కణుపు *n)
{
	dom_string *s = ఖాళీ;
	css_stylesheet *inline_style = ఖాళీ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ఒకవేళ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ఒకవేళ (err != DOM_NO_ERR) {
			తిరిగి ఖాళీ;
		}
	}

	ఒకవేళ (s != ఖాళీ) {
		inline_style = nscss_create_inline_style(
				(స్థిరరాశి uint8_t *) dom_string_దత్తాంశం(s),
				dom_string_byte_నిడివి(s),
				c->encoding,
				nsurl_access(c->పునాది_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_బహుళకం_NONE);

		dom_string_unref(s);

		ఒకవేళ (inline_style == ఖాళీ)
			తిరిగి ఖాళీ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
	ctx.పునాది_url = c->పునాది_url;
	ctx.universal = c->universal;
	ctx.మూలం_style = మూలం_style;
	ctx.మాతృ_style = మాతృ_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ఒకవేళ (inline_style != ఖాళీ)
		css_stylesheet_destroy(inline_style);

	తిరిగి styles;
}


/**
 * Construct the box required for a generated element.
 *
 * \param n        XML node of type XML_ELEMENT_NODE
 * \param content  Content of type CONTENT_HTML that is being processed
 * \param box      Box which may have generated content
 * \param style    Complete computed style for pseudo element, or NULL
 *
 * \todo This is currently incomplete. It just does enough to support
 * the clearfix hack. (http://www.positioniseverything.net/easyclearing.html )
 */
స్థిర శూన్య
box_construct_generate(dom_కణుపు *n,
		       html_content *content,
		       నిర్మాణం box *box,
		       స్థిరరాశి css_computed_style *style)
{
	నిర్మాణం box *gen = ఖాళీ;
	గణన css_display_e computed_display;
	స్థిరరాశి css_computed_content_అంశం *c_అంశం;

	/* Nothing to generate if the parent box is not a block */
	ఒకవేళ (box->type != BOX_ఖండం)
		తిరిగి;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ఒకవేళ (style == ఖాళీ ||
			css_computed_content(style, &c_అంశం) ==
			CSS_CONTENT_సాధారణ) {
		/* No pseudo element */
		తిరిగి;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_మూలం(n));
	ఒకవేళ (computed_display == CSS_DISPLAY_ఖండం ||
			computed_display == CSS_DISPLAY_పట్టిక) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ఖాళీ, (css_computed_style *) style,
				false, ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ, content->bctx);
		ఒకవేళ (gen == ఖాళీ) {
			తిరిగి;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_మూలం(n))];

		box_కూడు_బిడ్డ(box, gen);
	}
}


/**
 * Construct a list marker box
 *
 * \param box      Box to attach marker to
 * \param title    Current title attribute
 * \param ctx      Box construction context
 * \param parent   Current block-level container
 * \return true on success, false on memory exhaustion
 */
స్థిర bool
box_construct_marker(నిర్మాణం box *box,
		     స్థిరరాశి అక్షరం *title,
		     నిర్మాణం box_construct_ctx *ctx,
		     నిర్మాణం box *మాతృ)
{
	lwc_string *image_uri;
	నిర్మాణం box *marker;
	గణన css_list_style_type_e list_style_type;

	marker = box_create(ఖాళీ, box->style, false, ఖాళీ, ఖాళీ, title,
			ఖాళీ, ctx->bctx);
	ఒకవేళ (marker == false)
		తిరిగి false;

	marker->type = BOX_ఖండం;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	ఎంపిక (list_style_type) {
	స్థితి CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->పాఠ్యం = (అక్షరం *) "\342\200\242";
		marker->నిడివి = 3;
		ఆపు;

	స్థితి CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->పాఠ్యం = (అక్షరం *) "\342\227\213";
		marker->నిడివి = 3;
		ఆపు;

	స్థితి CSS_LIST_STYLE_TYPE_వర్గం:
		/* 25AA BLACK SMALL SQUARE */
		marker->పాఠ్యం = (అక్షరం *) "\342\226\252";
		marker->నిడివి = 3;
		ఆపు;

	అప్రమేయ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	స్థితి CSS_LIST_STYLE_TYPE_NONE:
		marker->పాఠ్యం = ఖాళీ;
		marker->నిడివి = 0;
		ఆపు;
	}

	ఒకవేళ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ఖాళీ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_దత్తాంశం(image_uri), &url);
		ఒకవేళ (error != NSERROR_OK)
			తిరిగి false;

		ఒకవేళ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			తిరిగి false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->మాతృ = box;

	తిరిగి true;
}

స్థిర అంతఃవరుస bool box__style_is_float(స్థిరరాశి నిర్మాణం box *box)
{
	తిరిగి css_computed_float(box->style) == CSS_FLOAT_ఎడమ ||
	       css_computed_float(box->style) == CSS_FLOAT_కుడి;
}

స్థిర అంతఃవరుస bool box__is_flex(స్థిరరాశి నిర్మాణం box *box)
{
	తిరిగి box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

స్థిర అంతఃవరుస bool box_containing_ఖండం_is_flex(
		స్థిరరాశి నిర్మాణం box_construct_props *props)
{
	తిరిగి props->containing_ఖండం != ఖాళీ &&
	       box__is_flex(props->containing_ఖండం);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
స్థిర bool
box_construct_element(నిర్మాణం box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ఖాళీ;
	గణన css_display_e css_display;
	నిర్మాణం box *box = ఖాళీ, *old_box;
	css_select_results *styles = ఖాళీ;
	lwc_string *bgimage_uri;
	dom_exception err;
	నిర్మాణం box_construct_props props;
	స్థిరరాశి css_computed_style *మూలం_style = ఖాళీ;

	ప్రతిజ్ఞా(ctx->n != ఖాళీ);

	box_వెలికితీయి_properties(ctx->n, &props);

	ఒకవేళ (props.containing_ఖండం != ఖాళీ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_ఖండం->flags &= ~PRE_STRIP;
	}

	ఒకవేళ (props.కణుపు_is_మూలం == false) {
		మూలం_style = ctx->మూలం_box->style;
	}

	styles = box_get_style(ctx->content, props.మాతృ_style, మూలం_style,
			ctx->n);
	ఒకవేళ (styles == ఖాళీ)
		తిరిగి false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (title0 != ఖాళీ) {
		అక్షరం *t = squash_whitespace(dom_string_దత్తాంశం(title0));

		dom_string_unref(title0);

		ఒకవేళ (t == ఖాళీ)
			తిరిగి false;

		props.title = talloc_strdup(ctx->bctx, t);

		విడుదల(t);

		ఒకవేళ (props.title == ఖాళీ)
			తిరిగి false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (s != ఖాళీ) {
		err = dom_string_intern(s, &id);
		ఒకవేళ (err != DOM_NO_ERR)
			id = ఖాళీ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.లక్ష్యం, props.title, id,
			ctx->bctx);
	ఒకవేళ (box == ఖాళీ)
		తిరిగి false;

	/* If this is the root box, add it to the context */
	ఒకవేళ (props.కణుపు_is_మూలం)
		ctx->మూలం_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (s != ఖాళీ) {
		స్థిరరాశి అక్షరం *val = dom_string_దత్తాంశం(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ఒకవేళ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ఖాళీ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (s != ఖాళీ) {
		స్థిరరాశి అక్షరం *val = dom_string_దత్తాంశం(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ఒకవేళ ('0' <= val[0] && val[0] <= '9')
			box->అడ్డువరుసలు = clamp(strtol(val, ఖాళీ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ఒకవేళ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_ఖండం ||
			 css_display == CSS_DISPLAY_INLINE_పట్టిక ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_ఖండం];
	} లేకపోతే ఒకవేళ (props.కణుపు_is_మూలం) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_ఖండం;
	} లేకపోతే {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.కణుపు_is_మూలం)];

		ఒకవేళ (props.containing_ఖండం->type == BOX_FLEX ||
		    props.containing_ఖండం->type == BOX_INLINE_FLEX) {
			/* Blockification */
			ఎంపిక (box->type) {
			స్థితి BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ఆపు;
			స్థితి BOX_INLINE_ఖండం:
				box->type = BOX_ఖండం;
				ఆపు;
			అప్రమేయ:
				ఆపు;
			}
		}
	}

	ఒకవేళ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		తిరిగి false;
	}

	/* Handle the :before pseudo element */
	ఒకవేళ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ఒకవేళ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.కణుపు_is_మూలం) == CSS_DISPLAY_NONE &&
			props.కణుపు_is_మూలం == false)) {
		css_select_results_destroy(styles);
		box->styles = ఖాళీ;
		box->style = ఖాళీ;

		/* Invalidate associated gadget, if any */
		ఒకవేళ (box->gadget != ఖాళీ) {
			box->gadget->box = ఖాళీ;
			box->gadget = ఖాళీ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		తిరిగి true;
	}

	/* Attach DOM node to box */
	err = dom_కణుపు_సమితి_user_దత్తాంశం(ctx->n,
			corestring_dom_ns_కీలకం_box_కణుపు_దత్తాంశం, box, ఖాళీ,
			(శూన్య *) &old_box);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	/* Attach box to DOM node */
	box->కణుపు = dom_కణుపు_ref(ctx->n);

	ఒకవేళ (props.inline_container == ఖాళీ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_ఖండం ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box_containing_ఖండం_is_flex(&props))) &&
			props.కణుపు_is_మూలం == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ప్రతిజ్ఞా(props.containing_ఖండం != ఖాళీ &&
				"Box must have containing block.");

		props.inline_container = box_create(ఖాళీ, ఖాళీ, false, ఖాళీ,
				ఖాళీ, ఖాళీ, ఖాళీ, ctx->bctx);
		ఒకవేళ (props.inline_container == ఖాళీ)
			తిరిగి false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_కూడు_బిడ్డ(props.containing_ఖండం, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ఒకవేళ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ఖాళీ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_దత్తాంశం(bgimage_uri), &url);
		ఒకవేళ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ఒకవేళ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				తిరిగి false;
			}
			nsurl_unref(url);
		}
	}

	ఒకవేళ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ఒకవేళ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_ఖండం) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ప్రతిజ్ఞా(props.inline_container != ఖాళీ);

		box_కూడు_బిడ్డ(props.inline_container, box);
	} లేకపోతే {
		ఒకవేళ (ns_computed_display(box->style, props.కణుపు_is_మూలం) ==
				CSS_DISPLAY_LIST_అంశం) {
			/* List item: compute marker */
			ఒకవేళ (box_construct_marker(box, props.title, ctx,
					props.containing_ఖండం) == false)
				తిరిగి false;
		}

		ఒకవేళ (props.కణుపు_is_మూలం == false &&
				box_containing_ఖండం_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ఎడమ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_కుడి)) {
			/* Float: insert a float between the parent and box. */
			నిర్మాణం box *flt = box_create(ఖాళీ, ఖాళీ, false,
					props.href, props.లక్ష్యం, props.title,
					ఖాళీ, ctx->bctx);
			ఒకవేళ (flt == ఖాళీ)
				తిరిగి false;

			ఒకవేళ (css_computed_float(box->style) == CSS_FLOAT_ఎడమ)
				flt->type = BOX_FLOAT_ఎడమ;
			లేకపోతే
				flt->type = BOX_FLOAT_కుడి;

			box_కూడు_బిడ్డ(props.inline_container, flt);
			box_కూడు_బిడ్డ(flt, box);
		} లేకపోతే {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ఒకవేళ (props.containing_ఖండం != ఖాళీ)
				box_కూడు_బిడ్డ(props.containing_ఖండం, box);
		}
	}

	తిరిగి true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
స్థిర శూన్య box_construct_element_after(dom_కణుపు *n, html_content *content)
{
	నిర్మాణం box_construct_props props;
	నిర్మాణం box *box = box_for_కణుపు(n);

	ప్రతిజ్ఞా(box != ఖాళీ);

	box_వెలికితీయి_properties(n, &props);

	ఒకవేళ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		నిర్మాణం box *inline_చివర;
		bool has_children;
		dom_exception err;

		err = dom_కణుపు_has_బిడ్డ_nodes(n, &has_children);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి;

		ఒకవేళ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			తిరిగి;
		}

		ఒకవేళ (props.inline_container == ఖాళీ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ఖాళీ, ఖాళీ, false,
					ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ, content->bctx);
			ఒకవేళ (props.inline_container == ఖాళీ)
				తిరిగి;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_కూడు_బిడ్డ(props.containing_ఖండం,
					props.inline_container);
		}

		inline_చివర = box_create(ఖాళీ, box->style, false,
				box->href, box->లక్ష్యం, box->title,
				box->id == ఖాళీ ? ఖాళీ :
				lwc_string_ref(box->id), content->bctx);
		ఒకవేళ (inline_చివర != ఖాళీ) {
			inline_చివర->type = BOX_INLINE_చివర;

			ప్రతిజ్ఞా(props.inline_container != ఖాళీ);

			box_కూడు_బిడ్డ(props.inline_container, inline_చివర);

			box->inline_చివర = inline_చివర;
			inline_చివర->inline_చివర = box;
		}
	} లేకపోతే ఒకవేళ (!(box->flags & IS_REPLACED)) {
		/* Handle the :after pseudo element */
		box_construct_generate(n, content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_AFTER]);
	}
}


/**
 * Find the next node in the DOM tree, completing element construction
 * where appropriate.
 *
 * \param n                 Current node
 * \param content           Containing content
 * \param convert_children  Whether to consider children of \a n
 * \return Next node to process, or NULL if complete
 *
 * \note \a n will be unreferenced
 */
స్థిర dom_కణుపు *
తదుపరి_కణుపు(dom_కణుపు *n, html_content *content, bool convert_children)
{
	dom_కణుపు *తదుపరి = ఖాళీ;
	bool has_children;
	dom_exception err;

	err = dom_కణుపు_has_బిడ్డ_nodes(n, &has_children);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_కణుపు_unref(n);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (convert_children && has_children) {
		err = dom_కణుపు_get_మొదటి_బిడ్డ(n, &తదుపరి);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి ఖాళీ;
		}
		dom_కణుపు_unref(n);
	} లేకపోతే {
		err = dom_కణుపు_get_తదుపరి_sibling(n, &తదుపరి);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి ఖాళీ;
		}

		ఒకవేళ (తదుపరి != ఖాళీ) {
			ఒకవేళ (box_for_కణుపు(n) != ఖాళీ)
				box_construct_element_after(n, content);
			dom_కణుపు_unref(n);
		} లేకపోతే {
			ఒకవేళ (box_for_కణుపు(n) != ఖాళీ)
				box_construct_element_after(n, content);

			అయితే (box_is_మూలం(n) == false) {
				dom_కణుపు *మాతృ = ఖాళీ;
				dom_కణుపు *మాతృ_తదుపరి = ఖాళీ;

				err = dom_కణుపు_get_మాతృ_కణుపు(n, &మాతృ);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(n);
					తిరిగి ఖాళీ;
				}

				ప్రతిజ్ఞా(మాతృ != ఖాళీ);

				err = dom_కణుపు_get_తదుపరి_sibling(మాతృ,
						&మాతృ_తదుపరి);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(మాతృ);
					dom_కణుపు_unref(n);
					తిరిగి ఖాళీ;
				}

				ఒకవేళ (మాతృ_తదుపరి != ఖాళీ) {
					dom_కణుపు_unref(మాతృ_తదుపరి);
					dom_కణుపు_unref(మాతృ);
					ఆపు;
				}

				dom_కణుపు_unref(n);
				n = మాతృ;
				మాతృ = ఖాళీ;

				ఒకవేళ (box_for_కణుపు(n) != ఖాళీ) {
					box_construct_element_after(
							n, content);
				}
			}

			ఒకవేళ (box_is_మూలం(n) == false) {
				dom_కణుపు *మాతృ = ఖాళీ;

				err = dom_కణుపు_get_మాతృ_కణుపు(n, &మాతృ);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(n);
					తిరిగి ఖాళీ;
				}

				ప్రతిజ్ఞా(మాతృ != ఖాళీ);

				err = dom_కణుపు_get_తదుపరి_sibling(మాతృ, &తదుపరి);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(మాతృ);
					dom_కణుపు_unref(n);
					తిరిగి ఖాళీ;
				}

				ఒకవేళ (box_for_కణుపు(మాతృ) != ఖాళీ) {
					box_construct_element_after(మాతృ,
							content);
				}

				dom_కణుపు_unref(మాతృ);
			}

			dom_కణుపు_unref(n);
		}
	}

	తిరిగి తదుపరి;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
స్థిర శూన్య
box_పాఠ్యం_transform(అక్షరం *s, గుర్తులేని పూర్ణాంకం len, గణన css_పాఠ్యం_transform_e tt)
{
	గుర్తులేని పూర్ణాంకం i;
	ఒకవేళ (len == 0)
		తిరిగి;
	ఎంపిక (tt) {
		స్థితి CSS_పాఠ్యం_TRANSFORM_UPPERCASE:
			కొరకు (i = 0; i < len; ++i)
				ఒకవేళ ((గుర్తులేని అక్షరం) s[i] < 0x80)
					s[i] = ascii_గమ్యం_పై(s[i]);
			ఆపు;
		స్థితి CSS_పాఠ్యం_TRANSFORM_LOWERCASE:
			కొరకు (i = 0; i < len; ++i)
				ఒకవేళ ((గుర్తులేని అక్షరం) s[i] < 0x80)
					s[i] = ascii_గమ్యం_క్రింది(s[i]);
			ఆపు;
		స్థితి CSS_పాఠ్యం_TRANSFORM_CAPITALIZE:
			ఒకవేళ ((గుర్తులేని అక్షరం) s[0] < 0x80)
				s[0] = ascii_గమ్యం_పై(s[0]);
			కొరకు (i = 1; i < len; ++i)
				ఒకవేళ ((గుర్తులేని అక్షరం) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_గమ్యం_పై(s[i]);
			ఆపు;
		అప్రమేయ:
			ఆపు;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
స్థిర bool box_construct_పాఠ్యం(నిర్మాణం box_construct_ctx *ctx)
{
	నిర్మాణం box_construct_props props;
	నిర్మాణం box *box = ఖాళీ;
	dom_string *content;
	dom_exception err;

	ప్రతిజ్ఞా(ctx->n != ఖాళీ);

	box_వెలికితీయి_properties(ctx->n, &props);

	ప్రతిజ్ఞా(props.containing_ఖండం != ఖాళీ);

	err = dom_characterdata_get_దత్తాంశం(ctx->n, &content);
	ఒకవేళ (err != DOM_NO_ERR || content == ఖాళీ)
		తిరిగి false;

	ఒకవేళ (css_computed_white_space(props.మాతృ_style) ==
			CSS_WHITE_SPACE_సాధారణ ||
			css_computed_white_space(props.మాతృ_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		అక్షరం *పాఠ్యం;

		పాఠ్యం = squash_whitespace(dom_string_దత్తాంశం(content));

		dom_string_unref(content);

		ఒకవేళ (పాఠ్యం == ఖాళీ)
			తిరిగి false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ఒకవేళ (పాఠ్యం[0] == ' ' && పాఠ్యం[1] == 0) {
			ఒకవేళ (props.inline_container != ఖాళీ) {
				ప్రతిజ్ఞా(props.inline_container->చివరి != ఖాళీ);

				props.inline_container->చివరి->space =
						UNKNOWN_వెడల్పు;
			}

			విడుదల(పాఠ్యం);

			తిరిగి true;
		}

		ఒకవేళ (props.inline_container == ఖాళీ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ఖాళీ, ఖాళీ, false,
					ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ, ctx->bctx);
			ఒకవేళ (props.inline_container == ఖాళీ) {
				విడుదల(పాఠ్యం);
				తిరిగి false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_కూడు_బిడ్డ(props.containing_ఖండం,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ఖాళీ,
				(css_computed_style *) props.మాతృ_style,
				false, props.href, props.లక్ష్యం, props.title,
				ఖాళీ, ctx->bctx);
		ఒకవేళ (box == ఖాళీ) {
			విడుదల(పాఠ్యం);
			తిరిగి false;
		}

		box->type = BOX_పాఠ్యం;

		box->పాఠ్యం = talloc_strdup(ctx->bctx, పాఠ్యం);
		విడుదల(పాఠ్యం);
		ఒకవేళ (box->పాఠ్యం == ఖాళీ)
			తిరిగి false;

		box->నిడివి = అక్షరశ్రేణి_పొడవు(box->పాఠ్యం);

		/* strip ending space char off */
		ఒకవేళ (box->నిడివి > 1 && box->పాఠ్యం[box->నిడివి - 1] == ' ') {
			box->space = UNKNOWN_వెడల్పు;
			box->నిడివి--;
		}

		ఒకవేళ (css_computed_పాఠ్యం_transform(props.మాతృ_style) !=
				CSS_పాఠ్యం_TRANSFORM_NONE)
			box_పాఠ్యం_transform(box->పాఠ్యం, box->నిడివి,
				css_computed_పాఠ్యం_transform(
					props.మాతృ_style));

		box_కూడు_బిడ్డ(props.inline_container, box);

		ఒకవేళ (box->పాఠ్యం[0] == ' ') {
			box->నిడివి--;

			memmove(box->పాఠ్యం, &box->పాఠ్యం[1], box->నిడివి);

			ఒకవేళ (box->prev != ఖాళీ)
				box->prev->space = UNKNOWN_వెడల్పు;
		}
	} లేకపోతే {
		/* white-space: pre */
		అక్షరం *పాఠ్యం;
		size_t పాఠ్యం_len = dom_string_byte_నిడివి(content);
		size_t i;
		అక్షరం *current;
		గణన css_white_space_e white_space =
				css_computed_white_space(props.మాతృ_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ప్రతిజ్ఞా(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_పంక్తి ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		పాఠ్యం = జ్ఞాపకం_ఇవ్వు(పాఠ్యం_len + 1);
		dom_string_unref(content);

		ఒకవేళ (పాఠ్యం == ఖాళీ)
			తిరిగి false;

		జ్ఞాపకం_నకలు(పాఠ్యం, dom_string_దత్తాంశం(content), పాఠ్యం_len);
		పాఠ్యం[పాఠ్యం_len] = '\0';

		/* TODO: Handle tabs properly */
		కొరకు (i = 0; i < పాఠ్యం_len; i++)
			ఒకవేళ (పాఠ్యం[i] == '\t')
				పాఠ్యం[i] = ' ';

		ఒకవేళ (css_computed_పాఠ్యం_transform(props.మాతృ_style) !=
				CSS_పాఠ్యం_TRANSFORM_NONE)
			box_పాఠ్యం_transform(పాఠ్యం, అక్షరశ్రేణి_పొడవు(పాఠ్యం),
				css_computed_పాఠ్యం_transform(
						props.మాతృ_style));

		current = పాఠ్యం;

		/* swallow a single leading new line */
		ఒకవేళ (props.containing_ఖండం->flags & PRE_STRIP) {
			ఎంపిక (*current) {
			స్థితి '\n':
				current++;
				ఆపు;
			స్థితి '\r':
				current++;
				ఒకవేళ (*current == '\n')
					current++;
				ఆపు;
			}
			props.containing_ఖండం->flags &= ~PRE_STRIP;
		}

		చేయి {
			size_t len = strcspn(current, "\r\n");

			అక్షరం old = current[len];

			current[len] = 0;

			ఒకవేళ (props.inline_container == ఖాళీ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ఖాళీ, ఖాళీ,
						false, ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ,
						ctx->bctx);
				ఒకవేళ (props.inline_container == ఖాళీ) {
					విడుదల(పాఠ్యం);
					తిరిగి false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_కూడు_బిడ్డ(props.containing_ఖండం,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ఖాళీ,
				(css_computed_style *) props.మాతృ_style,
				false, props.href, props.లక్ష్యం, props.title,
				ఖాళీ, ctx->bctx);
			ఒకవేళ (box == ఖాళీ) {
				విడుదల(పాఠ్యం);
				తిరిగి false;
			}

			box->type = BOX_పాఠ్యం;

			box->పాఠ్యం = talloc_strdup(ctx->bctx, current);
			ఒకవేళ (box->పాఠ్యం == ఖాళీ) {
				విడుదల(పాఠ్యం);
				తిరిగి false;
			}

			box->నిడివి = అక్షరశ్రేణి_పొడవు(box->పాఠ్యం);

			box_కూడు_బిడ్డ(props.inline_container, box);

			current[len] = old;

			current += len;

			ఒకవేళ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ఖాళీ, ఖాళీ,
						false, ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ,
						ctx->bctx);
				ఒకవేళ (props.inline_container == ఖాళీ) {
					విడుదల(పాఠ్యం);
					తిరిగి false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_కూడు_బిడ్డ(props.containing_ఖండం,
						props.inline_container);

				ఒకవేళ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				లేకపోతే
					current++;
			}
		} అయితే (*current);

		విడుదల(పాఠ్యం);
	}

	తిరిగి true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
స్థిర శూన్య convert_xml_గమ్యం_box(నిర్మాణం box_construct_ctx *ctx)
{
	dom_కణుపు *తదుపరి;
	bool convert_children;
	uint32_t అంకె_processed = 0;
	స్థిరరాశి uint32_t max_processed_before_yield = 10;

	చేయి {
		convert_children = true;

		ప్రతిజ్ఞా(ctx->n != ఖాళీ);

		ఒకవేళ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_కణుపు_unref(ctx->n);
			విడుదల(ctx);
			తిరిగి;
		}

		/* Find next element to process, converting text nodes as we go */
		తదుపరి = తదుపరి_కణుపు(ctx->n, ctx->content, convert_children);
		అయితే (తదుపరి != ఖాళీ) {
			dom_కణుపు_type type;
			dom_exception err;

			err = dom_కణుపు_get_కణుపు_type(తదుపరి, &type);
			ఒకవేళ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_కణుపు_unref(తదుపరి);
				విడుదల(ctx);
				తిరిగి;
			}

			ఒకవేళ (type == DOM_ELEMENT_కణుపు)
				ఆపు;

			ఒకవేళ (type == DOM_పాఠ్యం_కణుపు) {
				ctx->n = తదుపరి;
				ఒకవేళ (box_construct_పాఠ్యం(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_కణుపు_unref(ctx->n);
					విడుదల(ctx);
					తిరిగి;
				}
			}

			తదుపరి = తదుపరి_కణుపు(తదుపరి, ctx->content, true);
		}

		ctx->n = తదుపరి;

		ఒకవేళ (తదుపరి == ఖాళీ) {
			/* Conversion complete */
			నిర్మాణం box మూలం;

			memset(&మూలం, 0, పరిమాణం(మూలం));

			మూలం.type = BOX_ఖండం;
			మూలం.children = మూలం.చివరి = ctx->మూలం_box;
			మూలం.children->మాతృ = &మూలం;

			/** \todo Remove box_normalise_block */
			ఒకవేళ (box_normalise_ఖండం(&మూలం, ctx->మూలం_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} లేకపోతే {
				ctx->content->layout = మూలం.children;
				ctx->content->layout->మాతృ = ఖాళీ;

				ctx->cb(ctx->content, true);
			}

			ప్రతిజ్ఞా(ctx->n == ఖాళీ);

			విడుదల(ctx);
			తిరిగి;
		}
	} అయితే (++అంకె_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->కాలపట్టిక(0, (శూన్య *)convert_xml_గమ్యం_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_గమ్యం_box(dom_కణుపు *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   శూన్య **box_conversion_context)
{
	నిర్మాణం box_construct_ctx *ctx;

	ప్రతిజ్ఞా(box_conversion_context != ఖాళీ);

	ఒకవేళ (c->bctx == ఖాళీ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, పూర్ణాంకం);
		ఒకవేళ (c->bctx == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
	}

	ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_కణుపు_ref(n);
	ctx->మూలం_box = ఖాళీ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	తిరిగి guit->misc->కాలపట్టిక(0, (శూన్య *)convert_xml_గమ్యం_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_గమ్యం_box(శూన్య *box_conversion_context)
{
	నిర్మాణం box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->కాలపట్టిక(-1, (శూన్య *)convert_xml_గమ్యం_box, ctx);
	ఒకవేళ (err != NSERROR_OK) {
		తిరిగి err;
	}

	dom_కణుపు_unref(ctx->n);
	విడుదల(ctx);

	తిరిగి NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
నిర్మాణం box *box_for_కణుపు(dom_కణుపు *n)
{
	నిర్మాణం box *box = ఖాళీ;
	dom_exception err;

	err = dom_కణుపు_get_user_దత్తాంశం(n, corestring_dom_ns_కీలకం_box_కణుపు_దత్తాంశం,
			(శూన్య *) &box);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి ఖాళీ;

	తిరిగి box;
}

/* exported function documented in html/box_construct.h */
bool
box_వెలికితీయి_link(స్థిరరాశి html_content *content,
		 స్థిరరాశి dom_string *dsrel,
		 nsurl *పునాది,
		 nsurl **ఫలితం)
{
	అక్షరం *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	గుర్తులేని పూర్ణాంకం i, j, చివర;
	nserror error;
	స్థిరరాశి అక్షరం *rel;

	rel = dom_string_దత్తాంశం(dsrel);

	s1 = s = జ్ఞాపకం_ఇవ్వు(3 * అక్షరశ్రేణి_పొడవు(rel) + 1);
	ఒకవేళ (!s)
		తిరిగి false;

	/* copy to s, removing white space and control characters */
	కొరకు (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	కొరకు (చివర = అక్షరశ్రేణి_పొడవు(rel);
	     (చివర != i) && ascii_is_space(rel[చివర - 1]);
	     చివర--)
		;
	కొరకు (j = 0; i != చివర; i++) {
		ఒకవేళ ((గుర్తులేని అక్షరం) rel[i] < 0x20) {
			; /* skip control characters */
		} లేకపోతే ఒకవేళ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} లేకపోతే {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ఒకవేళ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ఒకవేళ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ఒకవేళ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ఒకవేళ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} లేకపోతే ఒకవేళ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(పునాది, s1, ఫలితం);
	విడుదల(s);
	ఒకవేళ (error != NSERROR_OK) {
		*ఫలితం = ఖాళీ;
		తిరిగి false;
	}

	తిరిగి true;
}
