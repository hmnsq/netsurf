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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>

#ضمن "utils/errors.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "content/fetch.h"

#ضمن "private.h"
#ضمن "query_auth.h"


/**
 * generate the description of the login query
 */
ساكن nserror
get_authentication_description(بنية nsurl *url,
			       ثابت حرف *realm,
			       ثابت حرف *username,
			       ثابت حرف *password,
			       حرف **out_str)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	حرف *str = عدم;
	ثابت حرف *مفتاح;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	إذا (res != NSERROR_OK) {
		إرجاع res;
	}

	إذا ((*username == 0) && (*password == 0)) {
		مفتاح = "LoginDescription";
	} وإلا {
		مفتاح = "LoginAgain";
	}

	str = messages_get_buff(مفتاح, url_s, realm);
	إذا (str != عدم) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      مفتاح, url_s, realm, str);
		*out_str = str;
	} وإلا {
		res = NSERROR_NOMEM;
	}

	حرر(url_s);

	إرجاع res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	ثابت حرف *realm = "";
	ثابت حرف *username = "";
	ثابت حرف *password = "";
	ثابت حرف *title;
	حرف *description = عدم;
	بنية nsurl *siteurl = عدم;
	ثابت بنية fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	بينما (curmd != عدم) {
		إذا (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->قيمة, &siteurl);
			إذا (res != NSERROR_OK) {
				إرجاع fetch_about_srverror(ctx);
			}
		} وإلا إذا (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->قيمة;
		} وإلا إذا (strcmp(curmd->name, "username") == 0) {
			username = curmd->قيمة;
		} وإلا إذا (strcmp(curmd->name, "password") == 0) {
			password = curmd->قيمة;
		}
		curmd = curmd->next;
	}

	إذا (siteurl == عدم) {
		إرجاع fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		اذهب fetch_about_query_auth_handler_aborted;
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
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	إذا (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		حرر(description);
		إذا (res != NSERROR_OK) {
			اذهب fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
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
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	إذا (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	حرر(url_s);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	إرجاع true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	إرجاع false;
}
