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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _NETSURF_DESKTOP_FETCHERS_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _NETSURF_DESKTOP_FETCHERS_H_

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/inet.h" /* this is necessary for the fd_set definition */
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

ਬਣਤਰ nsurl;
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ;
ਬਣਤਰ fetch;

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
ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ {
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
	bool (*acceptable)(ਅਚਲ ਬਣਤਰ nsurl *url);

	/**
	 * Setup a fetch
	 */
	ਖਾਲੀ *(*ਤਿਆਰੀ)(ਬਣਤਰ fetch *ਮਾਪਾ_fetch, ਬਣਤਰ nsurl *url,
		bool only_2xx, bool downgrade_tls, ਅਚਲ ਅੱਖਰ *post_urlenc,
		ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ);

	/**
	 * start a fetch.
	 */
	bool (*ਸ਼ੁਰੂ)(ਖਾਲੀ *fetch);

	/**
	 * abort a fetch.
	 */
	ਖਾਲੀ (*abort)(ਖਾਲੀ *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	ਖਾਲੀ (*ਮੁਕਤ)(ਖਾਲੀ *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	ਖਾਲੀ (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	ਪੂਰਨ (*fdset)(lwc_string *scheme, fd_set *read_ਸੈੱਟ, fd_set *write_ਸੈੱਟ,
		     fd_set *error_ਸੈੱਟ);

	/**
	 * Finalise the fetcher.
	 */
	ਖਾਲੀ (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_ਜੋੜੋ(lwc_string *scheme, ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(ਖਾਲੀ);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
ਖਾਲੀ fetcher_quit(ਖਾਲੀ);


#ਜੇ_ਅੰਤ
