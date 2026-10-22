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
संरचना list_प्रविष्टिः {
	/**
	 * previous match
	 */
	संरचना list_प्रविष्टिः *पूर्व;

	/**
	 * next match
	 */
	संरचना list_प्रविष्टिः *अग्र;

	/**
	 * start position of match
	 */
	अचिह्नित start_idx;

	/**
	 * end of match
	 */
	अचिह्नित अन्तः_idx;

	/**
	 * content opaque start pointer
	 */
	संरचना box *start_box;

	/**
	 * content opaque end pointer
	 */
	संरचना box *अन्तः_box;

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
	संरचना list_प्रविष्टिः *प्राप्तम्;

	/**
	 * current selected match
	 */
	संरचना list_प्रविष्टिः *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	वर्ण *string;
	bool पूर्व_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
स्थिर अन्तःस्थ शून्य
textsearch_प्रसारणम्(संरचना textsearch_context *textsearch,
		     पूर्णाङ्क type,
		     bool state,
		     ध्रुव वर्ण *string)
{
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	सन्देशः_दत्त.textsearch.type = type;
	सन्देशः_दत्त.textsearch.ctx = textsearch->gui_p;
	सन्देशः_दत्त.textsearch.state = state;
	सन्देशः_दत्त.textsearch.string = string;
	content_प्रसारणम्(textsearch->c, CONTENT_सन्देशः_TEXTSEARCH, &सन्देशः_दत्त);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
स्थिर शून्य free_matches(संरचना textsearch_context *textsearch)
{
	संरचना list_प्रविष्टिः *cur;
	संरचना list_प्रविष्टिः *nxt;

	cur = textsearch->प्राप्तम्->अग्र;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->प्राप्तम्->पूर्व = रिक्त;
	textsearch->प्राप्तम्->अग्र = रिक्त;

	प्रति (; cur; cur = nxt) {
		nxt = cur->अग्र;
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
स्थिर शून्य अन्वेषणम्_show_all(bool all, संरचना textsearch_context *context)
{
	संरचना list_प्रविष्टिः *a;

	प्रति (a = context->प्राप्तम्->अग्र; a; a = a->अग्र) {
		bool सङ्कलय = true;
		यदि (!all && a != context->current) {
			सङ्कलय = false;
			यदि (a->sel) {
				selection_destroy(a->sel);
				a->sel = रिक्त;
			}
		}

		यदि (सङ्कलय && !a->sel) {

			a->sel = selection_create(context->c);
			यदि (a->sel != रिक्त) {
				selection_init(a->sel);
				selection_समुच्चयः_position(a->sel,
						       a->start_idx,
						       a->अन्तः_idx);
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
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & अन्वेषणम्_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & अन्वेषणम्_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & अन्वेषणम्_FLAG_SHOWALL) != 0) ? true : false;

	यदि (context->c == रिक्त) {
		प्रत्यागम res;
	}

	/* check if we need to start a new search or continue an old one */
	यदि ((context->newsearch) ||
	    (context->पूर्व_case_sens != case_sensitive)) {

		यदि (context->string != रिक्त) {
			मुक्त(context->string);
		}

		context->current = रिक्त;
		free_matches(context);

		context->string = स्मृतिं_देहि(string_len + 1);
		यदि (context->string != रिक्त) {
			स्मृतिं_प्रतिलिख(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_प्रसारणम्(context, CONTENT_TEXTSEARCH_FIND, true, रिक्त);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_प्रसारणम्(context, CONTENT_TEXTSEARCH_FIND, false, रिक्त);

		यदि (res != NSERROR_OK) {
			free_matches(context);
			प्रत्यागम res;
		}

		context->पूर्व_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->प्राप्तम्->अग्र;
		context->newsearch = false;

	} अन्यथा यदि (context->current != रिक्त) {
		/* continued search in the direction specified */
		यदि (forwards) {
			यदि (context->current->अग्र)
				context->current = context->current->अग्र;
		} अन्यथा {
			यदि (context->current->पूर्व)
				context->current = context->current->पूर्व;
		}
	}

	/* update match state */
	textsearch_प्रसारणम्(context,
			     CONTENT_TEXTSEARCH_मेलनम्,
			     (context->current != रिक्त),
			     रिक्त);

	अन्वेषणम्_show_all(showall, context);

	/* update back state */
	textsearch_प्रसारणम्(context,
			     CONTENT_TEXTSEARCH_पृष्ठभागः,
			     ((context->current != रिक्त) &&
			      (context->current->पूर्व != रिक्त)),
			     रिक्त);

	/* update forward state */
	textsearch_प्रसारणम्(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != रिक्त) &&
			      (context->current->अग्र != रिक्त)),
			     रिक्त);


	यदि (context->current == रिक्त) {
		/* no current match */
		प्रत्यागम res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->अन्तः_idx,
					context->current->start_box,
					context->current->अन्तः_box,
					&bounds);
	यदि (res == NSERROR_OK) {
		सन्देशः_दत्त.scroll.area = true;
		सन्देशः_दत्त.scroll.x0 = bounds.x0;
		सन्देशः_दत्त.scroll.y0 = bounds.y0;
		सन्देशः_दत्त.scroll.x1 = bounds.x1;
		सन्देशः_दत्त.scroll.y1 = bounds.y1;
		content_प्रसारणम्(context->c, CONTENT_सन्देशः_SCROLL, &सन्देशः_दत्त);
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
content_textsearch_चरणः(संरचना textsearch_context *textsearch,
			अन्वेषणम्_flags_t flags,
			ध्रुव वर्ण *string)
{
	पूर्णाङ्क string_len;
	पूर्णाङ्क i = 0;
	nserror res = NSERROR_OK;

	प्रतिज्ञा(textsearch != रिक्त);

	/* broadcast recent query string */
	textsearch_प्रसारणम्(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = वाक्यदैर्घ्यम्(string);
	प्रति (i = 0; i < string_len; i++) {
		यदि (string[i] != '#' && string[i] != '*')
			भङ्ग;
	}

	यदि (i < string_len) {
		res = अन्वेषणम्_text(textsearch, string, string_len, flags);
	} अन्यथा {
		सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;

		free_matches(textsearch);

		/* update match state */
		textsearch_प्रसारणम्(textsearch,
				     CONTENT_TEXTSEARCH_मेलनम्,
				     true,
				     रिक्त);

		/* update back state */
		textsearch_प्रसारणम्(textsearch,
				     CONTENT_TEXTSEARCH_पृष्ठभागः,
				     false,
				     रिक्त);

		/* update forward state */
		textsearch_प्रसारणम्(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     रिक्त);

		/* clear scroll */
		सन्देशः_दत्त.scroll.area = false;
		सन्देशः_दत्त.scroll.x0 = 0;
		सन्देशः_दत्त.scroll.y0 = 0;
		content_प्रसारणम्(textsearch->c,
				  CONTENT_सन्देशः_SCROLL,
				  &सन्देशः_दत्त);
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
			  शून्य *gui_दत्त,
			  संरचना textsearch_context **textsearch_बहिर्गमनम्)
{
	संरचना textsearch_context *context;
	संरचना list_प्रविष्टिः *अन्वेषणम्_शीर्ष;
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

	context = स्मृतिं_देहि(परिमाण(संरचना textsearch_context));
	यदि (context == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	अन्वेषणम्_शीर्ष = स्मृतिं_देहि(परिमाण(संरचना list_प्रविष्टिः));
	यदि (अन्वेषणम्_शीर्ष == रिक्त) {
		मुक्त(context);
		प्रत्यागम NSERROR_NOMEM;
	}

	अन्वेषणम्_शीर्ष->start_idx = 0;
	अन्वेषणम्_शीर्ष->अन्तः_idx = 0;
	अन्वेषणम्_शीर्ष->start_box = रिक्त;
	अन्वेषणम्_शीर्ष->अन्तः_box = रिक्त;
	अन्वेषणम्_शीर्ष->sel = रिक्त;
	अन्वेषणम्_शीर्ष->पूर्व = रिक्त;
	अन्वेषणम्_शीर्ष->अग्र = रिक्त;

	context->प्राप्तम् = अन्वेषणम्_शीर्ष;
	context->current = रिक्त;
	context->string = रिक्त;
	context->पूर्व_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_दत्त;

	*textsearch_बहिर्गमनम् = context;

	प्रत्यागम NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ध्रुव वर्ण *
content_textsearch_find_आदर्शः(ध्रुव वर्ण *string,
				पूर्णाङ्क s_len,
				ध्रुव वर्ण *आदर्शः,
				पूर्णाङ्क p_len,
				bool case_sens,
				अचिह्नित पूर्णाङ्क *m_len)
{
	संरचना { ध्रुव वर्ण *ss, *s, *p; bool first; } context[16];
	ध्रुव वर्ण *ep = आदर्शः + p_len;
	ध्रुव वर्ण *es = string  + s_len;
	ध्रुव वर्ण *p = आदर्शः - 1;  /* a virtual '*' before the pattern */
	ध्रुव वर्ण *ss = string;
	ध्रुव वर्ण *s = string;
	bool first = true;
	पूर्णाङ्क शिखरम् = 0;

	यावत् (p < ep) {
		bool matches;
		यदि (p < आदर्शः || *p == '*') {
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
				यदि (!case_sens) ch = ascii_गन्तव्यम्_ऊर्ध्वस्थम्(ch);
				यावत् (s < es) {
					यदि (case_sens) {
						यदि (*s == ch) भङ्ग;
					} अन्यथा यदि (ascii_गन्तव्यम्_ऊर्ध्वस्थम्(*s) == ch)
						भङ्ग;
					s++;
				}
			}

			यदि (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				यदि (शिखरम् < (पूर्णाङ्क)NOF_ELEMENTS(context)) {
					context[शिखरम्].ss = ss;
					context[शिखरम्].s  = s + 1;
					context[शिखरम्].p  = p - 1;
					/* ptr to last asterisk */
					context[शिखरम्].first = first;
					शिखरम्++;
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
					matches = (ascii_गन्तव्यम्_ऊर्ध्वस्थम्(*s) == ascii_गन्तव्यम्_ऊर्ध्वस्थम्(ch));
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
			यदि (--शिखरम् < 0)
				प्रत्यागम रिक्त;  /* no match, give up */

			ss = context[शिखरम्].ss;
			s  = context[शिखरम्].s;
			p  = context[शिखरम्].p;
			first = context[शिखरम्].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	प्रत्यागम ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_सङ्कलय_मेलनम्(संरचना textsearch_context *context,
			     अचिह्नित start_idx,
			     अचिह्नित अन्तः_idx,
			     संरचना box *start_box,
			     संरचना box *अन्तः_box)
{
	संरचना list_प्रविष्टिः *प्रविष्टिः;

	/* found string in box => add to list */
	प्रविष्टिः = शून्यस्मृतिं_देहि(1, परिमाण(*प्रविष्टिः));
	यदि (प्रविष्टिः == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रविष्टिः->start_idx = start_idx;
	प्रविष्टिः->अन्तः_idx = अन्तः_idx;
	प्रविष्टिः->start_box = start_box;
	प्रविष्टिः->अन्तः_box = अन्तः_box;
	प्रविष्टिः->sel = रिक्त;

	प्रविष्टिः->अग्र = रिक्त;
	प्रविष्टिः->पूर्व = context->प्राप्तम्->पूर्व;

	यदि (context->प्राप्तम्->पूर्व == रिक्त) {
		context->प्राप्तम्->अग्र = प्रविष्टिः;
	} अन्यथा {
		context->प्राप्तम्->पूर्व->अग्र = प्रविष्टिः;
	}

	context->प्राप्तम्->पूर्व = प्रविष्टिः;

	प्रत्यागम NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(संरचना textsearch_context *textsearch,
				 अचिह्नित start_विस्थापनम्,
				 अचिह्नित अन्तः_विस्थापनम्,
				 अचिह्नित *start_idx,
				 अचिह्नित *अन्तः_idx)
{
	संरचना list_प्रविष्टिः *cur;

	प्रति (cur = textsearch->प्राप्तम्->अग्र; cur != रिक्त; cur = cur->अग्र) {
		यदि (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_विस्थापनम्,
					  अन्तः_विस्थापनम्,
					  start_idx,
					  अन्तः_idx)) {
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
		textsearch_प्रसारणम्(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		मुक्त(textsearch->string);
	}

	/* update back state */
	textsearch_प्रसारणम्(textsearch,
			     CONTENT_TEXTSEARCH_पृष्ठभागः,
			     true,
			     रिक्त);

	/* update forward state */
	textsearch_प्रसारणम्(textsearch,
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
		   अन्वेषणम्_flags_t flags,
		   ध्रुव वर्ण *string)
{
	संरचना content *c = hlcache_handle_get_content(h);
	nserror res;

	प्रतिज्ञा(c != रिक्त);

	यदि (string != रिक्त &&
	    c->textsearch.string != रिक्त &&
	    c->textsearch.context != रिक्त &&
	    वाक्यं_तुलय(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_चरणः(c->textsearch.context, flags, string);

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

		content_textsearch_चरणः(c->textsearch.context, flags, string);

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
