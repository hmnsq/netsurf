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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>

#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "netsurf/plot_style.h"
#അന്തര്ഭാവ "netsurf/url_db.h"
#അന്തര്ഭാവ "desktop/system_colour.h"

#അന്തര്ഭാവ "css/internal.h"
#അന്തര്ഭാവ "css/hints.h"
#അന്തര്ഭാവ "css/select.h"

സ്ഥിതം css_error കണ്ണി_name(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, css_qname *qname);
സ്ഥിതം css_error കണ്ണി_classes(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string ***classes, uint32_t *n_classes);
സ്ഥിതം css_error കണ്ണി_id(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, lwc_string **id);
സ്ഥിതം css_error named_parent_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **parent);
സ്ഥിതം css_error named_sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **sibling);
സ്ഥിതം css_error named_generic_sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **sibling);
സ്ഥിതം css_error parent_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം **parent);
സ്ഥിതം css_error sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം **sibling);
സ്ഥിതം css_error കണ്ണി_has_name(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, bool *match);
സ്ഥിതം css_error കണ്ണി_has_class(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *name, bool *match);
സ്ഥിതം css_error കണ്ണി_has_id(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *name, bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_equal(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_dashmatch(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_includes(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_prefix(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_suffix(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_has_attribute_substring(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match);
സ്ഥിതം css_error കണ്ണി_is_വേര്(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_എണ്ണം_siblings(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		bool same_name, bool after, int32_t *എണ്ണം);
സ്ഥിതം css_error കണ്ണി_is_empty(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_link(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_hover(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_active(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_focus(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_enabled(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_disabled(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_checked(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_target(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);
സ്ഥിതം css_error കണ്ണി_is_lang(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *lang, bool *match);
സ്ഥിതം css_error ua_default_for_property(ശൂന്യം *pw, uint32_t property,
		css_hint *hint);
സ്ഥിതം css_error set_libcss_കണ്ണി_data(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		ശൂന്യം *libcss_കണ്ണി_data);
സ്ഥിതം css_error get_libcss_കണ്ണി_data(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		ശൂന്യം **libcss_കണ്ണി_data);

/**
 * Selection callback table for libcss
 */
സ്ഥിതം css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	കണ്ണി_name,
	കണ്ണി_classes,
	കണ്ണി_id,
	named_ancestor_കണ്ണി,
	named_parent_കണ്ണി,
	named_sibling_കണ്ണി,
	named_generic_sibling_കണ്ണി,
	parent_കണ്ണി,
	sibling_കണ്ണി,
	കണ്ണി_has_name,
	കണ്ണി_has_class,
	കണ്ണി_has_id,
	കണ്ണി_has_attribute,
	കണ്ണി_has_attribute_equal,
	കണ്ണി_has_attribute_dashmatch,
	കണ്ണി_has_attribute_includes,
	കണ്ണി_has_attribute_prefix,
	കണ്ണി_has_attribute_suffix,
	കണ്ണി_has_attribute_substring,
	കണ്ണി_is_വേര്,
	കണ്ണി_എണ്ണം_siblings,
	കണ്ണി_is_empty,
	കണ്ണി_is_link,
	കണ്ണി_is_visited,
	കണ്ണി_is_hover,
	കണ്ണി_is_active,
	കണ്ണി_is_focus,
	കണ്ണി_is_enabled,
	കണ്ണി_is_disabled,
	കണ്ണി_is_checked,
	കണ്ണി_is_target,
	കണ്ണി_is_lang,
	കണ്ണി_presentational_hint,
	ua_default_for_property,
	set_libcss_കണ്ണി_data,
	get_libcss_കണ്ണി_data,
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
css_stylesheet *nscss_create_inline_style(സ്ഥിരം uint8_t *data, size_t len,
		സ്ഥിരം അക്ഷരം *charset, സ്ഥിരം അക്ഷരം *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ശൂന്യ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ശൂന്യ;
	params.import = ശൂന്യ;
	params.import_pw = ശൂന്യ;
	params.color = ns_system_colour;
	params.color_pw = ശൂന്യ;
	params.font = ശൂന്യ;
	params.font_pw = ശൂന്യ;

	error = css_stylesheet_create(&params, &sheet);
	എങ്കിൽ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		തിരികെ ശൂന്യ;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	എങ്കിൽ (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		തിരികെ ശൂന്യ;
	}

	error = css_stylesheet_data_done(sheet);
	എങ്കിൽ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		തിരികെ ശൂന്യ;
	}

	തിരികെ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
സ്ഥിതം ശൂന്യം nscss_dom_user_data_handler(dom_കണ്ണി_operation operation,
		dom_string *താക്കോൽ, ശൂന്യം *data, ഘടന dom_കണ്ണി *src,
		ഘടന dom_കണ്ണി *dst)
{
	css_error error;

	എങ്കിൽ (dom_string_isequal(corestring_dom_ns_താക്കോൽ_libcss_കണ്ണി_data,
			താക്കോൽ) == false || data == ശൂന്യ) {
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ് (operation) {
	അവസ്ഥ DOM_കണ്ണി_CLONED:
		error = css_libcss_കണ്ണി_data_handler(&selection_handler,
				CSS_കണ്ണി_CLONED,
				ശൂന്യ, src, dst, data);
		എങ്കിൽ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_RENAMED:
		error = css_libcss_കണ്ണി_data_handler(&selection_handler,
				CSS_കണ്ണി_MODIFIED,
				ശൂന്യ, src, ശൂന്യ, data);
		എങ്കിൽ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_IMPORTED:
	അവസ്ഥ DOM_കണ്ണി_ADOPTED:
	അവസ്ഥ DOM_കണ്ണി_DELETED:
		error = css_libcss_കണ്ണി_data_handler(&selection_handler,
				CSS_കണ്ണി_DELETED,
				ശൂന്യ, src, ശൂന്യ, data);
		എങ്കിൽ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		പ്രതിജ്ഞാ(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_കണ്ണി *n,
		സ്ഥിരം css_media *media,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	പൂർണ്ണസംഖ്യ pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	എങ്കിൽ (error != CSS_OK || styles == ശൂന്യ) {
		/* Failed selecting partial style -- bail out */
		തിരികെ ശൂന്യ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	എങ്കിൽ (ctx->parent_style != ശൂന്യ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		എങ്കിൽ (error != CSS_OK) {
			css_select_results_destroy(styles);
			തിരികെ ശൂന്യ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	വേണ്ടി (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_എണ്ണം;
			pseudo_element++) {

		എങ്കിൽ (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_വരി)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			തുടരുക;

		എങ്കിൽ (styles->styles[pseudo_element] == ശൂന്യ)
			/* There were no rules concerning this pseudo element */
			തുടരുക;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		എങ്കിൽ (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			തിരികെ ശൂന്യ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	തിരികെ styles;
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
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	എങ്കിൽ (error != CSS_OK) {
		തിരികെ ശൂന്യ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	എങ്കിൽ (error != CSS_OK) {
		css_computed_style_destroy(composed);
		തിരികെ ശൂന്യ;
	}

	തിരികെ composed;
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
css_error കണ്ണി_name(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, css_qname *qname)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_exception err;

	err = dom_കണ്ണി_get_കണ്ണി_name(n, &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	qname->ns = ശൂന്യ;

	err = dom_string_intern(name, &qname->name);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		തിരികെ CSS_NOMEM;
	}

	dom_string_unref(name);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_classes(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_exception err;

	*classes = ശൂന്യ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_id(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, lwc_string **id)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *attr;
	dom_exception err;

	*id = ശൂന്യ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	എങ്കിൽ (attr != ശൂന്യ) {
		err = dom_string_intern(attr, id);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			തിരികെ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	തിരികെ CSS_OK;
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
css_error named_ancestor_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **ancestor)
{
	dom_element_named_ancestor_കണ്ണി(കണ്ണി, qname->name,
			(ഘടന dom_element **)ancestor);
	dom_കണ്ണി_unref(*ancestor);

	തിരികെ CSS_OK;
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
css_error named_parent_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **parent)
{
	dom_element_named_parent_കണ്ണി(കണ്ണി, qname->name,
			(ഘടന dom_element **)parent);
	dom_കണ്ണി_unref(*parent);

	തിരികെ CSS_OK;
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
css_error named_sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **sibling)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_കണ്ണി *prev;
	dom_exception err;

	*sibling = ശൂന്യ;

	/* Find sibling element */
	err = dom_കണ്ണി_get_previous_sibling(n, &n);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_OK;

	ആകുമ്പോൾ (n != ശൂന്യ) {
		dom_കണ്ണി_type type;

		err = dom_കണ്ണി_get_കണ്ണി_type(n, &type);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		എങ്കിൽ (type == DOM_ELEMENT_കണ്ണി)
			നിർത്തുക;

		err = dom_കണ്ണി_get_previous_sibling(n, &prev);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		dom_കണ്ണി_unref(n);
		n = prev;
	}

	എങ്കിൽ (n != ശൂന്യ) {
		dom_string *name;

		err = dom_കണ്ണി_get_കണ്ണി_name(n, &name);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		dom_കണ്ണി_unref(n);

		എങ്കിൽ (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	തിരികെ CSS_OK;
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
css_error named_generic_sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **sibling)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_കണ്ണി *prev;
	dom_exception err;

	*sibling = ശൂന്യ;

	err = dom_കണ്ണി_get_previous_sibling(n, &n);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_OK;

	ആകുമ്പോൾ (n != ശൂന്യ) {
		dom_കണ്ണി_type type;
		dom_string *name;

		err = dom_കണ്ണി_get_കണ്ണി_type(n, &type);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		എങ്കിൽ (type == DOM_ELEMENT_കണ്ണി) {
			err = dom_കണ്ണി_get_കണ്ണി_name(n, &name);
			എങ്കിൽ (err != DOM_NO_ERR) {
				dom_കണ്ണി_unref(n);
				തിരികെ CSS_OK;
			}

			എങ്കിൽ (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_കണ്ണി_unref(n);
				*sibling = n;
				നിർത്തുക;
			}
			dom_string_unref(name);
		}

		err = dom_കണ്ണി_get_previous_sibling(n, &prev);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		dom_കണ്ണി_unref(n);
		n = prev;
	}

	തിരികെ CSS_OK;
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
css_error parent_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം **parent)
{
	dom_element_parent_കണ്ണി(കണ്ണി, (ഘടന dom_element **)parent);
	dom_കണ്ണി_unref(*parent);

	തിരികെ CSS_OK;
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
css_error sibling_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം **sibling)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_കണ്ണി *prev;
	dom_exception err;

	*sibling = ശൂന്യ;

	/* Find sibling element */
	err = dom_കണ്ണി_get_previous_sibling(n, &n);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_OK;

	ആകുമ്പോൾ (n != ശൂന്യ) {
		dom_കണ്ണി_type type;

		err = dom_കണ്ണി_get_കണ്ണി_type(n, &type);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		എങ്കിൽ (type == DOM_ELEMENT_കണ്ണി)
			നിർത്തുക;

		err = dom_കണ്ണി_get_previous_sibling(n, &prev);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_OK;
		}

		dom_കണ്ണി_unref(n);
		n = prev;
	}

	എങ്കിൽ (n != ശൂന്യ) {
		/** \todo Sort out reference counting */
		dom_കണ്ണി_unref(n);

		*sibling = n;
	}

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_name(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_കണ്ണി *n = കണ്ണി;

	എങ്കിൽ (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_കണ്ണി_get_കണ്ണി_name(n, &name);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_class(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *name, bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	പ്രതിജ്ഞാ(err == DOM_NO_ERR);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_id(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *name, bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_OK;

	എങ്കിൽ (attr != ശൂന്യ) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_equal(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(മൂല്യം);

	എങ്കിൽ (vlen == 0) {
		*match = false;
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, മൂല്യം);

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_dashmatch(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(മൂല്യം);

	എങ്കിൽ (vlen == 0) {
		*match = false;
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, മൂല്യം);

	/* check for dashmatch */
	എങ്കിൽ (*match == false) {
		സ്ഥിരം അക്ഷരം *vdata = lwc_string_data(മൂല്യം);
		സ്ഥിരം അക്ഷരം *data = (സ്ഥിരം അക്ഷരം *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		എങ്കിൽ (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_includes(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(മൂല്യം);
	സ്ഥിരം അക്ഷരം *p;
	സ്ഥിരം അക്ഷരം *start;
	സ്ഥിരം അക്ഷരം *end;

	*match = false;

	എങ്കിൽ (vlen == 0) {
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (സ്ഥിരം അക്ഷരം *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	വേണ്ടി (p = start; p <= end; p++) {
		എങ്കിൽ (*p == ' ' || *p == '\0') {
			എങ്കിൽ ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(മൂല്യം),
					vlen) == 0) {
				*match = true;
				നിർത്തുക;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_prefix(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(മൂല്യം);

	എങ്കിൽ (vlen == 0) {
		*match = false;
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, മൂല്യം);

	/* check for prefix match */
	എങ്കിൽ (*match == false) {
		സ്ഥിരം അക്ഷരം *data = (സ്ഥിരം അക്ഷരം *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		എങ്കിൽ ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(മൂല്യം), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_suffix(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(മൂല്യം);

	എങ്കിൽ (vlen == 0) {
		*match = false;
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, മൂല്യം);

	/* check for prefix match */
	എങ്കിൽ (*match == false) {
		സ്ഥിരം അക്ഷരം *data = (സ്ഥിരം അക്ഷരം *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		സ്ഥിരം അക്ഷരം *start = (അക്ഷരം *) data + len - vlen;

		എങ്കിൽ ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(മൂല്യം), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_has_attribute_substring(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, lwc_string *മൂല്യം,
		bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(മൂല്യം);

	എങ്കിൽ (vlen == 0) {
		*match = false;
		തിരികെ CSS_OK;
	}

	err = dom_string_create_interned(
		(സ്ഥിരം uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	എങ്കിൽ ((err != DOM_NO_ERR) || (atr_val == ശൂന്യ)) {
		dom_string_unref(name);
		*match = false;
		തിരികെ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, മൂല്യം);

	/* check for prefix match */
	എങ്കിൽ (*match == false) {
		സ്ഥിരം അക്ഷരം *vdata = lwc_string_data(മൂല്യം);
		സ്ഥിരം അക്ഷരം *start = (സ്ഥിരം അക്ഷരം *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		സ്ഥിരം അക്ഷരം *last_start = start + len - vlen;

		എങ്കിൽ (len >= vlen) {
			ആകുമ്പോൾ (start <= last_start) {
				എങ്കിൽ (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					നിർത്തുക;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_വേര്(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_കണ്ണി *parent;
	dom_കണ്ണി_type type;
	dom_exception err;

	err = dom_കണ്ണി_get_parent_കണ്ണി(n, &parent);
	എങ്കിൽ (err != DOM_NO_ERR) {
		തിരികെ CSS_NOMEM;
	}

	എങ്കിൽ (parent != ശൂന്യ) {
		err = dom_കണ്ണി_get_കണ്ണി_type(parent, &type);

		dom_കണ്ണി_unref(parent);

		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ CSS_NOMEM;

		എങ്കിൽ (type != DOM_DOCUMENT_കണ്ണി) {
			*match = false;
			തിരികെ CSS_OK;
		}
	}

	*match = true;

	തിരികെ CSS_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
കണ്ണി_എണ്ണം_siblings_check(dom_കണ്ണി *കണ്ണി,
			  bool check_name,
			  dom_string *name)
{
	dom_കണ്ണി_type type;
	പൂർണ്ണസംഖ്യ ret = 0;
	dom_exception exc;

	എങ്കിൽ (കണ്ണി == ശൂന്യ)
		തിരികെ 0;

	exc = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &type);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_കണ്ണി)) {
		തിരികെ 0;
	}

	എങ്കിൽ (check_name) {
		dom_string *കണ്ണി_name = ശൂന്യ;
		exc = dom_കണ്ണി_get_കണ്ണി_name(കണ്ണി, &കണ്ണി_name);

		എങ്കിൽ ((exc == DOM_NO_ERR) && (കണ്ണി_name != ശൂന്യ)) {

			എങ്കിൽ (dom_string_caseless_isequal(name,
							കണ്ണി_name)) {
				ret = 1;
			}
			dom_string_unref(കണ്ണി_name);
		}
	} അല്ലെങ്കിൽ {
		ret = 1;
	}

	തിരികെ ret;
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
css_error കണ്ണി_എണ്ണം_siblings(ശൂന്യം *pw, ശൂന്യം *n, bool same_name,
		bool after, int32_t *എണ്ണം)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *കണ്ണി_name = ശൂന്യ;

	എങ്കിൽ (same_name) {
		dom_കണ്ണി *കണ്ണി = n;
		exc = dom_കണ്ണി_get_കണ്ണി_name(കണ്ണി, &കണ്ണി_name);
		എങ്കിൽ ((exc != DOM_NO_ERR) || (കണ്ണി_name == ശൂന്യ)) {
			തിരികെ CSS_NOMEM;
		}
	}

	എങ്കിൽ (after) {
		dom_കണ്ണി *കണ്ണി = dom_കണ്ണി_ref(n);
		dom_കണ്ണി *next;

		ചെയ്യുക {
			exc = dom_കണ്ണി_get_next_sibling(കണ്ണി, &next);
			എങ്കിൽ ((exc != DOM_NO_ERR))
				നിർത്തുക;

			dom_കണ്ണി_unref(കണ്ണി);
			കണ്ണി = next;

			cnt += കണ്ണി_എണ്ണം_siblings_check(കണ്ണി, same_name, കണ്ണി_name);
		} ആകുമ്പോൾ (കണ്ണി != ശൂന്യ);
	} അല്ലെങ്കിൽ {
		dom_കണ്ണി *കണ്ണി = dom_കണ്ണി_ref(n);
		dom_കണ്ണി *next;

		ചെയ്യുക {
			exc = dom_കണ്ണി_get_previous_sibling(കണ്ണി, &next);
			എങ്കിൽ ((exc != DOM_NO_ERR))
				നിർത്തുക;

			dom_കണ്ണി_unref(കണ്ണി);
			കണ്ണി = next;

			cnt += കണ്ണി_എണ്ണം_siblings_check(കണ്ണി, same_name, കണ്ണി_name);

		} ആകുമ്പോൾ (കണ്ണി != ശൂന്യ);
	}

	എങ്കിൽ (കണ്ണി_name != ശൂന്യ) {
		dom_string_unref(കണ്ണി_name);
	}

	*എണ്ണം = cnt;
	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_empty(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	dom_കണ്ണി *n = കണ്ണി, *next;
	dom_exception err;

	*match = true;

	err = dom_കണ്ണി_get_first_child(n, &n);
	എങ്കിൽ (err != DOM_NO_ERR) {
		തിരികെ CSS_BADPARM;
	}

	ആകുമ്പോൾ (n != ശൂന്യ) {
		dom_കണ്ണി_type ntype;
		err = dom_കണ്ണി_get_കണ്ണി_type(n, &ntype);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_BADPARM;
		}

		എങ്കിൽ (ntype == DOM_ELEMENT_കണ്ണി ||
		    ntype == DOM_TEXT_കണ്ണി) {
			*match = false;
			dom_കണ്ണി_unref(n);
			നിർത്തുക;
		}

		err = dom_കണ്ണി_get_next_sibling(n, &next);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(n);
			തിരികെ CSS_BADPARM;
		}
		dom_കണ്ണി_unref(n);
		n = next;
	}

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_link(ശൂന്യം *pw, ശൂന്യം *n, bool *match)
{
	dom_കണ്ണി *കണ്ണി = n;
	dom_exception exc;
	dom_string *കണ്ണി_name = ശൂന്യ;

	exc = dom_കണ്ണി_get_കണ്ണി_name(കണ്ണി, &കണ്ണി_name);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (കണ്ണി_name == ശൂന്യ)) {
		തിരികെ CSS_NOMEM;
	}

	എങ്കിൽ (dom_string_caseless_lwc_isequal(കണ്ണി_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(കണ്ണി, corestring_dom_href,
				&has_href);
		എങ്കിൽ ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} അല്ലെങ്കിൽ {
			*match = false;
		}
	} അല്ലെങ്കിൽ {
		*match = false;
	}
	dom_string_unref(കണ്ണി_name);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_visited(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	സ്ഥിരം ഘടന url_data *data;

	dom_exception exc;
	dom_കണ്ണി *n = കണ്ണി;
	dom_string *s = ശൂന്യ;

	*match = false;

	exc = dom_കണ്ണി_get_കണ്ണി_name(n, &s);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (s == ശൂന്യ)) {
		തിരികെ CSS_NOMEM;
	}

	എങ്കിൽ (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		തിരികെ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ശൂന്യ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (s == ശൂന്യ)) {
		/* Can't be visited; not got a URL */
		തിരികെ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	എങ്കിൽ (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		തിരികെ CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	എങ്കിൽ (data != ശൂന്യ && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_hover(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_active(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_focus(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_enabled(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_disabled(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_checked(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_target(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match)
{
	/** \todo Support target */

	*match = false;

	തിരികെ CSS_OK;
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
css_error കണ്ണി_is_lang(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	തിരികെ CSS_OK;
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
css_error ua_default_for_property(ശൂന്യം *pw, uint32_t property, css_hint *hint)
{
	എങ്കിൽ (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} അല്ലെങ്കിൽ എങ്കിൽ (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = ശൂന്യ;
		തിരഞ്ഞെടുപ്പ് (nsoption_int(font_default)) {
		അവസ്ഥ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			നിർത്തുക;
		അവസ്ഥ PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			നിർത്തുക;
		അവസ്ഥ PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			നിർത്തുക;
		അവസ്ഥ PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			നിർത്തുക;
		അവസ്ഥ PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			നിർത്തുക;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = ശൂന്യ;
		hint->status = CSS_QUOTES_NONE;
	} അല്ലെങ്കിൽ എങ്കിൽ (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = ശൂന്യ;
		hint->status = 0;
	} അല്ലെങ്കിൽ {
		തിരികെ CSS_INVALID;
	}

	തിരികെ CSS_OK;
}

css_error set_libcss_കണ്ണി_data(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം *libcss_കണ്ണി_data)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_exception err;
	ശൂന്യം *old_കണ്ണി_data;

	/* Set this node's node data */
	err = dom_കണ്ണി_set_user_data(n,
			corestring_dom_ns_താക്കോൽ_libcss_കണ്ണി_data,
			libcss_കണ്ണി_data, nscss_dom_user_data_handler,
			(ശൂന്യം *) &old_കണ്ണി_data);
	എങ്കിൽ (err != DOM_NO_ERR) {
		തിരികെ CSS_NOMEM;
	}

	പ്രതിജ്ഞാ(old_കണ്ണി_data == ശൂന്യ);

	തിരികെ CSS_OK;
}

css_error get_libcss_കണ്ണി_data(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, ശൂന്യം **libcss_കണ്ണി_data)
{
	dom_കണ്ണി *n = കണ്ണി;
	dom_exception err;

	/* Get this node's node data */
	err = dom_കണ്ണി_get_user_data(n,
			corestring_dom_ns_താക്കോൽ_libcss_കണ്ണി_data,
			libcss_കണ്ണി_data);
	എങ്കിൽ (err != DOM_NO_ERR) {
		തിരികെ CSS_NOMEM;
	}

	തിരികെ CSS_OK;
}
