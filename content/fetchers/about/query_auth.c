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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "content/fetch.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "query_auth.h"


/**
 * generate the description of the login query
 */
স্থির nserror
get_authentication_description(সংরচনা nsurl *url,
			       ধ্রুব বর্ণ *realm,
			       ধ্রুব বর্ণ *username,
			       ধ্রুব বর্ণ *password,
			       বর্ণ **out_str)
{
	nserror res;
	বর্ণ *url_s;
	size_t url_l;
	বর্ণ *str = রিক্ত;
	ধ্রুব বর্ণ *চাবি;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	যদি (res != NSERROR_OK) {
		প্রত্যাগম res;
	}

	যদি ((*username == 0) && (*password == 0)) {
		চাবি = "LoginDescription";
	} অন্যথা {
		চাবি = "LoginAgain";
	}

	str = messages_get_buff(চাবি, url_s, realm);
	যদি (str != রিক্ত) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      চাবি, url_s, realm, str);
		*out_str = str;
	} অন্যথা {
		res = NSERROR_NOMEM;
	}

	মুক্ত(url_s);

	প্রত্যাগম res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(সংরচনা fetch_about_context *ctx)
{
	nserror res;
	বর্ণ *url_s;
	size_t url_l;
	ধ্রুব বর্ণ *realm = "";
	ধ্রুব বর্ণ *username = "";
	ধ্রুব বর্ণ *password = "";
	ধ্রুব বর্ণ *title;
	বর্ণ *description = রিক্ত;
	সংরচনা nsurl *siteurl = রিক্ত;
	ধ্রুব সংরচনা fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	যাবত্ (curmd != রিক্ত) {
		যদি (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->মান, &siteurl);
			যদি (res != NSERROR_OK) {
				প্রত্যাগম fetch_about_srverror(ctx);
			}
		} অন্যথা যদি (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->মান;
		} অন্যথা যদি (strcmp(curmd->name, "username") == 0) {
			username = curmd->মান;
		} অন্যথা যদি (strcmp(curmd->name, "password") == 0) {
			password = curmd->মান;
		}
		curmd = curmd->next;
	}

	যদি (siteurl == রিক্ত) {
		প্রত্যাগম fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
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
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	যদি (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		মুক্ত(description);
		যদি (res != NSERROR_OK) {
			গচ্ছ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
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
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	যদি (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	মুক্ত(url_s);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	প্রত্যাগম true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	প্রত্যাগম false;
}
