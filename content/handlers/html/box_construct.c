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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/select.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/object.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_manipulate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_construct.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_special.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_normalise.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"

/**
 * Context for box tree construction
 */
ਬਣਤਰ box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_ਗੰਢ *n;			/**< Current node to process */

	ਬਣਤਰ box *ਜੜ੍ਹ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	ਪੂਰਨ *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ਬਣਤਰ box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ਅਚਲ css_computed_style *ਮਾਪਾ_style;
	/** Current link target, or NULL if none */
	ਬਣਤਰ nsurl *href;
	/** Current frame target, or NULL if none */
	ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;
	/** Current title attribute, or NULL if none */
	ਅਚਲ ਅੱਖਰ *title;
	/** Identity of the current block-level container */
	ਬਣਤਰ box *containing_ਖੰਡ;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ਬਣਤਰ box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ਗੰਢ_is_ਜੜ੍ਹ;
};

ਸਥਿਰ ਅਚਲ content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ਸਥਿਰ ਅਚਲ box_type box_map[] = {
	BOX_ਖੰਡ,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_ਖੰਡ,           /* CSS_DISPLAY_BLOCK */
	BOX_ਖੰਡ,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_ਖੰਡ,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_ਸਾਰਣੀ,           /* CSS_DISPLAY_TABLE */
	BOX_ਸਾਰਣੀ,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_ਸਾਰਣੀ_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_ਖੰਡ,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_ਖੰਡ,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool box_is_ਜੜ੍ਹ(dom_ਗੰਢ *n)
{
	dom_ਗੰਢ *ਮਾਪਾ;
	dom_ਗੰਢ_type type;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(n, &ਮਾਪਾ);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (ਮਾਪਾ != ਨਿਰਾ) {
		err = dom_ਗੰਢ_get_ਗੰਢ_type(ਮਾਪਾ, &type);

		dom_ਗੰਢ_unref(ਮਾਪਾ);

		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		ਜੇ (type != DOM_DOCUMENT_ਗੰਢ)
			ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ਸਥਿਰ ਖਾਲੀ
box_ਕੱਢੋ_properties(dom_ਗੰਢ *n, ਬਣਤਰ box_construct_props *props)
{
	memset(props, 0, ਆਕਾਰ(*props));

	props->ਗੰਢ_is_ਜੜ੍ਹ = box_is_ਜੜ੍ਹ(n);

	/* Extract properties from containing DOM node */
	ਜੇ (props->ਗੰਢ_is_ਜੜ੍ਹ == false) {
		dom_ਗੰਢ *current_ਗੰਢ = n;
		dom_ਗੰਢ *ਮਾਪਾ_ਗੰਢ = ਨਿਰਾ;
		ਬਣਤਰ box *ਮਾਪਾ_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ਜਦੋਂ (true) {
			err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(current_ਗੰਢ,
					&ਮਾਪਾ_ਗੰਢ);
			ਜੇ (err != DOM_NO_ERR || ਮਾਪਾ_ਗੰਢ == ਨਿਰਾ)
				ਰੋਕੋ;

			ਮਾਪਾ_box = box_for_ਗੰਢ(ਮਾਪਾ_ਗੰਢ);

			ਜੇ (ਮਾਪਾ_box != ਨਿਰਾ) {
				props->ਮਾਪਾ_style = ਮਾਪਾ_box->style;
				props->href = ਮਾਪਾ_box->href;
				props->ਨਿਸ਼ਾਨਾ = ਮਾਪਾ_box->ਨਿਸ਼ਾਨਾ;
				props->title = ਮਾਪਾ_box->title;

				dom_ਗੰਢ_unref(ਮਾਪਾ_ਗੰਢ);
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (current_ਗੰਢ != n)
					dom_ਗੰਢ_unref(current_ਗੰਢ);
				current_ਗੰਢ = ਮਾਪਾ_ਗੰਢ;
				ਮਾਪਾ_ਗੰਢ = ਨਿਰਾ;
			}
		}

		/* Find containing block (may be parent) */
		ਜਦੋਂ (true) {
			ਬਣਤਰ box *b;

			err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(current_ਗੰਢ,
					&ਮਾਪਾ_ਗੰਢ);
			ਜੇ (err != DOM_NO_ERR || ਮਾਪਾ_ਗੰਢ == ਨਿਰਾ) {
				ਜੇ (current_ਗੰਢ != n)
					dom_ਗੰਢ_unref(current_ਗੰਢ);
				ਰੋਕੋ;
			}

			ਜੇ (current_ਗੰਢ != n)
				dom_ਗੰਢ_unref(current_ਗੰਢ);

			b = box_for_ਗੰਢ(ਮਾਪਾ_ਗੰਢ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ਜੇ (b != ਨਿਰਾ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_ਖੰਡ = b;

				dom_ਗੰਢ_unref(ਮਾਪਾ_ਗੰਢ);
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				current_ਗੰਢ = ਮਾਪਾ_ਗੰਢ;
				ਮਾਪਾ_ਗੰਢ = ਨਿਰਾ;
			}
		}
	}

	/* Compute current inline container, if any */
	ਜੇ (props->containing_ਖੰਡ != ਨਿਰਾ &&
			props->containing_ਖੰਡ->ਆਖਰੀ != ਨਿਰਾ &&
			props->containing_ਖੰਡ->ਆਖਰੀ->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_ਖੰਡ->ਆਖਰੀ;
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
ਸਥਿਰ css_select_results *
box_get_style(html_content *c,
	      ਅਚਲ css_computed_style *ਮਾਪਾ_style,
	      ਅਚਲ css_computed_style *ਜੜ੍ਹ_style,
	      dom_ਗੰਢ *n)
{
	dom_string *s = ਨਿਰਾ;
	css_stylesheet *inline_style = ਨਿਰਾ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ਜੇ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ਜੇ (err != DOM_NO_ERR) {
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	ਜੇ (s != ਨਿਰਾ) {
		inline_style = nscss_create_inline_style(
				(ਅਚਲ uint8_t *) dom_string_ਅੰਕੜੇ(s),
				dom_string_byte_ਲੰਮਾਈ(s),
				c->encoding,
				nsurl_access(c->ਅਧਾਰ_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_NONE);

		dom_string_unref(s);

		ਜੇ (inline_style == ਨਿਰਾ)
			ਵਾਪਸ ਨਿਰਾ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
	ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
	ctx.universal = c->universal;
	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ_style;
	ctx.ਮਾਪਾ_style = ਮਾਪਾ_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ਜੇ (inline_style != ਨਿਰਾ)
		css_stylesheet_destroy(inline_style);

	ਵਾਪਸ styles;
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
ਸਥਿਰ ਖਾਲੀ
box_construct_generate(dom_ਗੰਢ *n,
		       html_content *content,
		       ਬਣਤਰ box *box,
		       ਅਚਲ css_computed_style *style)
{
	ਬਣਤਰ box *gen = ਨਿਰਾ;
	ਗਿਣਤੀ css_display_e computed_display;
	ਅਚਲ css_computed_content_ਵਸਤੂ *c_ਵਸਤੂ;

	/* Nothing to generate if the parent box is not a block */
	ਜੇ (box->type != BOX_ਖੰਡ)
		ਵਾਪਸ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ਜੇ (style == ਨਿਰਾ ||
			css_computed_content(style, &c_ਵਸਤੂ) ==
			CSS_CONTENT_ਸਾਧਾਰਨ) {
		/* No pseudo element */
		ਵਾਪਸ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_ਜੜ੍ਹ(n));
	ਜੇ (computed_display == CSS_DISPLAY_ਖੰਡ ||
			computed_display == CSS_DISPLAY_ਸਾਰਣੀ) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ਨਿਰਾ, (css_computed_style *) style,
				false, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, content->bctx);
		ਜੇ (gen == ਨਿਰਾ) {
			ਵਾਪਸ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_ਜੜ੍ਹ(n))];

		box_ਜੋੜੋ_ਬੱਚਾ(box, gen);
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
ਸਥਿਰ bool
box_construct_marker(ਬਣਤਰ box *box,
		     ਅਚਲ ਅੱਖਰ *title,
		     ਬਣਤਰ box_construct_ctx *ctx,
		     ਬਣਤਰ box *ਮਾਪਾ)
{
	lwc_string *image_uri;
	ਬਣਤਰ box *marker;
	ਗਿਣਤੀ css_list_style_type_e list_style_type;

	marker = box_create(ਨਿਰਾ, box->style, false, ਨਿਰਾ, ਨਿਰਾ, title,
			ਨਿਰਾ, ctx->bctx);
	ਜੇ (marker == false)
		ਵਾਪਸ false;

	marker->type = BOX_ਖੰਡ;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	ਚੋਣ (list_style_type) {
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->ਪਾਠ = (ਅੱਖਰ *) "\342\200\242";
		marker->ਲੰਮਾਈ = 3;
		ਰੋਕੋ;

	ਹਾਲਤ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->ਪਾਠ = (ਅੱਖਰ *) "\342\227\213";
		marker->ਲੰਮਾਈ = 3;
		ਰੋਕੋ;

	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਵਰਗਫਲ:
		/* 25AA BLACK SMALL SQUARE */
		marker->ਪਾਠ = (ਅੱਖਰ *) "\342\226\252";
		marker->ਲੰਮਾਈ = 3;
		ਰੋਕੋ;

	ਮੂਲ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_NONE:
		marker->ਪਾਠ = ਨਿਰਾ;
		marker->ਲੰਮਾਈ = 0;
		ਰੋਕੋ;
	}

	ਜੇ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ਨਿਰਾ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ਅੰਕੜੇ(image_uri), &url);
		ਜੇ (error != NSERROR_OK)
			ਵਾਪਸ false;

		ਜੇ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ਵਾਪਸ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->ਮਾਪਾ = box;

	ਵਾਪਸ true;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool box__style_is_float(ਅਚਲ ਬਣਤਰ box *box)
{
	ਵਾਪਸ css_computed_float(box->style) == CSS_FLOAT_ਖੱਬਾ ||
	       css_computed_float(box->style) == CSS_FLOAT_ਸੱਜਾ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool box__is_flex(ਅਚਲ ਬਣਤਰ box *box)
{
	ਵਾਪਸ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool box_containing_ਖੰਡ_is_flex(
		ਅਚਲ ਬਣਤਰ box_construct_props *props)
{
	ਵਾਪਸ props->containing_ਖੰਡ != ਨਿਰਾ &&
	       box__is_flex(props->containing_ਖੰਡ);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ਸਥਿਰ bool
box_construct_element(ਬਣਤਰ box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ਨਿਰਾ;
	ਗਿਣਤੀ css_display_e css_display;
	ਬਣਤਰ box *box = ਨਿਰਾ, *old_box;
	css_select_results *styles = ਨਿਰਾ;
	lwc_string *bgimage_uri;
	dom_exception err;
	ਬਣਤਰ box_construct_props props;
	ਅਚਲ css_computed_style *ਜੜ੍ਹ_style = ਨਿਰਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(ctx->n != ਨਿਰਾ);

	box_ਕੱਢੋ_properties(ctx->n, &props);

	ਜੇ (props.containing_ਖੰਡ != ਨਿਰਾ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_ਖੰਡ->flags &= ~PRE_STRIP;
	}

	ਜੇ (props.ਗੰਢ_is_ਜੜ੍ਹ == false) {
		ਜੜ੍ਹ_style = ctx->ਜੜ੍ਹ_box->style;
	}

	styles = box_get_style(ctx->content, props.ਮਾਪਾ_style, ਜੜ੍ਹ_style,
			ctx->n);
	ਜੇ (styles == ਨਿਰਾ)
		ਵਾਪਸ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (title0 != ਨਿਰਾ) {
		ਅੱਖਰ *t = squash_whitespace(dom_string_ਅੰਕੜੇ(title0));

		dom_string_unref(title0);

		ਜੇ (t == ਨਿਰਾ)
			ਵਾਪਸ false;

		props.title = talloc_strdup(ctx->bctx, t);

		ਮੁਕਤ(t);

		ਜੇ (props.title == ਨਿਰਾ)
			ਵਾਪਸ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (s != ਨਿਰਾ) {
		err = dom_string_intern(s, &id);
		ਜੇ (err != DOM_NO_ERR)
			id = ਨਿਰਾ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.ਨਿਸ਼ਾਨਾ, props.title, id,
			ctx->bctx);
	ਜੇ (box == ਨਿਰਾ)
		ਵਾਪਸ false;

	/* If this is the root box, add it to the context */
	ਜੇ (props.ਗੰਢ_is_ਜੜ੍ਹ)
		ctx->ਜੜ੍ਹ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (s != ਨਿਰਾ) {
		ਅਚਲ ਅੱਖਰ *val = dom_string_ਅੰਕੜੇ(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ਜੇ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ਨਿਰਾ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (s != ਨਿਰਾ) {
		ਅਚਲ ਅੱਖਰ *val = dom_string_ਅੰਕੜੇ(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ਜੇ ('0' <= val[0] && val[0] <= '9')
			box->ਪੰਕਤੀਆਂ = clamp(strtol(val, ਨਿਰਾ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ਜੇ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_ਖੰਡ ||
			 css_display == CSS_DISPLAY_INLINE_ਸਾਰਣੀ ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_ਖੰਡ];
	} ਨਹੀਂਤਾਂ ਜੇ (props.ਗੰਢ_is_ਜੜ੍ਹ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_ਖੰਡ;
	} ਨਹੀਂਤਾਂ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ਗੰਢ_is_ਜੜ੍ਹ)];

		ਜੇ (props.containing_ਖੰਡ->type == BOX_FLEX ||
		    props.containing_ਖੰਡ->type == BOX_INLINE_FLEX) {
			/* Blockification */
			ਚੋਣ (box->type) {
			ਹਾਲਤ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ਰੋਕੋ;
			ਹਾਲਤ BOX_INLINE_ਖੰਡ:
				box->type = BOX_ਖੰਡ;
				ਰੋਕੋ;
			ਮੂਲ:
				ਰੋਕੋ;
			}
		}
	}

	ਜੇ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ਵਾਪਸ false;
	}

	/* Handle the :before pseudo element */
	ਜੇ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ਜੇ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ਗੰਢ_is_ਜੜ੍ਹ) == CSS_DISPLAY_NONE &&
			props.ਗੰਢ_is_ਜੜ੍ਹ == false)) {
		css_select_results_destroy(styles);
		box->styles = ਨਿਰਾ;
		box->style = ਨਿਰਾ;

		/* Invalidate associated gadget, if any */
		ਜੇ (box->gadget != ਨਿਰਾ) {
			box->gadget->box = ਨਿਰਾ;
			box->gadget = ਨਿਰਾ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ਵਾਪਸ true;
	}

	/* Attach DOM node to box */
	err = dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(ctx->n,
			corestring_dom_ns_ਕੁੰਜੀ_box_ਗੰਢ_ਅੰਕੜੇ, box, ਨਿਰਾ,
			(ਖਾਲੀ *) &old_box);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	/* Attach box to DOM node */
	box->ਗੰਢ = dom_ਗੰਢ_ref(ctx->n);

	ਜੇ (props.inline_container == ਨਿਰਾ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_ਖੰਡ ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box_containing_ਖੰਡ_is_flex(&props))) &&
			props.ਗੰਢ_is_ਜੜ੍ਹ == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ਪ੍ਰਤਿਜ੍ਞਾ(props.containing_ਖੰਡ != ਨਿਰਾ &&
				"Box must have containing block.");

		props.inline_container = box_create(ਨਿਰਾ, ਨਿਰਾ, false, ਨਿਰਾ,
				ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ctx->bctx);
		ਜੇ (props.inline_container == ਨਿਰਾ)
			ਵਾਪਸ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ਜੇ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ਨਿਰਾ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ਅੰਕੜੇ(bgimage_uri), &url);
		ਜੇ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ਜੇ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ਵਾਪਸ false;
			}
			nsurl_unref(url);
		}
	}

	ਜੇ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ਜੇ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_ਖੰਡ) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ਪ੍ਰਤਿਜ੍ਞਾ(props.inline_container != ਨਿਰਾ);

		box_ਜੋੜੋ_ਬੱਚਾ(props.inline_container, box);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (ns_computed_display(box->style, props.ਗੰਢ_is_ਜੜ੍ਹ) ==
				CSS_DISPLAY_LIST_ਵਸਤੂ) {
			/* List item: compute marker */
			ਜੇ (box_construct_marker(box, props.title, ctx,
					props.containing_ਖੰਡ) == false)
				ਵਾਪਸ false;
		}

		ਜੇ (props.ਗੰਢ_is_ਜੜ੍ਹ == false &&
				box_containing_ਖੰਡ_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ਖੱਬਾ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ਸੱਜਾ)) {
			/* Float: insert a float between the parent and box. */
			ਬਣਤਰ box *flt = box_create(ਨਿਰਾ, ਨਿਰਾ, false,
					props.href, props.ਨਿਸ਼ਾਨਾ, props.title,
					ਨਿਰਾ, ctx->bctx);
			ਜੇ (flt == ਨਿਰਾ)
				ਵਾਪਸ false;

			ਜੇ (css_computed_float(box->style) == CSS_FLOAT_ਖੱਬਾ)
				flt->type = BOX_FLOAT_ਖੱਬਾ;
			ਨਹੀਂਤਾਂ
				flt->type = BOX_FLOAT_ਸੱਜਾ;

			box_ਜੋੜੋ_ਬੱਚਾ(props.inline_container, flt);
			box_ਜੋੜੋ_ਬੱਚਾ(flt, box);
		} ਨਹੀਂਤਾਂ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ਜੇ (props.containing_ਖੰਡ != ਨਿਰਾ)
				box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ, box);
		}
	}

	ਵਾਪਸ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ਸਥਿਰ ਖਾਲੀ box_construct_element_after(dom_ਗੰਢ *n, html_content *content)
{
	ਬਣਤਰ box_construct_props props;
	ਬਣਤਰ box *box = box_for_ਗੰਢ(n);

	ਪ੍ਰਤਿਜ੍ਞਾ(box != ਨਿਰਾ);

	box_ਕੱਢੋ_properties(n, &props);

	ਜੇ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ਬਣਤਰ box *inline_ਸਿਰਾ;
		bool has_children;
		dom_exception err;

		err = dom_ਗੰਢ_has_ਬੱਚਾ_nodes(n, &has_children);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ;

		ਜੇ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ਵਾਪਸ;
		}

		ਜੇ (props.inline_container == ਨਿਰਾ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ਨਿਰਾ, ਨਿਰਾ, false,
					ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, content->bctx);
			ਜੇ (props.inline_container == ਨਿਰਾ)
				ਵਾਪਸ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ,
					props.inline_container);
		}

		inline_ਸਿਰਾ = box_create(ਨਿਰਾ, box->style, false,
				box->href, box->ਨਿਸ਼ਾਨਾ, box->title,
				box->id == ਨਿਰਾ ? ਨਿਰਾ :
				lwc_string_ref(box->id), content->bctx);
		ਜੇ (inline_ਸਿਰਾ != ਨਿਰਾ) {
			inline_ਸਿਰਾ->type = BOX_INLINE_ਸਿਰਾ;

			ਪ੍ਰਤਿਜ੍ਞਾ(props.inline_container != ਨਿਰਾ);

			box_ਜੋੜੋ_ਬੱਚਾ(props.inline_container, inline_ਸਿਰਾ);

			box->inline_ਸਿਰਾ = inline_ਸਿਰਾ;
			inline_ਸਿਰਾ->inline_ਸਿਰਾ = box;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (!(box->flags & IS_REPLACED)) {
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
ਸਥਿਰ dom_ਗੰਢ *
ਅਗਲਾ_ਗੰਢ(dom_ਗੰਢ *n, html_content *content, bool convert_children)
{
	dom_ਗੰਢ *ਅਗਲਾ = ਨਿਰਾ;
	bool has_children;
	dom_exception err;

	err = dom_ਗੰਢ_has_ਬੱਚਾ_nodes(n, &has_children);
	ਜੇ (err != DOM_NO_ERR) {
		dom_ਗੰਢ_unref(n);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (convert_children && has_children) {
		err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(n, &ਅਗਲਾ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ ਨਿਰਾ;
		}
		dom_ਗੰਢ_unref(n);
	} ਨਹੀਂਤਾਂ {
		err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(n, &ਅਗਲਾ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ ਨਿਰਾ;
		}

		ਜੇ (ਅਗਲਾ != ਨਿਰਾ) {
			ਜੇ (box_for_ਗੰਢ(n) != ਨਿਰਾ)
				box_construct_element_after(n, content);
			dom_ਗੰਢ_unref(n);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (box_for_ਗੰਢ(n) != ਨਿਰਾ)
				box_construct_element_after(n, content);

			ਜਦੋਂ (box_is_ਜੜ੍ਹ(n) == false) {
				dom_ਗੰਢ *ਮਾਪਾ = ਨਿਰਾ;
				dom_ਗੰਢ *ਮਾਪਾ_ਅਗਲਾ = ਨਿਰਾ;

				err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(n, &ਮਾਪਾ);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(n);
					ਵਾਪਸ ਨਿਰਾ;
				}

				ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ != ਨਿਰਾ);

				err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(ਮਾਪਾ,
						&ਮਾਪਾ_ਅਗਲਾ);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(ਮਾਪਾ);
					dom_ਗੰਢ_unref(n);
					ਵਾਪਸ ਨਿਰਾ;
				}

				ਜੇ (ਮਾਪਾ_ਅਗਲਾ != ਨਿਰਾ) {
					dom_ਗੰਢ_unref(ਮਾਪਾ_ਅਗਲਾ);
					dom_ਗੰਢ_unref(ਮਾਪਾ);
					ਰੋਕੋ;
				}

				dom_ਗੰਢ_unref(n);
				n = ਮਾਪਾ;
				ਮਾਪਾ = ਨਿਰਾ;

				ਜੇ (box_for_ਗੰਢ(n) != ਨਿਰਾ) {
					box_construct_element_after(
							n, content);
				}
			}

			ਜੇ (box_is_ਜੜ੍ਹ(n) == false) {
				dom_ਗੰਢ *ਮਾਪਾ = ਨਿਰਾ;

				err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(n, &ਮਾਪਾ);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(n);
					ਵਾਪਸ ਨਿਰਾ;
				}

				ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ != ਨਿਰਾ);

				err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(ਮਾਪਾ, &ਅਗਲਾ);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(ਮਾਪਾ);
					dom_ਗੰਢ_unref(n);
					ਵਾਪਸ ਨਿਰਾ;
				}

				ਜੇ (box_for_ਗੰਢ(ਮਾਪਾ) != ਨਿਰਾ) {
					box_construct_element_after(ਮਾਪਾ,
							content);
				}

				dom_ਗੰਢ_unref(ਮਾਪਾ);
			}

			dom_ਗੰਢ_unref(n);
		}
	}

	ਵਾਪਸ ਅਗਲਾ;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ਸਥਿਰ ਖਾਲੀ
box_ਪਾਠ_transform(ਅੱਖਰ *s, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len, ਗਿਣਤੀ css_ਪਾਠ_transform_e tt)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਜੇ (len == 0)
		ਵਾਪਸ;
	ਚੋਣ (tt) {
		ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_UPPERCASE:
			ਲਈ (i = 0; i < len; ++i)
				ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) s[i] < 0x80)
					s[i] = ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(s[i]);
			ਰੋਕੋ;
		ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_LOWERCASE:
			ਲਈ (i = 0; i < len; ++i)
				ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) s[i] < 0x80)
					s[i] = ascii_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(s[i]);
			ਰੋਕੋ;
		ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_CAPITALIZE:
			ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) s[0] < 0x80)
				s[0] = ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(s[0]);
			ਲਈ (i = 1; i < len; ++i)
				ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(s[i]);
			ਰੋਕੋ;
		ਮੂਲ:
			ਰੋਕੋ;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ਸਥਿਰ bool box_construct_ਪਾਠ(ਬਣਤਰ box_construct_ctx *ctx)
{
	ਬਣਤਰ box_construct_props props;
	ਬਣਤਰ box *box = ਨਿਰਾ;
	dom_string *content;
	dom_exception err;

	ਪ੍ਰਤਿਜ੍ਞਾ(ctx->n != ਨਿਰਾ);

	box_ਕੱਢੋ_properties(ctx->n, &props);

	ਪ੍ਰਤਿਜ੍ਞਾ(props.containing_ਖੰਡ != ਨਿਰਾ);

	err = dom_characterdata_get_ਅੰਕੜੇ(ctx->n, &content);
	ਜੇ (err != DOM_NO_ERR || content == ਨਿਰਾ)
		ਵਾਪਸ false;

	ਜੇ (css_computed_white_space(props.ਮਾਪਾ_style) ==
			CSS_WHITE_SPACE_ਸਾਧਾਰਨ ||
			css_computed_white_space(props.ਮਾਪਾ_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		ਅੱਖਰ *ਪਾਠ;

		ਪਾਠ = squash_whitespace(dom_string_ਅੰਕੜੇ(content));

		dom_string_unref(content);

		ਜੇ (ਪਾਠ == ਨਿਰਾ)
			ਵਾਪਸ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ਜੇ (ਪਾਠ[0] == ' ' && ਪਾਠ[1] == 0) {
			ਜੇ (props.inline_container != ਨਿਰਾ) {
				ਪ੍ਰਤਿਜ੍ਞਾ(props.inline_container->ਆਖਰੀ != ਨਿਰਾ);

				props.inline_container->ਆਖਰੀ->space =
						UNKNOWN_ਚੌੜਾਈ;
			}

			ਮੁਕਤ(ਪਾਠ);

			ਵਾਪਸ true;
		}

		ਜੇ (props.inline_container == ਨਿਰਾ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ਨਿਰਾ, ਨਿਰਾ, false,
					ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ctx->bctx);
			ਜੇ (props.inline_container == ਨਿਰਾ) {
				ਮੁਕਤ(ਪਾਠ);
				ਵਾਪਸ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ਨਿਰਾ,
				(css_computed_style *) props.ਮਾਪਾ_style,
				false, props.href, props.ਨਿਸ਼ਾਨਾ, props.title,
				ਨਿਰਾ, ctx->bctx);
		ਜੇ (box == ਨਿਰਾ) {
			ਮੁਕਤ(ਪਾਠ);
			ਵਾਪਸ false;
		}

		box->type = BOX_ਪਾਠ;

		box->ਪਾਠ = talloc_strdup(ctx->bctx, ਪਾਠ);
		ਮੁਕਤ(ਪਾਠ);
		ਜੇ (box->ਪਾਠ == ਨਿਰਾ)
			ਵਾਪਸ false;

		box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(box->ਪਾਠ);

		/* strip ending space char off */
		ਜੇ (box->ਲੰਮਾਈ > 1 && box->ਪਾਠ[box->ਲੰਮਾਈ - 1] == ' ') {
			box->space = UNKNOWN_ਚੌੜਾਈ;
			box->ਲੰਮਾਈ--;
		}

		ਜੇ (css_computed_ਪਾਠ_transform(props.ਮਾਪਾ_style) !=
				CSS_ਪਾਠ_TRANSFORM_NONE)
			box_ਪਾਠ_transform(box->ਪਾਠ, box->ਲੰਮਾਈ,
				css_computed_ਪਾਠ_transform(
					props.ਮਾਪਾ_style));

		box_ਜੋੜੋ_ਬੱਚਾ(props.inline_container, box);

		ਜੇ (box->ਪਾਠ[0] == ' ') {
			box->ਲੰਮਾਈ--;

			memmove(box->ਪਾਠ, &box->ਪਾਠ[1], box->ਲੰਮਾਈ);

			ਜੇ (box->prev != ਨਿਰਾ)
				box->prev->space = UNKNOWN_ਚੌੜਾਈ;
		}
	} ਨਹੀਂਤਾਂ {
		/* white-space: pre */
		ਅੱਖਰ *ਪਾਠ;
		size_t ਪਾਠ_len = dom_string_byte_ਲੰਮਾਈ(content);
		size_t i;
		ਅੱਖਰ *current;
		ਗਿਣਤੀ css_white_space_e white_space =
				css_computed_white_space(props.ਮਾਪਾ_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ਪ੍ਰਤਿਜ੍ਞਾ(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_ਸਤਰ ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		ਪਾਠ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਪਾਠ_len + 1);
		dom_string_unref(content);

		ਜੇ (ਪਾਠ == ਨਿਰਾ)
			ਵਾਪਸ false;

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਪਾਠ, dom_string_ਅੰਕੜੇ(content), ਪਾਠ_len);
		ਪਾਠ[ਪਾਠ_len] = '\0';

		/* TODO: Handle tabs properly */
		ਲਈ (i = 0; i < ਪਾਠ_len; i++)
			ਜੇ (ਪਾਠ[i] == '\t')
				ਪਾਠ[i] = ' ';

		ਜੇ (css_computed_ਪਾਠ_transform(props.ਮਾਪਾ_style) !=
				CSS_ਪਾਠ_TRANSFORM_NONE)
			box_ਪਾਠ_transform(ਪਾਠ, ਲੜੀ_ਲੰਬਾਈ(ਪਾਠ),
				css_computed_ਪਾਠ_transform(
						props.ਮਾਪਾ_style));

		current = ਪਾਠ;

		/* swallow a single leading new line */
		ਜੇ (props.containing_ਖੰਡ->flags & PRE_STRIP) {
			ਚੋਣ (*current) {
			ਹਾਲਤ '\n':
				current++;
				ਰੋਕੋ;
			ਹਾਲਤ '\r':
				current++;
				ਜੇ (*current == '\n')
					current++;
				ਰੋਕੋ;
			}
			props.containing_ਖੰਡ->flags &= ~PRE_STRIP;
		}

		ਕਰੋ {
			size_t len = strcspn(current, "\r\n");

			ਅੱਖਰ old = current[len];

			current[len] = 0;

			ਜੇ (props.inline_container == ਨਿਰਾ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ਨਿਰਾ, ਨਿਰਾ,
						false, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
						ctx->bctx);
				ਜੇ (props.inline_container == ਨਿਰਾ) {
					ਮੁਕਤ(ਪਾਠ);
					ਵਾਪਸ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ਨਿਰਾ,
				(css_computed_style *) props.ਮਾਪਾ_style,
				false, props.href, props.ਨਿਸ਼ਾਨਾ, props.title,
				ਨਿਰਾ, ctx->bctx);
			ਜੇ (box == ਨਿਰਾ) {
				ਮੁਕਤ(ਪਾਠ);
				ਵਾਪਸ false;
			}

			box->type = BOX_ਪਾਠ;

			box->ਪਾਠ = talloc_strdup(ctx->bctx, current);
			ਜੇ (box->ਪਾਠ == ਨਿਰਾ) {
				ਮੁਕਤ(ਪਾਠ);
				ਵਾਪਸ false;
			}

			box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(box->ਪਾਠ);

			box_ਜੋੜੋ_ਬੱਚਾ(props.inline_container, box);

			current[len] = old;

			current += len;

			ਜੇ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ਨਿਰਾ, ਨਿਰਾ,
						false, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
						ctx->bctx);
				ਜੇ (props.inline_container == ਨਿਰਾ) {
					ਮੁਕਤ(ਪਾਠ);
					ਵਾਪਸ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_ਜੋੜੋ_ਬੱਚਾ(props.containing_ਖੰਡ,
						props.inline_container);

				ਜੇ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ਨਹੀਂਤਾਂ
					current++;
			}
		} ਜਦੋਂ (*current);

		ਮੁਕਤ(ਪਾਠ);
	}

	ਵਾਪਸ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ਸਥਿਰ ਖਾਲੀ convert_xml_ਮੰਜ਼ਿਲ_box(ਬਣਤਰ box_construct_ctx *ctx)
{
	dom_ਗੰਢ *ਅਗਲਾ;
	bool convert_children;
	uint32_t ਅੰਕ_processed = 0;
	ਅਚਲ uint32_t max_processed_before_yield = 10;

	ਕਰੋ {
		convert_children = true;

		ਪ੍ਰਤਿਜ੍ਞਾ(ctx->n != ਨਿਰਾ);

		ਜੇ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ਗੰਢ_unref(ctx->n);
			ਮੁਕਤ(ctx);
			ਵਾਪਸ;
		}

		/* Find next element to process, converting text nodes as we go */
		ਅਗਲਾ = ਅਗਲਾ_ਗੰਢ(ctx->n, ctx->content, convert_children);
		ਜਦੋਂ (ਅਗਲਾ != ਨਿਰਾ) {
			dom_ਗੰਢ_type type;
			dom_exception err;

			err = dom_ਗੰਢ_get_ਗੰਢ_type(ਅਗਲਾ, &type);
			ਜੇ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ਗੰਢ_unref(ਅਗਲਾ);
				ਮੁਕਤ(ctx);
				ਵਾਪਸ;
			}

			ਜੇ (type == DOM_ELEMENT_ਗੰਢ)
				ਰੋਕੋ;

			ਜੇ (type == DOM_ਪਾਠ_ਗੰਢ) {
				ctx->n = ਅਗਲਾ;
				ਜੇ (box_construct_ਪਾਠ(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ਗੰਢ_unref(ctx->n);
					ਮੁਕਤ(ctx);
					ਵਾਪਸ;
				}
			}

			ਅਗਲਾ = ਅਗਲਾ_ਗੰਢ(ਅਗਲਾ, ctx->content, true);
		}

		ctx->n = ਅਗਲਾ;

		ਜੇ (ਅਗਲਾ == ਨਿਰਾ) {
			/* Conversion complete */
			ਬਣਤਰ box ਜੜ੍ਹ;

			memset(&ਜੜ੍ਹ, 0, ਆਕਾਰ(ਜੜ੍ਹ));

			ਜੜ੍ਹ.type = BOX_ਖੰਡ;
			ਜੜ੍ਹ.children = ਜੜ੍ਹ.ਆਖਰੀ = ctx->ਜੜ੍ਹ_box;
			ਜੜ੍ਹ.children->ਮਾਪਾ = &ਜੜ੍ਹ;

			/** \todo Remove box_normalise_block */
			ਜੇ (box_normalise_ਖੰਡ(&ਜੜ੍ਹ, ctx->ਜੜ੍ਹ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ਨਹੀਂਤਾਂ {
				ctx->content->layout = ਜੜ੍ਹ.children;
				ctx->content->layout->ਮਾਪਾ = ਨਿਰਾ;

				ctx->cb(ctx->content, true);
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(ctx->n == ਨਿਰਾ);

			ਮੁਕਤ(ctx);
			ਵਾਪਸ;
		}
	} ਜਦੋਂ (++ਅੰਕ_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(0, (ਖਾਲੀ *)convert_xml_ਮੰਜ਼ਿਲ_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_ਮੰਜ਼ਿਲ_box(dom_ਗੰਢ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   ਖਾਲੀ **box_conversion_context)
{
	ਬਣਤਰ box_construct_ctx *ctx;

	ਪ੍ਰਤਿਜ੍ਞਾ(box_conversion_context != ਨਿਰਾ);

	ਜੇ (c->bctx == ਨਿਰਾ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, ਪੂਰਨ);
		ਜੇ (c->bctx == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ਗੰਢ_ref(n);
	ctx->ਜੜ੍ਹ_box = ਨਿਰਾ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ਵਾਪਸ guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(0, (ਖਾਲੀ *)convert_xml_ਮੰਜ਼ਿਲ_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_ਮੰਜ਼ਿਲ_box(ਖਾਲੀ *box_conversion_context)
{
	ਬਣਤਰ box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, (ਖਾਲੀ *)convert_xml_ਮੰਜ਼ਿਲ_box, ctx);
	ਜੇ (err != NSERROR_OK) {
		ਵਾਪਸ err;
	}

	dom_ਗੰਢ_unref(ctx->n);
	ਮੁਕਤ(ctx);

	ਵਾਪਸ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ਬਣਤਰ box *box_for_ਗੰਢ(dom_ਗੰਢ *n)
{
	ਬਣਤਰ box *box = ਨਿਰਾ;
	dom_exception err;

	err = dom_ਗੰਢ_get_user_ਅੰਕੜੇ(n, corestring_dom_ns_ਕੁੰਜੀ_box_ਗੰਢ_ਅੰਕੜੇ,
			(ਖਾਲੀ *) &box);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ box;
}

/* exported function documented in html/box_construct.h */
bool
box_ਕੱਢੋ_link(ਅਚਲ html_content *content,
		 ਅਚਲ dom_string *dsrel,
		 nsurl *ਅਧਾਰ,
		 nsurl **ਨਤੀਜਾ)
{
	ਅੱਖਰ *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, j, ਸਿਰਾ;
	nserror error;
	ਅਚਲ ਅੱਖਰ *rel;

	rel = dom_string_ਅੰਕੜੇ(dsrel);

	s1 = s = ਸਮ੍ਰਿਤੀ_ਦਿਓ(3 * ਲੜੀ_ਲੰਬਾਈ(rel) + 1);
	ਜੇ (!s)
		ਵਾਪਸ false;

	/* copy to s, removing white space and control characters */
	ਲਈ (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	ਲਈ (ਸਿਰਾ = ਲੜੀ_ਲੰਬਾਈ(rel);
	     (ਸਿਰਾ != i) && ascii_is_space(rel[ਸਿਰਾ - 1]);
	     ਸਿਰਾ--)
		;
	ਲਈ (j = 0; i != ਸਿਰਾ; i++) {
		ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) rel[i] < 0x20) {
			; /* skip control characters */
		} ਨਹੀਂਤਾਂ ਜੇ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ਨਹੀਂਤਾਂ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ਜੇ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ਜੇ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ਜੇ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ਜੇ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ਨਹੀਂਤਾਂ ਜੇ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(ਅਧਾਰ, s1, ਨਤੀਜਾ);
	ਮੁਕਤ(s);
	ਜੇ (error != NSERROR_OK) {
		*ਨਤੀਜਾ = ਨਿਰਾ;
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}
