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

#include <string.h>
#include <dom/dom.h>

#include "utils/errors.h"
#include "utils/nsoption.h"
#include "utils/corestrings.h"
#include "utils/talloc.h"
#include "utils/string.h"
#include "utils/ascii.h"
#include "utils/nsurl.h"
#include "utils/utils.h"
#include "netsurf/misc.h"
#include "css/select.h"
#include "desktop/gui_internal.h"

#include "html/private.h"
#include "html/object.h"
#include "html/box.h"
#include "html/box_manipulate.h"
#include "html/box_construct.h"
#include "html/box_special.h"
#include "html/box_normalise.h"
#include "html/form_internal.h"

/**
 * Context for box tree construction
 */
sa_nracanaa box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_node *n;			/**< Current node to process */

	sa_nracanaa box *root_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	puur_naa_nka *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
sa_nracanaa box_construct_props {
	/** Style from which to inherit, or NULL if none */
	const css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	sa_nracanaa nsurl *href;
	/** Current frame target, or NULL if none */
	const var_na *target;
	/** Current title attribute, or NULL if none */
	const var_na *title;
	/** Identity of the current block-level container */
	sa_nracanaa box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	sa_nracanaa box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool node_is_root;
};

static const content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
static const box_type box_map[] = {
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
static anta_hstha bool box_is_root(dom_node *n)
{
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (parent != NULL) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
			ᬯᬮᬶ false;

		ᬬᬾᬦ᭄ (type != DOM_DOCUMENT_NODE)
			ᬯᬮᬶ false;
	}

	ᬯᬮᬶ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
static void
box_extract_properties(dom_node *n, sa_nracanaa box_construct_props *props)
{
	memset(props, 0, parimaa_na(*props));

	props->node_is_root = box_is_root(n);

	/* Extract properties from containing DOM node */
	ᬬᬾᬦ᭄ (props->node_is_root == false) {
		dom_node *current_node = n;
		dom_node *parent_node = NULL;
		sa_nracanaa box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ᬲᭂᬤᭂᬮ (true) {
			err = dom_node_get_parent_node(current_node,
					&parent_node);
			ᬬᬾᬦ᭄ (err != DOM_NO_ERR || parent_node == NULL)
				bha_nga;

			parent_box = box_for_node(parent_node);

			ᬬᬾᬦ᭄ (parent_box != NULL) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_node_unref(parent_node);
				bha_nga;
			} ᬮᬶᬬ {
				ᬬᬾᬦ᭄ (current_node != n)
					dom_node_unref(current_node);
				current_node = parent_node;
				parent_node = NULL;
			}
		}

		/* Find containing block (may be parent) */
		ᬲᭂᬤᭂᬮ (true) {
			sa_nracanaa box *b;

			err = dom_node_get_parent_node(current_node,
					&parent_node);
			ᬬᬾᬦ᭄ (err != DOM_NO_ERR || parent_node == NULL) {
				ᬬᬾᬦ᭄ (current_node != n)
					dom_node_unref(current_node);
				bha_nga;
			}

			ᬬᬾᬦ᭄ (current_node != n)
				dom_node_unref(current_node);

			b = box_for_node(parent_node);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ᬬᬾᬦ᭄ (b != NULL && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_node_unref(parent_node);
				bha_nga;
			} ᬮᬶᬬ {
				current_node = parent_node;
				parent_node = NULL;
			}
		}
	}

	/* Compute current inline container, if any */
	ᬬᬾᬦ᭄ (props->containing_block != NULL &&
			props->containing_block->last != NULL &&
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
static css_select_results *
box_get_style(html_content *c,
	      const css_computed_style *parent_style,
	      const css_computed_style *root_style,
	      dom_node *n)
{
	dom_string *s = NULL;
	css_stylesheet *inline_style = NULL;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ᬬᬾᬦ᭄ (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
			ᬯᬮᬶ NULL;
		}
	}

	ᬬᬾᬦ᭄ (s != NULL) {
		inline_style = nscss_create_inline_style(
				(const uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		ᬬᬾᬦ᭄ (inline_style == NULL)
			ᬯᬮᬶ NULL;
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
	ᬬᬾᬦ᭄ (inline_style != NULL)
		css_stylesheet_destroy(inline_style);

	ᬯᬮᬶ styles;
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
static void
box_construct_generate(dom_node *n,
		       html_content *content,
		       sa_nracanaa box *box,
		       const css_computed_style *style)
{
	sa_nracanaa box *gen = NULL;
	ga_nanaa css_display_e computed_display;
	const css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	ᬬᬾᬦ᭄ (box->type != BOX_BLOCK)
		ᬯᬮᬶ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ᬬᬾᬦ᭄ (style == NULL ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		ᬯᬮᬶ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_root(n));
	ᬬᬾᬦ᭄ (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(NULL, (css_computed_style *) style,
				false, NULL, NULL, NULL, NULL, content->bctx);
		ᬬᬾᬦ᭄ (gen == NULL) {
			ᬯᬮᬶ;
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
static bool
box_construct_marker(sa_nracanaa box *box,
		     const var_na *title,
		     sa_nracanaa box_construct_ctx *ctx,
		     sa_nracanaa box *parent)
{
	lwc_string *image_uri;
	sa_nracanaa box *marker;
	ga_nanaa css_list_style_type_e list_style_type;

	marker = box_create(NULL, box->style, false, NULL, NULL, title,
			NULL, ctx->bctx);
	ᬬᬾᬦ᭄ (marker == false)
		ᬯᬮᬶ false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	switch (list_style_type) {
	pak_sa CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (var_na *) "\342\200\242";
		marker->length = 3;
		bha_nga;

	pak_sa CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (var_na *) "\342\227\213";
		marker->length = 3;
		bha_nga;

	pak_sa CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (var_na *) "\342\226\252";
		marker->length = 3;
		bha_nga;

	default:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	pak_sa CSS_LIST_STYLE_TYPE_NONE:
		marker->text = NULL;
		marker->length = 0;
		bha_nga;
	}

	ᬬᬾᬦ᭄ (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != NULL) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		ᬬᬾᬦ᭄ (error != NSERROR_OK)
			ᬯᬮᬶ false;

		ᬬᬾᬦ᭄ (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ᬯᬮᬶ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	ᬯᬮᬶ true;
}

static anta_hstha bool box__style_is_float(const sa_nracanaa box *box)
{
	ᬯᬮᬶ css_computed_float(box->style) == CSS_FLOAT_LEFT ||
	       css_computed_float(box->style) == CSS_FLOAT_RIGHT;
}

static anta_hstha bool box__is_flex(const sa_nracanaa box *box)
{
	ᬯᬮᬶ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

static anta_hstha bool box__containing_block_is_flex(
		const sa_nracanaa box_construct_props *props)
{
	ᬯᬮᬶ props->containing_block != NULL &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
static bool
box_construct_element(sa_nracanaa box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = NULL;
	ga_nanaa css_display_e css_display;
	sa_nracanaa box *box = NULL, *old_box;
	css_select_results *styles = NULL;
	lwc_string *bgimage_uri;
	dom_exception err;
	sa_nracanaa box_construct_props props;
	const css_computed_style *root_style = NULL;

	assert(ctx->n != NULL);

	box_extract_properties(ctx->n, &props);

	ᬬᬾᬦ᭄ (props.containing_block != NULL) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	ᬬᬾᬦ᭄ (props.node_is_root == false) {
		root_style = ctx->root_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, root_style,
			ctx->n);
	ᬬᬾᬦ᭄ (styles == NULL)
		ᬯᬮᬶ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (title0 != NULL) {
		var_na *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		ᬬᬾᬦ᭄ (t == NULL)
			ᬯᬮᬶ false;

		props.title = talloc_strdup(ctx->bctx, t);

		free(t);

		ᬬᬾᬦ᭄ (props.title == NULL)
			ᬯᬮᬶ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (s != NULL) {
		err = dom_string_intern(s, &id);
		ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
			id = NULL;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	ᬬᬾᬦ᭄ (box == NULL)
		ᬯᬮᬶ false;

	/* If this is the root box, add it to the context */
	ᬬᬾᬦ᭄ (props.node_is_root)
		ctx->root_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (s != NULL) {
		const var_na *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ᬬᬾᬦ᭄ ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, NULL, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (s != NULL) {
		const var_na *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ᬬᬾᬦ᭄ ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, NULL, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ᬬᬾᬦ᭄ ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} ᬮᬶᬬ ᬬᬾᬦ᭄ (props.node_is_root) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} ᬮᬶᬬ {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.node_is_root)];

		ᬬᬾᬦ᭄ (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			switch (box->type) {
			pak_sa BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				bha_nga;
			pak_sa BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				bha_nga;
			default:
				bha_nga;
			}
		}
	}

	ᬬᬾᬦ᭄ (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ᬯᬮᬶ false;
	}

	/* Handle the :before pseudo element */
	ᬬᬾᬦ᭄ (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ᬬᬾᬦ᭄ (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.node_is_root) == CSS_DISPLAY_NONE &&
			props.node_is_root == false)) {
		css_select_results_destroy(styles);
		box->styles = NULL;
		box->style = NULL;

		/* Invalidate associated gadget, if any */
		ᬬᬾᬦ᭄ (box->gadget != NULL) {
			box->gadget->box = NULL;
			box->gadget = NULL;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ᬯᬮᬶ true;
	}

	/* Attach DOM node to box */
	err = dom_node_set_user_data(ctx->n,
			corestring_dom___ns_key_box_node_data, box, NULL,
			(void *) &old_box);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ false;

	/* Attach box to DOM node */
	box->node = dom_node_ref(ctx->n);

	ᬬᬾᬦ᭄ (props.inline_container == NULL &&
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
		assert(props.containing_block != NULL &&
				"Box must have containing block.");

		props.inline_container = box_create(NULL, NULL, false, NULL,
				NULL, NULL, NULL, ctx->bctx);
		ᬬᬾᬦ᭄ (props.inline_container == NULL)
			ᬯᬮᬶ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ᬬᬾᬦ᭄ (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != NULL &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		ᬬᬾᬦ᭄ (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ᬬᬾᬦ᭄ (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ᬯᬮᬶ false;
			}
			nsurl_unref(url);
		}
	}

	ᬬᬾᬦ᭄ (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ᬬᬾᬦ᭄ (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		assert(props.inline_container != NULL);

		box_add_child(props.inline_container, box);
	} ᬮᬶᬬ {
		ᬬᬾᬦ᭄ (ns_computed_display(box->style, props.node_is_root) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			ᬬᬾᬦ᭄ (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				ᬯᬮᬶ false;
		}

		ᬬᬾᬦ᭄ (props.node_is_root == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_LEFT ||
				css_computed_float(box->style) ==
				CSS_FLOAT_RIGHT)) {
			/* Float: insert a float between the parent and box. */
			sa_nracanaa box *flt = box_create(NULL, NULL, false,
					props.href, props.target, props.title,
					NULL, ctx->bctx);
			ᬬᬾᬦ᭄ (flt == NULL)
				ᬯᬮᬶ false;

			ᬬᬾᬦ᭄ (css_computed_float(box->style) == CSS_FLOAT_LEFT)
				flt->type = BOX_FLOAT_LEFT;
			ᬮᬶᬬ
				flt->type = BOX_FLOAT_RIGHT;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} ᬮᬶᬬ {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ᬬᬾᬦ᭄ (props.containing_block != NULL)
				box_add_child(props.containing_block, box);
		}
	}

	ᬯᬮᬶ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
static void box_construct_element_after(dom_node *n, html_content *content)
{
	sa_nracanaa box_construct_props props;
	sa_nracanaa box *box = box_for_node(n);

	assert(box != NULL);

	box_extract_properties(n, &props);

	ᬬᬾᬦ᭄ (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		sa_nracanaa box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_node_has_child_nodes(n, &has_children);
		ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
			ᬯᬮᬶ;

		ᬬᬾᬦ᭄ (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ᬯᬮᬶ;
		}

		ᬬᬾᬦ᭄ (props.inline_container == NULL) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(NULL, NULL, false,
					NULL, NULL, NULL, NULL, content->bctx);
			ᬬᬾᬦ᭄ (props.inline_container == NULL)
				ᬯᬮᬶ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(NULL, box->style, false,
				box->href, box->target, box->title,
				box->id == NULL ? NULL :
				lwc_string_ref(box->id), content->bctx);
		ᬬᬾᬦ᭄ (inline_end != NULL) {
			inline_end->type = BOX_INLINE_END;

			assert(props.inline_container != NULL);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} ᬮᬶᬬ ᬬᬾᬦ᭄ (!(box->flags & IS_REPLACED)) {
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
static dom_node *
next_node(dom_node *n, html_content *content, bool convert_children)
{
	dom_node *next = NULL;
	bool has_children;
	dom_exception err;

	err = dom_node_has_child_nodes(n, &has_children);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
		dom_node_unref(n);
		ᬯᬮᬶ NULL;
	}

	ᬬᬾᬦ᭄ (convert_children && has_children) {
		err = dom_node_get_first_child(n, &next);
		ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
			dom_node_unref(n);
			ᬯᬮᬶ NULL;
		}
		dom_node_unref(n);
	} ᬮᬶᬬ {
		err = dom_node_get_next_sibling(n, &next);
		ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
			dom_node_unref(n);
			ᬯᬮᬶ NULL;
		}

		ᬬᬾᬦ᭄ (next != NULL) {
			ᬬᬾᬦ᭄ (box_for_node(n) != NULL)
				box_construct_element_after(n, content);
			dom_node_unref(n);
		} ᬮᬶᬬ {
			ᬬᬾᬦ᭄ (box_for_node(n) != NULL)
				box_construct_element_after(n, content);

			ᬲᭂᬤᭂᬮ (box_is_root(n) == false) {
				dom_node *parent = NULL;
				dom_node *parent_next = NULL;

				err = dom_node_get_parent_node(n, &parent);
				ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
					dom_node_unref(n);
					ᬯᬮᬶ NULL;
				}

				assert(parent != NULL);

				err = dom_node_get_next_sibling(parent,
						&parent_next);
				ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
					dom_node_unref(parent);
					dom_node_unref(n);
					ᬯᬮᬶ NULL;
				}

				ᬬᬾᬦ᭄ (parent_next != NULL) {
					dom_node_unref(parent_next);
					dom_node_unref(parent);
					bha_nga;
				}

				dom_node_unref(n);
				n = parent;
				parent = NULL;

				ᬬᬾᬦ᭄ (box_for_node(n) != NULL) {
					box_construct_element_after(
							n, content);
				}
			}

			ᬬᬾᬦ᭄ (box_is_root(n) == false) {
				dom_node *parent = NULL;

				err = dom_node_get_parent_node(n, &parent);
				ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
					dom_node_unref(n);
					ᬯᬮᬶ NULL;
				}

				assert(parent != NULL);

				err = dom_node_get_next_sibling(parent, &next);
				ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
					dom_node_unref(parent);
					dom_node_unref(n);
					ᬯᬮᬶ NULL;
				}

				ᬬᬾᬦ᭄ (box_for_node(parent) != NULL) {
					box_construct_element_after(parent,
							content);
				}

				dom_node_unref(parent);
			}

			dom_node_unref(n);
		}
	}

	ᬯᬮᬶ next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
static void
box_text_transform(var_na *s, unsigned puur_naa_nka len, ga_nanaa css_text_transform_e tt)
{
	unsigned puur_naa_nka i;
	ᬬᬾᬦ᭄ (len == 0)
		ᬯᬮᬶ;
	switch (tt) {
		pak_sa CSS_TEXT_TRANSFORM_UPPERCASE:
			for (i = 0; i < len; ++i)
				ᬬᬾᬦ᭄ ((unsigned var_na) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			bha_nga;
		pak_sa CSS_TEXT_TRANSFORM_LOWERCASE:
			for (i = 0; i < len; ++i)
				ᬬᬾᬦ᭄ ((unsigned var_na) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			bha_nga;
		pak_sa CSS_TEXT_TRANSFORM_CAPITALIZE:
			ᬬᬾᬦ᭄ ((unsigned var_na) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			for (i = 1; i < len; ++i)
				ᬬᬾᬦ᭄ ((unsigned var_na) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			bha_nga;
		default:
			bha_nga;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
static bool box_construct_text(sa_nracanaa box_construct_ctx *ctx)
{
	sa_nracanaa box_construct_props props;
	sa_nracanaa box *box = NULL;
	dom_string *content;
	dom_exception err;

	assert(ctx->n != NULL);

	box_extract_properties(ctx->n, &props);

	assert(props.containing_block != NULL);

	err = dom_characterdata_get_data(ctx->n, &content);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR || content == NULL)
		ᬯᬮᬶ false;

	ᬬᬾᬦ᭄ (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		var_na *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		ᬬᬾᬦ᭄ (text == NULL)
			ᬯᬮᬶ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ᬬᬾᬦ᭄ (text[0] == ' ' && text[1] == 0) {
			ᬬᬾᬦ᭄ (props.inline_container != NULL) {
				assert(props.inline_container->last != NULL);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			free(text);

			ᬯᬮᬶ true;
		}

		ᬬᬾᬦ᭄ (props.inline_container == NULL) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(NULL, NULL, false,
					NULL, NULL, NULL, NULL, ctx->bctx);
			ᬬᬾᬦ᭄ (props.inline_container == NULL) {
				free(text);
				ᬯᬮᬶ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(NULL,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				NULL, ctx->bctx);
		ᬬᬾᬦ᭄ (box == NULL) {
			free(text);
			ᬯᬮᬶ false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		free(text);
		ᬬᬾᬦ᭄ (box->text == NULL)
			ᬯᬮᬶ false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		ᬬᬾᬦ᭄ (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		ᬬᬾᬦ᭄ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		ᬬᬾᬦ᭄ (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			ᬬᬾᬦ᭄ (box->prev != NULL)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} ᬮᬶᬬ {
		/* white-space: pre */
		var_na *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		var_na *current;
		ga_nanaa css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		assert(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_LINE ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		ᬬᬾᬦ᭄ (text == NULL)
			ᬯᬮᬶ false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		for (i = 0; i < text_len; i++)
			ᬬᬾᬦ᭄ (text[i] == '\t')
				text[i] = ' ';

		ᬬᬾᬦ᭄ (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		ᬬᬾᬦ᭄ (props.containing_block->flags & PRE_STRIP) {
			switch (*current) {
			pak_sa '\n':
				current++;
				bha_nga;
			pak_sa '\r':
				current++;
				ᬬᬾᬦ᭄ (*current == '\n')
					current++;
				bha_nga;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		do {
			size_t len = strcspn(current, "\r\n");

			var_na old = current[len];

			current[len] = 0;

			ᬬᬾᬦ᭄ (props.inline_container == NULL) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(NULL, NULL,
						false, NULL, NULL, NULL, NULL,
						ctx->bctx);
				ᬬᬾᬦ᭄ (props.inline_container == NULL) {
					free(text);
					ᬯᬮᬶ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(NULL,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				NULL, ctx->bctx);
			ᬬᬾᬦ᭄ (box == NULL) {
				free(text);
				ᬯᬮᬶ false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			ᬬᬾᬦ᭄ (box->text == NULL) {
				free(text);
				ᬯᬮᬶ false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			ᬬᬾᬦ᭄ (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(NULL, NULL,
						false, NULL, NULL, NULL, NULL,
						ctx->bctx);
				ᬬᬾᬦ᭄ (props.inline_container == NULL) {
					free(text);
					ᬯᬮᬶ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				ᬬᬾᬦ᭄ (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ᬮᬶᬬ
					current++;
			}
		} ᬲᭂᬤᭂᬮ (*current);

		free(text);
	}

	ᬯᬮᬶ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
static void convert_xml_to_box(sa_nracanaa box_construct_ctx *ctx)
{
	dom_node *next;
	bool convert_children;
	uint32_t num_processed = 0;
	const uint32_t max_processed_before_yield = 10;

	do {
		convert_children = true;

		assert(ctx->n != NULL);

		ᬬᬾᬦ᭄ (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_node_unref(ctx->n);
			free(ctx);
			ᬯᬮᬶ;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_node(ctx->n, ctx->content, convert_children);
		ᬲᭂᬤᭂᬮ (next != NULL) {
			dom_node_type type;
			dom_exception err;

			err = dom_node_get_node_type(next, &type);
			ᬬᬾᬦ᭄ (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_node_unref(next);
				free(ctx);
				ᬯᬮᬶ;
			}

			ᬬᬾᬦ᭄ (type == DOM_ELEMENT_NODE)
				bha_nga;

			ᬬᬾᬦ᭄ (type == DOM_TEXT_NODE) {
				ctx->n = next;
				ᬬᬾᬦ᭄ (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_node_unref(ctx->n);
					free(ctx);
					ᬯᬮᬶ;
				}
			}

			next = next_node(next, ctx->content, true);
		}

		ctx->n = next;

		ᬬᬾᬦ᭄ (next == NULL) {
			/* Conversion complete */
			sa_nracanaa box root;

			memset(&root, 0, parimaa_na(root));

			root.type = BOX_BLOCK;
			root.children = root.last = ctx->root_box;
			root.children->parent = &root;

			/** \todo Remove box_normalise_block */
			ᬬᬾᬦ᭄ (box_normalise_block(&root, ctx->root_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ᬮᬶᬬ {
				ctx->content->layout = root.children;
				ctx->content->layout->parent = NULL;

				ctx->cb(ctx->content, true);
			}

			assert(ctx->n == NULL);

			free(ctx);
			ᬯᬮᬶ;
		}
	} ᬲᭂᬤᭂᬮ (++num_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (void *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_node *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   void **box_conversion_context)
{
	sa_nracanaa box_construct_ctx *ctx;

	assert(box_conversion_context != NULL);

	ᬬᬾᬦ᭄ (c->bctx == NULL) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, puur_naa_nka);
		ᬬᬾᬦ᭄ (c->bctx == NULL) {
			ᬯᬮᬶ NSERROR_NOMEM;
		}
	}

	ctx = malloc(parimaa_na(*ctx));
	ᬬᬾᬦ᭄ (ctx == NULL) {
		ᬯᬮᬶ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_node_ref(n);
	ctx->root_box = NULL;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ᬯᬮᬶ guit->misc->schedule(0, (void *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(void *box_conversion_context)
{
	sa_nracanaa box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (void *)convert_xml_to_box, ctx);
	ᬬᬾᬦ᭄ (err != NSERROR_OK) {
		ᬯᬮᬶ err;
	}

	dom_node_unref(ctx->n);
	free(ctx);

	ᬯᬮᬶ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
sa_nracanaa box *box_for_node(dom_node *n)
{
	sa_nracanaa box *box = NULL;
	dom_exception err;

	err = dom_node_get_user_data(n, corestring_dom___ns_key_box_node_data,
			(void *) &box);
	ᬬᬾᬦ᭄ (err != DOM_NO_ERR)
		ᬯᬮᬶ NULL;

	ᬯᬮᬶ box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(const html_content *content,
		 const dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	var_na *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned puur_naa_nka i, j, end;
	nserror error;
	const var_na *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	ᬬᬾᬦ᭄ (!s)
		ᬯᬮᬶ false;

	/* copy to s, removing white space and control characters */
	for (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	for (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	for (j = 0; i != end; i++) {
		ᬬᬾᬦ᭄ ((unsigned var_na) rel[i] < 0x20) {
			; /* skip control characters */
		} ᬮᬶᬬ ᬬᬾᬦ᭄ (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ᬮᬶᬬ {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ᬬᬾᬦ᭄ (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ᬬᬾᬦ᭄ (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ᬬᬾᬦ᭄ (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ᬬᬾᬦ᭄ (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ᬬᬾᬦ᭄ (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ᬮᬶᬬ ᬬᬾᬦ᭄ (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	free(s);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		*result = NULL;
		ᬯᬮᬶ false;
	}

	ᬯᬮᬶ true;
}
