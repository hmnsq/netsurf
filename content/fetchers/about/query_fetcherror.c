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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query_fetcherror.h"

/**
 * Handler to generate about scheme fetch error query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_fetcherror_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	nserror res;
	ਅੱਖਰ *url_s;
	size_t url_l;
	ਅਚਲ ਅੱਖਰ *reason = "";
	ਅਚਲ ਅੱਖਰ *title;
	ਬਣਤਰ nsurl *siteurl = ਨਿਰਾ;
	ਅੱਖਰ *description = ਨਿਰਾ;
	ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ਜਦੋਂ (curmd != ਨਿਰਾ) {
		ਜੇ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->ਮੁੱਲ, &siteurl);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ fetch_about_srverror(ctx);
			}
		} ਨਹੀਂਤਾਂ ਜੇ (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->ਮੁੱਲ;
		}
		curmd = curmd->ਅਗਲਾ;
	}

	ਜੇ (siteurl == ਨਿਰਾ) {
		ਵਾਪਸ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_ਸੈੱਟ_http_ਕੋਡ(ctx, 200);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html; charset=utf-8")) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
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
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "FetchErrorDescription",
				    &description);
	ਜੇ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		ਮੁਕਤ(description);
		ਜੇ (res != NSERROR_OK) {
			ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
		}
	}
	res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", reason);
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
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
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ਜੇ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	ਮੁਕਤ(url_s);
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_query_fetcherror_handler_aborted;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	nsurl_unref(siteurl);

	ਵਾਪਸ true;

fetch_about_query_fetcherror_handler_aborted:
	nsurl_unref(siteurl);

	ਵਾਪਸ false;
}
