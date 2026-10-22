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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plot_style.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/url_db.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/system_colour.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "css/internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/hints.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/select.h"

ਸਥਿਰ css_error ਗੰਢ_name(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, css_qname *qname);
ਸਥਿਰ css_error ਗੰਢ_classes(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string ***classes, uint32_t *n_classes);
ਸਥਿਰ css_error ਗੰਢ_id(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, lwc_string **id);
ਸਥਿਰ css_error named_ਮਾਪਾ_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **ਮਾਪਾ);
ਸਥਿਰ css_error named_sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **sibling);
ਸਥਿਰ css_error named_generic_sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **sibling);
ਸਥਿਰ css_error ਮਾਪਾ_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ **ਮਾਪਾ);
ਸਥਿਰ css_error sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ **sibling);
ਸਥਿਰ css_error ਗੰਢ_has_name(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_class(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *name, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_id(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *name, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_equal(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_dashmatch(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_includes(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_ਅਗੇਤਰ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_suffix(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_has_attribute_substring(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_ਜੜ੍ਹ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_ਗਿਣਤੀ_siblings(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		bool same_name, bool after, int32_t *ਗਿਣਤੀ_2);
ਸਥਿਰ css_error ਗੰਢ_is_ਖਾਲੀ_ਥਾਵਾਂ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_link(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_hover(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_ਸਰਗਰਮ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_focus(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_enabled(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_disabled(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_checked(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_ਨਿਸ਼ਾਨਾ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);
ਸਥਿਰ css_error ਗੰਢ_is_lang(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *lang, bool *ਮੇਲ);
ਸਥਿਰ css_error ua_default_for_property(ਖਾਲੀ *pw, uint32_t property,
		css_hint *hint);
ਸਥਿਰ css_error ਸੈੱਟ_libcss_ਗੰਢ_ਅੰਕੜੇ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਖਾਲੀ *libcss_ਗੰਢ_ਅੰਕੜੇ);
ਸਥਿਰ css_error get_libcss_ਗੰਢ_ਅੰਕੜੇ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਖਾਲੀ **libcss_ਗੰਢ_ਅੰਕੜੇ);

/**
 * Selection callback table for libcss
 */
ਸਥਿਰ css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_ਸੰਸਕਰਣ_1,

	ਗੰਢ_name,
	ਗੰਢ_classes,
	ਗੰਢ_id,
	named_ancestor_ਗੰਢ,
	named_ਮਾਪਾ_ਗੰਢ,
	named_sibling_ਗੰਢ,
	named_generic_sibling_ਗੰਢ,
	ਮਾਪਾ_ਗੰਢ,
	sibling_ਗੰਢ,
	ਗੰਢ_has_name,
	ਗੰਢ_has_class,
	ਗੰਢ_has_id,
	ਗੰਢ_has_attribute,
	ਗੰਢ_has_attribute_equal,
	ਗੰਢ_has_attribute_dashmatch,
	ਗੰਢ_has_attribute_includes,
	ਗੰਢ_has_attribute_ਅਗੇਤਰ,
	ਗੰਢ_has_attribute_suffix,
	ਗੰਢ_has_attribute_substring,
	ਗੰਢ_is_ਜੜ੍ਹ,
	ਗੰਢ_ਗਿਣਤੀ_siblings,
	ਗੰਢ_is_ਖਾਲੀ_ਥਾਵਾਂ,
	ਗੰਢ_is_link,
	ਗੰਢ_is_ਦੇਖਿਆ,
	ਗੰਢ_is_hover,
	ਗੰਢ_is_ਸਰਗਰਮ,
	ਗੰਢ_is_focus,
	ਗੰਢ_is_enabled,
	ਗੰਢ_is_disabled,
	ਗੰਢ_is_checked,
	ਗੰਢ_is_ਨਿਸ਼ਾਨਾ,
	ਗੰਢ_is_lang,
	ਗੰਢ_presentational_hint,
	ua_default_for_property,
	ਸੈੱਟ_libcss_ਗੰਢ_ਅੰਕੜੇ,
	get_libcss_ਗੰਢ_ਅੰਕੜੇ,
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
css_stylesheet *nscss_create_inline_style(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		ਅਚਲ ਅੱਖਰ *charset, ਅਚਲ ਅੱਖਰ *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_ਸੰਸਕਰਣ = CSS_STYLESHEET_PARAMS_ਸੰਸਕਰਣ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ਨਿਰਾ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ਨਿਰਾ;
	params.import = ਨਿਰਾ;
	params.import_pw = ਨਿਰਾ;
	params.color = ns_system_ਰੰਗ;
	params.color_pw = ਨਿਰਾ;
	params.font = ਨਿਰਾ;
	params.font_pw = ਨਿਰਾ;

	error = css_stylesheet_create(&params, &sheet);
	ਜੇ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		ਵਾਪਸ ਨਿਰਾ;
	}

	error = css_stylesheet_append_ਅੰਕੜੇ(sheet, ਅੰਕੜੇ, len);
	ਜੇ (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		ਵਾਪਸ ਨਿਰਾ;
	}

	error = css_stylesheet_ਅੰਕੜੇ_ਪੂਰਾ(sheet);
	ਜੇ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ਸਥਿਰ ਖਾਲੀ nscss_dom_user_ਅੰਕੜੇ_handler(dom_ਗੰਢ_operation operation,
		dom_string *ਕੁੰਜੀ, ਖਾਲੀ *ਅੰਕੜੇ, ਬਣਤਰ dom_ਗੰਢ *src,
		ਬਣਤਰ dom_ਗੰਢ *dst)
{
	css_error error;

	ਜੇ (dom_string_isequal(corestring_dom_ns_ਕੁੰਜੀ_libcss_ਗੰਢ_ਅੰਕੜੇ,
			ਕੁੰਜੀ) == false || ਅੰਕੜੇ == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ਚੋਣ (operation) {
	ਹਾਲਤ DOM_ਗੰਢ_CLONED:
		error = css_libcss_ਗੰਢ_ਅੰਕੜੇ_handler(&selection_handler,
				CSS_ਗੰਢ_CLONED,
				ਨਿਰਾ, src, dst, ਅੰਕੜੇ);
		ਜੇ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_RENAMED:
		error = css_libcss_ਗੰਢ_ਅੰਕੜੇ_handler(&selection_handler,
				CSS_ਗੰਢ_MODIFIED,
				ਨਿਰਾ, src, ਨਿਰਾ, ਅੰਕੜੇ);
		ਜੇ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_IMPORTED:
	ਹਾਲਤ DOM_ਗੰਢ_ADOPTED:
	ਹਾਲਤ DOM_ਗੰਢ_DELETED:
		error = css_libcss_ਗੰਢ_ਅੰਕੜੇ_handler(&selection_handler,
				CSS_ਗੰਢ_DELETED,
				ਨਿਰਾ, src, ਨਿਰਾ, ਅੰਕੜੇ);
		ਜੇ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ਰੋਕੋ;

	ਮੂਲ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ਗੰਢ *n,
		ਅਚਲ css_media *media,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	ਪੂਰਨ pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	ਜੇ (error != CSS_OK || styles == ਨਿਰਾ) {
		/* Failed selecting partial style -- bail out */
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	ਜੇ (ctx->ਮਾਪਾ_style != ਨਿਰਾ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->ਮਾਪਾ_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		ਜੇ (error != CSS_OK) {
			css_select_results_destroy(styles);
			ਵਾਪਸ ਨਿਰਾ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	ਲਈ (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_ਗਿਣਤੀ;
			pseudo_element++) {

		ਜੇ (pseudo_element == CSS_PSEUDO_ELEMENT_ਪਹਿਲਾ_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_ਪਹਿਲਾ_ਸਤਰ)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ਜਾਰੀ;

		ਜੇ (styles->styles[pseudo_element] == ਨਿਰਾ)
			/* There were no rules concerning this pseudo element */
			ਜਾਰੀ;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		ਜੇ (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			ਵਾਪਸ ਨਿਰਾ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	ਵਾਪਸ styles;
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
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *ਮਾਪਾ)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	ਜੇ (error != CSS_OK) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(ਮਾਪਾ, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	ਜੇ (error != CSS_OK) {
		css_computed_style_destroy(composed);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ composed;
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
css_error ਗੰਢ_name(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, css_qname *qname)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਗੰਢ_name(n, &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	qname->ns = ਨਿਰਾ;

	err = dom_string_intern(name, &qname->name);
	ਜੇ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ਵਾਪਸ CSS_NOMEM;
	}

	dom_string_unref(name);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_classes(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_exception err;

	*classes = ਨਿਰਾ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_id(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, lwc_string **id)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *attr;
	dom_exception err;

	*id = ਨਿਰਾ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	ਜੇ (attr != ਨਿਰਾ) {
		err = dom_string_intern(attr, id);
		ਜੇ (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			ਵਾਪਸ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	ਵਾਪਸ CSS_OK;
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
css_error named_ancestor_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **ancestor)
{
	dom_element_named_ancestor_ਗੰਢ(ਗੰਢ, qname->name,
			(ਬਣਤਰ dom_element **)ancestor);
	dom_ਗੰਢ_unref(*ancestor);

	ਵਾਪਸ CSS_OK;
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
css_error named_ਮਾਪਾ_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **ਮਾਪਾ)
{
	dom_element_named_ਮਾਪਾ_ਗੰਢ(ਗੰਢ, qname->name,
			(ਬਣਤਰ dom_element **)ਮਾਪਾ);
	dom_ਗੰਢ_unref(*ਮਾਪਾ);

	ਵਾਪਸ CSS_OK;
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
css_error named_sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **sibling)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_ਗੰਢ *prev;
	dom_exception err;

	*sibling = ਨਿਰਾ;

	/* Find sibling element */
	err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &n);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_OK;

	ਜਦੋਂ (n != ਨਿਰਾ) {
		dom_ਗੰਢ_type type;

		err = dom_ਗੰਢ_get_ਗੰਢ_type(n, &type);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		ਜੇ (type == DOM_ELEMENT_ਗੰਢ)
			ਰੋਕੋ;

		err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &prev);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		dom_ਗੰਢ_unref(n);
		n = prev;
	}

	ਜੇ (n != ਨਿਰਾ) {
		dom_string *name;

		err = dom_ਗੰਢ_get_ਗੰਢ_name(n, &name);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		dom_ਗੰਢ_unref(n);

		ਜੇ (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	ਵਾਪਸ CSS_OK;
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
css_error named_generic_sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **sibling)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_ਗੰਢ *prev;
	dom_exception err;

	*sibling = ਨਿਰਾ;

	err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &n);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_OK;

	ਜਦੋਂ (n != ਨਿਰਾ) {
		dom_ਗੰਢ_type type;
		dom_string *name;

		err = dom_ਗੰਢ_get_ਗੰਢ_type(n, &type);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		ਜੇ (type == DOM_ELEMENT_ਗੰਢ) {
			err = dom_ਗੰਢ_get_ਗੰਢ_name(n, &name);
			ਜੇ (err != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(n);
				ਵਾਪਸ CSS_OK;
			}

			ਜੇ (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ਗੰਢ_unref(n);
				*sibling = n;
				ਰੋਕੋ;
			}
			dom_string_unref(name);
		}

		err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &prev);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		dom_ਗੰਢ_unref(n);
		n = prev;
	}

	ਵਾਪਸ CSS_OK;
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
css_error ਮਾਪਾ_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ **ਮਾਪਾ)
{
	dom_element_ਮਾਪਾ_ਗੰਢ(ਗੰਢ, (ਬਣਤਰ dom_element **)ਮਾਪਾ);
	dom_ਗੰਢ_unref(*ਮਾਪਾ);

	ਵਾਪਸ CSS_OK;
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
css_error sibling_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ **sibling)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_ਗੰਢ *prev;
	dom_exception err;

	*sibling = ਨਿਰਾ;

	/* Find sibling element */
	err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &n);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_OK;

	ਜਦੋਂ (n != ਨਿਰਾ) {
		dom_ਗੰਢ_type type;

		err = dom_ਗੰਢ_get_ਗੰਢ_type(n, &type);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		ਜੇ (type == DOM_ELEMENT_ਗੰਢ)
			ਰੋਕੋ;

		err = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(n, &prev);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_OK;
		}

		dom_ਗੰਢ_unref(n);
		n = prev;
	}

	ਜੇ (n != ਨਿਰਾ) {
		/** \todo Sort out reference counting */
		dom_ਗੰਢ_unref(n);

		*sibling = n;
	}

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_name(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, bool *ਮੇਲ)
{
	nscss_select_ctx *ctx = pw;
	dom_ਗੰਢ *n = ਗੰਢ;

	ਜੇ (lwc_string_isequal(qname->name, ctx->universal, ਮੇਲ) ==
			lwc_error_ok && *ਮੇਲ == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ਗੰਢ_get_ਗੰਢ_name(n, &name);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ CSS_OK;

		/* Element names are case insensitive in HTML */
		*ਮੇਲ = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_class(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *name, bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, ਮੇਲ);

	ਪ੍ਰਤਿਜ੍ਞਾ(err == DOM_NO_ERR);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_id(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *name, bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *attr;
	dom_exception err;

	*ਮੇਲ = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_OK;

	ਜੇ (attr != ਨਿਰਾ) {
		*ਮੇਲ = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
			lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, ਮੇਲ);
	ਜੇ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_equal(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);

	ਜੇ (vlen == 0) {
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	*ਮੇਲ = dom_string_caseless_lwc_isequal(atr_val, ਮੁੱਲ);

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_dashmatch(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);

	ਜੇ (vlen == 0) {
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ਮੇਲ = dom_string_caseless_lwc_isequal(atr_val, ਮੁੱਲ);

	/* check for dashmatch */
	ਜੇ (*ਮੇਲ == false) {
		ਅਚਲ ਅੱਖਰ *vdata = lwc_string_ਅੰਕੜੇ(ਮੁੱਲ);
		ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ = (ਅਚਲ ਅੱਖਰ *) dom_string_ਅੰਕੜੇ(atr_val);
		size_t len = dom_string_byte_ਲੰਮਾਈ(atr_val);

		ਜੇ (len > vlen && ਅੰਕੜੇ[vlen] == '-' &&
		    strncasecmp(ਅੰਕੜੇ, vdata, vlen) == 0) {
				*ਮੇਲ = true;
		}
	}

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_includes(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);
	ਅਚਲ ਅੱਖਰ *p;
	ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ;
	ਅਚਲ ਅੱਖਰ *ਸਿਰਾ;

	*ਮੇਲ = false;

	ਜੇ (vlen == 0) {
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	ਸ਼ੁਰੂ = (ਅਚਲ ਅੱਖਰ *) dom_string_ਅੰਕੜੇ(atr_val);
	ਸਿਰਾ = ਸ਼ੁਰੂ + dom_string_byte_ਲੰਮਾਈ(atr_val);

	ਲਈ (p = ਸ਼ੁਰੂ; p <= ਸਿਰਾ; p++) {
		ਜੇ (*p == ' ' || *p == '\0') {
			ਜੇ ((size_t) (p - ਸ਼ੁਰੂ) == vlen &&
			    strncasecmp(ਸ਼ੁਰੂ,
					lwc_string_ਅੰਕੜੇ(ਮੁੱਲ),
					vlen) == 0) {
				*ਮੇਲ = true;
				ਰੋਕੋ;
			}

			ਸ਼ੁਰੂ = p + 1;
		}
	}

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_ਅਗੇਤਰ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);

	ਜੇ (vlen == 0) {
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ਮੇਲ = dom_string_caseless_lwc_isequal(atr_val, ਮੁੱਲ);

	/* check for prefix match */
	ਜੇ (*ਮੇਲ == false) {
		ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ = (ਅਚਲ ਅੱਖਰ *) dom_string_ਅੰਕੜੇ(atr_val);
		size_t len = dom_string_byte_ਲੰਮਾਈ(atr_val);

		ਜੇ ((len >= vlen) &&
		    (strncasecmp(ਅੰਕੜੇ, lwc_string_ਅੰਕੜੇ(ਮੁੱਲ), vlen) == 0)) {
			*ਮੇਲ = true;
		}
	}

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_suffix(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);

	ਜੇ (vlen == 0) {
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ਮੇਲ = dom_string_caseless_lwc_isequal(atr_val, ਮੁੱਲ);

	/* check for prefix match */
	ਜੇ (*ਮੇਲ == false) {
		ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ = (ਅਚਲ ਅੱਖਰ *) dom_string_ਅੰਕੜੇ(atr_val);
		size_t len = dom_string_byte_ਲੰਮਾਈ(atr_val);

		ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = (ਅੱਖਰ *) ਅੰਕੜੇ + len - vlen;

		ਜੇ ((len >= vlen) &&
		    (strncasecmp(ਸ਼ੁਰੂ, lwc_string_ਅੰਕੜੇ(ਮੁੱਲ), vlen) == 0)) {
			*ਮੇਲ = true;
		}


	}

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_has_attribute_substring(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, lwc_string *ਮੁੱਲ,
		bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_ਲੰਮਾਈ(ਮੁੱਲ);

	ਜੇ (vlen == 0) {
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	err = dom_string_create_interned(
		(ਅਚਲ uint8_t *) lwc_string_ਅੰਕੜੇ(qname->name),
		lwc_string_ਲੰਮਾਈ(qname->name), &name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ਜੇ ((err != DOM_NO_ERR) || (atr_val == ਨਿਰਾ)) {
		dom_string_unref(name);
		*ਮੇਲ = false;
		ਵਾਪਸ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*ਮੇਲ = dom_string_caseless_lwc_isequal(atr_val, ਮੁੱਲ);

	/* check for prefix match */
	ਜੇ (*ਮੇਲ == false) {
		ਅਚਲ ਅੱਖਰ *vdata = lwc_string_ਅੰਕੜੇ(ਮੁੱਲ);
		ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = (ਅਚਲ ਅੱਖਰ *) dom_string_ਅੰਕੜੇ(atr_val);
		size_t len = dom_string_byte_ਲੰਮਾਈ(atr_val);
		ਅਚਲ ਅੱਖਰ *ਆਖਰੀ_ਸ਼ੁਰੂ = ਸ਼ੁਰੂ + len - vlen;

		ਜੇ (len >= vlen) {
			ਜਦੋਂ (ਸ਼ੁਰੂ <= ਆਖਰੀ_ਸ਼ੁਰੂ) {
				ਜੇ (strncasecmp(ਸ਼ੁਰੂ, vdata,
						vlen) == 0) {
					*ਮੇਲ = true;
					ਰੋਕੋ;
				}

				ਸ਼ੁਰੂ++;
			}
		}
	}

	dom_string_unref(atr_val);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_ਜੜ੍ਹ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_ਗੰਢ *ਮਾਪਾ;
	dom_ਗੰਢ_type type;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(n, &ਮਾਪਾ);
	ਜੇ (err != DOM_NO_ERR) {
		ਵਾਪਸ CSS_NOMEM;
	}

	ਜੇ (ਮਾਪਾ != ਨਿਰਾ) {
		err = dom_ਗੰਢ_get_ਗੰਢ_type(ਮਾਪਾ, &type);

		dom_ਗੰਢ_unref(ਮਾਪਾ);

		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ CSS_NOMEM;

		ਜੇ (type != DOM_DOCUMENT_ਗੰਢ) {
			*ਮੇਲ = false;
			ਵਾਪਸ CSS_OK;
		}
	}

	*ਮੇਲ = true;

	ਵਾਪਸ CSS_OK;
}

ਸਥਿਰ ਪੂਰਨ
ਗੰਢ_ਗਿਣਤੀ_siblings_check(dom_ਗੰਢ *ਗੰਢ,
			  bool check_name,
			  dom_string *name)
{
	dom_ਗੰਢ_type type;
	ਪੂਰਨ ret = 0;
	dom_exception exc;

	ਜੇ (ਗੰਢ == ਨਿਰਾ)
		ਵਾਪਸ 0;

	exc = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &type);
	ਜੇ ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ਗੰਢ)) {
		ਵਾਪਸ 0;
	}

	ਜੇ (check_name) {
		dom_string *ਗੰਢ_name = ਨਿਰਾ;
		exc = dom_ਗੰਢ_get_ਗੰਢ_name(ਗੰਢ, &ਗੰਢ_name);

		ਜੇ ((exc == DOM_NO_ERR) && (ਗੰਢ_name != ਨਿਰਾ)) {

			ਜੇ (dom_string_caseless_isequal(name,
							ਗੰਢ_name)) {
				ret = 1;
			}
			dom_string_unref(ਗੰਢ_name);
		}
	} ਨਹੀਂਤਾਂ {
		ret = 1;
	}

	ਵਾਪਸ ret;
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
css_error ਗੰਢ_ਗਿਣਤੀ_siblings(ਖਾਲੀ *pw, ਖਾਲੀ *n, bool same_name,
		bool after, int32_t *ਗਿਣਤੀ_2)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ਗੰਢ_name = ਨਿਰਾ;

	ਜੇ (same_name) {
		dom_ਗੰਢ *ਗੰਢ = n;
		exc = dom_ਗੰਢ_get_ਗੰਢ_name(ਗੰਢ, &ਗੰਢ_name);
		ਜੇ ((exc != DOM_NO_ERR) || (ਗੰਢ_name == ਨਿਰਾ)) {
			ਵਾਪਸ CSS_NOMEM;
		}
	}

	ਜੇ (after) {
		dom_ਗੰਢ *ਗੰਢ = dom_ਗੰਢ_ref(n);
		dom_ਗੰਢ *ਅਗਲਾ;

		ਕਰੋ {
			exc = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(ਗੰਢ, &ਅਗਲਾ);
			ਜੇ ((exc != DOM_NO_ERR))
				ਰੋਕੋ;

			dom_ਗੰਢ_unref(ਗੰਢ);
			ਗੰਢ = ਅਗਲਾ;

			cnt += ਗੰਢ_ਗਿਣਤੀ_siblings_check(ਗੰਢ, same_name, ਗੰਢ_name);
		} ਜਦੋਂ (ਗੰਢ != ਨਿਰਾ);
	} ਨਹੀਂਤਾਂ {
		dom_ਗੰਢ *ਗੰਢ = dom_ਗੰਢ_ref(n);
		dom_ਗੰਢ *ਅਗਲਾ;

		ਕਰੋ {
			exc = dom_ਗੰਢ_get_ਪਿਛਲਾ_sibling(ਗੰਢ, &ਅਗਲਾ);
			ਜੇ ((exc != DOM_NO_ERR))
				ਰੋਕੋ;

			dom_ਗੰਢ_unref(ਗੰਢ);
			ਗੰਢ = ਅਗਲਾ;

			cnt += ਗੰਢ_ਗਿਣਤੀ_siblings_check(ਗੰਢ, same_name, ਗੰਢ_name);

		} ਜਦੋਂ (ਗੰਢ != ਨਿਰਾ);
	}

	ਜੇ (ਗੰਢ_name != ਨਿਰਾ) {
		dom_string_unref(ਗੰਢ_name);
	}

	*ਗਿਣਤੀ_2 = cnt;
	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_ਖਾਲੀ_ਥਾਵਾਂ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	dom_ਗੰਢ *n = ਗੰਢ, *ਅਗਲਾ;
	dom_exception err;

	*ਮੇਲ = true;

	err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(n, &n);
	ਜੇ (err != DOM_NO_ERR) {
		ਵਾਪਸ CSS_BADPARM;
	}

	ਜਦੋਂ (n != ਨਿਰਾ) {
		dom_ਗੰਢ_type ntype;
		err = dom_ਗੰਢ_get_ਗੰਢ_type(n, &ntype);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_BADPARM;
		}

		ਜੇ (ntype == DOM_ELEMENT_ਗੰਢ ||
		    ntype == DOM_ਪਾਠ_ਗੰਢ) {
			*ਮੇਲ = false;
			dom_ਗੰਢ_unref(n);
			ਰੋਕੋ;
		}

		err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(n, &ਅਗਲਾ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(n);
			ਵਾਪਸ CSS_BADPARM;
		}
		dom_ਗੰਢ_unref(n);
		n = ਅਗਲਾ;
	}

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_link(ਖਾਲੀ *pw, ਖਾਲੀ *n, bool *ਮੇਲ)
{
	dom_ਗੰਢ *ਗੰਢ = n;
	dom_exception exc;
	dom_string *ਗੰਢ_name = ਨਿਰਾ;

	exc = dom_ਗੰਢ_get_ਗੰਢ_name(ਗੰਢ, &ਗੰਢ_name);
	ਜੇ ((exc != DOM_NO_ERR) || (ਗੰਢ_name == ਨਿਰਾ)) {
		ਵਾਪਸ CSS_NOMEM;
	}

	ਜੇ (dom_string_caseless_lwc_isequal(ਗੰਢ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ਗੰਢ, corestring_dom_href,
				&has_href);
		ਜੇ ((exc == DOM_NO_ERR) && (has_href)) {
			*ਮੇਲ = true;
		} ਨਹੀਂਤਾਂ {
			*ਮੇਲ = false;
		}
	} ਨਹੀਂਤਾਂ {
		*ਮੇਲ = false;
	}
	dom_string_unref(ਗੰਢ_name);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_ਦੇਖਿਆ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ;

	dom_exception exc;
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_string *s = ਨਿਰਾ;

	*ਮੇਲ = false;

	exc = dom_ਗੰਢ_get_ਗੰਢ_name(n, &s);
	ਜੇ ((exc != DOM_NO_ERR) || (s == ਨਿਰਾ)) {
		ਵਾਪਸ CSS_NOMEM;
	}

	ਜੇ (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		ਵਾਪਸ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ਨਿਰਾ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	ਜੇ ((exc != DOM_NO_ERR) || (s == ਨਿਰਾ)) {
		/* Can't be visited; not got a URL */
		ਵਾਪਸ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->ਅਧਾਰ_url, dom_string_ਅੰਕੜੇ(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	ਜੇ (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		ਵਾਪਸ CSS_NOMEM;
	}

	ਅੰਕੜੇ = urldb_get_url_ਅੰਕੜੇ(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	ਜੇ (ਅੰਕੜੇ != ਨਿਰਾ && ਅੰਕੜੇ->visits > 0)
		*ਮੇਲ = true;

	nsurl_unref(url);

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_hover(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support hovering */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_ਸਰਗਰਮ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support active nodes */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_focus(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support focussed nodes */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_enabled(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support enabled nodes */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_disabled(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support disabled nodes */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_checked(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support checked nodes */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_ਨਿਸ਼ਾਨਾ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ)
{
	/** \todo Support target */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ਗੰਢ_is_lang(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		lwc_string *lang, bool *ਮੇਲ)
{
	/** \todo Support languages */

	*ਮੇਲ = false;

	ਵਾਪਸ CSS_OK;
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
css_error ua_default_for_property(ਖਾਲੀ *pw, uint32_t property, css_hint *hint)
{
	ਜੇ (property == CSS_PROP_COLOR) {
		hint->ਅੰਕੜੇ.color = 0xff000000;
		hint->ਸਥਿਤੀ = CSS_COLOR_COLOR;
	} ਨਹੀਂਤਾਂ ਜੇ (property == CSS_PROP_FONT_FAMILY) {
		hint->ਅੰਕੜੇ.ਲੜੀਆਂ = ਨਿਰਾ;
		ਚੋਣ (nsoption_int(font_default)) {
		ਹਾਲਤ PLOT_FONT_FAMILY_SANS_SERIF:
			hint->ਸਥਿਤੀ = CSS_FONT_FAMILY_SANS_SERIF;
			ਰੋਕੋ;
		ਹਾਲਤ PLOT_FONT_FAMILY_SERIF:
			hint->ਸਥਿਤੀ = CSS_FONT_FAMILY_SERIF;
			ਰੋਕੋ;
		ਹਾਲਤ PLOT_FONT_FAMILY_MONOSPACE:
			hint->ਸਥਿਤੀ = CSS_FONT_FAMILY_MONOSPACE;
			ਰੋਕੋ;
		ਹਾਲਤ PLOT_FONT_FAMILY_CURSIVE:
			hint->ਸਥਿਤੀ = CSS_FONT_FAMILY_CURSIVE;
			ਰੋਕੋ;
		ਹਾਲਤ PLOT_FONT_FAMILY_FANTASY:
			hint->ਸਥਿਤੀ = CSS_FONT_FAMILY_FANTASY;
			ਰੋਕੋ;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->ਅੰਕੜੇ.ਲੜੀਆਂ = ਨਿਰਾ;
		hint->ਸਥਿਤੀ = CSS_QUOTES_NONE;
	} ਨਹੀਂਤਾਂ ਜੇ (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->ਅੰਕੜੇ.ਲੜੀਆਂ = ਨਿਰਾ;
		hint->ਸਥਿਤੀ = 0;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ CSS_INVALID;
	}

	ਵਾਪਸ CSS_OK;
}

css_error ਸੈੱਟ_libcss_ਗੰਢ_ਅੰਕੜੇ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ *libcss_ਗੰਢ_ਅੰਕੜੇ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_exception err;
	ਖਾਲੀ *old_ਗੰਢ_ਅੰਕੜੇ;

	/* Set this node's node data */
	err = dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(n,
			corestring_dom_ns_ਕੁੰਜੀ_libcss_ਗੰਢ_ਅੰਕੜੇ,
			libcss_ਗੰਢ_ਅੰਕੜੇ, nscss_dom_user_ਅੰਕੜੇ_handler,
			(ਖਾਲੀ *) &old_ਗੰਢ_ਅੰਕੜੇ);
	ਜੇ (err != DOM_NO_ERR) {
		ਵਾਪਸ CSS_NOMEM;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(old_ਗੰਢ_ਅੰਕੜੇ == ਨਿਰਾ);

	ਵਾਪਸ CSS_OK;
}

css_error get_libcss_ਗੰਢ_ਅੰਕੜੇ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, ਖਾਲੀ **libcss_ਗੰਢ_ਅੰਕੜੇ)
{
	dom_ਗੰਢ *n = ਗੰਢ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ਗੰਢ_get_user_ਅੰਕੜੇ(n,
			corestring_dom_ns_ਕੁੰਜੀ_libcss_ਗੰਢ_ਅੰਕੜੇ,
			libcss_ਗੰਢ_ਅੰਕੜੇ);
	ਜੇ (err != DOM_NO_ERR) {
		ਵਾਪਸ CSS_NOMEM;
	}

	ਵਾਪਸ CSS_OK;
}
