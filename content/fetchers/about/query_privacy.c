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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "query.h"
#ଅନ୍ତର୍ଭାଵ "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(ସଂରଚନା fetch_about_context *ctx)
{
	nserror res;
	ଅକ୍ଷର *url_s;
	size_t url_l;
	ଅଚଳ ଅକ୍ଷର *reason = "";
	ଅଚଳ ଅକ୍ଷର *title;
	ସଂରଚନା nsurl *siteurl = ଖାଲି;
	ଅକ୍ଷର *description = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *chainurl = ଖାଲି;
	ଅଚଳ ସଂରଚନା fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ଯେତେବେଳେ (curmd != ଖାଲି) {
		ଯଦି (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->ମୂଲ୍ୟ, &siteurl);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ fetch_about_srverror(ctx);
			}
		} ଅନ୍ୟଥା ଯଦି (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->ମୂଲ୍ୟ;
		} ଅନ୍ୟଥା ଯଦି (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->ମୂଲ୍ୟ;
		}
		curmd = curmd->next;
	}

	ଯଦି (siteurl == ଖାଲି) {
		ଫେରନ୍ତୁ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
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
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	ଯଦି (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		ମୁକ୍ତ(description);
		ଯଦି (res != NSERROR_OK) {
			ଯାଅ fetch_about_query_ssl_handler_aborted;
		}
	}

	ଯଦି (chainurl == ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} ଅନ୍ୟଥା {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
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
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ଯଦି (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	ମୁକ୍ତ(url_s);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	ଫେରନ୍ତୁ true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	ଫେରନ୍ତୁ false;
}
