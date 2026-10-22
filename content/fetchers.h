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

#جے_بنا_تعریف _NETSURF_DESKTOP_FETCHERS_H_
#تعریف_کرو _NETSURF_DESKTOP_FETCHERS_H_

#ضمن "utils/inet.h" /* this is necessary for the fd_set definition */
#ضمن <libwapcaplet/libwapcaplet.h>

بنية nsurl;
بنية fetch_multipart_انکڑے;
بنية fetch;

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
بنية fetcher_operation_جدول {
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
	bool (*acceptable)(ثابت بنية nsurl *url);

	/**
	 * Setup a fetch
	 */
	فراغ *(*تیاری)(بنية fetch *ماپا_fetch, بنية nsurl *url,
		bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		ثابت بنية fetch_multipart_انکڑے *post_multipart,
		ثابت حرف **سرخیاں);

	/**
	 * start a fetch.
	 */
	bool (*شروع)(فراغ *fetch);

	/**
	 * abort a fetch.
	 */
	فراغ (*abort)(فراغ *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	فراغ (*حرر)(فراغ *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	فراغ (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	صحيح (*fdset)(lwc_string *scheme, fd_set *read_سیٹ, fd_set *write_سیٹ,
		     fd_set *error_سیٹ);

	/**
	 * Finalise the fetcher.
	 */
	فراغ (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_جوڑو(lwc_string *scheme, ثابت بنية fetcher_operation_جدول *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(فراغ);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
فراغ fetcher_quit(فراغ);


#جے_انت
