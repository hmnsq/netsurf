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

#నిర్వచితం_కాకపోతే _NETSURF_DESKTOP_FETCHERS_H_
#నిర్వచించు _NETSURF_DESKTOP_FETCHERS_H_

#అన్తర్భావ "utils/inet.h" /* this is necessary for the fd_set definition */
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

నిర్మాణం nsurl;
నిర్మాణం fetch_multipart_దత్తాంశం;
నిర్మాణం fetch;

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
నిర్మాణం fetcher_operation_పట్టిక {
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
	bool (*acceptable)(స్థిరరాశి నిర్మాణం nsurl *url);

	/**
	 * Setup a fetch
	 */
	శూన్య *(*అమరిక)(నిర్మాణం fetch *మాతృ_fetch, నిర్మాణం nsurl *url,
		bool only_2xx, bool downgrade_tls, స్థిరరాశి అక్షరం *post_urlenc,
		స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		స్థిరరాశి అక్షరం **శీర్షికలు);

	/**
	 * start a fetch.
	 */
	bool (*ప్రారంభం)(శూన్య *fetch);

	/**
	 * abort a fetch.
	 */
	శూన్య (*abort)(శూన్య *fetch);

	/**
	 * free a fetch allocated through the setup method.
	 */
	శూన్య (*విడుదల)(శూన్య *fetch);

	/**
	 * poll a fetcher to let it make progress.
	 */
	శూన్య (*poll)(lwc_string *scheme);

	/**
	 * update an fdset with the FDs needed to poll cleanly
	 */
	పూర్ణాంకం (*fdset)(lwc_string *scheme, fd_set *read_సమితి, fd_set *write_సమితి,
		     fd_set *error_సమితి);

	/**
	 * Finalise the fetcher.
	 */
	శూన్య (*finalise)(lwc_string *scheme);
};


/**
 * Register a fetcher for a scheme
 *
 * \param scheme The scheme fetcher is for (caller relinquishes ownership)
 * \param ops The operations for the fetcher.
 * \return NSERROR_OK or appropriate error code.
 */
nserror fetcher_కూడు(lwc_string *scheme, స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక *ops);


/**
 * Initialise all registered fetchers.
 *
 * \return NSERROR_OK or error code
 */
nserror fetcher_init(శూన్య);


/**
 * Clean up for quit.
 *
 * Must be called before exiting.
 */
శూన్య fetcher_quit(శూన్య);


#అయితే_ముగింపు
