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
بنية list_ثبت {
	/**
	 * previous match
	 */
	بنية list_ثبت *prev;

	/**
	 * next match
	 */
	بنية list_ثبت *بل;

	/**
	 * start position of match
	 */
	بینښه پیل_idx;

	/**
	 * end of match
	 */
	بینښه پای_ټکی_idx;

	/**
	 * content opaque start pointer
	 */
	بنية box *پیل_box;

	/**
	 * content opaque end pointer
	 */
	بنية box *پای_ټکی_box;

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
	بنية list_ثبت *وموندل;

	/**
	 * current selected match
	 */
	بنية list_ثبت *current; /* first for select all */

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
textsearch_خپرونه(بنية textsearch_context *textsearch,
		     صحيح type,
		     bool state,
		     ثابت حرف *string)
{
	اتحاد content_پیغام_معلومات پیغام_معلومات;
	پیغام_معلومات.textsearch.type = type;
	پیغام_معلومات.textsearch.ctx = textsearch->gui_p;
	پیغام_معلومات.textsearch.state = state;
	پیغام_معلومات.textsearch.string = string;
	content_خپرونه(textsearch->c, CONTENT_پیغام_TEXTSEARCH, &پیغام_معلومات);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساكن فراغ free_matches(بنية textsearch_context *textsearch)
{
	بنية list_ثبت *cur;
	بنية list_ثبت *nxt;

	cur = textsearch->وموندل->بل;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->وموندل->prev = عدم;
	textsearch->وموندل->بل = عدم;

	لكل (; cur; cur = nxt) {
		nxt = cur->بل;
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
ساكن فراغ پلټنه_show_all(bool all, بنية textsearch_context *context)
{
	بنية list_ثبت *a;

	لكل (a = context->وموندل->بل; a; a = a->بل) {
		bool جمع_کړه = true;
		إذا (!all && a != context->current) {
			جمع_کړه = false;
			إذا (a->sel) {
				selection_destroy(a->sel);
				a->sel = عدم;
			}
		}

		إذا (جمع_کړه && !a->sel) {

			a->sel = selection_create(context->c);
			إذا (a->sel != عدم) {
				selection_init(a->sel);
				selection_سېټ_position(a->sel,
						       a->پیل_idx,
						       a->پای_ټکی_idx);
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
پلټنه_متن(بنية textsearch_context *context,
	    ثابت حرف *string,
	    صحيح string_len,
	    پلټنه_flags_t flags)
{
	بنية rect bounds;
	اتحاد content_پیغام_معلومات پیغام_معلومات;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & پلټنه_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & پلټنه_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & پلټنه_FLAG_SHOWALL) != 0) ? true : false;

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

		context->string = حافظه_ورکړه(string_len + 1);
		إذا (context->string != عدم) {
			حافظه_نقل(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_خپرونه(context, CONTENT_TEXTSEARCH_پیدا_کړه, true, عدم);


		/* call content find handler */
		res = context->c->handler->textsearch_پیدا_کړه(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_خپرونه(context, CONTENT_TEXTSEARCH_پیدا_کړه, false, عدم);

		إذا (res != NSERROR_OK) {
			free_matches(context);
			أرجع res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->وموندل->بل;
		context->newsearch = false;

	} وإلا إذا (context->current != عدم) {
		/* continued search in the direction specified */
		إذا (forwards) {
			إذا (context->current->بل)
				context->current = context->current->بل;
		} وإلا {
			إذا (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_خپرونه(context,
			     CONTENT_TEXTSEARCH_سمون,
			     (context->current != عدم),
			     عدم);

	پلټنه_show_all(showall, context);

	/* update back state */
	textsearch_خپرونه(context,
			     CONTENT_TEXTSEARCH_شا,
			     ((context->current != عدم) &&
			      (context->current->prev != عدم)),
			     عدم);

	/* update forward state */
	textsearch_خپرونه(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != عدم) &&
			      (context->current->بل != عدم)),
			     عدم);


	إذا (context->current == عدم) {
		/* no current match */
		أرجع res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->پیل_idx,
					context->current->پای_ټکی_idx,
					context->current->پیل_box,
					context->current->پای_ټکی_box,
					&bounds);
	إذا (res == NSERROR_OK) {
		پیغام_معلومات.scroll.area = true;
		پیغام_معلومات.scroll.x0 = bounds.x0;
		پیغام_معلومات.scroll.y0 = bounds.y0;
		پیغام_معلومات.scroll.x1 = bounds.x1;
		پیغام_معلومات.scroll.y1 = bounds.y1;
		content_خپرونه(context->c, CONTENT_پیغام_SCROLL, &پیغام_معلومات);
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
content_textsearch_ګام(بنية textsearch_context *textsearch,
			پلټنه_flags_t flags,
			ثابت حرف *string)
{
	صحيح string_len;
	صحيح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != عدم);

	/* broadcast recent query string */
	textsearch_خپرونه(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = مزي_اوږدوالی(string);
	لكل (i = 0; i < string_len; i++) {
		إذا (string[i] != '#' && string[i] != '*')
			اكسر;
	}

	إذا (i < string_len) {
		res = پلټنه_متن(textsearch, string, string_len, flags);
	} وإلا {
		اتحاد content_پیغام_معلومات پیغام_معلومات;

		free_matches(textsearch);

		/* update match state */
		textsearch_خپرونه(textsearch,
				     CONTENT_TEXTSEARCH_سمون,
				     true,
				     عدم);

		/* update back state */
		textsearch_خپرونه(textsearch,
				     CONTENT_TEXTSEARCH_شا,
				     false,
				     عدم);

		/* update forward state */
		textsearch_خپرونه(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     عدم);

		/* clear scroll */
		پیغام_معلومات.scroll.area = false;
		پیغام_معلومات.scroll.x0 = 0;
		پیغام_معلومات.scroll.y0 = 0;
		content_خپرونه(textsearch->c,
				  CONTENT_پیغام_SCROLL,
				  &پیغام_معلومات);
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
			  فراغ *gui_معلومات,
			  بنية textsearch_context **textsearch_بهر)
{
	بنية textsearch_context *context;
	بنية list_ثبت *پلټنه_سر;
	content_type type;

	إذا ((c->handler->textsearch_پیدا_کړه == عدم) ||
	    (c->handler->textsearch_bounds == عدم)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = حافظه_ورکړه(حجم(بنية textsearch_context));
	إذا (context == عدم) {
		أرجع NSERROR_NOMEM;
	}

	پلټنه_سر = حافظه_ورکړه(حجم(بنية list_ثبت));
	إذا (پلټنه_سر == عدم) {
		حرر(context);
		أرجع NSERROR_NOMEM;
	}

	پلټنه_سر->پیل_idx = 0;
	پلټنه_سر->پای_ټکی_idx = 0;
	پلټنه_سر->پیل_box = عدم;
	پلټنه_سر->پای_ټکی_box = عدم;
	پلټنه_سر->sel = عدم;
	پلټنه_سر->prev = عدم;
	پلټنه_سر->بل = عدم;

	context->وموندل = پلټنه_سر;
	context->current = عدم;
	context->string = عدم;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_معلومات;

	*textsearch_بهر = context;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_پیدا_کړه_نمونه(ثابت حرف *string,
				صحيح s_len,
				ثابت حرف *نمونه,
				صحيح p_len,
				bool case_sens,
				بینښه صحيح *m_len)
{
	بنية { ثابت حرف *ss, *s, *p; bool لومړی; } context[16];
	ثابت حرف *ep = نمونه + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = نمونه - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool لومړی = true;
	صحيح سر = 0;

	طالما (p < ep) {
		bool matches;
		إذا (p < نمونه || *p == '*') {
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
				إذا (!case_sens) ch = ascii_منزل_پاسنی(ch);
				طالما (s < es) {
					إذا (case_sens) {
						إذا (*s == ch) اكسر;
					} وإلا إذا (ascii_منزل_پاسنی(*s) == ch)
						اكسر;
					s++;
				}
			}

			إذا (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				إذا (سر < (صحيح)NOF_ELEMENTS(context)) {
					context[سر].ss = ss;
					context[سر].s  = s + 1;
					context[سر].p  = p - 1;
					/* ptr to last asterisk */
					context[سر].لومړی = لومړی;
					سر++;
				}

				إذا (لومړی) {
					ss = s;
					/* remember first non-'*' char */
					لومړی = false;
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
					matches = (ascii_منزل_پاسنی(*s) == ascii_منزل_پاسنی(ch));
			}
			إذا (matches && لومړی) {
				ss = s;  /* remember first non-'*' char */
				لومړی = false;
			}
		} وإلا {
			matches = false;
		}

		إذا (matches) {
			p++; s++;
		} وإلا {
			/* doesn't match,
			 * resume with stacked context if we have one */
			إذا (--سر < 0)
				أرجع عدم;  /* no match, give up */

			ss = context[سر].ss;
			s  = context[سر].s;
			p  = context[سر].p;
			لومړی = context[سر].لومړی;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	أرجع ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_جمع_کړه_سمون(بنية textsearch_context *context,
			     بینښه پیل_idx,
			     بینښه پای_ټکی_idx,
			     بنية box *پیل_box,
			     بنية box *پای_ټکی_box)
{
	بنية list_ثبت *ثبت;

	/* found string in box => add to list */
	ثبت = حافظه_صفر_ورکړه(1, حجم(*ثبت));
	إذا (ثبت == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ثبت->پیل_idx = پیل_idx;
	ثبت->پای_ټکی_idx = پای_ټکی_idx;
	ثبت->پیل_box = پیل_box;
	ثبت->پای_ټکی_box = پای_ټکی_box;
	ثبت->sel = عدم;

	ثبت->بل = عدم;
	ثبت->prev = context->وموندل->prev;

	إذا (context->وموندل->prev == عدم) {
		context->وموندل->بل = ثبت;
	} وإلا {
		context->وموندل->prev->بل = ثبت;
	}

	context->وموندل->prev = ثبت;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(بنية textsearch_context *textsearch,
				 بینښه پیل_دننه_واټن,
				 بینښه پای_ټکی_دننه_واټن,
				 بینښه *پیل_idx,
				 بینښه *پای_ټکی_idx)
{
	بنية list_ثبت *cur;

	لكل (cur = textsearch->وموندل->بل; cur != عدم; cur = cur->بل) {
		إذا (cur->sel &&
		    selection_highlighted(cur->sel,
					  پیل_دننه_واټن,
					  پای_ټکی_دننه_واټن,
					  پیل_idx,
					  پای_ټکی_idx)) {
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
		textsearch_خپرونه(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		حرر(textsearch->string);
	}

	/* update back state */
	textsearch_خپرونه(textsearch,
			     CONTENT_TEXTSEARCH_شا,
			     true,
			     عدم);

	/* update forward state */
	textsearch_خپرونه(textsearch,
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
		   پلټنه_flags_t flags,
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
		content_textsearch_ګام(c->textsearch.context, flags, string);

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

		content_textsearch_ګام(c->textsearch.context, flags, string);

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
