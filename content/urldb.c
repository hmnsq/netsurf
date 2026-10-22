/*
 * Copyright 2006 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2009 John Tytgat <joty@netsurf-browser.org>
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
 * \file
 * Unified URL information database implementation
 *
 * URLs are stored in a tree-based structure as follows:
 *
 * The host component is extracted from each URL and, if a FQDN, split on
 * every '.'.The tree is constructed by inserting each FQDN segment in
 * reverse order. Duplicate nodes are merged.
 *
 * If the host part of an URL is an IP address, then this is added to the
 * tree verbatim (as if it were a TLD).
 *
 * This provides something looking like:
 *
 *			      root (a sentinel)
 *				|
 *	-------------------------------------------------
 *	|	|	|	|	|	|	|
 *     com     edu     gov  127.0.0.1  net     org     uk	TLDs
 *	|	|	|		|	|	|
 *    google   ...     ...             ...     ...     co	2LDs
 *	|						|
 *     www					       bbc  Hosts/Subdomains
 *							|
 *						       www	...
 *
 * Each of the nodes in this tree is a struct host_part. This stores the
 * FQDN segment (or IP address) with which the node is concerned. Each node
 * may contain further information about paths on a host (struct path_data)
 * or SSL certificate processing on a host-wide basis
 * (host_part::permit_invalid_certs).
 *
 * Path data is concerned with storing various metadata about the path in
 * question. This includes global history data, HTTP authentication details
 * and any associated HTTP cookies. This is stored as a tree of path segments
 * hanging off the relevant host_part node.
 *
 * Therefore, to find the last visited time of the URL
 * http://www.example.com/path/to/resource.html, the FQDN tree would be
 * traversed in the order root -> "com" -> "example" -> "www". The "www"
 * node would have attached to it a tree of struct path_data:
 *
 *			    (sentinel)
 *				|
 *			       path
 *				|
 *			       to
 *				|
 *			   resource.html
 *
 * This represents the absolute path "/path/to/resource.html". The leaf node
 * "resource.html" contains the last visited time of the resource.
 *
 * The mechanism described above is, however, not particularly conducive to
 * fast searching of the database for a given URL (or URLs beginning with a
 * given prefix). Therefore, an anciliary data structure is used to enable
 * fast searching. This structure simply reflects the contents of the
 * database, with entries being added/removed at the same time as for the
 * core database. In order to ensure that degenerate cases are kept to a
 * minimum, we use an AAtree. This is an approximation of a Red-Black tree
 * with similar performance characteristics, but with a significantly
 * simpler implementation. Entries in this tree comprise pointers to the
 * leaf nodes of the host tree described above.
 *
 * REALLY IMPORTANT NOTE: urldb expects all URLs to be normalised. Use of
 * non-normalised URLs with urldb will result in undefined behaviour and
 * potential crashes.
 */

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <time.h>
#నిర్వచితమైతే WITH_NSPSL
#అన్తర్భావ <nspsl.h>
#అయితే_ముగింపు

#అన్తర్భావ "utils/inet.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/url.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/bloom.h"
#అన్తర్భావ "utils/time.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "desktop/cookie_manager.h"

#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
నిర్మాణం cookie_అంతర్గత_దత్తాంశం {
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *prev;	/**< Previous in list */
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *తదుపరి;	/**< Next in list */

	అక్షరం *name;		/**< Cookie name */
	అక్షరం *విలువ;		/**< Cookie value */
	bool విలువ_was_quoted;	/**< Value was quoted in Set-Cookie: */
	అక్షరం *comment;		/**< Cookie comment */
	bool domain_from_సమితి;	/**< Domain came from Set-Cookie: header */
	అక్షరం *domain;		/**< Domain */
	bool మార్గం_from_సమితి;	/**< Path came from Set-Cookie: header */
	అక్షరం *మార్గం;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t చివరి_వాడిన;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	గణన cookie_సంస్కరణ సంస్కరణ;	/**< Specification compliance */
	bool no_destroy;	/**< Never destroy this cookie,
				 * unless it's expired */

};


/**
 * A protection space
 *
 * This is defined as a tuple canonical_root_url and realm.  This
 * structure lives as linked list element in a leaf host_part struct
 * so we need additional scheme and port to have a canonical_root_url.
 */
నిర్మాణం prot_space_దత్తాంశం {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	గుర్తులేని పూర్ణాంకం పోర్టు;
	/** Protection realm */
	అక్షరం *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	అక్షరం *auth;
	/** Next sibling */
	నిర్మాణం prot_space_దత్తాంశం *తదుపరి;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
నిర్మాణం url_అంతర్గత_దత్తాంశం {
	అక్షరం *title;		/**< Resource title */
	గుర్తులేని పూర్ణాంకం visits;	/**< Visit count */
	time_t చివరి_సందర్శించు;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
నిర్మాణం మార్గం_దత్తాంశం {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	గుర్తులేని పూర్ణాంకం పోర్టు;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	అక్షరం *segment;		/**< Path segment for this node */
	గుర్తులేని పూర్ణాంకం frag_cnt;	/**< Number of entries in path_data::fragment */
	అక్షరం **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	నిర్మాణం url_అంతర్గత_దత్తాంశం urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	స్థిరరాశి నిర్మాణం prot_space_దత్తాంశం *prot_space;
	/** Cookies associated with resource */
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *cookies;
	/** Last cookie in list */
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *cookies_చివర;

	నిర్మాణం మార్గం_దత్తాంశం *తదుపరి;	/**< Next sibling */
	నిర్మాణం మార్గం_దత్తాంశం *prev;	/**< Previous sibling */
	నిర్మాణం మార్గం_దత్తాంశం *మాతృ; /**< Parent path segment */
	నిర్మాణం మార్గం_దత్తాంశం *children; /**< Child path segments */
	నిర్మాణం మార్గం_దత్తాంశం *చివరి; /**< Last child */
};

నిర్మాణం hsts_దత్తాంశం {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

నిర్మాణం host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	నిర్మాణం మార్గం_దత్తాంశం paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	నిర్మాణం hsts_దత్తాంశం hsts;

	/**
	 * Part of host string
	 */
	అక్షరం *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	నిర్మాణం prot_space_దత్తాంశం *prot_space;

	నిర్మాణం host_part *తదుపరి;	/**< Next sibling */
	నిర్మాణం host_part *prev;	/**< Previous sibling */
	నిర్మాణం host_part *మాతృ; /**< Parent host part */
	నిర్మాణం host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
నిర్మాణం శోధన_కణుపు {
	స్థిరరాశి నిర్మాణం host_part *దత్తాంశం;	/**< Host tree entry */

	గుర్తులేని పూర్ణాంకం level;		/**< Node level */

	నిర్మాణం శోధన_కణుపు *ఎడమ;	/**< Left subtree */
	నిర్మాణం శోధన_కణుపు *కుడి;	/**< Right subtree */
};

/** Root database handle */
స్థిర నిర్మాణం host_part db_మూలం;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#నిర్వచించు అంకె_శోధన_TREES 28
#నిర్వచించు ST_IP 0
#నిర్వచించు ST_EE 1
#నిర్వచించు ST_DN 2
స్థిర నిర్మాణం శోధన_కణుపు ఖాళీలు = { 0, 0, &ఖాళీలు, &ఖాళీలు };
స్థిర నిర్మాణం శోధన_కణుపు *శోధన_trees[అంకె_శోధన_TREES] = {
	&ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు,
	&ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు,
	&ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు,
	&ఖాళీలు, &ఖాళీలు, &ఖాళీలు, &ఖాళీలు
};

/** Minimum cookie database file version */
#నిర్వచించు MIN_COOKIE_FILE_సంస్కరణ 100
/** Current cookie database file version */
#నిర్వచించు COOKIE_FILE_సంస్కరణ 102
/** loaded cookie file version */
స్థిర పూర్ణాంకం loaded_cookie_file_సంస్కరణ;

/** Minimum URL database file version */
#నిర్వచించు MIN_URL_FILE_సంస్కరణ 106
/** Current URL database file version */
#నిర్వచించు URL_FILE_సంస్కరణ 107

/**
 * filter for url presence in database
 *
 * Bloom filter used for short-circuting the false case of "is this
 * URL in the database?".  BLOOM_SIZE controls how large the filter is
 * in bytes.  Primitive experimentation shows that for a filter of X
 * bytes filled with X items, searching for X items not in the filter
 * has a 5% false-positive rate.  We set it to 32kB, which should be
 * enough for all but the largest databases, while not being
 * shockingly wasteful on memory.
 */
స్థిర నిర్మాణం bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#నిర్వచించు BLOOM_కొలత (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
స్థిర nserror urldb_write_timet(FILE *fp, time_t val)
{
	పూర్ణాంకం use;
	అక్షరం op[32];

	use = nsc_sntimet(op, 32, &val);
	ఒకవేళ (use == 0) {
		fprintf(fp, "%i\n", (పూర్ణాంకం)val);
	} లేకపోతే {
		fprintf(fp, "%.*s\n", use, op);
	}
	తిరిగి NSERROR_OK;
}

/**
 * Write paths associated with a host
 *
 * \param parent Root of (sub)tree to write
 * \param host Current host name
 * \param fp File to write to
 * \param path Current path string
 * \param path_alloc Allocated size of path
 * \param path_used Used size of path
 * \param expiry Expiry time of URLs
 */
స్థిర శూన్య
urldb_write_paths(స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *మాతృ,
		  స్థిరరాశి అక్షరం *host,
		  FILE *fp,
		  అక్షరం **మార్గం,
		  పూర్ణాంకం *మార్గం_alloc,
		  పూర్ణాంకం *మార్గం_వాడిన,
		  time_t expiry)
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ;
	పూర్ణాంకం i;

	చేయి {
		పూర్ణాంకం seglen = p->segment != ఖాళీ ? అక్షరశ్రేణి_పొడవు(p->segment) : 0;
		పూర్ణాంకం len = *మార్గం_వాడిన + seglen + 1;

		ఒకవేళ (*మార్గం_alloc < len) {
			అక్షరం *తాత్కాలిక;
			తాత్కాలిక = realloc(*మార్గం,
				       (len > 64) ? len : *మార్గం_alloc + 64);
			ఒకవేళ (!తాత్కాలిక) {
				తిరిగి;
			}
			*మార్గం = తాత్కాలిక;
			*మార్గం_alloc = (len > 64) ? len : *మార్గం_alloc + 64;
		}

		ఒకవేళ (p->segment != ఖాళీ) {
			జ్ఞాపకం_నకలు(*మార్గం + *మార్గం_వాడిన - 1, p->segment, seglen);
		}

		ఒకవేళ (p->children != ఖాళీ) {
			(*మార్గం)[*మార్గం_వాడిన + seglen - 1] = '/';
			(*మార్గం)[*మార్గం_వాడిన + seglen] = '\0';
		} లేకపోతే {
			(*మార్గం)[*మార్గం_వాడిన + seglen - 1] = '\0';
			len -= 1;
		}

		*మార్గం_వాడిన = len;

		ఒకవేళ (p->children != ఖాళీ) {
			/* Drill down into children */
			p = p->children;
		} లేకపోతే {
			/* leaf node */
			ఒకవేళ (p->persistent ||
			    ((p->urld.చివరి_సందర్శించు > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_దత్తాంశం(p->scheme));

				ఒకవేళ (p->పోర్టు) {
					fprintf(fp,"%d\n", p->పోర్టు);
				} లేకపోతే {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *మార్గం);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.చివరి_సందర్శించు);

				/* entry type */
				fprintf(fp, "%i\n", (పూర్ణాంకం)p->urld.type);

				fprintf(fp, "\n");

				ఒకవేళ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					కొరకు (i = 0; s[i] != '\0'; i++)
						ఒకవేళ (s[i] < 32)
							s[i] = ' ';
					కొరకు (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} లేకపోతే {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			అయితే (p != మాతృ) {
				పూర్ణాంకం seglen = p->segment != ఖాళీ
					? అక్షరశ్రేణి_పొడవు(p->segment) : 0;

				/* Remove our segment from the path */
				*మార్గం_వాడిన -= seglen;
				(*మార్గం)[*మార్గం_వాడిన - 1] = '\0';

				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					/* Have a sibling, process that */
					p = p->తదుపరి;
					ఆపు;
				}

				/* Going up, so remove '/' */
				*మార్గం_వాడిన -= 1;
				(*మార్గం)[*మార్గం_వాడిన - 1] = '\0';

				/* Ascend tree */
				p = p->మాతృ;
			}
		}
	} అయితే (p != మాతృ);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
స్థిర శూన్య
urldb_లెక్క_urls(స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *మూలం,
		 time_t expiry,
		 గుర్తులేని పూర్ణాంకం *లెక్క)
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p = మూలం;

	చేయి {
		ఒకవేళ (p->children != ఖాళీ) {
			/* Drill down into children */
			p = p->children;
		} లేకపోతే {
			/* No more children, increment count if required */
			ఒకవేళ (p->persistent ||
			    ((p->urld.చివరి_సందర్శించు > expiry) &&
			     (p->urld.visits > 0))) {
				(*లెక్క)++;
			}

			/* Now, find next node to process. */
			అయితే (p != మూలం) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					/* Have a sibling, process that */
					p = p->తదుపరి;
					ఆపు;
				}

				/* Ascend tree */
				p = p->మాతృ;
			}
		}
	} అయితే (p != మూలం);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
స్థిర శూన్య urldb_save_శోధన_వృక్షం(నిర్మాణం శోధన_కణుపు *మాతృ, FILE *fp)
{
	అక్షరం host[256];
	స్థిరరాశి నిర్మాణం host_part *h;
	గుర్తులేని పూర్ణాంకం మార్గం_లెక్క = 0;
	అక్షరం *మార్గం, *p, *చివర;
	పూర్ణాంకం మార్గం_alloc = 64, మార్గం_వాడిన = 1;
	time_t expiry, hsts_expiry = 0;
	పూర్ణాంకం hsts_include_subdomains = 0;

	expiry = time(ఖాళీ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ఒకవేళ (మాతృ == &ఖాళీలు)
		తిరిగి;

	urldb_save_శోధన_వృక్షం(మాతృ->ఎడమ, fp);

	మార్గం = జ్ఞాపకం_ఇవ్వు(మార్గం_alloc);
	ఒకవేళ (!మార్గం)
		తిరిగి;

	మార్గం[0] = '\0';

	కొరకు (h = మాతృ->దత్తాంశం, p = host, చివర = host + పరిమాణం host;
	     h && h != &db_మూలం && p < చివర; h = h->మాతృ) {
		పూర్ణాంకం written = snprintf(p, చివర - p, "%s%s", h->part,
				       (h->మాతృ && h->మాతృ->మాతృ) ? "." : "");
		ఒకవేళ (written < 0) {
			విడుదల(మార్గం);
			తిరిగి;
		}
		p += written;
	}

	h = మాతృ->దత్తాంశం;
	ఒకవేళ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_లెక్క_urls(&మాతృ->దత్తాంశం->paths, expiry, &మార్గం_లెక్క);

	ఒకవేళ (మార్గం_లెక్క > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", మార్గం_లెక్క);

		urldb_write_paths(&మాతృ->దత్తాంశం->paths, host, fp,
				  &మార్గం, &మార్గం_alloc, &మార్గం_వాడిన, expiry);
	} లేకపోతే ఒకవేళ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	విడుదల(మార్గం);

	urldb_save_శోధన_వృక్షం(మాతృ->కుడి, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
స్థిర bool
urldb_iterate_entries_మార్గం(స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *మాతృ,
		bool (*url_callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం),
		bool (*cookie_callback)(స్థిరరాశి నిర్మాణం cookie_దత్తాంశం *దత్తాంశం))
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ;
	స్థిరరాశి నిర్మాణం cookie_దత్తాంశం *c;

	చేయి {
		ఒకవేళ (p->children != ఖాళీ) {
			/* Drill down into children */
			p = p->children;
		} లేకపోతే {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ప్రతిజ్ఞా(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ఒకవేళ (url_callback) {
				స్థిరరాశి నిర్మాణం url_అంతర్గత_దత్తాంశం *u = &p->urld;

				ప్రతిజ్ఞా(p->url);

				ఒకవేళ (!url_callback(p->url,
						  (స్థిరరాశి నిర్మాణం url_దత్తాంశం *) u))
					తిరిగి false;
			} లేకపోతే {
				c = (స్థిరరాశి నిర్మాణం cookie_దత్తాంశం *)p->cookies;
				కొరకు (; c != ఖాళీ; c = c->తదుపరి) {
					ఒకవేళ (!cookie_callback(c))
						తిరిగి false;
				}
			}

			/* Now, find next node to process. */
			అయితే (p != మాతృ) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					/* Have a sibling, process that */
					p = p->తదుపరి;
					ఆపు;
				}

				/* Ascend tree */
				p = p->మాతృ;
			}
		}
	} అయితే (p != మాతృ);

	తిరిగి true;
}


/**
 * Check whether a host string is an IP address.
 *
 * This call detects IPv4 addresses (all of dotted-quad or subsets,
 * decimal or hexadecimal notations) and IPv6 addresses (including
 * those containing embedded IPv4 addresses.)
 *
 * \param host a hostname terminated by '\0'
 * \return true if the hostname is an IP address, false otherwise
 */
స్థిర bool urldb_host_is_ip_చిరునామా(స్థిరరాశి అక్షరం *host)
{
	నిర్మాణం లోపల_addr ipv4;
	size_t host_len = అక్షరశ్రేణి_పొడవు(host);
	స్థిరరాశి అక్షరం *sane_host;
	స్థిరరాశి అక్షరం *slash;
#నిర్వచితం_కాకపోతే NO_IPV6
	నిర్మాణం in6_addr ipv6;
	అక్షరం ipv6_addr[64];
	గుర్తులేని పూర్ణాంకం ipv6_addr_len;
#అయితే_ముగింపు
	/**
	 * @todo FIXME Some parts of urldb.c make confusions between hosts
	 * and "prefixes", we can sometimes be erroneously passed more than
	 * just a host.  Sometimes we may be passed trailing slashes, or even
	 * whole path segments.  A specific criminal in this class is
	 * urldb_iterate_partial, which takes a prefix to search for, but
	 * passes that prefix to functions that expect only hosts.
	 *
	 * For the time being, we will accept such calls; we check if there
	 * is a / in the host parameter, and if there is, we take a copy and
	 * replace the / with a \0.  This is not a permanent solution; we
	 * should search through NetSurf and find all the callers that are
	 * in error and fix them.  When doing this task, it might be wise
	 * to replace the hideousness below with code that doesn't have to do
	 * this, and add assert(strchr(host, '/') == NULL); somewhere.
	 * -- rjek - 2010-11-04
	 */

	slash = strchr(host, '/');
	ఒకవేళ (slash == ఖాళీ) {
		sane_host = host;
	} లేకపోతే {
		అక్షరం *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ఒకవేళ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		వెళ్ళు బహిర్గతం_false;

	ఒకవేళ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		పూర్ణాంకం అంకె_dots = 0;
		size_t index;

		కొరకు (index = 0; index < host_len; index++) {
			ఒకవేళ (sane_host[index] == '.')
				అంకె_dots++;
		}

		ఒకవేళ (అంకె_dots == 3)
			వెళ్ళు బహిర్గతం_true;
		లేకపోతే
			వెళ్ళు బహిర్గతం_false;
	}

#నిర్వచితం_కాకపోతే NO_IPV6
	ఒకవేళ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		వెళ్ళు బహిర్గతం_false;
	}

	ipv6_addr_len = host_len - 2;
	ఒకవేళ (ipv6_addr_len >= పరిమాణం(ipv6_addr)) {
		ipv6_addr_len = పరిమాణం(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ఒకవేళ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		వెళ్ళు బహిర్గతం_true;
#అయితే_ముగింపు

బహిర్గతం_false:
	ఒకవేళ (slash != ఖాళీ) విడుదల((శూన్య *)sane_host);
	తిరిగి false;

బహిర్గతం_true:
	ఒకవేళ (slash != ఖాళీ) విడుదల((శూన్య *)sane_host);
	తిరిగి true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
స్థిర పూర్ణాంకం urldb_శోధన_సరిపోలిక_ఉపసర్గ(స్థిరరాశి నిర్మాణం host_part *a, స్థిరరాశి అక్షరం *b)
{
	స్థిరరాశి అక్షరం *చివర, *dot;
	పూర్ణాంకం plen, ret;

	ప్రతిజ్ఞా(a && a != &db_మూలం && b);

	ఒకవేళ (urldb_host_is_ip_చిరునామా(b)) {
		/* IP address */
		తిరిగి strncasecmp(a->part, b, అక్షరశ్రేణి_పొడవు(b));
	}

	చివర = b + అక్షరశ్రేణి_పొడవు(b) + 1;

	అయితే (b < చివర && a && a != &db_మూలం) {
		dot = strchr(b, '.');
		ఒకవేళ (!dot) {
			/* last segment */
			dot = చివర - 1;
		}

		/* Compare strings (length limited) */
		ఒకవేళ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			తిరిగి ret;

		/* The strings matched */
		ఒకవేళ (dot < చివర - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = అక్షరశ్రేణి_పొడవు(a->part);
			ఒకవేళ (plen > dot - b) {
				/* len(a) > len(b) */
				తిరిగి 1;
			} లేకపోతే ఒకవేళ (plen < dot - b) {
				/* len(a) < len(b) */
				తిరిగి -1;
			}
		}

		b = dot + 1;
		a = a->మాతృ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ఒకవేళ (a && a != &db_మూలం && b >= చివర) {
		/* len(a) > len(b) => prefix matches */
		తిరిగి 0;
	} లేకపోతే ఒకవేళ ((!a || a == &db_మూలం) && b < చివర) {
		/* len(a) < len(b) => prefix does not match */
		తిరిగి -1;
	}

	/* Identical */
	తిరిగి 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
స్థిర bool
urldb_iterate_partial_host(నిర్మాణం శోధన_కణుపు *మూలం,
		స్థిరరాశి అక్షరం *ఉపసర్గ,
		bool (*callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం))
{
	పూర్ణాంకం c;

	ప్రతిజ్ఞా(మూలం && ఉపసర్గ && callback);

	ఒకవేళ (మూలం == &ఖాళీలు)
		తిరిగి true;

	c = urldb_శోధన_సరిపోలిక_ఉపసర్గ(మూలం->దత్తాంశం, ఉపసర్గ);

	ఒకవేళ (c > 0) {
		/* No match => look in left subtree */
		తిరిగి urldb_iterate_partial_host(మూలం->ఎడమ,
						  ఉపసర్గ,
						  callback);
	} లేకపోతే ఒకవేళ (c < 0) {
		/* No match => look in right subtree */
		తిరిగి urldb_iterate_partial_host(మూలం->కుడి,
						  ఉపసర్గ,
						  callback);
	} లేకపోతే {
		/* Match => iterate over l/r subtrees & process this node */
		ఒకవేళ (!urldb_iterate_partial_host(మూలం->ఎడమ,
						ఉపసర్గ,
						callback)) {
			తిరిగి false;
		}

		ఒకవేళ (మూలం->దత్తాంశం->paths.children) {
			/* and extract all paths attached to this host */
			ఒకవేళ (!urldb_iterate_entries_మార్గం(&మూలం->దత్తాంశం->paths,
							callback,
							ఖాళీ)) {
				తిరిగి false;
			}
		}

		ఒకవేళ (!urldb_iterate_partial_host(మూలం->కుడి,
						ఉపసర్గ,
						callback)) {
			తిరిగి false;
		}
	}

	తిరిగి true;
}


/**
 * Partial path iterator (internal)
 *
 * Given: http://www.example.org/a/b/c/d//e
 * and assuming a path tree:
 *     ^
 *    /	\
 *   a1 b1
 *  / \
 * a2 b2
 *    /|\
 *   a b c
 *   3 3 |
 *       d
 *       |
 *       e
 *      / \
 *      f g
 *
 * Prefix will be:	p will be:
 *
 * a/b/c/d//e		a1
 *   b/c/d//e		a2
 *   b/c/d//e		b3
 *     c/d//e		a3
 *     c/d//e		b3
 *     c/d//e		c
 *       d//e		d
 *         /e		e		(skip /)
 *          e		e
 *
 * I.E. perform a breadth-first search of the tree.
 *
 * \param parent Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
స్థిర bool
urldb_iterate_partial_మార్గం(స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *మాతృ,
		స్థిరరాశి అక్షరం *ఉపసర్గ,
		bool (*callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం))
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ->children;
	స్థిరరాశి అక్షరం *slash, *చివర = ఉపసర్గ + అక్షరశ్రేణి_పొడవు(ఉపసర్గ);

	చేయి {
		slash = strchr(ఉపసర్గ, '/');
		ఒకవేళ (!slash) {
			slash = చివర;
		}

		ఒకవేళ (slash == ఉపసర్గ && *ఉపసర్గ == '/') {
			/* Ignore "//" */
			ఉపసర్గ++;
			కొనసాగు;
		}

		ఒకవేళ (strncasecmp(p->segment, ఉపసర్గ, slash - ఉపసర్గ) == 0) {
			/* prefix matches so far */
			ఒకవేళ (slash == చివర) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ఒకవేళ (!urldb_iterate_entries_మార్గం(p,
								callback,
								ఖాళీ)) {
					తిరిగి false;
				}

				/* Progress to next sibling */
				p = p->తదుపరి;
			} లేకపోతే {
				/* Skip over this segment */
				ఉపసర్గ = slash + 1;

				p = p->children;
			}
		} లేకపోతే {
			/* Doesn't match this segment, try next sibling */
			p = p->తదుపరి;
		}
	} అయితే (p != ఖాళీ);

	తిరిగి true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
స్థిర bool
urldb_iterate_entries_host(నిర్మాణం శోధన_కణుపు *మాతృ,
		bool (*url_callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం),
		bool (*cookie_callback)(స్థిరరాశి నిర్మాణం cookie_దత్తాంశం *దత్తాంశం))
{
	ఒకవేళ (మాతృ == &ఖాళీలు) {
		తిరిగి true;
	}

	ఒకవేళ (!urldb_iterate_entries_host(మాతృ->ఎడమ,
					url_callback,
					cookie_callback)) {
		తిరిగి false;
	}

	ఒకవేళ ((మాతృ->దత్తాంశం->paths.children) ||
	    ((cookie_callback) &&
	     (మాతృ->దత్తాంశం->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ఒకవేళ (!urldb_iterate_entries_మార్గం(&మాతృ->దత్తాంశం->paths,
						url_callback,
						cookie_callback)) {
			తిరిగి false;
		}
	}

	ఒకవేళ (!urldb_iterate_entries_host(మాతృ->కుడి,
					url_callback,
					cookie_callback)) {
		తిరిగి false;
	}

	తిరిగి true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
స్థిర నిర్మాణం host_part *
urldb_కూడు_host_కణుపు(స్థిరరాశి అక్షరం *part, నిర్మాణం host_part *మాతృ)
{
	నిర్మాణం host_part *d;

	ప్రతిజ్ఞా(part && మాతృ);

	d = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం host_part));
	ఒకవేళ (!d) {
		తిరిగి ఖాళీ;
	}

	d->part = strdup(part);
	ఒకవేళ (!d->part) {
		విడుదల(d);
		తిరిగి ఖాళీ;
	}

	d->తదుపరి = మాతృ->children;
	ఒకవేళ (మాతృ->children) {
		మాతృ->children->prev = d;
	}
	d->మాతృ = మాతృ;
	మాతృ->children = d;

	తిరిగి d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
స్థిర పూర్ణాంకం urldb_కూడు_మార్గం_fragment_cmp(స్థిరరాశి శూన్య *a, స్థిరరాశి శూన్య *b)
{
	తిరిగి strcasecmp(*((స్థిరరాశి అక్షరం **) a), *((స్థిరరాశి అక్షరం **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
స్థిర నిర్మాణం మార్గం_దత్తాంశం *
urldb_కూడు_మార్గం_fragment(నిర్మాణం మార్గం_దత్తాంశం *segment, lwc_string *fragment)
{
	అక్షరం **తాత్కాలిక;

	ప్రతిజ్ఞా(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ఒకవేళ (!fragment)
		తిరిగి segment;

	తాత్కాలిక = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * పరిమాణం(అక్షరం *));
	ఒకవేళ (!తాత్కాలిక)
		తిరిగి ఖాళీ;

	segment->fragment = తాత్కాలిక;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_దత్తాంశం(fragment));
	ఒకవేళ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		తిరిగి ఖాళీ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      పరిమాణం (అక్షరం *),
	      urldb_కూడు_మార్గం_fragment_cmp);

	తిరిగి segment;
}


/**
 * Add a path node to the tree
 *
 * \param scheme URL scheme associated with path (copied)
 * \param port Port number on host associated with path
 * \param segment Path segment to add (copied)
 * \param fragment URL fragment (copied), or NULL
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
స్థిర నిర్మాణం మార్గం_దత్తాంశం *
urldb_కూడు_మార్గం_కణుపు(lwc_string *scheme,
		    గుర్తులేని పూర్ణాంకం పోర్టు,
		    స్థిరరాశి అక్షరం *segment,
		    lwc_string *fragment,
		    నిర్మాణం మార్గం_దత్తాంశం *మాతృ)
{
	నిర్మాణం మార్గం_దత్తాంశం *d, *e;

	ప్రతిజ్ఞా(scheme && segment && మాతృ);

	d = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం మార్గం_దత్తాంశం));
	ఒకవేళ (!d)
		తిరిగి ఖాళీ;

	d->scheme = lwc_string_ref(scheme);

	d->పోర్టు = పోర్టు;

	d->segment = strdup(segment);
	ఒకవేళ (!d->segment) {
		lwc_string_unref(d->scheme);
		విడుదల(d);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (fragment) {
		ఒకవేళ (!urldb_కూడు_మార్గం_fragment(d, fragment)) {
			విడుదల(d->segment);
			lwc_string_unref(d->scheme);
			విడుదల(d);
			తిరిగి ఖాళీ;
		}
	}

	కొరకు (e = మాతృ->children; e; e = e->తదుపరి) {
		ఒకవేళ (strcmp(e->segment, d->segment) > 0)
			ఆపు;
	}

	ఒకవేళ (e) {
		d->prev = e->prev;
		d->తదుపరి = e;
		ఒకవేళ (e->prev)
			e->prev->తదుపరి = d;
		లేకపోతే
			మాతృ->children = d;
		e->prev = d;
	} లేకపోతే ఒకవేళ (!మాతృ->children) {
		d->prev = d->తదుపరి = ఖాళీ;
		మాతృ->children = మాతృ->చివరి = d;
	} లేకపోతే {
		d->తదుపరి = ఖాళీ;
		d->prev = మాతృ->చివరి;
		మాతృ->చివరి->తదుపరి = d;
		మాతృ->చివరి = d;
	}
	d->మాతృ = మాతృ;

	తిరిగి d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
స్థిర నిర్మాణం శోధన_కణుపు **urldb_get_శోధన_వృక్షం_ప్రత్యక్ష(స్థిరరాశి అక్షరం *host)
{
	ప్రతిజ్ఞా(host);

	ఒకవేళ (urldb_host_is_ip_చిరునామా(host)) {
		తిరిగి &శోధన_trees[ST_IP];
	} లేకపోతే ఒకవేళ (ascii_is_alpha(*host)) {
		తిరిగి &శోధన_trees[ST_DN + ascii_గమ్యం_క్రింది(*host) - 'a'];
	}
	తిరిగి &శోధన_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
స్థిర నిర్మాణం శోధన_కణుపు *urldb_get_శోధన_వృక్షం(స్థిరరాశి అక్షరం *host)
{
	తిరిగి *urldb_get_శోధన_వృక్షం_ప్రత్యక్ష(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
స్థిర పూర్ణాంకం urldb_శోధన_సరిపోలిక_string(స్థిరరాశి నిర్మాణం host_part *a, స్థిరరాశి అక్షరం *b)
{
	స్థిరరాశి అక్షరం *చివర, *dot;
	పూర్ణాంకం plen, ret;

	ప్రతిజ్ఞా(a && a != &db_మూలం && b);

	ఒకవేళ (urldb_host_is_ip_చిరునామా(b)) {
		/* IP address */
		తిరిగి strcasecmp(a->part, b);
	}

	చివర = b + అక్షరశ్రేణి_పొడవు(b) + 1;

	అయితే (b < చివర && a && a != &db_మూలం) {
		dot = strchr(b, '.');
		ఒకవేళ (!dot) {
			/* last segment */
			dot = చివర - 1;
		}

		/* Compare strings (length limited) */
		ఒకవేళ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			తిరిగి ret;

		/* The strings matched, now check that the lengths do, too */
		plen = అక్షరశ్రేణి_పొడవు(a->part);

		ఒకవేళ (plen > dot - b) {
			/* len(a) > len(b) */
			తిరిగి 1;
		} లేకపోతే ఒకవేళ (plen < dot - b) {
			/* len(a) < len(b) */
			తిరిగి -1;
		}

		b = dot + 1;
		a = a->మాతృ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ఒకవేళ (a && a != &db_మూలం && b >= చివర) {
		/* len(a) > len(b) */
		తిరిగి 1;
	} లేకపోతే ఒకవేళ ((!a || a == &db_మూలం) && b < చివర) {
		/* len(a) < len(b) */
		తిరిగి -1;
	}

	/* Identical */
	తిరిగి 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
స్థిర స్థిరరాశి నిర్మాణం host_part *
urldb_శోధన_కనుగొను(నిర్మాణం శోధన_కణుపు *మూలం, స్థిరరాశి అక్షరం *host)
{
	పూర్ణాంకం c;

	ప్రతిజ్ఞా(మూలం && host);

	ఒకవేళ (మూలం == &ఖాళీలు) {
		తిరిగి ఖాళీ;
	}

	c = urldb_శోధన_సరిపోలిక_string(మూలం->దత్తాంశం, host);

	ఒకవేళ (c > 0) {
		తిరిగి urldb_శోధన_కనుగొను(మూలం->ఎడమ, host);
	} లేకపోతే ఒకవేళ (c < 0) {
		తిరిగి urldb_శోధన_కనుగొను(మూలం->కుడి, host);
	}

	తిరిగి మూలం->దత్తాంశం;
}


/**
 * Match a path string
 *
 * \param parent Path (sub)tree to look in
 * \param path The path to search for
 * \param scheme The URL scheme associated with the path
 * \param port The port associated with the path
 * \return Pointer to path data or NULL if not found.
 */
స్థిర నిర్మాణం మార్గం_దత్తాంశం *
urldb_సరిపోలిక_మార్గం(స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *మాతృ,
		 స్థిరరాశి అక్షరం *మార్గం,
		 lwc_string *scheme,
		 గుర్తులేని పొట్టి పోర్టు)
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p;
	స్థిరరాశి అక్షరం *slash;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(మాతృ != ఖాళీ);
	ప్రతిజ్ఞా(మాతృ->segment == ఖాళీ);

	ఒకవేళ (మార్గం[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", మార్గం);
	}

	ప్రతిజ్ఞా(మార్గం[0] == '/');

	/* Start with children, as parent has no segment */
	p = మాతృ->children;

	అయితే (p != ఖాళీ) {
		slash = strchr(మార్గం + 1, '/');
		ఒకవేళ (!slash) {
			slash = మార్గం + అక్షరశ్రేణి_పొడవు(మార్గం);
		}

		ఒకవేళ (అక్షరశ్రేణి_పోలిక(p->segment, మార్గం + 1, slash - మార్గం - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &సరిపోలిక) == lwc_error_ok &&
		    సరిపోలిక == true &&
		    p->పోర్టు == పోర్టు) {
			ఒకవేళ (*slash == '\0') {
				/* Complete match */
				తిరిగి (నిర్మాణం మార్గం_దత్తాంశం *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			మార్గం = slash;
		} లేకపోతే {
			/* No match, try next sibling */
			p = p->తదుపరి;
		}
	}

	తిరిగి ఖాళీ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
స్థిర నిర్మాణం మార్గం_దత్తాంశం *urldb_కనుగొను_url(nsurl *url)
{
	స్థిరరాశి నిర్మాణం host_part *h;
	నిర్మాణం మార్గం_దత్తాంశం *p;
	నిర్మాణం శోధన_కణుపు *వృక్షం;
	అక్షరం *plq;
	స్థిరరాశి అక్షరం *host_str;
	lwc_string *scheme, *host, *పోర్టు;
	size_t len = 0;
	గుర్తులేని పూర్ణాంకం పోర్టు_int;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(url);

	ఒకవేళ (url_bloom != ఖాళీ) {
		ఒకవేళ (bloom_శోధన_hash(url_bloom, nsurl_hash(url)) == false) {
			తిరిగి ఖాళీ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (scheme == ఖాళీ)
		తిరిగి ఖాళీ;

	ఒకవేళ (lwc_string_isequal(scheme, corestring_lwc_mailto, &సరిపోలిక) ==
	    lwc_error_ok && సరిపోలిక == true) {
		lwc_string_unref(scheme);
		తిరిగి ఖాళీ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (host != ఖాళీ) {
		host_str = lwc_string_దత్తాంశం(host);
		lwc_string_unref(host);

	} లేకపోతే ఒకవేళ (lwc_string_isequal(scheme, corestring_lwc_file, &సరిపోలిక) ==
		   lwc_error_ok && సరిపోలిక == true) {
		host_str = "localhost";

	} లేకపోతే {
		lwc_string_unref(scheme);
		తిరిగి ఖాళీ;
	}

	వృక్షం = urldb_get_శోధన_వృక్షం(host_str);
	h = urldb_శోధన_కనుగొను(వృక్షం, host_str);
	ఒకవేళ (!h) {
		lwc_string_unref(scheme);
		తిరిగి ఖాళీ;
	}

	/* generate plq (path, leaf, query) */
	ఒకవేళ (nsurl_get(url, NSURL_మార్గం | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		తిరిగి ఖాళీ;
	}

	/* Get port */
	పోర్టు = nsurl_get_component(url, NSURL_పోర్టు);
	ఒకవేళ (పోర్టు != ఖాళీ) {
		పోర్టు_int = atoi(lwc_string_దత్తాంశం(పోర్టు));
		lwc_string_unref(పోర్టు);
	} లేకపోతే {
		పోర్టు_int = 0;
	}

	p = urldb_సరిపోలిక_మార్గం(&h->paths, plq, scheme, పోర్టు_int);

	విడుదల(plq);
	lwc_string_unref(scheme);

	తిరిగి p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
స్థిర శూన్య urldb_dump_paths(నిర్మాణం మార్గం_దత్తాంశం *మాతృ)
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ;
	గుర్తులేని పూర్ణాంకం i;

	చేయి {
		ఒకవేళ (p->segment != ఖాళీ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_దత్తాంశం(p->scheme), p->పోర్టు);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			కొరకు (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ఒకవేళ (p->children != ఖాళీ) {
			p = p->children;
		} లేకపోతే {
			అయితే (p != మాతృ) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					p = p->తదుపరి;
					ఆపు;
				}

				p = p->మాతృ;
			}
		}
	} అయితే (p != మాతృ);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
స్థిర శూన్య urldb_dump_hosts(నిర్మాణం host_part *మాతృ)
{
	నిర్మాణం host_part *h;

	ఒకవేళ (మాతృ->part) {
		NSLOG(netsurf, INFO, "%s", మాతృ->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      మాతృ->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&మాతృ->paths);

	/* and recurse */
	కొరకు (h = మాతృ->children; h; h = h->తదుపరి) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
స్థిర శూన్య urldb_dump_శోధన(నిర్మాణం శోధన_కణుపు *మాతృ, పూర్ణాంకం లోతు)
{
	స్థిరరాశి నిర్మాణం host_part *h;
	పూర్ణాంకం i; /* index into string */
	అక్షరం s[1024];
	పూర్ణాంకం r;
	పూర్ణాంకం sl = పరిమాణం(s) - 2;

	ఒకవేళ (మాతృ == &ఖాళీలు)
		తిరిగి;

	urldb_dump_శోధన(మాతృ->ఎడమ, లోతు + 1);

	కొరకు (i = 0; i != లోతు; i++) {
		s[i] = ' ';
	}

	కొరకు (h = మాతృ->దత్తాంశం; h; h = h->మాతృ) {
		ఒకవేళ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ఒకవేళ (r < 0) {
				ఆపు;
			}
			ఒకవేళ ((i + r) >= sl) {
				ఆపు;
			}
			i += r;
		}

		ఒకవేళ (h->మాతృ && h->మాతృ->మాతృ) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_శోధన(మాతృ->కుడి, లోతు + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
స్థిర పూర్ణాంకం
urldb_శోధన_సరిపోలిక_host(స్థిరరాశి నిర్మాణం host_part *a, స్థిరరాశి నిర్మాణం host_part *b)
{
	పూర్ణాంకం ret;

	ప్రతిజ్ఞా(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	కొరకు (; a && a != &db_మూలం && b && b != &db_మూలం;
	     a = a->మాతృ, b = b->మాతృ) {
		ఒకవేళ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			తిరిగి ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ఒకవేళ (a && a != &db_మూలం && (!b || b == &db_మూలం)) {
		/* len(a) > len(b) */
		తిరిగి 1;
	} లేకపోతే ఒకవేళ ((!a || a == &db_మూలం) && b && b != &db_మూలం) {
		/* len(a) < len(b) */
		తిరిగి -1;
	}

	/* identical */
	తిరిగి 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
స్థిర నిర్మాణం శోధన_కణుపు *urldb_శోధన_skew(నిర్మాణం శోధన_కణుపు *మూలం)
{
	ప్రతిజ్ఞా(మూలం);

	ఒకవేళ (మూలం->ఎడమ->level == మూలం->level) {
		నిర్మాణం శోధన_కణుపు *తాత్కాలిక;

		తాత్కాలిక = మూలం->ఎడమ;
		మూలం->ఎడమ = తాత్కాలిక->కుడి;
		తాత్కాలిక->కుడి = మూలం;
		మూలం = తాత్కాలిక;
	}

	తిరిగి మూలం;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
స్థిర నిర్మాణం శోధన_కణుపు *urldb_శోధన_విభజించు(నిర్మాణం శోధన_కణుపు *మూలం)
{
	ప్రతిజ్ఞా(మూలం);

	ఒకవేళ (మూలం->కుడి->కుడి->level == మూలం->level) {
		నిర్మాణం శోధన_కణుపు *తాత్కాలిక;

		తాత్కాలిక = మూలం->కుడి;
		మూలం->కుడి = తాత్కాలిక->ఎడమ;
		తాత్కాలిక->ఎడమ = మూలం;
		మూలం = తాత్కాలిక;

		మూలం->level++;
	}

	తిరిగి మూలం;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
స్థిర నిర్మాణం శోధన_కణుపు *
urldb_శోధన_చొప్పించు_అంతర్గత(నిర్మాణం శోధన_కణుపు *మూలం, నిర్మాణం శోధన_కణుపు *n)
{
	ప్రతిజ్ఞా(మూలం && n);

	ఒకవేళ (మూలం == &ఖాళీలు) {
		మూలం = n;
	} లేకపోతే {
		పూర్ణాంకం c = urldb_శోధన_సరిపోలిక_host(మూలం->దత్తాంశం, n->దత్తాంశం);

		ఒకవేళ (c > 0) {
			మూలం->ఎడమ = urldb_శోధన_చొప్పించు_అంతర్గత(
				మూలం->ఎడమ, n);
		} లేకపోతే ఒకవేళ (c < 0) {
			మూలం->కుడి = urldb_శోధన_చొప్పించు_అంతర్గత(
				మూలం->కుడి, n);
		} లేకపోతే {
			/* exact match */
			విడుదల(n);
			తిరిగి మూలం;
		}

		మూలం = urldb_శోధన_skew(మూలం);
		మూలం = urldb_శోధన_విభజించు(మూలం);
	}

	తిరిగి మూలం;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
స్థిర నిర్మాణం శోధన_కణుపు *
urldb_శోధన_చొప్పించు(నిర్మాణం శోధన_కణుపు *మూలం, స్థిరరాశి నిర్మాణం host_part *దత్తాంశం)
{
	నిర్మాణం శోధన_కణుపు *n;

	ప్రతిజ్ఞా(మూలం && దత్తాంశం);

	n = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం శోధన_కణుపు));
	ఒకవేళ (!n)
		తిరిగి ఖాళీ;

	n->level = 1;
	n->దత్తాంశం = దత్తాంశం;
	n->ఎడమ = n->కుడి = &ఖాళీలు;

	మూలం = urldb_శోధన_చొప్పించు_అంతర్గత(మూలం, n);

	తిరిగి మూలం;
}


/**
 * Parse a cookie avpair
 *
 * \param c Cookie struct to populate
 * \param n Name component
 * \param v Value component
 * \param was_quoted Whether \a v was quoted in the input
 * \return true on success, false on memory exhaustion
 */
స్థిర bool
urldb_parse_avpair(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c,
		   అక్షరం *n,
		   అక్షరం *v,
		   bool was_quoted)
{
	పూర్ణాంకం vlen;

	ప్రతిజ్ఞా(c && n && v);

	/* Strip whitespace from start of name */
	కొరకు (; *n; n++) {
		ఒకవేళ (*n != ' ' && *n != '\t')
			ఆపు;
	}

	/* Strip whitespace from end of name */
	కొరకు (vlen = అక్షరశ్రేణి_పొడవు(n); vlen; vlen--) {
		ఒకవేళ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		లేకపోతే
			ఆపు;
	}

	/* Strip whitespace from start of value */
	కొరకు (; *v; v++) {
		ఒకవేళ (*v != ' ' && *v != '\t')
			ఆపు;
	}

	/* Strip whitespace from end of value */
	కొరకు (vlen = అక్షరశ్రేణి_పొడవు(v); vlen; vlen--) {
		ఒకవేళ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		లేకపోతే
			ఆపు;
	}

	ఒకవేళ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ఒకవేళ (!c->comment)
			తిరిగి false;
	} లేకపోతే ఒకవేళ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ఒకవేళ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_సమితి = true;
			c->domain = strdup(v);
			ఒకవేళ (!c->domain)
				తిరిగి false;
		}
	} లేకపోతే ఒకవేళ (strcasecmp(n, "Max-Age") == 0) {
		పూర్ణాంకం తాత్కాలిక = atoi(v);
		ఒకవేళ (తాత్కాలిక == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		లేకపోతే
			c->expires = time(ఖాళీ) + తాత్కాలిక;
	} లేకపోతే ఒకవేళ (!c->మార్గం && strcasecmp(n, "Path") == 0) {
		c->మార్గం_from_సమితి = true;
		c->మార్గం = strdup(v);
		ఒకవేళ (!c->మార్గం)
			తిరిగి false;
	} లేకపోతే ఒకవేళ (strcasecmp(n, "Version") == 0) {
		c->సంస్కరణ = atoi(v);
	} లేకపోతే ఒకవేళ (strcasecmp(n, "Expires") == 0) {
		అక్షరం *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		కొరకు (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, అక్షరశ్రేణి_పొడవు(datenoday), &expires);
		ఒకవేళ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} లేకపోతే ఒకవేళ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} లేకపోతే ఒకవేళ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} లేకపోతే ఒకవేళ (!c->name) {
		c->name = strdup(n);
		c->విలువ = strdup(v);
		c->విలువ_was_quoted = was_quoted;
		ఒకవేళ (!c->name || !c->విలువ) {
			తిరిగి false;
		}
	}

	తిరిగి true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
స్థిర శూన్య urldb_free_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c)
{
	ప్రతిజ్ఞా(c);

	విడుదల(c->comment);
	విడుదల(c->domain);
	విడుదల(c->మార్గం);
	విడుదల(c->name);
	విడుదల(c->విలువ);
	విడుదల(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
స్థిర నిర్మాణం cookie_అంతర్గత_దత్తాంశం *
urldb_parse_cookie(nsurl *url, స్థిరరాశి అక్షరం **cookie)
{
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;
	స్థిరరాశి అక్షరం *cur;
	అక్షరం name[1024], విలువ[4096];
	అక్షరం *n = name, *v = విలువ;
	bool లోపల_విలువ = false;
	bool had_విలువ_దత్తాంశం = false;
	bool విలువ_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ప్రతిజ్ఞా(url && cookie && *cookie);

	c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం cookie_అంతర్గత_దత్తాంశం));
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	c->expires = -1;

	name[0] = '\0';
	విలువ[0] = '\0';

	కొరకు (cur = *cookie; *cur; cur++) {
		ఒకవేళ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ఒకవేళ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				విలువ[0] = '\0';

			}

			ఆపు;
		} లేకపోతే ఒకవేళ (*cur == '\r') {
			/* Spurious linefeed */
			కొనసాగు;
		} లేకపోతే ఒకవేళ (*cur == '\n') {
			/* Spurious newline */
			కొనసాగు;
		}

		ఒకవేళ (లోపల_విలువ && !had_విలువ_దత్తాంశం) {
			ఒకవేళ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				కొనసాగు;
			} లేకపోతే {
				had_విలువ_దత్తాంశం = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ఒకవేళ (*cur != '"') {
					విలువ_verbatim = true;
				}
			}
		}

		ఒకవేళ (లోపల_విలువ && !విలువ_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ఒకవేళ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				కొనసాగు;
			}
		}

		ఒకవేళ (!quoted && !లోపల_విలువ && *cur == '=') {
			/* First equals => attr-value separator */
			లోపల_విలువ = true;
			కొనసాగు;
		}

		ఒకవేళ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ఒకవేళ (!urldb_parse_avpair(c, name, విలువ, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				తిరిగి ఖాళీ;
			}

			/* And reset to start */
			n = name;
			v = విలువ;
			లోపల_విలువ = false;
			had_విలువ_దత్తాంశం = false;
			విలువ_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ఒకవేళ (*cur != ';') {
				cur--;
			}

			కొనసాగు;
		}

		/* And now handle commas. These are a pain as they may mean
		 * any of the following:
		 *
		 * + End of cookie
		 * + Day separator in Expires avpair
		 * + (Invalid) comma in unquoted value
		 *
		 * Therefore, in order to handle all 3 cases (2 and 3 are
		 * identical, the difference being that 2 is in the spec and
		 * 3 isn't), we need to determine where the comma actually
		 * lies. We use the following heuristic:
		 *
		 *   Given a comma at the current input position, find the
		 *   immediately following semicolon (or end of input if none
		 *   found). Then, consider the input characters between
		 *   these two positions. If any of these characters is an
		 *   '=', we must assume that the comma signified the end of
		 *   the current cookie.
		 *
		 * This holds as the first avpair of any cookie must be
		 * NAME=VALUE, so the '=' is guaranteed to appear in the
		 * case where the comma marks the end of a cookie.
		 *
		 * This will fail, however, in the case where '=' appears in
		 * the value of the current avpair after the comma or the
		 * subsequent cookie does not start with NAME=VALUE. Neither
		 * of these is particularly likely and if they do occur, the
		 * website is more broken than we can be bothered to handle.
		 */
		ఒకవేళ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			స్థిరరాశి అక్షరం *p;
			స్థిరరాశి అక్షరం *semi = strchr(cur + 1, ';');
			ఒకవేళ (!semi)
				semi = cur + అక్షరశ్రేణి_పొడవు(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			కొరకు (p = cur + 1; p < semi; p++)
				ఒకవేళ (*p == '=')
					ఆపు;

			ఒకవేళ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} లేకపోతే {
				/* found one => comma marks end of cookie */
				cur++;
				ఆపు;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ఒకవేళ (!లోపల_విలువ) {
			ఒకవేళ (n < name + (పరిమాణం(name) - 1))
				*n++ = *cur;
		} లేకపోతే {
			ఒకవేళ (v < విలువ + (పరిమాణం(విలువ) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ఒకవేళ (!urldb_parse_avpair(c, name, విలువ, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		తిరిగి ఖాళీ;
	}

	/* Now fix-up default values */
	ఒకవేళ (c->domain == ఖాళీ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ఒకవేళ (host == ఖాళీ) {
			urldb_free_cookie(c);
			తిరిగి ఖాళీ;
		}
		c->domain = strdup(lwc_string_దత్తాంశం(host));
		lwc_string_unref(host);
	}

	ఒకవేళ (c->మార్గం == ఖాళీ) {
		స్థిరరాశి అక్షరం *మార్గం_దత్తాంశం;
		అక్షరం *మార్గం, *slash;
		lwc_string *మార్గం_lwc;

		మార్గం_lwc = nsurl_get_component(url, NSURL_మార్గం);
		ఒకవేళ (మార్గం_lwc == ఖాళీ) {
			urldb_free_cookie(c);
			తిరిగి ఖాళీ;
		}
		మార్గం_దత్తాంశం = lwc_string_దత్తాంశం(మార్గం_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(మార్గం_దత్తాంశం, '/');
		ఒకవేళ (slash != ఖాళీ) {
			/* Special case: retain first slash in path */
			ఒకవేళ (slash == మార్గం_దత్తాంశం)
				slash++;

			slash = strndup(మార్గం_దత్తాంశం, slash - మార్గం_దత్తాంశం);
			ఒకవేళ (slash == ఖాళీ) {
				lwc_string_unref(మార్గం_lwc);
				urldb_free_cookie(c);
				తిరిగి ఖాళీ;
			}

			మార్గం = slash;
			lwc_string_unref(మార్గం_lwc);
		} లేకపోతే {
			మార్గం = strdup(lwc_string_దత్తాంశం(మార్గం_lwc));
			lwc_string_unref(మార్గం_lwc);
			ఒకవేళ (మార్గం == ఖాళీ) {
				urldb_free_cookie(c);
				తిరిగి ఖాళీ;
			}
		}

		c->మార్గం = మార్గం;
	}

	/* Write back current position */
	*cookie = cur;

	తిరిగి c;
}


/**
 * Add a path to the database, creating any intermediate entries
 *
 * \param scheme URL scheme associated with path
 * \param port Port number on host associated with path
 * \param host Host tree node to attach to
 * \param path_query Absolute path plus query to add (freed)
 * \param fragment URL fragment, or NULL
 * \param url URL (fragment ignored)
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
స్థిర నిర్మాణం మార్గం_దత్తాంశం *
urldb_కూడు_మార్గం(lwc_string *scheme,
	       గుర్తులేని పూర్ణాంకం పోర్టు,
	       స్థిరరాశి నిర్మాణం host_part *host,
	       అక్షరం *మార్గం_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *d, *e;
	అక్షరం *buf = మార్గం_query;
	అక్షరం *segment, *slash;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(scheme && host && url);

	d = (నిర్మాణం మార్గం_దత్తాంశం *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ఒకవేళ (*segment == '/')
		segment++;

	/* Process path segments */
	చేయి {
		slash = strchr(segment, '/');
		ఒకవేళ (!slash) {
			/* last segment */
			/* look for existing entry */
			కొరకు (e = d->children; e; e = e->తదుపరి)
				ఒకవేళ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &సరిపోలిక) ==
				    lwc_error_ok &&
				    సరిపోలిక == true &&
				    e->పోర్టు == పోర్టు)
					ఆపు;

			d = e ? urldb_కూడు_మార్గం_fragment(e, fragment) :
				urldb_కూడు_మార్గం_కణుపు(scheme, పోర్టు,
						    segment, fragment, d);
			ఆపు;
		}

		*slash = '\0';

		/* look for existing entry */
		కొరకు (e = d->children; e; e = e->తదుపరి)
			ఒకవేళ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &సరిపోలిక) == lwc_error_ok &&
			    సరిపోలిక == true &&
			    e->పోర్టు == పోర్టు)
				ఆపు;

		d = e ? e : urldb_కూడు_మార్గం_కణుపు(scheme, పోర్టు, segment, ఖాళీ, d);
		ఒకవేళ (!d)
			ఆపు;

		segment = slash + 1;
	} అయితే (1);

	విడుదల(మార్గం_query);

	ఒకవేళ (d && !d->url) {
		/* Insert defragmented URL */
		ఒకవేళ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			తిరిగి ఖాళీ;
	}

	తిరిగి d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
స్థిర నిర్మాణం host_part *urldb_కూడు_host(స్థిరరాశి అక్షరం *host)
{
	నిర్మాణం host_part *d = (నిర్మాణం host_part *) &db_మూలం, *e;
	నిర్మాణం శోధన_కణుపు *s;
	అక్షరం buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	అక్షరం *part;

	ప్రతిజ్ఞా(host);

	ఒకవేళ (urldb_host_is_ip_చిరునామా(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		కొరకు (e = d->children; e; e = e->తదుపరి)
			ఒకవేళ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				తిరిగి e;

		d = urldb_కూడు_host_కణుపు(host, d);

		s = urldb_శోధన_చొప్పించు(శోధన_trees[ST_IP], d);
		ఒకవేళ (!s) {
			/* failed */
			d = ఖాళీ;
		} లేకపోతే {
			శోధన_trees[ST_IP] = s;
		}

		తిరిగి d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, పరిమాణం buf);
	buf[పరిమాణం buf - 1] = '\0';

	/* Process FQDN segments backwards */
	చేయి {
		part = strrchr(buf, '.');
		ఒకవేళ (!part) {
			/* last segment */
			/* Check for existing entry */
			కొరకు (e = d->children; e; e = e->తదుపరి)
				ఒకవేళ (strcasecmp(buf, e->part) == 0)
					ఆపు;

			ఒకవేళ (e) {
				d = e;
			} లేకపోతే {
				d = urldb_కూడు_host_కణుపు(buf, d);
			}

			/* And insert into search tree */
			ఒకవేళ (d) {
				నిర్మాణం శోధన_కణుపు **r;

				r = urldb_get_శోధన_వృక్షం_ప్రత్యక్ష(buf);
				s = urldb_శోధన_చొప్పించు(*r, d);
				ఒకవేళ (!s) {
					/* failed */
					d = ఖాళీ;
				} లేకపోతే {
					*r = s;
				}
			}
			ఆపు;
		}

		/* Check for existing entry */
		కొరకు (e = d->children; e; e = e->తదుపరి)
			ఒకవేళ (strcasecmp(part + 1, e->part) == 0)
				ఆపు;

		d = e ? e : urldb_కూడు_host_కణుపు(part + 1, d);
		ఒకవేళ (!d)
			ఆపు;

		*part = '\0';
	} అయితే (1);

	తిరిగి d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
స్థిర bool
urldb_చొప్పించు_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *d;
	స్థిరరాశి నిర్మాణం host_part *h;
	నిర్మాణం మార్గం_దత్తాంశం *p;
	time_t now = time(ఖాళీ);

	ప్రతిజ్ఞా(c);

	ఒకవేళ (c->domain[0] == '.') {
		h = urldb_శోధన_కనుగొను(
			urldb_get_శోధన_వృక్షం(&(c->domain[1])),
			c->domain + 1);
		ఒకవేళ (!h) {
			h = urldb_కూడు_host(c->domain + 1);
			ఒకవేళ (!h) {
				urldb_free_cookie(c);
				తిరిగి false;
			}
		}

		p = (నిర్మాణం మార్గం_దత్తాంశం *) &h->paths;
	} లేకపోతే {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ప్రతిజ్ఞా(url != ఖాళీ);
		ప్రతిజ్ఞా(scheme != ఖాళీ);

		h = urldb_శోధన_కనుగొను(
			urldb_get_శోధన_వృక్షం(c->domain),
			c->domain);

		ఒకవేళ (!h) {
			h = urldb_కూడు_host(c->domain);
			ఒకవేళ (!h) {
				urldb_free_cookie(c);
				తిరిగి false;
			}
		}

		/* find path */
		p = urldb_కూడు_మార్గం(scheme, 0, h,
				   strdup(c->మార్గం), ఖాళీ, url);
		ఒకవేళ (!p) {
			urldb_free_cookie(c);
			తిరిగి false;
		}
	}

	/* add cookie */
	కొరకు (d = p->cookies; d; d = d->తదుపరి) {
		ఒకవేళ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->మార్గం, c->మార్గం) &&
		    !strcmp(d->name, c->name))
			ఆపు;
	}

	ఒకవేళ (d) {
		ఒకవేళ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ఒకవేళ (d->తదుపరి)
				d->తదుపరి->prev = d->prev;
			లేకపోతే
				p->cookies_చివర = d->prev;
			ఒకవేళ (d->prev)
				d->prev->తదుపరి = d->తదుపరి;
			లేకపోతే
				p->cookies = d->తదుపరి;

			cookie_manager_remove((నిర్మాణం cookie_దత్తాంశం *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} లేకపోతే {
			/* replace d with c */
			c->prev = d->prev;
			c->తదుపరి = d->తదుపరి;
			ఒకవేళ (c->తదుపరి)
				c->తదుపరి->prev = c;
			లేకపోతే
				p->cookies_చివర = c;
			ఒకవేళ (c->prev)
				c->prev->తదుపరి = c;
			లేకపోతే
				p->cookies = c;

			cookie_manager_remove((నిర్మాణం cookie_దత్తాంశం *)d);
			urldb_free_cookie(d);

			cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
		}
	} లేకపోతే {
		c->prev = p->cookies_చివర;
		c->తదుపరి = ఖాళీ;
		ఒకవేళ (p->cookies_చివర)
			p->cookies_చివర->తదుపరి = c;
		లేకపోతే
			p->cookies = c;
		p->cookies_చివర = c;

		cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
	}

	తిరిగి true;
}

/* Combined (A)BNF for the Cookie: request header:
 *
 * CHAR           = <any US-ASCII character (octets 0 - 127)>
 * CTL            = <any US-ASCII control character
 *                  (octets 0 - 31) and DEL (127)>
 * CR             = <US-ASCII CR, carriage return (13)>
 * LF             = <US-ASCII LF, linefeed (10)>
 * SP             = <US-ASCII SP, space (32)>
 * HT             = <US-ASCII HT, horizontal-tab (9)>
 * <">            = <US-ASCII double-quote mark (34)>
 *
 * CRLF           = CR LF
 *
 * LWS            = [CRLF] 1*( SP | HT )
 *
 * TEXT           = <any OCTET except CTLs,
 *                  but including LWS>
 *
 * token          = 1*<any CHAR except CTLs or separators>
 * separators     = "(" | ")" | "<" | ">" | "@"
 *                | "," | ";" | ":" | "\" | <">
 *                | "/" | "[" | "]" | "?" | "="
 *                | "{" | "}" | SP | HT
 *
 * quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
 * qdtext         = <any TEXT except <">>
 * quoted-pair    = "\" CHAR
 *
 * attr            =       token
 * value           =       word
 * word            =       token | quoted-string
 *
 * cookie          =       "Cookie:" cookie-version
 *                         1*((";" | ",") cookie-value)
 * cookie-value    =       NAME "=" VALUE [";" path] [";" domain]
 * cookie-version  =       "$Version" "=" value
 * NAME            =       attr
 * VALUE           =       value
 * path            =       "$Path" "=" value
 * domain          =       "$Domain" "=" value
 *
 */

/**
 * Concatenate a cookie into the provided buffer
 *
 * \param c Cookie to concatenate
 * \param used Pointer to amount of buffer used (updated)
 * \param alloc Pointer to allocated size of buffer (updated)
 * \param buf Pointer to Pointer to buffer (updated)
 * \return true on success, false on memory exhaustion
 */
స్థిర bool
concat_netscape_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c,
		    పూర్ణాంకం *వాడిన,
		    పూర్ణాంకం *alloc,
		    అక్షరం **buf)
{
	/*
	 * A note on quoted-string handling:
	 *   The cookie data stored in the db is verbatim (i.e. sans enclosing
	 *   <">, if any, and with all quoted-pairs intact) thus all that we
	 *   need to do here is ensure that value strings which were quoted
	 *   in Set-Cookie or which include any of the separators are quoted
	 *   before use.
	 *
	 * A note on cookie-value separation:
	 *   We use semicolons for all separators, including between
	 *   cookie-values. This simplifies things and is backwards compatible.
	 */
	పూర్ణాంకం max_len;
	పూర్ణాంకం outl; /* output length */

	ప్రతిజ్ఞా(c && వాడిన && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *వాడిన + 2 +
		అక్షరశ్రేణి_పొడవు(c->name) + 1 +
		అక్షరశ్రేణి_పొడవు(c->విలువ) + 2 +
		(c->మార్గం_from_సమితి ? 8 + అక్షరశ్రేణి_పొడవు(c->మార్గం) + 2 : 0) +
		(c->domain_from_సమితి ? 10 + అక్షరశ్రేణి_పొడవు(c->domain) + 2 : 0);
	ఒకవేళ (max_len >= *alloc) {
		అక్షరం *తాత్కాలిక = realloc(*buf, max_len);
		ఒకవేళ (!తాత్కాలిక) {
			తిరిగి false;
		}
		*buf = తాత్కాలిక;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *వాడిన - 1, *alloc - *వాడిన, "; %s=", c->name);
	ఒకవేళ (outl >= (*alloc - *వాడిన)) {
		/* output truncated */
		తిరిగి false;
	}
	*వాడిన += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ఒకవేళ (c->విలువ_was_quoted) {
		outl = snprintf(*buf + *వాడిన - 1,
				*alloc - *వాడిన,
				"\"%s\"", c->విలువ);
		ఒకవేళ (outl >= (*alloc - *వాడిన)) {
			/* output truncated */
			తిరిగి false;
		}
		*వాడిన += outl;
	} లేకపోతే {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *వాడిన - 1,*alloc - *వాడిన, "%s", c->విలువ);
		ఒకవేళ (outl >= (*alloc - *వాడిన)) {
			/* output truncated */
			తిరిగి false;
		}
		*వాడిన += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	తిరిగి true;
}


/**
 * Concatenate a cookie into the provided buffer
 *
 * \param c Cookie to concatenate
 * \param used Pointer to amount of buffer used (updated)
 * \param alloc Pointer to allocated size of buffer (updated)
 * \param buf Pointer to Pointer to buffer (updated)
 * \return true on success, false on memory exhaustion
 */
స్థిర bool
concat_rfc2965_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c,
		      పూర్ణాంకం *వాడిన,
		      పూర్ణాంకం *alloc,
		      అక్షరం **buf)
{
	/*
	 * A note on quoted-string handling:
	 *   The cookie data stored in the db is verbatim (i.e. sans enclosing
	 *   <">, if any, and with all quoted-pairs intact) thus all that we
	 *   need to do here is ensure that value strings which were quoted
	 *   in Set-Cookie or which include any of the separators are quoted
	 *   before use.
	 *
	 * A note on cookie-value separation:
	 *   We use semicolons for all separators, including between
	 *   cookie-values. This simplifies things and is backwards compatible.
	 */
	స్థిరరాశి అక్షరం * స్థిరరాశి separators = "()<>@,;:\\\"/[]?={} \t";
	పూర్ణాంకం max_len;
	పూర్ణాంకం outl; /* output length */

	ప్రతిజ్ఞా(c && వాడిన && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *వాడిన + 2 +
		అక్షరశ్రేణి_పొడవు(c->name) + 1 +
		అక్షరశ్రేణి_పొడవు(c->విలువ) + 2 +
		(c->మార్గం_from_సమితి ?
		 8 + అక్షరశ్రేణి_పొడవు(c->మార్గం) + 2 : 0) +
		(c->domain_from_సమితి ?
		 10 + అక్షరశ్రేణి_పొడవు(c->domain) + 2 : 0);

	ఒకవేళ (max_len >= *alloc) {
		అక్షరం *తాత్కాలిక = realloc(*buf, max_len);
		ఒకవేళ (!తాత్కాలిక) {
			తిరిగి false;
		}
		*buf = తాత్కాలిక;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *వాడిన - 1,
			*alloc - *వాడిన,
			"; %s=", c->name);
	ఒకవేళ (outl >= (*alloc - *వాడిన)) {
		/* output truncated */
		తిరిగి false;
	}
	*వాడిన += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ఒకవేళ (c->విలువ_was_quoted ||
	    strpbrk(c->విలువ, separators) != ఖాళీ) {
		outl = snprintf(*buf + *వాడిన - 1,
				*alloc - *వాడిన,
				"\"%s\"", c->విలువ);

	} లేకపోతే {
		outl = snprintf(*buf + *వాడిన - 1,
				*alloc - *వాడిన,
				"%s", c->విలువ);
	}
	ఒకవేళ (outl >= (*alloc - *వాడిన)) {
		/* output truncated */
		తిరిగి false;
	}
	*వాడిన += outl;

	ఒకవేళ (c->మార్గం_from_సమితి) {
		/* Path, quoted if necessary */

		ఒకవేళ (strpbrk(c->మార్గం, separators) != ఖాళీ) {
			outl = snprintf(*buf + *వాడిన - 1,
					*alloc - *వాడిన,
					"; $Path=\"%s\"", c->మార్గం);

		} లేకపోతే {
			outl = snprintf(*buf + *వాడిన - 1,
				       *alloc - *వాడిన,
				       "; $Path=%s", c->మార్గం);
		}
		ఒకవేళ (outl >= (*alloc - *వాడిన)) {
			/* output truncated */
			తిరిగి false;
		}
		*వాడిన += outl;
	}

	ఒకవేళ (c->domain_from_సమితి) {
		/* Domain, quoted if necessary */
		ఒకవేళ (strpbrk(c->domain, separators) != ఖాళీ) {
			outl = snprintf(*buf + *వాడిన - 1,
					*alloc - *వాడిన,
					"; $Domain=\"%s\"", c->domain);
		} లేకపోతే {
			outl = snprintf(*buf + *వాడిన - 1,
					*alloc - *వాడిన,
					"; $Domain=%s", c->domain);
		}
		ఒకవేళ (outl >= (*alloc - *వాడిన)) {
			/* output truncated */
			తిరిగి false;
		}
		*వాడిన += outl;
	}

	తిరిగి true;
}


/**
 * build output cookie string in netscape format
 */
స్థిర అక్షరం *
get_netscape_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం **matched_cookies, పూర్ణాంకం లెక్క)
{
	పూర్ణాంకం ret_alloc = 4096, ret_వాడిన = 1;
	అక్షరం *ret;
	పూర్ణాంకం midx; /* index into matched cookies */

	ret = జ్ఞాపకం_ఇవ్వు(ret_alloc);
	ఒకవేళ (ret == ఖాళీ) {
		తిరిగి ఖాళీ;
	}
	ret[0] = '\0';

	కొరకు (midx = 0; midx < లెక్క; midx++) {
		ఒకవేళ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_వాడిన, &ret_alloc, &ret)) {
			విడుదల(ret);
			తిరిగి ఖాళీ;
		}
	}

	ఒకవేళ (ret_వాడిన <= 2) {
		విడుదల(ret);
		తిరిగి ఖాళీ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_వాడిన - 2);
	ret_వాడిన -= 2;

	/* shrink the output buffer to the required size */
	అక్షరం *తాత్కాలిక = realloc(ret, ret_వాడిన);
	ఒకవేళ (తాత్కాలిక != ఖాళీ) {
		ret = తాత్కాలిక;
	}

	తిరిగి ret;
}


/**
 * build output cookie string in RFC2965 format
 */
స్థిర అక్షరం *
get_rfc2965_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం **matched_cookies,
		   పూర్ణాంకం లెక్క,
		   పూర్ణాంకం సంస్కరణ)
{
	పూర్ణాంకం ret_alloc = 4096; /* return string allocation */
	పూర్ణాంకం ret_వాడిన = 1; /* return sting buffer used */
	అక్షరం *ret; /* return string buffer */
	పూర్ణాంకం midx; /* index into matched cookies */
	పూర్ణాంకం outl;

	ret = జ్ఞాపకం_ఇవ్వు(ret_alloc);
	ఒకవేళ (ret == ఖాళీ) {
		తిరిగి ఖాళీ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_వాడిన, "$Version=%d", సంస్కరణ);
	ఒకవేళ (outl >= ret_alloc - ret_వాడిన) {
		/*అధికప్రవాహం*/
		విడుదల(ret);
		తిరిగి ఖాళీ;
	}
	ret_వాడిన += outl;

	కొరకు (midx = 0; midx < లెక్క; midx++) {
		ఒకవేళ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_వాడిన, &ret_alloc, &ret)) {
			విడుదల(ret);
			తిరిగి ఖాళీ;
		}
	}

	/* shrink the output buffer to the required size */
	అక్షరం *తాత్కాలిక = realloc(ret, ret_వాడిన);
	ఒకవేళ (తాత్కాలిక != ఖాళీ) {
		ret = తాత్కాలిక;
	}

	తిరిగి ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
స్థిర శూన్య
urldb_delete_cookie_paths(స్థిరరాశి అక్షరం *domain,
			  స్థిరరాశి అక్షరం *మార్గం,
			  స్థిరరాశి అక్షరం *name,
			  నిర్మాణం మార్గం_దత్తాంశం *మాతృ)
{
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;
	నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ;

	ప్రతిజ్ఞా(మాతృ);

	చేయి {
		కొరకు (c = p->cookies; c; c = c->తదుపరి) {
			ఒకవేళ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->మార్గం, మార్గం) == 0 &&
			    strcmp(c->name, name) == 0) {
				ఒకవేళ (c->prev) {
					c->prev->తదుపరి = c->తదుపరి;
				} లేకపోతే {
					p->cookies = c->తదుపరి;
				}

				ఒకవేళ (c->తదుపరి) {
					c->తదుపరి->prev = c->prev;
				} లేకపోతే {
					p->cookies_చివర = c->prev;
				}

				urldb_free_cookie(c);

				తిరిగి;
			}
		}

		ఒకవేళ (p->children) {
			p = p->children;
		} లేకపోతే {
			అయితే (p != మాతృ) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					p = p->తదుపరి;
					ఆపు;
				}

				p = p->మాతృ;
			}
		}
	} అయితే (p != మాతృ);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
స్థిర శూన్య
urldb_delete_cookie_hosts(స్థిరరాశి అక్షరం *domain,
			  స్థిరరాశి అక్షరం *మార్గం,
			  స్థిరరాశి అక్షరం *name,
			  నిర్మాణం host_part *మాతృ)
{
	నిర్మాణం host_part *h;
	ప్రతిజ్ఞా(మాతృ);

	urldb_delete_cookie_paths(domain, మార్గం, name, &మాతృ->paths);

	కొరకు (h = మాతృ->children; h; h = h->తదుపరి) {
		urldb_delete_cookie_hosts(domain, మార్గం, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
స్థిర శూన్య urldb_save_cookie_paths(FILE *fp, నిర్మాణం మార్గం_దత్తాంశం *మాతృ)
{
	నిర్మాణం మార్గం_దత్తాంశం *p = మాతృ;
	time_t now = time(ఖాళీ);

	ప్రతిజ్ఞా(fp && మాతృ);

	చేయి {
		ఒకవేళ (p->cookies != ఖాళీ) {
			నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;

			కొరకు (c = p->cookies; c != ఖాళీ; c = c->తదుపరి) {
				ఒకవేళ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					కొనసాగు;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->సంస్కరణ, c->domain,
					c->domain_from_సమితి, c->మార్గం,
					c->మార్గం_from_సమితి, c->secure,
					c->http_only,
					(పూర్ణాంకం)c->expires, (పూర్ణాంకం)c->చివరి_వాడిన,
					c->no_destroy, c->name, c->విలువ,
					c->విలువ_was_quoted,
					p->scheme ? lwc_string_దత్తాంశం(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ఒకవేళ (p->children != ఖాళీ) {
			p = p->children;
		} లేకపోతే {
			అయితే (p != మాతృ) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					p = p->తదుపరి;
					ఆపు;
				}

				p = p->మాతృ;
			}
		}
	} అయితే (p != మాతృ);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
స్థిర శూన్య urldb_save_cookie_hosts(FILE *fp, నిర్మాణం host_part *మాతృ)
{
	నిర్మాణం host_part *h;
	ప్రతిజ్ఞా(fp && మాతృ);

	urldb_save_cookie_paths(fp, &మాతృ->paths);

	కొరకు (h = మాతృ->children; h; h = h->తదుపరి)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
స్థిర శూన్య urldb_destroy_cookie(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c)
{
	విడుదల(c->name);
	విడుదల(c->విలువ);
	విడుదల(c->comment);
	విడుదల(c->domain);
	విడుదల(c->మార్గం);

	విడుదల(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
స్థిర శూన్య urldb_destroy_మార్గం_కణుపు_content(నిర్మాణం మార్గం_దత్తాంశం *కణుపు)
{
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *a, *b;
	గుర్తులేని పూర్ణాంకం i;

	ఒకవేళ (కణుపు->url != ఖాళీ) {
		nsurl_unref(కణుపు->url);
	}

	lwc_string_unref(కణుపు->scheme);

	విడుదల(కణుపు->segment);
	కొరకు (i = 0; i < కణుపు->frag_cnt; i++)
		విడుదల(కణుపు->fragment[i]);
	విడుదల(కణుపు->fragment);

	విడుదల(కణుపు->urld.title);

	కొరకు (a = కణుపు->cookies; a; a = b) {
		b = a->తదుపరి;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
స్థిర శూన్య urldb_destroy_prot_space(నిర్మాణం prot_space_దత్తాంశం *space)
{
	lwc_string_unref(space->scheme);
	విడుదల(space->realm);
	విడుదల(space->auth);

	విడుదల(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
స్థిర శూన్య urldb_destroy_మార్గం_వృక్షం(నిర్మాణం మార్గం_దత్తాంశం *మూలం)
{
	నిర్మాణం మార్గం_దత్తాంశం *p = మూలం;

	చేయి {
		ఒకవేళ (p->children != ఖాళీ) {
			p = p->children;
		} లేకపోతే {
			నిర్మాణం మార్గం_దత్తాంశం *q = p;

			అయితే (p != మూలం) {
				ఒకవేళ (p->తదుపరి != ఖాళీ) {
					p = p->తదుపరి;
					ఆపు;
				}

				p = p->మాతృ;

				urldb_destroy_మార్గం_కణుపు_content(q);
				విడుదల(q);

				q = p;
			}

			urldb_destroy_మార్గం_కణుపు_content(q);
			విడుదల(q);
		}
	} అయితే (p != మూలం);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
స్థిర శూన్య urldb_destroy_host_వృక్షం(నిర్మాణం host_part *మూలం)
{
	నిర్మాణం host_part *a, *b;
	నిర్మాణం మార్గం_దత్తాంశం *p, *q;
	నిర్మాణం prot_space_దత్తాంశం *s, *t;

	/* Destroy children */
	కొరకు (a = మూలం->children; a; a = b) {
		b = a->తదుపరి;
		urldb_destroy_host_వృక్షం(a);
	}

	/* Now clean up paths */
	కొరకు (p = మూలం->paths.children; p; p = q) {
		q = p->తదుపరి;
		urldb_destroy_మార్గం_వృక్షం(p);
	}

	/* Root path */
	urldb_destroy_మార్గం_కణుపు_content(&మూలం->paths);

	/* Proctection space data */
	కొరకు (s = మూలం->prot_space; s; s = t) {
		t = s->తదుపరి;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	విడుదల(మూలం->part);
	విడుదల(మూలం);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
స్థిర శూన్య urldb_destroy_శోధన_వృక్షం(నిర్మాణం శోధన_కణుపు *మూలం)
{
	/* Destroy children */
	ఒకవేళ (మూలం->ఎడమ != &ఖాళీలు)
		urldb_destroy_శోధన_వృక్షం(మూలం->ఎడమ);
	ఒకవేళ (మూలం->కుడి != &ఖాళీలు)
		urldb_destroy_శోధన_వృక్షం(మూలం->కుడి);

	/* And destroy ourselves */
	విడుదల(మూలం);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
శూన్య urldb_destroy(శూన్య)
{
	నిర్మాణం host_part *a, *b;
	పూర్ణాంకం i;

	/* Clean up search trees */
	కొరకు (i = 0; i < అంకె_శోధన_TREES; i++) {
		ఒకవేళ (శోధన_trees[i] != &ఖాళీలు) {
			urldb_destroy_శోధన_వృక్షం(శోధన_trees[i]);
			శోధన_trees[i] = &ఖాళీలు;
		}
	}

	/* And database */
	కొరకు (a = db_మూలం.children; a; a = b) {
		b = a->తదుపరి;
		urldb_destroy_host_వృక్షం(a);
	}
	memset(&db_మూలం, 0, పరిమాణం(db_మూలం));

	/* And the bloom filter */
	ఒకవేళ (url_bloom != ఖాళీ) {
		bloom_destroy(url_bloom);
		url_bloom = ఖాళీ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_ఎక్కించు(స్థిరరాశి అక్షరం *filename)
{
#నిర్వచించు గరిష్ఠ_URL_నిడివి 4096
	అక్షరం s[గరిష్ఠ_URL_నిడివి];
	అక్షరం host[256];
	నిర్మాణం host_part *h;
	పూర్ణాంకం urls;
	పూర్ణాంకం i;
	పూర్ణాంకం సంస్కరణ;
	పూర్ణాంకం నిడివి;
	FILE *fp;

	ప్రతిజ్ఞా(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ఒకవేళ (url_bloom == ఖాళీ)
		url_bloom = bloom_create(BLOOM_కొలత);

	fp = fopen(filename, "r");
	ఒకవేళ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp)) {
		fclose(fp);
		తిరిగి NSERROR_NEED_దత్తాంశం;
	}

	సంస్కరణ = atoi(s);
	ఒకవేళ (సంస్కరణ < MIN_URL_FILE_సంస్కరణ) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		తిరిగి NSERROR_INVALID;
	}
	ఒకవేళ (సంస్కరణ > URL_FILE_సంస్కరణ) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		తిరిగి NSERROR_INVALID;
	}

	అయితే (fgets(host, పరిమాణం host, fp)) {
		time_t hsts_expiry = 0;
		పూర్ణాంకం hsts_include_sub_domains = 0;

		/* get the hostname */
		నిడివి = అక్షరశ్రేణి_పొడవు(host) - 1;
		host[నిడివి] = '\0';

		/* skip data that has ended up with a host of '' */
		ఒకవేళ (నిడివి == 0) {
			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;
			urls = atoi(s);
			/* Eight fields/url */
			కొరకు (i = 0; i < (8 * urls); i++) {
				ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
					ఆపు;
			}
			కొనసాగు;
		}

		ఒకవేళ (సంస్కరణ >= 107) {
			అక్షరం *p = host;
			అయితే (*p && *p != ' ') p++;
			అయితే (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			అయితే (*p && *p != ' ') p++;
			అయితే (*p && *p == ' ') p++;
			nsc_snptimet(p, అక్షరశ్రేణి_పొడవు(p), &hsts_expiry);
		}

		h = urldb_కూడు_host(host);
		ఒకవేళ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			తిరిగి NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
			ఆపు;
		urls = atoi(s);

		/* no URLs => try next host */
		ఒకవేళ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			కొనసాగు;
		}

		/* load the non-corrupt data */
		కొరకు (i = 0; i < urls; i++) {
			నిర్మాణం మార్గం_దత్తాంశం *p = ఖాళీ;
			అక్షరం scheme[64], ports[10];
			అక్షరం url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			గుర్తులేని పూర్ణాంకం పోర్టు;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			అక్షరం *మార్గం_query;
			size_t len;

			ఒకవేళ (!fgets(scheme, పరిమాణం scheme, fp))
				ఆపు;
			నిడివి = అక్షరశ్రేణి_పొడవు(scheme) - 1;
			scheme[నిడివి] = '\0';

			ఒకవేళ (!fgets(ports, పరిమాణం ports, fp))
				ఆపు;
			నిడివి = అక్షరశ్రేణి_పొడవు(ports) - 1;
			ports[నిడివి] = '\0';
			పోర్టు = atoi(ports);

			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;
			నిడివి = అక్షరశ్రేణి_పొడవు(s) - 1;
			s[నిడివి] = '\0';

			ఒకవేళ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, పరిమాణం url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (పోర్టు ? ":" : ""),
				 (పోర్టు ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			ఒకవేళ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				తిరిగి NSERROR_NOMEM;
			}

			ఒకవేళ (url_bloom != ఖాళీ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_చొప్పించు_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ఒకవేళ (nsurl_get(nsurl, NSURL_మార్గం | NSURL_QUERY,
				      &మార్గం_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				తిరిగి NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_కూడు_మార్గం(scheme_lwc, పోర్టు, h, మార్గం_query,
					   fragment_lwc, nsurl);
			ఒకవేళ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				తిరిగి NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;
			ఒకవేళ (p)
				p->urld.visits = (గుర్తులేని పూర్ణాంకం)atoi(s);

			/* entry last use time */
			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp)) {
				ఆపు;
			}
			ఒకవేళ (p) {
				nsc_snptimet(s, అక్షరశ్రేణి_పొడవు(s) - 1, &p->urld.చివరి_సందర్శించు);
			}

			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;
			ఒకవేళ (p)
				p->urld.type = (content_type)atoi(s);

			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;


			ఒకవేళ (!fgets(s, గరిష్ఠ_URL_నిడివి, fp))
				ఆపు;
			నిడివి = అక్షరశ్రేణి_పొడవు(s) - 1;
			ఒకవేళ (p && నిడివి > 0) {
				s[నిడివి] = '\0';
				p->urld.title = జ్ఞాపకం_ఇవ్వు(నిడివి + 1);
				ఒకవేళ (p->urld.title)
					జ్ఞాపకం_నకలు(p->urld.title, s, నిడివి + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#నిర్వచనం_రద్దు గరిష్ఠ_URL_నిడివి

	తిరిగి NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(స్థిరరాశి అక్షరం *filename)
{
	FILE *fp;
	పూర్ణాంకం i;

	ప్రతిజ్ఞా(filename);

	fp = fopen(filename, "w");
	ఒకవేళ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_సంస్కరణ);

	కొరకు (i = 0; i != అంకె_శోధన_TREES; i++) {
		urldb_save_శోధన_వృక్షం(శోధన_trees[i], fp);
	}

	fclose(fp);

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_సమితి_url_persistence(nsurl *url, bool persist)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	p->persistent = persist;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_కూడు_url(nsurl *url)
{
	నిర్మాణం host_part *h;
	నిర్మాణం మార్గం_దత్తాంశం *p;
	lwc_string *scheme;
	lwc_string *పోర్టు;
	lwc_string *host;
	lwc_string *fragment;
	స్థిరరాశి అక్షరం *host_str;
	అక్షరం *మార్గం_query = ఖాళీ;
	size_t len;
	bool సరిపోలిక;
	గుర్తులేని పూర్ణాంకం పోర్టు_int;

	ప్రతిజ్ఞా(url);

	ఒకవేళ (url_bloom == ఖాళీ)
		url_bloom = bloom_create(BLOOM_కొలత);

	ఒకవేళ (url_bloom != ఖాళీ) {
		uint32_t hash = nsurl_hash(url);
		bloom_చొప్పించు_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ఒకవేళ (nsurl_get(url, NSURL_మార్గం | NSURL_QUERY, &మార్గం_query, &len) !=
	    NSERROR_OK) {
		తిరిగి false;
	}
	ప్రతిజ్ఞా(మార్గం_query != ఖాళీ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (scheme == ఖాళీ) {
		విడుదల(మార్గం_query);
		తిరిగి false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (host != ఖాళీ) {
		host_str = lwc_string_దత్తాంశం(host);
		lwc_string_unref(host);

	} లేకపోతే ఒకవేళ (lwc_string_isequal(scheme, corestring_lwc_file, &సరిపోలిక) ==
		   lwc_error_ok && సరిపోలిక == true) {
		host_str = "localhost";

	} లేకపోతే {
		lwc_string_unref(scheme);
		విడుదల(మార్గం_query);
		తిరిగి false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	పోర్టు = nsurl_get_component(url, NSURL_పోర్టు);
	ఒకవేళ (పోర్టు != ఖాళీ) {
		పోర్టు_int = atoi(lwc_string_దత్తాంశం(పోర్టు));
		lwc_string_unref(పోర్టు);
	} లేకపోతే {
		పోర్టు_int = 0;
	}

	/* Get host entry */
	h = urldb_కూడు_host(host_str);

	/* Get path entry */
	ఒకవేళ (h != ఖాళీ) {
		p = urldb_కూడు_మార్గం(scheme,
				   పోర్టు_int,
				   h,
				   మార్గం_query,
				   fragment,
				   url);
	} లేకపోతే {
		p = ఖాళీ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	తిరిగి (p != ఖాళీ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_సమితి_url_title(nsurl *url, స్థిరరాశి అక్షరం *title)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	అక్షరం *తాత్కాలిక;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (p == ఖాళీ) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	/* copy the parameter if necessary */
	ఒకవేళ (title != ఖాళీ) {
		తాత్కాలిక = strdup(title);
		ఒకవేళ (తాత్కాలిక == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
	} లేకపోతే {
		తాత్కాలిక = ఖాళీ;
	}

	విడుదల(p->urld.title);
	p->urld.title = తాత్కాలిక;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_సమితి_url_content_type(nsurl *url, content_type type)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	p->urld.type = type;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_సందర్శించు_దత్తాంశం(nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	p->urld.చివరి_సందర్శించు = time(ఖాళీ);
	p->urld.visits++;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
శూన్య urldb_reset_url_సందర్శించు_దత్తాంశం(nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి;

	p->urld.చివరి_సందర్శించు = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
స్థిరరాశి నిర్మాణం url_దత్తాంశం *urldb_get_url_దత్తాంశం(nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	నిర్మాణం url_అంతర్గత_దత్తాంశం *u;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి ఖాళీ;

	u = &p->urld;

	తిరిగి (స్థిరరాశి నిర్మాణం url_దత్తాంశం *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి ఖాళీ;

	తిరిగి p->url;
}


/* exported interface documented in netsurf/url_db.h */
శూన్య urldb_సమితి_auth_details(nsurl *url, స్థిరరాశి అక్షరం *realm, స్థిరరాశి అక్షరం *auth)
{
	నిర్మాణం మార్గం_దత్తాంశం *p, *pi;
	నిర్మాణం host_part *h;
	నిర్మాణం prot_space_దత్తాంశం *space, *space_alloc;
	అక్షరం *realm_alloc, *auth_alloc;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(url && realm && auth);

	/* add url, in case it's missing */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);

	ఒకవేళ (!p)
		తిరిగి;

	/* Search for host_part */
	కొరకు (pi = p; pi->మాతృ != ఖాళీ; pi = pi->మాతృ)
		;
	h = (నిర్మాణం host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	కొరకు (space = h->prot_space; space; space = space->తదుపరి) {
		ఒకవేళ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &సరిపోలిక) == lwc_error_ok &&
		    సరిపోలిక == true &&
		    space->పోర్టు == p->పోర్టు)
			ఆపు;
	}

	ఒకవేళ (space != ఖాళీ) {
		/* Overrule existing auth. */
		విడుదల(space->auth);
		space->auth = strdup(auth);
	} లేకపోతే {
		/* Create a new protection space. */
		space = space_alloc = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం prot_space_దత్తాంశం));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ఒకవేళ (!space_alloc || !realm_alloc || !auth_alloc) {
			విడుదల(space_alloc);
			విడుదల(realm_alloc);
			విడుదల(auth_alloc);
			తిరిగి;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->పోర్టు = p->పోర్టు;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->తదుపరి = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
స్థిరరాశి అక్షరం *urldb_get_auth_details(nsurl *url, స్థిరరాశి అక్షరం *realm)
{
	నిర్మాణం మార్గం_దత్తాంశం *p, *p_cur, *p_శిఖరం;

	ప్రతిజ్ఞా(url);

	/* add to the db, so our lookup will work */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి ఖాళీ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	కొరకు (p_cur = p; p_cur != ఖాళీ; p_శిఖరం = p_cur, p_cur = p_cur->మాతృ) {
		ఒకవేళ (p_cur->prot_space) {
			తిరిగి p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ఒకవేళ (realm != ఖాళీ) {
		స్థిరరాశి నిర్మాణం host_part *h = (స్థిరరాశి నిర్మాణం host_part *)p_శిఖరం;
		స్థిరరాశి నిర్మాణం prot_space_దత్తాంశం *space;
		bool సరిపోలిక;

		/* Search for a possible matching protection space. */
		కొరకు (space = h->prot_space; space != ఖాళీ;
		     space = space->తదుపరి) {
			ఒకవేళ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &సరిపోలిక) ==
			    lwc_error_ok &&
			    సరిపోలిక == true &&
			    space->పోర్టు == p->పోర్టు) {
				p->prot_space = space;
				తిరిగి p->prot_space->auth;
			}
		}
	}

	తిరిగి ఖాళీ;
}


/* exported interface documented in netsurf/url_db.h */
శూన్య urldb_సమితి_cert_permissions(nsurl *url, bool permit)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	నిర్మాణం host_part *h;

	ప్రతిజ్ఞా(url);

	/* add url, in case it's missing */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి;

	కొరకు (; p && p->మాతృ; p = p->మాతృ)
		/* do nothing */;
	ప్రతిజ్ఞా(p);

	h = (నిర్మాణం host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	స్థిరరాశి నిర్మాణం host_part *h;

	ప్రతిజ్ఞా(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి false;

	కొరకు (; p && p->మాతృ; p = p->మాతృ)
		/* do nothing */;
	ప్రతిజ్ఞా(p);

	h = (స్థిరరాశి నిర్మాణం host_part *)p;

	తిరిగి h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_సమితి_hsts_policy(నిర్మాణం nsurl *url, స్థిరరాశి అక్షరం *శీర్షిక)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	నిర్మాణం host_part *h;
	lwc_string *host;
	time_t now = time(ఖాళీ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ప్రతిజ్ఞా(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (host != ఖాళీ) {
		ఒకవేళ (urldb_host_is_ip_చిరునామా(lwc_string_దత్తాంశం(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			తిరిగి true;
		} లేకపోతే ఒకవేళ (lwc_string_నిడివి(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			తిరిగి true;
		}

		lwc_string_unref(host);
	} లేకపోతే {
		/* No host part: ignore */
		తిరిగి true;
	}

	/* add url, in case it's missing */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి false;

	కొరకు (; p && p->మాతృ; p = p->మాతృ)
		/* do nothing */;
	ప్రతిజ్ఞా(p);

	h = (నిర్మాణం host_part *)p;
	ఒకవేళ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		తిరిగి true;
	}

	error = http_parse_strict_transport_security(శీర్షిక, &sts);
	ఒకవేళ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		తిరిగి true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ఒకవేళ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} లేకపోతే ఒకవేళ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	తిరిగి true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(నిర్మాణం nsurl *url)
{
	నిర్మాణం మార్గం_దత్తాంశం *p;
	స్థిరరాశి నిర్మాణం host_part *h;
	lwc_string *host;
	time_t now = time(ఖాళీ);

	ప్రతిజ్ఞా(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (host != ఖాళీ) {
		ఒకవేళ (urldb_host_is_ip_చిరునామా(lwc_string_దత్తాంశం(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			తిరిగి false;
		} లేకపోతే ఒకవేళ (lwc_string_నిడివి(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			తిరిగి false;
		}

		lwc_string_unref(host);
	} లేకపోతే {
		/* No host part: not enabled */
		తిరిగి false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి false;

	కొరకు (; p && p->మాతృ; p = p->మాతృ)
		/* do nothing */;
	ప్రతిజ్ఞా(p);

	h = (స్థిరరాశి నిర్మాణం host_part *)p;

	/* Consult record for this host */
	ఒకవేళ (h->hsts.expires > now) {
		/* Not expired */
		తిరిగి true;
	}

	/* Consult parent domains */
	కొరకు (h = h->మాతృ; h && h != &db_మూలం; h = h->మాతృ) {
		ఒకవేళ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			తిరిగి true;
		}
	}

	తిరిగి false;
}


/* exported interface documented in netsurf/url_db.h */
శూన్య
urldb_iterate_partial(స్థిరరాశి అక్షరం *ఉపసర్గ,
		      bool (*callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం))
{
	అక్షరం host[256];
	అక్షరం buf[260]; /* max domain + "www." */
	స్థిరరాశి అక్షరం *slash, *scheme_sep;
	నిర్మాణం శోధన_కణుపు *వృక్షం;
	స్థిరరాశి నిర్మాణం host_part *h;

	ప్రతిజ్ఞా(ఉపసర్గ && callback);

	/* strip scheme */
	scheme_sep = అక్షరశ్రేణి_శోధన(ఉపసర్గ, "://");
	ఒకవేళ (scheme_sep)
		ఉపసర్గ = scheme_sep + 3;

	slash = strchr(ఉపసర్గ, '/');
	వృక్షం = urldb_get_శోధన_వృక్షం(ఉపసర్గ);

	ఒకవేళ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, పరిమాణం host, "%.*s",
			 (పూర్ణాంకం) (slash - ఉపసర్గ), ఉపసర్గ);

		h = urldb_శోధన_కనుగొను(వృక్షం, host);
		ఒకవేళ (!h) {
			పూర్ణాంకం len = slash - ఉపసర్గ;

			ఒకవేళ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, పరిమాణం buf, "www.%s", host);
				h = urldb_శోధన_కనుగొను(
					శోధన_trees[ST_DN + 'w' - 'a'],
					buf);
				ఒకవేళ (!h)
					తిరిగి;
			} లేకపోతే
				తిరిగి;
		}

		ఒకవేళ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_మార్గం(&h->paths, slash + 1,
						   callback);
		}

	} లేకపోతే {
		పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(ఉపసర్గ);

		/* looking for hosts */
		ఒకవేళ (!urldb_iterate_partial_host(వృక్షం, ఉపసర్గ, callback))
			తిరిగి;

		ఒకవేళ (len <= 3 || strncasecmp(ఉపసర్గ, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, పరిమాణం buf, "www.%s", ఉపసర్గ);
			ఒకవేళ(!urldb_iterate_partial_host(
				   శోధన_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				తిరిగి;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
శూన్య
urldb_iterate_entries(bool (*callback)(nsurl *url, స్థిరరాశి నిర్మాణం url_దత్తాంశం *దత్తాంశం))
{
	పూర్ణాంకం i;

	ప్రతిజ్ఞా(callback);

	కొరకు (i = 0; i < అంకె_శోధన_TREES; i++) {
		ఒకవేళ (!urldb_iterate_entries_host(శోధన_trees[i],
						callback,
						ఖాళీ)) {
			ఆపు;
		}
	}
}


/* exported interface documented in content/urldb.h */
శూన్య urldb_iterate_cookies(bool (*callback)(స్థిరరాశి నిర్మాణం cookie_దత్తాంశం *దత్తాంశం))
{
	పూర్ణాంకం i;

	ప్రతిజ్ఞా(callback);

	కొరకు (i = 0; i < అంకె_శోధన_TREES; i++) {
		ఒకవేళ (!urldb_iterate_entries_host(శోధన_trees[i],
						ఖాళీ, callback))
			ఆపు;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_సమితి_cookie(స్థిరరాశి అక్షరం *శీర్షిక, nsurl *url, nsurl *referer)
{
	స్థిరరాశి అక్షరం *cur = శీర్షిక, *చివర;
	lwc_string *మార్గం, *host, *scheme;
	nsurl *urlt;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(url && శీర్షిక);

	/* Get defragmented URL, as 'urlt' */
	ఒకవేళ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		తిరిగి ఖాళీ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (scheme == ఖాళీ) {
		nsurl_unref(urlt);
		తిరిగి false;
	}

	మార్గం = nsurl_get_component(url, NSURL_మార్గం);
	ఒకవేళ (మార్గం == ఖాళీ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		తిరిగి false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (host == ఖాళీ) {
		lwc_string_unref(మార్గం);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		తిరిగి false;
	}

	ఒకవేళ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ఒకవేళ (rhost == ఖాళీ) {
			వెళ్ళు error;
		}

		/* Domain match host names */
		ఒకవేళ (lwc_string_isequal(host, rhost, &సరిపోలిక) == lwc_error_ok &&
		    సరిపోలిక == false) {
			స్థిరరాశి అక్షరం *hptr;
			స్థిరరాశి అక్షరం *rptr;
			స్థిరరాశి అక్షరం *dot;
			స్థిరరాశి అక్షరం *host_దత్తాంశం = lwc_string_దత్తాంశం(host);
			స్థిరరాశి అక్షరం *rhost_దత్తాంశం = lwc_string_దత్తాంశం(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ఒకవేళ (urldb_host_is_ip_చిరునామా(host_దత్తాంశం) ||
			    urldb_host_is_ip_చిరునామా(rhost_దత్తాంశం)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				వెళ్ళు error;
			}

			/* Not exact match, so try the following:
			 *
			 * 1) Find the longest common suffix of host and rhost
			 *    (may be all of host/rhost)
			 * 2) Discard characters from the start of the suffix
			 *    until the suffix starts with a dot
			 *    (prevents foobar.com matching bar.com)
			 * 3) Ensure the suffix is non-empty and contains
			 *    embedded dots (to avoid permitting .com as a
			 *    suffix)
			 *
			 * Note that the above in no way resembles the
			 * domain matching algorithm found in RFC2109.
			 * It does, however, model the real world rather
			 * more accurately.
			 */

			/** \todo In future, we should consult a TLD service
			 * instead of just looking for embedded dots.
			 */

			hptr = host_దత్తాంశం + lwc_string_నిడివి(host) - 1;
			rptr = rhost_దత్తాంశం + lwc_string_నిడివి(rhost) - 1;

			/* 1 */
			అయితే (hptr >= host_దత్తాంశం && rptr >= rhost_దత్తాంశం) {
				ఒకవేళ (*hptr != *rptr)
					ఆపు;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			అయితే (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ఒకవేళ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ఖాళీ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				వెళ్ళు error;
			}
		}

		lwc_string_unref(rhost);
	}

	చివర = cur + అక్షరశ్రేణి_పొడవు(cur) - 2 /* Trailing CRLF */;

	చేయి {
		నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;
		అక్షరం *dot;
		size_t len;
#నిర్వచితమైతే WITH_NSPSL
		స్థిరరాశి అక్షరం *suffix;
#అయితే_ముగింపు

		c = urldb_parse_cookie(url, &cur);
		ఒకవేళ (!c) {
			/* failed => stop parsing */
			వెళ్ళు error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ఒకవేళ (!c->name || !c->విలువ) {
			urldb_free_cookie(c);
			వెళ్ళు error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = అక్షరశ్రేణి_పొడవు(c->మార్గం);
		ఒకవేళ (len > lwc_string_నిడివి(మార్గం) ||
		    అక్షరశ్రేణి_పోలిక(c->మార్గం, lwc_string_దత్తాంశం(మార్గం),
			    len) != 0) {
			urldb_free_cookie(c);
			వెళ్ళు error;
		}

#నిర్వచితమైతే WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ఒకవేళ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ఒకవేళ (suffix == ఖాళీ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			వెళ్ళు error;
		}
#లేకపోతే
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ఒకవేళ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			వెళ్ళు error;
		}
#అయితే_ముగింపు

		/* Domain match fetch host with cookie domain */
		ఒకవేళ (strcasecmp(lwc_string_దత్తాంశం(host), c->domain) != 0) {
			పూర్ణాంకం hlen, dlen;
			అక్షరం *domain = c->domain;

			/* c->domain must be a domain cookie here because:
			 * c->domain is either:
			 *   + specified in the header as a domain cookie
			 *     (non-domain cookies in the header are ignored
			 *      by urldb_parse_cookie / urldb_parse_avpair)
			 *   + defaulted to the URL's host part
			 *     (by urldb_parse_cookie if no valid domain was
			 *      specified in the header)
			 *
			 * The latter will pass the strcasecmp above, which
			 * leaves the former (i.e. a domain cookie)
			 */
			ప్రతిజ్ఞా(c->domain[0] == '.');

			/* 4.3.2:iii */
			ఒకవేళ (urldb_host_is_ip_చిరునామా(lwc_string_దత్తాంశం(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				వెళ్ళు error;
			}

			hlen = lwc_string_నిడివి(host);
			dlen = అక్షరశ్రేణి_పొడవు(c->domain);

			ఒకవేళ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				వెళ్ళు error;
			}

			ఒకవేళ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ఒకవేళ (strcasecmp(lwc_string_దత్తాంశం(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				వెళ్ళు error;
			}

			/* 4.3.2:iv Ensure H contains no dots
			 *
			 * If you believe the spec, H should contain no
			 * dots in _any_ cookie. Unfortunately, however,
			 * reality differs in that many sites send domain
			 * cookies of the form .foo.com from hosts such
			 * as bar.bat.foo.com and then expect domain
			 * matching to work. Thus we have to do what they
			 * expect, regardless of any potential security
			 * implications.
			 *
			 * This is what code conforming to the spec would
			 * look like:
			 *
			 * for (int i = 0; i < (hlen - dlen); i++) {
			 *	if (host[i] == '.') {
			 *		urldb_free_cookie(c);
			 *		goto error;
			 *	}
			 * }
			 */
		}

		/* Now insert into database */
		ఒకవేళ (!urldb_చొప్పించు_cookie(c, scheme, urlt))
			వెళ్ళు error;
	} అయితే (cur < చివర);

	lwc_string_unref(host);
	lwc_string_unref(మార్గం);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	తిరిగి true;

error:
	lwc_string_unref(host);
	lwc_string_unref(మార్గం);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	తిరిగి false;
}


/* exported interface documented in content/urldb.h */
అక్షరం *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	స్థిరరాశి నిర్మాణం మార్గం_దత్తాంశం *p, *q;
	స్థిరరాశి నిర్మాణం host_part *h;
	lwc_string *మార్గం_lwc;
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;
	పూర్ణాంకం లెక్క = 0, సంస్కరణ = COOKIE_RFC2965;
	నిర్మాణం cookie_అంతర్గత_దత్తాంశం **matched_cookies;
	పూర్ణాంకం matched_cookies_కొలత = 20;
	స్థిరరాశి అక్షరం *మార్గం;
	అక్షరం *ret;
	lwc_string *scheme;
	time_t now;
	bool సరిపోలిక;

	ప్రతిజ్ఞా(url != ఖాళీ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_కూడు_url(url);

	p = urldb_కనుగొను_url(url);
	ఒకవేళ (!p)
		తిరిగి ఖాళీ;

	scheme = p->scheme;

	matched_cookies = జ్ఞాపకం_ఇవ్వు(matched_cookies_కొలత *
				 పరిమాణం(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *));
	ఒకవేళ (!matched_cookies)
		తిరిగి ఖాళీ;

#నిర్వచించు GROW_MATCHED_COOKIES						\
	చేయి {								\
		ఒకవేళ (లెక్క == matched_cookies_కొలత) {			\
			నిర్మాణం cookie_అంతర్గత_దత్తాంశం **తాత్కాలిక;		\
			తాత్కాలిక = realloc(matched_cookies,			\
				       (matched_cookies_కొలత + 20) *	\
				       పరిమాణం(నిర్మాణం cookie_అంతర్గత_దత్తాంశం *)); \
									\
			ఒకవేళ (తాత్కాలిక == ఖాళీ) {				\
				విడుదల(matched_cookies);			\
				తిరిగి ఖాళీ;				\
			}						\
									\
			matched_cookies = తాత్కాలిక;				\
			matched_cookies_కొలత += 20;			\
		}							\
	} అయితే(0)

	మార్గం_lwc = nsurl_get_component(url, NSURL_మార్గం);
	ఒకవేళ (మార్గం_lwc == ఖాళీ) {
		విడుదల(matched_cookies);
		తిరిగి ఖాళీ;
	}
	మార్గం = lwc_string_దత్తాంశం(మార్గం_lwc);
	lwc_string_unref(మార్గం_lwc);

	now = time(ఖాళీ);

	ఒకవేళ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		కొరకు (q = p->మాతృ->children; q; q = q->తదుపరి) {
			ఒకవేళ (strcmp(q->segment, p->segment))
				కొనసాగు;

			/* Consider all cookies associated with
			 * this exact path */
			కొరకు (c = q->cookies; c; c = c->తదుపరి) {
				ఒకవేళ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					కొనసాగు;

				ఒకవేళ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &సరిపోలిక) &&
				    సరిపోలిక == false)
					/* secure cookie for insecure host.
					 * ignore */
					కొనసాగు;

				ఒకవేళ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					కొనసాగు;

				matched_cookies[లెక్క++] = c;

				GROW_MATCHED_COOKIES;

				ఒకవేళ (c->సంస్కరణ < (గుర్తులేని పూర్ణాంకం)సంస్కరణ)
					సంస్కరణ = c->సంస్కరణ;

				c->చివరి_వాడిన = now;

				cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	కొరకు (p = p->మాతృ; p; p = p->మాతృ) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		కొరకు (q = p->children; q; q = q->తదుపరి) {
			ఒకవేళ (*(q->segment) != '\0')
				కొనసాగు;

			కొరకు (c = q->cookies; c; c = c->తదుపరి) {
				ఒకవేళ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					కొనసాగు;

				ఒకవేళ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &సరిపోలిక) &&
				    సరిపోలిక == false)
					/* Secure cookie for insecure server
					 * => ignore */
					కొనసాగు;

				matched_cookies[లెక్క++] = c;

				GROW_MATCHED_COOKIES;

				ఒకవేళ (c->సంస్కరణ < (గుర్తులేని పూర్ణాంకం) సంస్కరణ)
					సంస్కరణ = c->సంస్కరణ;

				c->చివరి_వాడిన = now;

				cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
			}
		}

		ఒకవేళ (!p->మాతృ) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ఆపు;
		}

		/* Consider p itself - may be the result of Path=/foo */
		కొరకు (c = p->cookies; c; c = c->తదుపరి) {
			ఒకవేళ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				కొనసాగు;

			/* Ensure cookie path is a prefix of the resource */
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(c->మార్గం, మార్గం, అక్షరశ్రేణి_పొడవు(c->మార్గం)) != 0)
				/* paths don't match => ignore */
				కొనసాగు;

			ఒకవేళ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &సరిపోలిక) &&
			    సరిపోలిక == false)
				/* Secure cookie for insecure server
				 * => ignore */
				కొనసాగు;

			matched_cookies[లెక్క++] = c;

			GROW_MATCHED_COOKIES;

			ఒకవేళ (c->సంస్కరణ < (గుర్తులేని పూర్ణాంకం) సంస్కరణ)
				సంస్కరణ = c->సంస్కరణ;

			c->చివరి_వాడిన = now;

			cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	కొరకు (h = (స్థిరరాశి నిర్మాణం host_part *)p; h && h != &db_మూలం;
	     h = h->మాతృ) {
		కొరకు (c = h->paths.cookies; c; c = c->తదుపరి) {
			ఒకవేళ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				కొనసాగు;

			/* Ensure cookie path is a prefix of the resource */
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(c->మార్గం, మార్గం, అక్షరశ్రేణి_పొడవు(c->మార్గం)) != 0)
				/* paths don't match => ignore */
				కొనసాగు;

			ఒకవేళ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &సరిపోలిక) &&
			    సరిపోలిక == false)
				/* secure cookie for insecure host. ignore */
				కొనసాగు;

			matched_cookies[లెక్క++] = c;

			GROW_MATCHED_COOKIES;

			ఒకవేళ (c->సంస్కరణ < (గుర్తులేని పూర్ణాంకం)సంస్కరణ)
				సంస్కరణ = c->సంస్కరణ;

			c->చివరి_వాడిన = now;

			cookie_manager_కూడు((నిర్మాణం cookie_దత్తాంశం *)c);
		}
	}
#నిర్వచనం_రద్దు GROW_MATCHED_COOKIES

	ఒకవేళ (లెక్క == 0) {
		/* No cookies found */
		విడుదల(matched_cookies);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (సంస్కరణ == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, లెక్క);
	} లేకపోతే {
		ret = get_rfc2965_cookie(matched_cookies, లెక్క, సంస్కరణ);
	}
	విడుదల(matched_cookies);

	తిరిగి ret;
}


/* exported interface documented in content/urldb.h */
శూన్య urldb_delete_cookie(స్థిరరాశి అక్షరం *domain, స్థిరరాశి అక్షరం *మార్గం,
			 స్థిరరాశి అక్షరం *name)
{
	urldb_delete_cookie_hosts(domain, మార్గం, name, &db_మూలం);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_ఎక్కించు_cookies(స్థిరరాశి అక్షరం *filename)
{
	FILE *fp;
	అక్షరం s[16*1024];

	ప్రతిజ్ఞా(filename);

	fp = fopen(filename, "r");
	ఒకవేళ (!fp)
		తిరిగి NSERROR_NOT_దొరికింది;

#నిర్వచించు కనుగొను_T {				\
		కొరకు (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ఒకవేళ (p >= చివర) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			కొనసాగు;		\
		}				\
		*p++ = '\0';			\
	}

#నిర్వచించు SKIP_T {				\
		కొరకు (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ఒకవేళ (p >= చివర) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			కొనసాగు;		\
		}				\
	}

	అయితే (fgets(s, పరిమాణం s, fp)) {
		అక్షరం *p = s, *చివర = 0,
			*domain, *మార్గం, *name, *విలువ, *scheme, *url,
			*comment;
		పూర్ణాంకం సంస్కరణ, domain_specified, మార్గం_specified,
			secure, http_only, no_destroy, విలువ_quoted;
		time_t expires, చివరి_వాడిన;
		నిర్మాణం cookie_అంతర్గత_దత్తాంశం *c;

		ఒకవేళ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			కొనసాగు;

		s[అక్షరశ్రేణి_పొడవు(s) - 1] = '\0'; /* lose terminating newline */
		చివర = s + అక్షరశ్రేణి_పొడవు(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ఒకవేళ (strncasecmp(s, "Version:", 8) == 0) {
			కనుగొను_T; SKIP_T; loaded_cookie_file_సంస్కరణ = atoi(p);

			ఒకవేళ (loaded_cookie_file_సంస్కరణ <
			    MIN_COOKIE_FILE_సంస్కరణ) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ఆపు;
			}

			కొనసాగు;
		} లేకపోతే ఒకవేళ (loaded_cookie_file_సంస్కరణ == 0) {
			/* Haven't yet seen version; skip this input */
			కొనసాగు;
		}

		/* One cookie/line */

		/* Parse input */
		కనుగొను_T; సంస్కరణ = atoi(s);
		SKIP_T; domain = p; కనుగొను_T;
		SKIP_T; domain_specified = atoi(p); కనుగొను_T;
		SKIP_T; మార్గం = p; కనుగొను_T;
		SKIP_T; మార్గం_specified = atoi(p); కనుగొను_T;
		SKIP_T; secure = atoi(p); కనుగొను_T;
		ఒకవేళ (loaded_cookie_file_సంస్కరణ > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); కనుగొను_T;
		} లేకపోతే {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); కనుగొను_T;
		SKIP_T; చివరి_వాడిన = (time_t)atoi(p); కనుగొను_T;
		SKIP_T; no_destroy = atoi(p); కనుగొను_T;
		SKIP_T; name = p; కనుగొను_T;
		SKIP_T; విలువ = p; కనుగొను_T;
		ఒకవేళ (loaded_cookie_file_సంస్కరణ > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	విలువ_quoted = atoi(p); కనుగొను_T;
		} లేకపోతే {
			విలువ_quoted = 0;
		}
		SKIP_T; scheme = p; కనుగొను_T;
		SKIP_T; url = p; కనుగొను_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		కొరకు (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ప్రతిజ్ఞా(p <= చివర);

		/* Now create cookie */
		c = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం cookie_అంతర్గత_దత్తాంశం));
		ఒకవేళ (!c)
			ఆపు;

		c->name = strdup(name);
		c->విలువ = strdup(విలువ);
		c->విలువ_was_quoted = విలువ_quoted;
		c->comment = strdup(comment);
		c->domain_from_సమితి = domain_specified;
		c->domain = strdup(domain);
		c->మార్గం_from_సమితి = మార్గం_specified;
		c->మార్గం = strdup(మార్గం);
		c->expires = expires;
		c->చివరి_వాడిన = చివరి_వాడిన;
		c->secure = secure;
		c->http_only = http_only;
		c->సంస్కరణ = సంస్కరణ;
		c->no_destroy = no_destroy;

		ఒకవేళ (!(c->name && c->విలువ && c->comment &&
		      c->domain && c->మార్గం)) {
			urldb_free_cookie(c);
			ఆపు;
		}

		ఒకవేళ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ఖాళీ;
			nsurl *url_nsurl = ఖాళీ;

			ప్రతిజ్ఞా(scheme[0] != 'u');

			ఒకవేళ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ఆపు;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ఒకవేళ (!urldb_చొప్పించు_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ఆపు;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} లేకపోతే {
			ఒకవేళ (!urldb_చొప్పించు_cookie(c, ఖాళీ, ఖాళీ)) {
				/* Cookie freed for us */
				ఆపు;
			}
		}
	}

#నిర్వచనం_రద్దు SKIP_T
#నిర్వచనం_రద్దు కనుగొను_T

	fclose(fp);

	తిరిగి NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(స్థిరరాశి అక్షరం *filename)
{
	FILE *fp;
	పూర్ణాంకం cookie_file_సంస్కరణ = max(loaded_cookie_file_సంస్కరణ,
				      COOKIE_FILE_సంస్కరణ);

	ప్రతిజ్ఞా(filename);

	fp = fopen(filename, "w");
	ఒకవేళ (!fp)
		తిరిగి NSERROR_NOT_దొరికింది;

	fprintf(fp, "# NetSurf cookies file.\n"
		"#\n"
		"# Lines starting with a '#' are comments, "
		"blank lines are ignored.\n"
		"#\n"
		"# All lines prior to \"Version:\t%d\" are discarded.\n"
		"#\n"
		"# Version\tDomain\tDomain from Set-Cookie\tPath\t"
		"Path from Set-Cookie\tSecure\tHTTP-Only\tExpires\tLast used\t"
		"No destroy\tName\tValue\tValue was quoted\tScheme\t"
		"URL\tComment\n",
		cookie_file_సంస్కరణ);
	fprintf(fp, "Version:\t%d\n", cookie_file_సంస్కరణ);

	urldb_save_cookie_hosts(fp, &db_మూలం);

	fclose(fp);

	తిరిగి NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
శూన్య urldb_dump(శూన్య)
{
	పూర్ణాంకం i;

	urldb_dump_hosts(&db_మూలం);

	కొరకు (i = 0; i != అంకె_శోధన_TREES; i++) {
		urldb_dump_శోధన(శోధన_trees[i], 0);
	}
}
