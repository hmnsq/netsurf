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
ರಚನೆ box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_ಗಂಟು *n;			/**< Current node to process */

	ರಚನೆ box *ಬೇರು_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	ಪೂರ್ಣಾಂಕ *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ರಚನೆ box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ಸ್ಥಿರಾಂಕ css_computed_style *ಪೋಷಕ_style;
	/** Current link target, or NULL if none */
	ರಚನೆ nsurl *href;
	/** Current frame target, or NULL if none */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ;
	/** Current title attribute, or NULL if none */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;
	/** Identity of the current block-level container */
	ರಚನೆ box *containing_ಖಂಡ;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ರಚನೆ box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ಗಂಟು_is_ಬೇರು;
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ box_type box_map[] = {
	BOX_ಖಂಡ,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_ಖಂಡ,           /* CSS_DISPLAY_BLOCK */
	BOX_ಖಂಡ,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_ಖಂಡ,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_ಕೋಷ್ಟಕ,           /* CSS_DISPLAY_TABLE */
	BOX_ಕೋಷ್ಟಕ,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_ಕೋಷ್ಟಕ_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_ಖಂಡ,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_ಖಂಡ,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box_is_ಬೇರು(dom_ಗಂಟು *n)
{
	dom_ಗಂಟು *ಪೋಷಕ;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(n, &ಪೋಷಕ);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ಪೋಷಕ != ಖಾಲಿ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(ಪೋಷಕ, &type);

		dom_ಗಂಟು_unref(ಪೋಷಕ);

		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (type != DOM_DOCUMENT_ಗಂಟು)
			ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ಸ್ಥಿರ ಶೂನ್ಯ
box_ಹೊರತೆಗೆ_properties(dom_ಗಂಟು *n, ರಚನೆ box_construct_props *props)
{
	memset(props, 0, ಗಾತ್ರ(*props));

	props->ಗಂಟು_is_ಬೇರು = box_is_ಬೇರು(n);

	/* Extract properties from containing DOM node */
	ಆದರೆ (props->ಗಂಟು_is_ಬೇರು == false) {
		dom_ಗಂಟು *current_ಗಂಟು = n;
		dom_ಗಂಟು *ಪೋಷಕ_ಗಂಟು = ಖಾಲಿ;
		ರಚನೆ box *ಪೋಷಕ_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ಆಗ (true) {
			err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(current_ಗಂಟು,
					&ಪೋಷಕ_ಗಂಟು);
			ಆದರೆ (err != DOM_NO_ERR || ಪೋಷಕ_ಗಂಟು == ಖಾಲಿ)
				ನಿಲ್ಲಿಸು;

			ಪೋಷಕ_box = box_for_ಗಂಟು(ಪೋಷಕ_ಗಂಟು);

			ಆದರೆ (ಪೋಷಕ_box != ಖಾಲಿ) {
				props->ಪೋಷಕ_style = ಪೋಷಕ_box->style;
				props->href = ಪೋಷಕ_box->href;
				props->ಗುರಿ = ಪೋಷಕ_box->ಗುರಿ;
				props->title = ಪೋಷಕ_box->title;

				dom_ಗಂಟು_unref(ಪೋಷಕ_ಗಂಟು);
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (current_ಗಂಟು != n)
					dom_ಗಂಟು_unref(current_ಗಂಟು);
				current_ಗಂಟು = ಪೋಷಕ_ಗಂಟು;
				ಪೋಷಕ_ಗಂಟು = ಖಾಲಿ;
			}
		}

		/* Find containing block (may be parent) */
		ಆಗ (true) {
			ರಚನೆ box *b;

			err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(current_ಗಂಟು,
					&ಪೋಷಕ_ಗಂಟು);
			ಆದರೆ (err != DOM_NO_ERR || ಪೋಷಕ_ಗಂಟು == ಖಾಲಿ) {
				ಆದರೆ (current_ಗಂಟು != n)
					dom_ಗಂಟು_unref(current_ಗಂಟು);
				ನಿಲ್ಲಿಸು;
			}

			ಆದರೆ (current_ಗಂಟು != n)
				dom_ಗಂಟು_unref(current_ಗಂಟು);

			b = box_for_ಗಂಟು(ಪೋಷಕ_ಗಂಟು);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ಆದರೆ (b != ಖಾಲಿ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_ಖಂಡ = b;

				dom_ಗಂಟು_unref(ಪೋಷಕ_ಗಂಟು);
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				current_ಗಂಟು = ಪೋಷಕ_ಗಂಟು;
				ಪೋಷಕ_ಗಂಟು = ಖಾಲಿ;
			}
		}
	}

	/* Compute current inline container, if any */
	ಆದರೆ (props->containing_ಖಂಡ != ಖಾಲಿ &&
			props->containing_ಖಂಡ->ಕೊನೆಯ != ಖಾಲಿ &&
			props->containing_ಖಂಡ->ಕೊನೆಯ->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_ಖಂಡ->ಕೊನೆಯ;
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
	      ಸ್ಥಿರಾಂಕ css_computed_style *ಪೋಷಕ_style,
	      ಸ್ಥಿರಾಂಕ css_computed_style *ಬೇರು_style,
	      dom_ಗಂಟು *n)
{
	dom_string *s = ಖಾಲಿ;
	css_stylesheet *inline_style = ಖಾಲಿ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ಆದರೆ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ಆದರೆ (err != DOM_NO_ERR) {
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	ಆದರೆ (s != ಖಾಲಿ) {
		inline_style = nscss_create_inline_style(
				(ಸ್ಥಿರಾಂಕ uint8_t *) dom_string_ದತ್ತಾಂಶ(s),
				dom_string_byte_ಉದ್ದಳತೆ(s),
				c->encoding,
				nsurl_access(c->ತಳ_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_NONE);

		dom_string_unref(s);

		ಆದರೆ (inline_style == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
	ctx.ತಳ_url = c->ತಳ_url;
	ctx.universal = c->universal;
	ctx.ಬೇರು_style = ಬೇರು_style;
	ctx.ಪೋಷಕ_style = ಪೋಷಕ_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ಆದರೆ (inline_style != ಖಾಲಿ)
		css_stylesheet_destroy(inline_style);

	ಹಿಂತಿರುಗಿ styles;
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
		       ರಚನೆ box *box,
		       ಸ್ಥಿರಾಂಕ css_computed_style *style)
{
	ರಚನೆ box *gen = ಖಾಲಿ;
	ಎಣಿಕೆ css_display_e computed_display;
	ಸ್ಥಿರಾಂಕ css_computed_content_ಅಂಶ *c_ಅಂಶ;

	/* Nothing to generate if the parent box is not a block */
	ಆದರೆ (box->type != BOX_ಖಂಡ)
		ಹಿಂತಿರುಗಿ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ಆದರೆ (style == ಖಾಲಿ ||
			css_computed_content(style, &c_ಅಂಶ) ==
			CSS_CONTENT_ಸಾಮಾನ್ಯ) {
		/* No pseudo element */
		ಹಿಂತಿರುಗಿ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_ಬೇರು(n));
	ಆದರೆ (computed_display == CSS_DISPLAY_ಖಂಡ ||
			computed_display == CSS_DISPLAY_ಕೋಷ್ಟಕ) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ಖಾಲಿ, (css_computed_style *) style,
				false, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, content->bctx);
		ಆದರೆ (gen == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_ಬೇರು(n))];

		box_ಕೂಡಿಸು_ಮಗು(box, gen);
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
box_construct_marker(ರಚನೆ box *box,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title,
		     ರಚನೆ box_construct_ctx *ctx,
		     ರಚನೆ box *ಪೋಷಕ)
{
	lwc_string *image_uri;
	ರಚನೆ box *marker;
	ಎಣಿಕೆ css_list_style_type_e list_style_type;

	marker = box_create(ಖಾಲಿ, box->style, false, ಖಾಲಿ, ಖಾಲಿ, title,
			ಖಾಲಿ, ctx->bctx);
	ಆದರೆ (marker == false)
		ಹಿಂತಿರುಗಿ false;

	marker->type = BOX_ಖಂಡ;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	ಆಯ್ಕೆ (list_style_type) {
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->ಪಠ್ಯ = (ಅಕ್ಷರ *) "\342\200\242";
		marker->ಉದ್ದಳತೆ = 3;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->ಪಠ್ಯ = (ಅಕ್ಷರ *) "\342\227\213";
		marker->ಉದ್ದಳತೆ = 3;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ವರ್ಗಫಲ:
		/* 25AA BLACK SMALL SQUARE */
		marker->ಪಠ್ಯ = (ಅಕ್ಷರ *) "\342\226\252";
		marker->ಉದ್ದಳತೆ = 3;
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_NONE:
		marker->ಪಠ್ಯ = ಖಾಲಿ;
		marker->ಉದ್ದಳತೆ = 0;
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ಖಾಲಿ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ದತ್ತಾಂಶ(image_uri), &url);
		ಆದರೆ (error != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ಹಿಂತಿರುಗಿ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->ಪೋಷಕ = box;

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box__style_is_float(ಸ್ಥಿರಾಂಕ ರಚನೆ box *box)
{
	ಹಿಂತಿರುಗಿ css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
	       css_computed_float(box->style) == CSS_FLOAT_ಬಲ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box__is_flex(ಸ್ಥಿರಾಂಕ ರಚನೆ box *box)
{
	ಹಿಂತಿರುಗಿ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box_containing_ಖಂಡ_is_flex(
		ಸ್ಥಿರಾಂಕ ರಚನೆ box_construct_props *props)
{
	ಹಿಂತಿರುಗಿ props->containing_ಖಂಡ != ಖಾಲಿ &&
	       box__is_flex(props->containing_ಖಂಡ);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool
box_construct_element(ರಚನೆ box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ಖಾಲಿ;
	ಎಣಿಕೆ css_display_e css_display;
	ರಚನೆ box *box = ಖಾಲಿ, *old_box;
	css_select_results *styles = ಖಾಲಿ;
	lwc_string *bgimage_uri;
	dom_exception err;
	ರಚನೆ box_construct_props props;
	ಸ್ಥಿರಾಂಕ css_computed_style *ಬೇರು_style = ಖಾಲಿ;

	ಪ್ರತಿಜ್ಞಾ(ctx->n != ಖಾಲಿ);

	box_ಹೊರತೆಗೆ_properties(ctx->n, &props);

	ಆದರೆ (props.containing_ಖಂಡ != ಖಾಲಿ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_ಖಂಡ->flags &= ~PRE_STRIP;
	}

	ಆದರೆ (props.ಗಂಟು_is_ಬೇರು == false) {
		ಬೇರು_style = ctx->ಬೇರು_box->style;
	}

	styles = box_get_style(ctx->content, props.ಪೋಷಕ_style, ಬೇರು_style,
			ctx->n);
	ಆದರೆ (styles == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (title0 != ಖಾಲಿ) {
		ಅಕ್ಷರ *t = squash_whitespace(dom_string_ದತ್ತಾಂಶ(title0));

		dom_string_unref(title0);

		ಆದರೆ (t == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		props.title = talloc_strdup(ctx->bctx, t);

		ಬಿಡುಗಡೆ(t);

		ಆದರೆ (props.title == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (s != ಖಾಲಿ) {
		err = dom_string_intern(s, &id);
		ಆದರೆ (err != DOM_NO_ERR)
			id = ಖಾಲಿ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.ಗುರಿ, props.title, id,
			ctx->bctx);
	ಆದರೆ (box == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	/* If this is the root box, add it to the context */
	ಆದರೆ (props.ಗಂಟು_is_ಬೇರು)
		ctx->ಬೇರು_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (s != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val = dom_string_ದತ್ತಾಂಶ(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ಆದರೆ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ಖಾಲಿ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (s != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val = dom_string_ದತ್ತಾಂಶ(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ಆದರೆ ('0' <= val[0] && val[0] <= '9')
			box->ಅಡ್ಡಸಾಲುಗಳು = clamp(strtol(val, ಖಾಲಿ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ಆದರೆ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_ಖಂಡ ||
			 css_display == CSS_DISPLAY_INLINE_ಕೋಷ್ಟಕ ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_ಖಂಡ];
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (props.ಗಂಟು_is_ಬೇರು) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_ಖಂಡ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ಗಂಟು_is_ಬೇರು)];

		ಆದರೆ (props.containing_ಖಂಡ->type == BOX_FLEX ||
		    props.containing_ಖಂಡ->type == BOX_INLINE_FLEX) {
			/* Blockification */
			ಆಯ್ಕೆ (box->type) {
			ಸ್ಥಿತಿ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
				box->type = BOX_ಖಂಡ;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಆದರೆ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* Handle the :before pseudo element */
	ಆದರೆ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ಆದರೆ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ಗಂಟು_is_ಬೇರು) == CSS_DISPLAY_NONE &&
			props.ಗಂಟು_is_ಬೇರು == false)) {
		css_select_results_destroy(styles);
		box->styles = ಖಾಲಿ;
		box->style = ಖಾಲಿ;

		/* Invalidate associated gadget, if any */
		ಆದರೆ (box->gadget != ಖಾಲಿ) {
			box->gadget->box = ಖಾಲಿ;
			box->gadget = ಖಾಲಿ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ಹಿಂತಿರುಗಿ true;
	}

	/* Attach DOM node to box */
	err = dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(ctx->n,
			corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_ದತ್ತಾಂಶ, box, ಖಾಲಿ,
			(ಶೂನ್ಯ *) &old_box);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	/* Attach box to DOM node */
	box->ಗಂಟು = dom_ಗಂಟು_ref(ctx->n);

	ಆದರೆ (props.inline_container == ಖಾಲಿ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_ಖಂಡ ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box_containing_ಖಂಡ_is_flex(&props))) &&
			props.ಗಂಟು_is_ಬೇರು == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ಪ್ರತಿಜ್ಞಾ(props.containing_ಖಂಡ != ಖಾಲಿ &&
				"Box must have containing block.");

		props.inline_container = box_create(ಖಾಲಿ, ಖಾಲಿ, false, ಖಾಲಿ,
				ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ctx->bctx);
		ಆದರೆ (props.inline_container == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ಆದರೆ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ಖಾಲಿ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ದತ್ತಾಂಶ(bgimage_uri), &url);
		ಆದರೆ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ಆದರೆ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ಹಿಂತಿರುಗಿ false;
			}
			nsurl_unref(url);
		}
	}

	ಆದರೆ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ಆದರೆ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_ಖಂಡ) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ಪ್ರತಿಜ್ಞಾ(props.inline_container != ಖಾಲಿ);

		box_ಕೂಡಿಸು_ಮಗು(props.inline_container, box);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (ns_computed_display(box->style, props.ಗಂಟು_is_ಬೇರು) ==
				CSS_DISPLAY_LIST_ಅಂಶ) {
			/* List item: compute marker */
			ಆದರೆ (box_construct_marker(box, props.title, ctx,
					props.containing_ಖಂಡ) == false)
				ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (props.ಗಂಟು_is_ಬೇರು == false &&
				box_containing_ಖಂಡ_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ಎಡ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ಬಲ)) {
			/* Float: insert a float between the parent and box. */
			ರಚನೆ box *flt = box_create(ಖಾಲಿ, ಖಾಲಿ, false,
					props.href, props.ಗುರಿ, props.title,
					ಖಾಲಿ, ctx->bctx);
			ಆದರೆ (flt == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			ಆದರೆ (css_computed_float(box->style) == CSS_FLOAT_ಎಡ)
				flt->type = BOX_FLOAT_ಎಡ;
			ಇಲ್ಲದಿದ್ದರೆ
				flt->type = BOX_FLOAT_ಬಲ;

			box_ಕೂಡಿಸು_ಮಗು(props.inline_container, flt);
			box_ಕೂಡಿಸು_ಮಗು(flt, box);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ಆದರೆ (props.containing_ಖಂಡ != ಖಾಲಿ)
				box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ, box);
		}
	}

	ಹಿಂತಿರುಗಿ true;
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
	ರಚನೆ box_construct_props props;
	ರಚನೆ box *box = box_for_ಗಂಟು(n);

	ಪ್ರತಿಜ್ಞಾ(box != ಖಾಲಿ);

	box_ಹೊರತೆಗೆ_properties(n, &props);

	ಆದರೆ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ರಚನೆ box *inline_ಕೊನೆ;
		bool has_children;
		dom_exception err;

		err = dom_ಗಂಟು_has_ಮಗು_nodes(n, &has_children);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ;

		ಆದರೆ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ಹಿಂತಿರುಗಿ;
		}

		ಆದರೆ (props.inline_container == ಖಾಲಿ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ಖಾಲಿ, ಖಾಲಿ, false,
					ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, content->bctx);
			ಆದರೆ (props.inline_container == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ,
					props.inline_container);
		}

		inline_ಕೊನೆ = box_create(ಖಾಲಿ, box->style, false,
				box->href, box->ಗುರಿ, box->title,
				box->id == ಖಾಲಿ ? ಖಾಲಿ :
				lwc_string_ref(box->id), content->bctx);
		ಆದರೆ (inline_ಕೊನೆ != ಖಾಲಿ) {
			inline_ಕೊನೆ->type = BOX_INLINE_ಕೊನೆ;

			ಪ್ರತಿಜ್ಞಾ(props.inline_container != ಖಾಲಿ);

			box_ಕೂಡಿಸು_ಮಗು(props.inline_container, inline_ಕೊನೆ);

			box->inline_ಕೊನೆ = inline_ಕೊನೆ;
			inline_ಕೊನೆ->inline_ಕೊನೆ = box;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(box->flags & IS_REPLACED)) {
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
ಮುಂದಿನ_ಗಂಟು(dom_ಗಂಟು *n, html_content *content, bool convert_children)
{
	dom_ಗಂಟು *ಮುಂದಿನ = ಖಾಲಿ;
	bool has_children;
	dom_exception err;

	err = dom_ಗಂಟು_has_ಮಗು_nodes(n, &has_children);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_ಗಂಟು_unref(n);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (convert_children && has_children) {
		err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(n, &ಮುಂದಿನ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		dom_ಗಂಟು_unref(n);
	} ಇಲ್ಲದಿದ್ದರೆ {
		err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(n, &ಮುಂದಿನ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		ಆದರೆ (ಮುಂದಿನ != ಖಾಲಿ) {
			ಆದರೆ (box_for_ಗಂಟು(n) != ಖಾಲಿ)
				box_construct_element_after(n, content);
			dom_ಗಂಟು_unref(n);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (box_for_ಗಂಟು(n) != ಖಾಲಿ)
				box_construct_element_after(n, content);

			ಆಗ (box_is_ಬೇರು(n) == false) {
				dom_ಗಂಟು *ಪೋಷಕ = ಖಾಲಿ;
				dom_ಗಂಟು *ಪೋಷಕ_ಮುಂದಿನ = ಖಾಲಿ;

				err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(n, &ಪೋಷಕ);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(n);
					ಹಿಂತಿರುಗಿ ಖಾಲಿ;
				}

				ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ != ಖಾಲಿ);

				err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(ಪೋಷಕ,
						&ಪೋಷಕ_ಮುಂದಿನ);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(ಪೋಷಕ);
					dom_ಗಂಟು_unref(n);
					ಹಿಂತಿರುಗಿ ಖಾಲಿ;
				}

				ಆದರೆ (ಪೋಷಕ_ಮುಂದಿನ != ಖಾಲಿ) {
					dom_ಗಂಟು_unref(ಪೋಷಕ_ಮುಂದಿನ);
					dom_ಗಂಟು_unref(ಪೋಷಕ);
					ನಿಲ್ಲಿಸು;
				}

				dom_ಗಂಟು_unref(n);
				n = ಪೋಷಕ;
				ಪೋಷಕ = ಖಾಲಿ;

				ಆದರೆ (box_for_ಗಂಟು(n) != ಖಾಲಿ) {
					box_construct_element_after(
							n, content);
				}
			}

			ಆದರೆ (box_is_ಬೇರು(n) == false) {
				dom_ಗಂಟು *ಪೋಷಕ = ಖಾಲಿ;

				err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(n, &ಪೋಷಕ);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(n);
					ಹಿಂತಿರುಗಿ ಖಾಲಿ;
				}

				ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ != ಖಾಲಿ);

				err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(ಪೋಷಕ, &ಮುಂದಿನ);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(ಪೋಷಕ);
					dom_ಗಂಟು_unref(n);
					ಹಿಂತಿರುಗಿ ಖಾಲಿ;
				}

				ಆದರೆ (box_for_ಗಂಟು(ಪೋಷಕ) != ಖಾಲಿ) {
					box_construct_element_after(ಪೋಷಕ,
							content);
				}

				dom_ಗಂಟು_unref(ಪೋಷಕ);
			}

			dom_ಗಂಟು_unref(n);
		}
	}

	ಹಿಂತಿರುಗಿ ಮುಂದಿನ;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
box_ಪಠ್ಯ_transform(ಅಕ್ಷರ *s, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len, ಎಣಿಕೆ css_ಪಠ್ಯ_transform_e tt)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಆದರೆ (len == 0)
		ಹಿಂತಿರುಗಿ;
	ಆಯ್ಕೆ (tt) {
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_UPPERCASE:
			ಗಾಗಿ (i = 0; i < len; ++i)
				ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) s[i] < 0x80)
					s[i] = ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(s[i]);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_LOWERCASE:
			ಗಾಗಿ (i = 0; i < len; ++i)
				ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) s[i] < 0x80)
					s[i] = ascii_ಗಮ್ಯ_ಅಧೋ(s[i]);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_CAPITALIZE:
			ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) s[0] < 0x80)
				s[0] = ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(s[0]);
			ಗಾಗಿ (i = 1; i < len; ++i)
				ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(s[i]);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool box_construct_ಪಠ್ಯ(ರಚನೆ box_construct_ctx *ctx)
{
	ರಚನೆ box_construct_props props;
	ರಚನೆ box *box = ಖಾಲಿ;
	dom_string *content;
	dom_exception err;

	ಪ್ರತಿಜ್ಞಾ(ctx->n != ಖಾಲಿ);

	box_ಹೊರತೆಗೆ_properties(ctx->n, &props);

	ಪ್ರತಿಜ್ಞಾ(props.containing_ಖಂಡ != ಖಾಲಿ);

	err = dom_characterdata_get_ದತ್ತಾಂಶ(ctx->n, &content);
	ಆದರೆ (err != DOM_NO_ERR || content == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (css_computed_white_space(props.ಪೋಷಕ_style) ==
			CSS_WHITE_SPACE_ಸಾಮಾನ್ಯ ||
			css_computed_white_space(props.ಪೋಷಕ_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		ಅಕ್ಷರ *ಪಠ್ಯ;

		ಪಠ್ಯ = squash_whitespace(dom_string_ದತ್ತಾಂಶ(content));

		dom_string_unref(content);

		ಆದರೆ (ಪಠ್ಯ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ಆದರೆ (ಪಠ್ಯ[0] == ' ' && ಪಠ್ಯ[1] == 0) {
			ಆದರೆ (props.inline_container != ಖಾಲಿ) {
				ಪ್ರತಿಜ್ಞಾ(props.inline_container->ಕೊನೆಯ != ಖಾಲಿ);

				props.inline_container->ಕೊನೆಯ->space =
						UNKNOWN_ಅಗಲ;
			}

			ಬಿಡುಗಡೆ(ಪಠ್ಯ);

			ಹಿಂತಿರುಗಿ true;
		}

		ಆದರೆ (props.inline_container == ಖಾಲಿ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ಖಾಲಿ, ಖಾಲಿ, false,
					ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ctx->bctx);
			ಆದರೆ (props.inline_container == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ಪಠ್ಯ);
				ಹಿಂತಿರುಗಿ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ಖಾಲಿ,
				(css_computed_style *) props.ಪೋಷಕ_style,
				false, props.href, props.ಗುರಿ, props.title,
				ಖಾಲಿ, ctx->bctx);
		ಆದರೆ (box == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಪಠ್ಯ);
			ಹಿಂತಿರುಗಿ false;
		}

		box->type = BOX_ಪಠ್ಯ;

		box->ಪಠ್ಯ = talloc_strdup(ctx->bctx, ಪಠ್ಯ);
		ಬಿಡುಗಡೆ(ಪಠ್ಯ);
		ಆದರೆ (box->ಪಠ್ಯ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(box->ಪಠ್ಯ);

		/* strip ending space char off */
		ಆದರೆ (box->ಉದ್ದಳತೆ > 1 && box->ಪಠ್ಯ[box->ಉದ್ದಳತೆ - 1] == ' ') {
			box->space = UNKNOWN_ಅಗಲ;
			box->ಉದ್ದಳತೆ--;
		}

		ಆದರೆ (css_computed_ಪಠ್ಯ_transform(props.ಪೋಷಕ_style) !=
				CSS_ಪಠ್ಯ_TRANSFORM_NONE)
			box_ಪಠ್ಯ_transform(box->ಪಠ್ಯ, box->ಉದ್ದಳತೆ,
				css_computed_ಪಠ್ಯ_transform(
					props.ಪೋಷಕ_style));

		box_ಕೂಡಿಸು_ಮಗು(props.inline_container, box);

		ಆದರೆ (box->ಪಠ್ಯ[0] == ' ') {
			box->ಉದ್ದಳತೆ--;

			memmove(box->ಪಠ್ಯ, &box->ಪಠ್ಯ[1], box->ಉದ್ದಳತೆ);

			ಆದರೆ (box->prev != ಖಾಲಿ)
				box->prev->space = UNKNOWN_ಅಗಲ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* white-space: pre */
		ಅಕ್ಷರ *ಪಠ್ಯ;
		size_t ಪಠ್ಯ_len = dom_string_byte_ಉದ್ದಳತೆ(content);
		size_t i;
		ಅಕ್ಷರ *current;
		ಎಣಿಕೆ css_white_space_e white_space =
				css_computed_white_space(props.ಪೋಷಕ_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ಪ್ರತಿಜ್ಞಾ(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_ಸಾಲು ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		ಪಠ್ಯ = ಸ್ಮರಣೆ_ಕೊಡು(ಪಠ್ಯ_len + 1);
		dom_string_unref(content);

		ಆದರೆ (ಪಠ್ಯ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		ಸ್ಮರಣೆ_ನಕಲು(ಪಠ್ಯ, dom_string_ದತ್ತಾಂಶ(content), ಪಠ್ಯ_len);
		ಪಠ್ಯ[ಪಠ್ಯ_len] = '\0';

		/* TODO: Handle tabs properly */
		ಗಾಗಿ (i = 0; i < ಪಠ್ಯ_len; i++)
			ಆದರೆ (ಪಠ್ಯ[i] == '\t')
				ಪಠ್ಯ[i] = ' ';

		ಆದರೆ (css_computed_ಪಠ್ಯ_transform(props.ಪೋಷಕ_style) !=
				CSS_ಪಠ್ಯ_TRANSFORM_NONE)
			box_ಪಠ್ಯ_transform(ಪಠ್ಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪಠ್ಯ),
				css_computed_ಪಠ್ಯ_transform(
						props.ಪೋಷಕ_style));

		current = ಪಠ್ಯ;

		/* swallow a single leading new line */
		ಆದರೆ (props.containing_ಖಂಡ->flags & PRE_STRIP) {
			ಆಯ್ಕೆ (*current) {
			ಸ್ಥಿತಿ '\n':
				current++;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\r':
				current++;
				ಆದರೆ (*current == '\n')
					current++;
				ನಿಲ್ಲಿಸು;
			}
			props.containing_ಖಂಡ->flags &= ~PRE_STRIP;
		}

		ಮಾಡು {
			size_t len = strcspn(current, "\r\n");

			ಅಕ್ಷರ old = current[len];

			current[len] = 0;

			ಆದರೆ (props.inline_container == ಖಾಲಿ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ಖಾಲಿ, ಖಾಲಿ,
						false, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
						ctx->bctx);
				ಆದರೆ (props.inline_container == ಖಾಲಿ) {
					ಬಿಡುಗಡೆ(ಪಠ್ಯ);
					ಹಿಂತಿರುಗಿ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ಖಾಲಿ,
				(css_computed_style *) props.ಪೋಷಕ_style,
				false, props.href, props.ಗುರಿ, props.title,
				ಖಾಲಿ, ctx->bctx);
			ಆದರೆ (box == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ಪಠ್ಯ);
				ಹಿಂತಿರುಗಿ false;
			}

			box->type = BOX_ಪಠ್ಯ;

			box->ಪಠ್ಯ = talloc_strdup(ctx->bctx, current);
			ಆದರೆ (box->ಪಠ್ಯ == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ಪಠ್ಯ);
				ಹಿಂತಿರುಗಿ false;
			}

			box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(box->ಪಠ್ಯ);

			box_ಕೂಡಿಸು_ಮಗು(props.inline_container, box);

			current[len] = old;

			current += len;

			ಆದರೆ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ಖಾಲಿ, ಖಾಲಿ,
						false, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
						ctx->bctx);
				ಆದರೆ (props.inline_container == ಖಾಲಿ) {
					ಬಿಡುಗಡೆ(ಪಠ್ಯ);
					ಹಿಂತಿರುಗಿ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_ಕೂಡಿಸು_ಮಗು(props.containing_ಖಂಡ,
						props.inline_container);

				ಆದರೆ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ಇಲ್ಲದಿದ್ದರೆ
					current++;
			}
		} ಆಗ (*current);

		ಬಿಡುಗಡೆ(ಪಠ್ಯ);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ಸ್ಥಿರ ಶೂನ್ಯ convert_xml_ಗಮ್ಯ_box(ರಚನೆ box_construct_ctx *ctx)
{
	dom_ಗಂಟು *ಮುಂದಿನ;
	bool convert_children;
	uint32_t ಅಂಕೆ_processed = 0;
	ಸ್ಥಿರಾಂಕ uint32_t max_processed_before_yield = 10;

	ಮಾಡು {
		convert_children = true;

		ಪ್ರತಿಜ್ಞಾ(ctx->n != ಖಾಲಿ);

		ಆದರೆ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ಗಂಟು_unref(ctx->n);
			ಬಿಡುಗಡೆ(ctx);
			ಹಿಂತಿರುಗಿ;
		}

		/* Find next element to process, converting text nodes as we go */
		ಮುಂದಿನ = ಮುಂದಿನ_ಗಂಟು(ctx->n, ctx->content, convert_children);
		ಆಗ (ಮುಂದಿನ != ಖಾಲಿ) {
			dom_ಗಂಟು_type type;
			dom_exception err;

			err = dom_ಗಂಟು_get_ಗಂಟು_type(ಮುಂದಿನ, &type);
			ಆದರೆ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ಗಂಟು_unref(ಮುಂದಿನ);
				ಬಿಡುಗಡೆ(ctx);
				ಹಿಂತಿರುಗಿ;
			}

			ಆದರೆ (type == DOM_ELEMENT_ಗಂಟು)
				ನಿಲ್ಲಿಸು;

			ಆದರೆ (type == DOM_ಪಠ್ಯ_ಗಂಟು) {
				ctx->n = ಮುಂದಿನ;
				ಆದರೆ (box_construct_ಪಠ್ಯ(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ಗಂಟು_unref(ctx->n);
					ಬಿಡುಗಡೆ(ctx);
					ಹಿಂತಿರುಗಿ;
				}
			}

			ಮುಂದಿನ = ಮುಂದಿನ_ಗಂಟು(ಮುಂದಿನ, ctx->content, true);
		}

		ctx->n = ಮುಂದಿನ;

		ಆದರೆ (ಮುಂದಿನ == ಖಾಲಿ) {
			/* Conversion complete */
			ರಚನೆ box ಬೇರು;

			memset(&ಬೇರು, 0, ಗಾತ್ರ(ಬೇರು));

			ಬೇರು.type = BOX_ಖಂಡ;
			ಬೇರು.children = ಬೇರು.ಕೊನೆಯ = ctx->ಬೇರು_box;
			ಬೇರು.children->ಪೋಷಕ = &ಬೇರು;

			/** \todo Remove box_normalise_block */
			ಆದರೆ (box_normalise_ಖಂಡ(&ಬೇರು, ctx->ಬೇರು_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ctx->content->layout = ಬೇರು.children;
				ctx->content->layout->ಪೋಷಕ = ಖಾಲಿ;

				ctx->cb(ctx->content, true);
			}

			ಪ್ರತಿಜ್ಞಾ(ctx->n == ಖಾಲಿ);

			ಬಿಡುಗಡೆ(ctx);
			ಹಿಂತಿರುಗಿ;
		}
	} ಆಗ (++ಅಂಕೆ_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(0, (ಶೂನ್ಯ *)convert_xml_ಗಮ್ಯ_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_ಗಮ್ಯ_box(dom_ಗಂಟು *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   ಶೂನ್ಯ **box_conversion_context)
{
	ರಚನೆ box_construct_ctx *ctx;

	ಪ್ರತಿಜ್ಞಾ(box_conversion_context != ಖಾಲಿ);

	ಆದರೆ (c->bctx == ಖಾಲಿ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, ಪೂರ್ಣಾಂಕ);
		ಆದರೆ (c->bctx == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ಗಂಟು_ref(n);
	ctx->ಬೇರು_box = ಖಾಲಿ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ಹಿಂತಿರುಗಿ guit->misc->ವೇಳಾಪಟ್ಟಿ(0, (ಶೂನ್ಯ *)convert_xml_ಗಮ್ಯ_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_ಗಮ್ಯ_box(ಶೂನ್ಯ *box_conversion_context)
{
	ರಚನೆ box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, (ಶೂನ್ಯ *)convert_xml_ಗಮ್ಯ_box, ctx);
	ಆದರೆ (err != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ err;
	}

	dom_ಗಂಟು_unref(ctx->n);
	ಬಿಡುಗಡೆ(ctx);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ರಚನೆ box *box_for_ಗಂಟು(dom_ಗಂಟು *n)
{
	ರಚನೆ box *box = ಖಾಲಿ;
	dom_exception err;

	err = dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ(n, corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_ದತ್ತಾಂಶ,
			(ಶೂನ್ಯ *) &box);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ box;
}

/* exported function documented in html/box_construct.h */
bool
box_ಹೊರತೆಗೆ_link(ಸ್ಥಿರಾಂಕ html_content *content,
		 ಸ್ಥಿರಾಂಕ dom_string *dsrel,
		 nsurl *ತಳ,
		 nsurl **ಫಲಿತಾಂಶ)
{
	ಅಕ್ಷರ *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, j, ಕೊನೆ;
	nserror error;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rel;

	rel = dom_string_ದತ್ತಾಂಶ(dsrel);

	s1 = s = ಸ್ಮರಣೆ_ಕೊಡು(3 * ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(rel) + 1);
	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ false;

	/* copy to s, removing white space and control characters */
	ಗಾಗಿ (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	ಗಾಗಿ (ಕೊನೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(rel);
	     (ಕೊನೆ != i) && ascii_is_space(rel[ಕೊನೆ - 1]);
	     ಕೊನೆ--)
		;
	ಗಾಗಿ (j = 0; i != ಕೊನೆ; i++) {
		ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) rel[i] < 0x20) {
			; /* skip control characters */
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ಇಲ್ಲದಿದ್ದರೆ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ಆದರೆ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ಆದರೆ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ಆದರೆ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ಆದರೆ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(ತಳ, s1, ಫಲಿತಾಂಶ);
	ಬಿಡುಗಡೆ(s);
	ಆದರೆ (error != NSERROR_OK) {
		*ಫಲಿತಾಂಶ = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}
