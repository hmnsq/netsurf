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

#اگر_غیر_تعریف_شدہ _NETSURF_DESKTOP_FETCHERS_H_
#تعریف_کرو _NETSURF_DESKTOP_FETCHERS_H_

#شامل "utils/inet.h" /* this is necessary for the fd_set definition */
#شامل <libwapcaplet/libwapcaplet.h>

ساخت nsurl;
ساخت fetch_multipart_ڈیٹا;
ساخت fetch;

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
ساخت fetcher_operation_جدول {
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
	bool (*acceptable)(ثابت ساخت nsurl *url);

	/**
	 * Setup a fetch
	 */
	خالی *(*تیاری)(ساخت fetch *والد_fetch, ساخت nsurl *url,
		bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		ثابت حرف **سرخیاں);

	/**
	 * start a fetch.
	 */
	bool (*آغاز)(خالی *fetch);

	/**
	 * abort a fetch.
	 */
	خالی (*abort)(خالی *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	خالی (*آزاد)(خالی *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	خالی (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	صحیح (*fdset)(lwc_string *scheme, fd_set *read_سیٹ, fd_set *write_سیٹ,
		     fd_set *error_سیٹ);

	/**
	 * Finalise the fetcher.
	 */
	خالی (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_جوڑو(lwc_string *scheme, ثابت ساخت fetcher_operation_جدول *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(خالی);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
خالی fetcher_quit(خالی);


#اگر_ختم
