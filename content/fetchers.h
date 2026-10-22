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

#अन्तर्भाव "utils/inet.h" /* this is necessary for the fd_set definition */
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

संरचना nsurl;
संरचना fetch_multipart_data;
संरचना fetch;

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
संरचना fetcher_operation_table {
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
	bool (*acceptable)(ध्रुव संरचना nsurl *url);

	/**
	 * Setup a fetch
	 */
	शून्य *(*setup)(संरचना fetch *parent_fetch, संरचना nsurl *url,
		bool only_2xx, bool downgrade_tls, ध्रुव वर्ण *post_urlenc,
		ध्रुव संरचना fetch_multipart_data *post_multipart,
		ध्रुव वर्ण **headers);

	/**
	 * start a fetch.
	 */
	bool (*start)(शून्य *fetch);

	/**
	 * abort a fetch.
	 */
	शून्य (*abort)(शून्य *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	शून्य (*मुक्त)(शून्य *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	शून्य (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	पूर्णाङ्क (*fdset)(lwc_string *scheme, fd_set *read_set, fd_set *write_set,
		     fd_set *error_set);

	/**
	 * Finalise the fetcher.
	 */
	शून्य (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_add(lwc_string *scheme, ध्रुव संरचना fetcher_operation_table *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(शून्य);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
शून्य fetcher_quit(शून्य);


#endif
