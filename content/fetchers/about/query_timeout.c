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
 * content generator for the about scheme query timeout page
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "content/fetch.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "query.h"
#అన్తర్భావ "query_timeout.h"

/**
 * Handler to generate about scheme timeout query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_timeout_handler(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;
	అక్షరం *url_s;
	size_t url_l;
	స్థిరరాశి అక్షరం *reason = "";
	స్థిరరాశి అక్షరం *title;
	నిర్మాణం nsurl *siteurl = ఖాళీ;
	అక్షరం *description = ఖాళీ;
	స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	అయితే (curmd != ఖాళీ) {
		ఒకవేళ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->విలువ, &siteurl);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి fetch_about_srverror(ctx);
			}
		} లేకపోతే ఒకవేళ (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->విలువ;
		}
		curmd = curmd->తదుపరి;
	}

	ఒకవేళ (siteurl == ఖాళీ) {
		తిరిగి fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, 200);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html; charset=utf-8")) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	title = messages_get("TimeoutTitle");
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>%s</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\" id =\"timeout\">\n"
			"<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n",
			title, title);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "TimeoutDescription",
				    &description);
	ఒకవేళ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		విడుదల(description);
		ఒకవేళ (res != NSERROR_OK) {
			వెళ్ళు fetch_about_query_timeout_handler_aborted;
		}
	}
	res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", reason);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
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
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ఒకవేళ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	విడుదల(url_s);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_query_timeout_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	nsurl_unref(siteurl);

	తిరిగి true;

fetch_about_query_timeout_handler_aborted:
	nsurl_unref(siteurl);

	తిరిగి false;
}
