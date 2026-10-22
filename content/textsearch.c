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

#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "utils/errors.h"
#شامل "utils/utils.h"
#شامل "utils/ascii.h"
#شامل "netsurf/types.h"
#شامل "desktop/selection.h"

#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/hlcache.h"
#شامل "content/textsearch.h"

/**
 * search match
 */
ساختار list_entry {
	/**
	 * previous match
	 */
	ساختار list_entry *prev;

	/**
	 * next match
	 */
	ساختار list_entry *next;

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
	ساختار box *start_box;

	/**
	 * content opaque end pointer
	 */
	ساختار box *end_box;

	/**
	 * content specific selection object
	 */
	ساختار selection *sel;
};

/**
 * The context for a free text search
 */
ساختار textsearch_context {

	/**
	 * content search was performed upon
	 */
	ساختار content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	تهی *gui_p;

	/**
	 * List of matches
	 */
	ساختار list_entry *found;

	/**
	 * current selected match
	 */
	ساختار list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	نویسه *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ایستا درونخطی تهی
textsearch_broadcast(ساختار textsearch_context *textsearch,
		     صحیح type,
		     bool state,
		     ثابت نویسه *string)
{
	اجتماع content_پیام_data پیام_data;
	پیام_data.textsearch.type = type;
	پیام_data.textsearch.ctx = textsearch->gui_p;
	پیام_data.textsearch.state = state;
	پیام_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_پیام_TEXTSEARCH, &پیام_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ایستا تهی free_matches(ساختار textsearch_context *textsearch)
{
	ساختار list_entry *cur;
	ساختار list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = پوچ;
	textsearch->found->next = پوچ;

	برای (; cur; cur = nxt) {
		nxt = cur->next;
		اگر (cur->sel) {
			selection_destroy(cur->sel);
		}
		آزاد(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ایستا تهی جستجو_show_all(bool all, ساختار textsearch_context *context)
{
	ساختار list_entry *a;

	برای (a = context->found->next; a; a = a->next) {
		bool add = true;
		اگر (!all && a != context->current) {
			add = false;
			اگر (a->sel) {
				selection_destroy(a->sel);
				a->sel = پوچ;
			}
		}

		اگر (add && !a->sel) {

			a->sel = selection_create(context->c);
			اگر (a->sel != پوچ) {
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
ایستا nserror
جستجو_text(ساختار textsearch_context *context,
	    ثابت نویسه *string,
	    صحیح string_len,
	    جستجو_flags_t flags)
{
	ساختار rect bounds;
	اجتماع content_پیام_data پیام_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & جستجو_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & جستجو_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & جستجو_FLAG_SHOWALL) != 0) ? true : false;

	اگر (context->c == پوچ) {
		بازگشت res;
	}

	/* check if we need to start a new search or continue an old one */
	اگر ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		اگر (context->string != پوچ) {
			آزاد(context->string);
		}

		context->current = پوچ;
		free_matches(context);

		context->string = malloc(string_len + 1);
		اگر (context->string != پوچ) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, پوچ);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, پوچ);

		اگر (res != NSERROR_OK) {
			free_matches(context);
			بازگشت res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} وگرنه اگر (context->current != پوچ) {
		/* continued search in the direction specified */
		اگر (forwards) {
			اگر (context->current->next)
				context->current = context->current->next;
		} وگرنه {
			اگر (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != پوچ),
			     پوچ);

	جستجو_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != پوچ) &&
			      (context->current->prev != پوچ)),
			     پوچ);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != پوچ) &&
			      (context->current->next != پوچ)),
			     پوچ);


	اگر (context->current == پوچ) {
		/* no current match */
		بازگشت res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	اگر (res == NSERROR_OK) {
		پیام_data.scroll.area = true;
		پیام_data.scroll.x0 = bounds.x0;
		پیام_data.scroll.y0 = bounds.y0;
		پیام_data.scroll.x1 = bounds.x1;
		پیام_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_پیام_SCROLL, &پیام_data);
	}

	بازگشت res;
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
ایستا nserror
content_textsearch_step(ساختار textsearch_context *textsearch,
			جستجو_flags_t flags,
			ثابت نویسه *string)
{
	صحیح string_len;
	صحیح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != پوچ);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	برای (i = 0; i < string_len; i++) {
		اگر (string[i] != '#' && string[i] != '*')
			بشکن;
	}

	اگر (i < string_len) {
		res = جستجو_text(textsearch, string, string_len, flags);
	} وگرنه {
		اجتماع content_پیام_data پیام_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     پوچ);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     پوچ);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     پوچ);

		/* clear scroll */
		پیام_data.scroll.area = false;
		پیام_data.scroll.x0 = 0;
		پیام_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_پیام_SCROLL,
				  &پیام_data);
	}

	بازگشت res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ایستا nserror content_textsearch__clear(ساختار content *c)
{
	آزاد(c->textsearch.string);
	c->textsearch.string = پوچ;

	اگر (c->textsearch.context != پوچ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = پوچ;
	}
	بازگشت NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ایستا nserror
content_textsearch_create(ساختار content *c,
			  تهی *gui_data,
			  ساختار textsearch_context **textsearch_out)
{
	ساختار textsearch_context *context;
	ساختار list_entry *جستجو_head;
	content_type type;

	اگر ((c->handler->textsearch_find == پوچ) ||
	    (c->handler->textsearch_bounds == پوچ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		بازگشت NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(اندازه(ساختار textsearch_context));
	اگر (context == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	جستجو_head = malloc(اندازه(ساختار list_entry));
	اگر (جستجو_head == پوچ) {
		آزاد(context);
		بازگشت NSERROR_NOMEM;
	}

	جستجو_head->start_idx = 0;
	جستجو_head->end_idx = 0;
	جستجو_head->start_box = پوچ;
	جستجو_head->end_box = پوچ;
	جستجو_head->sel = پوچ;
	جستجو_head->prev = پوچ;
	جستجو_head->next = پوچ;

	context->found = جستجو_head;
	context->current = پوچ;
	context->string = پوچ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	بازگشت NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت نویسه *
content_textsearch_find_pattern(ثابت نویسه *string,
				صحیح s_len,
				ثابت نویسه *pattern,
				صحیح p_len,
				bool case_sens,
				unsigned صحیح *m_len)
{
	ساختار { ثابت نویسه *ss, *s, *p; bool first; } context[16];
	ثابت نویسه *ep = pattern + p_len;
	ثابت نویسه *es = string  + s_len;
	ثابت نویسه *p = pattern - 1;  /* a virtual '*' before the pattern */
	ثابت نویسه *ss = string;
	ثابت نویسه *s = string;
	bool first = true;
	صحیح بالاترین = 0;

	وقتی (p < ep) {
		bool matches;
		اگر (p < pattern || *p == '*') {
			نویسه ch;

			/* skip any further asterisks; one is the same as many
			*/
			انجامبده p++; وقتی (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			اگر (p >= ep) بشکن;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			اگر (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				اگر (!case_sens) ch = ascii_to_upper(ch);
				وقتی (s < es) {
					اگر (case_sens) {
						اگر (*s == ch) بشکن;
					} وگرنه اگر (ascii_to_upper(*s) == ch)
						بشکن;
					s++;
				}
			}

			اگر (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				اگر (بالاترین < (صحیح)NOF_ELEMENTS(context)) {
					context[بالاترین].ss = ss;
					context[بالاترین].s  = s + 1;
					context[بالاترین].p  = p - 1;
					/* ptr to last asterisk */
					context[بالاترین].first = first;
					بالاترین++;
				}

				اگر (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} وگرنه {
				matches = false;
			}

		} وگرنه اگر (s < es) {
			نویسه ch = *p;
			اگر (ch == '#')
				matches = true;
			وگرنه {
				اگر (case_sens)
					matches = (*s == ch);
				وگرنه
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			اگر (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} وگرنه {
			matches = false;
		}

		اگر (matches) {
			p++; s++;
		} وگرنه {
			/* doesn't match,
			 * resume with stacked context if we have one */
			اگر (--بالاترین < 0)
				بازگشت پوچ;  /* no match, give up */

			ss = context[بالاترین].ss;
			s  = context[بالاترین].s;
			p  = context[بالاترین].p;
			first = context[بالاترین].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	بازگشت ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(ساختار textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     ساختار box *start_box,
			     ساختار box *end_box)
{
	ساختار list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, اندازه(*entry));
	اگر (entry == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = پوچ;

	entry->next = پوچ;
	entry->prev = context->found->prev;

	اگر (context->found->prev == پوچ) {
		context->found->next = entry;
	} وگرنه {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	بازگشت NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ساختار textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	ساختار list_entry *cur;

	برای (cur = textsearch->found->next; cur != پوچ; cur = cur->next) {
		اگر (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			بازگشت true;
		}
	}

	بازگشت false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ساختار textsearch_context *textsearch)
{
	أكد(textsearch != پوچ);

	اگر (textsearch->string != پوچ) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		آزاد(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     پوچ);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     پوچ);

	free_matches(textsearch);
	آزاد(textsearch);

	بازگشت NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ساختار hlcache_handle *h,
		   تهی *context,
		   جستجو_flags_t flags,
		   ثابت نویسه *string)
{
	ساختار content *c = hlcache_handle_get_content(h);
	nserror res;

	أكد(c != پوچ);

	اگر (string != پوچ &&
	    c->textsearch.string != پوچ &&
	    c->textsearch.context != پوچ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} وگرنه اگر (string != پوچ) {
		/* New search */
		آزاد(c->textsearch.string);
		c->textsearch.string = strdup(string);
		اگر (c->textsearch.string == پوچ) {
			بازگشت NSERROR_NOMEM;
		}

		اگر (c->textsearch.context != پوچ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = پوچ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} وگرنه {
		/* Clear search */
		content_textsearch__clear(c);

		آزاد(c->textsearch.string);
		c->textsearch.string = پوچ;
	}

	بازگشت NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ساختار hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	بازگشت(content_textsearch__clear(c));
}
