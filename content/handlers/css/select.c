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

#شامل <assert.h>
#شامل <string.h>
#شامل <strings.h>

#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/plot_style.h"
#شامل "netsurf/url_db.h"
#شامل "desktop/system_colour.h"

#شامل "css/internal.h"
#شامل "css/hints.h"
#شامل "css/select.h"

ساکن css_error گرہ_name(خالی *pw, خالی *گرہ, css_qname *qname);
ساکن css_error گرہ_classes(خالی *pw, خالی *گرہ,
		lwc_string ***classes, uint32_t *n_classes);
ساکن css_error گرہ_id(خالی *pw, خالی *گرہ, lwc_string **id);
ساکن css_error named_والد_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **والد);
ساکن css_error named_sibling_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **sibling);
ساکن css_error named_generic_sibling_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **sibling);
ساکن css_error والد_گرہ(خالی *pw, خالی *گرہ, خالی **والد);
ساکن css_error sibling_گرہ(خالی *pw, خالی *گرہ, خالی **sibling);
ساکن css_error گرہ_has_name(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, bool *مطابقت);
ساکن css_error گرہ_has_class(خالی *pw, خالی *گرہ,
		lwc_string *name, bool *مطابقت);
ساکن css_error گرہ_has_id(خالی *pw, خالی *گرہ,
		lwc_string *name, bool *مطابقت);
ساکن css_error گرہ_has_attribute(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, bool *مطابقت);
ساکن css_error گرہ_has_attribute_equal(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_has_attribute_dashmatch(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_has_attribute_includes(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_has_attribute_سابقہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_has_attribute_suffix(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_has_attribute_substring(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت);
ساکن css_error گرہ_is_جڑ(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_گنتی_siblings(خالی *pw, خالی *گرہ,
		bool same_name, bool after, int32_t *گنتی);
ساکن css_error گرہ_is_خالی_خانے(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_link(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_hover(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_فعال(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_focus(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_enabled(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_disabled(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_checked(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_ہدف(خالی *pw, خالی *گرہ, bool *مطابقت);
ساکن css_error گرہ_is_lang(خالی *pw, خالی *گرہ,
		lwc_string *lang, bool *مطابقت);
ساکن css_error ua_default_for_property(خالی *pw, uint32_t property,
		css_hint *hint);
ساکن css_error سیٹ_libcss_گرہ_ڈیٹا(خالی *pw, خالی *گرہ,
		خالی *libcss_گرہ_ڈیٹا);
ساکن css_error get_libcss_گرہ_ڈیٹا(خالی *pw, خالی *گرہ,
		خالی **libcss_گرہ_ڈیٹا);

/**
 * Selection callback table for libcss
 */
ساکن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_نسخہ_1,

	گرہ_name,
	گرہ_classes,
	گرہ_id,
	named_ancestor_گرہ,
	named_والد_گرہ,
	named_sibling_گرہ,
	named_generic_sibling_گرہ,
	والد_گرہ,
	sibling_گرہ,
	گرہ_has_name,
	گرہ_has_class,
	گرہ_has_id,
	گرہ_has_attribute,
	گرہ_has_attribute_equal,
	گرہ_has_attribute_dashmatch,
	گرہ_has_attribute_includes,
	گرہ_has_attribute_سابقہ,
	گرہ_has_attribute_suffix,
	گرہ_has_attribute_substring,
	گرہ_is_جڑ,
	گرہ_گنتی_siblings,
	گرہ_is_خالی_خانے,
	گرہ_is_link,
	گرہ_is_دیکھا,
	گرہ_is_hover,
	گرہ_is_فعال,
	گرہ_is_focus,
	گرہ_is_enabled,
	گرہ_is_disabled,
	گرہ_is_checked,
	گرہ_is_ہدف,
	گرہ_is_lang,
	گرہ_presentational_hint,
	ua_default_for_property,
	سیٹ_libcss_گرہ_ڈیٹا,
	get_libcss_گرہ_ڈیٹا,
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
css_stylesheet *nscss_create_inline_style(ثابت uint8_t *ڈیٹا, size_t len,
		ثابت حرف *charset, ثابت حرف *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_نسخہ = CSS_STYLESHEET_PARAMS_نسخہ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = تہی;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = تہی;
	params.import = تہی;
	params.import_pw = تہی;
	params.color = ns_system_رنگ;
	params.color_pw = تہی;
	params.font = تہی;
	params.font_pw = تہی;

	error = css_stylesheet_create(&params, &sheet);
	اگر (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		واپس تہی;
	}

	error = css_stylesheet_append_ڈیٹا(sheet, ڈیٹا, len);
	اگر (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		واپس تہی;
	}

	error = css_stylesheet_ڈیٹا_مکمل(sheet);
	اگر (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		واپس تہی;
	}

	واپس sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساکن خالی nscss_dom_user_ڈیٹا_handler(dom_گرہ_operation operation,
		dom_string *کلید, خالی *ڈیٹا, ساخت dom_گرہ *src,
		ساخت dom_گرہ *dst)
{
	css_error error;

	اگر (dom_string_isequal(corestring_dom_ns_کلید_libcss_گرہ_ڈیٹا,
			کلید) == false || ڈیٹا == تہی) {
		واپس;
	}

	انتخاب (operation) {
	حالت DOM_گرہ_CLONED:
		error = css_libcss_گرہ_ڈیٹا_handler(&selection_handler,
				CSS_گرہ_CLONED,
				تہی, src, dst, ڈیٹا);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		توڑو;

	حالت DOM_گرہ_RENAMED:
		error = css_libcss_گرہ_ڈیٹا_handler(&selection_handler,
				CSS_گرہ_MODIFIED,
				تہی, src, تہی, ڈیٹا);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		توڑو;

	حالت DOM_گرہ_IMPORTED:
	حالت DOM_گرہ_ADOPTED:
	حالت DOM_گرہ_DELETED:
		error = css_libcss_گرہ_ڈیٹا_handler(&selection_handler,
				CSS_گرہ_DELETED,
				تہی, src, تہی, ڈیٹا);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		توڑو;

	طےشدہ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_گرہ *n,
		ثابت css_media *media,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	صحیح pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	اگر (error != CSS_OK || styles == تہی) {
		/* Failed selecting partial style -- bail out */
		واپس تہی;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	اگر (ctx->والد_style != تہی) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->والد_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		اگر (error != CSS_OK) {
			css_select_results_destroy(styles);
			واپس تہی;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	برائے (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_گنتی;
			pseudo_element++) {

		اگر (pseudo_element == CSS_PSEUDO_ELEMENT_پہلا_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_پہلا_سطر)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			جاری;

		اگر (styles->styles[pseudo_element] == تہی)
			/* There were no rules concerning this pseudo element */
			جاری;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		اگر (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			واپس تہی;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	واپس styles;
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
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *والد)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	اگر (error != CSS_OK) {
		واپس تہی;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(والد, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	اگر (error != CSS_OK) {
		css_computed_style_destroy(composed);
		واپس تہی;
	}

	واپس composed;
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
css_error گرہ_name(خالی *pw, خالی *گرہ, css_qname *qname)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_exception err;

	err = dom_گرہ_get_گرہ_name(n, &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	qname->ns = تہی;

	err = dom_string_intern(name, &qname->name);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(name);
		واپس CSS_NOMEM;
	}

	dom_string_unref(name);

	واپس CSS_OK;
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
css_error گرہ_classes(خالی *pw, خالی *گرہ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_گرہ *n = گرہ;
	dom_exception err;

	*classes = تہی;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	واپس CSS_OK;
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
css_error گرہ_id(خالی *pw, خالی *گرہ, lwc_string **id)
{
	dom_گرہ *n = گرہ;
	dom_string *attr;
	dom_exception err;

	*id = تہی;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	اگر (attr != تہی) {
		err = dom_string_intern(attr, id);
		اگر (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			واپس CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	واپس CSS_OK;
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
css_error named_ancestor_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **ancestor)
{
	dom_element_named_ancestor_گرہ(گرہ, qname->name,
			(ساخت dom_element **)ancestor);
	dom_گرہ_unref(*ancestor);

	واپس CSS_OK;
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
css_error named_والد_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **والد)
{
	dom_element_named_والد_گرہ(گرہ, qname->name,
			(ساخت dom_element **)والد);
	dom_گرہ_unref(*والد);

	واپس CSS_OK;
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
css_error named_sibling_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *پچھلا;
	dom_exception err;

	*sibling = تہی;

	/* Find sibling element */
	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		واپس CSS_OK;

	جب (n != تہی) {
		dom_گرہ_type type;

		err = dom_گرہ_get_گرہ_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گرہ)
			توڑو;

		err = dom_گرہ_get_پچھلا_sibling(n, &پچھلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		dom_گرہ_unref(n);
		n = پچھلا;
	}

	اگر (n != تہی) {
		dom_string *name;

		err = dom_گرہ_get_گرہ_name(n, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		dom_گرہ_unref(n);

		اگر (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	واپس CSS_OK;
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
css_error named_generic_sibling_گرہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, خالی **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *پچھلا;
	dom_exception err;

	*sibling = تہی;

	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		واپس CSS_OK;

	جب (n != تہی) {
		dom_گرہ_type type;
		dom_string *name;

		err = dom_گرہ_get_گرہ_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گرہ) {
			err = dom_گرہ_get_گرہ_name(n, &name);
			اگر (err != DOM_NO_ERR) {
				dom_گرہ_unref(n);
				واپس CSS_OK;
			}

			اگر (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_گرہ_unref(n);
				*sibling = n;
				توڑو;
			}
			dom_string_unref(name);
		}

		err = dom_گرہ_get_پچھلا_sibling(n, &پچھلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		dom_گرہ_unref(n);
		n = پچھلا;
	}

	واپس CSS_OK;
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
css_error والد_گرہ(خالی *pw, خالی *گرہ, خالی **والد)
{
	dom_element_والد_گرہ(گرہ, (ساخت dom_element **)والد);
	dom_گرہ_unref(*والد);

	واپس CSS_OK;
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
css_error sibling_گرہ(خالی *pw, خالی *گرہ, خالی **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *پچھلا;
	dom_exception err;

	*sibling = تہی;

	/* Find sibling element */
	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		واپس CSS_OK;

	جب (n != تہی) {
		dom_گرہ_type type;

		err = dom_گرہ_get_گرہ_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گرہ)
			توڑو;

		err = dom_گرہ_get_پچھلا_sibling(n, &پچھلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_OK;
		}

		dom_گرہ_unref(n);
		n = پچھلا;
	}

	اگر (n != تہی) {
		/** \todo Sort out reference counting */
		dom_گرہ_unref(n);

		*sibling = n;
	}

	واپس CSS_OK;
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
css_error گرہ_has_name(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, bool *مطابقت)
{
	nscss_select_ctx *ctx = pw;
	dom_گرہ *n = گرہ;

	اگر (lwc_string_isequal(qname->name, ctx->universal, مطابقت) ==
			lwc_error_ok && *مطابقت == false) {
		dom_string *name;
		dom_exception err;

		err = dom_گرہ_get_گرہ_name(n, &name);
		اگر (err != DOM_NO_ERR)
			واپس CSS_OK;

		/* Element names are case insensitive in HTML */
		*مطابقت = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	واپس CSS_OK;
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
css_error گرہ_has_class(خالی *pw, خالی *گرہ,
		lwc_string *name, bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, مطابقت);

	أكد(err == DOM_NO_ERR);

	واپس CSS_OK;
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
css_error گرہ_has_id(خالی *pw, خالی *گرہ,
		lwc_string *name, bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *attr;
	dom_exception err;

	*مطابقت = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	اگر (err != DOM_NO_ERR)
		واپس CSS_OK;

	اگر (attr != تہی) {
		*مطابقت = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	واپس CSS_OK;
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
css_error گرہ_has_attribute(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
			lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_has_attribute(n, name, مطابقت);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(name);
		واپس CSS_OK;
	}

	dom_string_unref(name);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_equal(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمبائی(قدر);

	اگر (vlen == 0) {
		*مطابقت = false;
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	*مطابقت = dom_string_caseless_lwc_isequal(atr_val, قدر);

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_dashmatch(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمبائی(قدر);

	اگر (vlen == 0) {
		*مطابقت = false;
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*مطابقت = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for dashmatch */
	اگر (*مطابقت == false) {
		ثابت حرف *vdata = lwc_string_ڈیٹا(قدر);
		ثابت حرف *ڈیٹا = (ثابت حرف *) dom_string_ڈیٹا(atr_val);
		size_t len = dom_string_byte_لمبائی(atr_val);

		اگر (len > vlen && ڈیٹا[vlen] == '-' &&
		    strncasecmp(ڈیٹا, vdata, vlen) == 0) {
				*مطابقت = true;
		}
	}

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_includes(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_لمبائی(قدر);
	ثابت حرف *p;
	ثابت حرف *آغاز;
	ثابت حرف *انتہا;

	*مطابقت = false;

	اگر (vlen == 0) {
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	آغاز = (ثابت حرف *) dom_string_ڈیٹا(atr_val);
	انتہا = آغاز + dom_string_byte_لمبائی(atr_val);

	برائے (p = آغاز; p <= انتہا; p++) {
		اگر (*p == ' ' || *p == '\0') {
			اگر ((size_t) (p - آغاز) == vlen &&
			    strncasecmp(آغاز,
					lwc_string_ڈیٹا(قدر),
					vlen) == 0) {
				*مطابقت = true;
				توڑو;
			}

			آغاز = p + 1;
		}
	}

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_سابقہ(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمبائی(قدر);

	اگر (vlen == 0) {
		*مطابقت = false;
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*مطابقت = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	اگر (*مطابقت == false) {
		ثابت حرف *ڈیٹا = (ثابت حرف *) dom_string_ڈیٹا(atr_val);
		size_t len = dom_string_byte_لمبائی(atr_val);

		اگر ((len >= vlen) &&
		    (strncasecmp(ڈیٹا, lwc_string_ڈیٹا(قدر), vlen) == 0)) {
			*مطابقت = true;
		}
	}

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_suffix(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمبائی(قدر);

	اگر (vlen == 0) {
		*مطابقت = false;
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*مطابقت = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	اگر (*مطابقت == false) {
		ثابت حرف *ڈیٹا = (ثابت حرف *) dom_string_ڈیٹا(atr_val);
		size_t len = dom_string_byte_لمبائی(atr_val);

		ثابت حرف *آغاز = (حرف *) ڈیٹا + len - vlen;

		اگر ((len >= vlen) &&
		    (strncasecmp(آغاز, lwc_string_ڈیٹا(قدر), vlen) == 0)) {
			*مطابقت = true;
		}


	}

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_has_attribute_substring(خالی *pw, خالی *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمبائی(قدر);

	اگر (vlen == 0) {
		*مطابقت = false;
		واپس CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_ڈیٹا(qname->name),
		lwc_string_لمبائی(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		واپس CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == تہی)) {
		dom_string_unref(name);
		*مطابقت = false;
		واپس CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*مطابقت = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	اگر (*مطابقت == false) {
		ثابت حرف *vdata = lwc_string_ڈیٹا(قدر);
		ثابت حرف *آغاز = (ثابت حرف *) dom_string_ڈیٹا(atr_val);
		size_t len = dom_string_byte_لمبائی(atr_val);
		ثابت حرف *آخری_آغاز = آغاز + len - vlen;

		اگر (len >= vlen) {
			جب (آغاز <= آخری_آغاز) {
				اگر (strncasecmp(آغاز, vdata,
						vlen) == 0) {
					*مطابقت = true;
					توڑو;
				}

				آغاز++;
			}
		}
	}

	dom_string_unref(atr_val);

	واپس CSS_OK;
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
css_error گرہ_is_جڑ(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *والد;
	dom_گرہ_type type;
	dom_exception err;

	err = dom_گرہ_get_والد_گرہ(n, &والد);
	اگر (err != DOM_NO_ERR) {
		واپس CSS_NOMEM;
	}

	اگر (والد != تہی) {
		err = dom_گرہ_get_گرہ_type(والد, &type);

		dom_گرہ_unref(والد);

		اگر (err != DOM_NO_ERR)
			واپس CSS_NOMEM;

		اگر (type != DOM_DOCUMENT_گرہ) {
			*مطابقت = false;
			واپس CSS_OK;
		}
	}

	*مطابقت = true;

	واپس CSS_OK;
}

ساکن صحیح
گرہ_گنتی_siblings_check(dom_گرہ *گرہ,
			  bool check_name,
			  dom_string *name)
{
	dom_گرہ_type type;
	صحیح ret = 0;
	dom_exception exc;

	اگر (گرہ == تہی)
		واپس 0;

	exc = dom_گرہ_get_گرہ_type(گرہ, &type);
	اگر ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_گرہ)) {
		واپس 0;
	}

	اگر (check_name) {
		dom_string *گرہ_name = تہی;
		exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);

		اگر ((exc == DOM_NO_ERR) && (گرہ_name != تہی)) {

			اگر (dom_string_caseless_isequal(name,
							گرہ_name)) {
				ret = 1;
			}
			dom_string_unref(گرہ_name);
		}
	} ورنہ {
		ret = 1;
	}

	واپس ret;
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
css_error گرہ_گنتی_siblings(خالی *pw, خالی *n, bool same_name,
		bool after, int32_t *گنتی)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *گرہ_name = تہی;

	اگر (same_name) {
		dom_گرہ *گرہ = n;
		exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);
		اگر ((exc != DOM_NO_ERR) || (گرہ_name == تہی)) {
			واپس CSS_NOMEM;
		}
	}

	اگر (after) {
		dom_گرہ *گرہ = dom_گرہ_ref(n);
		dom_گرہ *اگلا;

		کرو {
			exc = dom_گرہ_get_اگلا_sibling(گرہ, &اگلا);
			اگر ((exc != DOM_NO_ERR))
				توڑو;

			dom_گرہ_unref(گرہ);
			گرہ = اگلا;

			cnt += گرہ_گنتی_siblings_check(گرہ, same_name, گرہ_name);
		} جب (گرہ != تہی);
	} ورنہ {
		dom_گرہ *گرہ = dom_گرہ_ref(n);
		dom_گرہ *اگلا;

		کرو {
			exc = dom_گرہ_get_پچھلا_sibling(گرہ, &اگلا);
			اگر ((exc != DOM_NO_ERR))
				توڑو;

			dom_گرہ_unref(گرہ);
			گرہ = اگلا;

			cnt += گرہ_گنتی_siblings_check(گرہ, same_name, گرہ_name);

		} جب (گرہ != تہی);
	}

	اگر (گرہ_name != تہی) {
		dom_string_unref(گرہ_name);
	}

	*گنتی = cnt;
	واپس CSS_OK;
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
css_error گرہ_is_خالی_خانے(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	dom_گرہ *n = گرہ, *اگلا;
	dom_exception err;

	*مطابقت = true;

	err = dom_گرہ_get_پہلا_بچہ(n, &n);
	اگر (err != DOM_NO_ERR) {
		واپس CSS_BADPARM;
	}

	جب (n != تہی) {
		dom_گرہ_type ntype;
		err = dom_گرہ_get_گرہ_type(n, &ntype);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_BADPARM;
		}

		اگر (ntype == DOM_ELEMENT_گرہ ||
		    ntype == DOM_متن_گرہ) {
			*مطابقت = false;
			dom_گرہ_unref(n);
			توڑو;
		}

		err = dom_گرہ_get_اگلا_sibling(n, &اگلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			واپس CSS_BADPARM;
		}
		dom_گرہ_unref(n);
		n = اگلا;
	}

	واپس CSS_OK;
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
css_error گرہ_is_link(خالی *pw, خالی *n, bool *مطابقت)
{
	dom_گرہ *گرہ = n;
	dom_exception exc;
	dom_string *گرہ_name = تہی;

	exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);
	اگر ((exc != DOM_NO_ERR) || (گرہ_name == تہی)) {
		واپس CSS_NOMEM;
	}

	اگر (dom_string_caseless_lwc_isequal(گرہ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(گرہ, corestring_dom_href,
				&has_href);
		اگر ((exc == DOM_NO_ERR) && (has_href)) {
			*مطابقت = true;
		} ورنہ {
			*مطابقت = false;
		}
	} ورنہ {
		*مطابقت = false;
	}
	dom_string_unref(گرہ_name);

	واپس CSS_OK;
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
css_error گرہ_is_دیکھا(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت ساخت url_ڈیٹا *ڈیٹا;

	dom_exception exc;
	dom_گرہ *n = گرہ;
	dom_string *s = تہی;

	*مطابقت = false;

	exc = dom_گرہ_get_گرہ_name(n, &s);
	اگر ((exc != DOM_NO_ERR) || (s == تہی)) {
		واپس CSS_NOMEM;
	}

	اگر (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		واپس CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = تہی;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	اگر ((exc != DOM_NO_ERR) || (s == تہی)) {
		/* Can't be visited; not got a URL */
		واپس CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->بنیاد_url, dom_string_ڈیٹا(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	اگر (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		واپس CSS_NOMEM;
	}

	ڈیٹا = urldb_get_url_ڈیٹا(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	اگر (ڈیٹا != تہی && ڈیٹا->visits > 0)
		*مطابقت = true;

	nsurl_unref(url);

	واپس CSS_OK;
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
css_error گرہ_is_hover(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support hovering */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_فعال(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support active nodes */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_focus(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support focussed nodes */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_enabled(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support enabled nodes */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_disabled(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support disabled nodes */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_checked(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support checked nodes */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_ہدف(خالی *pw, خالی *گرہ, bool *مطابقت)
{
	/** \todo Support target */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error گرہ_is_lang(خالی *pw, خالی *گرہ,
		lwc_string *lang, bool *مطابقت)
{
	/** \todo Support languages */

	*مطابقت = false;

	واپس CSS_OK;
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
css_error ua_default_for_property(خالی *pw, uint32_t property, css_hint *hint)
{
	اگر (property == CSS_PROP_COLOR) {
		hint->ڈیٹا.color = 0xff000000;
		hint->حالت_2 = CSS_COLOR_COLOR;
	} ورنہ اگر (property == CSS_PROP_FONT_FAMILY) {
		hint->ڈیٹا.لڑیاں = تہی;
		انتخاب (nsoption_int(font_default)) {
		حالت PLOT_FONT_FAMILY_SANS_SERIF:
			hint->حالت_2 = CSS_FONT_FAMILY_SANS_SERIF;
			توڑو;
		حالت PLOT_FONT_FAMILY_SERIF:
			hint->حالت_2 = CSS_FONT_FAMILY_SERIF;
			توڑو;
		حالت PLOT_FONT_FAMILY_MONOSPACE:
			hint->حالت_2 = CSS_FONT_FAMILY_MONOSPACE;
			توڑو;
		حالت PLOT_FONT_FAMILY_CURSIVE:
			hint->حالت_2 = CSS_FONT_FAMILY_CURSIVE;
			توڑو;
		حالت PLOT_FONT_FAMILY_FANTASY:
			hint->حالت_2 = CSS_FONT_FAMILY_FANTASY;
			توڑو;
		}
	} ورنہ اگر (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->ڈیٹا.لڑیاں = تہی;
		hint->حالت_2 = CSS_QUOTES_NONE;
	} ورنہ اگر (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->ڈیٹا.لڑیاں = تہی;
		hint->حالت_2 = 0;
	} ورنہ {
		واپس CSS_INVALID;
	}

	واپس CSS_OK;
}

css_error سیٹ_libcss_گرہ_ڈیٹا(خالی *pw, خالی *گرہ, خالی *libcss_گرہ_ڈیٹا)
{
	dom_گرہ *n = گرہ;
	dom_exception err;
	خالی *old_گرہ_ڈیٹا;

	/* Set this node's node data */
	err = dom_گرہ_سیٹ_user_ڈیٹا(n,
			corestring_dom_ns_کلید_libcss_گرہ_ڈیٹا,
			libcss_گرہ_ڈیٹا, nscss_dom_user_ڈیٹا_handler,
			(خالی *) &old_گرہ_ڈیٹا);
	اگر (err != DOM_NO_ERR) {
		واپس CSS_NOMEM;
	}

	أكد(old_گرہ_ڈیٹا == تہی);

	واپس CSS_OK;
}

css_error get_libcss_گرہ_ڈیٹا(خالی *pw, خالی *گرہ, خالی **libcss_گرہ_ڈیٹا)
{
	dom_گرہ *n = گرہ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_گرہ_get_user_ڈیٹا(n,
			corestring_dom_ns_کلید_libcss_گرہ_ڈیٹا,
			libcss_گرہ_ڈیٹا);
	اگر (err != DOM_NO_ERR) {
		واپس CSS_NOMEM;
	}

	واپس CSS_OK;
}
