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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "css/select.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/box_special.h"
#ಅನ್ತರ್ಭಾವ "html/box_normalise.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"

/**
 * Context for box tree construction
 */
ಸಂರಚನಾ box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_ಗಂಟು *n;			/**< Current node to process */

	ಸಂರಚನಾ box *ಬೇರು_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	ಪೂರ್ಣಾಙ್ಕ *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ಸಂರಚನಾ box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ಧ್ರುವ css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	ಸಂರಚನಾ nsurl *href;
	/** Current frame target, or NULL if none */
	ಧ್ರುವ ವರ್ಣ *target;
	/** Current title attribute, or NULL if none */
	ಧ್ರುವ ವರ್ಣ *title;
	/** Identity of the current block-level container */
	ಸಂರಚನಾ box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ಸಂರಚನಾ box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ಗಂಟು_is_ಬೇರು;
};

ಸ್ಥಿರ ಧ್ರುವ content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ಸ್ಥಿರ ಧ್ರುವ box_type box_map[] = {
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box_is_ಬೇರು(dom_ಗಂಟು *n)
{
	dom_ಗಂಟು *parent;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_parent_ಗಂಟು(n, &parent);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (parent != ರಿಕ್ತ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(parent, &type);

		dom_ಗಂಟು_unref(parent);

		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (type != DOM_DOCUMENT_ಗಂಟು)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ಸ್ಥಿರ ಶೂನ್ಯ
box_extract_properties(dom_ಗಂಟು *n, ಸಂರಚನಾ box_construct_props *props)
{
	memset(props, 0, ಪರಿಮಾಣ(*props));

	props->ಗಂಟು_is_ಬೇರು = box_is_ಬೇರು(n);

	/* Extract properties from containing DOM node */
	ಯದಿ (props->ಗಂಟು_is_ಬೇರು == false) {
		dom_ಗಂಟು *current_ಗಂಟು = n;
		dom_ಗಂಟು *parent_ಗಂಟು = ರಿಕ್ತ;
		ಸಂರಚನಾ box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ಯಾವತ್ (true) {
			err = dom_ಗಂಟು_get_parent_ಗಂಟು(current_ಗಂಟು,
					&parent_ಗಂಟು);
			ಯದಿ (err != DOM_NO_ERR || parent_ಗಂಟು == ರಿಕ್ತ)
				ಭಙ್ಗ;

			parent_box = box_for_ಗಂಟು(parent_ಗಂಟು);

			ಯದಿ (parent_box != ರಿಕ್ತ) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_ಗಂಟು_unref(parent_ಗಂಟು);
				ಭಙ್ಗ;
			} ಅನ್ಯಥಾ {
				ಯದಿ (current_ಗಂಟು != n)
					dom_ಗಂಟು_unref(current_ಗಂಟು);
				current_ಗಂಟು = parent_ಗಂಟು;
				parent_ಗಂಟು = ರಿಕ್ತ;
			}
		}

		/* Find containing block (may be parent) */
		ಯಾವತ್ (true) {
			ಸಂರಚನಾ box *b;

			err = dom_ಗಂಟು_get_parent_ಗಂಟು(current_ಗಂಟು,
					&parent_ಗಂಟು);
			ಯದಿ (err != DOM_NO_ERR || parent_ಗಂಟು == ರಿಕ್ತ) {
				ಯದಿ (current_ಗಂಟು != n)
					dom_ಗಂಟು_unref(current_ಗಂಟು);
				ಭಙ್ಗ;
			}

			ಯದಿ (current_ಗಂಟು != n)
				dom_ಗಂಟು_unref(current_ಗಂಟು);

			b = box_for_ಗಂಟು(parent_ಗಂಟು);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ಯದಿ (b != ರಿಕ್ತ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_ಗಂಟು_unref(parent_ಗಂಟು);
				ಭಙ್ಗ;
			} ಅನ್ಯಥಾ {
				current_ಗಂಟು = parent_ಗಂಟು;
				parent_ಗಂಟು = ರಿಕ್ತ;
			}
		}
	}

	/* Compute current inline container, if any */
	ಯದಿ (props->containing_block != ರಿಕ್ತ &&
			props->containing_block->last != ರಿಕ್ತ &&
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
ಸ್ಥಿರ css_select_results *
box_get_style(html_content *c,
	      ಧ್ರುವ css_computed_style *parent_style,
	      ಧ್ರುವ css_computed_style *ಬೇರು_style,
	      dom_ಗಂಟು *n)
{
	dom_string *s = ರಿಕ್ತ;
	css_stylesheet *inline_style = ರಿಕ್ತ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ಯದಿ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ಯದಿ (err != DOM_NO_ERR) {
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	ಯದಿ (s != ರಿಕ್ತ) {
		inline_style = nscss_create_inline_style(
				(ಧ್ರುವ uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		ಯದಿ (inline_style == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.ಬೇರು_style = ಬೇರು_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ಯದಿ (inline_style != ರಿಕ್ತ)
		css_stylesheet_destroy(inline_style);

	ಪ್ರತ್ಯಾಗಮ styles;
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
ಸ್ಥಿರ ಶೂನ್ಯ
box_construct_generate(dom_ಗಂಟು *n,
		       html_content *content,
		       ಸಂರಚನಾ box *box,
		       ಧ್ರುವ css_computed_style *style)
{
	ಸಂರಚನಾ box *gen = ರಿಕ್ತ;
	ಗಣನಾ css_display_e computed_display;
	ಧ್ರುವ css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	ಯದಿ (box->type != BOX_BLOCK)
		ಪ್ರತ್ಯಾಗಮ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ಯದಿ (style == ರಿಕ್ತ ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_ಬೇರು(n));
	ಯದಿ (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ರಿಕ್ತ, (css_computed_style *) style,
				false, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, content->bctx);
		ಯದಿ (gen == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_ಬೇರು(n))];

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
ಸ್ಥಿರ bool
box_construct_marker(ಸಂರಚನಾ box *box,
		     ಧ್ರುವ ವರ್ಣ *title,
		     ಸಂರಚನಾ box_construct_ctx *ctx,
		     ಸಂರಚನಾ box *parent)
{
	lwc_string *image_uri;
	ಸಂರಚನಾ box *marker;
	ಗಣನಾ css_list_style_type_e list_style_type;

	marker = box_create(ರಿಕ್ತ, box->style, false, ರಿಕ್ತ, ರಿಕ್ತ, title,
			ರಿಕ್ತ, ctx->bctx);
	ಯದಿ (marker == false)
		ಪ್ರತ್ಯಾಗಮ false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	ವಿಕಲ್ಪ (list_style_type) {
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (ವರ್ಣ *) "\342\200\242";
		marker->length = 3;
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (ವರ್ಣ *) "\342\227\213";
		marker->length = 3;
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (ವರ್ಣ *) "\342\226\252";
		marker->length = 3;
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_NONE:
		marker->text = ರಿಕ್ತ;
		marker->length = 0;
		ಭಙ್ಗ;
	}

	ಯದಿ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ರಿಕ್ತ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		ಯದಿ (error != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box__style_is_float(ಧ್ರುವ ಸಂರಚನಾ box *box)
{
	ಪ್ರತ್ಯಾಗಮ css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
	       css_computed_float(box->style) == CSS_FLOAT_ಬಲ;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box__is_flex(ಧ್ರುವ ಸಂರಚನಾ box *box)
{
	ಪ್ರತ್ಯಾಗಮ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box__containing_block_is_flex(
		ಧ್ರುವ ಸಂರಚನಾ box_construct_props *props)
{
	ಪ್ರತ್ಯಾಗಮ props->containing_block != ರಿಕ್ತ &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool
box_construct_element(ಸಂರಚನಾ box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ರಿಕ್ತ;
	ಗಣನಾ css_display_e css_display;
	ಸಂರಚನಾ box *box = ರಿಕ್ತ, *old_box;
	css_select_results *styles = ರಿಕ್ತ;
	lwc_string *bgimage_uri;
	dom_exception err;
	ಸಂರಚನಾ box_construct_props props;
	ಧ್ರುವ css_computed_style *ಬೇರು_style = ರಿಕ್ತ;

	ಪ್ರತಿಜ್ಞಾ(ctx->n != ರಿಕ್ತ);

	box_extract_properties(ctx->n, &props);

	ಯದಿ (props.containing_block != ರಿಕ್ತ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	ಯದಿ (props.ಗಂಟು_is_ಬೇರು == false) {
		ಬೇರು_style = ctx->ಬೇರು_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, ಬೇರು_style,
			ctx->n);
	ಯದಿ (styles == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (title0 != ರಿಕ್ತ) {
		ವರ್ಣ *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		ಯದಿ (t == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		props.title = talloc_strdup(ctx->bctx, t);

		ಮುಕ್ತ(t);

		ಯದಿ (props.title == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (s != ರಿಕ್ತ) {
		err = dom_string_intern(s, &id);
		ಯದಿ (err != DOM_NO_ERR)
			id = ರಿಕ್ತ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	ಯದಿ (box == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	/* If this is the root box, add it to the context */
	ಯದಿ (props.ಗಂಟು_is_ಬೇರು)
		ctx->ಬೇರು_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (s != ರಿಕ್ತ) {
		ಧ್ರುವ ವರ್ಣ *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ಯದಿ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ರಿಕ್ತ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (s != ರಿಕ್ತ) {
		ಧ್ರುವ ವರ್ಣ *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ಯದಿ ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, ರಿಕ್ತ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ಯದಿ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} ಅನ್ಯಥಾ ಯದಿ (props.ಗಂಟು_is_ಬೇರು) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} ಅನ್ಯಥಾ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ಗಂಟು_is_ಬೇರು)];

		ಯದಿ (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			ವಿಕಲ್ಪ (box->type) {
			ಪಕ್ಷ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ಭಙ್ಗ;
			ಪಕ್ಷ BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
			}
		}
	}

	ಯದಿ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Handle the :before pseudo element */
	ಯದಿ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ಯದಿ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ಗಂಟು_is_ಬೇರು) == CSS_DISPLAY_NONE &&
			props.ಗಂಟು_is_ಬೇರು == false)) {
		css_select_results_destroy(styles);
		box->styles = ರಿಕ್ತ;
		box->style = ರಿಕ್ತ;

		/* Invalidate associated gadget, if any */
		ಯದಿ (box->gadget != ರಿಕ್ತ) {
			box->gadget->box = ರಿಕ್ತ;
			box->gadget = ರಿಕ್ತ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Attach DOM node to box */
	err = dom_ಗಂಟು_set_user_data(ctx->n,
			corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_data, box, ರಿಕ್ತ,
			(ಶೂನ್ಯ *) &old_box);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	/* Attach box to DOM node */
	box->ಗಂಟು = dom_ಗಂಟು_ref(ctx->n);

	ಯದಿ (props.inline_container == ರಿಕ್ತ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.ಗಂಟು_is_ಬೇರು == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ಪ್ರತಿಜ್ಞಾ(props.containing_block != ರಿಕ್ತ &&
				"Box must have containing block.");

		props.inline_container = box_create(ರಿಕ್ತ, ರಿಕ್ತ, false, ರಿಕ್ತ,
				ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ctx->bctx);
		ಯದಿ (props.inline_container == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ಯದಿ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ರಿಕ್ತ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		ಯದಿ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ಯದಿ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			nsurl_unref(url);
		}
	}

	ಯದಿ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ಯದಿ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ಪ್ರತಿಜ್ಞಾ(props.inline_container != ರಿಕ್ತ);

		box_add_child(props.inline_container, box);
	} ಅನ್ಯಥಾ {
		ಯದಿ (ns_computed_display(box->style, props.ಗಂಟು_is_ಬೇರು) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			ಯದಿ (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (props.ಗಂಟು_is_ಬೇರು == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ಎಡ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ಬಲ)) {
			/* Float: insert a float between the parent and box. */
			ಸಂರಚನಾ box *flt = box_create(ರಿಕ್ತ, ರಿಕ್ತ, false,
					props.href, props.target, props.title,
					ರಿಕ್ತ, ctx->bctx);
			ಯದಿ (flt == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			ಯದಿ (css_computed_float(box->style) == CSS_FLOAT_ಎಡ)
				flt->type = BOX_FLOAT_ಎಡ;
			ಅನ್ಯಥಾ
				flt->type = BOX_FLOAT_ಬಲ;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} ಅನ್ಯಥಾ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ಯದಿ (props.containing_block != ರಿಕ್ತ)
				box_add_child(props.containing_block, box);
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ಸ್ಥಿರ ಶೂನ್ಯ box_construct_element_after(dom_ಗಂಟು *n, html_content *content)
{
	ಸಂರಚನಾ box_construct_props props;
	ಸಂರಚನಾ box *box = box_for_ಗಂಟು(n);

	ಪ್ರತಿಜ್ಞಾ(box != ರಿಕ್ತ);

	box_extract_properties(n, &props);

	ಯದಿ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ಸಂರಚನಾ box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_ಗಂಟು_has_child_nodes(n, &has_children);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ;

		ಯದಿ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ಪ್ರತ್ಯಾಗಮ;
		}

		ಯದಿ (props.inline_container == ರಿಕ್ತ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ರಿಕ್ತ, ರಿಕ್ತ, false,
					ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, content->bctx);
			ಯದಿ (props.inline_container == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(ರಿಕ್ತ, box->style, false,
				box->href, box->target, box->title,
				box->id == ರಿಕ್ತ ? ರಿಕ್ತ :
				lwc_string_ref(box->id), content->bctx);
		ಯದಿ (inline_end != ರಿಕ್ತ) {
			inline_end->type = BOX_INLINE_END;

			ಪ್ರತಿಜ್ಞಾ(props.inline_container != ರಿಕ್ತ);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} ಅನ್ಯಥಾ ಯದಿ (!(box->flags & IS_REPLACED)) {
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
ಸ್ಥಿರ dom_ಗಂಟು *
next_ಗಂಟು(dom_ಗಂಟು *n, html_content *content, bool convert_children)
{
	dom_ಗಂಟು *next = ರಿಕ್ತ;
	bool has_children;
	dom_exception err;

	err = dom_ಗಂಟು_has_child_nodes(n, &has_children);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_ಗಂಟು_unref(n);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (convert_children && has_children) {
		err = dom_ಗಂಟು_get_first_child(n, &next);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		dom_ಗಂಟು_unref(n);
	} ಅನ್ಯಥಾ {
		err = dom_ಗಂಟು_get_next_sibling(n, &next);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		ಯದಿ (next != ರಿಕ್ತ) {
			ಯದಿ (box_for_ಗಂಟು(n) != ರಿಕ್ತ)
				box_construct_element_after(n, content);
			dom_ಗಂಟು_unref(n);
		} ಅನ್ಯಥಾ {
			ಯದಿ (box_for_ಗಂಟು(n) != ರಿಕ್ತ)
				box_construct_element_after(n, content);

			ಯಾವತ್ (box_is_ಬೇರು(n) == false) {
				dom_ಗಂಟು *parent = ರಿಕ್ತ;
				dom_ಗಂಟು *parent_next = ರಿಕ್ತ;

				err = dom_ಗಂಟು_get_parent_ಗಂಟು(n, &parent);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(n);
					ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
				}

				ಪ್ರತಿಜ್ಞಾ(parent != ರಿಕ್ತ);

				err = dom_ಗಂಟು_get_next_sibling(parent,
						&parent_next);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(parent);
					dom_ಗಂಟು_unref(n);
					ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
				}

				ಯದಿ (parent_next != ರಿಕ್ತ) {
					dom_ಗಂಟು_unref(parent_next);
					dom_ಗಂಟು_unref(parent);
					ಭಙ್ಗ;
				}

				dom_ಗಂಟು_unref(n);
				n = parent;
				parent = ರಿಕ್ತ;

				ಯದಿ (box_for_ಗಂಟು(n) != ರಿಕ್ತ) {
					box_construct_element_after(
							n, content);
				}
			}

			ಯದಿ (box_is_ಬೇರು(n) == false) {
				dom_ಗಂಟು *parent = ರಿಕ್ತ;

				err = dom_ಗಂಟು_get_parent_ಗಂಟು(n, &parent);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(n);
					ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
				}

				ಪ್ರತಿಜ್ಞಾ(parent != ರಿಕ್ತ);

				err = dom_ಗಂಟು_get_next_sibling(parent, &next);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(parent);
					dom_ಗಂಟು_unref(n);
					ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
				}

				ಯದಿ (box_for_ಗಂಟು(parent) != ರಿಕ್ತ) {
					box_construct_element_after(parent,
							content);
				}

				dom_ಗಂಟು_unref(parent);
			}

			dom_ಗಂಟು_unref(n);
		}
	}

	ಪ್ರತ್ಯಾಗಮ next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
box_text_transform(ವರ್ಣ *s, unsigned ಪೂರ್ಣಾಙ್ಕ len, ಗಣನಾ css_text_transform_e tt)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಯದಿ (len == 0)
		ಪ್ರತ್ಯಾಗಮ;
	ವಿಕಲ್ಪ (tt) {
		ಪಕ್ಷ CSS_TEXT_TRANSFORM_UPPERCASE:
			ಪ್ರತಿ (i = 0; i < len; ++i)
				ಯದಿ ((unsigned ವರ್ಣ) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_TEXT_TRANSFORM_LOWERCASE:
			ಪ್ರತಿ (i = 0; i < len; ++i)
				ಯದಿ ((unsigned ವರ್ಣ) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_TEXT_TRANSFORM_CAPITALIZE:
			ಯದಿ ((unsigned ವರ್ಣ) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			ಪ್ರತಿ (i = 1; i < len; ++i)
				ಯದಿ ((unsigned ವರ್ಣ) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool box_construct_text(ಸಂರಚನಾ box_construct_ctx *ctx)
{
	ಸಂರಚನಾ box_construct_props props;
	ಸಂರಚನಾ box *box = ರಿಕ್ತ;
	dom_string *content;
	dom_exception err;

	ಪ್ರತಿಜ್ಞಾ(ctx->n != ರಿಕ್ತ);

	box_extract_properties(ctx->n, &props);

	ಪ್ರತಿಜ್ಞಾ(props.containing_block != ರಿಕ್ತ);

	err = dom_characterdata_get_data(ctx->n, &content);
	ಯದಿ (err != DOM_NO_ERR || content == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		ವರ್ಣ *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		ಯದಿ (text == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ಯದಿ (text[0] == ' ' && text[1] == 0) {
			ಯದಿ (props.inline_container != ರಿಕ್ತ) {
				ಪ್ರತಿಜ್ಞಾ(props.inline_container->last != ರಿಕ್ತ);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			ಮುಕ್ತ(text);

			ಪ್ರತ್ಯಾಗಮ true;
		}

		ಯದಿ (props.inline_container == ರಿಕ್ತ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ರಿಕ್ತ, ರಿಕ್ತ, false,
					ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ctx->bctx);
			ಯದಿ (props.inline_container == ರಿಕ್ತ) {
				ಮುಕ್ತ(text);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ರಿಕ್ತ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ರಿಕ್ತ, ctx->bctx);
		ಯದಿ (box == ರಿಕ್ತ) {
			ಮುಕ್ತ(text);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		ಮುಕ್ತ(text);
		ಯದಿ (box->text == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		ಯದಿ (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		ಯದಿ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		ಯದಿ (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			ಯದಿ (box->prev != ರಿಕ್ತ)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} ಅನ್ಯಥಾ {
		/* white-space: pre */
		ವರ್ಣ *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		ವರ್ಣ *current;
		ಗಣನಾ css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ಪ್ರತಿಜ್ಞಾ(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_ಸಾಲು ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		ಯದಿ (text == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		ಪ್ರತಿ (i = 0; i < text_len; i++)
			ಯದಿ (text[i] == '\t')
				text[i] = ' ';

		ಯದಿ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		ಯದಿ (props.containing_block->flags & PRE_STRIP) {
			ವಿಕಲ್ಪ (*current) {
			ಪಕ್ಷ '\n':
				current++;
				ಭಙ್ಗ;
			ಪಕ್ಷ '\r':
				current++;
				ಯದಿ (*current == '\n')
					current++;
				ಭಙ್ಗ;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		ಕುರು {
			size_t len = strcspn(current, "\r\n");

			ವರ್ಣ old = current[len];

			current[len] = 0;

			ಯದಿ (props.inline_container == ರಿಕ್ತ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ರಿಕ್ತ, ರಿಕ್ತ,
						false, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ,
						ctx->bctx);
				ಯದಿ (props.inline_container == ರಿಕ್ತ) {
					ಮುಕ್ತ(text);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ರಿಕ್ತ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ರಿಕ್ತ, ctx->bctx);
			ಯದಿ (box == ರಿಕ್ತ) {
				ಮುಕ್ತ(text);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			ಯದಿ (box->text == ರಿಕ್ತ) {
				ಮುಕ್ತ(text);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			ಯದಿ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ರಿಕ್ತ, ರಿಕ್ತ,
						false, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ,
						ctx->bctx);
				ಯದಿ (props.inline_container == ರಿಕ್ತ) {
					ಮುಕ್ತ(text);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				ಯದಿ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ಅನ್ಯಥಾ
					current++;
			}
		} ಯಾವತ್ (*current);

		ಮುಕ್ತ(text);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ಸ್ಥಿರ ಶೂನ್ಯ convert_xml_to_box(ಸಂರಚನಾ box_construct_ctx *ctx)
{
	dom_ಗಂಟು *next;
	bool convert_children;
	uint32_t ಅಂಕೆ_processed = 0;
	ಧ್ರುವ uint32_t max_processed_before_yield = 10;

	ಕುರು {
		convert_children = true;

		ಪ್ರತಿಜ್ಞಾ(ctx->n != ರಿಕ್ತ);

		ಯದಿ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ಗಂಟು_unref(ctx->n);
			ಮುಕ್ತ(ctx);
			ಪ್ರತ್ಯಾಗಮ;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_ಗಂಟು(ctx->n, ctx->content, convert_children);
		ಯಾವತ್ (next != ರಿಕ್ತ) {
			dom_ಗಂಟು_type type;
			dom_exception err;

			err = dom_ಗಂಟು_get_ಗಂಟು_type(next, &type);
			ಯದಿ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ಗಂಟು_unref(next);
				ಮುಕ್ತ(ctx);
				ಪ್ರತ್ಯಾಗಮ;
			}

			ಯದಿ (type == DOM_ELEMENT_ಗಂಟು)
				ಭಙ್ಗ;

			ಯದಿ (type == DOM_TEXT_ಗಂಟು) {
				ctx->n = next;
				ಯದಿ (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ಗಂಟು_unref(ctx->n);
					ಮುಕ್ತ(ctx);
					ಪ್ರತ್ಯಾಗಮ;
				}
			}

			next = next_ಗಂಟು(next, ctx->content, true);
		}

		ctx->n = next;

		ಯದಿ (next == ರಿಕ್ತ) {
			/* Conversion complete */
			ಸಂರಚನಾ box ಬೇರು;

			memset(&ಬೇರು, 0, ಪರಿಮಾಣ(ಬೇರು));

			ಬೇರು.type = BOX_BLOCK;
			ಬೇರು.children = ಬೇರು.last = ctx->ಬೇರು_box;
			ಬೇರು.children->parent = &ಬೇರು;

			/** \todo Remove box_normalise_block */
			ಯದಿ (box_normalise_block(&ಬೇರು, ctx->ಬೇರು_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ಅನ್ಯಥಾ {
				ctx->content->layout = ಬೇರು.children;
				ctx->content->layout->parent = ರಿಕ್ತ;

				ctx->cb(ctx->content, true);
			}

			ಪ್ರತಿಜ್ಞಾ(ctx->n == ರಿಕ್ತ);

			ಮುಕ್ತ(ctx);
			ಪ್ರತ್ಯಾಗಮ;
		}
	} ಯಾವತ್ (++ಅಂಕೆ_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (ಶೂನ್ಯ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_ಗಂಟು *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   ಶೂನ್ಯ **box_conversion_context)
{
	ಸಂರಚನಾ box_construct_ctx *ctx;

	ಪ್ರತಿಜ್ಞಾ(box_conversion_context != ರಿಕ್ತ);

	ಯದಿ (c->bctx == ರಿಕ್ತ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, ಪೂರ್ಣಾಙ್ಕ);
		ಯದಿ (c->bctx == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	ctx = malloc(ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ಗಂಟು_ref(n);
	ctx->ಬೇರು_box = ರಿಕ್ತ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ಪ್ರತ್ಯಾಗಮ guit->misc->schedule(0, (ಶೂನ್ಯ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(ಶೂನ್ಯ *box_conversion_context)
{
	ಸಂರಚನಾ box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (ಶೂನ್ಯ *)convert_xml_to_box, ctx);
	ಯದಿ (err != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ err;
	}

	dom_ಗಂಟು_unref(ctx->n);
	ಮುಕ್ತ(ctx);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ಸಂರಚನಾ box *box_for_ಗಂಟು(dom_ಗಂಟು *n)
{
	ಸಂರಚನಾ box *box = ರಿಕ್ತ;
	dom_exception err;

	err = dom_ಗಂಟು_get_user_data(n, corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_data,
			(ಶೂನ್ಯ *) &box);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ಧ್ರುವ html_content *content,
		 ಧ್ರುವ dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	ವರ್ಣ *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ i, j, end;
	nserror error;
	ಧ್ರುವ ವರ್ಣ *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ false;

	/* copy to s, removing white space and control characters */
	ಪ್ರತಿ (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	ಪ್ರತಿ (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	ಪ್ರತಿ (j = 0; i != end; i++) {
		ಯದಿ ((unsigned ವರ್ಣ) rel[i] < 0x20) {
			; /* skip control characters */
		} ಅನ್ಯಥಾ ಯದಿ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ಅನ್ಯಥಾ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ಯದಿ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ಯದಿ (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ಯದಿ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ಯದಿ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ಯದಿ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ಅನ್ಯಥಾ ಯದಿ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	ಮುಕ್ತ(s);
	ಯದಿ (error != NSERROR_OK) {
		*result = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}
