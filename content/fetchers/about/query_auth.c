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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "query_auth.h"


/**
 * generate the description of the login query
 */
ಸ್ಥಿರ nserror
get_authentication_description(ಸಂರಚನಾ nsurl *url,
			       ಧ್ರುವ ವರ್ಣ *realm,
			       ಧ್ರುವ ವರ್ಣ *username,
			       ಧ್ರುವ ವರ್ಣ *password,
			       ವರ್ಣ **out_str)
{
	nserror res;
	ವರ್ಣ *url_s;
	size_t url_l;
	ವರ್ಣ *str = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ ((*username == 0) && (*password == 0)) {
		ಕೀಲಿ = "LoginDescription";
	} ಅನ್ಯಥಾ {
		ಕೀಲಿ = "LoginAgain";
	}

	str = messages_get_buff(ಕೀಲಿ, url_s, realm);
	ಯದಿ (str != ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      ಕೀಲಿ, url_s, realm, str);
		*out_str = str;
	} ಅನ್ಯಥಾ {
		res = NSERROR_NOMEM;
	}

	ಮುಕ್ತ(url_s);

	ಪ್ರತ್ಯಾಗಮ res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	nserror res;
	ವರ್ಣ *url_s;
	size_t url_l;
	ಧ್ರುವ ವರ್ಣ *realm = "";
	ಧ್ರುವ ವರ್ಣ *username = "";
	ಧ್ರುವ ವರ್ಣ *password = "";
	ಧ್ರುವ ವರ್ಣ *title;
	ವರ್ಣ *description = ರಿಕ್ತ;
	ಸಂರಚನಾ nsurl *siteurl = ರಿಕ್ತ;
	ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ಯಾವತ್ (curmd != ರಿಕ್ತ) {
		ಯದಿ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->ಮೌಲ್ಯ, &siteurl);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ fetch_about_srverror(ctx);
			}
		} ಅನ್ಯಥಾ ಯದಿ (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->ಮೌಲ್ಯ;
		} ಅನ್ಯಥಾ ಯದಿ (strcmp(curmd->name, "username") == 0) {
			username = curmd->ಮೌಲ್ಯ;
		} ಅನ್ಯಥಾ ಯದಿ (strcmp(curmd->name, "password") == 0) {
			password = curmd->ಮೌಲ್ಯ;
		}
		curmd = curmd->next;
	}

	ಯದಿ (siteurl == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
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
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	ಯದಿ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		ಮುಕ್ತ(description);
		ಯದಿ (res != NSERROR_OK) {
			ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
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
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ಯದಿ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	ಮುಕ್ತ(url_s);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	ಪ್ರತ್ಯಾಗಮ false;
}
