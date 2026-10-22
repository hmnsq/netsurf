/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf.
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
 * content generator for the about scheme query privacy page
 */

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "content/fetch.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "query.h"
#અન્તર્ભાવ "query_fetcherror.h"

/**
 * Handler to generate about scheme fetch error query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_fetcherror_handler(સંરચના fetch_about_context *ctx)
{
	nserror res;
	અક્ષર *url_s;
	size_t url_l;
	અચલ અક્ષર *reason = "";
	અચલ અક્ષર *title;
	સંરચના nsurl *siteurl = ખાલી;
	અક્ષર *description = ખાલી;
	અચલ સંરચના fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	જ્યારે (curmd != ખાલી) {
		જો (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->કિંમત, &siteurl);
			જો (res != NSERROR_OK) {
				પાછા fetch_about_srverror(ctx);
			}
		} નહીંતર જો (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->કિંમત;
		}
		curmd = curmd->next;
	}

	જો (siteurl == ખાલી) {
		પાછા fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	title = messages_get("FetchErrorTitle");
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>%s</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\" id =\"fetcherror\">\n"
			"<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n",
			title, title);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "FetchErrorDescription",
				    &description);
	જો (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		મુક્ત(description);
		જો (res != NSERROR_OK) {
			જાઓ fetch_about_query_fetcherror_handler_aborted;
		}
	}
	res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", reason);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<div id=\"buttons\">"
			 "<input type=\"submit\" id=\"back\" name=\"back\" "
			 "value=\"%s\" class=\"default-action\">"
			 "<input type=\"submit\" id=\"retry\" name=\"retry\" "
			 "value=\"%s\">"
			 "</div>",
			 messages_get("Backtoprevious"),
			 messages_get("TryAgain"));
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	જો (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	મુક્ત(url_s);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_fetcherror_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	પાછા true;

fetch_about_query_fetcherror_handler_aborted:
	nsurl_unref(siteurl);

	પાછા false;
}
