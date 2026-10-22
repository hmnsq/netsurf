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

#ଅନ୍ତର୍ଭାଵ "utils/inet.h" /* this is necessary for the fd_set definition */
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

ସଂରଚନା nsurl;
ସଂରଚନା fetch_multipart_data;
ସଂରଚନା fetch;

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
ସଂରଚନା fetcher_operation_table {
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
	bool (*acceptable)(ଅଚଳ ସଂରଚନା nsurl *url);

	/**
	 * Setup a fetch
	 */
	ଶୂନ୍ୟ *(*setup)(ସଂରଚନା fetch *parent_fetch, ସଂରଚନା nsurl *url,
		bool only_2xx, bool downgrade_tls, ଅଚଳ ଅକ୍ଷର *post_urlenc,
		ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		ଅଚଳ ଅକ୍ଷର **headers);

	/**
	 * start a fetch.
	 */
	bool (*start)(ଶୂନ୍ୟ *fetch);

	/**
	 * abort a fetch.
	 */
	ଶୂନ୍ୟ (*abort)(ଶୂନ୍ୟ *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	ଶୂନ୍ୟ (*ମୁକ୍ତ)(ଶୂନ୍ୟ *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	ଶୂନ୍ୟ (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*fdset)(lwc_string *scheme, fd_set *read_set, fd_set *write_set,
		     fd_set *error_set);

	/**
	 * Finalise the fetcher.
	 */
	ଶୂନ୍ୟ (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_add(lwc_string *scheme, ଅଚଳ ସଂରଚନା fetcher_operation_table *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(ଶୂନ୍ୟ);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
ଶୂନ୍ୟ fetcher_quit(ଶୂନ୍ୟ);


#endif
