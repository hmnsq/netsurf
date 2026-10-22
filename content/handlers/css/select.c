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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>

#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "netsurf/plot_style.h"
#அந்தர்பாவ "netsurf/url_db.h"
#அந்தர்பாவ "desktop/system_colour.h"

#அந்தர்பாவ "css/internal.h"
#அந்தர்பாவ "css/hints.h"
#அந்தர்பாவ "css/select.h"

நிலையான css_error முனை_name(வெறுமை *pw, வெறுமை *முனை, css_qname *qname);
நிலையான css_error முனை_classes(வெறுமை *pw, வெறுமை *முனை,
		lwc_string ***classes, uint32_t *n_classes);
நிலையான css_error முனை_id(வெறுமை *pw, வெறுமை *முனை, lwc_string **id);
நிலையான css_error named_parent_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **parent);
நிலையான css_error named_sibling_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **sibling);
நிலையான css_error named_generic_sibling_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **sibling);
நிலையான css_error parent_முனை(வெறுமை *pw, வெறுமை *முனை, வெறுமை **parent);
நிலையான css_error sibling_முனை(வெறுமை *pw, வெறுமை *முனை, வெறுமை **sibling);
நிலையான css_error முனை_has_name(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, bool *match);
நிலையான css_error முனை_has_class(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *name, bool *match);
நிலையான css_error முனை_has_id(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *name, bool *match);
நிலையான css_error முனை_has_attribute(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, bool *match);
நிலையான css_error முனை_has_attribute_equal(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_has_attribute_dashmatch(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_has_attribute_includes(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_has_attribute_prefix(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_has_attribute_suffix(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_has_attribute_substring(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match);
நிலையான css_error முனை_is_வேர்(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_மொத்தம்_siblings(வெறுமை *pw, வெறுமை *முனை,
		bool same_name, bool after, int32_t *மொத்தம்);
நிலையான css_error முனை_is_empty(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_link(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_hover(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_active(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_focus(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_enabled(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_disabled(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_checked(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_target(வெறுமை *pw, வெறுமை *முனை, bool *match);
நிலையான css_error முனை_is_lang(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *lang, bool *match);
நிலையான css_error ua_default_for_property(வெறுமை *pw, uint32_t property,
		css_hint *hint);
நிலையான css_error set_libcss_முனை_data(வெறுமை *pw, வெறுமை *முனை,
		வெறுமை *libcss_முனை_data);
நிலையான css_error get_libcss_முனை_data(வெறுமை *pw, வெறுமை *முனை,
		வெறுமை **libcss_முனை_data);

/**
 * Selection callback table for libcss
 */
நிலையான css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	முனை_name,
	முனை_classes,
	முனை_id,
	named_ancestor_முனை,
	named_parent_முனை,
	named_sibling_முனை,
	named_generic_sibling_முனை,
	parent_முனை,
	sibling_முனை,
	முனை_has_name,
	முனை_has_class,
	முனை_has_id,
	முனை_has_attribute,
	முனை_has_attribute_equal,
	முனை_has_attribute_dashmatch,
	முனை_has_attribute_includes,
	முனை_has_attribute_prefix,
	முனை_has_attribute_suffix,
	முனை_has_attribute_substring,
	முனை_is_வேர்,
	முனை_மொத்தம்_siblings,
	முனை_is_empty,
	முனை_is_link,
	முனை_is_visited,
	முனை_is_hover,
	முனை_is_active,
	முனை_is_focus,
	முனை_is_enabled,
	முனை_is_disabled,
	முனை_is_checked,
	முனை_is_target,
	முனை_is_lang,
	முனை_presentational_hint,
	ua_default_for_property,
	set_libcss_முனை_data,
	get_libcss_முனை_data,
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
css_stylesheet *nscss_create_inline_style(மாறிலி uint8_t *data, size_t len,
		மாறிலி எழுத்து *charset, மாறிலி எழுத்து *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = வெற்று;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = வெற்று;
	params.import = வெற்று;
	params.import_pw = வெற்று;
	params.color = ns_system_colour;
	params.color_pw = வெற்று;
	params.font = வெற்று;
	params.font_pw = வெற்று;

	error = css_stylesheet_create(&params, &sheet);
	எனில் (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		திரும்பு வெற்று;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	எனில் (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		திரும்பு வெற்று;
	}

	error = css_stylesheet_data_done(sheet);
	எனில் (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		திரும்பு வெற்று;
	}

	திரும்பு sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
நிலையான வெறுமை nscss_dom_user_data_handler(dom_முனை_operation operation,
		dom_string *சாவி, வெறுமை *data, கட்டமைப்பு dom_முனை *src,
		கட்டமைப்பு dom_முனை *dst)
{
	css_error error;

	எனில் (dom_string_isequal(corestring_dom_ns_சாவி_libcss_முனை_data,
			சாவி) == false || data == வெற்று) {
		திரும்பு;
	}

	தேர்வு (operation) {
	நிலை DOM_முனை_CLONED:
		error = css_libcss_முனை_data_handler(&selection_handler,
				CSS_முனை_CLONED,
				வெற்று, src, dst, data);
		எனில் (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		நிறுத்து;

	நிலை DOM_முனை_RENAMED:
		error = css_libcss_முனை_data_handler(&selection_handler,
				CSS_முனை_MODIFIED,
				வெற்று, src, வெற்று, data);
		எனில் (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		நிறுத்து;

	நிலை DOM_முனை_IMPORTED:
	நிலை DOM_முனை_ADOPTED:
	நிலை DOM_முனை_DELETED:
		error = css_libcss_முனை_data_handler(&selection_handler,
				CSS_முனை_DELETED,
				வெற்று, src, வெற்று, data);
		எனில் (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		நிறுத்து;

	இயல்பு:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ப்ரதிஜ்ஞா(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_முனை *n,
		மாறிலி css_media *media,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	முழு pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	எனில் (error != CSS_OK || styles == வெற்று) {
		/* Failed selecting partial style -- bail out */
		திரும்பு வெற்று;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	எனில் (ctx->parent_style != வெற்று) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		எனில் (error != CSS_OK) {
			css_select_results_destroy(styles);
			திரும்பு வெற்று;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	க்கு (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_மொத்தம்;
			pseudo_element++) {

		எனில் (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_வரி)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			தொடர்;

		எனில் (styles->styles[pseudo_element] == வெற்று)
			/* There were no rules concerning this pseudo element */
			தொடர்;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		எனில் (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			திரும்பு வெற்று;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	திரும்பு styles;
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
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	எனில் (error != CSS_OK) {
		திரும்பு வெற்று;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	எனில் (error != CSS_OK) {
		css_computed_style_destroy(composed);
		திரும்பு வெற்று;
	}

	திரும்பு composed;
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
css_error முனை_name(வெறுமை *pw, வெறுமை *முனை, css_qname *qname)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_exception err;

	err = dom_முனை_get_முனை_name(n, &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	qname->ns = வெற்று;

	err = dom_string_intern(name, &qname->name);
	எனில் (err != DOM_NO_ERR) {
		dom_string_unref(name);
		திரும்பு CSS_NOMEM;
	}

	dom_string_unref(name);

	திரும்பு CSS_OK;
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
css_error முனை_classes(வெறுமை *pw, வெறுமை *முனை,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_முனை *n = முனை;
	dom_exception err;

	*classes = வெற்று;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	திரும்பு CSS_OK;
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
css_error முனை_id(வெறுமை *pw, வெறுமை *முனை, lwc_string **id)
{
	dom_முனை *n = முனை;
	dom_string *attr;
	dom_exception err;

	*id = வெற்று;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	எனில் (attr != வெற்று) {
		err = dom_string_intern(attr, id);
		எனில் (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			திரும்பு CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	திரும்பு CSS_OK;
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
css_error named_ancestor_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **ancestor)
{
	dom_element_named_ancestor_முனை(முனை, qname->name,
			(கட்டமைப்பு dom_element **)ancestor);
	dom_முனை_unref(*ancestor);

	திரும்பு CSS_OK;
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
css_error named_parent_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **parent)
{
	dom_element_named_parent_முனை(முனை, qname->name,
			(கட்டமைப்பு dom_element **)parent);
	dom_முனை_unref(*parent);

	திரும்பு CSS_OK;
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
css_error named_sibling_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **sibling)
{
	dom_முனை *n = முனை;
	dom_முனை *prev;
	dom_exception err;

	*sibling = வெற்று;

	/* Find sibling element */
	err = dom_முனை_get_previous_sibling(n, &n);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_OK;

	போது (n != வெற்று) {
		dom_முனை_type type;

		err = dom_முனை_get_முனை_type(n, &type);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		எனில் (type == DOM_ELEMENT_முனை)
			நிறுத்து;

		err = dom_முனை_get_previous_sibling(n, &prev);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		dom_முனை_unref(n);
		n = prev;
	}

	எனில் (n != வெற்று) {
		dom_string *name;

		err = dom_முனை_get_முனை_name(n, &name);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		dom_முனை_unref(n);

		எனில் (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	திரும்பு CSS_OK;
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
css_error named_generic_sibling_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **sibling)
{
	dom_முனை *n = முனை;
	dom_முனை *prev;
	dom_exception err;

	*sibling = வெற்று;

	err = dom_முனை_get_previous_sibling(n, &n);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_OK;

	போது (n != வெற்று) {
		dom_முனை_type type;
		dom_string *name;

		err = dom_முனை_get_முனை_type(n, &type);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		எனில் (type == DOM_ELEMENT_முனை) {
			err = dom_முனை_get_முனை_name(n, &name);
			எனில் (err != DOM_NO_ERR) {
				dom_முனை_unref(n);
				திரும்பு CSS_OK;
			}

			எனில் (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_முனை_unref(n);
				*sibling = n;
				நிறுத்து;
			}
			dom_string_unref(name);
		}

		err = dom_முனை_get_previous_sibling(n, &prev);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		dom_முனை_unref(n);
		n = prev;
	}

	திரும்பு CSS_OK;
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
css_error parent_முனை(வெறுமை *pw, வெறுமை *முனை, வெறுமை **parent)
{
	dom_element_parent_முனை(முனை, (கட்டமைப்பு dom_element **)parent);
	dom_முனை_unref(*parent);

	திரும்பு CSS_OK;
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
css_error sibling_முனை(வெறுமை *pw, வெறுமை *முனை, வெறுமை **sibling)
{
	dom_முனை *n = முனை;
	dom_முனை *prev;
	dom_exception err;

	*sibling = வெற்று;

	/* Find sibling element */
	err = dom_முனை_get_previous_sibling(n, &n);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_OK;

	போது (n != வெற்று) {
		dom_முனை_type type;

		err = dom_முனை_get_முனை_type(n, &type);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		எனில் (type == DOM_ELEMENT_முனை)
			நிறுத்து;

		err = dom_முனை_get_previous_sibling(n, &prev);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_OK;
		}

		dom_முனை_unref(n);
		n = prev;
	}

	எனில் (n != வெற்று) {
		/** \todo Sort out reference counting */
		dom_முனை_unref(n);

		*sibling = n;
	}

	திரும்பு CSS_OK;
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
css_error முனை_has_name(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_முனை *n = முனை;

	எனில் (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_முனை_get_முனை_name(n, &name);
		எனில் (err != DOM_NO_ERR)
			திரும்பு CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	திரும்பு CSS_OK;
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
css_error முனை_has_class(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *name, bool *match)
{
	dom_முனை *n = முனை;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	ப்ரதிஜ்ஞா(err == DOM_NO_ERR);

	திரும்பு CSS_OK;
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
css_error முனை_has_id(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *name, bool *match)
{
	dom_முனை *n = முனை;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_OK;

	எனில் (attr != வெற்று) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(மாறிலி uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	எனில் (err != DOM_NO_ERR) {
		dom_string_unref(name);
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_equal(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(மதிப்பு);

	எனில் (vlen == 0) {
		*match = false;
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, மதிப்பு);

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_dashmatch(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(மதிப்பு);

	எனில் (vlen == 0) {
		*match = false;
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, மதிப்பு);

	/* check for dashmatch */
	எனில் (*match == false) {
		மாறிலி எழுத்து *vdata = lwc_string_data(மதிப்பு);
		மாறிலி எழுத்து *data = (மாறிலி எழுத்து *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		எனில் (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_includes(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(மதிப்பு);
	மாறிலி எழுத்து *p;
	மாறிலி எழுத்து *start;
	மாறிலி எழுத்து *end;

	*match = false;

	எனில் (vlen == 0) {
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (மாறிலி எழுத்து *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	க்கு (p = start; p <= end; p++) {
		எனில் (*p == ' ' || *p == '\0') {
			எனில் ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(மதிப்பு),
					vlen) == 0) {
				*match = true;
				நிறுத்து;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_prefix(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(மதிப்பு);

	எனில் (vlen == 0) {
		*match = false;
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, மதிப்பு);

	/* check for prefix match */
	எனில் (*match == false) {
		மாறிலி எழுத்து *data = (மாறிலி எழுத்து *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		எனில் ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(மதிப்பு), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_suffix(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(மதிப்பு);

	எனில் (vlen == 0) {
		*match = false;
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, மதிப்பு);

	/* check for prefix match */
	எனில் (*match == false) {
		மாறிலி எழுத்து *data = (மாறிலி எழுத்து *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		மாறிலி எழுத்து *start = (எழுத்து *) data + len - vlen;

		எனில் ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(மதிப்பு), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_has_attribute_substring(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, lwc_string *மதிப்பு,
		bool *match)
{
	dom_முனை *n = முனை;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(மதிப்பு);

	எனில் (vlen == 0) {
		*match = false;
		திரும்பு CSS_OK;
	}

	err = dom_string_create_interned(
		(மாறிலி uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	எனில் ((err != DOM_NO_ERR) || (atr_val == வெற்று)) {
		dom_string_unref(name);
		*match = false;
		திரும்பு CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, மதிப்பு);

	/* check for prefix match */
	எனில் (*match == false) {
		மாறிலி எழுத்து *vdata = lwc_string_data(மதிப்பு);
		மாறிலி எழுத்து *start = (மாறிலி எழுத்து *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		மாறிலி எழுத்து *last_start = start + len - vlen;

		எனில் (len >= vlen) {
			போது (start <= last_start) {
				எனில் (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					நிறுத்து;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	திரும்பு CSS_OK;
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
css_error முனை_is_வேர்(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	dom_முனை *n = முனை;
	dom_முனை *parent;
	dom_முனை_type type;
	dom_exception err;

	err = dom_முனை_get_parent_முனை(n, &parent);
	எனில் (err != DOM_NO_ERR) {
		திரும்பு CSS_NOMEM;
	}

	எனில் (parent != வெற்று) {
		err = dom_முனை_get_முனை_type(parent, &type);

		dom_முனை_unref(parent);

		எனில் (err != DOM_NO_ERR)
			திரும்பு CSS_NOMEM;

		எனில் (type != DOM_DOCUMENT_முனை) {
			*match = false;
			திரும்பு CSS_OK;
		}
	}

	*match = true;

	திரும்பு CSS_OK;
}

நிலையான முழு
முனை_மொத்தம்_siblings_check(dom_முனை *முனை,
			  bool check_name,
			  dom_string *name)
{
	dom_முனை_type type;
	முழு ret = 0;
	dom_exception exc;

	எனில் (முனை == வெற்று)
		திரும்பு 0;

	exc = dom_முனை_get_முனை_type(முனை, &type);
	எனில் ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_முனை)) {
		திரும்பு 0;
	}

	எனில் (check_name) {
		dom_string *முனை_name = வெற்று;
		exc = dom_முனை_get_முனை_name(முனை, &முனை_name);

		எனில் ((exc == DOM_NO_ERR) && (முனை_name != வெற்று)) {

			எனில் (dom_string_caseless_isequal(name,
							முனை_name)) {
				ret = 1;
			}
			dom_string_unref(முனை_name);
		}
	} இல்லையெனில் {
		ret = 1;
	}

	திரும்பு ret;
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
css_error முனை_மொத்தம்_siblings(வெறுமை *pw, வெறுமை *n, bool same_name,
		bool after, int32_t *மொத்தம்)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *முனை_name = வெற்று;

	எனில் (same_name) {
		dom_முனை *முனை = n;
		exc = dom_முனை_get_முனை_name(முனை, &முனை_name);
		எனில் ((exc != DOM_NO_ERR) || (முனை_name == வெற்று)) {
			திரும்பு CSS_NOMEM;
		}
	}

	எனில் (after) {
		dom_முனை *முனை = dom_முனை_ref(n);
		dom_முனை *next;

		செய் {
			exc = dom_முனை_get_next_sibling(முனை, &next);
			எனில் ((exc != DOM_NO_ERR))
				நிறுத்து;

			dom_முனை_unref(முனை);
			முனை = next;

			cnt += முனை_மொத்தம்_siblings_check(முனை, same_name, முனை_name);
		} போது (முனை != வெற்று);
	} இல்லையெனில் {
		dom_முனை *முனை = dom_முனை_ref(n);
		dom_முனை *next;

		செய் {
			exc = dom_முனை_get_previous_sibling(முனை, &next);
			எனில் ((exc != DOM_NO_ERR))
				நிறுத்து;

			dom_முனை_unref(முனை);
			முனை = next;

			cnt += முனை_மொத்தம்_siblings_check(முனை, same_name, முனை_name);

		} போது (முனை != வெற்று);
	}

	எனில் (முனை_name != வெற்று) {
		dom_string_unref(முனை_name);
	}

	*மொத்தம் = cnt;
	திரும்பு CSS_OK;
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
css_error முனை_is_empty(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	dom_முனை *n = முனை, *next;
	dom_exception err;

	*match = true;

	err = dom_முனை_get_first_child(n, &n);
	எனில் (err != DOM_NO_ERR) {
		திரும்பு CSS_BADPARM;
	}

	போது (n != வெற்று) {
		dom_முனை_type ntype;
		err = dom_முனை_get_முனை_type(n, &ntype);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_BADPARM;
		}

		எனில் (ntype == DOM_ELEMENT_முனை ||
		    ntype == DOM_TEXT_முனை) {
			*match = false;
			dom_முனை_unref(n);
			நிறுத்து;
		}

		err = dom_முனை_get_next_sibling(n, &next);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(n);
			திரும்பு CSS_BADPARM;
		}
		dom_முனை_unref(n);
		n = next;
	}

	திரும்பு CSS_OK;
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
css_error முனை_is_link(வெறுமை *pw, வெறுமை *n, bool *match)
{
	dom_முனை *முனை = n;
	dom_exception exc;
	dom_string *முனை_name = வெற்று;

	exc = dom_முனை_get_முனை_name(முனை, &முனை_name);
	எனில் ((exc != DOM_NO_ERR) || (முனை_name == வெற்று)) {
		திரும்பு CSS_NOMEM;
	}

	எனில் (dom_string_caseless_lwc_isequal(முனை_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(முனை, corestring_dom_href,
				&has_href);
		எனில் ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} இல்லையெனில் {
			*match = false;
		}
	} இல்லையெனில் {
		*match = false;
	}
	dom_string_unref(முனை_name);

	திரும்பு CSS_OK;
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
css_error முனை_is_visited(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	மாறிலி கட்டமைப்பு url_data *data;

	dom_exception exc;
	dom_முனை *n = முனை;
	dom_string *s = வெற்று;

	*match = false;

	exc = dom_முனை_get_முனை_name(n, &s);
	எனில் ((exc != DOM_NO_ERR) || (s == வெற்று)) {
		திரும்பு CSS_NOMEM;
	}

	எனில் (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		திரும்பு CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = வெற்று;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	எனில் ((exc != DOM_NO_ERR) || (s == வெற்று)) {
		/* Can't be visited; not got a URL */
		திரும்பு CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	எனில் (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		திரும்பு CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	எனில் (data != வெற்று && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	திரும்பு CSS_OK;
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
css_error முனை_is_hover(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_active(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_focus(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_enabled(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_disabled(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_checked(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_target(வெறுமை *pw, வெறுமை *முனை, bool *match)
{
	/** \todo Support target */

	*match = false;

	திரும்பு CSS_OK;
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
css_error முனை_is_lang(வெறுமை *pw, வெறுமை *முனை,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	திரும்பு CSS_OK;
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
css_error ua_default_for_property(வெறுமை *pw, uint32_t property, css_hint *hint)
{
	எனில் (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} இல்லையெனில் எனில் (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = வெற்று;
		தேர்வு (nsoption_int(font_default)) {
		நிலை PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			நிறுத்து;
		நிலை PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			நிறுத்து;
		நிலை PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			நிறுத்து;
		நிலை PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			நிறுத்து;
		நிலை PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			நிறுத்து;
		}
	} இல்லையெனில் எனில் (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = வெற்று;
		hint->status = CSS_QUOTES_NONE;
	} இல்லையெனில் எனில் (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = வெற்று;
		hint->status = 0;
	} இல்லையெனில் {
		திரும்பு CSS_INVALID;
	}

	திரும்பு CSS_OK;
}

css_error set_libcss_முனை_data(வெறுமை *pw, வெறுமை *முனை, வெறுமை *libcss_முனை_data)
{
	dom_முனை *n = முனை;
	dom_exception err;
	வெறுமை *old_முனை_data;

	/* Set this node's node data */
	err = dom_முனை_set_user_data(n,
			corestring_dom_ns_சாவி_libcss_முனை_data,
			libcss_முனை_data, nscss_dom_user_data_handler,
			(வெறுமை *) &old_முனை_data);
	எனில் (err != DOM_NO_ERR) {
		திரும்பு CSS_NOMEM;
	}

	ப்ரதிஜ்ஞா(old_முனை_data == வெற்று);

	திரும்பு CSS_OK;
}

css_error get_libcss_முனை_data(வெறுமை *pw, வெறுமை *முனை, வெறுமை **libcss_முனை_data)
{
	dom_முனை *n = முனை;
	dom_exception err;

	/* Get this node's node data */
	err = dom_முனை_get_user_data(n,
			corestring_dom_ns_சாவி_libcss_முனை_data,
			libcss_முனை_data);
	எனில் (err != DOM_NO_ERR) {
		திரும்பு CSS_NOMEM;
	}

	திரும்பு CSS_OK;
}
