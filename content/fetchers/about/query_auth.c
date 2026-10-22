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
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "content/fetch.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "query_auth.h"


/**
 * generate the description of the login query
 */
स्थिर nserror
get_authentication_description(संरचना nsurl *url,
			       स्थिरांक अक्षर *realm,
			       स्थिरांक अक्षर *username,
			       स्थिरांक अक्षर *password,
			       अक्षर **out_str)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	अक्षर *str = रिक्त;
	स्थिरांक अक्षर *किल्ली;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर ((*username == 0) && (*password == 0)) {
		किल्ली = "LoginDescription";
	} अन्यथा {
		किल्ली = "LoginAgain";
	}

	str = messages_get_buff(किल्ली, url_s, realm);
	जर (str != रिक्त) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      किल्ली, url_s, realm, str);
		*out_str = str;
	} अन्यथा {
		res = NSERROR_NOMEM;
	}

	मुक्त(url_s);

	परत res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	स्थिरांक अक्षर *realm = "";
	स्थिरांक अक्षर *username = "";
	स्थिरांक अक्षर *password = "";
	स्थिरांक अक्षर *title;
	अक्षर *description = रिक्त;
	संरचना nsurl *siteurl = रिक्त;
	स्थिरांक संरचना fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	पर्यंत (curmd != रिक्त) {
		जर (म_भेद(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->मूल्य, &siteurl);
			जर (res != NSERROR_OK) {
				परत fetch_about_srverror(ctx);
			}
		} अन्यथा जर (म_भेद(curmd->name, "realm") == 0) {
			realm = curmd->मूल्य;
		} अन्यथा जर (म_भेद(curmd->name, "username") == 0) {
			username = curmd->मूल्य;
		} अन्यथा जर (म_भेद(curmd->name, "password") == 0) {
			password = curmd->मूल्य;
		}
		curmd = curmd->next;
	}

	जर (siteurl == रिक्त) {
		परत fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	जर (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		जा fetch_about_query_auth_handler_aborted;
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
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	जर (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		मुक्त(description);
		जर (res != NSERROR_OK) {
			जा fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
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
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	जर (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	मुक्त(url_s);
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	जर (res != NSERROR_OK) {
		जा fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	परत true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	परत false;
}
