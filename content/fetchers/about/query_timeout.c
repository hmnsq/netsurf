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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "content/fetch.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "query.h"
#अन्तर्भाव "query_timeout.h"

/**
 * Handler to generate about scheme timeout query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_timeout_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	वर्ण *url_s;
	size_t url_l;
	ध्रुव वर्ण *reason = "";
	ध्रुव वर्ण *title;
	संरचना nsurl *siteurl = रिक्त;
	वर्ण *description = रिक्त;
	ध्रुव संरचना fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	याव (curmd != रिक्त) {
		सचे (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->मानम्, &siteurl);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त fetch_about_srverror(ctx);
			}
		} अञ्ञथा सचे (strcmp(curmd->name, "reason") == 0) {
			reason = curmd->मानम्;
		}
		curmd = curmd->next;
	}

	सचे (siteurl == रिक्त) {
		पटिनिवत्त fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	सचे (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		गच्छ fetch_about_query_timeout_handler_aborted;
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
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "TimeoutDescription",
				    &description);
	सचे (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		मुक्त(description);
		सचे (res != NSERROR_OK) {
			गच्छ fetch_about_query_timeout_handler_aborted;
		}
	}
	res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", reason);
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
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
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	सचे (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	मुक्त(url_s);
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_query_timeout_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	पटिनिवत्त true;

fetch_about_query_timeout_handler_aborted:
	nsurl_unref(siteurl);

	पटिनिवत्त false;
}
