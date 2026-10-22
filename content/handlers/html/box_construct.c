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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/string.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "css/select.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/object.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_manipulate.h"
#അന്തര്ഭാവ "html/box_construct.h"
#അന്തര്ഭാവ "html/box_special.h"
#അന്തര്ഭാവ "html/box_normalise.h"
#അന്തര്ഭാവ "html/form_internal.h"

/**
 * Context for box tree construction
 */
ഘടന box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_കണ്ണി *n;			/**< Current node to process */

	ഘടന box *വേര്_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	പൂർണ്ണസംഖ്യ *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ഘടന box_construct_props {
	/** Style from which to inherit, or NULL if none */
	സ്ഥിരം css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	ഘടന nsurl *href;
	/** Current frame target, or NULL if none */
	സ്ഥിരം അക്ഷരം *target;
	/** Current title attribute, or NULL if none */
	സ്ഥിരം അക്ഷരം *title;
	/** Identity of the current block-level container */
	ഘടന box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ഘടന box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool കണ്ണി_is_വേര്;
};

സ്ഥിതം സ്ഥിരം content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
സ്ഥിതം സ്ഥിരം box_type box_map[] = {
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
സ്ഥിതം വരിയിൽ bool box_is_വേര്(dom_കണ്ണി *n)
{
	dom_കണ്ണി *parent;
	dom_കണ്ണി_type type;
	dom_exception err;

	err = dom_കണ്ണി_get_parent_കണ്ണി(n, &parent);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (parent != ശൂന്യ) {
		err = dom_കണ്ണി_get_കണ്ണി_type(parent, &type);

		dom_കണ്ണി_unref(parent);

		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		എങ്കിൽ (type != DOM_DOCUMENT_കണ്ണി)
			തിരികെ false;
	}

	തിരികെ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
സ്ഥിതം ശൂന്യം
box_extract_properties(dom_കണ്ണി *n, ഘടന box_construct_props *props)
{
	memset(props, 0, വലുപ്പം(*props));

	props->കണ്ണി_is_വേര് = box_is_വേര്(n);

	/* Extract properties from containing DOM node */
	എങ്കിൽ (props->കണ്ണി_is_വേര് == false) {
		dom_കണ്ണി *current_കണ്ണി = n;
		dom_കണ്ണി *parent_കണ്ണി = ശൂന്യ;
		ഘടന box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ആകുമ്പോൾ (true) {
			err = dom_കണ്ണി_get_parent_കണ്ണി(current_കണ്ണി,
					&parent_കണ്ണി);
			എങ്കിൽ (err != DOM_NO_ERR || parent_കണ്ണി == ശൂന്യ)
				നിർത്തുക;

			parent_box = box_for_കണ്ണി(parent_കണ്ണി);

			എങ്കിൽ (parent_box != ശൂന്യ) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_കണ്ണി_unref(parent_കണ്ണി);
				നിർത്തുക;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (current_കണ്ണി != n)
					dom_കണ്ണി_unref(current_കണ്ണി);
				current_കണ്ണി = parent_കണ്ണി;
				parent_കണ്ണി = ശൂന്യ;
			}
		}

		/* Find containing block (may be parent) */
		ആകുമ്പോൾ (true) {
			ഘടന box *b;

			err = dom_കണ്ണി_get_parent_കണ്ണി(current_കണ്ണി,
					&parent_കണ്ണി);
			എങ്കിൽ (err != DOM_NO_ERR || parent_കണ്ണി == ശൂന്യ) {
				എങ്കിൽ (current_കണ്ണി != n)
					dom_കണ്ണി_unref(current_കണ്ണി);
				നിർത്തുക;
			}

			എങ്കിൽ (current_കണ്ണി != n)
				dom_കണ്ണി_unref(current_കണ്ണി);

			b = box_for_കണ്ണി(parent_കണ്ണി);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			എങ്കിൽ (b != ശൂന്യ && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_കണ്ണി_unref(parent_കണ്ണി);
				നിർത്തുക;
			} അല്ലെങ്കിൽ {
				current_കണ്ണി = parent_കണ്ണി;
				parent_കണ്ണി = ശൂന്യ;
			}
		}
	}

	/* Compute current inline container, if any */
	എങ്കിൽ (props->containing_block != ശൂന്യ &&
			props->containing_block->last != ശൂന്യ &&
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
സ്ഥിതം css_select_results *
box_get_style(html_content *c,
	      സ്ഥിരം css_computed_style *parent_style,
	      സ്ഥിരം css_computed_style *വേര്_style,
	      dom_കണ്ണി *n)
{
	dom_string *s = ശൂന്യ;
	css_stylesheet *inline_style = ശൂന്യ;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	എങ്കിൽ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		എങ്കിൽ (err != DOM_NO_ERR) {
			തിരികെ ശൂന്യ;
		}
	}

	എങ്കിൽ (s != ശൂന്യ) {
		inline_style = nscss_create_inline_style(
				(സ്ഥിരം uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		എങ്കിൽ (inline_style == ശൂന്യ)
			തിരികെ ശൂന്യ;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.വേര്_style = വേര്_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	എങ്കിൽ (inline_style != ശൂന്യ)
		css_stylesheet_destroy(inline_style);

	തിരികെ styles;
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
സ്ഥിതം ശൂന്യം
box_construct_generate(dom_കണ്ണി *n,
		       html_content *content,
		       ഘടന box *box,
		       സ്ഥിരം css_computed_style *style)
{
	ഘടന box *gen = ശൂന്യ;
	എണ്ണൽ css_display_e computed_display;
	സ്ഥിരം css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	എങ്കിൽ (box->type != BOX_BLOCK)
		തിരികെ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	എങ്കിൽ (style == ശൂന്യ ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		തിരികെ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_വേര്(n));
	എങ്കിൽ (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ശൂന്യ, (css_computed_style *) style,
				false, ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ, content->bctx);
		എങ്കിൽ (gen == ശൂന്യ) {
			തിരികെ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_വേര്(n))];

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
സ്ഥിതം bool
box_construct_marker(ഘടന box *box,
		     സ്ഥിരം അക്ഷരം *title,
		     ഘടന box_construct_ctx *ctx,
		     ഘടന box *parent)
{
	lwc_string *image_uri;
	ഘടന box *marker;
	എണ്ണൽ css_list_style_type_e list_style_type;

	marker = box_create(ശൂന്യ, box->style, false, ശൂന്യ, ശൂന്യ, title,
			ശൂന്യ, ctx->bctx);
	എങ്കിൽ (marker == false)
		തിരികെ false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	തിരഞ്ഞെടുപ്പ് (list_style_type) {
	അവസ്ഥ CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (അക്ഷരം *) "\342\200\242";
		marker->length = 3;
		നിർത്തുക;

	അവസ്ഥ CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (അക്ഷരം *) "\342\227\213";
		marker->length = 3;
		നിർത്തുക;

	അവസ്ഥ CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (അക്ഷരം *) "\342\226\252";
		marker->length = 3;
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	അവസ്ഥ CSS_LIST_STYLE_TYPE_NONE:
		marker->text = ശൂന്യ;
		marker->length = 0;
		നിർത്തുക;
	}

	എങ്കിൽ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ശൂന്യ) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		എങ്കിൽ (error != NSERROR_OK)
			തിരികെ false;

		എങ്കിൽ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			തിരികെ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	തിരികെ true;
}

സ്ഥിതം വരിയിൽ bool box__style_is_float(സ്ഥിരം ഘടന box *box)
{
	തിരികെ css_computed_float(box->style) == CSS_FLOAT_ഇടത് ||
	       css_computed_float(box->style) == CSS_FLOAT_വലത്;
}

സ്ഥിതം വരിയിൽ bool box__is_flex(സ്ഥിരം ഘടന box *box)
{
	തിരികെ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

സ്ഥിതം വരിയിൽ bool box__containing_block_is_flex(
		സ്ഥിരം ഘടന box_construct_props *props)
{
	തിരികെ props->containing_block != ശൂന്യ &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
സ്ഥിതം bool
box_construct_element(ഘടന box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ശൂന്യ;
	എണ്ണൽ css_display_e css_display;
	ഘടന box *box = ശൂന്യ, *old_box;
	css_select_results *styles = ശൂന്യ;
	lwc_string *bgimage_uri;
	dom_exception err;
	ഘടന box_construct_props props;
	സ്ഥിരം css_computed_style *വേര്_style = ശൂന്യ;

	പ്രതിജ്ഞാ(ctx->n != ശൂന്യ);

	box_extract_properties(ctx->n, &props);

	എങ്കിൽ (props.containing_block != ശൂന്യ) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	എങ്കിൽ (props.കണ്ണി_is_വേര് == false) {
		വേര്_style = ctx->വേര്_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, വേര്_style,
			ctx->n);
	എങ്കിൽ (styles == ശൂന്യ)
		തിരികെ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (title0 != ശൂന്യ) {
		അക്ഷരം *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		എങ്കിൽ (t == ശൂന്യ)
			തിരികെ false;

		props.title = talloc_strdup(ctx->bctx, t);

		സ്വതന്ത്രം(t);

		എങ്കിൽ (props.title == ശൂന്യ)
			തിരികെ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (s != ശൂന്യ) {
		err = dom_string_intern(s, &id);
		എങ്കിൽ (err != DOM_NO_ERR)
			id = ശൂന്യ;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	എങ്കിൽ (box == ശൂന്യ)
		തിരികെ false;

	/* If this is the root box, add it to the context */
	എങ്കിൽ (props.കണ്ണി_is_വേര്)
		ctx->വേര്_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (s != ശൂന്യ) {
		സ്ഥിരം അക്ഷരം *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		എങ്കിൽ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ശൂന്യ, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (s != ശൂന്യ) {
		സ്ഥിരം അക്ഷരം *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		എങ്കിൽ ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, ശൂന്യ, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	എങ്കിൽ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} അല്ലെങ്കിൽ എങ്കിൽ (props.കണ്ണി_is_വേര്) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} അല്ലെങ്കിൽ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.കണ്ണി_is_വേര്)];

		എങ്കിൽ (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			തിരഞ്ഞെടുപ്പ് (box->type) {
			അവസ്ഥ BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				നിർത്തുക;
			അവസ്ഥ BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
			}
		}
	}

	എങ്കിൽ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		തിരികെ false;
	}

	/* Handle the :before pseudo element */
	എങ്കിൽ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	എങ്കിൽ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.കണ്ണി_is_വേര്) == CSS_DISPLAY_NONE &&
			props.കണ്ണി_is_വേര് == false)) {
		css_select_results_destroy(styles);
		box->styles = ശൂന്യ;
		box->style = ശൂന്യ;

		/* Invalidate associated gadget, if any */
		എങ്കിൽ (box->gadget != ശൂന്യ) {
			box->gadget->box = ശൂന്യ;
			box->gadget = ശൂന്യ;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		തിരികെ true;
	}

	/* Attach DOM node to box */
	err = dom_കണ്ണി_set_user_data(ctx->n,
			corestring_dom_ns_താക്കോൽ_box_കണ്ണി_data, box, ശൂന്യ,
			(ശൂന്യം *) &old_box);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	/* Attach box to DOM node */
	box->കണ്ണി = dom_കണ്ണി_ref(ctx->n);

	എങ്കിൽ (props.inline_container == ശൂന്യ &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.കണ്ണി_is_വേര് == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		പ്രതിജ്ഞാ(props.containing_block != ശൂന്യ &&
				"Box must have containing block.");

		props.inline_container = box_create(ശൂന്യ, ശൂന്യ, false, ശൂന്യ,
				ശൂന്യ, ശൂന്യ, ശൂന്യ, ctx->bctx);
		എങ്കിൽ (props.inline_container == ശൂന്യ)
			തിരികെ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	എങ്കിൽ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ശൂന്യ &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		എങ്കിൽ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			എങ്കിൽ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				തിരികെ false;
			}
			nsurl_unref(url);
		}
	}

	എങ്കിൽ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	എങ്കിൽ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		പ്രതിജ്ഞാ(props.inline_container != ശൂന്യ);

		box_add_child(props.inline_container, box);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (ns_computed_display(box->style, props.കണ്ണി_is_വേര്) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			എങ്കിൽ (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				തിരികെ false;
		}

		എങ്കിൽ (props.കണ്ണി_is_വേര് == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ഇടത് ||
				css_computed_float(box->style) ==
				CSS_FLOAT_വലത്)) {
			/* Float: insert a float between the parent and box. */
			ഘടന box *flt = box_create(ശൂന്യ, ശൂന്യ, false,
					props.href, props.target, props.title,
					ശൂന്യ, ctx->bctx);
			എങ്കിൽ (flt == ശൂന്യ)
				തിരികെ false;

			എങ്കിൽ (css_computed_float(box->style) == CSS_FLOAT_ഇടത്)
				flt->type = BOX_FLOAT_ഇടത്;
			അല്ലെങ്കിൽ
				flt->type = BOX_FLOAT_വലത്;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} അല്ലെങ്കിൽ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			എങ്കിൽ (props.containing_block != ശൂന്യ)
				box_add_child(props.containing_block, box);
		}
	}

	തിരികെ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
സ്ഥിതം ശൂന്യം box_construct_element_after(dom_കണ്ണി *n, html_content *content)
{
	ഘടന box_construct_props props;
	ഘടന box *box = box_for_കണ്ണി(n);

	പ്രതിജ്ഞാ(box != ശൂന്യ);

	box_extract_properties(n, &props);

	എങ്കിൽ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ഘടന box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_കണ്ണി_has_child_nodes(n, &has_children);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ;

		എങ്കിൽ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			തിരികെ;
		}

		എങ്കിൽ (props.inline_container == ശൂന്യ) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ശൂന്യ, ശൂന്യ, false,
					ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ, content->bctx);
			എങ്കിൽ (props.inline_container == ശൂന്യ)
				തിരികെ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(ശൂന്യ, box->style, false,
				box->href, box->target, box->title,
				box->id == ശൂന്യ ? ശൂന്യ :
				lwc_string_ref(box->id), content->bctx);
		എങ്കിൽ (inline_end != ശൂന്യ) {
			inline_end->type = BOX_INLINE_END;

			പ്രതിജ്ഞാ(props.inline_container != ശൂന്യ);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (!(box->flags & IS_REPLACED)) {
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
സ്ഥിതം dom_കണ്ണി *
next_കണ്ണി(dom_കണ്ണി *n, html_content *content, bool convert_children)
{
	dom_കണ്ണി *next = ശൂന്യ;
	bool has_children;
	dom_exception err;

	err = dom_കണ്ണി_has_child_nodes(n, &has_children);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_കണ്ണി_unref(n);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (convert_children && has_children) {
		err = dom_കണ്ണി_get_first_child(n, &next);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ ശൂന്യ;
		}
		dom_കണ്ണി_unref(n);
	} അല്ലെങ്കിൽ {
		err = dom_കണ്ണി_get_next_sibling(n, &next);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ ശൂന്യ;
		}

		എങ്കിൽ (next != ശൂന്യ) {
			എങ്കിൽ (box_for_കണ്ണി(n) != ശൂന്യ)
				box_construct_element_after(n, content);
			dom_കണ്ണി_unref(n);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (box_for_കണ്ണി(n) != ശൂന്യ)
				box_construct_element_after(n, content);

			ആകുമ്പോൾ (box_is_വേര്(n) == false) {
				dom_കണ്ണി *parent = ശൂന്യ;
				dom_കണ്ണി *parent_next = ശൂന്യ;

				err = dom_കണ്ണി_get_parent_കണ്ണി(n, &parent);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(n);
					തിരികെ ശൂന്യ;
				}

				പ്രതിജ്ഞാ(parent != ശൂന്യ);

				err = dom_കണ്ണി_get_next_sibling(parent,
						&parent_next);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(parent);
					dom_കണ്ണി_unref(n);
					തിരികെ ശൂന്യ;
				}

				എങ്കിൽ (parent_next != ശൂന്യ) {
					dom_കണ്ണി_unref(parent_next);
					dom_കണ്ണി_unref(parent);
					നിർത്തുക;
				}

				dom_കണ്ണി_unref(n);
				n = parent;
				parent = ശൂന്യ;

				എങ്കിൽ (box_for_കണ്ണി(n) != ശൂന്യ) {
					box_construct_element_after(
							n, content);
				}
			}

			എങ്കിൽ (box_is_വേര്(n) == false) {
				dom_കണ്ണി *parent = ശൂന്യ;

				err = dom_കണ്ണി_get_parent_കണ്ണി(n, &parent);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(n);
					തിരികെ ശൂന്യ;
				}

				പ്രതിജ്ഞാ(parent != ശൂന്യ);

				err = dom_കണ്ണി_get_next_sibling(parent, &next);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(parent);
					dom_കണ്ണി_unref(n);
					തിരികെ ശൂന്യ;
				}

				എങ്കിൽ (box_for_കണ്ണി(parent) != ശൂന്യ) {
					box_construct_element_after(parent,
							content);
				}

				dom_കണ്ണി_unref(parent);
			}

			dom_കണ്ണി_unref(n);
		}
	}

	തിരികെ next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
സ്ഥിതം ശൂന്യം
box_text_transform(അക്ഷരം *s, unsigned പൂർണ്ണസംഖ്യ len, എണ്ണൽ css_text_transform_e tt)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	എങ്കിൽ (len == 0)
		തിരികെ;
	തിരഞ്ഞെടുപ്പ് (tt) {
		അവസ്ഥ CSS_TEXT_TRANSFORM_UPPERCASE:
			വേണ്ടി (i = 0; i < len; ++i)
				എങ്കിൽ ((unsigned അക്ഷരം) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			നിർത്തുക;
		അവസ്ഥ CSS_TEXT_TRANSFORM_LOWERCASE:
			വേണ്ടി (i = 0; i < len; ++i)
				എങ്കിൽ ((unsigned അക്ഷരം) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			നിർത്തുക;
		അവസ്ഥ CSS_TEXT_TRANSFORM_CAPITALIZE:
			എങ്കിൽ ((unsigned അക്ഷരം) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			വേണ്ടി (i = 1; i < len; ++i)
				എങ്കിൽ ((unsigned അക്ഷരം) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
സ്ഥിതം bool box_construct_text(ഘടന box_construct_ctx *ctx)
{
	ഘടന box_construct_props props;
	ഘടന box *box = ശൂന്യ;
	dom_string *content;
	dom_exception err;

	പ്രതിജ്ഞാ(ctx->n != ശൂന്യ);

	box_extract_properties(ctx->n, &props);

	പ്രതിജ്ഞാ(props.containing_block != ശൂന്യ);

	err = dom_characterdata_get_data(ctx->n, &content);
	എങ്കിൽ (err != DOM_NO_ERR || content == ശൂന്യ)
		തിരികെ false;

	എങ്കിൽ (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		അക്ഷരം *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		എങ്കിൽ (text == ശൂന്യ)
			തിരികെ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		എങ്കിൽ (text[0] == ' ' && text[1] == 0) {
			എങ്കിൽ (props.inline_container != ശൂന്യ) {
				പ്രതിജ്ഞാ(props.inline_container->last != ശൂന്യ);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			സ്വതന്ത്രം(text);

			തിരികെ true;
		}

		എങ്കിൽ (props.inline_container == ശൂന്യ) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ശൂന്യ, ശൂന്യ, false,
					ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ, ctx->bctx);
			എങ്കിൽ (props.inline_container == ശൂന്യ) {
				സ്വതന്ത്രം(text);
				തിരികെ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ശൂന്യ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ശൂന്യ, ctx->bctx);
		എങ്കിൽ (box == ശൂന്യ) {
			സ്വതന്ത്രം(text);
			തിരികെ false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		സ്വതന്ത്രം(text);
		എങ്കിൽ (box->text == ശൂന്യ)
			തിരികെ false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		എങ്കിൽ (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		എങ്കിൽ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		എങ്കിൽ (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			എങ്കിൽ (box->prev != ശൂന്യ)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} അല്ലെങ്കിൽ {
		/* white-space: pre */
		അക്ഷരം *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		അക്ഷരം *current;
		എണ്ണൽ css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		പ്രതിജ്ഞാ(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_വരി ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		എങ്കിൽ (text == ശൂന്യ)
			തിരികെ false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		വേണ്ടി (i = 0; i < text_len; i++)
			എങ്കിൽ (text[i] == '\t')
				text[i] = ' ';

		എങ്കിൽ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		എങ്കിൽ (props.containing_block->flags & PRE_STRIP) {
			തിരഞ്ഞെടുപ്പ് (*current) {
			അവസ്ഥ '\n':
				current++;
				നിർത്തുക;
			അവസ്ഥ '\r':
				current++;
				എങ്കിൽ (*current == '\n')
					current++;
				നിർത്തുക;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		ചെയ്യുക {
			size_t len = strcspn(current, "\r\n");

			അക്ഷരം old = current[len];

			current[len] = 0;

			എങ്കിൽ (props.inline_container == ശൂന്യ) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ശൂന്യ, ശൂന്യ,
						false, ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ,
						ctx->bctx);
				എങ്കിൽ (props.inline_container == ശൂന്യ) {
					സ്വതന്ത്രം(text);
					തിരികെ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ശൂന്യ,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ശൂന്യ, ctx->bctx);
			എങ്കിൽ (box == ശൂന്യ) {
				സ്വതന്ത്രം(text);
				തിരികെ false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			എങ്കിൽ (box->text == ശൂന്യ) {
				സ്വതന്ത്രം(text);
				തിരികെ false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			എങ്കിൽ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ശൂന്യ, ശൂന്യ,
						false, ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ,
						ctx->bctx);
				എങ്കിൽ (props.inline_container == ശൂന്യ) {
					സ്വതന്ത്രം(text);
					തിരികെ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				എങ്കിൽ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				അല്ലെങ്കിൽ
					current++;
			}
		} ആകുമ്പോൾ (*current);

		സ്വതന്ത്രം(text);
	}

	തിരികെ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
സ്ഥിതം ശൂന്യം convert_xml_to_box(ഘടന box_construct_ctx *ctx)
{
	dom_കണ്ണി *next;
	bool convert_children;
	uint32_t അക്കം_processed = 0;
	സ്ഥിരം uint32_t max_processed_before_yield = 10;

	ചെയ്യുക {
		convert_children = true;

		പ്രതിജ്ഞാ(ctx->n != ശൂന്യ);

		എങ്കിൽ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_കണ്ണി_unref(ctx->n);
			സ്വതന്ത്രം(ctx);
			തിരികെ;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_കണ്ണി(ctx->n, ctx->content, convert_children);
		ആകുമ്പോൾ (next != ശൂന്യ) {
			dom_കണ്ണി_type type;
			dom_exception err;

			err = dom_കണ്ണി_get_കണ്ണി_type(next, &type);
			എങ്കിൽ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_കണ്ണി_unref(next);
				സ്വതന്ത്രം(ctx);
				തിരികെ;
			}

			എങ്കിൽ (type == DOM_ELEMENT_കണ്ണി)
				നിർത്തുക;

			എങ്കിൽ (type == DOM_TEXT_കണ്ണി) {
				ctx->n = next;
				എങ്കിൽ (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_കണ്ണി_unref(ctx->n);
					സ്വതന്ത്രം(ctx);
					തിരികെ;
				}
			}

			next = next_കണ്ണി(next, ctx->content, true);
		}

		ctx->n = next;

		എങ്കിൽ (next == ശൂന്യ) {
			/* Conversion complete */
			ഘടന box വേര്;

			memset(&വേര്, 0, വലുപ്പം(വേര്));

			വേര്.type = BOX_BLOCK;
			വേര്.children = വേര്.last = ctx->വേര്_box;
			വേര്.children->parent = &വേര്;

			/** \todo Remove box_normalise_block */
			എങ്കിൽ (box_normalise_block(&വേര്, ctx->വേര്_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} അല്ലെങ്കിൽ {
				ctx->content->layout = വേര്.children;
				ctx->content->layout->parent = ശൂന്യ;

				ctx->cb(ctx->content, true);
			}

			പ്രതിജ്ഞാ(ctx->n == ശൂന്യ);

			സ്വതന്ത്രം(ctx);
			തിരികെ;
		}
	} ആകുമ്പോൾ (++അക്കം_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (ശൂന്യം *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_കണ്ണി *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   ശൂന്യം **box_conversion_context)
{
	ഘടന box_construct_ctx *ctx;

	പ്രതിജ്ഞാ(box_conversion_context != ശൂന്യ);

	എങ്കിൽ (c->bctx == ശൂന്യ) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, പൂർണ്ണസംഖ്യ);
		എങ്കിൽ (c->bctx == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
	}

	ctx = malloc(വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_കണ്ണി_ref(n);
	ctx->വേര്_box = ശൂന്യ;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	തിരികെ guit->misc->schedule(0, (ശൂന്യം *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(ശൂന്യം *box_conversion_context)
{
	ഘടന box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (ശൂന്യം *)convert_xml_to_box, ctx);
	എങ്കിൽ (err != NSERROR_OK) {
		തിരികെ err;
	}

	dom_കണ്ണി_unref(ctx->n);
	സ്വതന്ത്രം(ctx);

	തിരികെ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ഘടന box *box_for_കണ്ണി(dom_കണ്ണി *n)
{
	ഘടന box *box = ശൂന്യ;
	dom_exception err;

	err = dom_കണ്ണി_get_user_data(n, corestring_dom_ns_താക്കോൽ_box_കണ്ണി_data,
			(ശൂന്യം *) &box);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ ശൂന്യ;

	തിരികെ box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(സ്ഥിരം html_content *content,
		 സ്ഥിരം dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	അക്ഷരം *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned പൂർണ്ണസംഖ്യ i, j, end;
	nserror error;
	സ്ഥിരം അക്ഷരം *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	എങ്കിൽ (!s)
		തിരികെ false;

	/* copy to s, removing white space and control characters */
	വേണ്ടി (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	വേണ്ടി (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	വേണ്ടി (j = 0; i != end; i++) {
		എങ്കിൽ ((unsigned അക്ഷരം) rel[i] < 0x20) {
			; /* skip control characters */
		} അല്ലെങ്കിൽ എങ്കിൽ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} അല്ലെങ്കിൽ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	എങ്കിൽ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		എങ്കിൽ (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			എങ്കിൽ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			എങ്കിൽ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			എങ്കിൽ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} അല്ലെങ്കിൽ എങ്കിൽ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	സ്വതന്ത്രം(s);
	എങ്കിൽ (error != NSERROR_OK) {
		*result = ശൂന്യ;
		തിരികെ false;
	}

	തിരികെ true;
}
