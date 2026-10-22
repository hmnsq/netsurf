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

	dom_گرہ *n;			/**< Current node to process */

	بنية box *جڑ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	صحيح *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
بنية box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ثابت css_computed_style *ماپا_style;
	/** Current link target, or NULL if none */
	بنية nsurl *href;
	/** Current frame target, or NULL if none */
	ثابت حرف *نشانہ;
	/** Current title attribute, or NULL if none */
	ثابت حرف *title;
	/** Identity of the current block-level container */
	بنية box *containing_بلاک;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	بنية box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool گرہ_is_جڑ;
};

ساكن ثابت content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ساكن ثابت box_type box_map[] = {
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
ساكن مضمن bool box_is_جڑ(dom_گرہ *n)
{
	dom_گرہ *ماپا;
	dom_گرہ_type type;
	dom_exception err;

	err = dom_گرہ_get_ماپا_گرہ(n, &ماپا);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (ماپا != عدم) {
		err = dom_گرہ_get_گرہ_type(ماپا, &type);

		dom_گرہ_unref(ماپا);

		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (type != DOM_DOCUMENT_گرہ)
			أرجع false;
	}

	أرجع true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ساكن فراغ
box_کڈھو_properties(dom_گرہ *n, بنية box_construct_props *props)
{
	memset(props, 0, حجم(*props));

	props->گرہ_is_جڑ = box_is_جڑ(n);

	/* Extract properties from containing DOM node */
	إذا (props->گرہ_is_جڑ == false) {
		dom_گرہ *current_گرہ = n;
		dom_گرہ *ماپا_گرہ = عدم;
		بنية box *ماپا_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		طالما (true) {
			err = dom_گرہ_get_ماپا_گرہ(current_گرہ,
					&ماپا_گرہ);
			إذا (err != DOM_NO_ERR || ماپا_گرہ == عدم)
				اكسر;

			ماپا_box = box_for_گرہ(ماپا_گرہ);

			إذا (ماپا_box != عدم) {
				props->ماپا_style = ماپا_box->style;
				props->href = ماپا_box->href;
				props->نشانہ = ماپا_box->نشانہ;
				props->title = ماپا_box->title;

				dom_گرہ_unref(ماپا_گرہ);
				اكسر;
			} وإلا {
				إذا (current_گرہ != n)
					dom_گرہ_unref(current_گرہ);
				current_گرہ = ماپا_گرہ;
				ماپا_گرہ = عدم;
			}
		}

		/* Find containing block (may be parent) */
		طالما (true) {
			بنية box *b;

			err = dom_گرہ_get_ماپا_گرہ(current_گرہ,
					&ماپا_گرہ);
			إذا (err != DOM_NO_ERR || ماپا_گرہ == عدم) {
				إذا (current_گرہ != n)
					dom_گرہ_unref(current_گرہ);
				اكسر;
			}

			إذا (current_گرہ != n)
				dom_گرہ_unref(current_گرہ);

			b = box_for_گرہ(ماپا_گرہ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			إذا (b != عدم && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_بلاک = b;

				dom_گرہ_unref(ماپا_گرہ);
				اكسر;
			} وإلا {
				current_گرہ = ماپا_گرہ;
				ماپا_گرہ = عدم;
			}
		}
	}

	/* Compute current inline container, if any */
	إذا (props->containing_بلاک != عدم &&
			props->containing_بلاک->آخری != عدم &&
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
ساكن css_select_results *
box_get_style(html_content *c,
	      ثابت css_computed_style *ماپا_style,
	      ثابت css_computed_style *جڑ_style,
	      dom_گرہ *n)
{
	dom_string *s = عدم;
	css_stylesheet *inline_style = عدم;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	إذا (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		إذا (err != DOM_NO_ERR) {
			أرجع عدم;
		}
	}

	إذا (s != عدم) {
		inline_style = nscss_create_inline_style(
				(ثابت uint8_t *) dom_string_انکڑے(s),
				dom_string_byte_لمائی(s),
				c->encoding,
				nsurl_access(c->ادھار_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_منوال_NONE);

		dom_string_unref(s);

		إذا (inline_style == عدم)
			أرجع عدم;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
	ctx.ادھار_url = c->ادھار_url;
	ctx.universal = c->universal;
	ctx.جڑ_style = جڑ_style;
	ctx.ماپا_style = ماپا_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	إذا (inline_style != عدم)
		css_stylesheet_destroy(inline_style);

	أرجع styles;
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
box_construct_generate(dom_گرہ *n,
		       html_content *content,
		       بنية box *box,
		       ثابت css_computed_style *style)
{
	بنية box *gen = عدم;
	تعداد css_display_e computed_display;
	ثابت css_computed_content_شے *c_شے;

	/* Nothing to generate if the parent box is not a block */
	إذا (box->type != BOX_بلاک)
		أرجع;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	إذا (style == عدم ||
			css_computed_content(style, &c_شے) ==
			CSS_CONTENT_عام) {
		/* No pseudo element */
		أرجع;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_جڑ(n));
	إذا (computed_display == CSS_DISPLAY_بلاک ||
			computed_display == CSS_DISPLAY_جدول) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(عدم, (css_computed_style *) style,
				false, عدم, عدم, عدم, عدم, content->bctx);
		إذا (gen == عدم) {
			أرجع;
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
ساكن bool
box_construct_marker(بنية box *box,
		     ثابت حرف *title,
		     بنية box_construct_ctx *ctx,
		     بنية box *ماپا)
{
	lwc_string *image_uri;
	بنية box *marker;
	تعداد css_list_style_type_e list_style_type;

	marker = box_create(عدم, box->style, false, عدم, عدم, title,
			عدم, ctx->bctx);
	إذا (marker == false)
		أرجع false;

	marker->type = BOX_بلاک;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	بدل (list_style_type) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->متن = (حرف *) "\342\200\242";
		marker->لمائی = 3;
		اكسر;

	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->متن = (حرف *) "\342\227\213";
		marker->لمائی = 3;
		اكسر;

	حالة CSS_LIST_STYLE_TYPE_مربع:
		/* 25AA BLACK SMALL SQUARE */
		marker->متن = (حرف *) "\342\226\252";
		marker->لمائی = 3;
		اكسر;

	افتراضي:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_NONE:
		marker->متن = عدم;
		marker->لمائی = 0;
		اكسر;
	}

	إذا (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != عدم) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_انکڑے(image_uri), &url);
		إذا (error != NSERROR_OK)
			أرجع false;

		إذا (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			أرجع false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->ماپا = box;

	أرجع true;
}

ساكن مضمن bool box__style_is_float(ثابت بنية box *box)
{
	أرجع css_computed_float(box->style) == CSS_FLOAT_بایاں ||
	       css_computed_float(box->style) == CSS_FLOAT_دایاں;
}

ساكن مضمن bool box__is_flex(ثابت بنية box *box)
{
	أرجع box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ساكن مضمن bool box_containing_بلاک_is_flex(
		ثابت بنية box_construct_props *props)
{
	أرجع props->containing_بلاک != عدم &&
	       box__is_flex(props->containing_بلاک);
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
	ثابت css_computed_style *جڑ_style = عدم;

	أكد(ctx->n != عدم);

	box_کڈھو_properties(ctx->n, &props);

	إذا (props.containing_بلاک != عدم) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_بلاک->flags &= ~PRE_STRIP;
	}

	إذا (props.گرہ_is_جڑ == false) {
		جڑ_style = ctx->جڑ_box->style;
	}

	styles = box_get_style(ctx->content, props.ماپا_style, جڑ_style,
			ctx->n);
	إذا (styles == عدم)
		أرجع false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (title0 != عدم) {
		حرف *t = squash_whitespace(dom_string_انکڑے(title0));

		dom_string_unref(title0);

		إذا (t == عدم)
			أرجع false;

		props.title = talloc_strdup(ctx->bctx, t);

		حرر(t);

		إذا (props.title == عدم)
			أرجع false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (s != عدم) {
		err = dom_string_intern(s, &id);
		إذا (err != DOM_NO_ERR)
			id = عدم;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.نشانہ, props.title, id,
			ctx->bctx);
	إذا (box == عدم)
		أرجع false;

	/* If this is the root box, add it to the context */
	إذا (props.گرہ_is_جڑ)
		ctx->جڑ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (s != عدم) {
		ثابت حرف *val = dom_string_انکڑے(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		إذا ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, عدم, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (s != عدم) {
		ثابت حرف *val = dom_string_انکڑے(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		إذا ('0' <= val[0] && val[0] <= '9')
			box->صفاں = clamp(strtol(val, عدم, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	إذا ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} وإلا إذا (props.گرہ_is_جڑ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_بلاک;
	} وإلا {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.گرہ_is_جڑ)];

		إذا (props.containing_بلاک->type == BOX_FLEX ||
		    props.containing_بلاک->type == BOX_INLINE_FLEX) {
			/* Blockification */
			بدل (box->type) {
			حالة BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				اكسر;
			حالة BOX_INLINE_بلاک:
				box->type = BOX_بلاک;
				اكسر;
			افتراضي:
				اكسر;
			}
		}
	}

	إذا (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		أرجع false;
	}

	/* Handle the :before pseudo element */
	إذا (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	إذا (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.گرہ_is_جڑ) == CSS_DISPLAY_NONE &&
			props.گرہ_is_جڑ == false)) {
		css_select_results_destroy(styles);
		box->styles = عدم;
		box->style = عدم;

		/* Invalidate associated gadget, if any */
		إذا (box->gadget != عدم) {
			box->gadget->box = عدم;
			box->gadget = عدم;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		أرجع true;
	}

	/* Attach DOM node to box */
	err = dom_گرہ_سیٹ_user_انکڑے(ctx->n,
			corestring_dom_ns_کلید_box_گرہ_انکڑے, box, عدم,
			(فراغ *) &old_box);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	/* Attach box to DOM node */
	box->گرہ = dom_گرہ_ref(ctx->n);

	إذا (props.inline_container == عدم &&
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
		أكد(props.containing_بلاک != عدم &&
				"Box must have containing block.");

		props.inline_container = box_create(عدم, عدم, false, عدم,
				عدم, عدم, عدم, ctx->bctx);
		إذا (props.inline_container == عدم)
			أرجع false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_جوڑو_بچہ(props.containing_بلاک, props.inline_container);
	}

	/* Kick off fetch for any background image */
	إذا (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != عدم &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_انکڑے(bgimage_uri), &url);
		إذا (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			إذا (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				أرجع false;
			}
			nsurl_unref(url);
		}
	}

	إذا (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	إذا (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_بلاک) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		أكد(props.inline_container != عدم);

		box_جوڑو_بچہ(props.inline_container, box);
	} وإلا {
		إذا (ns_computed_display(box->style, props.گرہ_is_جڑ) ==
				CSS_DISPLAY_LIST_شے) {
			/* List item: compute marker */
			إذا (box_construct_marker(box, props.title, ctx,
					props.containing_بلاک) == false)
				أرجع false;
		}

		إذا (props.گرہ_is_جڑ == false &&
				box_containing_بلاک_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_بایاں ||
				css_computed_float(box->style) ==
				CSS_FLOAT_دایاں)) {
			/* Float: insert a float between the parent and box. */
			بنية box *flt = box_create(عدم, عدم, false,
					props.href, props.نشانہ, props.title,
					عدم, ctx->bctx);
			إذا (flt == عدم)
				أرجع false;

			إذا (css_computed_float(box->style) == CSS_FLOAT_بایاں)
				flt->type = BOX_FLOAT_بایاں;
			وإلا
				flt->type = BOX_FLOAT_دایاں;

			box_جوڑو_بچہ(props.inline_container, flt);
			box_جوڑو_بچہ(flt, box);
		} وإلا {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			إذا (props.containing_بلاک != عدم)
				box_جوڑو_بچہ(props.containing_بلاک, box);
		}
	}

	أرجع true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ساكن فراغ box_construct_element_after(dom_گرہ *n, html_content *content)
{
	بنية box_construct_props props;
	بنية box *box = box_for_گرہ(n);

	أكد(box != عدم);

	box_کڈھو_properties(n, &props);

	إذا (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		بنية box *inline_سرا;
		bool has_children;
		dom_exception err;

		err = dom_گرہ_has_بچہ_nodes(n, &has_children);
		إذا (err != DOM_NO_ERR)
			أرجع;

		إذا (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			أرجع;
		}

		إذا (props.inline_container == عدم) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(عدم, عدم, false,
					عدم, عدم, عدم, عدم, content->bctx);
			إذا (props.inline_container == عدم)
				أرجع;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_جوڑو_بچہ(props.containing_بلاک,
					props.inline_container);
		}

		inline_سرا = box_create(عدم, box->style, false,
				box->href, box->نشانہ, box->title,
				box->id == عدم ? عدم :
				lwc_string_ref(box->id), content->bctx);
		إذا (inline_سرا != عدم) {
			inline_سرا->type = BOX_INLINE_سرا;

			أكد(props.inline_container != عدم);

			box_جوڑو_بچہ(props.inline_container, inline_سرا);

			box->inline_سرا = inline_سرا;
			inline_سرا->inline_سرا = box;
		}
	} وإلا إذا (!(box->flags & IS_REPLACED)) {
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
ساكن dom_گرہ *
اگلا_گرہ(dom_گرہ *n, html_content *content, bool convert_children)
{
	dom_گرہ *اگلا = عدم;
	bool has_children;
	dom_exception err;

	err = dom_گرہ_has_بچہ_nodes(n, &has_children);
	إذا (err != DOM_NO_ERR) {
		dom_گرہ_unref(n);
		أرجع عدم;
	}

	إذا (convert_children && has_children) {
		err = dom_گرہ_get_پہلا_بچہ(n, &اگلا);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع عدم;
		}
		dom_گرہ_unref(n);
	} وإلا {
		err = dom_گرہ_get_اگلا_sibling(n, &اگلا);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع عدم;
		}

		إذا (اگلا != عدم) {
			إذا (box_for_گرہ(n) != عدم)
				box_construct_element_after(n, content);
			dom_گرہ_unref(n);
		} وإلا {
			إذا (box_for_گرہ(n) != عدم)
				box_construct_element_after(n, content);

			طالما (box_is_جڑ(n) == false) {
				dom_گرہ *ماپا = عدم;
				dom_گرہ *ماپا_اگلا = عدم;

				err = dom_گرہ_get_ماپا_گرہ(n, &ماپا);
				إذا (err != DOM_NO_ERR) {
					dom_گرہ_unref(n);
					أرجع عدم;
				}

				أكد(ماپا != عدم);

				err = dom_گرہ_get_اگلا_sibling(ماپا,
						&ماپا_اگلا);
				إذا (err != DOM_NO_ERR) {
					dom_گرہ_unref(ماپا);
					dom_گرہ_unref(n);
					أرجع عدم;
				}

				إذا (ماپا_اگلا != عدم) {
					dom_گرہ_unref(ماپا_اگلا);
					dom_گرہ_unref(ماپا);
					اكسر;
				}

				dom_گرہ_unref(n);
				n = ماپا;
				ماپا = عدم;

				إذا (box_for_گرہ(n) != عدم) {
					box_construct_element_after(
							n, content);
				}
			}

			إذا (box_is_جڑ(n) == false) {
				dom_گرہ *ماپا = عدم;

				err = dom_گرہ_get_ماپا_گرہ(n, &ماپا);
				إذا (err != DOM_NO_ERR) {
					dom_گرہ_unref(n);
					أرجع عدم;
				}

				أكد(ماپا != عدم);

				err = dom_گرہ_get_اگلا_sibling(ماپا, &اگلا);
				إذا (err != DOM_NO_ERR) {
					dom_گرہ_unref(ماپا);
					dom_گرہ_unref(n);
					أرجع عدم;
				}

				إذا (box_for_گرہ(ماپا) != عدم) {
					box_construct_element_after(ماپا,
							content);
				}

				dom_گرہ_unref(ماپا);
			}

			dom_گرہ_unref(n);
		}
	}

	أرجع اگلا;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ساكن فراغ
box_متن_transform(حرف *s, بنانشانی صحيح len, تعداد css_متن_transform_e tt)
{
	بنانشانی صحيح i;
	إذا (len == 0)
		أرجع;
	بدل (tt) {
		حالة CSS_متن_TRANSFORM_UPPERCASE:
			لكل (i = 0; i < len; ++i)
				إذا ((بنانشانی حرف) s[i] < 0x80)
					s[i] = ascii_منزل_اتلا(s[i]);
			اكسر;
		حالة CSS_متن_TRANSFORM_LOWERCASE:
			لكل (i = 0; i < len; ++i)
				إذا ((بنانشانی حرف) s[i] < 0x80)
					s[i] = ascii_منزل_ہیٹھلا(s[i]);
			اكسر;
		حالة CSS_متن_TRANSFORM_CAPITALIZE:
			إذا ((بنانشانی حرف) s[0] < 0x80)
				s[0] = ascii_منزل_اتلا(s[0]);
			لكل (i = 1; i < len; ++i)
				إذا ((بنانشانی حرف) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_منزل_اتلا(s[i]);
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
ساكن bool box_construct_متن(بنية box_construct_ctx *ctx)
{
	بنية box_construct_props props;
	بنية box *box = عدم;
	dom_string *content;
	dom_exception err;

	أكد(ctx->n != عدم);

	box_کڈھو_properties(ctx->n, &props);

	أكد(props.containing_بلاک != عدم);

	err = dom_characterdata_get_انکڑے(ctx->n, &content);
	إذا (err != DOM_NO_ERR || content == عدم)
		أرجع false;

	إذا (css_computed_white_space(props.ماپا_style) ==
			CSS_WHITE_SPACE_عام ||
			css_computed_white_space(props.ماپا_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		حرف *متن;

		متن = squash_whitespace(dom_string_انکڑے(content));

		dom_string_unref(content);

		إذا (متن == عدم)
			أرجع false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		إذا (متن[0] == ' ' && متن[1] == 0) {
			إذا (props.inline_container != عدم) {
				أكد(props.inline_container->آخری != عدم);

				props.inline_container->آخری->space =
						UNKNOWN_چوڑائی;
			}

			حرر(متن);

			أرجع true;
		}

		إذا (props.inline_container == عدم) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(عدم, عدم, false,
					عدم, عدم, عدم, عدم, ctx->bctx);
			إذا (props.inline_container == عدم) {
				حرر(متن);
				أرجع false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_جوڑو_بچہ(props.containing_بلاک,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(عدم,
				(css_computed_style *) props.ماپا_style,
				false, props.href, props.نشانہ, props.title,
				عدم, ctx->bctx);
		إذا (box == عدم) {
			حرر(متن);
			أرجع false;
		}

		box->type = BOX_متن;

		box->متن = talloc_strdup(ctx->bctx, متن);
		حرر(متن);
		إذا (box->متن == عدم)
			أرجع false;

		box->لمائی = لڑی_لمبائی(box->متن);

		/* strip ending space char off */
		إذا (box->لمائی > 1 && box->متن[box->لمائی - 1] == ' ') {
			box->space = UNKNOWN_چوڑائی;
			box->لمائی--;
		}

		إذا (css_computed_متن_transform(props.ماپا_style) !=
				CSS_متن_TRANSFORM_NONE)
			box_متن_transform(box->متن, box->لمائی,
				css_computed_متن_transform(
					props.ماپا_style));

		box_جوڑو_بچہ(props.inline_container, box);

		إذا (box->متن[0] == ' ') {
			box->لمائی--;

			memmove(box->متن, &box->متن[1], box->لمائی);

			إذا (box->prev != عدم)
				box->prev->space = UNKNOWN_چوڑائی;
		}
	} وإلا {
		/* white-space: pre */
		حرف *متن;
		size_t متن_len = dom_string_byte_لمائی(content);
		size_t i;
		حرف *current;
		تعداد css_white_space_e white_space =
				css_computed_white_space(props.ماپا_style);

		/* note: pre-wrap/pre-line are unimplemented */
		أكد(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_سطر ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		متن = حافظہ_دیو(متن_len + 1);
		dom_string_unref(content);

		إذا (متن == عدم)
			أرجع false;

		حافظہ_نقل(متن, dom_string_انکڑے(content), متن_len);
		متن[متن_len] = '\0';

		/* TODO: Handle tabs properly */
		لكل (i = 0; i < متن_len; i++)
			إذا (متن[i] == '\t')
				متن[i] = ' ';

		إذا (css_computed_متن_transform(props.ماپا_style) !=
				CSS_متن_TRANSFORM_NONE)
			box_متن_transform(متن, لڑی_لمبائی(متن),
				css_computed_متن_transform(
						props.ماپا_style));

		current = متن;

		/* swallow a single leading new line */
		إذا (props.containing_بلاک->flags & PRE_STRIP) {
			بدل (*current) {
			حالة '\n':
				current++;
				اكسر;
			حالة '\r':
				current++;
				إذا (*current == '\n')
					current++;
				اكسر;
			}
			props.containing_بلاک->flags &= ~PRE_STRIP;
		}

		افعل {
			size_t len = strcspn(current, "\r\n");

			حرف old = current[len];

			current[len] = 0;

			إذا (props.inline_container == عدم) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(عدم, عدم,
						false, عدم, عدم, عدم, عدم,
						ctx->bctx);
				إذا (props.inline_container == عدم) {
					حرر(متن);
					أرجع false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_جوڑو_بچہ(props.containing_بلاک,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(عدم,
				(css_computed_style *) props.ماپا_style,
				false, props.href, props.نشانہ, props.title,
				عدم, ctx->bctx);
			إذا (box == عدم) {
				حرر(متن);
				أرجع false;
			}

			box->type = BOX_متن;

			box->متن = talloc_strdup(ctx->bctx, current);
			إذا (box->متن == عدم) {
				حرر(متن);
				أرجع false;
			}

			box->لمائی = لڑی_لمبائی(box->متن);

			box_جوڑو_بچہ(props.inline_container, box);

			current[len] = old;

			current += len;

			إذا (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(عدم, عدم,
						false, عدم, عدم, عدم, عدم,
						ctx->bctx);
				إذا (props.inline_container == عدم) {
					حرر(متن);
					أرجع false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_جوڑو_بچہ(props.containing_بلاک,
						props.inline_container);

				إذا (current[0] == '\r' && current[1] == '\n')
					current += 2;
				وإلا
					current++;
			}
		} طالما (*current);

		حرر(متن);
	}

	أرجع true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ساكن فراغ convert_xml_منزل_box(بنية box_construct_ctx *ctx)
{
	dom_گرہ *اگلا;
	bool convert_children;
	uint32_t عدد_processed = 0;
	ثابت uint32_t max_processed_before_yield = 10;

	افعل {
		convert_children = true;

		أكد(ctx->n != عدم);

		إذا (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_گرہ_unref(ctx->n);
			حرر(ctx);
			أرجع;
		}

		/* Find next element to process, converting text nodes as we go */
		اگلا = اگلا_گرہ(ctx->n, ctx->content, convert_children);
		طالما (اگلا != عدم) {
			dom_گرہ_type type;
			dom_exception err;

			err = dom_گرہ_get_گرہ_type(اگلا, &type);
			إذا (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_گرہ_unref(اگلا);
				حرر(ctx);
				أرجع;
			}

			إذا (type == DOM_ELEMENT_گرہ)
				اكسر;

			إذا (type == DOM_متن_گرہ) {
				ctx->n = اگلا;
				إذا (box_construct_متن(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_گرہ_unref(ctx->n);
					حرر(ctx);
					أرجع;
				}
			}

			اگلا = اگلا_گرہ(اگلا, ctx->content, true);
		}

		ctx->n = اگلا;

		إذا (اگلا == عدم) {
			/* Conversion complete */
			بنية box جڑ;

			memset(&جڑ, 0, حجم(جڑ));

			جڑ.type = BOX_بلاک;
			جڑ.children = جڑ.آخری = ctx->جڑ_box;
			جڑ.children->ماپا = &جڑ;

			/** \todo Remove box_normalise_block */
			إذا (box_normalise_بلاک(&جڑ, ctx->جڑ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} وإلا {
				ctx->content->layout = جڑ.children;
				ctx->content->layout->ماپا = عدم;

				ctx->cb(ctx->content, true);
			}

			أكد(ctx->n == عدم);

			حرر(ctx);
			أرجع;
		}
	} طالما (++عدد_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->ویلا_سارنی(0, (فراغ *)convert_xml_منزل_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_منزل_box(dom_گرہ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   فراغ **box_conversion_context)
{
	بنية box_construct_ctx *ctx;

	أكد(box_conversion_context != عدم);

	إذا (c->bctx == عدم) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, صحيح);
		إذا (c->bctx == عدم) {
			أرجع NSERROR_NOMEM;
		}
	}

	ctx = حافظہ_دیو(حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_گرہ_ref(n);
	ctx->جڑ_box = عدم;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	أرجع guit->misc->ویلا_سارنی(0, (فراغ *)convert_xml_منزل_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_منزل_box(فراغ *box_conversion_context)
{
	بنية box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->ویلا_سارنی(-1, (فراغ *)convert_xml_منزل_box, ctx);
	إذا (err != NSERROR_OK) {
		أرجع err;
	}

	dom_گرہ_unref(ctx->n);
	حرر(ctx);

	أرجع NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
بنية box *box_for_گرہ(dom_گرہ *n)
{
	بنية box *box = عدم;
	dom_exception err;

	err = dom_گرہ_get_user_انکڑے(n, corestring_dom_ns_کلید_box_گرہ_انکڑے,
			(فراغ *) &box);
	إذا (err != DOM_NO_ERR)
		أرجع عدم;

	أرجع box;
}

/* exported function documented in html/box_construct.h */
bool
box_کڈھو_link(ثابت html_content *content,
		 ثابت dom_string *dsrel,
		 nsurl *ادھار,
		 nsurl **نتیجہ)
{
	حرف *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	بنانشانی صحيح i, j, سرا;
	nserror error;
	ثابت حرف *rel;

	rel = dom_string_انکڑے(dsrel);

	s1 = s = حافظہ_دیو(3 * لڑی_لمبائی(rel) + 1);
	إذا (!s)
		أرجع false;

	/* copy to s, removing white space and control characters */
	لكل (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	لكل (سرا = لڑی_لمبائی(rel);
	     (سرا != i) && ascii_is_space(rel[سرا - 1]);
	     سرا--)
		;
	لكل (j = 0; i != سرا; i++) {
		إذا ((بنانشانی حرف) rel[i] < 0x20) {
			; /* skip control characters */
		} وإلا إذا (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} وإلا {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	إذا (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		إذا (لڑی_مقابلہ(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			إذا (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			إذا (quot0)
				quot1 = strchr(quot0 + 1, '"');
			إذا (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} وإلا إذا (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(ادھار, s1, نتیجہ);
	حرر(s);
	إذا (error != NSERROR_OK) {
		*نتیجہ = عدم;
		أرجع false;
	}

	أرجع true;
}
