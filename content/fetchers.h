/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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
 * \file content/fetchers.h
 *
 * Interface for fetchers factory.
 */

#अगर_अघोषित _NETSURF_DESKTOP_FETCHERS_H_
#घोषणा _NETSURF_DESKTOP_FETCHERS_H_

#समावेश "utils/inet.h" /* this is necessary for the fd_set definition */
#समावेश <libwapcaplet/libwapcaplet.h>

काष्ठा nsurl;
काष्ठा fetch_multipart_तथ्य;
काष्ठा fetch;

/**
 * Fetcher operations API
 *
 * These are the operations a fetcher must implement.
 *
 * Each fetcher is called once for initialisaion and finalisation.
 * The poll entry point will be called to allow all active fetches to progress.
 * The flow of a fetch operation is:
 *   URL is checked for aceptability.
 *   setup with all applicable data.
 *   start is called before the first poll
 *   after completion or abort it is freed
 *
 */
काष्ठा fetcher_operation_table {
	/**
	 * The initialiser for the fetcher.
	 *
	 * Called once to initialise the fetcher.
	 */
	bool (*initialise)(lwc_string *scheme);

	/**
	 * Can this fetcher accept a url.
	 *
	 * \param url the URL to check
	 * \return true if the fetcher can handle the url else false.
	 */
	bool (*acceptable)(स्थिर काष्ठा nsurl *url);

	/**
	 * Setup a fetch
	 */
	व्योम *(*setup)(काष्ठा fetch *parent_fetch, काष्ठा nsurl *url,
		bool only_2xx, bool downgrade_tls, स्थिर अक्षर *post_urlenc,
		स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		स्थिर अक्षर **headers);

	/**
	 * start a fetch.
	 */
	bool (*start)(व्योम *fetch);

	/**
	 * abort a fetch.
	 */
	व्योम (*पात)(व्योम *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	व्योम (*मुक्त)(व्योम *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	व्योम (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	पूर्णांक (*fdset)(lwc_string *scheme, fd_set *read_set, fd_set *write_set,
		     fd_set *error_set);

	/**
	 * Finalise the fetcher.
	 */
	व्योम (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_add(lwc_string *scheme, स्थिर काष्ठा fetcher_operation_table *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(व्योम);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
व्योम fetcher_quit(व्योम);


#पूर्ण_अगर
