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

ساكن css_error گرہ_name(فراغ *pw, فراغ *گرہ, css_qname *qname);
ساكن css_error گرہ_classes(فراغ *pw, فراغ *گرہ,
		lwc_string ***classes, uint32_t *n_classes);
ساكن css_error گرہ_id(فراغ *pw, فراغ *گرہ, lwc_string **id);
ساكن css_error named_ماپا_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **ماپا);
ساكن css_error named_sibling_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error named_generic_sibling_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error ماپا_گرہ(فراغ *pw, فراغ *گرہ, فراغ **ماپا);
ساكن css_error sibling_گرہ(فراغ *pw, فراغ *گرہ, فراغ **sibling);
ساكن css_error گرہ_has_name(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, bool *میل);
ساكن css_error گرہ_has_class(فراغ *pw, فراغ *گرہ,
		lwc_string *name, bool *میل);
ساكن css_error گرہ_has_id(فراغ *pw, فراغ *گرہ,
		lwc_string *name, bool *میل);
ساكن css_error گرہ_has_attribute(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, bool *میل);
ساكن css_error گرہ_has_attribute_equal(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_has_attribute_dashmatch(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_has_attribute_includes(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_has_attribute_اگیتر(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_has_attribute_suffix(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_has_attribute_substring(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل);
ساكن css_error گرہ_is_جڑ(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_گنتی_siblings(فراغ *pw, فراغ *گرہ,
		bool same_name, bool after, int32_t *گنتی);
ساكن css_error گرہ_is_خالی_تھاواں(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_link(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_hover(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_سرگرم(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_focus(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_enabled(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_disabled(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_checked(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_نشانہ(فراغ *pw, فراغ *گرہ, bool *میل);
ساكن css_error گرہ_is_lang(فراغ *pw, فراغ *گرہ,
		lwc_string *lang, bool *میل);
ساكن css_error ua_default_for_property(فراغ *pw, uint32_t property,
		css_hint *hint);
ساكن css_error سیٹ_libcss_گرہ_انکڑے(فراغ *pw, فراغ *گرہ,
		فراغ *libcss_گرہ_انکڑے);
ساكن css_error get_libcss_گرہ_انکڑے(فراغ *pw, فراغ *گرہ,
		فراغ **libcss_گرہ_انکڑے);

/**
 * Selection callback table for libcss
 */
ساكن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_نسخہ_1,

	گرہ_name,
	گرہ_classes,
	گرہ_id,
	named_ancestor_گرہ,
	named_ماپا_گرہ,
	named_sibling_گرہ,
	named_generic_sibling_گرہ,
	ماپا_گرہ,
	sibling_گرہ,
	گرہ_has_name,
	گرہ_has_class,
	گرہ_has_id,
	گرہ_has_attribute,
	گرہ_has_attribute_equal,
	گرہ_has_attribute_dashmatch,
	گرہ_has_attribute_includes,
	گرہ_has_attribute_اگیتر,
	گرہ_has_attribute_suffix,
	گرہ_has_attribute_substring,
	گرہ_is_جڑ,
	گرہ_گنتی_siblings,
	گرہ_is_خالی_تھاواں,
	گرہ_is_link,
	گرہ_is_ویکھیا,
	گرہ_is_hover,
	گرہ_is_سرگرم,
	گرہ_is_focus,
	گرہ_is_enabled,
	گرہ_is_disabled,
	گرہ_is_checked,
	گرہ_is_نشانہ,
	گرہ_is_lang,
	گرہ_presentational_hint,
	ua_default_for_property,
	سیٹ_libcss_گرہ_انکڑے,
	get_libcss_گرہ_انکڑے,
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
css_stylesheet *nscss_create_inline_style(ثابت uint8_t *انکڑے, size_t len,
		ثابت حرف *charset, ثابت حرف *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_نسخہ = CSS_STYLESHEET_PARAMS_نسخہ_1;
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
	params.color = ns_system_رنگ;
	params.color_pw = عدم;
	params.font = عدم;
	params.font_pw = عدم;

	error = css_stylesheet_create(&params, &sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		أرجع عدم;
	}

	error = css_stylesheet_append_انکڑے(sheet, انکڑے, len);
	إذا (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	error = css_stylesheet_انکڑے_پورا(sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	أرجع sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساكن فراغ nscss_dom_user_انکڑے_handler(dom_گرہ_operation operation,
		dom_string *کلید, فراغ *انکڑے, بنية dom_گرہ *src,
		بنية dom_گرہ *dst)
{
	css_error error;

	إذا (dom_string_isequal(corestring_dom_ns_کلید_libcss_گرہ_انکڑے,
			کلید) == false || انکڑے == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_گرہ_CLONED:
		error = css_libcss_گرہ_انکڑے_handler(&selection_handler,
				CSS_گرہ_CLONED,
				عدم, src, dst, انکڑے);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		اكسر;

	حالة DOM_گرہ_RENAMED:
		error = css_libcss_گرہ_انکڑے_handler(&selection_handler,
				CSS_گرہ_MODIFIED,
				عدم, src, عدم, انکڑے);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		اكسر;

	حالة DOM_گرہ_IMPORTED:
	حالة DOM_گرہ_ADOPTED:
	حالة DOM_گرہ_DELETED:
		error = css_libcss_گرہ_انکڑے_handler(&selection_handler,
				CSS_گرہ_DELETED,
				عدم, src, عدم, انکڑے);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_گرہ *n,
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
	إذا (ctx->ماپا_style != عدم) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->ماپا_style,
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
			pseudo_element < CSS_PSEUDO_ELEMENT_گنتی;
			pseudo_element++) {

		إذا (pseudo_element == CSS_PSEUDO_ELEMENT_پہلا_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_پہلا_سطر)
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
		ثابت css_computed_style *ماپا)
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
	error = css_computed_style_compose(ماپا, partial,
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
css_error گرہ_name(فراغ *pw, فراغ *گرہ, css_qname *qname)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_exception err;

	err = dom_گرہ_get_گرہ_name(n, &name);
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
css_error گرہ_classes(فراغ *pw, فراغ *گرہ,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_گرہ *n = گرہ;
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
css_error گرہ_id(فراغ *pw, فراغ *گرہ, lwc_string **id)
{
	dom_گرہ *n = گرہ;
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
css_error named_ancestor_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **ancestor)
{
	dom_element_named_ancestor_گرہ(گرہ, qname->name,
			(بنية dom_element **)ancestor);
	dom_گرہ_unref(*ancestor);

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
css_error named_ماپا_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **ماپا)
{
	dom_element_named_ماپا_گرہ(گرہ, qname->name,
			(بنية dom_element **)ماپا);
	dom_گرہ_unref(*ماپا);

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
css_error named_sibling_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گرہ_type type;

		err = dom_گرہ_get_گرہ_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گرہ)
			اكسر;

		err = dom_گرہ_get_پچھلا_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		dom_گرہ_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		dom_string *name;

		err = dom_گرہ_get_گرہ_name(n, &name);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		dom_گرہ_unref(n);

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
css_error named_generic_sibling_گرہ(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *prev;
	dom_exception err;

	*sibling = عدم;

	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گرہ_type type;
		dom_string *name;

		err = dom_گرہ_get_گرہ_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گرہ) {
			err = dom_گرہ_get_گرہ_name(n, &name);
			إذا (err != DOM_NO_ERR) {
				dom_گرہ_unref(n);
				أرجع CSS_OK;
			}

			إذا (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_گرہ_unref(n);
				*sibling = n;
				اكسر;
			}
			dom_string_unref(name);
		}

		err = dom_گرہ_get_پچھلا_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		dom_گرہ_unref(n);
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
css_error ماپا_گرہ(فراغ *pw, فراغ *گرہ, فراغ **ماپا)
{
	dom_element_ماپا_گرہ(گرہ, (بنية dom_element **)ماپا);
	dom_گرہ_unref(*ماپا);

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
css_error sibling_گرہ(فراغ *pw, فراغ *گرہ, فراغ **sibling)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_گرہ_get_پچھلا_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گرہ_type type;

		err = dom_گرہ_get_گرہ_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گرہ)
			اكسر;

		err = dom_گرہ_get_پچھلا_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_OK;
		}

		dom_گرہ_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		/** \todo Sort out reference counting */
		dom_گرہ_unref(n);

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
css_error گرہ_has_name(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, bool *میل)
{
	nscss_select_ctx *ctx = pw;
	dom_گرہ *n = گرہ;

	إذا (lwc_string_isequal(qname->name, ctx->universal, میل) ==
			lwc_error_ok && *میل == false) {
		dom_string *name;
		dom_exception err;

		err = dom_گرہ_get_گرہ_name(n, &name);
		إذا (err != DOM_NO_ERR)
			أرجع CSS_OK;

		/* Element names are case insensitive in HTML */
		*میل = dom_string_caseless_lwc_isequal(name, qname->name);

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
css_error گرہ_has_class(فراغ *pw, فراغ *گرہ,
		lwc_string *name, bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, میل);

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
css_error گرہ_has_id(فراغ *pw, فراغ *گرہ,
		lwc_string *name, bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *attr;
	dom_exception err;

	*میل = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	إذا (attr != عدم) {
		*میل = dom_string_lwc_isequal(attr, name);

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
css_error گرہ_has_attribute(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
			lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_has_attribute(n, name, میل);
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
css_error گرہ_has_attribute_equal(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمائی(قدر);

	إذا (vlen == 0) {
		*میل = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	*میل = dom_string_caseless_lwc_isequal(atr_val, قدر);

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
css_error گرہ_has_attribute_dashmatch(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمائی(قدر);

	إذا (vlen == 0) {
		*میل = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*میل = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for dashmatch */
	إذا (*میل == false) {
		ثابت حرف *vdata = lwc_string_انکڑے(قدر);
		ثابت حرف *انکڑے = (ثابت حرف *) dom_string_انکڑے(atr_val);
		size_t len = dom_string_byte_لمائی(atr_val);

		إذا (len > vlen && انکڑے[vlen] == '-' &&
		    strncasecmp(انکڑے, vdata, vlen) == 0) {
				*میل = true;
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
css_error گرہ_has_attribute_includes(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_لمائی(قدر);
	ثابت حرف *p;
	ثابت حرف *شروع;
	ثابت حرف *سرا;

	*میل = false;

	إذا (vlen == 0) {
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	شروع = (ثابت حرف *) dom_string_انکڑے(atr_val);
	سرا = شروع + dom_string_byte_لمائی(atr_val);

	لكل (p = شروع; p <= سرا; p++) {
		إذا (*p == ' ' || *p == '\0') {
			إذا ((size_t) (p - شروع) == vlen &&
			    strncasecmp(شروع,
					lwc_string_انکڑے(قدر),
					vlen) == 0) {
				*میل = true;
				اكسر;
			}

			شروع = p + 1;
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
css_error گرہ_has_attribute_اگیتر(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمائی(قدر);

	إذا (vlen == 0) {
		*میل = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*میل = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	إذا (*میل == false) {
		ثابت حرف *انکڑے = (ثابت حرف *) dom_string_انکڑے(atr_val);
		size_t len = dom_string_byte_لمائی(atr_val);

		إذا ((len >= vlen) &&
		    (strncasecmp(انکڑے, lwc_string_انکڑے(قدر), vlen) == 0)) {
			*میل = true;
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
css_error گرہ_has_attribute_suffix(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمائی(قدر);

	إذا (vlen == 0) {
		*میل = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*میل = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	إذا (*میل == false) {
		ثابت حرف *انکڑے = (ثابت حرف *) dom_string_انکڑے(atr_val);
		size_t len = dom_string_byte_لمائی(atr_val);

		ثابت حرف *شروع = (حرف *) انکڑے + len - vlen;

		إذا ((len >= vlen) &&
		    (strncasecmp(شروع, lwc_string_انکڑے(قدر), vlen) == 0)) {
			*میل = true;
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
css_error گرہ_has_attribute_substring(فراغ *pw, فراغ *گرہ,
		ثابت css_qname *qname, lwc_string *قدر,
		bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_لمائی(قدر);

	إذا (vlen == 0) {
		*میل = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_انکڑے(qname->name),
		lwc_string_لمائی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*میل = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*میل = dom_string_caseless_lwc_isequal(atr_val, قدر);

	/* check for prefix match */
	إذا (*میل == false) {
		ثابت حرف *vdata = lwc_string_انکڑے(قدر);
		ثابت حرف *شروع = (ثابت حرف *) dom_string_انکڑے(atr_val);
		size_t len = dom_string_byte_لمائی(atr_val);
		ثابت حرف *آخری_شروع = شروع + len - vlen;

		إذا (len >= vlen) {
			طالما (شروع <= آخری_شروع) {
				إذا (strncasecmp(شروع, vdata,
						vlen) == 0) {
					*میل = true;
					اكسر;
				}

				شروع++;
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
css_error گرہ_is_جڑ(فراغ *pw, فراغ *گرہ, bool *میل)
{
	dom_گرہ *n = گرہ;
	dom_گرہ *ماپا;
	dom_گرہ_type type;
	dom_exception err;

	err = dom_گرہ_get_ماپا_گرہ(n, &ماپا);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	إذا (ماپا != عدم) {
		err = dom_گرہ_get_گرہ_type(ماپا, &type);

		dom_گرہ_unref(ماپا);

		إذا (err != DOM_NO_ERR)
			أرجع CSS_NOMEM;

		إذا (type != DOM_DOCUMENT_گرہ) {
			*میل = false;
			أرجع CSS_OK;
		}
	}

	*میل = true;

	أرجع CSS_OK;
}

ساكن صحيح
گرہ_گنتی_siblings_check(dom_گرہ *گرہ,
			  bool check_name,
			  dom_string *name)
{
	dom_گرہ_type type;
	صحيح ret = 0;
	dom_exception exc;

	إذا (گرہ == عدم)
		أرجع 0;

	exc = dom_گرہ_get_گرہ_type(گرہ, &type);
	إذا ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_گرہ)) {
		أرجع 0;
	}

	إذا (check_name) {
		dom_string *گرہ_name = عدم;
		exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);

		إذا ((exc == DOM_NO_ERR) && (گرہ_name != عدم)) {

			إذا (dom_string_caseless_isequal(name,
							گرہ_name)) {
				ret = 1;
			}
			dom_string_unref(گرہ_name);
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
css_error گرہ_گنتی_siblings(فراغ *pw, فراغ *n, bool same_name,
		bool after, int32_t *گنتی)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *گرہ_name = عدم;

	إذا (same_name) {
		dom_گرہ *گرہ = n;
		exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);
		إذا ((exc != DOM_NO_ERR) || (گرہ_name == عدم)) {
			أرجع CSS_NOMEM;
		}
	}

	إذا (after) {
		dom_گرہ *گرہ = dom_گرہ_ref(n);
		dom_گرہ *اگلا;

		افعل {
			exc = dom_گرہ_get_اگلا_sibling(گرہ, &اگلا);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_گرہ_unref(گرہ);
			گرہ = اگلا;

			cnt += گرہ_گنتی_siblings_check(گرہ, same_name, گرہ_name);
		} طالما (گرہ != عدم);
	} وإلا {
		dom_گرہ *گرہ = dom_گرہ_ref(n);
		dom_گرہ *اگلا;

		افعل {
			exc = dom_گرہ_get_پچھلا_sibling(گرہ, &اگلا);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_گرہ_unref(گرہ);
			گرہ = اگلا;

			cnt += گرہ_گنتی_siblings_check(گرہ, same_name, گرہ_name);

		} طالما (گرہ != عدم);
	}

	إذا (گرہ_name != عدم) {
		dom_string_unref(گرہ_name);
	}

	*گنتی = cnt;
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
css_error گرہ_is_خالی_تھاواں(فراغ *pw, فراغ *گرہ, bool *میل)
{
	dom_گرہ *n = گرہ, *اگلا;
	dom_exception err;

	*میل = true;

	err = dom_گرہ_get_پہلا_بچہ(n, &n);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_BADPARM;
	}

	طالما (n != عدم) {
		dom_گرہ_type ntype;
		err = dom_گرہ_get_گرہ_type(n, &ntype);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_BADPARM;
		}

		إذا (ntype == DOM_ELEMENT_گرہ ||
		    ntype == DOM_متن_گرہ) {
			*میل = false;
			dom_گرہ_unref(n);
			اكسر;
		}

		err = dom_گرہ_get_اگلا_sibling(n, &اگلا);
		إذا (err != DOM_NO_ERR) {
			dom_گرہ_unref(n);
			أرجع CSS_BADPARM;
		}
		dom_گرہ_unref(n);
		n = اگلا;
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
css_error گرہ_is_link(فراغ *pw, فراغ *n, bool *میل)
{
	dom_گرہ *گرہ = n;
	dom_exception exc;
	dom_string *گرہ_name = عدم;

	exc = dom_گرہ_get_گرہ_name(گرہ, &گرہ_name);
	إذا ((exc != DOM_NO_ERR) || (گرہ_name == عدم)) {
		أرجع CSS_NOMEM;
	}

	إذا (dom_string_caseless_lwc_isequal(گرہ_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(گرہ, corestring_dom_href,
				&has_href);
		إذا ((exc == DOM_NO_ERR) && (has_href)) {
			*میل = true;
		} وإلا {
			*میل = false;
		}
	} وإلا {
		*میل = false;
	}
	dom_string_unref(گرہ_name);

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
css_error گرہ_is_ویکھیا(فراغ *pw, فراغ *گرہ, bool *میل)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت بنية url_انکڑے *انکڑے;

	dom_exception exc;
	dom_گرہ *n = گرہ;
	dom_string *s = عدم;

	*میل = false;

	exc = dom_گرہ_get_گرہ_name(n, &s);
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
	error = nsurl_join(ctx->ادھار_url, dom_string_انکڑے(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	إذا (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		أرجع CSS_NOMEM;
	}

	انکڑے = urldb_get_url_انکڑے(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	إذا (انکڑے != عدم && انکڑے->visits > 0)
		*میل = true;

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
css_error گرہ_is_hover(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support hovering */

	*میل = false;

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
css_error گرہ_is_سرگرم(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support active nodes */

	*میل = false;

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
css_error گرہ_is_focus(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support focussed nodes */

	*میل = false;

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
css_error گرہ_is_enabled(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support enabled nodes */

	*میل = false;

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
css_error گرہ_is_disabled(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support disabled nodes */

	*میل = false;

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
css_error گرہ_is_checked(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support checked nodes */

	*میل = false;

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
css_error گرہ_is_نشانہ(فراغ *pw, فراغ *گرہ, bool *میل)
{
	/** \todo Support target */

	*میل = false;

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
css_error گرہ_is_lang(فراغ *pw, فراغ *گرہ,
		lwc_string *lang, bool *میل)
{
	/** \todo Support languages */

	*میل = false;

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
		hint->انکڑے.color = 0xff000000;
		hint->حالت = CSS_COLOR_COLOR;
	} وإلا إذا (property == CSS_PROP_FONT_FAMILY) {
		hint->انکڑے.لڑیاں = عدم;
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
		hint->انکڑے.لڑیاں = عدم;
		hint->حالت = CSS_QUOTES_NONE;
	} وإلا إذا (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->انکڑے.لڑیاں = عدم;
		hint->حالت = 0;
	} وإلا {
		أرجع CSS_INVALID;
	}

	أرجع CSS_OK;
}

css_error سیٹ_libcss_گرہ_انکڑے(فراغ *pw, فراغ *گرہ, فراغ *libcss_گرہ_انکڑے)
{
	dom_گرہ *n = گرہ;
	dom_exception err;
	فراغ *old_گرہ_انکڑے;

	/* Set this node's node data */
	err = dom_گرہ_سیٹ_user_انکڑے(n,
			corestring_dom_ns_کلید_libcss_گرہ_انکڑے,
			libcss_گرہ_انکڑے, nscss_dom_user_انکڑے_handler,
			(فراغ *) &old_گرہ_انکڑے);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أكد(old_گرہ_انکڑے == عدم);

	أرجع CSS_OK;
}

css_error get_libcss_گرہ_انکڑے(فراغ *pw, فراغ *گرہ, فراغ **libcss_گرہ_انکڑے)
{
	dom_گرہ *n = گرہ;
	dom_exception err;

	/* Get this node's node data */
	err = dom_گرہ_get_user_انکڑے(n,
			corestring_dom_ns_کلید_libcss_گرہ_انکڑے,
			libcss_گرہ_انکڑے);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أرجع CSS_OK;
}
