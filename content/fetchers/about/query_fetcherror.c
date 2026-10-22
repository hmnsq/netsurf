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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "utils/errors.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "content/fetch.h"

#অন্তর্ভাৱ "private.h"
#অন্তর্ভাৱ "query.h"
#অন্তর্ভাৱ "query_fetcherror.h"

/**
 * Handler to generate about scheme fetch error query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_fetcherror_handler(সংরচনা fetch_about_context *ctx)
{
	nserror res;
	ৱর্ণ *url_s;
	size_t url_l;
	ধ্রুৱ ৱর্ণ *reason = "";
	ধ্রুৱ ৱর্ণ *title;
	সংরচনা nsurl *siteurl = রিক্ত;
	ৱর্ণ *description = রিক্ত;
	ধ্রুৱ সংরচনা fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	যেতিয়ালৈ (curmd != রিক্ত) {
		যদি (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->মান, &siteurl);
			যদি (res != NSERROR_OK) {
				ঘূৰাই fetch_about_srverror(ctx);
			}
		} অন্যথা যদি (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->মান;
		}
		curmd = curmd->next;
	}

	যদি (siteurl == রিক্ত) {
		ঘূৰাই fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
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
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "FetchErrorDescription",
				    &description);
	যদি (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		মুক্ত(description);
		যদি (res != NSERROR_OK) {
			গচ্ছ fetch_about_query_fetcherror_handler_aborted;
		}
	}
	res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", reason);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
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
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	যদি (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	মুক্ত(url_s);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_fetcherror_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	ঘূৰাই true;

fetch_about_query_fetcherror_handler_aborted:
	nsurl_unref(siteurl);

	ঘূৰাই false;
}
