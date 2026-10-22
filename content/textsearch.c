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
কাঠামো list_entry {
	/**
	 * previous match
	 */
	কাঠামো list_entry *পূর্ব;

	/**
	 * next match
	 */
	কাঠামো list_entry *পরবর্তী;

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
	কাঠামো box *start_box;

	/**
	 * content opaque end pointer
	 */
	কাঠামো box *end_box;

	/**
	 * content specific selection object
	 */
	কাঠামো selection *sel;
};

/**
 * The context for a free text search
 */
কাঠামো textsearch_context {

	/**
	 * content search was performed upon
	 */
	কাঠামো content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	শূন্য *gui_p;

	/**
	 * List of matches
	 */
	কাঠামো list_entry *found;

	/**
	 * current selected match
	 */
	কাঠামো list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	অক্ষর *string;
	bool পূর্ব_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
স্থির অন্তঃরেখ শূন্য
textsearch_broadcast(কাঠামো textsearch_context *textsearch,
		     পূর্ণসংখ্যা type,
		     bool state,
		     ধ্রুবক অক্ষর *string)
{
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	বার্তা_তথ্য.textsearch.type = type;
	বার্তা_তথ্য.textsearch.ctx = textsearch->gui_p;
	বার্তা_তথ্য.textsearch.state = state;
	বার্তা_তথ্য.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_বার্তা_TEXTSEARCH, &বার্তা_তথ্য);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
স্থির শূন্য free_matches(কাঠামো textsearch_context *textsearch)
{
	কাঠামো list_entry *cur;
	কাঠামো list_entry *nxt;

	cur = textsearch->found->পরবর্তী;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->পূর্ব = ফাঁকা;
	textsearch->found->পরবর্তী = ফাঁকা;

	জন্য (; cur; cur = nxt) {
		nxt = cur->পরবর্তী;
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
স্থির শূন্য অনুসন্ধান_show_all(bool all, কাঠামো textsearch_context *context)
{
	কাঠামো list_entry *a;

	জন্য (a = context->found->পরবর্তী; a; a = a->পরবর্তী) {
		bool add = true;
		যদি (!all && a != context->current) {
			add = false;
			যদি (a->sel) {
				selection_destroy(a->sel);
				a->sel = ফাঁকা;
			}
		}

		যদি (add && !a->sel) {

			a->sel = selection_create(context->c);
			যদি (a->sel != ফাঁকা) {
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
অনুসন্ধান_text(কাঠামো textsearch_context *context,
	    ধ্রুবক অক্ষর *string,
	    পূর্ণসংখ্যা string_len,
	    অনুসন্ধান_flags_t flags)
{
	কাঠামো rect bounds;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & অনুসন্ধান_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & অনুসন্ধান_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & অনুসন্ধান_FLAG_SHOWALL) != 0) ? true : false;

	যদি (context->c == ফাঁকা) {
		ফেরত res;
	}

	/* check if we need to start a new search or continue an old one */
	যদি ((context->newsearch) ||
	    (context->পূর্ব_case_sens != case_sensitive)) {

		যদি (context->string != ফাঁকা) {
			মুক্ত(context->string);
		}

		context->current = ফাঁকা;
		free_matches(context);

		context->string = malloc(string_len + 1);
		যদি (context->string != ফাঁকা) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, ফাঁকা);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, ফাঁকা);

		যদি (res != NSERROR_OK) {
			free_matches(context);
			ফেরত res;
		}

		context->পূর্ব_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->পরবর্তী;
		context->newsearch = false;

	} অন্যথা যদি (context->current != ফাঁকা) {
		/* continued search in the direction specified */
		যদি (forwards) {
			যদি (context->current->পরবর্তী)
				context->current = context->current->পরবর্তী;
		} অন্যথা {
			যদি (context->current->পূর্ব)
				context->current = context->current->পূর্ব;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != ফাঁকা),
			     ফাঁকা);

	অনুসন্ধান_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != ফাঁকা) &&
			      (context->current->পূর্ব != ফাঁকা)),
			     ফাঁকা);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ফাঁকা) &&
			      (context->current->পরবর্তী != ফাঁকা)),
			     ফাঁকা);


	যদি (context->current == ফাঁকা) {
		/* no current match */
		ফেরত res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	যদি (res == NSERROR_OK) {
		বার্তা_তথ্য.scroll.area = true;
		বার্তা_তথ্য.scroll.x0 = bounds.x0;
		বার্তা_তথ্য.scroll.y0 = bounds.y0;
		বার্তা_তথ্য.scroll.x1 = bounds.x1;
		বার্তা_তথ্য.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_বার্তা_SCROLL, &বার্তা_তথ্য);
	}

	ফেরত res;
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
content_textsearch_step(কাঠামো textsearch_context *textsearch,
			অনুসন্ধান_flags_t flags,
			ধ্রুবক অক্ষর *string)
{
	পূর্ণসংখ্যা string_len;
	পূর্ণসংখ্যা i = 0;
	nserror res = NSERROR_OK;

	প্রতিজ্ঞা(textsearch != ফাঁকা);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	জন্য (i = 0; i < string_len; i++) {
		যদি (string[i] != '#' && string[i] != '*')
			বিরতি;
	}

	যদি (i < string_len) {
		res = অনুসন্ধান_text(textsearch, string, string_len, flags);
	} অন্যথা {
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     ফাঁকা);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     ফাঁকা);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ফাঁকা);

		/* clear scroll */
		বার্তা_তথ্য.scroll.area = false;
		বার্তা_তথ্য.scroll.x0 = 0;
		বার্তা_তথ্য.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_বার্তা_SCROLL,
				  &বার্তা_তথ্য);
	}

	ফেরত res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
স্থির nserror content_textsearch__clear(কাঠামো content *c)
{
	মুক্ত(c->textsearch.string);
	c->textsearch.string = ফাঁকা;

	যদি (c->textsearch.context != ফাঁকা) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ফাঁকা;
	}
	ফেরত NSERROR_OK;
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
content_textsearch_create(কাঠামো content *c,
			  শূন্য *gui_তথ্য,
			  কাঠামো textsearch_context **textsearch_out)
{
	কাঠামো textsearch_context *context;
	কাঠামো list_entry *অনুসন্ধান_শীর্ষ;
	content_type type;

	যদি ((c->handler->textsearch_find == ফাঁকা) ||
	    (c->handler->textsearch_bounds == ফাঁকা)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		ফেরত NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(আকার(কাঠামো textsearch_context));
	যদি (context == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	অনুসন্ধান_শীর্ষ = malloc(আকার(কাঠামো list_entry));
	যদি (অনুসন্ধান_শীর্ষ == ফাঁকা) {
		মুক্ত(context);
		ফেরত NSERROR_NOMEM;
	}

	অনুসন্ধান_শীর্ষ->start_idx = 0;
	অনুসন্ধান_শীর্ষ->end_idx = 0;
	অনুসন্ধান_শীর্ষ->start_box = ফাঁকা;
	অনুসন্ধান_শীর্ষ->end_box = ফাঁকা;
	অনুসন্ধান_শীর্ষ->sel = ফাঁকা;
	অনুসন্ধান_শীর্ষ->পূর্ব = ফাঁকা;
	অনুসন্ধান_শীর্ষ->পরবর্তী = ফাঁকা;

	context->found = অনুসন্ধান_শীর্ষ;
	context->current = ফাঁকা;
	context->string = ফাঁকা;
	context->পূর্ব_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_তথ্য;

	*textsearch_out = context;

	ফেরত NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ধ্রুবক অক্ষর *
content_textsearch_find_pattern(ধ্রুবক অক্ষর *string,
				পূর্ণসংখ্যা s_len,
				ধ্রুবক অক্ষর *pattern,
				পূর্ণসংখ্যা p_len,
				bool case_sens,
				unsigned পূর্ণসংখ্যা *m_len)
{
	কাঠামো { ধ্রুবক অক্ষর *ss, *s, *p; bool first; } context[16];
	ধ্রুবক অক্ষর *ep = pattern + p_len;
	ধ্রুবক অক্ষর *es = string  + s_len;
	ধ্রুবক অক্ষর *p = pattern - 1;  /* a virtual '*' before the pattern */
	ধ্রুবক অক্ষর *ss = string;
	ধ্রুবক অক্ষর *s = string;
	bool first = true;
	পূর্ণসংখ্যা শীর্ষ = 0;

	যখন (p < ep) {
		bool matches;
		যদি (p < pattern || *p == '*') {
			অক্ষর ch;

			/* skip any further asterisks; one is the same as many
			*/
			করো p++; যখন (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			যদি (p >= ep) বিরতি;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			যদি (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				যদি (!case_sens) ch = ascii_to_upper(ch);
				যখন (s < es) {
					যদি (case_sens) {
						যদি (*s == ch) বিরতি;
					} অন্যথা যদি (ascii_to_upper(*s) == ch)
						বিরতি;
					s++;
				}
			}

			যদি (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				যদি (শীর্ষ < (পূর্ণসংখ্যা)NOF_ELEMENTS(context)) {
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
			অক্ষর ch = *p;
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
				ফেরত ফাঁকা;  /* no match, give up */

			ss = context[শীর্ষ].ss;
			s  = context[শীর্ষ].s;
			p  = context[শীর্ষ].p;
			first = context[শীর্ষ].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	ফেরত ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(কাঠামো textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     কাঠামো box *start_box,
			     কাঠামো box *end_box)
{
	কাঠামো list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, আকার(*entry));
	যদি (entry == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = ফাঁকা;

	entry->পরবর্তী = ফাঁকা;
	entry->পূর্ব = context->found->পূর্ব;

	যদি (context->found->পূর্ব == ফাঁকা) {
		context->found->পরবর্তী = entry;
	} অন্যথা {
		context->found->পূর্ব->পরবর্তী = entry;
	}

	context->found->পূর্ব = entry;

	ফেরত NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(কাঠামো textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	কাঠামো list_entry *cur;

	জন্য (cur = textsearch->found->পরবর্তী; cur != ফাঁকা; cur = cur->পরবর্তী) {
		যদি (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			ফেরত true;
		}
	}

	ফেরত false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(কাঠামো textsearch_context *textsearch)
{
	প্রতিজ্ঞা(textsearch != ফাঁকা);

	যদি (textsearch->string != ফাঁকা) {
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
			     ফাঁকা);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ফাঁকা);

	free_matches(textsearch);
	মুক্ত(textsearch);

	ফেরত NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(কাঠামো hlcache_handle *h,
		   শূন্য *context,
		   অনুসন্ধান_flags_t flags,
		   ধ্রুবক অক্ষর *string)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	nserror res;

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (string != ফাঁকা &&
	    c->textsearch.string != ফাঁকা &&
	    c->textsearch.context != ফাঁকা &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} অন্যথা যদি (string != ফাঁকা) {
		/* New search */
		মুক্ত(c->textsearch.string);
		c->textsearch.string = strdup(string);
		যদি (c->textsearch.string == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}

		যদি (c->textsearch.context != ফাঁকা) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ফাঁকা;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} অন্যথা {
		/* Clear search */
		content_textsearch__clear(c);

		মুক্ত(c->textsearch.string);
		c->textsearch.string = ফাঁকা;
	}

	ফেরত NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(কাঠামো hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	ফেরত(content_textsearch__clear(c));
}
