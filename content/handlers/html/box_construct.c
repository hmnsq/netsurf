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

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/string.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "css/select.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/object.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_manipulate.h"
#અન્તર્ભાવ "html/box_construct.h"
#અન્તર્ભાવ "html/box_special.h"
#અન્તર્ભાવ "html/box_normalise.h"
#અન્તર્ભાવ "html/form_internal.h"

/**
 * Context for box tree construction
 */
સંરચના box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_ગાંઠ *n;			/**< Current node to process */

	સંરચના box *મૂળ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	પૂર્ણાંક *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
સંરચના box_construct_props {
	/** Style from which to inherit, or NULL if none */
	અચલ css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	સંરચના nsurl *href;
	/** Current frame target, or NULL if none */
	અચલ અક્ષર *target;
	/** Current title attribute, or NULL if none */
	અચલ અક્ષર *title;
	/** Identity of the current block-level container */
	સંરચના box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	સંરચના box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ગાંઠ_is_મૂળ;
};

સ્થિર અચલ content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
સ્થિર અચલ box_type box_map[] = {
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
સ્થિર અંતઃરેખ bool box_is_મૂળ(dom_ગાંઠ *n)
{
	dom_ગાંઠ *parent;
	dom_ગાંઠ_type type;
	dom_exception err;

	err = dom_ગાંઠ_get_parent_ગાંઠ(n, &parent);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (parent != ખાલી) {
		err = dom_ગાંઠ_get_ગાંઠ_type(parent, &type);

		dom_ગાંઠ_unref(parent);

		જો (err != DOM_NO_ERR)
			પાછા false;

		જો (type != DOM_DOCUMENT_ગાંઠ)
			પાછા false;
	}

	પાછા true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
સ્થિર શૂન્ય
box_extract_properties(dom_ગાંઠ *n, સંરચના box_construct_props *props)
{
	memset(props, 0, કદ(*props));

	props->ગાંઠ_is_મૂળ = box_is_મૂળ(n);

	/* Extract properties from containing DOM node */
	જો (props->ગાંઠ_is_મૂળ == false) {
		dom_ગાંઠ *current_ગાંઠ = n;
		dom_ગાંઠ *parent_ગાંઠ = ખાલી;
		સંરચના box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		જ્યારે (true) {
			err = dom_ગાંઠ_get_parent_ગાંઠ(current_ગાંઠ,
					&parent_ગાંઠ);
			જો (err != DOM_NO_ERR || parent_ગાંઠ == ખાલી)
				તોડો;

			parent_box = box_for_ગાંઠ(parent_ગાંઠ);

			જો (parent_box != ખાલી) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_ગાંઠ_unref(parent_ગાંઠ);
				તોડો;
			} નહીંતર {
				જો (current_ગાંઠ != n)
					dom_ગાંઠ_unref(current_ગાંઠ);
				current_ગાંઠ = parent_ગાંઠ;
				parent_ગાંઠ = ખાલી;
			}
		}

		/* Find containing block (may be parent) */
		જ્યારે (true) {
			સંરચના box *b;

			err = dom_ગાંઠ_get_parent_ગાંઠ(current_ગાંઠ,
					&parent_ગાંઠ);
			જો (err != DOM_NO_ERR || parent_ગાંઠ == ખાલી) {
				જો (current_ગાંઠ != n)
					dom_ગાંઠ_unref(current_ગાંઠ);
				તોડો;
			}

			જો (current_ગાંઠ != n)
				dom_ગાંઠ_unref(current_ગાંઠ);

			b = box_for_ગાંઠ(parent_ગાંઠ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			જો (b != ખાલી && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_ગાંઠ_unref(parent_ગાંઠ);
				તોડો;
			} નહીંતર {
				current_ગાંઠ = parent_ગાંઠ;
				parent_ગાંઠ = ખાલી;
			}
		}
	}

	/* Compute current inline container, if any */
	જો (props->containing_block != ખાલી &&
			props->containing_block->last != ખાલી &&
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
સ્થિર css_select_results *
box_get_style(html_content *c,
	      અચલ css_computed_style *parent_style,
	      અચલ css_computed_style *મૂળ_style,
	      dom_ગાંઠ *n)
{
	dom_string *s = ખાલી;
	css_stylesheet *inline_style = ખાલી;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	જો (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		જો (err != DOM_NO_ERR) {
			પાછા ખાલી;
		}
	}

	જો (s != ખાલી) {
		inline_style = nscss_create_inline_style(
				(અચલ uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		જો (inline_style == ખાલી)
			પાછા ખાલી;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.મૂળ_style = મૂળ_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	જો (inline_style != ખાલી)
		css_stylesheet_destroy(inline_style);

	પાછા styles;
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
સ્થિર શૂન્ય
box_construct_generate(dom_ગાંઠ *n,
		       html_content *content,
		       સંરચના box *box,
		       અચલ css_computed_style *style)
{
	સંરચના box *gen = ખાલી;
	ગણના css_display_e computed_display;
	અચલ css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	જો (box->type != BOX_BLOCK)
		પાછા;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	જો (style == ખાલી ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		પાછા;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_મૂળ(n));
	જો (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ખાલી, (css_computed_style *) style,
				false, ખાલી, ખાલી, ખાલી, ખાલી, content->bctx);
		જો (gen == ખાલી) {
			પાછા;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_મૂળ(n))];

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
સ્થિર bool
box_construct_marker(સંરચના box *box,
		     અચલ અક્ષર *title,
		     સંરચના box_construct_ctx *ctx,
		     સંરચના box *parent)
{
	lwc_string *image_uri;
	સંરચના box *marker;
	ગણના css_list_style_type_e list_style_type;

	marker = box_create(ખાલી, box->style, false, ખાલી, ખાલી, title,
			ખાલી, ctx->bctx);
	જો (marker == false)
		પાછા false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	પસંદગી (list_style_type) {
	સ્થિતિ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (અક્ષર *) "\342\200\242";
		marker->length = 3;
		તોડો;

	સ્થિતિ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (અક્ષર *) "\342\227\213";
		marker->length = 3;
		તોડો;

	સ્થિતિ CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (અક્ષર *) "\342\226\252";
		marker->length = 3;
		તોડો;

	મૂળભૂત:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	સ્થિતિ CSS_LIST_STYLE_TYPE_NONE:
		marker->text = ખાલી;
		marker->length = 0;
		તોડો;
	}

	જો (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ખાલી) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		જો (error != NSERROR_OK)
			પાછા false;

		જો (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			પાછા false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	પાછા true;
}

સ્થિર અંતઃરેખ bool box__style_is_float(અચલ સંરચના box *box)
{
	પાછા css_computed_float(box->style) == CSS_FLOAT_ડાબું ||
	       css_computed_float(box->style) == CSS_FLOAT_જમણું;
}

સ્થિર અંતઃરેખ bool box__is_flex(અચલ સંરચના box *box)
{
	પાછા box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

સ્થિર અંતઃરેખ bool box__containing_block_is_flex(
		અચલ સંરચના box_construct_props *props)
{
	પાછા props->containing_block != ખાલી &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
સ્થિર bool
box_construct_element(સંરચના box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ખાલી;
	ગણના css_display_e css_display;
	સંરચના box *box = ખાલી, *old_box;
	css_select_results *styles = ખાલી;
	lwc_string *bgimage_uri;
	dom_exception err;
	સંરચના box_construct_props props;
	અચલ css_computed_style *મૂળ_style = ખાલી;

	પ્રતિજ્ઞા(ctx->n != ખાલી);

	box_extract_properties(ctx->n, &props);

	જો (props.containing_block != ખાલી) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	જો (props.ગાંઠ_is_મૂળ == false) {
		મૂળ_style = ctx->મૂળ_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, મૂળ_style,
			ctx->n);
	જો (styles == ખાલી)
		પાછા false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (title0 != ખાલી) {
		અક્ષર *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		જો (t == ખાલી)
			પાછા false;

		props.title = talloc_strdup(ctx->bctx, t);

		મુક્ત(t);

		જો (props.title == ખાલી)
			પાછા false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (s != ખાલી) {
		err = dom_string_intern(s, &id);
		જો (err != DOM_NO_ERR)
			id = ખાલી;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	જો (box == ખાલી)
		પાછા false;

	/* If this is the root box, add it to the context */
	જો (props.ગાંઠ_is_મૂળ)
		ctx->મૂળ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (s != ખાલી) {
		અચલ અક્ષર *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		જો ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ખાલી, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (s != ખાલી) {
		અચલ અક્ષર *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		જો ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, ખાલી, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	જો ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} નહીંતર જો (props.ગાંઠ_is_મૂળ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} નહીંતર {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ગાંઠ_is_મૂળ)];

		જો (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			પસંદગી (box->type) {
			સ્થિતિ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				તોડો;
			સ્થિતિ BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				તોડો;
			મૂળભૂત:
				તોડો;
			}
		}
	}

	જો (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		પાછા false;
	}

	/* Handle the :before pseudo element */
	જો (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	જો (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ગાંઠ_is_મૂળ) == CSS_DISPLAY_NONE &&
			props.ગાંઠ_is_મૂળ == false)) {
		css_select_results_destroy(styles);
		box->styles = ખાલી;
		box->style = ખાલી;

		/* Invalidate associated gadget, if any */
		જો (box->gadget != ખાલી) {
			box->gadget->box = ખાલી;
			box->gadget = ખાલી;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		પાછા true;
	}

	/* Attach DOM node to box */
	err = dom_ગાંઠ_set_user_data(ctx->n,
			corestring_dom_ns_ચાવી_box_ગાંઠ_data, box, ખાલી,
			(શૂન્ય *) &old_box);
	જો (err != DOM_NO_ERR)
		પાછા false;

	/* Attach box to DOM node */
	box->ગાંઠ = dom_ગાંઠ_ref(ctx->n);

	જો (props.inline_container == ખાલી &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.ગાંઠ_is_મૂળ == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		પ્રતિજ્ઞા(props.containing_block != ખાલી &&
				"Box must have containing block.");

		props.inline_container = box_create(ખાલી, ખાલી, false, ખાલી,
				ખાલી, ખાલી, ખાલી, ctx->bctx);
		જો (props.inline_container == ખાલી)
			પાછા false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	જો (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ખાલી &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		જો (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			જો (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				પાછા false;
			}
			nsurl_unref(url);
		}
	}

	જો (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	જો (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		પ્રતિજ્ઞા(props.inline_container != ખાલી);

		box_add_child(props.inline_container, box);
	} નહીંતર {
		જો (ns_computed_display(box->style, props.ગાંઠ_is_મૂળ) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			જો (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				પાછા false;
		}

		જો (props.ગાંઠ_is_મૂળ == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ડાબું ||
				css_computed_float(box->style) ==
				CSS_FLOAT_જમણું)) {
			/* Float: insert a float between the parent and box. */
			સંરચના box *flt = box_create(ખાલી, ખાલી, false,
					props.href, props.target, props.title,
					ખાલી, ctx->bctx);
			જો (flt == ખાલી)
				પાછા false;

			જો (css_computed_float(box->style) == CSS_FLOAT_ડાબું)
				flt->type = BOX_FLOAT_ડાબું;
			નહીંતર
				flt->type = BOX_FLOAT_જમણું;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} નહીંતર {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			જો (props.containing_block != ખાલી)
				box_add_child(props.containing_block, box);
		}
	}

	પાછા true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
સ્થિર શૂન્ય box_construct_element_after(dom_ગાંઠ *n, html_content *content)
{
	સંરચના box_construct_props props;
	સંરચના box *box = box_for_ગાંઠ(n);

	પ્રતિજ્ઞા(box != ખાલી);

	box_extract_properties(n, &props);

	જો (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		સંરચના box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_ગાંઠ_has_child_nodes(n, &has_children);
		જો (err != DOM_NO_ERR)
			પાછા;

		જો (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			પાછા;
		}

		જો (props.inline_container == ખાલી) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ખાલી, ખાલી, false,
					ખાલી, ખાલી, ખાલી, ખાલી, content->bctx);
			જો (props.inline_container == ખાલી)
				પાછા;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(ખાલી, box->style, false,
				box->href, box->target, box->title,
				box->id == ખાલી ? ખાલી :
				lwc_string_ref(box->id), content->bctx);
		જો (inline_end != ખાલી) {
			inline_end->type = BOX_INLINE_END;

			પ્રતિજ્ઞા(props.inline_container != ખાલી);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} નહીંતર જો (!(box->flags & IS_REPLACED)) {
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
સ્થિર dom_ગાંઠ *
next_ગાંઠ(dom_ગાંઠ *n, html_content *content, bool convert_children)
{
	dom_ગાંઠ *next = ખાલી;
	bool has_children;
	dom_exception err;

	err = dom_ગાંઠ_has_child_nodes(n, &has_children);
	જો (err != DOM_NO_ERR) {
		dom_ગાંઠ_unref(n);
		પાછા ખાલી;
	}

	જો (convert_children && has_children) {
		err = dom_ગાંઠ_get_first_child(n, &next);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા ખાલી;
		}
		dom_ગાંઠ_unref(n);
	} નહીંતર {
		err = dom_ગાંઠ_get_next_sibling(n, &next);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા ખાલી;
		}

		જો (next != ખાલી) {
			જો (box_for_ગાંઠ(n) != ખાલી)
				box_construct_element_after(n, content);
			dom_ગાંઠ_unref(n);
		} નહીંતર {
			જો (box_for_ગાંઠ(n) != ખાલી)
				box_construct_element_after(n, content);

			જ્યારે (box_is_મૂળ(n) == false) {
				dom_ગાંઠ *parent = ખાલી;
				dom_ગાંઠ *parent_next = ખાલી;

				err = dom_ગાંઠ_get_parent_ગાંઠ(n, &parent);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(n);
					પાછા ખાલી;
				}

				પ્રતિજ્ઞા(parent != ખાલી);

				err = dom_ગાંઠ_get_next_sibling(parent,
						&parent_next);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(parent);
					dom_ગાંઠ_unref(n);
					પાછા ખાલી;
				}

				જો (parent_next != ખાલી) {
					dom_ગાંઠ_unref(parent_next);
					dom_ગાંઠ_unref(parent);
					તોડો;
				}

				dom_ગાંઠ_unref(n);
				n = parent;
				parent = ખાલી;

				જો (box_for_ગાંઠ(n) != ખાલી) {
					box_construct_element_after(
							n, content);
				}
			}

			જો (box_is_મૂળ(n) == false) {
				dom_ગાંઠ *parent = ખાલી;

				err = dom_ગાંઠ_get_parent_ગાંઠ(n, &parent);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(n);
					પાછા ખાલી;
				}

				પ્રતિજ્ઞા(parent != ખાલી);

				err = dom_ગાંઠ_get_next_sibling(parent, &next);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(parent);
					dom_ગાંઠ_unref(n);
					પાછા ખાલી;
				}

				જો (box_for_ગાંઠ(parent) != ખાલી) {
					box_construct_element_after(parent,
							content);
				}

				dom_ગાંઠ_unref(parent);
			}

			dom_ગાંઠ_unref(n);
		}
	}

	પાછા next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
સ્થિર શૂન્ય
box_text_transform(અક્ષર *s, unsigned પૂર્ણાંક len, ગણના css_text_transform_e tt)
{
	unsigned પૂર્ણાંક i;
	જો (len == 0)
		પાછા;
	પસંદગી (tt) {
		સ્થિતિ CSS_TEXT_TRANSFORM_UPPERCASE:
			માટે (i = 0; i < len; ++i)
				જો ((unsigned અક્ષર) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			તોડો;
		સ્થિતિ CSS_TEXT_TRANSFORM_LOWERCASE:
			માટે (i = 0; i < len; ++i)
				જો ((unsigned અક્ષર) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			તોડો;
		સ્થિતિ CSS_TEXT_TRANSFORM_CAPITALIZE:
			જો ((unsigned અક્ષર) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			માટે (i = 1; i < len; ++i)
				જો ((unsigned અક્ષર) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			તોડો;
		મૂળભૂત:
			તોડો;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
સ્થિર bool box_construct_text(સંરચના box_construct_ctx *ctx)
{
	સંરચના box_construct_props props;
	સંરચના box *box = ખાલી;
	dom_string *content;
	dom_exception err;

	પ્રતિજ્ઞા(ctx->n != ખાલી);

	box_extract_properties(ctx->n, &props);

	પ્રતિજ્ઞા(props.containing_block != ખાલી);

	err = dom_characterdata_get_data(ctx->n, &content);
	જો (err != DOM_NO_ERR || content == ખાલી)
		પાછા false;

	જો (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		અક્ષર *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		જો (text == ખાલી)
			પાછા false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		જો (text[0] == ' ' && text[1] == 0) {
			જો (props.inline_container != ખાલી) {
				પ્રતિજ્ઞા(props.inline_container->last != ખાલી);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			મુક્ત(text);

			પાછા true;
		}

		જો (props.inline_container == ખાલી) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ખાલી, ખાલી, false,
					ખાલી, ખાલી, ખાલી, ખાલી, ctx->bctx);
			જો (props.inline_container == ખાલી) {
				મુક્ત(text);
				પાછા false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ખાલી,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ખાલી, ctx->bctx);
		જો (box == ખાલી) {
			મુક્ત(text);
			પાછા false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		મુક્ત(text);
		જો (box->text == ખાલી)
			પાછા false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		જો (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		જો (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		જો (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			જો (box->prev != ખાલી)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} નહીંતર {
		/* white-space: pre */
		અક્ષર *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		અક્ષર *current;
		ગણના css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		પ્રતિજ્ઞા(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_લીટી ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		જો (text == ખાલી)
			પાછા false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		માટે (i = 0; i < text_len; i++)
			જો (text[i] == '\t')
				text[i] = ' ';

		જો (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		જો (props.containing_block->flags & PRE_STRIP) {
			પસંદગી (*current) {
			સ્થિતિ '\n':
				current++;
				તોડો;
			સ્થિતિ '\r':
				current++;
				જો (*current == '\n')
					current++;
				તોડો;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		કરો {
			size_t len = strcspn(current, "\r\n");

			અક્ષર old = current[len];

			current[len] = 0;

			જો (props.inline_container == ખાલી) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ખાલી, ખાલી,
						false, ખાલી, ખાલી, ખાલી, ખાલી,
						ctx->bctx);
				જો (props.inline_container == ખાલી) {
					મુક્ત(text);
					પાછા false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ખાલી,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ખાલી, ctx->bctx);
			જો (box == ખાલી) {
				મુક્ત(text);
				પાછા false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			જો (box->text == ખાલી) {
				મુક્ત(text);
				પાછા false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			જો (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ખાલી, ખાલી,
						false, ખાલી, ખાલી, ખાલી, ખાલી,
						ctx->bctx);
				જો (props.inline_container == ખાલી) {
					મુક્ત(text);
					પાછા false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				જો (current[0] == '\r' && current[1] == '\n')
					current += 2;
				નહીંતર
					current++;
			}
		} જ્યારે (*current);

		મુક્ત(text);
	}

	પાછા true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
સ્થિર શૂન્ય convert_xml_to_box(સંરચના box_construct_ctx *ctx)
{
	dom_ગાંઠ *next;
	bool convert_children;
	uint32_t અંક_processed = 0;
	અચલ uint32_t max_processed_before_yield = 10;

	કરો {
		convert_children = true;

		પ્રતિજ્ઞા(ctx->n != ખાલી);

		જો (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ગાંઠ_unref(ctx->n);
			મુક્ત(ctx);
			પાછા;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_ગાંઠ(ctx->n, ctx->content, convert_children);
		જ્યારે (next != ખાલી) {
			dom_ગાંઠ_type type;
			dom_exception err;

			err = dom_ગાંઠ_get_ગાંઠ_type(next, &type);
			જો (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ગાંઠ_unref(next);
				મુક્ત(ctx);
				પાછા;
			}

			જો (type == DOM_ELEMENT_ગાંઠ)
				તોડો;

			જો (type == DOM_TEXT_ગાંઠ) {
				ctx->n = next;
				જો (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ગાંઠ_unref(ctx->n);
					મુક્ત(ctx);
					પાછા;
				}
			}

			next = next_ગાંઠ(next, ctx->content, true);
		}

		ctx->n = next;

		જો (next == ખાલી) {
			/* Conversion complete */
			સંરચના box મૂળ;

			memset(&મૂળ, 0, કદ(મૂળ));

			મૂળ.type = BOX_BLOCK;
			મૂળ.children = મૂળ.last = ctx->મૂળ_box;
			મૂળ.children->parent = &મૂળ;

			/** \todo Remove box_normalise_block */
			જો (box_normalise_block(&મૂળ, ctx->મૂળ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} નહીંતર {
				ctx->content->layout = મૂળ.children;
				ctx->content->layout->parent = ખાલી;

				ctx->cb(ctx->content, true);
			}

			પ્રતિજ્ઞા(ctx->n == ખાલી);

			મુક્ત(ctx);
			પાછા;
		}
	} જ્યારે (++અંક_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (શૂન્ય *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_ગાંઠ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   શૂન્ય **box_conversion_context)
{
	સંરચના box_construct_ctx *ctx;

	પ્રતિજ્ઞા(box_conversion_context != ખાલી);

	જો (c->bctx == ખાલી) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, પૂર્ણાંક);
		જો (c->bctx == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
	}

	ctx = malloc(કદ(*ctx));
	જો (ctx == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ગાંઠ_ref(n);
	ctx->મૂળ_box = ખાલી;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	પાછા guit->misc->schedule(0, (શૂન્ય *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(શૂન્ય *box_conversion_context)
{
	સંરચના box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (શૂન્ય *)convert_xml_to_box, ctx);
	જો (err != NSERROR_OK) {
		પાછા err;
	}

	dom_ગાંઠ_unref(ctx->n);
	મુક્ત(ctx);

	પાછા NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
સંરચના box *box_for_ગાંઠ(dom_ગાંઠ *n)
{
	સંરચના box *box = ખાલી;
	dom_exception err;

	err = dom_ગાંઠ_get_user_data(n, corestring_dom_ns_ચાવી_box_ગાંઠ_data,
			(શૂન્ય *) &box);
	જો (err != DOM_NO_ERR)
		પાછા ખાલી;

	પાછા box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(અચલ html_content *content,
		 અચલ dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	અક્ષર *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned પૂર્ણાંક i, j, end;
	nserror error;
	અચલ અક્ષર *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	જો (!s)
		પાછા false;

	/* copy to s, removing white space and control characters */
	માટે (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	માટે (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	માટે (j = 0; i != end; i++) {
		જો ((unsigned અક્ષર) rel[i] < 0x20) {
			; /* skip control characters */
		} નહીંતર જો (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} નહીંતર {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	જો (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		જો (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			જો (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			જો (quot0)
				quot1 = strchr(quot0 + 1, '"');
			જો (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} નહીંતર જો (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	મુક્ત(s);
	જો (error != NSERROR_OK) {
		*result = ખાલી;
		પાછા false;
	}

	પાછા true;
}
