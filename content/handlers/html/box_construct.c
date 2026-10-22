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
ساخت box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_گرہ *n;			/**< Current node to process */

	ساخت box *جڑ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	صحیح *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ساخت box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ثابت css_computed_style *والد_style;
	/** Current link target, or NULL if none */
	ساخت nsurl *href;
	/** Current frame target, or NULL if none */
	ثابت حرف *ہدف;
	/** Current title attribute, or NULL if none */
	ثابت حرف *title;
	/** Identity of the current block-level container */
	ساخت box *containing_بلاک;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ساخت box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool گرہ_is_جڑ;
};

ساکن ثابت content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ساکن ثابت box_type box_map[] = {
	BOX_بلاک,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_بلاک,           /* CSS_DISPLAY_BLOCK */
	BOX_بلاک,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_بلاک,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_جدول,           /* CSS_DISPLAY_TABLE */
	BOX_جدول,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_جدول_صف_گروہ, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_جدول_صف_گروہ, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_جدول_صف_گروہ, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_جدول_صف,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_جدول_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_بلاک,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_بلاک,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ساکن اندرونی bool box_is_جڑ(dom_گرہ *n)
{
	dom_گرہ *والد;
	dom_گرہ_type type;
	dom_exception err;

	err = dom_گرہ_get_والد_گرہ(n, &والد);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (والد != تہی) {
		err = dom_گرہ_get_گرہ_type(والد, &type);

		dom_گرہ_unref(والد);

		اگر (err != DOM_NO_ERR)
			واپس false;

		اگر (type != DOM_DOCUMENT_گرہ)
			واپس false;
	}

	واپس true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ساکن خالی
box_نکالو_properties(dom_گرہ *n, ساخت box_construct_props *props)
{
	memset(props, 0, حجم(*props));

	props->گرہ_is_جڑ = box_is_جڑ(n);

	/* Extract properties from containing DOM node */
	اگر (props->گرہ_is_جڑ == false) {
		dom_گرہ *current_گرہ = n;
		dom_گرہ *والد_گرہ = تہی;
		ساخت box *والد_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		جب (true) {
			err = dom_گرہ_get_والد_گرہ(current_گرہ,
					&والد_گرہ);
			اگر (err != DOM_NO_ERR || والد_گرہ == تہی)
				توڑو;

			والد_box = box_for_گرہ(والد_گرہ);

			اگر (والد_box != تہی) {
				props->والد_style = والد_box->style;
				props->href = والد_box->href;
				props->ہدف = والد_box->ہدف;
				props->title = والد_box->title;

				dom_گرہ_unref(والد_گرہ);
				توڑو;
			} ورنہ {
				اگر (current_گرہ != n)
					dom_گرہ_unref(current_گرہ);
				current_گرہ = والد_گرہ;
				والد_گرہ = تہی;
			}
		}

		/* Find containing block (may be parent) */
		جب (true) {
			ساخت box *b;

			err = dom_گرہ_get_والد_گرہ(current_گرہ,
					&والد_گرہ);
			اگر (err != DOM_NO_ERR || والد_گرہ == تہی) {
				اگر (current_گرہ != n)
					dom_گرہ_unref(current_گرہ);
				توڑو;
			}

			اگر (current_گرہ != n)
				dom_گرہ_unref(current_گرہ);

			b = box_for_گرہ(والد_گرہ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			اگر (b != تہی && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_بلاک = b;

				dom_گرہ_unref(والد_گرہ);
				توڑو;
			} ورنہ {
				current_گرہ = والد_گرہ;
				والد_گرہ = تہی;
			}
		}
	}

	/* Compute current inline container, if any */
	اگر (props->containing_بلاک != تہی &&
			props->containing_بلاک->آخری != تہی &&
			props->containing_بلاک->آخری->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_بلاک->آخری;
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
ساکن css_select_results *
box_get_style(html_content *c,
	      ثابت css_computed_style *والد_style,
	      ثابت css_computed_style *جڑ_style,
	      dom_گرہ *n)
{
	dom_string *s = تہی;
	css_stylesheet *inline_style = تہی;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	اگر (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		اگر (err != DOM_NO_ERR) {
			واپس تہی;
		}
	}

	اگر (s != تہی) {
		inline_style = nscss_create_inline_style(
				(ثابت uint8_t *) dom_string_ڈیٹا(s),
				dom_string_byte_لمبائی(s),
				c->encoding,
				nsurl_access(c->بنیاد_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_منوال_NONE);

		dom_string_unref(s);

		اگر (inline_style == تہی)
			واپس تہی;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
	ctx.بنیاد_url = c->بنیاد_url;
	ctx.universal = c->universal;
	ctx.جڑ_style = جڑ_style;
	ctx.والد_style = والد_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	اگر (inline_style != تہی)
		css_stylesheet_destroy(inline_style);

	واپس styles;
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
ساکن خالی
box_construct_generate(dom_گرہ *n,
		       html_content *content,
		       ساخت box *box,
		       ثابت css_computed_style *style)
{
	ساخت box *gen = تہی;
	شمار css_display_e computed_display;
	ثابت css_computed_content_شے *c_شے;

	/* Nothing to generate if the parent box is not a block */
	اگر (box->type != BOX_بلاک)
		واپس;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	اگر (style == تہی ||
			css_computed_content(style, &c_شے) ==
			CSS_CONTENT_معمول) {
		/* No pseudo element */
		واپس;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_جڑ(n));
	اگر (computed_display == CSS_DISPLAY_بلاک ||
			computed_display == CSS_DISPLAY_جدول) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(تہی, (css_computed_style *) style,
				false, تہی, تہی, تہی, تہی, content->bctx);
		اگر (gen == تہی) {
			واپس;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_جڑ(n))];

		box_جوڑو_بچہ(box, gen);
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
ساکن bool
box_construct_marker(ساخت box *box,
		     ثابت حرف *title,
		     ساخت box_construct_ctx *ctx,
		     ساخت box *والد)
{
	lwc_string *image_uri;
	ساخت box *marker;
	شمار css_list_style_type_e list_style_type;

	marker = box_create(تہی, box->style, false, تہی, تہی, title,
			تہی, ctx->bctx);
	اگر (marker == false)
		واپس false;

	marker->type = BOX_بلاک;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	انتخاب (list_style_type) {
	حالت CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->متن = (حرف *) "\342\200\242";
		marker->لمبائی = 3;
		توڑو;

	حالت CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->متن = (حرف *) "\342\227\213";
		marker->لمبائی = 3;
		توڑو;

	حالت CSS_LIST_STYLE_TYPE_مربع:
		/* 25AA BLACK SMALL SQUARE */
		marker->متن = (حرف *) "\342\226\252";
		marker->لمبائی = 3;
		توڑو;

	طےشدہ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_NONE:
		marker->متن = تہی;
		marker->لمبائی = 0;
		توڑو;
	}

	اگر (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != تہی) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ڈیٹا(image_uri), &url);
		اگر (error != NSERROR_OK)
			واپس false;

		اگر (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			واپس false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->والد = box;

	واپس true;
}

ساکن اندرونی bool box__style_is_float(ثابت ساخت box *box)
{
	واپس css_computed_float(box->style) == CSS_FLOAT_بایاں ||
	       css_computed_float(box->style) == CSS_FLOAT_دایاں;
}

ساکن اندرونی bool box__is_flex(ثابت ساخت box *box)
{
	واپس box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ساکن اندرونی bool box_containing_بلاک_is_flex(
		ثابت ساخت box_construct_props *props)
{
	واپس props->containing_بلاک != تہی &&
	       box__is_flex(props->containing_بلاک);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ساکن bool
box_construct_element(ساخت box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = تہی;
	شمار css_display_e css_display;
	ساخت box *box = تہی, *old_box;
	css_select_results *styles = تہی;
	lwc_string *bgimage_uri;
	dom_exception err;
	ساخت box_construct_props props;
	ثابت css_computed_style *جڑ_style = تہی;

	أكد(ctx->n != تہی);

	box_نکالو_properties(ctx->n, &props);

	اگر (props.containing_بلاک != تہی) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_بلاک->flags &= ~PRE_STRIP;
	}

	اگر (props.گرہ_is_جڑ == false) {
		جڑ_style = ctx->جڑ_box->style;
	}

	styles = box_get_style(ctx->content, props.والد_style, جڑ_style,
			ctx->n);
	اگر (styles == تہی)
		واپس false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (title0 != تہی) {
		حرف *t = squash_whitespace(dom_string_ڈیٹا(title0));

		dom_string_unref(title0);

		اگر (t == تہی)
			واپس false;

		props.title = talloc_strdup(ctx->bctx, t);

		آزاد(t);

		اگر (props.title == تہی)
			واپس false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (s != تہی) {
		err = dom_string_intern(s, &id);
		اگر (err != DOM_NO_ERR)
			id = تہی;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.ہدف, props.title, id,
			ctx->bctx);
	اگر (box == تہی)
		واپس false;

	/* If this is the root box, add it to the context */
	اگر (props.گرہ_is_جڑ)
		ctx->جڑ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (s != تہی) {
		ثابت حرف *val = dom_string_ڈیٹا(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		اگر ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, تہی, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (s != تہی) {
		ثابت حرف *val = dom_string_ڈیٹا(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		اگر ('0' <= val[0] && val[0] <= '9')
			box->صفیں = clamp(strtol(val, تہی, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	اگر ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_بلاک ||
			 css_display == CSS_DISPLAY_INLINE_جدول ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_بلاک];
	} ورنہ اگر (props.گرہ_is_جڑ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_بلاک;
	} ورنہ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.گرہ_is_جڑ)];

		اگر (props.containing_بلاک->type == BOX_FLEX ||
		    props.containing_بلاک->type == BOX_INLINE_FLEX) {
			/* Blockification */
			انتخاب (box->type) {
			حالت BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				توڑو;
			حالت BOX_INLINE_بلاک:
				box->type = BOX_بلاک;
				توڑو;
			طےشدہ:
				توڑو;
			}
		}
	}

	اگر (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		واپس false;
	}

	/* Handle the :before pseudo element */
	اگر (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	اگر (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.گرہ_is_جڑ) == CSS_DISPLAY_NONE &&
			props.گرہ_is_جڑ == false)) {
		css_select_results_destroy(styles);
		box->styles = تہی;
		box->style = تہی;

		/* Invalidate associated gadget, if any */
		اگر (box->gadget != تہی) {
			box->gadget->box = تہی;
			box->gadget = تہی;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		واپس true;
	}

	/* Attach DOM node to box */
	err = dom_گرہ_سیٹ_user_ڈیٹا(ctx->n,
			corestring_dom_ns_کلید_box_گرہ_ڈیٹا, box, تہی,
			(خالی *) &old_box);
	اگر (err != DOM_NO_ERR)
		واپس false;

	/* Attach box to DOM node */
	box->گرہ = dom_گرہ_ref(ctx->n);

	اگر (props.inline_container == تہی &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_بلاک ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box_containing_بلاک_is_flex(&props))) &&
			props.گرہ_is_جڑ == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		أكد(props.containing_بلاک != تہی &&
				"Box must have containing block.");

		props.inline_container = box_create(تہی, تہی, false, تہی,
				تہی, تہی, تہی, ctx->bctx);
		اگر (props.inline_container == تہی)
			واپس false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_جوڑو_بچہ(props.containing_بلاک, props.inline_container);
	}

	/* Kick off fetch for any background image */
	اگر (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != تہی &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_ڈیٹا(bgimage_uri), &url);
		اگر (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			اگر (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				واپس false;
			}
			nsurl_unref(url);
		}
	}

	اگر (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	اگر (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_بلاک) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		أكد(props.inline_container != تہی);

		box_جوڑو_بچہ(props.inline_container, box);
	} ورنہ {
		اگر (ns_computed_display(box->style, props.گرہ_is_جڑ) ==
				CSS_DISPLAY_LIST_شے) {
			/* List item: compute marker */
			اگر (box_construct_marker(box, props.title, ctx,
					props.containing_بلاک) == false)
				واپس false;
		}

		اگر (props.گرہ_is_جڑ == false &&
				box_containing_بلاک_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_بایاں ||
				css_computed_float(box->style) ==
				CSS_FLOAT_دایاں)) {
			/* Float: insert a float between the parent and box. */
			ساخت box *flt = box_create(تہی, تہی, false,
					props.href, props.ہدف, props.title,
					تہی, ctx->bctx);
			اگر (flt == تہی)
				واپس false;

			اگر (css_computed_float(box->style) == CSS_FLOAT_بایاں)
				flt->type = BOX_FLOAT_بایاں;
			ورنہ
				flt->type = BOX_FLOAT_دایاں;

			box_جوڑو_بچہ(props.inline_container, flt);
			box_جوڑو_بچہ(flt, box);
		} ورنہ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			اگر (props.containing_بلاک != تہی)
				box_جوڑو_بچہ(props.containing_بلاک, box);
		}
	}

	واپس true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ساکن خالی box_construct_element_after(dom_گرہ *n, html_content *content)
{
	ساخت box_construct_props props;
	ساخت box *box = box_for_گرہ(n);

	أكد(box != تہی);

	box_نکالو_properties(n, &props);

	اگر (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ساخت box *inline_انتہا;
		bool has_children;
		dom_exception err;

		err = dom_گرہ_has_بچہ_nodes(n, &has_children);
		اگر (err != DOM_NO_ERR)
			واپس;

		اگر (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			واپس;
		}

		اگر (props.inline_container == تہی) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(تہی, تہی, false,
					تہی, تہی, تہی, تہی, content->bctx);
			اگر (props.inline_container == تہی)
				واپس;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_جوڑو_بچہ(props.containing_بلاک,
					props.inline_container);
		}

		inline_انتہا = box_create(تہی, box->style, false,
				box->href, box->ہدف, box->title,
				box->id == تہی ? تہی :
				lwc_string_ref(box->id), content->bctx);
		اگر (inline_انتہا != تہی) {
			inline_انتہا->type = BOX_INLINE_انتہا;

			أكد(props.inline_container != تہی);

			box_جوڑو_بچہ(props.inline_container, inline_انتہا);

			box->inline_انتہا = inline_انتہا;
			inline_انتہا->inline_انتہا = box;
		}
	} ورنہ اگر (!(box->flags & IS_REPLACED)) {
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
ساکن dom_گرہ *
اگلا_گرہ(dom_گرہ *n, html_content *content, bool convert_children)
{
	dom_گرہ *اگلا = تہی;
	bool has_children;
	dom_exception err;

	err = dom_گرہ_has_بچہ_nodes(n, &has_children);
	اگر (err != DOM_NO_ERR) {
		dom_گرہ_unref(n);
		واپس تہی;
	}

	اگر (convert_children && has_children) {
		err = dom_گرہ_get_پہلا_بچہ(n, &اگلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس تہی;
		}
		dom_گرہ_unref(n);
	} ورنہ {
		err = dom_گرہ_get_اگلا_sibling(n, &اگلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس تہی;
		}

		اگر (اگلا != تہی) {
			اگر (box_for_گرہ(n) != تہی)
				box_construct_element_after(n, content);
			dom_گرہ_unref(n);
		} ورنہ {
			اگر (box_for_گرہ(n) != تہی)
				box_construct_element_after(n, content);

			جب (box_is_جڑ(n) == false) {
				dom_گرہ *والد = تہی;
				dom_گرہ *والد_اگلا = تہی;

				err = dom_گرہ_get_والد_گرہ(n, &والد);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(n);
					واپس تہی;
				}

				أكد(والد != تہی);

				err = dom_گرہ_get_اگلا_sibling(والد,
						&والد_اگلا);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(والد);
					dom_گرہ_unref(n);
					واپس تہی;
				}

				اگر (والد_اگلا != تہی) {
					dom_گرہ_unref(والد_اگلا);
					dom_گرہ_unref(والد);
					توڑو;
				}

				dom_گرہ_unref(n);
				n = والد;
				والد = تہی;

				اگر (box_for_گرہ(n) != تہی) {
					box_construct_element_after(
							n, content);
				}
			}

			اگر (box_is_جڑ(n) == false) {
				dom_گرہ *والد = تہی;

				err = dom_گرہ_get_والد_گرہ(n, &والد);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(n);
					واپس تہی;
				}

				أكد(والد != تہی);

				err = dom_گرہ_get_اگلا_sibling(والد, &اگلا);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(والد);
					dom_گرہ_unref(n);
					واپس تہی;
				}

				اگر (box_for_گرہ(والد) != تہی) {
					box_construct_element_after(والد,
							content);
				}

				dom_گرہ_unref(والد);
			}

			dom_گرہ_unref(n);
		}
	}

	واپس اگلا;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ساکن خالی
box_متن_transform(حرف *s, غیرعلامتی صحیح len, شمار css_متن_transform_e tt)
{
	غیرعلامتی صحیح i;
	اگر (len == 0)
		واپس;
	انتخاب (tt) {
		حالت CSS_متن_TRANSFORM_UPPERCASE:
			برائے (i = 0; i < len; ++i)
				اگر ((غیرعلامتی حرف) s[i] < 0x80)
					s[i] = ascii_مقصد_بالائی(s[i]);
			توڑو;
		حالت CSS_متن_TRANSFORM_LOWERCASE:
			برائے (i = 0; i < len; ++i)
				اگر ((غیرعلامتی حرف) s[i] < 0x80)
					s[i] = ascii_مقصد_زیریں(s[i]);
			توڑو;
		حالت CSS_متن_TRANSFORM_CAPITALIZE:
			اگر ((غیرعلامتی حرف) s[0] < 0x80)
				s[0] = ascii_مقصد_بالائی(s[0]);
			برائے (i = 1; i < len; ++i)
				اگر ((غیرعلامتی حرف) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_مقصد_بالائی(s[i]);
			توڑو;
		طےشدہ:
			توڑو;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ساکن bool box_construct_متن(ساخت box_construct_ctx *ctx)
{
	ساخت box_construct_props props;
	ساخت box *box = تہی;
	dom_string *content;
	dom_exception err;

	أكد(ctx->n != تہی);

	box_نکالو_properties(ctx->n, &props);

	أكد(props.containing_بلاک != تہی);

	err = dom_characterdata_get_ڈیٹا(ctx->n, &content);
	اگر (err != DOM_NO_ERR || content == تہی)
		واپس false;

	اگر (css_computed_white_space(props.والد_style) ==
			CSS_WHITE_SPACE_معمول ||
			css_computed_white_space(props.والد_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		حرف *متن;

		متن = squash_whitespace(dom_string_ڈیٹا(content));

		dom_string_unref(content);

		اگر (متن == تہی)
			واپس false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		اگر (متن[0] == ' ' && متن[1] == 0) {
			اگر (props.inline_container != تہی) {
				أكد(props.inline_container->آخری != تہی);

				props.inline_container->آخری->space =
						UNKNOWN_چوڑائی;
			}

			آزاد(متن);

			واپس true;
		}

		اگر (props.inline_container == تہی) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(تہی, تہی, false,
					تہی, تہی, تہی, تہی, ctx->bctx);
			اگر (props.inline_container == تہی) {
				آزاد(متن);
				واپس false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_جوڑو_بچہ(props.containing_بلاک,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(تہی,
				(css_computed_style *) props.والد_style,
				false, props.href, props.ہدف, props.title,
				تہی, ctx->bctx);
		اگر (box == تہی) {
			آزاد(متن);
			واپس false;
		}

		box->type = BOX_متن;

		box->متن = talloc_strdup(ctx->bctx, متن);
		آزاد(متن);
		اگر (box->متن == تہی)
			واپس false;

		box->لمبائی = لڑی_لمبائی(box->متن);

		/* strip ending space char off */
		اگر (box->لمبائی > 1 && box->متن[box->لمبائی - 1] == ' ') {
			box->space = UNKNOWN_چوڑائی;
			box->لمبائی--;
		}

		اگر (css_computed_متن_transform(props.والد_style) !=
				CSS_متن_TRANSFORM_NONE)
			box_متن_transform(box->متن, box->لمبائی,
				css_computed_متن_transform(
					props.والد_style));

		box_جوڑو_بچہ(props.inline_container, box);

		اگر (box->متن[0] == ' ') {
			box->لمبائی--;

			memmove(box->متن, &box->متن[1], box->لمبائی);

			اگر (box->پچھلا != تہی)
				box->پچھلا->space = UNKNOWN_چوڑائی;
		}
	} ورنہ {
		/* white-space: pre */
		حرف *متن;
		size_t متن_len = dom_string_byte_لمبائی(content);
		size_t i;
		حرف *current;
		شمار css_white_space_e white_space =
				css_computed_white_space(props.والد_style);

		/* note: pre-wrap/pre-line are unimplemented */
		أكد(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_سطر ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		متن = حافظہ_دو(متن_len + 1);
		dom_string_unref(content);

		اگر (متن == تہی)
			واپس false;

		حافظہ_نقل(متن, dom_string_ڈیٹا(content), متن_len);
		متن[متن_len] = '\0';

		/* TODO: Handle tabs properly */
		برائے (i = 0; i < متن_len; i++)
			اگر (متن[i] == '\t')
				متن[i] = ' ';

		اگر (css_computed_متن_transform(props.والد_style) !=
				CSS_متن_TRANSFORM_NONE)
			box_متن_transform(متن, لڑی_لمبائی(متن),
				css_computed_متن_transform(
						props.والد_style));

		current = متن;

		/* swallow a single leading new line */
		اگر (props.containing_بلاک->flags & PRE_STRIP) {
			انتخاب (*current) {
			حالت '\n':
				current++;
				توڑو;
			حالت '\r':
				current++;
				اگر (*current == '\n')
					current++;
				توڑو;
			}
			props.containing_بلاک->flags &= ~PRE_STRIP;
		}

		کرو {
			size_t len = strcspn(current, "\r\n");

			حرف old = current[len];

			current[len] = 0;

			اگر (props.inline_container == تہی) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(تہی, تہی,
						false, تہی, تہی, تہی, تہی,
						ctx->bctx);
				اگر (props.inline_container == تہی) {
					آزاد(متن);
					واپس false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_جوڑو_بچہ(props.containing_بلاک,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(تہی,
				(css_computed_style *) props.والد_style,
				false, props.href, props.ہدف, props.title,
				تہی, ctx->bctx);
			اگر (box == تہی) {
				آزاد(متن);
				واپس false;
			}

			box->type = BOX_متن;

			box->متن = talloc_strdup(ctx->bctx, current);
			اگر (box->متن == تہی) {
				آزاد(متن);
				واپس false;
			}

			box->لمبائی = لڑی_لمبائی(box->متن);

			box_جوڑو_بچہ(props.inline_container, box);

			current[len] = old;

			current += len;

			اگر (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(تہی, تہی,
						false, تہی, تہی, تہی, تہی,
						ctx->bctx);
				اگر (props.inline_container == تہی) {
					آزاد(متن);
					واپس false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_جوڑو_بچہ(props.containing_بلاک,
						props.inline_container);

				اگر (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ورنہ
					current++;
			}
		} جب (*current);

		آزاد(متن);
	}

	واپس true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ساکن خالی convert_xml_مقصد_box(ساخت box_construct_ctx *ctx)
{
	dom_گرہ *اگلا;
	bool convert_children;
	uint32_t عدد_processed = 0;
	ثابت uint32_t max_processed_before_yield = 10;

	کرو {
		convert_children = true;

		أكد(ctx->n != تہی);

		اگر (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_گرہ_unref(ctx->n);
			آزاد(ctx);
			واپس;
		}

		/* Find next element to process, converting text nodes as we go */
		اگلا = اگلا_گرہ(ctx->n, ctx->content, convert_children);
		جب (اگلا != تہی) {
			dom_گرہ_type type;
			dom_exception err;

			err = dom_گرہ_get_گرہ_type(اگلا, &type);
			اگر (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_گرہ_unref(اگلا);
				آزاد(ctx);
				واپس;
			}

			اگر (type == DOM_ELEMENT_گرہ)
				توڑو;

			اگر (type == DOM_متن_گرہ) {
				ctx->n = اگلا;
				اگر (box_construct_متن(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_گرہ_unref(ctx->n);
					آزاد(ctx);
					واپس;
				}
			}

			اگلا = اگلا_گرہ(اگلا, ctx->content, true);
		}

		ctx->n = اگلا;

		اگر (اگلا == تہی) {
			/* Conversion complete */
			ساخت box جڑ;

			memset(&جڑ, 0, حجم(جڑ));

			جڑ.type = BOX_بلاک;
			جڑ.children = جڑ.آخری = ctx->جڑ_box;
			جڑ.children->والد = &جڑ;

			/** \todo Remove box_normalise_block */
			اگر (box_normalise_بلاک(&جڑ, ctx->جڑ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ورنہ {
				ctx->content->layout = جڑ.children;
				ctx->content->layout->والد = تہی;

				ctx->cb(ctx->content, true);
			}

			أكد(ctx->n == تہی);

			آزاد(ctx);
			واپس;
		}
	} جب (++عدد_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->جدول_وقت(0, (خالی *)convert_xml_مقصد_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_مقصد_box(dom_گرہ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   خالی **box_conversion_context)
{
	ساخت box_construct_ctx *ctx;

	أكد(box_conversion_context != تہی);

	اگر (c->bctx == تہی) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, صحیح);
		اگر (c->bctx == تہی) {
			واپس NSERROR_NOMEM;
		}
	}

	ctx = حافظہ_دو(حجم(*ctx));
	اگر (ctx == تہی) {
		واپس NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_گرہ_ref(n);
	ctx->جڑ_box = تہی;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	واپس guit->misc->جدول_وقت(0, (خالی *)convert_xml_مقصد_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_مقصد_box(خالی *box_conversion_context)
{
	ساخت box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->جدول_وقت(-1, (خالی *)convert_xml_مقصد_box, ctx);
	اگر (err != NSERROR_OK) {
		واپس err;
	}

	dom_گرہ_unref(ctx->n);
	آزاد(ctx);

	واپس NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ساخت box *box_for_گرہ(dom_گرہ *n)
{
	ساخت box *box = تہی;
	dom_exception err;

	err = dom_گرہ_get_user_ڈیٹا(n, corestring_dom_ns_کلید_box_گرہ_ڈیٹا,
			(خالی *) &box);
	اگر (err != DOM_NO_ERR)
		واپس تہی;

	واپس box;
}

/* exported function documented in html/box_construct.h */
bool
box_نکالو_link(ثابت html_content *content,
		 ثابت dom_string *dsrel,
		 nsurl *بنیاد,
		 nsurl **نتیجہ)
{
	حرف *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	غیرعلامتی صحیح i, j, انتہا;
	nserror error;
	ثابت حرف *rel;

	rel = dom_string_ڈیٹا(dsrel);

	s1 = s = حافظہ_دو(3 * لڑی_لمبائی(rel) + 1);
	اگر (!s)
		واپس false;

	/* copy to s, removing white space and control characters */
	برائے (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	برائے (انتہا = لڑی_لمبائی(rel);
	     (انتہا != i) && ascii_is_space(rel[انتہا - 1]);
	     انتہا--)
		;
	برائے (j = 0; i != انتہا; i++) {
		اگر ((غیرعلامتی حرف) rel[i] < 0x20) {
			; /* skip control characters */
		} ورنہ اگر (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ورنہ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	اگر (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		اگر (لڑی_موازنہ(s, "javascript:", 11) == 0) {
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
			} ورنہ اگر (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(بنیاد, s1, نتیجہ);
	آزاد(s);
	اگر (error != NSERROR_OK) {
		*نتیجہ = تہی;
		واپس false;
	}

	واپس true;
}
