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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "desktop/selection.h"

#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "content/textsearch.h"

/**
 * search match
 */
నిర్మాణం list_నమోదు {
	/**
	 * previous match
	 */
	నిర్మాణం list_నమోదు *prev;

	/**
	 * next match
	 */
	నిర్మాణం list_నమోదు *తదుపరి;

	/**
	 * start position of match
	 */
	గుర్తులేని ప్రారంభం_idx;

	/**
	 * end of match
	 */
	గుర్తులేని చివర_idx;

	/**
	 * content opaque start pointer
	 */
	నిర్మాణం box *ప్రారంభం_box;

	/**
	 * content opaque end pointer
	 */
	నిర్మాణం box *చివర_box;

	/**
	 * content specific selection object
	 */
	నిర్మాణం selection *sel;
};

/**
 * The context for a free text search
 */
నిర్మాణం textsearch_context {

	/**
	 * content search was performed upon
	 */
	నిర్మాణం content *c;

	/**
	 * opaque pointer passed to constructor.
	 */
	శూన్య *gui_p;

	/**
	 * List of matches
	 */
	నిర్మాణం list_నమోదు *దొరికింది;

	/**
	 * current selected match
	 */
	నిర్మాణం list_నమోదు *current; /* first for select all */

	/**
	 * query string search results are for
	 */
	అక్షరం *string;
	bool prev_case_sens;
	bool newsearch;
};


/**
 * broadcast textsearch message
 */
స్థిర అంతఃవరుస శూన్య
textsearch_ప్రసారం(నిర్మాణం textsearch_context *textsearch,
		     పూర్ణాంకం type,
		     bool state,
		     స్థిరరాశి అక్షరం *string)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	సందేశం_దత్తాంశం.textsearch.type = type;
	సందేశం_దత్తాంశం.textsearch.ctx = textsearch->gui_p;
	సందేశం_దత్తాంశం.textsearch.state = state;
	సందేశం_దత్తాంశం.textsearch.string = string;
	content_ప్రసారం(textsearch->c, CONTENT_సందేశం_TEXTSEARCH, &సందేశం_దత్తాంశం);
}


/**
 * Release the memory used by the list of matches,
 * deleting selection objects too
 */
స్థిర శూన్య free_matches(నిర్మాణం textsearch_context *textsearch)
{
	నిర్మాణం list_నమోదు *cur;
	నిర్మాణం list_నమోదు *nxt;

	cur = textsearch->దొరికింది->తదుపరి;

	/*
	 * empty the list before clearing and deleting the selections
	 * because the the clearing may update the toolkit immediately,
	 * causing nested accesses to the list
	 */

	textsearch->దొరికింది->prev = ఖాళీ;
	textsearch->దొరికింది->తదుపరి = ఖాళీ;

	కొరకు (; cur; cur = nxt) {
		nxt = cur->తదుపరి;
		ఒకవేళ (cur->sel) {
			selection_destroy(cur->sel);
		}
		విడుదల(cur);
	}
}


/**
 * Specifies whether all matches or just the current match should
 * be highlighted in the search text.
 */
స్థిర శూన్య శోధన_show_all(bool all, నిర్మాణం textsearch_context *context)
{
	నిర్మాణం list_నమోదు *a;

	కొరకు (a = context->దొరికింది->తదుపరి; a; a = a->తదుపరి) {
		bool కూడు = true;
		ఒకవేళ (!all && a != context->current) {
			కూడు = false;
			ఒకవేళ (a->sel) {
				selection_destroy(a->sel);
				a->sel = ఖాళీ;
			}
		}

		ఒకవేళ (కూడు && !a->sel) {

			a->sel = selection_create(context->c);
			ఒకవేళ (a->sel != ఖాళీ) {
				selection_init(a->sel);
				selection_సమితి_position(a->sel,
						       a->ప్రారంభం_idx,
						       a->చివర_idx);
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
స్థిర nserror
శోధన_పాఠ్యం(నిర్మాణం textsearch_context *context,
	    స్థిరరాశి అక్షరం *string,
	    పూర్ణాంకం string_len,
	    శోధన_flags_t flags)
{
	నిర్మాణం rect bounds;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	bool case_sensitive, forwards, showall;
	nserror res = NSERROR_OK;

	case_sensitive = ((flags & శోధన_FLAG_CASE_SENSITIVE) != 0) ?
			true : false;
	forwards = ((flags & శోధన_FLAG_FORWARDS) != 0) ? true : false;
	showall = ((flags & శోధన_FLAG_SHOWALL) != 0) ? true : false;

	ఒకవేళ (context->c == ఖాళీ) {
		తిరిగి res;
	}

	/* check if we need to start a new search or continue an old one */
	ఒకవేళ ((context->newsearch) ||
	    (context->prev_case_sens != case_sensitive)) {

		ఒకవేళ (context->string != ఖాళీ) {
			విడుదల(context->string);
		}

		context->current = ఖాళీ;
		free_matches(context);

		context->string = జ్ఞాపకం_ఇవ్వు(string_len + 1);
		ఒకవేళ (context->string != ఖాళీ) {
			జ్ఞాపకం_నకలు(context->string, string, string_len);
			context->string[string_len] = '\0';
		}

		/* indicate find operation starting */
		textsearch_ప్రసారం(context, CONTENT_TEXTSEARCH_కనుగొను, true, ఖాళీ);


		/* call content find handler */
		res = context->c->handler->textsearch_కనుగొను(context->c,
							   context,
							   string,
							   string_len,
							   case_sensitive);

		/* indicate find operation finished */
		textsearch_ప్రసారం(context, CONTENT_TEXTSEARCH_కనుగొను, false, ఖాళీ);

		ఒకవేళ (res != NSERROR_OK) {
			free_matches(context);
			తిరిగి res;
		}

		context->prev_case_sens = case_sensitive;

		/* new search, beginning at the top of the page */
		context->current = context->దొరికింది->తదుపరి;
		context->newsearch = false;

	} లేకపోతే ఒకవేళ (context->current != ఖాళీ) {
		/* continued search in the direction specified */
		ఒకవేళ (forwards) {
			ఒకవేళ (context->current->తదుపరి)
				context->current = context->current->తదుపరి;
		} లేకపోతే {
			ఒకవేళ (context->current->prev)
				context->current = context->current->prev;
		}
	}

	/* update match state */
	textsearch_ప్రసారం(context,
			     CONTENT_TEXTSEARCH_సరిపోలిక,
			     (context->current != ఖాళీ),
			     ఖాళీ);

	శోధన_show_all(showall, context);

	/* update back state */
	textsearch_ప్రసారం(context,
			     CONTENT_TEXTSEARCH_వెనుక,
			     ((context->current != ఖాళీ) &&
			      (context->current->prev != ఖాళీ)),
			     ఖాళీ);

	/* update forward state */
	textsearch_ప్రసారం(context,
			     CONTENT_TEXTSEARCH_FORWARD,
			     ((context->current != ఖాళీ) &&
			      (context->current->తదుపరి != ఖాళీ)),
			     ఖాళీ);


	ఒకవేళ (context->current == ఖాళీ) {
		/* no current match */
		తిరిగి res;
	}

	/* call content match bounds handler */
	res = context->c->handler->textsearch_bounds(context->c,
					context->current->ప్రారంభం_idx,
					context->current->చివర_idx,
					context->current->ప్రారంభం_box,
					context->current->చివర_box,
					&bounds);
	ఒకవేళ (res == NSERROR_OK) {
		సందేశం_దత్తాంశం.scroll.area = true;
		సందేశం_దత్తాంశం.scroll.x0 = bounds.x0;
		సందేశం_దత్తాంశం.scroll.y0 = bounds.y0;
		సందేశం_దత్తాంశం.scroll.x1 = bounds.x1;
		సందేశం_దత్తాంశం.scroll.y1 = bounds.y1;
		content_ప్రసారం(context->c, CONTENT_సందేశం_SCROLL, &సందేశం_దత్తాంశం);
	}

	తిరిగి res;
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
స్థిర nserror
content_textsearch_అడుగు(నిర్మాణం textsearch_context *textsearch,
			శోధన_flags_t flags,
			స్థిరరాశి అక్షరం *string)
{
	పూర్ణాంకం string_len;
	పూర్ణాంకం i = 0;
	nserror res = NSERROR_OK;

	ప్రతిజ్ఞా(textsearch != ఖాళీ);

	/* broadcast recent query string */
	textsearch_ప్రసారం(textsearch,
			     CONTENT_TEXTSEARCH_RECENT,
			     false,
			     string);

	string_len = అక్షరశ్రేణి_పొడవు(string);
	కొరకు (i = 0; i < string_len; i++) {
		ఒకవేళ (string[i] != '#' && string[i] != '*')
			ఆపు;
	}

	ఒకవేళ (i < string_len) {
		res = శోధన_పాఠ్యం(textsearch, string, string_len, flags);
	} లేకపోతే {
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

		free_matches(textsearch);

		/* update match state */
		textsearch_ప్రసారం(textsearch,
				     CONTENT_TEXTSEARCH_సరిపోలిక,
				     true,
				     ఖాళీ);

		/* update back state */
		textsearch_ప్రసారం(textsearch,
				     CONTENT_TEXTSEARCH_వెనుక,
				     false,
				     ఖాళీ);

		/* update forward state */
		textsearch_ప్రసారం(textsearch,
				     CONTENT_TEXTSEARCH_FORWARD,
				     false,
				     ఖాళీ);

		/* clear scroll */
		సందేశం_దత్తాంశం.scroll.area = false;
		సందేశం_దత్తాంశం.scroll.x0 = 0;
		సందేశం_దత్తాంశం.scroll.y0 = 0;
		content_ప్రసారం(textsearch->c,
				  CONTENT_సందేశం_SCROLL,
				  &సందేశం_దత్తాంశం);
	}

	తిరిగి res;
}


/**
 * Terminate a search.
 *
 * \param c content to clear
 */
స్థిర nserror content_textsearch__clear(నిర్మాణం content *c)
{
	విడుదల(c->textsearch.string);
	c->textsearch.string = ఖాళీ;

	ఒకవేళ (c->textsearch.context != ఖాళీ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ఖాళీ;
	}
	తిరిగి NSERROR_OK;
}


/**
 * create a search_context
 *
 * \param c The content the search_context is connected to
 * \param context A context pointer passed to the provider routines.
 * \param search_out A pointer to recive the new text search context
 * \return NSERROR_OK on success and \a search_out updated else error code
 */
స్థిర nserror
content_textsearch_create(నిర్మాణం content *c,
			  శూన్య *gui_దత్తాంశం,
			  నిర్మాణం textsearch_context **textsearch_బహిర్గతం)
{
	నిర్మాణం textsearch_context *context;
	నిర్మాణం list_నమోదు *శోధన_తల;
	content_type type;

	ఒకవేళ ((c->handler->textsearch_కనుగొను == ఖాళీ) ||
	    (c->handler->textsearch_bounds == ఖాళీ)) {
		/*
		 * content has no free text find handler so searching
		 *   is unsupported.
		 */
		తిరిగి NSERROR_NOT_IMPLEMENTED;
	}

	type = c->handler->type();

	context = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం textsearch_context));
	ఒకవేళ (context == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	శోధన_తల = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం list_నమోదు));
	ఒకవేళ (శోధన_తల == ఖాళీ) {
		విడుదల(context);
		తిరిగి NSERROR_NOMEM;
	}

	శోధన_తల->ప్రారంభం_idx = 0;
	శోధన_తల->చివర_idx = 0;
	శోధన_తల->ప్రారంభం_box = ఖాళీ;
	శోధన_తల->చివర_box = ఖాళీ;
	శోధన_తల->sel = ఖాళీ;
	శోధన_తల->prev = ఖాళీ;
	శోధన_తల->తదుపరి = ఖాళీ;

	context->దొరికింది = శోధన_తల;
	context->current = ఖాళీ;
	context->string = ఖాళీ;
	context->prev_case_sens = false;
	context->newsearch = true;
	context->c = c;
	context->gui_p = gui_దత్తాంశం;

	*textsearch_బహిర్గతం = context;

	తిరిగి NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
స్థిరరాశి అక్షరం *
content_textsearch_కనుగొను_నమూనా(స్థిరరాశి అక్షరం *string,
				పూర్ణాంకం s_len,
				స్థిరరాశి అక్షరం *నమూనా,
				పూర్ణాంకం p_len,
				bool case_sens,
				గుర్తులేని పూర్ణాంకం *m_len)
{
	నిర్మాణం { స్థిరరాశి అక్షరం *ss, *s, *p; bool మొదటి; } context[16];
	స్థిరరాశి అక్షరం *ep = నమూనా + p_len;
	స్థిరరాశి అక్షరం *es = string  + s_len;
	స్థిరరాశి అక్షరం *p = నమూనా - 1;  /* a virtual '*' before the pattern */
	స్థిరరాశి అక్షరం *ss = string;
	స్థిరరాశి అక్షరం *s = string;
	bool మొదటి = true;
	పూర్ణాంకం శిఖరం = 0;

	అయితే (p < ep) {
		bool matches;
		ఒకవేళ (p < నమూనా || *p == '*') {
			అక్షరం ch;

			/* skip any further asterisks; one is the same as many
			*/
			చేయి p++; అయితే (p < ep && *p == '*');

			/* if we're at the end of the pattern, yes, it matches
			*/
			ఒకవేళ (p >= ep) ఆపు;

			/* anything matches a # so continue matching from
			   here, and stack a context that will try to match
			   the wildcard against the next character */

			ch = *p;
			ఒకవేళ (ch != '#') {
				/* scan forwards until we find a match for
				   this char */
				ఒకవేళ (!case_sens) ch = ascii_గమ్యం_పై(ch);
				అయితే (s < es) {
					ఒకవేళ (case_sens) {
						ఒకవేళ (*s == ch) ఆపు;
					} లేకపోతే ఒకవేళ (ascii_గమ్యం_పై(*s) == ch)
						ఆపు;
					s++;
				}
			}

			ఒకవేళ (s < es) {
				/* remember where we are in case the match
				   fails; we may then resume */
				ఒకవేళ (శిఖరం < (పూర్ణాంకం)NOF_ELEMENTS(context)) {
					context[శిఖరం].ss = ss;
					context[శిఖరం].s  = s + 1;
					context[శిఖరం].p  = p - 1;
					/* ptr to last asterisk */
					context[శిఖరం].మొదటి = మొదటి;
					శిఖరం++;
				}

				ఒకవేళ (మొదటి) {
					ss = s;
					/* remember first non-'*' char */
					మొదటి = false;
				}

				matches = true;
			} లేకపోతే {
				matches = false;
			}

		} లేకపోతే ఒకవేళ (s < es) {
			అక్షరం ch = *p;
			ఒకవేళ (ch == '#')
				matches = true;
			లేకపోతే {
				ఒకవేళ (case_sens)
					matches = (*s == ch);
				లేకపోతే
					matches = (ascii_గమ్యం_పై(*s) == ascii_గమ్యం_పై(ch));
			}
			ఒకవేళ (matches && మొదటి) {
				ss = s;  /* remember first non-'*' char */
				మొదటి = false;
			}
		} లేకపోతే {
			matches = false;
		}

		ఒకవేళ (matches) {
			p++; s++;
		} లేకపోతే {
			/* doesn't match,
			 * resume with stacked context if we have one */
			ఒకవేళ (--శిఖరం < 0)
				తిరిగి ఖాళీ;  /* no match, give up */

			ss = context[శిఖరం].ss;
			s  = context[శిఖరం].s;
			p  = context[శిఖరం].p;
			మొదటి = context[శిఖరం].మొదటి;
		}
	}

	/* end of pattern reached */
	*m_len = max(s - ss, 1);
	తిరిగి ss;
}


/* exported interface, documented in content/textsearch.h */
nserror
content_textsearch_కూడు_సరిపోలిక(నిర్మాణం textsearch_context *context,
			     గుర్తులేని ప్రారంభం_idx,
			     గుర్తులేని చివర_idx,
			     నిర్మాణం box *ప్రారంభం_box,
			     నిర్మాణం box *చివర_box)
{
	నిర్మాణం list_నమోదు *నమోదు_2;

	/* found string in box => add to list */
	నమోదు_2 = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*నమోదు_2));
	ఒకవేళ (నమోదు_2 == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	నమోదు_2->ప్రారంభం_idx = ప్రారంభం_idx;
	నమోదు_2->చివర_idx = చివర_idx;
	నమోదు_2->ప్రారంభం_box = ప్రారంభం_box;
	నమోదు_2->చివర_box = చివర_box;
	నమోదు_2->sel = ఖాళీ;

	నమోదు_2->తదుపరి = ఖాళీ;
	నమోదు_2->prev = context->దొరికింది->prev;

	ఒకవేళ (context->దొరికింది->prev == ఖాళీ) {
		context->దొరికింది->తదుపరి = నమోదు_2;
	} లేకపోతే {
		context->దొరికింది->prev->తదుపరి = నమోదు_2;
	}

	context->దొరికింది->prev = నమోదు_2;

	తిరిగి NSERROR_OK;
}


/* exported interface, documented in content/textsearch.h */
bool
content_textsearch_ishighlighted(నిర్మాణం textsearch_context *textsearch,
				 గుర్తులేని ప్రారంభం_స్థానభ్రంశం,
				 గుర్తులేని చివర_స్థానభ్రంశం,
				 గుర్తులేని *ప్రారంభం_idx,
				 గుర్తులేని *చివర_idx)
{
	నిర్మాణం list_నమోదు *cur;

	కొరకు (cur = textsearch->దొరికింది->తదుపరి; cur != ఖాళీ; cur = cur->తదుపరి) {
		ఒకవేళ (cur->sel &&
		    selection_highlighted(cur->sel,
					  ప్రారంభం_స్థానభ్రంశం,
					  చివర_స్థానభ్రంశం,
					  ప్రారంభం_idx,
					  చివర_idx)) {
			తిరిగి true;
		}
	}

	తిరిగి false;
}


/* exported interface, documented in content/textsearch.h */
nserror content_textsearch_destroy(నిర్మాణం textsearch_context *textsearch)
{
	ప్రతిజ్ఞా(textsearch != ఖాళీ);

	ఒకవేళ (textsearch->string != ఖాళీ) {
		/* broadcast recent query string */
		textsearch_ప్రసారం(textsearch,
				     CONTENT_TEXTSEARCH_RECENT,
				     false,
				     textsearch->string);

		విడుదల(textsearch->string);
	}

	/* update back state */
	textsearch_ప్రసారం(textsearch,
			     CONTENT_TEXTSEARCH_వెనుక,
			     true,
			     ఖాళీ);

	/* update forward state */
	textsearch_ప్రసారం(textsearch,
			     CONTENT_TEXTSEARCH_FORWARD,
			     true,
			     ఖాళీ);

	free_matches(textsearch);
	విడుదల(textsearch);

	తిరిగి NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror
content_textsearch(నిర్మాణం hlcache_handle *h,
		   శూన్య *context,
		   శోధన_flags_t flags,
		   స్థిరరాశి అక్షరం *string)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	nserror res;

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (string != ఖాళీ &&
	    c->textsearch.string != ఖాళీ &&
	    c->textsearch.context != ఖాళీ &&
	    strcmp(string, c->textsearch.string) == 0) {
		/* Continue prev. search */
		content_textsearch_అడుగు(c->textsearch.context, flags, string);

	} లేకపోతే ఒకవేళ (string != ఖాళీ) {
		/* New search */
		విడుదల(c->textsearch.string);
		c->textsearch.string = strdup(string);
		ఒకవేళ (c->textsearch.string == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}

		ఒకవేళ (c->textsearch.context != ఖాళీ) {
			content_textsearch_destroy(c->textsearch.context);
			c->textsearch.context = ఖాళీ;
		}

		res = content_textsearch_create(c,
						context,
						&c->textsearch.context);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}

		content_textsearch_అడుగు(c->textsearch.context, flags, string);

	} లేకపోతే {
		/* Clear search */
		content_textsearch__clear(c);

		విడుదల(c->textsearch.string);
		c->textsearch.string = ఖాళీ;
	}

	తిరిగి NSERROR_OK;
}


/* exported interface, documented in content/content.h */
nserror content_textsearch_clear(నిర్మాణం hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	తిరిగి(content_textsearch__clear(c));
}
