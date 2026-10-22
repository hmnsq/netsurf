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

ایستا css_error گره_name(تهی *pw, تهی *گره, css_qname *qname);
ایستا css_error گره_classes(تهی *pw, تهی *گره,
		lwc_string ***classes, uint32_t *n_classes);
ایستا css_error گره_id(تهی *pw, تهی *گره, lwc_string **id);
ایستا css_error named_parent_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **parent);
ایستا css_error named_sibling_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **sibling);
ایستا css_error named_generic_sibling_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **sibling);
ایستا css_error parent_گره(تهی *pw, تهی *گره, تهی **parent);
ایستا css_error sibling_گره(تهی *pw, تهی *گره, تهی **sibling);
ایستا css_error گره_has_name(تهی *pw, تهی *گره,
		ثابت css_qname *qname, bool *match);
ایستا css_error گره_has_class(تهی *pw, تهی *گره,
		lwc_string *name, bool *match);
ایستا css_error گره_has_id(تهی *pw, تهی *گره,
		lwc_string *name, bool *match);
ایستا css_error گره_has_attribute(تهی *pw, تهی *گره,
		ثابت css_qname *qname, bool *match);
ایستا css_error گره_has_attribute_equal(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_has_attribute_dashmatch(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_has_attribute_includes(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_has_attribute_prefix(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_has_attribute_suffix(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_has_attribute_substring(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match);
ایستا css_error گره_is_ریشه(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_شمار_siblings(تهی *pw, تهی *گره,
		bool same_name, bool after, int32_t *شمار);
ایستا css_error گره_is_empty(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_link(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_hover(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_active(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_focus(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_enabled(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_disabled(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_checked(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_target(تهی *pw, تهی *گره, bool *match);
ایستا css_error گره_is_lang(تهی *pw, تهی *گره,
		lwc_string *lang, bool *match);
ایستا css_error ua_default_for_property(تهی *pw, uint32_t property,
		css_hint *hint);
ایستا css_error set_libcss_گره_data(تهی *pw, تهی *گره,
		تهی *libcss_گره_data);
ایستا css_error get_libcss_گره_data(تهی *pw, تهی *گره,
		تهی **libcss_گره_data);

/**
 * Selection callback table for libcss
 */
ایستا css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	گره_name,
	گره_classes,
	گره_id,
	named_ancestor_گره,
	named_parent_گره,
	named_sibling_گره,
	named_generic_sibling_گره,
	parent_گره,
	sibling_گره,
	گره_has_name,
	گره_has_class,
	گره_has_id,
	گره_has_attribute,
	گره_has_attribute_equal,
	گره_has_attribute_dashmatch,
	گره_has_attribute_includes,
	گره_has_attribute_prefix,
	گره_has_attribute_suffix,
	گره_has_attribute_substring,
	گره_is_ریشه,
	گره_شمار_siblings,
	گره_is_empty,
	گره_is_link,
	گره_is_visited,
	گره_is_hover,
	گره_is_active,
	گره_is_focus,
	گره_is_enabled,
	گره_is_disabled,
	گره_is_checked,
	گره_is_target,
	گره_is_lang,
	گره_presentational_hint,
	ua_default_for_property,
	set_libcss_گره_data,
	get_libcss_گره_data,
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
css_stylesheet *nscss_create_inline_style(ثابت uint8_t *data, size_t len,
		ثابت نویسه *charset, ثابت نویسه *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = پوچ;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = پوچ;
	params.import = پوچ;
	params.import_pw = پوچ;
	params.color = ns_system_colour;
	params.color_pw = پوچ;
	params.font = پوچ;
	params.font_pw = پوچ;

	error = css_stylesheet_create(&params, &sheet);
	اگر (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		بازگشت پوچ;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	اگر (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		بازگشت پوچ;
	}

	error = css_stylesheet_data_done(sheet);
	اگر (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		بازگشت پوچ;
	}

	بازگشت sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ایستا تهی nscss_dom_user_data_handler(dom_گره_operation operation,
		dom_string *کلید, تهی *data, ساختار dom_گره *src,
		ساختار dom_گره *dst)
{
	css_error error;

	اگر (dom_string_isequal(corestring_dom_ns_کلید_libcss_گره_data,
			کلید) == false || data == پوچ) {
		بازگشت;
	}

	انتخاب (operation) {
	حالت DOM_گره_CLONED:
		error = css_libcss_گره_data_handler(&selection_handler,
				CSS_گره_CLONED,
				پوچ, src, dst, data);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		بشکن;

	حالت DOM_گره_RENAMED:
		error = css_libcss_گره_data_handler(&selection_handler,
				CSS_گره_MODIFIED,
				پوچ, src, پوچ, data);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		بشکن;

	حالت DOM_گره_IMPORTED:
	حالت DOM_گره_ADOPTED:
	حالت DOM_گره_DELETED:
		error = css_libcss_گره_data_handler(&selection_handler,
				CSS_گره_DELETED,
				پوچ, src, پوچ, data);
		اگر (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		بشکن;

	پیشفرض:
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
	صحیح pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	اگر (error != CSS_OK || styles == پوچ) {
		/* Failed selecting partial style -- bail out */
		بازگشت پوچ;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	اگر (ctx->parent_style != پوچ) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		اگر (error != CSS_OK) {
			css_select_results_destroy(styles);
			بازگشت پوچ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	برای (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_شمار;
			pseudo_element++) {

		اگر (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_سطر)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			ادامه;

		اگر (styles->styles[pseudo_element] == پوچ)
			/* There were no rules concerning this pseudo element */
			ادامه;

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
			بازگشت پوچ;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	بازگشت styles;
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
		ثابت css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	اگر (error != CSS_OK) {
		بازگشت پوچ;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	اگر (error != CSS_OK) {
		css_computed_style_destroy(composed);
		بازگشت پوچ;
	}

	بازگشت composed;
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
css_error گره_name(تهی *pw, تهی *گره, css_qname *qname)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_exception err;

	err = dom_گره_get_گره_name(n, &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	qname->ns = پوچ;

	err = dom_string_intern(name, &qname->name);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(name);
		بازگشت CSS_NOMEM;
	}

	dom_string_unref(name);

	بازگشت CSS_OK;
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
css_error گره_classes(تهی *pw, تهی *گره,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_گره *n = گره;
	dom_exception err;

	*classes = پوچ;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	بازگشت CSS_OK;
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
css_error گره_id(تهی *pw, تهی *گره, lwc_string **id)
{
	dom_گره *n = گره;
	dom_string *attr;
	dom_exception err;

	*id = پوچ;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	اگر (attr != پوچ) {
		err = dom_string_intern(attr, id);
		اگر (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			بازگشت CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	بازگشت CSS_OK;
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
css_error named_ancestor_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **ancestor)
{
	dom_element_named_ancestor_گره(گره, qname->name,
			(ساختار dom_element **)ancestor);
	dom_گره_unref(*ancestor);

	بازگشت CSS_OK;
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
css_error named_parent_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **parent)
{
	dom_element_named_parent_گره(گره, qname->name,
			(ساختار dom_element **)parent);
	dom_گره_unref(*parent);

	بازگشت CSS_OK;
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
css_error named_sibling_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = پوچ;

	/* Find sibling element */
	err = dom_گره_get_previous_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_OK;

	وقتی (n != پوچ) {
		dom_گره_type type;

		err = dom_گره_get_گره_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گره)
			بشکن;

		err = dom_گره_get_previous_sibling(n, &prev);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		dom_گره_unref(n);
		n = prev;
	}

	اگر (n != پوچ) {
		dom_string *name;

		err = dom_گره_get_گره_name(n, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		dom_گره_unref(n);

		اگر (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	بازگشت CSS_OK;
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
css_error named_generic_sibling_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = پوچ;

	err = dom_گره_get_previous_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_OK;

	وقتی (n != پوچ) {
		dom_گره_type type;
		dom_string *name;

		err = dom_گره_get_گره_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گره) {
			err = dom_گره_get_گره_name(n, &name);
			اگر (err != DOM_NO_ERR) {
				dom_گره_unref(n);
				بازگشت CSS_OK;
			}

			اگر (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_گره_unref(n);
				*sibling = n;
				بشکن;
			}
			dom_string_unref(name);
		}

		err = dom_گره_get_previous_sibling(n, &prev);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		dom_گره_unref(n);
		n = prev;
	}

	بازگشت CSS_OK;
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
css_error parent_گره(تهی *pw, تهی *گره, تهی **parent)
{
	dom_element_parent_گره(گره, (ساختار dom_element **)parent);
	dom_گره_unref(*parent);

	بازگشت CSS_OK;
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
css_error sibling_گره(تهی *pw, تهی *گره, تهی **sibling)
{
	dom_گره *n = گره;
	dom_گره *prev;
	dom_exception err;

	*sibling = پوچ;

	/* Find sibling element */
	err = dom_گره_get_previous_sibling(n, &n);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_OK;

	وقتی (n != پوچ) {
		dom_گره_type type;

		err = dom_گره_get_گره_type(n, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		اگر (type == DOM_ELEMENT_گره)
			بشکن;

		err = dom_گره_get_previous_sibling(n, &prev);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_OK;
		}

		dom_گره_unref(n);
		n = prev;
	}

	اگر (n != پوچ) {
		/** \todo Sort out reference counting */
		dom_گره_unref(n);

		*sibling = n;
	}

	بازگشت CSS_OK;
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
css_error گره_has_name(تهی *pw, تهی *گره,
		ثابت css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_گره *n = گره;

	اگر (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_گره_get_گره_name(n, &name);
		اگر (err != DOM_NO_ERR)
			بازگشت CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	بازگشت CSS_OK;
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
css_error گره_has_class(تهی *pw, تهی *گره,
		lwc_string *name, bool *match)
{
	dom_گره *n = گره;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	أكد(err == DOM_NO_ERR);

	بازگشت CSS_OK;
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
css_error گره_has_id(تهی *pw, تهی *گره,
		lwc_string *name, bool *match)
{
	dom_گره *n = گره;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_OK;

	اگر (attr != پوچ) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	بازگشت CSS_OK;
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
css_error گره_has_attribute(تهی *pw, تهی *گره,
		ثابت css_qname *qname, bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(name);
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_equal(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(مقدار);

	اگر (vlen == 0) {
		*match = false;
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_dashmatch(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(مقدار);

	اگر (vlen == 0) {
		*match = false;
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for dashmatch */
	اگر (*match == false) {
		ثابت نویسه *vdata = lwc_string_data(مقدار);
		ثابت نویسه *data = (ثابت نویسه *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		اگر (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_includes(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(مقدار);
	ثابت نویسه *p;
	ثابت نویسه *start;
	ثابت نویسه *end;

	*match = false;

	اگر (vlen == 0) {
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ثابت نویسه *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	برای (p = start; p <= end; p++) {
		اگر (*p == ' ' || *p == '\0') {
			اگر ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(مقدار),
					vlen) == 0) {
				*match = true;
				بشکن;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_prefix(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(مقدار);

	اگر (vlen == 0) {
		*match = false;
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	اگر (*match == false) {
		ثابت نویسه *data = (ثابت نویسه *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		اگر ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(مقدار), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_suffix(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(مقدار);

	اگر (vlen == 0) {
		*match = false;
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	اگر (*match == false) {
		ثابت نویسه *data = (ثابت نویسه *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ثابت نویسه *start = (نویسه *) data + len - vlen;

		اگر ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(مقدار), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_has_attribute_substring(تهی *pw, تهی *گره,
		ثابت css_qname *qname, lwc_string *مقدار,
		bool *match)
{
	dom_گره *n = گره;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(مقدار);

	اگر (vlen == 0) {
		*match = false;
		بازگشت CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	اگر (err != DOM_NO_ERR)
		بازگشت CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	اگر ((err != DOM_NO_ERR) || (atr_val == پوچ)) {
		dom_string_unref(name);
		*match = false;
		بازگشت CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, مقدار);

	/* check for prefix match */
	اگر (*match == false) {
		ثابت نویسه *vdata = lwc_string_data(مقدار);
		ثابت نویسه *start = (ثابت نویسه *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ثابت نویسه *last_start = start + len - vlen;

		اگر (len >= vlen) {
			وقتی (start <= last_start) {
				اگر (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					بشکن;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	بازگشت CSS_OK;
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
css_error گره_is_ریشه(تهی *pw, تهی *گره, bool *match)
{
	dom_گره *n = گره;
	dom_گره *parent;
	dom_گره_type type;
	dom_exception err;

	err = dom_گره_get_parent_گره(n, &parent);
	اگر (err != DOM_NO_ERR) {
		بازگشت CSS_NOMEM;
	}

	اگر (parent != پوچ) {
		err = dom_گره_get_گره_type(parent, &type);

		dom_گره_unref(parent);

		اگر (err != DOM_NO_ERR)
			بازگشت CSS_NOMEM;

		اگر (type != DOM_DOCUMENT_گره) {
			*match = false;
			بازگشت CSS_OK;
		}
	}

	*match = true;

	بازگشت CSS_OK;
}

ایستا صحیح
گره_شمار_siblings_check(dom_گره *گره,
			  bool check_name,
			  dom_string *name)
{
	dom_گره_type type;
	صحیح ret = 0;
	dom_exception exc;

	اگر (گره == پوچ)
		بازگشت 0;

	exc = dom_گره_get_گره_type(گره, &type);
	اگر ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_گره)) {
		بازگشت 0;
	}

	اگر (check_name) {
		dom_string *گره_name = پوچ;
		exc = dom_گره_get_گره_name(گره, &گره_name);

		اگر ((exc == DOM_NO_ERR) && (گره_name != پوچ)) {

			اگر (dom_string_caseless_isequal(name,
							گره_name)) {
				ret = 1;
			}
			dom_string_unref(گره_name);
		}
	} وگرنه {
		ret = 1;
	}

	بازگشت ret;
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
css_error گره_شمار_siblings(تهی *pw, تهی *n, bool same_name,
		bool after, int32_t *شمار)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *گره_name = پوچ;

	اگر (same_name) {
		dom_گره *گره = n;
		exc = dom_گره_get_گره_name(گره, &گره_name);
		اگر ((exc != DOM_NO_ERR) || (گره_name == پوچ)) {
			بازگشت CSS_NOMEM;
		}
	}

	اگر (after) {
		dom_گره *گره = dom_گره_ref(n);
		dom_گره *next;

		انجامبده {
			exc = dom_گره_get_next_sibling(گره, &next);
			اگر ((exc != DOM_NO_ERR))
				بشکن;

			dom_گره_unref(گره);
			گره = next;

			cnt += گره_شمار_siblings_check(گره, same_name, گره_name);
		} وقتی (گره != پوچ);
	} وگرنه {
		dom_گره *گره = dom_گره_ref(n);
		dom_گره *next;

		انجامبده {
			exc = dom_گره_get_previous_sibling(گره, &next);
			اگر ((exc != DOM_NO_ERR))
				بشکن;

			dom_گره_unref(گره);
			گره = next;

			cnt += گره_شمار_siblings_check(گره, same_name, گره_name);

		} وقتی (گره != پوچ);
	}

	اگر (گره_name != پوچ) {
		dom_string_unref(گره_name);
	}

	*شمار = cnt;
	بازگشت CSS_OK;
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
css_error گره_is_empty(تهی *pw, تهی *گره, bool *match)
{
	dom_گره *n = گره, *next;
	dom_exception err;

	*match = true;

	err = dom_گره_get_first_child(n, &n);
	اگر (err != DOM_NO_ERR) {
		بازگشت CSS_BADPARM;
	}

	وقتی (n != پوچ) {
		dom_گره_type ntype;
		err = dom_گره_get_گره_type(n, &ntype);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_BADPARM;
		}

		اگر (ntype == DOM_ELEMENT_گره ||
		    ntype == DOM_TEXT_گره) {
			*match = false;
			dom_گره_unref(n);
			بشکن;
		}

		err = dom_گره_get_next_sibling(n, &next);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(n);
			بازگشت CSS_BADPARM;
		}
		dom_گره_unref(n);
		n = next;
	}

	بازگشت CSS_OK;
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
css_error گره_is_link(تهی *pw, تهی *n, bool *match)
{
	dom_گره *گره = n;
	dom_exception exc;
	dom_string *گره_name = پوچ;

	exc = dom_گره_get_گره_name(گره, &گره_name);
	اگر ((exc != DOM_NO_ERR) || (گره_name == پوچ)) {
		بازگشت CSS_NOMEM;
	}

	اگر (dom_string_caseless_lwc_isequal(گره_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(گره, corestring_dom_href,
				&has_href);
		اگر ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} وگرنه {
			*match = false;
		}
	} وگرنه {
		*match = false;
	}
	dom_string_unref(گره_name);

	بازگشت CSS_OK;
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
css_error گره_is_visited(تهی *pw, تهی *گره, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت ساختار url_data *data;

	dom_exception exc;
	dom_گره *n = گره;
	dom_string *s = پوچ;

	*match = false;

	exc = dom_گره_get_گره_name(n, &s);
	اگر ((exc != DOM_NO_ERR) || (s == پوچ)) {
		بازگشت CSS_NOMEM;
	}

	اگر (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		بازگشت CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = پوچ;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	اگر ((exc != DOM_NO_ERR) || (s == پوچ)) {
		/* Can't be visited; not got a URL */
		بازگشت CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	اگر (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		بازگشت CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	اگر (data != پوچ && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	بازگشت CSS_OK;
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
css_error گره_is_hover(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_active(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_focus(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_enabled(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_disabled(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_checked(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_target(تهی *pw, تهی *گره, bool *match)
{
	/** \todo Support target */

	*match = false;

	بازگشت CSS_OK;
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
css_error گره_is_lang(تهی *pw, تهی *گره,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	بازگشت CSS_OK;
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
css_error ua_default_for_property(تهی *pw, uint32_t property, css_hint *hint)
{
	اگر (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} وگرنه اگر (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = پوچ;
		انتخاب (nsoption_int(font_default)) {
		حالت PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			بشکن;
		حالت PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			بشکن;
		حالت PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			بشکن;
		حالت PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			بشکن;
		حالت PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			بشکن;
		}
	} وگرنه اگر (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = پوچ;
		hint->status = CSS_QUOTES_NONE;
	} وگرنه اگر (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = پوچ;
		hint->status = 0;
	} وگرنه {
		بازگشت CSS_INVALID;
	}

	بازگشت CSS_OK;
}

css_error set_libcss_گره_data(تهی *pw, تهی *گره, تهی *libcss_گره_data)
{
	dom_گره *n = گره;
	dom_exception err;
	تهی *old_گره_data;

	/* Set this node's node data */
	err = dom_گره_set_user_data(n,
			corestring_dom_ns_کلید_libcss_گره_data,
			libcss_گره_data, nscss_dom_user_data_handler,
			(تهی *) &old_گره_data);
	اگر (err != DOM_NO_ERR) {
		بازگشت CSS_NOMEM;
	}

	أكد(old_گره_data == پوچ);

	بازگشت CSS_OK;
}

css_error get_libcss_گره_data(تهی *pw, تهی *گره, تهی **libcss_گره_data)
{
	dom_گره *n = گره;
	dom_exception err;

	/* Get this node's node data */
	err = dom_گره_get_user_data(n,
			corestring_dom_ns_کلید_libcss_گره_data,
			libcss_گره_data);
	اگر (err != DOM_NO_ERR) {
		بازگشت CSS_NOMEM;
	}

	بازگشت CSS_OK;
}
