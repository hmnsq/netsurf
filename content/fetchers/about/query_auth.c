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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "content/fetch.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "query_auth.h"


/**
 * generate the description of the login query
 */
நிலையான nserror
get_authentication_description(கட்டமைப்பு nsurl *url,
			       மாறிலி எழுத்து *realm,
			       மாறிலி எழுத்து *username,
			       மாறிலி எழுத்து *password,
			       எழுத்து **out_str)
{
	nserror res;
	எழுத்து *url_s;
	size_t url_l;
	எழுத்து *str = வெற்று;
	மாறிலி எழுத்து *சாவி;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் ((*username == 0) && (*password == 0)) {
		சாவி = "LoginDescription";
	} இல்லையெனில் {
		சாவி = "LoginAgain";
	}

	str = messages_get_buff(சாவி, url_s, realm);
	எனில் (str != வெற்று) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      சாவி, url_s, realm, str);
		*out_str = str;
	} இல்லையெனில் {
		res = NSERROR_NOMEM;
	}

	விடுவி(url_s);

	திரும்பு res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;
	எழுத்து *url_s;
	size_t url_l;
	மாறிலி எழுத்து *realm = "";
	மாறிலி எழுத்து *username = "";
	மாறிலி எழுத்து *password = "";
	மாறிலி எழுத்து *title;
	எழுத்து *description = வெற்று;
	கட்டமைப்பு nsurl *siteurl = வெற்று;
	மாறிலி கட்டமைப்பு fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	போது (curmd != வெற்று) {
		எனில் (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->மதிப்பு, &siteurl);
			எனில் (res != NSERROR_OK) {
				திரும்பு fetch_about_srverror(ctx);
			}
		} இல்லையெனில் எனில் (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->மதிப்பு;
		} இல்லையெனில் எனில் (strcmp(curmd->name, "username") == 0) {
			username = curmd->மதிப்பு;
		} இல்லையெனில் எனில் (strcmp(curmd->name, "password") == 0) {
			password = curmd->மதிப்பு;
		}
		curmd = curmd->next;
	}

	எனில் (siteurl == வெற்று) {
		திரும்பு fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		செல் fetch_about_query_auth_handler_aborted;
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
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	எனில் (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		விடுவி(description);
		எனில் (res != NSERROR_OK) {
			செல் fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
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
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	எனில் (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	விடுவி(url_s);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	திரும்பு true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	திரும்பு false;
}
