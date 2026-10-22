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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/string.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "css/select.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/object.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_manipulate.h"
#ଅନ୍ତର୍ଭାଵ "html/box_construct.h"
#ଅନ୍ତର୍ଭାଵ "html/box_special.h"
#ଅନ୍ତର୍ଭାଵ "html/box_normalise.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"

/**
 * Context for box tree construction
 */
ସଂରଚନା box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_ଗଣ୍ଠି *n;			/**< Current node to process */

	ସଂରଚନା box *ମୂଳ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
ସଂରଚନା box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ଅଚଳ css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	ସଂରଚନା nsurl *href;
	/** Current frame target, or NULL if none */
	ଅଚଳ ଅକ୍ଷର *target;
	/** Current title attribute, or NULL if none */
	ଅଚଳ ଅକ୍ଷର *title;
	/** Identity of the current block-level container */
	ସଂରଚନା box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	ସଂରଚନା box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ଗଣ୍ଠି_is_ମୂଳ;
};

ସ୍ଥିର ଅଚଳ content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
ସ୍ଥିର ଅଚଳ box_type box_map[] = {
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box_is_ମୂଳ(dom_ଗଣ୍ଠି *n)
{
	dom_ଗଣ୍ଠି *parent;
	dom_ଗଣ୍ଠି_type type;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(n, &parent);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (parent != ଖାଲି) {
		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(parent, &type);

		dom_ଗଣ୍ଠି_unref(parent);

		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		ଯଦି (type != DOM_DOCUMENT_ଗଣ୍ଠି)
			ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
ସ୍ଥିର ଶୂନ୍ୟ
box_extract_properties(dom_ଗଣ୍ଠି *n, ସଂରଚନା box_construct_props *props)
{
	memset(props, 0, ଆକାର(*props));

	props->ଗଣ୍ଠି_is_ମୂଳ = box_is_ମୂଳ(n);

	/* Extract properties from containing DOM node */
	ଯଦି (props->ଗଣ୍ଠି_is_ମୂଳ == false) {
		dom_ଗଣ୍ଠି *current_ଗଣ୍ଠି = n;
		dom_ଗଣ୍ଠି *parent_ଗଣ୍ଠି = ଖାଲି;
		ସଂରଚନା box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		ଯେତେବେଳେ (true) {
			err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(current_ଗଣ୍ଠି,
					&parent_ଗଣ୍ଠି);
			ଯଦି (err != DOM_NO_ERR || parent_ଗଣ୍ଠି == ଖାଲି)
				ଭାଙ୍ଗ;

			parent_box = box_for_ଗଣ୍ଠି(parent_ଗଣ୍ଠି);

			ଯଦି (parent_box != ଖାଲି) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_ଗଣ୍ଠି_unref(parent_ଗଣ୍ଠି);
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				ଯଦି (current_ଗଣ୍ଠି != n)
					dom_ଗଣ୍ଠି_unref(current_ଗଣ୍ଠି);
				current_ଗଣ୍ଠି = parent_ଗଣ୍ଠି;
				parent_ଗଣ୍ଠି = ଖାଲି;
			}
		}

		/* Find containing block (may be parent) */
		ଯେତେବେଳେ (true) {
			ସଂରଚନା box *b;

			err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(current_ଗଣ୍ଠି,
					&parent_ଗଣ୍ଠି);
			ଯଦି (err != DOM_NO_ERR || parent_ଗଣ୍ଠି == ଖାଲି) {
				ଯଦି (current_ଗଣ୍ଠି != n)
					dom_ଗଣ୍ଠି_unref(current_ଗଣ୍ଠି);
				ଭାଙ୍ଗ;
			}

			ଯଦି (current_ଗଣ୍ଠି != n)
				dom_ଗଣ୍ଠି_unref(current_ଗଣ୍ଠି);

			b = box_for_ଗଣ୍ଠି(parent_ଗଣ୍ଠି);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			ଯଦି (b != ଖାଲି && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_ଗଣ୍ଠି_unref(parent_ଗଣ୍ଠି);
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				current_ଗଣ୍ଠି = parent_ଗଣ୍ଠି;
				parent_ଗଣ୍ଠି = ଖାଲି;
			}
		}
	}

	/* Compute current inline container, if any */
	ଯଦି (props->containing_block != ଖାଲି &&
			props->containing_block->last != ଖାଲି &&
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
ସ୍ଥିର css_select_results *
box_get_style(html_content *c,
	      ଅଚଳ css_computed_style *parent_style,
	      ଅଚଳ css_computed_style *ମୂଳ_style,
	      dom_ଗଣ୍ଠି *n)
{
	dom_string *s = ଖାଲି;
	css_stylesheet *inline_style = ଖାଲି;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	ଯଦି (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		ଯଦି (err != DOM_NO_ERR) {
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	ଯଦି (s != ଖାଲି) {
		inline_style = nscss_create_inline_style(
				(ଅଚଳ uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		ଯଦି (inline_style == ଖାଲି)
			ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.ମୂଳ_style = ମୂଳ_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	ଯଦି (inline_style != ଖାଲି)
		css_stylesheet_destroy(inline_style);

	ଫେରନ୍ତୁ styles;
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
ସ୍ଥିର ଶୂନ୍ୟ
box_construct_generate(dom_ଗଣ୍ଠି *n,
		       html_content *content,
		       ସଂରଚନା box *box,
		       ଅଚଳ css_computed_style *style)
{
	ସଂରଚନା box *gen = ଖାଲି;
	ଗଣନା css_display_e computed_display;
	ଅଚଳ css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	ଯଦି (box->type != BOX_BLOCK)
		ଫେରନ୍ତୁ;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	ଯଦି (style == ଖାଲି ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		ଫେରନ୍ତୁ;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_ମୂଳ(n));
	ଯଦି (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(ଖାଲି, (css_computed_style *) style,
				false, ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି, content->bctx);
		ଯଦି (gen == ଖାଲି) {
			ଫେରନ୍ତୁ;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_ମୂଳ(n))];

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
ସ୍ଥିର bool
box_construct_marker(ସଂରଚନା box *box,
		     ଅଚଳ ଅକ୍ଷର *title,
		     ସଂରଚନା box_construct_ctx *ctx,
		     ସଂରଚନା box *parent)
{
	lwc_string *image_uri;
	ସଂରଚନା box *marker;
	ଗଣନା css_list_style_type_e list_style_type;

	marker = box_create(ଖାଲି, box->style, false, ଖାଲି, ଖାଲି, title,
			ଖାଲି, ctx->bctx);
	ଯଦି (marker == false)
		ଫେରନ୍ତୁ false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	ବାଛ (list_style_type) {
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (ଅକ୍ଷର *) "\342\200\242";
		marker->length = 3;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (ଅକ୍ଷର *) "\342\227\213";
		marker->length = 3;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (ଅକ୍ଷର *) "\342\226\252";
		marker->length = 3;
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_NONE:
		marker->text = ଖାଲି;
		marker->length = 0;
		ଭାଙ୍ଗ;
	}

	ଯଦି (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != ଖାଲି) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		ଯଦି (error != NSERROR_OK)
			ଫେରନ୍ତୁ false;

		ଯଦି (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			ଫେରନ୍ତୁ false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box__style_is_float(ଅଚଳ ସଂରଚନା box *box)
{
	ଫେରନ୍ତୁ css_computed_float(box->style) == CSS_FLOAT_ବାମ ||
	       css_computed_float(box->style) == CSS_FLOAT_ଡାହାଣ;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box__is_flex(ଅଚଳ ସଂରଚନା box *box)
{
	ଫେରନ୍ତୁ box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box__containing_block_is_flex(
		ଅଚଳ ସଂରଚନା box_construct_props *props)
{
	ଫେରନ୍ତୁ props->containing_block != ଖାଲି &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
ସ୍ଥିର bool
box_construct_element(ସଂରଚନା box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = ଖାଲି;
	ଗଣନା css_display_e css_display;
	ସଂରଚନା box *box = ଖାଲି, *old_box;
	css_select_results *styles = ଖାଲି;
	lwc_string *bgimage_uri;
	dom_exception err;
	ସଂରଚନା box_construct_props props;
	ଅଚଳ css_computed_style *ମୂଳ_style = ଖାଲି;

	ପ୍ରତିଜ୍ଞା(ctx->n != ଖାଲି);

	box_extract_properties(ctx->n, &props);

	ଯଦି (props.containing_block != ଖାଲି) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	ଯଦି (props.ଗଣ୍ଠି_is_ମୂଳ == false) {
		ମୂଳ_style = ctx->ମୂଳ_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, ମୂଳ_style,
			ctx->n);
	ଯଦି (styles == ଖାଲି)
		ଫେରନ୍ତୁ false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (title0 != ଖାଲି) {
		ଅକ୍ଷର *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		ଯଦି (t == ଖାଲି)
			ଫେରନ୍ତୁ false;

		props.title = talloc_strdup(ctx->bctx, t);

		ମୁକ୍ତ(t);

		ଯଦି (props.title == ଖାଲି)
			ଫେରନ୍ତୁ false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (s != ଖାଲି) {
		err = dom_string_intern(s, &id);
		ଯଦି (err != DOM_NO_ERR)
			id = ଖାଲି;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	ଯଦି (box == ଖାଲି)
		ଫେରନ୍ତୁ false;

	/* If this is the root box, add it to the context */
	ଯଦି (props.ଗଣ୍ଠି_is_ମୂଳ)
		ctx->ମୂଳ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (s != ଖାଲି) {
		ଅଚଳ ଅକ୍ଷର *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		ଯଦି ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, ଖାଲି, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (s != ଖାଲି) {
		ଅଚଳ ଅକ୍ଷର *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		ଯଦି ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(strtol(val, ଖାଲି, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	ଯଦି ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} ଅନ୍ୟଥା ଯଦି (props.ଗଣ୍ଠି_is_ମୂଳ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} ଅନ୍ୟଥା {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ଗଣ୍ଠି_is_ମୂଳ)];

		ଯଦି (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			ବାଛ (box->type) {
			ସ୍ଥିତି BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
			}
		}
	}

	ଯଦି (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		ଫେରନ୍ତୁ false;
	}

	/* Handle the :before pseudo element */
	ଯଦି (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	ଯଦି (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ଗଣ୍ଠି_is_ମୂଳ) == CSS_DISPLAY_NONE &&
			props.ଗଣ୍ଠି_is_ମୂଳ == false)) {
		css_select_results_destroy(styles);
		box->styles = ଖାଲି;
		box->style = ଖାଲି;

		/* Invalidate associated gadget, if any */
		ଯଦି (box->gadget != ଖାଲି) {
			box->gadget->box = ଖାଲି;
			box->gadget = ଖାଲି;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		ଫେରନ୍ତୁ true;
	}

	/* Attach DOM node to box */
	err = dom_ଗଣ୍ଠି_set_user_data(ctx->n,
			corestring_dom_ns_ଚାବି_box_ଗଣ୍ଠି_data, box, ଖାଲି,
			(ଶୂନ୍ୟ *) &old_box);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	/* Attach box to DOM node */
	box->ଗଣ୍ଠି = dom_ଗଣ୍ଠି_ref(ctx->n);

	ଯଦି (props.inline_container == ଖାଲି &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.ଗଣ୍ଠି_is_ମୂଳ == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		ପ୍ରତିଜ୍ଞା(props.containing_block != ଖାଲି &&
				"Box must have containing block.");

		props.inline_container = box_create(ଖାଲି, ଖାଲି, false, ଖାଲି,
				ଖାଲି, ଖାଲି, ଖାଲି, ctx->bctx);
		ଯଦି (props.inline_container == ଖାଲି)
			ଫେରନ୍ତୁ false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	ଯଦି (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != ଖାଲି &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		ଯଦି (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			ଯଦି (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				ଫେରନ୍ତୁ false;
			}
			nsurl_unref(url);
		}
	}

	ଯଦି (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	ଯଦି (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		ପ୍ରତିଜ୍ଞା(props.inline_container != ଖାଲି);

		box_add_child(props.inline_container, box);
	} ଅନ୍ୟଥା {
		ଯଦି (ns_computed_display(box->style, props.ଗଣ୍ଠି_is_ମୂଳ) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			ଯଦି (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				ଫେରନ୍ତୁ false;
		}

		ଯଦି (props.ଗଣ୍ଠି_is_ମୂଳ == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_ବାମ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_ଡାହାଣ)) {
			/* Float: insert a float between the parent and box. */
			ସଂରଚନା box *flt = box_create(ଖାଲି, ଖାଲି, false,
					props.href, props.target, props.title,
					ଖାଲି, ctx->bctx);
			ଯଦି (flt == ଖାଲି)
				ଫେରନ୍ତୁ false;

			ଯଦି (css_computed_float(box->style) == CSS_FLOAT_ବାମ)
				flt->type = BOX_FLOAT_ବାମ;
			ଅନ୍ୟଥା
				flt->type = BOX_FLOAT_ଡାହାଣ;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} ଅନ୍ୟଥା {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			ଯଦି (props.containing_block != ଖାଲି)
				box_add_child(props.containing_block, box);
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
ସ୍ଥିର ଶୂନ୍ୟ box_construct_element_after(dom_ଗଣ୍ଠି *n, html_content *content)
{
	ସଂରଚନା box_construct_props props;
	ସଂରଚନା box *box = box_for_ଗଣ୍ଠି(n);

	ପ୍ରତିଜ୍ଞା(box != ଖାଲି);

	box_extract_properties(n, &props);

	ଯଦି (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		ସଂରଚନା box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_ଗଣ୍ଠି_has_child_nodes(n, &has_children);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ;

		ଯଦି (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			ଫେରନ୍ତୁ;
		}

		ଯଦି (props.inline_container == ଖାଲି) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(ଖାଲି, ଖାଲି, false,
					ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି, content->bctx);
			ଯଦି (props.inline_container == ଖାଲି)
				ଫେରନ୍ତୁ;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(ଖାଲି, box->style, false,
				box->href, box->target, box->title,
				box->id == ଖାଲି ? ଖାଲି :
				lwc_string_ref(box->id), content->bctx);
		ଯଦି (inline_end != ଖାଲି) {
			inline_end->type = BOX_INLINE_END;

			ପ୍ରତିଜ୍ଞା(props.inline_container != ଖାଲି);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} ଅନ୍ୟଥା ଯଦି (!(box->flags & IS_REPLACED)) {
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
ସ୍ଥିର dom_ଗଣ୍ଠି *
next_ଗଣ୍ଠି(dom_ଗଣ୍ଠି *n, html_content *content, bool convert_children)
{
	dom_ଗଣ୍ଠି *next = ଖାଲି;
	bool has_children;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_has_child_nodes(n, &has_children);
	ଯଦି (err != DOM_NO_ERR) {
		dom_ଗଣ୍ଠି_unref(n);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (convert_children && has_children) {
		err = dom_ଗଣ୍ଠି_get_first_child(n, &next);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		dom_ଗଣ୍ଠି_unref(n);
	} ଅନ୍ୟଥା {
		err = dom_ଗଣ୍ଠି_get_next_sibling(n, &next);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		ଯଦି (next != ଖାଲି) {
			ଯଦି (box_for_ଗଣ୍ଠି(n) != ଖାଲି)
				box_construct_element_after(n, content);
			dom_ଗଣ୍ଠି_unref(n);
		} ଅନ୍ୟଥା {
			ଯଦି (box_for_ଗଣ୍ଠି(n) != ଖାଲି)
				box_construct_element_after(n, content);

			ଯେତେବେଳେ (box_is_ମୂଳ(n) == false) {
				dom_ଗଣ୍ଠି *parent = ଖାଲି;
				dom_ଗଣ୍ଠି *parent_next = ଖାଲି;

				err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(n, &parent);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(n);
					ଫେରନ୍ତୁ ଖାଲି;
				}

				ପ୍ରତିଜ୍ଞା(parent != ଖାଲି);

				err = dom_ଗଣ୍ଠି_get_next_sibling(parent,
						&parent_next);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(parent);
					dom_ଗଣ୍ଠି_unref(n);
					ଫେରନ୍ତୁ ଖାଲି;
				}

				ଯଦି (parent_next != ଖାଲି) {
					dom_ଗଣ୍ଠି_unref(parent_next);
					dom_ଗଣ୍ଠି_unref(parent);
					ଭାଙ୍ଗ;
				}

				dom_ଗଣ୍ଠି_unref(n);
				n = parent;
				parent = ଖାଲି;

				ଯଦି (box_for_ଗଣ୍ଠି(n) != ଖାଲି) {
					box_construct_element_after(
							n, content);
				}
			}

			ଯଦି (box_is_ମୂଳ(n) == false) {
				dom_ଗଣ୍ଠି *parent = ଖାଲି;

				err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(n, &parent);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(n);
					ଫେରନ୍ତୁ ଖାଲି;
				}

				ପ୍ରତିଜ୍ଞା(parent != ଖାଲି);

				err = dom_ଗଣ୍ଠି_get_next_sibling(parent, &next);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(parent);
					dom_ଗଣ୍ଠି_unref(n);
					ଫେରନ୍ତୁ ଖାଲି;
				}

				ଯଦି (box_for_ଗଣ୍ଠି(parent) != ଖାଲି) {
					box_construct_element_after(parent,
							content);
				}

				dom_ଗଣ୍ଠି_unref(parent);
			}

			dom_ଗଣ୍ଠି_unref(n);
		}
	}

	ଫେରନ୍ତୁ next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
ସ୍ଥିର ଶୂନ୍ୟ
box_text_transform(ଅକ୍ଷର *s, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ଗଣନା css_text_transform_e tt)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଯଦି (len == 0)
		ଫେରନ୍ତୁ;
	ବାଛ (tt) {
		ସ୍ଥିତି CSS_TEXT_TRANSFORM_UPPERCASE:
			ପାଇଁ (i = 0; i < len; ++i)
				ଯଦି ((unsigned ଅକ୍ଷର) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_TEXT_TRANSFORM_LOWERCASE:
			ପାଇଁ (i = 0; i < len; ++i)
				ଯଦି ((unsigned ଅକ୍ଷର) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_TEXT_TRANSFORM_CAPITALIZE:
			ଯଦି ((unsigned ଅକ୍ଷର) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			ପାଇଁ (i = 1; i < len; ++i)
				ଯଦି ((unsigned ଅକ୍ଷର) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
ସ୍ଥିର bool box_construct_text(ସଂରଚନା box_construct_ctx *ctx)
{
	ସଂରଚନା box_construct_props props;
	ସଂରଚନା box *box = ଖାଲି;
	dom_string *content;
	dom_exception err;

	ପ୍ରତିଜ୍ଞା(ctx->n != ଖାଲି);

	box_extract_properties(ctx->n, &props);

	ପ୍ରତିଜ୍ଞା(props.containing_block != ଖାଲି);

	err = dom_characterdata_get_data(ctx->n, &content);
	ଯଦି (err != DOM_NO_ERR || content == ଖାଲି)
		ଫେରନ୍ତୁ false;

	ଯଦି (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		ଅକ୍ଷର *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		ଯଦି (text == ଖାଲି)
			ଫେରନ୍ତୁ false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		ଯଦି (text[0] == ' ' && text[1] == 0) {
			ଯଦି (props.inline_container != ଖାଲି) {
				ପ୍ରତିଜ୍ଞା(props.inline_container->last != ଖାଲି);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			ମୁକ୍ତ(text);

			ଫେରନ୍ତୁ true;
		}

		ଯଦି (props.inline_container == ଖାଲି) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(ଖାଲି, ଖାଲି, false,
					ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି, ctx->bctx);
			ଯଦି (props.inline_container == ଖାଲି) {
				ମୁକ୍ତ(text);
				ଫେରନ୍ତୁ false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(ଖାଲି,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ଖାଲି, ctx->bctx);
		ଯଦି (box == ଖାଲି) {
			ମୁକ୍ତ(text);
			ଫେରନ୍ତୁ false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		ମୁକ୍ତ(text);
		ଯଦି (box->text == ଖାଲି)
			ଫେରନ୍ତୁ false;

		box->length = strlen(box->text);

		/* strip ending space char off */
		ଯଦି (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		ଯଦି (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		ଯଦି (box->text[0] == ' ') {
			box->length--;

			memmove(box->text, &box->text[1], box->length);

			ଯଦି (box->prev != ଖାଲି)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} ଅନ୍ୟଥା {
		/* white-space: pre */
		ଅକ୍ଷର *text;
		size_t text_len = dom_string_byte_length(content);
		size_t i;
		ଅକ୍ଷର *current;
		ଗଣନା css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		ପ୍ରତିଜ୍ଞା(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_ଧାଡ଼ି ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = malloc(text_len + 1);
		dom_string_unref(content);

		ଯଦି (text == ଖାଲି)
			ଫେରନ୍ତୁ false;

		memcpy(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		ପାଇଁ (i = 0; i < text_len; i++)
			ଯଦି (text[i] == '\t')
				text[i] = ' ';

		ଯଦି (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, strlen(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		ଯଦି (props.containing_block->flags & PRE_STRIP) {
			ବାଛ (*current) {
			ସ୍ଥିତି '\n':
				current++;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\r':
				current++;
				ଯଦି (*current == '\n')
					current++;
				ଭାଙ୍ଗ;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		କର {
			size_t len = strcspn(current, "\r\n");

			ଅକ୍ଷର old = current[len];

			current[len] = 0;

			ଯଦି (props.inline_container == ଖାଲି) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(ଖାଲି, ଖାଲି,
						false, ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି,
						ctx->bctx);
				ଯଦି (props.inline_container == ଖାଲି) {
					ମୁକ୍ତ(text);
					ଫେରନ୍ତୁ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(ଖାଲି,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				ଖାଲି, ctx->bctx);
			ଯଦି (box == ଖାଲି) {
				ମୁକ୍ତ(text);
				ଫେରନ୍ତୁ false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			ଯଦି (box->text == ଖାଲି) {
				ମୁକ୍ତ(text);
				ଫେରନ୍ତୁ false;
			}

			box->length = strlen(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			ଯଦି (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(ଖାଲି, ଖାଲି,
						false, ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି,
						ctx->bctx);
				ଯଦି (props.inline_container == ଖାଲି) {
					ମୁକ୍ତ(text);
					ଫେରନ୍ତୁ false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				ଯଦି (current[0] == '\r' && current[1] == '\n')
					current += 2;
				ଅନ୍ୟଥା
					current++;
			}
		} ଯେତେବେଳେ (*current);

		ମୁକ୍ତ(text);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
ସ୍ଥିର ଶୂନ୍ୟ convert_xml_to_box(ସଂରଚନା box_construct_ctx *ctx)
{
	dom_ଗଣ୍ଠି *next;
	bool convert_children;
	uint32_t ଅଙ୍କ_processed = 0;
	ଅଚଳ uint32_t max_processed_before_yield = 10;

	କର {
		convert_children = true;

		ପ୍ରତିଜ୍ଞା(ctx->n != ଖାଲି);

		ଯଦି (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ଗଣ୍ଠି_unref(ctx->n);
			ମୁକ୍ତ(ctx);
			ଫେରନ୍ତୁ;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_ଗଣ୍ଠି(ctx->n, ctx->content, convert_children);
		ଯେତେବେଳେ (next != ଖାଲି) {
			dom_ଗଣ୍ଠି_type type;
			dom_exception err;

			err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(next, &type);
			ଯଦି (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ଗଣ୍ଠି_unref(next);
				ମୁକ୍ତ(ctx);
				ଫେରନ୍ତୁ;
			}

			ଯଦି (type == DOM_ELEMENT_ଗଣ୍ଠି)
				ଭାଙ୍ଗ;

			ଯଦି (type == DOM_TEXT_ଗଣ୍ଠି) {
				ctx->n = next;
				ଯଦି (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ଗଣ୍ଠି_unref(ctx->n);
					ମୁକ୍ତ(ctx);
					ଫେରନ୍ତୁ;
				}
			}

			next = next_ଗଣ୍ଠି(next, ctx->content, true);
		}

		ctx->n = next;

		ଯଦି (next == ଖାଲି) {
			/* Conversion complete */
			ସଂରଚନା box ମୂଳ;

			memset(&ମୂଳ, 0, ଆକାର(ମୂଳ));

			ମୂଳ.type = BOX_BLOCK;
			ମୂଳ.children = ମୂଳ.last = ctx->ମୂଳ_box;
			ମୂଳ.children->parent = &ମୂଳ;

			/** \todo Remove box_normalise_block */
			ଯଦି (box_normalise_block(&ମୂଳ, ctx->ମୂଳ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} ଅନ୍ୟଥା {
				ctx->content->layout = ମୂଳ.children;
				ctx->content->layout->parent = ଖାଲି;

				ctx->cb(ctx->content, true);
			}

			ପ୍ରତିଜ୍ଞା(ctx->n == ଖାଲି);

			ମୁକ୍ତ(ctx);
			ଫେରନ୍ତୁ;
		}
	} ଯେତେବେଳେ (++ଅଙ୍କ_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (ଶୂନ୍ୟ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_ଗଣ୍ଠି *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   ଶୂନ୍ୟ **box_conversion_context)
{
	ସଂରଚନା box_construct_ctx *ctx;

	ପ୍ରତିଜ୍ଞା(box_conversion_context != ଖାଲି);

	ଯଦି (c->bctx == ଖାଲି) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
		ଯଦି (c->bctx == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ctx = malloc(ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ଗଣ୍ଠି_ref(n);
	ctx->ମୂଳ_box = ଖାଲି;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	ଫେରନ୍ତୁ guit->misc->schedule(0, (ଶୂନ୍ୟ *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(ଶୂନ୍ୟ *box_conversion_context)
{
	ସଂରଚନା box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (ଶୂନ୍ୟ *)convert_xml_to_box, ctx);
	ଯଦି (err != NSERROR_OK) {
		ଫେରନ୍ତୁ err;
	}

	dom_ଗଣ୍ଠି_unref(ctx->n);
	ମୁକ୍ତ(ctx);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
ସଂରଚନା box *box_for_ଗଣ୍ଠି(dom_ଗଣ୍ଠି *n)
{
	ସଂରଚନା box *box = ଖାଲି;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_user_data(n, corestring_dom_ns_ଚାବି_box_ଗଣ୍ଠି_data,
			(ଶୂନ୍ୟ *) &box);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ଅଚଳ html_content *content,
		 ଅଚଳ dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	ଅକ୍ଷର *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, end;
	nserror error;
	ଅଚଳ ଅକ୍ଷର *rel;

	rel = dom_string_data(dsrel);

	s1 = s = malloc(3 * strlen(rel) + 1);
	ଯଦି (!s)
		ଫେରନ୍ତୁ false;

	/* copy to s, removing white space and control characters */
	ପାଇଁ (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	ପାଇଁ (end = strlen(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	ପାଇଁ (j = 0; i != end; i++) {
		ଯଦି ((unsigned ଅକ୍ଷର) rel[i] < 0x20) {
			; /* skip control characters */
		} ଅନ୍ୟଥା ଯଦି (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} ଅନ୍ୟଥା {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	ଯଦି (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		ଯଦି (strncmp(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			ଯଦି (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			ଯଦି (quot0)
				quot1 = strchr(quot0 + 1, '"');
			ଯଦି (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} ଅନ୍ୟଥା ଯଦି (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	ମୁକ୍ତ(s);
	ଯଦି (error != NSERROR_OK) {
		*result = ଖାଲି;
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}
