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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "content/fetch.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "query.h"
#அந்தர்பாவ "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;
	எழுத்து *url_s;
	size_t url_l;
	மாறிலி எழுத்து *reason = "";
	மாறிலி எழுத்து *title;
	கட்டமைப்பு nsurl *siteurl = வெற்று;
	எழுத்து *description = வெற்று;
	மாறிலி எழுத்து *chainurl = வெற்று;
	மாறிலி கட்டமைப்பு fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	போது (curmd != வெற்று) {
		எனில் (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->மதிப்பு, &siteurl);
			எனில் (res != NSERROR_OK) {
				திரும்பு fetch_about_srverror(ctx);
			}
		} இல்லையெனில் எனில் (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->மதிப்பு;
		} இல்லையெனில் எனில் (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->மதிப்பு;
		}
		curmd = curmd->next;
	}

	எனில் (siteurl == வெற்று) {
		திரும்பு fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		செல் fetch_about_query_ssl_handler_aborted;
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
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	எனில் (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		விடுவி(description);
		எனில் (res != NSERROR_OK) {
			செல் fetch_about_query_ssl_handler_aborted;
		}
	}

	எனில் (chainurl == வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} இல்லையெனில் {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
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
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	எனில் (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	விடுவி(url_s);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	திரும்பு true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	திரும்பு false;
}
