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

ساكن css_error تۈگۈن_name(فراغ *pw, فراغ *تۈگۈن, css_qname *qname);
ساكن css_error تۈگۈن_classes(فراغ *pw, فراغ *تۈگۈن,
		lwc_string ***classes, uint32_t *n_classes);
ساكن css_error تۈگۈن_id(فراغ *pw, فراغ *تۈگۈن, lwc_string **id);
ساكن css_error named_parent_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **parent);
ساكن css_error named_sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error named_generic_sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **sibling);
ساكن css_error parent_تۈگۈن(فراغ *pw, فراغ *تۈگۈن, فراغ **parent);
ساكن css_error sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن, فراغ **sibling);
ساكن css_error تۈگۈن_has_name(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, bool *match);
ساكن css_error تۈگۈن_has_class(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *name, bool *match);
ساكن css_error تۈگۈن_has_id(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *name, bool *match);
ساكن css_error تۈگۈن_has_attribute(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, bool *match);
ساكن css_error تۈگۈن_has_attribute_equal(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_has_attribute_dashmatch(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_has_attribute_includes(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_has_attribute_prefix(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_has_attribute_suffix(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_has_attribute_substring(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match);
ساكن css_error تۈگۈن_is_يىلتىز(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_سانى_siblings(فراغ *pw, فراغ *تۈگۈن,
		bool same_name, bool after, int32_t *سانى);
ساكن css_error تۈگۈن_is_empty(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_link(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_hover(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_active(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_focus(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_enabled(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_disabled(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_checked(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_target(فراغ *pw, فراغ *تۈگۈن, bool *match);
ساكن css_error تۈگۈن_is_lang(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *lang, bool *match);
ساكن css_error ua_default_for_property(فراغ *pw, uint32_t property,
		css_hint *hint);
ساكن css_error set_libcss_تۈگۈن_data(فراغ *pw, فراغ *تۈگۈن,
		فراغ *libcss_تۈگۈن_data);
ساكن css_error get_libcss_تۈگۈن_data(فراغ *pw, فراغ *تۈگۈن,
		فراغ **libcss_تۈگۈن_data);

/**
 * Selection callback table for libcss
 */
ساكن css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	تۈگۈن_name,
	تۈگۈن_classes,
	تۈگۈن_id,
	named_ancestor_تۈگۈن,
	named_parent_تۈگۈن,
	named_sibling_تۈگۈن,
	named_generic_sibling_تۈگۈن,
	parent_تۈگۈن,
	sibling_تۈگۈن,
	تۈگۈن_has_name,
	تۈگۈن_has_class,
	تۈگۈن_has_id,
	تۈگۈن_has_attribute,
	تۈگۈن_has_attribute_equal,
	تۈگۈن_has_attribute_dashmatch,
	تۈگۈن_has_attribute_includes,
	تۈگۈن_has_attribute_prefix,
	تۈگۈن_has_attribute_suffix,
	تۈگۈن_has_attribute_substring,
	تۈگۈن_is_يىلتىز,
	تۈگۈن_سانى_siblings,
	تۈگۈن_is_empty,
	تۈگۈن_is_link,
	تۈگۈن_is_visited,
	تۈگۈن_is_hover,
	تۈگۈن_is_active,
	تۈگۈن_is_focus,
	تۈگۈن_is_enabled,
	تۈگۈن_is_disabled,
	تۈگۈن_is_checked,
	تۈگۈن_is_target,
	تۈگۈن_is_lang,
	تۈگۈن_presentational_hint,
	ua_default_for_property,
	set_libcss_تۈگۈن_data,
	get_libcss_تۈگۈن_data,
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
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		أرجع عدم;
	}

	error = css_stylesheet_append_data(sheet, data, len);
	إذا (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	error = css_stylesheet_data_done(sheet);
	إذا (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		أرجع عدم;
	}

	أرجع sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
ساكن فراغ nscss_dom_user_data_handler(dom_تۈگۈن_operation operation,
		dom_string *ئاچقۇچ, فراغ *data, بنية dom_تۈگۈن *src,
		بنية dom_تۈگۈن *dst)
{
	css_error error;

	إذا (dom_string_isequal(corestring_dom_ns_ئاچقۇچ_libcss_تۈگۈن_data,
			ئاچقۇچ) == false || data == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_تۈگۈن_CLONED:
		error = css_libcss_تۈگۈن_data_handler(&selection_handler,
				CSS_تۈگۈن_CLONED,
				عدم, src, dst, data);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		اكسر;

	حالة DOM_تۈگۈن_RENAMED:
		error = css_libcss_تۈگۈن_data_handler(&selection_handler,
				CSS_تۈگۈن_MODIFIED,
				عدم, src, عدم, data);
		إذا (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		اكسر;

	حالة DOM_تۈگۈن_IMPORTED:
	حالة DOM_تۈگۈن_ADOPTED:
	حالة DOM_تۈگۈن_DELETED:
		error = css_libcss_تۈگۈن_data_handler(&selection_handler,
				CSS_تۈگۈن_DELETED,
				عدم, src, عدم, data);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_تۈگۈن *n,
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
	إذا (ctx->parent_style != عدم) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
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
			pseudo_element < CSS_PSEUDO_ELEMENT_سانى;
			pseudo_element++) {

		إذا (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_قۇر)
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
		ثابت css_computed_style *parent)
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
	error = css_computed_style_compose(parent, partial,
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
css_error تۈگۈن_name(فراغ *pw, فراغ *تۈگۈن, css_qname *qname)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_exception err;

	err = dom_تۈگۈن_get_تۈگۈن_name(n, &name);
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
css_error تۈگۈن_classes(فراغ *pw, فراغ *تۈگۈن,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_تۈگۈن *n = تۈگۈن;
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
css_error تۈگۈن_id(فراغ *pw, فراغ *تۈگۈن, lwc_string **id)
{
	dom_تۈگۈن *n = تۈگۈن;
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
css_error named_ancestor_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **ancestor)
{
	dom_element_named_ancestor_تۈگۈن(تۈگۈن, qname->name,
			(بنية dom_element **)ancestor);
	dom_تۈگۈن_unref(*ancestor);

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
css_error named_parent_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **parent)
{
	dom_element_named_parent_تۈگۈن(تۈگۈن, qname->name,
			(بنية dom_element **)parent);
	dom_تۈگۈن_unref(*parent);

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
css_error named_sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_تۈگۈن *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_تۈگۈن_get_previous_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_تۈگۈن_type type;

		err = dom_تۈگۈن_get_تۈگۈن_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_تۈگۈن)
			اكسر;

		err = dom_تۈگۈن_get_previous_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		dom_تۈگۈن_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		dom_string *name;

		err = dom_تۈگۈن_get_تۈگۈن_name(n, &name);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		dom_تۈگۈن_unref(n);

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
css_error named_generic_sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, فراغ **sibling)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_تۈگۈن *prev;
	dom_exception err;

	*sibling = عدم;

	err = dom_تۈگۈن_get_previous_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_تۈگۈن_type type;
		dom_string *name;

		err = dom_تۈگۈن_get_تۈگۈن_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_تۈگۈن) {
			err = dom_تۈگۈن_get_تۈگۈن_name(n, &name);
			إذا (err != DOM_NO_ERR) {
				dom_تۈگۈن_unref(n);
				أرجع CSS_OK;
			}

			إذا (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_تۈگۈن_unref(n);
				*sibling = n;
				اكسر;
			}
			dom_string_unref(name);
		}

		err = dom_تۈگۈن_get_previous_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		dom_تۈگۈن_unref(n);
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
css_error parent_تۈگۈن(فراغ *pw, فراغ *تۈگۈن, فراغ **parent)
{
	dom_element_parent_تۈگۈن(تۈگۈن, (بنية dom_element **)parent);
	dom_تۈگۈن_unref(*parent);

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
css_error sibling_تۈگۈن(فراغ *pw, فراغ *تۈگۈن, فراغ **sibling)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_تۈگۈن *prev;
	dom_exception err;

	*sibling = عدم;

	/* Find sibling element */
	err = dom_تۈگۈن_get_previous_sibling(n, &n);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	طالما (n != عدم) {
		dom_تۈگۈن_type type;

		err = dom_تۈگۈن_get_تۈگۈن_type(n, &type);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		إذا (type == DOM_ELEMENT_تۈگۈن)
			اكسر;

		err = dom_تۈگۈن_get_previous_sibling(n, &prev);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_OK;
		}

		dom_تۈگۈن_unref(n);
		n = prev;
	}

	إذا (n != عدم) {
		/** \todo Sort out reference counting */
		dom_تۈگۈن_unref(n);

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
css_error تۈگۈن_has_name(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, bool *match)
{
	nscss_select_ctx *ctx = pw;
	dom_تۈگۈن *n = تۈگۈن;

	إذا (lwc_string_isequal(qname->name, ctx->universal, match) ==
			lwc_error_ok && *match == false) {
		dom_string *name;
		dom_exception err;

		err = dom_تۈگۈن_get_تۈگۈن_name(n, &name);
		إذا (err != DOM_NO_ERR)
			أرجع CSS_OK;

		/* Element names are case insensitive in HTML */
		*match = dom_string_caseless_lwc_isequal(name, qname->name);

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
css_error تۈگۈن_has_class(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *name, bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, match);

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
css_error تۈگۈن_has_id(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *name, bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *attr;
	dom_exception err;

	*match = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_OK;

	إذا (attr != عدم) {
		*match = dom_string_lwc_isequal(attr, name);

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
css_error تۈگۈن_has_attribute(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ثابت uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
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
css_error تۈگۈن_has_attribute_equal(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(قىممەت);

	إذا (vlen == 0) {
		*match = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, قىممەت);

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
css_error تۈگۈن_has_attribute_dashmatch(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(قىممەت);

	إذا (vlen == 0) {
		*match = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, قىممەت);

	/* check for dashmatch */
	إذا (*match == false) {
		ثابت حرف *vdata = lwc_string_data(قىممەت);
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		إذا (len > vlen && data[vlen] == '-' &&
		    strncasecmp(data, vdata, vlen) == 0) {
				*match = true;
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
css_error تۈگۈن_has_attribute_includes(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(قىممەت);
	ثابت حرف *p;
	ثابت حرف *start;
	ثابت حرف *end;

	*match = false;

	إذا (vlen == 0) {
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ثابت حرف *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	لكل (p = start; p <= end; p++) {
		إذا (*p == ' ' || *p == '\0') {
			إذا ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_data(قىممەت),
					vlen) == 0) {
				*match = true;
				اكسر;
			}

			start = p + 1;
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
css_error تۈگۈن_has_attribute_prefix(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(قىممەت);

	إذا (vlen == 0) {
		*match = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, قىممەت);

	/* check for prefix match */
	إذا (*match == false) {
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		إذا ((len >= vlen) &&
		    (strncasecmp(data, lwc_string_data(قىممەت), vlen) == 0)) {
			*match = true;
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
css_error تۈگۈن_has_attribute_suffix(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(قىممەت);

	إذا (vlen == 0) {
		*match = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, قىممەت);

	/* check for prefix match */
	إذا (*match == false) {
		ثابت حرف *data = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		ثابت حرف *start = (حرف *) data + len - vlen;

		إذا ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_data(قىممەت), vlen) == 0)) {
			*match = true;
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
css_error تۈگۈن_has_attribute_substring(فراغ *pw, فراغ *تۈگۈن,
		ثابت css_qname *qname, lwc_string *قىممەت,
		bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_length(قىممەت);

	إذا (vlen == 0) {
		*match = false;
		أرجع CSS_OK;
	}

	err = dom_string_create_interned(
		(ثابت uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	إذا (err != DOM_NO_ERR)
		أرجع CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	إذا ((err != DOM_NO_ERR) || (atr_val == عدم)) {
		dom_string_unref(name);
		*match = false;
		أرجع CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, قىممەت);

	/* check for prefix match */
	إذا (*match == false) {
		ثابت حرف *vdata = lwc_string_data(قىممەت);
		ثابت حرف *start = (ثابت حرف *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		ثابت حرف *last_start = start + len - vlen;

		إذا (len >= vlen) {
			طالما (start <= last_start) {
				إذا (strncasecmp(start, vdata,
						vlen) == 0) {
					*match = true;
					اكسر;
				}

				start++;
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
css_error تۈگۈن_is_يىلتىز(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_تۈگۈن *parent;
	dom_تۈگۈن_type type;
	dom_exception err;

	err = dom_تۈگۈن_get_parent_تۈگۈن(n, &parent);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	إذا (parent != عدم) {
		err = dom_تۈگۈن_get_تۈگۈن_type(parent, &type);

		dom_تۈگۈن_unref(parent);

		إذا (err != DOM_NO_ERR)
			أرجع CSS_NOMEM;

		إذا (type != DOM_DOCUMENT_تۈگۈن) {
			*match = false;
			أرجع CSS_OK;
		}
	}

	*match = true;

	أرجع CSS_OK;
}

ساكن صحيح
تۈگۈن_سانى_siblings_check(dom_تۈگۈن *تۈگۈن,
			  bool check_name,
			  dom_string *name)
{
	dom_تۈگۈن_type type;
	صحيح ret = 0;
	dom_exception exc;

	إذا (تۈگۈن == عدم)
		أرجع 0;

	exc = dom_تۈگۈن_get_تۈگۈن_type(تۈگۈن, &type);
	إذا ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_تۈگۈن)) {
		أرجع 0;
	}

	إذا (check_name) {
		dom_string *تۈگۈن_name = عدم;
		exc = dom_تۈگۈن_get_تۈگۈن_name(تۈگۈن, &تۈگۈن_name);

		إذا ((exc == DOM_NO_ERR) && (تۈگۈن_name != عدم)) {

			إذا (dom_string_caseless_isequal(name,
							تۈگۈن_name)) {
				ret = 1;
			}
			dom_string_unref(تۈگۈن_name);
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
css_error تۈگۈن_سانى_siblings(فراغ *pw, فراغ *n, bool same_name,
		bool after, int32_t *سانى)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *تۈگۈن_name = عدم;

	إذا (same_name) {
		dom_تۈگۈن *تۈگۈن = n;
		exc = dom_تۈگۈن_get_تۈگۈن_name(تۈگۈن, &تۈگۈن_name);
		إذا ((exc != DOM_NO_ERR) || (تۈگۈن_name == عدم)) {
			أرجع CSS_NOMEM;
		}
	}

	إذا (after) {
		dom_تۈگۈن *تۈگۈن = dom_تۈگۈن_ref(n);
		dom_تۈگۈن *next;

		افعل {
			exc = dom_تۈگۈن_get_next_sibling(تۈگۈن, &next);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_تۈگۈن_unref(تۈگۈن);
			تۈگۈن = next;

			cnt += تۈگۈن_سانى_siblings_check(تۈگۈن, same_name, تۈگۈن_name);
		} طالما (تۈگۈن != عدم);
	} وإلا {
		dom_تۈگۈن *تۈگۈن = dom_تۈگۈن_ref(n);
		dom_تۈگۈن *next;

		افعل {
			exc = dom_تۈگۈن_get_previous_sibling(تۈگۈن, &next);
			إذا ((exc != DOM_NO_ERR))
				اكسر;

			dom_تۈگۈن_unref(تۈگۈن);
			تۈگۈن = next;

			cnt += تۈگۈن_سانى_siblings_check(تۈگۈن, same_name, تۈگۈن_name);

		} طالما (تۈگۈن != عدم);
	}

	إذا (تۈگۈن_name != عدم) {
		dom_string_unref(تۈگۈن_name);
	}

	*سانى = cnt;
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
css_error تۈگۈن_is_empty(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	dom_تۈگۈن *n = تۈگۈن, *next;
	dom_exception err;

	*match = true;

	err = dom_تۈگۈن_get_first_child(n, &n);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_BADPARM;
	}

	طالما (n != عدم) {
		dom_تۈگۈن_type ntype;
		err = dom_تۈگۈن_get_تۈگۈن_type(n, &ntype);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_BADPARM;
		}

		إذا (ntype == DOM_ELEMENT_تۈگۈن ||
		    ntype == DOM_TEXT_تۈگۈن) {
			*match = false;
			dom_تۈگۈن_unref(n);
			اكسر;
		}

		err = dom_تۈگۈن_get_next_sibling(n, &next);
		إذا (err != DOM_NO_ERR) {
			dom_تۈگۈن_unref(n);
			أرجع CSS_BADPARM;
		}
		dom_تۈگۈن_unref(n);
		n = next;
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
css_error تۈگۈن_is_link(فراغ *pw, فراغ *n, bool *match)
{
	dom_تۈگۈن *تۈگۈن = n;
	dom_exception exc;
	dom_string *تۈگۈن_name = عدم;

	exc = dom_تۈگۈن_get_تۈگۈن_name(تۈگۈن, &تۈگۈن_name);
	إذا ((exc != DOM_NO_ERR) || (تۈگۈن_name == عدم)) {
		أرجع CSS_NOMEM;
	}

	إذا (dom_string_caseless_lwc_isequal(تۈگۈن_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(تۈگۈن, corestring_dom_href,
				&has_href);
		إذا ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} وإلا {
			*match = false;
		}
	} وإلا {
		*match = false;
	}
	dom_string_unref(تۈگۈن_name);

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
css_error تۈگۈن_is_visited(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ثابت بنية url_data *data;

	dom_exception exc;
	dom_تۈگۈن *n = تۈگۈن;
	dom_string *s = عدم;

	*match = false;

	exc = dom_تۈگۈن_get_تۈگۈن_name(n, &s);
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
	error = nsurl_join(ctx->base_url, dom_string_data(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	إذا (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		أرجع CSS_NOMEM;
	}

	data = urldb_get_url_data(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	إذا (data != عدم && data->visits > 0)
		*match = true;

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
css_error تۈگۈن_is_hover(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support hovering */

	*match = false;

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
css_error تۈگۈن_is_active(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support active nodes */

	*match = false;

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
css_error تۈگۈن_is_focus(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support focussed nodes */

	*match = false;

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
css_error تۈگۈن_is_enabled(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support enabled nodes */

	*match = false;

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
css_error تۈگۈن_is_disabled(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support disabled nodes */

	*match = false;

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
css_error تۈگۈن_is_checked(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support checked nodes */

	*match = false;

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
css_error تۈگۈن_is_target(فراغ *pw, فراغ *تۈگۈن, bool *match)
{
	/** \todo Support target */

	*match = false;

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
css_error تۈگۈن_is_lang(فراغ *pw, فراغ *تۈگۈن,
		lwc_string *lang, bool *match)
{
	/** \todo Support languages */

	*match = false;

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
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} وإلا إذا (property == CSS_PROP_FONT_FAMILY) {
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
	} وإلا إذا (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = عدم;
		hint->status = CSS_QUOTES_NONE;
	} وإلا إذا (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = عدم;
		hint->status = 0;
	} وإلا {
		أرجع CSS_INVALID;
	}

	أرجع CSS_OK;
}

css_error set_libcss_تۈگۈن_data(فراغ *pw, فراغ *تۈگۈن, فراغ *libcss_تۈگۈن_data)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_exception err;
	فراغ *old_تۈگۈن_data;

	/* Set this node's node data */
	err = dom_تۈگۈن_set_user_data(n,
			corestring_dom_ns_ئاچقۇچ_libcss_تۈگۈن_data,
			libcss_تۈگۈن_data, nscss_dom_user_data_handler,
			(فراغ *) &old_تۈگۈن_data);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أكد(old_تۈگۈن_data == عدم);

	أرجع CSS_OK;
}

css_error get_libcss_تۈگۈن_data(فراغ *pw, فراغ *تۈگۈن, فراغ **libcss_تۈگۈن_data)
{
	dom_تۈگۈن *n = تۈگۈن;
	dom_exception err;

	/* Get this node's node data */
	err = dom_تۈگۈن_get_user_data(n,
			corestring_dom_ns_ئاچقۇچ_libcss_تۈگۈن_data,
			libcss_تۈگۈن_data);
	إذا (err != DOM_NO_ERR) {
		أرجع CSS_NOMEM;
	}

	أرجع CSS_OK;
}
