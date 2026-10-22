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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>

#ضمن "utils/errors.h"
#ضمن "utils/messages.h"
#ضمن "content/fetch.h"

#ضمن "private.h"
#ضمن "query.h"
#ضمن "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	ثابت حرف *reason = "";
	ثابت حرف *title;
	بنية nsurl *siteurl = عدم;
	حرف *description = عدم;
	ثابت حرف *chainurl = عدم;
	ثابت بنية fetch_multipart_انکڑے *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	طالما (curmd != عدم) {
		إذا (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->قدر, &siteurl);
			إذا (res != NSERROR_OK) {
				أرجع fetch_about_srverror(ctx);
			}
		} وإلا إذا (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->قدر;
		} وإلا إذا (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->قدر;
		}
		curmd = curmd->اگلا;
	}

	إذا (siteurl == عدم) {
		أرجع fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_سیٹ_http_کوڈ(ctx, 200);

	/* content type */
	إذا (fetch_about_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8")) {
		اذهب fetch_about_query_ssl_handler_aborted;
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
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	إذا (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		حرر(description);
		إذا (res != NSERROR_OK) {
			اذهب fetch_about_query_ssl_handler_aborted;
		}
	}

	إذا (chainurl == عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} وإلا {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
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
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	إذا (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	حرر(url_s);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_مکمل(ctx);

	nsurl_unref(siteurl);

	أرجع true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	أرجع false;
}
