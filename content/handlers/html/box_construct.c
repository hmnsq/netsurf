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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "utils/string.h"
#অন্তর্ভাব "utils/ascii.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "css/select.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/object.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_manipulate.h"
#অন্তর্ভাব "html/box_construct.h"
#অন্তর্ভাব "html/box_special.h"
#অন্তর্ভাব "html/box_normalise.h"
#অন্তর্ভাব "html/form_internal.h"

/**
 * Context for box tree construction
 */
সংরচনা box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_গ্রন্থি *n;			/**< Current node to process */

	সংরচনা box *মূল_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	পূর্ণাঙ্ক *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
সংরচনা box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ধ্রুব css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	সংরচনা nsurl *href;
	/** Current frame target, or NULL if none */
	ধ্রুব বর্ণ *target;
	/** Current title attribute, or NULL if none */
	ধ্রুব বর্ণ *title;
	/** Identity of the current block-level container */
	সংরচনা box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	সংরচনা box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool গ্রন্থি_is_মূল;
};

স্থির ধ্রুব content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
স্থির ধ্রুব box_type box_map[] = {
	BOX_BLOCK,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_BLOCK,           /* CSS_DISPLAY_BLOCK */
	BOX_BLOCK,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_BLOCK,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_TABLE,           /* CSS_DISPLAY_TABLE */
	BOX_TABLE,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_TABLE_ROW_GROUP, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_TABLE_ROW_GROUP, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_TABLE_ROW_GROUP, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_TABLE_ROW,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_TABLE_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_BLOCK,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_BLOCK,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
স্থির অন্তঃস্থ bool box_is_মূল(dom_গ্রন্থি *n)
{
	dom_গ্রন্থি *parent;
	dom_গ্রন্থি_type type;
	dom_exception err;

	err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (parent != রিক্ত) {
		err = dom_গ্রন্থি_get_গ্রন্থি_type(parent, &type);

		dom_গ্রন্থি_unref(parent);

		যদি (err != DOM_NO_ERR)
			প্রত্যাগম false;

		যদি (type != DOM_DOCUMENT_গ্রন্থি)
			প্রত্যাগম false;
	}

	প্রত্যাগম true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
স্থির শূন্য
box_extract_properties(dom_গ্রন্থি *n, সংরচনা box_construct_props *props)
{
	memset(props, 0, পরিমাণ(*props));

	props->গ্রন্থি_is_মূল = box_is_মূল(n);

	/* Extract properties from containing DOM node */
	যদি (props->গ্রন্থি_is_মূল == false) {
		dom_গ্রন্থি *current_গ্রন্থি = n;
		dom_গ্রন্থি *parent_গ্রন্থি = রিক্ত;
		সংরচনা box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		যাবত্ (true) {
			err = dom_গ্রন্থি_get_parent_গ্রন্থি(current_গ্রন্থি,
					&parent_গ্রন্থি);
			যদি (err != DOM_NO_ERR || parent_গ্রন্থি == রিক্ত)
				ভঙ্গ;

			parent_box = box_for_গ্রন্থি(parent_গ্রন্থি);

			যদি (parent_box != রিক্ত) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_গ্রন্থি_unref(parent_গ্রন্থি);
				ভঙ্গ;
			} অন্যথা {
				যদি (current_গ্রন্থি != n)
					dom_গ্রন্থি_unref(current_গ্রন্থি);
				current_গ্রন্থি = parent_গ্রন্থি;
				parent_গ্রন্থি = রিক্ত;
			}
		}

		/* Find containing block (may be parent) */
		যাবত্ (true) {
			সংরচনা box *b;

			err = dom_গ্রন্থি_get_parent_গ্রন্থি(current_গ্রন্থি,
					&parent_গ্রন্থি);
			যদি (err != DOM_NO_ERR || parent_গ্রন্থি == রিক্ত) {
				যদি (current_গ্রন্থি != n)
					dom_গ্রন্থি_unref(current_গ্রন্থি);
				ভঙ্গ;
			}

			যদি (current_গ্রন্থি != n)
				dom_গ্রন্থি_unref(current_গ্রন্থি);

			b = box_for_গ্রন্থি(parent_গ্রন্থি);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			যদি (b != রিক্ত && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_গ্রন্থি_unref(parent_গ্রন্থি);
				ভঙ্গ;
			} অন্যথা {
				current_গ্রন্থি = parent_গ্রন্থি;
				parent_গ্রন্থি = রিক্ত;
			}
		}
	}

	/* Compute current inline container, if any */
	যদি (props->containing_block != রিক্ত &&
			props->containing_block->last != রিক্ত &&
			props->containing_block->last->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_block->last;
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
স্থির css_select_results *
box_get_style(html_content *c,
	      ধ্রুব css_computed_style *parent_style,
	      ধ্রুব css_computed_style *মূল_style,
	      dom_গ্রন্থি *n)
{
	dom_string *s = রিক্ত;
	css_stylesheet *inline_style = রিক্ত;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	যদি (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		যদি (err != DOM_NO_ERR) {
			প্রত্যাগম রিক্ত;
		}
	}

	যদি (s != রিক্ত) {
		inline_style = nscss_create_inline_style(
				(ধ্রুব uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		যদি (inline_style == রিক্ত)
			প্রত্যাগম রিক্ত;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.মূল_style = মূল_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	যদি (inline_style != রিক্ত)
		css_stylesheet_destroy(inline_style);

	প্রত্যাগম styles;
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
স্থির শূন্য
box_construct_generate(dom_গ্রন্থি *n,
		       html_content *content,
		       সংরচনা box *box,
		       ধ্রুব css_computed_style *style)
{
	সংরচনা box *gen = রিক্ত;
	গণনা css_display_e computed_display;
	ধ্রুব css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	যদি (box->type != BOX_BLOCK)
		প্রত্যাগম;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	যদি (style == রিক্ত ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		প্রত্যাগম;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_মূল(n));
	যদি (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(রিক্ত, (css_computed_style *) style,
				false, রিক্ত, রিক্ত, রিক্ত, রিক্ত, content->bctx);
		যদি (gen == রিক্ত) {
			প্রত্যাগম;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_মূল(n))];

		box_add_child(box, gen);
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
স্থির bool
box_construct_marker(সংরচনা box *box,
		     ধ্রুব বর্ণ *title,
		     সংরচনা box_construct_ctx *ctx,
		     সংরচনা box *parent)
{
	lwc_string *image_uri;
	সংরচনা box *marker;
	গণনা css_list_style_type_e list_style_type;

	marker = box_create(রিক্ত, box->style, false, রিক্ত, রিক্ত, title,
			রিক্ত, ctx->bctx);
	যদি (marker == false)
		প্রত্যাগম false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	বিকল্প (list_style_type) {
	পক্ষ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (বর্ণ *) "\342\200\242";
		marker->length = 3;
		ভঙ্গ;

	পক্ষ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (বর্ণ *) "\342\227\213";
		marker->length = 3;
		ভঙ্গ;

	পক্ষ CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (বর্ণ *) "\342\226\252";
		marker->length = 3;
		ভঙ্গ;

	স্বভাব:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	পক্ষ CSS_LIST_STYLE_TYPE_NONE:
		marker->text = রিক্ত;
		marker->length = 0;
		ভঙ্গ;
	}

	যদি (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != রিক্ত) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		যদি (error != NSERROR_OK)
			প্রত্যাগম false;

		যদি (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			প্রত্যাগম false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	প্রত্যাগম true;
}

স্থির অন্তঃস্থ bool box__style_is_float(ধ্রুব সংরচনা box *box)
{
	প্রত্যাগম css_computed_float(box->style) == CSS_FLOAT_বাম ||
	       css_computed_float(box->style) == CSS_FLOAT_ডান;
}

স্থির অন্তঃস্থ bool box__is_flex(ধ্রুব সংরচনা box *box)
{
	প্রত্যাগম box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

স্থির অন্তঃস্থ bool box__containing_block_is_flex(
		ধ্রুব সংরচনা box_construct_props *props)
{
	প্রত্যাগম props->containing_block != রিক্ত &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
স্থির bool
box_construct_element(সংরচনা box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = রিক্ত;
	গণনা css_display_e css_display;
	সংরচনা box *box = রিক্ত, *old_box;
	css_select_results *styles = রিক্ত;
	lwc_string *bgimage_uri;
	dom_exception err;
	সংরচনা box_construct_props props;
	ধ্রুব css_computed_style *মূল_style = রিক্ত;

	প্রতিজ্ঞা(ctx->n != রিক্ত);

	box_extract_properties(ctx->n, &props);

	যদি (props.containing_block != রিক্ত) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	যদি (props.গ্রন্থি_is_মূল == false) {
		মূল_style = ctx->মূল_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, মূল_style,
			ctx->n);
	যদি (styles == রিক্ত)
		প্রত্যাগম false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (title0 != রিক্ত) {
		বর্ণ *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		যদি (t == রিক্ত)
			প্রত্যাগম false;

		props.title = talloc_strdup(ctx->bctx, t);

		মুক্ত(t);

		যদি (props.title == রিক্ত)
			প্রত্যাগম false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (s != রিক্ত) {
		err = dom_string_intern(s, &id);
		যদি (err != DOM_NO_ERR)
			id = রিক্ত;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	যদি (box == রিক্ত)
		প্রত্যাগম false;

	/* If this is the root box, add it to the context */
	যদি (props.গ্রন্থি_is_মূল)
		ctx->মূল_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (s != রিক্ত) {
		ধ্রুব বর্ণ *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		যদি ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, রিক্ত, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (s != রিক্ত) {
		ধ্রুব বর্ণ *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		যদি ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, রিক্ত, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	যদি ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_BLOCK ||
			 css_display == CSS_DISPLAY_INLINE_TABLE ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_BLOCK];
	} অন্যথা যদি (props.গ্রন্থি_is_মূল) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} অন্যথা {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.গ্রন্থি_is_মূল)];

		যদি (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			বিকল্প (box->type) {
			পক্ষ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ভঙ্গ;
			পক্ষ BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				ভঙ্গ;
			স্বভাব:
				ভঙ্গ;
			}
		}
	}

	যদি (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		প্রত্যাগম false;
	}

	/* Handle the :before pseudo element */
	যদি (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	যদি (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.গ্রন্থি_is_মূল) == CSS_DISPLAY_NONE &&
			props.গ্রন্থি_is_মূল == false)) {
		css_select_results_destroy(styles);
		box->styles = রিক্ত;
		box->style = রিক্ত;

		/* Invalidate associated gadget, if any */
		যদি (box->gadget != রিক্ত) {
			box->gadget->box = রিক্ত;
			box->gadget = রিক্ত;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		প্রত্যাগম true;
	}

	/* Attach DOM node to box */
	err = dom_গ্রন্থি_set_user_data(ctx->n,
			corestring_dom_ns_চাবি_box_গ্রন্থি_data, box, রিক্ত,
			(শূন্য *) &old_box);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	/* Attach box to DOM node */
	box->গ্রন্থি = dom_গ্রন্থি_ref(ctx->n);

	যদি (props.inline_container == রিক্ত &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.গ্রন্থি_is_মূল == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		প্রতিজ্ঞা(props.containing_block != রিক্ত &&
				"Box must have containing block.");

		props.inline_container = box_create(রিক্ত, রিক্ত, false, রিক্ত,
				রিক্ত, রিক্ত, রিক্ত, ctx->bctx);
		যদি (props.inline_container == রিক্ত)
			প্রত্যাগম false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	যদি (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != রিক্ত &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		যদি (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			যদি (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				প্রত্যাগম false;
			}
			nsurl_unref(url);
		}
	}

	যদি (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	যদি (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		প্রতিজ্ঞা(props.inline_container != রিক্ত);

		box_add_child(props.inline_container, box);
	} অন্যথা {
		যদি (ns_computed_display(box->style, props.গ্রন্থি_is_মূল) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			যদি (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				প্রত্যাগম false;
		}

		যদি (props.গ্রন্থি_is_মূল == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_বাম ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ডান)) {
			/* Float: insert a float between the parent and box. */
			সংরচনা box *flt = box_create(রিক্ত, রিক্ত, false,
					props.href, props.target, props.title,
					রিক্ত, ctx->bctx);
			যদি (flt == রিক্ত)
				প্রত্যাগম false;

			যদি (css_computed_float(box->style) == CSS_FLOAT_বাম)
				flt->type = BOX_FLOAT_বাম;
			অন্যথা
				flt->type = BOX_FLOAT_ডান;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} অন্যথা {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			যদি (props.containing_block != রিক্ত)
				box_add_child(props.containing_block, box);
		}
	}

	প্রত্যাগম true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
স্থির শূন্য box_construct_element_after(dom_গ্রন্থি *n, html_content *content)
{
	সংরচনা box_construct_props props;
	সংরচনা box *box = box_for_গ্রন্থি(n);

	প্রতিজ্ঞা(box != রিক্ত);

	box_extract_properties(n, &props);

	যদি (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		সংরচনা box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_গ্রন্থি_has_child_nodes(n, &has_children);
		যদি (err != DOM_NO_ERR)
			প্রত্যাগম;

		যদি (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			প্রত্যাগম;
		}

		যদি (props.inline_container == রিক্ত) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(রিক্ত, রিক্ত, false,
					রিক্ত, রিক্ত, রিক্ত, রিক্ত, content->bctx);
			যদি (props.inline_container == রিক্ত)
				প্রত্যাগম;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(রিক্ত, box->style, false,
				box->href, box->target, box->title,
				box->id == রিক্ত ? রিক্ত :
				lwc_string_ref(box->id), content->bctx);
		যদি (inline_end != রিক্ত) {
			inline_end->type = BOX_INLINE_END;

			প্রতিজ্ঞা(props.inline_container != রিক্ত);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} অন্যথা যদি (!(box->flags & IS_REPLACED)) {
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
স্থির dom_গ্রন্থি *
next_গ্রন্থি(dom_গ্রন্থি *n, html_content *content, bool convert_children)
{
	dom_গ্রন্থি *next = রিক্ত;
	bool has_children;
	dom_exception err;

	err = dom_গ্রন্থি_has_child_nodes(n, &has_children);
	যদি (err != DOM_NO_ERR) {
		dom_গ্রন্থি_unref(n);
		প্রত্যাগম রিক্ত;
	}

	যদি (convert_children && has_children) {
		err = dom_গ্রন্থি_get_first_child(n, &next);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম রিক্ত;
		}
		dom_গ্রন্থি_unref(n);
	} অন্যথা {
		err = dom_গ্রন্থি_get_next_sibling(n, &next);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম রিক্ত;
		}

		যদি (next != রিক্ত) {
			যদি (box_for_গ্রন্থি(n) != রিক্ত)
				box_construct_element_after(n, content);
			dom_গ্রন্থি_unref(n);
		} অন্যথা {
			যদি (box_for_গ্রন্থি(n) != রিক্ত)
				box_construct_element_after(n, content);

			যাবত্ (box_is_মূল(n) == false) {
				dom_গ্রন্থি *parent = রিক্ত;
				dom_গ্রন্থি *parent_next = রিক্ত;

				err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(n);
					প্রত্যাগম রিক্ত;
				}

				প্রতিজ্ঞা(parent != রিক্ত);

				err = dom_গ্রন্থি_get_next_sibling(parent,
						&parent_next);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(parent);
					dom_গ্রন্থি_unref(n);
					প্রত্যাগম রিক্ত;
				}

				যদি (parent_next != রিক্ত) {
					dom_গ্রন্থি_unref(parent_next);
					dom_গ্রন্থি_unref(parent);
					ভঙ্গ;
				}

				dom_গ্রন্থি_unref(n);
				n = parent;
				parent = রিক্ত;

				যদি (box_for_গ্রন্থি(n) != রিক্ত) {
					box_construct_element_after(
							n, content);
				}
			}

			যদি (box_is_মূল(n) == false) {
				dom_গ্রন্থি *parent = রিক্ত;

				err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(n);
					প্রত্যাগম রিক্ত;
				}

				প্রতিজ্ঞা(parent != রিক্ত);

				err = dom_গ্রন্থি_get_next_sibling(parent, &next);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(parent);
					dom_গ্রন্থি_unref(n);
					প্রত্যাগম রিক্ত;
				}

				যদি (box_for_গ্রন্থি(parent) != রিক্ত) {
					box_construct_element_after(parent,
							content);
				}

				dom_গ্রন্থি_unref(parent);
			}

			dom_গ্রন্থি_unref(n);
		}
	}

	প্রত্যাগম next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
স্থির শূন্য
box_text_transform(বর্ণ *s, unsigned পূর্ণাঙ্ক len, গণনা css_text_transform_e tt)
{
	unsigned পূর্ণাঙ্ক i;
	যদি (len == 0)
		প্রত্যাগম;
	বিকল্প (tt) {
		পক্ষ CSS_TEXT_TRANSFORM_UPPERCASE:
			প্রতি (i = 0; i < len; ++i)
				যদি ((unsigned বর্ণ) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			ভঙ্গ;
		পক্ষ CSS_TEXT_TRANSFORM_LOWERCASE:
			প্রতি (i = 0; i < len; ++i)
				যদি ((unsigned বর্ণ) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			ভঙ্গ;
		পক্ষ CSS_TEXT_TRANSFORM_CAPITALIZE:
			যদি ((unsigned বর্ণ) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			প্রতি (i = 1; i < len; ++i)
				যদি ((unsigned বর্ণ) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			ভঙ্গ;
		স্বভাব:
			ভঙ্গ;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
স্থির bool box_construct_text(সংরচনা box_construct_ctx *ctx)
{
	সংরচনা box_construct_props props;
	সংরচনা box *box = রিক্ত;
	dom_string *content;
	dom_exception err;

	প্রতিজ্ঞা(ctx->n != রিক্ত);

	box_extract_properties(ctx->n, &props);

	প্রতিজ্ঞা(props.containing_block != রিক্ত);

	err = dom_characterdata_get_data(ctx->n, &content);
	যদি (err != DOM_NO_ERR || content == রিক্ত)
		প্রত্যাগম false;

	যদি (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		বর্ণ *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		যদি (text == রিক্ত)
			প্রত্যাগম false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		যদি (text[0] == ' ' && text[1] == 0) {
			যদি (props.inline_container != রিক্ত) {
				প্রতিজ্ঞা(props.inline_container->last != রিক্ত);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			মুক্ত(text);

			প্রত্যাগম true;
		}

		যদি (props.inline_container == রিক্ত) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(রিক্ত, রিক্ত, false,
					রিক্ত, রিক্ত, রিক্ত, রিক্ত, ctx->bctx);
			যদি (props.inline_container == রিক্ত) {
				মুক্ত(text);
				প্রত্যাগম false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(রিক্ত,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				রিক্ত, ctx->bctx);
		যদি (box == রিক্ত) {
			মুক্ত(text);
			প্রত্যাগম false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		মুক্ত(text);
		যদি (box->text == রিক্ত)
			প্রত্যাগম false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		যদি (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		যদি (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		যদি (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			যদি (box->prev != রিক্ত)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} অন্যথা {
		/* white-space: pre */
		বর্ণ *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		বর্ণ *current;
		গণনা css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		প্রতিজ্ঞা(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_সারি ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		যদি (text == রিক্ত)
			প্রত্যাগম false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		প্রতি (i = 0; i < text_len; i++)
			যদি (text[i] == '\t')
				text[i] = ' ';

		যদি (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		যদি (props.containing_block->flags & PRE_STRIP) {
			বিকল্প (*current) {
			পক্ষ '\n':
				current++;
				ভঙ্গ;
			পক্ষ '\r':
				current++;
				যদি (*current == '\n')
					current++;
				ভঙ্গ;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		কুরু {
			size_t len = strcspn(current, "\r\n");

			বর্ণ old = current[len];

			current[len] = 0;

			যদি (props.inline_container == রিক্ত) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(রিক্ত, রিক্ত,
						false, রিক্ত, রিক্ত, রিক্ত, রিক্ত,
						ctx->bctx);
				যদি (props.inline_container == রিক্ত) {
					মুক্ত(text);
					প্রত্যাগম false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(রিক্ত,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				রিক্ত, ctx->bctx);
			যদি (box == রিক্ত) {
				মুক্ত(text);
				প্রত্যাগম false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			যদি (box->text == রিক্ত) {
				মুক্ত(text);
				প্রত্যাগম false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			যদি (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(রিক্ত, রিক্ত,
						false, রিক্ত, রিক্ত, রিক্ত, রিক্ত,
						ctx->bctx);
				যদি (props.inline_container == রিক্ত) {
					মুক্ত(text);
					প্রত্যাগম false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				যদি (current[0] == '\r' && current[1] == '\n')
					current += 2;
				অন্যথা
					current++;
			}
		} যাবত্ (*current);

		মুক্ত(text);
	}

	প্রত্যাগম true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
স্থির শূন্য convert_xml_to_box(সংরচনা box_construct_ctx *ctx)
{
	dom_গ্রন্থি *next;
	bool convert_children;
	uint32_t অঙ্ক_processed = 0;
	ধ্রুব uint32_t max_processed_before_yield = 10;

	কুরু {
		convert_children = true;

		প্রতিজ্ঞা(ctx->n != রিক্ত);

		যদি (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_গ্রন্থি_unref(ctx->n);
			মুক্ত(ctx);
			প্রত্যাগম;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_গ্রন্থি(ctx->n, ctx->content, convert_children);
		যাবত্ (next != রিক্ত) {
			dom_গ্রন্থি_type type;
			dom_exception err;

			err = dom_গ্রন্থি_get_গ্রন্থি_type(next, &type);
			যদি (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_গ্রন্থি_unref(next);
				মুক্ত(ctx);
				প্রত্যাগম;
			}

			যদি (type == DOM_ELEMENT_গ্রন্থি)
				ভঙ্গ;

			যদি (type == DOM_TEXT_গ্রন্থি) {
				ctx->n = next;
				যদি (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_গ্রন্থি_unref(ctx->n);
					মুক্ত(ctx);
					প্রত্যাগম;
				}
			}

			next = next_গ্রন্থি(next, ctx->content, true);
		}

		ctx->n = next;

		যদি (next == রিক্ত) {
			/* Conversion complete */
			সংরচনা box মূল;

			memset(&মূল, 0, পরিমাণ(মূল));

			মূল.type = BOX_BLOCK;
			মূল.children = মূল.last = ctx->মূল_box;
			মূল.children->parent = &মূল;

			/** \todo Remove box_normalise_block */
			যদি (box_normalise_block(&মূল, ctx->মূল_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} অন্যথা {
				ctx->content->layout = মূল.children;
				ctx->content->layout->parent = রিক্ত;

				ctx->cb(ctx->content, true);
			}

			প্রতিজ্ঞা(ctx->n == রিক্ত);

			মুক্ত(ctx);
			প্রত্যাগম;
		}
	} যাবত্ (++অঙ্ক_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (শূন্য *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_গ্রন্থি *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   শূন্য **box_conversion_context)
{
	সংরচনা box_construct_ctx *ctx;

	প্রতিজ্ঞা(box_conversion_context != রিক্ত);

	যদি (c->bctx == রিক্ত) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, পূর্ণাঙ্ক);
		যদি (c->bctx == রিক্ত) {
			প্রত্যাগম NSERROR_NOMEM;
		}
	}

	ctx = malloc(পরিমাণ(*ctx));
	যদি (ctx == রিক্ত) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_গ্রন্থি_ref(n);
	ctx->মূল_box = রিক্ত;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	প্রত্যাগম guit->misc->schedule(0, (শূন্য *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(শূন্য *box_conversion_context)
{
	সংরচনা box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (শূন্য *)convert_xml_to_box, ctx);
	যদি (err != NSERROR_OK) {
		প্রত্যাগম err;
	}

	dom_গ্রন্থি_unref(ctx->n);
	মুক্ত(ctx);

	প্রত্যাগম NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
সংরচনা box *box_for_গ্রন্থি(dom_গ্রন্থি *n)
{
	সংরচনা box *box = রিক্ত;
	dom_exception err;

	err = dom_গ্রন্থি_get_user_data(n, corestring_dom_ns_চাবি_box_গ্রন্থি_data,
			(শূন্য *) &box);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম রিক্ত;

	প্রত্যাগম box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ধ্রুব html_content *content,
		 ধ্রুব dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	বর্ণ *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned পূর্ণাঙ্ক i, j, end;
	nserror error;
	ধ্রুব বর্ণ *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	যদি (!s)
		প্রত্যাগম false;

	/* copy to s, removing white space and control characters */
	প্রতি (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	প্রতি (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	প্রতি (j = 0; i != end; i++) {
		যদি ((unsigned বর্ণ) rel[i] < 0x20) {
			; /* skip control characters */
		} অন্যথা যদি (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} অন্যথা {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	যদি (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		যদি (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			যদি (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			যদি (quot0)
				quot1 = strchr(quot0 + 1, '"');
			যদি (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} অন্যথা যদি (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	মুক্ত(s);
	যদি (error != NSERROR_OK) {
		*result = রিক্ত;
		প্রত্যাগম false;
	}

	প্রত্যাগম true;
}
