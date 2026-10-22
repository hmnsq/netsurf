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
struct list_entry {
	/**
	 * previous match
	 */
	struct list_entry *prev;

	/**
	 * next match
	 */
	struct list_entry *next;

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
	struct box *start_box;

	/**
	 * content opaque end pointer
	 */
	struct box *end_box;

	/**
	 * content specific selection object
	 */
	struct selection *sel;
};

/**
 * The context for a free text search
 */
struct textsearch_context {

	/**
	 * content search was performed upon
	 */
	struct content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	void *gui_p;

	/**
	 * List of matches
	 */
	struct list_entry *found;

	/**
	 * current selected match
	 */
	struct list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	char *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
static inline void
textsearch_broadcast(struct textsearch_context *textsearch,
		     цяло type,
		     bool state,
		     const char *string)
{
	union content_msg_data msg_data;
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
static void free_matches(struct textsearch_context *textsearch)
{
	struct list_entry *cur;
	struct list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = NULL;
	textsearch->found->next = NULL;

	за (; cur; cur = nxt) {
		nxt = cur->next;
		ако (cur->sel) {
			selection_destroy(cur->sel);
		}
		free(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
static void search_show_all(bool all, struct textsearch_context *context)
{
	struct list_entry *a;

	за (a = context->found->next; a; a = a->next) {
		bool add = true;
		ако (!all && a != context->current) {
			add = false;
			ако (a->sel) {
				selection_destroy(a->sel);
				a->sel = NULL;
			}
		}

		ако (add && !a->sel) {

			a->sel = selection_create(context->c);
			ако (a->sel != NULL) {
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
search_text(struct textsearch_context *context,
	    const char *string,
	    цяло string_len,
	    search_flags_t flags)
{
	struct rect bounds;
	union content_msg_data msg_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & SEARCH_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & SEARCH_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & SEARCH_FLAG_SHOWALL) != 0) ? true : false;

	ако (context->c == NULL) {
		върни res;
	}

	/* check if we need to start a new search or continue an old one */
	ако ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ако (context->string != NULL) {
			free(context->string);
		}

		context->current = NULL;
		free_matches(context);

		context->string = malloc(string_len + 1);
		ако (context->string != NULL) {
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

		ако (res != NSERROR_OK) {
			free_matches(context);
			върни res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} иначе ако (context->current != NULL) {
		/* continued search in the direction specified */
		ако (forwards) {
			ако (context->current->next)
				context->current = context->current->next;
		} иначе {
			ако (context->current->prev)
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


	ако (context->current == NULL) {
		/* no current match */
		върни res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	ако (res == NSERROR_OK) {
		msg_data.scroll.area = true;
		msg_data.scroll.x0 = bounds.x0;
		msg_data.scroll.y0 = bounds.y0;
		msg_data.scroll.x1 = bounds.x1;
		msg_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_MSG_SCROLL, &msg_data);
	}

	върни res;
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
content_textsearch_step(struct textsearch_context *textsearch,
			search_flags_t flags,
			const char *string)
{
	цяло string_len;
	цяло i = 0;
	nserror res = NSERROR_OK;

	assert(textsearch != NULL);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	за (i = 0; i < string_len; i++) {
		ако (string[i] != '#' && string[i] != '*')
			break;
	}

	ако (i < string_len) {
		res = search_text(textsearch, string, string_len, flags);
	} иначе {
		union content_msg_data msg_data;

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

	върни res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
static nserror content_textsearch__clear(struct content *c)
{
	free(c->textsearch.string);
	c->textsearch.string = NULL;

	ако (c->textsearch.context != NULL) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = NULL;
	}
	върни NSERROR_OK;
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
content_textsearch_create(struct content *c,
			  void *gui_data,
			  struct textsearch_context **textsearch_out)
{
	struct textsearch_context *context;
	struct list_entry *search_head;
	content_type type;

	ако ((c->handler->textsearch_find == NULL) ||
	    (c->handler->textsearch_bounds == NULL)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		върни NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(sizeof(struct textsearch_context));
	ако (context == NULL) {
		върни NSERROR_NOMEM;
	}

	search_head = malloc(sizeof(struct list_entry));
	ако (search_head == NULL) {
		free(context);
		върни NSERROR_NOMEM;
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

	върни NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
const char *
content_textsearch_find_pattern(const char *string,
				цяло s_len,
				const char *pattern,
				цяло p_len,
				bool case_sens,
				unsigned цяло *m_len)
{
	struct { const char *ss, *s, *p; bool first; } context[16];
	const char *ep = pattern + p_len;
	const char *es = string  + s_len;
	const char *p = pattern - 1;  /* a virtual '*' before the pattern */
	const char *ss = string;
	const char *s = string;
	bool first = true;
	цяло top = 0;

	докато (p < ep) {
		bool matches;
		ако (p < pattern || *p == '*') {
			char ch;

			/* skip any further asterisks; one is the same as many
			*/
			do p++; докато (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ако (p >= ep) break;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ако (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ако (!case_sens) ch = ascii_to_upper(ch);
				докато (s < es) {
					ако (case_sens) {
						ако (*s == ch) break;
					} иначе ако (ascii_to_upper(*s) == ch)
						break;
					s++;
				}
			}

			ако (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ако (top < (цяло)NOF_ELEMENTS(context)) {
					context[top].ss = ss;
					context[top].s  = s + 1;
					context[top].p  = p - 1;
					/* ptr to last asterisk */
					context[top].first = first;
					top++;
				}

				ако (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} иначе {
				matches = false;
			}

		} иначе ако (s < es) {
			char ch = *p;
			ако (ch == '#')
				matches = true;
			иначе {
				ако (case_sens)
					matches = (*s == ch);
				иначе
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			ако (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} иначе {
			matches = false;
		}

		ако (matches) {
			p++; s++;
		} иначе {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ако (--top < 0)
				върни NULL;  /* no match, give up */

			ss = context[top].ss;
			s  = context[top].s;
			p  = context[top].p;
			first = context[top].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	върни ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(struct textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     struct box *start_box,
			     struct box *end_box)
{
	struct list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, sizeof(*entry));
	ако (entry == NULL) {
		върни NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = NULL;

	entry->next = NULL;
	entry->prev = context->found->prev;

	ако (context->found->prev == NULL) {
		context->found->next = entry;
	} иначе {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	върни NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(struct textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	struct list_entry *cur;

	за (cur = textsearch->found->next; cur != NULL; cur = cur->next) {
		ако (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			върни true;
		}
	}

	върни false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(struct textsearch_context *textsearch)
{
	assert(textsearch != NULL);

	ако (textsearch->string != NULL) {
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

	върни NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(struct hlcache_handle *h,
		   void *context,
		   search_flags_t flags,
		   const char *string)
{
	struct content *c = hlcache_handle_get_content(h);
	nserror res;

	assert(c != NULL);

	ако (string != NULL &&
	    c->textsearch.string != NULL &&
	    c->textsearch.context != NULL &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} иначе ако (string != NULL) {
		/* New search */
		free(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ако (c->textsearch.string == NULL) {
			върни NSERROR_NOMEM;
		}

		ако (c->textsearch.context != NULL) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = NULL;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ако (res != NSERROR_OK) {
			върни res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} иначе {
		/* Clear search */
		content_textsearch__clear(c);

		free(c->textsearch.string);
		c->textsearch.string = NULL;
	}

	върни NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(struct hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	върни(content_textsearch__clear(c));
}
