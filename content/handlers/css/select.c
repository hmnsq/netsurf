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

#समावेश <assert.h>
#समावेश <string.h>
#समावेश <strings.h>

#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/nsurl.h"
#समावेश "netsurf/plot_style.h"
#समावेश "netsurf/url_db.h"
#समावेश "desktop/system_colour.h"

#समावेश "css/internal.h"
#समावेश "css/hints.h"
#समावेश "css/select.h"

अटल css_error गाँठ_name(व्योम *pw, व्योम *गाँठ, css_qname *qname);
अटल css_error गाँठ_classes(व्योम *pw, व्योम *गाँठ,
		lwc_string ***classes, uint32_t *n_classes);
अटल css_error गाँठ_id(व्योम *pw, व्योम *गाँठ, lwc_string **id);
अटल css_error named_parent_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **parent);
अटल css_error named_sibling_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **sibling);
अटल css_error named_generic_sibling_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **sibling);
अटल css_error parent_गाँठ(व्योम *pw, व्योम *गाँठ, व्योम **parent);
अटल css_error sibling_गाँठ(व्योम *pw, व्योम *गाँठ, व्योम **sibling);
अटल css_error गाँठ_has_name(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, bool *match);
अटल css_error गाँठ_has_class(व्योम *pw, व्योम *गाँठ,
		lwc_string *name, bool *match);
अटल css_error गाँठ_has_id(व्योम *pw, व्योम *गाँठ,
		lwc_string *name, bool *match);
अटल css_error गाँठ_has_attribute(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, bool *match);
अटल css_error गाँठ_has_attribute_equal(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_has_attribute_dashmatch(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_has_attribute_includes(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_has_attribute_prefix(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_has_attribute_suffix(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_has_attribute_substring(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match);
अटल css_error गाँठ_is_मूल(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_गिनती_siblings(व्योम *pw, व्योम *गाँठ,
		bool same_name, bool after, int32_t *गिनती);
अटल css_error गाँठ_is_empty(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_link(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_hover(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_active(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_focus(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_enabled(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_disabled(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_checked(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_target(व्योम *pw, व्योम *गाँठ, bool *match);
अटल css_error गाँठ_is_lang(व्योम *pw, व्योम *गाँठ,
		lwc_string *lang, bool *match);
अटल css_error ua_default_for_property(व्योम *pw, uint32_t property,
		css_hint *hint);
अटल css_error set_libcss_गाँठ_तथ्य(व्योम *pw, व्योम *गाँठ,
		व्योम *libcss_गाँठ_तथ्य);
अटल css_error get_libcss_गाँठ_तथ्य(व्योम *pw, व्योम *गाँठ,
		व्योम **libcss_गाँठ_तथ्य);

/**
 * Selection callback table for libcss
 */
अटल css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	गाँठ_name,
	गाँठ_classes,
	गाँठ_id,
	named_ancestor_गाँठ,
	named_parent_गाँठ,
	named_sibling_गाँठ,
	named_generic_sibling_गाँठ,
	parent_गाँठ,
	sibling_गाँठ,
	गाँठ_has_name,
	गाँठ_has_class,
	गाँठ_has_id,
	गाँठ_has_attribute,
	गाँठ_has_attribute_equal,
	गाँठ_has_attribute_dashmatch,
	गाँठ_has_attribute_includes,
	गाँठ_has_attribute_prefix,
	गाँठ_has_attribute_suffix,
	गाँठ_has_attribute_substring,
	गाँठ_is_मूल,
	गाँठ_गिनती_siblings,
	गाँठ_is_empty,
	गाँठ_is_link,
	गाँठ_is_visited,
	गाँठ_is_hover,
	गाँठ_is_active,
	गाँठ_is_focus,
	गाँठ_is_enabled,
	गाँठ_is_disabled,
	गाँठ_is_checked,
	गाँठ_is_target,
	गाँठ_is_lang,
	गाँठ_presentational_hint,
	ua_default_for_property,
	set_libcss_गाँठ_तथ्य,
	get_libcss_गाँठ_तथ्य,
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
css_stylesheet *nscss_create_inline_style(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		स्थिर अक्षर *charset, स्थिर अक्षर *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = शून्य;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = शून्य;
	params.import = शून्य;
	params.import_pw = शून्य;
	params.color = ns_system_colour;
	params.color_pw = शून्य;
	params.font = शून्य;
	params.font_pw = शून्य;

	error = css_stylesheet_create(&params, &sheet);
	अगर (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		वापस शून्य;
	}

	error = css_stylesheet_append_तथ्य(sheet, तथ्य, len);
	अगर (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		वापस शून्य;
	}

	error = css_stylesheet_तथ्य_done(sheet);
	अगर (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		वापस शून्य;
	}

	वापस sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
अटल व्योम nscss_dom_user_तथ्य_handler(dom_गाँठ_operation operation,
		dom_string *कुंजी, व्योम *तथ्य, काष्ठा dom_गाँठ *src,
		काष्ठा dom_गाँठ *dst)
{
	css_error error;

	अगर (dom_string_isequal(corestring_dom_ns_कुंजी_libcss_गाँठ_तथ्य,
			कुंजी) == false || तथ्य == शून्य) {
		वापस;
	}

	चयन (operation) {
	हाल DOM_गाँठ_CLONED:
		error = css_libcss_गाँठ_तथ्य_handler(&selection_handler,
				CSS_गाँठ_CLONED,
				शून्य, src, dst, तथ्य);
		अगर (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		अवरोध;

	हाल DOM_गाँठ_RENAMED:
		error = css_libcss_गाँठ_तथ्य_handler(&selection_handler,
				CSS_गाँठ_MODIFIED,
				शून्य, src, शून्य, तथ्य);
		अगर (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		अवरोध;

	हाल DOM_गाँठ_IMPORTED:
	हाल DOM_गाँठ_ADOPTED:
	हाल DOM_गाँठ_DELETED:
		error = css_libcss_गाँठ_तथ्य_handler(&selection_handler,
				CSS_गाँठ_DELETED,
				शून्य, src, शून्य, तथ्य);
		अगर (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		अवरोध;

	शेष:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		निश्चित(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_गाँठ *n,
		स्थिर css_media *media,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	पूर्णांक pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	अगर (error != CSS_OK || styles == शून्य) {
		/* Failed selecting partial style -- bail out */
		वापस शून्य;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	अगर (ctx->parent_style != शून्य) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		अगर (error != CSS_OK) {
			css_select_results_destroy(styles);
			वापस शून्य;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	क्रम (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_गिनती;
			pseudo_element++) {

		अगर (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_पंक्ति)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			जारी;

		अगर (styles->styles[pseudo_element] == शून्य)
			/* There were no rules concerning this pseudo element */
			जारी;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		अगर (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			वापस शून्य;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	वापस styles;
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
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	अगर (error != CSS_OK) {
		वापस शून्य;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	अगर (error != CSS_OK) {
		css_computed_style_destroy(composed);
		वापस शून्य;
	}

	वापस composed;
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
css_error गाँठ_name(व्योम *pw, व्योम *गाँठ, css_qname *qname)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_exception err;

	err = dom_गाँठ_get_गाँठ_name(n, &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	qname->ns = शून्य;

	err = dom_string_intern(name, &qname->name);
	अगर (err != DOM_NO_ERR) {
		dom_string_unref(name);
		वापस CSS_NOMEM;
	}

	dom_string_unref(name);

	वापस CSS_OK;
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
css_error गाँठ_classes(व्योम *pw, व्योम *गाँठ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_गाँठ *n = गाँठ;
	dom_exception err;

	*classes = शून्य;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	वापस CSS_OK;
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
css_error गाँठ_id(व्योम *pw, व्योम *गाँठ, lwc_string **id)
{
	dom_गाँठ *n = गाँठ;
	dom_string *attr;
	dom_exception err;

	*id = शून्य;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	अगर (attr != शून्य) {
		err = dom_string_intern(attr, id);
		अगर (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			वापस CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	वापस CSS_OK;
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
css_error named_ancestor_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **ancestor)
{
	dom_element_named_ancestor_गाँठ(गाँठ, qname->name,
			(काष्ठा dom_element **)ancestor);
	dom_गाँठ_unref(*ancestor);

	वापस CSS_OK;
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
css_error named_parent_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **parent)
{
	dom_element_named_parent_गाँठ(गाँठ, qname->name,
			(काष्ठा dom_element **)parent);
	dom_गाँठ_unref(*parent);

	वापस CSS_OK;
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
css_error named_sibling_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **sibling)
{
	dom_गाँठ *n = गाँठ;
	dom_गाँठ *पूर्व;
	dom_exception err;

	*sibling = शून्य;

	/* Find sibling element */
	err = dom_गाँठ_get_previous_sibling(n, &n);
	अगर (err != DOM_NO_ERR)
		वापस CSS_OK;

	जबतक (n != शून्य) {
		dom_गाँठ_type type;

		err = dom_गाँठ_get_गाँठ_type(n, &type);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		अगर (type == DOM_ELEMENT_गाँठ)
			अवरोध;

		err = dom_गाँठ_get_previous_sibling(n, &पूर्व);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		dom_गाँठ_unref(n);
		n = पूर्व;
	}

	अगर (n != शून्य) {
		dom_string *name;

		err = dom_गाँठ_get_गाँठ_name(n, &name);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		dom_गाँठ_unref(n);

		अगर (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	वापस CSS_OK;
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
css_error named_generic_sibling_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **sibling)
{
	dom_गाँठ *n = गाँठ;
	dom_गाँठ *पूर्व;
	dom_exception err;

	*sibling = शून्य;

	err = dom_गाँठ_get_previous_sibling(n, &n);
	अगर (err != DOM_NO_ERR)
		वापस CSS_OK;

	जबतक (n != शून्य) {
		dom_गाँठ_type type;
		dom_string *name;

		err = dom_गाँठ_get_गाँठ_type(n, &type);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		अगर (type == DOM_ELEMENT_गाँठ) {
			err = dom_गाँठ_get_गाँठ_name(n, &name);
			अगर (err != DOM_NO_ERR) {
				dom_गाँठ_unref(n);
				वापस CSS_OK;
			}

			अगर (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_गाँठ_unref(n);
				*sibling = n;
				अवरोध;
			}
			dom_string_unref(name);
		}

		err = dom_गाँठ_get_previous_sibling(n, &पूर्व);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		dom_गाँठ_unref(n);
		n = पूर्व;
	}

	वापस CSS_OK;
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
css_error parent_गाँठ(व्योम *pw, व्योम *गाँठ, व्योम **parent)
{
	dom_element_parent_गाँठ(गाँठ, (काष्ठा dom_element **)parent);
	dom_गाँठ_unref(*parent);

	वापस CSS_OK;
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
css_error sibling_गाँठ(व्योम *pw, व्योम *गाँठ, व्योम **sibling)
{
	dom_गाँठ *n = गाँठ;
	dom_गाँठ *पूर्व;
	dom_exception err;

	*sibling = शून्य;

	/* Find sibling element */
	err = dom_गाँठ_get_previous_sibling(n, &n);
	अगर (err != DOM_NO_ERR)
		वापस CSS_OK;

	जबतक (n != शून्य) {
		dom_गाँठ_type type;

		err = dom_गाँठ_get_गाँठ_type(n, &type);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		अगर (type == DOM_ELEMENT_गाँठ)
			अवरोध;

		err = dom_गाँठ_get_previous_sibling(n, &पूर्व);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_OK;
		}

		dom_गाँठ_unref(n);
		n = पूर्व;
	}

	अगर (n != शून्य) {
		/** \todo Sort out reference counting */
		dom_गाँठ_unref(n);

		*sibling = n;
	}

	वापस CSS_OK;
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
css_error गाँठ_has_name(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_गाँठ *n = गाँठ;

	अगर (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_गाँठ_get_गाँठ_name(n, &name);
		अगर (err != DOM_NO_ERR)
			वापस CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	वापस CSS_OK;
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
css_error गाँठ_has_class(व्योम *pw, व्योम *गाँठ,
		lwc_string *name, bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	निश्चित(err == DOM_NO_ERR);

	वापस CSS_OK;
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
css_error गाँठ_has_id(व्योम *pw, व्योम *गाँठ,
		lwc_string *name, bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	अगर (err != DOM_NO_ERR)
		वापस CSS_OK;

	अगर (attr != शून्य) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	वापस CSS_OK;
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
css_error गाँठ_has_attribute(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
			lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	अगर (err != DOM_NO_ERR) {
		dom_string_unref(name);
		वापस CSS_OK;
	}

	dom_string_unref(name);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_equal(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);

	अगर (vlen == 0) {
		*match = false;
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_dashmatch(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);

	अगर (vlen == 0) {
		*match = false;
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for dashmatch */
	अगर (*match == false) {
		स्थिर अक्षर *vdata = lwc_string_तथ्य(मूल्य);
		स्थिर अक्षर *तथ्य = (स्थिर अक्षर *) dom_string_तथ्य(atr_val);
		माप_प्रकार len = dom_string_byte_लंबाई(atr_val);

		अगर (len > vlen && तथ्य[vlen] == '-' &&
		    strncasecmp(तथ्य, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_includes(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);
	स्थिर अक्षर *p;
	स्थिर अक्षर *start;
	स्थिर अक्षर *end;

	*match = false;

	अगर (vlen == 0) {
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (स्थिर अक्षर *) dom_string_तथ्य(atr_val);
	end = start + dom_string_byte_लंबाई(atr_val);

	क्रम (p = start; p <= end; p++) {
		अगर (*p == ' ' || *p == '\0') {
			अगर ((माप_प्रकार) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_तथ्य(मूल्य),
					vlen) == 0) {
				*match = true;
				अवरोध;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_prefix(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);

	अगर (vlen == 0) {
		*match = false;
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	अगर (*match == false) {
		स्थिर अक्षर *तथ्य = (स्थिर अक्षर *) dom_string_तथ्य(atr_val);
		माप_प्रकार len = dom_string_byte_लंबाई(atr_val);

		अगर ((len >= vlen) &&
		    (strncasecmp(तथ्य, lwc_string_तथ्य(मूल्य), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_suffix(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);

	अगर (vlen == 0) {
		*match = false;
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	अगर (*match == false) {
		स्थिर अक्षर *तथ्य = (स्थिर अक्षर *) dom_string_तथ्य(atr_val);
		माप_प्रकार len = dom_string_byte_लंबाई(atr_val);

		स्थिर अक्षर *start = (अक्षर *) तथ्य + len - vlen;

		अगर ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_तथ्य(मूल्य), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_has_attribute_substring(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, lwc_string *मूल्य,
		bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	माप_प्रकार vlen = lwc_string_लंबाई(मूल्य);

	अगर (vlen == 0) {
		*match = false;
		वापस CSS_OK;
	}

	err = dom_string_create_interned(
		(स्थिर uint8_t *) lwc_string_तथ्य(qname->name),
		lwc_string_लंबाई(qname->name), &name);
	अगर (err != DOM_NO_ERR)
		वापस CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	अगर ((err != DOM_NO_ERR) || (atr_val == शून्य)) {
		dom_string_unref(name);
		*match = false;
		वापस CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, मूल्य);

	/* check for prefix match */
	अगर (*match == false) {
		स्थिर अक्षर *vdata = lwc_string_तथ्य(मूल्य);
		स्थिर अक्षर *start = (स्थिर अक्षर *) dom_string_तथ्य(atr_val);
		माप_प्रकार len = dom_string_byte_लंबाई(atr_val);
		स्थिर अक्षर *last_start = start + len - vlen;

		अगर (len >= vlen) {
			जबतक (start <= last_start) {
				अगर (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					अवरोध;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	वापस CSS_OK;
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
css_error गाँठ_is_मूल(व्योम *pw, व्योम *गाँठ, bool *match)
{
	dom_गाँठ *n = गाँठ;
	dom_गाँठ *parent;
	dom_गाँठ_type type;
	dom_exception err;

	err = dom_गाँठ_get_parent_गाँठ(n, &parent);
	अगर (err != DOM_NO_ERR) {
		वापस CSS_NOMEM;
	}

	अगर (parent != शून्य) {
		err = dom_गाँठ_get_गाँठ_type(parent, &type);

		dom_गाँठ_unref(parent);

		अगर (err != DOM_NO_ERR)
			वापस CSS_NOMEM;

		अगर (type != DOM_DOCUMENT_गाँठ) {
			*match = false;
			वापस CSS_OK;
		}
	}

	*match = true;

	वापस CSS_OK;
}

अटल पूर्णांक
गाँठ_गिनती_siblings_check(dom_गाँठ *गाँठ,
			  bool check_name,
			  dom_string *name)
{
	dom_गाँठ_type type;
	पूर्णांक ret = 0;
	dom_exception exc;

	अगर (गाँठ == शून्य)
		वापस 0;

	exc = dom_गाँठ_get_गाँठ_type(गाँठ, &type);
	अगर ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_गाँठ)) {
		वापस 0;
	}

	अगर (check_name) {
		dom_string *गाँठ_name = शून्य;
		exc = dom_गाँठ_get_गाँठ_name(गाँठ, &गाँठ_name);

		अगर ((exc == DOM_NO_ERR) && (गाँठ_name != शून्य)) {

			अगर (dom_string_caseless_isequal(name,
							गाँठ_name)) {
				ret = 1;
			}
			dom_string_unref(गाँठ_name);
		}
	} अन्यथा {
		ret = 1;
	}

	वापस ret;
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
css_error गाँठ_गिनती_siblings(व्योम *pw, व्योम *n, bool same_name,
		bool after, int32_t *गिनती)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *गाँठ_name = शून्य;

	अगर (same_name) {
		dom_गाँठ *गाँठ = n;
		exc = dom_गाँठ_get_गाँठ_name(गाँठ, &गाँठ_name);
		अगर ((exc != DOM_NO_ERR) || (गाँठ_name == शून्य)) {
			वापस CSS_NOMEM;
		}
	}

	अगर (after) {
		dom_गाँठ *गाँठ = dom_गाँठ_ref(n);
		dom_गाँठ *अग्र;

		करो {
			exc = dom_गाँठ_get_अग्र_sibling(गाँठ, &अग्र);
			अगर ((exc != DOM_NO_ERR))
				अवरोध;

			dom_गाँठ_unref(गाँठ);
			गाँठ = अग्र;

			cnt += गाँठ_गिनती_siblings_check(गाँठ, same_name, गाँठ_name);
		} जबतक (गाँठ != शून्य);
	} अन्यथा {
		dom_गाँठ *गाँठ = dom_गाँठ_ref(n);
		dom_गाँठ *अग्र;

		करो {
			exc = dom_गाँठ_get_previous_sibling(गाँठ, &अग्र);
			अगर ((exc != DOM_NO_ERR))
				अवरोध;

			dom_गाँठ_unref(गाँठ);
			गाँठ = अग्र;

			cnt += गाँठ_गिनती_siblings_check(गाँठ, same_name, गाँठ_name);

		} जबतक (गाँठ != शून्य);
	}

	अगर (गाँठ_name != शून्य) {
		dom_string_unref(गाँठ_name);
	}

	*गिनती = cnt;
	वापस CSS_OK;
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
css_error गाँठ_is_empty(व्योम *pw, व्योम *गाँठ, bool *match)
{
	dom_गाँठ *n = गाँठ, *अग्र;
	dom_exception err;

	*match = true;

	err = dom_गाँठ_get_first_child(n, &n);
	अगर (err != DOM_NO_ERR) {
		वापस CSS_BADPARM;
	}

	जबतक (n != शून्य) {
		dom_गाँठ_type ntype;
		err = dom_गाँठ_get_गाँठ_type(n, &ntype);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_BADPARM;
		}

		अगर (ntype == DOM_ELEMENT_गाँठ ||
		    ntype == DOM_TEXT_गाँठ) {
			*match = false;
			dom_गाँठ_unref(n);
			अवरोध;
		}

		err = dom_गाँठ_get_अग्र_sibling(n, &अग्र);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(n);
			वापस CSS_BADPARM;
		}
		dom_गाँठ_unref(n);
		n = अग्र;
	}

	वापस CSS_OK;
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
css_error गाँठ_is_link(व्योम *pw, व्योम *n, bool *match)
{
	dom_गाँठ *गाँठ = n;
	dom_exception exc;
	dom_string *गाँठ_name = शून्य;

	exc = dom_गाँठ_get_गाँठ_name(गाँठ, &गाँठ_name);
	अगर ((exc != DOM_NO_ERR) || (गाँठ_name == शून्य)) {
		वापस CSS_NOMEM;
	}

	अगर (dom_string_caseless_lwc_isequal(गाँठ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(गाँठ, corestring_dom_href,
				&has_href);
		अगर ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} अन्यथा {
			*match = false;
		}
	} अन्यथा {
		*match = false;
	}
	dom_string_unref(गाँठ_name);

	वापस CSS_OK;
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
css_error गाँठ_is_visited(व्योम *pw, व्योम *गाँठ, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	स्थिर काष्ठा url_तथ्य *तथ्य;

	dom_exception exc;
	dom_गाँठ *n = गाँठ;
	dom_string *s = शून्य;

	*match = false;

	exc = dom_गाँठ_get_गाँठ_name(n, &s);
	अगर ((exc != DOM_NO_ERR) || (s == शून्य)) {
		वापस CSS_NOMEM;
	}

	अगर (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		वापस CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = शून्य;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	अगर ((exc != DOM_NO_ERR) || (s == शून्य)) {
		/* Can't be visited; not got a URL */
		वापस CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_तथ्य(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	अगर (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		वापस CSS_NOMEM;
	}

	तथ्य = urldb_get_url_तथ्य(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	अगर (तथ्य != शून्य && तथ्य->visits > 0)
		*match = true;

	nsurl_unref(url);

	वापस CSS_OK;
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
css_error गाँठ_is_hover(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_active(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_focus(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_enabled(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_disabled(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_checked(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_target(व्योम *pw, व्योम *गाँठ, bool *match)
{
	/** \todo Support target */

	*match = false;

	वापस CSS_OK;
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
css_error गाँठ_is_lang(व्योम *pw, व्योम *गाँठ,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	वापस CSS_OK;
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
css_error ua_default_for_property(व्योम *pw, uint32_t property, css_hint *hint)
{
	अगर (property == CSS_PROP_COLOR) {
		hint->तथ्य.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} अन्यथा अगर (property == CSS_PROP_FONT_FAMILY) {
		hint->तथ्य.strings = शून्य;
		चयन (nsoption_int(font_default)) {
		हाल PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			अवरोध;
		हाल PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			अवरोध;
		हाल PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			अवरोध;
		हाल PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			अवरोध;
		हाल PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			अवरोध;
		}
	} अन्यथा अगर (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->तथ्य.strings = शून्य;
		hint->status = CSS_QUOTES_NONE;
	} अन्यथा अगर (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->तथ्य.strings = शून्य;
		hint->status = 0;
	} अन्यथा {
		वापस CSS_INVALID;
	}

	वापस CSS_OK;
}

css_error set_libcss_गाँठ_तथ्य(व्योम *pw, व्योम *गाँठ, व्योम *libcss_गाँठ_तथ्य)
{
	dom_गाँठ *n = गाँठ;
	dom_exception err;
	व्योम *old_गाँठ_तथ्य;

	/* Set this node's node data */
	err = dom_गाँठ_set_user_तथ्य(n,
			corestring_dom_ns_कुंजी_libcss_गाँठ_तथ्य,
			libcss_गाँठ_तथ्य, nscss_dom_user_तथ्य_handler,
			(व्योम *) &old_गाँठ_तथ्य);
	अगर (err != DOM_NO_ERR) {
		वापस CSS_NOMEM;
	}

	निश्चित(old_गाँठ_तथ्य == शून्य);

	वापस CSS_OK;
}

css_error get_libcss_गाँठ_तथ्य(व्योम *pw, व्योम *गाँठ, व्योम **libcss_गाँठ_तथ्य)
{
	dom_गाँठ *n = गाँठ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_गाँठ_get_user_तथ्य(n,
			corestring_dom_ns_कुंजी_libcss_गाँठ_तथ्य,
			libcss_गाँठ_तथ्य);
	अगर (err != DOM_NO_ERR) {
		वापस CSS_NOMEM;
	}

	वापस CSS_OK;
}
