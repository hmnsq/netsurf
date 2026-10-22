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

#ifndef _NETSURF_DESKTOP_FETCHERS_H_
#define _NETSURF_DESKTOP_FETCHERS_H_

#include "utils/inet.h" /* this is necessary for the fd_set definition */
#include <libwapcaplet/libwapcaplet.h>

sa_nracanaa nsurl;
sa_nracanaa fetch_multipart_data;
sa_nracanaa fetch;

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
sa_nracanaa fetcher_operation_table {
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
	bool (*acceptable)(const sa_nracanaa nsurl *url);

	/**
	 * Setup a fetch
	 */
	void *(*setup)(sa_nracanaa fetch *parent_fetch, sa_nracanaa nsurl *url,
		bool only_2xx, bool downgrade_tls, const var_na *post_urlenc,
		const sa_nracanaa fetch_multipart_data *post_multipart,
		const var_na **headers);

	/**
	 * start a fetch.
	 */
	bool (*start)(void *fetch);

	/**
	 * abort a fetch.
	 */
	void (*abort)(void *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	void (*free)(void *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	void (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	ཧྲིལ (*fdset)(lwc_string *scheme, fd_set *read_set, fd_set *write_set,
		     fd_set *error_set);

	/**
	 * Finalise the fetcher.
	 */
	void (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_add(lwc_string *scheme, const sa_nracanaa fetcher_operation_table *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(void);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
void fetcher_quit(void);


#endif
