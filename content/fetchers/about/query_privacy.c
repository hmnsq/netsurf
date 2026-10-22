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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils/errors.h"
#include "utils/messages.h"
#include "content/fetch.h"

#include "private.h"
#include "query.h"
#include "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(struct fetch_about_context *ctx)
{
	nserror res;
	char *url_s;
	size_t url_l;
	const char *reason = "";
	const char *title;
	struct nsurl *siteurl = NULL;
	char *description = NULL;
	const char *chainurl = NULL;
	const struct fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	동안 (curmd != NULL) {
		만약 (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->value, &siteurl);
			만약 (res != NSERROR_OK) {
				반환 fetch_about_srverror(ctx);
			}
		} 아니면 만약 (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->value;
		} 아니면 만약 (strcmp(curmd->name, "chainurl") == 0) {
			chainurl = curmd->value;
		}
		curmd = curmd->next;
	}

	만약 (siteurl == NULL) {
		반환 fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	만약 (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		goto fetch_about_query_ssl_handler_aborted;
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
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	만약 (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		free(description);
		만약 (res != NSERROR_OK) {
			goto fetch_about_query_ssl_handler_aborted;
		}
	}

	만약 (chainurl == NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} 아니면 {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
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
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	만약 (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	free(url_s);
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	만약 (res != NSERROR_OK) {
		goto fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	반환 true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	반환 false;
}
