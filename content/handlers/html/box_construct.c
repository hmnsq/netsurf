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

	dom_ग्रन्थिः *n;			/**< Current node to process */

	संरचना box *मूलम्_box;		/**< Root box in the tree */

	box_construct_complete_cb cb;	/**< Callback to invoke on completion */

	पूर्णाङ्क *bctx;			/**< talloc context */
};

/**
 * Transient properties for construction of current node
 */
संरचना box_construct_props {
	/** Style from which to inherit, or NULL if none */
	ध्रुव css_computed_style *parent_style;
	/** Current link target, or NULL if none */
	संरचना nsurl *href;
	/** Current frame target, or NULL if none */
	ध्रुव वर्ण *target;
	/** Current title attribute, or NULL if none */
	ध्रुव वर्ण *title;
	/** Identity of the current block-level container */
	संरचना box *containing_खण्डम्;
	/** Current container for inlines, or NULL if none
	 * \note If non-NULL, will be the last child of containing_block */
	संरचना box *inline_container;
	/** Whether the current node is the root of the DOM tree */
	bool ग्रन्थिः_is_मूलम्;
};

स्थिर ध्रुव content_type image_types = CONTENT_IMAGE;

/* mapping from CSS display to box type
 * this table must be in sync with libcss' css_display enum */
स्थिर ध्रुव box_type box_map[] = {
	BOX_खण्डम्,           /* CSS_DISPLAY_INHERIT */
	BOX_INLINE,          /* CSS_DISPLAY_INLINE */
	BOX_खण्डम्,           /* CSS_DISPLAY_BLOCK */
	BOX_खण्डम्,           /* CSS_DISPLAY_LIST_ITEM */
	BOX_INLINE,          /* CSS_DISPLAY_RUN_IN */
	BOX_INLINE_खण्डम्,    /* CSS_DISPLAY_INLINE_BLOCK */
	BOX_सारणी,           /* CSS_DISPLAY_TABLE */
	BOX_सारणी,           /* CSS_DISPLAY_INLINE_TABLE */
	BOX_सारणी_ROW_गणः, /* CSS_DISPLAY_TABLE_ROW_GROUP */
	BOX_सारणी_ROW_गणः, /* CSS_DISPLAY_TABLE_HEADER_GROUP */
	BOX_सारणी_ROW_गणः, /* CSS_DISPLAY_TABLE_FOOTER_GROUP */
	BOX_सारणी_ROW,       /* CSS_DISPLAY_TABLE_ROW */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN_GROUP */
	BOX_NONE,            /* CSS_DISPLAY_TABLE_COLUMN */
	BOX_सारणी_CELL,      /* CSS_DISPLAY_TABLE_CELL */
	BOX_INLINE,          /* CSS_DISPLAY_TABLE_CAPTION */
	BOX_NONE,            /* CSS_DISPLAY_NONE */
	BOX_FLEX,            /* CSS_DISPLAY_FLEX */
	BOX_INLINE_FLEX,     /* CSS_DISPLAY_INLINE_FLEX */
	BOX_खण्डम्,           /* CSS_DISPLAY_GRID */
	BOX_INLINE_खण्डम्,    /* CSS_DISPLAY_INLINE_GRID */
};


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
स्थिर अन्तःस्थ bool box_is_मूलम्(dom_ग्रन्थिः *n)
{
	dom_ग्रन्थिः *parent;
	dom_ग्रन्थिः_type type;
	dom_exception err;

	err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(n, &parent);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (parent != रिक्त) {
		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(parent, &type);

		dom_ग्रन्थिः_unref(parent);

		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		यदि (type != DOM_DOCUMENT_ग्रन्थिः)
			प्रत्यागम false;
	}

	प्रत्यागम true;
}

/**
 * Extract transient construction properties
 *
 * \param n      Current DOM node to convert
 * \param props  Property object to populate
 */
स्थिर शून्य
box_extract_properties(dom_ग्रन्थिः *n, संरचना box_construct_props *props)
{
	स्मृतिं_पूरय(props, 0, परिमाण(*props));

	props->ग्रन्थिः_is_मूलम् = box_is_मूलम्(n);

	/* Extract properties from containing DOM node */
	यदि (props->ग्रन्थिः_is_मूलम् == false) {
		dom_ग्रन्थिः *current_ग्रन्थिः = n;
		dom_ग्रन्थिः *parent_ग्रन्थिः = रिक्त;
		संरचना box *parent_box;
		dom_exception err;

		/* Find ancestor node containing parent box */
		यावत् (true) {
			err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(current_ग्रन्थिः,
					&parent_ग्रन्थिः);
			यदि (err != DOM_NO_ERR || parent_ग्रन्थिः == रिक्त)
				भङ्ग;

			parent_box = box_for_ग्रन्थिः(parent_ग्रन्थिः);

			यदि (parent_box != रिक्त) {
				props->parent_style = parent_box->style;
				props->href = parent_box->href;
				props->target = parent_box->target;
				props->title = parent_box->title;

				dom_ग्रन्थिः_unref(parent_ग्रन्थिः);
				भङ्ग;
			} अन्यथा {
				यदि (current_ग्रन्थिः != n)
					dom_ग्रन्थिः_unref(current_ग्रन्थिः);
				current_ग्रन्थिः = parent_ग्रन्थिः;
				parent_ग्रन्थिः = रिक्त;
			}
		}

		/* Find containing block (may be parent) */
		यावत् (true) {
			संरचना box *b;

			err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(current_ग्रन्थिः,
					&parent_ग्रन्थिः);
			यदि (err != DOM_NO_ERR || parent_ग्रन्थिः == रिक्त) {
				यदि (current_ग्रन्थिः != n)
					dom_ग्रन्थिः_unref(current_ग्रन्थिः);
				भङ्ग;
			}

			यदि (current_ग्रन्थिः != n)
				dom_ग्रन्थिः_unref(current_ग्रन्थिः);

			b = box_for_ग्रन्थिः(parent_ग्रन्थिः);

			/* Children of nodes that created an inline box
			 * will generate boxes which are attached as
			 * _siblings_ of the box generated for their
			 * parent node. Note, however, that we'll still
			 * use the parent node's styling as the parent
			 * style, above. */
			यदि (b != रिक्त && b->type != BOX_INLINE &&
					b->type != BOX_BR) {
				props->containing_खण्डम् = b;

				dom_ग्रन्थिः_unref(parent_ग्रन्थिः);
				भङ्ग;
			} अन्यथा {
				current_ग्रन्थिः = parent_ग्रन्थिः;
				parent_ग्रन्थिः = रिक्त;
			}
		}
	}

	/* Compute current inline container, if any */
	यदि (props->containing_खण्डम् != रिक्त &&
			props->containing_खण्डम्->चरमः != रिक्त &&
			props->containing_खण्डम्->चरमः->type ==
				BOX_INLINE_CONTAINER)
		props->inline_container = props->containing_खण्डम्->चरमः;
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
	      ध्रुव css_computed_style *parent_style,
	      ध्रुव css_computed_style *मूलम्_style,
	      dom_ग्रन्थिः *n)
{
	dom_string *s = रिक्त;
	css_stylesheet *inline_style = रिक्त;
	css_select_results *styles;
	nscss_select_ctx ctx;

	/* Firstly, construct inline stylesheet, if any */
	यदि (nsoption_bool(author_level_css)) {
		dom_exception err;
		err = dom_element_get_attribute(n, corestring_dom_style, &s);
		यदि (err != DOM_NO_ERR) {
			प्रत्यागम रिक्त;
		}
	}

	यदि (s != रिक्त) {
		inline_style = nscss_create_inline_style(
				(ध्रुव uint8_t *) dom_string_दत्त(s),
				dom_string_byte_दीर्घता(s),
				c->encoding,
				nsurl_access(c->आधारः_url),
				c->quirks != DOM_DOCUMENT_QUIRKS_बहुलकम्_NONE);

		dom_string_unref(s);

		यदि (inline_style == रिक्त)
			प्रत्यागम रिक्त;
	}

	/* Populate selection context */
	ctx.ctx = c->select_ctx;
	ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
	ctx.आधारः_url = c->आधारः_url;
	ctx.universal = c->universal;
	ctx.मूलम्_style = मूलम्_style;
	ctx.parent_style = parent_style;

	/* Select style for element */
	styles = nscss_get_style(&ctx, n, &c->media, &c->unit_len_ctx,
			inline_style);

	/* No longer need inline style */
	यदि (inline_style != रिक्त)
		css_stylesheet_destroy(inline_style);

	प्रत्यागम styles;
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
box_construct_generate(dom_ग्रन्थिः *n,
		       html_content *content,
		       संरचना box *box,
		       ध्रुव css_computed_style *style)
{
	संरचना box *gen = रिक्त;
	गणना css_display_e computed_display;
	ध्रुव css_computed_content_item *c_item;

	/* Nothing to generate if the parent box is not a block */
	यदि (box->type != BOX_खण्डम्)
		प्रत्यागम;

	/* To determine if an element has a pseudo element, we select
	 * for it and test to see if the returned style's content
	 * property is set to normal. */
	यदि (style == रिक्त ||
			css_computed_content(style, &c_item) ==
			CSS_CONTENT_प्रसामान्यम्) {
		/* No pseudo element */
		प्रत्यागम;
	}

	/* create box for this element */
	computed_display = ns_computed_display(style, box_is_मूलम्(n));
	यदि (computed_display == CSS_DISPLAY_खण्डम् ||
			computed_display == CSS_DISPLAY_सारणी) {
		/* currently only support block level boxes */

		/** \todo Not wise to drop const from the computed style */
		gen = box_create(रिक्त, (css_computed_style *) style,
				false, रिक्त, रिक्त, रिक्त, रिक्त, content->bctx);
		यदि (gen == रिक्त) {
			प्रत्यागम;
		}

		/* set box type from computed display */
		gen->type = box_map[ns_computed_display(
				style, box_is_मूलम्(n))];

		box_सङ्कलय_child(box, gen);
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
		     ध्रुव वर्ण *title,
		     संरचना box_construct_ctx *ctx,
		     संरचना box *parent)
{
	lwc_string *image_uri;
	संरचना box *marker;
	गणना css_list_style_type_e list_style_type;

	marker = box_create(रिक्त, box->style, false, रिक्त, रिक्त, title,
			रिक्त, ctx->bctx);
	यदि (marker == false)
		प्रत्यागम false;

	marker->type = BOX_खण्डम्;

	list_style_type = css_computed_list_style_type(box->style);

	/** \todo marker content (list-style-type) */
	विकल्प (list_style_type) {
	पक्ष CSS_LIST_STYLE_TYPE_DISC:
		/* 2022 BULLET */
		marker->text = (वर्ण *) "\342\200\242";
		marker->दीर्घता = 3;
		भङ्ग;

	पक्ष CSS_LIST_STYLE_TYPE_CIRCLE:
		/* 25CB WHITE CIRCLE */
		marker->text = (वर्ण *) "\342\227\213";
		marker->दीर्घता = 3;
		भङ्ग;

	पक्ष CSS_LIST_STYLE_TYPE_वर्गफलम्:
		/* 25AA BLACK SMALL SQUARE */
		marker->text = (वर्ण *) "\342\226\252";
		marker->दीर्घता = 3;
		भङ्ग;

	स्वभाव:
		/* Numerical list counters get handled in layout. */
		/* Fall through. */
	पक्ष CSS_LIST_STYLE_TYPE_NONE:
		marker->text = रिक्त;
		marker->दीर्घता = 0;
		भङ्ग;
	}

	यदि (css_computed_list_style_image(box->style, &image_uri) == CSS_LIST_STYLE_IMAGE_URI &&
	    (image_uri != रिक्त) &&
	    (nsoption_bool(foreground_images) == true)) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_दत्त(image_uri), &url);
		यदि (error != NSERROR_OK)
			प्रत्यागम false;

		यदि (html_fetch_object(ctx->content,
				      url,
				      marker,
				      image_types,
				      false) ==	false) {
			nsurl_unref(url);
			प्रत्यागम false;
		}
		nsurl_unref(url);
	}

	box->list_marker = marker;
	marker->parent = box;

	प्रत्यागम true;
}

स्थिर अन्तःस्थ bool box__style_is_float(ध्रुव संरचना box *box)
{
	प्रत्यागम css_computed_float(box->style) == CSS_FLOAT_वामः ||
	       css_computed_float(box->style) == CSS_FLOAT_दक्षिणः;
}

स्थिर अन्तःस्थ bool box__is_flex(ध्रुव संरचना box *box)
{
	प्रत्यागम box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX;
}

स्थिर अन्तःस्थ bool box_containing_खण्डम्_is_flex(
		ध्रुव संरचना box_construct_props *props)
{
	प्रत्यागम props->containing_खण्डम् != रिक्त &&
	       box__is_flex(props->containing_खण्डम्);
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
	ध्रुव css_computed_style *मूलम्_style = रिक्त;

	प्रतिज्ञा(ctx->n != रिक्त);

	box_extract_properties(ctx->n, &props);

	यदि (props.containing_खण्डम् != रिक्त) {
		/* In case the containing block is a pre block, we clear
		 * the PRE_STRIP flag since it is not used if we follow
		 * the pre with a tag */
		props.containing_खण्डम्->flags &= ~PRE_STRIP;
	}

	यदि (props.ग्रन्थिः_is_मूलम् == false) {
		मूलम्_style = ctx->मूलम्_box->style;
	}

	styles = box_get_style(ctx->content, props.parent_style, मूलम्_style,
			ctx->n);
	यदि (styles == रिक्त)
		प्रत्यागम false;

	/* Extract title attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_title, &title0);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (title0 != रिक्त) {
		वर्ण *t = squash_whitespace(dom_string_दत्त(title0));

		dom_string_unref(title0);

		यदि (t == रिक्त)
			प्रत्यागम false;

		props.title = talloc_strdup(ctx->bctx, t);

		मुक्त(t);

		यदि (props.title == रिक्त)
			प्रत्यागम false;
	}

	/* Extract id attribute, if present */
	err = dom_element_get_attribute(ctx->n, corestring_dom_id, &s);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (s != रिक्त) {
		err = dom_string_intern(s, &id);
		यदि (err != DOM_NO_ERR)
			id = रिक्त;

		dom_string_unref(s);
	}

	box = box_create(styles, styles->styles[CSS_PSEUDO_ELEMENT_NONE], false,
			props.href, props.target, props.title, id,
			ctx->bctx);
	यदि (box == रिक्त)
		प्रत्यागम false;

	/* If this is the root box, add it to the context */
	यदि (props.ग्रन्थिः_is_मूलम्)
		ctx->मूलम्_box = box;

	/* Deal with colspan/rowspan */
	err = dom_element_get_attribute(ctx->n, corestring_dom_colspan, &s);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (s != रिक्त) {
		ध्रुव वर्ण *val = dom_string_दत्त(s);

		/* Convert to a number, clamping to [1,1000] according to 4.9.11 */
		यदि ('0' <= val[0] && val[0] <= '9')
			box->columns = clamp(strtol(val, रिक्त, 10), 1, 1000);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(ctx->n, corestring_dom_rowspan, &s);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (s != रिक्त) {
		ध्रुव वर्ण *val = dom_string_दत्त(s);

		/* Convert to a number, clamping to [0,65534] according to 4.9.11 */
		यदि ('0' <= val[0] && val[0] <= '9')
			box->पङ्क्तिकाः = clamp(strtol(val, रिक्त, 10), 0, 65534);

		dom_string_unref(s);
	}

	css_display = ns_computed_display_static(box->style);

	/* Set box type from computed display */
	यदि ((css_computed_position(box->style) == CSS_POSITION_ABSOLUTE ||
	     css_computed_position(box->style) == CSS_POSITION_FIXED) &&
			(css_display == CSS_DISPLAY_INLINE ||
			 css_display == CSS_DISPLAY_INLINE_खण्डम् ||
			 css_display == CSS_DISPLAY_INLINE_सारणी ||
			 css_display == CSS_DISPLAY_INLINE_FLEX)) {
		/* Special case for absolute positioning: make absolute inlines
		 * into inline block so that the boxes are constructed in an
		 * inline container as if they were not absolutely positioned.
		 * Layout expects and handles this. */
		box->type = box_map[CSS_DISPLAY_INLINE_खण्डम्];
	} अन्यथा यदि (props.ग्रन्थिः_is_मूलम्) {
		/* Special case for root element: force it to BLOCK, or the
		 * rest of the layout will break. */
		box->type = BOX_खण्डम्;
	} अन्यथा {
		/* Normal mapping */
		box->type = box_map[ns_computed_display(box->style,
				props.ग्रन्थिः_is_मूलम्)];

		यदि (props.containing_खण्डम्->type == BOX_FLEX ||
		    props.containing_खण्डम्->type == BOX_INLINE_FLEX) {
			/* Blockification */
			विकल्प (box->type) {
			पक्ष BOX_INLINE_FLEX:
				box->type = BOX_FLEX;
				भङ्ग;
			पक्ष BOX_INLINE_खण्डम्:
				box->type = BOX_खण्डम्;
				भङ्ग;
			स्वभाव:
				भङ्ग;
			}
		}
	}

	यदि (convert_special_elements(ctx->n,
				     ctx->content,
				     box,
				     convert_children) == false) {
		प्रत्यागम false;
	}

	/* Handle the :before pseudo element */
	यदि (!(box->flags & IS_REPLACED)) {
		box_construct_generate(ctx->n, ctx->content, box,
				box->styles->styles[CSS_PSEUDO_ELEMENT_BEFORE]);
	}

	यदि (box->type == BOX_NONE || (ns_computed_display(box->style,
			props.ग्रन्थिः_is_मूलम्) == CSS_DISPLAY_NONE &&
			props.ग्रन्थिः_is_मूलम् == false)) {
		css_select_results_destroy(styles);
		box->styles = रिक्त;
		box->style = रिक्त;

		/* Invalidate associated gadget, if any */
		यदि (box->gadget != रिक्त) {
			box->gadget->box = रिक्त;
			box->gadget = रिक्त;
		}

		/* Can't do this, because the lifetimes of boxes and gadgets
		 * are inextricably linked. Fortunately, talloc will save us
		 * (for now) */
		/* box_free_box(box); */

		*convert_children = false;

		प्रत्यागम true;
	}

	/* Attach DOM node to box */
	err = dom_ग्रन्थिः_समुच्चयः_user_दत्त(ctx->n,
			corestring_dom_ns_कुञ्चिका_box_ग्रन्थिः_दत्त, box, रिक्त,
			(शून्य *) &old_box);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	/* Attach box to DOM node */
	box->ग्रन्थिः = dom_ग्रन्थिः_ref(ctx->n);

	यदि (props.inline_container == रिक्त &&
			(box->type == BOX_INLINE ||
			 box->type == BOX_BR ||
			 box->type == BOX_INLINE_खण्डम् ||
			 box->type == BOX_INLINE_FLEX ||
			 (box__style_is_float(box) &&
			  !box_containing_खण्डम्_is_flex(&props))) &&
			props.ग्रन्थिः_is_मूलम् == false) {
		/* Found an inline child of a block without a current container
		 * (i.e. this box is the first child of its parent, or was
		 * preceded by block-level siblings) */
		प्रतिज्ञा(props.containing_खण्डम् != रिक्त &&
				"Box must have containing block.");

		props.inline_container = box_create(रिक्त, रिक्त, false, रिक्त,
				रिक्त, रिक्त, रिक्त, ctx->bctx);
		यदि (props.inline_container == रिक्त)
			प्रत्यागम false;

		props.inline_container->type = BOX_INLINE_CONTAINER;

		box_सङ्कलय_child(props.containing_खण्डम्, props.inline_container);
	}

	/* Kick off fetch for any background image */
	यदि (css_computed_background_image(box->style, &bgimage_uri) ==
			CSS_BACKGROUND_IMAGE_IMAGE && bgimage_uri != रिक्त &&
			nsoption_bool(background_images) == true) {
		nsurl *url;
		nserror error;

		/* TODO: we get a url out of libcss as a lwc string, but
		 *       earlier we already had it as a nsurl after we
		 *       nsurl_joined it.  Can this be improved?
		 *       For now, just making another nsurl. */
		error = nsurl_create(lwc_string_दत्त(bgimage_uri), &url);
		यदि (error == NSERROR_OK) {
			/* Fetch image if we got a valid URL */
			यदि (html_fetch_object(ctx->content,
					      url,
					      box,
					      image_types,
					      true) == false) {
				nsurl_unref(url);
				प्रत्यागम false;
			}
			nsurl_unref(url);
		}
	}

	यदि (*convert_children)
		box->flags |= CONVERT_CHILDREN;

	यदि (box->type == BOX_INLINE || box->type == BOX_BR ||
			box->type == BOX_INLINE_FLEX ||
			box->type == BOX_INLINE_खण्डम्) {
		/* Inline container must exist, as we'll have
		 * created it above if it didn't */
		प्रतिज्ञा(props.inline_container != रिक्त);

		box_सङ्कलय_child(props.inline_container, box);
	} अन्यथा {
		यदि (ns_computed_display(box->style, props.ग्रन्थिः_is_मूलम्) ==
				CSS_DISPLAY_LIST_ITEM) {
			/* List item: compute marker */
			यदि (box_construct_marker(box, props.title, ctx,
					props.containing_खण्डम्) == false)
				प्रत्यागम false;
		}

		यदि (props.ग्रन्थिः_is_मूलम् == false &&
				box_containing_खण्डम्_is_flex(&props) == false &&
				(css_computed_float(box->style) ==
				CSS_FLOAT_वामः ||
				css_computed_float(box->style) ==
				CSS_FLOAT_दक्षिणः)) {
			/* Float: insert a float between the parent and box. */
			संरचना box *flt = box_create(रिक्त, रिक्त, false,
					props.href, props.target, props.title,
					रिक्त, ctx->bctx);
			यदि (flt == रिक्त)
				प्रत्यागम false;

			यदि (css_computed_float(box->style) == CSS_FLOAT_वामः)
				flt->type = BOX_FLOAT_वामः;
			अन्यथा
				flt->type = BOX_FLOAT_दक्षिणः;

			box_सङ्कलय_child(props.inline_container, flt);
			box_सङ्कलय_child(flt, box);
		} अन्यथा {
			/* Non-floated block-level box: add to containing block
			 * if there is one. If we're the root box, then there
			 * won't be. */
			यदि (props.containing_खण्डम् != रिक्त)
				box_सङ्कलय_child(props.containing_खण्डम्, box);
		}
	}

	प्रत्यागम true;
}


/**
 * Complete construction of the box tree for an element.
 *
 * \param n        DOM node to construct for
 * \param content  Containing document
 *
 * This will be called after all children of an element have been processed
 */
स्थिर शून्य box_construct_element_after(dom_ग्रन्थिः *n, html_content *content)
{
	संरचना box_construct_props props;
	संरचना box *box = box_for_ग्रन्थिः(n);

	प्रतिज्ञा(box != रिक्त);

	box_extract_properties(n, &props);

	यदि (box->type == BOX_INLINE || box->type == BOX_BR) {
		/* Insert INLINE_END into containing block */
		संरचना box *inline_अन्तः;
		bool has_children;
		dom_exception err;

		err = dom_ग्रन्थिः_has_child_nodes(n, &has_children);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम;

		यदि (has_children == false ||
				(box->flags & CONVERT_CHILDREN) == 0) {
			/* No children, or didn't want children converted */
			प्रत्यागम;
		}

		यदि (props.inline_container == रिक्त) {
			/* Create inline container if we don't have one */
			props.inline_container = box_create(रिक्त, रिक्त, false,
					रिक्त, रिक्त, रिक्त, रिक्त, content->bctx);
			यदि (props.inline_container == रिक्त)
				प्रत्यागम;

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_सङ्कलय_child(props.containing_खण्डम्,
					props.inline_container);
		}

		inline_अन्तः = box_create(रिक्त, box->style, false,
				box->href, box->target, box->title,
				box->id == रिक्त ? रिक्त :
				lwc_string_ref(box->id), content->bctx);
		यदि (inline_अन्तः != रिक्त) {
			inline_अन्तः->type = BOX_INLINE_अन्तः;

			प्रतिज्ञा(props.inline_container != रिक्त);

			box_सङ्कलय_child(props.inline_container, inline_अन्तः);

			box->inline_अन्तः = inline_अन्तः;
			inline_अन्तः->inline_अन्तः = box;
		}
	} अन्यथा यदि (!(box->flags & IS_REPLACED)) {
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
स्थिर dom_ग्रन्थिः *
अग्र_ग्रन्थिः(dom_ग्रन्थिः *n, html_content *content, bool convert_children)
{
	dom_ग्रन्थिः *अग्र = रिक्त;
	bool has_children;
	dom_exception err;

	err = dom_ग्रन्थिः_has_child_nodes(n, &has_children);
	यदि (err != DOM_NO_ERR) {
		dom_ग्रन्थिः_unref(n);
		प्रत्यागम रिक्त;
	}

	यदि (convert_children && has_children) {
		err = dom_ग्रन्थिः_get_first_child(n, &अग्र);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम रिक्त;
		}
		dom_ग्रन्थिः_unref(n);
	} अन्यथा {
		err = dom_ग्रन्थिः_get_अग्र_sibling(n, &अग्र);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम रिक्त;
		}

		यदि (अग्र != रिक्त) {
			यदि (box_for_ग्रन्थिः(n) != रिक्त)
				box_construct_element_after(n, content);
			dom_ग्रन्थिः_unref(n);
		} अन्यथा {
			यदि (box_for_ग्रन्थिः(n) != रिक्त)
				box_construct_element_after(n, content);

			यावत् (box_is_मूलम्(n) == false) {
				dom_ग्रन्थिः *parent = रिक्त;
				dom_ग्रन्थिः *parent_अग्र = रिक्त;

				err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(n, &parent);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(n);
					प्रत्यागम रिक्त;
				}

				प्रतिज्ञा(parent != रिक्त);

				err = dom_ग्रन्थिः_get_अग्र_sibling(parent,
						&parent_अग्र);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(parent);
					dom_ग्रन्थिः_unref(n);
					प्रत्यागम रिक्त;
				}

				यदि (parent_अग्र != रिक्त) {
					dom_ग्रन्थिः_unref(parent_अग्र);
					dom_ग्रन्थिः_unref(parent);
					भङ्ग;
				}

				dom_ग्रन्थिः_unref(n);
				n = parent;
				parent = रिक्त;

				यदि (box_for_ग्रन्थिः(n) != रिक्त) {
					box_construct_element_after(
							n, content);
				}
			}

			यदि (box_is_मूलम्(n) == false) {
				dom_ग्रन्थिः *parent = रिक्त;

				err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(n, &parent);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(n);
					प्रत्यागम रिक्त;
				}

				प्रतिज्ञा(parent != रिक्त);

				err = dom_ग्रन्थिः_get_अग्र_sibling(parent, &अग्र);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(parent);
					dom_ग्रन्थिः_unref(n);
					प्रत्यागम रिक्त;
				}

				यदि (box_for_ग्रन्थिः(parent) != रिक्त) {
					box_construct_element_after(parent,
							content);
				}

				dom_ग्रन्थिः_unref(parent);
			}

			dom_ग्रन्थिः_unref(n);
		}
	}

	प्रत्यागम अग्र;
}


/**
 * Apply the CSS text-transform property to given text for its ASCII chars.
 *
 * \param  s	string to transform
 * \param  len  length of s
 * \param  tt	transform type
 */
स्थिर शून्य
box_text_transform(वर्ण *s, अचिह्नित पूर्णाङ्क len, गणना css_text_transform_e tt)
{
	अचिह्नित पूर्णाङ्क i;
	यदि (len == 0)
		प्रत्यागम;
	विकल्प (tt) {
		पक्ष CSS_TEXT_TRANSFORM_UPPERCASE:
			प्रति (i = 0; i < len; ++i)
				यदि ((अचिह्नित वर्ण) s[i] < 0x80)
					s[i] = ascii_गन्तव्यम्_ऊर्ध्वस्थम्(s[i]);
			भङ्ग;
		पक्ष CSS_TEXT_TRANSFORM_LOWERCASE:
			प्रति (i = 0; i < len; ++i)
				यदि ((अचिह्नित वर्ण) s[i] < 0x80)
					s[i] = ascii_गन्तव्यम्_अधस्तनम्(s[i]);
			भङ्ग;
		पक्ष CSS_TEXT_TRANSFORM_CAPITALIZE:
			यदि ((अचिह्नित वर्ण) s[0] < 0x80)
				s[0] = ascii_गन्तव्यम्_ऊर्ध्वस्थम्(s[0]);
			प्रति (i = 1; i < len; ++i)
				यदि ((अचिह्नित वर्ण) s[i] < 0x80 &&
						ascii_is_space(s[i - 1]))
					s[i] = ascii_गन्तव्यम्_ऊर्ध्वस्थम्(s[i]);
			भङ्ग;
		स्वभाव:
			भङ्ग;
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

	प्रतिज्ञा(props.containing_खण्डम् != रिक्त);

	err = dom_characterdata_get_दत्त(ctx->n, &content);
	यदि (err != DOM_NO_ERR || content == रिक्त)
		प्रत्यागम false;

	यदि (css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_प्रसामान्यम् ||
			css_computed_white_space(props.parent_style) ==
			CSS_WHITE_SPACE_NOWRAP) {
		वर्ण *text;

		text = squash_whitespace(dom_string_दत्त(content));

		dom_string_unref(content);

		यदि (text == रिक्त)
			प्रत्यागम false;

		/* if the text is just a space, combine it with the preceding
		 * text node, if any */
		यदि (text[0] == ' ' && text[1] == 0) {
			यदि (props.inline_container != रिक्त) {
				प्रतिज्ञा(props.inline_container->चरमः != रिक्त);

				props.inline_container->चरमः->space =
						UNKNOWN_विस्तारः;
			}

			मुक्त(text);

			प्रत्यागम true;
		}

		यदि (props.inline_container == रिक्त) {
			/* Child of a block without a current container
			 * (i.e. this box is the first child of its parent, or
			 * was preceded by block-level siblings) */
			props.inline_container = box_create(रिक्त, रिक्त, false,
					रिक्त, रिक्त, रिक्त, रिक्त, ctx->bctx);
			यदि (props.inline_container == रिक्त) {
				मुक्त(text);
				प्रत्यागम false;
			}

			props.inline_container->type = BOX_INLINE_CONTAINER;

			box_सङ्कलय_child(props.containing_खण्डम्,
					props.inline_container);
		}

		/** \todo Dropping const here is not clever */
		box = box_create(रिक्त,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				रिक्त, ctx->bctx);
		यदि (box == रिक्त) {
			मुक्त(text);
			प्रत्यागम false;
		}

		box->type = BOX_TEXT;

		box->text = talloc_strdup(ctx->bctx, text);
		मुक्त(text);
		यदि (box->text == रिक्त)
			प्रत्यागम false;

		box->दीर्घता = वाक्यदैर्घ्यम्(box->text);

		/* strip ending space char off */
		यदि (box->दीर्घता > 1 && box->text[box->दीर्घता - 1] == ' ') {
			box->space = UNKNOWN_विस्तारः;
			box->दीर्घता--;
		}

		यदि (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(box->text, box->दीर्घता,
				css_computed_text_transform(
					props.parent_style));

		box_सङ्कलय_child(props.inline_container, box);

		यदि (box->text[0] == ' ') {
			box->दीर्घता--;

			memmove(box->text, &box->text[1], box->दीर्घता);

			यदि (box->पूर्व != रिक्त)
				box->पूर्व->space = UNKNOWN_विस्तारः;
		}
	} अन्यथा {
		/* white-space: pre */
		वर्ण *text;
		size_t text_len = dom_string_byte_दीर्घता(content);
		size_t i;
		वर्ण *current;
		गणना css_white_space_e white_space =
				css_computed_white_space(props.parent_style);

		/* note: pre-wrap/pre-line are unimplemented */
		प्रतिज्ञा(white_space == CSS_WHITE_SPACE_PRE ||
				white_space == CSS_WHITE_SPACE_PRE_पङ्क्तिः ||
				white_space == CSS_WHITE_SPACE_PRE_WRAP);

		text = स्मृतिं_देहि(text_len + 1);
		dom_string_unref(content);

		यदि (text == रिक्त)
			प्रत्यागम false;

		स्मृतिं_प्रतिलिख(text, dom_string_दत्त(content), text_len);
		text[text_len] = '\0';

		/* TODO: Handle tabs properly */
		प्रति (i = 0; i < text_len; i++)
			यदि (text[i] == '\t')
				text[i] = ' ';

		यदि (css_computed_text_transform(props.parent_style) !=
				CSS_TEXT_TRANSFORM_NONE)
			box_text_transform(text, वाक्यदैर्घ्यम्(text),
				css_computed_text_transform(
						props.parent_style));

		current = text;

		/* swallow a single leading new line */
		यदि (props.containing_खण्डम्->flags & PRE_STRIP) {
			विकल्प (*current) {
			पक्ष '\n':
				current++;
				भङ्ग;
			पक्ष '\r':
				current++;
				यदि (*current == '\n')
					current++;
				भङ्ग;
			}
			props.containing_खण्डम्->flags &= ~PRE_STRIP;
		}

		कुरु {
			size_t len = strcspn(current, "\r\n");

			वर्ण old = current[len];

			current[len] = 0;

			यदि (props.inline_container == रिक्त) {
				/* Child of a block without a current container
				 * (i.e. this box is the first child of its
				 * parent, or was preceded by block-level
				 * siblings) */
				props.inline_container = box_create(रिक्त, रिक्त,
						false, रिक्त, रिक्त, रिक्त, रिक्त,
						ctx->bctx);
				यदि (props.inline_container == रिक्त) {
					मुक्त(text);
					प्रत्यागम false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_सङ्कलय_child(props.containing_खण्डम्,
						props.inline_container);
			}

			/** \todo Dropping const isn't clever */
			box = box_create(रिक्त,
				(css_computed_style *) props.parent_style,
				false, props.href, props.target, props.title,
				रिक्त, ctx->bctx);
			यदि (box == रिक्त) {
				मुक्त(text);
				प्रत्यागम false;
			}

			box->type = BOX_TEXT;

			box->text = talloc_strdup(ctx->bctx, current);
			यदि (box->text == रिक्त) {
				मुक्त(text);
				प्रत्यागम false;
			}

			box->दीर्घता = वाक्यदैर्घ्यम्(box->text);

			box_सङ्कलय_child(props.inline_container, box);

			current[len] = old;

			current += len;

			यदि (current[0] != '\0') {
				/* Linebreak: create new inline container */
				props.inline_container = box_create(रिक्त, रिक्त,
						false, रिक्त, रिक्त, रिक्त, रिक्त,
						ctx->bctx);
				यदि (props.inline_container == रिक्त) {
					मुक्त(text);
					प्रत्यागम false;
				}

				props.inline_container->type =
						BOX_INLINE_CONTAINER;

				box_सङ्कलय_child(props.containing_खण्डम्,
						props.inline_container);

				यदि (current[0] == '\r' && current[1] == '\n')
					current += 2;
				अन्यथा
					current++;
			}
		} यावत् (*current);

		मुक्त(text);
	}

	प्रत्यागम true;
}


/**
 * Convert an ELEMENT node to a box tree fragment,
 * then schedule conversion of the next ELEMENT node
 */
स्थिर शून्य convert_xml_गन्तव्यम्_box(संरचना box_construct_ctx *ctx)
{
	dom_ग्रन्थिः *अग्र;
	bool convert_children;
	uint32_t अङ्कः_processed = 0;
	ध्रुव uint32_t max_processed_before_yield = 10;

	कुरु {
		convert_children = true;

		प्रतिज्ञा(ctx->n != रिक्त);

		यदि (box_construct_element(ctx, &convert_children) == false) {
			ctx->cb(ctx->content, false);
			dom_ग्रन्थिः_unref(ctx->n);
			मुक्त(ctx);
			प्रत्यागम;
		}

		/* Find next element to process, converting text nodes as we go */
		अग्र = अग्र_ग्रन्थिः(ctx->n, ctx->content, convert_children);
		यावत् (अग्र != रिक्त) {
			dom_ग्रन्थिः_type type;
			dom_exception err;

			err = dom_ग्रन्थिः_get_ग्रन्थिः_type(अग्र, &type);
			यदि (err != DOM_NO_ERR) {
				ctx->cb(ctx->content, false);
				dom_ग्रन्थिः_unref(अग्र);
				मुक्त(ctx);
				प्रत्यागम;
			}

			यदि (type == DOM_ELEMENT_ग्रन्थिः)
				भङ्ग;

			यदि (type == DOM_TEXT_ग्रन्थिः) {
				ctx->n = अग्र;
				यदि (box_construct_text(ctx) == false) {
					ctx->cb(ctx->content, false);
					dom_ग्रन्थिः_unref(ctx->n);
					मुक्त(ctx);
					प्रत्यागम;
				}
			}

			अग्र = अग्र_ग्रन्थिः(अग्र, ctx->content, true);
		}

		ctx->n = अग्र;

		यदि (अग्र == रिक्त) {
			/* Conversion complete */
			संरचना box मूलम्;

			स्मृतिं_पूरय(&मूलम्, 0, परिमाण(मूलम्));

			मूलम्.type = BOX_खण्डम्;
			मूलम्.children = मूलम्.चरमः = ctx->मूलम्_box;
			मूलम्.children->parent = &मूलम्;

			/** \todo Remove box_normalise_block */
			यदि (box_normalise_खण्डम्(&मूलम्, ctx->मूलम्_box,
					ctx->content) == false) {
				ctx->cb(ctx->content, false);
			} अन्यथा {
				ctx->content->layout = मूलम्.children;
				ctx->content->layout->parent = रिक्त;

				ctx->cb(ctx->content, true);
			}

			प्रतिज्ञा(ctx->n == रिक्त);

			मुक्त(ctx);
			प्रत्यागम;
		}
	} यावत् (++अङ्कः_processed < max_processed_before_yield);

	/* More work to do: schedule a continuation */
	guit->misc->कालसारणी(0, (शून्य *)convert_xml_गन्तव्यम्_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror
dom_गन्तव्यम्_box(dom_ग्रन्थिः *n,
	   html_content *c,
	   box_construct_complete_cb cb,
	   शून्य **box_conversion_context)
{
	संरचना box_construct_ctx *ctx;

	प्रतिज्ञा(box_conversion_context != रिक्त);

	यदि (c->bctx == रिक्त) {
		/* create a context allocation for this box tree */
		c->bctx = talloc_zero(0, पूर्णाङ्क);
		यदि (c->bctx == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	ctx = स्मृतिं_देहि(परिमाण(*ctx));
	यदि (ctx == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	ctx->content = c;
	ctx->n = dom_ग्रन्थिः_ref(n);
	ctx->मूलम्_box = रिक्त;
	ctx->cb = cb;
	ctx->bctx = c->bctx;

	*box_conversion_context = ctx;

	प्रत्यागम guit->misc->कालसारणी(0, (शून्य *)convert_xml_गन्तव्यम्_box, ctx);
}


/* exported function documented in html/box_construct.h */
nserror cancel_dom_गन्तव्यम्_box(शून्य *box_conversion_context)
{
	संरचना box_construct_ctx *ctx = box_conversion_context;
	nserror err;

	err = guit->misc->कालसारणी(-1, (शून्य *)convert_xml_गन्तव्यम्_box, ctx);
	यदि (err != NSERROR_OK) {
		प्रत्यागम err;
	}

	dom_ग्रन्थिः_unref(ctx->n);
	मुक्त(ctx);

	प्रत्यागम NSERROR_OK;
}


/* exported function documented in html/box_construct.h */
संरचना box *box_for_ग्रन्थिः(dom_ग्रन्थिः *n)
{
	संरचना box *box = रिक्त;
	dom_exception err;

	err = dom_ग्रन्थिः_get_user_दत्त(n, corestring_dom_ns_कुञ्चिका_box_ग्रन्थिः_दत्त,
			(शून्य *) &box);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम रिक्त;

	प्रत्यागम box;
}

/* exported function documented in html/box_construct.h */
bool
box_extract_link(ध्रुव html_content *content,
		 ध्रुव dom_string *dsrel,
		 nsurl *आधारः,
		 nsurl **परिणाम)
{
	वर्ण *s, *s1, *apos0 = 0, *apos1 = 0, *quot0 = 0, *quot1 = 0;
	अचिह्नित पूर्णाङ्क i, j, अन्तः;
	nserror error;
	ध्रुव वर्ण *rel;

	rel = dom_string_दत्त(dsrel);

	s1 = s = स्मृतिं_देहि(3 * वाक्यदैर्घ्यम्(rel) + 1);
	यदि (!s)
		प्रत्यागम false;

	/* copy to s, removing white space and control characters */
	प्रति (i = 0; rel[i] && ascii_is_space(rel[i]); i++)
		;
	प्रति (अन्तः = वाक्यदैर्घ्यम्(rel);
	     (अन्तः != i) && ascii_is_space(rel[अन्तः - 1]);
	     अन्तः--)
		;
	प्रति (j = 0; i != अन्तः; i++) {
		यदि ((अचिह्नित वर्ण) rel[i] < 0x20) {
			; /* skip control characters */
		} अन्यथा यदि (rel[i] == ' ') {
			s[j++] = '%';
			s[j++] = '2';
			s[j++] = '0';
		} अन्यथा {
			s[j++] = rel[i];
		}
	}
	s[j] = 0;

	यदि (content->enable_scripting == false) {
		/* extract first quoted string out of "javascript:" link */
		यदि (वाक्ये_तुलय(s, "javascript:", 11) == 0) {
			apos0 = strchr(s, '\'');
			यदि (apos0)
				apos1 = strchr(apos0 + 1, '\'');
			quot0 = strchr(s, '"');
			यदि (quot0)
				quot1 = strchr(quot0 + 1, '"');
			यदि (apos0 && apos1 &&
					(!quot0 || !quot1 || apos0 < quot0)) {
				*apos1 = 0;
				s1 = apos0 + 1;
			} अन्यथा यदि (quot0 && quot1) {
				*quot1 = 0;
				s1 = quot0 + 1;
			}
		}
	}

	/* construct absolute URL */
	error = nsurl_join(आधारः, s1, परिणाम);
	मुक्त(s);
	यदि (error != NSERROR_OK) {
		*परिणाम = रिक्त;
		प्रत्यागम false;
	}

	प्रत्यागम true;
}
