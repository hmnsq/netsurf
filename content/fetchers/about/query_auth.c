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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "content/fetch.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "query_auth.h"


/**
 * generate the description of the login query
 */
സ്ഥിതം nserror
get_authentication_description(ഘടന nsurl *url,
			       സ്ഥിരം അക്ഷരം *realm,
			       സ്ഥിരം അക്ഷരം *username,
			       സ്ഥിരം അക്ഷരം *password,
			       അക്ഷരം **out_str)
{
	nserror res;
	അക്ഷരം *url_s;
	size_t url_l;
	അക്ഷരം *str = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *താക്കോൽ;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ ((*username == 0) && (*password == 0)) {
		താക്കോൽ = "LoginDescription";
	} അല്ലെങ്കിൽ {
		താക്കോൽ = "LoginAgain";
	}

	str = messages_get_buff(താക്കോൽ, url_s, realm);
	എങ്കിൽ (str != ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      താക്കോൽ, url_s, realm, str);
		*out_str = str;
	} അല്ലെങ്കിൽ {
		res = NSERROR_NOMEM;
	}

	സ്വതന്ത്രം(url_s);

	തിരികെ res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(ഘടന fetch_about_context *ctx)
{
	nserror res;
	അക്ഷരം *url_s;
	size_t url_l;
	സ്ഥിരം അക്ഷരം *realm = "";
	സ്ഥിരം അക്ഷരം *username = "";
	സ്ഥിരം അക്ഷരം *password = "";
	സ്ഥിരം അക്ഷരം *title;
	അക്ഷരം *description = ശൂന്യ;
	ഘടന nsurl *siteurl = ശൂന്യ;
	സ്ഥിരം ഘടന fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ആകുമ്പോൾ (curmd != ശൂന്യ) {
		എങ്കിൽ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->മൂല്യം, &siteurl);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ fetch_about_srverror(ctx);
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->മൂല്യം;
		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(curmd->name, "username") == 0) {
			username = curmd->മൂല്യം;
		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(curmd->name, "password") == 0) {
			password = curmd->മൂല്യം;
		}
		curmd = curmd->next;
	}

	എങ്കിൽ (siteurl == ശൂന്യ) {
		തിരികെ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		പോകുക fetch_about_query_auth_handler_aborted;
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
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	എങ്കിൽ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		സ്വതന്ത്രം(description);
		എങ്കിൽ (res != NSERROR_OK) {
			പോകുക fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
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
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	എങ്കിൽ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	സ്വതന്ത്രം(url_s);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	തിരികെ true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	തിരികെ false;
}
