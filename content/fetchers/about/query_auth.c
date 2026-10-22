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
get_authentication_description(ರಚನೆ nsurl *url,
			       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm,
			       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *username,
			       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *password,
			       ಅಕ್ಷರ **ಹೊರಹೋಗು_str)
{
	nserror res;
	ಅಕ್ಷರ *url_s;
	size_t url_l;
	ಅಕ್ಷರ *str = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ ((*username == 0) && (*password == 0)) {
		ಕೀಲಿ = "LoginDescription";
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಕೀಲಿ = "LoginAgain";
	}

	str = messages_get_buff(ಕೀಲಿ, url_s, realm);
	ಆದರೆ (str != ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      ಕೀಲಿ, url_s, realm, str);
		*ಹೊರಹೋಗು_str = str;
	} ಇಲ್ಲದಿದ್ದರೆ {
		res = NSERROR_NOMEM;
	}

	ಬಿಡುಗಡೆ(url_s);

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;
	ಅಕ್ಷರ *url_s;
	size_t url_l;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm = "";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *username = "";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *password = "";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;
	ಅಕ್ಷರ *description = ಖಾಲಿ;
	ರಚನೆ nsurl *siteurl = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ಆಗ (curmd != ಖಾಲಿ) {
		ಆದರೆ (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->ಮೌಲ್ಯ, &siteurl);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ fetch_about_srverror(ctx);
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->ಮೌಲ್ಯ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(curmd->name, "username") == 0) {
			username = curmd->ಮೌಲ್ಯ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(curmd->name, "password") == 0) {
			password = curmd->ಮೌಲ್ಯ;
		}
		curmd = curmd->ಮುಂದಿನ;
	}

	ಆದರೆ (siteurl == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, 200);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html; charset=utf-8")) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
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
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	ಆದರೆ (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		ಬಿಡುಗಡೆ(description);
		ಆದರೆ (res != NSERROR_OK) {
			ಹೋಗು fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
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
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ಆದರೆ (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	ಬಿಡುಗಡೆ(url_s);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	nsurl_unref(siteurl);

	ಹಿಂತಿರುಗಿ true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	ಹಿಂತಿರುಗಿ false;
}
