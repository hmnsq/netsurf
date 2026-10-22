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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "desktop/selection.h"

#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "content/textsearch.h"

/**
 * search match
 */
ഘടന list_entry {
	/**
	 * previous match
	 */
	ഘടന list_entry *prev;

	/**
	 * next match
	 */
	ഘടന list_entry *next;

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
	ഘടന box *start_box;

	/**
	 * content opaque end pointer
	 */
	ഘടന box *end_box;

	/**
	 * content specific selection object
	 */
	ഘടന selection *sel;
};

/**
 * The context for a free text search
 */
ഘടന textsearch_context {

	/**
	 * content search was performed upon
	 */
	ഘടന content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	ശൂന്യം *gui_p;

	/**
	 * List of matches
	 */
	ഘടന list_entry *found;

	/**
	 * current selected match
	 */
	ഘടന list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	അക്ഷരം *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
സ്ഥിതം വരിയിൽ ശൂന്യം
textsearch_broadcast(ഘടന textsearch_context *textsearch,
		     പൂർണ്ണസംഖ്യ type,
		     bool state,
		     സ്ഥിരം അക്ഷരം *string)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	സന്ദേശം_data.textsearch.type = type;
	സന്ദേശം_data.textsearch.ctx = textsearch->gui_p;
	സന്ദേശം_data.textsearch.state = state;
	സന്ദേശം_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_സന്ദേശം_TEXTSEARCH, &സന്ദേശം_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
സ്ഥിതം ശൂന്യം free_matches(ഘടന textsearch_context *textsearch)
{
	ഘടന list_entry *cur;
	ഘടന list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = ശൂന്യ;
	textsearch->found->next = ശൂന്യ;

	വേണ്ടി (; cur; cur = nxt) {
		nxt = cur->next;
		എങ്കിൽ (cur->sel) {
			selection_destroy(cur->sel);
		}
		സ്വതന്ത്രം(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
സ്ഥിതം ശൂന്യം തിരയൽ_show_all(bool all, ഘടന textsearch_context *context)
{
	ഘടന list_entry *a;

	വേണ്ടി (a = context->found->next; a; a = a->next) {
		bool add = true;
		എങ്കിൽ (!all && a != context->current) {
			add = false;
			എങ്കിൽ (a->sel) {
				selection_destroy(a->sel);
				a->sel = ശൂന്യ;
			}
		}

		എങ്കിൽ (add && !a->sel) {

			a->sel = selection_create(context->c);
			എങ്കിൽ (a->sel != ശൂന്യ) {
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
സ്ഥിതം nserror
തിരയൽ_text(ഘടന textsearch_context *context,
	    സ്ഥിരം അക്ഷരം *string,
	    പൂർണ്ണസംഖ്യ string_len,
	    തിരയൽ_flags_t flags)
{
	ഘടന rect bounds;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & തിരയൽ_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & തിരയൽ_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & തിരയൽ_FLAG_SHOWALL) != 0) ? true : false;

	എങ്കിൽ (context->c == ശൂന്യ) {
		തിരികെ res;
	}

	/* check if we need to start a new search or continue an old one */
	എങ്കിൽ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		എങ്കിൽ (context->string != ശൂന്യ) {
			സ്വതന്ത്രം(context->string);
		}

		context->current = ശൂന്യ;
		free_matches(context);

		context->string = malloc(string_len + 1);
		എങ്കിൽ (context->string != ശൂന്യ) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, ശൂന്യ);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, ശൂന്യ);

		എങ്കിൽ (res != NSERROR_OK) {
			free_matches(context);
			തിരികെ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} അല്ലെങ്കിൽ എങ്കിൽ (context->current != ശൂന്യ) {
		/* continued search in the direction specified */
		എങ്കിൽ (forwards) {
			എങ്കിൽ (context->current->next)
				context->current = context->current->next;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != ശൂന്യ),
			     ശൂന്യ);

	തിരയൽ_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != ശൂന്യ) &&
			      (context->current->prev != ശൂന്യ)),
			     ശൂന്യ);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ശൂന്യ) &&
			      (context->current->next != ശൂന്യ)),
			     ശൂന്യ);


	എങ്കിൽ (context->current == ശൂന്യ) {
		/* no current match */
		തിരികെ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	എങ്കിൽ (res == NSERROR_OK) {
		സന്ദേശം_data.scroll.area = true;
		സന്ദേശം_data.scroll.x0 = bounds.x0;
		സന്ദേശം_data.scroll.y0 = bounds.y0;
		സന്ദേശം_data.scroll.x1 = bounds.x1;
		സന്ദേശം_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_സന്ദേശം_SCROLL, &സന്ദേശം_data);
	}

	തിരികെ res;
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
സ്ഥിതം nserror
content_textsearch_step(ഘടന textsearch_context *textsearch,
			തിരയൽ_flags_t flags,
			സ്ഥിരം അക്ഷരം *string)
{
	പൂർണ്ണസംഖ്യ string_len;
	പൂർണ്ണസംഖ്യ i = 0;
	nserror res = NSERROR_OK;

	പ്രതിജ്ഞാ(textsearch != ശൂന്യ);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	വേണ്ടി (i = 0; i < string_len; i++) {
		എങ്കിൽ (string[i] != '#' && string[i] != '*')
			നിർത്തുക;
	}

	എങ്കിൽ (i < string_len) {
		res = തിരയൽ_text(textsearch, string, string_len, flags);
	} അല്ലെങ്കിൽ {
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     ശൂന്യ);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     ശൂന്യ);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ശൂന്യ);

		/* clear scroll */
		സന്ദേശം_data.scroll.area = false;
		സന്ദേശം_data.scroll.x0 = 0;
		സന്ദേശം_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_സന്ദേശം_SCROLL,
				  &സന്ദേശം_data);
	}

	തിരികെ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
സ്ഥിതം nserror content_textsearch__clear(ഘടന content *c)
{
	സ്വതന്ത്രം(c->textsearch.string);
	c->textsearch.string = ശൂന്യ;

	എങ്കിൽ (c->textsearch.context != ശൂന്യ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ശൂന്യ;
	}
	തിരികെ NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
സ്ഥിതം nserror
content_textsearch_create(ഘടന content *c,
			  ശൂന്യം *gui_data,
			  ഘടന textsearch_context **textsearch_out)
{
	ഘടന textsearch_context *context;
	ഘടന list_entry *തിരയൽ_head;
	content_type type;

	എങ്കിൽ ((c->handler->textsearch_find == ശൂന്യ) ||
	    (c->handler->textsearch_bounds == ശൂന്യ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		തിരികെ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(വലുപ്പം(ഘടന textsearch_context));
	എങ്കിൽ (context == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	തിരയൽ_head = malloc(വലുപ്പം(ഘടന list_entry));
	എങ്കിൽ (തിരയൽ_head == ശൂന്യ) {
		സ്വതന്ത്രം(context);
		തിരികെ NSERROR_NOMEM;
	}

	തിരയൽ_head->start_idx = 0;
	തിരയൽ_head->end_idx = 0;
	തിരയൽ_head->start_box = ശൂന്യ;
	തിരയൽ_head->end_box = ശൂന്യ;
	തിരയൽ_head->sel = ശൂന്യ;
	തിരയൽ_head->prev = ശൂന്യ;
	തിരയൽ_head->next = ശൂന്യ;

	context->found = തിരയൽ_head;
	context->current = ശൂന്യ;
	context->string = ശൂന്യ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	തിരികെ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
സ്ഥിരം അക്ഷരം *
content_textsearch_find_pattern(സ്ഥിരം അക്ഷരം *string,
				പൂർണ്ണസംഖ്യ s_len,
				സ്ഥിരം അക്ഷരം *pattern,
				പൂർണ്ണസംഖ്യ p_len,
				bool case_sens,
				unsigned പൂർണ്ണസംഖ്യ *m_len)
{
	ഘടന { സ്ഥിരം അക്ഷരം *ss, *s, *p; bool first; } context[16];
	സ്ഥിരം അക്ഷരം *ep = pattern + p_len;
	സ്ഥിരം അക്ഷരം *es = string  + s_len;
	സ്ഥിരം അക്ഷരം *p = pattern - 1;  /* a virtual '*' before the pattern */
	സ്ഥിരം അക്ഷരം *ss = string;
	സ്ഥിരം അക്ഷരം *s = string;
	bool first = true;
	പൂർണ്ണസംഖ്യ മുകൾ = 0;

	ആകുമ്പോൾ (p < ep) {
		bool matches;
		എങ്കിൽ (p < pattern || *p == '*') {
			അക്ഷരം ch;

			/* skip any further asterisks; one is the same as many
			*/
			ചെയ്യുക p++; ആകുമ്പോൾ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			എങ്കിൽ (p >= ep) നിർത്തുക;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			എങ്കിൽ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				എങ്കിൽ (!case_sens) ch = ascii_to_upper(ch);
				ആകുമ്പോൾ (s < es) {
					എങ്കിൽ (case_sens) {
						എങ്കിൽ (*s == ch) നിർത്തുക;
					} അല്ലെങ്കിൽ എങ്കിൽ (ascii_to_upper(*s) == ch)
						നിർത്തുക;
					s++;
				}
			}

			എങ്കിൽ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				എങ്കിൽ (മുകൾ < (പൂർണ്ണസംഖ്യ)NOF_ELEMENTS(context)) {
					context[മുകൾ].ss = ss;
					context[മുകൾ].s  = s + 1;
					context[മുകൾ].p  = p - 1;
					/* ptr to last asterisk */
					context[മുകൾ].first = first;
					മുകൾ++;
				}

				എങ്കിൽ (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} അല്ലെങ്കിൽ {
				matches = false;
			}

		} അല്ലെങ്കിൽ എങ്കിൽ (s < es) {
			അക്ഷരം ch = *p;
			എങ്കിൽ (ch == '#')
				matches = true;
			അല്ലെങ്കിൽ {
				എങ്കിൽ (case_sens)
					matches = (*s == ch);
				അല്ലെങ്കിൽ
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			എങ്കിൽ (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} അല്ലെങ്കിൽ {
			matches = false;
		}

		എങ്കിൽ (matches) {
			p++; s++;
		} അല്ലെങ്കിൽ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			എങ്കിൽ (--മുകൾ < 0)
				തിരികെ ശൂന്യ;  /* no match, give up */

			ss = context[മുകൾ].ss;
			s  = context[മുകൾ].s;
			p  = context[മുകൾ].p;
			first = context[മുകൾ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	തിരികെ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(ഘടന textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     ഘടന box *start_box,
			     ഘടന box *end_box)
{
	ഘടന list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, വലുപ്പം(*entry));
	എങ്കിൽ (entry == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = ശൂന്യ;

	entry->next = ശൂന്യ;
	entry->prev = context->found->prev;

	എങ്കിൽ (context->found->prev == ശൂന്യ) {
		context->found->next = entry;
	} അല്ലെങ്കിൽ {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	തിരികെ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ഘടന textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	ഘടന list_entry *cur;

	വേണ്ടി (cur = textsearch->found->next; cur != ശൂന്യ; cur = cur->next) {
		എങ്കിൽ (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			തിരികെ true;
		}
	}

	തിരികെ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ഘടന textsearch_context *textsearch)
{
	പ്രതിജ്ഞാ(textsearch != ശൂന്യ);

	എങ്കിൽ (textsearch->string != ശൂന്യ) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		സ്വതന്ത്രം(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     ശൂന്യ);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ശൂന്യ);

	free_matches(textsearch);
	സ്വതന്ത്രം(textsearch);

	തിരികെ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ഘടന hlcache_handle *h,
		   ശൂന്യം *context,
		   തിരയൽ_flags_t flags,
		   സ്ഥിരം അക്ഷരം *string)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	nserror res;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (string != ശൂന്യ &&
	    c->textsearch.string != ശൂന്യ &&
	    c->textsearch.context != ശൂന്യ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} അല്ലെങ്കിൽ എങ്കിൽ (string != ശൂന്യ) {
		/* New search */
		സ്വതന്ത്രം(c->textsearch.string);
		c->textsearch.string = strdup(string);
		എങ്കിൽ (c->textsearch.string == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}

		എങ്കിൽ (c->textsearch.context != ശൂന്യ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ശൂന്യ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} അല്ലെങ്കിൽ {
		/* Clear search */
		content_textsearch__clear(c);

		സ്വതന്ത്രം(c->textsearch.string);
		c->textsearch.string = ശൂന്യ;
	}

	തിരികെ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ഘടന hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	തിരികെ(content_textsearch__clear(c));
}
