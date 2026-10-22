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

स्थिर css_error गाठ_name(शून्य *pw, शून्य *गाठ, css_qname *qname);
स्थिर css_error गाठ_classes(शून्य *pw, शून्य *गाठ,
		lwc_string ***classes, uint32_t *n_classes);
स्थिर css_error गाठ_id(शून्य *pw, शून्य *गाठ, lwc_string **id);
स्थिर css_error named_parent_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **parent);
स्थिर css_error named_sibling_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **sibling);
स्थिर css_error named_generic_sibling_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **sibling);
स्थिर css_error parent_गाठ(शून्य *pw, शून्य *गाठ, शून्य **parent);
स्थिर css_error sibling_गाठ(शून्य *pw, शून्य *गाठ, शून्य **sibling);
स्थिर css_error गाठ_has_name(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, bool *match);
स्थिर css_error गाठ_has_class(शून्य *pw, शून्य *गाठ,
		lwc_string *name, bool *match);
स्थिर css_error गाठ_has_id(शून्य *pw, शून्य *गाठ,
		lwc_string *name, bool *match);
स्थिर css_error गाठ_has_attribute(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, bool *match);
स्थिर css_error गाठ_has_attribute_equal(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_has_attribute_dashmatch(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_has_attribute_includes(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_has_attribute_prefix(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_has_attribute_suffix(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_has_attribute_substring(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match);
स्थिर css_error गाठ_is_मूळ(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_मोजणी_siblings(शून्य *pw, शून्य *गाठ,
		bool same_name, bool after, int32_t *मोजणी);
स्थिर css_error गाठ_is_empty(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_link(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_hover(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_active(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_focus(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_enabled(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_disabled(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_checked(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_target(शून्य *pw, शून्य *गाठ, bool *match);
स्थिर css_error गाठ_is_lang(शून्य *pw, शून्य *गाठ,
		lwc_string *lang, bool *match);
स्थिर css_error ua_default_for_property(शून्य *pw, uint32_t property,
		css_hint *hint);
स्थिर css_error set_libcss_गाठ_data(शून्य *pw, शून्य *गाठ,
		शून्य *libcss_गाठ_data);
स्थिर css_error get_libcss_गाठ_data(शून्य *pw, शून्य *गाठ,
		शून्य **libcss_गाठ_data);

/**
 * Selection callback table for libcss
 */
स्थिर css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	गाठ_name,
	गाठ_classes,
	गाठ_id,
	named_ancestor_गाठ,
	named_parent_गाठ,
	named_sibling_गाठ,
	named_generic_sibling_गाठ,
	parent_गाठ,
	sibling_गाठ,
	गाठ_has_name,
	गाठ_has_class,
	गाठ_has_id,
	गाठ_has_attribute,
	गाठ_has_attribute_equal,
	गाठ_has_attribute_dashmatch,
	गाठ_has_attribute_includes,
	गाठ_has_attribute_prefix,
	गाठ_has_attribute_suffix,
	गाठ_has_attribute_substring,
	गाठ_is_मूळ,
	गाठ_मोजणी_siblings,
	गाठ_is_empty,
	गाठ_is_link,
	गाठ_is_visited,
	गाठ_is_hover,
	गाठ_is_active,
	गाठ_is_focus,
	गाठ_is_enabled,
	गाठ_is_disabled,
	गाठ_is_checked,
	गाठ_is_target,
	गाठ_is_lang,
	गाठ_presentational_hint,
	ua_default_for_property,
	set_libcss_गाठ_data,
	get_libcss_गाठ_data,
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
css_stylesheet *nscss_create_inline_style(ध्रुव uint8_t *data, size_t len,
		ध्रुव वर्ण *charset, ध्रुव वर्ण *url, bool allow_quirks)
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
		प्रत्यागम रिक्त;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	यदि (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		प्रत्यागम रिक्त;
	}

	error = css_stylesheet_data_done(sheet);
	यदि (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
स्थिर शून्य nscss_dom_user_data_handler(dom_गाठ_operation operation,
		dom_string *किल्ली, शून्य *data, संरचना dom_गाठ *src,
		संरचना dom_गाठ *dst)
{
	css_error error;

	यदि (dom_string_isequal(corestring_dom_ns_किल्ली_libcss_गाठ_data,
			किल्ली) == false || data == रिक्त) {
		प्रत्यागम;
	}

	विकल्प (operation) {
	पक्ष DOM_गाठ_CLONED:
		error = css_libcss_गाठ_data_handler(&selection_handler,
				CSS_गाठ_CLONED,
				रिक्त, src, dst, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		भङ्ग;

	पक्ष DOM_गाठ_RENAMED:
		error = css_libcss_गाठ_data_handler(&selection_handler,
				CSS_गाठ_MODIFIED,
				रिक्त, src, रिक्त, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		भङ्ग;

	पक्ष DOM_गाठ_IMPORTED:
	पक्ष DOM_गाठ_ADOPTED:
	पक्ष DOM_गाठ_DELETED:
		error = css_libcss_गाठ_data_handler(&selection_handler,
				CSS_गाठ_DELETED,
				रिक्त, src, रिक्त, data);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		भङ्ग;

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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_गाठ *n,
		ध्रुव css_media *media,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_stylesheet *inline_style)
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
		प्रत्यागम रिक्त;
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
			प्रत्यागम रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	प्रति (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_मोजणी;
			pseudo_element++) {

		यदि (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_ओळ)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			अनुवर्त;

		यदि (styles->styles[pseudo_element] == रिक्त)
			/* There were no rules concerning this pseudo element */
			अनुवर्त;

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
			प्रत्यागम रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	प्रत्यागम styles;
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
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	यदि (error != CSS_OK) {
		प्रत्यागम रिक्त;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	यदि (error != CSS_OK) {
		css_computed_style_destroy(composed);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम composed;
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
css_error गाठ_name(शून्य *pw, शून्य *गाठ, css_qname *qname)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_exception err;

	err = dom_गाठ_get_गाठ_name(n, &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	qname->ns = रिक्त;

	err = dom_string_intern(name, &qname->name);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		प्रत्यागम CSS_NOMEM;
	}

	dom_string_unref(name);

	प्रत्यागम CSS_OK;
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
css_error गाठ_classes(शून्य *pw, शून्य *गाठ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_गाठ *n = गाठ;
	dom_exception err;

	*classes = रिक्त;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	प्रत्यागम CSS_OK;
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
css_error गाठ_id(शून्य *pw, शून्य *गाठ, lwc_string **id)
{
	dom_गाठ *n = गाठ;
	dom_string *attr;
	dom_exception err;

	*id = रिक्त;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	यदि (attr != रिक्त) {
		err = dom_string_intern(attr, id);
		यदि (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			प्रत्यागम CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	प्रत्यागम CSS_OK;
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
css_error named_ancestor_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **ancestor)
{
	dom_element_named_ancestor_गाठ(गाठ, qname->name,
			(संरचना dom_element **)ancestor);
	dom_गाठ_unref(*ancestor);

	प्रत्यागम CSS_OK;
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
css_error named_parent_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **parent)
{
	dom_element_named_parent_गाठ(गाठ, qname->name,
			(संरचना dom_element **)parent);
	dom_गाठ_unref(*parent);

	प्रत्यागम CSS_OK;
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
css_error named_sibling_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **sibling)
{
	dom_गाठ *n = गाठ;
	dom_गाठ *prev;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_गाठ_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_गाठ_type type;

		err = dom_गाठ_get_गाठ_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाठ)
			भङ्ग;

		err = dom_गाठ_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_गाठ_unref(n);
		n = prev;
	}

	यदि (n != रिक्त) {
		dom_string *name;

		err = dom_गाठ_get_गाठ_name(n, &name);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_गाठ_unref(n);

		यदि (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	प्रत्यागम CSS_OK;
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
css_error named_generic_sibling_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **sibling)
{
	dom_गाठ *n = गाठ;
	dom_गाठ *prev;
	dom_exception err;

	*sibling = रिक्त;

	err = dom_गाठ_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_गाठ_type type;
		dom_string *name;

		err = dom_गाठ_get_गाठ_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाठ) {
			err = dom_गाठ_get_गाठ_name(n, &name);
			यदि (err != DOM_NO_ERR) {
				dom_गाठ_unref(n);
				प्रत्यागम CSS_OK;
			}

			यदि (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_गाठ_unref(n);
				*sibling = n;
				भङ्ग;
			}
			dom_string_unref(name);
		}

		err = dom_गाठ_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_गाठ_unref(n);
		n = prev;
	}

	प्रत्यागम CSS_OK;
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
css_error parent_गाठ(शून्य *pw, शून्य *गाठ, शून्य **parent)
{
	dom_element_parent_गाठ(गाठ, (संरचना dom_element **)parent);
	dom_गाठ_unref(*parent);

	प्रत्यागम CSS_OK;
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
css_error sibling_गाठ(शून्य *pw, शून्य *गाठ, शून्य **sibling)
{
	dom_गाठ *n = गाठ;
	dom_गाठ *prev;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_गाठ_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_गाठ_type type;

		err = dom_गाठ_get_गाठ_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_गाठ)
			भङ्ग;

		err = dom_गाठ_get_previous_sibling(n, &prev);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_गाठ_unref(n);
		n = prev;
	}

	यदि (n != रिक्त) {
		/** \todo Sort out reference counting */
		dom_गाठ_unref(n);

		*sibling = n;
	}

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_name(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_गाठ *n = गाठ;

	यदि (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_गाठ_get_गाठ_name(n, &name);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_class(शून्य *pw, शून्य *गाठ,
		lwc_string *name, bool *match)
{
	dom_गाठ *n = गाठ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	प्रतिज्ञा(err == DOM_NO_ERR);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_id(शून्य *pw, शून्य *गाठ,
		lwc_string *name, bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यदि (attr != रिक्त) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ध्रुव uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_equal(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(मूल्य);

	यदि (vlen == 0) {
		*match = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_dashmatch(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(मूल्य);

	यदि (vlen == 0) {
		*match = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for dashmatch */
	यदि (*match == false) {
		ध्रुव वर्ण *vdata = lwc_string_data(मूल्य);
		ध्रुव वर्ण *data = (ध्रुव वर्ण *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		यदि (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_includes(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(मूल्य);
	ध्रुव वर्ण *p;
	ध्रुव वर्ण *start;
	ध्रुव वर्ण *end;

	*match = false;

	यदि (vlen == 0) {
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ध्रुव वर्ण *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	प्रति (p = start; p <= end; p++) {
		यदि (*p == ' ' || *p == '\0') {
			यदि ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(मूल्य),
					vlen) == 0) {
				*match = true;
				भङ्ग;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_prefix(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(मूल्य);

	यदि (vlen == 0) {
		*match = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	यदि (*match == false) {
		ध्रुव वर्ण *data = (ध्रुव वर्ण *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		यदि ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(मूल्य), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_suffix(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(मूल्य);

	यदि (vlen == 0) {
		*match = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	यदि (*match == false) {
		ध्रुव वर्ण *data = (ध्रुव वर्ण *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ध्रुव वर्ण *start = (वर्ण *) data + len - vlen;

		यदि ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(मूल्य), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_has_attribute_substring(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाठ *n = गाठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(मूल्य);

	यदि (vlen == 0) {
		*match = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*match = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	यदि (*match == false) {
		ध्रुव वर्ण *vdata = lwc_string_data(मूल्य);
		ध्रुव वर्ण *start = (ध्रुव वर्ण *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ध्रुव वर्ण *last_start = start + len - vlen;

		यदि (len >= vlen) {
			यावत् (start <= last_start) {
				यदि (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					भङ्ग;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_मूळ(शून्य *pw, शून्य *गाठ, bool *match)
{
	dom_गाठ *n = गाठ;
	dom_गाठ *parent;
	dom_गाठ_type type;
	dom_exception err;

	err = dom_गाठ_get_parent_गाठ(n, &parent);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (parent != रिक्त) {
		err = dom_गाठ_get_गाठ_type(parent, &type);

		dom_गाठ_unref(parent);

		यदि (err != DOM_NO_ERR)
			प्रत्यागम CSS_NOMEM;

		यदि (type != DOM_DOCUMENT_गाठ) {
			*match = false;
			प्रत्यागम CSS_OK;
		}
	}

	*match = true;

	प्रत्यागम CSS_OK;
}

स्थिर पूर्णाङ्क
गाठ_मोजणी_siblings_check(dom_गाठ *गाठ,
			  bool check_name,
			  dom_string *name)
{
	dom_गाठ_type type;
	पूर्णाङ्क ret = 0;
	dom_exception exc;

	यदि (गाठ == रिक्त)
		प्रत्यागम 0;

	exc = dom_गाठ_get_गाठ_type(गाठ, &type);
	यदि ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_गाठ)) {
		प्रत्यागम 0;
	}

	यदि (check_name) {
		dom_string *गाठ_name = रिक्त;
		exc = dom_गाठ_get_गाठ_name(गाठ, &गाठ_name);

		यदि ((exc == DOM_NO_ERR) && (गाठ_name != रिक्त)) {

			यदि (dom_string_caseless_isequal(name,
							गाठ_name)) {
				ret = 1;
			}
			dom_string_unref(गाठ_name);
		}
	} अन्यथा {
		ret = 1;
	}

	प्रत्यागम ret;
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
css_error गाठ_मोजणी_siblings(शून्य *pw, शून्य *n, bool same_name,
		bool after, int32_t *मोजणी)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *गाठ_name = रिक्त;

	यदि (same_name) {
		dom_गाठ *गाठ = n;
		exc = dom_गाठ_get_गाठ_name(गाठ, &गाठ_name);
		यदि ((exc != DOM_NO_ERR) || (गाठ_name == रिक्त)) {
			प्रत्यागम CSS_NOMEM;
		}
	}

	यदि (after) {
		dom_गाठ *गाठ = dom_गाठ_ref(n);
		dom_गाठ *next;

		कुरु {
			exc = dom_गाठ_get_next_sibling(गाठ, &next);
			यदि ((exc != DOM_NO_ERR))
				भङ्ग;

			dom_गाठ_unref(गाठ);
			गाठ = next;

			cnt += गाठ_मोजणी_siblings_check(गाठ, same_name, गाठ_name);
		} यावत् (गाठ != रिक्त);
	} अन्यथा {
		dom_गाठ *गाठ = dom_गाठ_ref(n);
		dom_गाठ *next;

		कुरु {
			exc = dom_गाठ_get_previous_sibling(गाठ, &next);
			यदि ((exc != DOM_NO_ERR))
				भङ्ग;

			dom_गाठ_unref(गाठ);
			गाठ = next;

			cnt += गाठ_मोजणी_siblings_check(गाठ, same_name, गाठ_name);

		} यावत् (गाठ != रिक्त);
	}

	यदि (गाठ_name != रिक्त) {
		dom_string_unref(गाठ_name);
	}

	*मोजणी = cnt;
	प्रत्यागम CSS_OK;
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
css_error गाठ_is_empty(शून्य *pw, शून्य *गाठ, bool *match)
{
	dom_गाठ *n = गाठ, *next;
	dom_exception err;

	*match = true;

	err = dom_गाठ_get_first_child(n, &n);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_BADPARM;
	}

	यावत् (n != रिक्त) {
		dom_गाठ_type ntype;
		err = dom_गाठ_get_गाठ_type(n, &ntype);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_BADPARM;
		}

		यदि (ntype == DOM_ELEMENT_गाठ ||
		    ntype == DOM_TEXT_गाठ) {
			*match = false;
			dom_गाठ_unref(n);
			भङ्ग;
		}

		err = dom_गाठ_get_next_sibling(n, &next);
		यदि (err != DOM_NO_ERR) {
			dom_गाठ_unref(n);
			प्रत्यागम CSS_BADPARM;
		}
		dom_गाठ_unref(n);
		n = next;
	}

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_link(शून्य *pw, शून्य *n, bool *match)
{
	dom_गाठ *गाठ = n;
	dom_exception exc;
	dom_string *गाठ_name = रिक्त;

	exc = dom_गाठ_get_गाठ_name(गाठ, &गाठ_name);
	यदि ((exc != DOM_NO_ERR) || (गाठ_name == रिक्त)) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (dom_string_caseless_lwc_isequal(गाठ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(गाठ, corestring_dom_href,
				&has_href);
		यदि ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} अन्यथा {
			*match = false;
		}
	} अन्यथा {
		*match = false;
	}
	dom_string_unref(गाठ_name);

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_visited(शून्य *pw, शून्य *गाठ, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ध्रुव संरचना url_data *data;

	dom_exception exc;
	dom_गाठ *n = गाठ;
	dom_string *s = रिक्त;

	*match = false;

	exc = dom_गाठ_get_गाठ_name(n, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		प्रत्यागम CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = रिक्त;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		/* Can't be visited; not got a URL */
		प्रत्यागम CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	यदि (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		प्रत्यागम CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	यदि (data != रिक्त && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_hover(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_active(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_focus(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_enabled(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_disabled(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_checked(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_target(शून्य *pw, शून्य *गाठ, bool *match)
{
	/** \todo Support target */

	*match = false;

	प्रत्यागम CSS_OK;
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
css_error गाठ_is_lang(शून्य *pw, शून्य *गाठ,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	प्रत्यागम CSS_OK;
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
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			भङ्ग;
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
		प्रत्यागम CSS_INVALID;
	}

	प्रत्यागम CSS_OK;
}

css_error set_libcss_गाठ_data(शून्य *pw, शून्य *गाठ, शून्य *libcss_गाठ_data)
{
	dom_गाठ *n = गाठ;
	dom_exception err;
	शून्य *old_गाठ_data;

	/* Set this node's node data */
	err = dom_गाठ_set_user_data(n,
			corestring_dom_ns_किल्ली_libcss_गाठ_data,
			libcss_गाठ_data, nscss_dom_user_data_handler,
			(शून्य *) &old_गाठ_data);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	प्रतिज्ञा(old_गाठ_data == रिक्त);

	प्रत्यागम CSS_OK;
}

css_error get_libcss_गाठ_data(शून्य *pw, शून्य *गाठ, शून्य **libcss_गाठ_data)
{
	dom_गाठ *n = गाठ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_गाठ_get_user_data(n,
			corestring_dom_ns_किल्ली_libcss_गाठ_data,
			libcss_गाठ_data);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	प्रत्यागम CSS_OK;
}
