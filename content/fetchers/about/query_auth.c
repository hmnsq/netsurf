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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "query_auth.h"


/**
 * generate the description of the login query
 */
ସ୍ଥିର nserror
get_authentication_description(ସଂରଚନା nsurl *url,
			       ଅଚଳ ଅକ୍ଷର *realm,
			       ଅଚଳ ଅକ୍ଷର *username,
			       ଅଚଳ ଅକ୍ଷର *password,
			       ଅକ୍ଷର **out_str)
{
	nserror res;
	ଅକ୍ଷର *url_s;
	size_t url_l;
	ଅକ୍ଷର *str = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *ଚାବି;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି ((*username == 0) && (*password == 0)) {
		ଚାବି = "LoginDescription";
	} ଅନ୍ୟଥା {
		ଚାବି = "LoginAgain";
	}

	str = messages_get_buff(ଚାବି, url_s, realm);
	ଯଦି (str != ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      ଚାବି, url_s, realm, str);
		*out_str = str;
	} ଅନ୍ୟଥା {
		res = NSERROR_NOMEM;
	}

	ମୁକ୍ତ(url_s);

	ଫେରନ୍ତୁ res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(ସଂରଚନା fetch_about_context *ctx)
{
	nserror res;
	ଅକ୍ଷର *url_s;
	size_t url_l;
	ଅଚଳ ଅକ୍ଷର *realm = "";
	ଅଚଳ ଅକ୍ଷର *username = "";
	ଅଚଳ ଅକ୍ଷର *password = "";
	ଅଚଳ ଅକ୍ଷର *title;
	ଅକ୍ଷର *description = ଖାଲି;
	ସଂରଚନା nsurl *siteurl = ଖାଲି;
	ଅଚଳ ସଂରଚନା fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ଯେତେବେଳେ (curmd != ଖାଲି) {
		ଯଦି (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->ମୂଲ୍ୟ, &siteurl);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ fetch_about_srverror(ctx);
			}
		} ଅନ୍ୟଥା ଯଦି (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->ମୂଲ୍ୟ;
		} ଅନ୍ୟଥା ଯଦି (strcmp(curmd->name, "username") == 0) {
			username = curmd->ମୂଲ୍ୟ;
		} ଅନ୍ୟଥା ଯଦି (strcmp(curmd->name, "password") == 0) {
			password = curmd->ମୂଲ୍ୟ;
		}
		curmd = curmd->next;
	}

	ଯଦି (siteurl == ଖାଲି) {
		ଫେରନ୍ତୁ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
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
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	ଯଦି (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		ମୁକ୍ତ(description);
		ଯଦି (res != NSERROR_OK) {
			ଯାଅ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
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
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ଯଦି (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	ମୁକ୍ତ(url_s);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	ଫେରନ୍ତୁ true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	ଫେରନ୍ତୁ false;
}
