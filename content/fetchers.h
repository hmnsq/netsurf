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

#અન્તર્ભાવ "utils/inet.h" /* this is necessary for the fd_set definition */
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

સંરચના nsurl;
સંરચના fetch_multipart_data;
સંરચના fetch;

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
સંરચના fetcher_operation_table {
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
	bool (*acceptable)(અચલ સંરચના nsurl *url);

	/**
	 * Setup a fetch
	 */
	શૂન્ય *(*setup)(સંરચના fetch *parent_fetch, સંરચના nsurl *url,
		bool only_2xx, bool downgrade_tls, અચલ અક્ષર *post_urlenc,
		અચલ સંરચના fetch_multipart_data *post_multipart,
		અચલ અક્ષર **headers);

	/**
	 * start a fetch.
	 */
	bool (*start)(શૂન્ય *fetch);

	/**
	 * abort a fetch.
	 */
	શૂન્ય (*abort)(શૂન્ય *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	શૂન્ય (*મુક્ત)(શૂન્ય *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	શૂન્ય (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	પૂર્ણાંક (*fdset)(lwc_string *scheme, fd_set *read_set, fd_set *write_set,
		     fd_set *error_set);

	/**
	 * Finalise the fetcher.
	 */
	શૂન્ય (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_add(lwc_string *scheme, અચલ સંરચના fetcher_operation_table *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(શૂન્ય);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
શૂન્ય fetcher_quit(શૂન્ય);


#endif
