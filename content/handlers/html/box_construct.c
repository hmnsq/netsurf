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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/string.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "css/select.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/object.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_manipulate.h"
#அந்தர்பாவ "html/box_construct.h"
#அந்தர்பாவ "html/box_special.h"
#அந்தர்பாவ "html/box_normalise.h"
#அந்தர்பாவ "html/form_internal.h"

/**
 * Context for box tree construction
 */
கட்டமைப்பு box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_முனை *n;			/**< Current node to process */

	கட்டமைப்பு box *வேர்_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	முழு *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
கட்டமைப்பு box_construct_props {
	/** Style from which to inherit, or NULL if none */
	மாறிலி css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	கட்டமைப்பு nsurl *href;
	/** Current frame target, or NULL if none */
	மாறிலி எழுத்து *target;
	/** Current title attribute, or NULL if none */
	மாறிலி எழுத்து *title;
	/** Identity of the current block-level container */
	கட்டமைப்பு box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	கட்டமைப்பு box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool முனை_is_வேர்;
};

நிலையான மாறிலி content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
நிலையான மாறிலி box_type box_map[] = {
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
நிலையான உள்வரி bool box_is_வேர்(dom_முனை *n)
{
	dom_முனை *parent;
	dom_முனை_type type;
	dom_exception err;

	err = dom_முனை_get_parent_முனை(n, &parent);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (parent != வெற்று) {
		err = dom_முனை_get_முனை_type(parent, &type);

		dom_முனை_unref(parent);

		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		எனில் (type != DOM_DOCUMENT_முனை)
			திரும்பு false;
	}

	திரும்பு true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
நிலையான வெறுமை
box_extract_properties(dom_முனை *n, கட்டமைப்பு box_construct_props *props)
{
	memset(props, 0, அளவு(*props));

	props->முனை_is_வேர் = box_is_வேர்(n);

	/* Extract properties from containing DOM node */
	எனில் (props->முனை_is_வேர் == false) {
		dom_முனை *current_முனை = n;
		dom_முனை *parent_முனை = வெற்று;
		கட்டமைப்பு box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		போது (true) {
			err = dom_முனை_get_parent_முனை(current_முனை,
					&parent_முனை);
			எனில் (err != DOM_NO_ERR || parent_முனை == வெற்று)
				நிறுத்து;

			parent_box = box_for_முனை(parent_முனை);

			எனில் (parent_box != வெற்று) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_முனை_unref(parent_முனை);
				நிறுத்து;
			} இல்லையெனில் {
				எனில் (current_முனை != n)
					dom_முனை_unref(current_முனை);
				current_முனை = parent_முனை;
				parent_முனை = வெற்று;
			}
		}

		/* Find containing block (may be parent) */
		போது (true) {
			கட்டமைப்பு box *b;

			err = dom_முனை_get_parent_முனை(current_முனை,
					&parent_முனை);
			எனில் (err != DOM_NO_ERR || parent_முனை == வெற்று) {
				எனில் (current_முனை != n)
					dom_முனை_unref(current_முனை);
				நிறுத்து;
			}

			எனில் (current_முனை != n)
				dom_முனை_unref(current_முனை);

			b = box_for_முனை(parent_முனை);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			எனில் (b != வெற்று && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_முனை_unref(parent_முனை);
				நிறுத்து;
			} இல்லையெனில் {
				current_முனை = parent_முனை;
				parent_முனை = வெற்று;
			}
		}
	}

	/* Compute current inline container, if any */
	எனில் (props->containing_block != வெற்று &&
			props->containing_block->last != வெற்று &&
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
நிலையான css_select_results *
box_get_style(html_content *c,
	      மாறிலி css_computed_style *parent_style,
	      மாறிலி css_computed_style *வேர்_style,
	      dom_முனை *n)
{
	dom_string *s = வெற்று;
	css_stylesheet *inline_style = வெற்று;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	எனில் (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		எனில் (err != DOM_NO_ERR) {
			திரும்பு வெற்று;
		}
	}

	எனில் (s != வெற்று) {
		inline_style = nscss_create_inline_style(
				(மாறிலி uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		எனில் (inline_style == வெற்று)
			திரும்பு வெற்று;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.வேர்_style = வேர்_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	எனில் (inline_style != வெற்று)
		css_stylesheet_destroy(inline_style);

	திரும்பு styles;
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
நிலையான வெறுமை
box_construct_generate(dom_முனை *n,
		       html_content *content,
		       கட்டமைப்பு box *box,
		       மாறிலி css_computed_style *style)
{
	கட்டமைப்பு box *gen = வெற்று;
	எண்ணிக்கை css_display_e computed_display;
	மாறிலி css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	எனில் (box->type != BOX_BLOCK)
		திரும்பு;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	எனில் (style == வெற்று ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		திரும்பு;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_வேர்(n));
	எனில் (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(வெற்று, (css_computed_style *) style,
				false, வெற்று, வெற்று, வெற்று, வெற்று, content->bctx);
		எனில் (gen == வெற்று) {
			திரும்பு;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_வேர்(n))];

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
நிலையான bool
box_construct_marker(கட்டமைப்பு box *box,
		     மாறிலி எழுத்து *title,
		     கட்டமைப்பு box_construct_ctx *ctx,
		     கட்டமைப்பு box *parent)
{
	lwc_string *image_uri;
	கட்டமைப்பு box *marker;
	எண்ணிக்கை css_list_style_type_e list_style_type;

	marker = box_create(வெற்று, box->style, false, வெற்று, வெற்று, title,
			வெற்று, ctx->bctx);
	எனில் (marker == false)
		திரும்பு false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	தேர்வு (list_style_type) {
	நிலை CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (எழுத்து *) "\342\200\242";
		marker->length = 3;
		நிறுத்து;

	நிலை CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (எழுத்து *) "\342\227\213";
		marker->length = 3;
		நிறுத்து;

	நிலை CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (எழுத்து *) "\342\226\252";
		marker->length = 3;
		நிறுத்து;

	இயல்பு:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	நிலை CSS_LIST_STYLE_TYPE_NONE:
		marker->text = வெற்று;
		marker->length = 0;
		நிறுத்து;
	}

	எனில் (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != வெற்று) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		எனில் (error != NSERROR_OK)
			திரும்பு false;

		எனில் (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			திரும்பு false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	திரும்பு true;
}

நிலையான உள்வரி bool box__style_is_float(மாறிலி கட்டமைப்பு box *box)
{
	திரும்பு css_computed_float(box->style) == CSS_FLOAT_இடது ||
	       css_computed_float(box->style) == CSS_FLOAT_வலது;
}

நிலையான உள்வரி bool box__is_flex(மாறிலி கட்டமைப்பு box *box)
{
	திரும்பு box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

நிலையான உள்வரி bool box__containing_block_is_flex(
		மாறிலி கட்டமைப்பு box_construct_props *props)
{
	திரும்பு props->containing_block != வெற்று &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
நிலையான bool
box_construct_element(கட்டமைப்பு box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = வெற்று;
	எண்ணிக்கை css_display_e css_display;
	கட்டமைப்பு box *box = வெற்று, *old_box;
	css_select_results *styles = வெற்று;
	lwc_string *bgimage_uri;
	dom_exception err;
	கட்டமைப்பு box_construct_props props;
	மாறிலி css_computed_style *வேர்_style = வெற்று;

	ப்ரதிஜ்ஞா(ctx->n != வெற்று);

	box_extract_properties(ctx->n, &props);

	எனில் (props.containing_block != வெற்று) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	எனில் (props.முனை_is_வேர் == false) {
		வேர்_style = ctx->வேர்_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, வேர்_style,
			ctx->n);
	எனில் (styles == வெற்று)
		திரும்பு false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (title0 != வெற்று) {
		எழுத்து *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		எனில் (t == வெற்று)
			திரும்பு false;

		props.title = talloc_strdup(ctx->bctx, t);

		விடுவி(t);

		எனில் (props.title == வெற்று)
			திரும்பு false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (s != வெற்று) {
		err = dom_string_intern(s, &id);
		எனில் (err != DOM_NO_ERR)
			id = வெற்று;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	எனில் (box == வெற்று)
		திரும்பு false;

	/* If this is the root box, add it to the context */
	எனில் (props.முனை_is_வேர்)
		ctx->வேர்_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (s != வெற்று) {
		மாறிலி எழுத்து *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		எனில் ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, வெற்று, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (s != வெற்று) {
		மாறிலி எழுத்து *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		எனில் ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, வெற்று, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	எனில் ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} இல்லையெனில் எனில் (props.முனை_is_வேர்) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} இல்லையெனில் {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.முனை_is_வேர்)];

		எனில் (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			தேர்வு (box->type) {
			நிலை BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				நிறுத்து;
			நிலை BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				நிறுத்து;
			இயல்பு:
				நிறுத்து;
			}
		}
	}

	எனில் (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		திரும்பு false;
	}

	/* Handle the :before pseudo element */
	எனில் (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	எனில் (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.முனை_is_வேர்) == CSS_DISPLAY_NONE &&
			props.முனை_is_வேர் == false)) {
		css_select_results_destroy(styles);
		box->styles = வெற்று;
		box->style = வெற்று;

		/* Invalidate associated gadget, if any */
		எனில் (box->gadget != வெற்று) {
			box->gadget->box = வெற்று;
			box->gadget = வெற்று;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		திரும்பு true;
	}

	/* Attach DOM node to box */
	err = dom_முனை_set_user_data(ctx->n,
			corestring_dom_ns_சாவி_box_முனை_data, box, வெற்று,
			(வெறுமை *) &old_box);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	/* Attach box to DOM node */
	box->முனை = dom_முனை_ref(ctx->n);

	எனில் (props.inline_container == வெற்று &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.முனை_is_வேர் == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ப்ரதிஜ்ஞா(props.containing_block != வெற்று &&
				"Box must have containing block.");

		props.inline_container = box_create(வெற்று, வெற்று, false, வெற்று,
				வெற்று, வெற்று, வெற்று, ctx->bctx);
		எனில் (props.inline_container == வெற்று)
			திரும்பு false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	எனில் (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != வெற்று &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		எனில் (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			எனில் (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				திரும்பு false;
			}
			nsurl_unref(url);
		}
	}

	எனில் (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	எனில் (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ப்ரதிஜ்ஞா(props.inline_container != வெற்று);

		box_add_child(props.inline_container, box);
	} இல்லையெனில் {
		எனில் (ns_computed_display(box->style, props.முனை_is_வேர்) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			எனில் (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				திரும்பு false;
		}

		எனில் (props.முனை_is_வேர் == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_இடது ||
				css_computed_float(box->style) ==
				CSS_FLOAT_வலது)) {
			/* Float: insert a float between the parent and box. */
			கட்டமைப்பு box *flt = box_create(வெற்று, வெற்று, false,
					props.href, props.target, props.title,
					வெற்று, ctx->bctx);
			எனில் (flt == வெற்று)
				திரும்பு false;

			எனில் (css_computed_float(box->style) == CSS_FLOAT_இடது)
				flt->type = BOX_FLOAT_இடது;
			இல்லையெனில்
				flt->type = BOX_FLOAT_வலது;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} இல்லையெனில் {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			எனில் (props.containing_block != வெற்று)
				box_add_child(props.containing_block, box);
		}
	}

	திரும்பு true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
நிலையான வெறுமை box_construct_element_after(dom_முனை *n, html_content *content)
{
	கட்டமைப்பு box_construct_props props;
	கட்டமைப்பு box *box = box_for_முனை(n);

	ப்ரதிஜ்ஞா(box != வெற்று);

	box_extract_properties(n, &props);

	எனில் (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		கட்டமைப்பு box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_முனை_has_child_nodes(n, &has_children);
		எனில் (err != DOM_NO_ERR)
			திரும்பு;

		எனில் (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			திரும்பு;
		}

		எனில் (props.inline_container == வெற்று) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(வெற்று, வெற்று, false,
					வெற்று, வெற்று, வெற்று, வெற்று, content->bctx);
			எனில் (props.inline_container == வெற்று)
				திரும்பு;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(வெற்று, box->style, false,
				box->href, box->target, box->title,
				box->id == வெற்று ? வெற்று :
				lwc_string_ref(box->id), content->bctx);
		எனில் (inline_end != வெற்று) {
			inline_end->type = BOX_INLINE_END;

			ப்ரதிஜ்ஞா(props.inline_container != வெற்று);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} இல்லையெனில் எனில் (!(box->flags & IS_REPLACED)) {
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
நிலையான dom_முனை *
next_முனை(dom_முனை *n, html_content *content, bool convert_children)
{
	dom_முனை *next = வெற்று;
	bool has_children;
	dom_exception err;

	err = dom_முனை_has_child_nodes(n, &has_children);
	எனில் (err != DOM_NO_ERR) {
		dom_முனை_unref(n);
		திரும்பு வெற்று;
	}

	எனில் (convert_children && has_children) {
		err = dom_முனை_get_first_child(n, &next);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு வெற்று;
		}
		dom_முனை_unref(n);
	} இல்லையெனில் {
		err = dom_முனை_get_next_sibling(n, &next);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு வெற்று;
		}

		எனில் (next != வெற்று) {
			எனில் (box_for_முனை(n) != வெற்று)
				box_construct_element_after(n, content);
			dom_முனை_unref(n);
		} இல்லையெனில் {
			எனில் (box_for_முனை(n) != வெற்று)
				box_construct_element_after(n, content);

			போது (box_is_வேர்(n) == false) {
				dom_முனை *parent = வெற்று;
				dom_முனை *parent_next = வெற்று;

				err = dom_முனை_get_parent_முனை(n, &parent);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(n);
					திரும்பு வெற்று;
				}

				ப்ரதிஜ்ஞா(parent != வெற்று);

				err = dom_முனை_get_next_sibling(parent,
						&parent_next);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(parent);
					dom_முனை_unref(n);
					திரும்பு வெற்று;
				}

				எனில் (parent_next != வெற்று) {
					dom_முனை_unref(parent_next);
					dom_முனை_unref(parent);
					நிறுத்து;
				}

				dom_முனை_unref(n);
				n = parent;
				parent = வெற்று;

				எனில் (box_for_முனை(n) != வெற்று) {
					box_construct_element_after(
							n, content);
				}
			}

			எனில் (box_is_வேர்(n) == false) {
				dom_முனை *parent = வெற்று;

				err = dom_முனை_get_parent_முனை(n, &parent);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(n);
					திரும்பு வெற்று;
				}

				ப்ரதிஜ்ஞா(parent != வெற்று);

				err = dom_முனை_get_next_sibling(parent, &next);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(parent);
					dom_முனை_unref(n);
					திரும்பு வெற்று;
				}

				எனில் (box_for_முனை(parent) != வெற்று) {
					box_construct_element_after(parent,
							content);
				}

				dom_முனை_unref(parent);
			}

			dom_முனை_unref(n);
		}
	}

	திரும்பு next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
நிலையான வெறுமை
box_text_transform(எழுத்து *s, unsigned முழு len, எண்ணிக்கை css_text_transform_e tt)
{
	unsigned முழு i;
	எனில் (len == 0)
		திரும்பு;
	தேர்வு (tt) {
		நிலை CSS_TEXT_TRANSFORM_UPPERCASE:
			க்கு (i = 0; i < len; ++i)
				எனில் ((unsigned எழுத்து) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			நிறுத்து;
		நிலை CSS_TEXT_TRANSFORM_LOWERCASE:
			க்கு (i = 0; i < len; ++i)
				எனில் ((unsigned எழுத்து) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			நிறுத்து;
		நிலை CSS_TEXT_TRANSFORM_CAPITALIZE:
			எனில் ((unsigned எழுத்து) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			க்கு (i = 1; i < len; ++i)
				எனில் ((unsigned எழுத்து) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			நிறுத்து;
		இயல்பு:
			நிறுத்து;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
நிலையான bool box_construct_text(கட்டமைப்பு box_construct_ctx *ctx)
{
	கட்டமைப்பு box_construct_props props;
	கட்டமைப்பு box *box = வெற்று;
	dom_string *content;
	dom_exception err;

	ப்ரதிஜ்ஞா(ctx->n != வெற்று);

	box_extract_properties(ctx->n, &props);

	ப்ரதிஜ்ஞா(props.containing_block != வெற்று);

	err = dom_characterdata_get_data(ctx->n, &content);
	எனில் (err != DOM_NO_ERR || content == வெற்று)
		திரும்பு false;

	எனில் (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		எழுத்து *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		எனில் (text == வெற்று)
			திரும்பு false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		எனில் (text[0] == ' ' && text[1] == 0) {
			எனில் (props.inline_container != வெற்று) {
				ப்ரதிஜ்ஞா(props.inline_container->last != வெற்று);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			விடுவி(text);

			திரும்பு true;
		}

		எனில் (props.inline_container == வெற்று) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(வெற்று, வெற்று, false,
					வெற்று, வெற்று, வெற்று, வெற்று, ctx->bctx);
			எனில் (props.inline_container == வெற்று) {
				விடுவி(text);
				திரும்பு false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(வெற்று,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				வெற்று, ctx->bctx);
		எனில் (box == வெற்று) {
			விடுவி(text);
			திரும்பு false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		விடுவி(text);
		எனில் (box->text == வெற்று)
			திரும்பு false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		எனில் (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		எனில் (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		எனில் (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			எனில் (box->prev != வெற்று)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} இல்லையெனில் {
		/* white-space: pre */
		எழுத்து *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		எழுத்து *current;
		எண்ணிக்கை css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ப்ரதிஜ்ஞா(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_வரி ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		எனில் (text == வெற்று)
			திரும்பு false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		க்கு (i = 0; i < text_len; i++)
			எனில் (text[i] == '\t')
				text[i] = ' ';

		எனில் (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		எனில் (props.containing_block->flags & PRE_STRIP) {
			தேர்வு (*current) {
			நிலை '\n':
				current++;
				நிறுத்து;
			நிலை '\r':
				current++;
				எனில் (*current == '\n')
					current++;
				நிறுத்து;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		செய் {
			size_t len = strcspn(current, "\r\n");

			எழுத்து old = current[len];

			current[len] = 0;

			எனில் (props.inline_container == வெற்று) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(வெற்று, வெற்று,
						false, வெற்று, வெற்று, வெற்று, வெற்று,
						ctx->bctx);
				எனில் (props.inline_container == வெற்று) {
					விடுவி(text);
					திரும்பு false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(வெற்று,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				வெற்று, ctx->bctx);
			எனில் (box == வெற்று) {
				விடுவி(text);
				திரும்பு false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			எனில் (box->text == வெற்று) {
				விடுவி(text);
				திரும்பு false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			எனில் (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(வெற்று, வெற்று,
						false, வெற்று, வெற்று, வெற்று, வெற்று,
						ctx->bctx);
				எனில் (props.inline_container == வெற்று) {
					விடுவி(text);
					திரும்பு false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				எனில் (current[0] == '\r' && current[1] == '\n')
					current += 2;
				இல்லையெனில்
					current++;
			}
		} போது (*current);

		விடுவி(text);
	}

	திரும்பு true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
நிலையான வெறுமை convert_xml_to_box(கட்டமைப்பு box_construct_ctx *ctx)
{
	dom_முனை *next;
	bool convert_children;
	uint32_t இலக்கம்_processed = 0;
	மாறிலி uint32_t max_processed_before_yield = 10;

	செய் {
		convert_children = true;

		ப்ரதிஜ்ஞா(ctx->n != வெற்று);

		எனில் (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_முனை_unref(ctx->n);
			விடுவி(ctx);
			திரும்பு;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_முனை(ctx->n, ctx->content, convert_children);
		போது (next != வெற்று) {
			dom_முனை_type type;
			dom_exception err;

			err = dom_முனை_get_முனை_type(next, &type);
			எனில் (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_முனை_unref(next);
				விடுவி(ctx);
				திரும்பு;
			}

			எனில் (type == DOM_ELEMENT_முனை)
				நிறுத்து;

			எனில் (type == DOM_TEXT_முனை) {
				ctx->n = next;
				எனில் (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_முனை_unref(ctx->n);
					விடுவி(ctx);
					திரும்பு;
				}
			}

			next = next_முனை(next, ctx->content, true);
		}

		ctx->n = next;

		எனில் (next == வெற்று) {
			/* Conversion complete */
			கட்டமைப்பு box வேர்;

			memset(&வேர், 0, அளவு(வேர்));

			வேர்.type = BOX_BLOCK;
			வேர்.children = வேர்.last = ctx->வேர்_box;
			வேர்.children->parent = &வேர்;

			/** \todo Remove box_normalise_block */
			எனில் (box_normalise_block(&வேர், ctx->வேர்_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} இல்லையெனில் {
				ctx->content->layout = வேர்.children;
				ctx->content->layout->parent = வெற்று;

				ctx->cb(ctx->content, true);
			}

			ப்ரதிஜ்ஞா(ctx->n == வெற்று);

			விடுவி(ctx);
			திரும்பு;
		}
	} போது (++இலக்கம்_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (வெறுமை *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_முனை *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   வெறுமை **box_conversion_context)
{
	கட்டமைப்பு box_construct_ctx *ctx;

	ப்ரதிஜ்ஞா(box_conversion_context != வெற்று);

	எனில் (c->bctx == வெற்று) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, முழு);
		எனில் (c->bctx == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
	}

	ctx = malloc(அளவு(*ctx));
	எனில் (ctx == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_முனை_ref(n);
	ctx->வேர்_box = வெற்று;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	திரும்பு guit->misc->schedule(0, (வெறுமை *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(வெறுமை *box_conversion_context)
{
	கட்டமைப்பு box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (வெறுமை *)convert_xml_to_box, ctx);
	எனில் (err != NSERROR_OK) {
		திரும்பு err;
	}

	dom_முனை_unref(ctx->n);
	விடுவி(ctx);

	திரும்பு NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
கட்டமைப்பு box *box_for_முனை(dom_முனை *n)
{
	கட்டமைப்பு box *box = வெற்று;
	dom_exception err;

	err = dom_முனை_get_user_data(n, corestring_dom_ns_சாவி_box_முனை_data,
			(வெறுமை *) &box);
	எனில் (err != DOM_NO_ERR)
		திரும்பு வெற்று;

	திரும்பு box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(மாறிலி html_content *content,
		 மாறிலி dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	எழுத்து *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned முழு i, j, end;
	nserror error;
	மாறிலி எழுத்து *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	எனில் (!s)
		திரும்பு false;

	/* copy to s, removing white space and control characters */
	க்கு (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	க்கு (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	க்கு (j = 0; i != end; i++) {
		எனில் ((unsigned எழுத்து) rel[i] < 0x20) {
			; /* skip control characters */
		} இல்லையெனில் எனில் (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} இல்லையெனில் {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	எனில் (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		எனில் (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			எனில் (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			எனில் (quot0)
				quot1 = strchr(quot0 + 1, '"');
			எனில் (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} இல்லையெனில் எனில் (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	விடுவி(s);
	எனில் (error != NSERROR_OK) {
		*result = வெற்று;
		திரும்பு false;
	}

	திரும்பு true;
}
