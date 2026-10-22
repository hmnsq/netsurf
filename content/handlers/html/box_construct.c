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
কাঠামো box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_গ্রন্থি *n;			/**< Current node to process */

	কাঠামো box *মূল_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	পূর্ণসংখ্যা *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
কাঠামো box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ধ্রুবক css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	কাঠামো nsurl *href;
	/** Current frame target, or NULL if none */
	ধ্রুবক অক্ষর *target;
	/** Current title attribute, or NULL if none */
	ধ্রুবক অক্ষর *title;
	/** Identity of the current block-level container */
	কাঠামো box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	কাঠামো box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool গ্রন্থি_is_মূল;
};

স্থির ধ্রুবক content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
স্থির ধ্রুবক box_type box_map[] = {
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
স্থির অন্তঃরেখ bool box_is_মূল(dom_গ্রন্থি *n)
{
	dom_গ্রন্থি *parent;
	dom_গ্রন্থি_type type;
	dom_exception err;

	err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	যদি (parent != ফাঁকা) {
		err = dom_গ্রন্থি_get_গ্রন্থি_type(parent, &type);

		dom_গ্রন্থি_unref(parent);

		যদি (err != DOM_NO_ERR)
			ফেরত false;

		যদি (type != DOM_DOCUMENT_গ্রন্থি)
			ফেরত false;
	}

	ফেরত true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
স্থির শূন্য
box_extract_properties(dom_গ্রন্থি *n, কাঠামো box_construct_props *props)
{
	memset(props, 0, আকার(*props));

	props->গ্রন্থি_is_মূল = box_is_মূল(n);

	/* Extract properties from containing DOM node */
	যদি (props->গ্রন্থি_is_মূল == false) {
		dom_গ্রন্থি *current_গ্রন্থি = n;
		dom_গ্রন্থি *parent_গ্রন্থি = ফাঁকা;
		কাঠামো box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		যখন (true) {
			err = dom_গ্রন্থি_get_parent_গ্রন্থি(current_গ্রন্থি,
					&parent_গ্রন্থি);
			যদি (err != DOM_NO_ERR || parent_গ্রন্থি == ফাঁকা)
				বিরতি;

			parent_box = box_for_গ্রন্থি(parent_গ্রন্থি);

			যদি (parent_box != ফাঁকা) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_গ্রন্থি_unref(parent_গ্রন্থি);
				বিরতি;
			} অন্যথা {
				যদি (current_গ্রন্থি != n)
					dom_গ্রন্থি_unref(current_গ্রন্থি);
				current_গ্রন্থি = parent_গ্রন্থি;
				parent_গ্রন্থি = ফাঁকা;
			}
		}

		/* Find containing block (may be parent) */
		যখন (true) {
			কাঠামো box *b;

			err = dom_গ্রন্থি_get_parent_গ্রন্থি(current_গ্রন্থি,
					&parent_গ্রন্থি);
			যদি (err != DOM_NO_ERR || parent_গ্রন্থি == ফাঁকা) {
				যদি (current_গ্রন্থি != n)
					dom_গ্রন্থি_unref(current_গ্রন্থি);
				বিরতি;
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
			যদি (b != ফাঁকা && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_গ্রন্থি_unref(parent_গ্রন্থি);
				বিরতি;
			} অন্যথা {
				current_গ্রন্থি = parent_গ্রন্থি;
				parent_গ্রন্থি = ফাঁকা;
			}
		}
	}

	/* Compute current inline container, if any */
	যদি (props->containing_block != ফাঁকা &&
			props->containing_block->last != ফাঁকা &&
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
	      ধ্রুবক css_computed_style *parent_style,
	      ধ্রুবক css_computed_style *মূল_style,
	      dom_গ্রন্থি *n)
{
	dom_string *s = ফাঁকা;
	css_stylesheet *inline_style = ফাঁকা;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	যদি (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		যদি (err != DOM_NO_ERR) {
			ফেরত ফাঁকা;
		}
	}

	যদি (s != ফাঁকা) {
		inline_style = nscss_create_inline_style(
				(ধ্রুবক uint8_t *) dom_string_তথ্য(s),
				dom_string_byte_দৈর্ঘ্য(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		যদি (inline_style == ফাঁকা)
			ফেরত ফাঁকা;
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
	যদি (inline_style != ফাঁকা)
		css_stylesheet_destroy(inline_style);

	ফেরত styles;
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
		       কাঠামো box *box,
		       ধ্রুবক css_computed_style *style)
{
	কাঠামো box *gen = ফাঁকা;
	গণনা css_display_e computed_display;
	ধ্রুবক css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	যদি (box->type != BOX_BLOCK)
		ফেরত;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	যদি (style == ফাঁকা ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		ফেরত;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_মূল(n));
	যদি (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ফাঁকা, (css_computed_style *) style,
				false, ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা, content->bctx);
		যদি (gen == ফাঁকা) {
			ফেরত;
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
box_construct_marker(কাঠামো box *box,
		     ধ্রুবক অক্ষর *title,
		     কাঠামো box_construct_ctx *ctx,
		     কাঠামো box *parent)
{
	lwc_string *image_uri;
	কাঠামো box *marker;
	গণনা css_list_style_type_e list_style_type;

	marker = box_create(ফাঁকা, box->style, false, ফাঁকা, ফাঁকা, title,
			ফাঁকা, ctx->bctx);
	যদি (marker == false)
		ফেরত false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	নির্বাচন (list_style_type) {
	ক্ষেত্র CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (অক্ষর *) "\342\200\242";
		marker->দৈর্ঘ্য = 3;
		বিরতি;

	ক্ষেত্র CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (অক্ষর *) "\342\227\213";
		marker->দৈর্ঘ্য = 3;
		বিরতি;

	ক্ষেত্র CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (অক্ষর *) "\342\226\252";
		marker->দৈর্ঘ্য = 3;
		বিরতি;

	পূর্বনির্ধারিত:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	ক্ষেত্র CSS_LIST_STYLE_TYPE_NONE:
		marker->text = ফাঁকা;
		marker->দৈর্ঘ্য = 0;
		বিরতি;
	}

	যদি (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ফাঁকা) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_তথ্য(image_uri), &url);
		যদি (error != NSERROR_OK)
			ফেরত false;

		যদি (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ফেরত false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	ফেরত true;
}

স্থির অন্তঃরেখ bool box__style_is_float(ধ্রুবক কাঠামো box *box)
{
	ফেরত css_computed_float(box->style) == CSS_FLOAT_বাম ||
	       css_computed_float(box->style) == CSS_FLOAT_ডান;
}

স্থির অন্তঃরেখ bool box__is_flex(ধ্রুবক কাঠামো box *box)
{
	ফেরত box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

স্থির অন্তঃরেখ bool box__containing_block_is_flex(
		ধ্রুবক কাঠামো box_construct_props *props)
{
	ফেরত props->containing_block != ফাঁকা &&
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
box_construct_element(কাঠামো box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ফাঁকা;
	গণনা css_display_e css_display;
	কাঠামো box *box = ফাঁকা, *old_box;
	css_select_results *styles = ফাঁকা;
	lwc_string *bgimage_uri;
	dom_exception err;
	কাঠামো box_construct_props props;
	ধ্রুবক css_computed_style *মূল_style = ফাঁকা;

	প্রতিজ্ঞা(ctx->n != ফাঁকা);

	box_extract_properties(ctx->n, &props);

	যদি (props.containing_block != ফাঁকা) {
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
	যদি (styles == ফাঁকা)
		ফেরত false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	যদি (title0 != ফাঁকা) {
		অক্ষর *t = squash_whitespace(dom_string_তথ্য(title0));

		dom_string_unref(title0);

		যদি (t == ফাঁকা)
			ফেরত false;

		props.title = talloc_strdup(ctx->bctx, t);

		মুক্ত(t);

		যদি (props.title == ফাঁকা)
			ফেরত false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	যদি (s != ফাঁকা) {
		err = dom_string_intern(s, &id);
		যদি (err != DOM_NO_ERR)
			id = ফাঁকা;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	যদি (box == ফাঁকা)
		ফেরত false;

	/* If this is the root box, add it to the context */
	যদি (props.গ্রন্থি_is_মূল)
		ctx->মূল_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	যদি (s != ফাঁকা) {
		ধ্রুবক অক্ষর *val = dom_string_তথ্য(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		যদি ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ফাঁকা, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	যদি (s != ফাঁকা) {
		ধ্রুবক অক্ষর *val = dom_string_তথ্য(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		যদি ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, ফাঁকা, 10), 0, 65534);

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
			নির্বাচন (box->type) {
			ক্ষেত্র BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				বিরতি;
			ক্ষেত্র BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				বিরতি;
			পূর্বনির্ধারিত:
				বিরতি;
			}
		}
	}

	যদি (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ফেরত false;
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
		box->styles = ফাঁকা;
		box->style = ফাঁকা;

		/* Invalidate associated gadget, if any */
		যদি (box->gadget != ফাঁকা) {
			box->gadget->box = ফাঁকা;
			box->gadget = ফাঁকা;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ফেরত true;
	}

	/* Attach DOM node to box */
	err = dom_গ্রন্থি_set_user_তথ্য(ctx->n,
			corestring_dom_ns_চাবি_box_গ্রন্থি_তথ্য, box, ফাঁকা,
			(শূন্য *) &old_box);
	যদি (err != DOM_NO_ERR)
		ফেরত false;

	/* Attach box to DOM node */
	box->গ্রন্থি = dom_গ্রন্থি_ref(ctx->n);

	যদি (props.inline_container == ফাঁকা &&
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
		প্রতিজ্ঞা(props.containing_block != ফাঁকা &&
				"Box must have containing block.");

		props.inline_container = box_create(ফাঁকা, ফাঁকা, false, ফাঁকা,
				ফাঁকা, ফাঁকা, ফাঁকা, ctx->bctx);
		যদি (props.inline_container == ফাঁকা)
			ফেরত false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	যদি (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ফাঁকা &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_তথ্য(bgimage_uri), &url);
		যদি (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			যদি (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ফেরত false;
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
		প্রতিজ্ঞা(props.inline_container != ফাঁকা);

		box_add_child(props.inline_container, box);
	} অন্যথা {
		যদি (ns_computed_display(box->style, props.গ্রন্থি_is_মূল) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			যদি (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				ফেরত false;
		}

		যদি (props.গ্রন্থি_is_মূল == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_বাম ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ডান)) {
			/* Float: insert a float between the parent and box. */
			কাঠামো box *flt = box_create(ফাঁকা, ফাঁকা, false,
					props.href, props.target, props.title,
					ফাঁকা, ctx->bctx);
			যদি (flt == ফাঁকা)
				ফেরত false;

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
			যদি (props.containing_block != ফাঁকা)
				box_add_child(props.containing_block, box);
		}
	}

	ফেরত true;
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
	কাঠামো box_construct_props props;
	কাঠামো box *box = box_for_গ্রন্থি(n);

	প্রতিজ্ঞা(box != ফাঁকা);

	box_extract_properties(n, &props);

	যদি (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		কাঠামো box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_গ্রন্থি_has_child_nodes(n, &has_children);
		যদি (err != DOM_NO_ERR)
			ফেরত;

		যদি (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ফেরত;
		}

		যদি (props.inline_container == ফাঁকা) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ফাঁকা, ফাঁকা, false,
					ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা, content->bctx);
			যদি (props.inline_container == ফাঁকা)
				ফেরত;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(ফাঁকা, box->style, false,
				box->href, box->target, box->title,
				box->id == ফাঁকা ? ফাঁকা :
				lwc_string_ref(box->id), content->bctx);
		যদি (inline_end != ফাঁকা) {
			inline_end->type = BOX_INLINE_END;

			প্রতিজ্ঞা(props.inline_container != ফাঁকা);

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
পরবর্তী_গ্রন্থি(dom_গ্রন্থি *n, html_content *content, bool convert_children)
{
	dom_গ্রন্থি *পরবর্তী = ফাঁকা;
	bool has_children;
	dom_exception err;

	err = dom_গ্রন্থি_has_child_nodes(n, &has_children);
	যদি (err != DOM_NO_ERR) {
		dom_গ্রন্থি_unref(n);
		ফেরত ফাঁকা;
	}

	যদি (convert_children && has_children) {
		err = dom_গ্রন্থি_get_first_child(n, &পরবর্তী);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			ফেরত ফাঁকা;
		}
		dom_গ্রন্থি_unref(n);
	} অন্যথা {
		err = dom_গ্রন্থি_get_পরবর্তী_sibling(n, &পরবর্তী);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			ফেরত ফাঁকা;
		}

		যদি (পরবর্তী != ফাঁকা) {
			যদি (box_for_গ্রন্থি(n) != ফাঁকা)
				box_construct_element_after(n, content);
			dom_গ্রন্থি_unref(n);
		} অন্যথা {
			যদি (box_for_গ্রন্থি(n) != ফাঁকা)
				box_construct_element_after(n, content);

			যখন (box_is_মূল(n) == false) {
				dom_গ্রন্থি *parent = ফাঁকা;
				dom_গ্রন্থি *parent_পরবর্তী = ফাঁকা;

				err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(n);
					ফেরত ফাঁকা;
				}

				প্রতিজ্ঞা(parent != ফাঁকা);

				err = dom_গ্রন্থি_get_পরবর্তী_sibling(parent,
						&parent_পরবর্তী);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(parent);
					dom_গ্রন্থি_unref(n);
					ফেরত ফাঁকা;
				}

				যদি (parent_পরবর্তী != ফাঁকা) {
					dom_গ্রন্থি_unref(parent_পরবর্তী);
					dom_গ্রন্থি_unref(parent);
					বিরতি;
				}

				dom_গ্রন্থি_unref(n);
				n = parent;
				parent = ফাঁকা;

				যদি (box_for_গ্রন্থি(n) != ফাঁকা) {
					box_construct_element_after(
							n, content);
				}
			}

			যদি (box_is_মূল(n) == false) {
				dom_গ্রন্থি *parent = ফাঁকা;

				err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(n);
					ফেরত ফাঁকা;
				}

				প্রতিজ্ঞা(parent != ফাঁকা);

				err = dom_গ্রন্থি_get_পরবর্তী_sibling(parent, &পরবর্তী);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(parent);
					dom_গ্রন্থি_unref(n);
					ফেরত ফাঁকা;
				}

				যদি (box_for_গ্রন্থি(parent) != ফাঁকা) {
					box_construct_element_after(parent,
							content);
				}

				dom_গ্রন্থি_unref(parent);
			}

			dom_গ্রন্থি_unref(n);
		}
	}

	ফেরত পরবর্তী;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
স্থির শূন্য
box_text_transform(অক্ষর *s, unsigned পূর্ণসংখ্যা len, গণনা css_text_transform_e tt)
{
	unsigned পূর্ণসংখ্যা i;
	যদি (len == 0)
		ফেরত;
	নির্বাচন (tt) {
		ক্ষেত্র CSS_TEXT_TRANSFORM_UPPERCASE:
			জন্য (i = 0; i < len; ++i)
				যদি ((unsigned অক্ষর) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			বিরতি;
		ক্ষেত্র CSS_TEXT_TRANSFORM_LOWERCASE:
			জন্য (i = 0; i < len; ++i)
				যদি ((unsigned অক্ষর) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			বিরতি;
		ক্ষেত্র CSS_TEXT_TRANSFORM_CAPITALIZE:
			যদি ((unsigned অক্ষর) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			জন্য (i = 1; i < len; ++i)
				যদি ((unsigned অক্ষর) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			বিরতি;
		পূর্বনির্ধারিত:
			বিরতি;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
স্থির bool box_construct_text(কাঠামো box_construct_ctx *ctx)
{
	কাঠামো box_construct_props props;
	কাঠামো box *box = ফাঁকা;
	dom_string *content;
	dom_exception err;

	প্রতিজ্ঞা(ctx->n != ফাঁকা);

	box_extract_properties(ctx->n, &props);

	প্রতিজ্ঞা(props.containing_block != ফাঁকা);

	err = dom_characterdata_get_তথ্য(ctx->n, &content);
	যদি (err != DOM_NO_ERR || content == ফাঁকা)
		ফেরত false;

	যদি (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		অক্ষর *text;

		text = squash_whitespace(dom_string_তথ্য(content));

		dom_string_unref(content);

		যদি (text == ফাঁকা)
			ফেরত false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		যদি (text[0] == ' ' && text[1] == 0) {
			যদি (props.inline_container != ফাঁকা) {
				প্রতিজ্ঞা(props.inline_container->last != ফাঁকা);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			মুক্ত(text);

			ফেরত true;
		}

		যদি (props.inline_container == ফাঁকা) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ফাঁকা, ফাঁকা, false,
					ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা, ctx->bctx);
			যদি (props.inline_container == ফাঁকা) {
				মুক্ত(text);
				ফেরত false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ফাঁকা,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ফাঁকা, ctx->bctx);
		যদি (box == ফাঁকা) {
			মুক্ত(text);
			ফেরত false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		মুক্ত(text);
		যদি (box->text == ফাঁকা)
			ফেরত false;

		box->দৈর্ঘ্য = strlen(box->text);

		/* strip ending space char off */
		যদি (box->দৈর্ঘ্য > 1 && box->text[box->দৈর্ঘ্য - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->দৈর্ঘ্য--;
		}

		যদি (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->দৈর্ঘ্য,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		যদি (box->text[0] == ' ') {
			box->দৈর্ঘ্য--;

			memmove(box->text, &box->text[1], box->দৈর্ঘ্য);

			যদি (box->পূর্ব != ফাঁকা)
				box->পূর্ব->space = UNKNOWN_WIDTH;
		}
	} অন্যথা {
		/* white-space: pre */
		অক্ষর *text;
		size_t text_len = dom_string_byte_দৈর্ঘ্য(content);
		size_t i;
		অক্ষর *current;
		গণনা css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		প্রতিজ্ঞা(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_সারি ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		যদি (text == ফাঁকা)
			ফেরত false;

		memcpy(text, dom_string_তথ্য(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		জন্য (i = 0; i < text_len; i++)
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
			নির্বাচন (*current) {
			ক্ষেত্র '\n':
				current++;
				বিরতি;
			ক্ষেত্র '\r':
				current++;
				যদি (*current == '\n')
					current++;
				বিরতি;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		করো {
			size_t len = strcspn(current, "\r\n");

			অক্ষর old = current[len];

			current[len] = 0;

			যদি (props.inline_container == ফাঁকা) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ফাঁকা, ফাঁকা,
						false, ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা,
						ctx->bctx);
				যদি (props.inline_container == ফাঁকা) {
					মুক্ত(text);
					ফেরত false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ফাঁকা,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ফাঁকা, ctx->bctx);
			যদি (box == ফাঁকা) {
				মুক্ত(text);
				ফেরত false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			যদি (box->text == ফাঁকা) {
				মুক্ত(text);
				ফেরত false;
			}

			box->দৈর্ঘ্য = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			যদি (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ফাঁকা, ফাঁকা,
						false, ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা,
						ctx->bctx);
				যদি (props.inline_container == ফাঁকা) {
					মুক্ত(text);
					ফেরত false;
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
		} যখন (*current);

		মুক্ত(text);
	}

	ফেরত true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
স্থির শূন্য convert_xml_to_box(কাঠামো box_construct_ctx *ctx)
{
	dom_গ্রন্থি *পরবর্তী;
	bool convert_children;
	uint32_t অঙ্ক_processed = 0;
	ধ্রুবক uint32_t max_processed_before_yield = 10;

	করো {
		convert_children = true;

		প্রতিজ্ঞা(ctx->n != ফাঁকা);

		যদি (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_গ্রন্থি_unref(ctx->n);
			মুক্ত(ctx);
			ফেরত;
		}

		/* Find next element to process, converting text nodes as we go */
		পরবর্তী = পরবর্তী_গ্রন্থি(ctx->n, ctx->content, convert_children);
		যখন (পরবর্তী != ফাঁকা) {
			dom_গ্রন্থি_type type;
			dom_exception err;

			err = dom_গ্রন্থি_get_গ্রন্থি_type(পরবর্তী, &type);
			যদি (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_গ্রন্থি_unref(পরবর্তী);
				মুক্ত(ctx);
				ফেরত;
			}

			যদি (type == DOM_ELEMENT_গ্রন্থি)
				বিরতি;

			যদি (type == DOM_TEXT_গ্রন্থি) {
				ctx->n = পরবর্তী;
				যদি (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_গ্রন্থি_unref(ctx->n);
					মুক্ত(ctx);
					ফেরত;
				}
			}

			পরবর্তী = পরবর্তী_গ্রন্থি(পরবর্তী, ctx->content, true);
		}

		ctx->n = পরবর্তী;

		যদি (পরবর্তী == ফাঁকা) {
			/* Conversion complete */
			কাঠামো box মূল;

			memset(&মূল, 0, আকার(মূল));

			মূল.type = BOX_BLOCK;
			মূল.children = মূল.last = ctx->মূল_box;
			মূল.children->parent = &মূল;

			/** \todo Remove box_normalise_block */
			যদি (box_normalise_block(&মূল, ctx->মূল_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} অন্যথা {
				ctx->content->layout = মূল.children;
				ctx->content->layout->parent = ফাঁকা;

				ctx->cb(ctx->content, true);
			}

			প্রতিজ্ঞা(ctx->n == ফাঁকা);

			মুক্ত(ctx);
			ফেরত;
		}
	} যখন (++অঙ্ক_processed < max_processed_before_yield);

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
	কাঠামো box_construct_ctx *ctx;

	প্রতিজ্ঞা(box_conversion_context != ফাঁকা);

	যদি (c->bctx == ফাঁকা) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, পূর্ণসংখ্যা);
		যদি (c->bctx == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
	}

	ctx = malloc(আকার(*ctx));
	যদি (ctx == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_গ্রন্থি_ref(n);
	ctx->মূল_box = ফাঁকা;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ফেরত guit->misc->schedule(0, (শূন্য *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(শূন্য *box_conversion_context)
{
	কাঠামো box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (শূন্য *)convert_xml_to_box, ctx);
	যদি (err != NSERROR_OK) {
		ফেরত err;
	}

	dom_গ্রন্থি_unref(ctx->n);
	মুক্ত(ctx);

	ফেরত NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
কাঠামো box *box_for_গ্রন্থি(dom_গ্রন্থি *n)
{
	কাঠামো box *box = ফাঁকা;
	dom_exception err;

	err = dom_গ্রন্থি_get_user_তথ্য(n, corestring_dom_ns_চাবি_box_গ্রন্থি_তথ্য,
			(শূন্য *) &box);
	যদি (err != DOM_NO_ERR)
		ফেরত ফাঁকা;

	ফেরত box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ধ্রুবক html_content *content,
		 ধ্রুবক dom_string *dsrel,
		 nsurl *base,
		 nsurl **ফলাফল)
{
	অক্ষর *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned পূর্ণসংখ্যা i, j, end;
	nserror error;
	ধ্রুবক অক্ষর *rel;

	rel = dom_string_তথ্য(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	যদি (!s)
		ফেরত false;

	/* copy to s, removing white space and control characters */
	জন্য (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	জন্য (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	জন্য (j = 0; i != end; i++) {
		যদি ((unsigned অক্ষর) rel[i] < 0x20) {
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
	error = nsurl_join(base, s1, ফলাফল);
	মুক্ত(s);
	যদি (error != NSERROR_OK) {
		*ফলাফল = ফাঁকা;
		ফেরত false;
	}

	ফেরত true;
}
