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

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "utils/errors.h"
#include "utils/utils.h"
#include "utils/ascii.h"
#include "netsurf/types.h"
#include "desktop/selection.h"

#include "content/content.h"
#include "content/content_protected.h"
#include "content/hlcache.h"
#include "content/textsearch.h"

/**
 * search match
 */
sa_nracanaa list_entry {
	/**
	 * previous match
	 */
	sa_nracanaa list_entry *prev;

	/**
	 * next match
	 */
	sa_nracanaa list_entry *next;

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
	sa_nracanaa box *start_box;

	/**
	 * content opaque end pointer
	 */
	sa_nracanaa box *end_box;

	/**
	 * content specific selection object
	 */
	sa_nracanaa selection *sel;
};

/**
 * The context for a free text search
 */
sa_nracanaa textsearch_context {

	/**
	 * content search was performed upon
	 */
	sa_nracanaa content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	void *gui_p;

	/**
	 * List of matches
	 */
	sa_nracanaa list_entry *found;

	/**
	 * current selected match
	 */
	sa_nracanaa list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	var_na *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
static anta_hstha void
textsearch_broadcast(sa_nracanaa textsearch_context *textsearch,
		     puur_naa_nka type,
		     bool state,
		     const var_na *string)
{
	sa_ngha content_msg_data msg_data;
	msg_data.textsearch.type = type;
	msg_data.textsearch.ctx = textsearch->gui_p;
	msg_data.textsearch.state = state;
	msg_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_MSG_TEXTSEARCH, &msg_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
static void free_matches(sa_nracanaa textsearch_context *textsearch)
{
	sa_nracanaa list_entry *cur;
	sa_nracanaa list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = NULL;
	textsearch->found->next = NULL;

	for (; cur; cur = nxt) {
		nxt = cur->next;
		ᮚᮦᮔ᮪ (cur->sel) {
			selection_destroy(cur->sel);
		}
		free(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
static void search_show_all(bool all, sa_nracanaa textsearch_context *context)
{
	sa_nracanaa list_entry *a;

	for (a = context->found->next; a; a = a->next) {
		bool add = true;
		ᮚᮦᮔ᮪ (!all && a != context->current) {
			add = false;
			ᮚᮦᮔ᮪ (a->sel) {
				selection_destroy(a->sel);
				a->sel = NULL;
			}
		}

		ᮚᮦᮔ᮪ (add && !a->sel) {

			a->sel = selection_create(context->c);
			ᮚᮦᮔ᮪ (a->sel != NULL) {
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
static nserror
search_text(sa_nracanaa textsearch_context *context,
	    const var_na *string,
	    puur_naa_nka string_len,
	    search_flags_t flags)
{
	sa_nracanaa rect bounds;
	sa_ngha content_msg_data msg_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & SEARCH_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & SEARCH_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & SEARCH_FLAG_SHOWALL) != 0) ? true : false;

	ᮚᮦᮔ᮪ (context->c == NULL) {
		ᮝᮜᮤ res;
	}

	/* check if we need to start a new search or continue an old one */
	ᮚᮦᮔ᮪ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ᮚᮦᮔ᮪ (context->string != NULL) {
			free(context->string);
		}

		context->current = NULL;
		free_matches(context);

		context->string = malloc(string_len + 1);
		ᮚᮦᮔ᮪ (context->string != NULL) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, NULL);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, NULL);

		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			free_matches(context);
			ᮝᮜᮤ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (context->current != NULL) {
		/* continued search in the direction specified */
		ᮚᮦᮔ᮪ (forwards) {
			ᮚᮦᮔ᮪ (context->current->next)
				context->current = context->current->next;
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != NULL),
			     NULL);

	search_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != NULL) &&
			      (context->current->prev != NULL)),
			     NULL);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != NULL) &&
			      (context->current->next != NULL)),
			     NULL);


	ᮚᮦᮔ᮪ (context->current == NULL) {
		/* no current match */
		ᮝᮜᮤ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	ᮚᮦᮔ᮪ (res == NSERROR_OK) {
		msg_data.scroll.area = true;
		msg_data.scroll.x0 = bounds.x0;
		msg_data.scroll.y0 = bounds.y0;
		msg_data.scroll.x1 = bounds.x1;
		msg_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_MSG_SCROLL, &msg_data);
	}

	ᮝᮜᮤ res;
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
static nserror
content_textsearch_step(sa_nracanaa textsearch_context *textsearch,
			search_flags_t flags,
			const var_na *string)
{
	puur_naa_nka string_len;
	puur_naa_nka i = 0;
	nserror res = NSERROR_OK;

	assert(textsearch != NULL);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	for (i = 0; i < string_len; i++) {
		ᮚᮦᮔ᮪ (string[i] != '#' && string[i] != '*')
			bha_nga;
	}

	ᮚᮦᮔ᮪ (i < string_len) {
		res = search_text(textsearch, string, string_len, flags);
	} ᮜᮤᮚ {
		sa_ngha content_msg_data msg_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     NULL);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     NULL);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     NULL);

		/* clear scroll */
		msg_data.scroll.area = false;
		msg_data.scroll.x0 = 0;
		msg_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_MSG_SCROLL,
				  &msg_data);
	}

	ᮝᮜᮤ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
static nserror content_textsearch__clear(sa_nracanaa content *c)
{
	free(c->textsearch.string);
	c->textsearch.string = NULL;

	ᮚᮦᮔ᮪ (c->textsearch.context != NULL) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = NULL;
	}
	ᮝᮜᮤ NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
static nserror
content_textsearch_create(sa_nracanaa content *c,
			  void *gui_data,
			  sa_nracanaa textsearch_context **textsearch_out)
{
	sa_nracanaa textsearch_context *context;
	sa_nracanaa list_entry *search_head;
	content_type type;

	ᮚᮦᮔ᮪ ((c->handler->textsearch_find == NULL) ||
	    (c->handler->textsearch_bounds == NULL)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ᮝᮜᮤ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(parimaa_na(sa_nracanaa textsearch_context));
	ᮚᮦᮔ᮪ (context == NULL) {
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	search_head = malloc(parimaa_na(sa_nracanaa list_entry));
	ᮚᮦᮔ᮪ (search_head == NULL) {
		free(context);
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	search_head->start_idx = 0;
	search_head->end_idx = 0;
	search_head->start_box = NULL;
	search_head->end_box = NULL;
	search_head->sel = NULL;
	search_head->prev = NULL;
	search_head->next = NULL;

	context->found = search_head;
	context->current = NULL;
	context->string = NULL;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
const var_na *
content_textsearch_find_pattern(const var_na *string,
				puur_naa_nka s_len,
				const var_na *pattern,
				puur_naa_nka p_len,
				bool case_sens,
				unsigned puur_naa_nka *m_len)
{
	sa_nracanaa { const var_na *ss, *s, *p; bool first; } context[16];
	const var_na *ep = pattern + p_len;
	const var_na *es = string  + s_len;
	const var_na *p = pattern - 1;  /* a virtual '*' before the pattern */
	const var_na *ss = string;
	const var_na *s = string;
	bool first = true;
	puur_naa_nka top = 0;

	ᮞᮨᮓᮨᮜ (p < ep) {
		bool matches;
		ᮚᮦᮔ᮪ (p < pattern || *p == '*') {
			var_na ch;

			/* skip any further asterisks; one is the same as many
			*/
			do p++; ᮞᮨᮓᮨᮜ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ᮚᮦᮔ᮪ (p >= ep) bha_nga;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ᮚᮦᮔ᮪ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ᮚᮦᮔ᮪ (!case_sens) ch = ascii_to_upper(ch);
				ᮞᮨᮓᮨᮜ (s < es) {
					ᮚᮦᮔ᮪ (case_sens) {
						ᮚᮦᮔ᮪ (*s == ch) bha_nga;
					} ᮜᮤᮚ ᮚᮦᮔ᮪ (ascii_to_upper(*s) == ch)
						bha_nga;
					s++;
				}
			}

			ᮚᮦᮔ᮪ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ᮚᮦᮔ᮪ (top < (puur_naa_nka)NOF_ELEMENTS(context)) {
					context[top].ss = ss;
					context[top].s  = s + 1;
					context[top].p  = p - 1;
					/* ptr to last asterisk */
					context[top].first = first;
					top++;
				}

				ᮚᮦᮔ᮪ (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} ᮜᮤᮚ {
				matches = false;
			}

		} ᮜᮤᮚ ᮚᮦᮔ᮪ (s < es) {
			var_na ch = *p;
			ᮚᮦᮔ᮪ (ch == '#')
				matches = true;
			ᮜᮤᮚ {
				ᮚᮦᮔ᮪ (case_sens)
					matches = (*s == ch);
				ᮜᮤᮚ
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			ᮚᮦᮔ᮪ (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} ᮜᮤᮚ {
			matches = false;
		}

		ᮚᮦᮔ᮪ (matches) {
			p++; s++;
		} ᮜᮤᮚ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ᮚᮦᮔ᮪ (--top < 0)
				ᮝᮜᮤ NULL;  /* no match, give up */

			ss = context[top].ss;
			s  = context[top].s;
			p  = context[top].p;
			first = context[top].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ᮝᮜᮤ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(sa_nracanaa textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     sa_nracanaa box *start_box,
			     sa_nracanaa box *end_box)
{
	sa_nracanaa list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, parimaa_na(*entry));
	ᮚᮦᮔ᮪ (entry == NULL) {
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = NULL;

	entry->next = NULL;
	entry->prev = context->found->prev;

	ᮚᮦᮔ᮪ (context->found->prev == NULL) {
		context->found->next = entry;
	} ᮜᮤᮚ {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(sa_nracanaa textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	sa_nracanaa list_entry *cur;

	for (cur = textsearch->found->next; cur != NULL; cur = cur->next) {
		ᮚᮦᮔ᮪ (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			ᮝᮜᮤ true;
		}
	}

	ᮝᮜᮤ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(sa_nracanaa textsearch_context *textsearch)
{
	assert(textsearch != NULL);

	ᮚᮦᮔ᮪ (textsearch->string != NULL) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		free(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     NULL);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     NULL);

	free_matches(textsearch);
	free(textsearch);

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(sa_nracanaa hlcache_handle *h,
		   void *context,
		   search_flags_t flags,
		   const var_na *string)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	nserror res;

	assert(c != NULL);

	ᮚᮦᮔ᮪ (string != NULL &&
	    c->textsearch.string != NULL &&
	    c->textsearch.context != NULL &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (string != NULL) {
		/* New search */
		free(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ᮚᮦᮔ᮪ (c->textsearch.string == NULL) {
			ᮝᮜᮤ NSERROR_NOMEM;
		}

		ᮚᮦᮔ᮪ (c->textsearch.context != NULL) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = NULL;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			ᮝᮜᮤ res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} ᮜᮤᮚ {
		/* Clear search */
		content_textsearch__clear(c);

		free(c->textsearch.string);
		c->textsearch.string = NULL;
	}

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(sa_nracanaa hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮝᮜᮤ(content_textsearch__clear(c));
}
