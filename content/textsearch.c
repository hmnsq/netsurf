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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "utils/errors.h"
#ضمن "utils/utils.h"
#ضمن "utils/ascii.h"
#ضمن "netsurf/types.h"
#ضمن "desktop/selection.h"

#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/hlcache.h"
#ضمن "content/textsearch.h"

/**
 * search match
 */
بنية list_entry {
	/**
	 * previous match
	 */
	بنية list_entry *prev;

	/**
	 * next match
	 */
	بنية list_entry *next;

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
	بنية box *start_box;

	/**
	 * content opaque end pointer
	 */
	بنية box *end_box;

	/**
	 * content specific selection object
	 */
	بنية selection *sel;
};

/**
 * The context for a free text search
 */
بنية textsearch_context {

	/**
	 * content search was performed upon
	 */
	بنية content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	فراغ *gui_p;

	/**
	 * List of matches
	 */
	بنية list_entry *found;

	/**
	 * current selected match
	 */
	بنية list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	حرف *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ساكن مضمن فراغ
textsearch_broadcast(بنية textsearch_context *textsearch,
		     صحيح type,
		     bool state,
		     ثابت حرف *string)
{
	اتحاد content_سكو_data سكو_data;
	سكو_data.textsearch.type = type;
	سكو_data.textsearch.ctx = textsearch->gui_p;
	سكو_data.textsearch.state = state;
	سكو_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_سكو_TEXTSEARCH, &سكو_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساكن فراغ free_matches(بنية textsearch_context *textsearch)
{
	بنية list_entry *cur;
	بنية list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = عدم;
	textsearch->found->next = عدم;

	لكل (; cur; cur = nxt) {
		nxt = cur->next;
		إذا (cur->sel) {
			selection_destroy(cur->sel);
		}
		حرر(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
ساكن فراغ نيما_show_all(bool all, بنية textsearch_context *context)
{
	بنية list_entry *a;

	لكل (a = context->found->next; a; a = a->next) {
		bool add = true;
		إذا (!all && a != context->current) {
			add = false;
			إذا (a->sel) {
				selection_destroy(a->sel);
				a->sel = عدم;
			}
		}

		إذا (add && !a->sel) {

			a->sel = selection_create(context->c);
			إذا (a->sel != عدم) {
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
ساكن nserror
نيما_text(بنية textsearch_context *context,
	    ثابت حرف *string,
	    صحيح string_len,
	    نيما_flags_t flags)
{
	بنية rect bounds;
	اتحاد content_سكو_data سكو_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & نيما_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & نيما_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & نيما_FLAG_SHOWALL) != 0) ? true : false;

	إذا (context->c == عدم) {
		أرجع res;
	}

	/* check if we need to start a new search or continue an old one */
	إذا ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		إذا (context->string != عدم) {
			حرر(context->string);
		}

		context->current = عدم;
		free_matches(context);

		context->string = malloc(string_len + 1);
		إذا (context->string != عدم) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, عدم);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, عدم);

		إذا (res != NSERROR_OK) {
			free_matches(context);
			أرجع res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} وإلا إذا (context->current != عدم) {
		/* continued search in the direction specified */
		إذا (forwards) {
			إذا (context->current->next)
				context->current = context->current->next;
		} وإلا {
			إذا (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != عدم),
			     عدم);

	نيما_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != عدم) &&
			      (context->current->prev != عدم)),
			     عدم);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != عدم) &&
			      (context->current->next != عدم)),
			     عدم);


	إذا (context->current == عدم) {
		/* no current match */
		أرجع res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	إذا (res == NSERROR_OK) {
		سكو_data.scroll.area = true;
		سكو_data.scroll.x0 = bounds.x0;
		سكو_data.scroll.y0 = bounds.y0;
		سكو_data.scroll.x1 = bounds.x1;
		سكو_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_سكو_SCROLL, &سكو_data);
	}

	أرجع res;
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
ساكن nserror
content_textsearch_step(بنية textsearch_context *textsearch,
			نيما_flags_t flags,
			ثابت حرف *string)
{
	صحيح string_len;
	صحيح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != عدم);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	لكل (i = 0; i < string_len; i++) {
		إذا (string[i] != '#' && string[i] != '*')
			اكسر;
	}

	إذا (i < string_len) {
		res = نيما_text(textsearch, string, string_len, flags);
	} وإلا {
		اتحاد content_سكو_data سكو_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     عدم);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     عدم);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     عدم);

		/* clear scroll */
		سكو_data.scroll.area = false;
		سكو_data.scroll.x0 = 0;
		سكو_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_سكو_SCROLL,
				  &سكو_data);
	}

	أرجع res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ساكن nserror content_textsearch__clear(بنية content *c)
{
	حرر(c->textsearch.string);
	c->textsearch.string = عدم;

	إذا (c->textsearch.context != عدم) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = عدم;
	}
	أرجع NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ساكن nserror
content_textsearch_create(بنية content *c,
			  فراغ *gui_data,
			  بنية textsearch_context **textsearch_out)
{
	بنية textsearch_context *context;
	بنية list_entry *نيما_head;
	content_type type;

	إذا ((c->handler->textsearch_find == عدم) ||
	    (c->handler->textsearch_bounds == عدم)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(حجم(بنية textsearch_context));
	إذا (context == عدم) {
		أرجع NSERROR_NOMEM;
	}

	نيما_head = malloc(حجم(بنية list_entry));
	إذا (نيما_head == عدم) {
		حرر(context);
		أرجع NSERROR_NOMEM;
	}

	نيما_head->start_idx = 0;
	نيما_head->end_idx = 0;
	نيما_head->start_box = عدم;
	نيما_head->end_box = عدم;
	نيما_head->sel = عدم;
	نيما_head->prev = عدم;
	نيما_head->next = عدم;

	context->found = نيما_head;
	context->current = عدم;
	context->string = عدم;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_find_pattern(ثابت حرف *string,
				صحيح s_len,
				ثابت حرف *pattern,
				صحيح p_len,
				bool case_sens,
				unsigned صحيح *m_len)
{
	بنية { ثابت حرف *ss, *s, *p; bool first; } context[16];
	ثابت حرف *ep = pattern + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = pattern - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool first = true;
	صحيح كولي = 0;

	طالما (p < ep) {
		bool matches;
		إذا (p < pattern || *p == '*') {
			حرف ch;

			/* skip any further asterisks; one is the same as many
			*/
			افعل p++; طالما (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			إذا (p >= ep) اكسر;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			إذا (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				إذا (!case_sens) ch = ascii_to_upper(ch);
				طالما (s < es) {
					إذا (case_sens) {
						إذا (*s == ch) اكسر;
					} وإلا إذا (ascii_to_upper(*s) == ch)
						اكسر;
					s++;
				}
			}

			إذا (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				إذا (كولي < (صحيح)NOF_ELEMENTS(context)) {
					context[كولي].ss = ss;
					context[كولي].s  = s + 1;
					context[كولي].p  = p - 1;
					/* ptr to last asterisk */
					context[كولي].first = first;
					كولي++;
				}

				إذا (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} وإلا {
				matches = false;
			}

		} وإلا إذا (s < es) {
			حرف ch = *p;
			إذا (ch == '#')
				matches = true;
			وإلا {
				إذا (case_sens)
					matches = (*s == ch);
				وإلا
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			إذا (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} وإلا {
			matches = false;
		}

		إذا (matches) {
			p++; s++;
		} وإلا {
			/* doesn't match,
			 * resume with stacked context if we have one */
			إذا (--كولي < 0)
				أرجع عدم;  /* no match, give up */

			ss = context[كولي].ss;
			s  = context[كولي].s;
			p  = context[كولي].p;
			first = context[كولي].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	أرجع ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(بنية textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     بنية box *start_box,
			     بنية box *end_box)
{
	بنية list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, حجم(*entry));
	إذا (entry == عدم) {
		أرجع NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = عدم;

	entry->next = عدم;
	entry->prev = context->found->prev;

	إذا (context->found->prev == عدم) {
		context->found->next = entry;
	} وإلا {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(بنية textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	بنية list_entry *cur;

	لكل (cur = textsearch->found->next; cur != عدم; cur = cur->next) {
		إذا (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			أرجع true;
		}
	}

	أرجع false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(بنية textsearch_context *textsearch)
{
	أكد(textsearch != عدم);

	إذا (textsearch->string != عدم) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		حرر(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     عدم);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     عدم);

	free_matches(textsearch);
	حرر(textsearch);

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(بنية hlcache_handle *h,
		   فراغ *context,
		   نيما_flags_t flags,
		   ثابت حرف *string)
{
	بنية content *c = hlcache_handle_get_content(h);
	nserror res;

	أكد(c != عدم);

	إذا (string != عدم &&
	    c->textsearch.string != عدم &&
	    c->textsearch.context != عدم &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} وإلا إذا (string != عدم) {
		/* New search */
		حرر(c->textsearch.string);
		c->textsearch.string = strdup(string);
		إذا (c->textsearch.string == عدم) {
			أرجع NSERROR_NOMEM;
		}

		إذا (c->textsearch.context != عدم) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = عدم;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} وإلا {
		/* Clear search */
		content_textsearch__clear(c);

		حرر(c->textsearch.string);
		c->textsearch.string = عدم;
	}

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(بنية hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	أرجع(content_textsearch__clear(c));
}
