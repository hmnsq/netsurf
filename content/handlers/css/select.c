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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"
#ಅನ್ತರ್ಭಾವ "netsurf/url_db.h"
#ಅನ್ತರ್ಭಾವ "desktop/system_colour.h"

#ಅನ್ತರ್ಭಾವ "css/internal.h"
#ಅನ್ತರ್ಭಾವ "css/hints.h"
#ಅನ್ತರ್ಭಾವ "css/select.h"

ಸ್ಥಿರ css_error ಗಂಟು_name(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, css_qname *qname);
ಸ್ಥಿರ css_error ಗಂಟು_classes(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string ***classes, uint32_t *n_classes);
ಸ್ಥಿರ css_error ಗಂಟು_id(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, lwc_string **id);
ಸ್ಥಿರ css_error named_parent_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **parent);
ಸ್ಥಿರ css_error named_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error named_generic_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error parent_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **parent);
ಸ್ಥಿರ css_error sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error ಗಂಟು_has_name(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_class(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_id(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_equal(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_dashmatch(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_includes(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_prefix(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_suffix(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_substring(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_ಬೇರು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_ಎಣಿಕೆ_siblings(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		bool same_name, bool after, int32_t *ಎಣಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_empty(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_link(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_hover(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_active(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_focus(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_enabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_disabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_checked(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_target(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);
ಸ್ಥಿರ css_error ಗಂಟು_is_lang(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *lang, bool *match);
ಸ್ಥಿರ css_error ua_default_for_property(ಶೂನ್ಯ *pw, uint32_t property,
		css_hint *hint);
ಸ್ಥಿರ css_error set_libcss_ಗಂಟು_data(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಶೂನ್ಯ *libcss_ಗಂಟು_data);
ಸ್ಥಿರ css_error get_libcss_ಗಂಟು_data(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಶೂನ್ಯ **libcss_ಗಂಟು_data);

/**
 * Selection callback table for libcss
 */
ಸ್ಥಿರ css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	ಗಂಟು_name,
	ಗಂಟು_classes,
	ಗಂಟು_id,
	named_ancestor_ಗಂಟು,
	named_parent_ಗಂಟು,
	named_sibling_ಗಂಟು,
	named_generic_sibling_ಗಂಟು,
	parent_ಗಂಟು,
	sibling_ಗಂಟು,
	ಗಂಟು_has_name,
	ಗಂಟು_has_class,
	ಗಂಟು_has_id,
	ಗಂಟು_has_attribute,
	ಗಂಟು_has_attribute_equal,
	ಗಂಟು_has_attribute_dashmatch,
	ಗಂಟು_has_attribute_includes,
	ಗಂಟು_has_attribute_prefix,
	ಗಂಟು_has_attribute_suffix,
	ಗಂಟು_has_attribute_substring,
	ಗಂಟು_is_ಬೇರು,
	ಗಂಟು_ಎಣಿಕೆ_siblings,
	ಗಂಟು_is_empty,
	ಗಂಟು_is_link,
	ಗಂಟು_is_visited,
	ಗಂಟು_is_hover,
	ಗಂಟು_is_active,
	ಗಂಟು_is_focus,
	ಗಂಟು_is_enabled,
	ಗಂಟು_is_disabled,
	ಗಂಟು_is_checked,
	ಗಂಟು_is_target,
	ಗಂಟು_is_lang,
	ಗಂಟು_presentational_hint,
	ua_default_for_property,
	set_libcss_ಗಂಟು_data,
	get_libcss_ಗಂಟು_data,
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
css_stylesheet *nscss_create_inline_style(ಧ್ರುವ uint8_t *data, size_t len,
		ಧ್ರುವ ವರ್ಣ *charset, ಧ್ರುವ ವರ್ಣ *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ರಿಕ್ತ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ರಿಕ್ತ;
	params.import = ರಿಕ್ತ;
	params.import_pw = ರಿಕ್ತ;
	params.color = ns_system_colour;
	params.color_pw = ರಿಕ್ತ;
	params.font = ರಿಕ್ತ;
	params.font_pw = ರಿಕ್ತ;

	error = css_stylesheet_create(&params, &sheet);
	ಯದಿ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	ಯದಿ (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	error = css_stylesheet_data_done(sheet);
	ಯದಿ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_dom_user_data_handler(dom_ಗಂಟು_operation operation,
		dom_string *ಕೀಲಿ, ಶೂನ್ಯ *data, ಸಂರಚನಾ dom_ಗಂಟು *src,
		ಸಂರಚನಾ dom_ಗಂಟು *dst)
{
	css_error error;

	ಯದಿ (dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_data,
			ಕೀಲಿ) == false || data == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ (operation) {
	ಪಕ್ಷ DOM_ಗಂಟು_CLONED:
		error = css_libcss_ಗಂಟು_data_handler(&selection_handler,
				CSS_ಗಂಟು_CLONED,
				ರಿಕ್ತ, src, dst, data);
		ಯದಿ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_RENAMED:
		error = css_libcss_ಗಂಟು_data_handler(&selection_handler,
				CSS_ಗಂಟು_MODIFIED,
				ರಿಕ್ತ, src, ರಿಕ್ತ, data);
		ಯದಿ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_IMPORTED:
	ಪಕ್ಷ DOM_ಗಂಟು_ADOPTED:
	ಪಕ್ಷ DOM_ಗಂಟು_DELETED:
		error = css_libcss_ಗಂಟು_data_handler(&selection_handler,
				CSS_ಗಂಟು_DELETED,
				ರಿಕ್ತ, src, ರಿಕ್ತ, data);
		ಯದಿ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ಗಂಟು *n,
		ಧ್ರುವ css_media *media,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	ಪೂರ್ಣಾಙ್ಕ pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	ಯದಿ (error != CSS_OK || styles == ರಿಕ್ತ) {
		/* Failed selecting partial style -- bail out */
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	ಯದಿ (ctx->parent_style != ರಿಕ್ತ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		ಯದಿ (error != CSS_OK) {
			css_select_results_destroy(styles);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	ಪ್ರತಿ (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_ಎಣಿಕೆ;
			pseudo_element++) {

		ಯದಿ (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_ಸಾಲು)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ಅನುವರ್ತ;

		ಯದಿ (styles->styles[pseudo_element] == ರಿಕ್ತ)
			/* There were no rules concerning this pseudo element */
			ಅನುವರ್ತ;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		ಯದಿ (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	ಪ್ರತ್ಯಾಗಮ styles;
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
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	ಯದಿ (error != CSS_OK) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	ಯದಿ (error != CSS_OK) {
		css_computed_style_destroy(composed);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ composed;
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
css_error ಗಂಟು_name(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, css_qname *qname)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	qname->ns = ರಿಕ್ತ;

	err = dom_string_intern(name, &qname->name);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	dom_string_unref(name);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_classes(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;

	*classes = ರಿಕ್ತ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_id(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, lwc_string **id)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *attr;
	dom_exception err;

	*id = ರಿಕ್ತ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	ಯದಿ (attr != ರಿಕ್ತ) {
		err = dom_string_intern(attr, id);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error named_ancestor_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **ancestor)
{
	dom_element_named_ancestor_ಗಂಟು(ಗಂಟು, qname->name,
			(ಸಂರಚನಾ dom_element **)ancestor);
	dom_ಗಂಟು_unref(*ancestor);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error named_parent_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **parent)
{
	dom_element_named_parent_ಗಂಟು(ಗಂಟು, qname->name,
			(ಸಂರಚನಾ dom_element **)parent);
	dom_ಗಂಟು_unref(*parent);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error named_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **sibling)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *prev;
	dom_exception err;

	*sibling = ರಿಕ್ತ;

	/* Find sibling element */
	err = dom_ಗಂಟು_get_previous_sibling(n, &n);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_OK;

	ಯಾವತ್ (n != ರಿಕ್ತ) {
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		ಯದಿ (type == DOM_ELEMENT_ಗಂಟು)
			ಭಙ್ಗ;

		err = dom_ಗಂಟು_get_previous_sibling(n, &prev);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಯದಿ (n != ರಿಕ್ತ) {
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);

		ಯದಿ (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error named_generic_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **sibling)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *prev;
	dom_exception err;

	*sibling = ರಿಕ್ತ;

	err = dom_ಗಂಟು_get_previous_sibling(n, &n);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_OK;

	ಯಾವತ್ (n != ರಿಕ್ತ) {
		dom_ಗಂಟು_type type;
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		ಯದಿ (type == DOM_ELEMENT_ಗಂಟು) {
			err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
			ಯದಿ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(n);
				ಪ್ರತ್ಯಾಗಮ CSS_OK;
			}

			ಯದಿ (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ಗಂಟು_unref(n);
				*sibling = n;
				ಭಙ್ಗ;
			}
			dom_string_unref(name);
		}

		err = dom_ಗಂಟು_get_previous_sibling(n, &prev);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error parent_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **parent)
{
	dom_element_parent_ಗಂಟು(ಗಂಟು, (ಸಂರಚನಾ dom_element **)parent);
	dom_ಗಂಟು_unref(*parent);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **sibling)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *prev;
	dom_exception err;

	*sibling = ರಿಕ್ತ;

	/* Find sibling element */
	err = dom_ಗಂಟು_get_previous_sibling(n, &n);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_OK;

	ಯಾವತ್ (n != ರಿಕ್ತ) {
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		ಯದಿ (type == DOM_ELEMENT_ಗಂಟು)
			ಭಙ್ಗ;

		err = dom_ಗಂಟು_get_previous_sibling(n, &prev);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಯದಿ (n != ರಿಕ್ತ) {
		/** \todo Sort out reference counting */
		dom_ಗಂಟು_unref(n);

		*sibling = n;
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_name(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_ಗಂಟು *n = ಗಂಟು;

	ಯದಿ (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_class(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	ಪ್ರತಿಜ್ಞಾ(err == DOM_NO_ERR);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_id(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_OK;

	ಯದಿ (attr != ರಿಕ್ತ) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_equal(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);

	ಯದಿ (vlen == 0) {
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_dashmatch(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);

	ಯದಿ (vlen == 0) {
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for dashmatch */
	ಯದಿ (*match == false) {
		ಧ್ರುವ ವರ್ಣ *vdata = lwc_string_data(ಮೌಲ್ಯ);
		ಧ್ರುವ ವರ್ಣ *data = (ಧ್ರುವ ವರ್ಣ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ಯದಿ (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_includes(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);
	ಧ್ರುವ ವರ್ಣ *p;
	ಧ್ರುವ ವರ್ಣ *start;
	ಧ್ರುವ ವರ್ಣ *end;

	*match = false;

	ಯದಿ (vlen == 0) {
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ಧ್ರುವ ವರ್ಣ *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	ಪ್ರತಿ (p = start; p <= end; p++) {
		ಯದಿ (*p == ' ' || *p == '\0') {
			ಯದಿ ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(ಮೌಲ್ಯ),
					vlen) == 0) {
				*match = true;
				ಭಙ್ಗ;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_prefix(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);

	ಯದಿ (vlen == 0) {
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಯದಿ (*match == false) {
		ಧ್ರುವ ವರ್ಣ *data = (ಧ್ರುವ ವರ್ಣ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ಯದಿ ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(ಮೌಲ್ಯ), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_suffix(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);

	ಯದಿ (vlen == 0) {
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಯದಿ (*match == false) {
		ಧ್ರುವ ವರ್ಣ *data = (ಧ್ರುವ ವರ್ಣ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ಧ್ರುವ ವರ್ಣ *start = (ವರ್ಣ *) data + len - vlen;

		ಯದಿ ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(ಮೌಲ್ಯ), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_has_attribute_substring(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ಮೌಲ್ಯ);

	ಯದಿ (vlen == 0) {
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಧ್ರುವ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಯದಿ ((err != DOM_NO_ERR) || (atr_val == ರಿಕ್ತ)) {
		dom_string_unref(name);
		*match = false;
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಯದಿ (*match == false) {
		ಧ್ರುವ ವರ್ಣ *vdata = lwc_string_data(ಮೌಲ್ಯ);
		ಧ್ರುವ ವರ್ಣ *start = (ಧ್ರುವ ವರ್ಣ *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ಧ್ರುವ ವರ್ಣ *last_start = start + len - vlen;

		ಯದಿ (len >= vlen) {
			ಯಾವತ್ (start <= last_start) {
				ಯದಿ (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					ಭಙ್ಗ;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_ಬೇರು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *parent;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_parent_ಗಂಟು(n, &parent);
	ಯದಿ (err != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	ಯದಿ (parent != ರಿಕ್ತ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(parent, &type);

		dom_ಗಂಟು_unref(parent);

		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

		ಯದಿ (type != DOM_DOCUMENT_ಗಂಟು) {
			*match = false;
			ಪ್ರತ್ಯಾಗಮ CSS_OK;
		}
	}

	*match = true;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ಗಂಟು_ಎಣಿಕೆ_siblings_check(dom_ಗಂಟು *ಗಂಟು,
			  bool check_name,
			  dom_string *name)
{
	dom_ಗಂಟು_type type;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	dom_exception exc;

	ಯದಿ (ಗಂಟು == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
	ಯದಿ ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ಗಂಟು)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (check_name) {
		dom_string *ಗಂಟು_name = ರಿಕ್ತ;
		exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);

		ಯದಿ ((exc == DOM_NO_ERR) && (ಗಂಟು_name != ರಿಕ್ತ)) {

			ಯದಿ (dom_string_caseless_isequal(name,
							ಗಂಟು_name)) {
				ret = 1;
			}
			dom_string_unref(ಗಂಟು_name);
		}
	} ಅನ್ಯಥಾ {
		ret = 1;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
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
css_error ಗಂಟು_ಎಣಿಕೆ_siblings(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *n, bool same_name,
		bool after, int32_t *ಎಣಿಕೆ)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ಗಂಟು_name = ರಿಕ್ತ;

	ಯದಿ (same_name) {
		dom_ಗಂಟು *ಗಂಟು = n;
		exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);
		ಯದಿ ((exc != DOM_NO_ERR) || (ಗಂಟು_name == ರಿಕ್ತ)) {
			ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
		}
	}

	ಯದಿ (after) {
		dom_ಗಂಟು *ಗಂಟು = dom_ಗಂಟು_ref(n);
		dom_ಗಂಟು *next;

		ಕುರು {
			exc = dom_ಗಂಟು_get_next_sibling(ಗಂಟು, &next);
			ಯದಿ ((exc != DOM_NO_ERR))
				ಭಙ್ಗ;

			dom_ಗಂಟು_unref(ಗಂಟು);
			ಗಂಟು = next;

			cnt += ಗಂಟು_ಎಣಿಕೆ_siblings_check(ಗಂಟು, same_name, ಗಂಟು_name);
		} ಯಾವತ್ (ಗಂಟು != ರಿಕ್ತ);
	} ಅನ್ಯಥಾ {
		dom_ಗಂಟು *ಗಂಟು = dom_ಗಂಟು_ref(n);
		dom_ಗಂಟು *next;

		ಕುರು {
			exc = dom_ಗಂಟು_get_previous_sibling(ಗಂಟು, &next);
			ಯದಿ ((exc != DOM_NO_ERR))
				ಭಙ್ಗ;

			dom_ಗಂಟು_unref(ಗಂಟು);
			ಗಂಟು = next;

			cnt += ಗಂಟು_ಎಣಿಕೆ_siblings_check(ಗಂಟು, same_name, ಗಂಟು_name);

		} ಯಾವತ್ (ಗಂಟು != ರಿಕ್ತ);
	}

	ಯದಿ (ಗಂಟು_name != ರಿಕ್ತ) {
		dom_string_unref(ಗಂಟು_name);
	}

	*ಎಣಿಕೆ = cnt;
	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_empty(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	dom_ಗಂಟು *n = ಗಂಟು, *next;
	dom_exception err;

	*match = true;

	err = dom_ಗಂಟು_get_first_child(n, &n);
	ಯದಿ (err != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ CSS_BADPARM;
	}

	ಯಾವತ್ (n != ರಿಕ್ತ) {
		dom_ಗಂಟು_type ntype;
		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &ntype);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_BADPARM;
		}

		ಯದಿ (ntype == DOM_ELEMENT_ಗಂಟು ||
		    ntype == DOM_TEXT_ಗಂಟು) {
			*match = false;
			dom_ಗಂಟು_unref(n);
			ಭಙ್ಗ;
		}

		err = dom_ಗಂಟು_get_next_sibling(n, &next);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಪ್ರತ್ಯಾಗಮ CSS_BADPARM;
		}
		dom_ಗಂಟು_unref(n);
		n = next;
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_link(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *n, bool *match)
{
	dom_ಗಂಟು *ಗಂಟು = n;
	dom_exception exc;
	dom_string *ಗಂಟು_name = ರಿಕ್ತ;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);
	ಯದಿ ((exc != DOM_NO_ERR) || (ಗಂಟು_name == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	ಯದಿ (dom_string_caseless_lwc_isequal(ಗಂಟು_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ಗಂಟು, corestring_dom_href,
				&has_href);
		ಯದಿ ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} ಅನ್ಯಥಾ {
			*match = false;
		}
	} ಅನ್ಯಥಾ {
		*match = false;
	}
	dom_string_unref(ಗಂಟು_name);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_visited(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ಧ್ರುವ ಸಂರಚನಾ url_data *data;

	dom_exception exc;
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *s = ರಿಕ್ತ;

	*match = false;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(n, &s);
	ಯದಿ ((exc != DOM_NO_ERR) || (s == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	ಯದಿ (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ರಿಕ್ತ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	ಯದಿ ((exc != DOM_NO_ERR) || (s == ರಿಕ್ತ)) {
		/* Can't be visited; not got a URL */
		ಪ್ರತ್ಯಾಗಮ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	ಯದಿ (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	ಯದಿ (data != ರಿಕ್ತ && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_hover(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_active(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_focus(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_enabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_disabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_checked(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_target(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match)
{
	/** \todo Support target */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ಗಂಟು_is_lang(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
css_error ua_default_for_property(ಶೂನ್ಯ *pw, uint32_t property, css_hint *hint)
{
	ಯದಿ (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} ಅನ್ಯಥಾ ಯದಿ (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = ರಿಕ್ತ;
		ವಿಕಲ್ಪ (nsoption_int(font_default)) {
		ಪಕ್ಷ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			ಭಙ್ಗ;
		ಪಕ್ಷ PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			ಭಙ್ಗ;
		ಪಕ್ಷ PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			ಭಙ್ಗ;
		ಪಕ್ಷ PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			ಭಙ್ಗ;
		ಪಕ್ಷ PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			ಭಙ್ಗ;
		}
	} ಅನ್ಯಥಾ ಯದಿ (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = ರಿಕ್ತ;
		hint->status = CSS_QUOTES_NONE;
	} ಅನ್ಯಥಾ ಯದಿ (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = ರಿಕ್ತ;
		hint->status = 0;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ CSS_INVALID;
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
}

css_error set_libcss_ಗಂಟು_data(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ *libcss_ಗಂಟು_data)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;
	ಶೂನ್ಯ *old_ಗಂಟು_data;

	/* Set this node's node data */
	err = dom_ಗಂಟು_set_user_data(n,
			corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_data,
			libcss_ಗಂಟು_data, nscss_dom_user_data_handler,
			(ಶೂನ್ಯ *) &old_ಗಂಟು_data);
	ಯದಿ (err != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	ಪ್ರತಿಜ್ಞಾ(old_ಗಂಟು_data == ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
}

css_error get_libcss_ಗಂಟು_data(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **libcss_ಗಂಟು_data)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ಗಂಟು_get_user_data(n,
			corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_data,
			libcss_ಗಂಟು_data);
	ಯದಿ (err != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
}
