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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "content/fetch.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "query_auth.h"


/**
 * generate the description of the login query
 */
સ્થિર nserror
get_authentication_description(સંરચના nsurl *url,
			       અચલ અક્ષર *realm,
			       અચલ અક્ષર *username,
			       અચલ અક્ષર *password,
			       અક્ષર **out_str)
{
	nserror res;
	અક્ષર *url_s;
	size_t url_l;
	અક્ષર *str = ખાલી;
	અચલ અક્ષર *ચાવી;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો ((*username == 0) && (*password == 0)) {
		ચાવી = "LoginDescription";
	} નહીંતર {
		ચાવી = "LoginAgain";
	}

	str = messages_get_buff(ચાવી, url_s, realm);
	જો (str != ખાલી) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      ચાવી, url_s, realm, str);
		*out_str = str;
	} નહીંતર {
		res = NSERROR_NOMEM;
	}

	મુક્ત(url_s);

	પાછા res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(સંરચના fetch_about_context *ctx)
{
	nserror res;
	અક્ષર *url_s;
	size_t url_l;
	અચલ અક્ષર *realm = "";
	અચલ અક્ષર *username = "";
	અચલ અક્ષર *password = "";
	અચલ અક્ષર *title;
	અક્ષર *description = ખાલી;
	સંરચના nsurl *siteurl = ખાલી;
	અચલ સંરચના fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	જ્યારે (curmd != ખાલી) {
		જો (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->કિંમત, &siteurl);
			જો (res != NSERROR_OK) {
				પાછા fetch_about_srverror(ctx);
			}
		} નહીંતર જો (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->કિંમત;
		} નહીંતર જો (strcmp(curmd->name, "username") == 0) {
			username = curmd->કિંમત;
		} નહીંતર જો (strcmp(curmd->name, "password") == 0) {
			password = curmd->કિંમત;
		}
		curmd = curmd->next;
	}

	જો (siteurl == ખાલી) {
		પાછા fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		જાઓ fetch_about_query_auth_handler_aborted;
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
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	જો (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		મુક્ત(description);
		જો (res != NSERROR_OK) {
			જાઓ fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
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
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	જો (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	મુક્ત(url_s);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	પાછા true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	પાછા false;
}
