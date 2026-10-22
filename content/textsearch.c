/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 * Free text search
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"

#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"

/**
 * search match
 */
ರಚನೆ list_ನಮೂದು {
	/**
	 * previous match
	 */
	ರಚನೆ list_ನಮೂದು *prev;

	/**
	 * next match
	 */
	ರಚನೆ list_ನಮೂದು *ಮುಂದಿನ;

	/**
	 * start position of match
	 */
	ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx;

	/**
	 * end of match
	 */
	ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx;

	/**
	 * content opaque start pointer
	 */
	ರಚನೆ box *ಆರಂಭ_box;

	/**
	 * content opaque end pointer
	 */
	ರಚನೆ box *ಕೊನೆ_box;

	/**
	 * content specific selection object
	 */
	ರಚನೆ selection *sel;
};

/**
 * The context for a free text search
 */
ರಚನೆ textsearch_context {

	/**
	 * content search was performed upon
	 */
	ರಚನೆ content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	ಶೂನ್ಯ *gui_p;

	/**
	 * List of matches
	 */
	ರಚನೆ list_ನಮೂದು *ಸಿಕ್ಕಿತು;

	/**
	 * current selected match
	 */
	ರಚನೆ list_ನಮೂದು *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	ಅಕ್ಷರ *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
textsearch_ಪ್ರಸಾರ(ರಚನೆ textsearch_context *textsearch,
		     ಪೂರ್ಣಾಂಕ type,
		     bool state,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ಸಂದೇಶ_ದತ್ತಾಂಶ.textsearch.type = type;
	ಸಂದೇಶ_ದತ್ತಾಂಶ.textsearch.ctx = textsearch->gui_p;
	ಸಂದೇಶ_ದತ್ತಾಂಶ.textsearch.state = state;
	ಸಂದೇಶ_ದತ್ತಾಂಶ.textsearch.string = string;
	content_ಪ್ರಸಾರ(textsearch->c, CONTENT_ಸಂದೇಶ_TEXTSEARCH, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ಸ್ಥಿರ ಶೂನ್ಯ free_matches(ರಚನೆ textsearch_context *textsearch)
{
	ರಚನೆ list_ನಮೂದು *cur;
	ರಚನೆ list_ನಮೂದು *nxt;

	cur = textsearch->ಸಿಕ್ಕಿತು->ಮುಂದಿನ;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->ಸಿಕ್ಕಿತು->prev = ಖಾಲಿ;
	textsearch->ಸಿಕ್ಕಿತು->ಮುಂದಿನ = ಖಾಲಿ;

	ಗಾಗಿ (; cur; cur = nxt) {
		nxt = cur->ಮುಂದಿನ;
		ಆದರೆ (cur->sel) {
			selection_destroy(cur->sel);
		}
		ಬಿಡುಗಡೆ(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಹುಡುಕು_show_all(bool all, ರಚನೆ textsearch_context *context)
{
	ರಚನೆ list_ನಮೂದು *a;

	ಗಾಗಿ (a = context->ಸಿಕ್ಕಿತು->ಮುಂದಿನ; a; a = a->ಮುಂದಿನ) {
		bool ಕೂಡಿಸು = true;
		ಆದರೆ (!all && a != context->current) {
			ಕೂಡಿಸು = false;
			ಆದರೆ (a->sel) {
				selection_destroy(a->sel);
				a->sel = ಖಾಲಿ;
			}
		}

		ಆದರೆ (ಕೂಡಿಸು && !a->sel) {

			a->sel = selection_create(context->c);
			ಆದರೆ (a->sel != ಖಾಲಿ) {
				selection_init(a->sel);
				selection_ಗಣ_position(a->sel,
						       a->ಆರಂಭ_idx,
						       a->ಕೊನೆ_idx);
			}
		}
	}
}


/**
 * Search for a string in a content.
 *
 * \param context The search context.
 * \param string the string to search for
 * \param string_len length of search string
 * \param flags flags to control the search.
 */
ಸ್ಥಿರ nserror
ಹುಡುಕು_ಪಠ್ಯ(ರಚನೆ textsearch_context *context,
	    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string,
	    ಪೂರ್ಣಾಂಕ string_len,
	    ಹುಡುಕು_flags_t flags)
{
	ರಚನೆ rect bounds;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & ಹುಡುಕು_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & ಹುಡುಕು_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & ಹುಡುಕು_FLAG_SHOWALL) != 0) ? true : false;

	ಆದರೆ (context->c == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* check if we need to start a new search or continue an old one */
	ಆದರೆ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ಆದರೆ (context->string != ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(context->string);
		}

		context->current = ಖಾಲಿ;
		free_matches(context);

		context->string = ಸ್ಮರಣೆ_ಕೊಡು(string_len + 1);
		ಆದರೆ (context->string != ಖಾಲಿ) {
			ಸ್ಮರಣೆ_ನಕಲು(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_ಪ್ರಸಾರ(context, CONTENT_TEXTSEARCH_ಕಂಡುಹಿಡಿ, true, ಖಾಲಿ);


		/* call content find handler */
		res = context->c->handler->textsearch_ಕಂಡುಹಿಡಿ(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_ಪ್ರಸಾರ(context, CONTENT_TEXTSEARCH_ಕಂಡುಹಿಡಿ, false, ಖಾಲಿ);

		ಆದರೆ (res != NSERROR_OK) {
			free_matches(context);
			ಹಿಂತಿರುಗಿ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->ಸಿಕ್ಕಿತು->ಮುಂದಿನ;
		context->newsearch = false;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (context->current != ಖಾಲಿ) {
		/* continued search in the direction specified */
		ಆದರೆ (forwards) {
			ಆದರೆ (context->current->ಮುಂದಿನ)
				context->current = context->current->ಮುಂದಿನ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_ಪ್ರಸಾರ(context,
			     CONTENT_TEXTSEARCH_ಹೊಂದಿಕೆ,
			     (context->current != ಖಾಲಿ),
			     ಖಾಲಿ);

	ಹುಡುಕು_show_all(showall, context);

	/* update back state */
	textsearch_ಪ್ರಸಾರ(context,
			     CONTENT_TEXTSEARCH_ಹಿಂಭಾಗ,
			     ((context->current != ಖಾಲಿ) &&
			      (context->current->prev != ಖಾಲಿ)),
			     ಖಾಲಿ);

	/* update forward state */
	textsearch_ಪ್ರಸಾರ(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ಖಾಲಿ) &&
			      (context->current->ಮುಂದಿನ != ಖಾಲಿ)),
			     ಖಾಲಿ);


	ಆದರೆ (context->current == ಖಾಲಿ) {
		/* no current match */
		ಹಿಂತಿರುಗಿ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->ಆರಂಭ_idx,
					context->current->ಕೊನೆ_idx,
					context->current->ಆರಂಭ_box,
					context->current->ಕೊನೆ_box,
					&bounds);
	ಆದರೆ (res == NSERROR_OK) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.area = true;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.x0 = bounds.x0;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.y0 = bounds.y0;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.x1 = bounds.x1;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.y1 = bounds.y1;
		content_ಪ್ರಸಾರ(context->c, CONTENT_ಸಂದೇಶ_SCROLL, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Begins/continues the search process
 *
 * \note that this may be called many times for a single search.
 *
 * \param context The search context in use.
 * \param flags   The flags forward/back etc
 * \param string  The string to match
 */
ಸ್ಥಿರ nserror
content_textsearch_ಹೆಜ್ಜೆ(ರಚನೆ textsearch_context *textsearch,
			ಹುಡುಕು_flags_t flags,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
	ಪೂರ್ಣಾಂಕ string_len;
	ಪೂರ್ಣಾಂಕ i = 0;
	nserror res = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(textsearch != ಖಾಲಿ);

	/* broadcast recent query string */
	textsearch_ಪ್ರಸಾರ(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string);
	ಗಾಗಿ (i = 0; i < string_len; i++) {
		ಆದರೆ (string[i] != '#' && string[i] != '*')
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (i < string_len) {
		res = ಹುಡುಕು_ಪಠ್ಯ(textsearch, string, string_len, flags);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

		free_matches(textsearch);

		/* update match state */
		textsearch_ಪ್ರಸಾರ(textsearch,
				     CONTENT_TEXTSEARCH_ಹೊಂದಿಕೆ,
				     true,
				     ಖಾಲಿ);

		/* update back state */
		textsearch_ಪ್ರಸಾರ(textsearch,
				     CONTENT_TEXTSEARCH_ಹಿಂಭಾಗ,
				     false,
				     ಖಾಲಿ);

		/* update forward state */
		textsearch_ಪ್ರಸಾರ(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ಖಾಲಿ);

		/* clear scroll */
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.area = false;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.x0 = 0;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.scroll.y0 = 0;
		content_ಪ್ರಸಾರ(textsearch->c,
				  CONTENT_ಸಂದೇಶ_SCROLL,
				  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ಸ್ಥಿರ nserror content_textsearch__clear(ರಚನೆ content *c)
{
	ಬಿಡುಗಡೆ(c->textsearch.string);
	c->textsearch.string = ಖಾಲಿ;

	ಆದರೆ (c->textsearch.context != ಖಾಲಿ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ಖಾಲಿ;
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ಸ್ಥಿರ nserror
content_textsearch_create(ರಚನೆ content *c,
			  ಶೂನ್ಯ *gui_ದತ್ತಾಂಶ,
			  ರಚನೆ textsearch_context **textsearch_ಹೊರಹೋಗು)
{
	ರಚನೆ textsearch_context *context;
	ರಚನೆ list_ನಮೂದು *ಹುಡುಕು_ತಲೆ;
	content_type type;

	ಆದರೆ ((c->handler->textsearch_ಕಂಡುಹಿಡಿ == ಖಾಲಿ) ||
	    (c->handler->textsearch_bounds == ಖಾಲಿ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ಹಿಂತಿರುಗಿ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ textsearch_context));
	ಆದರೆ (context == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹುಡುಕು_ತಲೆ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ list_ನಮೂದು));
	ಆದರೆ (ಹುಡುಕು_ತಲೆ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(context);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹುಡುಕು_ತಲೆ->ಆರಂಭ_idx = 0;
	ಹುಡುಕು_ತಲೆ->ಕೊನೆ_idx = 0;
	ಹುಡುಕು_ತಲೆ->ಆರಂಭ_box = ಖಾಲಿ;
	ಹುಡುಕು_ತಲೆ->ಕೊನೆ_box = ಖಾಲಿ;
	ಹುಡುಕು_ತಲೆ->sel = ಖಾಲಿ;
	ಹುಡುಕು_ತಲೆ->prev = ಖಾಲಿ;
	ಹುಡುಕು_ತಲೆ->ಮುಂದಿನ = ಖಾಲಿ;

	context->ಸಿಕ್ಕಿತು = ಹುಡುಕು_ತಲೆ;
	context->current = ಖಾಲಿ;
	context->string = ಖಾಲಿ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_ದತ್ತಾಂಶ;

	*textsearch_ಹೊರಹೋಗು = context;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
content_textsearch_ಕಂಡುಹಿಡಿ_ನಮೂನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string,
				ಪೂರ್ಣಾಂಕ s_len,
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ,
				ಪೂರ್ಣಾಂಕ p_len,
				bool case_sens,
				ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *m_len)
{
	ರಚನೆ { ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ss, *s, *p; bool ಮೊದಲ; } context[16];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep = ನಮೂನೆ + p_len;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *es = string  + s_len;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ನಮೂನೆ - 1;  /* a virtual '*' before the pattern */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ss = string;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = string;
	bool ಮೊದಲ = true;
	ಪೂರ್ಣಾಂಕ ತುದಿ = 0;

	ಆಗ (p < ep) {
		bool matches;
		ಆದರೆ (p < ನಮೂನೆ || *p == '*') {
			ಅಕ್ಷರ ch;

			/* skip any further asterisks; one is the same as many
			*/
			ಮಾಡು p++; ಆಗ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ಆದರೆ (p >= ep) ನಿಲ್ಲಿಸು;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ಆದರೆ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ಆದರೆ (!case_sens) ch = ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(ch);
				ಆಗ (s < es) {
					ಆದರೆ (case_sens) {
						ಆದರೆ (*s == ch) ನಿಲ್ಲಿಸು;
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(*s) == ch)
						ನಿಲ್ಲಿಸು;
					s++;
				}
			}

			ಆದರೆ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ಆದರೆ (ತುದಿ < (ಪೂರ್ಣಾಂಕ)NOF_ELEMENTS(context)) {
					context[ತುದಿ].ss = ss;
					context[ತುದಿ].s  = s + 1;
					context[ತುದಿ].p  = p - 1;
					/* ptr to last asterisk */
					context[ತುದಿ].ಮೊದಲ = ಮೊದಲ;
					ತುದಿ++;
				}

				ಆದರೆ (ಮೊದಲ) {
					ss = s;
					/* remember first non-'*' char */
					ಮೊದಲ = false;
				}

				matches = true;
			} ಇಲ್ಲದಿದ್ದರೆ {
				matches = false;
			}

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s < es) {
			ಅಕ್ಷರ ch = *p;
			ಆದರೆ (ch == '#')
				matches = true;
			ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (case_sens)
					matches = (*s == ch);
				ಇಲ್ಲದಿದ್ದರೆ
					matches = (ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(*s) == ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(ch));
			}
			ಆದರೆ (matches && ಮೊದಲ) {
				ss = s;  /* remember first non-'*' char */
				ಮೊದಲ = false;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			matches = false;
		}

		ಆದರೆ (matches) {
			p++; s++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ಆದರೆ (--ತುದಿ < 0)
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no match, give up */

			ss = context[ತುದಿ].ss;
			s  = context[ತುದಿ].s;
			p  = context[ತುದಿ].p;
			ಮೊದಲ = context[ತುದಿ].ಮೊದಲ;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ಹಿಂತಿರುಗಿ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_ಕೂಡಿಸು_ಹೊಂದಿಕೆ(ರಚನೆ textsearch_context *context,
			     ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			     ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
			     ರಚನೆ box *ಆರಂಭ_box,
			     ರಚನೆ box *ಕೊನೆ_box)
{
	ರಚನೆ list_ನಮೂದು *ನಮೂದು;

	/* found string in box => add to list */
	ನಮೂದು = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ನಮೂದು));
	ಆದರೆ (ನಮೂದು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ನಮೂದು->ಆರಂಭ_idx = ಆರಂಭ_idx;
	ನಮೂದು->ಕೊನೆ_idx = ಕೊನೆ_idx;
	ನಮೂದು->ಆರಂಭ_box = ಆರಂಭ_box;
	ನಮೂದು->ಕೊನೆ_box = ಕೊನೆ_box;
	ನಮೂದು->sel = ಖಾಲಿ;

	ನಮೂದು->ಮುಂದಿನ = ಖಾಲಿ;
	ನಮೂದು->prev = context->ಸಿಕ್ಕಿತು->prev;

	ಆದರೆ (context->ಸಿಕ್ಕಿತು->prev == ಖಾಲಿ) {
		context->ಸಿಕ್ಕಿತು->ಮುಂದಿನ = ನಮೂದು;
	} ಇಲ್ಲದಿದ್ದರೆ {
		context->ಸಿಕ್ಕಿತು->prev->ಮುಂದಿನ = ನಮೂದು;
	}

	context->ಸಿಕ್ಕಿತು->prev = ನಮೂದು;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ರಚನೆ textsearch_context *textsearch,
				 ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_ಸ್ಥಾನಾಂತರ,
				 ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_ಸ್ಥಾನಾಂತರ,
				 ಚಿಹ್ನೆರಹಿತ *ಆರಂಭ_idx,
				 ಚಿಹ್ನೆರಹಿತ *ಕೊನೆ_idx)
{
	ರಚನೆ list_ನಮೂದು *cur;

	ಗಾಗಿ (cur = textsearch->ಸಿಕ್ಕಿತು->ಮುಂದಿನ; cur != ಖಾಲಿ; cur = cur->ಮುಂದಿನ) {
		ಆದರೆ (cur->sel &&
		    selection_highlighted(cur->sel,
					  ಆರಂಭ_ಸ್ಥಾನಾಂತರ,
					  ಕೊನೆ_ಸ್ಥಾನಾಂತರ,
					  ಆರಂಭ_idx,
					  ಕೊನೆ_idx)) {
			ಹಿಂತಿರುಗಿ true;
		}
	}

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ರಚನೆ textsearch_context *textsearch)
{
	ಪ್ರತಿಜ್ಞಾ(textsearch != ಖಾಲಿ);

	ಆದರೆ (textsearch->string != ಖಾಲಿ) {
		/* broadcast recent query string */
		textsearch_ಪ್ರಸಾರ(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		ಬಿಡುಗಡೆ(textsearch->string);
	}

	/* update back state */
	textsearch_ಪ್ರಸಾರ(textsearch,
			     CONTENT_TEXTSEARCH_ಹಿಂಭಾಗ,
			     true,
			     ಖಾಲಿ);

	/* update forward state */
	textsearch_ಪ್ರಸಾರ(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ಖಾಲಿ);

	free_matches(textsearch);
	ಬಿಡುಗಡೆ(textsearch);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ರಚನೆ hlcache_handle *h,
		   ಶೂನ್ಯ *context,
		   ಹುಡುಕು_flags_t flags,
		   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	nserror res;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (string != ಖಾಲಿ &&
	    c->textsearch.string != ಖಾಲಿ &&
	    c->textsearch.context != ಖಾಲಿ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_ಹೆಜ್ಜೆ(c->textsearch.context, flags, string);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (string != ಖಾಲಿ) {
		/* New search */
		ಬಿಡುಗಡೆ(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ಆದರೆ (c->textsearch.string == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		ಆದರೆ (c->textsearch.context != ಖಾಲಿ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ಖಾಲಿ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}

		content_textsearch_ಹೆಜ್ಜೆ(c->textsearch.context, flags, string);

	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Clear search */
		content_textsearch__clear(c);

		ಬಿಡುಗಡೆ(c->textsearch.string);
		c->textsearch.string = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ರಚನೆ hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಹಿಂತಿರುಗಿ(content_textsearch__clear(c));
}
