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

#अन्तर्भाव <string.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/string.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "css/select.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/object.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_manipulate.h"
#अन्तर्भाव "html/box_construct.h"
#अन्तर्भाव "html/box_special.h"
#अन्तर्भाव "html/box_normalise.h"
#अन्तर्भाव "html/form_internal.h"

/**
 * Context for box tree construction
 */
संरचना box_construct_ctx {
	html_content *content;		/**< Content we're constructing for */

	dom_गाठ *n;			/**< Current node to process */

	संरचना box *मूळ_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	पूर्णांक *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
संरचना box_construct_props {
	/** Style from which to inherit, or NULL if none */
	स्थिरांक css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	संरचना nsurl *href;
	/** Current frame target, or NULL if none */
	स्थिरांक अक्षर *target;
	/** Current title attribute, or NULL if none */
	स्थिरांक अक्षर *title;
	/** Identity of the current block-level container */
	संरचना box *containing_block;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	संरचना box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool गाठ_is_मूळ;
};

स्थिर स्थिरांक content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
स्थिर स्थिरांक box_type box_map[] = {
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
स्थिर ओळीत bool box_is_मूळ(dom_गाठ *n)
{
	dom_गाठ *parent;
	dom_गाठ_type type;
	dom_exception err;

	err = dom_गाठ_get_parent_गाठ(n, &parent);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (parent != रिक्त) {
		err = dom_गाठ_get_गाठ_type(parent, &type);

		dom_गाठ_unref(parent);

		जर (err != DOM_NO_ERR)
			परत false;

		जर (type != DOM_DOCUMENT_गाठ)
			परत false;
	}

	परत true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
स्थिर शून्य
box_extract_properties(dom_गाठ *n, संरचना box_construct_props *props)
{
	स_रखो(props, 0, आकार(*props));

	props->गाठ_is_मूळ = box_is_मूळ(n);

	/* Extract properties from containing DOM node */
	जर (props->गाठ_is_मूळ == false) {
		dom_गाठ *current_गाठ = n;
		dom_गाठ *parent_गाठ = रिक्त;
		संरचना box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		पर्यंत (true) {
			err = dom_गाठ_get_parent_गाठ(current_गाठ,
					&parent_गाठ);
			जर (err != DOM_NO_ERR || parent_गाठ == रिक्त)
				थांबा;

			parent_box = box_for_गाठ(parent_गाठ);

			जर (parent_box != रिक्त) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_गाठ_unref(parent_गाठ);
				थांबा;
			} अन्यथा {
				जर (current_गाठ != n)
					dom_गाठ_unref(current_गाठ);
				current_गाठ = parent_गाठ;
				parent_गाठ = रिक्त;
			}
		}

		/* Find containing block (may be parent) */
		पर्यंत (true) {
			संरचना box *b;

			err = dom_गाठ_get_parent_गाठ(current_गाठ,
					&parent_गाठ);
			जर (err != DOM_NO_ERR || parent_गाठ == रिक्त) {
				जर (current_गाठ != n)
					dom_गाठ_unref(current_गाठ);
				थांबा;
			}

			जर (current_गाठ != n)
				dom_गाठ_unref(current_गाठ);

			b = box_for_गाठ(parent_गाठ);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			जर (b != रिक्त && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_block = b;

				dom_गाठ_unref(parent_गाठ);
				थांबा;
			} अन्यथा {
				current_गाठ = parent_गाठ;
				parent_गाठ = रिक्त;
			}
		}
	}

	/* Compute current inline container, if any */
	जर (props->containing_block != रिक्त &&
			props->containing_block->last != रिक्त &&
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
स्थिर css_select_results *
box_get_style(html_content *c,
	      स्थिरांक css_computed_style *parent_style,
	      स्थिरांक css_computed_style *मूळ_style,
	      dom_गाठ *n)
{
	dom_string *s = रिक्त;
	css_stylesheet *inline_style = रिक्त;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	जर (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		जर (err != DOM_NO_ERR) {
			परत रिक्त;
		}
	}

	जर (s != रिक्त) {
		inline_style = nscss_create_inline_style(
				(स्थिरांक uint8_t *) dom_string_data(s),
				dom_string_byte_length(s),
				c->encoding,
				nsurl_access(c->base_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_MODE_NONE);

		dom_string_unref(s);

		जर (inline_style == रिक्त)
			परत रिक्त;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
	ctx.base_url = c->base_url;
	ctx.universal = c->universal;
	ctx.मूळ_style = मूळ_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	जर (inline_style != रिक्त)
		css_stylesheet_destroy(inline_style);

	परत styles;
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
स्थिर शून्य
box_construct_generate(dom_गाठ *n,
		       html_content *content,
		       संरचना box *box,
		       स्थिरांक css_computed_style *style)
{
	संरचना box *gen = रिक्त;
	गणना css_display_e computed_display;
	स्थिरांक css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	जर (box->type != BOX_BLOCK)
		परत;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	जर (style == रिक्त ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_NORMAL) {
		/* No pseudo element */
		परत;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_मूळ(n));
	जर (computed_display == CSS_DISPLAY_BLOCK ||
			computed_display == CSS_DISPLAY_TABLE) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(रिक्त, (css_computed_style *) style,
				false, रिक्त, रिक्त, रिक्त, रिक्त, content->bctx);
		जर (gen == रिक्त) {
			परत;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_मूळ(n))];

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
स्थिर bool
box_construct_marker(संरचना box *box,
		     स्थिरांक अक्षर *title,
		     संरचना box_construct_ctx *ctx,
		     संरचना box *parent)
{
	lwc_string *image_uri;
	संरचना box *marker;
	गणना css_list_style_type_e list_style_type;

	marker = box_create(रिक्त, box->style, false, रिक्त, रिक्त, title,
			रिक्त, ctx->bctx);
	जर (marker == false)
		परत false;

	marker->type = BOX_BLOCK;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	निवड (list_style_type) {
	स्थिती CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (अक्षर *) "\342\200\242";
		marker->length = 3;
		थांबा;

	स्थिती CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (अक्षर *) "\342\227\213";
		marker->length = 3;
		थांबा;

	स्थिती CSS_LIST_STYLE_TYPE_SQUARE:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (अक्षर *) "\342\226\252";
		marker->length = 3;
		थांबा;

	पूर्वनिर्धारित:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	स्थिती CSS_LIST_STYLE_TYPE_NONE:
		marker->text = रिक्त;
		marker->length = 0;
		थांबा;
	}

	जर (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != रिक्त) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(image_uri), &url);
		जर (error != NSERROR_OK)
			परत false;

		जर (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			परत false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	परत true;
}

स्थिर ओळीत bool box__style_is_float(स्थिरांक संरचना box *box)
{
	परत css_computed_float(box->style) == CSS_FLOAT_डावा ||
	       css_computed_float(box->style) == CSS_FLOAT_उजवा;
}

स्थिर ओळीत bool box__is_flex(स्थिरांक संरचना box *box)
{
	परत box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

स्थिर ओळीत bool box__containing_block_is_flex(
		स्थिरांक संरचना box_construct_props *props)
{
	परत props->containing_block != रिक्त &&
	       box__is_flex(props->containing_block);
}

/**
 * Construct the box tree for an XML element.
 *
 * \param ctx               Tree construction context
 * \param convert_children  Whether to convert children
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool
box_construct_element(संरचना box_construct_ctx *ctx, bool *convert_children)
{
	dom_string *title0, *s;
	lwc_string *id = रिक्त;
	गणना css_display_e css_display;
	संरचना box *box = रिक्त, *old_box;
	css_select_results *styles = रिक्त;
	lwc_string *bgimage_uri;
	dom_exception err;
	संरचना box_construct_props props;
	स्थिरांक css_computed_style *मूळ_style = रिक्त;

	प्रतिज्ञा(ctx->n != रिक्त);

	box_extract_properties(ctx->n, &props);

	जर (props.containing_block != रिक्त) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_block->flags &= ~PRE_STRIP;
	}

	जर (props.गाठ_is_मूळ == false) {
		मूळ_style = ctx->मूळ_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, मूळ_style,
			ctx->n);
	जर (styles == रिक्त)
		परत false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (title0 != रिक्त) {
		अक्षर *t = squash_whitespace(dom_string_data(title0));

		dom_string_unref(title0);

		जर (t == रिक्त)
			परत false;

		props.title = talloc_strdup(ctx->bctx, t);

		मुक्त(t);

		जर (props.title == रिक्त)
			परत false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (s != रिक्त) {
		err = dom_string_intern(s, &id);
		जर (err != DOM_NO_ERR)
			id = रिक्त;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	जर (box == रिक्त)
		परत false;

	/* If this is the root box, add it to the context */
	जर (props.गाठ_is_मूळ)
		ctx->मूळ_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (s != रिक्त) {
		स्थिरांक अक्षर *val = dom_string_data(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		जर ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(म_से_दीर्घ(val, रिक्त, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (s != रिक्त) {
		स्थिरांक अक्षर *val = dom_string_data(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		जर ('0' <= val[0] && val[0] <= '9')
			box->rows = clamp(म_से_दीर्घ(val, रिक्त, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	जर ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
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
	} अन्यथा जर (props.गाठ_is_मूळ) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_BLOCK;
	} अन्यथा {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.गाठ_is_मूळ)];

		जर (props.containing_block->type == BOX_FLEX ||
		    props.containing_block->type == BOX_INLINE_FLEX) {
			/* Blockification */
			निवड (box->type) {
			स्थिती BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				थांबा;
			स्थिती BOX_INLINE_BLOCK:
				box->type = BOX_BLOCK;
				थांबा;
			पूर्वनिर्धारित:
				थांबा;
			}
		}
	}

	जर (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		परत false;
	}

	/* Handle the :before pseudo element */
	जर (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	जर (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.गाठ_is_मूळ) == CSS_DISPLAY_NONE &&
			props.गाठ_is_मूळ == false)) {
		css_select_results_destroy(styles);
		box->styles = रिक्त;
		box->style = रिक्त;

		/* Invalidate associated gadget, if any */
		जर (box->gadget != रिक्त) {
			box->gadget->box = रिक्त;
			box->gadget = रिक्त;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		परत true;
	}

	/* Attach DOM node to box */
	err = dom_गाठ_set_user_data(ctx->n,
			corestring_dom_ns_किल्ली_box_गाठ_data, box, रिक्त,
			(शून्य *) &old_box);
	जर (err != DOM_NO_ERR)
		परत false;

	/* Attach box to DOM node */
	box->गाठ = dom_गाठ_ref(ctx->n);

	जर (props.inline_container == रिक्त &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_BLOCK ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box__containing_block_is_flex(&props))) &&
			props.गाठ_is_मूळ == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		प्रतिज्ञा(props.containing_block != रिक्त &&
				"Box must have containing block.");

		props.inline_container = box_create(रिक्त, रिक्त, false, रिक्त,
				रिक्त, रिक्त, रिक्त, ctx->bctx);
		जर (props.inline_container == रिक्त)
			परत false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_add_child(props.containing_block, props.inline_container);
	}

	/* Kick off fetch for any background image */
	जर (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != रिक्त &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_data(bgimage_uri), &url);
		जर (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			जर (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				परत false;
			}
			nsurl_unref(url);
		}
	}

	जर (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	जर (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_BLOCK) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		प्रतिज्ञा(props.inline_container != रिक्त);

		box_add_child(props.inline_container, box);
	} अन्यथा {
		जर (ns_computed_display(box->style, props.गाठ_is_मूळ) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			जर (box_construct_marker(box, props.title, ctx,
					props.containing_block) == false)
				परत false;
		}

		जर (props.गाठ_is_मूळ == false &&
				box__containing_block_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_डावा ||
				css_computed_float(box->style) ==
				CSS_FLOAT_उजवा)) {
			/* Float: insert a float between the parent and box. */
			संरचना box *flt = box_create(रिक्त, रिक्त, false,
					props.href, props.target, props.title,
					रिक्त, ctx->bctx);
			जर (flt == रिक्त)
				परत false;

			जर (css_computed_float(box->style) == CSS_FLOAT_डावा)
				flt->type = BOX_FLOAT_डावा;
			अन्यथा
				flt->type = BOX_FLOAT_उजवा;

			box_add_child(props.inline_container, flt);
			box_add_child(flt, box);
		} अन्यथा {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			जर (props.containing_block != रिक्त)
				box_add_child(props.containing_block, box);
		}
	}

	परत true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
स्थिर शून्य box_construct_element_after(dom_गाठ *n, html_content *content)
{
	संरचना box_construct_props props;
	संरचना box *box = box_for_गाठ(n);

	प्रतिज्ञा(box != रिक्त);

	box_extract_properties(n, &props);

	जर (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		संरचना box *inline_end;
		bool has_children;
		dom_exception err;

		err = dom_गाठ_has_child_nodes(n, &has_children);
		जर (err != DOM_NO_ERR)
			परत;

		जर (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			परत;
		}

		जर (props.inline_container == रिक्त) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(रिक्त, रिक्त, false,
					रिक्त, रिक्त, रिक्त, रिक्त, content->bctx);
			जर (props.inline_container == रिक्त)
				परत;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		inline_end = box_create(रिक्त, box->style, false,
				box->href, box->target, box->title,
				box->id == रिक्त ? रिक्त :
				lwc_string_ref(box->id), content->bctx);
		जर (inline_end != रिक्त) {
			inline_end->type = BOX_INLINE_END;

			प्रतिज्ञा(props.inline_container != रिक्त);

			box_add_child(props.inline_container, inline_end);

			box->inline_end = inline_end;
			inline_end->inline_end = box;
		}
	} अन्यथा जर (!(box->flags & IS_REPLACED)) {
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
स्थिर dom_गाठ *
next_गाठ(dom_गाठ *n, html_content *content, bool convert_children)
{
	dom_गाठ *next = रिक्त;
	bool has_children;
	dom_exception err;

	err = dom_गाठ_has_child_nodes(n, &has_children);
	जर (err != DOM_NO_ERR) {
		dom_गाठ_unref(n);
		परत रिक्त;
	}

	जर (convert_children && has_children) {
		err = dom_गाठ_get_first_child(n, &next);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			परत रिक्त;
		}
		dom_गाठ_unref(n);
	} अन्यथा {
		err = dom_गाठ_get_next_sibling(n, &next);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			परत रिक्त;
		}

		जर (next != रिक्त) {
			जर (box_for_गाठ(n) != रिक्त)
				box_construct_element_after(n, content);
			dom_गाठ_unref(n);
		} अन्यथा {
			जर (box_for_गाठ(n) != रिक्त)
				box_construct_element_after(n, content);

			पर्यंत (box_is_मूळ(n) == false) {
				dom_गाठ *parent = रिक्त;
				dom_गाठ *parent_next = रिक्त;

				err = dom_गाठ_get_parent_गाठ(n, &parent);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(n);
					परत रिक्त;
				}

				प्रतिज्ञा(parent != रिक्त);

				err = dom_गाठ_get_next_sibling(parent,
						&parent_next);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(parent);
					dom_गाठ_unref(n);
					परत रिक्त;
				}

				जर (parent_next != रिक्त) {
					dom_गाठ_unref(parent_next);
					dom_गाठ_unref(parent);
					थांबा;
				}

				dom_गाठ_unref(n);
				n = parent;
				parent = रिक्त;

				जर (box_for_गाठ(n) != रिक्त) {
					box_construct_element_after(
							n, content);
				}
			}

			जर (box_is_मूळ(n) == false) {
				dom_गाठ *parent = रिक्त;

				err = dom_गाठ_get_parent_गाठ(n, &parent);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(n);
					परत रिक्त;
				}

				प्रतिज्ञा(parent != रिक्त);

				err = dom_गाठ_get_next_sibling(parent, &next);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(parent);
					dom_गाठ_unref(n);
					परत रिक्त;
				}

				जर (box_for_गाठ(parent) != रिक्त) {
					box_construct_element_after(parent,
							content);
				}

				dom_गाठ_unref(parent);
			}

			dom_गाठ_unref(n);
		}
	}

	परत next;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
स्थिर शून्य
box_text_transform(अक्षर *s, अचिन्हित पूर्णांक len, गणना css_text_transform_e tt)
{
	अचिन्हित पूर्णांक i;
	जर (len == 0)
		परत;
	निवड (tt) {
		स्थिती CSS_TEXT_TRANSFORM_UPPERCASE:
			साठी (i = 0; i < len; ++i)
				जर ((अचिन्हित अक्षर) s[i] < 0x80)
					s[i] = ascii_to_upper(s[i]);
			थांबा;
		स्थिती CSS_TEXT_TRANSFORM_LOWERCASE:
			साठी (i = 0; i < len; ++i)
				जर ((अचिन्हित अक्षर) s[i] < 0x80)
					s[i] = ascii_to_lower(s[i]);
			थांबा;
		स्थिती CSS_TEXT_TRANSFORM_CAPITALIZE:
			जर ((अचिन्हित अक्षर) s[0] < 0x80)
				s[0] = ascii_to_upper(s[0]);
			साठी (i = 1; i < len; ++i)
				जर ((अचिन्हित अक्षर) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_to_upper(s[i]);
			थांबा;
		पूर्वनिर्धारित:
			थांबा;
	}
}


/**
 * Construct the box tree for an XML text node.
 *
 * \param  ctx  Tree construction context
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool box_construct_text(संरचना box_construct_ctx *ctx)
{
	संरचना box_construct_props props;
	संरचना box *box = रिक्त;
	dom_string *content;
	dom_exception err;

	प्रतिज्ञा(ctx->n != रिक्त);

	box_extract_properties(ctx->n, &props);

	प्रतिज्ञा(props.containing_block != रिक्त);

	err = dom_characterdata_get_data(ctx->n, &content);
	जर (err != DOM_NO_ERR || content == रिक्त)
		परत false;

	जर (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NORMAL ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		अक्षर *text;

		text = squash_whitespace(dom_string_data(content));

		dom_string_unref(content);

		जर (text == रिक्त)
			परत false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		जर (text[0] == ' ' && text[1] == 0) {
			जर (props.inline_container != रिक्त) {
				प्रतिज्ञा(props.inline_container->last != रिक्त);

				props.inline_container->last->space =
						UNKNOWN_WIDTH;
			}

			मुक्त(text);

			परत true;
		}

		जर (props.inline_container == रिक्त) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(रिक्त, रिक्त, false,
					रिक्त, रिक्त, रिक्त, रिक्त, ctx->bctx);
			जर (props.inline_container == रिक्त) {
				मुक्त(text);
				परत false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_add_child(props.containing_block,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(रिक्त,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				रिक्त, ctx->bctx);
		जर (box == रिक्त) {
			मुक्त(text);
			परत false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		मुक्त(text);
		जर (box->text == रिक्त)
			परत false;

		box->length = म_माप(box->text);

		/* strip ending space char off */
		जर (box->length > 1 && box->text[box->length - 1] == ' ') {
			box->space = UNKNOWN_WIDTH;
			box->length--;
		}

		जर (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->length,
				css_computed_text_transform(
					props.parent_style));

		box_add_child(props.inline_container, box);

		जर (box->text[0] == ' ') {
			box->length--;

			स_हटाओ(box->text, &box->text[1], box->length);

			जर (box->prev != रिक्त)
				box->prev->space = UNKNOWN_WIDTH;
		}
	} अन्यथा {
		/* white-space: pre */
		अक्षर *text;
		माप_प्रकार text_len = dom_string_byte_length(content);
		माप_प्रकार i;
		अक्षर *current;
		गणना css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		प्रतिज्ञा(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_ओळ ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = दो_स्मृति(text_len + 1);
		dom_string_unref(content);

		जर (text == रिक्त)
			परत false;

		स_नकल(text, dom_string_data(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		साठी (i = 0; i < text_len; i++)
			जर (text[i] == '\t')
				text[i] = ' ';

		जर (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, म_माप(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		जर (props.containing_block->flags & PRE_STRIP) {
			निवड (*current) {
			स्थिती '\n':
				current++;
				थांबा;
			स्थिती '\r':
				current++;
				जर (*current == '\n')
					current++;
				थांबा;
			}
			props.containing_block->flags &= ~PRE_STRIP;
		}

		करा {
			माप_प्रकार len = म_खोज(current, "\r\n");

			अक्षर old = current[len];

			current[len] = 0;

			जर (props.inline_container == रिक्त) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(रिक्त, रिक्त,
						false, रिक्त, रिक्त, रिक्त, रिक्त,
						ctx->bctx);
				जर (props.inline_container == रिक्त) {
					मुक्त(text);
					परत false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(रिक्त,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				रिक्त, ctx->bctx);
			जर (box == रिक्त) {
				मुक्त(text);
				परत false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			जर (box->text == रिक्त) {
				मुक्त(text);
				परत false;
			}

			box->length = म_माप(box->text);

			box_add_child(props.inline_container, box);

			current[len] = old;

			current += len;

			जर (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(रिक्त, रिक्त,
						false, रिक्त, रिक्त, रिक्त, रिक्त,
						ctx->bctx);
				जर (props.inline_container == रिक्त) {
					मुक्त(text);
					परत false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_add_child(props.containing_block,
						props.inline_container);

				जर (current[0] == '\r' && current[1] == '\n')
					current += 2;
				अन्यथा
					current++;
			}
		} पर्यंत (*current);

		मुक्त(text);
	}

	परत true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
स्थिर शून्य convert_xml_to_box(संरचना box_construct_ctx *ctx)
{
	dom_गाठ *next;
	bool convert_children;
	uint32_t अंक_processed = 0;
	स्थिरांक uint32_t max_processed_before_yield = 10;

	करा {
		convert_children = true;

		प्रतिज्ञा(ctx->n != रिक्त);

		जर (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_गाठ_unref(ctx->n);
			मुक्त(ctx);
			परत;
		}

		/* Find next element to process, converting text nodes as we go */
		next = next_गाठ(ctx->n, ctx->content, convert_children);
		पर्यंत (next != रिक्त) {
			dom_गाठ_type type;
			dom_exception err;

			err = dom_गाठ_get_गाठ_type(next, &type);
			जर (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_गाठ_unref(next);
				मुक्त(ctx);
				परत;
			}

			जर (type == DOM_ELEMENT_गाठ)
				थांबा;

			जर (type == DOM_TEXT_गाठ) {
				ctx->n = next;
				जर (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_गाठ_unref(ctx->n);
					मुक्त(ctx);
					परत;
				}
			}

			next = next_गाठ(next, ctx->content, true);
		}

		ctx->n = next;

		जर (next == रिक्त) {
			/* Conversion complete */
			संरचना box मूळ;

			स_रखो(&मूळ, 0, आकार(मूळ));

			मूळ.type = BOX_BLOCK;
			मूळ.children = मूळ.last = ctx->मूळ_box;
			मूळ.children->parent = &मूळ;

			/** \todo Remove box_normalise_block */
			जर (box_normalise_block(&मूळ, ctx->मूळ_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} अन्यथा {
				ctx->content->layout = मूळ.children;
				ctx->content->layout->parent = रिक्त;

				ctx->cb(ctx->content, true);
			}

			प्रतिज्ञा(ctx->n == रिक्त);

			मुक्त(ctx);
			परत;
		}
	} पर्यंत (++अंक_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->schedule(0, (शून्य *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_to_box(dom_गाठ *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   शून्य **box_conversion_context)
{
	संरचना box_construct_ctx *ctx;

	प्रतिज्ञा(box_conversion_context != रिक्त);

	जर (c->bctx == रिक्त) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, पूर्णांक);
		जर (c->bctx == रिक्त) {
			परत NSERROR_NOMEM;
		}
	}

	ctx = दो_स्मृति(आकार(*ctx));
	जर (ctx == रिक्त) {
		परत NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_गाठ_ref(n);
	ctx->मूळ_box = रिक्त;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	परत guit->misc->schedule(0, (शून्य *)convert_xml_to_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_to_box(शून्य *box_conversion_context)
{
	संरचना box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->schedule(-1, (शून्य *)convert_xml_to_box, ctx);
	जर (err != NSERROR_OK) {
		परत err;
	}

	dom_गाठ_unref(ctx->n);
	मुक्त(ctx);

	परत NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
संरचना box *box_for_गाठ(dom_गाठ *n)
{
	संरचना box *box = रिक्त;
	dom_exception err;

	err = dom_गाठ_get_user_data(n, corestring_dom_ns_किल्ली_box_गाठ_data,
			(शून्य *) &box);
	जर (err != DOM_NO_ERR)
		परत रिक्त;

	परत box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(स्थिरांक html_content *content,
		 स्थिरांक dom_string *dsrel,
		 nsurl *base,
		 nsurl **result)
{
	अक्षर *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	अचिन्हित पूर्णांक i, j, end;
	nserror error;
	स्थिरांक अक्षर *rel;

	rel = dom_string_data(dsrel);

	s1 = s = दो_स्मृति(3 * म_माप(rel) + 1);
	जर (!s)
		परत false;

	/* copy to s, removing white space and control characters */
	साठी (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	साठी (end = म_माप(rel);
	     (end != i) && ascii_is_space(rel[end - 1]);
	     end--)
		;
	साठी (j = 0; i != end; i++) {
		जर ((अचिन्हित अक्षर) rel[i] < 0x20) {
			; /* skip control characters */
		} अन्यथा जर (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} अन्यथा {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	जर (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		जर (म_भेदन(s, "javascript:", 11) == 0) {
			apos0 = म_अक्षर(s, '\'');
			जर (apos0)
				apos1 = म_अक्षर(apos0 + 1, '\'');
			quot0 = म_अक्षर(s, '"');
			जर (quot0)
				quot1 = म_अक्षर(quot0 + 1, '"');
			जर (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} अन्यथा जर (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(base, s1, result);
	मुक्त(s);
	जर (error != NSERROR_OK) {
		*result = रिक्त;
		परत false;
	}

	परत true;
}
