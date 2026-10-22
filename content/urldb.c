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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_NSPSL
#ಅನ್ತರ್ಭಾವ <nspsl.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "utils/inet.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/url.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/bloom.h"
#ಅನ್ತರ್ಭಾವ "utils/time.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "desktop/cookie_manager.h"

#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ {
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *prev;	/**< Previous in list */
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *ಮುಂದಿನ;	/**< Next in list */

	ಅಕ್ಷರ *name;		/**< Cookie name */
	ಅಕ್ಷರ *ಮೌಲ್ಯ;		/**< Cookie value */
	bool ಮೌಲ್ಯ_was_quoted;	/**< Value was quoted in Set-Cookie: */
	ಅಕ್ಷರ *comment;		/**< Cookie comment */
	bool domain_from_ಗಣ;	/**< Domain came from Set-Cookie: header */
	ಅಕ್ಷರ *domain;		/**< Domain */
	bool ಮಾರ್ಗ_from_ಗಣ;	/**< Path came from Set-Cookie: header */
	ಅಕ್ಷರ *ಮಾರ್ಗ;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t ಕೊನೆಯ_ಬಳಸಿದ;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ಎಣಿಕೆ cookie_ಆವೃತ್ತಿ ಆವೃತ್ತಿ;	/**< Specification compliance */
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
ರಚನೆ prot_space_ದತ್ತಾಂಶ {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್;
	/** Protection realm */
	ಅಕ್ಷರ *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	ಅಕ್ಷರ *auth;
	/** Next sibling */
	ರಚನೆ prot_space_ದತ್ತಾಂಶ *ಮುಂದಿನ;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ರಚನೆ url_ಆಂತರಿಕ_ದತ್ತಾಂಶ {
	ಅಕ್ಷರ *title;		/**< Resource title */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ visits;	/**< Visit count */
	time_t ಕೊನೆಯ_ಭೇಟಿ;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	ಅಕ್ಷರ *segment;		/**< Path segment for this node */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ frag_cnt;	/**< Number of entries in path_data::fragment */
	ಅಕ್ಷರ **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ರಚನೆ url_ಆಂತರಿಕ_ದತ್ತಾಂಶ urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ಸ್ಥಿರಾಂಕ ರಚನೆ prot_space_ದತ್ತಾಂಶ *prot_space;
	/** Cookies associated with resource */
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *cookies;
	/** Last cookie in list */
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *cookies_ಕೊನೆ;

	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಮುಂದಿನ;	/**< Next sibling */
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *prev;	/**< Previous sibling */
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ; /**< Parent path segment */
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *children; /**< Child path segments */
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಕೊನೆಯ; /**< Last child */
};

ರಚನೆ hsts_ದತ್ತಾಂಶ {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ರಚನೆ host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ರಚನೆ hsts_ದತ್ತಾಂಶ hsts;

	/**
	 * Part of host string
	 */
	ಅಕ್ಷರ *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ರಚನೆ prot_space_ದತ್ತಾಂಶ *prot_space;

	ರಚನೆ host_part *ಮುಂದಿನ;	/**< Next sibling */
	ರಚನೆ host_part *prev;	/**< Previous sibling */
	ರಚನೆ host_part *ಪೋಷಕ; /**< Parent host part */
	ರಚನೆ host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ರಚನೆ ಹುಡುಕು_ಗಂಟು {
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *ದತ್ತಾಂಶ;	/**< Host tree entry */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ level;		/**< Node level */

	ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಎಡ;	/**< Left subtree */
	ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬಲ;	/**< Right subtree */
};

/** Root database handle */
ಸ್ಥಿರ ರಚನೆ host_part db_ಬೇರು;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_ಹುಡುಕು_TREES 28
#ವ್ಯಾಖ್ಯಾನಿಸು ST_IP 0
#ವ್ಯಾಖ್ಯಾನಿಸು ST_EE 1
#ವ್ಯಾಖ್ಯಾನಿಸು ST_DN 2
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು ಖಾಲಿಗಳು = { 0, 0, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು };
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಹುಡುಕು_trees[ಅಂಕೆ_ಹುಡುಕು_TREES] = {
	&ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು,
	&ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು,
	&ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು,
	&ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು, &ಖಾಲಿಗಳು
};

/** Minimum cookie database file version */
#ವ್ಯಾಖ್ಯಾನಿಸು MIN_COOKIE_FILE_ಆವೃತ್ತಿ 100
/** Current cookie database file version */
#ವ್ಯಾಖ್ಯಾನಿಸು COOKIE_FILE_ಆವೃತ್ತಿ 102
/** loaded cookie file version */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loaded_cookie_file_ಆವೃತ್ತಿ;

/** Minimum URL database file version */
#ವ್ಯಾಖ್ಯಾನಿಸು MIN_URL_FILE_ಆವೃತ್ತಿ 106
/** Current URL database file version */
#ವ್ಯಾಖ್ಯಾನಿಸು URL_FILE_ಆವೃತ್ತಿ 107

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
ಸ್ಥಿರ ರಚನೆ bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#ವ್ಯಾಖ್ಯಾನಿಸು BLOOM_ಗಾತ್ರ (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
ಸ್ಥಿರ nserror urldb_write_timet(FILE *fp, time_t val)
{
	ಪೂರ್ಣಾಂಕ use;
	ಅಕ್ಷರ op[32];

	use = nsc_sntimet(op, 32, &val);
	ಆದರೆ (use == 0) {
		fprintf(fp, "%i\n", (ಪೂರ್ಣಾಂಕ)val);
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(fp, "%.*s\n", use, op);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ ಶೂನ್ಯ
urldb_write_paths(ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host,
		  FILE *fp,
		  ಅಕ್ಷರ **ಮಾರ್ಗ,
		  ಪೂರ್ಣಾಂಕ *ಮಾರ್ಗ_alloc,
		  ಪೂರ್ಣಾಂಕ *ಮಾರ್ಗ_ಬಳಸಿದ,
		  time_t expiry)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ;
	ಪೂರ್ಣಾಂಕ i;

	ಮಾಡು {
		ಪೂರ್ಣಾಂಕ seglen = p->segment != ಖಾಲಿ ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p->segment) : 0;
		ಪೂರ್ಣಾಂಕ len = *ಮಾರ್ಗ_ಬಳಸಿದ + seglen + 1;

		ಆದರೆ (*ಮಾರ್ಗ_alloc < len) {
			ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ;
			ತಾತ್ಕಾಲಿಕ = realloc(*ಮಾರ್ಗ,
				       (len > 64) ? len : *ಮಾರ್ಗ_alloc + 64);
			ಆದರೆ (!ತಾತ್ಕಾಲಿಕ) {
				ಹಿಂತಿರುಗಿ;
			}
			*ಮಾರ್ಗ = ತಾತ್ಕಾಲಿಕ;
			*ಮಾರ್ಗ_alloc = (len > 64) ? len : *ಮಾರ್ಗ_alloc + 64;
		}

		ಆದರೆ (p->segment != ಖಾಲಿ) {
			ಸ್ಮರಣೆ_ನಕಲು(*ಮಾರ್ಗ + *ಮಾರ್ಗ_ಬಳಸಿದ - 1, p->segment, seglen);
		}

		ಆದರೆ (p->children != ಖಾಲಿ) {
			(*ಮಾರ್ಗ)[*ಮಾರ್ಗ_ಬಳಸಿದ + seglen - 1] = '/';
			(*ಮಾರ್ಗ)[*ಮಾರ್ಗ_ಬಳಸಿದ + seglen] = '\0';
		} ಇಲ್ಲದಿದ್ದರೆ {
			(*ಮಾರ್ಗ)[*ಮಾರ್ಗ_ಬಳಸಿದ + seglen - 1] = '\0';
			len -= 1;
		}

		*ಮಾರ್ಗ_ಬಳಸಿದ = len;

		ಆದರೆ (p->children != ಖಾಲಿ) {
			/* Drill down into children */
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* leaf node */
			ಆದರೆ (p->persistent ||
			    ((p->urld.ಕೊನೆಯ_ಭೇಟಿ > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_ದತ್ತಾಂಶ(p->scheme));

				ಆದರೆ (p->ಪೋರ್ಟ್) {
					fprintf(fp,"%d\n", p->ಪೋರ್ಟ್);
				} ಇಲ್ಲದಿದ್ದರೆ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *ಮಾರ್ಗ);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.ಕೊನೆಯ_ಭೇಟಿ);

				/* entry type */
				fprintf(fp, "%i\n", (ಪೂರ್ಣಾಂಕ)p->urld.type);

				fprintf(fp, "\n");

				ಆದರೆ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					ಗಾಗಿ (i = 0; s[i] != '\0'; i++)
						ಆದರೆ (s[i] < 32)
							s[i] = ' ';
					ಗಾಗಿ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ಇಲ್ಲದಿದ್ದರೆ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ಆಗ (p != ಪೋಷಕ) {
				ಪೂರ್ಣಾಂಕ seglen = p->segment != ಖಾಲಿ
					? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p->segment) : 0;

				/* Remove our segment from the path */
				*ಮಾರ್ಗ_ಬಳಸಿದ -= seglen;
				(*ಮಾರ್ಗ)[*ಮಾರ್ಗ_ಬಳಸಿದ - 1] = '\0';

				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					/* Have a sibling, process that */
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				/* Going up, so remove '/' */
				*ಮಾರ್ಗ_ಬಳಸಿದ -= 1;
				(*ಮಾರ್ಗ)[*ಮಾರ್ಗ_ಬಳಸಿದ - 1] = '\0';

				/* Ascend tree */
				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಪೋಷಕ);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ಸ್ಥಿರ ಶೂನ್ಯ
urldb_ಎಣಿಕೆ_urls(ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಬೇರು,
		 time_t expiry,
		 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *ಎಣಿಕೆ_2)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಬೇರು;

	ಮಾಡು {
		ಆದರೆ (p->children != ಖಾಲಿ) {
			/* Drill down into children */
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* No more children, increment count if required */
			ಆದರೆ (p->persistent ||
			    ((p->urld.ಕೊನೆಯ_ಭೇಟಿ > expiry) &&
			     (p->urld.visits > 0))) {
				(*ಎಣಿಕೆ_2)++;
			}

			/* Now, find next node to process. */
			ಆಗ (p != ಬೇರು) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					/* Have a sibling, process that */
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				/* Ascend tree */
				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಬೇರು);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಪೋಷಕ, FILE *fp)
{
	ಅಕ್ಷರ host[256];
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮಾರ್ಗ_ಎಣಿಕೆ = 0;
	ಅಕ್ಷರ *ಮಾರ್ಗ, *p, *ಕೊನೆ;
	ಪೂರ್ಣಾಂಕ ಮಾರ್ಗ_alloc = 64, ಮಾರ್ಗ_ಬಳಸಿದ = 1;
	time_t expiry, hsts_expiry = 0;
	ಪೂರ್ಣಾಂಕ hsts_include_subdomains = 0;

	expiry = time(ಖಾಲಿ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ಆದರೆ (ಪೋಷಕ == &ಖಾಲಿಗಳು)
		ಹಿಂತಿರುಗಿ;

	urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ಪೋಷಕ->ಎಡ, fp);

	ಮಾರ್ಗ = ಸ್ಮರಣೆ_ಕೊಡು(ಮಾರ್ಗ_alloc);
	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ;

	ಮಾರ್ಗ[0] = '\0';

	ಗಾಗಿ (h = ಪೋಷಕ->ದತ್ತಾಂಶ, p = host, ಕೊನೆ = host + ಗಾತ್ರ host;
	     h && h != &db_ಬೇರು && p < ಕೊನೆ; h = h->ಪೋಷಕ) {
		ಪೂರ್ಣಾಂಕ written = snprintf(p, ಕೊನೆ - p, "%s%s", h->part,
				       (h->ಪೋಷಕ && h->ಪೋಷಕ->ಪೋಷಕ) ? "." : "");
		ಆದರೆ (written < 0) {
			ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
			ಹಿಂತಿರುಗಿ;
		}
		p += written;
	}

	h = ಪೋಷಕ->ದತ್ತಾಂಶ;
	ಆದರೆ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_ಎಣಿಕೆ_urls(&ಪೋಷಕ->ದತ್ತಾಂಶ->paths, expiry, &ಮಾರ್ಗ_ಎಣಿಕೆ);

	ಆದರೆ (ಮಾರ್ಗ_ಎಣಿಕೆ > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", ಮಾರ್ಗ_ಎಣಿಕೆ);

		urldb_write_paths(&ಪೋಷಕ->ದತ್ತಾಂಶ->paths, host, fp,
				  &ಮಾರ್ಗ, &ಮಾರ್ಗ_alloc, &ಮಾರ್ಗ_ಬಳಸಿದ, expiry);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	ಬಿಡುಗಡೆ(ಮಾರ್ಗ);

	urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ಪೋಷಕ->ಬಲ, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ಸ್ಥಿರ bool
urldb_iterate_entries_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ,
		bool (*url_callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ),
		bool (*cookie_callback)(ಸ್ಥಿರಾಂಕ ರಚನೆ cookie_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ;
	ಸ್ಥಿರಾಂಕ ರಚನೆ cookie_ದತ್ತಾಂಶ *c;

	ಮಾಡು {
		ಆದರೆ (p->children != ಖಾಲಿ) {
			/* Drill down into children */
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ಪ್ರತಿಜ್ಞಾ(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ಆದರೆ (url_callback) {
				ಸ್ಥಿರಾಂಕ ರಚನೆ url_ಆಂತರಿಕ_ದತ್ತಾಂಶ *u = &p->urld;

				ಪ್ರತಿಜ್ಞಾ(p->url);

				ಆದರೆ (!url_callback(p->url,
						  (ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *) u))
					ಹಿಂತಿರುಗಿ false;
			} ಇಲ್ಲದಿದ್ದರೆ {
				c = (ಸ್ಥಿರಾಂಕ ರಚನೆ cookie_ದತ್ತಾಂಶ *)p->cookies;
				ಗಾಗಿ (; c != ಖಾಲಿ; c = c->ಮುಂದಿನ) {
					ಆದರೆ (!cookie_callback(c))
						ಹಿಂತಿರುಗಿ false;
				}
			}

			/* Now, find next node to process. */
			ಆಗ (p != ಪೋಷಕ) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					/* Have a sibling, process that */
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				/* Ascend tree */
				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಪೋಷಕ);

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool urldb_host_is_ip_ವಿಳಾಸ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host)
{
	ರಚನೆ ಒಳ_addr ipv4;
	size_t host_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(host);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sane_host;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_IPV6
	ರಚನೆ in6_addr ipv6;
	ಅಕ್ಷರ ipv6_addr[64];
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ipv6_addr_len;
#ಆದರೆ_ಅಂತ್ಯ
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
	ಆದರೆ (slash == ಖಾಲಿ) {
		sane_host = host;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ಆದರೆ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		ಹೋಗು ಹೊರಹೋಗು_false;

	ಆದರೆ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		ಪೂರ್ಣಾಂಕ ಅಂಕೆ_dots = 0;
		size_t index;

		ಗಾಗಿ (index = 0; index < host_len; index++) {
			ಆದರೆ (sane_host[index] == '.')
				ಅಂಕೆ_dots++;
		}

		ಆದರೆ (ಅಂಕೆ_dots == 3)
			ಹೋಗು ಹೊರಹೋಗು_true;
		ಇಲ್ಲದಿದ್ದರೆ
			ಹೋಗು ಹೊರಹೋಗು_false;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_IPV6
	ಆದರೆ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		ಹೋಗು ಹೊರಹೋಗು_false;
	}

	ipv6_addr_len = host_len - 2;
	ಆದರೆ (ipv6_addr_len >= ಗಾತ್ರ(ipv6_addr)) {
		ipv6_addr_len = ಗಾತ್ರ(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ಆದರೆ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		ಹೋಗು ಹೊರಹೋಗು_true;
#ಆದರೆ_ಅಂತ್ಯ

ಹೊರಹೋಗು_false:
	ಆದರೆ (slash != ಖಾಲಿ) ಬಿಡುಗಡೆ((ಶೂನ್ಯ *)sane_host);
	ಹಿಂತಿರುಗಿ false;

ಹೊರಹೋಗು_true:
	ಆದರೆ (slash != ಖಾಲಿ) ಬಿಡುಗಡೆ((ಶೂನ್ಯ *)sane_host);
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ, *dot;
	ಪೂರ್ಣಾಂಕ plen, ret;

	ಪ್ರತಿಜ್ಞಾ(a && a != &db_ಬೇರು && b);

	ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(b)) {
		/* IP address */
		ಹಿಂತಿರುಗಿ strncasecmp(a->part, b, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b));
	}

	ಕೊನೆ = b + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b) + 1;

	ಆಗ (b < ಕೊನೆ && a && a != &db_ಬೇರು) {
		dot = strchr(b, '.');
		ಆದರೆ (!dot) {
			/* last segment */
			dot = ಕೊನೆ - 1;
		}

		/* Compare strings (length limited) */
		ಆದರೆ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ಹಿಂತಿರುಗಿ ret;

		/* The strings matched */
		ಆದರೆ (dot < ಕೊನೆ - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(a->part);
			ಆದರೆ (plen > dot - b) {
				/* len(a) > len(b) */
				ಹಿಂತಿರುಗಿ 1;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (plen < dot - b) {
				/* len(a) < len(b) */
				ಹಿಂತಿರುಗಿ -1;
			}
		}

		b = dot + 1;
		a = a->ಪೋಷಕ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಆದರೆ (a && a != &db_ಬೇರು && b >= ಕೊನೆ) {
		/* len(a) > len(b) => prefix matches */
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((!a || a == &db_ಬೇರು) && b < ಕೊನೆ) {
		/* len(a) < len(b) => prefix does not match */
		ಹಿಂತಿರುಗಿ -1;
	}

	/* Identical */
	ಹಿಂತಿರುಗಿ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ಸ್ಥಿರ bool
urldb_iterate_partial_host(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ,
		bool (*callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಪೂರ್ಣಾಂಕ c;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && ಪೂರ್ವಪ್ರತ್ಯಯ && callback);

	ಆದರೆ (ಬೇರು == &ಖಾಲಿಗಳು)
		ಹಿಂತಿರುಗಿ true;

	c = urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_ಪೂರ್ವಪ್ರತ್ಯಯ(ಬೇರು->ದತ್ತಾಂಶ, ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಆದರೆ (c > 0) {
		/* No match => look in left subtree */
		ಹಿಂತಿರುಗಿ urldb_iterate_partial_host(ಬೇರು->ಎಡ,
						  ಪೂರ್ವಪ್ರತ್ಯಯ,
						  callback);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c < 0) {
		/* No match => look in right subtree */
		ಹಿಂತಿರುಗಿ urldb_iterate_partial_host(ಬೇರು->ಬಲ,
						  ಪೂರ್ವಪ್ರತ್ಯಯ,
						  callback);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Match => iterate over l/r subtrees & process this node */
		ಆದರೆ (!urldb_iterate_partial_host(ಬೇರು->ಎಡ,
						ಪೂರ್ವಪ್ರತ್ಯಯ,
						callback)) {
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (ಬೇರು->ದತ್ತಾಂಶ->paths.children) {
			/* and extract all paths attached to this host */
			ಆದರೆ (!urldb_iterate_entries_ಮಾರ್ಗ(&ಬೇರು->ದತ್ತಾಂಶ->paths,
							callback,
							ಖಾಲಿ)) {
				ಹಿಂತಿರುಗಿ false;
			}
		}

		ಆದರೆ (!urldb_iterate_partial_host(ಬೇರು->ಬಲ,
						ಪೂರ್ವಪ್ರತ್ಯಯ,
						callback)) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
urldb_iterate_partial_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ,
		bool (*callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ->children;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash, *ಕೊನೆ = ಪೂರ್ವಪ್ರತ್ಯಯ + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಮಾಡು {
		slash = strchr(ಪೂರ್ವಪ್ರತ್ಯಯ, '/');
		ಆದರೆ (!slash) {
			slash = ಕೊನೆ;
		}

		ಆದರೆ (slash == ಪೂರ್ವಪ್ರತ್ಯಯ && *ಪೂರ್ವಪ್ರತ್ಯಯ == '/') {
			/* Ignore "//" */
			ಪೂರ್ವಪ್ರತ್ಯಯ++;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (strncasecmp(p->segment, ಪೂರ್ವಪ್ರತ್ಯಯ, slash - ಪೂರ್ವಪ್ರತ್ಯಯ) == 0) {
			/* prefix matches so far */
			ಆದರೆ (slash == ಕೊನೆ) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ಆದರೆ (!urldb_iterate_entries_ಮಾರ್ಗ(p,
								callback,
								ಖಾಲಿ)) {
					ಹಿಂತಿರುಗಿ false;
				}

				/* Progress to next sibling */
				p = p->ಮುಂದಿನ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Skip over this segment */
				ಪೂರ್ವಪ್ರತ್ಯಯ = slash + 1;

				p = p->children;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Doesn't match this segment, try next sibling */
			p = p->ಮುಂದಿನ;
		}
	} ಆಗ (p != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ಸ್ಥಿರ bool
urldb_iterate_entries_host(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಪೋಷಕ,
		bool (*url_callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ),
		bool (*cookie_callback)(ಸ್ಥಿರಾಂಕ ರಚನೆ cookie_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಆದರೆ (ಪೋಷಕ == &ಖಾಲಿಗಳು) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಆದರೆ (!urldb_iterate_entries_host(ಪೋಷಕ->ಎಡ,
					url_callback,
					cookie_callback)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ ((ಪೋಷಕ->ದತ್ತಾಂಶ->paths.children) ||
	    ((cookie_callback) &&
	     (ಪೋಷಕ->ದತ್ತಾಂಶ->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ಆದರೆ (!urldb_iterate_entries_ಮಾರ್ಗ(&ಪೋಷಕ->ದತ್ತಾಂಶ->paths,
						url_callback,
						cookie_callback)) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಆದರೆ (!urldb_iterate_entries_host(ಪೋಷಕ->ಬಲ,
					url_callback,
					cookie_callback)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ರಚನೆ host_part *
urldb_ಕೂಡಿಸು_host_ಗಂಟು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *part, ರಚನೆ host_part *ಪೋಷಕ)
{
	ರಚನೆ host_part *d;

	ಪ್ರತಿಜ್ಞಾ(part && ಪೋಷಕ);

	d = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ host_part));
	ಆದರೆ (!d) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	d->part = strdup(part);
	ಆದರೆ (!d->part) {
		ಬಿಡುಗಡೆ(d);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	d->ಮುಂದಿನ = ಪೋಷಕ->children;
	ಆದರೆ (ಪೋಷಕ->children) {
		ಪೋಷಕ->children->prev = d;
	}
	d->ಪೋಷಕ = ಪೋಷಕ;
	ಪೋಷಕ->children = d;

	ಹಿಂತಿರುಗಿ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_fragment_cmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b)
{
	ಹಿಂತಿರುಗಿ strcasecmp(*((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **) a), *((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *
urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_fragment(ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *segment, lwc_string *fragment)
{
	ಅಕ್ಷರ **ತಾತ್ಕಾಲಿಕ;

	ಪ್ರತಿಜ್ಞಾ(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ಆದರೆ (!fragment)
		ಹಿಂತಿರುಗಿ segment;

	ತಾತ್ಕಾಲಿಕ = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * ಗಾತ್ರ(ಅಕ್ಷರ *));
	ಆದರೆ (!ತಾತ್ಕಾಲಿಕ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	segment->fragment = ತಾತ್ಕಾಲಿಕ;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_ದತ್ತಾಂಶ(fragment));
	ಆದರೆ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      ಗಾತ್ರ (ಅಕ್ಷರ *),
	      urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_fragment_cmp);

	ಹಿಂತಿರುಗಿ segment;
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
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *
urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_ಗಂಟು(lwc_string *scheme,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *segment,
		    lwc_string *fragment,
		    ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *d, *e;

	ಪ್ರತಿಜ್ಞಾ(scheme && segment && ಪೋಷಕ);

	d = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ));
	ಆದರೆ (!d)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	d->scheme = lwc_string_ref(scheme);

	d->ಪೋರ್ಟ್ = ಪೋರ್ಟ್;

	d->segment = strdup(segment);
	ಆದರೆ (!d->segment) {
		lwc_string_unref(d->scheme);
		ಬಿಡುಗಡೆ(d);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (fragment) {
		ಆದರೆ (!urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_fragment(d, fragment)) {
			ಬಿಡುಗಡೆ(d->segment);
			lwc_string_unref(d->scheme);
			ಬಿಡುಗಡೆ(d);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	ಗಾಗಿ (e = ಪೋಷಕ->children; e; e = e->ಮುಂದಿನ) {
		ಆದರೆ (strcmp(e->segment, d->segment) > 0)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (e) {
		d->prev = e->prev;
		d->ಮುಂದಿನ = e;
		ಆದರೆ (e->prev)
			e->prev->ಮುಂದಿನ = d;
		ಇಲ್ಲದಿದ್ದರೆ
			ಪೋಷಕ->children = d;
		e->prev = d;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಪೋಷಕ->children) {
		d->prev = d->ಮುಂದಿನ = ಖಾಲಿ;
		ಪೋಷಕ->children = ಪೋಷಕ->ಕೊನೆಯ = d;
	} ಇಲ್ಲದಿದ್ದರೆ {
		d->ಮುಂದಿನ = ಖಾಲಿ;
		d->prev = ಪೋಷಕ->ಕೊನೆಯ;
		ಪೋಷಕ->ಕೊನೆಯ->ಮುಂದಿನ = d;
		ಪೋಷಕ->ಕೊನೆಯ = d;
	}
	d->ಪೋಷಕ = ಪೋಷಕ;

	ಹಿಂತಿರುಗಿ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು **urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_ನೇರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host)
{
	ಪ್ರತಿಜ್ಞಾ(host);

	ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(host)) {
		ಹಿಂತಿರುಗಿ &ಹುಡುಕು_trees[ST_IP];
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ascii_is_alpha(*host)) {
		ಹಿಂತಿರುಗಿ &ಹುಡುಕು_trees[ST_DN + ascii_ಗಮ್ಯ_ಅಧೋ(*host) - 'a'];
	}
	ಹಿಂತಿರುಗಿ &ಹುಡುಕು_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host)
{
	ಹಿಂತಿರುಗಿ *urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_ನೇರ(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_string(ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ, *dot;
	ಪೂರ್ಣಾಂಕ plen, ret;

	ಪ್ರತಿಜ್ಞಾ(a && a != &db_ಬೇರು && b);

	ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(b)) {
		/* IP address */
		ಹಿಂತಿರುಗಿ strcasecmp(a->part, b);
	}

	ಕೊನೆ = b + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b) + 1;

	ಆಗ (b < ಕೊನೆ && a && a != &db_ಬೇರು) {
		dot = strchr(b, '.');
		ಆದರೆ (!dot) {
			/* last segment */
			dot = ಕೊನೆ - 1;
		}

		/* Compare strings (length limited) */
		ಆದರೆ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ಹಿಂತಿರುಗಿ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(a->part);

		ಆದರೆ (plen > dot - b) {
			/* len(a) > len(b) */
			ಹಿಂತಿರುಗಿ 1;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (plen < dot - b) {
			/* len(a) < len(b) */
			ಹಿಂತಿರುಗಿ -1;
		}

		b = dot + 1;
		a = a->ಪೋಷಕ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಆದರೆ (a && a != &db_ಬೇರು && b >= ಕೊನೆ) {
		/* len(a) > len(b) */
		ಹಿಂತಿರುಗಿ 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((!a || a == &db_ಬೇರು) && b < ಕೊನೆ) {
		/* len(a) < len(b) */
		ಹಿಂತಿರುಗಿ -1;
	}

	/* Identical */
	ಹಿಂತಿರುಗಿ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *
urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host)
{
	ಪೂರ್ಣಾಂಕ c;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && host);

	ಆದರೆ (ಬೇರು == &ಖಾಲಿಗಳು) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	c = urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_string(ಬೇರು->ದತ್ತಾಂಶ, host);

	ಆದರೆ (c > 0) {
		ಹಿಂತಿರುಗಿ urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(ಬೇರು->ಎಡ, host);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c < 0) {
		ಹಿಂತಿರುಗಿ urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(ಬೇರು->ಬಲ, host);
	}

	ಹಿಂತಿರುಗಿ ಬೇರು->ದತ್ತಾಂಶ;
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
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *
urldb_ಹೊಂದಿಕೆ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
		 lwc_string *scheme,
		 ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಪೋರ್ಟ್)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ->segment == ಖಾಲಿ);

	ಆದರೆ (ಮಾರ್ಗ[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", ಮಾರ್ಗ);
	}

	ಪ್ರತಿಜ್ಞಾ(ಮಾರ್ಗ[0] == '/');

	/* Start with children, as parent has no segment */
	p = ಪೋಷಕ->children;

	ಆಗ (p != ಖಾಲಿ) {
		slash = strchr(ಮಾರ್ಗ + 1, '/');
		ಆದರೆ (!slash) {
			slash = ಮಾರ್ಗ + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ);
		}

		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p->segment, ಮಾರ್ಗ + 1, slash - ಮಾರ್ಗ - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &ಹೊಂದಿಕೆ) == lwc_error_ok &&
		    ಹೊಂದಿಕೆ == true &&
		    p->ಪೋರ್ಟ್ == ಪೋರ್ಟ್) {
			ಆದರೆ (*slash == '\0') {
				/* Complete match */
				ಹಿಂತಿರುಗಿ (ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			ಮಾರ್ಗ = slash;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* No match, try next sibling */
			p = p->ಮುಂದಿನ;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *urldb_ಕಂಡುಹಿಡಿ_url(nsurl *url)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ರಚನೆ ಹುಡುಕು_ಗಂಟು *ವೃಕ್ಷ;
	ಅಕ್ಷರ *plq;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host_str;
	lwc_string *scheme, *host, *ಪೋರ್ಟ್;
	size_t len = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್_int;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(url);

	ಆದರೆ (url_bloom != ಖಾಲಿ) {
		ಆದರೆ (bloom_ಹುಡುಕು_hash(url_bloom, nsurl_hash(url)) == false) {
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಆದರೆ (scheme == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (lwc_string_isequal(scheme, corestring_lwc_mailto, &ಹೊಂದಿಕೆ) ==
	    lwc_error_ok && ಹೊಂದಿಕೆ == true) {
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (host != ಖಾಲಿ) {
		host_str = lwc_string_ದತ್ತಾಂಶ(host);
		lwc_string_unref(host);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lwc_string_isequal(scheme, corestring_lwc_file, &ಹೊಂದಿಕೆ) ==
		   lwc_error_ok && ಹೊಂದಿಕೆ == true) {
		host_str = "localhost";

	} ಇಲ್ಲದಿದ್ದರೆ {
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ವೃಕ್ಷ = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(host_str);
	h = urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(ವೃಕ್ಷ, host_str);
	ಆದರೆ (!h) {
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* generate plq (path, leaf, query) */
	ಆದರೆ (nsurl_get(url, NSURL_ಮಾರ್ಗ | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Get port */
	ಪೋರ್ಟ್ = nsurl_get_component(url, NSURL_ಪೋರ್ಟ್);
	ಆದರೆ (ಪೋರ್ಟ್ != ಖಾಲಿ) {
		ಪೋರ್ಟ್_int = atoi(lwc_string_ದತ್ತಾಂಶ(ಪೋರ್ಟ್));
		lwc_string_unref(ಪೋರ್ಟ್);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೋರ್ಟ್_int = 0;
	}

	p = urldb_ಹೊಂದಿಕೆ_ಮಾರ್ಗ(&h->paths, plq, scheme, ಪೋರ್ಟ್_int);

	ಬಿಡುಗಡೆ(plq);
	lwc_string_unref(scheme);

	ಹಿಂತಿರುಗಿ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_paths(ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಮಾಡು {
		ಆದರೆ (p->segment != ಖಾಲಿ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_ದತ್ತಾಂಶ(p->scheme), p->ಪೋರ್ಟ್);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			ಗಾಗಿ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ಆದರೆ (p->children != ಖಾಲಿ) {
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆಗ (p != ಪೋಷಕ) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಪೋಷಕ);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_hosts(ರಚನೆ host_part *ಪೋಷಕ)
{
	ರಚನೆ host_part *h;

	ಆದರೆ (ಪೋಷಕ->part) {
		NSLOG(netsurf, INFO, "%s", ಪೋಷಕ->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      ಪೋಷಕ->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&ಪೋಷಕ->paths);

	/* and recurse */
	ಗಾಗಿ (h = ಪೋಷಕ->children; h; h = h->ಮುಂದಿನ) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_ಹುಡುಕು(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಪೋಷಕ, ಪೂರ್ಣಾಂಕ ಆಳ)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	ಪೂರ್ಣಾಂಕ i; /* index into string */
	ಅಕ್ಷರ s[1024];
	ಪೂರ್ಣಾಂಕ r;
	ಪೂರ್ಣಾಂಕ sl = ಗಾತ್ರ(s) - 2;

	ಆದರೆ (ಪೋಷಕ == &ಖಾಲಿಗಳು)
		ಹಿಂತಿರುಗಿ;

	urldb_dump_ಹುಡುಕು(ಪೋಷಕ->ಎಡ, ಆಳ + 1);

	ಗಾಗಿ (i = 0; i != ಆಳ; i++) {
		s[i] = ' ';
	}

	ಗಾಗಿ (h = ಪೋಷಕ->ದತ್ತಾಂಶ; h; h = h->ಪೋಷಕ) {
		ಆದರೆ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ಆದರೆ (r < 0) {
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ ((i + r) >= sl) {
				ನಿಲ್ಲಿಸು;
			}
			i += r;
		}

		ಆದರೆ (h->ಪೋಷಕ && h->ಪೋಷಕ->ಪೋಷಕ) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_ಹುಡುಕು(ಪೋಷಕ->ಬಲ, ಆಳ + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_host(ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *a, ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *b)
{
	ಪೂರ್ಣಾಂಕ ret;

	ಪ್ರತಿಜ್ಞಾ(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	ಗಾಗಿ (; a && a != &db_ಬೇರು && b && b != &db_ಬೇರು;
	     a = a->ಪೋಷಕ, b = b->ಪೋಷಕ) {
		ಆದರೆ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ಹಿಂತಿರುಗಿ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಆದರೆ (a && a != &db_ಬೇರು && (!b || b == &db_ಬೇರು)) {
		/* len(a) > len(b) */
		ಹಿಂತಿರುಗಿ 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((!a || a == &db_ಬೇರು) && b && b != &db_ಬೇರು) {
		/* len(a) < len(b) */
		ಹಿಂತಿರುಗಿ -1;
	}

	/* identical */
	ಹಿಂತಿರುಗಿ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *urldb_ಹುಡುಕು_skew(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	ಆದರೆ (ಬೇರು->ಎಡ->level == ಬೇರು->level) {
		ರಚನೆ ಹುಡುಕು_ಗಂಟು *ತಾತ್ಕಾಲಿಕ;

		ತಾತ್ಕಾಲಿಕ = ಬೇರು->ಎಡ;
		ಬೇರು->ಎಡ = ತಾತ್ಕಾಲಿಕ->ಬಲ;
		ತಾತ್ಕಾಲಿಕ->ಬಲ = ಬೇರು;
		ಬೇರು = ತಾತ್ಕಾಲಿಕ;
	}

	ಹಿಂತಿರುಗಿ ಬೇರು;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *urldb_ಹುಡುಕು_ವಿಭಜಿಸು(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	ಆದರೆ (ಬೇರು->ಬಲ->ಬಲ->level == ಬೇರು->level) {
		ರಚನೆ ಹುಡುಕು_ಗಂಟು *ತಾತ್ಕಾಲಿಕ;

		ತಾತ್ಕಾಲಿಕ = ಬೇರು->ಬಲ;
		ಬೇರು->ಬಲ = ತಾತ್ಕಾಲಿಕ->ಎಡ;
		ತಾತ್ಕಾಲಿಕ->ಎಡ = ಬೇರು;
		ಬೇರು = ತಾತ್ಕಾಲಿಕ;

		ಬೇರು->level++;
	}

	ಹಿಂತಿರುಗಿ ಬೇರು;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *
urldb_ಹುಡುಕು_ಸೇರಿಸು_ಆಂತರಿಕ(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ರಚನೆ ಹುಡುಕು_ಗಂಟು *n)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && n);

	ಆದರೆ (ಬೇರು == &ಖಾಲಿಗಳು) {
		ಬೇರು = n;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ c = urldb_ಹುಡುಕು_ಹೊಂದಿಕೆ_host(ಬೇರು->ದತ್ತಾಂಶ, n->ದತ್ತಾಂಶ);

		ಆದರೆ (c > 0) {
			ಬೇರು->ಎಡ = urldb_ಹುಡುಕು_ಸೇರಿಸು_ಆಂತರಿಕ(
				ಬೇರು->ಎಡ, n);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c < 0) {
			ಬೇರು->ಬಲ = urldb_ಹುಡುಕು_ಸೇರಿಸು_ಆಂತರಿಕ(
				ಬೇರು->ಬಲ, n);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* exact match */
			ಬಿಡುಗಡೆ(n);
			ಹಿಂತಿರುಗಿ ಬೇರು;
		}

		ಬೇರು = urldb_ಹುಡುಕು_skew(ಬೇರು);
		ಬೇರು = urldb_ಹುಡುಕು_ವಿಭಜಿಸು(ಬೇರು);
	}

	ಹಿಂತಿರುಗಿ ಬೇರು;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ಸ್ಥಿರ ರಚನೆ ಹುಡುಕು_ಗಂಟು *
urldb_ಹುಡುಕು_ಸೇರಿಸು(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *ದತ್ತಾಂಶ)
{
	ರಚನೆ ಹುಡುಕು_ಗಂಟು *n;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && ದತ್ತಾಂಶ);

	n = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ ಹುಡುಕು_ಗಂಟು));
	ಆದರೆ (!n)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	n->level = 1;
	n->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
	n->ಎಡ = n->ಬಲ = &ಖಾಲಿಗಳು;

	ಬೇರು = urldb_ಹುಡುಕು_ಸೇರಿಸು_ಆಂತರಿಕ(ಬೇರು, n);

	ಹಿಂತಿರುಗಿ ಬೇರು;
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
ಸ್ಥಿರ bool
urldb_parse_avpair(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c,
		   ಅಕ್ಷರ *n,
		   ಅಕ್ಷರ *v,
		   bool was_quoted)
{
	ಪೂರ್ಣಾಂಕ vlen;

	ಪ್ರತಿಜ್ಞಾ(c && n && v);

	/* Strip whitespace from start of name */
	ಗಾಗಿ (; *n; n++) {
		ಆದರೆ (*n != ' ' && *n != '\t')
			ನಿಲ್ಲಿಸು;
	}

	/* Strip whitespace from end of name */
	ಗಾಗಿ (vlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(n); vlen; vlen--) {
		ಆದರೆ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ಇಲ್ಲದಿದ್ದರೆ
			ನಿಲ್ಲಿಸು;
	}

	/* Strip whitespace from start of value */
	ಗಾಗಿ (; *v; v++) {
		ಆದರೆ (*v != ' ' && *v != '\t')
			ನಿಲ್ಲಿಸು;
	}

	/* Strip whitespace from end of value */
	ಗಾಗಿ (vlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(v); vlen; vlen--) {
		ಆದರೆ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ಇಲ್ಲದಿದ್ದರೆ
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ಆದರೆ (!c->comment)
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ಆದರೆ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_ಗಣ = true;
			c->domain = strdup(v);
			ಆದರೆ (!c->domain)
				ಹಿಂತಿರುಗಿ false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(n, "Max-Age") == 0) {
		ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ = atoi(v);
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ಇಲ್ಲದಿದ್ದರೆ
			c->expires = time(ಖಾಲಿ) + ತಾತ್ಕಾಲಿಕ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!c->ಮಾರ್ಗ && strcasecmp(n, "Path") == 0) {
		c->ಮಾರ್ಗ_from_ಗಣ = true;
		c->ಮಾರ್ಗ = strdup(v);
		ಆದರೆ (!c->ಮಾರ್ಗ)
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(n, "Version") == 0) {
		c->ಆವೃತ್ತಿ = atoi(v);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(n, "Expires") == 0) {
		ಅಕ್ಷರ *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		ಗಾಗಿ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(datenoday), &expires);
		ಆದರೆ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!c->name) {
		c->name = strdup(n);
		c->ಮೌಲ್ಯ = strdup(v);
		c->ಮೌಲ್ಯ_was_quoted = was_quoted;
		ಆದರೆ (!c->name || !c->ಮೌಲ್ಯ) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_free_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c)
{
	ಪ್ರತಿಜ್ಞಾ(c);

	ಬಿಡುಗಡೆ(c->comment);
	ಬಿಡುಗಡೆ(c->domain);
	ಬಿಡುಗಡೆ(c->ಮಾರ್ಗ);
	ಬಿಡುಗಡೆ(c->name);
	ಬಿಡುಗಡೆ(c->ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ಸ್ಥಿರ ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *
urldb_parse_cookie(nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **cookie)
{
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cur;
	ಅಕ್ಷರ name[1024], ಮೌಲ್ಯ[4096];
	ಅಕ್ಷರ *n = name, *v = ಮೌಲ್ಯ;
	bool ಒಳ_ಮೌಲ್ಯ = false;
	bool had_ಮೌಲ್ಯ_ದತ್ತಾಂಶ = false;
	bool ಮೌಲ್ಯ_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ಪ್ರತಿಜ್ಞಾ(url && cookie && *cookie);

	c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ));
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	c->expires = -1;

	name[0] = '\0';
	ಮೌಲ್ಯ[0] = '\0';

	ಗಾಗಿ (cur = *cookie; *cur; cur++) {
		ಆದರೆ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ಆದರೆ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				ಮೌಲ್ಯ[0] = '\0';

			}

			ನಿಲ್ಲಿಸು;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*cur == '\r') {
			/* Spurious linefeed */
			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*cur == '\n') {
			/* Spurious newline */
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಒಳ_ಮೌಲ್ಯ && !had_ಮೌಲ್ಯ_ದತ್ತಾಂಶ) {
			ಆದರೆ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				had_ಮೌಲ್ಯ_ದತ್ತಾಂಶ = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ಆದರೆ (*cur != '"') {
					ಮೌಲ್ಯ_verbatim = true;
				}
			}
		}

		ಆದರೆ (ಒಳ_ಮೌಲ್ಯ && !ಮೌಲ್ಯ_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ಆದರೆ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ಮುಂದುವರಿ;
			}
		}

		ಆದರೆ (!quoted && !ಒಳ_ಮೌಲ್ಯ && *cur == '=') {
			/* First equals => attr-value separator */
			ಒಳ_ಮೌಲ್ಯ = true;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ಆದರೆ (!urldb_parse_avpair(c, name, ಮೌಲ್ಯ, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}

			/* And reset to start */
			n = name;
			v = ಮೌಲ್ಯ;
			ಒಳ_ಮೌಲ್ಯ = false;
			had_ಮೌಲ್ಯ_ದತ್ತಾಂಶ = false;
			ಮೌಲ್ಯ_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ಆದರೆ (*cur != ';') {
				cur--;
			}

			ಮುಂದುವರಿ;
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
		ಆದರೆ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *semi = strchr(cur + 1, ';');
			ಆದರೆ (!semi)
				semi = cur + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			ಗಾಗಿ (p = cur + 1; p < semi; p++)
				ಆದರೆ (*p == '=')
					ನಿಲ್ಲಿಸು;

			ಆದರೆ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* found one => comma marks end of cookie */
				cur++;
				ನಿಲ್ಲಿಸು;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ಆದರೆ (!ಒಳ_ಮೌಲ್ಯ) {
			ಆದರೆ (n < name + (ಗಾತ್ರ(name) - 1))
				*n++ = *cur;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (v < ಮೌಲ್ಯ + (ಗಾತ್ರ(ಮೌಲ್ಯ) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ಆದರೆ (!urldb_parse_avpair(c, name, ಮೌಲ್ಯ, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Now fix-up default values */
	ಆದರೆ (c->domain == ಖಾಲಿ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ಆದರೆ (host == ಖಾಲಿ) {
			urldb_free_cookie(c);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		c->domain = strdup(lwc_string_ದತ್ತಾಂಶ(host));
		lwc_string_unref(host);
	}

	ಆದರೆ (c->ಮಾರ್ಗ == ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ_ದತ್ತಾಂಶ;
		ಅಕ್ಷರ *ಮಾರ್ಗ, *slash;
		lwc_string *ಮಾರ್ಗ_lwc;

		ಮಾರ್ಗ_lwc = nsurl_get_component(url, NSURL_ಮಾರ್ಗ);
		ಆದರೆ (ಮಾರ್ಗ_lwc == ಖಾಲಿ) {
			urldb_free_cookie(c);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		ಮಾರ್ಗ_ದತ್ತಾಂಶ = lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(ಮಾರ್ಗ_ದತ್ತಾಂಶ, '/');
		ಆದರೆ (slash != ಖಾಲಿ) {
			/* Special case: retain first slash in path */
			ಆದರೆ (slash == ಮಾರ್ಗ_ದತ್ತಾಂಶ)
				slash++;

			slash = strndup(ಮಾರ್ಗ_ದತ್ತಾಂಶ, slash - ಮಾರ್ಗ_ದತ್ತಾಂಶ);
			ಆದರೆ (slash == ಖಾಲಿ) {
				lwc_string_unref(ಮಾರ್ಗ_lwc);
				urldb_free_cookie(c);
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}

			ಮಾರ್ಗ = slash;
			lwc_string_unref(ಮಾರ್ಗ_lwc);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಮಾರ್ಗ = strdup(lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ_lwc));
			lwc_string_unref(ಮಾರ್ಗ_lwc);
			ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ) {
				urldb_free_cookie(c);
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}
		}

		c->ಮಾರ್ಗ = ಮಾರ್ಗ;
	}

	/* Write back current position */
	*cookie = cur;

	ಹಿಂತಿರುಗಿ c;
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
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *
urldb_ಕೂಡಿಸು_ಮಾರ್ಗ(lwc_string *scheme,
	       ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್,
	       ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *host,
	       ಅಕ್ಷರ *ಮಾರ್ಗ_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *d, *e;
	ಅಕ್ಷರ *buf = ಮಾರ್ಗ_query;
	ಅಕ್ಷರ *segment, *slash;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(scheme && host && url);

	d = (ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ಆದರೆ (*segment == '/')
		segment++;

	/* Process path segments */
	ಮಾಡು {
		slash = strchr(segment, '/');
		ಆದರೆ (!slash) {
			/* last segment */
			/* look for existing entry */
			ಗಾಗಿ (e = d->children; e; e = e->ಮುಂದಿನ)
				ಆದರೆ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &ಹೊಂದಿಕೆ) ==
				    lwc_error_ok &&
				    ಹೊಂದಿಕೆ == true &&
				    e->ಪೋರ್ಟ್ == ಪೋರ್ಟ್)
					ನಿಲ್ಲಿಸು;

			d = e ? urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_fragment(e, fragment) :
				urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_ಗಂಟು(scheme, ಪೋರ್ಟ್,
						    segment, fragment, d);
			ನಿಲ್ಲಿಸು;
		}

		*slash = '\0';

		/* look for existing entry */
		ಗಾಗಿ (e = d->children; e; e = e->ಮುಂದಿನ)
			ಆದರೆ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &ಹೊಂದಿಕೆ) == lwc_error_ok &&
			    ಹೊಂದಿಕೆ == true &&
			    e->ಪೋರ್ಟ್ == ಪೋರ್ಟ್)
				ನಿಲ್ಲಿಸು;

		d = e ? e : urldb_ಕೂಡಿಸು_ಮಾರ್ಗ_ಗಂಟು(scheme, ಪೋರ್ಟ್, segment, ಖಾಲಿ, d);
		ಆದರೆ (!d)
			ನಿಲ್ಲಿಸು;

		segment = slash + 1;
	} ಆಗ (1);

	ಬಿಡುಗಡೆ(ಮಾರ್ಗ_query);

	ಆದರೆ (d && !d->url) {
		/* Insert defragmented URL */
		ಆದರೆ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ರಚನೆ host_part *urldb_ಕೂಡಿಸು_host(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host)
{
	ರಚನೆ host_part *d = (ರಚನೆ host_part *) &db_ಬೇರು, *e;
	ರಚನೆ ಹುಡುಕು_ಗಂಟು *s;
	ಅಕ್ಷರ buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	ಅಕ್ಷರ *part;

	ಪ್ರತಿಜ್ಞಾ(host);

	ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		ಗಾಗಿ (e = d->children; e; e = e->ಮುಂದಿನ)
			ಆದರೆ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ಹಿಂತಿರುಗಿ e;

		d = urldb_ಕೂಡಿಸು_host_ಗಂಟು(host, d);

		s = urldb_ಹುಡುಕು_ಸೇರಿಸು(ಹುಡುಕು_trees[ST_IP], d);
		ಆದರೆ (!s) {
			/* failed */
			d = ಖಾಲಿ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹುಡುಕು_trees[ST_IP] = s;
		}

		ಹಿಂತಿರುಗಿ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, ಗಾತ್ರ buf);
	buf[ಗಾತ್ರ buf - 1] = '\0';

	/* Process FQDN segments backwards */
	ಮಾಡು {
		part = strrchr(buf, '.');
		ಆದರೆ (!part) {
			/* last segment */
			/* Check for existing entry */
			ಗಾಗಿ (e = d->children; e; e = e->ಮುಂದಿನ)
				ಆದರೆ (strcasecmp(buf, e->part) == 0)
					ನಿಲ್ಲಿಸು;

			ಆದರೆ (e) {
				d = e;
			} ಇಲ್ಲದಿದ್ದರೆ {
				d = urldb_ಕೂಡಿಸು_host_ಗಂಟು(buf, d);
			}

			/* And insert into search tree */
			ಆದರೆ (d) {
				ರಚನೆ ಹುಡುಕು_ಗಂಟು **r;

				r = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_ನೇರ(buf);
				s = urldb_ಹುಡುಕು_ಸೇರಿಸು(*r, d);
				ಆದರೆ (!s) {
					/* failed */
					d = ಖಾಲಿ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					*r = s;
				}
			}
			ನಿಲ್ಲಿಸು;
		}

		/* Check for existing entry */
		ಗಾಗಿ (e = d->children; e; e = e->ಮುಂದಿನ)
			ಆದರೆ (strcasecmp(part + 1, e->part) == 0)
				ನಿಲ್ಲಿಸು;

		d = e ? e : urldb_ಕೂಡಿಸು_host_ಗಂಟು(part + 1, d);
		ಆದರೆ (!d)
			ನಿಲ್ಲಿಸು;

		*part = '\0';
	} ಆಗ (1);

	ಹಿಂತಿರುಗಿ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ಸ್ಥಿರ bool
urldb_ಸೇರಿಸು_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *d;
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	time_t now = time(ಖಾಲಿ);

	ಪ್ರತಿಜ್ಞಾ(c);

	ಆದರೆ (c->domain[0] == '.') {
		h = urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(
			urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(&(c->domain[1])),
			c->domain + 1);
		ಆದರೆ (!h) {
			h = urldb_ಕೂಡಿಸು_host(c->domain + 1);
			ಆದರೆ (!h) {
				urldb_free_cookie(c);
				ಹಿಂತಿರುಗಿ false;
			}
		}

		p = (ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *) &h->paths;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ಪ್ರತಿಜ್ಞಾ(url != ಖಾಲಿ);
		ಪ್ರತಿಜ್ಞಾ(scheme != ಖಾಲಿ);

		h = urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(
			urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(c->domain),
			c->domain);

		ಆದರೆ (!h) {
			h = urldb_ಕೂಡಿಸು_host(c->domain);
			ಆದರೆ (!h) {
				urldb_free_cookie(c);
				ಹಿಂತಿರುಗಿ false;
			}
		}

		/* find path */
		p = urldb_ಕೂಡಿಸು_ಮಾರ್ಗ(scheme, 0, h,
				   strdup(c->ಮಾರ್ಗ), ಖಾಲಿ, url);
		ಆದರೆ (!p) {
			urldb_free_cookie(c);
			ಹಿಂತಿರುಗಿ false;
		}
	}

	/* add cookie */
	ಗಾಗಿ (d = p->cookies; d; d = d->ಮುಂದಿನ) {
		ಆದರೆ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->ಮಾರ್ಗ, c->ಮಾರ್ಗ) &&
		    !strcmp(d->name, c->name))
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (d) {
		ಆದರೆ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ಆದರೆ (d->ಮುಂದಿನ)
				d->ಮುಂದಿನ->prev = d->prev;
			ಇಲ್ಲದಿದ್ದರೆ
				p->cookies_ಕೊನೆ = d->prev;
			ಆದರೆ (d->prev)
				d->prev->ಮುಂದಿನ = d->ಮುಂದಿನ;
			ಇಲ್ಲದಿದ್ದರೆ
				p->cookies = d->ಮುಂದಿನ;

			cookie_manager_remove((ರಚನೆ cookie_ದತ್ತಾಂಶ *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* replace d with c */
			c->prev = d->prev;
			c->ಮುಂದಿನ = d->ಮುಂದಿನ;
			ಆದರೆ (c->ಮುಂದಿನ)
				c->ಮುಂದಿನ->prev = c;
			ಇಲ್ಲದಿದ್ದರೆ
				p->cookies_ಕೊನೆ = c;
			ಆದರೆ (c->prev)
				c->prev->ಮುಂದಿನ = c;
			ಇಲ್ಲದಿದ್ದರೆ
				p->cookies = c;

			cookie_manager_remove((ರಚನೆ cookie_ದತ್ತಾಂಶ *)d);
			urldb_free_cookie(d);

			cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		c->prev = p->cookies_ಕೊನೆ;
		c->ಮುಂದಿನ = ಖಾಲಿ;
		ಆದರೆ (p->cookies_ಕೊನೆ)
			p->cookies_ಕೊನೆ->ಮುಂದಿನ = c;
		ಇಲ್ಲದಿದ್ದರೆ
			p->cookies = c;
		p->cookies_ಕೊನೆ = c;

		cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
	}

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
concat_netscape_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c,
		    ಪೂರ್ಣಾಂಕ *ಬಳಸಿದ,
		    ಪೂರ್ಣಾಂಕ *alloc,
		    ಅಕ್ಷರ **buf)
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
	ಪೂರ್ಣಾಂಕ max_len;
	ಪೂರ್ಣಾಂಕ outl; /* output length */

	ಪ್ರತಿಜ್ಞಾ(c && ಬಳಸಿದ && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *ಬಳಸಿದ + 2 +
		ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->name) + 1 +
		ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮೌಲ್ಯ) + 2 +
		(c->ಮಾರ್ಗ_from_ಗಣ ? 8 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮಾರ್ಗ) + 2 : 0) +
		(c->domain_from_ಗಣ ? 10 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->domain) + 2 : 0);
	ಆದರೆ (max_len >= *alloc) {
		ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ = realloc(*buf, max_len);
		ಆದರೆ (!ತಾತ್ಕಾಲಿಕ) {
			ಹಿಂತಿರುಗಿ false;
		}
		*buf = ತಾತ್ಕಾಲಿಕ;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *ಬಳಸಿದ - 1, *alloc - *ಬಳಸಿದ, "; %s=", c->name);
	ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
		/* output truncated */
		ಹಿಂತಿರುಗಿ false;
	}
	*ಬಳಸಿದ += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ಆದರೆ (c->ಮೌಲ್ಯ_was_quoted) {
		outl = snprintf(*buf + *ಬಳಸಿದ - 1,
				*alloc - *ಬಳಸಿದ,
				"\"%s\"", c->ಮೌಲ್ಯ);
		ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
			/* output truncated */
			ಹಿಂತಿರುಗಿ false;
		}
		*ಬಳಸಿದ += outl;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *ಬಳಸಿದ - 1,*alloc - *ಬಳಸಿದ, "%s", c->ಮೌಲ್ಯ);
		ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
			/* output truncated */
			ಹಿಂತಿರುಗಿ false;
		}
		*ಬಳಸಿದ += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
concat_rfc2965_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c,
		      ಪೂರ್ಣಾಂಕ *ಬಳಸಿದ,
		      ಪೂರ್ಣಾಂಕ *alloc,
		      ಅಕ್ಷರ **buf)
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
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ separators = "()<>@,;:\\\"/[]?={} \t";
	ಪೂರ್ಣಾಂಕ max_len;
	ಪೂರ್ಣಾಂಕ outl; /* output length */

	ಪ್ರತಿಜ್ಞಾ(c && ಬಳಸಿದ && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *ಬಳಸಿದ + 2 +
		ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->name) + 1 +
		ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮೌಲ್ಯ) + 2 +
		(c->ಮಾರ್ಗ_from_ಗಣ ?
		 8 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮಾರ್ಗ) + 2 : 0) +
		(c->domain_from_ಗಣ ?
		 10 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->domain) + 2 : 0);

	ಆದರೆ (max_len >= *alloc) {
		ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ = realloc(*buf, max_len);
		ಆದರೆ (!ತಾತ್ಕಾಲಿಕ) {
			ಹಿಂತಿರುಗಿ false;
		}
		*buf = ತಾತ್ಕಾಲಿಕ;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *ಬಳಸಿದ - 1,
			*alloc - *ಬಳಸಿದ,
			"; %s=", c->name);
	ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
		/* output truncated */
		ಹಿಂತಿರುಗಿ false;
	}
	*ಬಳಸಿದ += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ಆದರೆ (c->ಮೌಲ್ಯ_was_quoted ||
	    strpbrk(c->ಮೌಲ್ಯ, separators) != ಖಾಲಿ) {
		outl = snprintf(*buf + *ಬಳಸಿದ - 1,
				*alloc - *ಬಳಸಿದ,
				"\"%s\"", c->ಮೌಲ್ಯ);

	} ಇಲ್ಲದಿದ್ದರೆ {
		outl = snprintf(*buf + *ಬಳಸಿದ - 1,
				*alloc - *ಬಳಸಿದ,
				"%s", c->ಮೌಲ್ಯ);
	}
	ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
		/* output truncated */
		ಹಿಂತಿರುಗಿ false;
	}
	*ಬಳಸಿದ += outl;

	ಆದರೆ (c->ಮಾರ್ಗ_from_ಗಣ) {
		/* Path, quoted if necessary */

		ಆದರೆ (strpbrk(c->ಮಾರ್ಗ, separators) != ಖಾಲಿ) {
			outl = snprintf(*buf + *ಬಳಸಿದ - 1,
					*alloc - *ಬಳಸಿದ,
					"; $Path=\"%s\"", c->ಮಾರ್ಗ);

		} ಇಲ್ಲದಿದ್ದರೆ {
			outl = snprintf(*buf + *ಬಳಸಿದ - 1,
				       *alloc - *ಬಳಸಿದ,
				       "; $Path=%s", c->ಮಾರ್ಗ);
		}
		ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
			/* output truncated */
			ಹಿಂತಿರುಗಿ false;
		}
		*ಬಳಸಿದ += outl;
	}

	ಆದರೆ (c->domain_from_ಗಣ) {
		/* Domain, quoted if necessary */
		ಆದರೆ (strpbrk(c->domain, separators) != ಖಾಲಿ) {
			outl = snprintf(*buf + *ಬಳಸಿದ - 1,
					*alloc - *ಬಳಸಿದ,
					"; $Domain=\"%s\"", c->domain);
		} ಇಲ್ಲದಿದ್ದರೆ {
			outl = snprintf(*buf + *ಬಳಸಿದ - 1,
					*alloc - *ಬಳಸಿದ,
					"; $Domain=%s", c->domain);
		}
		ಆದರೆ (outl >= (*alloc - *ಬಳಸಿದ)) {
			/* output truncated */
			ಹಿಂತಿರುಗಿ false;
		}
		*ಬಳಸಿದ += outl;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * build output cookie string in netscape format
 */
ಸ್ಥಿರ ಅಕ್ಷರ *
get_netscape_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ **matched_cookies, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಪೂರ್ಣಾಂಕ ret_alloc = 4096, ret_ಬಳಸಿದ = 1;
	ಅಕ್ಷರ *ret;
	ಪೂರ್ಣಾಂಕ midx; /* index into matched cookies */

	ret = ಸ್ಮರಣೆ_ಕೊಡು(ret_alloc);
	ಆದರೆ (ret == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ret[0] = '\0';

	ಗಾಗಿ (midx = 0; midx < ಎಣಿಕೆ_2; midx++) {
		ಆದರೆ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_ಬಳಸಿದ, &ret_alloc, &ret)) {
			ಬಿಡುಗಡೆ(ret);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	ಆದರೆ (ret_ಬಳಸಿದ <= 2) {
		ಬಿಡುಗಡೆ(ret);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_ಬಳಸಿದ - 2);
	ret_ಬಳಸಿದ -= 2;

	/* shrink the output buffer to the required size */
	ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ = realloc(ret, ret_ಬಳಸಿದ);
	ಆದರೆ (ತಾತ್ಕಾಲಿಕ != ಖಾಲಿ) {
		ret = ತಾತ್ಕಾಲಿಕ;
	}

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ಸ್ಥಿರ ಅಕ್ಷರ *
get_rfc2965_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ **matched_cookies,
		   ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2,
		   ಪೂರ್ಣಾಂಕ ಆವೃತ್ತಿ)
{
	ಪೂರ್ಣಾಂಕ ret_alloc = 4096; /* return string allocation */
	ಪೂರ್ಣಾಂಕ ret_ಬಳಸಿದ = 1; /* return sting buffer used */
	ಅಕ್ಷರ *ret; /* return string buffer */
	ಪೂರ್ಣಾಂಕ midx; /* index into matched cookies */
	ಪೂರ್ಣಾಂಕ outl;

	ret = ಸ್ಮರಣೆ_ಕೊಡು(ret_alloc);
	ಆದರೆ (ret == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_ಬಳಸಿದ, "$Version=%d", ಆವೃತ್ತಿ);
	ಆದರೆ (outl >= ret_alloc - ret_ಬಳಸಿದ) {
		/*ಅತಿಪ್ರವಾಹ*/
		ಬಿಡುಗಡೆ(ret);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ret_ಬಳಸಿದ += outl;

	ಗಾಗಿ (midx = 0; midx < ಎಣಿಕೆ_2; midx++) {
		ಆದರೆ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_ಬಳಸಿದ, &ret_alloc, &ret)) {
			ಬಿಡುಗಡೆ(ret);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	/* shrink the output buffer to the required size */
	ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ = realloc(ret, ret_ಬಳಸಿದ);
	ಆದರೆ (ತಾತ್ಕಾಲಿಕ != ಖಾಲಿ) {
		ret = ತಾತ್ಕಾಲಿಕ;
	}

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ಸ್ಥಿರ ಶೂನ್ಯ
urldb_delete_cookie_paths(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *domain,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			  ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ)
{
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ;

	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ);

	ಮಾಡು {
		ಗಾಗಿ (c = p->cookies; c; c = c->ಮುಂದಿನ) {
			ಆದರೆ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->ಮಾರ್ಗ, ಮಾರ್ಗ) == 0 &&
			    strcmp(c->name, name) == 0) {
				ಆದರೆ (c->prev) {
					c->prev->ಮುಂದಿನ = c->ಮುಂದಿನ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					p->cookies = c->ಮುಂದಿನ;
				}

				ಆದರೆ (c->ಮುಂದಿನ) {
					c->ಮುಂದಿನ->prev = c->prev;
				} ಇಲ್ಲದಿದ್ದರೆ {
					p->cookies_ಕೊನೆ = c->prev;
				}

				urldb_free_cookie(c);

				ಹಿಂತಿರುಗಿ;
			}
		}

		ಆದರೆ (p->children) {
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆಗ (p != ಪೋಷಕ) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಪೋಷಕ);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ಸ್ಥಿರ ಶೂನ್ಯ
urldb_delete_cookie_hosts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *domain,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			  ರಚನೆ host_part *ಪೋಷಕ)
{
	ರಚನೆ host_part *h;
	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ);

	urldb_delete_cookie_paths(domain, ಮಾರ್ಗ, name, &ಪೋಷಕ->paths);

	ಗಾಗಿ (h = ಪೋಷಕ->children; h; h = h->ಮುಂದಿನ) {
		urldb_delete_cookie_hosts(domain, ಮಾರ್ಗ, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_cookie_paths(FILE *fp, ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಪೋಷಕ)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಪೋಷಕ;
	time_t now = time(ಖಾಲಿ);

	ಪ್ರತಿಜ್ಞಾ(fp && ಪೋಷಕ);

	ಮಾಡು {
		ಆದರೆ (p->cookies != ಖಾಲಿ) {
			ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;

			ಗಾಗಿ (c = p->cookies; c != ಖಾಲಿ; c = c->ಮುಂದಿನ) {
				ಆದರೆ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ಮುಂದುವರಿ;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->ಆವೃತ್ತಿ, c->domain,
					c->domain_from_ಗಣ, c->ಮಾರ್ಗ,
					c->ಮಾರ್ಗ_from_ಗಣ, c->secure,
					c->http_only,
					(ಪೂರ್ಣಾಂಕ)c->expires, (ಪೂರ್ಣಾಂಕ)c->ಕೊನೆಯ_ಬಳಸಿದ,
					c->no_destroy, c->name, c->ಮೌಲ್ಯ,
					c->ಮೌಲ್ಯ_was_quoted,
					p->scheme ? lwc_string_ದತ್ತಾಂಶ(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ಆದರೆ (p->children != ಖಾಲಿ) {
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆಗ (p != ಪೋಷಕ) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				p = p->ಪೋಷಕ;
			}
		}
	} ಆಗ (p != ಪೋಷಕ);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_cookie_hosts(FILE *fp, ರಚನೆ host_part *ಪೋಷಕ)
{
	ರಚನೆ host_part *h;
	ಪ್ರತಿಜ್ಞಾ(fp && ಪೋಷಕ);

	urldb_save_cookie_paths(fp, &ಪೋಷಕ->paths);

	ಗಾಗಿ (h = ಪೋಷಕ->children; h; h = h->ಮುಂದಿನ)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_cookie(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c)
{
	ಬಿಡುಗಡೆ(c->name);
	ಬಿಡುಗಡೆ(c->ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(c->comment);
	ಬಿಡುಗಡೆ(c->domain);
	ಬಿಡುಗಡೆ(c->ಮಾರ್ಗ);

	ಬಿಡುಗಡೆ(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_ಮಾರ್ಗ_ಗಂಟು_content(ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಗಂಟು)
{
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *a, *b;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (ಗಂಟು->url != ಖಾಲಿ) {
		nsurl_unref(ಗಂಟು->url);
	}

	lwc_string_unref(ಗಂಟು->scheme);

	ಬಿಡುಗಡೆ(ಗಂಟು->segment);
	ಗಾಗಿ (i = 0; i < ಗಂಟು->frag_cnt; i++)
		ಬಿಡುಗಡೆ(ಗಂಟು->fragment[i]);
	ಬಿಡುಗಡೆ(ಗಂಟು->fragment);

	ಬಿಡುಗಡೆ(ಗಂಟು->urld.title);

	ಗಾಗಿ (a = ಗಂಟು->cookies; a; a = b) {
		b = a->ಮುಂದಿನ;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_prot_space(ರಚನೆ prot_space_ದತ್ತಾಂಶ *space)
{
	lwc_string_unref(space->scheme);
	ಬಿಡುಗಡೆ(space->realm);
	ಬಿಡುಗಡೆ(space->auth);

	ಬಿಡುಗಡೆ(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_ಮಾರ್ಗ_ವೃಕ್ಷ(ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *ಬೇರು)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಬೇರು;

	ಮಾಡು {
		ಆದರೆ (p->children != ಖಾಲಿ) {
			p = p->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *q = p;

			ಆಗ (p != ಬೇರು) {
				ಆದರೆ (p->ಮುಂದಿನ != ಖಾಲಿ) {
					p = p->ಮುಂದಿನ;
					ನಿಲ್ಲಿಸು;
				}

				p = p->ಪೋಷಕ;

				urldb_destroy_ಮಾರ್ಗ_ಗಂಟು_content(q);
				ಬಿಡುಗಡೆ(q);

				q = p;
			}

			urldb_destroy_ಮಾರ್ಗ_ಗಂಟು_content(q);
			ಬಿಡುಗಡೆ(q);
		}
	} ಆಗ (p != ಬೇರು);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_host_ವೃಕ್ಷ(ರಚನೆ host_part *ಬೇರು)
{
	ರಚನೆ host_part *a, *b;
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p, *q;
	ರಚನೆ prot_space_ದತ್ತಾಂಶ *s, *t;

	/* Destroy children */
	ಗಾಗಿ (a = ಬೇರು->children; a; a = b) {
		b = a->ಮುಂದಿನ;
		urldb_destroy_host_ವೃಕ್ಷ(a);
	}

	/* Now clean up paths */
	ಗಾಗಿ (p = ಬೇರು->paths.children; p; p = q) {
		q = p->ಮುಂದಿನ;
		urldb_destroy_ಮಾರ್ಗ_ವೃಕ್ಷ(p);
	}

	/* Root path */
	urldb_destroy_ಮಾರ್ಗ_ಗಂಟು_content(&ಬೇರು->paths);

	/* Proctection space data */
	ಗಾಗಿ (s = ಬೇರು->prot_space; s; s = t) {
		t = s->ಮುಂದಿನ;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	ಬಿಡುಗಡೆ(ಬೇರು->part);
	ಬಿಡುಗಡೆ(ಬೇರು);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ರಚನೆ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	/* Destroy children */
	ಆದರೆ (ಬೇರು->ಎಡ != &ಖಾಲಿಗಳು)
		urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಬೇರು->ಎಡ);
	ಆದರೆ (ಬೇರು->ಬಲ != &ಖಾಲಿಗಳು)
		urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಬೇರು->ಬಲ);

	/* And destroy ourselves */
	ಬಿಡುಗಡೆ(ಬೇರು);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_destroy(ಶೂನ್ಯ)
{
	ರಚನೆ host_part *a, *b;
	ಪೂರ್ಣಾಂಕ i;

	/* Clean up search trees */
	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಆದರೆ (ಹುಡುಕು_trees[i] != &ಖಾಲಿಗಳು) {
			urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಹುಡುಕು_trees[i]);
			ಹುಡುಕು_trees[i] = &ಖಾಲಿಗಳು;
		}
	}

	/* And database */
	ಗಾಗಿ (a = db_ಬೇರು.children; a; a = b) {
		b = a->ಮುಂದಿನ;
		urldb_destroy_host_ವೃಕ್ಷ(a);
	}
	memset(&db_ಬೇರು, 0, ಗಾತ್ರ(db_ಬೇರು));

	/* And the bloom filter */
	ಆದರೆ (url_bloom != ಖಾಲಿ) {
		bloom_destroy(url_bloom);
		url_bloom = ಖಾಲಿ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_ತುಂಬು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
#ವ್ಯಾಖ್ಯಾನಿಸು ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ 4096
	ಅಕ್ಷರ s[ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ];
	ಅಕ್ಷರ host[256];
	ರಚನೆ host_part *h;
	ಪೂರ್ಣಾಂಕ urls;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಆವೃತ್ತಿ;
	ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;
	FILE *fp;

	ಪ್ರತಿಜ್ಞಾ(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ಆದರೆ (url_bloom == ಖಾಲಿ)
		url_bloom = bloom_create(BLOOM_ಗಾತ್ರ);

	fp = fopen(filename, "r");
	ಆದರೆ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp)) {
		fclose(fp);
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;
	}

	ಆವೃತ್ತಿ = atoi(s);
	ಆದರೆ (ಆವೃತ್ತಿ < MIN_URL_FILE_ಆವೃತ್ತಿ) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}
	ಆದರೆ (ಆವೃತ್ತಿ > URL_FILE_ಆವೃತ್ತಿ) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಆಗ (fgets(host, ಗಾತ್ರ host, fp)) {
		time_t hsts_expiry = 0;
		ಪೂರ್ಣಾಂಕ hsts_include_sub_domains = 0;

		/* get the hostname */
		ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(host) - 1;
		host[ಉದ್ದಳತೆ] = '\0';

		/* skip data that has ended up with a host of '' */
		ಆದರೆ (ಉದ್ದಳತೆ == 0) {
			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;
			urls = atoi(s);
			/* Eight fields/url */
			ಗಾಗಿ (i = 0; i < (8 * urls); i++) {
				ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
					ನಿಲ್ಲಿಸು;
			}
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಆವೃತ್ತಿ >= 107) {
			ಅಕ್ಷರ *p = host;
			ಆಗ (*p && *p != ' ') p++;
			ಆಗ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ಆಗ (*p && *p != ' ') p++;
			ಆಗ (*p && *p == ' ') p++;
			nsc_snptimet(p, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p), &hsts_expiry);
		}

		h = urldb_ಕೂಡಿಸು_host(host);
		ಆದರೆ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
			ನಿಲ್ಲಿಸು;
		urls = atoi(s);

		/* no URLs => try next host */
		ಆದರೆ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ಮುಂದುವರಿ;
		}

		/* load the non-corrupt data */
		ಗಾಗಿ (i = 0; i < urls; i++) {
			ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p = ಖಾಲಿ;
			ಅಕ್ಷರ scheme[64], ports[10];
			ಅಕ್ಷರ url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			ಅಕ್ಷರ *ಮಾರ್ಗ_query;
			size_t len;

			ಆದರೆ (!fgets(scheme, ಗಾತ್ರ scheme, fp))
				ನಿಲ್ಲಿಸು;
			ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(scheme) - 1;
			scheme[ಉದ್ದಳತೆ] = '\0';

			ಆದರೆ (!fgets(ports, ಗಾತ್ರ ports, fp))
				ನಿಲ್ಲಿಸು;
			ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ports) - 1;
			ports[ಉದ್ದಳತೆ] = '\0';
			ಪೋರ್ಟ್ = atoi(ports);

			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;
			ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) - 1;
			s[ಉದ್ದಳತೆ] = '\0';

			ಆದರೆ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, ಗಾತ್ರ url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (ಪೋರ್ಟ್ ? ":" : ""),
				 (ಪೋರ್ಟ್ ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			ಆದರೆ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}

			ಆದರೆ (url_bloom != ಖಾಲಿ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ಸೇರಿಸು_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ಆದರೆ (nsurl_get(nsurl, NSURL_ಮಾರ್ಗ | NSURL_QUERY,
				      &ಮಾರ್ಗ_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_ಕೂಡಿಸು_ಮಾರ್ಗ(scheme_lwc, ಪೋರ್ಟ್, h, ಮಾರ್ಗ_query,
					   fragment_lwc, nsurl);
			ಆದರೆ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (p)
				p->urld.visits = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)atoi(s);

			/* entry last use time */
			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp)) {
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (p) {
				nsc_snptimet(s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) - 1, &p->urld.ಕೊನೆಯ_ಭೇಟಿ);
			}

			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (p)
				p->urld.type = (content_type)atoi(s);

			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;


			ಆದರೆ (!fgets(s, ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ, fp))
				ನಿಲ್ಲಿಸು;
			ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) - 1;
			ಆದರೆ (p && ಉದ್ದಳತೆ > 0) {
				s[ಉದ್ದಳತೆ] = '\0';
				p->urld.title = ಸ್ಮರಣೆ_ಕೊಡು(ಉದ್ದಳತೆ + 1);
				ಆದರೆ (p->urld.title)
					ಸ್ಮರಣೆ_ನಕಲು(p->urld.title, s, ಉದ್ದಳತೆ + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಗರಿಷ್ಠ_URL_ಉದ್ದಳತೆ

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	FILE *fp;
	ಪೂರ್ಣಾಂಕ i;

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "w");
	ಆದರೆ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_ಆವೃತ್ತಿ);

	ಗಾಗಿ (i = 0; i != ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ಹುಡುಕು_trees[i], fp);
	}

	fclose(fp);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_ಗಣ_url_persistence(nsurl *url, bool persist)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	p->persistent = persist;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_ಕೂಡಿಸು_url(nsurl *url)
{
	ರಚನೆ host_part *h;
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	lwc_string *scheme;
	lwc_string *ಪೋರ್ಟ್;
	lwc_string *host;
	lwc_string *fragment;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host_str;
	ಅಕ್ಷರ *ಮಾರ್ಗ_query = ಖಾಲಿ;
	size_t len;
	bool ಹೊಂದಿಕೆ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್_int;

	ಪ್ರತಿಜ್ಞಾ(url);

	ಆದರೆ (url_bloom == ಖಾಲಿ)
		url_bloom = bloom_create(BLOOM_ಗಾತ್ರ);

	ಆದರೆ (url_bloom != ಖಾಲಿ) {
		uint32_t hash = nsurl_hash(url);
		bloom_ಸೇರಿಸು_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ಆದರೆ (nsurl_get(url, NSURL_ಮಾರ್ಗ | NSURL_QUERY, &ಮಾರ್ಗ_query, &len) !=
	    NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}
	ಪ್ರತಿಜ್ಞಾ(ಮಾರ್ಗ_query != ಖಾಲಿ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಆದರೆ (scheme == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ_query);
		ಹಿಂತಿರುಗಿ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (host != ಖಾಲಿ) {
		host_str = lwc_string_ದತ್ತಾಂಶ(host);
		lwc_string_unref(host);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lwc_string_isequal(scheme, corestring_lwc_file, &ಹೊಂದಿಕೆ) ==
		   lwc_error_ok && ಹೊಂದಿಕೆ == true) {
		host_str = "localhost";

	} ಇಲ್ಲದಿದ್ದರೆ {
		lwc_string_unref(scheme);
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ_query);
		ಹಿಂತಿರುಗಿ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	ಪೋರ್ಟ್ = nsurl_get_component(url, NSURL_ಪೋರ್ಟ್);
	ಆದರೆ (ಪೋರ್ಟ್ != ಖಾಲಿ) {
		ಪೋರ್ಟ್_int = atoi(lwc_string_ದತ್ತಾಂಶ(ಪೋರ್ಟ್));
		lwc_string_unref(ಪೋರ್ಟ್);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೋರ್ಟ್_int = 0;
	}

	/* Get host entry */
	h = urldb_ಕೂಡಿಸು_host(host_str);

	/* Get path entry */
	ಆದರೆ (h != ಖಾಲಿ) {
		p = urldb_ಕೂಡಿಸು_ಮಾರ್ಗ(scheme,
				   ಪೋರ್ಟ್_int,
				   h,
				   ಮಾರ್ಗ_query,
				   fragment,
				   url);
	} ಇಲ್ಲದಿದ್ದರೆ {
		p = ಖಾಲಿ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ಹಿಂತಿರುಗಿ (p != ಖಾಲಿ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_ಗಣ_url_title(nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (p == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	/* copy the parameter if necessary */
	ಆದರೆ (title != ಖಾಲಿ) {
		ತಾತ್ಕಾಲಿಕ = strdup(title);
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ತಾತ್ಕಾಲಿಕ = ಖಾಲಿ;
	}

	ಬಿಡುಗಡೆ(p->urld.title);
	p->urld.title = ತಾತ್ಕಾಲಿಕ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_ಗಣ_url_content_type(nsurl *url, content_type type)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	p->urld.type = type;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_ಭೇಟಿ_ದತ್ತಾಂಶ(nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	p->urld.ಕೊನೆಯ_ಭೇಟಿ = time(ಖಾಲಿ);
	p->urld.visits++;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_reset_url_ಭೇಟಿ_ದತ್ತಾಂಶ(nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ;

	p->urld.ಕೊನೆಯ_ಭೇಟಿ = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *urldb_get_url_ದತ್ತಾಂಶ(nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ರಚನೆ url_ಆಂತರಿಕ_ದತ್ತಾಂಶ *u;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	u = &p->urld;

	ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ p->url;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_ಗಣ_auth_details(nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *auth)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p, *pi;
	ರಚನೆ host_part *h;
	ರಚನೆ prot_space_ದತ್ತಾಂಶ *space, *space_alloc;
	ಅಕ್ಷರ *realm_alloc, *auth_alloc;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(url && realm && auth);

	/* add url, in case it's missing */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ;

	/* Search for host_part */
	ಗಾಗಿ (pi = p; pi->ಪೋಷಕ != ಖಾಲಿ; pi = pi->ಪೋಷಕ)
		;
	h = (ರಚನೆ host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	ಗಾಗಿ (space = h->prot_space; space; space = space->ಮುಂದಿನ) {
		ಆದರೆ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &ಹೊಂದಿಕೆ) == lwc_error_ok &&
		    ಹೊಂದಿಕೆ == true &&
		    space->ಪೋರ್ಟ್ == p->ಪೋರ್ಟ್)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (space != ಖಾಲಿ) {
		/* Overrule existing auth. */
		ಬಿಡುಗಡೆ(space->auth);
		space->auth = strdup(auth);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Create a new protection space. */
		space = space_alloc = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ prot_space_ದತ್ತಾಂಶ));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ಆದರೆ (!space_alloc || !realm_alloc || !auth_alloc) {
			ಬಿಡುಗಡೆ(space_alloc);
			ಬಿಡುಗಡೆ(realm_alloc);
			ಬಿಡುಗಡೆ(auth_alloc);
			ಹಿಂತಿರುಗಿ;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->ಪೋರ್ಟ್ = p->ಪೋರ್ಟ್;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->ಮುಂದಿನ = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *urldb_get_auth_details(nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p, *p_cur, *p_ತುದಿ;

	ಪ್ರತಿಜ್ಞಾ(url);

	/* add to the db, so our lookup will work */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	ಗಾಗಿ (p_cur = p; p_cur != ಖಾಲಿ; p_ತುದಿ = p_cur, p_cur = p_cur->ಪೋಷಕ) {
		ಆದರೆ (p_cur->prot_space) {
			ಹಿಂತಿರುಗಿ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ಆದರೆ (realm != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h = (ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *)p_ತುದಿ;
		ಸ್ಥಿರಾಂಕ ರಚನೆ prot_space_ದತ್ತಾಂಶ *space;
		bool ಹೊಂದಿಕೆ;

		/* Search for a possible matching protection space. */
		ಗಾಗಿ (space = h->prot_space; space != ಖಾಲಿ;
		     space = space->ಮುಂದಿನ) {
			ಆದರೆ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &ಹೊಂದಿಕೆ) ==
			    lwc_error_ok &&
			    ಹೊಂದಿಕೆ == true &&
			    space->ಪೋರ್ಟ್ == p->ಪೋರ್ಟ್) {
				p->prot_space = space;
				ಹಿಂತಿರುಗಿ p->prot_space->auth;
			}
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_ಗಣ_cert_permissions(nsurl *url, bool permit)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ರಚನೆ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(url);

	/* add url, in case it's missing */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (; p && p->ಪೋಷಕ; p = p->ಪೋಷಕ)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ರಚನೆ host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (; p && p->ಪೋಷಕ; p = p->ಪೋಷಕ)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *)p;

	ಹಿಂತಿರುಗಿ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_ಗಣ_hsts_policy(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆ)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ರಚನೆ host_part *h;
	lwc_string *host;
	time_t now = time(ಖಾಲಿ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (host != ಖಾಲಿ) {
		ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(lwc_string_ದತ್ತಾಂಶ(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ಹಿಂತಿರುಗಿ true;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lwc_string_ಉದ್ದಳತೆ(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ಹಿಂತಿರುಗಿ true;
		}

		lwc_string_unref(host);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* No host part: ignore */
		ಹಿಂತಿರುಗಿ true;
	}

	/* add url, in case it's missing */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (; p && p->ಪೋಷಕ; p = p->ಪೋಷಕ)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ರಚನೆ host_part *)p;
	ಆದರೆ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ಹಿಂತಿರುಗಿ true;
	}

	error = http_parse_strict_transport_security(ಶೀರ್ಷಿಕೆ, &sts);
	ಆದರೆ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ಹಿಂತಿರುಗಿ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ಆದರೆ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ರಚನೆ nsurl *url)
{
	ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p;
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	lwc_string *host;
	time_t now = time(ಖಾಲಿ);

	ಪ್ರತಿಜ್ಞಾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (host != ಖಾಲಿ) {
		ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(lwc_string_ದತ್ತಾಂಶ(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ಹಿಂತಿರುಗಿ false;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lwc_string_ಉದ್ದಳತೆ(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ಹಿಂತಿರುಗಿ false;
		}

		lwc_string_unref(host);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* No host part: not enabled */
		ಹಿಂತಿರುಗಿ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (; p && p->ಪೋಷಕ; p = p->ಪೋಷಕ)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *)p;

	/* Consult record for this host */
	ಆದರೆ (h->hsts.expires > now) {
		/* Not expired */
		ಹಿಂತಿರುಗಿ true;
	}

	/* Consult parent domains */
	ಗಾಗಿ (h = h->ಪೋಷಕ; h && h != &db_ಬೇರು; h = h->ಪೋಷಕ) {
		ಆದರೆ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ಹಿಂತಿರುಗಿ true;
		}
	}

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ
urldb_iterate_partial(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ,
		      bool (*callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಅಕ್ಷರ host[256];
	ಅಕ್ಷರ buf[260]; /* max domain + "www." */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *slash, *scheme_sep;
	ರಚನೆ ಹುಡುಕು_ಗಂಟು *ವೃಕ್ಷ;
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(ಪೂರ್ವಪ್ರತ್ಯಯ && callback);

	/* strip scheme */
	scheme_sep = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ಪೂರ್ವಪ್ರತ್ಯಯ, "://");
	ಆದರೆ (scheme_sep)
		ಪೂರ್ವಪ್ರತ್ಯಯ = scheme_sep + 3;

	slash = strchr(ಪೂರ್ವಪ್ರತ್ಯಯ, '/');
	ವೃಕ್ಷ = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಆದರೆ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, ಗಾತ್ರ host, "%.*s",
			 (ಪೂರ್ಣಾಂಕ) (slash - ಪೂರ್ವಪ್ರತ್ಯಯ), ಪೂರ್ವಪ್ರತ್ಯಯ);

		h = urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(ವೃಕ್ಷ, host);
		ಆದರೆ (!h) {
			ಪೂರ್ಣಾಂಕ len = slash - ಪೂರ್ವಪ್ರತ್ಯಯ;

			ಆದರೆ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, ಗಾತ್ರ buf, "www.%s", host);
				h = urldb_ಹುಡುಕು_ಕಂಡುಹಿಡಿ(
					ಹುಡುಕು_trees[ST_DN + 'w' - 'a'],
					buf);
				ಆದರೆ (!h)
					ಹಿಂತಿರುಗಿ;
			} ಇಲ್ಲದಿದ್ದರೆ
				ಹಿಂತಿರುಗಿ;
		}

		ಆದರೆ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_ಮಾರ್ಗ(&h->paths, slash + 1,
						   callback);
		}

	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪೂರ್ವಪ್ರತ್ಯಯ);

		/* looking for hosts */
		ಆದರೆ (!urldb_iterate_partial_host(ವೃಕ್ಷ, ಪೂರ್ವಪ್ರತ್ಯಯ, callback))
			ಹಿಂತಿರುಗಿ;

		ಆದರೆ (len <= 3 || strncasecmp(ಪೂರ್ವಪ್ರತ್ಯಯ, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, ಗಾತ್ರ buf, "www.%s", ಪೂರ್ವಪ್ರತ್ಯಯ);
			ಆದರೆ(!urldb_iterate_partial_host(
				   ಹುಡುಕು_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ಹಿಂತಿರುಗಿ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ
urldb_iterate_entries(bool (*callback)(nsurl *url, ಸ್ಥಿರಾಂಕ ರಚನೆ url_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಪೂರ್ಣಾಂಕ i;

	ಪ್ರತಿಜ್ಞಾ(callback);

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಆದರೆ (!urldb_iterate_entries_host(ಹುಡುಕು_trees[i],
						callback,
						ಖಾಲಿ)) {
			ನಿಲ್ಲಿಸು;
		}
	}
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_iterate_cookies(bool (*callback)(ಸ್ಥಿರಾಂಕ ರಚನೆ cookie_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ))
{
	ಪೂರ್ಣಾಂಕ i;

	ಪ್ರತಿಜ್ಞಾ(callback);

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಆದರೆ (!urldb_iterate_entries_host(ಹುಡುಕು_trees[i],
						ಖಾಲಿ, callback))
			ನಿಲ್ಲಿಸು;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_ಗಣ_cookie(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆ, nsurl *url, nsurl *referer)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cur = ಶೀರ್ಷಿಕೆ, *ಕೊನೆ;
	lwc_string *ಮಾರ್ಗ, *host, *scheme;
	nsurl *urlt;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(url && ಶೀರ್ಷಿಕೆ);

	/* Get defragmented URL, as 'urlt' */
	ಆದರೆ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಆದರೆ (scheme == ಖಾಲಿ) {
		nsurl_unref(urlt);
		ಹಿಂತಿರುಗಿ false;
	}

	ಮಾರ್ಗ = nsurl_get_component(url, NSURL_ಮಾರ್ಗ);
	ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ಹಿಂತಿರುಗಿ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (host == ಖಾಲಿ) {
		lwc_string_unref(ಮಾರ್ಗ);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ಆದರೆ (rhost == ಖಾಲಿ) {
			ಹೋಗು error;
		}

		/* Domain match host names */
		ಆದರೆ (lwc_string_isequal(host, rhost, &ಹೊಂದಿಕೆ) == lwc_error_ok &&
		    ಹೊಂದಿಕೆ == false) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hptr;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rptr;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dot;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host_ದತ್ತಾಂಶ = lwc_string_ದತ್ತಾಂಶ(host);
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rhost_ದತ್ತಾಂಶ = lwc_string_ದತ್ತಾಂಶ(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(host_ದತ್ತಾಂಶ) ||
			    urldb_host_is_ip_ವಿಳಾಸ(rhost_ದತ್ತಾಂಶ)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				ಹೋಗು error;
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

			hptr = host_ದತ್ತಾಂಶ + lwc_string_ಉದ್ದಳತೆ(host) - 1;
			rptr = rhost_ದತ್ತಾಂಶ + lwc_string_ಉದ್ದಳತೆ(rhost) - 1;

			/* 1 */
			ಆಗ (hptr >= host_ದತ್ತಾಂಶ && rptr >= rhost_ದತ್ತಾಂಶ) {
				ಆದರೆ (*hptr != *rptr)
					ನಿಲ್ಲಿಸು;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ಆಗ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ಆದರೆ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ಖಾಲಿ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				ಹೋಗು error;
			}
		}

		lwc_string_unref(rhost);
	}

	ಕೊನೆ = cur + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cur) - 2 /* Trailing CRLF */;

	ಮಾಡು {
		ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;
		ಅಕ್ಷರ *dot;
		size_t len;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_NSPSL
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix;
#ಆದರೆ_ಅಂತ್ಯ

		c = urldb_parse_cookie(url, &cur);
		ಆದರೆ (!c) {
			/* failed => stop parsing */
			ಹೋಗು error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ಆದರೆ (!c->name || !c->ಮೌಲ್ಯ) {
			urldb_free_cookie(c);
			ಹೋಗು error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮಾರ್ಗ);
		ಆದರೆ (len > lwc_string_ಉದ್ದಳತೆ(ಮಾರ್ಗ) ||
		    ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(c->ಮಾರ್ಗ, lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ),
			    len) != 0) {
			urldb_free_cookie(c);
			ಹೋಗು error;
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ಆದರೆ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ಆದರೆ (suffix == ಖಾಲಿ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			ಹೋಗು error;
		}
#ಇಲ್ಲದಿದ್ದರೆ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ಆದರೆ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			ಹೋಗು error;
		}
#ಆದರೆ_ಅಂತ್ಯ

		/* Domain match fetch host with cookie domain */
		ಆದರೆ (strcasecmp(lwc_string_ದತ್ತಾಂಶ(host), c->domain) != 0) {
			ಪೂರ್ಣಾಂಕ hlen, dlen;
			ಅಕ್ಷರ *domain = c->domain;

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
			ಪ್ರತಿಜ್ಞಾ(c->domain[0] == '.');

			/* 4.3.2:iii */
			ಆದರೆ (urldb_host_is_ip_ವಿಳಾಸ(lwc_string_ದತ್ತಾಂಶ(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				ಹೋಗು error;
			}

			hlen = lwc_string_ಉದ್ದಳತೆ(host);
			dlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->domain);

			ಆದರೆ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				ಹೋಗು error;
			}

			ಆದರೆ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ಆದರೆ (strcasecmp(lwc_string_ದತ್ತಾಂಶ(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				ಹೋಗು error;
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
		ಆದರೆ (!urldb_ಸೇರಿಸು_cookie(c, scheme, urlt))
			ಹೋಗು error;
	} ಆಗ (cur < ಕೊನೆ);

	lwc_string_unref(host);
	lwc_string_unref(ಮಾರ್ಗ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ಹಿಂತಿರುಗಿ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(ಮಾರ್ಗ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in content/urldb.h */
ಅಕ್ಷರ *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಮಾರ್ಗ_ದತ್ತಾಂಶ *p, *q;
	ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *h;
	lwc_string *ಮಾರ್ಗ_lwc;
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0, ಆವೃತ್ತಿ = COOKIE_RFC2965;
	ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ **matched_cookies;
	ಪೂರ್ಣಾಂಕ matched_cookies_ಗಾತ್ರ = 20;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಅಕ್ಷರ *ret;
	lwc_string *scheme;
	time_t now;
	bool ಹೊಂದಿಕೆ;

	ಪ್ರತಿಜ್ಞಾ(url != ಖಾಲಿ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_ಕೂಡಿಸು_url(url);

	p = urldb_ಕಂಡುಹಿಡಿ_url(url);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	scheme = p->scheme;

	matched_cookies = ಸ್ಮರಣೆ_ಕೊಡು(matched_cookies_ಗಾತ್ರ *
				 ಗಾತ್ರ(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *));
	ಆದರೆ (!matched_cookies)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

#ವ್ಯಾಖ್ಯಾನಿಸು GROW_MATCHED_COOKIES						\
	ಮಾಡು {								\
		ಆದರೆ (ಎಣಿಕೆ_2 == matched_cookies_ಗಾತ್ರ) {			\
			ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ **ತಾತ್ಕಾಲಿಕ;		\
			ತಾತ್ಕಾಲಿಕ = realloc(matched_cookies,			\
				       (matched_cookies_ಗಾತ್ರ + 20) *	\
				       ಗಾತ್ರ(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *)); \
									\
			ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ) {				\
				ಬಿಡುಗಡೆ(matched_cookies);			\
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;				\
			}						\
									\
			matched_cookies = ತಾತ್ಕಾಲಿಕ;				\
			matched_cookies_ಗಾತ್ರ += 20;			\
		}							\
	} ಆಗ(0)

	ಮಾರ್ಗ_lwc = nsurl_get_component(url, NSURL_ಮಾರ್ಗ);
	ಆದರೆ (ಮಾರ್ಗ_lwc == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(matched_cookies);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಮಾರ್ಗ = lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ_lwc);
	lwc_string_unref(ಮಾರ್ಗ_lwc);

	now = time(ಖಾಲಿ);

	ಆದರೆ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		ಗಾಗಿ (q = p->ಪೋಷಕ->children; q; q = q->ಮುಂದಿನ) {
			ಆದರೆ (strcmp(q->segment, p->segment))
				ಮುಂದುವರಿ;

			/* Consider all cookies associated with
			 * this exact path */
			ಗಾಗಿ (c = q->cookies; c; c = c->ಮುಂದಿನ) {
				ಆದರೆ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ಮುಂದುವರಿ;

				ಆದರೆ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &ಹೊಂದಿಕೆ) &&
				    ಹೊಂದಿಕೆ == false)
					/* secure cookie for insecure host.
					 * ignore */
					ಮುಂದುವರಿ;

				ಆದರೆ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ಮುಂದುವರಿ;

				matched_cookies[ಎಣಿಕೆ_2++] = c;

				GROW_MATCHED_COOKIES;

				ಆದರೆ (c->ಆವೃತ್ತಿ < (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಆವೃತ್ತಿ)
					ಆವೃತ್ತಿ = c->ಆವೃತ್ತಿ;

				c->ಕೊನೆಯ_ಬಳಸಿದ = now;

				cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	ಗಾಗಿ (p = p->ಪೋಷಕ; p; p = p->ಪೋಷಕ) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		ಗಾಗಿ (q = p->children; q; q = q->ಮುಂದಿನ) {
			ಆದರೆ (*(q->segment) != '\0')
				ಮುಂದುವರಿ;

			ಗಾಗಿ (c = q->cookies; c; c = c->ಮುಂದಿನ) {
				ಆದರೆ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ಮುಂದುವರಿ;

				ಆದರೆ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &ಹೊಂದಿಕೆ) &&
				    ಹೊಂದಿಕೆ == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ಮುಂದುವರಿ;

				matched_cookies[ಎಣಿಕೆ_2++] = c;

				GROW_MATCHED_COOKIES;

				ಆದರೆ (c->ಆವೃತ್ತಿ < (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) ಆವೃತ್ತಿ)
					ಆವೃತ್ತಿ = c->ಆವೃತ್ತಿ;

				c->ಕೊನೆಯ_ಬಳಸಿದ = now;

				cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
			}
		}

		ಆದರೆ (!p->ಪೋಷಕ) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ನಿಲ್ಲಿಸು;
		}

		/* Consider p itself - may be the result of Path=/foo */
		ಗಾಗಿ (c = p->cookies; c; c = c->ಮುಂದಿನ) {
			ಆದರೆ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ಮುಂದುವರಿ;

			/* Ensure cookie path is a prefix of the resource */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(c->ಮಾರ್ಗ, ಮಾರ್ಗ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮಾರ್ಗ)) != 0)
				/* paths don't match => ignore */
				ಮುಂದುವರಿ;

			ಆದರೆ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &ಹೊಂದಿಕೆ) &&
			    ಹೊಂದಿಕೆ == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ಮುಂದುವರಿ;

			matched_cookies[ಎಣಿಕೆ_2++] = c;

			GROW_MATCHED_COOKIES;

			ಆದರೆ (c->ಆವೃತ್ತಿ < (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) ಆವೃತ್ತಿ)
				ಆವೃತ್ತಿ = c->ಆವೃತ್ತಿ;

			c->ಕೊನೆಯ_ಬಳಸಿದ = now;

			cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	ಗಾಗಿ (h = (ಸ್ಥಿರಾಂಕ ರಚನೆ host_part *)p; h && h != &db_ಬೇರು;
	     h = h->ಪೋಷಕ) {
		ಗಾಗಿ (c = h->paths.cookies; c; c = c->ಮುಂದಿನ) {
			ಆದರೆ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ಮುಂದುವರಿ;

			/* Ensure cookie path is a prefix of the resource */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(c->ಮಾರ್ಗ, ಮಾರ್ಗ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->ಮಾರ್ಗ)) != 0)
				/* paths don't match => ignore */
				ಮುಂದುವರಿ;

			ಆದರೆ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &ಹೊಂದಿಕೆ) &&
			    ಹೊಂದಿಕೆ == false)
				/* secure cookie for insecure host. ignore */
				ಮುಂದುವರಿ;

			matched_cookies[ಎಣಿಕೆ_2++] = c;

			GROW_MATCHED_COOKIES;

			ಆದರೆ (c->ಆವೃತ್ತಿ < (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಆವೃತ್ತಿ)
				ಆವೃತ್ತಿ = c->ಆವೃತ್ತಿ;

			c->ಕೊನೆಯ_ಬಳಸಿದ = now;

			cookie_manager_ಕೂಡಿಸು((ರಚನೆ cookie_ದತ್ತಾಂಶ *)c);
		}
	}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು GROW_MATCHED_COOKIES

	ಆದರೆ (ಎಣಿಕೆ_2 == 0) {
		/* No cookies found */
		ಬಿಡುಗಡೆ(matched_cookies);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (ಆವೃತ್ತಿ == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, ಎಣಿಕೆ_2);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = get_rfc2965_cookie(matched_cookies, ಎಣಿಕೆ_2, ಆವೃತ್ತಿ);
	}
	ಬಿಡುಗಡೆ(matched_cookies);

	ಹಿಂತಿರುಗಿ ret;
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_delete_cookie(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *domain, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	urldb_delete_cookie_hosts(domain, ಮಾರ್ಗ, name, &db_ಬೇರು);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_ತುಂಬು_cookies(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	FILE *fp;
	ಅಕ್ಷರ s[16*1024];

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "r");
	ಆದರೆ (!fp)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

#ವ್ಯಾಖ್ಯಾನಿಸು ಕಂಡುಹಿಡಿ_T {				\
		ಗಾಗಿ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ಆದರೆ (p >= ಕೊನೆ) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ಮುಂದುವರಿ;		\
		}				\
		*p++ = '\0';			\
	}

#ವ್ಯಾಖ್ಯಾನಿಸು SKIP_T {				\
		ಗಾಗಿ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ಆದರೆ (p >= ಕೊನೆ) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ಮುಂದುವರಿ;		\
		}				\
	}

	ಆಗ (fgets(s, ಗಾತ್ರ s, fp)) {
		ಅಕ್ಷರ *p = s, *ಕೊನೆ = 0,
			*domain, *ಮಾರ್ಗ, *name, *ಮೌಲ್ಯ, *scheme, *url,
			*comment;
		ಪೂರ್ಣಾಂಕ ಆವೃತ್ತಿ, domain_specified, ಮಾರ್ಗ_specified,
			secure, http_only, no_destroy, ಮೌಲ್ಯ_quoted;
		time_t expires, ಕೊನೆಯ_ಬಳಸಿದ;
		ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ *c;

		ಆದರೆ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ಮುಂದುವರಿ;

		s[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) - 1] = '\0'; /* lose terminating newline */
		ಕೊನೆ = s + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ಆದರೆ (strncasecmp(s, "Version:", 8) == 0) {
			ಕಂಡುಹಿಡಿ_T; SKIP_T; loaded_cookie_file_ಆವೃತ್ತಿ = atoi(p);

			ಆದರೆ (loaded_cookie_file_ಆವೃತ್ತಿ <
			    MIN_COOKIE_FILE_ಆವೃತ್ತಿ) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ನಿಲ್ಲಿಸು;
			}

			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (loaded_cookie_file_ಆವೃತ್ತಿ == 0) {
			/* Haven't yet seen version; skip this input */
			ಮುಂದುವರಿ;
		}

		/* One cookie/line */

		/* Parse input */
		ಕಂಡುಹಿಡಿ_T; ಆವೃತ್ತಿ = atoi(s);
		SKIP_T; domain = p; ಕಂಡುಹಿಡಿ_T;
		SKIP_T; domain_specified = atoi(p); ಕಂಡುಹಿಡಿ_T;
		SKIP_T; ಮಾರ್ಗ = p; ಕಂಡುಹಿಡಿ_T;
		SKIP_T; ಮಾರ್ಗ_specified = atoi(p); ಕಂಡುಹಿಡಿ_T;
		SKIP_T; secure = atoi(p); ಕಂಡುಹಿಡಿ_T;
		ಆದರೆ (loaded_cookie_file_ಆವೃತ್ತಿ > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); ಕಂಡುಹಿಡಿ_T;
		} ಇಲ್ಲದಿದ್ದರೆ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); ಕಂಡುಹಿಡಿ_T;
		SKIP_T; ಕೊನೆಯ_ಬಳಸಿದ = (time_t)atoi(p); ಕಂಡುಹಿಡಿ_T;
		SKIP_T; no_destroy = atoi(p); ಕಂಡುಹಿಡಿ_T;
		SKIP_T; name = p; ಕಂಡುಹಿಡಿ_T;
		SKIP_T; ಮೌಲ್ಯ = p; ಕಂಡುಹಿಡಿ_T;
		ಆದರೆ (loaded_cookie_file_ಆವೃತ್ತಿ > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	ಮೌಲ್ಯ_quoted = atoi(p); ಕಂಡುಹಿಡಿ_T;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಮೌಲ್ಯ_quoted = 0;
		}
		SKIP_T; scheme = p; ಕಂಡುಹಿಡಿ_T;
		SKIP_T; url = p; ಕಂಡುಹಿಡಿ_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		ಗಾಗಿ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ಪ್ರತಿಜ್ಞಾ(p <= ಕೊನೆ);

		/* Now create cookie */
		c = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ cookie_ಆಂತರಿಕ_ದತ್ತಾಂಶ));
		ಆದರೆ (!c)
			ನಿಲ್ಲಿಸು;

		c->name = strdup(name);
		c->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
		c->ಮೌಲ್ಯ_was_quoted = ಮೌಲ್ಯ_quoted;
		c->comment = strdup(comment);
		c->domain_from_ಗಣ = domain_specified;
		c->domain = strdup(domain);
		c->ಮಾರ್ಗ_from_ಗಣ = ಮಾರ್ಗ_specified;
		c->ಮಾರ್ಗ = strdup(ಮಾರ್ಗ);
		c->expires = expires;
		c->ಕೊನೆಯ_ಬಳಸಿದ = ಕೊನೆಯ_ಬಳಸಿದ;
		c->secure = secure;
		c->http_only = http_only;
		c->ಆವೃತ್ತಿ = ಆವೃತ್ತಿ;
		c->no_destroy = no_destroy;

		ಆದರೆ (!(c->name && c->ಮೌಲ್ಯ && c->comment &&
		      c->domain && c->ಮಾರ್ಗ)) {
			urldb_free_cookie(c);
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ಖಾಲಿ;
			nsurl *url_nsurl = ಖಾಲಿ;

			ಪ್ರತಿಜ್ಞಾ(scheme[0] != 'u');

			ಆದರೆ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ನಿಲ್ಲಿಸು;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ಆದರೆ (!urldb_ಸೇರಿಸು_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ನಿಲ್ಲಿಸು;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (!urldb_ಸೇರಿಸು_cookie(c, ಖಾಲಿ, ಖಾಲಿ)) {
				/* Cookie freed for us */
				ನಿಲ್ಲಿಸು;
			}
		}
	}

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SKIP_T
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಕಂಡುಹಿಡಿ_T

	fclose(fp);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	FILE *fp;
	ಪೂರ್ಣಾಂಕ cookie_file_ಆವೃತ್ತಿ = max(loaded_cookie_file_ಆವೃತ್ತಿ,
				      COOKIE_FILE_ಆವೃತ್ತಿ);

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "w");
	ಆದರೆ (!fp)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

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
		cookie_file_ಆವೃತ್ತಿ);
	fprintf(fp, "Version:\t%d\n", cookie_file_ಆವೃತ್ತಿ);

	urldb_save_cookie_hosts(fp, &db_ಬೇರು);

	fclose(fp);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_dump(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;

	urldb_dump_hosts(&db_ಬೇರು);

	ಗಾಗಿ (i = 0; i != ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		urldb_dump_ಹುಡುಕು(ಹುಡುಕು_trees[i], 0);
	}
}
