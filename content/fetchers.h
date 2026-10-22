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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _NETSURF_DESKTOP_FETCHERS_H_
#ವ್ಯಾಖ್ಯಾನಿಸು _NETSURF_DESKTOP_FETCHERS_H_

#ಅನ್ತರ್ಭಾವ "utils/inet.h" /* this is necessary for the fd_set definition */
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

ರಚನೆ nsurl;
ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ;
ರಚನೆ fetch;

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
ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ {
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
	bool (*acceptable)(ಸ್ಥಿರಾಂಕ ರಚನೆ nsurl *url);

	/**
	 * Setup a fetch
	 */
	ಶೂನ್ಯ *(*ಸಿದ್ಧತೆ)(ರಚನೆ fetch *ಪೋಷಕ_fetch, ರಚನೆ nsurl *url,
		bool only_2xx, bool downgrade_tls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು);

	/**
	 * start a fetch.
	 */
	bool (*ಆರಂಭ)(ಶೂನ್ಯ *fetch);

	/**
	 * abort a fetch.
	 */
	ಶೂನ್ಯ (*abort)(ಶೂನ್ಯ *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	ಶೂನ್ಯ (*ಬಿಡುಗಡೆ)(ಶೂನ್ಯ *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	ಶೂನ್ಯ (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	ಪೂರ್ಣಾಂಕ (*fdset)(lwc_string *scheme, fd_set *read_ಗಣ, fd_set *write_ಗಣ,
		     fd_set *error_ಗಣ);

	/**
	 * Finalise the fetcher.
	 */
	ಶೂನ್ಯ (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_ಕೂಡಿಸು(lwc_string *scheme, ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(ಶೂನ್ಯ);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
ಶೂನ್ಯ fetcher_quit(ಶೂನ್ಯ);


#ಆದರೆ_ಅಂತ್ಯ
