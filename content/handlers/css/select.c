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
ಸ್ಥಿರ css_error named_ಪೋಷಕ_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **ಪೋಷಕ);
ಸ್ಥಿರ css_error named_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error named_generic_sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error ಪೋಷಕ_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **ಪೋಷಕ);
ಸ್ಥಿರ css_error sibling_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **sibling);
ಸ್ಥಿರ css_error ಗಂಟು_has_name(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_class(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_id(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *name, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_equal(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_dashmatch(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_includes(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_ಪೂರ್ವಪ್ರತ್ಯಯ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_suffix(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_has_attribute_substring(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_ಬೇರು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_ಎಣಿಕೆ_siblings(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		bool same_name, bool after, int32_t *ಎಣಿಕೆ_2);
ಸ್ಥಿರ css_error ಗಂಟು_is_ಖಾಲಿಗಳು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_link(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_hover(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_ಸಕ್ರಿಯ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_focus(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_enabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_disabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_checked(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_ಗುರಿ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ಗಂಟು_is_lang(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		lwc_string *lang, bool *ಹೊಂದಿಕೆ);
ಸ್ಥಿರ css_error ua_default_for_property(ಶೂನ್ಯ *pw, uint32_t property,
		css_hint *hint);
ಸ್ಥಿರ css_error ಗಣ_libcss_ಗಂಟು_ದತ್ತಾಂಶ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಶೂನ್ಯ *libcss_ಗಂಟು_ದತ್ತಾಂಶ);
ಸ್ಥಿರ css_error get_libcss_ಗಂಟು_ದತ್ತಾಂಶ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಶೂನ್ಯ **libcss_ಗಂಟು_ದತ್ತಾಂಶ);

/**
 * Selection callback table for libcss
 */
ಸ್ಥಿರ css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_ಆವೃತ್ತಿ_1,

	ಗಂಟು_name,
	ಗಂಟು_classes,
	ಗಂಟು_id,
	named_ancestor_ಗಂಟು,
	named_ಪೋಷಕ_ಗಂಟು,
	named_sibling_ಗಂಟು,
	named_generic_sibling_ಗಂಟು,
	ಪೋಷಕ_ಗಂಟು,
	sibling_ಗಂಟು,
	ಗಂಟು_has_name,
	ಗಂಟು_has_class,
	ಗಂಟು_has_id,
	ಗಂಟು_has_attribute,
	ಗಂಟು_has_attribute_equal,
	ಗಂಟು_has_attribute_dashmatch,
	ಗಂಟು_has_attribute_includes,
	ಗಂಟು_has_attribute_ಪೂರ್ವಪ್ರತ್ಯಯ,
	ಗಂಟು_has_attribute_suffix,
	ಗಂಟು_has_attribute_substring,
	ಗಂಟು_is_ಬೇರು,
	ಗಂಟು_ಎಣಿಕೆ_siblings,
	ಗಂಟು_is_ಖಾಲಿಗಳು,
	ಗಂಟು_is_link,
	ಗಂಟು_is_ಭೇಟಿಯಾದ,
	ಗಂಟು_is_hover,
	ಗಂಟು_is_ಸಕ್ರಿಯ,
	ಗಂಟು_is_focus,
	ಗಂಟು_is_enabled,
	ಗಂಟು_is_disabled,
	ಗಂಟು_is_checked,
	ಗಂಟು_is_ಗುರಿ,
	ಗಂಟು_is_lang,
	ಗಂಟು_presentational_hint,
	ua_default_for_property,
	ಗಣ_libcss_ಗಂಟು_ದತ್ತಾಂಶ,
	get_libcss_ಗಂಟು_ದತ್ತಾಂಶ,
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
css_stylesheet *nscss_create_inline_style(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_ಆವೃತ್ತಿ = CSS_STYLESHEET_PARAMS_ಆವೃತ್ತಿ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ಖಾಲಿ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ಖಾಲಿ;
	params.import = ಖಾಲಿ;
	params.import_pw = ಖಾಲಿ;
	params.color = ns_system_ಬಣ್ಣ;
	params.color_pw = ಖಾಲಿ;
	params.font = ಖಾಲಿ;
	params.font_pw = ಖಾಲಿ;

	error = css_stylesheet_create(&params, &sheet);
	ಆದರೆ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	error = css_stylesheet_append_ದತ್ತಾಂಶ(sheet, ದತ್ತಾಂಶ, len);
	ಆದರೆ (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	error = css_stylesheet_ದತ್ತಾಂಶ_ಮುಗಿದ(sheet);
	ಆದರೆ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_dom_user_ದತ್ತಾಂಶ_handler(dom_ಗಂಟು_operation operation,
		dom_string *ಕೀಲಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ರಚನೆ dom_ಗಂಟು *src,
		ರಚನೆ dom_ಗಂಟು *dst)
{
	css_error error;

	ಆದರೆ (dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_ದತ್ತಾಂಶ,
			ಕೀಲಿ) == false || ದತ್ತಾಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ (operation) {
	ಸ್ಥಿತಿ DOM_ಗಂಟು_CLONED:
		error = css_libcss_ಗಂಟು_ದತ್ತಾಂಶ_handler(&selection_handler,
				CSS_ಗಂಟು_CLONED,
				ಖಾಲಿ, src, dst, ದತ್ತಾಂಶ);
		ಆದರೆ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_RENAMED:
		error = css_libcss_ಗಂಟು_ದತ್ತಾಂಶ_handler(&selection_handler,
				CSS_ಗಂಟು_MODIFIED,
				ಖಾಲಿ, src, ಖಾಲಿ, ದತ್ತಾಂಶ);
		ಆದರೆ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_IMPORTED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_ADOPTED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_DELETED:
		error = css_libcss_ಗಂಟು_ದತ್ತಾಂಶ_handler(&selection_handler,
				CSS_ಗಂಟು_DELETED,
				ಖಾಲಿ, src, ಖಾಲಿ, ದತ್ತಾಂಶ);
		ಆದರೆ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
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
		ಸ್ಥಿರಾಂಕ css_media *media,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	ಪೂರ್ಣಾಂಕ pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	ಆದರೆ (error != CSS_OK || styles == ಖಾಲಿ) {
		/* Failed selecting partial style -- bail out */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	ಆದರೆ (ctx->ಪೋಷಕ_style != ಖಾಲಿ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->ಪೋಷಕ_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		ಆದರೆ (error != CSS_OK) {
			css_select_results_destroy(styles);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	ಗಾಗಿ (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_ಎಣಿಕೆ;
			pseudo_element++) {

		ಆದರೆ (pseudo_element == CSS_PSEUDO_ELEMENT_ಮೊದಲ_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_ಮೊದಲ_ಸಾಲು)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ಮುಂದುವರಿ;

		ಆದರೆ (styles->styles[pseudo_element] == ಖಾಲಿ)
			/* There were no rules concerning this pseudo element */
			ಮುಂದುವರಿ;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		ಆದರೆ (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	ಹಿಂತಿರುಗಿ styles;
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
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *ಪೋಷಕ)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	ಆದರೆ (error != CSS_OK) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(ಪೋಷಕ, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	ಆದರೆ (error != CSS_OK) {
		css_computed_style_destroy(composed);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ composed;
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
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	qname->ns = ಖಾಲಿ;

	err = dom_string_intern(name, &qname->name);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	dom_string_unref(name);

	ಹಿಂತಿರುಗಿ CSS_OK;
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

	*classes = ಖಾಲಿ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	ಹಿಂತಿರುಗಿ CSS_OK;
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

	*id = ಖಾಲಿ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	ಆದರೆ (attr != ಖಾಲಿ) {
		err = dom_string_intern(attr, id);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			ಹಿಂತಿರುಗಿ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **ancestor)
{
	dom_element_named_ancestor_ಗಂಟು(ಗಂಟು, qname->name,
			(ರಚನೆ dom_element **)ancestor);
	dom_ಗಂಟು_unref(*ancestor);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error named_ಪೋಷಕ_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **ಪೋಷಕ)
{
	dom_element_named_ಪೋಷಕ_ಗಂಟು(ಗಂಟು, qname->name,
			(ರಚನೆ dom_element **)ಪೋಷಕ);
	dom_ಗಂಟು_unref(*ಪೋಷಕ);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **sibling)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *prev;
	dom_exception err;

	*sibling = ಖಾಲಿ;

	/* Find sibling element */
	err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &n);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_OK;

	ಆಗ (n != ಖಾಲಿ) {
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		ಆದರೆ (type == DOM_ELEMENT_ಗಂಟು)
			ನಿಲ್ಲಿಸು;

		err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &prev);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಆದರೆ (n != ಖಾಲಿ) {
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);

		ಆದರೆ (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **sibling)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *prev;
	dom_exception err;

	*sibling = ಖಾಲಿ;

	err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &n);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_OK;

	ಆಗ (n != ಖಾಲಿ) {
		dom_ಗಂಟು_type type;
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		ಆದರೆ (type == DOM_ELEMENT_ಗಂಟು) {
			err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
			ಆದರೆ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(n);
				ಹಿಂತಿರುಗಿ CSS_OK;
			}

			ಆದರೆ (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ಗಂಟು_unref(n);
				*sibling = n;
				ನಿಲ್ಲಿಸು;
			}
			dom_string_unref(name);
		}

		err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &prev);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಪೋಷಕ_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **ಪೋಷಕ)
{
	dom_element_ಪೋಷಕ_ಗಂಟು(ಗಂಟು, (ರಚನೆ dom_element **)ಪೋಷಕ);
	dom_ಗಂಟು_unref(*ಪೋಷಕ);

	ಹಿಂತಿರುಗಿ CSS_OK;
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

	*sibling = ಖಾಲಿ;

	/* Find sibling element */
	err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &n);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_OK;

	ಆಗ (n != ಖಾಲಿ) {
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &type);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		ಆದರೆ (type == DOM_ELEMENT_ಗಂಟು)
			ನಿಲ್ಲಿಸು;

		err = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(n, &prev);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_OK;
		}

		dom_ಗಂಟು_unref(n);
		n = prev;
	}

	ಆದರೆ (n != ಖಾಲಿ) {
		/** \todo Sort out reference counting */
		dom_ಗಂಟು_unref(n);

		*sibling = n;
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, bool *ಹೊಂದಿಕೆ)
{
	nscss_select_ctx *ctx = pw;
	dom_ಗಂಟು *n = ಗಂಟು;

	ಆದರೆ (lwc_string_isequal(qname->name, ctx->universal, ಹೊಂದಿಕೆ) ==
			lwc_error_ok && *ಹೊಂದಿಕೆ == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(n, &name);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ CSS_OK;

		/* Element names are case insensitive in HTML */
		*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		lwc_string *name, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, ಹೊಂದಿಕೆ);

	ಪ್ರತಿಜ್ಞಾ(err == DOM_NO_ERR);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		lwc_string *name, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *attr;
	dom_exception err;

	*ಹೊಂದಿಕೆ = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_OK;

	ಆದರೆ (attr != ಖಾಲಿ) {
		*ಹೊಂದಿಕೆ = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
			lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, ಹೊಂದಿಕೆ);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);

	ಆದರೆ (vlen == 0) {
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);

	ಆದರೆ (vlen == 0) {
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for dashmatch */
	ಆದರೆ (*ಹೊಂದಿಕೆ == false) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vdata = lwc_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) dom_string_ದತ್ತಾಂಶ(atr_val);
		size_t len = dom_string_byte_ಉದ್ದಳತೆ(atr_val);

		ಆದರೆ (len > vlen && ದತ್ತಾಂಶ[vlen] == '-' &&
		    strncasecmp(ದತ್ತಾಂಶ, vdata, vlen) == 0) {
				*ಹೊಂದಿಕೆ = true;
		}
	}

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ;

	*ಹೊಂದಿಕೆ = false;

	ಆದರೆ (vlen == 0) {
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	ಆರಂಭ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) dom_string_ದತ್ತಾಂಶ(atr_val);
	ಕೊನೆ = ಆರಂಭ + dom_string_byte_ಉದ್ದಳತೆ(atr_val);

	ಗಾಗಿ (p = ಆರಂಭ; p <= ಕೊನೆ; p++) {
		ಆದರೆ (*p == ' ' || *p == '\0') {
			ಆದರೆ ((size_t) (p - ಆರಂಭ) == vlen &&
			    strncasecmp(ಆರಂಭ,
					lwc_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ),
					vlen) == 0) {
				*ಹೊಂದಿಕೆ = true;
				ನಿಲ್ಲಿಸು;
			}

			ಆರಂಭ = p + 1;
		}
	}

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_has_attribute_ಪೂರ್ವಪ್ರತ್ಯಯ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);

	ಆದರೆ (vlen == 0) {
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಆದರೆ (*ಹೊಂದಿಕೆ == false) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) dom_string_ದತ್ತಾಂಶ(atr_val);
		size_t len = dom_string_byte_ಉದ್ದಳತೆ(atr_val);

		ಆದರೆ ((len >= vlen) &&
		    (strncasecmp(ದತ್ತಾಂಶ, lwc_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ), vlen) == 0)) {
			*ಹೊಂದಿಕೆ = true;
		}
	}

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);

	ಆದರೆ (vlen == 0) {
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಆದರೆ (*ಹೊಂದಿಕೆ == false) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) dom_string_ದತ್ತಾಂಶ(atr_val);
		size_t len = dom_string_byte_ಉದ್ದಳತೆ(atr_val);

		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = (ಅಕ್ಷರ *) ದತ್ತಾಂಶ + len - vlen;

		ಆದರೆ ((len >= vlen) &&
		    (strncasecmp(ಆರಂಭ, lwc_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ), vlen) == 0)) {
			*ಹೊಂದಿಕೆ = true;
		}


	}

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ css_qname *qname, lwc_string *ಮೌಲ್ಯ,
		bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ಉದ್ದಳತೆ(ಮೌಲ್ಯ);

	ಆದರೆ (vlen == 0) {
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	err = dom_string_create_interned(
		(ಸ್ಥಿರಾಂಕ uint8_t *) lwc_string_ದತ್ತಾಂಶ(qname->name),
		lwc_string_ಉದ್ದಳತೆ(qname->name), &name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ಆದರೆ ((err != DOM_NO_ERR) || (atr_val == ಖಾಲಿ)) {
		dom_string_unref(name);
		*ಹೊಂದಿಕೆ = false;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ಹೊಂದಿಕೆ = dom_string_caseless_lwc_isequal(atr_val, ಮೌಲ್ಯ);

	/* check for prefix match */
	ಆದರೆ (*ಹೊಂದಿಕೆ == false) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vdata = lwc_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) dom_string_ದತ್ತಾಂಶ(atr_val);
		size_t len = dom_string_byte_ಉದ್ದಳತೆ(atr_val);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆಯ_ಆರಂಭ = ಆರಂಭ + len - vlen;

		ಆದರೆ (len >= vlen) {
			ಆಗ (ಆರಂಭ <= ಕೊನೆಯ_ಆರಂಭ) {
				ಆದರೆ (strncasecmp(ಆರಂಭ, vdata,
						vlen) == 0) {
					*ಹೊಂದಿಕೆ = true;
					ನಿಲ್ಲಿಸು;
				}

				ಆರಂಭ++;
			}
		}
	}

	dom_string_unref(atr_val);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_ಬೇರು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_ಗಂಟು *ಪೋಷಕ;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(n, &ಪೋಷಕ);
	ಆದರೆ (err != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ಆದರೆ (ಪೋಷಕ != ಖಾಲಿ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(ಪೋಷಕ, &type);

		dom_ಗಂಟು_unref(ಪೋಷಕ);

		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ CSS_NOMEM;

		ಆದರೆ (type != DOM_DOCUMENT_ಗಂಟು) {
			*ಹೊಂದಿಕೆ = false;
			ಹಿಂತಿರುಗಿ CSS_OK;
		}
	}

	*ಹೊಂದಿಕೆ = true;

	ಹಿಂತಿರುಗಿ CSS_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಂಟು_ಎಣಿಕೆ_siblings_check(dom_ಗಂಟು *ಗಂಟು,
			  bool check_name,
			  dom_string *name)
{
	dom_ಗಂಟು_type type;
	ಪೂರ್ಣಾಂಕ ret = 0;
	dom_exception exc;

	ಆದರೆ (ಗಂಟು == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
	ಆದರೆ ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ಗಂಟು)) {
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (check_name) {
		dom_string *ಗಂಟು_name = ಖಾಲಿ;
		exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);

		ಆದರೆ ((exc == DOM_NO_ERR) && (ಗಂಟು_name != ಖಾಲಿ)) {

			ಆದರೆ (dom_string_caseless_isequal(name,
							ಗಂಟು_name)) {
				ret = 1;
			}
			dom_string_unref(ಗಂಟು_name);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = 1;
	}

	ಹಿಂತಿರುಗಿ ret;
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
		bool after, int32_t *ಎಣಿಕೆ_2)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ಗಂಟು_name = ಖಾಲಿ;

	ಆದರೆ (same_name) {
		dom_ಗಂಟು *ಗಂಟು = n;
		exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);
		ಆದರೆ ((exc != DOM_NO_ERR) || (ಗಂಟು_name == ಖಾಲಿ)) {
			ಹಿಂತಿರುಗಿ CSS_NOMEM;
		}
	}

	ಆದರೆ (after) {
		dom_ಗಂಟು *ಗಂಟು = dom_ಗಂಟು_ref(n);
		dom_ಗಂಟು *ಮುಂದಿನ;

		ಮಾಡು {
			exc = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(ಗಂಟು, &ಮುಂದಿನ);
			ಆದರೆ ((exc != DOM_NO_ERR))
				ನಿಲ್ಲಿಸು;

			dom_ಗಂಟು_unref(ಗಂಟು);
			ಗಂಟು = ಮುಂದಿನ;

			cnt += ಗಂಟು_ಎಣಿಕೆ_siblings_check(ಗಂಟು, same_name, ಗಂಟು_name);
		} ಆಗ (ಗಂಟು != ಖಾಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		dom_ಗಂಟು *ಗಂಟು = dom_ಗಂಟು_ref(n);
		dom_ಗಂಟು *ಮುಂದಿನ;

		ಮಾಡು {
			exc = dom_ಗಂಟು_get_ಹಿಂದಿನ_sibling(ಗಂಟು, &ಮುಂದಿನ);
			ಆದರೆ ((exc != DOM_NO_ERR))
				ನಿಲ್ಲಿಸು;

			dom_ಗಂಟು_unref(ಗಂಟು);
			ಗಂಟು = ಮುಂದಿನ;

			cnt += ಗಂಟು_ಎಣಿಕೆ_siblings_check(ಗಂಟು, same_name, ಗಂಟು_name);

		} ಆಗ (ಗಂಟು != ಖಾಲಿ);
	}

	ಆದರೆ (ಗಂಟು_name != ಖಾಲಿ) {
		dom_string_unref(ಗಂಟು_name);
	}

	*ಎಣಿಕೆ_2 = cnt;
	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_ಖಾಲಿಗಳು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *n = ಗಂಟು, *ಮುಂದಿನ;
	dom_exception err;

	*ಹೊಂದಿಕೆ = true;

	err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(n, &n);
	ಆದರೆ (err != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ CSS_BADPARM;
	}

	ಆಗ (n != ಖಾಲಿ) {
		dom_ಗಂಟು_type ntype;
		err = dom_ಗಂಟು_get_ಗಂಟು_type(n, &ntype);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_BADPARM;
		}

		ಆದರೆ (ntype == DOM_ELEMENT_ಗಂಟು ||
		    ntype == DOM_ಪಠ್ಯ_ಗಂಟು) {
			*ಹೊಂದಿಕೆ = false;
			dom_ಗಂಟು_unref(n);
			ನಿಲ್ಲಿಸು;
		}

		err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(n, &ಮುಂದಿನ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(n);
			ಹಿಂತಿರುಗಿ CSS_BADPARM;
		}
		dom_ಗಂಟು_unref(n);
		n = ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_link(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *n, bool *ಹೊಂದಿಕೆ)
{
	dom_ಗಂಟು *ಗಂಟು = n;
	dom_exception exc;
	dom_string *ಗಂಟು_name = ಖಾಲಿ;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &ಗಂಟು_name);
	ಆದರೆ ((exc != DOM_NO_ERR) || (ಗಂಟು_name == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ಆದರೆ (dom_string_caseless_lwc_isequal(ಗಂಟು_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ಗಂಟು, corestring_dom_href,
				&has_href);
		ಆದರೆ ((exc == DOM_NO_ERR) && (has_href)) {
			*ಹೊಂದಿಕೆ = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ಹೊಂದಿಕೆ = false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಹೊಂದಿಕೆ = false;
	}
	dom_string_unref(ಗಂಟು_name);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_ಭೇಟಿಯಾದ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;

	dom_exception exc;
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_string *s = ಖಾಲಿ;

	*ಹೊಂದಿಕೆ = false;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(n, &s);
	ಆದರೆ ((exc != DOM_NO_ERR) || (s == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ಆದರೆ (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ಖಾಲಿ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	ಆದರೆ ((exc != DOM_NO_ERR) || (s == ಖಾಲಿ)) {
		/* Can't be visited; not got a URL */
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->ತಳ_url, dom_string_ದತ್ತಾಂಶ(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	ಆದರೆ (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ದತ್ತಾಂಶ = urldb_get_url_ದತ್ತಾಂಶ(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	ಆದರೆ (ದತ್ತಾಂಶ != ಖಾಲಿ && ದತ್ತಾಂಶ->visits > 0)
		*ಹೊಂದಿಕೆ = true;

	nsurl_unref(url);

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_hover(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support hovering */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_ಸಕ್ರಿಯ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support active nodes */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_focus(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support focussed nodes */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_enabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support enabled nodes */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_disabled(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support disabled nodes */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_checked(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support checked nodes */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
css_error ಗಂಟು_is_ಗುರಿ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support target */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		lwc_string *lang, bool *ಹೊಂದಿಕೆ)
{
	/** \todo Support languages */

	*ಹೊಂದಿಕೆ = false;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
	ಆದರೆ (property == CSS_PROP_COLOR) {
		hint->ದತ್ತಾಂಶ.color = 0xff000000;
		hint->ಸ್ಥಿತಿ_2 = CSS_COLOR_COLOR;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (property == CSS_PROP_FONT_FAMILY) {
		hint->ದತ್ತಾಂಶ.ಅಕ್ಷರಸರಣಿಗಳು = ಖಾಲಿ;
		ಆಯ್ಕೆ (nsoption_int(font_default)) {
		ಸ್ಥಿತಿ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->ಸ್ಥಿತಿ_2 = CSS_FONT_FAMILY_SANS_SERIF;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ PLOT_FONT_FAMILY_SERIF:
			hint->ಸ್ಥಿತಿ_2 = CSS_FONT_FAMILY_SERIF;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ PLOT_FONT_FAMILY_MONOSPACE:
			hint->ಸ್ಥಿತಿ_2 = CSS_FONT_FAMILY_MONOSPACE;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ PLOT_FONT_FAMILY_CURSIVE:
			hint->ಸ್ಥಿತಿ_2 = CSS_FONT_FAMILY_CURSIVE;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ PLOT_FONT_FAMILY_FANTASY:
			hint->ಸ್ಥಿತಿ_2 = CSS_FONT_FAMILY_FANTASY;
			ನಿಲ್ಲಿಸು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->ದತ್ತಾಂಶ.ಅಕ್ಷರಸರಣಿಗಳು = ಖಾಲಿ;
		hint->ಸ್ಥಿತಿ_2 = CSS_QUOTES_NONE;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->ದತ್ತಾಂಶ.ಅಕ್ಷರಸರಣಿಗಳು = ಖಾಲಿ;
		hint->ಸ್ಥಿತಿ_2 = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ CSS_INVALID;
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
}

css_error ಗಣ_libcss_ಗಂಟು_ದತ್ತಾಂಶ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ *libcss_ಗಂಟು_ದತ್ತಾಂಶ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;
	ಶೂನ್ಯ *old_ಗಂಟು_ದತ್ತಾಂಶ;

	/* Set this node's node data */
	err = dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(n,
			corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_ದತ್ತಾಂಶ,
			libcss_ಗಂಟು_ದತ್ತಾಂಶ, nscss_dom_user_ದತ್ತಾಂಶ_handler,
			(ಶೂನ್ಯ *) &old_ಗಂಟು_ದತ್ತಾಂಶ);
	ಆದರೆ (err != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ಪ್ರತಿಜ್ಞಾ(old_ಗಂಟು_ದತ್ತಾಂಶ == ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ CSS_OK;
}

css_error get_libcss_ಗಂಟು_ದತ್ತಾಂಶ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, ಶೂನ್ಯ **libcss_ಗಂಟು_ದತ್ತಾಂಶ)
{
	dom_ಗಂಟು *n = ಗಂಟು;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ(n,
			corestring_dom_ns_ಕೀಲಿ_libcss_ಗಂಟು_ದತ್ತಾಂಶ,
			libcss_ಗಂಟು_ದತ್ತಾಂಶ);
	ಆದರೆ (err != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
}
