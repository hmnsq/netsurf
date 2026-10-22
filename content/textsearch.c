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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "desktop/selection.h"

#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "content/textsearch.h"

/**
 * search match
 */
கட்டமைப்பு list_entry {
	/**
	 * previous match
	 */
	கட்டமைப்பு list_entry *prev;

	/**
	 * next match
	 */
	கட்டமைப்பு list_entry *next;

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
	கட்டமைப்பு box *start_box;

	/**
	 * content opaque end pointer
	 */
	கட்டமைப்பு box *end_box;

	/**
	 * content specific selection object
	 */
	கட்டமைப்பு selection *sel;
};

/**
 * The context for a free text search
 */
கட்டமைப்பு textsearch_context {

	/**
	 * content search was performed upon
	 */
	கட்டமைப்பு content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	வெறுமை *gui_p;

	/**
	 * List of matches
	 */
	கட்டமைப்பு list_entry *found;

	/**
	 * current selected match
	 */
	கட்டமைப்பு list_entry *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	எழுத்து *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
நிலையான உள்வரி வெறுமை
textsearch_broadcast(கட்டமைப்பு textsearch_context *textsearch,
		     முழு type,
		     bool state,
		     மாறிலி எழுத்து *string)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	செய்தி_data.textsearch.type = type;
	செய்தி_data.textsearch.ctx = textsearch->gui_p;
	செய்தி_data.textsearch.state = state;
	செய்தி_data.textsearch.string = string;
	content_broadcast(textsearch->c, CONTENT_செய்தி_TEXTSEARCH, &செய்தி_data);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
நிலையான வெறுமை free_matches(கட்டமைப்பு textsearch_context *textsearch)
{
	கட்டமைப்பு list_entry *cur;
	கட்டமைப்பு list_entry *nxt;

	cur = textsearch->found->next;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->found->prev = வெற்று;
	textsearch->found->next = வெற்று;

	க்கு (; cur; cur = nxt) {
		nxt = cur->next;
		எனில் (cur->sel) {
			selection_destroy(cur->sel);
		}
		விடுவி(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
நிலையான வெறுமை தேடல்_show_all(bool all, கட்டமைப்பு textsearch_context *context)
{
	கட்டமைப்பு list_entry *a;

	க்கு (a = context->found->next; a; a = a->next) {
		bool add = true;
		எனில் (!all && a != context->current) {
			add = false;
			எனில் (a->sel) {
				selection_destroy(a->sel);
				a->sel = வெற்று;
			}
		}

		எனில் (add && !a->sel) {

			a->sel = selection_create(context->c);
			எனில் (a->sel != வெற்று) {
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
நிலையான nserror
தேடல்_text(கட்டமைப்பு textsearch_context *context,
	    மாறிலி எழுத்து *string,
	    முழு string_len,
	    தேடல்_flags_t flags)
{
	கட்டமைப்பு rect bounds;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & தேடல்_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & தேடல்_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & தேடல்_FLAG_SHOWALL) != 0) ? true : false;

	எனில் (context->c == வெற்று) {
		திரும்பு res;
	}

	/* check if we need to start a new search or continue an old one */
	எனில் ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		எனில் (context->string != வெற்று) {
			விடுவி(context->string);
		}

		context->current = வெற்று;
		free_matches(context);

		context->string = malloc(string_len + 1);
		எனில் (context->string != வெற்று) {
			memcpy(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, true, வெற்று);


		/* call content find handler */
		res = context->c->handler->textsearch_find(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_broadcast(context, CONTENT_TEXTSEARCH_FIND, false, வெற்று);

		எனில் (res != NSERROR_OK) {
			free_matches(context);
			திரும்பு res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->found->next;
		context->newsearch = false;

	} இல்லையெனில் எனில் (context->current != வெற்று) {
		/* continued search in the direction specified */
		எனில் (forwards) {
			எனில் (context->current->next)
				context->current = context->current->next;
		} இல்லையெனில் {
			எனில் (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_MATCH,
			     (context->current != வெற்று),
			     வெற்று);

	தேடல்_show_all(showall, context);

	/* update back state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_BACK,
			     ((context->current != வெற்று) &&
			      (context->current->prev != வெற்று)),
			     வெற்று);

	/* update forward state */
	textsearch_broadcast(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != வெற்று) &&
			      (context->current->next != வெற்று)),
			     வெற்று);


	எனில் (context->current == வெற்று) {
		/* no current match */
		திரும்பு res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->start_idx,
					context->current->end_idx,
					context->current->start_box,
					context->current->end_box,
					&bounds);
	எனில் (res == NSERROR_OK) {
		செய்தி_data.scroll.area = true;
		செய்தி_data.scroll.x0 = bounds.x0;
		செய்தி_data.scroll.y0 = bounds.y0;
		செய்தி_data.scroll.x1 = bounds.x1;
		செய்தி_data.scroll.y1 = bounds.y1;
		content_broadcast(context->c, CONTENT_செய்தி_SCROLL, &செய்தி_data);
	}

	திரும்பு res;
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
நிலையான nserror
content_textsearch_step(கட்டமைப்பு textsearch_context *textsearch,
			தேடல்_flags_t flags,
			மாறிலி எழுத்து *string)
{
	முழு string_len;
	முழு i = 0;
	nserror res = NSERROR_OK;

	ப்ரதிஜ்ஞா(textsearch != வெற்று);

	/* broadcast recent query string */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = strlen(string);
	க்கு (i = 0; i < string_len; i++) {
		எனில் (string[i] != '#' && string[i] != '*')
			நிறுத்து;
	}

	எனில் (i < string_len) {
		res = தேடல்_text(textsearch, string, string_len, flags);
	} இல்லையெனில் {
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;

		free_matches(textsearch);

		/* update match state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_MATCH,
				     true,
				     வெற்று);

		/* update back state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_BACK,
				     false,
				     வெற்று);

		/* update forward state */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     வெற்று);

		/* clear scroll */
		செய்தி_data.scroll.area = false;
		செய்தி_data.scroll.x0 = 0;
		செய்தி_data.scroll.y0 = 0;
		content_broadcast(textsearch->c,
				  CONTENT_செய்தி_SCROLL,
				  &செய்தி_data);
	}

	திரும்பு res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
நிலையான nserror content_textsearch__clear(கட்டமைப்பு content *c)
{
	விடுவி(c->textsearch.string);
	c->textsearch.string = வெற்று;

	எனில் (c->textsearch.context != வெற்று) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = வெற்று;
	}
	திரும்பு NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
நிலையான nserror
content_textsearch_create(கட்டமைப்பு content *c,
			  வெறுமை *gui_data,
			  கட்டமைப்பு textsearch_context **textsearch_out)
{
	கட்டமைப்பு textsearch_context *context;
	கட்டமைப்பு list_entry *தேடல்_head;
	content_type type;

	எனில் ((c->handler->textsearch_find == வெற்று) ||
	    (c->handler->textsearch_bounds == வெற்று)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		திரும்பு NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = malloc(அளவு(கட்டமைப்பு textsearch_context));
	எனில் (context == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	தேடல்_head = malloc(அளவு(கட்டமைப்பு list_entry));
	எனில் (தேடல்_head == வெற்று) {
		விடுவி(context);
		திரும்பு NSERROR_NOMEM;
	}

	தேடல்_head->start_idx = 0;
	தேடல்_head->end_idx = 0;
	தேடல்_head->start_box = வெற்று;
	தேடல்_head->end_box = வெற்று;
	தேடல்_head->sel = வெற்று;
	தேடல்_head->prev = வெற்று;
	தேடல்_head->next = வெற்று;

	context->found = தேடல்_head;
	context->current = வெற்று;
	context->string = வெற்று;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_data;

	*textsearch_out = context;

	திரும்பு NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
மாறிலி எழுத்து *
content_textsearch_find_pattern(மாறிலி எழுத்து *string,
				முழு s_len,
				மாறிலி எழுத்து *pattern,
				முழு p_len,
				bool case_sens,
				unsigned முழு *m_len)
{
	கட்டமைப்பு { மாறிலி எழுத்து *ss, *s, *p; bool first; } context[16];
	மாறிலி எழுத்து *ep = pattern + p_len;
	மாறிலி எழுத்து *es = string  + s_len;
	மாறிலி எழுத்து *p = pattern - 1;  /* a virtual '*' before the pattern */
	மாறிலி எழுத்து *ss = string;
	மாறிலி எழுத்து *s = string;
	bool first = true;
	முழு உச்சி = 0;

	போது (p < ep) {
		bool matches;
		எனில் (p < pattern || *p == '*') {
			எழுத்து ch;

			/* skip any further asterisks; one is the same as many
			*/
			செய் p++; போது (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			எனில் (p >= ep) நிறுத்து;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			எனில் (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				எனில் (!case_sens) ch = ascii_to_upper(ch);
				போது (s < es) {
					எனில் (case_sens) {
						எனில் (*s == ch) நிறுத்து;
					} இல்லையெனில் எனில் (ascii_to_upper(*s) == ch)
						நிறுத்து;
					s++;
				}
			}

			எனில் (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				எனில் (உச்சி < (முழு)NOF_ELEMENTS(context)) {
					context[உச்சி].ss = ss;
					context[உச்சி].s  = s + 1;
					context[உச்சி].p  = p - 1;
					/* ptr to last asterisk */
					context[உச்சி].first = first;
					உச்சி++;
				}

				எனில் (first) {
					ss = s;
					/* remember first non-'*' char */
					first = false;
				}

				matches = true;
			} இல்லையெனில் {
				matches = false;
			}

		} இல்லையெனில் எனில் (s < es) {
			எழுத்து ch = *p;
			எனில் (ch == '#')
				matches = true;
			இல்லையெனில் {
				எனில் (case_sens)
					matches = (*s == ch);
				இல்லையெனில்
					matches = (ascii_to_upper(*s) == ascii_to_upper(ch));
			}
			எனில் (matches && first) {
				ss = s;  /* remember first non-'*' char */
				first = false;
			}
		} இல்லையெனில் {
			matches = false;
		}

		எனில் (matches) {
			p++; s++;
		} இல்லையெனில் {
			/* doesn't match,
			 * resume with stacked context if we have one */
			எனில் (--உச்சி < 0)
				திரும்பு வெற்று;  /* no match, give up */

			ss = context[உச்சி].ss;
			s  = context[உச்சி].s;
			p  = context[உச்சி].p;
			first = context[உச்சி].first;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	திரும்பு ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_add_match(கட்டமைப்பு textsearch_context *context,
			     unsigned start_idx,
			     unsigned end_idx,
			     கட்டமைப்பு box *start_box,
			     கட்டமைப்பு box *end_box)
{
	கட்டமைப்பு list_entry *entry;

	/* found string in box => add to list */
	entry = calloc(1, அளவு(*entry));
	எனில் (entry == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	entry->start_idx = start_idx;
	entry->end_idx = end_idx;
	entry->start_box = start_box;
	entry->end_box = end_box;
	entry->sel = வெற்று;

	entry->next = வெற்று;
	entry->prev = context->found->prev;

	எனில் (context->found->prev == வெற்று) {
		context->found->next = entry;
	} இல்லையெனில் {
		context->found->prev->next = entry;
	}

	context->found->prev = entry;

	திரும்பு NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(கட்டமைப்பு textsearch_context *textsearch,
				 unsigned start_offset,
				 unsigned end_offset,
				 unsigned *start_idx,
				 unsigned *end_idx)
{
	கட்டமைப்பு list_entry *cur;

	க்கு (cur = textsearch->found->next; cur != வெற்று; cur = cur->next) {
		எனில் (cur->sel &&
		    selection_highlighted(cur->sel,
					  start_offset,
					  end_offset,
					  start_idx,
					  end_idx)) {
			திரும்பு true;
		}
	}

	திரும்பு false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(கட்டமைப்பு textsearch_context *textsearch)
{
	ப்ரதிஜ்ஞா(textsearch != வெற்று);

	எனில் (textsearch->string != வெற்று) {
		/* broadcast recent query string */
		textsearch_broadcast(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		விடுவி(textsearch->string);
	}

	/* update back state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_BACK,
			     true,
			     வெற்று);

	/* update forward state */
	textsearch_broadcast(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     வெற்று);

	free_matches(textsearch);
	விடுவி(textsearch);

	திரும்பு NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(கட்டமைப்பு hlcache_handle *h,
		   வெறுமை *context,
		   தேடல்_flags_t flags,
		   மாறிலி எழுத்து *string)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	nserror res;

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (string != வெற்று &&
	    c->textsearch.string != வெற்று &&
	    c->textsearch.context != வெற்று &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_step(c->textsearch.context, flags, string);

	} இல்லையெனில் எனில் (string != வெற்று) {
		/* New search */
		விடுவி(c->textsearch.string);
		c->textsearch.string = strdup(string);
		எனில் (c->textsearch.string == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}

		எனில் (c->textsearch.context != வெற்று) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = வெற்று;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}

		content_textsearch_step(c->textsearch.context, flags, string);

	} இல்லையெனில் {
		/* Clear search */
		content_textsearch__clear(c);

		விடுவி(c->textsearch.string);
		c->textsearch.string = வெற்று;
	}

	திரும்பு NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(கட்டமைப்பு hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	திரும்பு(content_textsearch__clear(c));
}
