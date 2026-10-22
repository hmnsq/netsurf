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
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "content/fetch.h"

#समावेश "private.h"
#समावेश "query_auth.h"


/**
 * generate the description of the login query
 */
अटल nserror
get_authentication_description(काष्ठा nsurl *url,
			       स्थिर अक्षर *realm,
			       स्थिर अक्षर *username,
			       स्थिर अक्षर *password,
			       अक्षर **out_str)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	अक्षर *str = शून्य;
	स्थिर अक्षर *कुंजी;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर ((*username == 0) && (*password == 0)) {
		कुंजी = "LoginDescription";
	} अन्यथा {
		कुंजी = "LoginAgain";
	}

	str = messages_get_buff(कुंजी, url_s, realm);
	अगर (str != शून्य) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      कुंजी, url_s, realm, str);
		*out_str = str;
	} अन्यथा {
		res = NSERROR_NOMEM;
	}

	मुक्त(url_s);

	वापस res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(काष्ठा fetch_about_context *ctx)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	स्थिर अक्षर *realm = "";
	स्थिर अक्षर *username = "";
	स्थिर अक्षर *password = "";
	स्थिर अक्षर *title;
	अक्षर *description = शून्य;
	काष्ठा nsurl *siteurl = शून्य;
	स्थिर काष्ठा fetch_multipart_तथ्य *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	जबतक (curmd != शून्य) {
		अगर (म_भेद(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->मूल्य, &siteurl);
			अगर (res != NSERROR_OK) {
				वापस fetch_about_srverror(ctx);
			}
		} अन्यथा अगर (म_भेद(curmd->name, "realm") == 0) {
			realm = curmd->मूल्य;
		} अन्यथा अगर (म_भेद(curmd->name, "username") == 0) {
			username = curmd->मूल्य;
		} अन्यथा अगर (म_भेद(curmd->name, "password") == 0) {
			password = curmd->मूल्य;
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
		जाओ fetch_about_query_auth_handler_aborted;
	}

	title = messages_get("LoginTitle");
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>%s</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\" id =\"authentication\">\n"
			"<h1 class=\"ns-border\">%s</h1>\n",
			title, title);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	अगर (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		मुक्त(description);
		अगर (res != NSERROR_OK) {
			जाओ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<div id=\"buttons\">"
			 "<input type=\"submit\" id=\"login\" name=\"login\" "
			 "value=\"%s\" class=\"default-action\">"
			 "<input type=\"submit\" id=\"cancel\" name=\"cancel\" "
			 "value=\"%s\">"
			 "</div>",
			 messages_get("Login"),
			 messages_get("Cancel"));
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
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
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	वापस true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	वापस false;
}
