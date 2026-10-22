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
	सङ्घ content_संदेश_data संदेश_data;
	संदेश_data.textsearch.type = type;
	संदेश_data.textsearch.ctx = textsearch->gui_p;
	संदेश_data.textsearch.state = state;
	संदेश_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_संदेश_TEXTSEARCH, &संदेश_data);
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

	प्रति (; cur; cur = nxt) {
		nxt = cur->next;
		यदि (cur->sel) {
			selection_destroy(cur->sel);
		}
		मुक्त(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
स्थिर शून्य शोध_show_all(bool all, संरचना textsearch_context *context)
{
	संरचना list_entry *a;

	प्रति (a = context->found->next; a; a = a->next) {
		bool add = true;
		यदि (!all && a != context->current) {
			add = false;
			यदि (a->sel) {
				selection_destroy(a->sel);
				a->sel = रिक्त;
			}
		}

		यदि (add && !a->sel) {

			a->sel = selection_create(context->c);
			यदि (a->sel != रिक्त) {
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
शोध_text(संरचना textsearch_context *context,
	    ध्रुव वर्ण *string,
	    पूर्णाङ्क string_len,
	    शोध_flags_t flags)
{
	संरचना rect bounds;
	सङ्घ content_संदेश_data संदेश_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & शोध_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & शोध_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & शोध_FLAG_SHOWALL) != 0) ? true : false;

	यदि (context->c == रिक्त) {
		प्रत्यागम res;
	}

	/* check if we need to start a new search or continue an old one */
	यदि ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		यदि (context->string != रिक्त) {
			मुक्त(context->string);
		}

		context->current = रिक्त;
		free_matches(context);

		context->string = malloc(string_len + 1);
		यदि (context->string != रिक्त) {
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

		यदि (res != NSERROR_OK) {
			free_matches(context);
			प्रत्यागम res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} अन्यथा यदि (context->current != रिक्त) {
		/* continued search in the direction specified */
		यदि (forwards) {
			यदि (context->current->next)
				context->current = context->current->next;
		} अन्यथा {
			यदि (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != रिक्त),
			     रिक्त);

	शोध_show_all(showall, context);

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


	यदि (context->current == रिक्त) {
		/* no current match */
		प्रत्यागम res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	यदि (res == NSERROR_OK) {
		संदेश_data.scroll.area = true;
		संदेश_data.scroll.x0 = bounds.x0;
		संदेश_data.scroll.y0 = bounds.y0;
		संदेश_data.scroll.x1 = bounds.x1;
		संदेश_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_संदेश_SCROLL, &संदेश_data);
	}

	प्रत्यागम res;
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
			शोध_flags_t flags,
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
	प्रति (i = 0; i < string_len; i++) {
		यदि (string[i] != '#' && string[i] != '*')
			भङ्ग;
	}

	यदि (i < string_len) {
		res = शोध_text(textsearch, string, string_len, flags);
	} अन्यथा {
		सङ्घ content_संदेश_data संदेश_data;

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
		संदेश_data.scroll.area = false;
		संदेश_data.scroll.x0 = 0;
		संदेश_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_संदेश_SCROLL,
				  &संदेश_data);
	}

	प्रत्यागम res;
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

	यदि (c->textsearch.context != रिक्त) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = रिक्त;
	}
	प्रत्यागम NSERROR_OK;
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
	संरचना list_entry *शोध_head;
	content_type type;

	यदि ((c->handler->textsearch_find == रिक्त) ||
	    (c->handler->textsearch_bounds == रिक्त)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		प्रत्यागम NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(परिमाण(संरचना textsearch_context));
	यदि (context == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	शोध_head = malloc(परिमाण(संरचना list_entry));
	यदि (शोध_head == रिक्त) {
		मुक्त(context);
		प्रत्यागम NSERROR_NOMEM;
	}

	शोध_head->start_idx = 0;
	शोध_head->end_idx = 0;
	शोध_head->start_box = रिक्त;
	शोध_head->end_box = रिक्त;
	शोध_head->sel = रिक्त;
	शोध_head->prev = रिक्त;
	शोध_head->next = रिक्त;

	context->found = शोध_head;
	context->current = रिक्त;
	context->string = रिक्त;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	प्रत्यागम NSERROR_OK;
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
	पूर्णाङ्क शीर्ष = 0;

	यावत् (p < ep) {
		bool matches;
		यदि (p < pattern || *p == '*') {
			वर्ण ch;

			/* skip any further asterisks; one is the same as many
			*/
			कुरु p++; यावत् (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			यदि (p >= ep) भङ्ग;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			यदि (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				यदि (!case_sens) ch = ascii_to_upper(ch);
				यावत् (s < es) {
					यदि (case_sens) {
						यदि (*s == ch) भङ्ग;
					} अन्यथा यदि (ascii_to_upper(*s) == ch)
						भङ्ग;
					s++;
				}
			}

			यदि (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				यदि (शीर्ष < (पूर्णाङ्क)NOF_ELEMENTS(context)) {
					context[शीर्ष].ss = ss;
					context[शीर्ष].s  = s + 1;
					context[शीर्ष].p  = p - 1;
					/* ptr to last asterisk */
					context[शीर्ष].first = first;
					शीर्ष++;
				}

				यदि (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} अन्यथा {
				matches = false;
			}

		} अन्यथा यदि (s < es) {
			वर्ण ch = *p;
			यदि (ch == '#')
				matches = true;
			अन्यथा {
				यदि (case_sens)
					matches = (*s == ch);
				अन्यथा
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			यदि (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} अन्यथा {
			matches = false;
		}

		यदि (matches) {
			p++; s++;
		} अन्यथा {
			/* doesn't match,
			 * resume with stacked context if we have one */
			यदि (--शीर्ष < 0)
				प्रत्यागम रिक्त;  /* no match, give up */

			ss = context[शीर्ष].ss;
			s  = context[शीर्ष].s;
			p  = context[शीर्ष].p;
			first = context[शीर्ष].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	प्रत्यागम ss;
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
	यदि (entry == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = रिक्त;

	entry->next = रिक्त;
	entry->prev = context->found->prev;

	यदि (context->found->prev == रिक्त) {
		context->found->next = entry;
	} अन्यथा {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	प्रत्यागम NSERROR_OK;
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

	प्रति (cur = textsearch->found->next; cur != रिक्त; cur = cur->next) {
		यदि (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			प्रत्यागम true;
		}
	}

	प्रत्यागम false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(संरचना textsearch_context *textsearch)
{
	प्रतिज्ञा(textsearch != रिक्त);

	यदि (textsearch->string != रिक्त) {
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

	प्रत्यागम NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(संरचना hlcache_handle *h,
		   शून्य *context,
		   शोध_flags_t flags,
		   ध्रुव वर्ण *string)
{
	संरचना content *c = hlcache_handle_get_content(h);
	nserror res;

	प्रतिज्ञा(c != रिक्त);

	यदि (string != रिक्त &&
	    c->textsearch.string != रिक्त &&
	    c->textsearch.context != रिक्त &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} अन्यथा यदि (string != रिक्त) {
		/* New search */
		मुक्त(c->textsearch.string);
		c->textsearch.string = strdup(string);
		यदि (c->textsearch.string == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}

		यदि (c->textsearch.context != रिक्त) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = रिक्त;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} अन्यथा {
		/* Clear search */
		content_textsearch__clear(c);

		मुक्त(c->textsearch.string);
		c->textsearch.string = रिक्त;
	}

	प्रत्यागम NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(संरचना hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	प्रत्यागम(content_textsearch__clear(c));
}
