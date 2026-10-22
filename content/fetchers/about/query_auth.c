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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils/errors.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "content/fetch.h"

#include "private.h"
#include "query_auth.h"


/**
 * generate the description of the login query
 */
static nserror
get_authentication_description(sa_nracanaa nsurl *url,
			       const var_na *realm,
			       const var_na *username,
			       const var_na *password,
			       var_na **out_str)
{
	nserror res;
	var_na *url_s;
	size_t url_l;
	var_na *str = NULL;
	const var_na *key;

	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ပေႃး (res != NSERROR_OK) {
		ပွၵ်ႈ res;
	}

	ပေႃး ((*username == 0) && (*password == 0)) {
		key = "LoginDescription";
	} ပေႃးဢမ်ႇ {
		key = "LoginAgain";
	}

	str = messages_get_buff(key, url_s, realm);
	ပေႃး (str != NULL) {
		NSLOG(netsurf, INFO,
		      "key:%s url:%s realm:%s str:%s",
		      key, url_s, realm, str);
		*out_str = str;
	} ပေႃးဢမ်ႇ {
		res = NSERROR_NOMEM;
	}

	free(url_s);

	ပွၵ်ႈ res;
}


/**
 * Handler to generate about scheme authentication query page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_query_auth_handler(sa_nracanaa fetch_about_context *ctx)
{
	nserror res;
	var_na *url_s;
	size_t url_l;
	const var_na *realm = "";
	const var_na *username = "";
	const var_na *password = "";
	const var_na *title;
	var_na *description = NULL;
	sa_nracanaa nsurl *siteurl = NULL;
	const sa_nracanaa fetch_multipart_data *curmd; /* mutipart data iterator */

	/* extract parameters from multipart post data */
	curmd = fetch_about_get_multipart(ctx);
	ၶၢဝ်း (curmd != NULL) {
		ပေႃး (strcmp(curmd->name, "siteurl") == 0) {
			res = nsurl_create(curmd->value, &siteurl);
			ပေႃး (res != NSERROR_OK) {
				ပွၵ်ႈ fetch_about_srverror(ctx);
			}
		} ပေႃးဢမ်ႇ ပေႃး (strcmp(curmd->name, "realm") == 0) {
			realm = curmd->value;
		} ပေႃးဢမ်ႇ ပေႃး (strcmp(curmd->name, "username") == 0) {
			username = curmd->value;
		} ပေႃးဢမ်ႇ ပေႃး (strcmp(curmd->name, "password") == 0) {
			password = curmd->value;
		}
		curmd = curmd->next;
	}

	ပေႃး (siteurl == NULL) {
		ပွၵ်ႈ fetch_about_srverror(ctx);
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ပေႃး (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8")) {
		goto fetch_about_query_auth_handler_aborted;
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
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<form method=\"post\""
			 " enctype=\"multipart/form-data\">");
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = get_authentication_description(siteurl,
					     realm,
					     username,
					     password,
					     &description);
	ပေႃး (res == NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>%s</p>", description);
		free(description);
		ပေႃး (res != NSERROR_OK) {
			goto fetch_about_query_auth_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "<table>");
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"name\">%s:</label></th>"
			 "<td><input type=\"text\" id=\"username\" "
			 "name=\"username\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Username"), username);
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr>"
			 "<th><label for=\"password\">%s:</label></th>"
			 "<td><input type=\"password\" id=\"password\" "
			 "name=\"password\" value=\"%s\"></td>"
			 "</tr>",
			 messages_get("Password"), password);
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</table>");
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
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
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = nsurl_get(siteurl, NSURL_COMPLETE, &url_s, &url_l);
	ပေႃး (res != NSERROR_OK) {
		url_s = strdup("");
	}
	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"siteurl\" value=\"%s\">",
			 url_s);
	free(url_s);
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<input type=\"hidden\" name=\"realm\" value=\"%s\">",
			 realm);
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx, "</form></body>\n</html>\n");
	ပေႃး (res != NSERROR_OK) {
		goto fetch_about_query_auth_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	nsurl_unref(siteurl);

	ပွၵ်ႈ true;

fetch_about_query_auth_handler_aborted:

	nsurl_unref(siteurl);

	ပွၵ်ႈ false;
}
