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

#شامل <string.h>
#شامل <dom/dom.h>

#شامل "utils/errors.h"
#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/talloc.h"
#شامل "utils/string.h"
#شامل "utils/ascii.h"
#شامل "utils/nsurl.h"
#شامل "utils/utils.h"
#شامل "netsurf/misc.h"
#شامل "css/select.h"
#شامل "desktop/gui_internal.h"

#شامل "html/private.h"
#شامل "html/object.h"
#شامل "html/box.h"
#شامل "html/box_manipulate.h"
#شامل "html/box_construct.h"
#شامل "html/box_special.h"
#شامل "html/box_normalise.h"
#شامل "html/form_internal.h"

/**
 * Context for box tree construction
 */
ساختار box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_گره *n;			/**< Current node to process */

	ساختار box *ریشه_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	صحیح *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ساختار box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ثابت css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	ساختار nsurl *href;
	/** Current frame target, or NULL if none */
	ثابت نویسه *target;
	/** Current title attribute, or NULL if none */
	ثابت نویسه *title;
	/** Identity of the current block-level container */
	ساختار box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ساختار box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool گره_is_ریشه;
};

ایستا ثابت content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ایستا ثابت box_type box_map[] = {
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
ایستا درونخطی bool box_is_ریشه(dom_گره *n)
{
	dom_گره *parent;
	dom_گره_type type;
	dom_exception err;

	err = dom_گره_get_parent_گره(n, &parent);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (parent != پوچ) {
		err = dom_گره_get_گره_type(parent, &type);

		dom_گره_unref(parent);

		اگر (err != DOM_NO_ERR)
			بازگشت false;

		اگر (type != DOM_DOCUMENT_گره)
			بازگشت false;
	}

	بازگشت true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ایستا تهی
box_extract_properties(dom_گره *n, ساختار box_construct_props *props)
{
	memset(props, 0, اندازه(*props));

	props->گره_is_ریشه = box_is_ریشه(n);

	/* Extract properties from containing DOM node */
	اگر (props->گره_is_ریشه == false) {
		dom_گره *current_گره = n;
		dom_گره *parent_گره = پوچ;
		ساختار box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		وقتی (true) {
			err = dom_گره_get_parent_گره(current_گره,
					&parent_گره);
			اگر (err != DOM_NO_ERR || parent_گره == پوچ)
				بشکن;

			parent_box = box_for_گره(parent_گره);

			اگر (parent_box != پوچ) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_گره_unref(parent_گره);
				بشکن;
			} وگرنه {
				اگر (current_گره != n)
					dom_گره_unref(current_گره);
				current_گره = parent_گره;
				parent_گره = پوچ;
			}
		}

		/* Find containing block (may be parent) */
		وقتی (true) {
			ساختار box *b;

			err = dom_گره_get_parent_گره(current_گره,
					&parent_گره);
			اگر (err != DOM_NO_ERR || parent_گره == پوچ) {
				اگر (current_گره != n)
					dom_گره_unref(current_گره);
				بشکن;
			}

			اگر (current_گره != n)
				dom_گره_unref(current_گره);

			b = box_for_گره(parent_گره);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			اگر (b != پوچ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_گره_unref(parent_گره);
				بشکن;
			} وگرنه {
				current_گره = parent_گره;
				parent_گره = پوچ;
			}
		}
	}

	/* Compute current inline container, if any */
	اگر (props->containing_block != پوچ &&
			props->containing_block->last != پوچ &&
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
ایستا css_select_results *
box_get_style(html_content *c,
	      ثابت css_computed_style *parent_style,
	      ثابت css_computed_style *ریشه_style,
	      dom_گره *n)
{
	dom_string *s = پوچ;
	css_stylesheet *inline_style = پوچ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	اگر (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		اگر (err != DOM_NO_ERR) {
			بازگشت پوچ;
		}
	}

	اگر (s != پوچ) {
		inline_style = nscss_create_inline_style(
				(ثابت uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		اگر (inline_style == پوچ)
			بازگشت پوچ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.ریشه_style = ریشه_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	اگر (inline_style != پوچ)
		css_stylesheet_destroy(inline_style);

	بازگشت styles;
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
ایستا تهی
box_construct_generate(dom_گره *n,
		       html_content *content,
		       ساختار box *box,
		       ثابت css_computed_style *style)
{
	ساختار box *gen = پوچ;
	شمارش css_display_e computed_display;
	ثابت css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	اگر (box->type != BOX_BLOCK)
		بازگشت;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	اگر (style == پوچ ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		بازگشت;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_ریشه(n));
	اگر (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(پوچ, (css_computed_style *) style,
				false, پوچ, پوچ, پوچ, پوچ, content->bctx);
		اگر (gen == پوچ) {
			بازگشت;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_ریشه(n))];

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
ایستا bool
box_construct_marker(ساختار box *box,
		     ثابت نویسه *title,
		     ساختار box_construct_ctx *ctx,
		     ساختار box *parent)
{
	lwc_string *image_uri;
	ساختار box *marker;
	شمارش css_list_style_type_e list_style_type;

	marker = box_create(پوچ, box->style, false, پوچ, پوچ, title,
			پوچ, ctx->bctx);
	اگر (marker == false)
		بازگشت false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	انتخاب (list_style_type) {
	حالت CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (نویسه *) "\342\200\242";
		marker->length = 3;
		بشکن;

	حالت CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (نویسه *) "\342\227\213";
		marker->length = 3;
		بشکن;

	حالت CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (نویسه *) "\342\226\252";
		marker->length = 3;
		بشکن;

	پیشفرض:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_NONE:
		marker->text = پوچ;
		marker->length = 0;
		بشکن;
	}

	اگر (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != پوچ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		اگر (error != NSERROR_OK)
			بازگشت false;

		اگر (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			بازگشت false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	بازگشت true;
}

ایستا درونخطی bool box__style_is_float(ثابت ساختار box *box)
{
	بازگشت css_computed_float(box->style) == CSS_FLOAT_چپ ||
	       css_computed_float(box->style) == CSS_FLOAT_راست;
}

ایستا درونخطی bool box__is_flex(ثابت ساختار box *box)
{
	بازگشت box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ایستا درونخطی bool box__containing_block_is_flex(
		ثابت ساختار box_construct_props *props)
{
	بازگشت props->containing_block != پوچ &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ایستا bool
box_construct_element(ساختار box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = پوچ;
	شمارش css_display_e css_display;
	ساختار box *box = پوچ, *old_box;
	css_select_results *styles = پوچ;
	lwc_string *bgimage_uri;
	dom_exception err;
	ساختار box_construct_props props;
	ثابت css_computed_style *ریشه_style = پوچ;

	أكد(ctx->n != پوچ);

	box_extract_properties(ctx->n, &props);

	اگر (props.containing_block != پوچ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	اگر (props.گره_is_ریشه == false) {
		ریشه_style = ctx->ریشه_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, ریشه_style,
			ctx->n);
	اگر (styles == پوچ)
		بازگشت false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (title0 != پوچ) {
		نویسه *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		اگر (t == پوچ)
			بازگشت false;

		props.title = talloc_strdup(ctx->bctx, t);

		آزاد(t);

		اگر (props.title == پوچ)
			بازگشت false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (s != پوچ) {
		err = dom_string_intern(s, &id);
		اگر (err != DOM_NO_ERR)
			id = پوچ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	اگر (box == پوچ)
		بازگشت false;

	/* If this is the root box, add it to the context */
	اگر (props.گره_is_ریشه)
		ctx->ریشه_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (s != پوچ) {
		ثابت نویسه *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		اگر ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, پوچ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (s != پوچ) {
		ثابت نویسه *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		اگر ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, پوچ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	اگر ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} وگرنه اگر (props.گره_is_ریشه) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} وگرنه {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.گره_is_ریشه)];

		اگر (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			انتخاب (box->type) {
			حالت BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				بشکن;
			حالت BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				بشکن;
			پیشفرض:
				بشکن;
			}
		}
	}

	اگر (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		بازگشت false;
	}

	/* Handle the :before pseudo element */
	اگر (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	اگر (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.گره_is_ریشه) == CSS_DISPLAY_NONE &&
			props.گره_is_ریشه == false)) {
		css_select_results_destroy(styles);
		box->styles = پوچ;
		box->style = پوچ;

		/* Invalidate associated gadget, if any */
		اگر (box->gadget != پوچ) {
			box->gadget->box = پوچ;
			box->gadget = پوچ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		بازگشت true;
	}

	/* Attach DOM node to box */
	err = dom_گره_set_user_data(ctx->n,
			corestring_dom_ns_کلید_box_گره_data, box, پوچ,
			(تهی *) &old_box);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	/* Attach box to DOM node */
	box->گره = dom_گره_ref(ctx->n);

	اگر (props.inline_container == پوچ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.گره_is_ریشه == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		أكد(props.containing_block != پوچ &&
				"Box must have containing block.");

		props.inline_container = box_create(پوچ, پوچ, false, پوچ,
				پوچ, پوچ, پوچ, ctx->bctx);
		اگر (props.inline_container == پوچ)
			بازگشت false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	اگر (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != پوچ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		اگر (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			اگر (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				بازگشت false;
			}
			nsurl_unref(url);
		}
	}

	اگر (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	اگر (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		أكد(props.inline_container != پوچ);

		box_add_child(props.inline_container, box);
	} وگرنه {
		اگر (ns_computed_display(box->style, props.گره_is_ریشه) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			اگر (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				بازگشت false;
		}

		اگر (props.گره_is_ریشه == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_چپ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_راست)) {
			/* Float: insert a float between the parent and box. */
			ساختار box *flt = box_create(پوچ, پوچ, false,
					props.href, props.target, props.title,
					پوچ, ctx->bctx);
			اگر (flt == پوچ)
				بازگشت false;

			اگر (css_computed_float(box->style) == CSS_FLOAT_چپ)
				flt->type = BOX_FLOAT_چپ;
			وگرنه
				flt->type = BOX_FLOAT_راست;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} وگرنه {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			اگر (props.containing_block != پوچ)
				box_add_child(props.containing_block, box);
		}
	}

	بازگشت true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ایستا تهی box_construct_element_after(dom_گره *n, html_content *content)
{
	ساختار box_construct_props props;
	ساختار box *box = box_for_گره(n);

	أكد(box != پوچ);

	box_extract_properties(n, &props);

	اگر (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ساختار box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_گره_has_child_nodes(n, &has_children);
		اگر (err != DOM_NO_ERR)
			بازگشت;

		اگر (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			بازگشت;
		}

		اگر (props.inline_container == پوچ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(پوچ, پوچ, false,
					پوچ, پوچ, پوچ, پوچ, content->bctx);
			اگر (props.inline_container == پوچ)
				بازگشت;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(پوچ, box->style, false,
				box->href, box->target, box->title,
				box->id == پوچ ? پوچ :
				lwc_string_ref(box->id), content->bctx);
		اگر (inline_end != پوچ) {
			inline_end->type = BOX_INLINE_END;

			أكد(props.inline_container != پوچ);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} وگرنه اگر (!(box->flags & IS_REPLACED)) {
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
ایستا dom_گره *
next_گره(dom_گره *n, html_content *content, bool convert_children)
{
	dom_گره *next = پوچ;
	bool has_children;
	dom_exception err;

	err = dom_گره_has_child_nodes(n, &has_children);
	اگر (err != DOM_NO_ERR) {
		dom_گره_unref(n);
		بازگشت پوچ;
	}

	اگر (convert_children && has_children) {
		err = dom_گره_get_first_child(n, &next);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت پوچ;
		}
		dom_گره_unref(n);
	} وگرنه {
		err = dom_گره_get_next_sibling(n, &next);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت پوچ;
		}

		اگر (next != پوچ) {
			اگر (box_for_گره(n) != پوچ)
				box_construct_element_after(n, content);
			dom_گره_unref(n);
		} وگرنه {
			اگر (box_for_گره(n) != پوچ)
				box_construct_element_after(n, content);

			وقتی (box_is_ریشه(n) == false) {
				dom_گره *parent = پوچ;
				dom_گره *parent_next = پوچ;

				err = dom_گره_get_parent_گره(n, &parent);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(n);
					بازگشت پوچ;
				}

				أكد(parent != پوچ);

				err = dom_گره_get_next_sibling(parent,
						&parent_next);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(parent);
					dom_گره_unref(n);
					بازگشت پوچ;
				}

				اگر (parent_next != پوچ) {
					dom_گره_unref(parent_next);
					dom_گره_unref(parent);
					بشکن;
				}

				dom_گره_unref(n);
				n = parent;
				parent = پوچ;

				اگر (box_for_گره(n) != پوچ) {
					box_construct_element_after(
							n, content);
				}
			}

			اگر (box_is_ریشه(n) == false) {
				dom_گره *parent = پوچ;

				err = dom_گره_get_parent_گره(n, &parent);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(n);
					بازگشت پوچ;
				}

				أكد(parent != پوچ);

				err = dom_گره_get_next_sibling(parent, &next);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(parent);
					dom_گره_unref(n);
					بازگشت پوچ;
				}

				اگر (box_for_گره(parent) != پوچ) {
					box_construct_element_after(parent,
							content);
				}

				dom_گره_unref(parent);
			}

			dom_گره_unref(n);
		}
	}

	بازگشت next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ایستا تهی
box_text_transform(نویسه *s, unsigned صحیح len, شمارش css_text_transform_e tt)
{
	unsigned صحیح i;
	اگر (len == 0)
		بازگشت;
	انتخاب (tt) {
		حالت CSS_TEXT_TRANSFORM_UPPERCASE:
			برای (i = 0; i < len; ++i)
				اگر ((unsigned نویسه) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			بشکن;
		حالت CSS_TEXT_TRANSFORM_LOWERCASE:
			برای (i = 0; i < len; ++i)
				اگر ((unsigned نویسه) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			بشکن;
		حالت CSS_TEXT_TRANSFORM_CAPITALIZE:
			اگر ((unsigned نویسه) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			برای (i = 1; i < len; ++i)
				اگر ((unsigned نویسه) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			بشکن;
		پیشفرض:
			بشکن;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ایستا bool box_construct_text(ساختار box_construct_ctx *ctx)
{
	ساختار box_construct_props props;
	ساختار box *box = پوچ;
	dom_string *content;
	dom_exception err;

	أكد(ctx->n != پوچ);

	box_extract_properties(ctx->n, &props);

	أكد(props.containing_block != پوچ);

	err = dom_characterdata_get_data(ctx->n, &content);
	اگر (err != DOM_NO_ERR || content == پوچ)
		بازگشت false;

	اگر (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		نویسه *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		اگر (text == پوچ)
			بازگشت false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		اگر (text[0] == ' ' && text[1] == 0) {
			اگر (props.inline_container != پوچ) {
				أكد(props.inline_container->last != پوچ);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			آزاد(text);

			بازگشت true;
		}

		اگر (props.inline_container == پوچ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(پوچ, پوچ, false,
					پوچ, پوچ, پوچ, پوچ, ctx->bctx);
			اگر (props.inline_container == پوچ) {
				آزاد(text);
				بازگشت false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(پوچ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				پوچ, ctx->bctx);
		اگر (box == پوچ) {
			آزاد(text);
			بازگشت false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		آزاد(text);
		اگر (box->text == پوچ)
			بازگشت false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		اگر (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		اگر (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		اگر (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			اگر (box->prev != پوچ)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} وگرنه {
		/* white-space: pre */
		نویسه *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		نویسه *current;
		شمارش css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		أكد(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_سطر ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		اگر (text == پوچ)
			بازگشت false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		برای (i = 0; i < text_len; i++)
			اگر (text[i] == '\t')
				text[i] = ' ';

		اگر (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		اگر (props.containing_block->flags & PRE_STRIP) {
			انتخاب (*current) {
			حالت '\n':
				current++;
				بشکن;
			حالت '\r':
				current++;
				اگر (*current == '\n')
					current++;
				بشکن;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		انجامبده {
			size_t len = strcspn(current, "\r\n");

			نویسه old = current[len];

			current[len] = 0;

			اگر (props.inline_container == پوچ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(پوچ, پوچ,
						false, پوچ, پوچ, پوچ, پوچ,
						ctx->bctx);
				اگر (props.inline_container == پوچ) {
					آزاد(text);
					بازگشت false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(پوچ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				پوچ, ctx->bctx);
			اگر (box == پوچ) {
				آزاد(text);
				بازگشت false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			اگر (box->text == پوچ) {
				آزاد(text);
				بازگشت false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			اگر (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(پوچ, پوچ,
						false, پوچ, پوچ, پوچ, پوچ,
						ctx->bctx);
				اگر (props.inline_container == پوچ) {
					آزاد(text);
					بازگشت false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				اگر (current[0] == '\r' && current[1] == '\n')
					current += 2;
				وگرنه
					current++;
			}
		} وقتی (*current);

		آزاد(text);
	}

	بازگشت true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ایستا تهی convert_xml_to_box(ساختار box_construct_ctx *ctx)
{
	dom_گره *next;
	bool convert_children;
	uint32_t رقم_processed = 0;
	ثابت uint32_t max_processed_before_yield = 10;

	انجامبده {
		convert_children = true;

		أكد(ctx->n != پوچ);

		اگر (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_گره_unref(ctx->n);
			آزاد(ctx);
			بازگشت;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_گره(ctx->n, ctx->content, convert_children);
		وقتی (next != پوچ) {
			dom_گره_type type;
			dom_exception err;

			err = dom_گره_get_گره_type(next, &type);
			اگر (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_گره_unref(next);
				آزاد(ctx);
				بازگشت;
			}

			اگر (type == DOM_ELEMENT_گره)
				بشکن;

			اگر (type == DOM_TEXT_گره) {
				ctx->n = next;
				اگر (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_گره_unref(ctx->n);
					آزاد(ctx);
					بازگشت;
				}
			}

			next = next_گره(next, ctx->content, true);
		}

		ctx->n = next;

		اگر (next == پوچ) {
			/* Conversion complete */
			ساختار box ریشه;

			memset(&ریشه, 0, اندازه(ریشه));

			ریشه.type = BOX_BLOCK;
			ریشه.children = ریشه.last = ctx->ریشه_box;
			ریشه.children->parent = &ریشه;

			/** \todo Remove box_normalise_block */
			اگر (box_normalise_block(&ریشه, ctx->ریشه_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} وگرنه {
				ctx->content->layout = ریشه.children;
				ctx->content->layout->parent = پوچ;

				ctx->cb(ctx->content, true);
			}

			أكد(ctx->n == پوچ);

			آزاد(ctx);
			بازگشت;
		}
	} وقتی (++رقم_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (تهی *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_گره *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   تهی **box_conversion_context)
{
	ساختار box_construct_ctx *ctx;

	أكد(box_conversion_context != پوچ);

	اگر (c->bctx == پوچ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, صحیح);
		اگر (c->bctx == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
	}

	ctx = malloc(اندازه(*ctx));
	اگر (ctx == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_گره_ref(n);
	ctx->ریشه_box = پوچ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	بازگشت guit->misc->schedule(0, (تهی *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(تهی *box_conversion_context)
{
	ساختار box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (تهی *)convert_xml_to_box, ctx);
	اگر (err != NSERROR_OK) {
		بازگشت err;
	}

	dom_گره_unref(ctx->n);
	آزاد(ctx);

	بازگشت NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ساختار box *box_for_گره(dom_گره *n)
{
	ساختار box *box = پوچ;
	dom_exception err;

	err = dom_گره_get_user_data(n, corestring_dom_ns_کلید_box_گره_data,
			(تهی *) &box);
	اگر (err != DOM_NO_ERR)
		بازگشت پوچ;

	بازگشت box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ثابت html_content *content,
		 ثابت dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	نویسه *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned صحیح i, j, end;
	nserror error;
	ثابت نویسه *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	اگر (!s)
		بازگشت false;

	/* copy to s, removing white space and control characters */
	برای (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	برای (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	برای (j = 0; i != end; i++) {
		اگر ((unsigned نویسه) rel[i] < 0x20) {
			; /* skip control characters */
		} وگرنه اگر (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} وگرنه {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	اگر (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		اگر (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			اگر (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			اگر (quot0)
				quot1 = strchr(quot0 + 1, '"');
			اگر (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} وگرنه اگر (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	آزاد(s);
	اگر (error != NSERROR_OK) {
		*result = پوچ;
		بازگشت false;
	}

	بازگشت true;
}
