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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"

/**
 * search match
 */
ਬਣਤਰ list_ਇੰਦਰਾਜ {
	/**
	 * previous match
	 */
	ਬਣਤਰ list_ਇੰਦਰਾਜ *prev;

	/**
	 * next match
	 */
	ਬਣਤਰ list_ਇੰਦਰਾਜ *ਅਗਲਾ;

	/**
	 * start position of match
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx;

	/**
	 * end of match
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx;

	/**
	 * content opaque start pointer
	 */
	ਬਣਤਰ box *ਸ਼ੁਰੂ_box;

	/**
	 * content opaque end pointer
	 */
	ਬਣਤਰ box *ਸਿਰਾ_box;

	/**
	 * content specific selection object
	 */
	ਬਣਤਰ selection *sel;
};

/**
 * The context for a free text search
 */
ਬਣਤਰ textsearch_context {

	/**
	 * content search was performed upon
	 */
	ਬਣਤਰ content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	ਖਾਲੀ *gui_p;

	/**
	 * List of matches
	 */
	ਬਣਤਰ list_ਇੰਦਰਾਜ *ਮਿਲਿਆ;

	/**
	 * current selected match
	 */
	ਬਣਤਰ list_ਇੰਦਰਾਜ *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	ਅੱਖਰ *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
textsearch_ਪ੍ਰਸਾਰਣ(ਬਣਤਰ textsearch_context *textsearch,
		     ਪੂਰਨ type,
		     bool state,
		     ਅਚਲ ਅੱਖਰ *string)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.textsearch.type = type;
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.textsearch.ctx = textsearch->gui_p;
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.textsearch.state = state;
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.textsearch.string = string;
	content_ਪ੍ਰਸਾਰਣ(textsearch->c, CONTENT_ਸੁਨੇਹਾ_TEXTSEARCH, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ਸਥਿਰ ਖਾਲੀ free_matches(ਬਣਤਰ textsearch_context *textsearch)
{
	ਬਣਤਰ list_ਇੰਦਰਾਜ *cur;
	ਬਣਤਰ list_ਇੰਦਰਾਜ *nxt;

	cur = textsearch->ਮਿਲਿਆ->ਅਗਲਾ;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->ਮਿਲਿਆ->prev = ਨਿਰਾ;
	textsearch->ਮਿਲਿਆ->ਅਗਲਾ = ਨਿਰਾ;

	ਲਈ (; cur; cur = nxt) {
		nxt = cur->ਅਗਲਾ;
		ਜੇ (cur->sel) {
			selection_destroy(cur->sel);
		}
		ਮੁਕਤ(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ਸਥਿਰ ਖਾਲੀ ਖੋਜ_show_all(bool all, ਬਣਤਰ textsearch_context *context)
{
	ਬਣਤਰ list_ਇੰਦਰਾਜ *a;

	ਲਈ (a = context->ਮਿਲਿਆ->ਅਗਲਾ; a; a = a->ਅਗਲਾ) {
		bool ਜੋੜੋ = true;
		ਜੇ (!all && a != context->current) {
			ਜੋੜੋ = false;
			ਜੇ (a->sel) {
				selection_destroy(a->sel);
				a->sel = ਨਿਰਾ;
			}
		}

		ਜੇ (ਜੋੜੋ && !a->sel) {

			a->sel = selection_create(context->c);
			ਜੇ (a->sel != ਨਿਰਾ) {
				selection_init(a->sel);
				selection_ਸੈੱਟ_position(a->sel,
						       a->ਸ਼ੁਰੂ_idx,
						       a->ਸਿਰਾ_idx);
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
ਸਥਿਰ nserror
ਖੋਜ_ਪਾਠ(ਬਣਤਰ textsearch_context *context,
	    ਅਚਲ ਅੱਖਰ *string,
	    ਪੂਰਨ string_len,
	    ਖੋਜ_flags_t flags)
{
	ਬਣਤਰ rect bounds;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & ਖੋਜ_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & ਖੋਜ_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & ਖੋਜ_FLAG_SHOWALL) != 0) ? true : false;

	ਜੇ (context->c == ਨਿਰਾ) {
		ਵਾਪਸ res;
	}

	/* check if we need to start a new search or continue an old one */
	ਜੇ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ਜੇ (context->string != ਨਿਰਾ) {
			ਮੁਕਤ(context->string);
		}

		context->current = ਨਿਰਾ;
		free_matches(context);

		context->string = ਸਮ੍ਰਿਤੀ_ਦਿਓ(string_len + 1);
		ਜੇ (context->string != ਨਿਰਾ) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_ਪ੍ਰਸਾਰਣ(context, CONTENT_TEXTSEARCH_ਲੱਭੋ, true, ਨਿਰਾ);


		/* call content find handler */
		res = context->c->handler->textsearch_ਲੱਭੋ(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_ਪ੍ਰਸਾਰਣ(context, CONTENT_TEXTSEARCH_ਲੱਭੋ, false, ਨਿਰਾ);

		ਜੇ (res != NSERROR_OK) {
			free_matches(context);
			ਵਾਪਸ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->ਮਿਲਿਆ->ਅਗਲਾ;
		context->newsearch = false;

	} ਨਹੀਂਤਾਂ ਜੇ (context->current != ਨਿਰਾ) {
		/* continued search in the direction specified */
		ਜੇ (forwards) {
			ਜੇ (context->current->ਅਗਲਾ)
				context->current = context->current->ਅਗਲਾ;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_ਪ੍ਰਸਾਰਣ(context,
			     CONTENT_TEXTSEARCH_ਮੇਲ,
			     (context->current != ਨਿਰਾ),
			     ਨਿਰਾ);

	ਖੋਜ_show_all(showall, context);

	/* update back state */
	textsearch_ਪ੍ਰਸਾਰਣ(context,
			     CONTENT_TEXTSEARCH_ਪਿੱਠ,
			     ((context->current != ਨਿਰਾ) &&
			      (context->current->prev != ਨਿਰਾ)),
			     ਨਿਰਾ);

	/* update forward state */
	textsearch_ਪ੍ਰਸਾਰਣ(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ਨਿਰਾ) &&
			      (context->current->ਅਗਲਾ != ਨਿਰਾ)),
			     ਨਿਰਾ);


	ਜੇ (context->current == ਨਿਰਾ) {
		/* no current match */
		ਵਾਪਸ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->ਸ਼ੁਰੂ_idx,
					context->current->ਸਿਰਾ_idx,
					context->current->ਸ਼ੁਰੂ_box,
					context->current->ਸਿਰਾ_box,
					&bounds);
	ਜੇ (res == NSERROR_OK) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.area = true;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.x0 = bounds.x0;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.y0 = bounds.y0;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.x1 = bounds.x1;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.y1 = bounds.y1;
		content_ਪ੍ਰਸਾਰਣ(context->c, CONTENT_ਸੁਨੇਹਾ_SCROLL, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
content_textsearch_ਕਦਮ(ਬਣਤਰ textsearch_context *textsearch,
			ਖੋਜ_flags_t flags,
			ਅਚਲ ਅੱਖਰ *string)
{
	ਪੂਰਨ string_len;
	ਪੂਰਨ i = 0;
	nserror res = NSERROR_OK;

	ਪ੍ਰਤਿਜ੍ਞਾ(textsearch != ਨਿਰਾ);

	/* broadcast recent query string */
	textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = ਲੜੀ_ਲੰਬਾਈ(string);
	ਲਈ (i = 0; i < string_len; i++) {
		ਜੇ (string[i] != '#' && string[i] != '*')
			ਰੋਕੋ;
	}

	ਜੇ (i < string_len) {
		res = ਖੋਜ_ਪਾਠ(textsearch, string, string_len, flags);
	} ਨਹੀਂਤਾਂ {
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

		free_matches(textsearch);

		/* update match state */
		textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
				     CONTENT_TEXTSEARCH_ਮੇਲ,
				     true,
				     ਨਿਰਾ);

		/* update back state */
		textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
				     CONTENT_TEXTSEARCH_ਪਿੱਠ,
				     false,
				     ਨਿਰਾ);

		/* update forward state */
		textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ਨਿਰਾ);

		/* clear scroll */
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.area = false;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.x0 = 0;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.scroll.y0 = 0;
		content_ਪ੍ਰਸਾਰਣ(textsearch->c,
				  CONTENT_ਸੁਨੇਹਾ_SCROLL,
				  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}

	ਵਾਪਸ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ਸਥਿਰ nserror content_textsearch__clear(ਬਣਤਰ content *c)
{
	ਮੁਕਤ(c->textsearch.string);
	c->textsearch.string = ਨਿਰਾ;

	ਜੇ (c->textsearch.context != ਨਿਰਾ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ਨਿਰਾ;
	}
	ਵਾਪਸ NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ਸਥਿਰ nserror
content_textsearch_create(ਬਣਤਰ content *c,
			  ਖਾਲੀ *gui_ਅੰਕੜੇ,
			  ਬਣਤਰ textsearch_context **textsearch_ਬਾਹਰਲਾ)
{
	ਬਣਤਰ textsearch_context *context;
	ਬਣਤਰ list_ਇੰਦਰਾਜ *ਖੋਜ_ਸਿਰ;
	content_type type;

	ਜੇ ((c->handler->textsearch_ਲੱਭੋ == ਨਿਰਾ) ||
	    (c->handler->textsearch_bounds == ਨਿਰਾ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ਵਾਪਸ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ textsearch_context));
	ਜੇ (context == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਖੋਜ_ਸਿਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ list_ਇੰਦਰਾਜ));
	ਜੇ (ਖੋਜ_ਸਿਰ == ਨਿਰਾ) {
		ਮੁਕਤ(context);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਖੋਜ_ਸਿਰ->ਸ਼ੁਰੂ_idx = 0;
	ਖੋਜ_ਸਿਰ->ਸਿਰਾ_idx = 0;
	ਖੋਜ_ਸਿਰ->ਸ਼ੁਰੂ_box = ਨਿਰਾ;
	ਖੋਜ_ਸਿਰ->ਸਿਰਾ_box = ਨਿਰਾ;
	ਖੋਜ_ਸਿਰ->sel = ਨਿਰਾ;
	ਖੋਜ_ਸਿਰ->prev = ਨਿਰਾ;
	ਖੋਜ_ਸਿਰ->ਅਗਲਾ = ਨਿਰਾ;

	context->ਮਿਲਿਆ = ਖੋਜ_ਸਿਰ;
	context->current = ਨਿਰਾ;
	context->string = ਨਿਰਾ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_ਅੰਕੜੇ;

	*textsearch_ਬਾਹਰਲਾ = context;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ਅਚਲ ਅੱਖਰ *
content_textsearch_ਲੱਭੋ_ਨਮੂਨਾ(ਅਚਲ ਅੱਖਰ *string,
				ਪੂਰਨ s_len,
				ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ,
				ਪੂਰਨ p_len,
				bool case_sens,
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *m_len)
{
	ਬਣਤਰ { ਅਚਲ ਅੱਖਰ *ss, *s, *p; bool ਪਹਿਲਾ; } context[16];
	ਅਚਲ ਅੱਖਰ *ep = ਨਮੂਨਾ + p_len;
	ਅਚਲ ਅੱਖਰ *es = string  + s_len;
	ਅਚਲ ਅੱਖਰ *p = ਨਮੂਨਾ - 1;  /* a virtual '*' before the pattern */
	ਅਚਲ ਅੱਖਰ *ss = string;
	ਅਚਲ ਅੱਖਰ *s = string;
	bool ਪਹਿਲਾ = true;
	ਪੂਰਨ ਸਿਖਰ = 0;

	ਜਦੋਂ (p < ep) {
		bool matches;
		ਜੇ (p < ਨਮੂਨਾ || *p == '*') {
			ਅੱਖਰ ch;

			/* skip any further asterisks; one is the same as many
			*/
			ਕਰੋ p++; ਜਦੋਂ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ਜੇ (p >= ep) ਰੋਕੋ;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ਜੇ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ਜੇ (!case_sens) ch = ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(ch);
				ਜਦੋਂ (s < es) {
					ਜੇ (case_sens) {
						ਜੇ (*s == ch) ਰੋਕੋ;
					} ਨਹੀਂਤਾਂ ਜੇ (ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(*s) == ch)
						ਰੋਕੋ;
					s++;
				}
			}

			ਜੇ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ਜੇ (ਸਿਖਰ < (ਪੂਰਨ)NOF_ELEMENTS(context)) {
					context[ਸਿਖਰ].ss = ss;
					context[ਸਿਖਰ].s  = s + 1;
					context[ਸਿਖਰ].p  = p - 1;
					/* ptr to last asterisk */
					context[ਸਿਖਰ].ਪਹਿਲਾ = ਪਹਿਲਾ;
					ਸਿਖਰ++;
				}

				ਜੇ (ਪਹਿਲਾ) {
					ss = s;
					/* remember first non-'*' char */
					ਪਹਿਲਾ = false;
				}

				matches = true;
			} ਨਹੀਂਤਾਂ {
				matches = false;
			}

		} ਨਹੀਂਤਾਂ ਜੇ (s < es) {
			ਅੱਖਰ ch = *p;
			ਜੇ (ch == '#')
				matches = true;
			ਨਹੀਂਤਾਂ {
				ਜੇ (case_sens)
					matches = (*s == ch);
				ਨਹੀਂਤਾਂ
					matches = (ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(*s) == ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(ch));
			}
			ਜੇ (matches && ਪਹਿਲਾ) {
				ss = s;  /* remember first non-'*' char */
				ਪਹਿਲਾ = false;
			}
		} ਨਹੀਂਤਾਂ {
			matches = false;
		}

		ਜੇ (matches) {
			p++; s++;
		} ਨਹੀਂਤਾਂ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ਜੇ (--ਸਿਖਰ < 0)
				ਵਾਪਸ ਨਿਰਾ;  /* no match, give up */

			ss = context[ਸਿਖਰ].ss;
			s  = context[ਸਿਖਰ].s;
			p  = context[ਸਿਖਰ].p;
			ਪਹਿਲਾ = context[ਸਿਖਰ].ਪਹਿਲਾ;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ਵਾਪਸ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_ਜੋੜੋ_ਮੇਲ(ਬਣਤਰ textsearch_context *context,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
			     ਬਣਤਰ box *ਸ਼ੁਰੂ_box,
			     ਬਣਤਰ box *ਸਿਰਾ_box)
{
	ਬਣਤਰ list_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;

	/* found string in box => add to list */
	ਇੰਦਰਾਜ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ਇੰਦਰਾਜ));
	ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਇੰਦਰਾਜ->ਸ਼ੁਰੂ_idx = ਸ਼ੁਰੂ_idx;
	ਇੰਦਰਾਜ->ਸਿਰਾ_idx = ਸਿਰਾ_idx;
	ਇੰਦਰਾਜ->ਸ਼ੁਰੂ_box = ਸ਼ੁਰੂ_box;
	ਇੰਦਰਾਜ->ਸਿਰਾ_box = ਸਿਰਾ_box;
	ਇੰਦਰਾਜ->sel = ਨਿਰਾ;

	ਇੰਦਰਾਜ->ਅਗਲਾ = ਨਿਰਾ;
	ਇੰਦਰਾਜ->prev = context->ਮਿਲਿਆ->prev;

	ਜੇ (context->ਮਿਲਿਆ->prev == ਨਿਰਾ) {
		context->ਮਿਲਿਆ->ਅਗਲਾ = ਇੰਦਰਾਜ;
	} ਨਹੀਂਤਾਂ {
		context->ਮਿਲਿਆ->prev->ਅਗਲਾ = ਇੰਦਰਾਜ;
	}

	context->ਮਿਲਿਆ->prev = ਇੰਦਰਾਜ;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ਬਣਤਰ textsearch_context *textsearch,
				 ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_ਵਿੱਥ,
				 ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_ਵਿੱਥ,
				 ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸ਼ੁਰੂ_idx,
				 ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸਿਰਾ_idx)
{
	ਬਣਤਰ list_ਇੰਦਰਾਜ *cur;

	ਲਈ (cur = textsearch->ਮਿਲਿਆ->ਅਗਲਾ; cur != ਨਿਰਾ; cur = cur->ਅਗਲਾ) {
		ਜੇ (cur->sel &&
		    selection_highlighted(cur->sel,
					  ਸ਼ੁਰੂ_ਵਿੱਥ,
					  ਸਿਰਾ_ਵਿੱਥ,
					  ਸ਼ੁਰੂ_idx,
					  ਸਿਰਾ_idx)) {
			ਵਾਪਸ true;
		}
	}

	ਵਾਪਸ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ਬਣਤਰ textsearch_context *textsearch)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(textsearch != ਨਿਰਾ);

	ਜੇ (textsearch->string != ਨਿਰਾ) {
		/* broadcast recent query string */
		textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		ਮੁਕਤ(textsearch->string);
	}

	/* update back state */
	textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
			     CONTENT_TEXTSEARCH_ਪਿੱਠ,
			     true,
			     ਨਿਰਾ);

	/* update forward state */
	textsearch_ਪ੍ਰਸਾਰਣ(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ਨਿਰਾ);

	free_matches(textsearch);
	ਮੁਕਤ(textsearch);

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ਬਣਤਰ hlcache_handle *h,
		   ਖਾਲੀ *context,
		   ਖੋਜ_flags_t flags,
		   ਅਚਲ ਅੱਖਰ *string)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	nserror res;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (string != ਨਿਰਾ &&
	    c->textsearch.string != ਨਿਰਾ &&
	    c->textsearch.context != ਨਿਰਾ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_ਕਦਮ(c->textsearch.context, flags, string);

	} ਨਹੀਂਤਾਂ ਜੇ (string != ਨਿਰਾ) {
		/* New search */
		ਮੁਕਤ(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ਜੇ (c->textsearch.string == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}

		ਜੇ (c->textsearch.context != ਨਿਰਾ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ਨਿਰਾ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}

		content_textsearch_ਕਦਮ(c->textsearch.context, flags, string);

	} ਨਹੀਂਤਾਂ {
		/* Clear search */
		content_textsearch__clear(c);

		ਮੁਕਤ(c->textsearch.string);
		c->textsearch.string = ਨਿਰਾ;
	}

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ਬਣਤਰ hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਵਾਪਸ(content_textsearch__clear(c));
}
