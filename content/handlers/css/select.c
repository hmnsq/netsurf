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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>

#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "netsurf/plot_style.h"
#અન્તર્ભાવ "netsurf/url_db.h"
#અન્તર્ભાવ "desktop/system_colour.h"

#અન્તર્ભાવ "css/internal.h"
#અન્તર્ભાવ "css/hints.h"
#અન્તર્ભાવ "css/select.h"

સ્થિર css_error ગાંઠ_name(શૂન્ય *pw, શૂન્ય *ગાંઠ, css_qname *qname);
સ્થિર css_error ગાંઠ_classes(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string ***classes, uint32_t *n_classes);
સ્થિર css_error ગાંઠ_id(શૂન્ય *pw, શૂન્ય *ગાંઠ, lwc_string **id);
સ્થિર css_error named_parent_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **parent);
સ્થિર css_error named_sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **sibling);
સ્થિર css_error named_generic_sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **sibling);
સ્થિર css_error parent_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય **parent);
સ્થિર css_error sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય **sibling);
સ્થિર css_error ગાંઠ_has_name(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, bool *match);
સ્થિર css_error ગાંઠ_has_class(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *name, bool *match);
સ્થિર css_error ગાંઠ_has_id(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *name, bool *match);
સ્થિર css_error ગાંઠ_has_attribute(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, bool *match);
સ્થિર css_error ગાંઠ_has_attribute_equal(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_has_attribute_dashmatch(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_has_attribute_includes(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_has_attribute_prefix(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_has_attribute_suffix(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_has_attribute_substring(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match);
સ્થિર css_error ગાંઠ_is_મૂળ(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_ગણતરી_siblings(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		bool same_name, bool after, int32_t *ગણતરી);
સ્થિર css_error ગાંઠ_is_empty(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_link(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_hover(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_active(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_focus(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_enabled(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_disabled(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_checked(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_target(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);
સ્થિર css_error ગાંઠ_is_lang(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *lang, bool *match);
સ્થિર css_error ua_default_for_property(શૂન્ય *pw, uint32_t property,
		css_hint *hint);
સ્થિર css_error set_libcss_ગાંઠ_data(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		શૂન્ય *libcss_ગાંઠ_data);
સ્થિર css_error get_libcss_ગાંઠ_data(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		શૂન્ય **libcss_ગાંઠ_data);

/**
 * Selection callback table for libcss
 */
સ્થિર css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	ગાંઠ_name,
	ગાંઠ_classes,
	ગાંઠ_id,
	named_ancestor_ગાંઠ,
	named_parent_ગાંઠ,
	named_sibling_ગાંઠ,
	named_generic_sibling_ગાંઠ,
	parent_ગાંઠ,
	sibling_ગાંઠ,
	ગાંઠ_has_name,
	ગાંઠ_has_class,
	ગાંઠ_has_id,
	ગાંઠ_has_attribute,
	ગાંઠ_has_attribute_equal,
	ગાંઠ_has_attribute_dashmatch,
	ગાંઠ_has_attribute_includes,
	ગાંઠ_has_attribute_prefix,
	ગાંઠ_has_attribute_suffix,
	ગાંઠ_has_attribute_substring,
	ગાંઠ_is_મૂળ,
	ગાંઠ_ગણતરી_siblings,
	ગાંઠ_is_empty,
	ગાંઠ_is_link,
	ગાંઠ_is_visited,
	ગાંઠ_is_hover,
	ગાંઠ_is_active,
	ગાંઠ_is_focus,
	ગાંઠ_is_enabled,
	ગાંઠ_is_disabled,
	ગાંઠ_is_checked,
	ગાંઠ_is_target,
	ગાંઠ_is_lang,
	ગાંઠ_presentational_hint,
	ua_default_for_property,
	set_libcss_ગાંઠ_data,
	get_libcss_ગાંઠ_data,
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
css_stylesheet *nscss_create_inline_style(અચલ uint8_t *data, size_t len,
		અચલ અક્ષર *charset, અચલ અક્ષર *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ખાલી;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ખાલી;
	params.import = ખાલી;
	params.import_pw = ખાલી;
	params.color = ns_system_colour;
	params.color_pw = ખાલી;
	params.font = ખાલી;
	params.font_pw = ખાલી;

	error = css_stylesheet_create(&params, &sheet);
	જો (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		પાછા ખાલી;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	જો (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		પાછા ખાલી;
	}

	error = css_stylesheet_data_done(sheet);
	જો (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		પાછા ખાલી;
	}

	પાછા sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
સ્થિર શૂન્ય nscss_dom_user_data_handler(dom_ગાંઠ_operation operation,
		dom_string *ચાવી, શૂન્ય *data, સંરચના dom_ગાંઠ *src,
		સંરચના dom_ગાંઠ *dst)
{
	css_error error;

	જો (dom_string_isequal(corestring_dom_ns_ચાવી_libcss_ગાંઠ_data,
			ચાવી) == false || data == ખાલી) {
		પાછા;
	}

	પસંદગી (operation) {
	સ્થિતિ DOM_ગાંઠ_CLONED:
		error = css_libcss_ગાંઠ_data_handler(&selection_handler,
				CSS_ગાંઠ_CLONED,
				ખાલી, src, dst, data);
		જો (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		તોડો;

	સ્થિતિ DOM_ગાંઠ_RENAMED:
		error = css_libcss_ગાંઠ_data_handler(&selection_handler,
				CSS_ગાંઠ_MODIFIED,
				ખાલી, src, ખાલી, data);
		જો (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		તોડો;

	સ્થિતિ DOM_ગાંઠ_IMPORTED:
	સ્થિતિ DOM_ગાંઠ_ADOPTED:
	સ્થિતિ DOM_ગાંઠ_DELETED:
		error = css_libcss_ગાંઠ_data_handler(&selection_handler,
				CSS_ગાંઠ_DELETED,
				ખાલી, src, ખાલી, data);
		જો (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		તોડો;

	મૂળભૂત:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		પ્રતિજ્ઞા(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ગાંઠ *n,
		અચલ css_media *media,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	પૂર્ણાંક pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	જો (error != CSS_OK || styles == ખાલી) {
		/* Failed selecting partial style -- bail out */
		પાછા ખાલી;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	જો (ctx->parent_style != ખાલી) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		જો (error != CSS_OK) {
			css_select_results_destroy(styles);
			પાછા ખાલી;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	માટે (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_ગણતરી;
			pseudo_element++) {

		જો (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_લીટી)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ચાલુ;

		જો (styles->styles[pseudo_element] == ખાલી)
			/* There were no rules concerning this pseudo element */
			ચાલુ;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		જો (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			પાછા ખાલી;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	પાછા styles;
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
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	જો (error != CSS_OK) {
		પાછા ખાલી;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	જો (error != CSS_OK) {
		css_computed_style_destroy(composed);
		પાછા ખાલી;
	}

	પાછા composed;
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
css_error ગાંઠ_name(શૂન્ય *pw, શૂન્ય *ગાંઠ, css_qname *qname)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_exception err;

	err = dom_ગાંઠ_get_ગાંઠ_name(n, &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	qname->ns = ખાલી;

	err = dom_string_intern(name, &qname->name);
	જો (err != DOM_NO_ERR) {
		dom_string_unref(name);
		પાછા CSS_NOMEM;
	}

	dom_string_unref(name);

	પાછા CSS_OK;
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
css_error ગાંઠ_classes(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_exception err;

	*classes = ખાલી;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	પાછા CSS_OK;
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
css_error ગાંઠ_id(શૂન્ય *pw, શૂન્ય *ગાંઠ, lwc_string **id)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *attr;
	dom_exception err;

	*id = ખાલી;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	જો (attr != ખાલી) {
		err = dom_string_intern(attr, id);
		જો (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			પાછા CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	પાછા CSS_OK;
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
css_error named_ancestor_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **ancestor)
{
	dom_element_named_ancestor_ગાંઠ(ગાંઠ, qname->name,
			(સંરચના dom_element **)ancestor);
	dom_ગાંઠ_unref(*ancestor);

	પાછા CSS_OK;
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
css_error named_parent_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **parent)
{
	dom_element_named_parent_ગાંઠ(ગાંઠ, qname->name,
			(સંરચના dom_element **)parent);
	dom_ગાંઠ_unref(*parent);

	પાછા CSS_OK;
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
css_error named_sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **sibling)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_ગાંઠ *prev;
	dom_exception err;

	*sibling = ખાલી;

	/* Find sibling element */
	err = dom_ગાંઠ_get_previous_sibling(n, &n);
	જો (err != DOM_NO_ERR)
		પાછા CSS_OK;

	જ્યારે (n != ખાલી) {
		dom_ગાંઠ_type type;

		err = dom_ગાંઠ_get_ગાંઠ_type(n, &type);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		જો (type == DOM_ELEMENT_ગાંઠ)
			તોડો;

		err = dom_ગાંઠ_get_previous_sibling(n, &prev);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		dom_ગાંઠ_unref(n);
		n = prev;
	}

	જો (n != ખાલી) {
		dom_string *name;

		err = dom_ગાંઠ_get_ગાંઠ_name(n, &name);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		dom_ગાંઠ_unref(n);

		જો (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	પાછા CSS_OK;
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
css_error named_generic_sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **sibling)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_ગાંઠ *prev;
	dom_exception err;

	*sibling = ખાલી;

	err = dom_ગાંઠ_get_previous_sibling(n, &n);
	જો (err != DOM_NO_ERR)
		પાછા CSS_OK;

	જ્યારે (n != ખાલી) {
		dom_ગાંઠ_type type;
		dom_string *name;

		err = dom_ગાંઠ_get_ગાંઠ_type(n, &type);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		જો (type == DOM_ELEMENT_ગાંઠ) {
			err = dom_ગાંઠ_get_ગાંઠ_name(n, &name);
			જો (err != DOM_NO_ERR) {
				dom_ગાંઠ_unref(n);
				પાછા CSS_OK;
			}

			જો (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ગાંઠ_unref(n);
				*sibling = n;
				તોડો;
			}
			dom_string_unref(name);
		}

		err = dom_ગાંઠ_get_previous_sibling(n, &prev);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		dom_ગાંઠ_unref(n);
		n = prev;
	}

	પાછા CSS_OK;
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
css_error parent_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય **parent)
{
	dom_element_parent_ગાંઠ(ગાંઠ, (સંરચના dom_element **)parent);
	dom_ગાંઠ_unref(*parent);

	પાછા CSS_OK;
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
css_error sibling_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય **sibling)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_ગાંઠ *prev;
	dom_exception err;

	*sibling = ખાલી;

	/* Find sibling element */
	err = dom_ગાંઠ_get_previous_sibling(n, &n);
	જો (err != DOM_NO_ERR)
		પાછા CSS_OK;

	જ્યારે (n != ખાલી) {
		dom_ગાંઠ_type type;

		err = dom_ગાંઠ_get_ગાંઠ_type(n, &type);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		જો (type == DOM_ELEMENT_ગાંઠ)
			તોડો;

		err = dom_ગાંઠ_get_previous_sibling(n, &prev);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_OK;
		}

		dom_ગાંઠ_unref(n);
		n = prev;
	}

	જો (n != ખાલી) {
		/** \todo Sort out reference counting */
		dom_ગાંઠ_unref(n);

		*sibling = n;
	}

	પાછા CSS_OK;
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
css_error ગાંઠ_has_name(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_ગાંઠ *n = ગાંઠ;

	જો (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ગાંઠ_get_ગાંઠ_name(n, &name);
		જો (err != DOM_NO_ERR)
			પાછા CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	પાછા CSS_OK;
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
css_error ગાંઠ_has_class(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *name, bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	પ્રતિજ્ઞા(err == DOM_NO_ERR);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_id(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *name, bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	જો (err != DOM_NO_ERR)
		પાછા CSS_OK;

	જો (attr != ખાલી) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(અચલ uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	જો (err != DOM_NO_ERR) {
		dom_string_unref(name);
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_equal(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(કિંમત);

	જો (vlen == 0) {
		*match = false;
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, કિંમત);

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_dashmatch(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(કિંમત);

	જો (vlen == 0) {
		*match = false;
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, કિંમત);

	/* check for dashmatch */
	જો (*match == false) {
		અચલ અક્ષર *vdata = lwc_string_data(કિંમત);
		અચલ અક્ષર *data = (અચલ અક્ષર *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		જો (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_includes(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(કિંમત);
	અચલ અક્ષર *p;
	અચલ અક્ષર *start;
	અચલ અક્ષર *end;

	*match = false;

	જો (vlen == 0) {
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (અચલ અક્ષર *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	માટે (p = start; p <= end; p++) {
		જો (*p == ' ' || *p == '\0') {
			જો ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(કિંમત),
					vlen) == 0) {
				*match = true;
				તોડો;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_prefix(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(કિંમત);

	જો (vlen == 0) {
		*match = false;
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, કિંમત);

	/* check for prefix match */
	જો (*match == false) {
		અચલ અક્ષર *data = (અચલ અક્ષર *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		જો ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(કિંમત), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_suffix(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(કિંમત);

	જો (vlen == 0) {
		*match = false;
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, કિંમત);

	/* check for prefix match */
	જો (*match == false) {
		અચલ અક્ષર *data = (અચલ અક્ષર *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		અચલ અક્ષર *start = (અક્ષર *) data + len - vlen;

		જો ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(કિંમત), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_has_attribute_substring(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, lwc_string *કિંમત,
		bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(કિંમત);

	જો (vlen == 0) {
		*match = false;
		પાછા CSS_OK;
	}

	err = dom_string_create_interned(
		(અચલ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	જો (err != DOM_NO_ERR)
		પાછા CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	જો ((err != DOM_NO_ERR) || (atr_val == ખાલી)) {
		dom_string_unref(name);
		*match = false;
		પાછા CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, કિંમત);

	/* check for prefix match */
	જો (*match == false) {
		અચલ અક્ષર *vdata = lwc_string_data(કિંમત);
		અચલ અક્ષર *start = (અચલ અક્ષર *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		અચલ અક્ષર *last_start = start + len - vlen;

		જો (len >= vlen) {
			જ્યારે (start <= last_start) {
				જો (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					તોડો;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	પાછા CSS_OK;
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
css_error ગાંઠ_is_મૂળ(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_ગાંઠ *parent;
	dom_ગાંઠ_type type;
	dom_exception err;

	err = dom_ગાંઠ_get_parent_ગાંઠ(n, &parent);
	જો (err != DOM_NO_ERR) {
		પાછા CSS_NOMEM;
	}

	જો (parent != ખાલી) {
		err = dom_ગાંઠ_get_ગાંઠ_type(parent, &type);

		dom_ગાંઠ_unref(parent);

		જો (err != DOM_NO_ERR)
			પાછા CSS_NOMEM;

		જો (type != DOM_DOCUMENT_ગાંઠ) {
			*match = false;
			પાછા CSS_OK;
		}
	}

	*match = true;

	પાછા CSS_OK;
}

સ્થિર પૂર્ણાંક
ગાંઠ_ગણતરી_siblings_check(dom_ગાંઠ *ગાંઠ,
			  bool check_name,
			  dom_string *name)
{
	dom_ગાંઠ_type type;
	પૂર્ણાંક ret = 0;
	dom_exception exc;

	જો (ગાંઠ == ખાલી)
		પાછા 0;

	exc = dom_ગાંઠ_get_ગાંઠ_type(ગાંઠ, &type);
	જો ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ગાંઠ)) {
		પાછા 0;
	}

	જો (check_name) {
		dom_string *ગાંઠ_name = ખાલી;
		exc = dom_ગાંઠ_get_ગાંઠ_name(ગાંઠ, &ગાંઠ_name);

		જો ((exc == DOM_NO_ERR) && (ગાંઠ_name != ખાલી)) {

			જો (dom_string_caseless_isequal(name,
							ગાંઠ_name)) {
				ret = 1;
			}
			dom_string_unref(ગાંઠ_name);
		}
	} નહીંતર {
		ret = 1;
	}

	પાછા ret;
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
css_error ગાંઠ_ગણતરી_siblings(શૂન્ય *pw, શૂન્ય *n, bool same_name,
		bool after, int32_t *ગણતરી)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ગાંઠ_name = ખાલી;

	જો (same_name) {
		dom_ગાંઠ *ગાંઠ = n;
		exc = dom_ગાંઠ_get_ગાંઠ_name(ગાંઠ, &ગાંઠ_name);
		જો ((exc != DOM_NO_ERR) || (ગાંઠ_name == ખાલી)) {
			પાછા CSS_NOMEM;
		}
	}

	જો (after) {
		dom_ગાંઠ *ગાંઠ = dom_ગાંઠ_ref(n);
		dom_ગાંઠ *next;

		કરો {
			exc = dom_ગાંઠ_get_next_sibling(ગાંઠ, &next);
			જો ((exc != DOM_NO_ERR))
				તોડો;

			dom_ગાંઠ_unref(ગાંઠ);
			ગાંઠ = next;

			cnt += ગાંઠ_ગણતરી_siblings_check(ગાંઠ, same_name, ગાંઠ_name);
		} જ્યારે (ગાંઠ != ખાલી);
	} નહીંતર {
		dom_ગાંઠ *ગાંઠ = dom_ગાંઠ_ref(n);
		dom_ગાંઠ *next;

		કરો {
			exc = dom_ગાંઠ_get_previous_sibling(ગાંઠ, &next);
			જો ((exc != DOM_NO_ERR))
				તોડો;

			dom_ગાંઠ_unref(ગાંઠ);
			ગાંઠ = next;

			cnt += ગાંઠ_ગણતરી_siblings_check(ગાંઠ, same_name, ગાંઠ_name);

		} જ્યારે (ગાંઠ != ખાલી);
	}

	જો (ગાંઠ_name != ખાલી) {
		dom_string_unref(ગાંઠ_name);
	}

	*ગણતરી = cnt;
	પાછા CSS_OK;
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
css_error ગાંઠ_is_empty(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	dom_ગાંઠ *n = ગાંઠ, *next;
	dom_exception err;

	*match = true;

	err = dom_ગાંઠ_get_first_child(n, &n);
	જો (err != DOM_NO_ERR) {
		પાછા CSS_BADPARM;
	}

	જ્યારે (n != ખાલી) {
		dom_ગાંઠ_type ntype;
		err = dom_ગાંઠ_get_ગાંઠ_type(n, &ntype);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_BADPARM;
		}

		જો (ntype == DOM_ELEMENT_ગાંઠ ||
		    ntype == DOM_TEXT_ગાંઠ) {
			*match = false;
			dom_ગાંઠ_unref(n);
			તોડો;
		}

		err = dom_ગાંઠ_get_next_sibling(n, &next);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(n);
			પાછા CSS_BADPARM;
		}
		dom_ગાંઠ_unref(n);
		n = next;
	}

	પાછા CSS_OK;
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
css_error ગાંઠ_is_link(શૂન્ય *pw, શૂન્ય *n, bool *match)
{
	dom_ગાંઠ *ગાંઠ = n;
	dom_exception exc;
	dom_string *ગાંઠ_name = ખાલી;

	exc = dom_ગાંઠ_get_ગાંઠ_name(ગાંઠ, &ગાંઠ_name);
	જો ((exc != DOM_NO_ERR) || (ગાંઠ_name == ખાલી)) {
		પાછા CSS_NOMEM;
	}

	જો (dom_string_caseless_lwc_isequal(ગાંઠ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ગાંઠ, corestring_dom_href,
				&has_href);
		જો ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} નહીંતર {
			*match = false;
		}
	} નહીંતર {
		*match = false;
	}
	dom_string_unref(ગાંઠ_name);

	પાછા CSS_OK;
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
css_error ગાંઠ_is_visited(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	અચલ સંરચના url_data *data;

	dom_exception exc;
	dom_ગાંઠ *n = ગાંઠ;
	dom_string *s = ખાલી;

	*match = false;

	exc = dom_ગાંઠ_get_ગાંઠ_name(n, &s);
	જો ((exc != DOM_NO_ERR) || (s == ખાલી)) {
		પાછા CSS_NOMEM;
	}

	જો (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		પાછા CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ખાલી;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	જો ((exc != DOM_NO_ERR) || (s == ખાલી)) {
		/* Can't be visited; not got a URL */
		પાછા CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	જો (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		પાછા CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	જો (data != ખાલી && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	પાછા CSS_OK;
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
css_error ગાંઠ_is_hover(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_active(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_focus(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_enabled(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_disabled(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_checked(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_target(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match)
{
	/** \todo Support target */

	*match = false;

	પાછા CSS_OK;
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
css_error ગાંઠ_is_lang(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	પાછા CSS_OK;
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
css_error ua_default_for_property(શૂન્ય *pw, uint32_t property, css_hint *hint)
{
	જો (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} નહીંતર જો (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = ખાલી;
		પસંદગી (nsoption_int(font_default)) {
		સ્થિતિ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			તોડો;
		સ્થિતિ PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			તોડો;
		સ્થિતિ PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			તોડો;
		સ્થિતિ PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			તોડો;
		સ્થિતિ PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			તોડો;
		}
	} નહીંતર જો (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = ખાલી;
		hint->status = CSS_QUOTES_NONE;
	} નહીંતર જો (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = ખાલી;
		hint->status = 0;
	} નહીંતર {
		પાછા CSS_INVALID;
	}

	પાછા CSS_OK;
}

css_error set_libcss_ગાંઠ_data(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય *libcss_ગાંઠ_data)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_exception err;
	શૂન્ય *old_ગાંઠ_data;

	/* Set this node's node data */
	err = dom_ગાંઠ_set_user_data(n,
			corestring_dom_ns_ચાવી_libcss_ગાંઠ_data,
			libcss_ગાંઠ_data, nscss_dom_user_data_handler,
			(શૂન્ય *) &old_ગાંઠ_data);
	જો (err != DOM_NO_ERR) {
		પાછા CSS_NOMEM;
	}

	પ્રતિજ્ઞા(old_ગાંઠ_data == ખાલી);

	પાછા CSS_OK;
}

css_error get_libcss_ગાંઠ_data(શૂન્ય *pw, શૂન્ય *ગાંઠ, શૂન્ય **libcss_ગાંઠ_data)
{
	dom_ગાંઠ *n = ગાંઠ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ગાંઠ_get_user_data(n,
			corestring_dom_ns_ચાવી_libcss_ગાંઠ_data,
			libcss_ગાંઠ_data);
	જો (err != DOM_NO_ERR) {
		પાછા CSS_NOMEM;
	}

	પાછા CSS_OK;
}
