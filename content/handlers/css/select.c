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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plot_style.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/url_db.h"
#ଅନ୍ତର୍ଭାଵ "desktop/system_colour.h"

#ଅନ୍ତର୍ଭାଵ "css/internal.h"
#ଅନ୍ତର୍ଭାଵ "css/hints.h"
#ଅନ୍ତର୍ଭାଵ "css/select.h"

ସ୍ଥିର css_error ଗଣ୍ଠି_name(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, css_qname *qname);
ସ୍ଥିର css_error ଗଣ୍ଠି_classes(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string ***classes, uint32_t *n_classes);
ସ୍ଥିର css_error ଗଣ୍ଠି_id(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, lwc_string **id);
ସ୍ଥିର css_error named_parent_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **parent);
ସ୍ଥିର css_error named_sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **sibling);
ସ୍ଥିର css_error named_generic_sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **sibling);
ସ୍ଥିର css_error parent_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ **parent);
ସ୍ଥିର css_error sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ **sibling);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_name(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_class(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *name, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_id(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *name, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_equal(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_dashmatch(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_includes(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_prefix(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_suffix(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_has_attribute_substring(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_ମୂଳ(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_ଗଣନା_siblings(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		bool same_name, bool after, int32_t *ଗଣନା_2);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_empty(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_link(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_hover(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_active(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_focus(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_enabled(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_disabled(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_checked(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_target(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);
ସ୍ଥିର css_error ଗଣ୍ଠି_is_lang(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *lang, bool *match);
ସ୍ଥିର css_error ua_default_for_property(ଶୂନ୍ୟ *pw, uint32_t property,
		css_hint *hint);
ସ୍ଥିର css_error set_libcss_ଗଣ୍ଠି_data(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଶୂନ୍ୟ *libcss_ଗଣ୍ଠି_data);
ସ୍ଥିର css_error get_libcss_ଗଣ୍ଠି_data(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଶୂନ୍ୟ **libcss_ଗଣ୍ଠି_data);

/**
 * Selection callback table for libcss
 */
ସ୍ଥିର css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	ଗଣ୍ଠି_name,
	ଗଣ୍ଠି_classes,
	ଗଣ୍ଠି_id,
	named_ancestor_ଗଣ୍ଠି,
	named_parent_ଗଣ୍ଠି,
	named_sibling_ଗଣ୍ଠି,
	named_generic_sibling_ଗଣ୍ଠି,
	parent_ଗଣ୍ଠି,
	sibling_ଗଣ୍ଠି,
	ଗଣ୍ଠି_has_name,
	ଗଣ୍ଠି_has_class,
	ଗଣ୍ଠି_has_id,
	ଗଣ୍ଠି_has_attribute,
	ଗଣ୍ଠି_has_attribute_equal,
	ଗଣ୍ଠି_has_attribute_dashmatch,
	ଗଣ୍ଠି_has_attribute_includes,
	ଗଣ୍ଠି_has_attribute_prefix,
	ଗଣ୍ଠି_has_attribute_suffix,
	ଗଣ୍ଠି_has_attribute_substring,
	ଗଣ୍ଠି_is_ମୂଳ,
	ଗଣ୍ଠି_ଗଣନା_siblings,
	ଗଣ୍ଠି_is_empty,
	ଗଣ୍ଠି_is_link,
	ଗଣ୍ଠି_is_visited,
	ଗଣ୍ଠି_is_hover,
	ଗଣ୍ଠି_is_active,
	ଗଣ୍ଠି_is_focus,
	ଗଣ୍ଠି_is_enabled,
	ଗଣ୍ଠି_is_disabled,
	ଗଣ୍ଠି_is_checked,
	ଗଣ୍ଠି_is_target,
	ଗଣ୍ଠି_is_lang,
	ଗଣ୍ଠି_presentational_hint,
	ua_default_for_property,
	set_libcss_ଗଣ୍ଠି_data,
	get_libcss_ଗଣ୍ଠି_data,
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
css_stylesheet *nscss_create_inline_style(ଅଚଳ uint8_t *data, size_t len,
		ଅଚଳ ଅକ୍ଷର *charset, ଅଚଳ ଅକ୍ଷର *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ଖାଲି;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ଖାଲି;
	params.import = ଖାଲି;
	params.import_pw = ଖାଲି;
	params.color = ns_system_colour;
	params.color_pw = ଖାଲି;
	params.font = ଖାଲି;
	params.font_pw = ଖାଲି;

	error = css_stylesheet_create(&params, &sheet);
	ଯଦି (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	ଯଦି (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	error = css_stylesheet_data_done(sheet);
	ଯଦି (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ସ୍ଥିର ଶୂନ୍ୟ nscss_dom_user_data_handler(dom_ଗଣ୍ଠି_operation operation,
		dom_string *ଚାବି, ଶୂନ୍ୟ *data, ସଂରଚନା dom_ଗଣ୍ଠି *src,
		ସଂରଚନା dom_ଗଣ୍ଠି *dst)
{
	css_error error;

	ଯଦି (dom_string_isequal(corestring_dom_ns_ଚାବି_libcss_ଗଣ୍ଠି_data,
			ଚାବି) == false || data == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ବାଛ (operation) {
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_CLONED:
		error = css_libcss_ଗଣ୍ଠି_data_handler(&selection_handler,
				CSS_ଗଣ୍ଠି_CLONED,
				ଖାଲି, src, dst, data);
		ଯଦି (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_RENAMED:
		error = css_libcss_ଗଣ୍ଠି_data_handler(&selection_handler,
				CSS_ଗଣ୍ଠି_MODIFIED,
				ଖାଲି, src, ଖାଲି, data);
		ଯଦି (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_IMPORTED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_ADOPTED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_DELETED:
		error = css_libcss_ଗଣ୍ଠି_data_handler(&selection_handler,
				CSS_ଗଣ୍ଠି_DELETED,
				ଖାଲି, src, ଖାଲି, data);
		ଯଦି (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ପ୍ରତିଜ୍ଞା(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ଗଣ୍ଠି *n,
		ଅଚଳ css_media *media,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	ଯଦି (error != CSS_OK || styles == ଖାଲି) {
		/* Failed selecting partial style -- bail out */
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	ଯଦି (ctx->parent_style != ଖାଲି) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		ଯଦି (error != CSS_OK) {
			css_select_results_destroy(styles);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	ପାଇଁ (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_ଗଣନା;
			pseudo_element++) {

		ଯଦି (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_ଧାଡ଼ି)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ଜାରି;

		ଯଦି (styles->styles[pseudo_element] == ଖାଲି)
			/* There were no rules concerning this pseudo element */
			ଜାରି;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		ଯଦି (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	ଫେରନ୍ତୁ styles;
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
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	ଯଦି (error != CSS_OK) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	ଯଦି (error != CSS_OK) {
		css_computed_style_destroy(composed);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ composed;
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
css_error ଗଣ୍ଠି_name(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, css_qname *qname)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(n, &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	qname->ns = ଖାଲି;

	err = dom_string_intern(name, &qname->name);
	ଯଦି (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	dom_string_unref(name);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_classes(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_exception err;

	*classes = ଖାଲି;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_id(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, lwc_string **id)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *attr;
	dom_exception err;

	*id = ଖାଲି;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	ଯଦି (attr != ଖାଲି) {
		err = dom_string_intern(attr, id);
		ଯଦି (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			ଫେରନ୍ତୁ CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error named_ancestor_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **ancestor)
{
	dom_element_named_ancestor_ଗଣ୍ଠି(ଗଣ୍ଠି, qname->name,
			(ସଂରଚନା dom_element **)ancestor);
	dom_ଗଣ୍ଠି_unref(*ancestor);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error named_parent_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **parent)
{
	dom_element_named_parent_ଗଣ୍ଠି(ଗଣ୍ଠି, qname->name,
			(ସଂରଚନା dom_element **)parent);
	dom_ଗଣ୍ଠି_unref(*parent);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error named_sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **sibling)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି *prev;
	dom_exception err;

	*sibling = ଖାଲି;

	/* Find sibling element */
	err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &n);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_OK;

	ଯେତେବେଳେ (n != ଖାଲି) {
		dom_ଗଣ୍ଠି_type type;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(n, &type);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		ଯଦି (type == DOM_ELEMENT_ଗଣ୍ଠି)
			ଭାଙ୍ଗ;

		err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &prev);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		dom_ଗଣ୍ଠି_unref(n);
		n = prev;
	}

	ଯଦି (n != ଖାଲି) {
		dom_string *name;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(n, &name);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		dom_ଗଣ୍ଠି_unref(n);

		ଯଦି (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error named_generic_sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **sibling)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି *prev;
	dom_exception err;

	*sibling = ଖାଲି;

	err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &n);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_OK;

	ଯେତେବେଳେ (n != ଖାଲି) {
		dom_ଗଣ୍ଠି_type type;
		dom_string *name;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(n, &type);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		ଯଦି (type == DOM_ELEMENT_ଗଣ୍ଠି) {
			err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(n, &name);
			ଯଦି (err != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(n);
				ଫେରନ୍ତୁ CSS_OK;
			}

			ଯଦି (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ଗଣ୍ଠି_unref(n);
				*sibling = n;
				ଭାଙ୍ଗ;
			}
			dom_string_unref(name);
		}

		err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &prev);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		dom_ଗଣ୍ଠି_unref(n);
		n = prev;
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error parent_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ **parent)
{
	dom_element_parent_ଗଣ୍ଠି(ଗଣ୍ଠି, (ସଂରଚନା dom_element **)parent);
	dom_ଗଣ୍ଠି_unref(*parent);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error sibling_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ **sibling)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି *prev;
	dom_exception err;

	*sibling = ଖାଲି;

	/* Find sibling element */
	err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &n);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_OK;

	ଯେତେବେଳେ (n != ଖାଲି) {
		dom_ଗଣ୍ଠି_type type;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(n, &type);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		ଯଦି (type == DOM_ELEMENT_ଗଣ୍ଠି)
			ଭାଙ୍ଗ;

		err = dom_ଗଣ୍ଠି_get_previous_sibling(n, &prev);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_OK;
		}

		dom_ଗଣ୍ଠି_unref(n);
		n = prev;
	}

	ଯଦି (n != ଖାଲି) {
		/** \todo Sort out reference counting */
		dom_ଗଣ୍ଠି_unref(n);

		*sibling = n;
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_name(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;

	ଯଦି (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(n, &name);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_class(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *name, bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	ପ୍ରତିଜ୍ଞା(err == DOM_NO_ERR);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_id(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *name, bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_OK;

	ଯଦି (attr != ଖାଲି) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	ଯଦି (err != DOM_NO_ERR) {
		dom_string_unref(name);
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_equal(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);

	ଯଦି (vlen == 0) {
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, ମୂଲ୍ୟ);

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_dashmatch(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);

	ଯଦି (vlen == 0) {
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ମୂଲ୍ୟ);

	/* check for dashmatch */
	ଯଦି (*match == false) {
		ଅଚଳ ଅକ୍ଷର *vdata = lwc_string_data(ମୂଲ୍ୟ);
		ଅଚଳ ଅକ୍ଷର *data = (ଅଚଳ ଅକ୍ଷର *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ଯଦି (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_includes(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);
	ଅଚଳ ଅକ୍ଷର *p;
	ଅଚଳ ଅକ୍ଷର *start;
	ଅଚଳ ଅକ୍ଷର *end;

	*match = false;

	ଯଦି (vlen == 0) {
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ଅଚଳ ଅକ୍ଷର *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	ପାଇଁ (p = start; p <= end; p++) {
		ଯଦି (*p == ' ' || *p == '\0') {
			ଯଦି ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(ମୂଲ୍ୟ),
					vlen) == 0) {
				*match = true;
				ଭାଙ୍ଗ;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_prefix(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);

	ଯଦି (vlen == 0) {
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ମୂଲ୍ୟ);

	/* check for prefix match */
	ଯଦି (*match == false) {
		ଅଚଳ ଅକ୍ଷର *data = (ଅଚଳ ଅକ୍ଷର *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ଯଦି ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(ମୂଲ୍ୟ), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_suffix(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);

	ଯଦି (vlen == 0) {
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ମୂଲ୍ୟ);

	/* check for prefix match */
	ଯଦି (*match == false) {
		ଅଚଳ ଅକ୍ଷର *data = (ଅଚଳ ଅକ୍ଷର *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ଅଚଳ ଅକ୍ଷର *start = (ଅକ୍ଷର *) data + len - vlen;

		ଯଦି ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(ମୂଲ୍ୟ), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_has_attribute_substring(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, lwc_string *ମୂଲ୍ୟ,
		bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(ମୂଲ୍ୟ);

	ଯଦି (vlen == 0) {
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	err = dom_string_create_interned(
		(ଅଚଳ uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ଯଦି ((err != DOM_NO_ERR) || (atr_val == ଖାଲି)) {
		dom_string_unref(name);
		*match = false;
		ଫେରନ୍ତୁ CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, ମୂଲ୍ୟ);

	/* check for prefix match */
	ଯଦି (*match == false) {
		ଅଚଳ ଅକ୍ଷର *vdata = lwc_string_data(ମୂଲ୍ୟ);
		ଅଚଳ ଅକ୍ଷର *start = (ଅଚଳ ଅକ୍ଷର *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ଅଚଳ ଅକ୍ଷର *last_start = start + len - vlen;

		ଯଦି (len >= vlen) {
			ଯେତେବେଳେ (start <= last_start) {
				ଯଦି (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					ଭାଙ୍ଗ;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_ମୂଳ(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି *parent;
	dom_ଗଣ୍ଠି_type type;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(n, &parent);
	ଯଦି (err != DOM_NO_ERR) {
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	ଯଦି (parent != ଖାଲି) {
		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(parent, &type);

		dom_ଗଣ୍ଠି_unref(parent);

		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ CSS_NOMEM;

		ଯଦି (type != DOM_DOCUMENT_ଗଣ୍ଠି) {
			*match = false;
			ଫେରନ୍ତୁ CSS_OK;
		}
	}

	*match = true;

	ଫେରନ୍ତୁ CSS_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ଗଣ୍ଠି_ଗଣନା_siblings_check(dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
			  bool check_name,
			  dom_string *name)
{
	dom_ଗଣ୍ଠି_type type;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	dom_exception exc;

	ଯଦି (ଗଣ୍ଠି == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &type);
	ଯଦି ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ଗଣ୍ଠି)) {
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (check_name) {
		dom_string *ଗଣ୍ଠି_name = ଖାଲି;
		exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(ଗଣ୍ଠି, &ଗଣ୍ଠି_name);

		ଯଦି ((exc == DOM_NO_ERR) && (ଗଣ୍ଠି_name != ଖାଲି)) {

			ଯଦି (dom_string_caseless_isequal(name,
							ଗଣ୍ଠି_name)) {
				ret = 1;
			}
			dom_string_unref(ଗଣ୍ଠି_name);
		}
	} ଅନ୍ୟଥା {
		ret = 1;
	}

	ଫେରନ୍ତୁ ret;
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
css_error ଗଣ୍ଠି_ଗଣନା_siblings(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *n, bool same_name,
		bool after, int32_t *ଗଣନା_2)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ଗଣ୍ଠି_name = ଖାଲି;

	ଯଦି (same_name) {
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି = n;
		exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(ଗଣ୍ଠି, &ଗଣ୍ଠି_name);
		ଯଦି ((exc != DOM_NO_ERR) || (ଗଣ୍ଠି_name == ଖାଲି)) {
			ଫେରନ୍ତୁ CSS_NOMEM;
		}
	}

	ଯଦି (after) {
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି = dom_ଗଣ୍ଠି_ref(n);
		dom_ଗଣ୍ଠି *next;

		କର {
			exc = dom_ଗଣ୍ଠି_get_next_sibling(ଗଣ୍ଠି, &next);
			ଯଦି ((exc != DOM_NO_ERR))
				ଭାଙ୍ଗ;

			dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
			ଗଣ୍ଠି = next;

			cnt += ଗଣ୍ଠି_ଗଣନା_siblings_check(ଗଣ୍ଠି, same_name, ଗଣ୍ଠି_name);
		} ଯେତେବେଳେ (ଗଣ୍ଠି != ଖାଲି);
	} ଅନ୍ୟଥା {
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି = dom_ଗଣ୍ଠି_ref(n);
		dom_ଗଣ୍ଠି *next;

		କର {
			exc = dom_ଗଣ୍ଠି_get_previous_sibling(ଗଣ୍ଠି, &next);
			ଯଦି ((exc != DOM_NO_ERR))
				ଭାଙ୍ଗ;

			dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
			ଗଣ୍ଠି = next;

			cnt += ଗଣ୍ଠି_ଗଣନା_siblings_check(ଗଣ୍ଠି, same_name, ଗଣ୍ଠି_name);

		} ଯେତେବେଳେ (ଗଣ୍ଠି != ଖାଲି);
	}

	ଯଦି (ଗଣ୍ଠି_name != ଖାଲି) {
		dom_string_unref(ଗଣ୍ଠି_name);
	}

	*ଗଣନା_2 = cnt;
	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_empty(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି, *next;
	dom_exception err;

	*match = true;

	err = dom_ଗଣ୍ଠି_get_first_child(n, &n);
	ଯଦି (err != DOM_NO_ERR) {
		ଫେରନ୍ତୁ CSS_BADPARM;
	}

	ଯେତେବେଳେ (n != ଖାଲି) {
		dom_ଗଣ୍ଠି_type ntype;
		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(n, &ntype);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_BADPARM;
		}

		ଯଦି (ntype == DOM_ELEMENT_ଗଣ୍ଠି ||
		    ntype == DOM_TEXT_ଗଣ୍ଠି) {
			*match = false;
			dom_ଗଣ୍ଠି_unref(n);
			ଭାଙ୍ଗ;
		}

		err = dom_ଗଣ୍ଠି_get_next_sibling(n, &next);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(n);
			ଫେରନ୍ତୁ CSS_BADPARM;
		}
		dom_ଗଣ୍ଠି_unref(n);
		n = next;
	}

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_link(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *n, bool *match)
{
	dom_ଗଣ୍ଠି *ଗଣ୍ଠି = n;
	dom_exception exc;
	dom_string *ଗଣ୍ଠି_name = ଖାଲି;

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(ଗଣ୍ଠି, &ଗଣ୍ଠି_name);
	ଯଦି ((exc != DOM_NO_ERR) || (ଗଣ୍ଠି_name == ଖାଲି)) {
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	ଯଦି (dom_string_caseless_lwc_isequal(ଗଣ୍ଠି_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ଗଣ୍ଠି, corestring_dom_href,
				&has_href);
		ଯଦି ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} ଅନ୍ୟଥା {
			*match = false;
		}
	} ଅନ୍ୟଥା {
		*match = false;
	}
	dom_string_unref(ଗଣ୍ଠି_name);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_visited(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ଅଚଳ ସଂରଚନା url_data *data;

	dom_exception exc;
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_string *s = ଖାଲି;

	*match = false;

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(n, &s);
	ଯଦି ((exc != DOM_NO_ERR) || (s == ଖାଲି)) {
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	ଯଦି (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		ଫେରନ୍ତୁ CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ଖାଲି;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	ଯଦି ((exc != DOM_NO_ERR) || (s == ଖାଲି)) {
		/* Can't be visited; not got a URL */
		ଫେରନ୍ତୁ CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	ଯଦି (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	ଯଦି (data != ଖାଲି && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_hover(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_active(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_focus(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_enabled(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_disabled(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_checked(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_target(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match)
{
	/** \todo Support target */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ଗଣ୍ଠି_is_lang(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	ଫେରନ୍ତୁ CSS_OK;
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
css_error ua_default_for_property(ଶୂନ୍ୟ *pw, uint32_t property, css_hint *hint)
{
	ଯଦି (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} ଅନ୍ୟଥା ଯଦି (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = ଖାଲି;
		ବାଛ (nsoption_int(font_default)) {
		ସ୍ଥିତି PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			ଭାଙ୍ଗ;
		}
	} ଅନ୍ୟଥା ଯଦି (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = ଖାଲି;
		hint->status = CSS_QUOTES_NONE;
	} ଅନ୍ୟଥା ଯଦି (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = ଖାଲି;
		hint->status = 0;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ CSS_INVALID;
	}

	ଫେରନ୍ତୁ CSS_OK;
}

css_error set_libcss_ଗଣ୍ଠି_data(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ *libcss_ଗଣ୍ଠି_data)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_exception err;
	ଶୂନ୍ୟ *old_ଗଣ୍ଠି_data;

	/* Set this node's node data */
	err = dom_ଗଣ୍ଠି_set_user_data(n,
			corestring_dom_ns_ଚାବି_libcss_ଗଣ୍ଠି_data,
			libcss_ଗଣ୍ଠି_data, nscss_dom_user_data_handler,
			(ଶୂନ୍ୟ *) &old_ଗଣ୍ଠି_data);
	ଯଦି (err != DOM_NO_ERR) {
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	ପ୍ରତିଜ୍ଞା(old_ଗଣ୍ଠି_data == ଖାଲି);

	ଫେରନ୍ତୁ CSS_OK;
}

css_error get_libcss_ଗଣ୍ଠି_data(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, ଶୂନ୍ୟ **libcss_ଗଣ୍ଠି_data)
{
	dom_ଗଣ୍ଠି *n = ଗଣ୍ଠି;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ଗଣ୍ଠି_get_user_data(n,
			corestring_dom_ns_ଚାବି_libcss_ଗଣ୍ଠି_data,
			libcss_ଗଣ୍ଠି_data);
	ଯଦି (err != DOM_NO_ERR) {
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	ଫେରନ୍ତୁ CSS_OK;
}
