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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"

#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"

/**
 * search match
 */
ସଂରଚନା list_entry {
	/**
	 * previous match
	 */
	ସଂରଚନା list_entry *prev;

	/**
	 * next match
	 */
	ସଂରଚନା list_entry *next;

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
	ସଂରଚନା box *start_box;

	/**
	 * content opaque end pointer
	 */
	ସଂରଚନା box *end_box;

	/**
	 * content specific selection object
	 */
	ସଂରଚନା selection *sel;
};

/**
 * The context for a free text search
 */
ସଂରଚନା textsearch_context {

	/**
	 * content search was performed upon
	 */
	ସଂରଚନା content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	ଶୂନ୍ୟ *gui_p;

	/**
	 * List of matches
	 */
	ସଂରଚନା list_entry *found;

	/**
	 * current selected match
	 */
	ସଂରଚନା list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	ଅକ୍ଷର *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
textsearch_broadcast(ସଂରଚନା textsearch_context *textsearch,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା type,
		     bool state,
		     ଅଚଳ ଅକ୍ଷର *string)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ସନ୍ଦେଶ_data.textsearch.type = type;
	ସନ୍ଦେଶ_data.textsearch.ctx = textsearch->gui_p;
	ସନ୍ଦେଶ_data.textsearch.state = state;
	ସନ୍ଦେଶ_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_ସନ୍ଦେଶ_TEXTSEARCH, &ସନ୍ଦେଶ_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ସ୍ଥିର ଶୂନ୍ୟ free_matches(ସଂରଚନା textsearch_context *textsearch)
{
	ସଂରଚନା list_entry *cur;
	ସଂରଚନା list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = ଖାଲି;
	textsearch->found->next = ଖାଲି;

	ପାଇଁ (; cur; cur = nxt) {
		nxt = cur->next;
		ଯଦି (cur->sel) {
			selection_destroy(cur->sel);
		}
		ମୁକ୍ତ(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ସ୍ଥିର ଶୂନ୍ୟ ସନ୍ଧାନ_show_all(bool all, ସଂରଚନା textsearch_context *context)
{
	ସଂରଚନା list_entry *a;

	ପାଇଁ (a = context->found->next; a; a = a->next) {
		bool add = true;
		ଯଦି (!all && a != context->current) {
			add = false;
			ଯଦି (a->sel) {
				selection_destroy(a->sel);
				a->sel = ଖାଲି;
			}
		}

		ଯଦି (add && !a->sel) {

			a->sel = selection_create(context->c);
			ଯଦି (a->sel != ଖାଲି) {
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
ସ୍ଥିର nserror
ସନ୍ଧାନ_text(ସଂରଚନା textsearch_context *context,
	    ଅଚଳ ଅକ୍ଷର *string,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_len,
	    ସନ୍ଧାନ_flags_t flags)
{
	ସଂରଚନା rect bounds;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & ସନ୍ଧାନ_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & ସନ୍ଧାନ_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & ସନ୍ଧାନ_FLAG_SHOWALL) != 0) ? true : false;

	ଯଦି (context->c == ଖାଲି) {
		ଫେରନ୍ତୁ res;
	}

	/* check if we need to start a new search or continue an old one */
	ଯଦି ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ଯଦି (context->string != ଖାଲି) {
			ମୁକ୍ତ(context->string);
		}

		context->current = ଖାଲି;
		free_matches(context);

		context->string = malloc(string_len + 1);
		ଯଦି (context->string != ଖାଲି) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, ଖାଲି);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, ଖାଲି);

		ଯଦି (res != NSERROR_OK) {
			free_matches(context);
			ଫେରନ୍ତୁ res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} ଅନ୍ୟଥା ଯଦି (context->current != ଖାଲି) {
		/* continued search in the direction specified */
		ଯଦି (forwards) {
			ଯଦି (context->current->next)
				context->current = context->current->next;
		} ଅନ୍ୟଥା {
			ଯଦି (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != ଖାଲି),
			     ଖାଲି);

	ସନ୍ଧାନ_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != ଖାଲି) &&
			      (context->current->prev != ଖାଲି)),
			     ଖାଲି);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ଖାଲି) &&
			      (context->current->next != ଖାଲି)),
			     ଖାଲି);


	ଯଦି (context->current == ଖାଲି) {
		/* no current match */
		ଫେରନ୍ତୁ res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	ଯଦି (res == NSERROR_OK) {
		ସନ୍ଦେଶ_data.scroll.area = true;
		ସନ୍ଦେଶ_data.scroll.x0 = bounds.x0;
		ସନ୍ଦେଶ_data.scroll.y0 = bounds.y0;
		ସନ୍ଦେଶ_data.scroll.x1 = bounds.x1;
		ସନ୍ଦେଶ_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_ସନ୍ଦେଶ_SCROLL, &ସନ୍ଦେଶ_data);
	}

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror
content_textsearch_step(ସଂରଚନା textsearch_context *textsearch,
			ସନ୍ଧାନ_flags_t flags,
			ଅଚଳ ଅକ୍ଷର *string)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	nserror res = NSERROR_OK;

	ପ୍ରତିଜ୍ଞା(textsearch != ଖାଲି);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	ପାଇଁ (i = 0; i < string_len; i++) {
		ଯଦି (string[i] != '#' && string[i] != '*')
			ଭାଙ୍ଗ;
	}

	ଯଦି (i < string_len) {
		res = ସନ୍ଧାନ_text(textsearch, string, string_len, flags);
	} ଅନ୍ୟଥା {
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     ଖାଲି);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     ଖାଲି);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ଖାଲି);

		/* clear scroll */
		ସନ୍ଦେଶ_data.scroll.area = false;
		ସନ୍ଦେଶ_data.scroll.x0 = 0;
		ସନ୍ଦେଶ_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_ସନ୍ଦେଶ_SCROLL,
				  &ସନ୍ଦେଶ_data);
	}

	ଫେରନ୍ତୁ res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ସ୍ଥିର nserror content_textsearch__clear(ସଂରଚନା content *c)
{
	ମୁକ୍ତ(c->textsearch.string);
	c->textsearch.string = ଖାଲି;

	ଯଦି (c->textsearch.context != ଖାଲି) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ଖାଲି;
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ସ୍ଥିର nserror
content_textsearch_create(ସଂରଚନା content *c,
			  ଶୂନ୍ୟ *gui_data,
			  ସଂରଚନା textsearch_context **textsearch_out)
{
	ସଂରଚନା textsearch_context *context;
	ସଂରଚନା list_entry *ସନ୍ଧାନ_head;
	content_type type;

	ଯଦି ((c->handler->textsearch_find == ଖାଲି) ||
	    (c->handler->textsearch_bounds == ଖାଲି)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ଫେରନ୍ତୁ NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(ଆକାର(ସଂରଚନା textsearch_context));
	ଯଦି (context == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ସନ୍ଧାନ_head = malloc(ଆକାର(ସଂରଚନା list_entry));
	ଯଦି (ସନ୍ଧାନ_head == ଖାଲି) {
		ମୁକ୍ତ(context);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ସନ୍ଧାନ_head->start_idx = 0;
	ସନ୍ଧାନ_head->end_idx = 0;
	ସନ୍ଧାନ_head->start_box = ଖାଲି;
	ସନ୍ଧାନ_head->end_box = ଖାଲି;
	ସନ୍ଧାନ_head->sel = ଖାଲି;
	ସନ୍ଧାନ_head->prev = ଖାଲି;
	ସନ୍ଧାନ_head->next = ଖାଲି;

	context->found = ସନ୍ଧାନ_head;
	context->current = ଖାଲି;
	context->string = ଖାଲି;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ଅଚଳ ଅକ୍ଷର *
content_textsearch_find_pattern(ଅଚଳ ଅକ୍ଷର *string,
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା s_len,
				ଅଚଳ ଅକ୍ଷର *pattern,
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_len,
				bool case_sens,
				unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *m_len)
{
	ସଂରଚନା { ଅଚଳ ଅକ୍ଷର *ss, *s, *p; bool first; } context[16];
	ଅଚଳ ଅକ୍ଷର *ep = pattern + p_len;
	ଅଚଳ ଅକ୍ଷର *es = string  + s_len;
	ଅଚଳ ଅକ୍ଷର *p = pattern - 1;  /* a virtual '*' before the pattern */
	ଅଚଳ ଅକ୍ଷର *ss = string;
	ଅଚଳ ଅକ୍ଷର *s = string;
	bool first = true;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ = 0;

	ଯେତେବେଳେ (p < ep) {
		bool matches;
		ଯଦି (p < pattern || *p == '*') {
			ଅକ୍ଷର ch;

			/* skip any further asterisks; one is the same as many
			*/
			କର p++; ଯେତେବେଳେ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ଯଦି (p >= ep) ଭାଙ୍ଗ;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ଯଦି (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ଯଦି (!case_sens) ch = ascii_to_upper(ch);
				ଯେତେବେଳେ (s < es) {
					ଯଦି (case_sens) {
						ଯଦି (*s == ch) ଭାଙ୍ଗ;
					} ଅନ୍ୟଥା ଯଦି (ascii_to_upper(*s) == ch)
						ଭାଙ୍ଗ;
					s++;
				}
			}

			ଯଦି (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ଯଦି (ଶୀର୍ଷ < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NOF_ELEMENTS(context)) {
					context[ଶୀର୍ଷ].ss = ss;
					context[ଶୀର୍ଷ].s  = s + 1;
					context[ଶୀର୍ଷ].p  = p - 1;
					/* ptr to last asterisk */
					context[ଶୀର୍ଷ].first = first;
					ଶୀର୍ଷ++;
				}

				ଯଦି (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} ଅନ୍ୟଥା {
				matches = false;
			}

		} ଅନ୍ୟଥା ଯଦି (s < es) {
			ଅକ୍ଷର ch = *p;
			ଯଦି (ch == '#')
				matches = true;
			ଅନ୍ୟଥା {
				ଯଦି (case_sens)
					matches = (*s == ch);
				ଅନ୍ୟଥା
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			ଯଦି (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} ଅନ୍ୟଥା {
			matches = false;
		}

		ଯଦି (matches) {
			p++; s++;
		} ଅନ୍ୟଥା {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ଯଦି (--ଶୀର୍ଷ < 0)
				ଫେରନ୍ତୁ ଖାଲି;  /* no match, give up */

			ss = context[ଶୀର୍ଷ].ss;
			s  = context[ଶୀର୍ଷ].s;
			p  = context[ଶୀର୍ଷ].p;
			first = context[ଶୀର୍ଷ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ଫେରନ୍ତୁ ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(ସଂରଚନା textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     ସଂରଚନା box *start_box,
			     ସଂରଚନା box *end_box)
{
	ସଂରଚନା list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, ଆକାର(*entry));
	ଯଦି (entry == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = ଖାଲି;

	entry->next = ଖାଲି;
	entry->prev = context->found->prev;

	ଯଦି (context->found->prev == ଖାଲି) {
		context->found->next = entry;
	} ଅନ୍ୟଥା {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ସଂରଚନା textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	ସଂରଚନା list_entry *cur;

	ପାଇଁ (cur = textsearch->found->next; cur != ଖାଲି; cur = cur->next) {
		ଯଦି (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			ଫେରନ୍ତୁ true;
		}
	}

	ଫେରନ୍ତୁ false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ସଂରଚନା textsearch_context *textsearch)
{
	ପ୍ରତିଜ୍ଞା(textsearch != ଖାଲି);

	ଯଦି (textsearch->string != ଖାଲି) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		ମୁକ୍ତ(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     ଖାଲି);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ଖାଲି);

	free_matches(textsearch);
	ମୁକ୍ତ(textsearch);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ସଂରଚନା hlcache_handle *h,
		   ଶୂନ୍ୟ *context,
		   ସନ୍ଧାନ_flags_t flags,
		   ଅଚଳ ଅକ୍ଷର *string)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	nserror res;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (string != ଖାଲି &&
	    c->textsearch.string != ଖାଲି &&
	    c->textsearch.context != ଖାଲି &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} ଅନ୍ୟଥା ଯଦି (string != ଖାଲି) {
		/* New search */
		ମୁକ୍ତ(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ଯଦି (c->textsearch.string == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		ଯଦି (c->textsearch.context != ଖାଲି) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ଖାଲି;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} ଅନ୍ୟଥା {
		/* Clear search */
		content_textsearch__clear(c);

		ମୁକ୍ତ(c->textsearch.string);
		c->textsearch.string = ଖାଲି;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ସଂରଚନା hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଫେରନ୍ତୁ(content_textsearch__clear(c));
}
