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
بنية list_مدخل {
	/**
	 * previous match
	 */
	بنية list_مدخل *prev;

	/**
	 * next match
	 */
	بنية list_مدخل *بعدی;

	/**
	 * start position of match
	 */
	بیعلامت شروع_idx;

	/**
	 * end of match
	 */
	بیعلامت انتها_idx;

	/**
	 * content opaque start pointer
	 */
	بنية box *شروع_box;

	/**
	 * content opaque end pointer
	 */
	بنية box *انتها_box;

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
	بنية list_مدخل *یافت;

	/**
	 * current selected match
	 */
	بنية list_مدخل *current; /* first for select all */

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
textsearch_پخش(بنية textsearch_context *textsearch,
		     صحيح type,
		     bool state,
		     ثابت حرف *string)
{
	اتحاد content_پیام_داده پیام_داده;
	پیام_داده.textsearch.type = type;
	پیام_داده.textsearch.ctx = textsearch->gui_p;
	پیام_داده.textsearch.state = state;
	پیام_داده.textsearch.string = string;
	content_پخش(textsearch->c, CONTENT_پیام_TEXTSEARCH, &پیام_داده);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساكن فراغ free_matches(بنية textsearch_context *textsearch)
{
	بنية list_مدخل *cur;
	بنية list_مدخل *nxt;

	cur = textsearch->یافت->بعدی;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->یافت->prev = عدم;
	textsearch->یافت->بعدی = عدم;

	لكل (; cur; cur = nxt) {
		nxt = cur->بعدی;
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
ساكن فراغ جستجو_show_all(bool all, بنية textsearch_context *context)
{
	بنية list_مدخل *a;

	لكل (a = context->یافت->بعدی; a; a = a->بعدی) {
		bool جمع_کن = true;
		إذا (!all && a != context->current) {
			جمع_کن = false;
			إذا (a->sel) {
				selection_destroy(a->sel);
				a->sel = عدم;
			}
		}

		إذا (جمع_کن && !a->sel) {

			a->sel = selection_create(context->c);
			إذا (a->sel != عدم) {
				selection_init(a->sel);
				selection_مجموعه_position(a->sel,
						       a->شروع_idx,
						       a->انتها_idx);
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
جستجو_متن(بنية textsearch_context *context,
	    ثابت حرف *string,
	    صحيح string_len,
	    جستجو_flags_t flags)
{
	بنية rect bounds;
	اتحاد content_پیام_داده پیام_داده;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & جستجو_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & جستجو_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & جستجو_FLAG_SHOWALL) != 0) ? true : false;

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

		context->string = حافظه_بده(string_len + 1);
		إذا (context->string != عدم) {
			کپی_حافظه(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_پخش(context, CONTENT_TEXTSEARCH_پیدا_کن, true, عدم);


		/* call content find handler */
		res = context->c->handler->textsearch_پیدا_کن(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_پخش(context, CONTENT_TEXTSEARCH_پیدا_کن, false, عدم);

		إذا (res != NSERROR_OK) {
			free_matches(context);
			أرجع res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->یافت->بعدی;
		context->newsearch = false;

	} وإلا إذا (context->current != عدم) {
		/* continued search in the direction specified */
		إذا (forwards) {
			إذا (context->current->بعدی)
				context->current = context->current->بعدی;
		} وإلا {
			إذا (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_پخش(context,
			     CONTENT_TEXTSEARCH_تطابق,
			     (context->current != عدم),
			     عدم);

	جستجو_show_all(showall, context);

	/* update back state */
	textsearch_پخش(context,
			     CONTENT_TEXTSEARCH_پشت,
			     ((context->current != عدم) &&
			      (context->current->prev != عدم)),
			     عدم);

	/* update forward state */
	textsearch_پخش(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != عدم) &&
			      (context->current->بعدی != عدم)),
			     عدم);


	إذا (context->current == عدم) {
		/* no current match */
		أرجع res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->شروع_idx,
					context->current->انتها_idx,
					context->current->شروع_box,
					context->current->انتها_box,
					&bounds);
	إذا (res == NSERROR_OK) {
		پیام_داده.scroll.area = true;
		پیام_داده.scroll.x0 = bounds.x0;
		پیام_داده.scroll.y0 = bounds.y0;
		پیام_داده.scroll.x1 = bounds.x1;
		پیام_داده.scroll.y1 = bounds.y1;
		content_پخش(context->c, CONTENT_پیام_SCROLL, &پیام_داده);
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
content_textsearch_گام(بنية textsearch_context *textsearch,
			جستجو_flags_t flags,
			ثابت حرف *string)
{
	صحيح string_len;
	صحيح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != عدم);

	/* broadcast recent query string */
	textsearch_پخش(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = طول_رشته(string);
	لكل (i = 0; i < string_len; i++) {
		إذا (string[i] != '#' && string[i] != '*')
			اكسر;
	}

	إذا (i < string_len) {
		res = جستجو_متن(textsearch, string, string_len, flags);
	} وإلا {
		اتحاد content_پیام_داده پیام_داده;

		free_matches(textsearch);

		/* update match state */
		textsearch_پخش(textsearch,
				     CONTENT_TEXTSEARCH_تطابق,
				     true,
				     عدم);

		/* update back state */
		textsearch_پخش(textsearch,
				     CONTENT_TEXTSEARCH_پشت,
				     false,
				     عدم);

		/* update forward state */
		textsearch_پخش(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     عدم);

		/* clear scroll */
		پیام_داده.scroll.area = false;
		پیام_داده.scroll.x0 = 0;
		پیام_داده.scroll.y0 = 0;
		content_پخش(textsearch->c,
				  CONTENT_پیام_SCROLL,
				  &پیام_داده);
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
			  فراغ *gui_داده,
			  بنية textsearch_context **textsearch_بیرون)
{
	بنية textsearch_context *context;
	بنية list_مدخل *جستجو_سر;
	content_type type;

	إذا ((c->handler->textsearch_پیدا_کن == عدم) ||
	    (c->handler->textsearch_bounds == عدم)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = حافظه_بده(حجم(بنية textsearch_context));
	إذا (context == عدم) {
		أرجع NSERROR_NOMEM;
	}

	جستجو_سر = حافظه_بده(حجم(بنية list_مدخل));
	إذا (جستجو_سر == عدم) {
		حرر(context);
		أرجع NSERROR_NOMEM;
	}

	جستجو_سر->شروع_idx = 0;
	جستجو_سر->انتها_idx = 0;
	جستجو_سر->شروع_box = عدم;
	جستجو_سر->انتها_box = عدم;
	جستجو_سر->sel = عدم;
	جستجو_سر->prev = عدم;
	جستجو_سر->بعدی = عدم;

	context->یافت = جستجو_سر;
	context->current = عدم;
	context->string = عدم;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_داده;

	*textsearch_بیرون = context;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_پیدا_کن_الگو(ثابت حرف *string,
				صحيح s_len,
				ثابت حرف *الگو,
				صحيح p_len,
				bool case_sens,
				بیعلامت صحيح *m_len)
{
	بنية { ثابت حرف *ss, *s, *p; bool اولی; } context[16];
	ثابت حرف *ep = الگو + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = الگو - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool اولی = true;
	صحيح بالاترین = 0;

	طالما (p < ep) {
		bool matches;
		إذا (p < الگو || *p == '*') {
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
				إذا (!case_sens) ch = ascii_مقصد_فوقانی(ch);
				طالما (s < es) {
					إذا (case_sens) {
						إذا (*s == ch) اكسر;
					} وإلا إذا (ascii_مقصد_فوقانی(*s) == ch)
						اكسر;
					s++;
				}
			}

			إذا (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				إذا (بالاترین < (صحيح)NOF_ELEMENTS(context)) {
					context[بالاترین].ss = ss;
					context[بالاترین].s  = s + 1;
					context[بالاترین].p  = p - 1;
					/* ptr to last asterisk */
					context[بالاترین].اولی = اولی;
					بالاترین++;
				}

				إذا (اولی) {
					ss = s;
					/* remember first non-'*' char */
					اولی = false;
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
					matches = (ascii_مقصد_فوقانی(*s) == ascii_مقصد_فوقانی(ch));
			}
			إذا (matches && اولی) {
				ss = s;  /* remember first non-'*' char */
				اولی = false;
			}
		} وإلا {
			matches = false;
		}

		إذا (matches) {
			p++; s++;
		} وإلا {
			/* doesn't match,
			 * resume with stacked context if we have one */
			إذا (--بالاترین < 0)
				أرجع عدم;  /* no match, give up */

			ss = context[بالاترین].ss;
			s  = context[بالاترین].s;
			p  = context[بالاترین].p;
			اولی = context[بالاترین].اولی;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	أرجع ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_جمع_کن_تطابق(بنية textsearch_context *context,
			     بیعلامت شروع_idx,
			     بیعلامت انتها_idx,
			     بنية box *شروع_box,
			     بنية box *انتها_box)
{
	بنية list_مدخل *مدخل;

	/* found string in box => add to list */
	مدخل = حافظه_صفر_بده(1, حجم(*مدخل));
	إذا (مدخل == عدم) {
		أرجع NSERROR_NOMEM;
	}

	مدخل->شروع_idx = شروع_idx;
	مدخل->انتها_idx = انتها_idx;
	مدخل->شروع_box = شروع_box;
	مدخل->انتها_box = انتها_box;
	مدخل->sel = عدم;

	مدخل->بعدی = عدم;
	مدخل->prev = context->یافت->prev;

	إذا (context->یافت->prev == عدم) {
		context->یافت->بعدی = مدخل;
	} وإلا {
		context->یافت->prev->بعدی = مدخل;
	}

	context->یافت->prev = مدخل;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(بنية textsearch_context *textsearch,
				 بیعلامت شروع_جابه_جایی,
				 بیعلامت انتها_جابه_جایی,
				 بیعلامت *شروع_idx,
				 بیعلامت *انتها_idx)
{
	بنية list_مدخل *cur;

	لكل (cur = textsearch->یافت->بعدی; cur != عدم; cur = cur->بعدی) {
		إذا (cur->sel &&
		    selection_highlighted(cur->sel,
					  شروع_جابه_جایی,
					  انتها_جابه_جایی,
					  شروع_idx,
					  انتها_idx)) {
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
		textsearch_پخش(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		حرر(textsearch->string);
	}

	/* update back state */
	textsearch_پخش(textsearch,
			     CONTENT_TEXTSEARCH_پشت,
			     true,
			     عدم);

	/* update forward state */
	textsearch_پخش(textsearch,
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
		   جستجو_flags_t flags,
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
		content_textsearch_گام(c->textsearch.context, flags, string);

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

		content_textsearch_گام(c->textsearch.context, flags, string);

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
