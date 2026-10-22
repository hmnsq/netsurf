/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/plot_style.h"
#अन्तर्भाव "netsurf/url_db.h"
#अन्तर्भाव "desktop/system_colour.h"

#अन्तर्भाव "css/internal.h"
#अन्तर्भाव "css/hints.h"
#अन्तर्भाव "css/select.h"

स्थिर css_error गाँठो_name(शून्य *pw, शून्य *गाँठो, css_qname *qname);
स्थिर css_error गाँठो_classes(शून्य *pw, शून्य *गाँठो,
		lwc_string ***classes, uint32_t *n_classes);
स्थिर css_error गाँठो_id(शून्य *pw, शून्य *गाँठो, lwc_string **id);
स्थिर css_error named_parent_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **parent);
स्थिर css_error named_sibling_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **sibling);
स्थिर css_error named_generic_sibling_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **sibling);
स्थिर css_error parent_गाँठो(शून्य *pw, शून्य *गाँठो, शून्य **parent);
स्थिर css_error sibling_गाँठो(शून्य *pw, शून्य *गाँठो, शून्य **sibling);
स्थिर css_error गाँठो_has_name(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, bool *match);
स्थिर css_error गाँठो_has_class(शून्य *pw, शून्य *गाँठो,
		lwc_string *name, bool *match);
स्थिर css_error गाँठो_has_id(शून्य *pw, शून्य *गाँठो,
		lwc_string *name, bool *match);
स्थिर css_error गाँठो_has_attribute(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, bool *match);
स्थिर css_error गाँठो_has_attribute_equal(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_has_attribute_dashmatch(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_has_attribute_includes(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_has_attribute_prefix(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_has_attribute_suffix(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_has_attribute_substring(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match);
स्थिर css_error गाँठो_is_जरा(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_गन्ती_siblings(शून्य *pw, शून्य *गाँठो,
		bool same_name, bool after, int32_t *गन्ती);
स्थिर css_error गाँठो_is_empty(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_link(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_hover(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_active(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_focus(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_enabled(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_disabled(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_checked(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_target(शून्य *pw, शून्य *गाँठो, bool *match);
स्थिर css_error गाँठो_is_lang(शून्य *pw, शून्य *गाँठो,
		lwc_string *lang, bool *match);
स्थिर css_error ua_default_for_property(शून्य *pw, uint32_t property,
		css_hint *hint);
स्थिर css_error set_libcss_गाँठो_data(शून्य *pw, शून्य *गाँठो,
		शून्य *libcss_गाँठो_data);
स्थिर css_error get_libcss_गाँठो_data(शून्य *pw, शून्य *गाँठो,
		शून्य **libcss_गाँठो_data);

/**
 * Selection callback table for libcss
 */
स्थिर css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	गाँठो_name,
	गाँठो_classes,
	गाँठो_id,
	named_ancestor_गाँठो,
	named_parent_गाँठो,
	named_sibling_गाँठो,
	named_generic_sibling_गाँठो,
	parent_गाँठो,
	sibling_गाँठो,
	गाँठो_has_name,
	गाँठो_has_class,
	गाँठो_has_id,
	गाँठो_has_attribute,
	गाँठो_has_attribute_equal,
	गाँठो_has_attribute_dashmatch,
	गाँठो_has_attribute_includes,
	गाँठो_has_attribute_prefix,
	गाँठो_has_attribute_suffix,
	गाँठो_has_attribute_substring,
	गाँठो_is_जरा,
	गाँठो_गन्ती_siblings,
	गाँठो_is_empty,
	गाँठो_is_link,
	गाँठो_is_visited,
	गाँठो_is_hover,
	गाँठो_is_active,
	गाँठो_is_focus,
	गाँठो_is_enabled,
	गाँठो_is_disabled,
	गाँठो_is_checked,
	गाँठो_is_target,
	गाँठो_is_lang,
	गाँठो_presentational_hint,
	ua_default_for_property,
	set_libcss_गाँठो_data,
	get_libcss_गाँठो_data,
};

/**
 * Create an inline style
 *
 * \param data          Source data
 * \param len           Length of data in bytes
 * \param charset       Charset of data, or NULL if unknown
 * \param url           Base URL of document containing data
 * \param allow_quirks  True to permit CSS parsing quirks
 * \return Pointer to stylesheet, or NULL on failure.
 */
css_stylesheet *nscss_create_inline_style(अचर uint8_t *data, माप_प्रकार len,
		अचर वर्ण *charset, अचर वर्ण *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = रिक्त;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = रिक्त;
	params.import = रिक्त;
	params.import_pw = रिक्त;
	params.color = ns_system_colour;
	params.color_pw = रिक्त;
	params.font = रिक्त;
	params.font_pw = रिक्त;

	error = css_stylesheet_create(&params, &sheet);
	यदि (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		फर्काऊ रिक्त;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	यदि (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		फर्काऊ रिक्त;
	}

	error = css_stylesheet_data_done(sheet);
	यदि (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		फर्काऊ रिक्त;
	}

	फर्काऊ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
स्थिर शून्य nscss_dom_user_data_handler(dom_गाँठो_operation operation,
		dom_string *साँचो, शून्य *data, संरचना dom_गाँठो *src,
		संरचना dom_गाँठो *dst)
{
	css_error error;

	यदि (dom_string_isequal(corestring_dom_ns_साँचो_libcss_गाँठो_data,
			साँचो) == false || data == रिक्त) {
		फर्काऊ;
	}

	विकल्प (operation) {
	पक्ष DOM_गाँठो_CLONED:
		error = css_libcss_गाँठो_data_handler(&selection_handler,
				CSS_गाँठो_CLONED,
				रिक्त, src, dst, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		रोक;

	पक्ष DOM_गाँठो_RENAMED:
		error = css_libcss_गाँठो_data_handler(&selection_handler,
				CSS_गाँठो_MODIFIED,
				रिक्त, src, रिक्त, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		रोक;

	पक्ष DOM_गाँठो_IMPORTED:
	पक्ष DOM_गाँठो_ADOPTED:
	पक्ष DOM_गाँठो_DELETED:
		error = css_libcss_गाँठो_data_handler(&selection_handler,
				CSS_गाँठो_DELETED,
				रिक्त, src, रिक्त, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		रोक;

	स्वभाव:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		प्रतिज्ञा(0);
	}
}

/**
 * Get style selection results for an element
 *
 * \param ctx             CSS selection context
 * \param n               Element to select for
 * \param media           Permitted media types
 * \param unit_unit_len_ctx    Unit length conversion context
 * \param inline_style    Inline style associated with element, or NULL
 * \return Pointer to selection results (containing computed styles),
 *         or NULL on failure
 */
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_गाँठो *n,
		अचर css_media *media,
		अचर css_unit_ctx *unit_len_ctx,
		अचर css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	पूर्णाङ्क pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	यदि (error != CSS_OK || styles == रिक्त) {
		/* Failed selecting partial style -- bail out */
		फर्काऊ रिक्त;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	यदि (ctx->parent_style != रिक्त) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		यदि (error != CSS_OK) {
			css_select_results_destroy(styles);
			फर्काऊ रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	लागि (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_गन्ती;
			pseudo_element++) {

		यदि (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_पङ्क्ति)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			जारी;

		यदि (styles->styles[pseudo_element] == रिक्त)
			/* There were no rules concerning this pseudo element */
			जारी;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		यदि (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			फर्काऊ रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	फर्काऊ styles;
}

/**
 * Get a blank style
 *
 * \param ctx           CSS selection context
 * \param unit_unit_len_ctx  Unit length conversion context
 * \param parent        Parent style to cascade inherited properties from
 * \return Pointer to blank style, or NULL on failure
 */
css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		अचर css_unit_ctx *unit_len_ctx,
		अचर css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	यदि (error != CSS_OK) {
		फर्काऊ रिक्त;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	यदि (error != CSS_OK) {
		css_computed_style_destroy(composed);
		फर्काऊ रिक्त;
	}

	फर्काऊ composed;
}

/******************************************************************************
 * Style selection callbacks                                                  *
 ******************************************************************************/

/**
 * Callback to retrieve a node's name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Pointer to location to receive node name
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 */
css_error गाँठो_name(शून्य *pw, शून्य *गाँठो, css_qname *qname)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_exception err;

	err = dom_गाँठो_get_गाँठो_name(n, &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	qname->ns = रिक्त;

	err = dom_string_intern(name, &qname->name);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		फर्काऊ CSS_NOMEM;
	}

	dom_string_unref(name);

	फर्काऊ CSS_OK;
}

/**
 * Callback to retrieve a node's classes.
 *
 * \param pw         HTML document
 * \param node       DOM node
 * \param classes    Pointer to location to receive class name array
 * \param n_classes  Pointer to location to receive length of class name array
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \note The returned array will be destroyed by libcss. Therefore, it must
 *       be allocated using the same allocator as used by libcss during style
 *       selection.
 */
css_error गाँठो_classes(शून्य *pw, शून्य *गाँठो,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_गाँठो *n = गाँठो;
	dom_exception err;

	*classes = रिक्त;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	फर्काऊ CSS_OK;
}

/**
 * Callback to retrieve a node's ID.
 *
 * \param pw    HTML document
 * \param node  DOM node
 * \param id    Pointer to location to receive id value
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 */
css_error गाँठो_id(शून्य *pw, शून्य *गाँठो, lwc_string **id)
{
	dom_गाँठो *n = गाँठो;
	dom_string *attr;
	dom_exception err;

	*id = रिक्त;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	यदि (attr != रिक्त) {
		err = dom_string_intern(attr, id);
		यदि (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			फर्काऊ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to find a named ancestor node.
 *
 * \param pw        HTML document
 * \param node      DOM node
 * \param qname     Node name to search for
 * \param ancestor  Pointer to location to receive ancestor
 * \return CSS_OK.
 *
 * \post \a ancestor will contain the result, or NULL if there is no match
 */
css_error named_ancestor_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **ancestor)
{
	dom_element_named_ancestor_गाँठो(गाँठो, qname->name,
			(संरचना dom_element **)ancestor);
	dom_गाँठो_unref(*ancestor);

	फर्काऊ CSS_OK;
}

/**
 * Callback to find a named parent node
 *
 * \param pw      HTML document
 * \param node    DOM node
 * \param qname   Node name to search for
 * \param parent  Pointer to location to receive parent
 * \return CSS_OK.
 *
 * \post \a parent will contain the result, or NULL if there is no match
 */
css_error named_parent_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **parent)
{
	dom_element_named_parent_गाँठो(गाँठो, qname->name,
			(संरचना dom_element **)parent);
	dom_गाँठो_unref(*parent);

	फर्काऊ CSS_OK;
}

/**
 * Callback to find a named sibling node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param qname    Node name to search for
 * \param sibling  Pointer to location to receive sibling
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
css_error named_sibling_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **sibling)
{
	dom_गाँठो *n = गाँठो;
	dom_गाँठो *prev;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_गाँठो_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_OK;

	जबसम्म (n != रिक्त) {
		dom_गाँठो_type type;

		err = dom_गाँठो_get_गाँठो_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाँठो)
			रोक;

		err = dom_गाँठो_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		dom_गाँठो_unref(n);
		n = prev;
	}

	यदि (n != रिक्त) {
		dom_string *name;

		err = dom_गाँठो_get_गाँठो_name(n, &name);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		dom_गाँठो_unref(n);

		यदि (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to find a named generic sibling node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param qname    Node name to search for
 * \param sibling  Pointer to location to receive ancestor
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
css_error named_generic_sibling_गाँठो(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, शून्य **sibling)
{
	dom_गाँठो *n = गाँठो;
	dom_गाँठो *prev;
	dom_exception err;

	*sibling = रिक्त;

	err = dom_गाँठो_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_OK;

	जबसम्म (n != रिक्त) {
		dom_गाँठो_type type;
		dom_string *name;

		err = dom_गाँठो_get_गाँठो_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाँठो) {
			err = dom_गाँठो_get_गाँठो_name(n, &name);
			यदि (err != DOM_NO_ERR) {
				dom_गाँठो_unref(n);
				फर्काऊ CSS_OK;
			}

			यदि (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_गाँठो_unref(n);
				*sibling = n;
				रोक;
			}
			dom_string_unref(name);
		}

		err = dom_गाँठो_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		dom_गाँठो_unref(n);
		n = prev;
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to retrieve the parent of a node.
 *
 * \param pw      HTML document
 * \param node    DOM node
 * \param parent  Pointer to location to receive parent
 * \return CSS_OK.
 *
 * \post \a parent will contain the result, or NULL if there is no match
 */
css_error parent_गाँठो(शून्य *pw, शून्य *गाँठो, शून्य **parent)
{
	dom_element_parent_गाँठो(गाँठो, (संरचना dom_element **)parent);
	dom_गाँठो_unref(*parent);

	फर्काऊ CSS_OK;
}

/**
 * Callback to retrieve the preceding sibling of a node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param sibling  Pointer to location to receive sibling
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
css_error sibling_गाँठो(शून्य *pw, शून्य *गाँठो, शून्य **sibling)
{
	dom_गाँठो *n = गाँठो;
	dom_गाँठो *prev;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_गाँठो_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_OK;

	जबसम्म (n != रिक्त) {
		dom_गाँठो_type type;

		err = dom_गाँठो_get_गाँठो_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाँठो)
			रोक;

		err = dom_गाँठो_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_OK;
		}

		dom_गाँठो_unref(n);
		n = prev;
	}

	यदि (n != रिक्त) {
		/** \todo Sort out reference counting */
		dom_गाँठो_unref(n);

		*sibling = n;
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has the given name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_name(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_गाँठो *n = गाँठो;

	यदि (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_गाँठो_get_गाँठो_name(n, &name);
		यदि (err != DOM_NO_ERR)
			फर्काऊ CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has the given class.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param name   Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_class(शून्य *pw, शून्य *गाँठो,
		lwc_string *name, bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	प्रतिज्ञा(err == DOM_NO_ERR);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has the given id.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param name   Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_id(शून्य *pw, शून्य *गाँठो,
		lwc_string *name, bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_OK;

	यदि (attr != रिक्त) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(अचर uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with given name and value.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_equal(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_length(मान);

	यदि (vlen == 0) {
		*match = false;
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, मान);

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value dashmatches that given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_dashmatch(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_length(मान);

	यदि (vlen == 0) {
		*match = false;
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मान);

	/* check for dashmatch */
	यदि (*match == false) {
		अचर वर्ण *vdata = lwc_string_data(मान);
		अचर वर्ण *data = (अचर वर्ण *) dom_string_data(atr_val);
		माप_प्रकार len = dom_string_byte_length(atr_val);

		यदि (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value includes that given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_includes(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	माप_प्रकार vlen = lwc_string_length(मान);
	अचर वर्ण *p;
	अचर वर्ण *start;
	अचर वर्ण *end;

	*match = false;

	यदि (vlen == 0) {
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (अचर वर्ण *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	लागि (p = start; p <= end; p++) {
		यदि (*p == ' ' || *p == '\0') {
			यदि ((माप_प्रकार) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(मान),
					vlen) == 0) {
				*match = true;
				रोक;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value has the prefix given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_prefix(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_length(मान);

	यदि (vlen == 0) {
		*match = false;
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मान);

	/* check for prefix match */
	यदि (*match == false) {
		अचर वर्ण *data = (अचर वर्ण *) dom_string_data(atr_val);
		माप_प्रकार len = dom_string_byte_length(atr_val);

		यदि ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(मान), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value has the suffix given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_suffix(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_length(मान);

	यदि (vlen == 0) {
		*match = false;
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मान);

	/* check for prefix match */
	यदि (*match == false) {
		अचर वर्ण *data = (अचर वर्ण *) dom_string_data(atr_val);
		माप_प्रकार len = dom_string_byte_length(atr_val);

		अचर वर्ण *start = (वर्ण *) data + len - vlen;

		यदि ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(मान), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value contains the substring given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_has_attribute_substring(शून्य *pw, शून्य *गाँठो,
		अचर css_qname *qname, lwc_string *मान,
		bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_length(मान);

	यदि (vlen == 0) {
		*match = false;
		फर्काऊ CSS_OK;
	}

	err = dom_string_create_interned(
		(अचर uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		फर्काऊ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		फर्काऊ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मान);

	/* check for prefix match */
	यदि (*match == false) {
		अचर वर्ण *vdata = lwc_string_data(मान);
		अचर वर्ण *start = (अचर वर्ण *) dom_string_data(atr_val);
		माप_प्रकार len = dom_string_byte_length(atr_val);
		अचर वर्ण *last_start = start + len - vlen;

		यदि (len >= vlen) {
			जबसम्म (start <= last_start) {
				यदि (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					रोक;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is the root node of the document.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_जरा(शून्य *pw, शून्य *गाँठो, bool *match)
{
	dom_गाँठो *n = गाँठो;
	dom_गाँठो *parent;
	dom_गाँठो_type type;
	dom_exception err;

	err = dom_गाँठो_get_parent_गाँठो(n, &parent);
	यदि (err != DOM_NO_ERR) {
		फर्काऊ CSS_NOMEM;
	}

	यदि (parent != रिक्त) {
		err = dom_गाँठो_get_गाँठो_type(parent, &type);

		dom_गाँठो_unref(parent);

		यदि (err != DOM_NO_ERR)
			फर्काऊ CSS_NOMEM;

		यदि (type != DOM_DOCUMENT_गाँठो) {
			*match = false;
			फर्काऊ CSS_OK;
		}
	}

	*match = true;

	फर्काऊ CSS_OK;
}

स्थिर पूर्णाङ्क
गाँठो_गन्ती_siblings_check(dom_गाँठो *गाँठो,
			  bool check_name,
			  dom_string *name)
{
	dom_गाँठो_type type;
	पूर्णाङ्क ret = 0;
	dom_exception exc;

	यदि (गाँठो == रिक्त)
		फर्काऊ 0;

	exc = dom_गाँठो_get_गाँठो_type(गाँठो, &type);
	यदि ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_गाँठो)) {
		फर्काऊ 0;
	}

	यदि (check_name) {
		dom_string *गाँठो_name = रिक्त;
		exc = dom_गाँठो_get_गाँठो_name(गाँठो, &गाँठो_name);

		यदि ((exc == DOM_NO_ERR) && (गाँठो_name != रिक्त)) {

			यदि (dom_string_caseless_isequal(name,
							गाँठो_name)) {
				ret = 1;
			}
			dom_string_unref(गाँठो_name);
		}
	} अन्यथा {
		ret = 1;
	}

	फर्काऊ ret;
}

/**
 * Callback to count a node's siblings.
 *
 * \param pw         HTML document
 * \param n          DOM node
 * \param same_name  Only count siblings with the same name, or all
 * \param after      Count anteceding instead of preceding siblings
 * \param count      Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a count will contain the number of siblings
 */
css_error गाँठो_गन्ती_siblings(शून्य *pw, शून्य *n, bool same_name,
		bool after, int32_t *गन्ती)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *गाँठो_name = रिक्त;

	यदि (same_name) {
		dom_गाँठो *गाँठो = n;
		exc = dom_गाँठो_get_गाँठो_name(गाँठो, &गाँठो_name);
		यदि ((exc != DOM_NO_ERR) || (गाँठो_name == रिक्त)) {
			फर्काऊ CSS_NOMEM;
		}
	}

	यदि (after) {
		dom_गाँठो *गाँठो = dom_गाँठो_ref(n);
		dom_गाँठो *next;

		गर {
			exc = dom_गाँठो_get_next_sibling(गाँठो, &next);
			यदि ((exc != DOM_NO_ERR))
				रोक;

			dom_गाँठो_unref(गाँठो);
			गाँठो = next;

			cnt += गाँठो_गन्ती_siblings_check(गाँठो, same_name, गाँठो_name);
		} जबसम्म (गाँठो != रिक्त);
	} अन्यथा {
		dom_गाँठो *गाँठो = dom_गाँठो_ref(n);
		dom_गाँठो *next;

		गर {
			exc = dom_गाँठो_get_previous_sibling(गाँठो, &next);
			यदि ((exc != DOM_NO_ERR))
				रोक;

			dom_गाँठो_unref(गाँठो);
			गाँठो = next;

			cnt += गाँठो_गन्ती_siblings_check(गाँठो, same_name, गाँठो_name);

		} जबसम्म (गाँठो != रिक्त);
	}

	यदि (गाँठो_name != रिक्त) {
		dom_string_unref(गाँठो_name);
	}

	*गन्ती = cnt;
	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is empty.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node is empty and false otherwise.
 */
css_error गाँठो_is_empty(शून्य *pw, शून्य *गाँठो, bool *match)
{
	dom_गाँठो *n = गाँठो, *next;
	dom_exception err;

	*match = true;

	err = dom_गाँठो_get_first_child(n, &n);
	यदि (err != DOM_NO_ERR) {
		फर्काऊ CSS_BADPARM;
	}

	जबसम्म (n != रिक्त) {
		dom_गाँठो_type ntype;
		err = dom_गाँठो_get_गाँठो_type(n, &ntype);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_BADPARM;
		}

		यदि (ntype == DOM_ELEMENT_गाँठो ||
		    ntype == DOM_TEXT_गाँठो) {
			*match = false;
			dom_गाँठो_unref(n);
			रोक;
		}

		err = dom_गाँठो_get_next_sibling(n, &next);
		यदि (err != DOM_NO_ERR) {
			dom_गाँठो_unref(n);
			फर्काऊ CSS_BADPARM;
		}
		dom_गाँठो_unref(n);
		n = next;
	}

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is a linking element.
 *
 * \param pw     HTML document
 * \param n      DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_link(शून्य *pw, शून्य *n, bool *match)
{
	dom_गाँठो *गाँठो = n;
	dom_exception exc;
	dom_string *गाँठो_name = रिक्त;

	exc = dom_गाँठो_get_गाँठो_name(गाँठो, &गाँठो_name);
	यदि ((exc != DOM_NO_ERR) || (गाँठो_name == रिक्त)) {
		फर्काऊ CSS_NOMEM;
	}

	यदि (dom_string_caseless_lwc_isequal(गाँठो_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(गाँठो, corestring_dom_href,
				&has_href);
		यदि ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} अन्यथा {
			*match = false;
		}
	} अन्यथा {
		*match = false;
	}
	dom_string_unref(गाँठो_name);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is a linking element whose target has been
 * visited.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_visited(शून्य *pw, शून्य *गाँठो, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	अचर संरचना url_data *data;

	dom_exception exc;
	dom_गाँठो *n = गाँठो;
	dom_string *s = रिक्त;

	*match = false;

	exc = dom_गाँठो_get_गाँठो_name(n, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		फर्काऊ CSS_NOMEM;
	}

	यदि (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		फर्काऊ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = रिक्त;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		/* Can't be visited; not got a URL */
		फर्काऊ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	यदि (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		फर्काऊ CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	यदि (data != रिक्त && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is currently being hovered over.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_hover(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is currently activated.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_active(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has the input focus.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_focus(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is enabled.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is enabled and false otherwise.
 */
css_error गाँठो_is_enabled(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is disabled.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is disabled and false otherwise.
 */
css_error गाँठो_is_disabled(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is checked.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is checked and false otherwise.
 */
css_error गाँठो_is_checked(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node is the target of the document URL.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_target(शून्य *pw, शून्य *गाँठो, bool *match)
{
	/** \todo Support target */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to determine if a node has the given language
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param lang   Language specifier to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
css_error गाँठो_is_lang(शून्य *pw, शून्य *गाँठो,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	फर्काऊ CSS_OK;
}

/**
 * Callback to retrieve the User-Agent defaults for a CSS property.
 *
 * \param pw        HTML document
 * \param property  Property to retrieve defaults for
 * \param hint      Pointer to hint object to populate
 * \return CSS_OK       on success,
 *         CSS_INVALID  if the property should not have a user-agent default.
 */
css_error ua_default_for_property(शून्य *pw, uint32_t property, css_hint *hint)
{
	यदि (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} अन्यथा यदि (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = रिक्त;
		विकल्प (nsoption_int(font_default)) {
		पक्ष PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			रोक;
		पक्ष PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			रोक;
		पक्ष PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			रोक;
		पक्ष PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			रोक;
		पक्ष PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			रोक;
		}
	} अन्यथा यदि (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = रिक्त;
		hint->status = CSS_QUOTES_NONE;
	} अन्यथा यदि (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = रिक्त;
		hint->status = 0;
	} अन्यथा {
		फर्काऊ CSS_INVALID;
	}

	फर्काऊ CSS_OK;
}

css_error set_libcss_गाँठो_data(शून्य *pw, शून्य *गाँठो, शून्य *libcss_गाँठो_data)
{
	dom_गाँठो *n = गाँठो;
	dom_exception err;
	शून्य *old_गाँठो_data;

	/* Set this node's node data */
	err = dom_गाँठो_set_user_data(n,
			corestring_dom_ns_साँचो_libcss_गाँठो_data,
			libcss_गाँठो_data, nscss_dom_user_data_handler,
			(शून्य *) &old_गाँठो_data);
	यदि (err != DOM_NO_ERR) {
		फर्काऊ CSS_NOMEM;
	}

	प्रतिज्ञा(old_गाँठो_data == रिक्त);

	फर्काऊ CSS_OK;
}

css_error get_libcss_गाँठो_data(शून्य *pw, शून्य *गाँठो, शून्य **libcss_गाँठो_data)
{
	dom_गाँठो *n = गाँठो;
	dom_exception err;

	/* Get this node's node data */
	err = dom_गाँठो_get_user_data(n,
			corestring_dom_ns_साँचो_libcss_गाँठो_data,
			libcss_गाँठो_data);
	यदि (err != DOM_NO_ERR) {
		फर्काऊ CSS_NOMEM;
	}

	फर्काऊ CSS_OK;
}
