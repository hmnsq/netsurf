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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/ascii.h"
#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "desktop/selection.h"

#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "content/textsearch.h"

/**
 * search match
 */
সংরচনা list_entry {
	/**
	 * previous match
	 */
	সংরচনা list_entry *prev;

	/**
	 * next match
	 */
	সংরচনা list_entry *next;

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
	সংরচনা box *start_box;

	/**
	 * content opaque end pointer
	 */
	সংরচনা box *end_box;

	/**
	 * content specific selection object
	 */
	সংরচনা selection *sel;
};

/**
 * The context for a free text search
 */
সংরচনা textsearch_context {

	/**
	 * content search was performed upon
	 */
	সংরচনা content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	শূন্য *gui_p;

	/**
	 * List of matches
	 */
	সংরচনা list_entry *found;

	/**
	 * current selected match
	 */
	সংরচনা list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	বর্ণ *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
স্থির অন্তঃস্থ শূন্য
textsearch_broadcast(সংরচনা textsearch_context *textsearch,
		     পূর্ণাঙ্ক type,
		     bool state,
		     ধ্রুব বর্ণ *string)
{
	সঙ্ঘ content_বার্তা_data বার্তা_data;
	বার্তা_data.textsearch.type = type;
	বার্তা_data.textsearch.ctx = textsearch->gui_p;
	বার্তা_data.textsearch.state = state;
	বার্তা_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_বার্তা_TEXTSEARCH, &বার্তা_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
স্থির শূন্য free_matches(সংরচনা textsearch_context *textsearch)
{
	সংরচনা list_entry *cur;
	সংরচনা list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = রিক্ত;
	textsearch->found->next = রিক্ত;

	প্রতি (; cur; cur = nxt) {
		nxt = cur->next;
		যদি (cur->sel) {
			selection_destroy(cur->sel);
		}
		মুক্ত(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
স্থির শূন্য অনুসন্ধান_show_all(bool all, সংরচনা textsearch_context *context)
{
	সংরচনা list_entry *a;

	প্রতি (a = context->found->next; a; a = a->next) {
		bool add = true;
		যদি (!all && a != context->current) {
			add = false;
			যদি (a->sel) {
				selection_destroy(a->sel);
				a->sel = রিক্ত;
			}
		}

		যদি (add && !a->sel) {

			a->sel = selection_create(context->c);
			যদি (a->sel != রিক্ত) {
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
স্থির nserror
অনুসন্ধান_text(সংরচনা textsearch_context *context,
	    ধ্রুব বর্ণ *string,
	    পূর্ণাঙ্ক string_len,
	    অনুসন্ধান_flags_t flags)
{
	সংরচনা rect bounds;
	সঙ্ঘ content_বার্তা_data বার্তা_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & অনুসন্ধান_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & অনুসন্ধান_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & অনুসন্ধান_FLAG_SHOWALL) != 0) ? true : false;

	যদি (context->c == রিক্ত) {
		প্রত্যাগম res;
	}

	/* check if we need to start a new search or continue an old one */
	যদি ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		যদি (context->string != রিক্ত) {
			মুক্ত(context->string);
		}

		context->current = রিক্ত;
		free_matches(context);

		context->string = malloc(string_len + 1);
		যদি (context->string != রিক্ত) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, রিক্ত);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, রিক্ত);

		যদি (res != NSERROR_OK) {
			free_matches(context);
			প্রত্যাগম res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} অন্যথা যদি (context->current != রিক্ত) {
		/* continued search in the direction specified */
		যদি (forwards) {
			যদি (context->current->next)
				context->current = context->current->next;
		} অন্যথা {
			যদি (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != রিক্ত),
			     রিক্ত);

	অনুসন্ধান_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != রিক্ত) &&
			      (context->current->prev != রিক্ত)),
			     রিক্ত);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != রিক্ত) &&
			      (context->current->next != রিক্ত)),
			     রিক্ত);


	যদি (context->current == রিক্ত) {
		/* no current match */
		প্রত্যাগম res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	যদি (res == NSERROR_OK) {
		বার্তা_data.scroll.area = true;
		বার্তা_data.scroll.x0 = bounds.x0;
		বার্তা_data.scroll.y0 = bounds.y0;
		বার্তা_data.scroll.x1 = bounds.x1;
		বার্তা_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_বার্তা_SCROLL, &বার্তা_data);
	}

	প্রত্যাগম res;
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
স্থির nserror
content_textsearch_step(সংরচনা textsearch_context *textsearch,
			অনুসন্ধান_flags_t flags,
			ধ্রুব বর্ণ *string)
{
	পূর্ণাঙ্ক string_len;
	পূর্ণাঙ্ক i = 0;
	nserror res = NSERROR_OK;

	প্রতিজ্ঞা(textsearch != রিক্ত);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	প্রতি (i = 0; i < string_len; i++) {
		যদি (string[i] != '#' && string[i] != '*')
			ভঙ্গ;
	}

	যদি (i < string_len) {
		res = অনুসন্ধান_text(textsearch, string, string_len, flags);
	} অন্যথা {
		সঙ্ঘ content_বার্তা_data বার্তা_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     রিক্ত);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     রিক্ত);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     রিক্ত);

		/* clear scroll */
		বার্তা_data.scroll.area = false;
		বার্তা_data.scroll.x0 = 0;
		বার্তা_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_বার্তা_SCROLL,
				  &বার্তা_data);
	}

	প্রত্যাগম res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
স্থির nserror content_textsearch__clear(সংরচনা content *c)
{
	মুক্ত(c->textsearch.string);
	c->textsearch.string = রিক্ত;

	যদি (c->textsearch.context != রিক্ত) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = রিক্ত;
	}
	প্রত্যাগম NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
স্থির nserror
content_textsearch_create(সংরচনা content *c,
			  শূন্য *gui_data,
			  সংরচনা textsearch_context **textsearch_out)
{
	সংরচনা textsearch_context *context;
	সংরচনা list_entry *অনুসন্ধান_head;
	content_type type;

	যদি ((c->handler->textsearch_find == রিক্ত) ||
	    (c->handler->textsearch_bounds == রিক্ত)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		প্রত্যাগম NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(পরিমাণ(সংরচনা textsearch_context));
	যদি (context == রিক্ত) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	অনুসন্ধান_head = malloc(পরিমাণ(সংরচনা list_entry));
	যদি (অনুসন্ধান_head == রিক্ত) {
		মুক্ত(context);
		প্রত্যাগম NSERROR_NOMEM;
	}

	অনুসন্ধান_head->start_idx = 0;
	অনুসন্ধান_head->end_idx = 0;
	অনুসন্ধান_head->start_box = রিক্ত;
	অনুসন্ধান_head->end_box = রিক্ত;
	অনুসন্ধান_head->sel = রিক্ত;
	অনুসন্ধান_head->prev = রিক্ত;
	অনুসন্ধান_head->next = রিক্ত;

	context->found = অনুসন্ধান_head;
	context->current = রিক্ত;
	context->string = রিক্ত;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ধ্রুব বর্ণ *
content_textsearch_find_pattern(ধ্রুব বর্ণ *string,
				পূর্ণাঙ্ক s_len,
				ধ্রুব বর্ণ *pattern,
				পূর্ণাঙ্ক p_len,
				bool case_sens,
				unsigned পূর্ণাঙ্ক *m_len)
{
	সংরচনা { ধ্রুব বর্ণ *ss, *s, *p; bool first; } context[16];
	ধ্রুব বর্ণ *ep = pattern + p_len;
	ধ্রুব বর্ণ *es = string  + s_len;
	ধ্রুব বর্ণ *p = pattern - 1;  /* a virtual '*' before the pattern */
	ধ্রুব বর্ণ *ss = string;
	ধ্রুব বর্ণ *s = string;
	bool first = true;
	পূর্ণাঙ্ক শীর্ষ = 0;

	যাবত্ (p < ep) {
		bool matches;
		যদি (p < pattern || *p == '*') {
			বর্ণ ch;

			/* skip any further asterisks; one is the same as many
			*/
			কুরু p++; যাবত্ (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			যদি (p >= ep) ভঙ্গ;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			যদি (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				যদি (!case_sens) ch = ascii_to_upper(ch);
				যাবত্ (s < es) {
					যদি (case_sens) {
						যদি (*s == ch) ভঙ্গ;
					} অন্যথা যদি (ascii_to_upper(*s) == ch)
						ভঙ্গ;
					s++;
				}
			}

			যদি (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				যদি (শীর্ষ < (পূর্ণাঙ্ক)NOF_ELEMENTS(context)) {
					context[শীর্ষ].ss = ss;
					context[শীর্ষ].s  = s + 1;
					context[শীর্ষ].p  = p - 1;
					/* ptr to last asterisk */
					context[শীর্ষ].first = first;
					শীর্ষ++;
				}

				যদি (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} অন্যথা {
				matches = false;
			}

		} অন্যথা যদি (s < es) {
			বর্ণ ch = *p;
			যদি (ch == '#')
				matches = true;
			অন্যথা {
				যদি (case_sens)
					matches = (*s == ch);
				অন্যথা
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			যদি (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} অন্যথা {
			matches = false;
		}

		যদি (matches) {
			p++; s++;
		} অন্যথা {
			/* doesn't match,
			 * resume with stacked context if we have one */
			যদি (--শীর্ষ < 0)
				প্রত্যাগম রিক্ত;  /* no match, give up */

			ss = context[শীর্ষ].ss;
			s  = context[শীর্ষ].s;
			p  = context[শীর্ষ].p;
			first = context[শীর্ষ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	প্রত্যাগম ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(সংরচনা textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     সংরচনা box *start_box,
			     সংরচনা box *end_box)
{
	সংরচনা list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, পরিমাণ(*entry));
	যদি (entry == রিক্ত) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = রিক্ত;

	entry->next = রিক্ত;
	entry->prev = context->found->prev;

	যদি (context->found->prev == রিক্ত) {
		context->found->next = entry;
	} অন্যথা {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(সংরচনা textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	সংরচনা list_entry *cur;

	প্রতি (cur = textsearch->found->next; cur != রিক্ত; cur = cur->next) {
		যদি (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			প্রত্যাগম true;
		}
	}

	প্রত্যাগম false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(সংরচনা textsearch_context *textsearch)
{
	প্রতিজ্ঞা(textsearch != রিক্ত);

	যদি (textsearch->string != রিক্ত) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		মুক্ত(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     রিক্ত);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     রিক্ত);

	free_matches(textsearch);
	মুক্ত(textsearch);

	প্রত্যাগম NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(সংরচনা hlcache_handle *h,
		   শূন্য *context,
		   অনুসন্ধান_flags_t flags,
		   ধ্রুব বর্ণ *string)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	nserror res;

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (string != রিক্ত &&
	    c->textsearch.string != রিক্ত &&
	    c->textsearch.context != রিক্ত &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} অন্যথা যদি (string != রিক্ত) {
		/* New search */
		মুক্ত(c->textsearch.string);
		c->textsearch.string = strdup(string);
		যদি (c->textsearch.string == রিক্ত) {
			প্রত্যাগম NSERROR_NOMEM;
		}

		যদি (c->textsearch.context != রিক্ত) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = রিক্ত;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		যদি (res != NSERROR_OK) {
			প্রত্যাগম res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} অন্যথা {
		/* Clear search */
		content_textsearch__clear(c);

		মুক্ত(c->textsearch.string);
		c->textsearch.string = রিক্ত;
	}

	প্রত্যাগম NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(সংরচনা hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	প্রত্যাগম(content_textsearch__clear(c));
}
