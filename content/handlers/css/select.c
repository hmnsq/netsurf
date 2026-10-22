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

ساكن css_error گره_name(فراغ *pw, فراغ *گره, css_qname *qname);
ساكن css_error گره_classes(فراغ *pw, فراغ *گره,
		lwc_string ***classes, uint32_t *n_classes);
ساكن css_error گره_id(فراغ *pw, فراغ *گره, lwc_string **id);
ساكن css_error named_والد_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **والد);
ساكن css_error named_sibling_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error named_generic_sibling_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error والد_گره(فراغ *pw, فراغ *گره, فراغ **والد);
ساكن css_error sibling_گره(فراغ *pw, فراغ *گره, فراغ **sibling);
ساكن css_error گره_has_name(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, bool *تطابق);
ساكن css_error گره_has_class(فراغ *pw, فراغ *گره,
		lwc_string *name, bool *تطابق);
ساكن css_error گره_has_id(فراغ *pw, فراغ *گره,
		lwc_string *name, bool *تطابق);
ساكن css_error گره_has_attribute(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, bool *تطابق);
ساكن css_error گره_has_attribute_equal(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_has_attribute_dashmatch(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_has_attribute_includes(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_has_attribute_پیشوند(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_has_attribute_suffix(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_has_attribute_substring(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق);
ساكن css_error گره_is_ریشه(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_شمار_siblings(فراغ *pw, فراغ *گره,
		bool same_name, bool after, int32_t *شمار);
ساكن css_error گره_is_خالی(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_link(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_hover(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_فعال(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_focus(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_enabled(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_disabled(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_checked(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_هدف(فراغ *pw, فراغ *گره, bool *تطابق);
ساكن css_error گره_is_lang(فراغ *pw, فراغ *گره,
		lwc_string *lang, bool *تطابق);
ساكن css_error ua_default_for_property(فراغ *pw, uint32_t property,
		css_hint *hint);
ساكن css_error مجموعه_libcss_گره_داده(فراغ *pw, فراغ *گره,
		فراغ *libcss_گره_داده);
ساكن css_error get_libcss_گره_داده(فراغ *pw, فراغ *گره,
		فراغ **libcss_گره_داده);

/**
 * Selection callback table for libcss
 */
ساكن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_نسخه_1,

	گره_name,
	گره_classes,
	گره_id,
	named_ancestor_گره,
	named_والد_گره,
	named_sibling_گره,
	named_generic_sibling_گره,
	والد_گره,
	sibling_گره,
	گره_has_name,
	گره_has_class,
	گره_has_id,
	گره_has_attribute,
	گره_has_attribute_equal,
	گره_has_attribute_dashmatch,
	گره_has_attribute_includes,
	گره_has_attribute_پیشوند,
	گره_has_attribute_suffix,
	گره_has_attribute_substring,
	گره_is_ریشه,
	گره_شمار_siblings,
	گره_is_خالی,
	گره_is_link,
	گره_is_دیدهشده,
	گره_is_hover,
	گره_is_فعال,
	گره_is_focus,
	گره_is_enabled,
	گره_is_disabled,
	گره_is_checked,
	گره_is_هدف,
	گره_is_lang,
	گره_presentational_hint,
	ua_default_for_property,
	مجموعه_libcss_گره_داده,
	get_libcss_گره_داده,
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
css_stylesheet *nscss_create_inline_style(ثابت uint8_t *داده, size_t len,
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
	params.color = ns_system_رنگ;
	params.color_pw = عدم;
	params.font = عدم;
	params.font_pw = عدم;

	error = css_stylesheet_create(&params, &sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		أرجع عدم;
	}

	error = css_stylesheet_append_داده(sheet, داده, len);
	إذا (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	error = css_stylesheet_داده_انجامشده(sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	أرجع sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساكن فراغ nscss_dom_user_داده_handler(dom_گره_operation operation,
		dom_string *کلید, فراغ *داده, بنية dom_گره *src,
		بنية dom_گره *dst)
{
	css_error error;

	إذا (dom_string_isequal(corestring_dom_ns_کلید_libcss_گره_داده,
			کلید) == false || داده == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_گره_CLONED:
		error = css_libcss_گره_داده_handler(&selection_handler,
				CSS_گره_CLONED,
				عدم, src, dst, داده);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		اكسر;

	حالة DOM_گره_RENAMED:
		error = css_libcss_گره_داده_handler(&selection_handler,
				CSS_گره_MODIFIED,
				عدم, src, عدم, داده);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		اكسر;

	حالة DOM_گره_IMPORTED:
	حالة DOM_گره_ADOPTED:
	حالة DOM_گره_DELETED:
		error = css_libcss_گره_داده_handler(&selection_handler,
				CSS_گره_DELETED,
				عدم, src, عدم, داده);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_گره *n,
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
	إذا (ctx->والد_style != عدم) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->والد_style,
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
			pseudo_element < CSS_PSEUDO_ELEMENT_شمار;
			pseudo_element++) {

		إذا (pseudo_element == CSS_PSEUDO_ELEMENT_اولی_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_اولی_سطر)
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
		ثابت css_computed_style *والد)
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
	error = css_computed_style_compose(والد, partial,
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
css_error گره_name(فراغ *pw, فراغ *گره, css_qname *qname)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_exception err;

	err = dom_گره_get_گره_name(n, &name);
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
css_error گره_classes(فراغ *pw, فراغ *گره,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_گره *n = گره;
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
css_error گره_id(فراغ *pw, فراغ *گره, lwc_string **id)
{
	dom_گره *n = گره;
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
css_error named_ancestor_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **ancestor)
{
	dom_element_named_ancestor_گره(گره, qname->name,
			(بنية dom_element **)ancestor);
	dom_گره_unref(*ancestor);

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
css_error named_والد_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **والد)
{
	dom_element_named_والد_گره(گره, qname->name,
			(بنية dom_element **)والد);
	dom_گره_unref(*والد);

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
css_error named_sibling_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_گره_get_قبلی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گره_type type;

		err = dom_گره_get_گره_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گره)
			اكسر;

		err = dom_گره_get_قبلی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		dom_گره_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		dom_string *name;

		err = dom_گره_get_گره_name(n, &name);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		dom_گره_unref(n);

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
css_error named_generic_sibling_گره(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = عدم;

	err = dom_گره_get_قبلی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گره_type type;
		dom_string *name;

		err = dom_گره_get_گره_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گره) {
			err = dom_گره_get_گره_name(n, &name);
			إذا (err != DOM_NO_ERR) {
				dom_گره_unref(n);
				أرجع CSS_OK;
			}

			إذا (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_گره_unref(n);
				*sibling = n;
				اكسر;
			}
			dom_string_unref(name);
		}

		err = dom_گره_get_قبلی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		dom_گره_unref(n);
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
css_error والد_گره(فراغ *pw, فراغ *گره, فراغ **والد)
{
	dom_element_والد_گره(گره, (بنية dom_element **)والد);
	dom_گره_unref(*والد);

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
css_error sibling_گره(فراغ *pw, فراغ *گره, فراغ **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_گره_get_قبلی_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_گره_type type;

		err = dom_گره_get_گره_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_گره)
			اكسر;

		err = dom_گره_get_قبلی_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_OK;
		}

		dom_گره_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		/** \todo Sort out reference counting */
		dom_گره_unref(n);

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
css_error گره_has_name(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, bool *تطابق)
{
	nscss_select_ctx *ctx = pw;
	dom_گره *n = گره;

	إذا (lwc_string_isequal(qname->name, ctx->universal, تطابق) ==
			lwc_error_ok && *تطابق == false) {
		dom_string *name;
		dom_exception err;

		err = dom_گره_get_گره_name(n, &name);
		إذا (err != DOM_NO_ERR)
			أرجع CSS_OK;

		/* Element names are case insensitive in HTML */
		*تطابق = dom_string_caseless_lwc_isequal(name, qname->name);

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
css_error گره_has_class(فراغ *pw, فراغ *گره,
		lwc_string *name, bool *تطابق)
{
	dom_گره *n = گره;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, تطابق);

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
css_error گره_has_id(فراغ *pw, فراغ *گره,
		lwc_string *name, bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *attr;
	dom_exception err;

	*تطابق = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	إذا (attr != عدم) {
		*تطابق = dom_string_lwc_isequal(attr, name);

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
css_error گره_has_attribute(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_داده(qname->name),
			lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_has_attribute(n, name, تطابق);
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
css_error گره_has_attribute_equal(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_درازی(مقدار);

	إذا (vlen == 0) {
		*تطابق = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	*تطابق = dom_string_caseless_lwc_isequal(atr_val, مقدار);

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
css_error گره_has_attribute_dashmatch(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_درازی(مقدار);

	إذا (vlen == 0) {
		*تطابق = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*تطابق = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for dashmatch */
	إذا (*تطابق == false) {
		ثابت حرف *vdata = lwc_string_داده(مقدار);
		ثابت حرف *داده = (ثابت حرف *) dom_string_داده(atr_val);
		size_t len = dom_string_byte_درازی(atr_val);

		إذا (len > vlen && داده[vlen] == '-' &&
		    strncasecmp(داده, vdata, vlen) == 0) {
				*تطابق = true;
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
css_error گره_has_attribute_includes(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_درازی(مقدار);
	ثابت حرف *p;
	ثابت حرف *شروع;
	ثابت حرف *انتها;

	*تطابق = false;

	إذا (vlen == 0) {
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	شروع = (ثابت حرف *) dom_string_داده(atr_val);
	انتها = شروع + dom_string_byte_درازی(atr_val);

	لكل (p = شروع; p <= انتها; p++) {
		إذا (*p == ' ' || *p == '\0') {
			إذا ((size_t) (p - شروع) == vlen &&
			    strncasecmp(شروع,
					lwc_string_داده(مقدار),
					vlen) == 0) {
				*تطابق = true;
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
css_error گره_has_attribute_پیشوند(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_درازی(مقدار);

	إذا (vlen == 0) {
		*تطابق = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*تطابق = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	إذا (*تطابق == false) {
		ثابت حرف *داده = (ثابت حرف *) dom_string_داده(atr_val);
		size_t len = dom_string_byte_درازی(atr_val);

		إذا ((len >= vlen) &&
		    (strncasecmp(داده, lwc_string_داده(مقدار), vlen) == 0)) {
			*تطابق = true;
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
css_error گره_has_attribute_suffix(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_درازی(مقدار);

	إذا (vlen == 0) {
		*تطابق = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*تطابق = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	إذا (*تطابق == false) {
		ثابت حرف *داده = (ثابت حرف *) dom_string_داده(atr_val);
		size_t len = dom_string_byte_درازی(atr_val);

		ثابت حرف *شروع = (حرف *) داده + len - vlen;

		إذا ((len >= vlen) &&
		    (strncasecmp(شروع, lwc_string_داده(مقدار), vlen) == 0)) {
			*تطابق = true;
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
css_error گره_has_attribute_substring(فراغ *pw, فراغ *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *تطابق)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_درازی(مقدار);

	إذا (vlen == 0) {
		*تطابق = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_داده(qname->name),
		lwc_string_درازی(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*تطابق = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*تطابق = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	إذا (*تطابق == false) {
		ثابت حرف *vdata = lwc_string_داده(مقدار);
		ثابت حرف *شروع = (ثابت حرف *) dom_string_داده(atr_val);
		size_t len = dom_string_byte_درازی(atr_val);
		ثابت حرف *آخرین_شروع = شروع + len - vlen;

		إذا (len >= vlen) {
			طالما (شروع <= آخرین_شروع) {
				إذا (strncasecmp(شروع, vdata,
						vlen) == 0) {
					*تطابق = true;
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
css_error گره_is_ریشه(فراغ *pw, فراغ *گره, bool *تطابق)
{
	dom_گره *n = گره;
	dom_گره *والد;
	dom_گره_type type;
	dom_exception err;

	err = dom_گره_get_والد_گره(n, &والد);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	إذا (والد != عدم) {
		err = dom_گره_get_گره_type(والد, &type);

		dom_گره_unref(والد);

		إذا (err != DOM_NO_ERR)
			أرجع CSS_NOMEM;

		إذا (type != DOM_DOCUMENT_گره) {
			*تطابق = false;
			أرجع CSS_OK;
		}
	}

	*تطابق = true;

	أرجع CSS_OK;
}

ساكن صحيح
گره_شمار_siblings_check(dom_گره *گره,
			  bool check_name,
			  dom_string *name)
{
	dom_گره_type type;
	صحيح ret = 0;
	dom_exception exc;

	إذا (گره == عدم)
		أرجع 0;

	exc = dom_گره_get_گره_type(گره, &type);
	إذا ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_گره)) {
		أرجع 0;
	}

	إذا (check_name) {
		dom_string *گره_name = عدم;
		exc = dom_گره_get_گره_name(گره, &گره_name);

		إذا ((exc == DOM_NO_ERR) && (گره_name != عدم)) {

			إذا (dom_string_caseless_isequal(name,
							گره_name)) {
				ret = 1;
			}
			dom_string_unref(گره_name);
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
css_error گره_شمار_siblings(فراغ *pw, فراغ *n, bool same_name,
		bool after, int32_t *شمار)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *گره_name = عدم;

	إذا (same_name) {
		dom_گره *گره = n;
		exc = dom_گره_get_گره_name(گره, &گره_name);
		إذا ((exc != DOM_NO_ERR) || (گره_name == عدم)) {
			أرجع CSS_NOMEM;
		}
	}

	إذا (after) {
		dom_گره *گره = dom_گره_ref(n);
		dom_گره *بعدی;

		افعل {
			exc = dom_گره_get_بعدی_sibling(گره, &بعدی);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_گره_unref(گره);
			گره = بعدی;

			cnt += گره_شمار_siblings_check(گره, same_name, گره_name);
		} طالما (گره != عدم);
	} وإلا {
		dom_گره *گره = dom_گره_ref(n);
		dom_گره *بعدی;

		افعل {
			exc = dom_گره_get_قبلی_sibling(گره, &بعدی);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_گره_unref(گره);
			گره = بعدی;

			cnt += گره_شمار_siblings_check(گره, same_name, گره_name);

		} طالما (گره != عدم);
	}

	إذا (گره_name != عدم) {
		dom_string_unref(گره_name);
	}

	*شمار = cnt;
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
css_error گره_is_خالی(فراغ *pw, فراغ *گره, bool *تطابق)
{
	dom_گره *n = گره, *بعدی;
	dom_exception err;

	*تطابق = true;

	err = dom_گره_get_اولی_فرزند(n, &n);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_BADPARM;
	}

	طالما (n != عدم) {
		dom_گره_type ntype;
		err = dom_گره_get_گره_type(n, &ntype);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_BADPARM;
		}

		إذا (ntype == DOM_ELEMENT_گره ||
		    ntype == DOM_متن_گره) {
			*تطابق = false;
			dom_گره_unref(n);
			اكسر;
		}

		err = dom_گره_get_بعدی_sibling(n, &بعدی);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			أرجع CSS_BADPARM;
		}
		dom_گره_unref(n);
		n = بعدی;
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
css_error گره_is_link(فراغ *pw, فراغ *n, bool *تطابق)
{
	dom_گره *گره = n;
	dom_exception exc;
	dom_string *گره_name = عدم;

	exc = dom_گره_get_گره_name(گره, &گره_name);
	إذا ((exc != DOM_NO_ERR) || (گره_name == عدم)) {
		أرجع CSS_NOMEM;
	}

	إذا (dom_string_caseless_lwc_isequal(گره_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(گره, corestring_dom_href,
				&has_href);
		إذا ((exc == DOM_NO_ERR) && (has_href)) {
			*تطابق = true;
		} وإلا {
			*تطابق = false;
		}
	} وإلا {
		*تطابق = false;
	}
	dom_string_unref(گره_name);

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
css_error گره_is_دیدهشده(فراغ *pw, فراغ *گره, bool *تطابق)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت بنية url_داده *داده;

	dom_exception exc;
	dom_گره *n = گره;
	dom_string *s = عدم;

	*تطابق = false;

	exc = dom_گره_get_گره_name(n, &s);
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
	error = nsurl_join(ctx->پایه_url, dom_string_داده(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	إذا (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		أرجع CSS_NOMEM;
	}

	داده = urldb_get_url_داده(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	إذا (داده != عدم && داده->visits > 0)
		*تطابق = true;

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
css_error گره_is_hover(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support hovering */

	*تطابق = false;

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
css_error گره_is_فعال(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support active nodes */

	*تطابق = false;

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
css_error گره_is_focus(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support focussed nodes */

	*تطابق = false;

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
css_error گره_is_enabled(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support enabled nodes */

	*تطابق = false;

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
css_error گره_is_disabled(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support disabled nodes */

	*تطابق = false;

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
css_error گره_is_checked(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support checked nodes */

	*تطابق = false;

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
css_error گره_is_هدف(فراغ *pw, فراغ *گره, bool *تطابق)
{
	/** \todo Support target */

	*تطابق = false;

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
css_error گره_is_lang(فراغ *pw, فراغ *گره,
		lwc_string *lang, bool *تطابق)
{
	/** \todo Support languages */

	*تطابق = false;

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
		hint->داده.color = 0xff000000;
		hint->وضعیت = CSS_COLOR_COLOR;
	} وإلا إذا (property == CSS_PROP_FONT_FAMILY) {
		hint->داده.رشتهها = عدم;
		بدل (nsoption_int(font_default)) {
		حالة PLOT_FONT_FAMILY_SANS_SERIF:
			hint->وضعیت = CSS_FONT_FAMILY_SANS_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_SERIF:
			hint->وضعیت = CSS_FONT_FAMILY_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_MONOSPACE:
			hint->وضعیت = CSS_FONT_FAMILY_MONOSPACE;
			اكسر;
		حالة PLOT_FONT_FAMILY_CURSIVE:
			hint->وضعیت = CSS_FONT_FAMILY_CURSIVE;
			اكسر;
		حالة PLOT_FONT_FAMILY_FANTASY:
			hint->وضعیت = CSS_FONT_FAMILY_FANTASY;
			اكسر;
		}
	} وإلا إذا (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->داده.رشتهها = عدم;
		hint->وضعیت = CSS_QUOTES_NONE;
	} وإلا إذا (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->داده.رشتهها = عدم;
		hint->وضعیت = 0;
	} وإلا {
		أرجع CSS_INVALID;
	}

	أرجع CSS_OK;
}

css_error مجموعه_libcss_گره_داده(فراغ *pw, فراغ *گره, فراغ *libcss_گره_داده)
{
	dom_گره *n = گره;
	dom_exception err;
	فراغ *old_گره_داده;

	/* Set this node's node data */
	err = dom_گره_مجموعه_user_داده(n,
			corestring_dom_ns_کلید_libcss_گره_داده,
			libcss_گره_داده, nscss_dom_user_داده_handler,
			(فراغ *) &old_گره_داده);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أكد(old_گره_داده == عدم);

	أرجع CSS_OK;
}

css_error get_libcss_گره_داده(فراغ *pw, فراغ *گره, فراغ **libcss_گره_داده)
{
	dom_گره *n = گره;
	dom_exception err;

	/* Get this node's node data */
	err = dom_گره_get_user_داده(n,
			corestring_dom_ns_کلید_libcss_گره_داده,
			libcss_گره_داده);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أرجع CSS_OK;
}
