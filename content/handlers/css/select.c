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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>

#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "netsurf/plot_style.h"
#అన్తర్భావ "netsurf/url_db.h"
#అన్తర్భావ "desktop/system_colour.h"

#అన్తర్భావ "css/internal.h"
#అన్తర్భావ "css/hints.h"
#అన్తర్భావ "css/select.h"

స్థిర css_error కణుపు_name(శూన్య *pw, శూన్య *కణుపు, css_qname *qname);
స్థిర css_error కణుపు_classes(శూన్య *pw, శూన్య *కణుపు,
		lwc_string ***classes, uint32_t *n_classes);
స్థిర css_error కణుపు_id(శూన్య *pw, శూన్య *కణుపు, lwc_string **id);
స్థిర css_error named_మాతృ_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **మాతృ);
స్థిర css_error named_sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **sibling);
స్థిర css_error named_generic_sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **sibling);
స్థిర css_error మాతృ_కణుపు(శూన్య *pw, శూన్య *కణుపు, శూన్య **మాతృ);
స్థిర css_error sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు, శూన్య **sibling);
స్థిర css_error కణుపు_has_name(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, bool *సరిపోలిక);
స్థిర css_error కణుపు_has_class(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *name, bool *సరిపోలిక);
స్థిర css_error కణుపు_has_id(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *name, bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_equal(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_dashmatch(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_includes(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_ఉపసర్గ(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_suffix(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_has_attribute_substring(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక);
స్థిర css_error కణుపు_is_మూలం(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_లెక్క_siblings(శూన్య *pw, శూన్య *కణుపు,
		bool same_name, bool after, int32_t *లెక్క);
స్థిర css_error కణుపు_is_ఖాళీలు(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_link(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_hover(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_చురుకైన(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_focus(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_enabled(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_disabled(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_checked(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_లక్ష్యం(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);
స్థిర css_error కణుపు_is_lang(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *lang, bool *సరిపోలిక);
స్థిర css_error ua_default_for_property(శూన్య *pw, uint32_t property,
		css_hint *hint);
స్థిర css_error సమితి_libcss_కణుపు_దత్తాంశం(శూన్య *pw, శూన్య *కణుపు,
		శూన్య *libcss_కణుపు_దత్తాంశం);
స్థిర css_error get_libcss_కణుపు_దత్తాంశం(శూన్య *pw, శూన్య *కణుపు,
		శూన్య **libcss_కణుపు_దత్తాంశం);

/**
 * Selection callback table for libcss
 */
స్థిర css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_సంస్కరణ_1,

	కణుపు_name,
	కణుపు_classes,
	కణుపు_id,
	named_ancestor_కణుపు,
	named_మాతృ_కణుపు,
	named_sibling_కణుపు,
	named_generic_sibling_కణుపు,
	మాతృ_కణుపు,
	sibling_కణుపు,
	కణుపు_has_name,
	కణుపు_has_class,
	కణుపు_has_id,
	కణుపు_has_attribute,
	కణుపు_has_attribute_equal,
	కణుపు_has_attribute_dashmatch,
	కణుపు_has_attribute_includes,
	కణుపు_has_attribute_ఉపసర్గ,
	కణుపు_has_attribute_suffix,
	కణుపు_has_attribute_substring,
	కణుపు_is_మూలం,
	కణుపు_లెక్క_siblings,
	కణుపు_is_ఖాళీలు,
	కణుపు_is_link,
	కణుపు_is_సందర్శించిన,
	కణుపు_is_hover,
	కణుపు_is_చురుకైన,
	కణుపు_is_focus,
	కణుపు_is_enabled,
	కణుపు_is_disabled,
	కణుపు_is_checked,
	కణుపు_is_లక్ష్యం,
	కణుపు_is_lang,
	కణుపు_presentational_hint,
	ua_default_for_property,
	సమితి_libcss_కణుపు_దత్తాంశం,
	get_libcss_కణుపు_దత్తాంశం,
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
css_stylesheet *nscss_create_inline_style(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		స్థిరరాశి అక్షరం *charset, స్థిరరాశి అక్షరం *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_సంస్కరణ = CSS_STYLESHEET_PARAMS_సంస్కరణ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ఖాళీ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ఖాళీ;
	params.import = ఖాళీ;
	params.import_pw = ఖాళీ;
	params.color = ns_system_రంగు;
	params.color_pw = ఖాళీ;
	params.font = ఖాళీ;
	params.font_pw = ఖాళీ;

	error = css_stylesheet_create(&params, &sheet);
	ఒకవేళ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		తిరిగి ఖాళీ;
	}

	error = css_stylesheet_append_దత్తాంశం(sheet, దత్తాంశం, len);
	ఒకవేళ (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		తిరిగి ఖాళీ;
	}

	error = css_stylesheet_దత్తాంశం_పూర్తి(sheet);
	ఒకవేళ (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		తిరిగి ఖాళీ;
	}

	తిరిగి sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
స్థిర శూన్య nscss_dom_user_దత్తాంశం_handler(dom_కణుపు_operation operation,
		dom_string *కీలకం, శూన్య *దత్తాంశం, నిర్మాణం dom_కణుపు *src,
		నిర్మాణం dom_కణుపు *dst)
{
	css_error error;

	ఒకవేళ (dom_string_isequal(corestring_dom_ns_కీలకం_libcss_కణుపు_దత్తాంశం,
			కీలకం) == false || దత్తాంశం == ఖాళీ) {
		తిరిగి;
	}

	ఎంపిక (operation) {
	స్థితి DOM_కణుపు_CLONED:
		error = css_libcss_కణుపు_దత్తాంశం_handler(&selection_handler,
				CSS_కణుపు_CLONED,
				ఖాళీ, src, dst, దత్తాంశం);
		ఒకవేళ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		ఆపు;

	స్థితి DOM_కణుపు_RENAMED:
		error = css_libcss_కణుపు_దత్తాంశం_handler(&selection_handler,
				CSS_కణుపు_MODIFIED,
				ఖాళీ, src, ఖాళీ, దత్తాంశం);
		ఒకవేళ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		ఆపు;

	స్థితి DOM_కణుపు_IMPORTED:
	స్థితి DOM_కణుపు_ADOPTED:
	స్థితి DOM_కణుపు_DELETED:
		error = css_libcss_కణుపు_దత్తాంశం_handler(&selection_handler,
				CSS_కణుపు_DELETED,
				ఖాళీ, src, ఖాళీ, దత్తాంశం);
		ఒకవేళ (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		ఆపు;

	అప్రమేయ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ప్రతిజ్ఞా(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_కణుపు *n,
		స్థిరరాశి css_media *media,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	పూర్ణాంకం pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	ఒకవేళ (error != CSS_OK || styles == ఖాళీ) {
		/* Failed selecting partial style -- bail out */
		తిరిగి ఖాళీ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	ఒకవేళ (ctx->మాతృ_style != ఖాళీ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->మాతృ_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		ఒకవేళ (error != CSS_OK) {
			css_select_results_destroy(styles);
			తిరిగి ఖాళీ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	కొరకు (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_లెక్క;
			pseudo_element++) {

		ఒకవేళ (pseudo_element == CSS_PSEUDO_ELEMENT_మొదటి_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_మొదటి_పంక్తి)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			కొనసాగు;

		ఒకవేళ (styles->styles[pseudo_element] == ఖాళీ)
			/* There were no rules concerning this pseudo element */
			కొనసాగు;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		ఒకవేళ (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			తిరిగి ఖాళీ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	తిరిగి styles;
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
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *మాతృ)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	ఒకవేళ (error != CSS_OK) {
		తిరిగి ఖాళీ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(మాతృ, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	ఒకవేళ (error != CSS_OK) {
		css_computed_style_destroy(composed);
		తిరిగి ఖాళీ;
	}

	తిరిగి composed;
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
css_error కణుపు_name(శూన్య *pw, శూన్య *కణుపు, css_qname *qname)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_exception err;

	err = dom_కణుపు_get_కణుపు_name(n, &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	qname->ns = ఖాళీ;

	err = dom_string_intern(name, &qname->name);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		తిరిగి CSS_NOMEM;
	}

	dom_string_unref(name);

	తిరిగి CSS_OK;
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
css_error కణుపు_classes(శూన్య *pw, శూన్య *కణుపు,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_కణుపు *n = కణుపు;
	dom_exception err;

	*classes = ఖాళీ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	తిరిగి CSS_OK;
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
css_error కణుపు_id(శూన్య *pw, శూన్య *కణుపు, lwc_string **id)
{
	dom_కణుపు *n = కణుపు;
	dom_string *attr;
	dom_exception err;

	*id = ఖాళీ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	ఒకవేళ (attr != ఖాళీ) {
		err = dom_string_intern(attr, id);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			తిరిగి CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	తిరిగి CSS_OK;
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
css_error named_ancestor_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **ancestor)
{
	dom_element_named_ancestor_కణుపు(కణుపు, qname->name,
			(నిర్మాణం dom_element **)ancestor);
	dom_కణుపు_unref(*ancestor);

	తిరిగి CSS_OK;
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
css_error named_మాతృ_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **మాతృ)
{
	dom_element_named_మాతృ_కణుపు(కణుపు, qname->name,
			(నిర్మాణం dom_element **)మాతృ);
	dom_కణుపు_unref(*మాతృ);

	తిరిగి CSS_OK;
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
css_error named_sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **sibling)
{
	dom_కణుపు *n = కణుపు;
	dom_కణుపు *prev;
	dom_exception err;

	*sibling = ఖాళీ;

	/* Find sibling element */
	err = dom_కణుపు_get_మునుపటి_sibling(n, &n);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_OK;

	అయితే (n != ఖాళీ) {
		dom_కణుపు_type type;

		err = dom_కణుపు_get_కణుపు_type(n, &type);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		ఒకవేళ (type == DOM_ELEMENT_కణుపు)
			ఆపు;

		err = dom_కణుపు_get_మునుపటి_sibling(n, &prev);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		dom_కణుపు_unref(n);
		n = prev;
	}

	ఒకవేళ (n != ఖాళీ) {
		dom_string *name;

		err = dom_కణుపు_get_కణుపు_name(n, &name);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		dom_కణుపు_unref(n);

		ఒకవేళ (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	తిరిగి CSS_OK;
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
css_error named_generic_sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **sibling)
{
	dom_కణుపు *n = కణుపు;
	dom_కణుపు *prev;
	dom_exception err;

	*sibling = ఖాళీ;

	err = dom_కణుపు_get_మునుపటి_sibling(n, &n);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_OK;

	అయితే (n != ఖాళీ) {
		dom_కణుపు_type type;
		dom_string *name;

		err = dom_కణుపు_get_కణుపు_type(n, &type);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		ఒకవేళ (type == DOM_ELEMENT_కణుపు) {
			err = dom_కణుపు_get_కణుపు_name(n, &name);
			ఒకవేళ (err != DOM_NO_ERR) {
				dom_కణుపు_unref(n);
				తిరిగి CSS_OK;
			}

			ఒకవేళ (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_కణుపు_unref(n);
				*sibling = n;
				ఆపు;
			}
			dom_string_unref(name);
		}

		err = dom_కణుపు_get_మునుపటి_sibling(n, &prev);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		dom_కణుపు_unref(n);
		n = prev;
	}

	తిరిగి CSS_OK;
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
css_error మాతృ_కణుపు(శూన్య *pw, శూన్య *కణుపు, శూన్య **మాతృ)
{
	dom_element_మాతృ_కణుపు(కణుపు, (నిర్మాణం dom_element **)మాతృ);
	dom_కణుపు_unref(*మాతృ);

	తిరిగి CSS_OK;
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
css_error sibling_కణుపు(శూన్య *pw, శూన్య *కణుపు, శూన్య **sibling)
{
	dom_కణుపు *n = కణుపు;
	dom_కణుపు *prev;
	dom_exception err;

	*sibling = ఖాళీ;

	/* Find sibling element */
	err = dom_కణుపు_get_మునుపటి_sibling(n, &n);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_OK;

	అయితే (n != ఖాళీ) {
		dom_కణుపు_type type;

		err = dom_కణుపు_get_కణుపు_type(n, &type);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		ఒకవేళ (type == DOM_ELEMENT_కణుపు)
			ఆపు;

		err = dom_కణుపు_get_మునుపటి_sibling(n, &prev);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_OK;
		}

		dom_కణుపు_unref(n);
		n = prev;
	}

	ఒకవేళ (n != ఖాళీ) {
		/** \todo Sort out reference counting */
		dom_కణుపు_unref(n);

		*sibling = n;
	}

	తిరిగి CSS_OK;
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
css_error కణుపు_has_name(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, bool *సరిపోలిక)
{
	nscss_select_ctx *ctx = pw;
	dom_కణుపు *n = కణుపు;

	ఒకవేళ (lwc_string_isequal(qname->name, ctx->universal, సరిపోలిక) ==
			lwc_error_ok && *సరిపోలిక == false) {
		dom_string *name;
		dom_exception err;

		err = dom_కణుపు_get_కణుపు_name(n, &name);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి CSS_OK;

		/* Element names are case insensitive in HTML */
		*సరిపోలిక = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	తిరిగి CSS_OK;
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
css_error కణుపు_has_class(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *name, bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, సరిపోలిక);

	ప్రతిజ్ఞా(err == DOM_NO_ERR);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_id(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *name, bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *attr;
	dom_exception err;

	*సరిపోలిక = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_OK;

	ఒకవేళ (attr != ఖాళీ) {
		*సరిపోలిక = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
			lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_has_attribute(n, name, సరిపోలిక);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_string_unref(name);
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_equal(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_నిడివి(విలువ);

	ఒకవేళ (vlen == 0) {
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	*సరిపోలిక = dom_string_caseless_lwc_isequal(atr_val, విలువ);

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_dashmatch(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_నిడివి(విలువ);

	ఒకవేళ (vlen == 0) {
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*సరిపోలిక = dom_string_caseless_lwc_isequal(atr_val, విలువ);

	/* check for dashmatch */
	ఒకవేళ (*సరిపోలిక == false) {
		స్థిరరాశి అక్షరం *vdata = lwc_string_దత్తాంశం(విలువ);
		స్థిరరాశి అక్షరం *దత్తాంశం = (స్థిరరాశి అక్షరం *) dom_string_దత్తాంశం(atr_val);
		size_t len = dom_string_byte_నిడివి(atr_val);

		ఒకవేళ (len > vlen && దత్తాంశం[vlen] == '-' &&
		    strncasecmp(దత్తాంశం, vdata, vlen) == 0) {
				*సరిపోలిక = true;
		}
	}

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_includes(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_నిడివి(విలువ);
	స్థిరరాశి అక్షరం *p;
	స్థిరరాశి అక్షరం *ప్రారంభం;
	స్థిరరాశి అక్షరం *చివర;

	*సరిపోలిక = false;

	ఒకవేళ (vlen == 0) {
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	ప్రారంభం = (స్థిరరాశి అక్షరం *) dom_string_దత్తాంశం(atr_val);
	చివర = ప్రారంభం + dom_string_byte_నిడివి(atr_val);

	కొరకు (p = ప్రారంభం; p <= చివర; p++) {
		ఒకవేళ (*p == ' ' || *p == '\0') {
			ఒకవేళ ((size_t) (p - ప్రారంభం) == vlen &&
			    strncasecmp(ప్రారంభం,
					lwc_string_దత్తాంశం(విలువ),
					vlen) == 0) {
				*సరిపోలిక = true;
				ఆపు;
			}

			ప్రారంభం = p + 1;
		}
	}

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_ఉపసర్గ(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_నిడివి(విలువ);

	ఒకవేళ (vlen == 0) {
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*సరిపోలిక = dom_string_caseless_lwc_isequal(atr_val, విలువ);

	/* check for prefix match */
	ఒకవేళ (*సరిపోలిక == false) {
		స్థిరరాశి అక్షరం *దత్తాంశం = (స్థిరరాశి అక్షరం *) dom_string_దత్తాంశం(atr_val);
		size_t len = dom_string_byte_నిడివి(atr_val);

		ఒకవేళ ((len >= vlen) &&
		    (strncasecmp(దత్తాంశం, lwc_string_దత్తాంశం(విలువ), vlen) == 0)) {
			*సరిపోలిక = true;
		}
	}

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_suffix(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_నిడివి(విలువ);

	ఒకవేళ (vlen == 0) {
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*సరిపోలిక = dom_string_caseless_lwc_isequal(atr_val, విలువ);

	/* check for prefix match */
	ఒకవేళ (*సరిపోలిక == false) {
		స్థిరరాశి అక్షరం *దత్తాంశం = (స్థిరరాశి అక్షరం *) dom_string_దత్తాంశం(atr_val);
		size_t len = dom_string_byte_నిడివి(atr_val);

		స్థిరరాశి అక్షరం *ప్రారంభం = (అక్షరం *) దత్తాంశం + len - vlen;

		ఒకవేళ ((len >= vlen) &&
		    (strncasecmp(ప్రారంభం, lwc_string_దత్తాంశం(విలువ), vlen) == 0)) {
			*సరిపోలిక = true;
		}


	}

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_has_attribute_substring(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, lwc_string *విలువ,
		bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_నిడివి(విలువ);

	ఒకవేళ (vlen == 0) {
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	err = dom_string_create_interned(
		(స్థిరరాశి uint8_t *) lwc_string_దత్తాంశం(qname->name),
		lwc_string_నిడివి(qname->name), &name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	ఒకవేళ ((err != DOM_NO_ERR) || (atr_val == ఖాళీ)) {
		dom_string_unref(name);
		*సరిపోలిక = false;
		తిరిగి CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*సరిపోలిక = dom_string_caseless_lwc_isequal(atr_val, విలువ);

	/* check for prefix match */
	ఒకవేళ (*సరిపోలిక == false) {
		స్థిరరాశి అక్షరం *vdata = lwc_string_దత్తాంశం(విలువ);
		స్థిరరాశి అక్షరం *ప్రారంభం = (స్థిరరాశి అక్షరం *) dom_string_దత్తాంశం(atr_val);
		size_t len = dom_string_byte_నిడివి(atr_val);
		స్థిరరాశి అక్షరం *చివరి_ప్రారంభం = ప్రారంభం + len - vlen;

		ఒకవేళ (len >= vlen) {
			అయితే (ప్రారంభం <= చివరి_ప్రారంభం) {
				ఒకవేళ (strncasecmp(ప్రారంభం, vdata,
						vlen) == 0) {
					*సరిపోలిక = true;
					ఆపు;
				}

				ప్రారంభం++;
			}
		}
	}

	dom_string_unref(atr_val);

	తిరిగి CSS_OK;
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
css_error కణుపు_is_మూలం(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు;
	dom_కణుపు *మాతృ;
	dom_కణుపు_type type;
	dom_exception err;

	err = dom_కణుపు_get_మాతృ_కణుపు(n, &మాతృ);
	ఒకవేళ (err != DOM_NO_ERR) {
		తిరిగి CSS_NOMEM;
	}

	ఒకవేళ (మాతృ != ఖాళీ) {
		err = dom_కణుపు_get_కణుపు_type(మాతృ, &type);

		dom_కణుపు_unref(మాతృ);

		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి CSS_NOMEM;

		ఒకవేళ (type != DOM_DOCUMENT_కణుపు) {
			*సరిపోలిక = false;
			తిరిగి CSS_OK;
		}
	}

	*సరిపోలిక = true;

	తిరిగి CSS_OK;
}

స్థిర పూర్ణాంకం
కణుపు_లెక్క_siblings_check(dom_కణుపు *కణుపు,
			  bool check_name,
			  dom_string *name)
{
	dom_కణుపు_type type;
	పూర్ణాంకం ret = 0;
	dom_exception exc;

	ఒకవేళ (కణుపు == ఖాళీ)
		తిరిగి 0;

	exc = dom_కణుపు_get_కణుపు_type(కణుపు, &type);
	ఒకవేళ ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_కణుపు)) {
		తిరిగి 0;
	}

	ఒకవేళ (check_name) {
		dom_string *కణుపు_name = ఖాళీ;
		exc = dom_కణుపు_get_కణుపు_name(కణుపు, &కణుపు_name);

		ఒకవేళ ((exc == DOM_NO_ERR) && (కణుపు_name != ఖాళీ)) {

			ఒకవేళ (dom_string_caseless_isequal(name,
							కణుపు_name)) {
				ret = 1;
			}
			dom_string_unref(కణుపు_name);
		}
	} లేకపోతే {
		ret = 1;
	}

	తిరిగి ret;
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
css_error కణుపు_లెక్క_siblings(శూన్య *pw, శూన్య *n, bool same_name,
		bool after, int32_t *లెక్క)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *కణుపు_name = ఖాళీ;

	ఒకవేళ (same_name) {
		dom_కణుపు *కణుపు = n;
		exc = dom_కణుపు_get_కణుపు_name(కణుపు, &కణుపు_name);
		ఒకవేళ ((exc != DOM_NO_ERR) || (కణుపు_name == ఖాళీ)) {
			తిరిగి CSS_NOMEM;
		}
	}

	ఒకవేళ (after) {
		dom_కణుపు *కణుపు = dom_కణుపు_ref(n);
		dom_కణుపు *తదుపరి;

		చేయి {
			exc = dom_కణుపు_get_తదుపరి_sibling(కణుపు, &తదుపరి);
			ఒకవేళ ((exc != DOM_NO_ERR))
				ఆపు;

			dom_కణుపు_unref(కణుపు);
			కణుపు = తదుపరి;

			cnt += కణుపు_లెక్క_siblings_check(కణుపు, same_name, కణుపు_name);
		} అయితే (కణుపు != ఖాళీ);
	} లేకపోతే {
		dom_కణుపు *కణుపు = dom_కణుపు_ref(n);
		dom_కణుపు *తదుపరి;

		చేయి {
			exc = dom_కణుపు_get_మునుపటి_sibling(కణుపు, &తదుపరి);
			ఒకవేళ ((exc != DOM_NO_ERR))
				ఆపు;

			dom_కణుపు_unref(కణుపు);
			కణుపు = తదుపరి;

			cnt += కణుపు_లెక్క_siblings_check(కణుపు, same_name, కణుపు_name);

		} అయితే (కణుపు != ఖాళీ);
	}

	ఒకవేళ (కణుపు_name != ఖాళీ) {
		dom_string_unref(కణుపు_name);
	}

	*లెక్క = cnt;
	తిరిగి CSS_OK;
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
css_error కణుపు_is_ఖాళీలు(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	dom_కణుపు *n = కణుపు, *తదుపరి;
	dom_exception err;

	*సరిపోలిక = true;

	err = dom_కణుపు_get_మొదటి_బిడ్డ(n, &n);
	ఒకవేళ (err != DOM_NO_ERR) {
		తిరిగి CSS_BADPARM;
	}

	అయితే (n != ఖాళీ) {
		dom_కణుపు_type ntype;
		err = dom_కణుపు_get_కణుపు_type(n, &ntype);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_BADPARM;
		}

		ఒకవేళ (ntype == DOM_ELEMENT_కణుపు ||
		    ntype == DOM_పాఠ్యం_కణుపు) {
			*సరిపోలిక = false;
			dom_కణుపు_unref(n);
			ఆపు;
		}

		err = dom_కణుపు_get_తదుపరి_sibling(n, &తదుపరి);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(n);
			తిరిగి CSS_BADPARM;
		}
		dom_కణుపు_unref(n);
		n = తదుపరి;
	}

	తిరిగి CSS_OK;
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
css_error కణుపు_is_link(శూన్య *pw, శూన్య *n, bool *సరిపోలిక)
{
	dom_కణుపు *కణుపు = n;
	dom_exception exc;
	dom_string *కణుపు_name = ఖాళీ;

	exc = dom_కణుపు_get_కణుపు_name(కణుపు, &కణుపు_name);
	ఒకవేళ ((exc != DOM_NO_ERR) || (కణుపు_name == ఖాళీ)) {
		తిరిగి CSS_NOMEM;
	}

	ఒకవేళ (dom_string_caseless_lwc_isequal(కణుపు_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(కణుపు, corestring_dom_href,
				&has_href);
		ఒకవేళ ((exc == DOM_NO_ERR) && (has_href)) {
			*సరిపోలిక = true;
		} లేకపోతే {
			*సరిపోలిక = false;
		}
	} లేకపోతే {
		*సరిపోలిక = false;
	}
	dom_string_unref(కణుపు_name);

	తిరిగి CSS_OK;
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
css_error కణుపు_is_సందర్శించిన(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం;

	dom_exception exc;
	dom_కణుపు *n = కణుపు;
	dom_string *s = ఖాళీ;

	*సరిపోలిక = false;

	exc = dom_కణుపు_get_కణుపు_name(n, &s);
	ఒకవేళ ((exc != DOM_NO_ERR) || (s == ఖాళీ)) {
		తిరిగి CSS_NOMEM;
	}

	ఒకవేళ (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		తిరిగి CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = ఖాళీ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	ఒకవేళ ((exc != DOM_NO_ERR) || (s == ఖాళీ)) {
		/* Can't be visited; not got a URL */
		తిరిగి CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->పునాది_url, dom_string_దత్తాంశం(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	ఒకవేళ (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		తిరిగి CSS_NOMEM;
	}

	దత్తాంశం = urldb_get_url_దత్తాంశం(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	ఒకవేళ (దత్తాంశం != ఖాళీ && దత్తాంశం->visits > 0)
		*సరిపోలిక = true;

	nsurl_unref(url);

	తిరిగి CSS_OK;
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
css_error కణుపు_is_hover(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support hovering */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_చురుకైన(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support active nodes */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_focus(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support focussed nodes */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_enabled(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support enabled nodes */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_disabled(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support disabled nodes */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_checked(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support checked nodes */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_లక్ష్యం(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక)
{
	/** \todo Support target */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error కణుపు_is_lang(శూన్య *pw, శూన్య *కణుపు,
		lwc_string *lang, bool *సరిపోలిక)
{
	/** \todo Support languages */

	*సరిపోలిక = false;

	తిరిగి CSS_OK;
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
css_error ua_default_for_property(శూన్య *pw, uint32_t property, css_hint *hint)
{
	ఒకవేళ (property == CSS_PROP_COLOR) {
		hint->దత్తాంశం.color = 0xff000000;
		hint->స్థితి_2 = CSS_COLOR_COLOR;
	} లేకపోతే ఒకవేళ (property == CSS_PROP_FONT_FAMILY) {
		hint->దత్తాంశం.అక్షరశ్రేణులు = ఖాళీ;
		ఎంపిక (nsoption_int(font_default)) {
		స్థితి PLOT_FONT_FAMILY_SANS_SERIF:
			hint->స్థితి_2 = CSS_FONT_FAMILY_SANS_SERIF;
			ఆపు;
		స్థితి PLOT_FONT_FAMILY_SERIF:
			hint->స్థితి_2 = CSS_FONT_FAMILY_SERIF;
			ఆపు;
		స్థితి PLOT_FONT_FAMILY_MONOSPACE:
			hint->స్థితి_2 = CSS_FONT_FAMILY_MONOSPACE;
			ఆపు;
		స్థితి PLOT_FONT_FAMILY_CURSIVE:
			hint->స్థితి_2 = CSS_FONT_FAMILY_CURSIVE;
			ఆపు;
		స్థితి PLOT_FONT_FAMILY_FANTASY:
			hint->స్థితి_2 = CSS_FONT_FAMILY_FANTASY;
			ఆపు;
		}
	} లేకపోతే ఒకవేళ (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->దత్తాంశం.అక్షరశ్రేణులు = ఖాళీ;
		hint->స్థితి_2 = CSS_QUOTES_NONE;
	} లేకపోతే ఒకవేళ (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->దత్తాంశం.అక్షరశ్రేణులు = ఖాళీ;
		hint->స్థితి_2 = 0;
	} లేకపోతే {
		తిరిగి CSS_INVALID;
	}

	తిరిగి CSS_OK;
}

css_error సమితి_libcss_కణుపు_దత్తాంశం(శూన్య *pw, శూన్య *కణుపు, శూన్య *libcss_కణుపు_దత్తాంశం)
{
	dom_కణుపు *n = కణుపు;
	dom_exception err;
	శూన్య *old_కణుపు_దత్తాంశం;

	/* Set this node's node data */
	err = dom_కణుపు_సమితి_user_దత్తాంశం(n,
			corestring_dom_ns_కీలకం_libcss_కణుపు_దత్తాంశం,
			libcss_కణుపు_దత్తాంశం, nscss_dom_user_దత్తాంశం_handler,
			(శూన్య *) &old_కణుపు_దత్తాంశం);
	ఒకవేళ (err != DOM_NO_ERR) {
		తిరిగి CSS_NOMEM;
	}

	ప్రతిజ్ఞా(old_కణుపు_దత్తాంశం == ఖాళీ);

	తిరిగి CSS_OK;
}

css_error get_libcss_కణుపు_దత్తాంశం(శూన్య *pw, శూన్య *కణుపు, శూన్య **libcss_కణుపు_దత్తాంశం)
{
	dom_కణుపు *n = కణుపు;
	dom_exception err;

	/* Get this node's node data */
	err = dom_కణుపు_get_user_దత్తాంశం(n,
			corestring_dom_ns_కీలకం_libcss_కణుపు_దత్తాంశం,
			libcss_కణుపు_దత్తాంశం);
	ఒకవేళ (err != DOM_NO_ERR) {
		తిరిగి CSS_NOMEM;
	}

	తిరిగి CSS_OK;
}
