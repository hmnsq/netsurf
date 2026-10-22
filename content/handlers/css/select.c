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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>

#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "netsurf/plot_style.h"
#অন্তর্ভাব "netsurf/url_db.h"
#অন্তর্ভাব "desktop/system_colour.h"

#অন্তর্ভাব "css/internal.h"
#অন্তর্ভাব "css/hints.h"
#অন্তর্ভাব "css/select.h"

স্থির css_error গ্রন্থি_name(শূন্য *pw, শূন্য *গ্রন্থি, css_qname *qname);
স্থির css_error গ্রন্থি_classes(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string ***classes, uint32_t *n_classes);
স্থির css_error গ্রন্থি_id(শূন্য *pw, শূন্য *গ্রন্থি, lwc_string **id);
স্থির css_error named_parent_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **parent);
স্থির css_error named_sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **sibling);
স্থির css_error named_generic_sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **sibling);
স্থির css_error parent_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য **parent);
স্থির css_error sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য **sibling);
স্থির css_error গ্রন্থি_has_name(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, bool *match);
স্থির css_error গ্রন্থি_has_class(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *name, bool *match);
স্থির css_error গ্রন্থি_has_id(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *name, bool *match);
স্থির css_error গ্রন্থি_has_attribute(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, bool *match);
স্থির css_error গ্রন্থি_has_attribute_equal(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_has_attribute_dashmatch(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_has_attribute_includes(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_has_attribute_prefix(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_has_attribute_suffix(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_has_attribute_substring(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match);
স্থির css_error গ্রন্থি_is_মূল(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_গুনতি_siblings(শূন্য *pw, শূন্য *গ্রন্থি,
		bool same_name, bool after, int32_t *গুনতি);
স্থির css_error গ্রন্থি_is_empty(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_link(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_hover(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_active(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_focus(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_enabled(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_disabled(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_checked(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_target(শূন্য *pw, শূন্য *গ্রন্থি, bool *match);
স্থির css_error গ্রন্থি_is_lang(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *lang, bool *match);
স্থির css_error ua_default_for_property(শূন্য *pw, uint32_t property,
		css_hint *hint);
স্থির css_error set_libcss_গ্রন্থি_data(শূন্য *pw, শূন্য *গ্রন্থি,
		শূন্য *libcss_গ্রন্থি_data);
স্থির css_error get_libcss_গ্রন্থি_data(শূন্য *pw, শূন্য *গ্রন্থি,
		শূন্য **libcss_গ্রন্থি_data);

/**
 * Selection callback table for libcss
 */
স্থির css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	গ্রন্থি_name,
	গ্রন্থি_classes,
	গ্রন্থি_id,
	named_ancestor_গ্রন্থি,
	named_parent_গ্রন্থি,
	named_sibling_গ্রন্থি,
	named_generic_sibling_গ্রন্থি,
	parent_গ্রন্থি,
	sibling_গ্রন্থি,
	গ্রন্থি_has_name,
	গ্রন্থি_has_class,
	গ্রন্থি_has_id,
	গ্রন্থি_has_attribute,
	গ্রন্থি_has_attribute_equal,
	গ্রন্থি_has_attribute_dashmatch,
	গ্রন্থি_has_attribute_includes,
	গ্রন্থি_has_attribute_prefix,
	গ্রন্থি_has_attribute_suffix,
	গ্রন্থি_has_attribute_substring,
	গ্রন্থি_is_মূল,
	গ্রন্থি_গুনতি_siblings,
	গ্রন্থি_is_empty,
	গ্রন্থি_is_link,
	গ্রন্থি_is_visited,
	গ্রন্থি_is_hover,
	গ্রন্থি_is_active,
	গ্রন্থি_is_focus,
	গ্রন্থি_is_enabled,
	গ্রন্থি_is_disabled,
	গ্রন্থি_is_checked,
	গ্রন্থি_is_target,
	গ্রন্থি_is_lang,
	গ্রন্থি_presentational_hint,
	ua_default_for_property,
	set_libcss_গ্রন্থি_data,
	get_libcss_গ্রন্থি_data,
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
css_stylesheet *nscss_create_inline_style(ধ্রুব uint8_t *data, size_t len,
		ধ্রুব বর্ণ *charset, ধ্রুব বর্ণ *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = রিক্ত;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = রিক্ত;
	params.import = রিক্ত;
	params.import_pw = রিক্ত;
	params.color = ns_system_colour;
	params.color_pw = রিক্ত;
	params.font = রিক্ত;
	params.font_pw = রিক্ত;

	error = css_stylesheet_create(&params, &sheet);
	যদি (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		প্রত্যাগম রিক্ত;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	যদি (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		প্রত্যাগম রিক্ত;
	}

	error = css_stylesheet_data_done(sheet);
	যদি (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
স্থির শূন্য nscss_dom_user_data_handler(dom_গ্রন্থি_operation operation,
		dom_string *চাবি, শূন্য *data, সংরচনা dom_গ্রন্থি *src,
		সংরচনা dom_গ্রন্থি *dst)
{
	css_error error;

	যদি (dom_string_isequal(corestring_dom_ns_চাবি_libcss_গ্রন্থি_data,
			চাবি) == false || data == রিক্ত) {
		প্রত্যাগম;
	}

	বিকল্প (operation) {
	পক্ষ DOM_গ্রন্থি_CLONED:
		error = css_libcss_গ্রন্থি_data_handler(&selection_handler,
				CSS_গ্রন্থি_CLONED,
				রিক্ত, src, dst, data);
		যদি (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ভঙ্গ;

	পক্ষ DOM_গ্রন্থি_RENAMED:
		error = css_libcss_গ্রন্থি_data_handler(&selection_handler,
				CSS_গ্রন্থি_MODIFIED,
				রিক্ত, src, রিক্ত, data);
		যদি (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ভঙ্গ;

	পক্ষ DOM_গ্রন্থি_IMPORTED:
	পক্ষ DOM_গ্রন্থি_ADOPTED:
	পক্ষ DOM_গ্রন্থি_DELETED:
		error = css_libcss_গ্রন্থি_data_handler(&selection_handler,
				CSS_গ্রন্থি_DELETED,
				রিক্ত, src, রিক্ত, data);
		যদি (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ভঙ্গ;

	স্বভাব:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		প্রতিজ্ঞা(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_গ্রন্থি *n,
		ধ্রুব css_media *media,
		ধ্রুব css_unit_ctx *unit_len_ctx,
		ধ্রুব css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	পূর্ণাঙ্ক pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	যদি (error != CSS_OK || styles == রিক্ত) {
		/* Failed selecting partial style -- bail out */
		প্রত্যাগম রিক্ত;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	যদি (ctx->parent_style != রিক্ত) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		যদি (error != CSS_OK) {
			css_select_results_destroy(styles);
			প্রত্যাগম রিক্ত;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	প্রতি (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_গুনতি;
			pseudo_element++) {

		যদি (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_সারি)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			অনুবর্ত;

		যদি (styles->styles[pseudo_element] == রিক্ত)
			/* There were no rules concerning this pseudo element */
			অনুবর্ত;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		যদি (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			প্রত্যাগম রিক্ত;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	প্রত্যাগম styles;
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
		ধ্রুব css_unit_ctx *unit_len_ctx,
		ধ্রুব css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	যদি (error != CSS_OK) {
		প্রত্যাগম রিক্ত;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	যদি (error != CSS_OK) {
		css_computed_style_destroy(composed);
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম composed;
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
css_error গ্রন্থি_name(শূন্য *pw, শূন্য *গ্রন্থি, css_qname *qname)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_exception err;

	err = dom_গ্রন্থি_get_গ্রন্থি_name(n, &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	qname->ns = রিক্ত;

	err = dom_string_intern(name, &qname->name);
	যদি (err != DOM_NO_ERR) {
		dom_string_unref(name);
		প্রত্যাগম CSS_NOMEM;
	}

	dom_string_unref(name);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_classes(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_exception err;

	*classes = রিক্ত;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_id(শূন্য *pw, শূন্য *গ্রন্থি, lwc_string **id)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *attr;
	dom_exception err;

	*id = রিক্ত;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	যদি (attr != রিক্ত) {
		err = dom_string_intern(attr, id);
		যদি (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			প্রত্যাগম CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	প্রত্যাগম CSS_OK;
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
css_error named_ancestor_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **ancestor)
{
	dom_element_named_ancestor_গ্রন্থি(গ্রন্থি, qname->name,
			(সংরচনা dom_element **)ancestor);
	dom_গ্রন্থি_unref(*ancestor);

	প্রত্যাগম CSS_OK;
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
css_error named_parent_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **parent)
{
	dom_element_named_parent_গ্রন্থি(গ্রন্থি, qname->name,
			(সংরচনা dom_element **)parent);
	dom_গ্রন্থি_unref(*parent);

	প্রত্যাগম CSS_OK;
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
css_error named_sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **sibling)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_গ্রন্থি *prev;
	dom_exception err;

	*sibling = রিক্ত;

	/* Find sibling element */
	err = dom_গ্রন্থি_get_previous_sibling(n, &n);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_OK;

	যাবত্ (n != রিক্ত) {
		dom_গ্রন্থি_type type;

		err = dom_গ্রন্থি_get_গ্রন্থি_type(n, &type);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		যদি (type == DOM_ELEMENT_গ্রন্থি)
			ভঙ্গ;

		err = dom_গ্রন্থি_get_previous_sibling(n, &prev);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		dom_গ্রন্থি_unref(n);
		n = prev;
	}

	যদি (n != রিক্ত) {
		dom_string *name;

		err = dom_গ্রন্থি_get_গ্রন্থি_name(n, &name);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		dom_গ্রন্থি_unref(n);

		যদি (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	প্রত্যাগম CSS_OK;
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
css_error named_generic_sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, শূন্য **sibling)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_গ্রন্থি *prev;
	dom_exception err;

	*sibling = রিক্ত;

	err = dom_গ্রন্থি_get_previous_sibling(n, &n);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_OK;

	যাবত্ (n != রিক্ত) {
		dom_গ্রন্থি_type type;
		dom_string *name;

		err = dom_গ্রন্থি_get_গ্রন্থি_type(n, &type);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		যদি (type == DOM_ELEMENT_গ্রন্থি) {
			err = dom_গ্রন্থি_get_গ্রন্থি_name(n, &name);
			যদি (err != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(n);
				প্রত্যাগম CSS_OK;
			}

			যদি (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_গ্রন্থি_unref(n);
				*sibling = n;
				ভঙ্গ;
			}
			dom_string_unref(name);
		}

		err = dom_গ্রন্থি_get_previous_sibling(n, &prev);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		dom_গ্রন্থি_unref(n);
		n = prev;
	}

	প্রত্যাগম CSS_OK;
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
css_error parent_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য **parent)
{
	dom_element_parent_গ্রন্থি(গ্রন্থি, (সংরচনা dom_element **)parent);
	dom_গ্রন্থি_unref(*parent);

	প্রত্যাগম CSS_OK;
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
css_error sibling_গ্রন্থি(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য **sibling)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_গ্রন্থি *prev;
	dom_exception err;

	*sibling = রিক্ত;

	/* Find sibling element */
	err = dom_গ্রন্থি_get_previous_sibling(n, &n);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_OK;

	যাবত্ (n != রিক্ত) {
		dom_গ্রন্থি_type type;

		err = dom_গ্রন্থি_get_গ্রন্থি_type(n, &type);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		যদি (type == DOM_ELEMENT_গ্রন্থি)
			ভঙ্গ;

		err = dom_গ্রন্থি_get_previous_sibling(n, &prev);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_OK;
		}

		dom_গ্রন্থি_unref(n);
		n = prev;
	}

	যদি (n != রিক্ত) {
		/** \todo Sort out reference counting */
		dom_গ্রন্থি_unref(n);

		*sibling = n;
	}

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_name(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_গ্রন্থি *n = গ্রন্থি;

	যদি (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_গ্রন্থি_get_গ্রন্থি_name(n, &name);
		যদি (err != DOM_NO_ERR)
			প্রত্যাগম CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_class(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *name, bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	প্রতিজ্ঞা(err == DOM_NO_ERR);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_id(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *name, bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_OK;

	যদি (attr != রিক্ত) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ধ্রুব uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	যদি (err != DOM_NO_ERR) {
		dom_string_unref(name);
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_equal(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(মান);

	যদি (vlen == 0) {
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, মান);

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_dashmatch(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(মান);

	যদি (vlen == 0) {
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, মান);

	/* check for dashmatch */
	যদি (*match == false) {
		ধ্রুব বর্ণ *vdata = lwc_string_data(মান);
		ধ্রুব বর্ণ *data = (ধ্রুব বর্ণ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		যদি (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_includes(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(মান);
	ধ্রুব বর্ণ *p;
	ধ্রুব বর্ণ *start;
	ধ্রুব বর্ণ *end;

	*match = false;

	যদি (vlen == 0) {
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ধ্রুব বর্ণ *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	প্রতি (p = start; p <= end; p++) {
		যদি (*p == ' ' || *p == '\0') {
			যদি ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(মান),
					vlen) == 0) {
				*match = true;
				ভঙ্গ;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_prefix(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(মান);

	যদি (vlen == 0) {
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, মান);

	/* check for prefix match */
	যদি (*match == false) {
		ধ্রুব বর্ণ *data = (ধ্রুব বর্ণ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		যদি ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(মান), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_suffix(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(মান);

	যদি (vlen == 0) {
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, মান);

	/* check for prefix match */
	যদি (*match == false) {
		ধ্রুব বর্ণ *data = (ধ্রুব বর্ণ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ধ্রুব বর্ণ *start = (বর্ণ *) data + len - vlen;

		যদি ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(মান), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_has_attribute_substring(শূন্য *pw, শূন্য *গ্রন্থি,
		ধ্রুব css_qname *qname, lwc_string *মান,
		bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(মান);

	যদি (vlen == 0) {
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	err = dom_string_create_interned(
		(ধ্রুব uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	যদি ((err != DOM_NO_ERR) || (atr_val == রিক্ত)) {
		dom_string_unref(name);
		*match = false;
		প্রত্যাগম CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, মান);

	/* check for prefix match */
	যদি (*match == false) {
		ধ্রুব বর্ণ *vdata = lwc_string_data(মান);
		ধ্রুব বর্ণ *start = (ধ্রুব বর্ণ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ধ্রুব বর্ণ *last_start = start + len - vlen;

		যদি (len >= vlen) {
			যাবত্ (start <= last_start) {
				যদি (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					ভঙ্গ;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_মূল(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_গ্রন্থি *parent;
	dom_গ্রন্থি_type type;
	dom_exception err;

	err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
	যদি (err != DOM_NO_ERR) {
		প্রত্যাগম CSS_NOMEM;
	}

	যদি (parent != রিক্ত) {
		err = dom_গ্রন্থি_get_গ্রন্থি_type(parent, &type);

		dom_গ্রন্থি_unref(parent);

		যদি (err != DOM_NO_ERR)
			প্রত্যাগম CSS_NOMEM;

		যদি (type != DOM_DOCUMENT_গ্রন্থি) {
			*match = false;
			প্রত্যাগম CSS_OK;
		}
	}

	*match = true;

	প্রত্যাগম CSS_OK;
}

স্থির পূর্ণাঙ্ক
গ্রন্থি_গুনতি_siblings_check(dom_গ্রন্থি *গ্রন্থি,
			  bool check_name,
			  dom_string *name)
{
	dom_গ্রন্থি_type type;
	পূর্ণাঙ্ক ret = 0;
	dom_exception exc;

	যদি (গ্রন্থি == রিক্ত)
		প্রত্যাগম 0;

	exc = dom_গ্রন্থি_get_গ্রন্থি_type(গ্রন্থি, &type);
	যদি ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_গ্রন্থি)) {
		প্রত্যাগম 0;
	}

	যদি (check_name) {
		dom_string *গ্রন্থি_name = রিক্ত;
		exc = dom_গ্রন্থি_get_গ্রন্থি_name(গ্রন্থি, &গ্রন্থি_name);

		যদি ((exc == DOM_NO_ERR) && (গ্রন্থি_name != রিক্ত)) {

			যদি (dom_string_caseless_isequal(name,
							গ্রন্থি_name)) {
				ret = 1;
			}
			dom_string_unref(গ্রন্থি_name);
		}
	} অন্যথা {
		ret = 1;
	}

	প্রত্যাগম ret;
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
css_error গ্রন্থি_গুনতি_siblings(শূন্য *pw, শূন্য *n, bool same_name,
		bool after, int32_t *গুনতি)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *গ্রন্থি_name = রিক্ত;

	যদি (same_name) {
		dom_গ্রন্থি *গ্রন্থি = n;
		exc = dom_গ্রন্থি_get_গ্রন্থি_name(গ্রন্থি, &গ্রন্থি_name);
		যদি ((exc != DOM_NO_ERR) || (গ্রন্থি_name == রিক্ত)) {
			প্রত্যাগম CSS_NOMEM;
		}
	}

	যদি (after) {
		dom_গ্রন্থি *গ্রন্থি = dom_গ্রন্থি_ref(n);
		dom_গ্রন্থি *next;

		কুরু {
			exc = dom_গ্রন্থি_get_next_sibling(গ্রন্থি, &next);
			যদি ((exc != DOM_NO_ERR))
				ভঙ্গ;

			dom_গ্রন্থি_unref(গ্রন্থি);
			গ্রন্থি = next;

			cnt += গ্রন্থি_গুনতি_siblings_check(গ্রন্থি, same_name, গ্রন্থি_name);
		} যাবত্ (গ্রন্থি != রিক্ত);
	} অন্যথা {
		dom_গ্রন্থি *গ্রন্থি = dom_গ্রন্থি_ref(n);
		dom_গ্রন্থি *next;

		কুরু {
			exc = dom_গ্রন্থি_get_previous_sibling(গ্রন্থি, &next);
			যদি ((exc != DOM_NO_ERR))
				ভঙ্গ;

			dom_গ্রন্থি_unref(গ্রন্থি);
			গ্রন্থি = next;

			cnt += গ্রন্থি_গুনতি_siblings_check(গ্রন্থি, same_name, গ্রন্থি_name);

		} যাবত্ (গ্রন্থি != রিক্ত);
	}

	যদি (গ্রন্থি_name != রিক্ত) {
		dom_string_unref(গ্রন্থি_name);
	}

	*গুনতি = cnt;
	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_empty(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	dom_গ্রন্থি *n = গ্রন্থি, *next;
	dom_exception err;

	*match = true;

	err = dom_গ্রন্থি_get_first_child(n, &n);
	যদি (err != DOM_NO_ERR) {
		প্রত্যাগম CSS_BADPARM;
	}

	যাবত্ (n != রিক্ত) {
		dom_গ্রন্থি_type ntype;
		err = dom_গ্রন্থি_get_গ্রন্থি_type(n, &ntype);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_BADPARM;
		}

		যদি (ntype == DOM_ELEMENT_গ্রন্থি ||
		    ntype == DOM_TEXT_গ্রন্থি) {
			*match = false;
			dom_গ্রন্থি_unref(n);
			ভঙ্গ;
		}

		err = dom_গ্রন্থি_get_next_sibling(n, &next);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(n);
			প্রত্যাগম CSS_BADPARM;
		}
		dom_গ্রন্থি_unref(n);
		n = next;
	}

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_link(শূন্য *pw, শূন্য *n, bool *match)
{
	dom_গ্রন্থি *গ্রন্থি = n;
	dom_exception exc;
	dom_string *গ্রন্থি_name = রিক্ত;

	exc = dom_গ্রন্থি_get_গ্রন্থি_name(গ্রন্থি, &গ্রন্থি_name);
	যদি ((exc != DOM_NO_ERR) || (গ্রন্থি_name == রিক্ত)) {
		প্রত্যাগম CSS_NOMEM;
	}

	যদি (dom_string_caseless_lwc_isequal(গ্রন্থি_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(গ্রন্থি, corestring_dom_href,
				&has_href);
		যদি ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} অন্যথা {
			*match = false;
		}
	} অন্যথা {
		*match = false;
	}
	dom_string_unref(গ্রন্থি_name);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_visited(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ধ্রুব সংরচনা url_data *data;

	dom_exception exc;
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_string *s = রিক্ত;

	*match = false;

	exc = dom_গ্রন্থি_get_গ্রন্থি_name(n, &s);
	যদি ((exc != DOM_NO_ERR) || (s == রিক্ত)) {
		প্রত্যাগম CSS_NOMEM;
	}

	যদি (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		প্রত্যাগম CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = রিক্ত;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	যদি ((exc != DOM_NO_ERR) || (s == রিক্ত)) {
		/* Can't be visited; not got a URL */
		প্রত্যাগম CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	যদি (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		প্রত্যাগম CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	যদি (data != রিক্ত && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_hover(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_active(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_focus(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_enabled(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_disabled(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_checked(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_target(শূন্য *pw, শূন্য *গ্রন্থি, bool *match)
{
	/** \todo Support target */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error গ্রন্থি_is_lang(শূন্য *pw, শূন্য *গ্রন্থি,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	প্রত্যাগম CSS_OK;
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
css_error ua_default_for_property(শূন্য *pw, uint32_t property, css_hint *hint)
{
	যদি (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} অন্যথা যদি (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = রিক্ত;
		বিকল্প (nsoption_int(font_default)) {
		পক্ষ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			ভঙ্গ;
		পক্ষ PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			ভঙ্গ;
		পক্ষ PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			ভঙ্গ;
		পক্ষ PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			ভঙ্গ;
		পক্ষ PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			ভঙ্গ;
		}
	} অন্যথা যদি (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = রিক্ত;
		hint->status = CSS_QUOTES_NONE;
	} অন্যথা যদি (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = রিক্ত;
		hint->status = 0;
	} অন্যথা {
		প্রত্যাগম CSS_INVALID;
	}

	প্রত্যাগম CSS_OK;
}

css_error set_libcss_গ্রন্থি_data(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য *libcss_গ্রন্থি_data)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_exception err;
	শূন্য *old_গ্রন্থি_data;

	/* Set this node's node data */
	err = dom_গ্রন্থি_set_user_data(n,
			corestring_dom_ns_চাবি_libcss_গ্রন্থি_data,
			libcss_গ্রন্থি_data, nscss_dom_user_data_handler,
			(শূন্য *) &old_গ্রন্থি_data);
	যদি (err != DOM_NO_ERR) {
		প্রত্যাগম CSS_NOMEM;
	}

	প্রতিজ্ঞা(old_গ্রন্থি_data == রিক্ত);

	প্রত্যাগম CSS_OK;
}

css_error get_libcss_গ্রন্থি_data(শূন্য *pw, শূন্য *গ্রন্থি, শূন্য **libcss_গ্রন্থি_data)
{
	dom_গ্রন্থি *n = গ্রন্থি;
	dom_exception err;

	/* Get this node's node data */
	err = dom_গ্রন্থি_get_user_data(n,
			corestring_dom_ns_চাবি_libcss_গ্রন্থি_data,
			libcss_গ্রন্থি_data);
	যদি (err != DOM_NO_ERR) {
		প্রত্যাগম CSS_NOMEM;
	}

	প্রত্যাগম CSS_OK;
}
