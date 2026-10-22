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
ساخت list_اندراج {
	/**
	 * previous match
	 */
	ساخت list_اندراج *پچھلا;

	/**
	 * next match
	 */
	ساخت list_اندراج *اگلا;

	/**
	 * start position of match
	 */
	غیرعلامتی آغاز_idx;

	/**
	 * end of match
	 */
	غیرعلامتی انتہا_idx;

	/**
	 * content opaque start pointer
	 */
	ساخت box *آغاز_box;

	/**
	 * content opaque end pointer
	 */
	ساخت box *انتہا_box;

	/**
	 * content specific selection object
	 */
	ساخت selection *sel;
};

/**
 * The context for a free text search
 */
ساخت textsearch_context {

	/**
	 * content search was performed upon
	 */
	ساخت content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	خالی *gui_p;

	/**
	 * List of matches
	 */
	ساخت list_اندراج *ملا;

	/**
	 * current selected match
	 */
	ساخت list_اندراج *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	حرف *string;
	bool پچھلا_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
ساکن اندرونی خالی
textsearch_نشر(ساخت textsearch_context *textsearch,
		     صحیح type,
		     bool state,
		     ثابت حرف *string)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	پیغام_ڈیٹا.textsearch.type = type;
	پیغام_ڈیٹا.textsearch.ctx = textsearch->gui_p;
	پیغام_ڈیٹا.textsearch.state = state;
	پیغام_ڈیٹا.textsearch.string = string;
	content_نشر(textsearch->c, CONTENT_پیغام_TEXTSEARCH, &پیغام_ڈیٹا);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
ساکن خالی free_matches(ساخت textsearch_context *textsearch)
{
	ساخت list_اندراج *cur;
	ساخت list_اندراج *nxt;

	cur = textsearch->ملا->اگلا;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->ملا->پچھلا = تہی;
	textsearch->ملا->اگلا = تہی;

	برائے (; cur; cur = nxt) {
		nxt = cur->اگلا;
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
ساکن خالی تلاش_show_all(bool all, ساخت textsearch_context *context)
{
	ساخت list_اندراج *a;

	برائے (a = context->ملا->اگلا; a; a = a->اگلا) {
		bool جوڑو = true;
		اگر (!all && a != context->current) {
			جوڑو = false;
			اگر (a->sel) {
				selection_destroy(a->sel);
				a->sel = تہی;
			}
		}

		اگر (جوڑو && !a->sel) {

			a->sel = selection_create(context->c);
			اگر (a->sel != تہی) {
				selection_init(a->sel);
				selection_سیٹ_position(a->sel,
						       a->آغاز_idx,
						       a->انتہا_idx);
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
ساکن nserror
تلاش_متن(ساخت textsearch_context *context,
	    ثابت حرف *string,
	    صحیح string_len,
	    تلاش_flags_t flags)
{
	ساخت rect bounds;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & تلاش_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & تلاش_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & تلاش_FLAG_SHOWALL) != 0) ? true : false;

	اگر (context->c == تہی) {
		واپس res;
	}

	/* check if we need to start a new search or continue an old one */
	اگر ((context->newsearch) ||
	    (context->پچھلا_case_sens != case_sensitive)) {

		اگر (context->string != تہی) {
			آزاد(context->string);
		}

		context->current = تہی;
		free_matches(context);

		context->string = حافظہ_دو(string_len + 1);
		اگر (context->string != تہی) {
			حافظہ_نقل(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_ڈھونڈو, true, تہی);


		/* call content find handler */
		res = context->c->handler->textsearch_ڈھونڈو(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_نشر(context, CONTENT_TEXTSEARCH_ڈھونڈو, false, تہی);

		اگر (res != NSERROR_OK) {
			free_matches(context);
			واپس res;
		}

		context->پچھلا_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->ملا->اگلا;
		context->newsearch = false;

	} ورنہ اگر (context->current != تہی) {
		/* continued search in the direction specified */
		اگر (forwards) {
			اگر (context->current->اگلا)
				context->current = context->current->اگلا;
		} ورنہ {
			اگر (context->current->پچھلا)
				context->current = context->current->پچھلا;
		}
	}

	/* update match state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_مطابقت,
			     (context->current != تہی),
			     تہی);

	تلاش_show_all(showall, context);

	/* update back state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_عقب,
			     ((context->current != تہی) &&
			      (context->current->پچھلا != تہی)),
			     تہی);

	/* update forward state */
	textsearch_نشر(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != تہی) &&
			      (context->current->اگلا != تہی)),
			     تہی);


	اگر (context->current == تہی) {
		/* no current match */
		واپس res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->آغاز_idx,
					context->current->انتہا_idx,
					context->current->آغاز_box,
					context->current->انتہا_box,
					&bounds);
	اگر (res == NSERROR_OK) {
		پیغام_ڈیٹا.scroll.area = true;
		پیغام_ڈیٹا.scroll.x0 = bounds.x0;
		پیغام_ڈیٹا.scroll.y0 = bounds.y0;
		پیغام_ڈیٹا.scroll.x1 = bounds.x1;
		پیغام_ڈیٹا.scroll.y1 = bounds.y1;
		content_نشر(context->c, CONTENT_پیغام_SCROLL, &پیغام_ڈیٹا);
	}

	واپس res;
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
ساکن nserror
content_textsearch_قدم(ساخت textsearch_context *textsearch,
			تلاش_flags_t flags,
			ثابت حرف *string)
{
	صحیح string_len;
	صحیح i = 0;
	nserror res = NSERROR_OK;

	أكد(textsearch != تہی);

	/* broadcast recent query string */
	textsearch_نشر(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = لڑی_لمبائی(string);
	برائے (i = 0; i < string_len; i++) {
		اگر (string[i] != '#' && string[i] != '*')
			توڑو;
	}

	اگر (i < string_len) {
		res = تلاش_متن(textsearch, string, string_len, flags);
	} ورنہ {
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

		free_matches(textsearch);

		/* update match state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_مطابقت,
				     true,
				     تہی);

		/* update back state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_عقب,
				     false,
				     تہی);

		/* update forward state */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     تہی);

		/* clear scroll */
		پیغام_ڈیٹا.scroll.area = false;
		پیغام_ڈیٹا.scroll.x0 = 0;
		پیغام_ڈیٹا.scroll.y0 = 0;
		content_نشر(textsearch->c,
				  CONTENT_پیغام_SCROLL,
				  &پیغام_ڈیٹا);
	}

	واپس res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
ساکن nserror content_textsearch__clear(ساخت content *c)
{
	آزاد(c->textsearch.string);
	c->textsearch.string = تہی;

	اگر (c->textsearch.context != تہی) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = تہی;
	}
	واپس NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
ساکن nserror
content_textsearch_create(ساخت content *c,
			  خالی *gui_ڈیٹا,
			  ساخت textsearch_context **textsearch_برآمد)
{
	ساخت textsearch_context *context;
	ساخت list_اندراج *تلاش_سر;
	content_type type;

	اگر ((c->handler->textsearch_ڈھونڈو == تہی) ||
	    (c->handler->textsearch_bounds == تہی)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		واپس NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = حافظہ_دو(حجم(ساخت textsearch_context));
	اگر (context == تہی) {
		واپس NSERROR_NOMEM;
	}

	تلاش_سر = حافظہ_دو(حجم(ساخت list_اندراج));
	اگر (تلاش_سر == تہی) {
		آزاد(context);
		واپس NSERROR_NOMEM;
	}

	تلاش_سر->آغاز_idx = 0;
	تلاش_سر->انتہا_idx = 0;
	تلاش_سر->آغاز_box = تہی;
	تلاش_سر->انتہا_box = تہی;
	تلاش_سر->sel = تہی;
	تلاش_سر->پچھلا = تہی;
	تلاش_سر->اگلا = تہی;

	context->ملا = تلاش_سر;
	context->current = تہی;
	context->string = تہی;
	context->پچھلا_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_ڈیٹا;

	*textsearch_برآمد = context;

	واپس NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
ثابت حرف *
content_textsearch_ڈھونڈو_نمونہ(ثابت حرف *string,
				صحیح s_len,
				ثابت حرف *نمونہ,
				صحیح p_len,
				bool case_sens,
				غیرعلامتی صحیح *m_len)
{
	ساخت { ثابت حرف *ss, *s, *p; bool پہلا; } context[16];
	ثابت حرف *ep = نمونہ + p_len;
	ثابت حرف *es = string  + s_len;
	ثابت حرف *p = نمونہ - 1;  /* a virtual '*' before the pattern */
	ثابت حرف *ss = string;
	ثابت حرف *s = string;
	bool پہلا = true;
	صحیح چوٹی = 0;

	جب (p < ep) {
		bool matches;
		اگر (p < نمونہ || *p == '*') {
			حرف ch;

			/* skip any further asterisks; one is the same as many
			*/
			کرو p++; جب (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			اگر (p >= ep) توڑو;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			اگر (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				اگر (!case_sens) ch = ascii_مقصد_بالائی(ch);
				جب (s < es) {
					اگر (case_sens) {
						اگر (*s == ch) توڑو;
					} ورنہ اگر (ascii_مقصد_بالائی(*s) == ch)
						توڑو;
					s++;
				}
			}

			اگر (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				اگر (چوٹی < (صحیح)NOF_ELEMENTS(context)) {
					context[چوٹی].ss = ss;
					context[چوٹی].s  = s + 1;
					context[چوٹی].p  = p - 1;
					/* ptr to last asterisk */
					context[چوٹی].پہلا = پہلا;
					چوٹی++;
				}

				اگر (پہلا) {
					ss = s;
					/* remember first non-'*' char */
					پہلا = false;
				}

				matches = true;
			} ورنہ {
				matches = false;
			}

		} ورنہ اگر (s < es) {
			حرف ch = *p;
			اگر (ch == '#')
				matches = true;
			ورنہ {
				اگر (case_sens)
					matches = (*s == ch);
				ورنہ
					matches = (ascii_مقصد_بالائی(*s) == ascii_مقصد_بالائی(ch));
			}
			اگر (matches && پہلا) {
				ss = s;  /* remember first non-'*' char */
				پہلا = false;
			}
		} ورنہ {
			matches = false;
		}

		اگر (matches) {
			p++; s++;
		} ورنہ {
			/* doesn't match,
			 * resume with stacked context if we have one */
			اگر (--چوٹی < 0)
				واپس تہی;  /* no match, give up */

			ss = context[چوٹی].ss;
			s  = context[چوٹی].s;
			p  = context[چوٹی].p;
			پہلا = context[چوٹی].پہلا;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	واپس ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_جوڑو_مطابقت(ساخت textsearch_context *context,
			     غیرعلامتی آغاز_idx,
			     غیرعلامتی انتہا_idx,
			     ساخت box *آغاز_box,
			     ساخت box *انتہا_box)
{
	ساخت list_اندراج *اندراج_2;

	/* found string in box => add to list */
	اندراج_2 = صفر_حافظہ_دو(1, حجم(*اندراج_2));
	اگر (اندراج_2 == تہی) {
		واپس NSERROR_NOMEM;
	}

	اندراج_2->آغاز_idx = آغاز_idx;
	اندراج_2->انتہا_idx = انتہا_idx;
	اندراج_2->آغاز_box = آغاز_box;
	اندراج_2->انتہا_box = انتہا_box;
	اندراج_2->sel = تہی;

	اندراج_2->اگلا = تہی;
	اندراج_2->پچھلا = context->ملا->پچھلا;

	اگر (context->ملا->پچھلا == تہی) {
		context->ملا->اگلا = اندراج_2;
	} ورنہ {
		context->ملا->پچھلا->اگلا = اندراج_2;
	}

	context->ملا->پچھلا = اندراج_2;

	واپس NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(ساخت textsearch_context *textsearch,
				 غیرعلامتی آغاز_انحراف,
				 غیرعلامتی انتہا_انحراف,
				 غیرعلامتی *آغاز_idx,
				 غیرعلامتی *انتہا_idx)
{
	ساخت list_اندراج *cur;

	برائے (cur = textsearch->ملا->اگلا; cur != تہی; cur = cur->اگلا) {
		اگر (cur->sel &&
		    selection_highlighted(cur->sel,
					  آغاز_انحراف,
					  انتہا_انحراف,
					  آغاز_idx,
					  انتہا_idx)) {
			واپس true;
		}
	}

	واپس false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(ساخت textsearch_context *textsearch)
{
	أكد(textsearch != تہی);

	اگر (textsearch->string != تہی) {
		/* broadcast recent query string */
		textsearch_نشر(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		آزاد(textsearch->string);
	}

	/* update back state */
	textsearch_نشر(textsearch,
			     CONTENT_TEXTSEARCH_عقب,
			     true,
			     تہی);

	/* update forward state */
	textsearch_نشر(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     تہی);

	free_matches(textsearch);
	آزاد(textsearch);

	واپس NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(ساخت hlcache_handle *h,
		   خالی *context,
		   تلاش_flags_t flags,
		   ثابت حرف *string)
{
	ساخت content *c = hlcache_handle_get_content(h);
	nserror res;

	أكد(c != تہی);

	اگر (string != تہی &&
	    c->textsearch.string != تہی &&
	    c->textsearch.context != تہی &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_قدم(c->textsearch.context, flags, string);

	} ورنہ اگر (string != تہی) {
		/* New search */
		آزاد(c->textsearch.string);
		c->textsearch.string = strdup(string);
		اگر (c->textsearch.string == تہی) {
			واپس NSERROR_NOMEM;
		}

		اگر (c->textsearch.context != تہی) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = تہی;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		اگر (res != NSERROR_OK) {
			واپس res;
		}

		content_textsearch_قدم(c->textsearch.context, flags, string);

	} ورنہ {
		/* Clear search */
		content_textsearch__clear(c);

		آزاد(c->textsearch.string);
		c->textsearch.string = تہی;
	}

	واپس NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(ساخت hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	واپس(content_textsearch__clear(c));
}
