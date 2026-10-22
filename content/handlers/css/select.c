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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/plot_style.h"
#अन्तर्भाव "netsurf/url_db.h"
#अन्तर्भाव "desktop/system_colour.h"

#अन्तर्भाव "css/internal.h"
#अन्तर्भाव "css/hints.h"
#अन्तर्भाव "css/select.h"

स्थिर css_error ग्रन्थिः_name(शून्य *pw, शून्य *ग्रन्थिः, css_qname *qname);
स्थिर css_error ग्रन्थिः_classes(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string ***classes, uint32_t *n_classes);
स्थिर css_error ग्रन्थिः_id(शून्य *pw, शून्य *ग्रन्थिः, lwc_string **id);
स्थिर css_error named_parent_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **parent);
स्थिर css_error named_sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **sibling);
स्थिर css_error named_generic_sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **sibling);
स्थिर css_error parent_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः, शून्य **parent);
स्थिर css_error sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः, शून्य **sibling);
स्थिर css_error ग्रन्थिः_has_name(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_class(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *name, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_id(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *name, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_equal(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_dashmatch(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_includes(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_उपसर्गः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_suffix(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_has_attribute_substring(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_मूलम्(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_गणनम्_siblings(शून्य *pw, शून्य *ग्रन्थिः,
		bool same_name, bool after, int32_t *गणनम्);
स्थिर css_error ग्रन्थिः_is_रिक्तानि(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_link(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_hover(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_सक्रिय(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_focus(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_enabled(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_disabled(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_checked(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_target(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्);
स्थिर css_error ग्रन्थिः_is_lang(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *lang, bool *मेलनम्);
स्थिर css_error ua_default_for_property(शून्य *pw, uint32_t property,
		css_hint *hint);
स्थिर css_error समुच्चयः_libcss_ग्रन्थिः_दत्त(शून्य *pw, शून्य *ग्रन्थिः,
		शून्य *libcss_ग्रन्थिः_दत्त);
स्थिर css_error get_libcss_ग्रन्थिः_दत्त(शून्य *pw, शून्य *ग्रन्थिः,
		शून्य **libcss_ग्रन्थिः_दत्त);

/**
 * Selection callback table for libcss
 */
स्थिर css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_संस्करणम्_1,

	ग्रन्थिः_name,
	ग्रन्थिः_classes,
	ग्रन्थिः_id,
	named_ancestor_ग्रन्थिः,
	named_parent_ग्रन्थिः,
	named_sibling_ग्रन्थिः,
	named_generic_sibling_ग्रन्थिः,
	parent_ग्रन्थिः,
	sibling_ग्रन्थिः,
	ग्रन्थिः_has_name,
	ग्रन्थिः_has_class,
	ग्रन्थिः_has_id,
	ग्रन्थिः_has_attribute,
	ग्रन्थिः_has_attribute_equal,
	ग्रन्थिः_has_attribute_dashmatch,
	ग्रन्थिः_has_attribute_includes,
	ग्रन्थिः_has_attribute_उपसर्गः,
	ग्रन्थिः_has_attribute_suffix,
	ग्रन्थिः_has_attribute_substring,
	ग्रन्थिः_is_मूलम्,
	ग्रन्थिः_गणनम्_siblings,
	ग्रन्थिः_is_रिक्तानि,
	ग्रन्थिः_is_link,
	ग्रन्थिः_is_visited,
	ग्रन्थिः_is_hover,
	ग्रन्थिः_is_सक्रिय,
	ग्रन्थिः_is_focus,
	ग्रन्थिः_is_enabled,
	ग्रन्थिः_is_disabled,
	ग्रन्थिः_is_checked,
	ग्रन्थिः_is_target,
	ग्रन्थिः_is_lang,
	ग्रन्थिः_presentational_hint,
	ua_default_for_property,
	समुच्चयः_libcss_ग्रन्थिः_दत्त,
	get_libcss_ग्रन्थिः_दत्त,
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
css_stylesheet *nscss_create_inline_style(ध्रुव uint8_t *दत्त, size_t len,
		ध्रुव वर्ण *charset, ध्रुव वर्ण *url, bool allow_quirks)
{
	css_stylesheet_params params;
	css_stylesheet *sheet;
	css_error error;

	params.params_संस्करणम् = CSS_STYLESHEET_PARAMS_संस्करणम्_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = रिक्त;
	params.allow_quirks = allow_quirks;
	params.inline_style = true;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = रिक्त;
	params.import = रिक्त;
	params.import_pw = रिक्त;
	params.color = ns_system_रङ्गः;
	params.color_pw = रिक्त;
	params.font = रिक्त;
	params.font_pw = रिक्त;

	error = css_stylesheet_create(&params, &sheet);
	यदि (error != CSS_OK) {
		NSLOG(netsurf, INFO, "Failed creating sheet: %d", error);
		प्रत्यागम रिक्त;
	}

	error = css_stylesheet_append_दत्त(sheet, दत्त, len);
	यदि (error != CSS_OK && error != CSS_NEEDDATA) {
		NSLOG(netsurf, INFO, "failed appending data: %d", error);
		css_stylesheet_destroy(sheet);
		प्रत्यागम रिक्त;
	}

	error = css_stylesheet_दत्त_समाप्तम्(sheet);
	यदि (error != CSS_OK) {
		NSLOG(netsurf, INFO, "failed completing parse: %d", error);
		css_stylesheet_destroy(sheet);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम sheet;
}

/* Handler for libcss_node_data, stored as libdom node user data */
स्थिर शून्य nscss_dom_user_दत्त_handler(dom_ग्रन्थिः_operation operation,
		dom_string *कुञ्चिका, शून्य *दत्त, संरचना dom_ग्रन्थिः *src,
		संरचना dom_ग्रन्थिः *dst)
{
	css_error error;

	यदि (dom_string_isequal(corestring_dom_ns_कुञ्चिका_libcss_ग्रन्थिः_दत्त,
			कुञ्चिका) == false || दत्त == रिक्त) {
		प्रत्यागम;
	}

	विकल्प (operation) {
	पक्ष DOM_ग्रन्थिः_CLONED:
		error = css_libcss_ग्रन्थिः_दत्त_handler(&selection_handler,
				CSS_ग्रन्थिः_CLONED,
				रिक्त, src, dst, दत्त);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to clone libcss_node_data.");
		भङ्ग;

	पक्ष DOM_ग्रन्थिः_RENAMED:
		error = css_libcss_ग्रन्थिः_दत्त_handler(&selection_handler,
				CSS_ग्रन्थिः_MODIFIED,
				रिक्त, src, रिक्त, दत्त);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to update libcss_node_data.");
		भङ्ग;

	पक्ष DOM_ग्रन्थिः_IMPORTED:
	पक्ष DOM_ग्रन्थिः_ADOPTED:
	पक्ष DOM_ग्रन्थिः_DELETED:
		error = css_libcss_ग्रन्थिः_दत्त_handler(&selection_handler,
				CSS_ग्रन्थिः_DELETED,
				रिक्त, src, रिक्त, दत्त);
		यदि (error != CSS_OK)
			NSLOG(netsurf, INFO,
			      "Failed to delete libcss_node_data.");
		भङ्ग;

	स्वभाव:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		प्रतिज्ञा(0);
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
css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ग्रन्थिः *n,
		ध्रुव css_media *media,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_stylesheet *inline_style)
{
	css_computed_style *composed;
	css_select_results *styles;
	पूर्णाङ्क pseudo_element;
	css_error error;

	/* Select style for node */
	error = css_select_style(ctx->ctx, n, unit_len_ctx, media, inline_style,
			&selection_handler, ctx, &styles);

	यदि (error != CSS_OK || styles == रिक्त) {
		/* Failed selecting partial style -- bail out */
		प्रत्यागम रिक्त;
	}

	/* If there's a parent style, compose with partial to obtain
	 * complete computed style for element */
	यदि (ctx->parent_style != रिक्त) {
		/* Complete the computed style, by composing with the parent
		 * element's style */
		error = css_computed_style_compose(ctx->parent_style,
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				unit_len_ctx, &composed);
		यदि (error != CSS_OK) {
			css_select_results_destroy(styles);
			प्रत्यागम रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE]);
		styles->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
	}

	प्रति (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_गणनम्;
			pseudo_element++) {

		यदि (pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_LETTER ||
				pseudo_element == CSS_PSEUDO_ELEMENT_FIRST_पङ्क्तिः)
			/* TODO: Handle first-line and first-letter pseudo
			 *       element computed style completion */
			अनुवर्त;

		यदि (styles->styles[pseudo_element] == रिक्त)
			/* There were no rules concerning this pseudo element */
			अनुवर्त;

		/* Complete the pseudo element's computed style, by composing
		 * with the base element's style */
		error = css_computed_style_compose(
				styles->styles[CSS_PSEUDO_ELEMENT_NONE],
				styles->styles[pseudo_element],
				unit_len_ctx, &composed);
		यदि (error != CSS_OK) {
			/* TODO: perhaps this shouldn't be quite so
			 * catastrophic? */
			css_select_results_destroy(styles);
			प्रत्यागम रिक्त;
		}

		/* Replace select_results style with composed style */
		css_computed_style_destroy(styles->styles[pseudo_element]);
		styles->styles[pseudo_element] = composed;
	}

	प्रत्यागम styles;
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
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *parent)
{
	css_computed_style *partial, *composed;
	css_error error;

	error = css_select_default_style(ctx->ctx,
			&selection_handler, ctx, &partial);
	यदि (error != CSS_OK) {
		प्रत्यागम रिक्त;
	}

	/* TODO: Do we really need to compose?  Initial style shouldn't
	 * have any inherited properties. */
	error = css_computed_style_compose(parent, partial,
			unit_len_ctx, &composed);
	css_computed_style_destroy(partial);
	यदि (error != CSS_OK) {
		css_computed_style_destroy(composed);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम composed;
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
css_error ग्रन्थिः_name(शून्य *pw, शून्य *ग्रन्थिः, css_qname *qname)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_exception err;

	err = dom_ग्रन्थिः_get_ग्रन्थिः_name(n, &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	qname->ns = रिक्त;

	err = dom_string_intern(name, &qname->name);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		प्रत्यागम CSS_NOMEM;
	}

	dom_string_unref(name);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_classes(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_exception err;

	*classes = रिक्त;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_id(शून्य *pw, शून्य *ग्रन्थिः, lwc_string **id)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *attr;
	dom_exception err;

	*id = रिक्त;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	यदि (attr != रिक्त) {
		err = dom_string_intern(attr, id);
		यदि (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			प्रत्यागम CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	प्रत्यागम CSS_OK;
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
css_error named_ancestor_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **ancestor)
{
	dom_element_named_ancestor_ग्रन्थिः(ग्रन्थिः, qname->name,
			(संरचना dom_element **)ancestor);
	dom_ग्रन्थिः_unref(*ancestor);

	प्रत्यागम CSS_OK;
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
css_error named_parent_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **parent)
{
	dom_element_named_parent_ग्रन्थिः(ग्रन्थिः, qname->name,
			(संरचना dom_element **)parent);
	dom_ग्रन्थिः_unref(*parent);

	प्रत्यागम CSS_OK;
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
css_error named_sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **sibling)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_ग्रन्थिः *पूर्व;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_ग्रन्थिः_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_ग्रन्थिः_type type;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_ग्रन्थिः)
			भङ्ग;

		err = dom_ग्रन्थिः_get_previous_sibling(n, &पूर्व);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_ग्रन्थिः_unref(n);
		n = पूर्व;
	}

	यदि (n != रिक्त) {
		dom_string *name;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_name(n, &name);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_ग्रन्थिः_unref(n);

		यदि (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	प्रत्यागम CSS_OK;
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
css_error named_generic_sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, शून्य **sibling)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_ग्रन्थिः *पूर्व;
	dom_exception err;

	*sibling = रिक्त;

	err = dom_ग्रन्थिः_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_ग्रन्थिः_type type;
		dom_string *name;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_ग्रन्थिः) {
			err = dom_ग्रन्थिः_get_ग्रन्थिः_name(n, &name);
			यदि (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(n);
				प्रत्यागम CSS_OK;
			}

			यदि (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_ग्रन्थिः_unref(n);
				*sibling = n;
				भङ्ग;
			}
			dom_string_unref(name);
		}

		err = dom_ग्रन्थिः_get_previous_sibling(n, &पूर्व);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_ग्रन्थिः_unref(n);
		n = पूर्व;
	}

	प्रत्यागम CSS_OK;
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
css_error parent_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः, शून्य **parent)
{
	dom_element_parent_ग्रन्थिः(ग्रन्थिः, (संरचना dom_element **)parent);
	dom_ग्रन्थिः_unref(*parent);

	प्रत्यागम CSS_OK;
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
css_error sibling_ग्रन्थिः(शून्य *pw, शून्य *ग्रन्थिः, शून्य **sibling)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_ग्रन्थिः *पूर्व;
	dom_exception err;

	*sibling = रिक्त;

	/* Find sibling element */
	err = dom_ग्रन्थिः_get_previous_sibling(n, &n);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यावत् (n != रिक्त) {
		dom_ग्रन्थिः_type type;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(n, &type);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		यदि (type == DOM_ELEMENT_ग्रन्थिः)
			भङ्ग;

		err = dom_ग्रन्थिः_get_previous_sibling(n, &पूर्व);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_OK;
		}

		dom_ग्रन्थिः_unref(n);
		n = पूर्व;
	}

	यदि (n != रिक्त) {
		/** \todo Sort out reference counting */
		dom_ग्रन्थिः_unref(n);

		*sibling = n;
	}

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_name(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, bool *मेलनम्)
{
	nscss_select_ctx *ctx = pw;
	dom_ग्रन्थिः *n = ग्रन्थिः;

	यदि (lwc_string_isequal(qname->name, ctx->universal, मेलनम्) ==
			lwc_error_ok && *मेलनम् == false) {
		dom_string *name;
		dom_exception err;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_name(n, &name);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम CSS_OK;

		/* Element names are case insensitive in HTML */
		*मेलनम् = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_class(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *name, bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_exception err;

	/** \todo: Ensure that libdom performs case-insensitive
	 * matching in quirks mode */
	err = dom_element_has_class(n, name, मेलनम्);

	प्रतिज्ञा(err == DOM_NO_ERR);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_id(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *name, bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *attr;
	dom_exception err;

	*मेलनम् = false;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_OK;

	यदि (attr != रिक्त) {
		*मेलनम् = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_exception err;

	err = dom_string_create_interned(
			(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
			lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_has_attribute(n, name, मेलनम्);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(name);
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_equal(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_दीर्घता(मानम्);

	यदि (vlen == 0) {
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	*मेलनम् = dom_string_caseless_lwc_isequal(atr_val, मानम्);

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_dashmatch(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_दीर्घता(मानम्);

	यदि (vlen == 0) {
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*मेलनम् = dom_string_caseless_lwc_isequal(atr_val, मानम्);

	/* check for dashmatch */
	यदि (*मेलनम् == false) {
		ध्रुव वर्ण *vdata = lwc_string_दत्त(मानम्);
		ध्रुव वर्ण *दत्त = (ध्रुव वर्ण *) dom_string_दत्त(atr_val);
		size_t len = dom_string_byte_दीर्घता(atr_val);

		यदि (len > vlen && दत्त[vlen] == '-' &&
		    strncasecmp(दत्त, vdata, vlen) == 0) {
				*मेलनम् = true;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_includes(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_दीर्घता(मानम्);
	ध्रुव वर्ण *p;
	ध्रुव वर्ण *start;
	ध्रुव वर्ण *अन्तः;

	*मेलनम् = false;

	यदि (vlen == 0) {
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (ध्रुव वर्ण *) dom_string_दत्त(atr_val);
	अन्तः = start + dom_string_byte_दीर्घता(atr_val);

	प्रति (p = start; p <= अन्तः; p++) {
		यदि (*p == ' ' || *p == '\0') {
			यदि ((size_t) (p - start) == vlen &&
			    strncasecmp(start,
					lwc_string_दत्त(मानम्),
					vlen) == 0) {
				*मेलनम् = true;
				भङ्ग;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_उपसर्गः(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_दीर्घता(मानम्);

	यदि (vlen == 0) {
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*मेलनम् = dom_string_caseless_lwc_isequal(atr_val, मानम्);

	/* check for prefix match */
	यदि (*मेलनम् == false) {
		ध्रुव वर्ण *दत्त = (ध्रुव वर्ण *) dom_string_दत्त(atr_val);
		size_t len = dom_string_byte_दीर्घता(atr_val);

		यदि ((len >= vlen) &&
		    (strncasecmp(दत्त, lwc_string_दत्त(मानम्), vlen) == 0)) {
			*मेलनम् = true;
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_suffix(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_दीर्घता(मानम्);

	यदि (vlen == 0) {
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*मेलनम् = dom_string_caseless_lwc_isequal(atr_val, मानम्);

	/* check for prefix match */
	यदि (*मेलनम् == false) {
		ध्रुव वर्ण *दत्त = (ध्रुव वर्ण *) dom_string_दत्त(atr_val);
		size_t len = dom_string_byte_दीर्घता(atr_val);

		ध्रुव वर्ण *start = (वर्ण *) दत्त + len - vlen;

		यदि ((len >= vlen) &&
		    (strncasecmp(start, lwc_string_दत्त(मानम्), vlen) == 0)) {
			*मेलनम् = true;
		}


	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_has_attribute_substring(शून्य *pw, शून्य *ग्रन्थिः,
		ध्रुव css_qname *qname, lwc_string *मानम्,
		bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	size_t vlen = lwc_string_दीर्घता(मानम्);

	यदि (vlen == 0) {
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	err = dom_string_create_interned(
		(ध्रुव uint8_t *) lwc_string_दत्त(qname->name),
		lwc_string_दीर्घता(qname->name), &name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	यदि ((err != DOM_NO_ERR) || (atr_val == रिक्त)) {
		dom_string_unref(name);
		*मेलनम् = false;
		प्रत्यागम CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*मेलनम् = dom_string_caseless_lwc_isequal(atr_val, मानम्);

	/* check for prefix match */
	यदि (*मेलनम् == false) {
		ध्रुव वर्ण *vdata = lwc_string_दत्त(मानम्);
		ध्रुव वर्ण *start = (ध्रुव वर्ण *) dom_string_दत्त(atr_val);
		size_t len = dom_string_byte_दीर्घता(atr_val);
		ध्रुव वर्ण *चरमः_start = start + len - vlen;

		यदि (len >= vlen) {
			यावत् (start <= चरमः_start) {
				यदि (strncasecmp(start, vdata,
						vlen) == 0) {
					*मेलनम् = true;
					भङ्ग;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_मूलम्(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_ग्रन्थिः *parent;
	dom_ग्रन्थिः_type type;
	dom_exception err;

	err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(n, &parent);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (parent != रिक्त) {
		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(parent, &type);

		dom_ग्रन्थिः_unref(parent);

		यदि (err != DOM_NO_ERR)
			प्रत्यागम CSS_NOMEM;

		यदि (type != DOM_DOCUMENT_ग्रन्थिः) {
			*मेलनम् = false;
			प्रत्यागम CSS_OK;
		}
	}

	*मेलनम् = true;

	प्रत्यागम CSS_OK;
}

स्थिर पूर्णाङ्क
ग्रन्थिः_गणनम्_siblings_check(dom_ग्रन्थिः *ग्रन्थिः,
			  bool check_name,
			  dom_string *name)
{
	dom_ग्रन्थिः_type type;
	पूर्णाङ्क ret = 0;
	dom_exception exc;

	यदि (ग्रन्थिः == रिक्त)
		प्रत्यागम 0;

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &type);
	यदि ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_ग्रन्थिः)) {
		प्रत्यागम 0;
	}

	यदि (check_name) {
		dom_string *ग्रन्थिः_name = रिक्त;
		exc = dom_ग्रन्थिः_get_ग्रन्थिः_name(ग्रन्थिः, &ग्रन्थिः_name);

		यदि ((exc == DOM_NO_ERR) && (ग्रन्थिः_name != रिक्त)) {

			यदि (dom_string_caseless_isequal(name,
							ग्रन्थिः_name)) {
				ret = 1;
			}
			dom_string_unref(ग्रन्थिः_name);
		}
	} अन्यथा {
		ret = 1;
	}

	प्रत्यागम ret;
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
css_error ग्रन्थिः_गणनम्_siblings(शून्य *pw, शून्य *n, bool same_name,
		bool after, int32_t *गणनम्)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *ग्रन्थिः_name = रिक्त;

	यदि (same_name) {
		dom_ग्रन्थिः *ग्रन्थिः = n;
		exc = dom_ग्रन्थिः_get_ग्रन्थिः_name(ग्रन्थिः, &ग्रन्थिः_name);
		यदि ((exc != DOM_NO_ERR) || (ग्रन्थिः_name == रिक्त)) {
			प्रत्यागम CSS_NOMEM;
		}
	}

	यदि (after) {
		dom_ग्रन्थिः *ग्रन्थिः = dom_ग्रन्थिः_ref(n);
		dom_ग्रन्थिः *अग्र;

		कुरु {
			exc = dom_ग्रन्थिः_get_अग्र_sibling(ग्रन्थिः, &अग्र);
			यदि ((exc != DOM_NO_ERR))
				भङ्ग;

			dom_ग्रन्थिः_unref(ग्रन्थिः);
			ग्रन्थिः = अग्र;

			cnt += ग्रन्थिः_गणनम्_siblings_check(ग्रन्थिः, same_name, ग्रन्थिः_name);
		} यावत् (ग्रन्थिः != रिक्त);
	} अन्यथा {
		dom_ग्रन्थिः *ग्रन्थिः = dom_ग्रन्थिः_ref(n);
		dom_ग्रन्थिः *अग्र;

		कुरु {
			exc = dom_ग्रन्थिः_get_previous_sibling(ग्रन्थिः, &अग्र);
			यदि ((exc != DOM_NO_ERR))
				भङ्ग;

			dom_ग्रन्थिः_unref(ग्रन्थिः);
			ग्रन्थिः = अग्र;

			cnt += ग्रन्थिः_गणनम्_siblings_check(ग्रन्थिः, same_name, ग्रन्थिः_name);

		} यावत् (ग्रन्थिः != रिक्त);
	}

	यदि (ग्रन्थिः_name != रिक्त) {
		dom_string_unref(ग्रन्थिः_name);
	}

	*गणनम् = cnt;
	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_रिक्तानि(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	dom_ग्रन्थिः *n = ग्रन्थिः, *अग्र;
	dom_exception err;

	*मेलनम् = true;

	err = dom_ग्रन्थिः_get_first_child(n, &n);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_BADPARM;
	}

	यावत् (n != रिक्त) {
		dom_ग्रन्थिः_type ntype;
		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(n, &ntype);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_BADPARM;
		}

		यदि (ntype == DOM_ELEMENT_ग्रन्थिः ||
		    ntype == DOM_TEXT_ग्रन्थिः) {
			*मेलनम् = false;
			dom_ग्रन्थिः_unref(n);
			भङ्ग;
		}

		err = dom_ग्रन्थिः_get_अग्र_sibling(n, &अग्र);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(n);
			प्रत्यागम CSS_BADPARM;
		}
		dom_ग्रन्थिः_unref(n);
		n = अग्र;
	}

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_link(शून्य *pw, शून्य *n, bool *मेलनम्)
{
	dom_ग्रन्थिः *ग्रन्थिः = n;
	dom_exception exc;
	dom_string *ग्रन्थिः_name = रिक्त;

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_name(ग्रन्थिः, &ग्रन्थिः_name);
	यदि ((exc != DOM_NO_ERR) || (ग्रन्थिः_name == रिक्त)) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (dom_string_caseless_lwc_isequal(ग्रन्थिः_name, corestring_lwc_a)) {
		bool has_href;
		exc = dom_element_has_attribute(ग्रन्थिः, corestring_dom_href,
				&has_href);
		यदि ((exc == DOM_NO_ERR) && (has_href)) {
			*मेलनम् = true;
		} अन्यथा {
			*मेलनम् = false;
		}
	} अन्यथा {
		*मेलनम् = false;
	}
	dom_string_unref(ग्रन्थिः_name);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_visited(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	nscss_select_ctx *ctx = pw;
	nsurl *url;
	nserror error;
	ध्रुव संरचना url_दत्त *दत्त;

	dom_exception exc;
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_string *s = रिक्त;

	*मेलनम् = false;

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_name(n, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		प्रत्यागम CSS_NOMEM;
	}

	यदि (!dom_string_caseless_lwc_isequal(s, corestring_lwc_a)) {
		/* Can't be visited; not ancher element */
		dom_string_unref(s);
		प्रत्यागम CSS_OK;
	}

	/* Finished with node name string */
	dom_string_unref(s);
	s = रिक्त;

	exc = dom_element_get_attribute(n, corestring_dom_href, &s);
	यदि ((exc != DOM_NO_ERR) || (s == रिक्त)) {
		/* Can't be visited; not got a URL */
		प्रत्यागम CSS_OK;
	}

	/* Make href absolute */
	/* TODO: this duplicates what we do for box->href
	 *       should we put the absolute URL on the dom node? */
	error = nsurl_join(ctx->आधारः_url, dom_string_दत्त(s), &url);

	/* Finished with href string */
	dom_string_unref(s);

	यदि (error != NSERROR_OK) {
		/* Couldn't make nsurl object */
		प्रत्यागम CSS_NOMEM;
	}

	दत्त = urldb_get_url_दत्त(url);

	/* Visited if in the db and has
	 * non-zero visit count */
	यदि (दत्त != रिक्त && दत्त->visits > 0)
		*मेलनम् = true;

	nsurl_unref(url);

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_hover(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support hovering */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_सक्रिय(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support active nodes */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_focus(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support focussed nodes */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_enabled(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support enabled nodes */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_disabled(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support disabled nodes */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_checked(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support checked nodes */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_target(शून्य *pw, शून्य *ग्रन्थिः, bool *मेलनम्)
{
	/** \todo Support target */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ग्रन्थिः_is_lang(शून्य *pw, शून्य *ग्रन्थिः,
		lwc_string *lang, bool *मेलनम्)
{
	/** \todo Support languages */

	*मेलनम् = false;

	प्रत्यागम CSS_OK;
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
css_error ua_default_for_property(शून्य *pw, uint32_t property, css_hint *hint)
{
	यदि (property == CSS_PROP_COLOR) {
		hint->दत्त.color = 0xff000000;
		hint->स्थितिः = CSS_COLOR_COLOR;
	} अन्यथा यदि (property == CSS_PROP_FONT_FAMILY) {
		hint->दत्त.वाक्यानि = रिक्त;
		विकल्प (nsoption_int(font_default)) {
		पक्ष PLOT_FONT_FAMILY_SANS_SERIF:
			hint->स्थितिः = CSS_FONT_FAMILY_SANS_SERIF;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_SERIF:
			hint->स्थितिः = CSS_FONT_FAMILY_SERIF;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_MONOSPACE:
			hint->स्थितिः = CSS_FONT_FAMILY_MONOSPACE;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_CURSIVE:
			hint->स्थितिः = CSS_FONT_FAMILY_CURSIVE;
			भङ्ग;
		पक्ष PLOT_FONT_FAMILY_FANTASY:
			hint->स्थितिः = CSS_FONT_FAMILY_FANTASY;
			भङ्ग;
		}
	} अन्यथा यदि (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->दत्त.वाक्यानि = रिक्त;
		hint->स्थितिः = CSS_QUOTES_NONE;
	} अन्यथा यदि (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->दत्त.वाक्यानि = रिक्त;
		hint->स्थितिः = 0;
	} अन्यथा {
		प्रत्यागम CSS_INVALID;
	}

	प्रत्यागम CSS_OK;
}

css_error समुच्चयः_libcss_ग्रन्थिः_दत्त(शून्य *pw, शून्य *ग्रन्थिः, शून्य *libcss_ग्रन्थिः_दत्त)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_exception err;
	शून्य *old_ग्रन्थिः_दत्त;

	/* Set this node's node data */
	err = dom_ग्रन्थिः_समुच्चयः_user_दत्त(n,
			corestring_dom_ns_कुञ्चिका_libcss_ग्रन्थिः_दत्त,
			libcss_ग्रन्थिः_दत्त, nscss_dom_user_दत्त_handler,
			(शून्य *) &old_ग्रन्थिः_दत्त);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	प्रतिज्ञा(old_ग्रन्थिः_दत्त == रिक्त);

	प्रत्यागम CSS_OK;
}

css_error get_libcss_ग्रन्थिः_दत्त(शून्य *pw, शून्य *ग्रन्थिः, शून्य **libcss_ग्रन्थिः_दत्त)
{
	dom_ग्रन्थिः *n = ग्रन्थिः;
	dom_exception err;

	/* Get this node's node data */
	err = dom_ग्रन्थिः_get_user_दत्त(n,
			corestring_dom_ns_कुञ्चिका_libcss_ग्रन्थिः_दत्त,
			libcss_ग्रन्थिः_दत्त);
	यदि (err != DOM_NO_ERR) {
		प्रत्यागम CSS_NOMEM;
	}

	प्रत्यागम CSS_OK;
}
