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

#ضمن <assert.h>
#ضمن <string.h>
#ضمن <strings.h>

#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/plot_style.h"
#ضمن "netsurf/url_db.h"
#ضمن "desktop/system_colour.h"

#ضمن "css/internal.h"
#ضمن "css/hints.h"
#ضمن "css/select.h"

ساكن css_error غوټه_name(فراغ *pw, فراغ *غوټه, css_qname *qname);
ساكن css_error غوټه_classes(فراغ *pw, فراغ *غوټه,
		lwc_string ***classes, uint32_t *n_classes);
ساكن css_error غوټه_id(فراغ *pw, فراغ *غوټه, lwc_string **id);
ساكن css_error named_پلار_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **پلار);
ساكن css_error named_sibling_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error named_generic_sibling_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error پلار_غوټه(فراغ *pw, فراغ *غوټه, فراغ **پلار);
ساكن css_error sibling_غوټه(فراغ *pw, فراغ *غوټه, فراغ **sibling);
ساكن css_error غوټه_has_name(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, bool *سمون);
ساكن css_error غوټه_has_class(فراغ *pw, فراغ *غوټه,
		lwc_string *name, bool *سمون);
ساكن css_error غوټه_has_id(فراغ *pw, فراغ *غوټه,
		lwc_string *name, bool *سمون);
ساكن css_error غوټه_has_attribute(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, bool *سمون);
ساكن css_error غوټه_has_attribute_equal(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_has_attribute_dashmatch(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_has_attribute_includes(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_has_attribute_مختاړی(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_has_attribute_suffix(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_has_attribute_substring(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون);
ساكن css_error غوټه_is_ریښه(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_شمېر_siblings(فراغ *pw, فراغ *غوټه,
		bool same_name, bool after, int32_t *شمېر);
ساكن css_error غوټه_is_تشه(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_link(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_hover(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_فعال(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_focus(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_enabled(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_disabled(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_checked(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_موخه(فراغ *pw, فراغ *غوټه, bool *سمون);
ساكن css_error غوټه_is_lang(فراغ *pw, فراغ *غوټه,
		lwc_string *lang, bool *سمون);
ساكن css_error ua_default_for_property(فراغ *pw, uint32_t property,
		css_hint *hint);
ساكن css_error سېټ_libcss_غوټه_معلومات(فراغ *pw, فراغ *غوټه,
		فراغ *libcss_غوټه_معلومات);
ساكن css_error get_libcss_غوټه_معلومات(فراغ *pw, فراغ *غوټه,
		فراغ **libcss_غوټه_معلومات);

/**
 * Selection callback table for libcss
 */
ساكن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_نسخه_1,

	غوټه_name,
	غوټه_classes,
	غوټه_id,
	named_ancestor_غوټه,
	named_پلار_غوټه,
	named_sibling_غوټه,
	named_generic_sibling_غوټه,
	پلار_غوټه,
	sibling_غوټه,
	غوټه_has_name,
	غوټه_has_class,
	غوټه_has_id,
	غوټه_has_attribute,
	غوټه_has_attribute_equal,
	غوټه_has_attribute_dashmatch,
	غوټه_has_attribute_includes,
	غوټه_has_attribute_مختاړی,
	غوټه_has_attribute_suffix,
	غوټه_has_attribute_substring,
	غوټه_is_ریښه,
	غوټه_شمېر_siblings,
	غوټه_is_تشه,
	غوټه_is_link,
	غوټه_is_لیدل_شوی,
	غوټه_is_hover,
	غوټه_is_فعال,
	غوټه_is_focus,
	غوټه_is_enabled,
	غوټه_is_disabled,
	غوټه_is_checked,
	غوټه_is_موخه,
	غوټه_is_lang,
	غوټه_presentational_hint,
	ua_default_for_property,
	سېټ_libcss_غوټه_معلومات,
	get_libcss_غوټه_معلومات,
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
css_stylesheet *nscss_create_inline_style(ثابت uint8_t *معلومات, size_t len,
		ثابت حرف *charset, ثابت حرف *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_نسخه = CSS_STYLESHEET_PARAMS_نسخه_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = عدم;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = عدم;
	params.import = عدم;
	params.import_pw = عدم;
	params.color = ns_system_رنګ;
	params.color_pw = عدم;
	params.font = عدم;
	params.font_pw = عدم;

	error = css_stylesheet_create(&params, &sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		أرجع عدم;
	}

	error = css_stylesheet_append_معلومات(sheet, معلومات, len);
	إذا (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	error = css_stylesheet_معلومات_بشپړ(sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	أرجع sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساكن فراغ nscss_dom_user_معلومات_handler(dom_غوټه_operation operation,
		dom_string *کلی, فراغ *معلومات, بنية dom_غوټه *src,
		بنية dom_غوټه *dst)
{
	css_error error;

	إذا (dom_string_isequal(corestring_dom_ns_کلی_libcss_غوټه_معلومات,
			کلی) == false || معلومات == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_غوټه_CLONED:
		error = css_libcss_غوټه_معلومات_handler(&selection_handler,
				CSS_غوټه_CLONED,
				عدم, src, dst, معلومات);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		اكسر;

	حالة DOM_غوټه_RENAMED:
		error = css_libcss_غوټه_معلومات_handler(&selection_handler,
				CSS_غوټه_MODIFIED,
				عدم, src, عدم, معلومات);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		اكسر;

	حالة DOM_غوټه_IMPORTED:
	حالة DOM_غوټه_ADOPTED:
	حالة DOM_غوټه_DELETED:
		error = css_libcss_غوټه_معلومات_handler(&selection_handler,
				CSS_غوټه_DELETED,
				عدم, src, عدم, معلومات);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		اكسر;

	افتراضي:
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_غوټه *n,
		ثابت css_media *media,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	صحيح pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	إذا (error != CSS_OK || styles == عدم) {
		/* Failed selecting partial style -- bail out */
		أرجع عدم;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	إذا (ctx->پلار_style != عدم) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->پلار_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		إذا (error != CSS_OK) {
			css_select_results_destroy(styles);
			أرجع عدم;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	لكل (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_شمېر;
			pseudo_element++) {

		إذا (pseudo_element == CSS_PSEUDO_ELEMENT_لومړی_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_لومړی_کرښه)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			استمر;

		إذا (styles->styles[pseudo_element] == عدم)
			/* There were no rules concerning this pseudo element */
			استمر;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		إذا (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			أرجع عدم;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	أرجع styles;
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
		ثابت css_computed_style *پلار)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	إذا (error != CSS_OK) {
		أرجع عدم;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(پلار, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	إذا (error != CSS_OK) {
		css_computed_style_destroy(composed);
		أرجع عدم;
	}

	أرجع composed;
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
css_error غوټه_name(فراغ *pw, فراغ *غوټه, css_qname *qname)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_exception err;

	err = dom_غوټه_get_غوټه_name(n, &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	qname->ns = عدم;

	err = dom_string_intern(name, &qname->name);
	إذا (err != DOM_NO_ERR) {
		dom_string_unref(name);
		أرجع CSS_NOMEM;
	}

	dom_string_unref(name);

	أرجع CSS_OK;
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
css_error غوټه_classes(فراغ *pw, فراغ *غوټه,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_غوټه *n = غوټه;
	dom_exception err;

	*classes = عدم;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	أرجع CSS_OK;
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
css_error غوټه_id(فراغ *pw, فراغ *غوټه, lwc_string **id)
{
	dom_غوټه *n = غوټه;
	dom_string *attr;
	dom_exception err;

	*id = عدم;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	إذا (attr != عدم) {
		err = dom_string_intern(attr, id);
		إذا (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			أرجع CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	أرجع CSS_OK;
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
css_error named_ancestor_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **ancestor)
{
	dom_element_named_ancestor_غوټه(غوټه, qname->name,
			(بنية dom_element **)ancestor);
	dom_غوټه_unref(*ancestor);

	أرجع CSS_OK;
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
css_error named_پلار_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **پلار)
{
	dom_element_named_پلار_غوټه(غوټه, qname->name,
			(بنية dom_element **)پلار);
	dom_غوټه_unref(*پلار);

	أرجع CSS_OK;
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
css_error named_sibling_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_غوټه *n = غوټه;
	dom_غوټه *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_غوټه_get_پخوانی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_غوټه_type type;

		err = dom_غوټه_get_غوټه_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_غوټه)
			اكسر;

		err = dom_غوټه_get_پخوانی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		dom_غوټه_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		dom_string *name;

		err = dom_غوټه_get_غوټه_name(n, &name);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		dom_غوټه_unref(n);

		إذا (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	أرجع CSS_OK;
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
css_error named_generic_sibling_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_غوټه *n = غوټه;
	dom_غوټه *prev;
	dom_exception err;

	*sibling = عدم;

	err = dom_غوټه_get_پخوانی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_غوټه_type type;
		dom_string *name;

		err = dom_غوټه_get_غوټه_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_غوټه) {
			err = dom_غوټه_get_غوټه_name(n, &name);
			إذا (err != DOM_NO_ERR) {
				dom_غوټه_unref(n);
				أرجع CSS_OK;
			}

			إذا (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_غوټه_unref(n);
				*sibling = n;
				اكسر;
			}
			dom_string_unref(name);
		}

		err = dom_غوټه_get_پخوانی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		dom_غوټه_unref(n);
		n = prev;
	}

	أرجع CSS_OK;
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
css_error پلار_غوټه(فراغ *pw, فراغ *غوټه, فراغ **پلار)
{
	dom_element_پلار_غوټه(غوټه, (بنية dom_element **)پلار);
	dom_غوټه_unref(*پلار);

	أرجع CSS_OK;
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
css_error sibling_غوټه(فراغ *pw, فراغ *غوټه, فراغ **sibling)
{
	dom_غوټه *n = غوټه;
	dom_غوټه *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_غوټه_get_پخوانی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_غوټه_type type;

		err = dom_غوټه_get_غوټه_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_غوټه)
			اكسر;

		err = dom_غوټه_get_پخوانی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_OK;
		}

		dom_غوټه_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		/** \todo Sort out reference counting */
		dom_غوټه_unref(n);

		*sibling = n;
	}

	أرجع CSS_OK;
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
css_error غوټه_has_name(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, bool *سمون)
{
	nscss_select_ctx *ctx = pw;
	dom_غوټه *n = غوټه;

	إذا (lwc_string_isequal(qname->name, ctx->universal, سمون) ==
			lwc_error_ok && *سمون == false) {
		dom_string *name;
		dom_exception err;

		err = dom_غوټه_get_غوټه_name(n, &name);
		إذا (err != DOM_NO_ERR)
			أرجع CSS_OK;

		/* Element names are case insensitive in HTML */
		*سمون = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	أرجع CSS_OK;
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
css_error غوټه_has_class(فراغ *pw, فراغ *غوټه,
		lwc_string *name, bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, سمون);

	أكد(err == DOM_NO_ERR);

	أرجع CSS_OK;
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
css_error غوټه_has_id(فراغ *pw, فراغ *غوټه,
		lwc_string *name, bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *attr;
	dom_exception err;

	*سمون = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	إذا (attr != عدم) {
		*سمون = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	أرجع CSS_OK;
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
css_error غوټه_has_attribute(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_معلومات(qname->name),
			lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_has_attribute(n, name, سمون);
	إذا (err != DOM_NO_ERR) {
		dom_string_unref(name);
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_equal(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_اوږدتیا(ارزښت);

	إذا (vlen == 0) {
		*سمون = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	*سمون = dom_string_caseless_lwc_isequal(atr_val, ارزښت);

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_dashmatch(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_اوږدتیا(ارزښت);

	إذا (vlen == 0) {
		*سمون = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*سمون = dom_string_caseless_lwc_isequal(atr_val, ارزښت);

	/* check for dashmatch */
	إذا (*سمون == false) {
		ثابت حرف *vdata = lwc_string_معلومات(ارزښت);
		ثابت حرف *معلومات = (ثابت حرف *) dom_string_معلومات(atr_val);
		size_t len = dom_string_byte_اوږدتیا(atr_val);

		إذا (len > vlen && معلومات[vlen] == '-' &&
		    strncasecmp(معلومات, vdata, vlen) == 0) {
				*سمون = true;
		}
	}

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_includes(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_اوږدتیا(ارزښت);
	ثابت حرف *p;
	ثابت حرف *پیل;
	ثابت حرف *پای_ټکی;

	*سمون = false;

	إذا (vlen == 0) {
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	پیل = (ثابت حرف *) dom_string_معلومات(atr_val);
	پای_ټکی = پیل + dom_string_byte_اوږدتیا(atr_val);

	لكل (p = پیل; p <= پای_ټکی; p++) {
		إذا (*p == ' ' || *p == '\0') {
			إذا ((size_t) (p - پیل) == vlen &&
			    strncasecmp(پیل,
					lwc_string_معلومات(ارزښت),
					vlen) == 0) {
				*سمون = true;
				اكسر;
			}

			پیل = p + 1;
		}
	}

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_مختاړی(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_اوږدتیا(ارزښت);

	إذا (vlen == 0) {
		*سمون = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*سمون = dom_string_caseless_lwc_isequal(atr_val, ارزښت);

	/* check for prefix match */
	إذا (*سمون == false) {
		ثابت حرف *معلومات = (ثابت حرف *) dom_string_معلومات(atr_val);
		size_t len = dom_string_byte_اوږدتیا(atr_val);

		إذا ((len >= vlen) &&
		    (strncasecmp(معلومات, lwc_string_معلومات(ارزښت), vlen) == 0)) {
			*سمون = true;
		}
	}

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_suffix(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_اوږدتیا(ارزښت);

	إذا (vlen == 0) {
		*سمون = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*سمون = dom_string_caseless_lwc_isequal(atr_val, ارزښت);

	/* check for prefix match */
	إذا (*سمون == false) {
		ثابت حرف *معلومات = (ثابت حرف *) dom_string_معلومات(atr_val);
		size_t len = dom_string_byte_اوږدتیا(atr_val);

		ثابت حرف *پیل = (حرف *) معلومات + len - vlen;

		إذا ((len >= vlen) &&
		    (strncasecmp(پیل, lwc_string_معلومات(ارزښت), vlen) == 0)) {
			*سمون = true;
		}


	}

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_has_attribute_substring(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, lwc_string *ارزښت,
		bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_اوږدتیا(ارزښت);

	إذا (vlen == 0) {
		*سمون = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_معلومات(qname->name),
		lwc_string_اوږدتیا(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*سمون = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*سمون = dom_string_caseless_lwc_isequal(atr_val, ارزښت);

	/* check for prefix match */
	إذا (*سمون == false) {
		ثابت حرف *vdata = lwc_string_معلومات(ارزښت);
		ثابت حرف *پیل = (ثابت حرف *) dom_string_معلومات(atr_val);
		size_t len = dom_string_byte_اوږدتیا(atr_val);
		ثابت حرف *وروستی_پیل = پیل + len - vlen;

		إذا (len >= vlen) {
			طالما (پیل <= وروستی_پیل) {
				إذا (strncasecmp(پیل, vdata,
						vlen) == 0) {
					*سمون = true;
					اكسر;
				}

				پیل++;
			}
		}
	}

	dom_string_unref(atr_val);

	أرجع CSS_OK;
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
css_error غوټه_is_ریښه(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	dom_غوټه *n = غوټه;
	dom_غوټه *پلار;
	dom_غوټه_type type;
	dom_exception err;

	err = dom_غوټه_get_پلار_غوټه(n, &پلار);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	إذا (پلار != عدم) {
		err = dom_غوټه_get_غوټه_type(پلار, &type);

		dom_غوټه_unref(پلار);

		إذا (err != DOM_NO_ERR)
			أرجع CSS_NOMEM;

		إذا (type != DOM_DOCUMENT_غوټه) {
			*سمون = false;
			أرجع CSS_OK;
		}
	}

	*سمون = true;

	أرجع CSS_OK;
}

ساكن صحيح
غوټه_شمېر_siblings_check(dom_غوټه *غوټه,
			  bool check_name,
			  dom_string *name)
{
	dom_غوټه_type type;
	صحيح ret = 0;
	dom_exception exc;

	إذا (غوټه == عدم)
		أرجع 0;

	exc = dom_غوټه_get_غوټه_type(غوټه, &type);
	إذا ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_غوټه)) {
		أرجع 0;
	}

	إذا (check_name) {
		dom_string *غوټه_name = عدم;
		exc = dom_غوټه_get_غوټه_name(غوټه, &غوټه_name);

		إذا ((exc == DOM_NO_ERR) && (غوټه_name != عدم)) {

			إذا (dom_string_caseless_isequal(name,
							غوټه_name)) {
				ret = 1;
			}
			dom_string_unref(غوټه_name);
		}
	} وإلا {
		ret = 1;
	}

	أرجع ret;
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
css_error غوټه_شمېر_siblings(فراغ *pw, فراغ *n, bool same_name,
		bool after, int32_t *شمېر)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *غوټه_name = عدم;

	إذا (same_name) {
		dom_غوټه *غوټه = n;
		exc = dom_غوټه_get_غوټه_name(غوټه, &غوټه_name);
		إذا ((exc != DOM_NO_ERR) || (غوټه_name == عدم)) {
			أرجع CSS_NOMEM;
		}
	}

	إذا (after) {
		dom_غوټه *غوټه = dom_غوټه_ref(n);
		dom_غوټه *بل;

		افعل {
			exc = dom_غوټه_get_بل_sibling(غوټه, &بل);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_غوټه_unref(غوټه);
			غوټه = بل;

			cnt += غوټه_شمېر_siblings_check(غوټه, same_name, غوټه_name);
		} طالما (غوټه != عدم);
	} وإلا {
		dom_غوټه *غوټه = dom_غوټه_ref(n);
		dom_غوټه *بل;

		افعل {
			exc = dom_غوټه_get_پخوانی_sibling(غوټه, &بل);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_غوټه_unref(غوټه);
			غوټه = بل;

			cnt += غوټه_شمېر_siblings_check(غوټه, same_name, غوټه_name);

		} طالما (غوټه != عدم);
	}

	إذا (غوټه_name != عدم) {
		dom_string_unref(غوټه_name);
	}

	*شمېر = cnt;
	أرجع CSS_OK;
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
css_error غوټه_is_تشه(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	dom_غوټه *n = غوټه, *بل;
	dom_exception err;

	*سمون = true;

	err = dom_غوټه_get_لومړی_ماشوم(n, &n);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_BADPARM;
	}

	طالما (n != عدم) {
		dom_غوټه_type ntype;
		err = dom_غوټه_get_غوټه_type(n, &ntype);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_BADPARM;
		}

		إذا (ntype == DOM_ELEMENT_غوټه ||
		    ntype == DOM_متن_غوټه) {
			*سمون = false;
			dom_غوټه_unref(n);
			اكسر;
		}

		err = dom_غوټه_get_بل_sibling(n, &بل);
		إذا (err != DOM_NO_ERR) {
			dom_غوټه_unref(n);
			أرجع CSS_BADPARM;
		}
		dom_غوټه_unref(n);
		n = بل;
	}

	أرجع CSS_OK;
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
css_error غوټه_is_link(فراغ *pw, فراغ *n, bool *سمون)
{
	dom_غوټه *غوټه = n;
	dom_exception exc;
	dom_string *غوټه_name = عدم;

	exc = dom_غوټه_get_غوټه_name(غوټه, &غوټه_name);
	إذا ((exc != DOM_NO_ERR) || (غوټه_name == عدم)) {
		أرجع CSS_NOMEM;
	}

	إذا (dom_string_caseless_lwc_isequal(غوټه_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(غوټه, corestring_dom_href,
				&has_href);
		إذا ((exc == DOM_NO_ERR) && (has_href)) {
			*سمون = true;
		} وإلا {
			*سمون = false;
		}
	} وإلا {
		*سمون = false;
	}
	dom_string_unref(غوټه_name);

	أرجع CSS_OK;
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
css_error غوټه_is_لیدل_شوی(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت بنية url_معلومات *معلومات;

	dom_exception exc;
	dom_غوټه *n = غوټه;
	dom_string *s = عدم;

	*سمون = false;

	exc = dom_غوټه_get_غوټه_name(n, &s);
	إذا ((exc != DOM_NO_ERR) || (s == عدم)) {
		أرجع CSS_NOMEM;
	}

	إذا (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		أرجع CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = عدم;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	إذا ((exc != DOM_NO_ERR) || (s == عدم)) {
		/* Can't be visited; not got a URL */
		أرجع CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->بنسټ_url, dom_string_معلومات(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	إذا (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		أرجع CSS_NOMEM;
	}

	معلومات = urldb_get_url_معلومات(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	إذا (معلومات != عدم && معلومات->visits > 0)
		*سمون = true;

	nsurl_unref(url);

	أرجع CSS_OK;
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
css_error غوټه_is_hover(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support hovering */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_فعال(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support active nodes */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_focus(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support focussed nodes */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_enabled(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support enabled nodes */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_disabled(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support disabled nodes */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_checked(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support checked nodes */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_موخه(فراغ *pw, فراغ *غوټه, bool *سمون)
{
	/** \todo Support target */

	*سمون = false;

	أرجع CSS_OK;
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
css_error غوټه_is_lang(فراغ *pw, فراغ *غوټه,
		lwc_string *lang, bool *سمون)
{
	/** \todo Support languages */

	*سمون = false;

	أرجع CSS_OK;
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
css_error ua_default_for_property(فراغ *pw, uint32_t property, css_hint *hint)
{
	إذا (property == CSS_PROP_COLOR) {
		hint->معلومات.color = 0xff000000;
		hint->حالت = CSS_COLOR_COLOR;
	} وإلا إذا (property == CSS_PROP_FONT_FAMILY) {
		hint->معلومات.مزي_ګانې = عدم;
		بدل (nsoption_int(font_default)) {
		حالة PLOT_FONT_FAMILY_SANS_SERIF:
			hint->حالت = CSS_FONT_FAMILY_SANS_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_SERIF:
			hint->حالت = CSS_FONT_FAMILY_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_MONOSPACE:
			hint->حالت = CSS_FONT_FAMILY_MONOSPACE;
			اكسر;
		حالة PLOT_FONT_FAMILY_CURSIVE:
			hint->حالت = CSS_FONT_FAMILY_CURSIVE;
			اكسر;
		حالة PLOT_FONT_FAMILY_FANTASY:
			hint->حالت = CSS_FONT_FAMILY_FANTASY;
			اكسر;
		}
	} وإلا إذا (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->معلومات.مزي_ګانې = عدم;
		hint->حالت = CSS_QUOTES_NONE;
	} وإلا إذا (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->معلومات.مزي_ګانې = عدم;
		hint->حالت = 0;
	} وإلا {
		أرجع CSS_INVALID;
	}

	أرجع CSS_OK;
}

css_error سېټ_libcss_غوټه_معلومات(فراغ *pw, فراغ *غوټه, فراغ *libcss_غوټه_معلومات)
{
	dom_غوټه *n = غوټه;
	dom_exception err;
	فراغ *old_غوټه_معلومات;

	/* Set this node's node data */
	err = dom_غوټه_سېټ_user_معلومات(n,
			corestring_dom_ns_کلی_libcss_غوټه_معلومات,
			libcss_غوټه_معلومات, nscss_dom_user_معلومات_handler,
			(فراغ *) &old_غوټه_معلومات);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أكد(old_غوټه_معلومات == عدم);

	أرجع CSS_OK;
}

css_error get_libcss_غوټه_معلومات(فراغ *pw, فراغ *غوټه, فراغ **libcss_غوټه_معلومات)
{
	dom_غوټه *n = غوټه;
	dom_exception err;

	/* Get this node's node data */
	err = dom_غوټه_get_user_معلومات(n,
			corestring_dom_ns_کلی_libcss_غوټه_معلومات,
			libcss_غوټه_معلومات);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أرجع CSS_OK;
}
