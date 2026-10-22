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

#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <stdint.h>
#समावेश <stdlib.h>

#समावेश "utils/errors.h"
#समावेश "utils/messages.h"
#समावेश "content/fetch.h"

#समावेश "private.h"
#समावेश "query.h"
#समावेश "query_privacy.h"

/**
 * Handler to generate about scheme privacy query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_privacy_handler(काष्ठा fetch_about_context *ctx)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	स्थिर अक्षर *reason = "";
	स्थिर अक्षर *title;
	काष्ठा nsurl *siteurl = शून्य;
	अक्षर *description = शून्य;
	स्थिर अक्षर *chainurl = शून्य;
	स्थिर काष्ठा fetch_multipart_तथ्य *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	जबतक (curmd != शून्य) {
		अगर (म_भेद(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->मूल्य, &siteurl);
			अगर (res != NSERROR_OK) {
				वापस fetch_about_srverror(ctx);
			}
		} अन्यथा अगर (म_भेद(curmd->name, "reason") == 0) {
			reason = curmd->मूल्य;
		} अन्यथा अगर (म_भेद(curmd->name, "chainurl") == 0) {
			chainurl = curmd->मूल्य;
		}
		curmd = curmd->अग्र;
	}

	अगर (siteurl == शून्य) {
		वापस fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		जाओ fetch_about_query_ssl_handler_aborted;
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
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
	}

	res = get_query_description(siteurl,
				    "PrivacyDescription",
				    &description);
	अगर (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<div><p>%s</p></div>", description);
		मुक्त(description);
		अगर (res != NSERROR_OK) {
			जाओ fetch_about_query_ssl_handler_aborted;
		}
	}

	अगर (chainurl == शून्य) {
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
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
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
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	अगर (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	मुक्त(url_s);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_ssl_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	वापस true;

fetch_about_query_ssl_handler_aborted:
	nsurl_unref(siteurl);

	वापस false;
}
