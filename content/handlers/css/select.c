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

ساكن css_error node_name(فراغ *pw, فراغ *node, css_qname *qname);
ساكن css_error node_classes(فراغ *pw, فراغ *node,
		lwc_string ***classes, uint32_t *n_classes);
ساكن css_error node_id(فراغ *pw, فراغ *node, lwc_string **id);
ساكن css_error named_parent_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **parent);
ساكن css_error named_sibling_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error named_generic_sibling_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error parent_node(فراغ *pw, فراغ *node, فراغ **parent);
ساكن css_error sibling_node(فراغ *pw, فراغ *node, فراغ **sibling);
ساكن css_error node_has_name(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, bool *match);
ساكن css_error node_has_class(فراغ *pw, فراغ *node,
		lwc_string *name, bool *match);
ساكن css_error node_has_id(فراغ *pw, فراغ *node,
		lwc_string *name, bool *match);
ساكن css_error node_has_attribute(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, bool *match);
ساكن css_error node_has_attribute_equal(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_has_attribute_dashmatch(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_has_attribute_includes(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_has_attribute_prefix(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_has_attribute_suffix(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_has_attribute_substring(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match);
ساكن css_error node_is_root(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_count_siblings(فراغ *pw, فراغ *node,
		bool same_name, bool after, int32_t *count);
ساكن css_error node_is_empty(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_link(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_hover(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_active(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_focus(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_enabled(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_disabled(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_checked(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_target(فراغ *pw, فراغ *node, bool *match);
ساكن css_error node_is_lang(فراغ *pw, فراغ *node,
		lwc_string *lang, bool *match);
ساكن css_error ua_default_for_property(فراغ *pw, uint32_t property,
		css_hint *hint);
ساكن css_error set_libcss_node_data(فراغ *pw, فراغ *node,
		فراغ *libcss_node_data);
ساكن css_error get_libcss_node_data(فراغ *pw, فراغ *node,
		فراغ **libcss_node_data);

/**
 * Selection callback table for libcss
 */
ساكن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	node_name,
	node_classes,
	node_id,
	named_ancestor_node,
	named_parent_node,
	named_sibling_node,
	named_generic_sibling_node,
	parent_node,
	sibling_node,
	node_has_name,
	node_has_class,
	node_has_id,
	node_has_attribute,
	node_has_attribute_equal,
	node_has_attribute_dashmatch,
	node_has_attribute_includes,
	node_has_attribute_prefix,
	node_has_attribute_suffix,
	node_has_attribute_substring,
	node_is_root,
	node_count_siblings,
	node_is_empty,
	node_is_link,
	node_is_visited,
	node_is_hover,
	node_is_active,
	node_is_focus,
	node_is_enabled,
	node_is_disabled,
	node_is_checked,
	node_is_target,
	node_is_lang,
	node_presentational_hint,
	ua_default_for_property,
	set_libcss_node_data,
	get_libcss_node_data,
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
		ثابت حرف *charset, ثابت حرف *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
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
	params.color = ns_system_colour;
	params.color_pw = عدم;
	params.font = عدم;
	params.font_pw = عدم;

	error = css_stylesheet_create(&params, &sheet);
	אם (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		החזר عدم;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	אם (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		החזר عدم;
	}

	error = css_stylesheet_data_done(sheet);
	אם (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		החזר عدم;
	}

	החזר sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساكن فراغ nscss_dom_user_data_handler(dom_node_operation operation,
		dom_string *key, فراغ *data, بنية dom_node *src,
		بنية dom_node *dst)
{
	css_error error;

	אם (dom_string_isequal(corestring_dom___ns_key_libcss_node_data,
			key) == false || data == عدم) {
		החזר;
	}

	بدل (operation) {
	حالة DOM_NODE_CLONED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_CLONED,
				عدم, src, dst, data);
		אם (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		اكسر;

	حالة DOM_NODE_RENAMED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_MODIFIED,
				عدم, src, عدم, data);
		אם (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		اكسر;

	حالة DOM_NODE_IMPORTED:
	حالة DOM_NODE_ADOPTED:
	حالة DOM_NODE_DELETED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_DELETED,
				عدم, src, عدم, data);
		אם (error != CSS_OK)
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_node *n,
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

	אם (error != CSS_OK || styles == عدم) {
		/* Failed selecting partial style -- bail out */
		החזר عدم;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	אם (ctx->parent_style != عدم) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		אם (error != CSS_OK) {
			css_select_results_destroy(styles);
			החזר عدم;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	עבור (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_COUNT;
			pseudo_element++) {

		אם (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LINE)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			استمر;

		אם (styles->styles[pseudo_element] == عدم)
			/* There were no rules concerning this pseudo element */
			استمر;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		אם (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			החזר عدم;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	החזר styles;
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
	אם (error != CSS_OK) {
		החזר عدم;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	אם (error != CSS_OK) {
		css_computed_style_destroy(composed);
		החזר عدم;
	}

	החזר composed;
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
css_error node_name(فراغ *pw, فراغ *node, css_qname *qname)
{
	dom_node *n = node;
	dom_string *name;
	dom_exception err;

	err = dom_node_get_node_name(n, &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	qname->ns = عدم;

	err = dom_string_intern(name, &qname->name);
	אם (err != DOM_NO_ERR) {
		dom_string_unref(name);
		החזר CSS_NOMEM;
	}

	dom_string_unref(name);

	החזר CSS_OK;
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
css_error node_classes(فراغ *pw, فراغ *node,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_node *n = node;
	dom_exception err;

	*classes = عدم;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	החזר CSS_OK;
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
css_error node_id(فراغ *pw, فراغ *node, lwc_string **id)
{
	dom_node *n = node;
	dom_string *attr;
	dom_exception err;

	*id = عدم;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	אם (attr != عدم) {
		err = dom_string_intern(attr, id);
		אם (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			החזר CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	החזר CSS_OK;
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
css_error named_ancestor_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **ancestor)
{
	dom_element_named_ancestor_node(node, qname->name,
			(بنية dom_element **)ancestor);
	dom_node_unref(*ancestor);

	החזר CSS_OK;
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
css_error named_parent_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **parent)
{
	dom_element_named_parent_node(node, qname->name,
			(بنية dom_element **)parent);
	dom_node_unref(*parent);

	החזר CSS_OK;
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
css_error named_sibling_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_node_get_previous_sibling(n, &n);
	אם (err != DOM_NO_ERR)
		החזר CSS_OK;

	כלעוד (n != عدم) {
		dom_node_type type;

		err = dom_node_get_node_type(n, &type);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		אם (type == DOM_ELEMENT_NODE)
			اكسر;

		err = dom_node_get_previous_sibling(n, &prev);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	אם (n != عدم) {
		dom_string *name;

		err = dom_node_get_node_name(n, &name);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		dom_node_unref(n);

		אם (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	החזר CSS_OK;
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
css_error named_generic_sibling_node(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	*sibling = عدم;

	err = dom_node_get_previous_sibling(n, &n);
	אם (err != DOM_NO_ERR)
		החזר CSS_OK;

	כלעוד (n != عدم) {
		dom_node_type type;
		dom_string *name;

		err = dom_node_get_node_type(n, &type);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		אם (type == DOM_ELEMENT_NODE) {
			err = dom_node_get_node_name(n, &name);
			אם (err != DOM_NO_ERR) {
				dom_node_unref(n);
				החזר CSS_OK;
			}

			אם (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_node_unref(n);
				*sibling = n;
				اكسر;
			}
			dom_string_unref(name);
		}

		err = dom_node_get_previous_sibling(n, &prev);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	החזר CSS_OK;
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
css_error parent_node(فراغ *pw, فراغ *node, فراغ **parent)
{
	dom_element_parent_node(node, (بنية dom_element **)parent);
	dom_node_unref(*parent);

	החזר CSS_OK;
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
css_error sibling_node(فراغ *pw, فراغ *node, فراغ **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_node_get_previous_sibling(n, &n);
	אם (err != DOM_NO_ERR)
		החזר CSS_OK;

	כלעוד (n != عدم) {
		dom_node_type type;

		err = dom_node_get_node_type(n, &type);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		אם (type == DOM_ELEMENT_NODE)
			اكسر;

		err = dom_node_get_previous_sibling(n, &prev);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	אם (n != عدم) {
		/** \todo Sort out reference counting */
		dom_node_unref(n);

		*sibling = n;
	}

	החזר CSS_OK;
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
css_error node_has_name(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_node *n = node;

	אם (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_node_get_node_name(n, &name);
		אם (err != DOM_NO_ERR)
			החזר CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	החזר CSS_OK;
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
css_error node_has_class(فراغ *pw, فراغ *node,
		lwc_string *name, bool *match)
{
	dom_node *n = node;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

	أكد(err == DOM_NO_ERR);

	החזר CSS_OK;
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
css_error node_has_id(فراغ *pw, فراغ *node,
		lwc_string *name, bool *match)
{
	dom_node *n = node;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	אם (err != DOM_NO_ERR)
		החזר CSS_OK;

	אם (attr != عدم) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	החזר CSS_OK;
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
css_error node_has_attribute(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	אם (err != DOM_NO_ERR) {
		dom_string_unref(name);
		החזר CSS_OK;
	}

	dom_string_unref(name);

	החזר CSS_OK;
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
css_error node_has_attribute_equal(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(value);

	אם (vlen == 0) {
		*match = false;
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_has_attribute_dashmatch(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(value);

	אם (vlen == 0) {
		*match = false;
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for dashmatch */
	אם (*match == false) {
		ثابت حرف *vdata = lwc_string_data(value);
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		אם (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_has_attribute_includes(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(value);
	ثابت حرف *p;
	ثابت حرف *start;
	ثابت حرف *end;

	*match = false;

	אם (vlen == 0) {
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ثابت حرف *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	עבור (p = start; p <= end; p++) {
		אם (*p == ' ' || *p == '\0') {
			אם ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(value),
					vlen) == 0) {
				*match = true;
				اكسر;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_has_attribute_prefix(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(value);

	אם (vlen == 0) {
		*match = false;
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	אם (*match == false) {
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		אם ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(value), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_has_attribute_suffix(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(value);

	אם (vlen == 0) {
		*match = false;
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	אם (*match == false) {
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ثابت حرف *start = (حرف *) data + len - vlen;

		אם ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(value), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_has_attribute_substring(فراغ *pw, فراغ *node,
		ثابت css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(value);

	אם (vlen == 0) {
		*match = false;
		החזר CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	אם (err != DOM_NO_ERR)
		החזר CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	אם ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		החזר CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	אם (*match == false) {
		ثابت حرف *vdata = lwc_string_data(value);
		ثابت حرف *start = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ثابت حرف *last_start = start + len - vlen;

		אם (len >= vlen) {
			כלעוד (start <= last_start) {
				אם (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					اكسر;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	החזר CSS_OK;
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
css_error node_is_root(فراغ *pw, فراغ *node, bool *match)
{
	dom_node *n = node;
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	אם (err != DOM_NO_ERR) {
		החזר CSS_NOMEM;
	}

	אם (parent != عدم) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		אם (err != DOM_NO_ERR)
			החזר CSS_NOMEM;

		אם (type != DOM_DOCUMENT_NODE) {
			*match = false;
			החזר CSS_OK;
		}
	}

	*match = true;

	החזר CSS_OK;
}

ساكن صحيح
node_count_siblings_check(dom_node *node,
			  bool check_name,
			  dom_string *name)
{
	dom_node_type type;
	صحيح ret = 0;
	dom_exception exc;

	אם (node == عدم)
		החזר 0;

	exc = dom_node_get_node_type(node, &type);
	אם ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_NODE)) {
		החזר 0;
	}

	אם (check_name) {
		dom_string *node_name = عدم;
		exc = dom_node_get_node_name(node, &node_name);

		אם ((exc == DOM_NO_ERR) && (node_name != عدم)) {

			אם (dom_string_caseless_isequal(name,
							node_name)) {
				ret = 1;
			}
			dom_string_unref(node_name);
		}
	} אחרת {
		ret = 1;
	}

	החזר ret;
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
css_error node_count_siblings(فراغ *pw, فراغ *n, bool same_name,
		bool after, int32_t *count)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *node_name = عدم;

	אם (same_name) {
		dom_node *node = n;
		exc = dom_node_get_node_name(node, &node_name);
		אם ((exc != DOM_NO_ERR) || (node_name == عدم)) {
			החזר CSS_NOMEM;
		}
	}

	אם (after) {
		dom_node *node = dom_node_ref(n);
		dom_node *next;

		עשה {
			exc = dom_node_get_next_sibling(node, &next);
			אם ((exc != DOM_NO_ERR))
				اكسر;

			dom_node_unref(node);
			node = next;

			cnt += node_count_siblings_check(node, same_name, node_name);
		} כלעוד (node != عدم);
	} אחרת {
		dom_node *node = dom_node_ref(n);
		dom_node *next;

		עשה {
			exc = dom_node_get_previous_sibling(node, &next);
			אם ((exc != DOM_NO_ERR))
				اكسر;

			dom_node_unref(node);
			node = next;

			cnt += node_count_siblings_check(node, same_name, node_name);

		} כלעוד (node != عدم);
	}

	אם (node_name != عدم) {
		dom_string_unref(node_name);
	}

	*count = cnt;
	החזר CSS_OK;
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
css_error node_is_empty(فراغ *pw, فراغ *node, bool *match)
{
	dom_node *n = node, *next;
	dom_exception err;

	*match = true;

	err = dom_node_get_first_child(n, &n);
	אם (err != DOM_NO_ERR) {
		החזר CSS_BADPARM;
	}

	כלעוד (n != عدم) {
		dom_node_type ntype;
		err = dom_node_get_node_type(n, &ntype);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_BADPARM;
		}

		אם (ntype == DOM_ELEMENT_NODE ||
		    ntype == DOM_TEXT_NODE) {
			*match = false;
			dom_node_unref(n);
			اكسر;
		}

		err = dom_node_get_next_sibling(n, &next);
		אם (err != DOM_NO_ERR) {
			dom_node_unref(n);
			החזר CSS_BADPARM;
		}
		dom_node_unref(n);
		n = next;
	}

	החזר CSS_OK;
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
css_error node_is_link(فراغ *pw, فراغ *n, bool *match)
{
	dom_node *node = n;
	dom_exception exc;
	dom_string *node_name = عدم;

	exc = dom_node_get_node_name(node, &node_name);
	אם ((exc != DOM_NO_ERR) || (node_name == عدم)) {
		החזר CSS_NOMEM;
	}

	אם (dom_string_caseless_lwc_isequal(node_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(node, corestring_dom_href,
				&has_href);
		אם ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} אחרת {
			*match = false;
		}
	} אחרת {
		*match = false;
	}
	dom_string_unref(node_name);

	החזר CSS_OK;
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
css_error node_is_visited(فراغ *pw, فراغ *node, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت بنية url_data *data;

	dom_exception exc;
	dom_node *n = node;
	dom_string *s = عدم;

	*match = false;

	exc = dom_node_get_node_name(n, &s);
	אם ((exc != DOM_NO_ERR) || (s == عدم)) {
		החזר CSS_NOMEM;
	}

	אם (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		החזר CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = عدم;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	אם ((exc != DOM_NO_ERR) || (s == عدم)) {
		/* Can't be visited; not got a URL */
		החזר CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	אם (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		החזר CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	אם (data != عدم && data->visits > 0)
		*match = true;

	nsurl_unref(url);

	החזר CSS_OK;
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
css_error node_is_hover(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support hovering */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_active(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_focus(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_enabled(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_disabled(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_checked(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_target(فراغ *pw, فراغ *node, bool *match)
{
	/** \todo Support target */

	*match = false;

	החזר CSS_OK;
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
css_error node_is_lang(فراغ *pw, فراغ *node,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

	החזר CSS_OK;
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
	אם (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} אחרת אם (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = عدم;
		بدل (nsoption_int(font_default)) {
		حالة PLOT_FONT_FAMILY_SANS_SERIF:
			hint->status = CSS_FONT_FAMILY_SANS_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_SERIF:
			hint->status = CSS_FONT_FAMILY_SERIF;
			اكسر;
		حالة PLOT_FONT_FAMILY_MONOSPACE:
			hint->status = CSS_FONT_FAMILY_MONOSPACE;
			اكسر;
		حالة PLOT_FONT_FAMILY_CURSIVE:
			hint->status = CSS_FONT_FAMILY_CURSIVE;
			اكسر;
		حالة PLOT_FONT_FAMILY_FANTASY:
			hint->status = CSS_FONT_FAMILY_FANTASY;
			اكسر;
		}
	} אחרת אם (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = عدم;
		hint->status = CSS_QUOTES_NONE;
	} אחרת אם (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = عدم;
		hint->status = 0;
	} אחרת {
		החזר CSS_INVALID;
	}

	החזר CSS_OK;
}

css_error set_libcss_node_data(فراغ *pw, فراغ *node, فراغ *libcss_node_data)
{
	dom_node *n = node;
	dom_exception err;
	فراغ *old_node_data;

	/* Set this node's node data */
	err = dom_node_set_user_data(n,
			corestring_dom___ns_key_libcss_node_data,
			libcss_node_data, nscss_dom_user_data_handler,
			(فراغ *) &old_node_data);
	אם (err != DOM_NO_ERR) {
		החזר CSS_NOMEM;
	}

	أكد(old_node_data == عدم);

	החזר CSS_OK;
}

css_error get_libcss_node_data(فراغ *pw, فراغ *node, فراغ **libcss_node_data)
{
	dom_node *n = node;
	dom_exception err;

	/* Get this node's node data */
	err = dom_node_get_user_data(n,
			corestring_dom___ns_key_libcss_node_data,
			libcss_node_data);
	אם (err != DOM_NO_ERR) {
		החזר CSS_NOMEM;
	}

	החזר CSS_OK;
}
