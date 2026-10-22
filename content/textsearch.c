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
ಸಂರಚನಾ list_entry {
	/**
	 * previous match
	 */
	ಸಂರಚನಾ list_entry *prev;

	/**
	 * next match
	 */
	ಸಂರಚನಾ list_entry *next;

	/**
	 * start position of match
	 */
	unsigned start_idx;

	/**
	 * end of match
	 */
	unsigned end_idx;

	/**
	 * content opaque start pointer
	 */
	ಸಂರಚನಾ box *start_box;

	/**
	 * content opaque end pointer
	 */
	ಸಂರಚನಾ box *end_box;

	/**
	 * content specific selection object
	 */
	ಸಂರಚನಾ selection *sel;
};

/**
 * The context for a free text search
 */
ಸಂರಚನಾ textsearch_context {

	/**
	 * content search was performed upon
	 */
	ಸಂರಚನಾ content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	ಶೂನ್ಯ *gui_p;

	/**
	 * List of matches
	 */
	ಸಂರಚನಾ list_entry *found;

	/**
	 * current selected match
	 */
	ಸಂರಚನಾ list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	ವರ್ಣ *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
textsearch_broadcast(ಸಂರಚನಾ textsearch_context *textsearch,
		     ಪೂರ್ಣಾಙ್ಕ type,
		     bool state,
		     ಧ್ರುವ ವರ್ಣ *string)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಸಂದೇಶ_data.textsearch.type = type;
	ಸಂದೇಶ_data.textsearch.ctx = textsearch->gui_p;
	ಸಂದೇಶ_data.textsearch.state = state;
	ಸಂದೇಶ_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_ಸಂದೇಶ_TEXTSEARCH, &ಸಂದೇಶ_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ಸ್ಥಿರ ಶೂನ್ಯ free_matches(ಸಂರಚನಾ textsearch_context *textsearch)
{
	ಸಂರಚನಾ list_entry *cur;
	ಸಂರಚನಾ list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = ರಿಕ್ತ;
	textsearch->found->next = ರಿಕ್ತ;

	ಪ್ರತಿ (; cur; cur = nxt) {
		nxt = cur->next;
		ಯದಿ (cur->sel) {
			selection_destroy(cur->sel);
		}
		ಮುಕ್ತ(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಹುಡುಕು_show_all(bool all, ಸಂರಚನಾ textsearch_context *context)
{
	ಸಂರಚನಾ list_entry *a;

	ಪ್ರತಿ (a = context->found->next; a; a = a->next) {
		bool add = true;
		ಯದಿ (!all && a != context->current) {
			add = false;
			ಯದಿ (a->sel) {
				selection_destroy(a->sel);
				a->sel = ರಿಕ್ತ;
			}
		}

		ಯದಿ (add && !a->sel) {

			a->sel = selection_create(context->c);
			ಯದಿ (a->sel != ರಿಕ್ತ) {
				selection_init(a->sel);
				selection_set_position(a->sel,
						       a->start_idx,
						       a->end_idx);
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
ಹುಡುಕು_text(ಸಂರಚನಾ textsearch_context *context,
	    ಧ್ರುವ ವರ್ಣ *string,
	    ಪೂರ್ಣಾಙ್ಕ string_len,
	    ಹುಡುಕು_flags_t flags)
{
	ಸಂರಚನಾ rect bounds;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & ಹುಡುಕು_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & ಹುಡುಕು_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & ಹುಡುಕು_FLAG_SHOWALL) != 0) ? true : false;

	ಯದಿ (context->c == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* check if we need to start a new search or continue an old one */
	ಯದಿ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ಯದಿ (context->string != ರಿಕ್ತ) {
			ಮುಕ್ತ(context->string);
		}

		context->current = ರಿಕ್ತ;
		free_matches(context);

		context->string = malloc(string_len + 1);
		ಯದಿ (context->string != ರಿಕ್ತ) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, ರಿಕ್ತ);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, ರಿಕ್ತ);

		ಯದಿ (res != NSERROR_OK) {
			free_matches(context);
			ಪ್ರತ್ಯಾಗಮ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} ಅನ್ಯಥಾ ಯದಿ (context->current != ರಿಕ್ತ) {
		/* continued search in the direction specified */
		ಯದಿ (forwards) {
			ಯದಿ (context->current->next)
				context->current = context->current->next;
		} ಅನ್ಯಥಾ {
			ಯದಿ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != ರಿಕ್ತ),
			     ರಿಕ್ತ);

	ಹುಡುಕು_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != ರಿಕ್ತ) &&
			      (context->current->prev != ರಿಕ್ತ)),
			     ರಿಕ್ತ);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ರಿಕ್ತ) &&
			      (context->current->next != ರಿಕ್ತ)),
			     ರಿಕ್ತ);


	ಯದಿ (context->current == ರಿಕ್ತ) {
		/* no current match */
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	ಯದಿ (res == NSERROR_OK) {
		ಸಂದೇಶ_data.scroll.area = true;
		ಸಂದೇಶ_data.scroll.x0 = bounds.x0;
		ಸಂದೇಶ_data.scroll.y0 = bounds.y0;
		ಸಂದೇಶ_data.scroll.x1 = bounds.x1;
		ಸಂದೇಶ_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_ಸಂದೇಶ_SCROLL, &ಸಂದೇಶ_data);
	}

	ಪ್ರತ್ಯಾಗಮ res;
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
content_textsearch_step(ಸಂರಚನಾ textsearch_context *textsearch,
			ಹುಡುಕು_flags_t flags,
			ಧ್ರುವ ವರ್ಣ *string)
{
	ಪೂರ್ಣಾಙ್ಕ string_len;
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	nserror res = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(textsearch != ರಿಕ್ತ);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	ಪ್ರತಿ (i = 0; i < string_len; i++) {
		ಯದಿ (string[i] != '#' && string[i] != '*')
			ಭಙ್ಗ;
	}

	ಯದಿ (i < string_len) {
		res = ಹುಡುಕು_text(textsearch, string, string_len, flags);
	} ಅನ್ಯಥಾ {
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     ರಿಕ್ತ);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     ರಿಕ್ತ);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ರಿಕ್ತ);

		/* clear scroll */
		ಸಂದೇಶ_data.scroll.area = false;
		ಸಂದೇಶ_data.scroll.x0 = 0;
		ಸಂದೇಶ_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_ಸಂದೇಶ_SCROLL,
				  &ಸಂದೇಶ_data);
	}

	ಪ್ರತ್ಯಾಗಮ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ಸ್ಥಿರ nserror content_textsearch__clear(ಸಂರಚನಾ content *c)
{
	ಮುಕ್ತ(c->textsearch.string);
	c->textsearch.string = ರಿಕ್ತ;

	ಯದಿ (c->textsearch.context != ರಿಕ್ತ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ರಿಕ್ತ;
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
content_textsearch_create(ಸಂರಚನಾ content *c,
			  ಶೂನ್ಯ *gui_data,
			  ಸಂರಚನಾ textsearch_context **textsearch_out)
{
	ಸಂರಚನಾ textsearch_context *context;
	ಸಂರಚನಾ list_entry *ಹುಡುಕು_head;
	content_type type;

	ಯದಿ ((c->handler->textsearch_find == ರಿಕ್ತ) ||
	    (c->handler->textsearch_bounds == ರಿಕ್ತ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ textsearch_context));
	ಯದಿ (context == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಹುಡುಕು_head = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ list_entry));
	ಯದಿ (ಹುಡುಕು_head == ರಿಕ್ತ) {
		ಮುಕ್ತ(context);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಹುಡುಕು_head->start_idx = 0;
	ಹುಡುಕು_head->end_idx = 0;
	ಹುಡುಕು_head->start_box = ರಿಕ್ತ;
	ಹುಡುಕು_head->end_box = ರಿಕ್ತ;
	ಹುಡುಕು_head->sel = ರಿಕ್ತ;
	ಹುಡುಕು_head->prev = ರಿಕ್ತ;
	ಹುಡುಕು_head->next = ರಿಕ್ತ;

	context->found = ಹುಡುಕು_head;
	context->current = ರಿಕ್ತ;
	context->string = ರಿಕ್ತ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ಧ್ರುವ ವರ್ಣ *
content_textsearch_find_pattern(ಧ್ರುವ ವರ್ಣ *string,
				ಪೂರ್ಣಾಙ್ಕ s_len,
				ಧ್ರುವ ವರ್ಣ *pattern,
				ಪೂರ್ಣಾಙ್ಕ p_len,
				bool case_sens,
				unsigned ಪೂರ್ಣಾಙ್ಕ *m_len)
{
	ಸಂರಚನಾ { ಧ್ರುವ ವರ್ಣ *ss, *s, *p; bool first; } context[16];
	ಧ್ರುವ ವರ್ಣ *ep = pattern + p_len;
	ಧ್ರುವ ವರ್ಣ *es = string  + s_len;
	ಧ್ರುವ ವರ್ಣ *p = pattern - 1;  /* a virtual '*' before the pattern */
	ಧ್ರುವ ವರ್ಣ *ss = string;
	ಧ್ರುವ ವರ್ಣ *s = string;
	bool first = true;
	ಪೂರ್ಣಾಙ್ಕ ತುದಿ = 0;

	ಯಾವತ್ (p < ep) {
		bool matches;
		ಯದಿ (p < pattern || *p == '*') {
			ವರ್ಣ ch;

			/* skip any further asterisks; one is the same as many
			*/
			ಕುರು p++; ಯಾವತ್ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ಯದಿ (p >= ep) ಭಙ್ಗ;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ಯದಿ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ಯದಿ (!case_sens) ch = ascii_to_upper(ch);
				ಯಾವತ್ (s < es) {
					ಯದಿ (case_sens) {
						ಯದಿ (*s == ch) ಭಙ್ಗ;
					} ಅನ್ಯಥಾ ಯದಿ (ascii_to_upper(*s) == ch)
						ಭಙ್ಗ;
					s++;
				}
			}

			ಯದಿ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ಯದಿ (ತುದಿ < (ಪೂರ್ಣಾಙ್ಕ)NOF_ELEMENTS(context)) {
					context[ತುದಿ].ss = ss;
					context[ತುದಿ].s  = s + 1;
					context[ತುದಿ].p  = p - 1;
					/* ptr to last asterisk */
					context[ತುದಿ].first = first;
					ತುದಿ++;
				}

				ಯದಿ (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} ಅನ್ಯಥಾ {
				matches = false;
			}

		} ಅನ್ಯಥಾ ಯದಿ (s < es) {
			ವರ್ಣ ch = *p;
			ಯದಿ (ch == '#')
				matches = true;
			ಅನ್ಯಥಾ {
				ಯದಿ (case_sens)
					matches = (*s == ch);
				ಅನ್ಯಥಾ
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			ಯದಿ (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} ಅನ್ಯಥಾ {
			matches = false;
		}

		ಯದಿ (matches) {
			p++; s++;
		} ಅನ್ಯಥಾ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ಯದಿ (--ತುದಿ < 0)
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no match, give up */

			ss = context[ತುದಿ].ss;
			s  = context[ತುದಿ].s;
			p  = context[ತುದಿ].p;
			first = context[ತುದಿ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ಪ್ರತ್ಯಾಗಮ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(ಸಂರಚನಾ textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     ಸಂರಚನಾ box *start_box,
			     ಸಂರಚನಾ box *end_box)
{
	ಸಂರಚನಾ list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, ಪರಿಮಾಣ(*entry));
	ಯದಿ (entry == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = ರಿಕ್ತ;

	entry->next = ರಿಕ್ತ;
	entry->prev = context->found->prev;

	ಯದಿ (context->found->prev == ರಿಕ್ತ) {
		context->found->next = entry;
	} ಅನ್ಯಥಾ {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ಸಂರಚನಾ textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	ಸಂರಚನಾ list_entry *cur;

	ಪ್ರತಿ (cur = textsearch->found->next; cur != ರಿಕ್ತ; cur = cur->next) {
		ಯದಿ (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ಸಂರಚನಾ textsearch_context *textsearch)
{
	ಪ್ರತಿಜ್ಞಾ(textsearch != ರಿಕ್ತ);

	ಯದಿ (textsearch->string != ರಿಕ್ತ) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		ಮುಕ್ತ(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     ರಿಕ್ತ);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ರಿಕ್ತ);

	free_matches(textsearch);
	ಮುಕ್ತ(textsearch);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ಸಂರಚನಾ hlcache_handle *h,
		   ಶೂನ್ಯ *context,
		   ಹುಡುಕು_flags_t flags,
		   ಧ್ರುವ ವರ್ಣ *string)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	nserror res;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (string != ರಿಕ್ತ &&
	    c->textsearch.string != ರಿಕ್ತ &&
	    c->textsearch.context != ರಿಕ್ತ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} ಅನ್ಯಥಾ ಯದಿ (string != ರಿಕ್ತ) {
		/* New search */
		ಮುಕ್ತ(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ಯದಿ (c->textsearch.string == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		ಯದಿ (c->textsearch.context != ರಿಕ್ತ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ರಿಕ್ತ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} ಅನ್ಯಥಾ {
		/* Clear search */
		content_textsearch__clear(c);

		ಮುಕ್ತ(c->textsearch.string);
		c->textsearch.string = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ಸಂರಚನಾ hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಪ್ರತ್ಯಾಗಮ(content_textsearch__clear(c));
}
