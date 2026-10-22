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
بنية list_اندراج {
	/**
	 * previous match
	 */
	بنية list_اندراج *prev;

	/**
	 * next match
	 */
	بنية list_اندراج *اگلا;

	/**
	 * start position of match
	 */
	بنانشانی شروع_idx;

	/**
	 * end of match
	 */
	بنانشانی سرا_idx;

	/**
	 * content opaque start pointer
	 */
	بنية box *شروع_box;

	/**
	 * content opaque end pointer
	 */
	بنية box *سرا_box;

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
	بنية list_اندراج *لبھیا;

	/**
	 * current selected match
	 */
	بنية list_اندراج *current; /* first for select all */

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
textsearch_نشر(بنية textsearch_context *textsearch,
		     صحيح type,
		     bool state,
		     ثابت حرف *string)
{
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	پیغام_انکڑے.textsearch.type = type;
	پیغام_انکڑے.textsearch.ctx = textsearch->gui_p;
	پیغام_انکڑے.textsearch.state = state;
	پیغام_انکڑے.textsearch.string = string;
	content_نشر(textsearch->c, CONTENT_پیغام_TEXTSEARCH, &پیغام_انکڑے);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساكن فراغ free_matches(بنية textsearch_context *textsearch)
{
	بنية list_اندراج *cur;
	بنية list_اندراج *nxt;

	cur = textsearch->لبھیا->اگلا;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->لبھیا->prev = عدم;
	textsearch->لبھیا->اگلا = عدم;

	لكل (; cur; cur = nxt) {
		nxt = cur->اگلا;
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
ساكن فراغ تلاش_show_all(bool all, بنية textsearch_context *context)
{
	بنية list_اندراج *a;

	لكل (a = context->لبھیا->اگلا; a; a = a->اگلا) {
		bool جوڑو = true;
		إذا (!all && a != context->current) {
			جوڑو = false;
			إذا (a->sel) {
				selection_destroy(a->sel);
				a->sel = عدم;
			}
		}

		إذا (جوڑو && !a->sel) {

			a->sel = selection_create(context->c);
			إذا (a->sel != عدم) {
				selection_init(a->sel);
				selection_سیٹ_position(a->sel,
						       a->شروع_idx,
						       a->سرا_idx);
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
تلاش_متن(بنية textsearch_context *context,
	    ثابت حرف *string,
	    صحيح string_len,
	    تلاش_flags_t flags)
{
	بنية rect bounds;
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & تلاش_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & تلاش_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & تلاش_FLAG_SHOWALL) != 0) ? true : false;

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

		context->string = حافظہ_دیو(string_len + 1);
		إذا (context->string != عدم) {
			حافظہ_نقل(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_لبھو, true, عدم);


		/* call content find handler */
		res = context->c->handler->textsearch_لبھو(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_لبھو, false, عدم);

		إذا (res != NSERROR_OK) {
			free_matches(context);
			أرجع res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->لبھیا->اگلا;
		context->newsearch = false;

	} وإلا إذا (context->current != عدم) {
		/* continued search in the direction specified */
		إذا (forwards) {
			إذا (context->current->اگلا)
				context->current = context->current->اگلا;
		} وإلا {
			إذا (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_میل,
			     (context->current != عدم),
			     عدم);

	تلاش_show_all(showall, context);

	/* update back state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_پٹھ,
			     ((context->current != عدم) &&
			      (context->current->prev != عدم)),
			     عدم);

	/* update forward state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != عدم) &&
			      (context->current->اگلا != عدم)),
			     عدم);


	إذا (context->current == عدم) {
		/* no current match */
		أرجع res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->شروع_idx,
					context->current->سرا_idx,
					context->current->شروع_box,
					context->current->سرا_box,
					&bounds);
	إذا (res == NSERROR_OK) {
		پیغام_انکڑے.scroll.area = true;
		پیغام_انکڑے.scroll.x0 = bounds.x0;
		پیغام_انکڑے.scroll.y0 = bounds.y0;
		پیغام_انکڑے.scroll.x1 = bounds.x1;
		پیغام_انکڑے.scroll.y1 = bounds.y1;
		content_نشر(context->c, CONTENT_پیغام_SCROLL, &پیغام_انکڑے);
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
content_textsearch_قدم(بنية textsearch_context *textsearch,
			تلاش_flags_t flags,
			ثابت حرف *string)
{
	صحيح string_len;
	صحيح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != عدم);

	/* broadcast recent query string */
	textsearch_نشر(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = لڑی_لمبائی(string);
	لكل (i = 0; i < string_len; i++) {
		إذا (string[i] != '#' && string[i] != '*')
			اكسر;
	}

	إذا (i < string_len) {
		res = تلاش_متن(textsearch, string, string_len, flags);
	} وإلا {
		اتحاد content_پیغام_انکڑے پیغام_انکڑے;

		free_matches(textsearch);

		/* update match state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_میل,
				     true,
				     عدم);

		/* update back state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_پٹھ,
				     false,
				     عدم);

		/* update forward state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     عدم);

		/* clear scroll */
		پیغام_انکڑے.scroll.area = false;
		پیغام_انکڑے.scroll.x0 = 0;
		پیغام_انکڑے.scroll.y0 = 0;
		content_نشر(textsearch->c,
				  CONTENT_پیغام_SCROLL,
				  &پیغام_انکڑے);
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
			  فراغ *gui_انکڑے,
			  بنية textsearch_context **textsearch_باہرلا)
{
	بنية textsearch_context *context;
	بنية list_اندراج *تلاش_سر;
	content_type type;

	إذا ((c->handler->textsearch_لبھو == عدم) ||
	    (c->handler->textsearch_bounds == عدم)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = حافظہ_دیو(حجم(بنية textsearch_context));
	إذا (context == عدم) {
		أرجع NSERROR_NOMEM;
	}

	تلاش_سر = حافظہ_دیو(حجم(بنية list_اندراج));
	إذا (تلاش_سر == عدم) {
		حرر(context);
		أرجع NSERROR_NOMEM;
	}

	تلاش_سر->شروع_idx = 0;
	تلاش_سر->سرا_idx = 0;
	تلاش_سر->شروع_box = عدم;
	تلاش_سر->سرا_box = عدم;
	تلاش_سر->sel = عدم;
	تلاش_سر->prev = عدم;
	تلاش_سر->اگلا = عدم;

	context->لبھیا = تلاش_سر;
	context->current = عدم;
	context->string = عدم;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_انکڑے;

	*textsearch_باہرلا = context;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_لبھو_نمونہ(ثابت حرف *string,
				صحيح s_len,
				ثابت حرف *نمونہ,
				صحيح p_len,
				bool case_sens,
				بنانشانی صحيح *m_len)
{
	بنية { ثابت حرف *ss, *s, *p; bool پہلا; } context[16];
	ثابت حرف *ep = نمونہ + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = نمونہ - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool پہلا = true;
	صحيح چوٹی = 0;

	طالما (p < ep) {
		bool matches;
		إذا (p < نمونہ || *p == '*') {
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
				إذا (!case_sens) ch = ascii_منزل_اتلا(ch);
				طالما (s < es) {
					إذا (case_sens) {
						إذا (*s == ch) اكسر;
					} وإلا إذا (ascii_منزل_اتلا(*s) == ch)
						اكسر;
					s++;
				}
			}

			إذا (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				إذا (چوٹی < (صحيح)NOF_ELEMENTS(context)) {
					context[چوٹی].ss = ss;
					context[چوٹی].s  = s + 1;
					context[چوٹی].p  = p - 1;
					/* ptr to last asterisk */
					context[چوٹی].پہلا = پہلا;
					چوٹی++;
				}

				إذا (پہلا) {
					ss = s;
					/* remember first non-'*' char */
					پہلا = false;
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
					matches = (ascii_منزل_اتلا(*s) == ascii_منزل_اتلا(ch));
			}
			إذا (matches && پہلا) {
				ss = s;  /* remember first non-'*' char */
				پہلا = false;
			}
		} وإلا {
			matches = false;
		}

		إذا (matches) {
			p++; s++;
		} وإلا {
			/* doesn't match,
			 * resume with stacked context if we have one */
			إذا (--چوٹی < 0)
				أرجع عدم;  /* no match, give up */

			ss = context[چوٹی].ss;
			s  = context[چوٹی].s;
			p  = context[چوٹی].p;
			پہلا = context[چوٹی].پہلا;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	أرجع ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_جوڑو_میل(بنية textsearch_context *context,
			     بنانشانی شروع_idx,
			     بنانشانی سرا_idx,
			     بنية box *شروع_box,
			     بنية box *سرا_box)
{
	بنية list_اندراج *اندراج;

	/* found string in box => add to list */
	اندراج = صفر_حافظہ_دیو(1, حجم(*اندراج));
	إذا (اندراج == عدم) {
		أرجع NSERROR_NOMEM;
	}

	اندراج->شروع_idx = شروع_idx;
	اندراج->سرا_idx = سرا_idx;
	اندراج->شروع_box = شروع_box;
	اندراج->سرا_box = سرا_box;
	اندراج->sel = عدم;

	اندراج->اگلا = عدم;
	اندراج->prev = context->لبھیا->prev;

	إذا (context->لبھیا->prev == عدم) {
		context->لبھیا->اگلا = اندراج;
	} وإلا {
		context->لبھیا->prev->اگلا = اندراج;
	}

	context->لبھیا->prev = اندراج;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(بنية textsearch_context *textsearch,
				 بنانشانی شروع_وتھ,
				 بنانشانی سرا_وتھ,
				 بنانشانی *شروع_idx,
				 بنانشانی *سرا_idx)
{
	بنية list_اندراج *cur;

	لكل (cur = textsearch->لبھیا->اگلا; cur != عدم; cur = cur->اگلا) {
		إذا (cur->sel &&
		    selection_highlighted(cur->sel,
					  شروع_وتھ,
					  سرا_وتھ,
					  شروع_idx,
					  سرا_idx)) {
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
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		حرر(textsearch->string);
	}

	/* update back state */
	textsearch_نشر(textsearch,
			     CONTENT_TEXTSEARCH_پٹھ,
			     true,
			     عدم);

	/* update forward state */
	textsearch_نشر(textsearch,
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
		   تلاش_flags_t flags,
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
		content_textsearch_قدم(c->textsearch.context, flags, string);

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

		content_textsearch_قدم(c->textsearch.context, flags, string);

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
