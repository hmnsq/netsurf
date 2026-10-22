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

#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <stdint.h>
#شامل <stdlib.h>

#شامل "utils/errors.h"
#شامل "utils/messages.h"
#شامل "content/fetch.h"

#شامل "private.h"
#شامل "query.h"
#شامل "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(ساختار fetch_about_context *ctx)
{
	nserror res;
	نویسه *url_s;
	size_t url_l;
	ثابت نویسه *reason = "";
	ثابت نویسه *title;
	ساختار nsurl *siteurl = پوچ;
	نویسه *description = پوچ;
	ثابت نویسه *chainurl = پوچ;
	ثابت ساختار fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	وقتی (curmd != پوچ) {
		اگر (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->مقدار, &siteurl);
			اگر (res != NSERROR_OK) {
				بازگشت fetch_about_srverror(ctx);
			}
		} وگرنه اگر (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->مقدار;
		} وگرنه اگر (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->مقدار;
		}
		curmd = curmd->next;
	}

	اگر (siteurl == پوچ) {
		بازگشت fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	title = messages_get("PrivacyTitle");
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>%s</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\" id =\"privacy\">\n"
			"<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n",
			title, title);
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	اگر (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		آزاد(description);
		اگر (res != NSERROR_OK) {
			برو fetch_about_query_ssl_handler_aborted;
		}
	}

	اگر (chainurl == پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} وگرنه {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}
	res = fetch_about_ssenddataf(ctx,
			 "<div id=\"buttons\">"
			 "<input type=\"submit\" id=\"back\" name=\"back\" "
			 "value=\"%s\" class=\"default-action\">"
			 "<input type=\"submit\" id=\"proceed\" name=\"proceed\" "
			 "value=\"%s\">"
			 "</div>",
			 messages_get("Backtosafety"),
			 messages_get("Proceed"));
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	اگر (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	آزاد(url_s);
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		برو fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	بازگشت true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	بازگشت false;
}
