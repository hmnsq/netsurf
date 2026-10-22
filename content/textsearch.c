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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "desktop/selection.h"

#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "content/textsearch.h"

/**
 * search match
 */
संरचना list_entry {
	/**
	 * previous match
	 */
	संरचना list_entry *prev;

	/**
	 * next match
	 */
	संरचना list_entry *next;

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
	संरचना box *start_box;

	/**
	 * content opaque end pointer
	 */
	संरचना box *end_box;

	/**
	 * content specific selection object
	 */
	संरचना selection *sel;
};

/**
 * The context for a free text search
 */
संरचना textsearch_context {

	/**
	 * content search was performed upon
	 */
	संरचना content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	शून्य *gui_p;

	/**
	 * List of matches
	 */
	संरचना list_entry *found;

	/**
	 * current selected match
	 */
	संरचना list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	वर्ण *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
स्थिर अन्तःस्थ शून्य
textsearch_broadcast(संरचना textsearch_context *textsearch,
		     पूर्णाङ्क type,
		     bool state,
		     ध्रुव वर्ण *string)
{
	सङ्घ content_सन्देशः_data सन्देशः_data;
	सन्देशः_data.textsearch.type = type;
	सन्देशः_data.textsearch.ctx = textsearch->gui_p;
	सन्देशः_data.textsearch.state = state;
	सन्देशः_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_सन्देशः_TEXTSEARCH, &सन्देशः_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
स्थिर शून्य free_matches(संरचना textsearch_context *textsearch)
{
	संरचना list_entry *cur;
	संरचना list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = रिक्त;
	textsearch->found->next = रिक्त;

	अत्थाय (; cur; cur = nxt) {
		nxt = cur->next;
		सचे (cur->sel) {
			selection_destroy(cur->sel);
		}
		मुक्त(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
स्थिर शून्य अन्वेषणम्_show_all(bool all, संरचना textsearch_context *context)
{
	संरचना list_entry *a;

	अत्थाय (a = context->found->next; a; a = a->next) {
		bool add = true;
		सचे (!all && a != context->current) {
			add = false;
			सचे (a->sel) {
				selection_destroy(a->sel);
				a->sel = रिक्त;
			}
		}

		सचे (add && !a->sel) {

			a->sel = selection_create(context->c);
			सचे (a->sel != रिक्त) {
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
स्थिर nserror
अन्वेषणम्_text(संरचना textsearch_context *context,
	    ध्रुव वर्ण *string,
	    पूर्णाङ्क string_len,
	    अन्वेषणम्_flags_t flags)
{
	संरचना rect bounds;
	सङ्घ content_सन्देशः_data सन्देशः_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & अन्वेषणम्_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & अन्वेषणम्_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & अन्वेषणम्_FLAG_SHOWALL) != 0) ? true : false;

	सचे (context->c == रिक्त) {
		पटिनिवत्त res;
	}

	/* check if we need to start a new search or continue an old one */
	सचे ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		सचे (context->string != रिक्त) {
			मुक्त(context->string);
		}

		context->current = रिक्त;
		free_matches(context);

		context->string = malloc(string_len + 1);
		सचे (context->string != रिक्त) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, रिक्त);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, रिक्त);

		सचे (res != NSERROR_OK) {
			free_matches(context);
			पटिनिवत्त res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} अञ्ञथा सचे (context->current != रिक्त) {
		/* continued search in the direction specified */
		सचे (forwards) {
			सचे (context->current->next)
				context->current = context->current->next;
		} अञ्ञथा {
			सचे (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != रिक्त),
			     रिक्त);

	अन्वेषणम्_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != रिक्त) &&
			      (context->current->prev != रिक्त)),
			     रिक्त);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != रिक्त) &&
			      (context->current->next != रिक्त)),
			     रिक्त);


	सचे (context->current == रिक्त) {
		/* no current match */
		पटिनिवत्त res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	सचे (res == NSERROR_OK) {
		सन्देशः_data.scroll.area = true;
		सन्देशः_data.scroll.x0 = bounds.x0;
		सन्देशः_data.scroll.y0 = bounds.y0;
		सन्देशः_data.scroll.x1 = bounds.x1;
		सन्देशः_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_सन्देशः_SCROLL, &सन्देशः_data);
	}

	पटिनिवत्त res;
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
स्थिर nserror
content_textsearch_step(संरचना textsearch_context *textsearch,
			अन्वेषणम्_flags_t flags,
			ध्रुव वर्ण *string)
{
	पूर्णाङ्क string_len;
	पूर्णाङ्क i = 0;
	nserror res = NSERROR_OK;

	प्रतिज्ञा(textsearch != रिक्त);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	अत्थाय (i = 0; i < string_len; i++) {
		सचे (string[i] != '#' && string[i] != '*')
			भङ्ग;
	}

	सचे (i < string_len) {
		res = अन्वेषणम्_text(textsearch, string, string_len, flags);
	} अञ्ञथा {
		सङ्घ content_सन्देशः_data सन्देशः_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     रिक्त);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     रिक्त);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     रिक्त);

		/* clear scroll */
		सन्देशः_data.scroll.area = false;
		सन्देशः_data.scroll.x0 = 0;
		सन्देशः_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_सन्देशः_SCROLL,
				  &सन्देशः_data);
	}

	पटिनिवत्त res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
स्थिर nserror content_textsearch__clear(संरचना content *c)
{
	मुक्त(c->textsearch.string);
	c->textsearch.string = रिक्त;

	सचे (c->textsearch.context != रिक्त) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = रिक्त;
	}
	पटिनिवत्त NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
स्थिर nserror
content_textsearch_create(संरचना content *c,
			  शून्य *gui_data,
			  संरचना textsearch_context **textsearch_out)
{
	संरचना textsearch_context *context;
	संरचना list_entry *अन्वेषणम्_head;
	content_type type;

	सचे ((c->handler->textsearch_find == रिक्त) ||
	    (c->handler->textsearch_bounds == रिक्त)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		पटिनिवत्त NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(परिमाण(संरचना textsearch_context));
	सचे (context == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	अन्वेषणम्_head = malloc(परिमाण(संरचना list_entry));
	सचे (अन्वेषणम्_head == रिक्त) {
		मुक्त(context);
		पटिनिवत्त NSERROR_NOMEM;
	}

	अन्वेषणम्_head->start_idx = 0;
	अन्वेषणम्_head->end_idx = 0;
	अन्वेषणम्_head->start_box = रिक्त;
	अन्वेषणम्_head->end_box = रिक्त;
	अन्वेषणम्_head->sel = रिक्त;
	अन्वेषणम्_head->prev = रिक्त;
	अन्वेषणम्_head->next = रिक्त;

	context->found = अन्वेषणम्_head;
	context->current = रिक्त;
	context->string = रिक्त;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	पटिनिवत्त NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ध्रुव वर्ण *
content_textsearch_find_pattern(ध्रुव वर्ण *string,
				पूर्णाङ्क s_len,
				ध्रुव वर्ण *pattern,
				पूर्णाङ्क p_len,
				bool case_sens,
				unsigned पूर्णाङ्क *m_len)
{
	संरचना { ध्रुव वर्ण *ss, *s, *p; bool first; } context[16];
	ध्रुव वर्ण *ep = pattern + p_len;
	ध्रुव वर्ण *es = string  + s_len;
	ध्रुव वर्ण *p = pattern - 1;  /* a virtual '*' before the pattern */
	ध्रुव वर्ण *ss = string;
	ध्रुव वर्ण *s = string;
	bool first = true;
	पूर्णाङ्क शिखरम् = 0;

	याव (p < ep) {
		bool matches;
		सचे (p < pattern || *p == '*') {
			वर्ण ch;

			/* skip any further asterisks; one is the same as many
			*/
			करोहि p++; याव (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			सचे (p >= ep) भङ्ग;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			सचे (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				सचे (!case_sens) ch = ascii_to_upper(ch);
				याव (s < es) {
					सचे (case_sens) {
						सचे (*s == ch) भङ्ग;
					} अञ्ञथा सचे (ascii_to_upper(*s) == ch)
						भङ्ग;
					s++;
				}
			}

			सचे (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				सचे (शिखरम् < (पूर्णाङ्क)NOF_ELEMENTS(context)) {
					context[शिखरम्].ss = ss;
					context[शिखरम्].s  = s + 1;
					context[शिखरम्].p  = p - 1;
					/* ptr to last asterisk */
					context[शिखरम्].first = first;
					शिखरम्++;
				}

				सचे (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} अञ्ञथा {
				matches = false;
			}

		} अञ्ञथा सचे (s < es) {
			वर्ण ch = *p;
			सचे (ch == '#')
				matches = true;
			अञ्ञथा {
				सचे (case_sens)
					matches = (*s == ch);
				अञ्ञथा
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			सचे (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} अञ्ञथा {
			matches = false;
		}

		सचे (matches) {
			p++; s++;
		} अञ्ञथा {
			/* doesn't match,
			 * resume with stacked context if we have one */
			सचे (--शिखरम् < 0)
				पटिनिवत्त रिक्त;  /* no match, give up */

			ss = context[शिखरम्].ss;
			s  = context[शिखरम्].s;
			p  = context[शिखरम्].p;
			first = context[शिखरम्].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	पटिनिवत्त ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(संरचना textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     संरचना box *start_box,
			     संरचना box *end_box)
{
	संरचना list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, परिमाण(*entry));
	सचे (entry == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = रिक्त;

	entry->next = रिक्त;
	entry->prev = context->found->prev;

	सचे (context->found->prev == रिक्त) {
		context->found->next = entry;
	} अञ्ञथा {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	पटिनिवत्त NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(संरचना textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	संरचना list_entry *cur;

	अत्थाय (cur = textsearch->found->next; cur != रिक्त; cur = cur->next) {
		सचे (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			पटिनिवत्त true;
		}
	}

	पटिनिवत्त false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(संरचना textsearch_context *textsearch)
{
	प्रतिज्ञा(textsearch != रिक्त);

	सचे (textsearch->string != रिक्त) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		मुक्त(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     रिक्त);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     रिक्त);

	free_matches(textsearch);
	मुक्त(textsearch);

	पटिनिवत्त NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(संरचना hlcache_handle *h,
		   शून्य *context,
		   अन्वेषणम्_flags_t flags,
		   ध्रुव वर्ण *string)
{
	संरचना content *c = hlcache_handle_get_content(h);
	nserror res;

	प्रतिज्ञा(c != रिक्त);

	सचे (string != रिक्त &&
	    c->textsearch.string != रिक्त &&
	    c->textsearch.context != रिक्त &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} अञ्ञथा सचे (string != रिक्त) {
		/* New search */
		मुक्त(c->textsearch.string);
		c->textsearch.string = strdup(string);
		सचे (c->textsearch.string == रिक्त) {
			पटिनिवत्त NSERROR_NOMEM;
		}

		सचे (c->textsearch.context != रिक्त) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = रिक्त;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} अञ्ञथा {
		/* Clear search */
		content_textsearch__clear(c);

		मुक्त(c->textsearch.string);
		c->textsearch.string = रिक्त;
	}

	पटिनिवत्त NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(संरचना hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	पटिनिवत्त(content_textsearch__clear(c));
}
