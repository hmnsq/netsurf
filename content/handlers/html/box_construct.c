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

#ضمن <string.h>
#ضمن <dom/dom.h>

#ضمن "utils/errors.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/talloc.h"
#ضمن "utils/string.h"
#ضمن "utils/ascii.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/utils.h"
#ضمن "netsurf/misc.h"
#ضمن "css/select.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/private.h"
#ضمن "html/object.h"
#ضمن "html/box.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/box_construct.h"
#ضمن "html/box_special.h"
#ضمن "html/box_normalise.h"
#ضمن "html/form_internal.h"

/**
 * Context for box tree construction
 */
بنية box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_node *n;			/**< Current node to process */

	بنية box *root_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	صحيح *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
بنية box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ثابت css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	بنية nsurl *href;
	/** Current frame target, or NULL if none */
	ثابت حرف *target;
	/** Current title attribute, or NULL if none */
	ثابت حرف *title;
	/** Identity of the current block-level container */
	بنية box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	بنية box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool node_is_root;
};

ساكن ثابت content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ساكن ثابت box_type box_map[] = {
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
ساكن مضمن bool box_is_root(dom_node *n)
{
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (parent != عدم) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		ܐܢ (err != DOM_NO_ERR)
			ܦܢܐ false;

		ܐܢ (type != DOM_DOCUMENT_NODE)
			ܦܢܐ false;
	}

	ܦܢܐ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ساكن فراغ
box_extract_properties(dom_node *n, بنية box_construct_props *props)
{
	memset(props, 0, حجم(*props));

	props->node_is_root = box_is_root(n);

	/* Extract properties from containing DOM node */
	ܐܢ (props->node_is_root == false) {
		dom_node *current_node = n;
		dom_node *parent_node = عدم;
		بنية box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ܟܡܐ (true) {
			err = dom_node_get_parent_node(current_node,
					&parent_node);
			ܐܢ (err != DOM_NO_ERR || parent_node == عدم)
				اكسر;

			parent_box = box_for_node(parent_node);

			ܐܢ (parent_box != عدم) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_node_unref(parent_node);
				اكسر;
			} ܐܚܪܢܐ {
				ܐܢ (current_node != n)
					dom_node_unref(current_node);
				current_node = parent_node;
				parent_node = عدم;
			}
		}

		/* Find containing block (may be parent) */
		ܟܡܐ (true) {
			بنية box *b;

			err = dom_node_get_parent_node(current_node,
					&parent_node);
			ܐܢ (err != DOM_NO_ERR || parent_node == عدم) {
				ܐܢ (current_node != n)
					dom_node_unref(current_node);
				اكسر;
			}

			ܐܢ (current_node != n)
				dom_node_unref(current_node);

			b = box_for_node(parent_node);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ܐܢ (b != عدم && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_node_unref(parent_node);
				اكسر;
			} ܐܚܪܢܐ {
				current_node = parent_node;
				parent_node = عدم;
			}
		}
	}

	/* Compute current inline container, if any */
	ܐܢ (props->containing_block != عدم &&
			props->containing_block->last != عدم &&
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
ساكن css_select_results *
box_get_style(html_content *c,
	      ثابت css_computed_style *parent_style,
	      ثابت css_computed_style *root_style,
	      dom_node *n)
{
	dom_string *s = عدم;
	css_stylesheet *inline_style = عدم;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ܐܢ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ܐܢ (err != DOM_NO_ERR) {
			ܦܢܐ عدم;
		}
	}

	ܐܢ (s != عدم) {
		inline_style = nscss_create_inline_style(
				(ثابت uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		ܐܢ (inline_style == عدم)
			ܦܢܐ عدم;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.root_style = root_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ܐܢ (inline_style != عدم)
		css_stylesheet_destroy(inline_style);

	ܦܢܐ styles;
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
ساكن فراغ
box_construct_generate(dom_node *n,
		       html_content *content,
		       بنية box *box,
		       ثابت css_computed_style *style)
{
	بنية box *gen = عدم;
	تعداد css_display_e computed_display;
	ثابت css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	ܐܢ (box->type != BOX_BLOCK)
		ܦܢܐ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ܐܢ (style == عدم ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		ܦܢܐ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_root(n));
	ܐܢ (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(عدم, (css_computed_style *) style,
				false, عدم, عدم, عدم, عدم, content->bctx);
		ܐܢ (gen == عدم) {
			ܦܢܐ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_root(n))];

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
ساكن bool
box_construct_marker(بنية box *box,
		     ثابت حرف *title,
		     بنية box_construct_ctx *ctx,
		     بنية box *parent)
{
	lwc_string *image_uri;
	بنية box *marker;
	تعداد css_list_style_type_e list_style_type;

	marker = box_create(عدم, box->style, false, عدم, عدم, title,
			عدم, ctx->bctx);
	ܐܢ (marker == false)
		ܦܢܐ false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	بدل (list_style_type) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (حرف *) "\342\200\242";
		marker->length = 3;
		اكسر;

	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (حرف *) "\342\227\213";
		marker->length = 3;
		اكسر;

	حالة CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (حرف *) "\342\226\252";
		marker->length = 3;
		اكسر;

	افتراضي:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_NONE:
		marker->text = عدم;
		marker->length = 0;
		اكسر;
	}

	ܐܢ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != عدم) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		ܐܢ (error != NSERROR_OK)
			ܦܢܐ false;

		ܐܢ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ܦܢܐ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	ܦܢܐ true;
}

ساكن مضمن bool box__style_is_float(ثابت بنية box *box)
{
	ܦܢܐ css_computed_float(box->style) == CSS_FLOAT_LEFT ||
	       css_computed_float(box->style) == CSS_FLOAT_RIGHT;
}

ساكن مضمن bool box__is_flex(ثابت بنية box *box)
{
	ܦܢܐ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ساكن مضمن bool box__containing_block_is_flex(
		ثابت بنية box_construct_props *props)
{
	ܦܢܐ props->containing_block != عدم &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ساكن bool
box_construct_element(بنية box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = عدم;
	تعداد css_display_e css_display;
	بنية box *box = عدم, *old_box;
	css_select_results *styles = عدم;
	lwc_string *bgimage_uri;
	dom_exception err;
	بنية box_construct_props props;
	ثابت css_computed_style *root_style = عدم;

	أكد(ctx->n != عدم);

	box_extract_properties(ctx->n, &props);

	ܐܢ (props.containing_block != عدم) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	ܐܢ (props.node_is_root == false) {
		root_style = ctx->root_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, root_style,
			ctx->n);
	ܐܢ (styles == عدم)
		ܦܢܐ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (title0 != عدم) {
		حرف *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		ܐܢ (t == عدم)
			ܦܢܐ false;

		props.title = talloc_strdup(ctx->bctx, t);

		حرر(t);

		ܐܢ (props.title == عدم)
			ܦܢܐ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (s != عدم) {
		err = dom_string_intern(s, &id);
		ܐܢ (err != DOM_NO_ERR)
			id = عدم;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	ܐܢ (box == عدم)
		ܦܢܐ false;

	/* If this is the root box, add it to the context */
	ܐܢ (props.node_is_root)
		ctx->root_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (s != عدم) {
		ثابت حرف *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ܐܢ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, عدم, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (s != عدم) {
		ثابت حرف *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ܐܢ ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, عدم, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ܐܢ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} ܐܚܪܢܐ ܐܢ (props.node_is_root) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} ܐܚܪܢܐ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.node_is_root)];

		ܐܢ (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			بدل (box->type) {
			حالة BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				اكسر;
			حالة BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				اكسر;
			افتراضي:
				اكسر;
			}
		}
	}

	ܐܢ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ܦܢܐ false;
	}

	/* Handle the :before pseudo element */
	ܐܢ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ܐܢ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.node_is_root) == CSS_DISPLAY_NONE &&
			props.node_is_root == false)) {
		css_select_results_destroy(styles);
		box->styles = عدم;
		box->style = عدم;

		/* Invalidate associated gadget, if any */
		ܐܢ (box->gadget != عدم) {
			box->gadget->box = عدم;
			box->gadget = عدم;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ܦܢܐ true;
	}

	/* Attach DOM node to box */
	err = dom_node_set_user_data(ctx->n,
			corestring_dom___ns_key_box_node_data, box, عدم,
			(فراغ *) &old_box);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	/* Attach box to DOM node */
	box->node = dom_node_ref(ctx->n);

	ܐܢ (props.inline_container == عدم &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.node_is_root == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		أكد(props.containing_block != عدم &&
				"Box must have containing block.");

		props.inline_container = box_create(عدم, عدم, false, عدم,
				عدم, عدم, عدم, ctx->bctx);
		ܐܢ (props.inline_container == عدم)
			ܦܢܐ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ܐܢ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != عدم &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		ܐܢ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ܐܢ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ܦܢܐ false;
			}
			nsurl_unref(url);
		}
	}

	ܐܢ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ܐܢ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		أكد(props.inline_container != عدم);

		box_add_child(props.inline_container, box);
	} ܐܚܪܢܐ {
		ܐܢ (ns_computed_display(box->style, props.node_is_root) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			ܐܢ (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				ܦܢܐ false;
		}

		ܐܢ (props.node_is_root == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_LEFT ||
				css_computed_float(box->style) ==
				CSS_FLOAT_RIGHT)) {
			/* Float: insert a float between the parent and box. */
			بنية box *flt = box_create(عدم, عدم, false,
					props.href, props.target, props.title,
					عدم, ctx->bctx);
			ܐܢ (flt == عدم)
				ܦܢܐ false;

			ܐܢ (css_computed_float(box->style) == CSS_FLOAT_LEFT)
				flt->type = BOX_FLOAT_LEFT;
			ܐܚܪܢܐ
				flt->type = BOX_FLOAT_RIGHT;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} ܐܚܪܢܐ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ܐܢ (props.containing_block != عدم)
				box_add_child(props.containing_block, box);
		}
	}

	ܦܢܐ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ساكن فراغ box_construct_element_after(dom_node *n, html_content *content)
{
	بنية box_construct_props props;
	بنية box *box = box_for_node(n);

	أكد(box != عدم);

	box_extract_properties(n, &props);

	ܐܢ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		بنية box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_node_has_child_nodes(n, &has_children);
		ܐܢ (err != DOM_NO_ERR)
			ܦܢܐ;

		ܐܢ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ܦܢܐ;
		}

		ܐܢ (props.inline_container == عدم) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(عدم, عدم, false,
					عدم, عدم, عدم, عدم, content->bctx);
			ܐܢ (props.inline_container == عدم)
				ܦܢܐ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(عدم, box->style, false,
				box->href, box->target, box->title,
				box->id == عدم ? عدم :
				lwc_string_ref(box->id), content->bctx);
		ܐܢ (inline_end != عدم) {
			inline_end->type = BOX_INLINE_END;

			أكد(props.inline_container != عدم);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} ܐܚܪܢܐ ܐܢ (!(box->flags & IS_REPLACED)) {
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
ساكن dom_node *
next_node(dom_node *n, html_content *content, bool convert_children)
{
	dom_node *next = عدم;
	bool has_children;
	dom_exception err;

	err = dom_node_has_child_nodes(n, &has_children);
	ܐܢ (err != DOM_NO_ERR) {
		dom_node_unref(n);
		ܦܢܐ عدم;
	}

	ܐܢ (convert_children && has_children) {
		err = dom_node_get_first_child(n, &next);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(n);
			ܦܢܐ عدم;
		}
		dom_node_unref(n);
	} ܐܚܪܢܐ {
		err = dom_node_get_next_sibling(n, &next);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(n);
			ܦܢܐ عدم;
		}

		ܐܢ (next != عدم) {
			ܐܢ (box_for_node(n) != عدم)
				box_construct_element_after(n, content);
			dom_node_unref(n);
		} ܐܚܪܢܐ {
			ܐܢ (box_for_node(n) != عدم)
				box_construct_element_after(n, content);

			ܟܡܐ (box_is_root(n) == false) {
				dom_node *parent = عدم;
				dom_node *parent_next = عدم;

				err = dom_node_get_parent_node(n, &parent);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(n);
					ܦܢܐ عدم;
				}

				أكد(parent != عدم);

				err = dom_node_get_next_sibling(parent,
						&parent_next);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(parent);
					dom_node_unref(n);
					ܦܢܐ عدم;
				}

				ܐܢ (parent_next != عدم) {
					dom_node_unref(parent_next);
					dom_node_unref(parent);
					اكسر;
				}

				dom_node_unref(n);
				n = parent;
				parent = عدم;

				ܐܢ (box_for_node(n) != عدم) {
					box_construct_element_after(
							n, content);
				}
			}

			ܐܢ (box_is_root(n) == false) {
				dom_node *parent = عدم;

				err = dom_node_get_parent_node(n, &parent);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(n);
					ܦܢܐ عدم;
				}

				أكد(parent != عدم);

				err = dom_node_get_next_sibling(parent, &next);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(parent);
					dom_node_unref(n);
					ܦܢܐ عدم;
				}

				ܐܢ (box_for_node(parent) != عدم) {
					box_construct_element_after(parent,
							content);
				}

				dom_node_unref(parent);
			}

			dom_node_unref(n);
		}
	}

	ܦܢܐ next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ساكن فراغ
box_text_transform(حرف *s, unsigned صحيح len, تعداد css_text_transform_e tt)
{
	unsigned صحيح i;
	ܐܢ (len == 0)
		ܦܢܐ;
	بدل (tt) {
		حالة CSS_TEXT_TRANSFORM_UPPERCASE:
			لكل (i = 0; i < len; ++i)
				ܐܢ ((unsigned حرف) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			اكسر;
		حالة CSS_TEXT_TRANSFORM_LOWERCASE:
			لكل (i = 0; i < len; ++i)
				ܐܢ ((unsigned حرف) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			اكسر;
		حالة CSS_TEXT_TRANSFORM_CAPITALIZE:
			ܐܢ ((unsigned حرف) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			لكل (i = 1; i < len; ++i)
				ܐܢ ((unsigned حرف) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			اكسر;
		افتراضي:
			اكسر;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ساكن bool box_construct_text(بنية box_construct_ctx *ctx)
{
	بنية box_construct_props props;
	بنية box *box = عدم;
	dom_string *content;
	dom_exception err;

	أكد(ctx->n != عدم);

	box_extract_properties(ctx->n, &props);

	أكد(props.containing_block != عدم);

	err = dom_characterdata_get_data(ctx->n, &content);
	ܐܢ (err != DOM_NO_ERR || content == عدم)
		ܦܢܐ false;

	ܐܢ (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		حرف *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		ܐܢ (text == عدم)
			ܦܢܐ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ܐܢ (text[0] == ' ' && text[1] == 0) {
			ܐܢ (props.inline_container != عدم) {
				أكد(props.inline_container->last != عدم);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			حرر(text);

			ܦܢܐ true;
		}

		ܐܢ (props.inline_container == عدم) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(عدم, عدم, false,
					عدم, عدم, عدم, عدم, ctx->bctx);
			ܐܢ (props.inline_container == عدم) {
				حرر(text);
				ܦܢܐ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(عدم,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				عدم, ctx->bctx);
		ܐܢ (box == عدم) {
			حرر(text);
			ܦܢܐ false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		حرر(text);
		ܐܢ (box->text == عدم)
			ܦܢܐ false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		ܐܢ (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		ܐܢ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		ܐܢ (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			ܐܢ (box->prev != عدم)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} ܐܚܪܢܐ {
		/* white-space: pre */
		حرف *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		حرف *current;
		تعداد css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		أكد(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_LINE ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		ܐܢ (text == عدم)
			ܦܢܐ false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		لكل (i = 0; i < text_len; i++)
			ܐܢ (text[i] == '\t')
				text[i] = ' ';

		ܐܢ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		ܐܢ (props.containing_block->flags & PRE_STRIP) {
			بدل (*current) {
			حالة '\n':
				current++;
				اكسر;
			حالة '\r':
				current++;
				ܐܢ (*current == '\n')
					current++;
				اكسر;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		ܥܒܕ {
			size_t len = strcspn(current, "\r\n");

			حرف old = current[len];

			current[len] = 0;

			ܐܢ (props.inline_container == عدم) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(عدم, عدم,
						false, عدم, عدم, عدم, عدم,
						ctx->bctx);
				ܐܢ (props.inline_container == عدم) {
					حرر(text);
					ܦܢܐ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(عدم,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				عدم, ctx->bctx);
			ܐܢ (box == عدم) {
				حرر(text);
				ܦܢܐ false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			ܐܢ (box->text == عدم) {
				حرر(text);
				ܦܢܐ false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			ܐܢ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(عدم, عدم,
						false, عدم, عدم, عدم, عدم,
						ctx->bctx);
				ܐܢ (props.inline_container == عدم) {
					حرر(text);
					ܦܢܐ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				ܐܢ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ܐܚܪܢܐ
					current++;
			}
		} ܟܡܐ (*current);

		حرر(text);
	}

	ܦܢܐ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ساكن فراغ convert_xml_to_box(بنية box_construct_ctx *ctx)
{
	dom_node *next;
	bool convert_children;
	uint32_t num_processed = 0;
	ثابت uint32_t max_processed_before_yield = 10;

	ܥܒܕ {
		convert_children = true;

		أكد(ctx->n != عدم);

		ܐܢ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_node_unref(ctx->n);
			حرر(ctx);
			ܦܢܐ;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_node(ctx->n, ctx->content, convert_children);
		ܟܡܐ (next != عدم) {
			dom_node_type type;
			dom_exception err;

			err = dom_node_get_node_type(next, &type);
			ܐܢ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_node_unref(next);
				حرر(ctx);
				ܦܢܐ;
			}

			ܐܢ (type == DOM_ELEMENT_NODE)
				اكسر;

			ܐܢ (type == DOM_TEXT_NODE) {
				ctx->n = next;
				ܐܢ (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_node_unref(ctx->n);
					حرر(ctx);
					ܦܢܐ;
				}
			}

			next = next_node(next, ctx->content, true);
		}

		ctx->n = next;

		ܐܢ (next == عدم) {
			/* Conversion complete */
			بنية box root;

			memset(&root, 0, حجم(root));

			root.type = BOX_BLOCK;
			root.children = root.last = ctx->root_box;
			root.children->parent = &root;

			/** \todo Remove box_normalise_block */
			ܐܢ (box_normalise_block(&root, ctx->root_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ܐܚܪܢܐ {
				ctx->content->layout = root.children;
				ctx->content->layout->parent = عدم;

				ctx->cb(ctx->content, true);
			}

			أكد(ctx->n == عدم);

			حرر(ctx);
			ܦܢܐ;
		}
	} ܟܡܐ (++num_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (فراغ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_node *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   فراغ **box_conversion_context)
{
	بنية box_construct_ctx *ctx;

	أكد(box_conversion_context != عدم);

	ܐܢ (c->bctx == عدم) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, صحيح);
		ܐܢ (c->bctx == عدم) {
			ܦܢܐ NSERROR_NOMEM;
		}
	}

	ctx = malloc(حجم(*ctx));
	ܐܢ (ctx == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_node_ref(n);
	ctx->root_box = عدم;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ܦܢܐ guit->misc->schedule(0, (فراغ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(فراغ *box_conversion_context)
{
	بنية box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (فراغ *)convert_xml_to_box, ctx);
	ܐܢ (err != NSERROR_OK) {
		ܦܢܐ err;
	}

	dom_node_unref(ctx->n);
	حرر(ctx);

	ܦܢܐ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
بنية box *box_for_node(dom_node *n)
{
	بنية box *box = عدم;
	dom_exception err;

	err = dom_node_get_user_data(n, corestring_dom___ns_key_box_node_data,
			(فراغ *) &box);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ عدم;

	ܦܢܐ box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ثابت html_content *content,
		 ثابت dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	حرف *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned صحيح i, j, end;
	nserror error;
	ثابت حرف *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	ܐܢ (!s)
		ܦܢܐ false;

	/* copy to s, removing white space and control characters */
	لكل (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	لكل (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	لكل (j = 0; i != end; i++) {
		ܐܢ ((unsigned حرف) rel[i] < 0x20) {
			; /* skip control characters */
		} ܐܚܪܢܐ ܐܢ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ܐܚܪܢܐ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ܐܢ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ܐܢ (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ܐܢ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ܐܢ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ܐܢ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ܐܚܪܢܐ ܐܢ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	حرر(s);
	ܐܢ (error != NSERROR_OK) {
		*result = عدم;
		ܦܢܐ false;
	}

	ܦܢܐ true;
}
