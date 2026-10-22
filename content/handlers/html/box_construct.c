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

#समावेश <string.h>
#समावेश <dom/dom.h>

#समावेश "utils/errors.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/talloc.h"
#समावेश "utils/string.h"
#समावेश "utils/ascii.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/utils.h"
#समावेश "netsurf/misc.h"
#समावेश "css/select.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/private.h"
#समावेश "html/object.h"
#समावेश "html/box.h"
#समावेश "html/box_manipulate.h"
#समावेश "html/box_construct.h"
#समावेश "html/box_special.h"
#समावेश "html/box_normalise.h"
#समावेश "html/form_internal.h"

/**
 * Context for box tree construction
 */
काष्ठा box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_गाँठ *n;			/**< Current node to process */

	काष्ठा box *मूल_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	पूर्णांक *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
काष्ठा box_construct_props {
	/** Style from which to inherit, or NULL if none */
	स्थिर css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	काष्ठा nsurl *href;
	/** Current frame target, or NULL if none */
	स्थिर अक्षर *target;
	/** Current title attribute, or NULL if none */
	स्थिर अक्षर *title;
	/** Identity of the current block-level container */
	काष्ठा box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	काष्ठा box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool गाँठ_is_मूल;
};

अटल स्थिर content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
अटल स्थिर box_type box_map[] = {
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
अटल अंतरभूत bool box_is_मूल(dom_गाँठ *n)
{
	dom_गाँठ *parent;
	dom_गाँठ_type type;
	dom_exception err;

	err = dom_गाँठ_get_parent_गाँठ(n, &parent);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (parent != शून्य) {
		err = dom_गाँठ_get_गाँठ_type(parent, &type);

		dom_गाँठ_unref(parent);

		अगर (err != DOM_NO_ERR)
			वापस false;

		अगर (type != DOM_DOCUMENT_गाँठ)
			वापस false;
	}

	वापस true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
अटल व्योम
box_extract_properties(dom_गाँठ *n, काष्ठा box_construct_props *props)
{
	स_रखो(props, 0, माप(*props));

	props->गाँठ_is_मूल = box_is_मूल(n);

	/* Extract properties from containing DOM node */
	अगर (props->गाँठ_is_मूल == false) {
		dom_गाँठ *current_गाँठ = n;
		dom_गाँठ *parent_गाँठ = शून्य;
		काष्ठा box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		जबतक (true) {
			err = dom_गाँठ_get_parent_गाँठ(current_गाँठ,
					&parent_गाँठ);
			अगर (err != DOM_NO_ERR || parent_गाँठ == शून्य)
				अवरोध;

			parent_box = box_for_गाँठ(parent_गाँठ);

			अगर (parent_box != शून्य) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_गाँठ_unref(parent_गाँठ);
				अवरोध;
			} अन्यथा {
				अगर (current_गाँठ != n)
					dom_गाँठ_unref(current_गाँठ);
				current_गाँठ = parent_गाँठ;
				parent_गाँठ = शून्य;
			}
		}

		/* Find containing block (may be parent) */
		जबतक (true) {
			काष्ठा box *b;

			err = dom_गाँठ_get_parent_गाँठ(current_गाँठ,
					&parent_गाँठ);
			अगर (err != DOM_NO_ERR || parent_गाँठ == शून्य) {
				अगर (current_गाँठ != n)
					dom_गाँठ_unref(current_गाँठ);
				अवरोध;
			}

			अगर (current_गाँठ != n)
				dom_गाँठ_unref(current_गाँठ);

			b = box_for_गाँठ(parent_गाँठ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			अगर (b != शून्य && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_गाँठ_unref(parent_गाँठ);
				अवरोध;
			} अन्यथा {
				current_गाँठ = parent_गाँठ;
				parent_गाँठ = शून्य;
			}
		}
	}

	/* Compute current inline container, if any */
	अगर (props->containing_block != शून्य &&
			props->containing_block->last != शून्य &&
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
अटल css_select_results *
box_get_style(html_content *c,
	      स्थिर css_computed_style *parent_style,
	      स्थिर css_computed_style *मूल_style,
	      dom_गाँठ *n)
{
	dom_string *s = शून्य;
	css_stylesheet *inline_style = शून्य;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	अगर (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		अगर (err != DOM_NO_ERR) {
			वापस शून्य;
		}
	}

	अगर (s != शून्य) {
		inline_style = nscss_create_inline_style(
				(स्थिर uint8_t *) dom_string_तथ्य(s),
				dom_string_byte_लंबाई(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		अगर (inline_style == शून्य)
			वापस शून्य;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.मूल_style = मूल_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	अगर (inline_style != शून्य)
		css_stylesheet_destroy(inline_style);

	वापस styles;
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
अटल व्योम
box_construct_generate(dom_गाँठ *n,
		       html_content *content,
		       काष्ठा box *box,
		       स्थिर css_computed_style *style)
{
	काष्ठा box *gen = शून्य;
	क्रमागत css_display_e computed_display;
	स्थिर css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	अगर (box->type != BOX_BLOCK)
		वापस;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	अगर (style == शून्य ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		वापस;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_मूल(n));
	अगर (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(शून्य, (css_computed_style *) style,
				false, शून्य, शून्य, शून्य, शून्य, content->bctx);
		अगर (gen == शून्य) {
			वापस;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_मूल(n))];

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
अटल bool
box_construct_marker(काष्ठा box *box,
		     स्थिर अक्षर *title,
		     काष्ठा box_construct_ctx *ctx,
		     काष्ठा box *parent)
{
	lwc_string *image_uri;
	काष्ठा box *marker;
	क्रमागत css_list_style_type_e list_style_type;

	marker = box_create(शून्य, box->style, false, शून्य, शून्य, title,
			शून्य, ctx->bctx);
	अगर (marker == false)
		वापस false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	चयन (list_style_type) {
	हाल CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (अक्षर *) "\342\200\242";
		marker->लंबाई = 3;
		अवरोध;

	हाल CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (अक्षर *) "\342\227\213";
		marker->लंबाई = 3;
		अवरोध;

	हाल CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (अक्षर *) "\342\226\252";
		marker->लंबाई = 3;
		अवरोध;

	शेष:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	हाल CSS_LIST_STYLE_TYPE_NONE:
		marker->text = शून्य;
		marker->लंबाई = 0;
		अवरोध;
	}

	अगर (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != शून्य) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_तथ्य(image_uri), &url);
		अगर (error != NSERROR_OK)
			वापस false;

		अगर (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			वापस false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	वापस true;
}

अटल अंतरभूत bool box__style_is_float(स्थिर काष्ठा box *box)
{
	वापस css_computed_float(box->style) == CSS_FLOAT_बायाँ ||
	       css_computed_float(box->style) == CSS_FLOAT_दायाँ;
}

अटल अंतरभूत bool box__is_flex(स्थिर काष्ठा box *box)
{
	वापस box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

अटल अंतरभूत bool box__containing_block_is_flex(
		स्थिर काष्ठा box_construct_props *props)
{
	वापस props->containing_block != शून्य &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
अटल bool
box_construct_element(काष्ठा box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = शून्य;
	क्रमागत css_display_e css_display;
	काष्ठा box *box = शून्य, *old_box;
	css_select_results *styles = शून्य;
	lwc_string *bgimage_uri;
	dom_exception err;
	काष्ठा box_construct_props props;
	स्थिर css_computed_style *मूल_style = शून्य;

	निश्चित(ctx->n != शून्य);

	box_extract_properties(ctx->n, &props);

	अगर (props.containing_block != शून्य) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	अगर (props.गाँठ_is_मूल == false) {
		मूल_style = ctx->मूल_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, मूल_style,
			ctx->n);
	अगर (styles == शून्य)
		वापस false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (title0 != शून्य) {
		अक्षर *t = squash_whitespace(dom_string_तथ्य(title0));

		dom_string_unref(title0);

		अगर (t == शून्य)
			वापस false;

		props.title = talloc_strdup(ctx->bctx, t);

		मुक्त(t);

		अगर (props.title == शून्य)
			वापस false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (s != शून्य) {
		err = dom_string_intern(s, &id);
		अगर (err != DOM_NO_ERR)
			id = शून्य;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	अगर (box == शून्य)
		वापस false;

	/* If this is the root box, add it to the context */
	अगर (props.गाँठ_is_मूल)
		ctx->मूल_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (s != शून्य) {
		स्थिर अक्षर *val = dom_string_तथ्य(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		अगर ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(म_से_दीर्घ(val, शून्य, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (s != शून्य) {
		स्थिर अक्षर *val = dom_string_तथ्य(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		अगर ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(म_से_दीर्घ(val, शून्य, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	अगर ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} अन्यथा अगर (props.गाँठ_is_मूल) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} अन्यथा {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.गाँठ_is_मूल)];

		अगर (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			चयन (box->type) {
			हाल BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				अवरोध;
			हाल BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				अवरोध;
			शेष:
				अवरोध;
			}
		}
	}

	अगर (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		वापस false;
	}

	/* Handle the :before pseudo element */
	अगर (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	अगर (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.गाँठ_is_मूल) == CSS_DISPLAY_NONE &&
			props.गाँठ_is_मूल == false)) {
		css_select_results_destroy(styles);
		box->styles = शून्य;
		box->style = शून्य;

		/* Invalidate associated gadget, if any */
		अगर (box->gadget != शून्य) {
			box->gadget->box = शून्य;
			box->gadget = शून्य;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		वापस true;
	}

	/* Attach DOM node to box */
	err = dom_गाँठ_set_user_तथ्य(ctx->n,
			corestring_dom_ns_कुंजी_box_गाँठ_तथ्य, box, शून्य,
			(व्योम *) &old_box);
	अगर (err != DOM_NO_ERR)
		वापस false;

	/* Attach box to DOM node */
	box->गाँठ = dom_गाँठ_ref(ctx->n);

	अगर (props.inline_container == शून्य &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.गाँठ_is_मूल == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		निश्चित(props.containing_block != शून्य &&
				"Box must have containing block.");

		props.inline_container = box_create(शून्य, शून्य, false, शून्य,
				शून्य, शून्य, शून्य, ctx->bctx);
		अगर (props.inline_container == शून्य)
			वापस false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	अगर (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != शून्य &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_तथ्य(bgimage_uri), &url);
		अगर (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			अगर (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				वापस false;
			}
			nsurl_unref(url);
		}
	}

	अगर (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	अगर (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		निश्चित(props.inline_container != शून्य);

		box_add_child(props.inline_container, box);
	} अन्यथा {
		अगर (ns_computed_display(box->style, props.गाँठ_is_मूल) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			अगर (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				वापस false;
		}

		अगर (props.गाँठ_is_मूल == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_बायाँ ||
				css_computed_float(box->style) ==
				CSS_FLOAT_दायाँ)) {
			/* Float: insert a float between the parent and box. */
			काष्ठा box *flt = box_create(शून्य, शून्य, false,
					props.href, props.target, props.title,
					शून्य, ctx->bctx);
			अगर (flt == शून्य)
				वापस false;

			अगर (css_computed_float(box->style) == CSS_FLOAT_बायाँ)
				flt->type = BOX_FLOAT_बायाँ;
			अन्यथा
				flt->type = BOX_FLOAT_दायाँ;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} अन्यथा {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			अगर (props.containing_block != शून्य)
				box_add_child(props.containing_block, box);
		}
	}

	वापस true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
अटल व्योम box_construct_element_after(dom_गाँठ *n, html_content *content)
{
	काष्ठा box_construct_props props;
	काष्ठा box *box = box_for_गाँठ(n);

	निश्चित(box != शून्य);

	box_extract_properties(n, &props);

	अगर (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		काष्ठा box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_गाँठ_has_child_nodes(n, &has_children);
		अगर (err != DOM_NO_ERR)
			वापस;

		अगर (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			वापस;
		}

		अगर (props.inline_container == शून्य) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(शून्य, शून्य, false,
					शून्य, शून्य, शून्य, शून्य, content->bctx);
			अगर (props.inline_container == शून्य)
				वापस;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(शून्य, box->style, false,
				box->href, box->target, box->title,
				box->id == शून्य ? शून्य :
				lwc_string_ref(box->id), content->bctx);
		अगर (inline_end != शून्य) {
			inline_end->type = BOX_INLINE_END;

			निश्चित(props.inline_container != शून्य);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} अन्यथा अगर (!(box->flags & IS_REPLACED)) {
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
अटल dom_गाँठ *
अग्र_गाँठ(dom_गाँठ *n, html_content *content, bool convert_children)
{
	dom_गाँठ *अग्र = शून्य;
	bool has_children;
	dom_exception err;

	err = dom_गाँठ_has_child_nodes(n, &has_children);
	अगर (err != DOM_NO_ERR) {
		dom_गाँठ_unref(n);
		वापस शून्य;
	}

	अगर (convert_children && has_children) {
		err = dom_गाँठ_get_first_child(n, &अग्र);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस शून्य;
		}
		dom_गाँठ_unref(n);
	} अन्यथा {
		err = dom_गाँठ_get_अग्र_sibling(n, &अग्र);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस शून्य;
		}

		अगर (अग्र != शून्य) {
			अगर (box_for_गाँठ(n) != शून्य)
				box_construct_element_after(n, content);
			dom_गाँठ_unref(n);
		} अन्यथा {
			अगर (box_for_गाँठ(n) != शून्य)
				box_construct_element_after(n, content);

			जबतक (box_is_मूल(n) == false) {
				dom_गाँठ *parent = शून्य;
				dom_गाँठ *parent_अग्र = शून्य;

				err = dom_गाँठ_get_parent_गाँठ(n, &parent);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(n);
					वापस शून्य;
				}

				निश्चित(parent != शून्य);

				err = dom_गाँठ_get_अग्र_sibling(parent,
						&parent_अग्र);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(parent);
					dom_गाँठ_unref(n);
					वापस शून्य;
				}

				अगर (parent_अग्र != शून्य) {
					dom_गाँठ_unref(parent_अग्र);
					dom_गाँठ_unref(parent);
					अवरोध;
				}

				dom_गाँठ_unref(n);
				n = parent;
				parent = शून्य;

				अगर (box_for_गाँठ(n) != शून्य) {
					box_construct_element_after(
							n, content);
				}
			}

			अगर (box_is_मूल(n) == false) {
				dom_गाँठ *parent = शून्य;

				err = dom_गाँठ_get_parent_गाँठ(n, &parent);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(n);
					वापस शून्य;
				}

				निश्चित(parent != शून्य);

				err = dom_गाँठ_get_अग्र_sibling(parent, &अग्र);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(parent);
					dom_गाँठ_unref(n);
					वापस शून्य;
				}

				अगर (box_for_गाँठ(parent) != शून्य) {
					box_construct_element_after(parent,
							content);
				}

				dom_गाँठ_unref(parent);
			}

			dom_गाँठ_unref(n);
		}
	}

	वापस अग्र;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
अटल व्योम
box_text_transform(अक्षर *s, अचिन्हित पूर्णांक len, क्रमागत css_text_transform_e tt)
{
	अचिन्हित पूर्णांक i;
	अगर (len == 0)
		वापस;
	चयन (tt) {
		हाल CSS_TEXT_TRANSFORM_UPPERCASE:
			क्रम (i = 0; i < len; ++i)
				अगर ((अचिन्हित अक्षर) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			अवरोध;
		हाल CSS_TEXT_TRANSFORM_LOWERCASE:
			क्रम (i = 0; i < len; ++i)
				अगर ((अचिन्हित अक्षर) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			अवरोध;
		हाल CSS_TEXT_TRANSFORM_CAPITALIZE:
			अगर ((अचिन्हित अक्षर) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			क्रम (i = 1; i < len; ++i)
				अगर ((अचिन्हित अक्षर) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			अवरोध;
		शेष:
			अवरोध;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
अटल bool box_construct_text(काष्ठा box_construct_ctx *ctx)
{
	काष्ठा box_construct_props props;
	काष्ठा box *box = शून्य;
	dom_string *content;
	dom_exception err;

	निश्चित(ctx->n != शून्य);

	box_extract_properties(ctx->n, &props);

	निश्चित(props.containing_block != शून्य);

	err = dom_characterdata_get_तथ्य(ctx->n, &content);
	अगर (err != DOM_NO_ERR || content == शून्य)
		वापस false;

	अगर (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		अक्षर *text;

		text = squash_whitespace(dom_string_तथ्य(content));

		dom_string_unref(content);

		अगर (text == शून्य)
			वापस false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		अगर (text[0] == ' ' && text[1] == 0) {
			अगर (props.inline_container != शून्य) {
				निश्चित(props.inline_container->last != शून्य);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			मुक्त(text);

			वापस true;
		}

		अगर (props.inline_container == शून्य) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(शून्य, शून्य, false,
					शून्य, शून्य, शून्य, शून्य, ctx->bctx);
			अगर (props.inline_container == शून्य) {
				मुक्त(text);
				वापस false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(शून्य,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				शून्य, ctx->bctx);
		अगर (box == शून्य) {
			मुक्त(text);
			वापस false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		मुक्त(text);
		अगर (box->text == शून्य)
			वापस false;

		box->लंबाई = म_माप(box->text);

		/* strip ending space char off */
		अगर (box->लंबाई > 1 && box->text[box->लंबाई - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->लंबाई--;
		}

		अगर (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->लंबाई,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		अगर (box->text[0] == ' ') {
			box->लंबाई--;

			स_हटाओ(box->text, &box->text[1], box->लंबाई);

			अगर (box->पूर्व != शून्य)
				box->पूर्व->space = UNKNOWN_WIDTH;
		}
	} अन्यथा {
		/* white-space: pre */
		अक्षर *text;
		माप_प्रकार text_len = dom_string_byte_लंबाई(content);
		माप_प्रकार i;
		अक्षर *current;
		क्रमागत css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		निश्चित(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_पंक्ति ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = दो_स्मृति(text_len + 1);
		dom_string_unref(content);

		अगर (text == शून्य)
			वापस false;

		स_नकल(text, dom_string_तथ्य(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		क्रम (i = 0; i < text_len; i++)
			अगर (text[i] == '\t')
				text[i] = ' ';

		अगर (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, म_माप(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		अगर (props.containing_block->flags & PRE_STRIP) {
			चयन (*current) {
			हाल '\n':
				current++;
				अवरोध;
			हाल '\r':
				current++;
				अगर (*current == '\n')
					current++;
				अवरोध;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		करो {
			माप_प्रकार len = म_खोज(current, "\r\n");

			अक्षर old = current[len];

			current[len] = 0;

			अगर (props.inline_container == शून्य) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(शून्य, शून्य,
						false, शून्य, शून्य, शून्य, शून्य,
						ctx->bctx);
				अगर (props.inline_container == शून्य) {
					मुक्त(text);
					वापस false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(शून्य,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				शून्य, ctx->bctx);
			अगर (box == शून्य) {
				मुक्त(text);
				वापस false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			अगर (box->text == शून्य) {
				मुक्त(text);
				वापस false;
			}

			box->लंबाई = म_माप(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			अगर (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(शून्य, शून्य,
						false, शून्य, शून्य, शून्य, शून्य,
						ctx->bctx);
				अगर (props.inline_container == शून्य) {
					मुक्त(text);
					वापस false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				अगर (current[0] == '\r' && current[1] == '\n')
					current += 2;
				अन्यथा
					current++;
			}
		} जबतक (*current);

		मुक्त(text);
	}

	वापस true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
अटल व्योम convert_xml_to_box(काष्ठा box_construct_ctx *ctx)
{
	dom_गाँठ *अग्र;
	bool convert_children;
	uint32_t अंक_processed = 0;
	स्थिर uint32_t max_processed_before_yield = 10;

	करो {
		convert_children = true;

		निश्चित(ctx->n != शून्य);

		अगर (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_गाँठ_unref(ctx->n);
			मुक्त(ctx);
			वापस;
		}

		/* Find next element to process, converting text nodes as we go */
		अग्र = अग्र_गाँठ(ctx->n, ctx->content, convert_children);
		जबतक (अग्र != शून्य) {
			dom_गाँठ_type type;
			dom_exception err;

			err = dom_गाँठ_get_गाँठ_type(अग्र, &type);
			अगर (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_गाँठ_unref(अग्र);
				मुक्त(ctx);
				वापस;
			}

			अगर (type == DOM_ELEMENT_गाँठ)
				अवरोध;

			अगर (type == DOM_TEXT_गाँठ) {
				ctx->n = अग्र;
				अगर (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_गाँठ_unref(ctx->n);
					मुक्त(ctx);
					वापस;
				}
			}

			अग्र = अग्र_गाँठ(अग्र, ctx->content, true);
		}

		ctx->n = अग्र;

		अगर (अग्र == शून्य) {
			/* Conversion complete */
			काष्ठा box मूल;

			स_रखो(&मूल, 0, माप(मूल));

			मूल.type = BOX_BLOCK;
			मूल.children = मूल.last = ctx->मूल_box;
			मूल.children->parent = &मूल;

			/** \todo Remove box_normalise_block */
			अगर (box_normalise_block(&मूल, ctx->मूल_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} अन्यथा {
				ctx->content->layout = मूल.children;
				ctx->content->layout->parent = शून्य;

				ctx->cb(ctx->content, true);
			}

			निश्चित(ctx->n == शून्य);

			मुक्त(ctx);
			वापस;
		}
	} जबतक (++अंक_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (व्योम *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_गाँठ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   व्योम **box_conversion_context)
{
	काष्ठा box_construct_ctx *ctx;

	निश्चित(box_conversion_context != शून्य);

	अगर (c->bctx == शून्य) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, पूर्णांक);
		अगर (c->bctx == शून्य) {
			वापस NSERROR_NOMEM;
		}
	}

	ctx = दो_स्मृति(माप(*ctx));
	अगर (ctx == शून्य) {
		वापस NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_गाँठ_ref(n);
	ctx->मूल_box = शून्य;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	वापस guit->misc->schedule(0, (व्योम *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(व्योम *box_conversion_context)
{
	काष्ठा box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (व्योम *)convert_xml_to_box, ctx);
	अगर (err != NSERROR_OK) {
		वापस err;
	}

	dom_गाँठ_unref(ctx->n);
	मुक्त(ctx);

	वापस NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
काष्ठा box *box_for_गाँठ(dom_गाँठ *n)
{
	काष्ठा box *box = शून्य;
	dom_exception err;

	err = dom_गाँठ_get_user_तथ्य(n, corestring_dom_ns_कुंजी_box_गाँठ_तथ्य,
			(व्योम *) &box);
	अगर (err != DOM_NO_ERR)
		वापस शून्य;

	वापस box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(स्थिर html_content *content,
		 स्थिर dom_string *dsrel,
		 nsurl *base,
		 nsurl **परिणाम)
{
	अक्षर *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	अचिन्हित पूर्णांक i, j, end;
	nserror error;
	स्थिर अक्षर *rel;

	rel = dom_string_तथ्य(dsrel);

	s1 = s = दो_स्मृति(3 * म_माप(rel) + 1);
	अगर (!s)
		वापस false;

	/* copy to s, removing white space and control characters */
	क्रम (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	क्रम (end = म_माप(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	क्रम (j = 0; i != end; i++) {
		अगर ((अचिन्हित अक्षर) rel[i] < 0x20) {
			; /* skip control characters */
		} अन्यथा अगर (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} अन्यथा {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	अगर (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		अगर (म_भेदन(s, "javascript:", 11) == 0) {
			apos0 = म_अक्षर(s, '\'');
			अगर (apos0)
				apos1 = म_अक्षर(apos0 + 1, '\'');
			quot0 = म_अक्षर(s, '"');
			अगर (quot0)
				quot1 = म_अक्षर(quot0 + 1, '"');
			अगर (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} अन्यथा अगर (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, परिणाम);
	मुक्त(s);
	अगर (error != NSERROR_OK) {
		*परिणाम = शून्य;
		वापस false;
	}

	वापस true;
}
