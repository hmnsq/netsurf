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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "content/fetch.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "query.h"
#അന്തര്ഭാവ "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(ഘടന fetch_about_context *ctx)
{
	nserror res;
	അക്ഷരം *url_s;
	size_t url_l;
	സ്ഥിരം അക്ഷരം *reason = "";
	സ്ഥിരം അക്ഷരം *title;
	ഘടന nsurl *siteurl = ശൂന്യ;
	അക്ഷരം *description = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *chainurl = ശൂന്യ;
	സ്ഥിരം ഘടന fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ആകുമ്പോൾ (curmd != ശൂന്യ) {
		എങ്കിൽ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->മൂല്യം, &siteurl);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ fetch_about_srverror(ctx);
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->മൂല്യം;
		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->മൂല്യം;
		}
		curmd = curmd->next;
	}

	എങ്കിൽ (siteurl == ശൂന്യ) {
		തിരികെ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		പോകുക fetch_about_query_ssl_handler_aborted;
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
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	എങ്കിൽ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		സ്വതന്ത്രം(description);
		എങ്കിൽ (res != NSERROR_OK) {
			പോകുക fetch_about_query_ssl_handler_aborted;
		}
	}

	എങ്കിൽ (chainurl == ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} അല്ലെങ്കിൽ {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
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
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	എങ്കിൽ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	സ്വതന്ത്രം(url_s);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	തിരികെ true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	തിരികെ false;
}
