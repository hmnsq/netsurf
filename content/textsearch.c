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
بنية list_داخلا {
	/**
	 * previous match
	 */
	بنية list_داخلا *prev;

	/**
	 * next match
	 */
	بنية list_داخلا *ايندڙ;

	/**
	 * start position of match
	 */
	بنانشان شروعات_idx;

	/**
	 * end of match
	 */
	بنانشان پڇاڙي_idx;

	/**
	 * content opaque start pointer
	 */
	بنية box *شروعات_box;

	/**
	 * content opaque end pointer
	 */
	بنية box *پڇاڙي_box;

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
	بنية list_داخلا *مليو;

	/**
	 * current selected match
	 */
	بنية list_داخلا *current; /* first for select all */

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
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	نياپو_معلومات.textsearch.type = type;
	نياپو_معلومات.textsearch.ctx = textsearch->gui_p;
	نياپو_معلومات.textsearch.state = state;
	نياپو_معلومات.textsearch.string = string;
	content_نشر(textsearch->c, CONTENT_نياپو_TEXTSEARCH, &نياپو_معلومات);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساكن فراغ free_matches(بنية textsearch_context *textsearch)
{
	بنية list_داخلا *cur;
	بنية list_داخلا *nxt;

	cur = textsearch->مليو->ايندڙ;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->مليو->prev = عدم;
	textsearch->مليو->ايندڙ = عدم;

	لكل (; cur; cur = nxt) {
		nxt = cur->ايندڙ;
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
ساكن فراغ ڳولا_show_all(bool all, بنية textsearch_context *context)
{
	بنية list_داخلا *a;

	لكل (a = context->مليو->ايندڙ; a; a = a->ايندڙ) {
		bool جوڙيو = true;
		إذا (!all && a != context->current) {
			جوڙيو = false;
			إذا (a->sel) {
				selection_destroy(a->sel);
				a->sel = عدم;
			}
		}

		إذا (جوڙيو && !a->sel) {

			a->sel = selection_create(context->c);
			إذا (a->sel != عدم) {
				selection_init(a->sel);
				selection_سيٽ_position(a->sel,
						       a->شروعات_idx,
						       a->پڇاڙي_idx);
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
ڳولا_متن(بنية textsearch_context *context,
	    ثابت حرف *string,
	    صحيح string_len,
	    ڳولا_flags_t flags)
{
	بنية rect bounds;
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & ڳولا_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & ڳولا_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & ڳولا_FLAG_SHOWALL) != 0) ? true : false;

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

		context->string = يادگيري_ڏيو(string_len + 1);
		إذا (context->string != عدم) {
			يادگيري_نقل(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_ڳوليو, true, عدم);


		/* call content find handler */
		res = context->c->handler->textsearch_ڳوليو(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_ڳوليو, false, عدم);

		إذا (res != NSERROR_OK) {
			free_matches(context);
			أرجع res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->مليو->ايندڙ;
		context->newsearch = false;

	} وإلا إذا (context->current != عدم) {
		/* continued search in the direction specified */
		إذا (forwards) {
			إذا (context->current->ايندڙ)
				context->current = context->current->ايندڙ;
		} وإلا {
			إذا (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_ميلاپ,
			     (context->current != عدم),
			     عدم);

	ڳولا_show_all(showall, context);

	/* update back state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_پٺ,
			     ((context->current != عدم) &&
			      (context->current->prev != عدم)),
			     عدم);

	/* update forward state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != عدم) &&
			      (context->current->ايندڙ != عدم)),
			     عدم);


	إذا (context->current == عدم) {
		/* no current match */
		أرجع res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->شروعات_idx,
					context->current->پڇاڙي_idx,
					context->current->شروعات_box,
					context->current->پڇاڙي_box,
					&bounds);
	إذا (res == NSERROR_OK) {
		نياپو_معلومات.scroll.area = true;
		نياپو_معلومات.scroll.x0 = bounds.x0;
		نياپو_معلومات.scroll.y0 = bounds.y0;
		نياپو_معلومات.scroll.x1 = bounds.x1;
		نياپو_معلومات.scroll.y1 = bounds.y1;
		content_نشر(context->c, CONTENT_نياپو_SCROLL, &نياپو_معلومات);
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
			ڳولا_flags_t flags,
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

	string_len = لڙي_ڊيگهه(string);
	لكل (i = 0; i < string_len; i++) {
		إذا (string[i] != '#' && string[i] != '*')
			اكسر;
	}

	إذا (i < string_len) {
		res = ڳولا_متن(textsearch, string, string_len, flags);
	} وإلا {
		اتحاد content_نياپو_معلومات نياپو_معلومات;

		free_matches(textsearch);

		/* update match state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_ميلاپ,
				     true,
				     عدم);

		/* update back state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_پٺ,
				     false,
				     عدم);

		/* update forward state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     عدم);

		/* clear scroll */
		نياپو_معلومات.scroll.area = false;
		نياپو_معلومات.scroll.x0 = 0;
		نياپو_معلومات.scroll.y0 = 0;
		content_نشر(textsearch->c,
				  CONTENT_نياپو_SCROLL,
				  &نياپو_معلومات);
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
			  بنية textsearch_context **textsearch_ٻاهريون)
{
	بنية textsearch_context *context;
	بنية list_داخلا *ڳولا_سر;
	content_type type;

	إذا ((c->handler->textsearch_ڳوليو == عدم) ||
	    (c->handler->textsearch_bounds == عدم)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = يادگيري_ڏيو(حجم(بنية textsearch_context));
	إذا (context == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ڳولا_سر = يادگيري_ڏيو(حجم(بنية list_داخلا));
	إذا (ڳولا_سر == عدم) {
		حرر(context);
		أرجع NSERROR_NOMEM;
	}

	ڳولا_سر->شروعات_idx = 0;
	ڳولا_سر->پڇاڙي_idx = 0;
	ڳولا_سر->شروعات_box = عدم;
	ڳولا_سر->پڇاڙي_box = عدم;
	ڳولا_سر->sel = عدم;
	ڳولا_سر->prev = عدم;
	ڳولا_سر->ايندڙ = عدم;

	context->مليو = ڳولا_سر;
	context->current = عدم;
	context->string = عدم;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_معلومات;

	*textsearch_ٻاهريون = context;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_ڳوليو_نمونو(ثابت حرف *string,
				صحيح s_len,
				ثابت حرف *نمونو,
				صحيح p_len,
				bool case_sens,
				بنانشان صحيح *m_len)
{
	بنية { ثابت حرف *ss, *s, *p; bool پهريون; } context[16];
	ثابت حرف *ep = نمونو + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = نمونو - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool پهريون = true;
	صحيح چوٽي = 0;

	طالما (p < ep) {
		bool matches;
		إذا (p < نمونو || *p == '*') {
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
				إذا (!case_sens) ch = ascii_منزل_مٿانهون(ch);
				طالما (s < es) {
					إذا (case_sens) {
						إذا (*s == ch) اكسر;
					} وإلا إذا (ascii_منزل_مٿانهون(*s) == ch)
						اكسر;
					s++;
				}
			}

			إذا (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				إذا (چوٽي < (صحيح)NOF_ELEMENTS(context)) {
					context[چوٽي].ss = ss;
					context[چوٽي].s  = s + 1;
					context[چوٽي].p  = p - 1;
					/* ptr to last asterisk */
					context[چوٽي].پهريون = پهريون;
					چوٽي++;
				}

				إذا (پهريون) {
					ss = s;
					/* remember first non-'*' char */
					پهريون = false;
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
					matches = (ascii_منزل_مٿانهون(*s) == ascii_منزل_مٿانهون(ch));
			}
			إذا (matches && پهريون) {
				ss = s;  /* remember first non-'*' char */
				پهريون = false;
			}
		} وإلا {
			matches = false;
		}

		إذا (matches) {
			p++; s++;
		} وإلا {
			/* doesn't match,
			 * resume with stacked context if we have one */
			إذا (--چوٽي < 0)
				أرجع عدم;  /* no match, give up */

			ss = context[چوٽي].ss;
			s  = context[چوٽي].s;
			p  = context[چوٽي].p;
			پهريون = context[چوٽي].پهريون;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	أرجع ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_جوڙيو_ميلاپ(بنية textsearch_context *context,
			     بنانشان شروعات_idx,
			     بنانشان پڇاڙي_idx,
			     بنية box *شروعات_box,
			     بنية box *پڇاڙي_box)
{
	بنية list_داخلا *داخلا;

	/* found string in box => add to list */
	داخلا = صفر_يادگيري_ڏيو(1, حجم(*داخلا));
	إذا (داخلا == عدم) {
		أرجع NSERROR_NOMEM;
	}

	داخلا->شروعات_idx = شروعات_idx;
	داخلا->پڇاڙي_idx = پڇاڙي_idx;
	داخلا->شروعات_box = شروعات_box;
	داخلا->پڇاڙي_box = پڇاڙي_box;
	داخلا->sel = عدم;

	داخلا->ايندڙ = عدم;
	داخلا->prev = context->مليو->prev;

	إذا (context->مليو->prev == عدم) {
		context->مليو->ايندڙ = داخلا;
	} وإلا {
		context->مليو->prev->ايندڙ = داخلا;
	}

	context->مليو->prev = داخلا;

	أرجع NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(بنية textsearch_context *textsearch,
				 بنانشان شروعات_وٿي,
				 بنانشان پڇاڙي_وٿي,
				 بنانشان *شروعات_idx,
				 بنانشان *پڇاڙي_idx)
{
	بنية list_داخلا *cur;

	لكل (cur = textsearch->مليو->ايندڙ; cur != عدم; cur = cur->ايندڙ) {
		إذا (cur->sel &&
		    selection_highlighted(cur->sel,
					  شروعات_وٿي,
					  پڇاڙي_وٿي,
					  شروعات_idx,
					  پڇاڙي_idx)) {
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
			     CONTENT_TEXTSEARCH_پٺ,
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
		   ڳولا_flags_t flags,
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
