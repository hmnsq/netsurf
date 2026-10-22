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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_NSPSL
#ਅਨ੍ਤਰ੍ਭਾਵ <nspsl.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/inet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/url.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/bloom.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/time.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/cookie_manager.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ {
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *prev;	/**< Previous in list */
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *ਅਗਲਾ;	/**< Next in list */

	ਅੱਖਰ *name;		/**< Cookie name */
	ਅੱਖਰ *ਮੁੱਲ;		/**< Cookie value */
	bool ਮੁੱਲ_was_quoted;	/**< Value was quoted in Set-Cookie: */
	ਅੱਖਰ *comment;		/**< Cookie comment */
	bool domain_from_ਸੈੱਟ;	/**< Domain came from Set-Cookie: header */
	ਅੱਖਰ *domain;		/**< Domain */
	bool ਰਸਤਾ_from_ਸੈੱਟ;	/**< Path came from Set-Cookie: header */
	ਅੱਖਰ *ਰਸਤਾ;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t ਆਖਰੀ_ਵਰਤਿਆ;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ਗਿਣਤੀ cookie_ਸੰਸਕਰਣ ਸੰਸਕਰਣ;	/**< Specification compliance */
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
ਬਣਤਰ prot_space_ਅੰਕੜੇ {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ;
	/** Protection realm */
	ਅੱਖਰ *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	ਅੱਖਰ *auth;
	/** Next sibling */
	ਬਣਤਰ prot_space_ਅੰਕੜੇ *ਅਗਲਾ;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ਬਣਤਰ url_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ {
	ਅੱਖਰ *title;		/**< Resource title */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ visits;	/**< Visit count */
	time_t ਆਖਰੀ_ਦੇਖੋ;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	ਅੱਖਰ *segment;		/**< Path segment for this node */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ frag_cnt;	/**< Number of entries in path_data::fragment */
	ਅੱਖਰ **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ਬਣਤਰ url_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ਅਚਲ ਬਣਤਰ prot_space_ਅੰਕੜੇ *prot_space;
	/** Cookies associated with resource */
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *cookies;
	/** Last cookie in list */
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *cookies_ਸਿਰਾ;

	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਅਗਲਾ;	/**< Next sibling */
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *prev;	/**< Previous sibling */
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ; /**< Parent path segment */
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *children; /**< Child path segments */
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਆਖਰੀ; /**< Last child */
};

ਬਣਤਰ hsts_ਅੰਕੜੇ {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ਬਣਤਰ host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ਬਣਤਰ hsts_ਅੰਕੜੇ hsts;

	/**
	 * Part of host string
	 */
	ਅੱਖਰ *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ਬਣਤਰ prot_space_ਅੰਕੜੇ *prot_space;

	ਬਣਤਰ host_part *ਅਗਲਾ;	/**< Next sibling */
	ਬਣਤਰ host_part *prev;	/**< Previous sibling */
	ਬਣਤਰ host_part *ਮਾਪਾ; /**< Parent host part */
	ਬਣਤਰ host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ਬਣਤਰ ਖੋਜ_ਗੰਢ {
	ਅਚਲ ਬਣਤਰ host_part *ਅੰਕੜੇ;	/**< Host tree entry */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ level;		/**< Node level */

	ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਖੱਬਾ;	/**< Left subtree */
	ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਸੱਜਾ;	/**< Right subtree */
};

/** Root database handle */
ਸਥਿਰ ਬਣਤਰ host_part db_ਜੜ੍ਹ;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_ਖੋਜ_TREES 28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_IP 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_EE 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_DN 2
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ ਖਾਲੀ_ਥਾਵਾਂ = { 0, 0, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ };
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਖੋਜ_trees[ਅੰਕ_ਖੋਜ_TREES] = {
	&ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ,
	&ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ,
	&ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ,
	&ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ, &ਖਾਲੀ_ਥਾਵਾਂ
};

/** Minimum cookie database file version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_COOKIE_FILE_ਸੰਸਕਰਣ 100
/** Current cookie database file version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COOKIE_FILE_ਸੰਸਕਰਣ 102
/** loaded cookie file version */
ਸਥਿਰ ਪੂਰਨ loaded_cookie_file_ਸੰਸਕਰਣ;

/** Minimum URL database file version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_URL_FILE_ਸੰਸਕਰਣ 106
/** Current URL database file version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ URL_FILE_ਸੰਸਕਰਣ 107

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
ਸਥਿਰ ਬਣਤਰ bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLOOM_ਮਾਪ (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
ਸਥਿਰ nserror urldb_write_timet(FILE *fp, time_t val)
{
	ਪੂਰਨ use;
	ਅੱਖਰ op[32];

	use = nsc_sntimet(op, 32, &val);
	ਜੇ (use == 0) {
		fprintf(fp, "%i\n", (ਪੂਰਨ)val);
	} ਨਹੀਂਤਾਂ {
		fprintf(fp, "%.*s\n", use, op);
	}
	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ ਖਾਲੀ
urldb_write_paths(ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ,
		  ਅਚਲ ਅੱਖਰ *host,
		  FILE *fp,
		  ਅੱਖਰ **ਰਸਤਾ,
		  ਪੂਰਨ *ਰਸਤਾ_alloc,
		  ਪੂਰਨ *ਰਸਤਾ_ਵਰਤਿਆ,
		  time_t expiry)
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ;
	ਪੂਰਨ i;

	ਕਰੋ {
		ਪੂਰਨ seglen = p->segment != ਨਿਰਾ ? ਲੜੀ_ਲੰਬਾਈ(p->segment) : 0;
		ਪੂਰਨ len = *ਰਸਤਾ_ਵਰਤਿਆ + seglen + 1;

		ਜੇ (*ਰਸਤਾ_alloc < len) {
			ਅੱਖਰ *ਆਰਜ਼ੀ;
			ਆਰਜ਼ੀ = realloc(*ਰਸਤਾ,
				       (len > 64) ? len : *ਰਸਤਾ_alloc + 64);
			ਜੇ (!ਆਰਜ਼ੀ) {
				ਵਾਪਸ;
			}
			*ਰਸਤਾ = ਆਰਜ਼ੀ;
			*ਰਸਤਾ_alloc = (len > 64) ? len : *ਰਸਤਾ_alloc + 64;
		}

		ਜੇ (p->segment != ਨਿਰਾ) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(*ਰਸਤਾ + *ਰਸਤਾ_ਵਰਤਿਆ - 1, p->segment, seglen);
		}

		ਜੇ (p->children != ਨਿਰਾ) {
			(*ਰਸਤਾ)[*ਰਸਤਾ_ਵਰਤਿਆ + seglen - 1] = '/';
			(*ਰਸਤਾ)[*ਰਸਤਾ_ਵਰਤਿਆ + seglen] = '\0';
		} ਨਹੀਂਤਾਂ {
			(*ਰਸਤਾ)[*ਰਸਤਾ_ਵਰਤਿਆ + seglen - 1] = '\0';
			len -= 1;
		}

		*ਰਸਤਾ_ਵਰਤਿਆ = len;

		ਜੇ (p->children != ਨਿਰਾ) {
			/* Drill down into children */
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			/* leaf node */
			ਜੇ (p->persistent ||
			    ((p->urld.ਆਖਰੀ_ਦੇਖੋ > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_ਅੰਕੜੇ(p->scheme));

				ਜੇ (p->ਪੋਰਟ) {
					fprintf(fp,"%d\n", p->ਪੋਰਟ);
				} ਨਹੀਂਤਾਂ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *ਰਸਤਾ);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.ਆਖਰੀ_ਦੇਖੋ);

				/* entry type */
				fprintf(fp, "%i\n", (ਪੂਰਨ)p->urld.type);

				fprintf(fp, "\n");

				ਜੇ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					ਲਈ (i = 0; s[i] != '\0'; i++)
						ਜੇ (s[i] < 32)
							s[i] = ' ';
					ਲਈ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ਨਹੀਂਤਾਂ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ਜਦੋਂ (p != ਮਾਪਾ) {
				ਪੂਰਨ seglen = p->segment != ਨਿਰਾ
					? ਲੜੀ_ਲੰਬਾਈ(p->segment) : 0;

				/* Remove our segment from the path */
				*ਰਸਤਾ_ਵਰਤਿਆ -= seglen;
				(*ਰਸਤਾ)[*ਰਸਤਾ_ਵਰਤਿਆ - 1] = '\0';

				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					/* Have a sibling, process that */
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				/* Going up, so remove '/' */
				*ਰਸਤਾ_ਵਰਤਿਆ -= 1;
				(*ਰਸਤਾ)[*ਰਸਤਾ_ਵਰਤਿਆ - 1] = '\0';

				/* Ascend tree */
				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਮਾਪਾ);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ਸਥਿਰ ਖਾਲੀ
urldb_ਗਿਣਤੀ_urls(ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਜੜ੍ਹ,
		 time_t expiry,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *ਗਿਣਤੀ_2)
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਜੜ੍ਹ;

	ਕਰੋ {
		ਜੇ (p->children != ਨਿਰਾ) {
			/* Drill down into children */
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			/* No more children, increment count if required */
			ਜੇ (p->persistent ||
			    ((p->urld.ਆਖਰੀ_ਦੇਖੋ > expiry) &&
			     (p->urld.visits > 0))) {
				(*ਗਿਣਤੀ_2)++;
			}

			/* Now, find next node to process. */
			ਜਦੋਂ (p != ਜੜ੍ਹ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					/* Have a sibling, process that */
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				/* Ascend tree */
				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਜੜ੍ਹ);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ਸਥਿਰ ਖਾਲੀ urldb_save_ਖੋਜ_ਰੁੱਖ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਮਾਪਾ, FILE *fp)
{
	ਅੱਖਰ host[256];
	ਅਚਲ ਬਣਤਰ host_part *h;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਰਸਤਾ_ਗਿਣਤੀ = 0;
	ਅੱਖਰ *ਰਸਤਾ, *p, *ਸਿਰਾ;
	ਪੂਰਨ ਰਸਤਾ_alloc = 64, ਰਸਤਾ_ਵਰਤਿਆ = 1;
	time_t expiry, hsts_expiry = 0;
	ਪੂਰਨ hsts_include_subdomains = 0;

	expiry = time(ਨਿਰਾ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ਜੇ (ਮਾਪਾ == &ਖਾਲੀ_ਥਾਵਾਂ)
		ਵਾਪਸ;

	urldb_save_ਖੋਜ_ਰੁੱਖ(ਮਾਪਾ->ਖੱਬਾ, fp);

	ਰਸਤਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਰਸਤਾ_alloc);
	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ;

	ਰਸਤਾ[0] = '\0';

	ਲਈ (h = ਮਾਪਾ->ਅੰਕੜੇ, p = host, ਸਿਰਾ = host + ਆਕਾਰ host;
	     h && h != &db_ਜੜ੍ਹ && p < ਸਿਰਾ; h = h->ਮਾਪਾ) {
		ਪੂਰਨ written = snprintf(p, ਸਿਰਾ - p, "%s%s", h->part,
				       (h->ਮਾਪਾ && h->ਮਾਪਾ->ਮਾਪਾ) ? "." : "");
		ਜੇ (written < 0) {
			ਮੁਕਤ(ਰਸਤਾ);
			ਵਾਪਸ;
		}
		p += written;
	}

	h = ਮਾਪਾ->ਅੰਕੜੇ;
	ਜੇ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_ਗਿਣਤੀ_urls(&ਮਾਪਾ->ਅੰਕੜੇ->paths, expiry, &ਰਸਤਾ_ਗਿਣਤੀ);

	ਜੇ (ਰਸਤਾ_ਗਿਣਤੀ > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", ਰਸਤਾ_ਗਿਣਤੀ);

		urldb_write_paths(&ਮਾਪਾ->ਅੰਕੜੇ->paths, host, fp,
				  &ਰਸਤਾ, &ਰਸਤਾ_alloc, &ਰਸਤਾ_ਵਰਤਿਆ, expiry);
	} ਨਹੀਂਤਾਂ ਜੇ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	ਮੁਕਤ(ਰਸਤਾ);

	urldb_save_ਖੋਜ_ਰੁੱਖ(ਮਾਪਾ->ਸੱਜਾ, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ਸਥਿਰ bool
urldb_iterate_entries_ਰਸਤਾ(ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ,
		bool (*url_callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ),
		bool (*cookie_callback)(ਅਚਲ ਬਣਤਰ cookie_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ;
	ਅਚਲ ਬਣਤਰ cookie_ਅੰਕੜੇ *c;

	ਕਰੋ {
		ਜੇ (p->children != ਨਿਰਾ) {
			/* Drill down into children */
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ਪ੍ਰਤਿਜ੍ਞਾ(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ਜੇ (url_callback) {
				ਅਚਲ ਬਣਤਰ url_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *u = &p->urld;

				ਪ੍ਰਤਿਜ੍ਞਾ(p->url);

				ਜੇ (!url_callback(p->url,
						  (ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *) u))
					ਵਾਪਸ false;
			} ਨਹੀਂਤਾਂ {
				c = (ਅਚਲ ਬਣਤਰ cookie_ਅੰਕੜੇ *)p->cookies;
				ਲਈ (; c != ਨਿਰਾ; c = c->ਅਗਲਾ) {
					ਜੇ (!cookie_callback(c))
						ਵਾਪਸ false;
				}
			}

			/* Now, find next node to process. */
			ਜਦੋਂ (p != ਮਾਪਾ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					/* Have a sibling, process that */
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				/* Ascend tree */
				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਮਾਪਾ);

	ਵਾਪਸ true;
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
ਸਥਿਰ bool urldb_host_is_ip_ਪਤਾ(ਅਚਲ ਅੱਖਰ *host)
{
	ਬਣਤਰ ਵਿੱਚ_addr ipv4;
	size_t host_len = ਲੜੀ_ਲੰਬਾਈ(host);
	ਅਚਲ ਅੱਖਰ *sane_host;
	ਅਚਲ ਅੱਖਰ *slash;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_IPV6
	ਬਣਤਰ in6_addr ipv6;
	ਅੱਖਰ ipv6_addr[64];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ipv6_addr_len;
#ਜੇ_ਅੰਤ
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
	ਜੇ (slash == ਨਿਰਾ) {
		sane_host = host;
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ਜੇ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		ਜਾਓ ਬਾਹਰਲਾ_false;

	ਜੇ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		ਪੂਰਨ ਅੰਕ_dots = 0;
		size_t index;

		ਲਈ (index = 0; index < host_len; index++) {
			ਜੇ (sane_host[index] == '.')
				ਅੰਕ_dots++;
		}

		ਜੇ (ਅੰਕ_dots == 3)
			ਜਾਓ ਬਾਹਰਲਾ_true;
		ਨਹੀਂਤਾਂ
			ਜਾਓ ਬਾਹਰਲਾ_false;
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_IPV6
	ਜੇ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		ਜਾਓ ਬਾਹਰਲਾ_false;
	}

	ipv6_addr_len = host_len - 2;
	ਜੇ (ipv6_addr_len >= ਆਕਾਰ(ipv6_addr)) {
		ipv6_addr_len = ਆਕਾਰ(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ਜੇ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		ਜਾਓ ਬਾਹਰਲਾ_true;
#ਜੇ_ਅੰਤ

ਬਾਹਰਲਾ_false:
	ਜੇ (slash != ਨਿਰਾ) ਮੁਕਤ((ਖਾਲੀ *)sane_host);
	ਵਾਪਸ false;

ਬਾਹਰਲਾ_true:
	ਜੇ (slash != ਨਿਰਾ) ਮੁਕਤ((ਖਾਲੀ *)sane_host);
	ਵਾਪਸ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ਸਥਿਰ ਪੂਰਨ urldb_ਖੋਜ_ਮੇਲ_ਅਗੇਤਰ(ਅਚਲ ਬਣਤਰ host_part *a, ਅਚਲ ਅੱਖਰ *b)
{
	ਅਚਲ ਅੱਖਰ *ਸਿਰਾ, *dot;
	ਪੂਰਨ plen, ret;

	ਪ੍ਰਤਿਜ੍ਞਾ(a && a != &db_ਜੜ੍ਹ && b);

	ਜੇ (urldb_host_is_ip_ਪਤਾ(b)) {
		/* IP address */
		ਵਾਪਸ strncasecmp(a->part, b, ਲੜੀ_ਲੰਬਾਈ(b));
	}

	ਸਿਰਾ = b + ਲੜੀ_ਲੰਬਾਈ(b) + 1;

	ਜਦੋਂ (b < ਸਿਰਾ && a && a != &db_ਜੜ੍ਹ) {
		dot = strchr(b, '.');
		ਜੇ (!dot) {
			/* last segment */
			dot = ਸਿਰਾ - 1;
		}

		/* Compare strings (length limited) */
		ਜੇ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ਵਾਪਸ ret;

		/* The strings matched */
		ਜੇ (dot < ਸਿਰਾ - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = ਲੜੀ_ਲੰਬਾਈ(a->part);
			ਜੇ (plen > dot - b) {
				/* len(a) > len(b) */
				ਵਾਪਸ 1;
			} ਨਹੀਂਤਾਂ ਜੇ (plen < dot - b) {
				/* len(a) < len(b) */
				ਵਾਪਸ -1;
			}
		}

		b = dot + 1;
		a = a->ਮਾਪਾ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ਜੇ (a && a != &db_ਜੜ੍ਹ && b >= ਸਿਰਾ) {
		/* len(a) > len(b) => prefix matches */
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ ਜੇ ((!a || a == &db_ਜੜ੍ਹ) && b < ਸਿਰਾ) {
		/* len(a) < len(b) => prefix does not match */
		ਵਾਪਸ -1;
	}

	/* Identical */
	ਵਾਪਸ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ਸਥਿਰ bool
urldb_iterate_partial_host(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ,
		ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ,
		bool (*callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਪੂਰਨ c;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ && ਅਗੇਤਰ && callback);

	ਜੇ (ਜੜ੍ਹ == &ਖਾਲੀ_ਥਾਵਾਂ)
		ਵਾਪਸ true;

	c = urldb_ਖੋਜ_ਮੇਲ_ਅਗੇਤਰ(ਜੜ੍ਹ->ਅੰਕੜੇ, ਅਗੇਤਰ);

	ਜੇ (c > 0) {
		/* No match => look in left subtree */
		ਵਾਪਸ urldb_iterate_partial_host(ਜੜ੍ਹ->ਖੱਬਾ,
						  ਅਗੇਤਰ,
						  callback);
	} ਨਹੀਂਤਾਂ ਜੇ (c < 0) {
		/* No match => look in right subtree */
		ਵਾਪਸ urldb_iterate_partial_host(ਜੜ੍ਹ->ਸੱਜਾ,
						  ਅਗੇਤਰ,
						  callback);
	} ਨਹੀਂਤਾਂ {
		/* Match => iterate over l/r subtrees & process this node */
		ਜੇ (!urldb_iterate_partial_host(ਜੜ੍ਹ->ਖੱਬਾ,
						ਅਗੇਤਰ,
						callback)) {
			ਵਾਪਸ false;
		}

		ਜੇ (ਜੜ੍ਹ->ਅੰਕੜੇ->paths.children) {
			/* and extract all paths attached to this host */
			ਜੇ (!urldb_iterate_entries_ਰਸਤਾ(&ਜੜ੍ਹ->ਅੰਕੜੇ->paths,
							callback,
							ਨਿਰਾ)) {
				ਵਾਪਸ false;
			}
		}

		ਜੇ (!urldb_iterate_partial_host(ਜੜ੍ਹ->ਸੱਜਾ,
						ਅਗੇਤਰ,
						callback)) {
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
urldb_iterate_partial_ਰਸਤਾ(ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ,
		ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ,
		bool (*callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ->children;
	ਅਚਲ ਅੱਖਰ *slash, *ਸਿਰਾ = ਅਗੇਤਰ + ਲੜੀ_ਲੰਬਾਈ(ਅਗੇਤਰ);

	ਕਰੋ {
		slash = strchr(ਅਗੇਤਰ, '/');
		ਜੇ (!slash) {
			slash = ਸਿਰਾ;
		}

		ਜੇ (slash == ਅਗੇਤਰ && *ਅਗੇਤਰ == '/') {
			/* Ignore "//" */
			ਅਗੇਤਰ++;
			ਜਾਰੀ;
		}

		ਜੇ (strncasecmp(p->segment, ਅਗੇਤਰ, slash - ਅਗੇਤਰ) == 0) {
			/* prefix matches so far */
			ਜੇ (slash == ਸਿਰਾ) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ਜੇ (!urldb_iterate_entries_ਰਸਤਾ(p,
								callback,
								ਨਿਰਾ)) {
					ਵਾਪਸ false;
				}

				/* Progress to next sibling */
				p = p->ਅਗਲਾ;
			} ਨਹੀਂਤਾਂ {
				/* Skip over this segment */
				ਅਗੇਤਰ = slash + 1;

				p = p->children;
			}
		} ਨਹੀਂਤਾਂ {
			/* Doesn't match this segment, try next sibling */
			p = p->ਅਗਲਾ;
		}
	} ਜਦੋਂ (p != ਨਿਰਾ);

	ਵਾਪਸ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ਸਥਿਰ bool
urldb_iterate_entries_host(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਮਾਪਾ,
		bool (*url_callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ),
		bool (*cookie_callback)(ਅਚਲ ਬਣਤਰ cookie_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਜੇ (ਮਾਪਾ == &ਖਾਲੀ_ਥਾਵਾਂ) {
		ਵਾਪਸ true;
	}

	ਜੇ (!urldb_iterate_entries_host(ਮਾਪਾ->ਖੱਬਾ,
					url_callback,
					cookie_callback)) {
		ਵਾਪਸ false;
	}

	ਜੇ ((ਮਾਪਾ->ਅੰਕੜੇ->paths.children) ||
	    ((cookie_callback) &&
	     (ਮਾਪਾ->ਅੰਕੜੇ->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ਜੇ (!urldb_iterate_entries_ਰਸਤਾ(&ਮਾਪਾ->ਅੰਕੜੇ->paths,
						url_callback,
						cookie_callback)) {
			ਵਾਪਸ false;
		}
	}

	ਜੇ (!urldb_iterate_entries_host(ਮਾਪਾ->ਸੱਜਾ,
					url_callback,
					cookie_callback)) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ਸਥਿਰ ਬਣਤਰ host_part *
urldb_ਜੋੜੋ_host_ਗੰਢ(ਅਚਲ ਅੱਖਰ *part, ਬਣਤਰ host_part *ਮਾਪਾ)
{
	ਬਣਤਰ host_part *d;

	ਪ੍ਰਤਿਜ੍ਞਾ(part && ਮਾਪਾ);

	d = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ host_part));
	ਜੇ (!d) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	d->part = strdup(part);
	ਜੇ (!d->part) {
		ਮੁਕਤ(d);
		ਵਾਪਸ ਨਿਰਾ;
	}

	d->ਅਗਲਾ = ਮਾਪਾ->children;
	ਜੇ (ਮਾਪਾ->children) {
		ਮਾਪਾ->children->prev = d;
	}
	d->ਮਾਪਾ = ਮਾਪਾ;
	ਮਾਪਾ->children = d;

	ਵਾਪਸ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ਸਥਿਰ ਪੂਰਨ urldb_ਜੋੜੋ_ਰਸਤਾ_fragment_cmp(ਅਚਲ ਖਾਲੀ *a, ਅਚਲ ਖਾਲੀ *b)
{
	ਵਾਪਸ strcasecmp(*((ਅਚਲ ਅੱਖਰ **) a), *((ਅਚਲ ਅੱਖਰ **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *
urldb_ਜੋੜੋ_ਰਸਤਾ_fragment(ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *segment, lwc_string *fragment)
{
	ਅੱਖਰ **ਆਰਜ਼ੀ;

	ਪ੍ਰਤਿਜ੍ਞਾ(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ਜੇ (!fragment)
		ਵਾਪਸ segment;

	ਆਰਜ਼ੀ = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * ਆਕਾਰ(ਅੱਖਰ *));
	ਜੇ (!ਆਰਜ਼ੀ)
		ਵਾਪਸ ਨਿਰਾ;

	segment->fragment = ਆਰਜ਼ੀ;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_ਅੰਕੜੇ(fragment));
	ਜੇ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ਵਾਪਸ ਨਿਰਾ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      ਆਕਾਰ (ਅੱਖਰ *),
	      urldb_ਜੋੜੋ_ਰਸਤਾ_fragment_cmp);

	ਵਾਪਸ segment;
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
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *
urldb_ਜੋੜੋ_ਰਸਤਾ_ਗੰਢ(lwc_string *scheme,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ,
		    ਅਚਲ ਅੱਖਰ *segment,
		    lwc_string *fragment,
		    ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *d, *e;

	ਪ੍ਰਤਿਜ੍ਞਾ(scheme && segment && ਮਾਪਾ);

	d = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ));
	ਜੇ (!d)
		ਵਾਪਸ ਨਿਰਾ;

	d->scheme = lwc_string_ref(scheme);

	d->ਪੋਰਟ = ਪੋਰਟ;

	d->segment = strdup(segment);
	ਜੇ (!d->segment) {
		lwc_string_unref(d->scheme);
		ਮੁਕਤ(d);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (fragment) {
		ਜੇ (!urldb_ਜੋੜੋ_ਰਸਤਾ_fragment(d, fragment)) {
			ਮੁਕਤ(d->segment);
			lwc_string_unref(d->scheme);
			ਮੁਕਤ(d);
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	ਲਈ (e = ਮਾਪਾ->children; e; e = e->ਅਗਲਾ) {
		ਜੇ (strcmp(e->segment, d->segment) > 0)
			ਰੋਕੋ;
	}

	ਜੇ (e) {
		d->prev = e->prev;
		d->ਅਗਲਾ = e;
		ਜੇ (e->prev)
			e->prev->ਅਗਲਾ = d;
		ਨਹੀਂਤਾਂ
			ਮਾਪਾ->children = d;
		e->prev = d;
	} ਨਹੀਂਤਾਂ ਜੇ (!ਮਾਪਾ->children) {
		d->prev = d->ਅਗਲਾ = ਨਿਰਾ;
		ਮਾਪਾ->children = ਮਾਪਾ->ਆਖਰੀ = d;
	} ਨਹੀਂਤਾਂ {
		d->ਅਗਲਾ = ਨਿਰਾ;
		d->prev = ਮਾਪਾ->ਆਖਰੀ;
		ਮਾਪਾ->ਆਖਰੀ->ਅਗਲਾ = d;
		ਮਾਪਾ->ਆਖਰੀ = d;
	}
	d->ਮਾਪਾ = ਮਾਪਾ;

	ਵਾਪਸ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ **urldb_get_ਖੋਜ_ਰੁੱਖ_ਸਿੱਧਾ(ਅਚਲ ਅੱਖਰ *host)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(host);

	ਜੇ (urldb_host_is_ip_ਪਤਾ(host)) {
		ਵਾਪਸ &ਖੋਜ_trees[ST_IP];
	} ਨਹੀਂਤਾਂ ਜੇ (ascii_is_alpha(*host)) {
		ਵਾਪਸ &ਖੋਜ_trees[ST_DN + ascii_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(*host) - 'a'];
	}
	ਵਾਪਸ &ਖੋਜ_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *urldb_get_ਖੋਜ_ਰੁੱਖ(ਅਚਲ ਅੱਖਰ *host)
{
	ਵਾਪਸ *urldb_get_ਖੋਜ_ਰੁੱਖ_ਸਿੱਧਾ(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ਸਥਿਰ ਪੂਰਨ urldb_ਖੋਜ_ਮੇਲ_string(ਅਚਲ ਬਣਤਰ host_part *a, ਅਚਲ ਅੱਖਰ *b)
{
	ਅਚਲ ਅੱਖਰ *ਸਿਰਾ, *dot;
	ਪੂਰਨ plen, ret;

	ਪ੍ਰਤਿਜ੍ਞਾ(a && a != &db_ਜੜ੍ਹ && b);

	ਜੇ (urldb_host_is_ip_ਪਤਾ(b)) {
		/* IP address */
		ਵਾਪਸ strcasecmp(a->part, b);
	}

	ਸਿਰਾ = b + ਲੜੀ_ਲੰਬਾਈ(b) + 1;

	ਜਦੋਂ (b < ਸਿਰਾ && a && a != &db_ਜੜ੍ਹ) {
		dot = strchr(b, '.');
		ਜੇ (!dot) {
			/* last segment */
			dot = ਸਿਰਾ - 1;
		}

		/* Compare strings (length limited) */
		ਜੇ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ਵਾਪਸ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = ਲੜੀ_ਲੰਬਾਈ(a->part);

		ਜੇ (plen > dot - b) {
			/* len(a) > len(b) */
			ਵਾਪਸ 1;
		} ਨਹੀਂਤਾਂ ਜੇ (plen < dot - b) {
			/* len(a) < len(b) */
			ਵਾਪਸ -1;
		}

		b = dot + 1;
		a = a->ਮਾਪਾ;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ਜੇ (a && a != &db_ਜੜ੍ਹ && b >= ਸਿਰਾ) {
		/* len(a) > len(b) */
		ਵਾਪਸ 1;
	} ਨਹੀਂਤਾਂ ਜੇ ((!a || a == &db_ਜੜ੍ਹ) && b < ਸਿਰਾ) {
		/* len(a) < len(b) */
		ਵਾਪਸ -1;
	}

	/* Identical */
	ਵਾਪਸ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ host_part *
urldb_ਖੋਜ_ਲੱਭੋ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ, ਅਚਲ ਅੱਖਰ *host)
{
	ਪੂਰਨ c;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ && host);

	ਜੇ (ਜੜ੍ਹ == &ਖਾਲੀ_ਥਾਵਾਂ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	c = urldb_ਖੋਜ_ਮੇਲ_string(ਜੜ੍ਹ->ਅੰਕੜੇ, host);

	ਜੇ (c > 0) {
		ਵਾਪਸ urldb_ਖੋਜ_ਲੱਭੋ(ਜੜ੍ਹ->ਖੱਬਾ, host);
	} ਨਹੀਂਤਾਂ ਜੇ (c < 0) {
		ਵਾਪਸ urldb_ਖੋਜ_ਲੱਭੋ(ਜੜ੍ਹ->ਸੱਜਾ, host);
	}

	ਵਾਪਸ ਜੜ੍ਹ->ਅੰਕੜੇ;
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
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *
urldb_ਮੇਲ_ਰਸਤਾ(ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ,
		 ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
		 lwc_string *scheme,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਪੋਰਟ)
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਅਚਲ ਅੱਖਰ *slash;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ->segment == ਨਿਰਾ);

	ਜੇ (ਰਸਤਾ[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", ਰਸਤਾ);
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(ਰਸਤਾ[0] == '/');

	/* Start with children, as parent has no segment */
	p = ਮਾਪਾ->children;

	ਜਦੋਂ (p != ਨਿਰਾ) {
		slash = strchr(ਰਸਤਾ + 1, '/');
		ਜੇ (!slash) {
			slash = ਰਸਤਾ + ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ);
		}

		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(p->segment, ਰਸਤਾ + 1, slash - ਰਸਤਾ - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &ਮੇਲ) == lwc_error_ok &&
		    ਮੇਲ == true &&
		    p->ਪੋਰਟ == ਪੋਰਟ) {
			ਜੇ (*slash == '\0') {
				/* Complete match */
				ਵਾਪਸ (ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			ਰਸਤਾ = slash;
		} ਨਹੀਂਤਾਂ {
			/* No match, try next sibling */
			p = p->ਅਗਲਾ;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *urldb_ਲੱਭੋ_url(nsurl *url)
{
	ਅਚਲ ਬਣਤਰ host_part *h;
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਰੁੱਖ;
	ਅੱਖਰ *plq;
	ਅਚਲ ਅੱਖਰ *host_str;
	lwc_string *scheme, *host, *ਪੋਰਟ;
	size_t len = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ_int;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	ਜੇ (url_bloom != ਨਿਰਾ) {
		ਜੇ (bloom_ਖੋਜ_hash(url_bloom, nsurl_hash(url)) == false) {
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (scheme == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (lwc_string_isequal(scheme, corestring_lwc_mailto, &ਮੇਲ) ==
	    lwc_error_ok && ਮੇਲ == true) {
		lwc_string_unref(scheme);
		ਵਾਪਸ ਨਿਰਾ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (host != ਨਿਰਾ) {
		host_str = lwc_string_ਅੰਕੜੇ(host);
		lwc_string_unref(host);

	} ਨਹੀਂਤਾਂ ਜੇ (lwc_string_isequal(scheme, corestring_lwc_file, &ਮੇਲ) ==
		   lwc_error_ok && ਮੇਲ == true) {
		host_str = "localhost";

	} ਨਹੀਂਤਾਂ {
		lwc_string_unref(scheme);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਰੁੱਖ = urldb_get_ਖੋਜ_ਰੁੱਖ(host_str);
	h = urldb_ਖੋਜ_ਲੱਭੋ(ਰੁੱਖ, host_str);
	ਜੇ (!h) {
		lwc_string_unref(scheme);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* generate plq (path, leaf, query) */
	ਜੇ (nsurl_get(url, NSURL_ਰਸਤਾ | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Get port */
	ਪੋਰਟ = nsurl_get_component(url, NSURL_ਪੋਰਟ);
	ਜੇ (ਪੋਰਟ != ਨਿਰਾ) {
		ਪੋਰਟ_int = atoi(lwc_string_ਅੰਕੜੇ(ਪੋਰਟ));
		lwc_string_unref(ਪੋਰਟ);
	} ਨਹੀਂਤਾਂ {
		ਪੋਰਟ_int = 0;
	}

	p = urldb_ਮੇਲ_ਰਸਤਾ(&h->paths, plq, scheme, ਪੋਰਟ_int);

	ਮੁਕਤ(plq);
	lwc_string_unref(scheme);

	ਵਾਪਸ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ਸਥਿਰ ਖਾਲੀ urldb_dump_paths(ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ)
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਕਰੋ {
		ਜੇ (p->segment != ਨਿਰਾ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_ਅੰਕੜੇ(p->scheme), p->ਪੋਰਟ);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			ਲਈ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ਜੇ (p->children != ਨਿਰਾ) {
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			ਜਦੋਂ (p != ਮਾਪਾ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਮਾਪਾ);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ਸਥਿਰ ਖਾਲੀ urldb_dump_hosts(ਬਣਤਰ host_part *ਮਾਪਾ)
{
	ਬਣਤਰ host_part *h;

	ਜੇ (ਮਾਪਾ->part) {
		NSLOG(netsurf, INFO, "%s", ਮਾਪਾ->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      ਮਾਪਾ->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&ਮਾਪਾ->paths);

	/* and recurse */
	ਲਈ (h = ਮਾਪਾ->children; h; h = h->ਅਗਲਾ) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ਸਥਿਰ ਖਾਲੀ urldb_dump_ਖੋਜ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਮਾਪਾ, ਪੂਰਨ ਡੂੰਘਾਈ)
{
	ਅਚਲ ਬਣਤਰ host_part *h;
	ਪੂਰਨ i; /* index into string */
	ਅੱਖਰ s[1024];
	ਪੂਰਨ r;
	ਪੂਰਨ sl = ਆਕਾਰ(s) - 2;

	ਜੇ (ਮਾਪਾ == &ਖਾਲੀ_ਥਾਵਾਂ)
		ਵਾਪਸ;

	urldb_dump_ਖੋਜ(ਮਾਪਾ->ਖੱਬਾ, ਡੂੰਘਾਈ + 1);

	ਲਈ (i = 0; i != ਡੂੰਘਾਈ; i++) {
		s[i] = ' ';
	}

	ਲਈ (h = ਮਾਪਾ->ਅੰਕੜੇ; h; h = h->ਮਾਪਾ) {
		ਜੇ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ਜੇ (r < 0) {
				ਰੋਕੋ;
			}
			ਜੇ ((i + r) >= sl) {
				ਰੋਕੋ;
			}
			i += r;
		}

		ਜੇ (h->ਮਾਪਾ && h->ਮਾਪਾ->ਮਾਪਾ) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_ਖੋਜ(ਮਾਪਾ->ਸੱਜਾ, ਡੂੰਘਾਈ + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ਸਥਿਰ ਪੂਰਨ
urldb_ਖੋਜ_ਮੇਲ_host(ਅਚਲ ਬਣਤਰ host_part *a, ਅਚਲ ਬਣਤਰ host_part *b)
{
	ਪੂਰਨ ret;

	ਪ੍ਰਤਿਜ੍ਞਾ(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	ਲਈ (; a && a != &db_ਜੜ੍ਹ && b && b != &db_ਜੜ੍ਹ;
	     a = a->ਮਾਪਾ, b = b->ਮਾਪਾ) {
		ਜੇ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ਵਾਪਸ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ਜੇ (a && a != &db_ਜੜ੍ਹ && (!b || b == &db_ਜੜ੍ਹ)) {
		/* len(a) > len(b) */
		ਵਾਪਸ 1;
	} ਨਹੀਂਤਾਂ ਜੇ ((!a || a == &db_ਜੜ੍ਹ) && b && b != &db_ਜੜ੍ਹ) {
		/* len(a) < len(b) */
		ਵਾਪਸ -1;
	}

	/* identical */
	ਵਾਪਸ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *urldb_ਖੋਜ_skew(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ);

	ਜੇ (ਜੜ੍ਹ->ਖੱਬਾ->level == ਜੜ੍ਹ->level) {
		ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਆਰਜ਼ੀ;

		ਆਰਜ਼ੀ = ਜੜ੍ਹ->ਖੱਬਾ;
		ਜੜ੍ਹ->ਖੱਬਾ = ਆਰਜ਼ੀ->ਸੱਜਾ;
		ਆਰਜ਼ੀ->ਸੱਜਾ = ਜੜ੍ਹ;
		ਜੜ੍ਹ = ਆਰਜ਼ੀ;
	}

	ਵਾਪਸ ਜੜ੍ਹ;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *urldb_ਖੋਜ_ਵੰਡੋ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ);

	ਜੇ (ਜੜ੍ਹ->ਸੱਜਾ->ਸੱਜਾ->level == ਜੜ੍ਹ->level) {
		ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਆਰਜ਼ੀ;

		ਆਰਜ਼ੀ = ਜੜ੍ਹ->ਸੱਜਾ;
		ਜੜ੍ਹ->ਸੱਜਾ = ਆਰਜ਼ੀ->ਖੱਬਾ;
		ਆਰਜ਼ੀ->ਖੱਬਾ = ਜੜ੍ਹ;
		ਜੜ੍ਹ = ਆਰਜ਼ੀ;

		ਜੜ੍ਹ->level++;
	}

	ਵਾਪਸ ਜੜ੍ਹ;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *
urldb_ਖੋਜ_ਪਾਓ_ਅੰਦਰੂਨੀ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ, ਬਣਤਰ ਖੋਜ_ਗੰਢ *n)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ && n);

	ਜੇ (ਜੜ੍ਹ == &ਖਾਲੀ_ਥਾਵਾਂ) {
		ਜੜ੍ਹ = n;
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ c = urldb_ਖੋਜ_ਮੇਲ_host(ਜੜ੍ਹ->ਅੰਕੜੇ, n->ਅੰਕੜੇ);

		ਜੇ (c > 0) {
			ਜੜ੍ਹ->ਖੱਬਾ = urldb_ਖੋਜ_ਪਾਓ_ਅੰਦਰੂਨੀ(
				ਜੜ੍ਹ->ਖੱਬਾ, n);
		} ਨਹੀਂਤਾਂ ਜੇ (c < 0) {
			ਜੜ੍ਹ->ਸੱਜਾ = urldb_ਖੋਜ_ਪਾਓ_ਅੰਦਰੂਨੀ(
				ਜੜ੍ਹ->ਸੱਜਾ, n);
		} ਨਹੀਂਤਾਂ {
			/* exact match */
			ਮੁਕਤ(n);
			ਵਾਪਸ ਜੜ੍ਹ;
		}

		ਜੜ੍ਹ = urldb_ਖੋਜ_skew(ਜੜ੍ਹ);
		ਜੜ੍ਹ = urldb_ਖੋਜ_ਵੰਡੋ(ਜੜ੍ਹ);
	}

	ਵਾਪਸ ਜੜ੍ਹ;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ਸਥਿਰ ਬਣਤਰ ਖੋਜ_ਗੰਢ *
urldb_ਖੋਜ_ਪਾਓ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ, ਅਚਲ ਬਣਤਰ host_part *ਅੰਕੜੇ)
{
	ਬਣਤਰ ਖੋਜ_ਗੰਢ *n;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ && ਅੰਕੜੇ);

	n = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ ਖੋਜ_ਗੰਢ));
	ਜੇ (!n)
		ਵਾਪਸ ਨਿਰਾ;

	n->level = 1;
	n->ਅੰਕੜੇ = ਅੰਕੜੇ;
	n->ਖੱਬਾ = n->ਸੱਜਾ = &ਖਾਲੀ_ਥਾਵਾਂ;

	ਜੜ੍ਹ = urldb_ਖੋਜ_ਪਾਓ_ਅੰਦਰੂਨੀ(ਜੜ੍ਹ, n);

	ਵਾਪਸ ਜੜ੍ਹ;
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
ਸਥਿਰ bool
urldb_parse_avpair(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c,
		   ਅੱਖਰ *n,
		   ਅੱਖਰ *v,
		   bool was_quoted)
{
	ਪੂਰਨ vlen;

	ਪ੍ਰਤਿਜ੍ਞਾ(c && n && v);

	/* Strip whitespace from start of name */
	ਲਈ (; *n; n++) {
		ਜੇ (*n != ' ' && *n != '\t')
			ਰੋਕੋ;
	}

	/* Strip whitespace from end of name */
	ਲਈ (vlen = ਲੜੀ_ਲੰਬਾਈ(n); vlen; vlen--) {
		ਜੇ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ਨਹੀਂਤਾਂ
			ਰੋਕੋ;
	}

	/* Strip whitespace from start of value */
	ਲਈ (; *v; v++) {
		ਜੇ (*v != ' ' && *v != '\t')
			ਰੋਕੋ;
	}

	/* Strip whitespace from end of value */
	ਲਈ (vlen = ਲੜੀ_ਲੰਬਾਈ(v); vlen; vlen--) {
		ਜੇ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ਨਹੀਂਤਾਂ
			ਰੋਕੋ;
	}

	ਜੇ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ਜੇ (!c->comment)
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ ਜੇ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ਜੇ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_ਸੈੱਟ = true;
			c->domain = strdup(v);
			ਜੇ (!c->domain)
				ਵਾਪਸ false;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(n, "Max-Age") == 0) {
		ਪੂਰਨ ਆਰਜ਼ੀ = atoi(v);
		ਜੇ (ਆਰਜ਼ੀ == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ਨਹੀਂਤਾਂ
			c->expires = time(ਨਿਰਾ) + ਆਰਜ਼ੀ;
	} ਨਹੀਂਤਾਂ ਜੇ (!c->ਰਸਤਾ && strcasecmp(n, "Path") == 0) {
		c->ਰਸਤਾ_from_ਸੈੱਟ = true;
		c->ਰਸਤਾ = strdup(v);
		ਜੇ (!c->ਰਸਤਾ)
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(n, "Version") == 0) {
		c->ਸੰਸਕਰਣ = atoi(v);
	} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(n, "Expires") == 0) {
		ਅੱਖਰ *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		ਲਈ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, ਲੜੀ_ਲੰਬਾਈ(datenoday), &expires);
		ਜੇ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ਨਹੀਂਤਾਂ ਜੇ (!c->name) {
		c->name = strdup(n);
		c->ਮੁੱਲ = strdup(v);
		c->ਮੁੱਲ_was_quoted = was_quoted;
		ਜੇ (!c->name || !c->ਮੁੱਲ) {
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ਸਥਿਰ ਖਾਲੀ urldb_free_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(c);

	ਮੁਕਤ(c->comment);
	ਮੁਕਤ(c->domain);
	ਮੁਕਤ(c->ਰਸਤਾ);
	ਮੁਕਤ(c->name);
	ਮੁਕਤ(c->ਮੁੱਲ);
	ਮੁਕਤ(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ਸਥਿਰ ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *
urldb_parse_cookie(nsurl *url, ਅਚਲ ਅੱਖਰ **cookie)
{
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;
	ਅਚਲ ਅੱਖਰ *cur;
	ਅੱਖਰ name[1024], ਮੁੱਲ[4096];
	ਅੱਖਰ *n = name, *v = ਮੁੱਲ;
	bool ਵਿੱਚ_ਮੁੱਲ = false;
	bool had_ਮੁੱਲ_ਅੰਕੜੇ = false;
	bool ਮੁੱਲ_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ਪ੍ਰਤਿਜ੍ਞਾ(url && cookie && *cookie);

	c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ));
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	c->expires = -1;

	name[0] = '\0';
	ਮੁੱਲ[0] = '\0';

	ਲਈ (cur = *cookie; *cur; cur++) {
		ਜੇ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ਜੇ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				ਮੁੱਲ[0] = '\0';

			}

			ਰੋਕੋ;
		} ਨਹੀਂਤਾਂ ਜੇ (*cur == '\r') {
			/* Spurious linefeed */
			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ (*cur == '\n') {
			/* Spurious newline */
			ਜਾਰੀ;
		}

		ਜੇ (ਵਿੱਚ_ਮੁੱਲ && !had_ਮੁੱਲ_ਅੰਕੜੇ) {
			ਜੇ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ {
				had_ਮੁੱਲ_ਅੰਕੜੇ = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ਜੇ (*cur != '"') {
					ਮੁੱਲ_verbatim = true;
				}
			}
		}

		ਜੇ (ਵਿੱਚ_ਮੁੱਲ && !ਮੁੱਲ_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ਜੇ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ਜਾਰੀ;
			}
		}

		ਜੇ (!quoted && !ਵਿੱਚ_ਮੁੱਲ && *cur == '=') {
			/* First equals => attr-value separator */
			ਵਿੱਚ_ਮੁੱਲ = true;
			ਜਾਰੀ;
		}

		ਜੇ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ਜੇ (!urldb_parse_avpair(c, name, ਮੁੱਲ, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ਵਾਪਸ ਨਿਰਾ;
			}

			/* And reset to start */
			n = name;
			v = ਮੁੱਲ;
			ਵਿੱਚ_ਮੁੱਲ = false;
			had_ਮੁੱਲ_ਅੰਕੜੇ = false;
			ਮੁੱਲ_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ਜੇ (*cur != ';') {
				cur--;
			}

			ਜਾਰੀ;
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
		ਜੇ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ਅਚਲ ਅੱਖਰ *p;
			ਅਚਲ ਅੱਖਰ *semi = strchr(cur + 1, ';');
			ਜੇ (!semi)
				semi = cur + ਲੜੀ_ਲੰਬਾਈ(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			ਲਈ (p = cur + 1; p < semi; p++)
				ਜੇ (*p == '=')
					ਰੋਕੋ;

			ਜੇ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ਨਹੀਂਤਾਂ {
				/* found one => comma marks end of cookie */
				cur++;
				ਰੋਕੋ;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ਜੇ (!ਵਿੱਚ_ਮੁੱਲ) {
			ਜੇ (n < name + (ਆਕਾਰ(name) - 1))
				*n++ = *cur;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (v < ਮੁੱਲ + (ਆਕਾਰ(ਮੁੱਲ) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ਜੇ (!urldb_parse_avpair(c, name, ਮੁੱਲ, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Now fix-up default values */
	ਜੇ (c->domain == ਨਿਰਾ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ਜੇ (host == ਨਿਰਾ) {
			urldb_free_cookie(c);
			ਵਾਪਸ ਨਿਰਾ;
		}
		c->domain = strdup(lwc_string_ਅੰਕੜੇ(host));
		lwc_string_unref(host);
	}

	ਜੇ (c->ਰਸਤਾ == ਨਿਰਾ) {
		ਅਚਲ ਅੱਖਰ *ਰਸਤਾ_ਅੰਕੜੇ;
		ਅੱਖਰ *ਰਸਤਾ, *slash;
		lwc_string *ਰਸਤਾ_lwc;

		ਰਸਤਾ_lwc = nsurl_get_component(url, NSURL_ਰਸਤਾ);
		ਜੇ (ਰਸਤਾ_lwc == ਨਿਰਾ) {
			urldb_free_cookie(c);
			ਵਾਪਸ ਨਿਰਾ;
		}
		ਰਸਤਾ_ਅੰਕੜੇ = lwc_string_ਅੰਕੜੇ(ਰਸਤਾ_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(ਰਸਤਾ_ਅੰਕੜੇ, '/');
		ਜੇ (slash != ਨਿਰਾ) {
			/* Special case: retain first slash in path */
			ਜੇ (slash == ਰਸਤਾ_ਅੰਕੜੇ)
				slash++;

			slash = strndup(ਰਸਤਾ_ਅੰਕੜੇ, slash - ਰਸਤਾ_ਅੰਕੜੇ);
			ਜੇ (slash == ਨਿਰਾ) {
				lwc_string_unref(ਰਸਤਾ_lwc);
				urldb_free_cookie(c);
				ਵਾਪਸ ਨਿਰਾ;
			}

			ਰਸਤਾ = slash;
			lwc_string_unref(ਰਸਤਾ_lwc);
		} ਨਹੀਂਤਾਂ {
			ਰਸਤਾ = strdup(lwc_string_ਅੰਕੜੇ(ਰਸਤਾ_lwc));
			lwc_string_unref(ਰਸਤਾ_lwc);
			ਜੇ (ਰਸਤਾ == ਨਿਰਾ) {
				urldb_free_cookie(c);
				ਵਾਪਸ ਨਿਰਾ;
			}
		}

		c->ਰਸਤਾ = ਰਸਤਾ;
	}

	/* Write back current position */
	*cookie = cur;

	ਵਾਪਸ c;
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
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *
urldb_ਜੋੜੋ_ਰਸਤਾ(lwc_string *scheme,
	       ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ,
	       ਅਚਲ ਬਣਤਰ host_part *host,
	       ਅੱਖਰ *ਰਸਤਾ_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *d, *e;
	ਅੱਖਰ *buf = ਰਸਤਾ_query;
	ਅੱਖਰ *segment, *slash;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(scheme && host && url);

	d = (ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ਜੇ (*segment == '/')
		segment++;

	/* Process path segments */
	ਕਰੋ {
		slash = strchr(segment, '/');
		ਜੇ (!slash) {
			/* last segment */
			/* look for existing entry */
			ਲਈ (e = d->children; e; e = e->ਅਗਲਾ)
				ਜੇ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &ਮੇਲ) ==
				    lwc_error_ok &&
				    ਮੇਲ == true &&
				    e->ਪੋਰਟ == ਪੋਰਟ)
					ਰੋਕੋ;

			d = e ? urldb_ਜੋੜੋ_ਰਸਤਾ_fragment(e, fragment) :
				urldb_ਜੋੜੋ_ਰਸਤਾ_ਗੰਢ(scheme, ਪੋਰਟ,
						    segment, fragment, d);
			ਰੋਕੋ;
		}

		*slash = '\0';

		/* look for existing entry */
		ਲਈ (e = d->children; e; e = e->ਅਗਲਾ)
			ਜੇ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &ਮੇਲ) == lwc_error_ok &&
			    ਮੇਲ == true &&
			    e->ਪੋਰਟ == ਪੋਰਟ)
				ਰੋਕੋ;

		d = e ? e : urldb_ਜੋੜੋ_ਰਸਤਾ_ਗੰਢ(scheme, ਪੋਰਟ, segment, ਨਿਰਾ, d);
		ਜੇ (!d)
			ਰੋਕੋ;

		segment = slash + 1;
	} ਜਦੋਂ (1);

	ਮੁਕਤ(ਰਸਤਾ_query);

	ਜੇ (d && !d->url) {
		/* Insert defragmented URL */
		ਜੇ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ਸਥਿਰ ਬਣਤਰ host_part *urldb_ਜੋੜੋ_host(ਅਚਲ ਅੱਖਰ *host)
{
	ਬਣਤਰ host_part *d = (ਬਣਤਰ host_part *) &db_ਜੜ੍ਹ, *e;
	ਬਣਤਰ ਖੋਜ_ਗੰਢ *s;
	ਅੱਖਰ buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	ਅੱਖਰ *part;

	ਪ੍ਰਤਿਜ੍ਞਾ(host);

	ਜੇ (urldb_host_is_ip_ਪਤਾ(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		ਲਈ (e = d->children; e; e = e->ਅਗਲਾ)
			ਜੇ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ਵਾਪਸ e;

		d = urldb_ਜੋੜੋ_host_ਗੰਢ(host, d);

		s = urldb_ਖੋਜ_ਪਾਓ(ਖੋਜ_trees[ST_IP], d);
		ਜੇ (!s) {
			/* failed */
			d = ਨਿਰਾ;
		} ਨਹੀਂਤਾਂ {
			ਖੋਜ_trees[ST_IP] = s;
		}

		ਵਾਪਸ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, ਆਕਾਰ buf);
	buf[ਆਕਾਰ buf - 1] = '\0';

	/* Process FQDN segments backwards */
	ਕਰੋ {
		part = strrchr(buf, '.');
		ਜੇ (!part) {
			/* last segment */
			/* Check for existing entry */
			ਲਈ (e = d->children; e; e = e->ਅਗਲਾ)
				ਜੇ (strcasecmp(buf, e->part) == 0)
					ਰੋਕੋ;

			ਜੇ (e) {
				d = e;
			} ਨਹੀਂਤਾਂ {
				d = urldb_ਜੋੜੋ_host_ਗੰਢ(buf, d);
			}

			/* And insert into search tree */
			ਜੇ (d) {
				ਬਣਤਰ ਖੋਜ_ਗੰਢ **r;

				r = urldb_get_ਖੋਜ_ਰੁੱਖ_ਸਿੱਧਾ(buf);
				s = urldb_ਖੋਜ_ਪਾਓ(*r, d);
				ਜੇ (!s) {
					/* failed */
					d = ਨਿਰਾ;
				} ਨਹੀਂਤਾਂ {
					*r = s;
				}
			}
			ਰੋਕੋ;
		}

		/* Check for existing entry */
		ਲਈ (e = d->children; e; e = e->ਅਗਲਾ)
			ਜੇ (strcasecmp(part + 1, e->part) == 0)
				ਰੋਕੋ;

		d = e ? e : urldb_ਜੋੜੋ_host_ਗੰਢ(part + 1, d);
		ਜੇ (!d)
			ਰੋਕੋ;

		*part = '\0';
	} ਜਦੋਂ (1);

	ਵਾਪਸ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ਸਥਿਰ bool
urldb_ਪਾਓ_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *d;
	ਅਚਲ ਬਣਤਰ host_part *h;
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	time_t now = time(ਨਿਰਾ);

	ਪ੍ਰਤਿਜ੍ਞਾ(c);

	ਜੇ (c->domain[0] == '.') {
		h = urldb_ਖੋਜ_ਲੱਭੋ(
			urldb_get_ਖੋਜ_ਰੁੱਖ(&(c->domain[1])),
			c->domain + 1);
		ਜੇ (!h) {
			h = urldb_ਜੋੜੋ_host(c->domain + 1);
			ਜੇ (!h) {
				urldb_free_cookie(c);
				ਵਾਪਸ false;
			}
		}

		p = (ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *) &h->paths;
	} ਨਹੀਂਤਾਂ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ਪ੍ਰਤਿਜ੍ਞਾ(url != ਨਿਰਾ);
		ਪ੍ਰਤਿਜ੍ਞਾ(scheme != ਨਿਰਾ);

		h = urldb_ਖੋਜ_ਲੱਭੋ(
			urldb_get_ਖੋਜ_ਰੁੱਖ(c->domain),
			c->domain);

		ਜੇ (!h) {
			h = urldb_ਜੋੜੋ_host(c->domain);
			ਜੇ (!h) {
				urldb_free_cookie(c);
				ਵਾਪਸ false;
			}
		}

		/* find path */
		p = urldb_ਜੋੜੋ_ਰਸਤਾ(scheme, 0, h,
				   strdup(c->ਰਸਤਾ), ਨਿਰਾ, url);
		ਜੇ (!p) {
			urldb_free_cookie(c);
			ਵਾਪਸ false;
		}
	}

	/* add cookie */
	ਲਈ (d = p->cookies; d; d = d->ਅਗਲਾ) {
		ਜੇ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->ਰਸਤਾ, c->ਰਸਤਾ) &&
		    !strcmp(d->name, c->name))
			ਰੋਕੋ;
	}

	ਜੇ (d) {
		ਜੇ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ਜੇ (d->ਅਗਲਾ)
				d->ਅਗਲਾ->prev = d->prev;
			ਨਹੀਂਤਾਂ
				p->cookies_ਸਿਰਾ = d->prev;
			ਜੇ (d->prev)
				d->prev->ਅਗਲਾ = d->ਅਗਲਾ;
			ਨਹੀਂਤਾਂ
				p->cookies = d->ਅਗਲਾ;

			cookie_manager_remove((ਬਣਤਰ cookie_ਅੰਕੜੇ *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ਨਹੀਂਤਾਂ {
			/* replace d with c */
			c->prev = d->prev;
			c->ਅਗਲਾ = d->ਅਗਲਾ;
			ਜੇ (c->ਅਗਲਾ)
				c->ਅਗਲਾ->prev = c;
			ਨਹੀਂਤਾਂ
				p->cookies_ਸਿਰਾ = c;
			ਜੇ (c->prev)
				c->prev->ਅਗਲਾ = c;
			ਨਹੀਂਤਾਂ
				p->cookies = c;

			cookie_manager_remove((ਬਣਤਰ cookie_ਅੰਕੜੇ *)d);
			urldb_free_cookie(d);

			cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
		}
	} ਨਹੀਂਤਾਂ {
		c->prev = p->cookies_ਸਿਰਾ;
		c->ਅਗਲਾ = ਨਿਰਾ;
		ਜੇ (p->cookies_ਸਿਰਾ)
			p->cookies_ਸਿਰਾ->ਅਗਲਾ = c;
		ਨਹੀਂਤਾਂ
			p->cookies = c;
		p->cookies_ਸਿਰਾ = c;

		cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
	}

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
concat_netscape_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c,
		    ਪੂਰਨ *ਵਰਤਿਆ,
		    ਪੂਰਨ *alloc,
		    ਅੱਖਰ **buf)
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
	ਪੂਰਨ max_len;
	ਪੂਰਨ outl; /* output length */

	ਪ੍ਰਤਿਜ੍ਞਾ(c && ਵਰਤਿਆ && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *ਵਰਤਿਆ + 2 +
		ਲੜੀ_ਲੰਬਾਈ(c->name) + 1 +
		ਲੜੀ_ਲੰਬਾਈ(c->ਮੁੱਲ) + 2 +
		(c->ਰਸਤਾ_from_ਸੈੱਟ ? 8 + ਲੜੀ_ਲੰਬਾਈ(c->ਰਸਤਾ) + 2 : 0) +
		(c->domain_from_ਸੈੱਟ ? 10 + ਲੜੀ_ਲੰਬਾਈ(c->domain) + 2 : 0);
	ਜੇ (max_len >= *alloc) {
		ਅੱਖਰ *ਆਰਜ਼ੀ = realloc(*buf, max_len);
		ਜੇ (!ਆਰਜ਼ੀ) {
			ਵਾਪਸ false;
		}
		*buf = ਆਰਜ਼ੀ;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *ਵਰਤਿਆ - 1, *alloc - *ਵਰਤਿਆ, "; %s=", c->name);
	ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
		/* output truncated */
		ਵਾਪਸ false;
	}
	*ਵਰਤਿਆ += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ਜੇ (c->ਮੁੱਲ_was_quoted) {
		outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
				*alloc - *ਵਰਤਿਆ,
				"\"%s\"", c->ਮੁੱਲ);
		ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
			/* output truncated */
			ਵਾਪਸ false;
		}
		*ਵਰਤਿਆ += outl;
	} ਨਹੀਂਤਾਂ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *ਵਰਤਿਆ - 1,*alloc - *ਵਰਤਿਆ, "%s", c->ਮੁੱਲ);
		ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
			/* output truncated */
			ਵਾਪਸ false;
		}
		*ਵਰਤਿਆ += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
concat_rfc2965_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c,
		      ਪੂਰਨ *ਵਰਤਿਆ,
		      ਪੂਰਨ *alloc,
		      ਅੱਖਰ **buf)
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
	ਅਚਲ ਅੱਖਰ * ਅਚਲ separators = "()<>@,;:\\\"/[]?={} \t";
	ਪੂਰਨ max_len;
	ਪੂਰਨ outl; /* output length */

	ਪ੍ਰਤਿਜ੍ਞਾ(c && ਵਰਤਿਆ && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *ਵਰਤਿਆ + 2 +
		ਲੜੀ_ਲੰਬਾਈ(c->name) + 1 +
		ਲੜੀ_ਲੰਬਾਈ(c->ਮੁੱਲ) + 2 +
		(c->ਰਸਤਾ_from_ਸੈੱਟ ?
		 8 + ਲੜੀ_ਲੰਬਾਈ(c->ਰਸਤਾ) + 2 : 0) +
		(c->domain_from_ਸੈੱਟ ?
		 10 + ਲੜੀ_ਲੰਬਾਈ(c->domain) + 2 : 0);

	ਜੇ (max_len >= *alloc) {
		ਅੱਖਰ *ਆਰਜ਼ੀ = realloc(*buf, max_len);
		ਜੇ (!ਆਰਜ਼ੀ) {
			ਵਾਪਸ false;
		}
		*buf = ਆਰਜ਼ੀ;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
			*alloc - *ਵਰਤਿਆ,
			"; %s=", c->name);
	ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
		/* output truncated */
		ਵਾਪਸ false;
	}
	*ਵਰਤਿਆ += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ਜੇ (c->ਮੁੱਲ_was_quoted ||
	    strpbrk(c->ਮੁੱਲ, separators) != ਨਿਰਾ) {
		outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
				*alloc - *ਵਰਤਿਆ,
				"\"%s\"", c->ਮੁੱਲ);

	} ਨਹੀਂਤਾਂ {
		outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
				*alloc - *ਵਰਤਿਆ,
				"%s", c->ਮੁੱਲ);
	}
	ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
		/* output truncated */
		ਵਾਪਸ false;
	}
	*ਵਰਤਿਆ += outl;

	ਜੇ (c->ਰਸਤਾ_from_ਸੈੱਟ) {
		/* Path, quoted if necessary */

		ਜੇ (strpbrk(c->ਰਸਤਾ, separators) != ਨਿਰਾ) {
			outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
					*alloc - *ਵਰਤਿਆ,
					"; $Path=\"%s\"", c->ਰਸਤਾ);

		} ਨਹੀਂਤਾਂ {
			outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
				       *alloc - *ਵਰਤਿਆ,
				       "; $Path=%s", c->ਰਸਤਾ);
		}
		ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
			/* output truncated */
			ਵਾਪਸ false;
		}
		*ਵਰਤਿਆ += outl;
	}

	ਜੇ (c->domain_from_ਸੈੱਟ) {
		/* Domain, quoted if necessary */
		ਜੇ (strpbrk(c->domain, separators) != ਨਿਰਾ) {
			outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
					*alloc - *ਵਰਤਿਆ,
					"; $Domain=\"%s\"", c->domain);
		} ਨਹੀਂਤਾਂ {
			outl = snprintf(*buf + *ਵਰਤਿਆ - 1,
					*alloc - *ਵਰਤਿਆ,
					"; $Domain=%s", c->domain);
		}
		ਜੇ (outl >= (*alloc - *ਵਰਤਿਆ)) {
			/* output truncated */
			ਵਾਪਸ false;
		}
		*ਵਰਤਿਆ += outl;
	}

	ਵਾਪਸ true;
}


/**
 * build output cookie string in netscape format
 */
ਸਥਿਰ ਅੱਖਰ *
get_netscape_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ **matched_cookies, ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਪੂਰਨ ret_alloc = 4096, ret_ਵਰਤਿਆ = 1;
	ਅੱਖਰ *ret;
	ਪੂਰਨ midx; /* index into matched cookies */

	ret = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ret_alloc);
	ਜੇ (ret == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}
	ret[0] = '\0';

	ਲਈ (midx = 0; midx < ਗਿਣਤੀ_2; midx++) {
		ਜੇ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_ਵਰਤਿਆ, &ret_alloc, &ret)) {
			ਮੁਕਤ(ret);
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	ਜੇ (ret_ਵਰਤਿਆ <= 2) {
		ਮੁਕਤ(ret);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_ਵਰਤਿਆ - 2);
	ret_ਵਰਤਿਆ -= 2;

	/* shrink the output buffer to the required size */
	ਅੱਖਰ *ਆਰਜ਼ੀ = realloc(ret, ret_ਵਰਤਿਆ);
	ਜੇ (ਆਰਜ਼ੀ != ਨਿਰਾ) {
		ret = ਆਰਜ਼ੀ;
	}

	ਵਾਪਸ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ਸਥਿਰ ਅੱਖਰ *
get_rfc2965_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ **matched_cookies,
		   ਪੂਰਨ ਗਿਣਤੀ_2,
		   ਪੂਰਨ ਸੰਸਕਰਣ)
{
	ਪੂਰਨ ret_alloc = 4096; /* return string allocation */
	ਪੂਰਨ ret_ਵਰਤਿਆ = 1; /* return sting buffer used */
	ਅੱਖਰ *ret; /* return string buffer */
	ਪੂਰਨ midx; /* index into matched cookies */
	ਪੂਰਨ outl;

	ret = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ret_alloc);
	ਜੇ (ret == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_ਵਰਤਿਆ, "$Version=%d", ਸੰਸਕਰਣ);
	ਜੇ (outl >= ret_alloc - ret_ਵਰਤਿਆ) {
		/*ਉਛਾਲ*/
		ਮੁਕਤ(ret);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ret_ਵਰਤਿਆ += outl;

	ਲਈ (midx = 0; midx < ਗਿਣਤੀ_2; midx++) {
		ਜੇ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_ਵਰਤਿਆ, &ret_alloc, &ret)) {
			ਮੁਕਤ(ret);
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	/* shrink the output buffer to the required size */
	ਅੱਖਰ *ਆਰਜ਼ੀ = realloc(ret, ret_ਵਰਤਿਆ);
	ਜੇ (ਆਰਜ਼ੀ != ਨਿਰਾ) {
		ret = ਆਰਜ਼ੀ;
	}

	ਵਾਪਸ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ਸਥਿਰ ਖਾਲੀ
urldb_delete_cookie_paths(ਅਚਲ ਅੱਖਰ *domain,
			  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
			  ਅਚਲ ਅੱਖਰ *name,
			  ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ)
{
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ);

	ਕਰੋ {
		ਲਈ (c = p->cookies; c; c = c->ਅਗਲਾ) {
			ਜੇ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->ਰਸਤਾ, ਰਸਤਾ) == 0 &&
			    strcmp(c->name, name) == 0) {
				ਜੇ (c->prev) {
					c->prev->ਅਗਲਾ = c->ਅਗਲਾ;
				} ਨਹੀਂਤਾਂ {
					p->cookies = c->ਅਗਲਾ;
				}

				ਜੇ (c->ਅਗਲਾ) {
					c->ਅਗਲਾ->prev = c->prev;
				} ਨਹੀਂਤਾਂ {
					p->cookies_ਸਿਰਾ = c->prev;
				}

				urldb_free_cookie(c);

				ਵਾਪਸ;
			}
		}

		ਜੇ (p->children) {
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			ਜਦੋਂ (p != ਮਾਪਾ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਮਾਪਾ);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ਸਥਿਰ ਖਾਲੀ
urldb_delete_cookie_hosts(ਅਚਲ ਅੱਖਰ *domain,
			  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
			  ਅਚਲ ਅੱਖਰ *name,
			  ਬਣਤਰ host_part *ਮਾਪਾ)
{
	ਬਣਤਰ host_part *h;
	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ);

	urldb_delete_cookie_paths(domain, ਰਸਤਾ, name, &ਮਾਪਾ->paths);

	ਲਈ (h = ਮਾਪਾ->children; h; h = h->ਅਗਲਾ) {
		urldb_delete_cookie_hosts(domain, ਰਸਤਾ, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ਸਥਿਰ ਖਾਲੀ urldb_save_cookie_paths(FILE *fp, ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਮਾਪਾ)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਮਾਪਾ;
	time_t now = time(ਨਿਰਾ);

	ਪ੍ਰਤਿਜ੍ਞਾ(fp && ਮਾਪਾ);

	ਕਰੋ {
		ਜੇ (p->cookies != ਨਿਰਾ) {
			ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;

			ਲਈ (c = p->cookies; c != ਨਿਰਾ; c = c->ਅਗਲਾ) {
				ਜੇ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ਜਾਰੀ;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->ਸੰਸਕਰਣ, c->domain,
					c->domain_from_ਸੈੱਟ, c->ਰਸਤਾ,
					c->ਰਸਤਾ_from_ਸੈੱਟ, c->secure,
					c->http_only,
					(ਪੂਰਨ)c->expires, (ਪੂਰਨ)c->ਆਖਰੀ_ਵਰਤਿਆ,
					c->no_destroy, c->name, c->ਮੁੱਲ,
					c->ਮੁੱਲ_was_quoted,
					p->scheme ? lwc_string_ਅੰਕੜੇ(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ਜੇ (p->children != ਨਿਰਾ) {
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			ਜਦੋਂ (p != ਮਾਪਾ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				p = p->ਮਾਪਾ;
			}
		}
	} ਜਦੋਂ (p != ਮਾਪਾ);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ਸਥਿਰ ਖਾਲੀ urldb_save_cookie_hosts(FILE *fp, ਬਣਤਰ host_part *ਮਾਪਾ)
{
	ਬਣਤਰ host_part *h;
	ਪ੍ਰਤਿਜ੍ਞਾ(fp && ਮਾਪਾ);

	urldb_save_cookie_paths(fp, &ਮਾਪਾ->paths);

	ਲਈ (h = ਮਾਪਾ->children; h; h = h->ਅਗਲਾ)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_cookie(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c)
{
	ਮੁਕਤ(c->name);
	ਮੁਕਤ(c->ਮੁੱਲ);
	ਮੁਕਤ(c->comment);
	ਮੁਕਤ(c->domain);
	ਮੁਕਤ(c->ਰਸਤਾ);

	ਮੁਕਤ(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_ਰਸਤਾ_ਗੰਢ_content(ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਗੰਢ)
{
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *a, *b;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਜੇ (ਗੰਢ->url != ਨਿਰਾ) {
		nsurl_unref(ਗੰਢ->url);
	}

	lwc_string_unref(ਗੰਢ->scheme);

	ਮੁਕਤ(ਗੰਢ->segment);
	ਲਈ (i = 0; i < ਗੰਢ->frag_cnt; i++)
		ਮੁਕਤ(ਗੰਢ->fragment[i]);
	ਮੁਕਤ(ਗੰਢ->fragment);

	ਮੁਕਤ(ਗੰਢ->urld.title);

	ਲਈ (a = ਗੰਢ->cookies; a; a = b) {
		b = a->ਅਗਲਾ;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_prot_space(ਬਣਤਰ prot_space_ਅੰਕੜੇ *space)
{
	lwc_string_unref(space->scheme);
	ਮੁਕਤ(space->realm);
	ਮੁਕਤ(space->auth);

	ਮੁਕਤ(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_ਰਸਤਾ_ਰੁੱਖ(ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *ਜੜ੍ਹ)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਜੜ੍ਹ;

	ਕਰੋ {
		ਜੇ (p->children != ਨਿਰਾ) {
			p = p->children;
		} ਨਹੀਂਤਾਂ {
			ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *q = p;

			ਜਦੋਂ (p != ਜੜ੍ਹ) {
				ਜੇ (p->ਅਗਲਾ != ਨਿਰਾ) {
					p = p->ਅਗਲਾ;
					ਰੋਕੋ;
				}

				p = p->ਮਾਪਾ;

				urldb_destroy_ਰਸਤਾ_ਗੰਢ_content(q);
				ਮੁਕਤ(q);

				q = p;
			}

			urldb_destroy_ਰਸਤਾ_ਗੰਢ_content(q);
			ਮੁਕਤ(q);
		}
	} ਜਦੋਂ (p != ਜੜ੍ਹ);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_host_ਰੁੱਖ(ਬਣਤਰ host_part *ਜੜ੍ਹ)
{
	ਬਣਤਰ host_part *a, *b;
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p, *q;
	ਬਣਤਰ prot_space_ਅੰਕੜੇ *s, *t;

	/* Destroy children */
	ਲਈ (a = ਜੜ੍ਹ->children; a; a = b) {
		b = a->ਅਗਲਾ;
		urldb_destroy_host_ਰੁੱਖ(a);
	}

	/* Now clean up paths */
	ਲਈ (p = ਜੜ੍ਹ->paths.children; p; p = q) {
		q = p->ਅਗਲਾ;
		urldb_destroy_ਰਸਤਾ_ਰੁੱਖ(p);
	}

	/* Root path */
	urldb_destroy_ਰਸਤਾ_ਗੰਢ_content(&ਜੜ੍ਹ->paths);

	/* Proctection space data */
	ਲਈ (s = ਜੜ੍ਹ->prot_space; s; s = t) {
		t = s->ਅਗਲਾ;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	ਮੁਕਤ(ਜੜ੍ਹ->part);
	ਮੁਕਤ(ਜੜ੍ਹ);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ਸਥਿਰ ਖਾਲੀ urldb_destroy_ਖੋਜ_ਰੁੱਖ(ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਜੜ੍ਹ)
{
	/* Destroy children */
	ਜੇ (ਜੜ੍ਹ->ਖੱਬਾ != &ਖਾਲੀ_ਥਾਵਾਂ)
		urldb_destroy_ਖੋਜ_ਰੁੱਖ(ਜੜ੍ਹ->ਖੱਬਾ);
	ਜੇ (ਜੜ੍ਹ->ਸੱਜਾ != &ਖਾਲੀ_ਥਾਵਾਂ)
		urldb_destroy_ਖੋਜ_ਰੁੱਖ(ਜੜ੍ਹ->ਸੱਜਾ);

	/* And destroy ourselves */
	ਮੁਕਤ(ਜੜ੍ਹ);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
ਖਾਲੀ urldb_destroy(ਖਾਲੀ)
{
	ਬਣਤਰ host_part *a, *b;
	ਪੂਰਨ i;

	/* Clean up search trees */
	ਲਈ (i = 0; i < ਅੰਕ_ਖੋਜ_TREES; i++) {
		ਜੇ (ਖੋਜ_trees[i] != &ਖਾਲੀ_ਥਾਵਾਂ) {
			urldb_destroy_ਖੋਜ_ਰੁੱਖ(ਖੋਜ_trees[i]);
			ਖੋਜ_trees[i] = &ਖਾਲੀ_ਥਾਵਾਂ;
		}
	}

	/* And database */
	ਲਈ (a = db_ਜੜ੍ਹ.children; a; a = b) {
		b = a->ਅਗਲਾ;
		urldb_destroy_host_ਰੁੱਖ(a);
	}
	memset(&db_ਜੜ੍ਹ, 0, ਆਕਾਰ(db_ਜੜ੍ਹ));

	/* And the bloom filter */
	ਜੇ (url_bloom != ਨਿਰਾ) {
		bloom_destroy(url_bloom);
		url_bloom = ਨਿਰਾ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_ਲੱਦੋ(ਅਚਲ ਅੱਖਰ *filename)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ 4096
	ਅੱਖਰ s[ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ];
	ਅੱਖਰ host[256];
	ਬਣਤਰ host_part *h;
	ਪੂਰਨ urls;
	ਪੂਰਨ i;
	ਪੂਰਨ ਸੰਸਕਰਣ;
	ਪੂਰਨ ਲੰਮਾਈ;
	FILE *fp;

	ਪ੍ਰਤਿਜ੍ਞਾ(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ਜੇ (url_bloom == ਨਿਰਾ)
		url_bloom = bloom_create(BLOOM_ਮਾਪ);

	fp = fopen(filename, "r");
	ਜੇ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp)) {
		fclose(fp);
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;
	}

	ਸੰਸਕਰਣ = atoi(s);
	ਜੇ (ਸੰਸਕਰਣ < MIN_URL_FILE_ਸੰਸਕਰਣ) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ਵਾਪਸ NSERROR_INVALID;
	}
	ਜੇ (ਸੰਸਕਰਣ > URL_FILE_ਸੰਸਕਰਣ) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਜਦੋਂ (fgets(host, ਆਕਾਰ host, fp)) {
		time_t hsts_expiry = 0;
		ਪੂਰਨ hsts_include_sub_domains = 0;

		/* get the hostname */
		ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(host) - 1;
		host[ਲੰਮਾਈ] = '\0';

		/* skip data that has ended up with a host of '' */
		ਜੇ (ਲੰਮਾਈ == 0) {
			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;
			urls = atoi(s);
			/* Eight fields/url */
			ਲਈ (i = 0; i < (8 * urls); i++) {
				ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
					ਰੋਕੋ;
			}
			ਜਾਰੀ;
		}

		ਜੇ (ਸੰਸਕਰਣ >= 107) {
			ਅੱਖਰ *p = host;
			ਜਦੋਂ (*p && *p != ' ') p++;
			ਜਦੋਂ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ਜਦੋਂ (*p && *p != ' ') p++;
			ਜਦੋਂ (*p && *p == ' ') p++;
			nsc_snptimet(p, ਲੜੀ_ਲੰਬਾਈ(p), &hsts_expiry);
		}

		h = urldb_ਜੋੜੋ_host(host);
		ਜੇ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ਵਾਪਸ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
			ਰੋਕੋ;
		urls = atoi(s);

		/* no URLs => try next host */
		ਜੇ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ਜਾਰੀ;
		}

		/* load the non-corrupt data */
		ਲਈ (i = 0; i < urls; i++) {
			ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p = ਨਿਰਾ;
			ਅੱਖਰ scheme[64], ports[10];
			ਅੱਖਰ url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			ਅੱਖਰ *ਰਸਤਾ_query;
			size_t len;

			ਜੇ (!fgets(scheme, ਆਕਾਰ scheme, fp))
				ਰੋਕੋ;
			ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(scheme) - 1;
			scheme[ਲੰਮਾਈ] = '\0';

			ਜੇ (!fgets(ports, ਆਕਾਰ ports, fp))
				ਰੋਕੋ;
			ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(ports) - 1;
			ports[ਲੰਮਾਈ] = '\0';
			ਪੋਰਟ = atoi(ports);

			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;
			ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(s) - 1;
			s[ਲੰਮਾਈ] = '\0';

			ਜੇ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, ਆਕਾਰ url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (ਪੋਰਟ ? ":" : ""),
				 (ਪੋਰਟ ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			ਜੇ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ਵਾਪਸ NSERROR_NOMEM;
			}

			ਜੇ (url_bloom != ਨਿਰਾ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ਪਾਓ_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ਜੇ (nsurl_get(nsurl, NSURL_ਰਸਤਾ | NSURL_QUERY,
				      &ਰਸਤਾ_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ਵਾਪਸ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_ਜੋੜੋ_ਰਸਤਾ(scheme_lwc, ਪੋਰਟ, h, ਰਸਤਾ_query,
					   fragment_lwc, nsurl);
			ਜੇ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ਵਾਪਸ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;
			ਜੇ (p)
				p->urld.visits = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)atoi(s);

			/* entry last use time */
			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp)) {
				ਰੋਕੋ;
			}
			ਜੇ (p) {
				nsc_snptimet(s, ਲੜੀ_ਲੰਬਾਈ(s) - 1, &p->urld.ਆਖਰੀ_ਦੇਖੋ);
			}

			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;
			ਜੇ (p)
				p->urld.type = (content_type)atoi(s);

			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;


			ਜੇ (!fgets(s, ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ, fp))
				ਰੋਕੋ;
			ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(s) - 1;
			ਜੇ (p && ਲੰਮਾਈ > 0) {
				s[ਲੰਮਾਈ] = '\0';
				p->urld.title = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੰਮਾਈ + 1);
				ਜੇ (p->urld.title)
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->urld.title, s, ਲੰਮਾਈ + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਵੱਧ_ਤੋਂ_ਵੱਧ_URL_ਲੰਮਾਈ

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ਅਚਲ ਅੱਖਰ *filename)
{
	FILE *fp;
	ਪੂਰਨ i;

	ਪ੍ਰਤਿਜ੍ਞਾ(filename);

	fp = fopen(filename, "w");
	ਜੇ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_ਸੰਸਕਰਣ);

	ਲਈ (i = 0; i != ਅੰਕ_ਖੋਜ_TREES; i++) {
		urldb_save_ਖੋਜ_ਰੁੱਖ(ਖੋਜ_trees[i], fp);
	}

	fclose(fp);

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_ਸੈੱਟ_url_persistence(nsurl *url, bool persist)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	p->persistent = persist;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_ਜੋੜੋ_url(nsurl *url)
{
	ਬਣਤਰ host_part *h;
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	lwc_string *scheme;
	lwc_string *ਪੋਰਟ;
	lwc_string *host;
	lwc_string *fragment;
	ਅਚਲ ਅੱਖਰ *host_str;
	ਅੱਖਰ *ਰਸਤਾ_query = ਨਿਰਾ;
	size_t len;
	bool ਮੇਲ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੋਰਟ_int;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	ਜੇ (url_bloom == ਨਿਰਾ)
		url_bloom = bloom_create(BLOOM_ਮਾਪ);

	ਜੇ (url_bloom != ਨਿਰਾ) {
		uint32_t hash = nsurl_hash(url);
		bloom_ਪਾਓ_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ਜੇ (nsurl_get(url, NSURL_ਰਸਤਾ | NSURL_QUERY, &ਰਸਤਾ_query, &len) !=
	    NSERROR_OK) {
		ਵਾਪਸ false;
	}
	ਪ੍ਰਤਿਜ੍ਞਾ(ਰਸਤਾ_query != ਨਿਰਾ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (scheme == ਨਿਰਾ) {
		ਮੁਕਤ(ਰਸਤਾ_query);
		ਵਾਪਸ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (host != ਨਿਰਾ) {
		host_str = lwc_string_ਅੰਕੜੇ(host);
		lwc_string_unref(host);

	} ਨਹੀਂਤਾਂ ਜੇ (lwc_string_isequal(scheme, corestring_lwc_file, &ਮੇਲ) ==
		   lwc_error_ok && ਮੇਲ == true) {
		host_str = "localhost";

	} ਨਹੀਂਤਾਂ {
		lwc_string_unref(scheme);
		ਮੁਕਤ(ਰਸਤਾ_query);
		ਵਾਪਸ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	ਪੋਰਟ = nsurl_get_component(url, NSURL_ਪੋਰਟ);
	ਜੇ (ਪੋਰਟ != ਨਿਰਾ) {
		ਪੋਰਟ_int = atoi(lwc_string_ਅੰਕੜੇ(ਪੋਰਟ));
		lwc_string_unref(ਪੋਰਟ);
	} ਨਹੀਂਤਾਂ {
		ਪੋਰਟ_int = 0;
	}

	/* Get host entry */
	h = urldb_ਜੋੜੋ_host(host_str);

	/* Get path entry */
	ਜੇ (h != ਨਿਰਾ) {
		p = urldb_ਜੋੜੋ_ਰਸਤਾ(scheme,
				   ਪੋਰਟ_int,
				   h,
				   ਰਸਤਾ_query,
				   fragment,
				   url);
	} ਨਹੀਂਤਾਂ {
		p = ਨਿਰਾ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ਵਾਪਸ (p != ਨਿਰਾ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_ਸੈੱਟ_url_title(nsurl *url, ਅਚਲ ਅੱਖਰ *title)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਅੱਖਰ *ਆਰਜ਼ੀ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (p == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	/* copy the parameter if necessary */
	ਜੇ (title != ਨਿਰਾ) {
		ਆਰਜ਼ੀ = strdup(title);
		ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	} ਨਹੀਂਤਾਂ {
		ਆਰਜ਼ੀ = ਨਿਰਾ;
	}

	ਮੁਕਤ(p->urld.title);
	p->urld.title = ਆਰਜ਼ੀ;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_ਸੈੱਟ_url_content_type(nsurl *url, content_type type)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	p->urld.type = type;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_ਦੇਖੋ_ਅੰਕੜੇ(nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	p->urld.ਆਖਰੀ_ਦੇਖੋ = time(ਨਿਰਾ);
	p->urld.visits++;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
ਖਾਲੀ urldb_reset_url_ਦੇਖੋ_ਅੰਕੜੇ(nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ;

	p->urld.ਆਖਰੀ_ਦੇਖੋ = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *urldb_get_url_ਅੰਕੜੇ(nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਬਣਤਰ url_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *u;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ ਨਿਰਾ;

	u = &p->urld;

	ਵਾਪਸ (ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ p->url;
}


/* exported interface documented in netsurf/url_db.h */
ਖਾਲੀ urldb_ਸੈੱਟ_auth_details(nsurl *url, ਅਚਲ ਅੱਖਰ *realm, ਅਚਲ ਅੱਖਰ *auth)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p, *pi;
	ਬਣਤਰ host_part *h;
	ਬਣਤਰ prot_space_ਅੰਕੜੇ *space, *space_alloc;
	ਅੱਖਰ *realm_alloc, *auth_alloc;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url && realm && auth);

	/* add url, in case it's missing */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);

	ਜੇ (!p)
		ਵਾਪਸ;

	/* Search for host_part */
	ਲਈ (pi = p; pi->ਮਾਪਾ != ਨਿਰਾ; pi = pi->ਮਾਪਾ)
		;
	h = (ਬਣਤਰ host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	ਲਈ (space = h->prot_space; space; space = space->ਅਗਲਾ) {
		ਜੇ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &ਮੇਲ) == lwc_error_ok &&
		    ਮੇਲ == true &&
		    space->ਪੋਰਟ == p->ਪੋਰਟ)
			ਰੋਕੋ;
	}

	ਜੇ (space != ਨਿਰਾ) {
		/* Overrule existing auth. */
		ਮੁਕਤ(space->auth);
		space->auth = strdup(auth);
	} ਨਹੀਂਤਾਂ {
		/* Create a new protection space. */
		space = space_alloc = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ prot_space_ਅੰਕੜੇ));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ਜੇ (!space_alloc || !realm_alloc || !auth_alloc) {
			ਮੁਕਤ(space_alloc);
			ਮੁਕਤ(realm_alloc);
			ਮੁਕਤ(auth_alloc);
			ਵਾਪਸ;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->ਪੋਰਟ = p->ਪੋਰਟ;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->ਅਗਲਾ = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ਅਚਲ ਅੱਖਰ *urldb_get_auth_details(nsurl *url, ਅਚਲ ਅੱਖਰ *realm)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p, *p_cur, *p_ਸਿਖਰ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	/* add to the db, so our lookup will work */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ ਨਿਰਾ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	ਲਈ (p_cur = p; p_cur != ਨਿਰਾ; p_ਸਿਖਰ = p_cur, p_cur = p_cur->ਮਾਪਾ) {
		ਜੇ (p_cur->prot_space) {
			ਵਾਪਸ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ਜੇ (realm != ਨਿਰਾ) {
		ਅਚਲ ਬਣਤਰ host_part *h = (ਅਚਲ ਬਣਤਰ host_part *)p_ਸਿਖਰ;
		ਅਚਲ ਬਣਤਰ prot_space_ਅੰਕੜੇ *space;
		bool ਮੇਲ;

		/* Search for a possible matching protection space. */
		ਲਈ (space = h->prot_space; space != ਨਿਰਾ;
		     space = space->ਅਗਲਾ) {
			ਜੇ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &ਮੇਲ) ==
			    lwc_error_ok &&
			    ਮੇਲ == true &&
			    space->ਪੋਰਟ == p->ਪੋਰਟ) {
				p->prot_space = space;
				ਵਾਪਸ p->prot_space->auth;
			}
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/* exported interface documented in netsurf/url_db.h */
ਖਾਲੀ urldb_ਸੈੱਟ_cert_permissions(nsurl *url, bool permit)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਬਣਤਰ host_part *h;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	/* add url, in case it's missing */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ;

	ਲਈ (; p && p->ਮਾਪਾ; p = p->ਮਾਪਾ)
		/* do nothing */;
	ਪ੍ਰਤਿਜ੍ਞਾ(p);

	h = (ਬਣਤਰ host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਅਚਲ ਬਣਤਰ host_part *h;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ false;

	ਲਈ (; p && p->ਮਾਪਾ; p = p->ਮਾਪਾ)
		/* do nothing */;
	ਪ੍ਰਤਿਜ੍ਞਾ(p);

	h = (ਅਚਲ ਬਣਤਰ host_part *)p;

	ਵਾਪਸ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_ਸੈੱਟ_hsts_policy(ਬਣਤਰ nsurl *url, ਅਚਲ ਅੱਖਰ *ਸਿਰਲੇਖ)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਬਣਤਰ host_part *h;
	lwc_string *host;
	time_t now = time(ਨਿਰਾ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (host != ਨਿਰਾ) {
		ਜੇ (urldb_host_is_ip_ਪਤਾ(lwc_string_ਅੰਕੜੇ(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ਵਾਪਸ true;
		} ਨਹੀਂਤਾਂ ਜੇ (lwc_string_ਲੰਮਾਈ(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ਵਾਪਸ true;
		}

		lwc_string_unref(host);
	} ਨਹੀਂਤਾਂ {
		/* No host part: ignore */
		ਵਾਪਸ true;
	}

	/* add url, in case it's missing */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ false;

	ਲਈ (; p && p->ਮਾਪਾ; p = p->ਮਾਪਾ)
		/* do nothing */;
	ਪ੍ਰਤਿਜ੍ਞਾ(p);

	h = (ਬਣਤਰ host_part *)p;
	ਜੇ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ਵਾਪਸ true;
	}

	error = http_parse_strict_transport_security(ਸਿਰਲੇਖ, &sts);
	ਜੇ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ਵਾਪਸ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ਜੇ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ਨਹੀਂਤਾਂ ਜੇ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ਵਾਪਸ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ਬਣਤਰ nsurl *url)
{
	ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p;
	ਅਚਲ ਬਣਤਰ host_part *h;
	lwc_string *host;
	time_t now = time(ਨਿਰਾ);

	ਪ੍ਰਤਿਜ੍ਞਾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (host != ਨਿਰਾ) {
		ਜੇ (urldb_host_is_ip_ਪਤਾ(lwc_string_ਅੰਕੜੇ(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ਵਾਪਸ false;
		} ਨਹੀਂਤਾਂ ਜੇ (lwc_string_ਲੰਮਾਈ(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ਵਾਪਸ false;
		}

		lwc_string_unref(host);
	} ਨਹੀਂਤਾਂ {
		/* No host part: not enabled */
		ਵਾਪਸ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ false;

	ਲਈ (; p && p->ਮਾਪਾ; p = p->ਮਾਪਾ)
		/* do nothing */;
	ਪ੍ਰਤਿਜ੍ਞਾ(p);

	h = (ਅਚਲ ਬਣਤਰ host_part *)p;

	/* Consult record for this host */
	ਜੇ (h->hsts.expires > now) {
		/* Not expired */
		ਵਾਪਸ true;
	}

	/* Consult parent domains */
	ਲਈ (h = h->ਮਾਪਾ; h && h != &db_ਜੜ੍ਹ; h = h->ਮਾਪਾ) {
		ਜੇ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ਵਾਪਸ true;
		}
	}

	ਵਾਪਸ false;
}


/* exported interface documented in netsurf/url_db.h */
ਖਾਲੀ
urldb_iterate_partial(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ,
		      bool (*callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਅੱਖਰ host[256];
	ਅੱਖਰ buf[260]; /* max domain + "www." */
	ਅਚਲ ਅੱਖਰ *slash, *scheme_sep;
	ਬਣਤਰ ਖੋਜ_ਗੰਢ *ਰੁੱਖ;
	ਅਚਲ ਬਣਤਰ host_part *h;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗੇਤਰ && callback);

	/* strip scheme */
	scheme_sep = ਲੜੀ_ਖੋਜ(ਅਗੇਤਰ, "://");
	ਜੇ (scheme_sep)
		ਅਗੇਤਰ = scheme_sep + 3;

	slash = strchr(ਅਗੇਤਰ, '/');
	ਰੁੱਖ = urldb_get_ਖੋਜ_ਰੁੱਖ(ਅਗੇਤਰ);

	ਜੇ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, ਆਕਾਰ host, "%.*s",
			 (ਪੂਰਨ) (slash - ਅਗੇਤਰ), ਅਗੇਤਰ);

		h = urldb_ਖੋਜ_ਲੱਭੋ(ਰੁੱਖ, host);
		ਜੇ (!h) {
			ਪੂਰਨ len = slash - ਅਗੇਤਰ;

			ਜੇ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, ਆਕਾਰ buf, "www.%s", host);
				h = urldb_ਖੋਜ_ਲੱਭੋ(
					ਖੋਜ_trees[ST_DN + 'w' - 'a'],
					buf);
				ਜੇ (!h)
					ਵਾਪਸ;
			} ਨਹੀਂਤਾਂ
				ਵਾਪਸ;
		}

		ਜੇ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_ਰਸਤਾ(&h->paths, slash + 1,
						   callback);
		}

	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(ਅਗੇਤਰ);

		/* looking for hosts */
		ਜੇ (!urldb_iterate_partial_host(ਰੁੱਖ, ਅਗੇਤਰ, callback))
			ਵਾਪਸ;

		ਜੇ (len <= 3 || strncasecmp(ਅਗੇਤਰ, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, ਆਕਾਰ buf, "www.%s", ਅਗੇਤਰ);
			ਜੇ(!urldb_iterate_partial_host(
				   ਖੋਜ_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ਵਾਪਸ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
ਖਾਲੀ
urldb_iterate_entries(bool (*callback)(nsurl *url, ਅਚਲ ਬਣਤਰ url_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਪੂਰਨ i;

	ਪ੍ਰਤਿਜ੍ਞਾ(callback);

	ਲਈ (i = 0; i < ਅੰਕ_ਖੋਜ_TREES; i++) {
		ਜੇ (!urldb_iterate_entries_host(ਖੋਜ_trees[i],
						callback,
						ਨਿਰਾ)) {
			ਰੋਕੋ;
		}
	}
}


/* exported interface documented in content/urldb.h */
ਖਾਲੀ urldb_iterate_cookies(bool (*callback)(ਅਚਲ ਬਣਤਰ cookie_ਅੰਕੜੇ *ਅੰਕੜੇ))
{
	ਪੂਰਨ i;

	ਪ੍ਰਤਿਜ੍ਞਾ(callback);

	ਲਈ (i = 0; i < ਅੰਕ_ਖੋਜ_TREES; i++) {
		ਜੇ (!urldb_iterate_entries_host(ਖੋਜ_trees[i],
						ਨਿਰਾ, callback))
			ਰੋਕੋ;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_ਸੈੱਟ_cookie(ਅਚਲ ਅੱਖਰ *ਸਿਰਲੇਖ, nsurl *url, nsurl *referer)
{
	ਅਚਲ ਅੱਖਰ *cur = ਸਿਰਲੇਖ, *ਸਿਰਾ;
	lwc_string *ਰਸਤਾ, *host, *scheme;
	nsurl *urlt;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url && ਸਿਰਲੇਖ);

	/* Get defragmented URL, as 'urlt' */
	ਜੇ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ਵਾਪਸ ਨਿਰਾ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (scheme == ਨਿਰਾ) {
		nsurl_unref(urlt);
		ਵਾਪਸ false;
	}

	ਰਸਤਾ = nsurl_get_component(url, NSURL_ਰਸਤਾ);
	ਜੇ (ਰਸਤਾ == ਨਿਰਾ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ਵਾਪਸ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (host == ਨਿਰਾ) {
		lwc_string_unref(ਰਸਤਾ);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ਵਾਪਸ false;
	}

	ਜੇ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ਜੇ (rhost == ਨਿਰਾ) {
			ਜਾਓ error;
		}

		/* Domain match host names */
		ਜੇ (lwc_string_isequal(host, rhost, &ਮੇਲ) == lwc_error_ok &&
		    ਮੇਲ == false) {
			ਅਚਲ ਅੱਖਰ *hptr;
			ਅਚਲ ਅੱਖਰ *rptr;
			ਅਚਲ ਅੱਖਰ *dot;
			ਅਚਲ ਅੱਖਰ *host_ਅੰਕੜੇ = lwc_string_ਅੰਕੜੇ(host);
			ਅਚਲ ਅੱਖਰ *rhost_ਅੰਕੜੇ = lwc_string_ਅੰਕੜੇ(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ਜੇ (urldb_host_is_ip_ਪਤਾ(host_ਅੰਕੜੇ) ||
			    urldb_host_is_ip_ਪਤਾ(rhost_ਅੰਕੜੇ)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				ਜਾਓ error;
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

			hptr = host_ਅੰਕੜੇ + lwc_string_ਲੰਮਾਈ(host) - 1;
			rptr = rhost_ਅੰਕੜੇ + lwc_string_ਲੰਮਾਈ(rhost) - 1;

			/* 1 */
			ਜਦੋਂ (hptr >= host_ਅੰਕੜੇ && rptr >= rhost_ਅੰਕੜੇ) {
				ਜੇ (*hptr != *rptr)
					ਰੋਕੋ;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ਜਦੋਂ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ਜੇ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ਨਿਰਾ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				ਜਾਓ error;
			}
		}

		lwc_string_unref(rhost);
	}

	ਸਿਰਾ = cur + ਲੜੀ_ਲੰਬਾਈ(cur) - 2 /* Trailing CRLF */;

	ਕਰੋ {
		ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;
		ਅੱਖਰ *dot;
		size_t len;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_NSPSL
		ਅਚਲ ਅੱਖਰ *suffix;
#ਜੇ_ਅੰਤ

		c = urldb_parse_cookie(url, &cur);
		ਜੇ (!c) {
			/* failed => stop parsing */
			ਜਾਓ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ਜੇ (!c->name || !c->ਮੁੱਲ) {
			urldb_free_cookie(c);
			ਜਾਓ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = ਲੜੀ_ਲੰਬਾਈ(c->ਰਸਤਾ);
		ਜੇ (len > lwc_string_ਲੰਮਾਈ(ਰਸਤਾ) ||
		    ਲੜੀ_ਤੁਲਨਾ(c->ਰਸਤਾ, lwc_string_ਅੰਕੜੇ(ਰਸਤਾ),
			    len) != 0) {
			urldb_free_cookie(c);
			ਜਾਓ error;
		}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ਜੇ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ਜੇ (suffix == ਨਿਰਾ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			ਜਾਓ error;
		}
#ਨਹੀਂਤਾਂ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ਜੇ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			ਜਾਓ error;
		}
#ਜੇ_ਅੰਤ

		/* Domain match fetch host with cookie domain */
		ਜੇ (strcasecmp(lwc_string_ਅੰਕੜੇ(host), c->domain) != 0) {
			ਪੂਰਨ hlen, dlen;
			ਅੱਖਰ *domain = c->domain;

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
			ਪ੍ਰਤਿਜ੍ਞਾ(c->domain[0] == '.');

			/* 4.3.2:iii */
			ਜੇ (urldb_host_is_ip_ਪਤਾ(lwc_string_ਅੰਕੜੇ(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				ਜਾਓ error;
			}

			hlen = lwc_string_ਲੰਮਾਈ(host);
			dlen = ਲੜੀ_ਲੰਬਾਈ(c->domain);

			ਜੇ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				ਜਾਓ error;
			}

			ਜੇ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ਜੇ (strcasecmp(lwc_string_ਅੰਕੜੇ(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				ਜਾਓ error;
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
		ਜੇ (!urldb_ਪਾਓ_cookie(c, scheme, urlt))
			ਜਾਓ error;
	} ਜਦੋਂ (cur < ਸਿਰਾ);

	lwc_string_unref(host);
	lwc_string_unref(ਰਸਤਾ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ਵਾਪਸ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(ਰਸਤਾ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ਵਾਪਸ false;
}


/* exported interface documented in content/urldb.h */
ਅੱਖਰ *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ਅਚਲ ਬਣਤਰ ਰਸਤਾ_ਅੰਕੜੇ *p, *q;
	ਅਚਲ ਬਣਤਰ host_part *h;
	lwc_string *ਰਸਤਾ_lwc;
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0, ਸੰਸਕਰਣ = COOKIE_RFC2965;
	ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ **matched_cookies;
	ਪੂਰਨ matched_cookies_ਮਾਪ = 20;
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ;
	ਅੱਖਰ *ret;
	lwc_string *scheme;
	time_t now;
	bool ਮੇਲ;

	ਪ੍ਰਤਿਜ੍ਞਾ(url != ਨਿਰਾ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_ਜੋੜੋ_url(url);

	p = urldb_ਲੱਭੋ_url(url);
	ਜੇ (!p)
		ਵਾਪਸ ਨਿਰਾ;

	scheme = p->scheme;

	matched_cookies = ਸਮ੍ਰਿਤੀ_ਦਿਓ(matched_cookies_ਮਾਪ *
				 ਆਕਾਰ(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *));
	ਜੇ (!matched_cookies)
		ਵਾਪਸ ਨਿਰਾ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GROW_MATCHED_COOKIES						\
	ਕਰੋ {								\
		ਜੇ (ਗਿਣਤੀ_2 == matched_cookies_ਮਾਪ) {			\
			ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ **ਆਰਜ਼ੀ;		\
			ਆਰਜ਼ੀ = realloc(matched_cookies,			\
				       (matched_cookies_ਮਾਪ + 20) *	\
				       ਆਕਾਰ(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *)); \
									\
			ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ) {				\
				ਮੁਕਤ(matched_cookies);			\
				ਵਾਪਸ ਨਿਰਾ;				\
			}						\
									\
			matched_cookies = ਆਰਜ਼ੀ;				\
			matched_cookies_ਮਾਪ += 20;			\
		}							\
	} ਜਦੋਂ(0)

	ਰਸਤਾ_lwc = nsurl_get_component(url, NSURL_ਰਸਤਾ);
	ਜੇ (ਰਸਤਾ_lwc == ਨਿਰਾ) {
		ਮੁਕਤ(matched_cookies);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਰਸਤਾ = lwc_string_ਅੰਕੜੇ(ਰਸਤਾ_lwc);
	lwc_string_unref(ਰਸਤਾ_lwc);

	now = time(ਨਿਰਾ);

	ਜੇ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		ਲਈ (q = p->ਮਾਪਾ->children; q; q = q->ਅਗਲਾ) {
			ਜੇ (strcmp(q->segment, p->segment))
				ਜਾਰੀ;

			/* Consider all cookies associated with
			 * this exact path */
			ਲਈ (c = q->cookies; c; c = c->ਅਗਲਾ) {
				ਜੇ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ਜਾਰੀ;

				ਜੇ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &ਮੇਲ) &&
				    ਮੇਲ == false)
					/* secure cookie for insecure host.
					 * ignore */
					ਜਾਰੀ;

				ਜੇ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ਜਾਰੀ;

				matched_cookies[ਗਿਣਤੀ_2++] = c;

				GROW_MATCHED_COOKIES;

				ਜੇ (c->ਸੰਸਕਰਣ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਸੰਸਕਰਣ)
					ਸੰਸਕਰਣ = c->ਸੰਸਕਰਣ;

				c->ਆਖਰੀ_ਵਰਤਿਆ = now;

				cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	ਲਈ (p = p->ਮਾਪਾ; p; p = p->ਮਾਪਾ) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		ਲਈ (q = p->children; q; q = q->ਅਗਲਾ) {
			ਜੇ (*(q->segment) != '\0')
				ਜਾਰੀ;

			ਲਈ (c = q->cookies; c; c = c->ਅਗਲਾ) {
				ਜੇ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ਜਾਰੀ;

				ਜੇ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &ਮੇਲ) &&
				    ਮੇਲ == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ਜਾਰੀ;

				matched_cookies[ਗਿਣਤੀ_2++] = c;

				GROW_MATCHED_COOKIES;

				ਜੇ (c->ਸੰਸਕਰਣ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) ਸੰਸਕਰਣ)
					ਸੰਸਕਰਣ = c->ਸੰਸਕਰਣ;

				c->ਆਖਰੀ_ਵਰਤਿਆ = now;

				cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
			}
		}

		ਜੇ (!p->ਮਾਪਾ) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ਰੋਕੋ;
		}

		/* Consider p itself - may be the result of Path=/foo */
		ਲਈ (c = p->cookies; c; c = c->ਅਗਲਾ) {
			ਜੇ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ਜਾਰੀ;

			/* Ensure cookie path is a prefix of the resource */
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(c->ਰਸਤਾ, ਰਸਤਾ, ਲੜੀ_ਲੰਬਾਈ(c->ਰਸਤਾ)) != 0)
				/* paths don't match => ignore */
				ਜਾਰੀ;

			ਜੇ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &ਮੇਲ) &&
			    ਮੇਲ == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ਜਾਰੀ;

			matched_cookies[ਗਿਣਤੀ_2++] = c;

			GROW_MATCHED_COOKIES;

			ਜੇ (c->ਸੰਸਕਰਣ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) ਸੰਸਕਰਣ)
				ਸੰਸਕਰਣ = c->ਸੰਸਕਰਣ;

			c->ਆਖਰੀ_ਵਰਤਿਆ = now;

			cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	ਲਈ (h = (ਅਚਲ ਬਣਤਰ host_part *)p; h && h != &db_ਜੜ੍ਹ;
	     h = h->ਮਾਪਾ) {
		ਲਈ (c = h->paths.cookies; c; c = c->ਅਗਲਾ) {
			ਜੇ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ਜਾਰੀ;

			/* Ensure cookie path is a prefix of the resource */
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(c->ਰਸਤਾ, ਰਸਤਾ, ਲੜੀ_ਲੰਬਾਈ(c->ਰਸਤਾ)) != 0)
				/* paths don't match => ignore */
				ਜਾਰੀ;

			ਜੇ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &ਮੇਲ) &&
			    ਮੇਲ == false)
				/* secure cookie for insecure host. ignore */
				ਜਾਰੀ;

			matched_cookies[ਗਿਣਤੀ_2++] = c;

			GROW_MATCHED_COOKIES;

			ਜੇ (c->ਸੰਸਕਰਣ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਸੰਸਕਰਣ)
				ਸੰਸਕਰਣ = c->ਸੰਸਕਰਣ;

			c->ਆਖਰੀ_ਵਰਤਿਆ = now;

			cookie_manager_ਜੋੜੋ((ਬਣਤਰ cookie_ਅੰਕੜੇ *)c);
		}
	}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ GROW_MATCHED_COOKIES

	ਜੇ (ਗਿਣਤੀ_2 == 0) {
		/* No cookies found */
		ਮੁਕਤ(matched_cookies);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (ਸੰਸਕਰਣ == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, ਗਿਣਤੀ_2);
	} ਨਹੀਂਤਾਂ {
		ret = get_rfc2965_cookie(matched_cookies, ਗਿਣਤੀ_2, ਸੰਸਕਰਣ);
	}
	ਮੁਕਤ(matched_cookies);

	ਵਾਪਸ ret;
}


/* exported interface documented in content/urldb.h */
ਖਾਲੀ urldb_delete_cookie(ਅਚਲ ਅੱਖਰ *domain, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
			 ਅਚਲ ਅੱਖਰ *name)
{
	urldb_delete_cookie_hosts(domain, ਰਸਤਾ, name, &db_ਜੜ੍ਹ);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_ਲੱਦੋ_cookies(ਅਚਲ ਅੱਖਰ *filename)
{
	FILE *fp;
	ਅੱਖਰ s[16*1024];

	ਪ੍ਰਤਿਜ੍ਞਾ(filename);

	fp = fopen(filename, "r");
	ਜੇ (!fp)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਭੋ_T {				\
		ਲਈ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ਜੇ (p >= ਸਿਰਾ) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ਜਾਰੀ;		\
		}				\
		*p++ = '\0';			\
	}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SKIP_T {				\
		ਲਈ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ਜੇ (p >= ਸਿਰਾ) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ਜਾਰੀ;		\
		}				\
	}

	ਜਦੋਂ (fgets(s, ਆਕਾਰ s, fp)) {
		ਅੱਖਰ *p = s, *ਸਿਰਾ = 0,
			*domain, *ਰਸਤਾ, *name, *ਮੁੱਲ, *scheme, *url,
			*comment;
		ਪੂਰਨ ਸੰਸਕਰਣ, domain_specified, ਰਸਤਾ_specified,
			secure, http_only, no_destroy, ਮੁੱਲ_quoted;
		time_t expires, ਆਖਰੀ_ਵਰਤਿਆ;
		ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ *c;

		ਜੇ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ਜਾਰੀ;

		s[ਲੜੀ_ਲੰਬਾਈ(s) - 1] = '\0'; /* lose terminating newline */
		ਸਿਰਾ = s + ਲੜੀ_ਲੰਬਾਈ(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ਜੇ (strncasecmp(s, "Version:", 8) == 0) {
			ਲੱਭੋ_T; SKIP_T; loaded_cookie_file_ਸੰਸਕਰਣ = atoi(p);

			ਜੇ (loaded_cookie_file_ਸੰਸਕਰਣ <
			    MIN_COOKIE_FILE_ਸੰਸਕਰਣ) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ਰੋਕੋ;
			}

			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ (loaded_cookie_file_ਸੰਸਕਰਣ == 0) {
			/* Haven't yet seen version; skip this input */
			ਜਾਰੀ;
		}

		/* One cookie/line */

		/* Parse input */
		ਲੱਭੋ_T; ਸੰਸਕਰਣ = atoi(s);
		SKIP_T; domain = p; ਲੱਭੋ_T;
		SKIP_T; domain_specified = atoi(p); ਲੱਭੋ_T;
		SKIP_T; ਰਸਤਾ = p; ਲੱਭੋ_T;
		SKIP_T; ਰਸਤਾ_specified = atoi(p); ਲੱਭੋ_T;
		SKIP_T; secure = atoi(p); ਲੱਭੋ_T;
		ਜੇ (loaded_cookie_file_ਸੰਸਕਰਣ > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); ਲੱਭੋ_T;
		} ਨਹੀਂਤਾਂ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); ਲੱਭੋ_T;
		SKIP_T; ਆਖਰੀ_ਵਰਤਿਆ = (time_t)atoi(p); ਲੱਭੋ_T;
		SKIP_T; no_destroy = atoi(p); ਲੱਭੋ_T;
		SKIP_T; name = p; ਲੱਭੋ_T;
		SKIP_T; ਮੁੱਲ = p; ਲੱਭੋ_T;
		ਜੇ (loaded_cookie_file_ਸੰਸਕਰਣ > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	ਮੁੱਲ_quoted = atoi(p); ਲੱਭੋ_T;
		} ਨਹੀਂਤਾਂ {
			ਮੁੱਲ_quoted = 0;
		}
		SKIP_T; scheme = p; ਲੱਭੋ_T;
		SKIP_T; url = p; ਲੱਭੋ_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		ਲਈ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ਪ੍ਰਤਿਜ੍ਞਾ(p <= ਸਿਰਾ);

		/* Now create cookie */
		c = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ cookie_ਅੰਦਰੂਨੀ_ਅੰਕੜੇ));
		ਜੇ (!c)
			ਰੋਕੋ;

		c->name = strdup(name);
		c->ਮੁੱਲ = strdup(ਮੁੱਲ);
		c->ਮੁੱਲ_was_quoted = ਮੁੱਲ_quoted;
		c->comment = strdup(comment);
		c->domain_from_ਸੈੱਟ = domain_specified;
		c->domain = strdup(domain);
		c->ਰਸਤਾ_from_ਸੈੱਟ = ਰਸਤਾ_specified;
		c->ਰਸਤਾ = strdup(ਰਸਤਾ);
		c->expires = expires;
		c->ਆਖਰੀ_ਵਰਤਿਆ = ਆਖਰੀ_ਵਰਤਿਆ;
		c->secure = secure;
		c->http_only = http_only;
		c->ਸੰਸਕਰਣ = ਸੰਸਕਰਣ;
		c->no_destroy = no_destroy;

		ਜੇ (!(c->name && c->ਮੁੱਲ && c->comment &&
		      c->domain && c->ਰਸਤਾ)) {
			urldb_free_cookie(c);
			ਰੋਕੋ;
		}

		ਜੇ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ਨਿਰਾ;
			nsurl *url_nsurl = ਨਿਰਾ;

			ਪ੍ਰਤਿਜ੍ਞਾ(scheme[0] != 'u');

			ਜੇ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ਰੋਕੋ;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ਜੇ (!urldb_ਪਾਓ_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ਰੋਕੋ;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ਨਹੀਂਤਾਂ {
			ਜੇ (!urldb_ਪਾਓ_cookie(c, ਨਿਰਾ, ਨਿਰਾ)) {
				/* Cookie freed for us */
				ਰੋਕੋ;
			}
		}
	}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SKIP_T
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਲੱਭੋ_T

	fclose(fp);

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ਅਚਲ ਅੱਖਰ *filename)
{
	FILE *fp;
	ਪੂਰਨ cookie_file_ਸੰਸਕਰਣ = max(loaded_cookie_file_ਸੰਸਕਰਣ,
				      COOKIE_FILE_ਸੰਸਕਰਣ);

	ਪ੍ਰਤਿਜ੍ਞਾ(filename);

	fp = fopen(filename, "w");
	ਜੇ (!fp)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

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
		cookie_file_ਸੰਸਕਰਣ);
	fprintf(fp, "Version:\t%d\n", cookie_file_ਸੰਸਕਰਣ);

	urldb_save_cookie_hosts(fp, &db_ਜੜ੍ਹ);

	fclose(fp);

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
ਖਾਲੀ urldb_dump(ਖਾਲੀ)
{
	ਪੂਰਨ i;

	urldb_dump_hosts(&db_ਜੜ੍ਹ);

	ਲਈ (i = 0; i != ਅੰਕ_ਖੋਜ_TREES; i++) {
		urldb_dump_ਖੋਜ(ਖੋਜ_trees[i], 0);
	}
}
