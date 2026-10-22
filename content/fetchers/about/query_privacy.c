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
#अन्तर्भाव "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	वर्ण *url_s;
	माप_प्रकार url_l;
	अचर वर्ण *reason = "";
	अचर वर्ण *title;
	संरचना nsurl *siteurl = रिक्त;
	वर्ण *description = रिक्त;
	अचर वर्ण *chainurl = रिक्त;
	अचर संरचना fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	जबसम्म (curmd != रिक्त) {
		यदि (म_भेद(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->मान, &siteurl);
			यदि (res != NSERROR_OK) {
				फर्काऊ fetch_about_srverror(ctx);
			}
		} अन्यथा यदि (म_भेद(curmd->name, "reason") == 0) {
			reason = curmd->मान;
		} अन्यथा यदि (म_भेद(curmd->name, "chainurl") == 0) {
			chainurl = curmd->मान;
		}
		curmd = curmd->next;
	}

	यदि (siteurl == रिक्त) {
		फर्काऊ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	यदि (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		गच्छ fetch_about_query_ssl_handler_aborted;
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
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	यदि (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		मुक्त(description);
		यदि (res != NSERROR_OK) {
			गच्छ fetch_about_query_ssl_handler_aborted;
		}
	}

	यदि (chainurl == रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p>%s</p></div>",
				 reason,
				 messages_get("ViewCertificatesNotPossible"));
	} अन्यथा {
		res = fetch_about_ssenddataf(ctx,
				 "<div><p>%s</p></div>"
				 "<div><p><a href=\"%s\" target=\"_blank\">%s</a></p></div>",
				 reason,
				 chainurl,
				 messages_get("ViewCertificates"));
	}
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
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
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	यदि (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	मुक्त(url_s);
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	फर्काऊ true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	फर्काऊ false;
}
