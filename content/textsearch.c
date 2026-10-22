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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "netsurf/types.h"
#અન્તર્ભાવ "desktop/selection.h"

#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "content/textsearch.h"

/**
 * search match
 */
સંરચના list_entry {
	/**
	 * previous match
	 */
	સંરચના list_entry *prev;

	/**
	 * next match
	 */
	સંરચના list_entry *next;

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
	સંરચના box *start_box;

	/**
	 * content opaque end pointer
	 */
	સંરચના box *end_box;

	/**
	 * content specific selection object
	 */
	સંરચના selection *sel;
};

/**
 * The context for a free text search
 */
સંરચના textsearch_context {

	/**
	 * content search was performed upon
	 */
	સંરચના content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	શૂન્ય *gui_p;

	/**
	 * List of matches
	 */
	સંરચના list_entry *found;

	/**
	 * current selected match
	 */
	સંરચના list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	અક્ષર *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
સ્થિર અંતઃરેખ શૂન્ય
textsearch_broadcast(સંરચના textsearch_context *textsearch,
		     પૂર્ણાંક type,
		     bool state,
		     અચલ અક્ષર *string)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	સંદેશ_data.textsearch.type = type;
	સંદેશ_data.textsearch.ctx = textsearch->gui_p;
	સંદેશ_data.textsearch.state = state;
	સંદેશ_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_સંદેશ_TEXTSEARCH, &સંદેશ_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
સ્થિર શૂન્ય free_matches(સંરચના textsearch_context *textsearch)
{
	સંરચના list_entry *cur;
	સંરચના list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = ખાલી;
	textsearch->found->next = ખાલી;

	માટે (; cur; cur = nxt) {
		nxt = cur->next;
		જો (cur->sel) {
			selection_destroy(cur->sel);
		}
		મુક્ત(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
સ્થિર શૂન્ય શોધ_show_all(bool all, સંરચના textsearch_context *context)
{
	સંરચના list_entry *a;

	માટે (a = context->found->next; a; a = a->next) {
		bool add = true;
		જો (!all && a != context->current) {
			add = false;
			જો (a->sel) {
				selection_destroy(a->sel);
				a->sel = ખાલી;
			}
		}

		જો (add && !a->sel) {

			a->sel = selection_create(context->c);
			જો (a->sel != ખાલી) {
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
સ્થિર nserror
શોધ_text(સંરચના textsearch_context *context,
	    અચલ અક્ષર *string,
	    પૂર્ણાંક string_len,
	    શોધ_flags_t flags)
{
	સંરચના rect bounds;
	સંઘ content_સંદેશ_data સંદેશ_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & શોધ_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & શોધ_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & શોધ_FLAG_SHOWALL) != 0) ? true : false;

	જો (context->c == ખાલી) {
		પાછા res;
	}

	/* check if we need to start a new search or continue an old one */
	જો ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		જો (context->string != ખાલી) {
			મુક્ત(context->string);
		}

		context->current = ખાલી;
		free_matches(context);

		context->string = malloc(string_len + 1);
		જો (context->string != ખાલી) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, ખાલી);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, ખાલી);

		જો (res != NSERROR_OK) {
			free_matches(context);
			પાછા res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} નહીંતર જો (context->current != ખાલી) {
		/* continued search in the direction specified */
		જો (forwards) {
			જો (context->current->next)
				context->current = context->current->next;
		} નહીંતર {
			જો (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != ખાલી),
			     ખાલી);

	શોધ_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != ખાલી) &&
			      (context->current->prev != ખાલી)),
			     ખાલી);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ખાલી) &&
			      (context->current->next != ખાલી)),
			     ખાલી);


	જો (context->current == ખાલી) {
		/* no current match */
		પાછા res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	જો (res == NSERROR_OK) {
		સંદેશ_data.scroll.area = true;
		સંદેશ_data.scroll.x0 = bounds.x0;
		સંદેશ_data.scroll.y0 = bounds.y0;
		સંદેશ_data.scroll.x1 = bounds.x1;
		સંદેશ_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_સંદેશ_SCROLL, &સંદેશ_data);
	}

	પાછા res;
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
સ્થિર nserror
content_textsearch_step(સંરચના textsearch_context *textsearch,
			શોધ_flags_t flags,
			અચલ અક્ષર *string)
{
	પૂર્ણાંક string_len;
	પૂર્ણાંક i = 0;
	nserror res = NSERROR_OK;

	પ્રતિજ્ઞા(textsearch != ખાલી);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	માટે (i = 0; i < string_len; i++) {
		જો (string[i] != '#' && string[i] != '*')
			તોડો;
	}

	જો (i < string_len) {
		res = શોધ_text(textsearch, string, string_len, flags);
	} નહીંતર {
		સંઘ content_સંદેશ_data સંદેશ_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     ખાલી);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     ખાલી);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ખાલી);

		/* clear scroll */
		સંદેશ_data.scroll.area = false;
		સંદેશ_data.scroll.x0 = 0;
		સંદેશ_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_સંદેશ_SCROLL,
				  &સંદેશ_data);
	}

	પાછા res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
સ્થિર nserror content_textsearch__clear(સંરચના content *c)
{
	મુક્ત(c->textsearch.string);
	c->textsearch.string = ખાલી;

	જો (c->textsearch.context != ખાલી) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ખાલી;
	}
	પાછા NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
સ્થિર nserror
content_textsearch_create(સંરચના content *c,
			  શૂન્ય *gui_data,
			  સંરચના textsearch_context **textsearch_out)
{
	સંરચના textsearch_context *context;
	સંરચના list_entry *શોધ_head;
	content_type type;

	જો ((c->handler->textsearch_find == ખાલી) ||
	    (c->handler->textsearch_bounds == ખાલી)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		પાછા NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(કદ(સંરચના textsearch_context));
	જો (context == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	શોધ_head = malloc(કદ(સંરચના list_entry));
	જો (શોધ_head == ખાલી) {
		મુક્ત(context);
		પાછા NSERROR_NOMEM;
	}

	શોધ_head->start_idx = 0;
	શોધ_head->end_idx = 0;
	શોધ_head->start_box = ખાલી;
	શોધ_head->end_box = ખાલી;
	શોધ_head->sel = ખાલી;
	શોધ_head->prev = ખાલી;
	શોધ_head->next = ખાલી;

	context->found = શોધ_head;
	context->current = ખાલી;
	context->string = ખાલી;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	પાછા NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
અચલ અક્ષર *
content_textsearch_find_pattern(અચલ અક્ષર *string,
				પૂર્ણાંક s_len,
				અચલ અક્ષર *pattern,
				પૂર્ણાંક p_len,
				bool case_sens,
				unsigned પૂર્ણાંક *m_len)
{
	સંરચના { અચલ અક્ષર *ss, *s, *p; bool first; } context[16];
	અચલ અક્ષર *ep = pattern + p_len;
	અચલ અક્ષર *es = string  + s_len;
	અચલ અક્ષર *p = pattern - 1;  /* a virtual '*' before the pattern */
	અચલ અક્ષર *ss = string;
	અચલ અક્ષર *s = string;
	bool first = true;
	પૂર્ણાંક ટોચ = 0;

	જ્યારે (p < ep) {
		bool matches;
		જો (p < pattern || *p == '*') {
			અક્ષર ch;

			/* skip any further asterisks; one is the same as many
			*/
			કરો p++; જ્યારે (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			જો (p >= ep) તોડો;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			જો (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				જો (!case_sens) ch = ascii_to_upper(ch);
				જ્યારે (s < es) {
					જો (case_sens) {
						જો (*s == ch) તોડો;
					} નહીંતર જો (ascii_to_upper(*s) == ch)
						તોડો;
					s++;
				}
			}

			જો (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				જો (ટોચ < (પૂર્ણાંક)NOF_ELEMENTS(context)) {
					context[ટોચ].ss = ss;
					context[ટોચ].s  = s + 1;
					context[ટોચ].p  = p - 1;
					/* ptr to last asterisk */
					context[ટોચ].first = first;
					ટોચ++;
				}

				જો (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} નહીંતર {
				matches = false;
			}

		} નહીંતર જો (s < es) {
			અક્ષર ch = *p;
			જો (ch == '#')
				matches = true;
			નહીંતર {
				જો (case_sens)
					matches = (*s == ch);
				નહીંતર
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			જો (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} નહીંતર {
			matches = false;
		}

		જો (matches) {
			p++; s++;
		} નહીંતર {
			/* doesn't match,
			 * resume with stacked context if we have one */
			જો (--ટોચ < 0)
				પાછા ખાલી;  /* no match, give up */

			ss = context[ટોચ].ss;
			s  = context[ટોચ].s;
			p  = context[ટોચ].p;
			first = context[ટોચ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	પાછા ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(સંરચના textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     સંરચના box *start_box,
			     સંરચના box *end_box)
{
	સંરચના list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, કદ(*entry));
	જો (entry == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = ખાલી;

	entry->next = ખાલી;
	entry->prev = context->found->prev;

	જો (context->found->prev == ખાલી) {
		context->found->next = entry;
	} નહીંતર {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	પાછા NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(સંરચના textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	સંરચના list_entry *cur;

	માટે (cur = textsearch->found->next; cur != ખાલી; cur = cur->next) {
		જો (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			પાછા true;
		}
	}

	પાછા false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(સંરચના textsearch_context *textsearch)
{
	પ્રતિજ્ઞા(textsearch != ખાલી);

	જો (textsearch->string != ખાલી) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		મુક્ત(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     ખાલી);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ખાલી);

	free_matches(textsearch);
	મુક્ત(textsearch);

	પાછા NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(સંરચના hlcache_handle *h,
		   શૂન્ય *context,
		   શોધ_flags_t flags,
		   અચલ અક્ષર *string)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	nserror res;

	પ્રતિજ્ઞા(c != ખાલી);

	જો (string != ખાલી &&
	    c->textsearch.string != ખાલી &&
	    c->textsearch.context != ખાલી &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} નહીંતર જો (string != ખાલી) {
		/* New search */
		મુક્ત(c->textsearch.string);
		c->textsearch.string = strdup(string);
		જો (c->textsearch.string == ખાલી) {
			પાછા NSERROR_NOMEM;
		}

		જો (c->textsearch.context != ખાલી) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ખાલી;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		જો (res != NSERROR_OK) {
			પાછા res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} નહીંતર {
		/* Clear search */
		content_textsearch__clear(c);

		મુક્ત(c->textsearch.string);
		c->textsearch.string = ખાલી;
	}

	પાછા NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(સંરચના hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	પાછા(content_textsearch__clear(c));
}
