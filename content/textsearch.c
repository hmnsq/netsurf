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

#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "utils/errors.h"
#समावेश "utils/utils.h"
#समावेश "utils/ascii.h"
#समावेश "netsurf/types.h"
#समावेश "desktop/selection.h"

#समावेश "content/content.h"
#समावेश "content/content_protected.h"
#समावेश "content/hlcache.h"
#समावेश "content/textsearch.h"

/**
 * search match
 */
काष्ठा list_entry {
	/**
	 * previous match
	 */
	काष्ठा list_entry *पूर्व;

	/**
	 * next match
	 */
	काष्ठा list_entry *अग्र;

	/**
	 * start position of match
	 */
	अचिन्हित start_idx;

	/**
	 * end of match
	 */
	अचिन्हित end_idx;

	/**
	 * content opaque start pointer
	 */
	काष्ठा box *start_box;

	/**
	 * content opaque end pointer
	 */
	काष्ठा box *end_box;

	/**
	 * content specific selection object
	 */
	काष्ठा selection *sel;
};

/**
 * The context for a free text search
 */
काष्ठा textsearch_context {

	/**
	 * content search was performed upon
	 */
	काष्ठा content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	व्योम *gui_p;

	/**
	 * List of matches
	 */
	काष्ठा list_entry *found;

	/**
	 * current selected match
	 */
	काष्ठा list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	अक्षर *string;
	bool पूर्व_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
अटल अंतरभूत व्योम
textsearch_broadcast(काष्ठा textsearch_context *textsearch,
		     पूर्णांक type,
		     bool state,
		     स्थिर अक्षर *string)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	संदेश_तथ्य.textsearch.type = type;
	संदेश_तथ्य.textsearch.ctx = textsearch->gui_p;
	संदेश_तथ्य.textsearch.state = state;
	संदेश_तथ्य.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_संदेश_TEXTSEARCH, &संदेश_तथ्य);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
अटल व्योम free_matches(काष्ठा textsearch_context *textsearch)
{
	काष्ठा list_entry *cur;
	काष्ठा list_entry *nxt;

	cur = textsearch->found->अग्र;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->पूर्व = शून्य;
	textsearch->found->अग्र = शून्य;

	क्रम (; cur; cur = nxt) {
		nxt = cur->अग्र;
		अगर (cur->sel) {
			selection_destroy(cur->sel);
		}
		मुक्त(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
अटल व्योम खोज_show_all(bool all, काष्ठा textsearch_context *context)
{
	काष्ठा list_entry *a;

	क्रम (a = context->found->अग्र; a; a = a->अग्र) {
		bool add = true;
		अगर (!all && a != context->current) {
			add = false;
			अगर (a->sel) {
				selection_destroy(a->sel);
				a->sel = शून्य;
			}
		}

		अगर (add && !a->sel) {

			a->sel = selection_create(context->c);
			अगर (a->sel != शून्य) {
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
अटल nserror
खोज_text(काष्ठा textsearch_context *context,
	    स्थिर अक्षर *string,
	    पूर्णांक string_len,
	    खोज_flags_t flags)
{
	काष्ठा rect bounds;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & खोज_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & खोज_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & खोज_FLAG_SHOWALL) != 0) ? true : false;

	अगर (context->c == शून्य) {
		वापस res;
	}

	/* check if we need to start a new search or continue an old one */
	अगर ((context->newsearch) ||
	    (context->पूर्व_case_sens != case_sensitive)) {

		अगर (context->string != शून्य) {
			मुक्त(context->string);
		}

		context->current = शून्य;
		free_matches(context);

		context->string = दो_स्मृति(string_len + 1);
		अगर (context->string != शून्य) {
			स_नकल(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, शून्य);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, शून्य);

		अगर (res != NSERROR_OK) {
			free_matches(context);
			वापस res;
		}

		context->पूर्व_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->अग्र;
		context->newsearch = false;

	} अन्यथा अगर (context->current != शून्य) {
		/* continued search in the direction specified */
		अगर (forwards) {
			अगर (context->current->अग्र)
				context->current = context->current->अग्र;
		} अन्यथा {
			अगर (context->current->पूर्व)
				context->current = context->current->पूर्व;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != शून्य),
			     शून्य);

	खोज_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != शून्य) &&
			      (context->current->पूर्व != शून्य)),
			     शून्य);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != शून्य) &&
			      (context->current->अग्र != शून्य)),
			     शून्य);


	अगर (context->current == शून्य) {
		/* no current match */
		वापस res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	अगर (res == NSERROR_OK) {
		संदेश_तथ्य.scroll.area = true;
		संदेश_तथ्य.scroll.x0 = bounds.x0;
		संदेश_तथ्य.scroll.y0 = bounds.y0;
		संदेश_तथ्य.scroll.x1 = bounds.x1;
		संदेश_तथ्य.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_संदेश_SCROLL, &संदेश_तथ्य);
	}

	वापस res;
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
अटल nserror
content_textsearch_step(काष्ठा textsearch_context *textsearch,
			खोज_flags_t flags,
			स्थिर अक्षर *string)
{
	पूर्णांक string_len;
	पूर्णांक i = 0;
	nserror res = NSERROR_OK;

	निश्चित(textsearch != शून्य);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = म_माप(string);
	क्रम (i = 0; i < string_len; i++) {
		अगर (string[i] != '#' && string[i] != '*')
			अवरोध;
	}

	अगर (i < string_len) {
		res = खोज_text(textsearch, string, string_len, flags);
	} अन्यथा {
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     शून्य);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     शून्य);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     शून्य);

		/* clear scroll */
		संदेश_तथ्य.scroll.area = false;
		संदेश_तथ्य.scroll.x0 = 0;
		संदेश_तथ्य.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_संदेश_SCROLL,
				  &संदेश_तथ्य);
	}

	वापस res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
अटल nserror content_textsearch__clear(काष्ठा content *c)
{
	मुक्त(c->textsearch.string);
	c->textsearch.string = शून्य;

	अगर (c->textsearch.context != शून्य) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = शून्य;
	}
	वापस NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
अटल nserror
content_textsearch_create(काष्ठा content *c,
			  व्योम *gui_तथ्य,
			  काष्ठा textsearch_context **textsearch_out)
{
	काष्ठा textsearch_context *context;
	काष्ठा list_entry *खोज_शीर्ष;
	content_type type;

	अगर ((c->handler->textsearch_find == शून्य) ||
	    (c->handler->textsearch_bounds == शून्य)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		वापस NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = दो_स्मृति(माप(काष्ठा textsearch_context));
	अगर (context == शून्य) {
		वापस NSERROR_NOMEM;
	}

	खोज_शीर्ष = दो_स्मृति(माप(काष्ठा list_entry));
	अगर (खोज_शीर्ष == शून्य) {
		मुक्त(context);
		वापस NSERROR_NOMEM;
	}

	खोज_शीर्ष->start_idx = 0;
	खोज_शीर्ष->end_idx = 0;
	खोज_शीर्ष->start_box = शून्य;
	खोज_शीर्ष->end_box = शून्य;
	खोज_शीर्ष->sel = शून्य;
	खोज_शीर्ष->पूर्व = शून्य;
	खोज_शीर्ष->अग्र = शून्य;

	context->found = खोज_शीर्ष;
	context->current = शून्य;
	context->string = शून्य;
	context->पूर्व_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_तथ्य;

	*textsearch_out = context;

	वापस NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
स्थिर अक्षर *
content_textsearch_find_pattern(स्थिर अक्षर *string,
				पूर्णांक s_len,
				स्थिर अक्षर *pattern,
				पूर्णांक p_len,
				bool case_sens,
				अचिन्हित पूर्णांक *m_len)
{
	काष्ठा { स्थिर अक्षर *ss, *s, *p; bool first; } context[16];
	स्थिर अक्षर *ep = pattern + p_len;
	स्थिर अक्षर *es = string  + s_len;
	स्थिर अक्षर *p = pattern - 1;  /* a virtual '*' before the pattern */
	स्थिर अक्षर *ss = string;
	स्थिर अक्षर *s = string;
	bool first = true;
	पूर्णांक शीर्ष = 0;

	जबतक (p < ep) {
		bool matches;
		अगर (p < pattern || *p == '*') {
			अक्षर ch;

			/* skip any further asterisks; one is the same as many
			*/
			करो p++; जबतक (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			अगर (p >= ep) अवरोध;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			अगर (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				अगर (!case_sens) ch = ascii_to_upper(ch);
				जबतक (s < es) {
					अगर (case_sens) {
						अगर (*s == ch) अवरोध;
					} अन्यथा अगर (ascii_to_upper(*s) == ch)
						अवरोध;
					s++;
				}
			}

			अगर (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				अगर (शीर्ष < (पूर्णांक)NOF_ELEMENTS(context)) {
					context[शीर्ष].ss = ss;
					context[शीर्ष].s  = s + 1;
					context[शीर्ष].p  = p - 1;
					/* ptr to last asterisk */
					context[शीर्ष].first = first;
					शीर्ष++;
				}

				अगर (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} अन्यथा {
				matches = false;
			}

		} अन्यथा अगर (s < es) {
			अक्षर ch = *p;
			अगर (ch == '#')
				matches = true;
			अन्यथा {
				अगर (case_sens)
					matches = (*s == ch);
				अन्यथा
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			अगर (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} अन्यथा {
			matches = false;
		}

		अगर (matches) {
			p++; s++;
		} अन्यथा {
			/* doesn't match,
			 * resume with stacked context if we have one */
			अगर (--शीर्ष < 0)
				वापस शून्य;  /* no match, give up */

			ss = context[शीर्ष].ss;
			s  = context[शीर्ष].s;
			p  = context[शीर्ष].p;
			first = context[शीर्ष].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	वापस ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(काष्ठा textsearch_context *context,
			     अचिन्हित start_idx,
			     अचिन्हित end_idx,
			     काष्ठा box *start_box,
			     काष्ठा box *end_box)
{
	काष्ठा list_entry *entry;

	/* found string in box => add to list */
	entry = सुस्मृति(1, माप(*entry));
	अगर (entry == शून्य) {
		वापस NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = शून्य;

	entry->अग्र = शून्य;
	entry->पूर्व = context->found->पूर्व;

	अगर (context->found->पूर्व == शून्य) {
		context->found->अग्र = entry;
	} अन्यथा {
		context->found->पूर्व->अग्र = entry;
	}

	context->found->पूर्व = entry;

	वापस NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(काष्ठा textsearch_context *textsearch,
				 अचिन्हित start_offset,
				 अचिन्हित end_offset,
				 अचिन्हित *start_idx,
				 अचिन्हित *end_idx)
{
	काष्ठा list_entry *cur;

	क्रम (cur = textsearch->found->अग्र; cur != शून्य; cur = cur->अग्र) {
		अगर (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			वापस true;
		}
	}

	वापस false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(काष्ठा textsearch_context *textsearch)
{
	निश्चित(textsearch != शून्य);

	अगर (textsearch->string != शून्य) {
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
			     शून्य);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     शून्य);

	free_matches(textsearch);
	मुक्त(textsearch);

	वापस NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(काष्ठा hlcache_handle *h,
		   व्योम *context,
		   खोज_flags_t flags,
		   स्थिर अक्षर *string)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	nserror res;

	निश्चित(c != शून्य);

	अगर (string != शून्य &&
	    c->textsearch.string != शून्य &&
	    c->textsearch.context != शून्य &&
	    म_भेद(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} अन्यथा अगर (string != शून्य) {
		/* New search */
		मुक्त(c->textsearch.string);
		c->textsearch.string = strdup(string);
		अगर (c->textsearch.string == शून्य) {
			वापस NSERROR_NOMEM;
		}

		अगर (c->textsearch.context != शून्य) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = शून्य;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		अगर (res != NSERROR_OK) {
			वापस res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} अन्यथा {
		/* Clear search */
		content_textsearch__clear(c);

		मुक्त(c->textsearch.string);
		c->textsearch.string = शून्य;
	}

	वापस NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(काष्ठा hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	वापस(content_textsearch__clear(c));
}
