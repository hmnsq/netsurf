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

#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <stdint.h>
#شامل <stdlib.h>

#شامل "utils/errors.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "content/fetch.h"

#شامل "private.h"
#شامل "query_auth.h"


/**
 * generate the description of the login query
 */
ساکن nserror
get_authentication_description(ساخت nsurl *url,
			       ثابت حرف *realm,
			       ثابت حرف *username,
			       ثابت حرف *password,
			       حرف **برآمد_str)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	حرف *str = تہی;
	ثابت حرف *کلید;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر ((*username == 0) && (*password == 0)) {
		کلید = "LoginDescription";
	} ورنہ {
		کلید = "LoginAgain";
	}

	str = messages_get_buff(کلید, url_s, realm);
	اگر (str != تہی) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      کلید, url_s, realm, str);
		*برآمد_str = str;
	} ورنہ {
		res = NSERROR_NOMEM;
	}

	آزاد(url_s);

	واپس res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(ساخت fetch_about_context *ctx)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	ثابت حرف *realm = "";
	ثابت حرف *username = "";
	ثابت حرف *password = "";
	ثابت حرف *title;
	حرف *description = تہی;
	ساخت nsurl *siteurl = تہی;
	ثابت ساخت fetch_multipart_ڈیٹا *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	جب (curmd != تہی) {
		اگر (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->قدر, &siteurl);
			اگر (res != NSERROR_OK) {
				واپس fetch_about_srverror(ctx);
			}
		} ورنہ اگر (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->قدر;
		} ورنہ اگر (strcmp(curmd->name, "username") == 0) {
			username = curmd->قدر;
		} ورنہ اگر (strcmp(curmd->name, "password") == 0) {
			password = curmd->قدر;
		}
		curmd = curmd->اگلا;
	}

	اگر (siteurl == تہی) {
		واپس fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_سیٹ_http_رمز(ctx, 200);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8")) {
		جاؤ fetch_about_query_auth_handler_aborted;
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
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	اگر (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		آزاد(description);
		اگر (res != NSERROR_OK) {
			جاؤ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
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
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	اگر (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	آزاد(url_s);
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_ختم_شدہ(ctx);

	nsurl_unref(siteurl);

	واپس true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	واپس false;
}
