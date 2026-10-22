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
#ifdef WITH_NSPSL
#ಅನ್ತರ್ಭಾವ <nspsl.h>
#endif

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
ಸಂರಚನಾ cookie_internal_data {
	ಸಂರಚನಾ cookie_internal_data *prev;	/**< Previous in list */
	ಸಂರಚನಾ cookie_internal_data *next;	/**< Next in list */

	ವರ್ಣ *name;		/**< Cookie name */
	ವರ್ಣ *ಮೌಲ್ಯ;		/**< Cookie value */
	bool ಮೌಲ್ಯ_was_quoted;	/**< Value was quoted in Set-Cookie: */
	ವರ್ಣ *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	ವರ್ಣ *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	ವರ್ಣ *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ಗಣನಾ cookie_version version;	/**< Specification compliance */
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
ಸಂರಚನಾ prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned ಪೂರ್ಣಾಙ್ಕ port;
	/** Protection realm */
	ವರ್ಣ *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	ವರ್ಣ *auth;
	/** Next sibling */
	ಸಂರಚನಾ prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ಸಂರಚನಾ url_internal_data {
	ವರ್ಣ *title;		/**< Resource title */
	unsigned ಪೂರ್ಣಾಙ್ಕ visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ಸಂರಚನಾ path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned ಪೂರ್ಣಾಙ್ಕ port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	ವರ್ಣ *segment;		/**< Path segment for this node */
	unsigned ಪೂರ್ಣಾಙ್ಕ frag_cnt;	/**< Number of entries in path_data::fragment */
	ವರ್ಣ **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ಸಂರಚನಾ url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ಧ್ರುವ ಸಂರಚನಾ prot_space_data *prot_space;
	/** Cookies associated with resource */
	ಸಂರಚನಾ cookie_internal_data *cookies;
	/** Last cookie in list */
	ಸಂರಚನಾ cookie_internal_data *cookies_end;

	ಸಂರಚನಾ path_data *next;	/**< Next sibling */
	ಸಂರಚನಾ path_data *prev;	/**< Previous sibling */
	ಸಂರಚನಾ path_data *parent; /**< Parent path segment */
	ಸಂರಚನಾ path_data *children; /**< Child path segments */
	ಸಂರಚನಾ path_data *last; /**< Last child */
};

ಸಂರಚನಾ hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ಸಂರಚನಾ host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ಸಂರಚನಾ path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ಸಂರಚನಾ hsts_data hsts;

	/**
	 * Part of host string
	 */
	ವರ್ಣ *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ಸಂರಚನಾ prot_space_data *prot_space;

	ಸಂರಚನಾ host_part *next;	/**< Next sibling */
	ಸಂರಚನಾ host_part *prev;	/**< Previous sibling */
	ಸಂರಚನಾ host_part *parent; /**< Parent host part */
	ಸಂರಚನಾ host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು {
	ಧ್ರುವ ಸಂರಚನಾ host_part *data;	/**< Host tree entry */

	unsigned ಪೂರ್ಣಾಙ್ಕ level;		/**< Node level */

	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಎಡ;	/**< Left subtree */
	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬಲ;	/**< Right subtree */
};

/** Root database handle */
ಸ್ಥಿರ ಸಂರಚನಾ host_part db_ಬೇರು;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define ಅಂಕೆ_ಹುಡುಕು_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು empty = { 0, 0, &empty, &empty };
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಹುಡುಕು_trees[ಅಂಕೆ_ಹುಡುಕು_TREES] = {
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty
};

/** Minimum cookie database file version */
#define MIN_COOKIE_FILE_VERSION 100
/** Current cookie database file version */
#define COOKIE_FILE_VERSION 102
/** loaded cookie file version */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loaded_cookie_file_version;

/** Minimum URL database file version */
#define MIN_URL_FILE_VERSION 106
/** Current URL database file version */
#define URL_FILE_VERSION 107

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
ಸ್ಥಿರ ಸಂರಚನಾ bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#define BLOOM_SIZE (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
ಸ್ಥಿರ nserror urldb_write_timet(FILE *fp, time_t val)
{
	ಪೂರ್ಣಾಙ್ಕ use;
	ವರ್ಣ op[32];

	use = nsc_sntimet(op, 32, &val);
	ಯದಿ (use == 0) {
		fprintf(fp, "%i\n", (ಪೂರ್ಣಾಙ್ಕ)val);
	} ಅನ್ಯಥಾ {
		fprintf(fp, "%.*s\n", use, op);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
urldb_write_paths(ಧ್ರುವ ಸಂರಚನಾ path_data *parent,
		  ಧ್ರುವ ವರ್ಣ *host,
		  FILE *fp,
		  ವರ್ಣ **path,
		  ಪೂರ್ಣಾಙ್ಕ *path_alloc,
		  ಪೂರ್ಣಾಙ್ಕ *path_used,
		  time_t expiry)
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p = parent;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಕುರು {
		ಪೂರ್ಣಾಙ್ಕ seglen = p->segment != ರಿಕ್ತ ? strlen(p->segment) : 0;
		ಪೂರ್ಣಾಙ್ಕ len = *path_used + seglen + 1;

		ಯದಿ (*path_alloc < len) {
			ವರ್ಣ *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			ಯದಿ (!temp) {
				ಪ್ರತ್ಯಾಗಮ;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		ಯದಿ (p->segment != ರಿಕ್ತ) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		ಯದಿ (p->children != ರಿಕ್ತ) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} ಅನ್ಯಥಾ {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		ಯದಿ (p->children != ರಿಕ್ತ) {
			/* Drill down into children */
			p = p->children;
		} ಅನ್ಯಥಾ {
			/* leaf node */
			ಯದಿ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				ಯದಿ (p->port) {
					fprintf(fp,"%d\n", p->port);
				} ಅನ್ಯಥಾ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (ಪೂರ್ಣಾಙ್ಕ)p->urld.type);

				fprintf(fp, "\n");

				ಯದಿ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					ಪ್ರತಿ (i = 0; s[i] != '\0'; i++)
						ಯದಿ (s[i] < 32)
							s[i] = ' ';
					ಪ್ರತಿ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ಅನ್ಯಥಾ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ಯಾವತ್ (p != parent) {
				ಪೂರ್ಣಾಙ್ಕ seglen = p->segment != ರಿಕ್ತ
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				ಯದಿ (p->next != ರಿಕ್ತ) {
					/* Have a sibling, process that */
					p = p->next;
					ಭಙ್ಗ;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ಸ್ಥಿರ ಶೂನ್ಯ
urldb_ಎಣಿಕೆ_urls(ಧ್ರುವ ಸಂರಚನಾ path_data *ಬೇರು,
		 time_t expiry,
		 unsigned ಪೂರ್ಣಾಙ್ಕ *ಎಣಿಕೆ)
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p = ಬೇರು;

	ಕುರು {
		ಯದಿ (p->children != ರಿಕ್ತ) {
			/* Drill down into children */
			p = p->children;
		} ಅನ್ಯಥಾ {
			/* No more children, increment count if required */
			ಯದಿ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*ಎಣಿಕೆ)++;
			}

			/* Now, find next node to process. */
			ಯಾವತ್ (p != ಬೇರು) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					/* Have a sibling, process that */
					p = p->next;
					ಭಙ್ಗ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != ಬೇರು);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *parent, FILE *fp)
{
	ವರ್ಣ host[256];
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	unsigned ಪೂರ್ಣಾಙ್ಕ path_ಎಣಿಕೆ = 0;
	ವರ್ಣ *path, *p, *end;
	ಪೂರ್ಣಾಙ್ಕ path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	ಪೂರ್ಣಾಙ್ಕ hsts_include_subdomains = 0;

	expiry = time(ರಿಕ್ತ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ಯದಿ (parent == &empty)
		ಪ್ರತ್ಯಾಗಮ;

	urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(parent->ಎಡ, fp);

	path = malloc(path_alloc);
	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ;

	path[0] = '\0';

	ಪ್ರತಿ (h = parent->data, p = host, end = host + ಪರಿಮಾಣ host;
	     h && h != &db_ಬೇರು && p < end; h = h->parent) {
		ಪೂರ್ಣಾಙ್ಕ written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		ಯದಿ (written < 0) {
			ಮುಕ್ತ(path);
			ಪ್ರತ್ಯಾಗಮ;
		}
		p += written;
	}

	h = parent->data;
	ಯದಿ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_ಎಣಿಕೆ_urls(&parent->data->paths, expiry, &path_ಎಣಿಕೆ);

	ಯದಿ (path_ಎಣಿಕೆ > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_ಎಣಿಕೆ);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} ಅನ್ಯಥಾ ಯದಿ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	ಮುಕ್ತ(path);

	urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(parent->ಬಲ, fp);
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
urldb_iterate_entries_path(ಧ್ರುವ ಸಂರಚನಾ path_data *parent,
		bool (*url_callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data),
		bool (*cookie_callback)(ಧ್ರುವ ಸಂರಚನಾ cookie_data *data))
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p = parent;
	ಧ್ರುವ ಸಂರಚನಾ cookie_data *c;

	ಕುರು {
		ಯದಿ (p->children != ರಿಕ್ತ) {
			/* Drill down into children */
			p = p->children;
		} ಅನ್ಯಥಾ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ಪ್ರತಿಜ್ಞಾ(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ಯದಿ (url_callback) {
				ಧ್ರುವ ಸಂರಚನಾ url_internal_data *u = &p->urld;

				ಪ್ರತಿಜ್ಞಾ(p->url);

				ಯದಿ (!url_callback(p->url,
						  (ಧ್ರುವ ಸಂರಚನಾ url_data *) u))
					ಪ್ರತ್ಯಾಗಮ false;
			} ಅನ್ಯಥಾ {
				c = (ಧ್ರುವ ಸಂರಚನಾ cookie_data *)p->cookies;
				ಪ್ರತಿ (; c != ರಿಕ್ತ; c = c->next) {
					ಯದಿ (!cookie_callback(c))
						ಪ್ರತ್ಯಾಗಮ false;
				}
			}

			/* Now, find next node to process. */
			ಯಾವತ್ (p != parent) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					/* Have a sibling, process that */
					p = p->next;
					ಭಙ್ಗ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != parent);

	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool urldb__host_is_ip_address(ಧ್ರುವ ವರ್ಣ *host)
{
	ಸಂರಚನಾ in_addr ipv4;
	size_t host_len = strlen(host);
	ಧ್ರುವ ವರ್ಣ *sane_host;
	ಧ್ರುವ ವರ್ಣ *slash;
#ifndef NO_IPV6
	ಸಂರಚನಾ in6_addr ipv6;
	ವರ್ಣ ipv6_addr[64];
	unsigned ಪೂರ್ಣಾಙ್ಕ ipv6_addr_len;
#endif
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
	ಯದಿ (slash == ರಿಕ್ತ) {
		sane_host = host;
	} ಅನ್ಯಥಾ {
		ವರ್ಣ *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ಯದಿ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		ಗಚ್ಛ out_false;

	ಯದಿ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_dots = 0;
		size_t index;

		ಪ್ರತಿ (index = 0; index < host_len; index++) {
			ಯದಿ (sane_host[index] == '.')
				ಅಂಕೆ_dots++;
		}

		ಯದಿ (ಅಂಕೆ_dots == 3)
			ಗಚ್ಛ out_true;
		ಅನ್ಯಥಾ
			ಗಚ್ಛ out_false;
	}

#ifndef NO_IPV6
	ಯದಿ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		ಗಚ್ಛ out_false;
	}

	ipv6_addr_len = host_len - 2;
	ಯದಿ (ipv6_addr_len >= ಪರಿಮಾಣ(ipv6_addr)) {
		ipv6_addr_len = ಪರಿಮಾಣ(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ಯದಿ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		ಗಚ್ಛ out_true;
#endif

out_false:
	ಯದಿ (slash != ರಿಕ್ತ) ಮುಕ್ತ((ಶೂನ್ಯ *)sane_host);
	ಪ್ರತ್ಯಾಗಮ false;

out_true:
	ಯದಿ (slash != ರಿಕ್ತ) ಮುಕ್ತ((ಶೂನ್ಯ *)sane_host);
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ urldb_ಹುಡುಕು_match_prefix(ಧ್ರುವ ಸಂರಚನಾ host_part *a, ಧ್ರುವ ವರ್ಣ *b)
{
	ಧ್ರುವ ವರ್ಣ *end, *dot;
	ಪೂರ್ಣಾಙ್ಕ plen, ret;

	ಪ್ರತಿಜ್ಞಾ(a && a != &db_ಬೇರು && b);

	ಯದಿ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ಪ್ರತ್ಯಾಗಮ strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ಯಾವತ್ (b < end && a && a != &db_ಬೇರು) {
		dot = strchr(b, '.');
		ಯದಿ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ಯದಿ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ಪ್ರತ್ಯಾಗಮ ret;

		/* The strings matched */
		ಯದಿ (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			ಯದಿ (plen > dot - b) {
				/* len(a) > len(b) */
				ಪ್ರತ್ಯಾಗಮ 1;
			} ಅನ್ಯಥಾ ಯದಿ (plen < dot - b) {
				/* len(a) < len(b) */
				ಪ್ರತ್ಯಾಗಮ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಯದಿ (a && a != &db_ಬೇರು && b >= end) {
		/* len(a) > len(b) => prefix matches */
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ ಯದಿ ((!a || a == &db_ಬೇರು) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* Identical */
	ಪ್ರತ್ಯಾಗಮ 0;
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
urldb_iterate_partial_host(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು,
		ಧ್ರುವ ವರ್ಣ *prefix,
		bool (*callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data))
{
	ಪೂರ್ಣಾಙ್ಕ c;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && prefix && callback);

	ಯದಿ (ಬೇರು == &empty)
		ಪ್ರತ್ಯಾಗಮ true;

	c = urldb_ಹುಡುಕು_match_prefix(ಬೇರು->data, prefix);

	ಯದಿ (c > 0) {
		/* No match => look in left subtree */
		ಪ್ರತ್ಯಾಗಮ urldb_iterate_partial_host(ಬೇರು->ಎಡ,
						  prefix,
						  callback);
	} ಅನ್ಯಥಾ ಯದಿ (c < 0) {
		/* No match => look in right subtree */
		ಪ್ರತ್ಯಾಗಮ urldb_iterate_partial_host(ಬೇರು->ಬಲ,
						  prefix,
						  callback);
	} ಅನ್ಯಥಾ {
		/* Match => iterate over l/r subtrees & process this node */
		ಯದಿ (!urldb_iterate_partial_host(ಬೇರು->ಎಡ,
						prefix,
						callback)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (ಬೇರು->data->paths.children) {
			/* and extract all paths attached to this host */
			ಯದಿ (!urldb_iterate_entries_path(&ಬೇರು->data->paths,
							callback,
							ರಿಕ್ತ)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}

		ಯದಿ (!urldb_iterate_partial_host(ಬೇರು->ಬಲ,
						prefix,
						callback)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
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
urldb_iterate_partial_path(ಧ್ರುವ ಸಂರಚನಾ path_data *parent,
		ಧ್ರುವ ವರ್ಣ *prefix,
		bool (*callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data))
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p = parent->children;
	ಧ್ರುವ ವರ್ಣ *slash, *end = prefix + strlen(prefix);

	ಕುರು {
		slash = strchr(prefix, '/');
		ಯದಿ (!slash) {
			slash = end;
		}

		ಯದಿ (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			ಅನುವರ್ತ;
		}

		ಯದಿ (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			ಯದಿ (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ಯದಿ (!urldb_iterate_entries_path(p,
								callback,
								ರಿಕ್ತ)) {
					ಪ್ರತ್ಯಾಗಮ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} ಅನ್ಯಥಾ {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} ಅನ್ಯಥಾ {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ಯಾವತ್ (p != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ true;
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
urldb_iterate_entries_host(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *parent,
		bool (*url_callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data),
		bool (*cookie_callback)(ಧ್ರುವ ಸಂರಚನಾ cookie_data *data))
{
	ಯದಿ (parent == &empty) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಯದಿ (!urldb_iterate_entries_host(parent->ಎಡ,
					url_callback,
					cookie_callback)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ಯದಿ (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಯದಿ (!urldb_iterate_entries_host(parent->ಬಲ,
					url_callback,
					cookie_callback)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಸಂರಚನಾ host_part *
urldb_add_host_ಗಂಟು(ಧ್ರುವ ವರ್ಣ *part, ಸಂರಚನಾ host_part *parent)
{
	ಸಂರಚನಾ host_part *d;

	ಪ್ರತಿಜ್ಞಾ(part && parent);

	d = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ host_part));
	ಯದಿ (!d) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	d->part = strdup(part);
	ಯದಿ (!d->part) {
		ಮುಕ್ತ(d);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	d->next = parent->children;
	ಯದಿ (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	ಪ್ರತ್ಯಾಗಮ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ urldb_add_path_fragment_cmp(ಧ್ರುವ ಶೂನ್ಯ *a, ಧ್ರುವ ಶೂನ್ಯ *b)
{
	ಪ್ರತ್ಯಾಗಮ strcasecmp(*((ಧ್ರುವ ವರ್ಣ **) a), *((ಧ್ರುವ ವರ್ಣ **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಸಂರಚನಾ path_data *
urldb_add_path_fragment(ಸಂರಚನಾ path_data *segment, lwc_string *fragment)
{
	ವರ್ಣ **temp;

	ಪ್ರತಿಜ್ಞಾ(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ಯದಿ (!fragment)
		ಪ್ರತ್ಯಾಗಮ segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * ಪರಿಮಾಣ(ವರ್ಣ *));
	ಯದಿ (!temp)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	ಯದಿ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      ಪರಿಮಾಣ (ವರ್ಣ *),
	      urldb_add_path_fragment_cmp);

	ಪ್ರತ್ಯಾಗಮ segment;
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
ಸ್ಥಿರ ಸಂರಚನಾ path_data *
urldb_add_path_ಗಂಟು(lwc_string *scheme,
		    unsigned ಪೂರ್ಣಾಙ್ಕ port,
		    ಧ್ರುವ ವರ್ಣ *segment,
		    lwc_string *fragment,
		    ಸಂರಚನಾ path_data *parent)
{
	ಸಂರಚನಾ path_data *d, *e;

	ಪ್ರತಿಜ್ಞಾ(scheme && segment && parent);

	d = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ path_data));
	ಯದಿ (!d)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	ಯದಿ (!d->segment) {
		lwc_string_unref(d->scheme);
		ಮುಕ್ತ(d);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (fragment) {
		ಯದಿ (!urldb_add_path_fragment(d, fragment)) {
			ಮುಕ್ತ(d->segment);
			lwc_string_unref(d->scheme);
			ಮುಕ್ತ(d);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	ಪ್ರತಿ (e = parent->children; e; e = e->next) {
		ಯದಿ (strcmp(e->segment, d->segment) > 0)
			ಭಙ್ಗ;
	}

	ಯದಿ (e) {
		d->prev = e->prev;
		d->next = e;
		ಯದಿ (e->prev)
			e->prev->next = d;
		ಅನ್ಯಥಾ
			parent->children = d;
		e->prev = d;
	} ಅನ್ಯಥಾ ಯದಿ (!parent->children) {
		d->prev = d->next = ರಿಕ್ತ;
		parent->children = parent->last = d;
	} ಅನ್ಯಥಾ {
		d->next = ರಿಕ್ತ;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	ಪ್ರತ್ಯಾಗಮ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು **urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_direct(ಧ್ರುವ ವರ್ಣ *host)
{
	ಪ್ರತಿಜ್ಞಾ(host);

	ಯದಿ (urldb__host_is_ip_address(host)) {
		ಪ್ರತ್ಯಾಗಮ &ಹುಡುಕು_trees[ST_IP];
	} ಅನ್ಯಥಾ ಯದಿ (ascii_is_alpha(*host)) {
		ಪ್ರತ್ಯಾಗಮ &ಹುಡುಕು_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	ಪ್ರತ್ಯಾಗಮ &ಹುಡುಕು_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(ಧ್ರುವ ವರ್ಣ *host)
{
	ಪ್ರತ್ಯಾಗಮ *urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ urldb_ಹುಡುಕು_match_string(ಧ್ರುವ ಸಂರಚನಾ host_part *a, ಧ್ರುವ ವರ್ಣ *b)
{
	ಧ್ರುವ ವರ್ಣ *end, *dot;
	ಪೂರ್ಣಾಙ್ಕ plen, ret;

	ಪ್ರತಿಜ್ಞಾ(a && a != &db_ಬೇರು && b);

	ಯದಿ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ಪ್ರತ್ಯಾಗಮ strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ಯಾವತ್ (b < end && a && a != &db_ಬೇರು) {
		dot = strchr(b, '.');
		ಯದಿ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ಯದಿ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ಪ್ರತ್ಯಾಗಮ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		ಯದಿ (plen > dot - b) {
			/* len(a) > len(b) */
			ಪ್ರತ್ಯಾಗಮ 1;
		} ಅನ್ಯಥಾ ಯದಿ (plen < dot - b) {
			/* len(a) < len(b) */
			ಪ್ರತ್ಯಾಗಮ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಯದಿ (a && a != &db_ಬೇರು && b >= end) {
		/* len(a) > len(b) */
		ಪ್ರತ್ಯಾಗಮ 1;
	} ಅನ್ಯಥಾ ಯದಿ ((!a || a == &db_ಬೇರು) && b < end) {
		/* len(a) < len(b) */
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* Identical */
	ಪ್ರತ್ಯಾಗಮ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ host_part *
urldb_ಹುಡುಕು_find(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ಧ್ರುವ ವರ್ಣ *host)
{
	ಪೂರ್ಣಾಙ್ಕ c;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && host);

	ಯದಿ (ಬೇರು == &empty) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	c = urldb_ಹುಡುಕು_match_string(ಬೇರು->data, host);

	ಯದಿ (c > 0) {
		ಪ್ರತ್ಯಾಗಮ urldb_ಹುಡುಕು_find(ಬೇರು->ಎಡ, host);
	} ಅನ್ಯಥಾ ಯದಿ (c < 0) {
		ಪ್ರತ್ಯಾಗಮ urldb_ಹುಡುಕು_find(ಬೇರು->ಬಲ, host);
	}

	ಪ್ರತ್ಯಾಗಮ ಬೇರು->data;
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
ಸ್ಥಿರ ಸಂರಚನಾ path_data *
urldb_match_path(ಧ್ರುವ ಸಂರಚನಾ path_data *parent,
		 ಧ್ರುವ ವರ್ಣ *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p;
	ಧ್ರುವ ವರ್ಣ *slash;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(parent != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(parent->segment == ರಿಕ್ತ);

	ಯದಿ (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	ಪ್ರತಿಜ್ಞಾ(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ಯಾವತ್ (p != ರಿಕ್ತ) {
		slash = strchr(path + 1, '/');
		ಯದಿ (!slash) {
			slash = path + strlen(path);
		}

		ಯದಿ (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			ಯದಿ (*slash == '\0') {
				/* Complete match */
				ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} ಅನ್ಯಥಾ {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ಸ್ಥಿರ ಸಂರಚನಾ path_data *urldb_find_url(nsurl *url)
{
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	ಸಂರಚನಾ path_data *p;
	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ವೃಕ್ಷ;
	ವರ್ಣ *plq;
	ಧ್ರುವ ವರ್ಣ *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ port_int;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(url);

	ಯದಿ (url_bloom != ರಿಕ್ತ) {
		ಯದಿ (bloom_ಹುಡುಕು_hash(url_bloom, nsurl_hash(url)) == false) {
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಯದಿ (scheme == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (host != ರಿಕ್ತ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ಅನ್ಯಥಾ ಯದಿ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ಅನ್ಯಥಾ {
		lwc_string_unref(scheme);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ವೃಕ್ಷ = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(host_str);
	h = urldb_ಹುಡುಕು_find(ವೃಕ್ಷ, host_str);
	ಯದಿ (!h) {
		lwc_string_unref(scheme);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* generate plq (path, leaf, query) */
	ಯದಿ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	ಯದಿ (port != ರಿಕ್ತ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ಅನ್ಯಥಾ {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	ಮುಕ್ತ(plq);
	lwc_string_unref(scheme);

	ಪ್ರತ್ಯಾಗಮ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_paths(ಸಂರಚನಾ path_data *parent)
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p = parent;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಕುರು {
		ಯದಿ (p->segment != ರಿಕ್ತ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			ಪ್ರತಿ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ಯದಿ (p->children != ರಿಕ್ತ) {
			p = p->children;
		} ಅನ್ಯಥಾ {
			ಯಾವತ್ (p != parent) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					p = p->next;
					ಭಙ್ಗ;
				}

				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_hosts(ಸಂರಚನಾ host_part *parent)
{
	ಸಂರಚನಾ host_part *h;

	ಯದಿ (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	ಪ್ರತಿ (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_dump_ಹುಡುಕು(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *parent, ಪೂರ್ಣಾಙ್ಕ depth)
{
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	ಪೂರ್ಣಾಙ್ಕ i; /* index into string */
	ವರ್ಣ s[1024];
	ಪೂರ್ಣಾಙ್ಕ r;
	ಪೂರ್ಣಾಙ್ಕ sl = ಪರಿಮಾಣ(s) - 2;

	ಯದಿ (parent == &empty)
		ಪ್ರತ್ಯಾಗಮ;

	urldb_dump_ಹುಡುಕು(parent->ಎಡ, depth + 1);

	ಪ್ರತಿ (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	ಪ್ರತಿ (h = parent->data; h; h = h->parent) {
		ಯದಿ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ಯದಿ (r < 0) {
				ಭಙ್ಗ;
			}
			ಯದಿ ((i + r) >= sl) {
				ಭಙ್ಗ;
			}
			i += r;
		}

		ಯದಿ (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_ಹುಡುಕು(parent->ಬಲ, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
urldb_ಹುಡುಕು_match_host(ಧ್ರುವ ಸಂರಚನಾ host_part *a, ಧ್ರುವ ಸಂರಚನಾ host_part *b)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಪ್ರತಿಜ್ಞಾ(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	ಪ್ರತಿ (; a && a != &db_ಬೇರು && b && b != &db_ಬೇರು;
	     a = a->parent, b = b->parent) {
		ಯದಿ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ಪ್ರತ್ಯಾಗಮ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ಯದಿ (a && a != &db_ಬೇರು && (!b || b == &db_ಬೇರು)) {
		/* len(a) > len(b) */
		ಪ್ರತ್ಯಾಗಮ 1;
	} ಅನ್ಯಥಾ ಯದಿ ((!a || a == &db_ಬೇರು) && b && b != &db_ಬೇರು) {
		/* len(a) < len(b) */
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* identical */
	ಪ್ರತ್ಯಾಗಮ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *urldb_ಹುಡುಕು_skew(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	ಯದಿ (ಬೇರು->ಎಡ->level == ಬೇರು->level) {
		ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *temp;

		temp = ಬೇರು->ಎಡ;
		ಬೇರು->ಎಡ = temp->ಬಲ;
		temp->ಬಲ = ಬೇರು;
		ಬೇರು = temp;
	}

	ಪ್ರತ್ಯಾಗಮ ಬೇರು;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *urldb_ಹುಡುಕು_split(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	ಯದಿ (ಬೇರು->ಬಲ->ಬಲ->level == ಬೇರು->level) {
		ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *temp;

		temp = ಬೇರು->ಬಲ;
		ಬೇರು->ಬಲ = temp->ಎಡ;
		temp->ಎಡ = ಬೇರು;
		ಬೇರು = temp;

		ಬೇರು->level++;
	}

	ಪ್ರತ್ಯಾಗಮ ಬೇರು;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *
urldb_ಹುಡುಕು_ಸೇರಿಸು_internal(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *n)
{
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && n);

	ಯದಿ (ಬೇರು == &empty) {
		ಬೇರು = n;
	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ c = urldb_ಹುಡುಕು_match_host(ಬೇರು->data, n->data);

		ಯದಿ (c > 0) {
			ಬೇರು->ಎಡ = urldb_ಹುಡುಕು_ಸೇರಿಸು_internal(
				ಬೇರು->ಎಡ, n);
		} ಅನ್ಯಥಾ ಯದಿ (c < 0) {
			ಬೇರು->ಬಲ = urldb_ಹುಡುಕು_ಸೇರಿಸು_internal(
				ಬೇರು->ಬಲ, n);
		} ಅನ್ಯಥಾ {
			/* exact match */
			ಮುಕ್ತ(n);
			ಪ್ರತ್ಯಾಗಮ ಬೇರು;
		}

		ಬೇರು = urldb_ಹುಡುಕು_skew(ಬೇರು);
		ಬೇರು = urldb_ಹುಡುಕು_split(ಬೇರು);
	}

	ಪ್ರತ್ಯಾಗಮ ಬೇರು;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ಸ್ಥಿರ ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *
urldb_ಹುಡುಕು_ಸೇರಿಸು(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು, ಧ್ರುವ ಸಂರಚನಾ host_part *data)
{
	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *n;

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು && data);

	n = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು));
	ಯದಿ (!n)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	n->level = 1;
	n->data = data;
	n->ಎಡ = n->ಬಲ = &empty;

	ಬೇರು = urldb_ಹುಡುಕು_ಸೇರಿಸು_internal(ಬೇರು, n);

	ಪ್ರತ್ಯಾಗಮ ಬೇರು;
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
urldb_parse_avpair(ಸಂರಚನಾ cookie_internal_data *c,
		   ವರ್ಣ *n,
		   ವರ್ಣ *v,
		   bool was_quoted)
{
	ಪೂರ್ಣಾಙ್ಕ vlen;

	ಪ್ರತಿಜ್ಞಾ(c && n && v);

	/* Strip whitespace from start of name */
	ಪ್ರತಿ (; *n; n++) {
		ಯದಿ (*n != ' ' && *n != '\t')
			ಭಙ್ಗ;
	}

	/* Strip whitespace from end of name */
	ಪ್ರತಿ (vlen = strlen(n); vlen; vlen--) {
		ಯದಿ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ಅನ್ಯಥಾ
			ಭಙ್ಗ;
	}

	/* Strip whitespace from start of value */
	ಪ್ರತಿ (; *v; v++) {
		ಯದಿ (*v != ' ' && *v != '\t')
			ಭಙ್ಗ;
	}

	/* Strip whitespace from end of value */
	ಪ್ರತಿ (vlen = strlen(v); vlen; vlen--) {
		ಯದಿ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ಅನ್ಯಥಾ
			ಭಙ್ಗ;
	}

	ಯದಿ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ಯದಿ (!c->comment)
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ ಯದಿ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ಯದಿ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			ಯದಿ (!c->domain)
				ಪ್ರತ್ಯಾಗಮ false;
		}
	} ಅನ್ಯಥಾ ಯದಿ (strcasecmp(n, "Max-Age") == 0) {
		ಪೂರ್ಣಾಙ್ಕ temp = atoi(v);
		ಯದಿ (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ಅನ್ಯಥಾ
			c->expires = time(ರಿಕ್ತ) + temp;
	} ಅನ್ಯಥಾ ಯದಿ (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		ಯದಿ (!c->path)
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ ಯದಿ (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} ಅನ್ಯಥಾ ಯದಿ (strcasecmp(n, "Expires") == 0) {
		ವರ್ಣ *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		ಪ್ರತಿ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		ಯದಿ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ಅನ್ಯಥಾ ಯದಿ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ಅನ್ಯಥಾ ಯದಿ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ಅನ್ಯಥಾ ಯದಿ (!c->name) {
		c->name = strdup(n);
		c->ಮೌಲ್ಯ = strdup(v);
		c->ಮೌಲ್ಯ_was_quoted = was_quoted;
		ಯದಿ (!c->name || !c->ಮೌಲ್ಯ) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_free_cookie(ಸಂರಚನಾ cookie_internal_data *c)
{
	ಪ್ರತಿಜ್ಞಾ(c);

	ಮುಕ್ತ(c->comment);
	ಮುಕ್ತ(c->domain);
	ಮುಕ್ತ(c->path);
	ಮುಕ್ತ(c->name);
	ಮುಕ್ತ(c->ಮೌಲ್ಯ);
	ಮುಕ್ತ(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ಸ್ಥಿರ ಸಂರಚನಾ cookie_internal_data *
urldb_parse_cookie(nsurl *url, ಧ್ರುವ ವರ್ಣ **cookie)
{
	ಸಂರಚನಾ cookie_internal_data *c;
	ಧ್ರುವ ವರ್ಣ *cur;
	ವರ್ಣ name[1024], ಮೌಲ್ಯ[4096];
	ವರ್ಣ *n = name, *v = ಮೌಲ್ಯ;
	bool in_ಮೌಲ್ಯ = false;
	bool had_ಮೌಲ್ಯ_data = false;
	bool ಮೌಲ್ಯ_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ಪ್ರತಿಜ್ಞಾ(url && cookie && *cookie);

	c = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ cookie_internal_data));
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	c->expires = -1;

	name[0] = '\0';
	ಮೌಲ್ಯ[0] = '\0';

	ಪ್ರತಿ (cur = *cookie; *cur; cur++) {
		ಯದಿ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ಯದಿ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				ಮೌಲ್ಯ[0] = '\0';

			}

			ಭಙ್ಗ;
		} ಅನ್ಯಥಾ ಯದಿ (*cur == '\r') {
			/* Spurious linefeed */
			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (*cur == '\n') {
			/* Spurious newline */
			ಅನುವರ್ತ;
		}

		ಯದಿ (in_ಮೌಲ್ಯ && !had_ಮೌಲ್ಯ_data) {
			ಯದಿ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ಅನುವರ್ತ;
			} ಅನ್ಯಥಾ {
				had_ಮೌಲ್ಯ_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ಯದಿ (*cur != '"') {
					ಮೌಲ್ಯ_verbatim = true;
				}
			}
		}

		ಯದಿ (in_ಮೌಲ್ಯ && !ಮೌಲ್ಯ_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ಯದಿ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ಅನುವರ್ತ;
			}
		}

		ಯದಿ (!quoted && !in_ಮೌಲ್ಯ && *cur == '=') {
			/* First equals => attr-value separator */
			in_ಮೌಲ್ಯ = true;
			ಅನುವರ್ತ;
		}

		ಯದಿ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ಯದಿ (!urldb_parse_avpair(c, name, ಮೌಲ್ಯ, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}

			/* And reset to start */
			n = name;
			v = ಮೌಲ್ಯ;
			in_ಮೌಲ್ಯ = false;
			had_ಮೌಲ್ಯ_data = false;
			ಮೌಲ್ಯ_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ಯದಿ (*cur != ';') {
				cur--;
			}

			ಅನುವರ್ತ;
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
		ಯದಿ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ಧ್ರುವ ವರ್ಣ *p;
			ಧ್ರುವ ವರ್ಣ *semi = strchr(cur + 1, ';');
			ಯದಿ (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			ಪ್ರತಿ (p = cur + 1; p < semi; p++)
				ಯದಿ (*p == '=')
					ಭಙ್ಗ;

			ಯದಿ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ಅನ್ಯಥಾ {
				/* found one => comma marks end of cookie */
				cur++;
				ಭಙ್ಗ;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ಯದಿ (!in_ಮೌಲ್ಯ) {
			ಯದಿ (n < name + (ಪರಿಮಾಣ(name) - 1))
				*n++ = *cur;
		} ಅನ್ಯಥಾ {
			ಯದಿ (v < ಮೌಲ್ಯ + (ಪರಿಮಾಣ(ಮೌಲ್ಯ) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ಯದಿ (!urldb_parse_avpair(c, name, ಮೌಲ್ಯ, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Now fix-up default values */
	ಯದಿ (c->domain == ರಿಕ್ತ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ಯದಿ (host == ರಿಕ್ತ) {
			urldb_free_cookie(c);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	ಯದಿ (c->path == ರಿಕ್ತ) {
		ಧ್ರುವ ವರ್ಣ *path_data;
		ವರ್ಣ *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		ಯದಿ (path_lwc == ರಿಕ್ತ) {
			urldb_free_cookie(c);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		ಯದಿ (slash != ರಿಕ್ತ) {
			/* Special case: retain first slash in path */
			ಯದಿ (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			ಯದಿ (slash == ರಿಕ್ತ) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} ಅನ್ಯಥಾ {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			ಯದಿ (path == ರಿಕ್ತ) {
				urldb_free_cookie(c);
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	ಪ್ರತ್ಯಾಗಮ c;
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
ಸ್ಥಿರ ಸಂರಚನಾ path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned ಪೂರ್ಣಾಙ್ಕ port,
	       ಧ್ರುವ ಸಂರಚನಾ host_part *host,
	       ವರ್ಣ *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ಸಂರಚನಾ path_data *d, *e;
	ವರ್ಣ *buf = path_query;
	ವರ್ಣ *segment, *slash;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(scheme && host && url);

	d = (ಸಂರಚನಾ path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ಯದಿ (*segment == '/')
		segment++;

	/* Process path segments */
	ಕುರು {
		slash = strchr(segment, '/');
		ಯದಿ (!slash) {
			/* last segment */
			/* look for existing entry */
			ಪ್ರತಿ (e = d->children; e; e = e->next)
				ಯದಿ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					ಭಙ್ಗ;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_ಗಂಟು(scheme, port,
						    segment, fragment, d);
			ಭಙ್ಗ;
		}

		*slash = '\0';

		/* look for existing entry */
		ಪ್ರತಿ (e = d->children; e; e = e->next)
			ಯದಿ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				ಭಙ್ಗ;

		d = e ? e : urldb_add_path_ಗಂಟು(scheme, port, segment, ರಿಕ್ತ, d);
		ಯದಿ (!d)
			ಭಙ್ಗ;

		segment = slash + 1;
	} ಯಾವತ್ (1);

	ಮುಕ್ತ(path_query);

	ಯದಿ (d && !d->url) {
		/* Insert defragmented URL */
		ಯದಿ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಸಂರಚನಾ host_part *urldb_add_host(ಧ್ರುವ ವರ್ಣ *host)
{
	ಸಂರಚನಾ host_part *d = (ಸಂರಚನಾ host_part *) &db_ಬೇರು, *e;
	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *s;
	ವರ್ಣ buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	ವರ್ಣ *part;

	ಪ್ರತಿಜ್ಞಾ(host);

	ಯದಿ (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		ಪ್ರತಿ (e = d->children; e; e = e->next)
			ಯದಿ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ಪ್ರತ್ಯಾಗಮ e;

		d = urldb_add_host_ಗಂಟು(host, d);

		s = urldb_ಹುಡುಕು_ಸೇರಿಸು(ಹುಡುಕು_trees[ST_IP], d);
		ಯದಿ (!s) {
			/* failed */
			d = ರಿಕ್ತ;
		} ಅನ್ಯಥಾ {
			ಹುಡುಕು_trees[ST_IP] = s;
		}

		ಪ್ರತ್ಯಾಗಮ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, ಪರಿಮಾಣ buf);
	buf[ಪರಿಮಾಣ buf - 1] = '\0';

	/* Process FQDN segments backwards */
	ಕುರು {
		part = strrchr(buf, '.');
		ಯದಿ (!part) {
			/* last segment */
			/* Check for existing entry */
			ಪ್ರತಿ (e = d->children; e; e = e->next)
				ಯದಿ (strcasecmp(buf, e->part) == 0)
					ಭಙ್ಗ;

			ಯದಿ (e) {
				d = e;
			} ಅನ್ಯಥಾ {
				d = urldb_add_host_ಗಂಟು(buf, d);
			}

			/* And insert into search tree */
			ಯದಿ (d) {
				ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು **r;

				r = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ_direct(buf);
				s = urldb_ಹುಡುಕು_ಸೇರಿಸು(*r, d);
				ಯದಿ (!s) {
					/* failed */
					d = ರಿಕ್ತ;
				} ಅನ್ಯಥಾ {
					*r = s;
				}
			}
			ಭಙ್ಗ;
		}

		/* Check for existing entry */
		ಪ್ರತಿ (e = d->children; e; e = e->next)
			ಯದಿ (strcasecmp(part + 1, e->part) == 0)
				ಭಙ್ಗ;

		d = e ? e : urldb_add_host_ಗಂಟು(part + 1, d);
		ಯದಿ (!d)
			ಭಙ್ಗ;

		*part = '\0';
	} ಯಾವತ್ (1);

	ಪ್ರತ್ಯಾಗಮ d;
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
urldb_ಸೇರಿಸು_cookie(ಸಂರಚನಾ cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ಸಂರಚನಾ cookie_internal_data *d;
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	ಸಂರಚನಾ path_data *p;
	time_t now = time(ರಿಕ್ತ);

	ಪ್ರತಿಜ್ಞಾ(c);

	ಯದಿ (c->domain[0] == '.') {
		h = urldb_ಹುಡುಕು_find(
			urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(&(c->domain[1])),
			c->domain + 1);
		ಯದಿ (!h) {
			h = urldb_add_host(c->domain + 1);
			ಯದಿ (!h) {
				urldb_free_cookie(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}

		p = (ಸಂರಚನಾ path_data *) &h->paths;
	} ಅನ್ಯಥಾ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ಪ್ರತಿಜ್ಞಾ(url != ರಿಕ್ತ);
		ಪ್ರತಿಜ್ಞಾ(scheme != ರಿಕ್ತ);

		h = urldb_ಹುಡುಕು_find(
			urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(c->domain),
			c->domain);

		ಯದಿ (!h) {
			h = urldb_add_host(c->domain);
			ಯದಿ (!h) {
				urldb_free_cookie(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), ರಿಕ್ತ, url);
		ಯದಿ (!p) {
			urldb_free_cookie(c);
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* add cookie */
	ಪ್ರತಿ (d = p->cookies; d; d = d->next) {
		ಯದಿ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			ಭಙ್ಗ;
	}

	ಯದಿ (d) {
		ಯದಿ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ಯದಿ (d->next)
				d->next->prev = d->prev;
			ಅನ್ಯಥಾ
				p->cookies_end = d->prev;
			ಯದಿ (d->prev)
				d->prev->next = d->next;
			ಅನ್ಯಥಾ
				p->cookies = d->next;

			cookie_manager_remove((ಸಂರಚನಾ cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ಅನ್ಯಥಾ {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			ಯದಿ (c->next)
				c->next->prev = c;
			ಅನ್ಯಥಾ
				p->cookies_end = c;
			ಯದಿ (c->prev)
				c->prev->next = c;
			ಅನ್ಯಥಾ
				p->cookies = c;

			cookie_manager_remove((ಸಂರಚನಾ cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
		}
	} ಅನ್ಯಥಾ {
		c->prev = p->cookies_end;
		c->next = ರಿಕ್ತ;
		ಯದಿ (p->cookies_end)
			p->cookies_end->next = c;
		ಅನ್ಯಥಾ
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
	}

	ಪ್ರತ್ಯಾಗಮ true;
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
concat_netscape_cookie(ಸಂರಚನಾ cookie_internal_data *c,
		    ಪೂರ್ಣಾಙ್ಕ *used,
		    ಪೂರ್ಣಾಙ್ಕ *alloc,
		    ವರ್ಣ **buf)
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
	ಪೂರ್ಣಾಙ್ಕ max_len;
	ಪೂರ್ಣಾಙ್ಕ outl; /* output length */

	ಪ್ರತಿಜ್ಞಾ(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->ಮೌಲ್ಯ) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	ಯದಿ (max_len >= *alloc) {
		ವರ್ಣ *temp = realloc(*buf, max_len);
		ಯದಿ (!temp) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	ಯದಿ (outl >= (*alloc - *used)) {
		/* output truncated */
		ಪ್ರತ್ಯಾಗಮ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ಯದಿ (c->ಮೌಲ್ಯ_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->ಮೌಲ್ಯ);
		ಯದಿ (outl >= (*alloc - *used)) {
			/* output truncated */
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*used += outl;
	} ಅನ್ಯಥಾ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->ಮೌಲ್ಯ);
		ಯದಿ (outl >= (*alloc - *used)) {
			/* output truncated */
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ಪ್ರತ್ಯಾಗಮ true;
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
concat_rfc2965_cookie(ಸಂರಚನಾ cookie_internal_data *c,
		      ಪೂರ್ಣಾಙ್ಕ *used,
		      ಪೂರ್ಣಾಙ್ಕ *alloc,
		      ವರ್ಣ **buf)
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
	ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ separators = "()<>@,;:\\\"/[]?={} \t";
	ಪೂರ್ಣಾಙ್ಕ max_len;
	ಪೂರ್ಣಾಙ್ಕ outl; /* output length */

	ಪ್ರತಿಜ್ಞಾ(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->ಮೌಲ್ಯ) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	ಯದಿ (max_len >= *alloc) {
		ವರ್ಣ *temp = realloc(*buf, max_len);
		ಯದಿ (!temp) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	ಯದಿ (outl >= (*alloc - *used)) {
		/* output truncated */
		ಪ್ರತ್ಯಾಗಮ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ಯದಿ (c->ಮೌಲ್ಯ_was_quoted ||
	    strpbrk(c->ಮೌಲ್ಯ, separators) != ರಿಕ್ತ) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->ಮೌಲ್ಯ);

	} ಅನ್ಯಥಾ {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->ಮೌಲ್ಯ);
	}
	ಯದಿ (outl >= (*alloc - *used)) {
		/* output truncated */
		ಪ್ರತ್ಯಾಗಮ false;
	}
	*used += outl;

	ಯದಿ (c->path_from_set) {
		/* Path, quoted if necessary */

		ಯದಿ (strpbrk(c->path, separators) != ರಿಕ್ತ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} ಅನ್ಯಥಾ {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		ಯದಿ (outl >= (*alloc - *used)) {
			/* output truncated */
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*used += outl;
	}

	ಯದಿ (c->domain_from_set) {
		/* Domain, quoted if necessary */
		ಯದಿ (strpbrk(c->domain, separators) != ರಿಕ್ತ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} ಅನ್ಯಥಾ {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		ಯದಿ (outl >= (*alloc - *used)) {
			/* output truncated */
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*used += outl;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * build output cookie string in netscape format
 */
ಸ್ಥಿರ ವರ್ಣ *
get_netscape_cookie(ಸಂರಚನಾ cookie_internal_data **matched_cookies, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ಪೂರ್ಣಾಙ್ಕ ret_alloc = 4096, ret_used = 1;
	ವರ್ಣ *ret;
	ಪೂರ್ಣಾಙ್ಕ midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	ಯದಿ (ret == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ret[0] = '\0';

	ಪ್ರತಿ (midx = 0; midx < ಎಣಿಕೆ; midx++) {
		ಯದಿ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			ಮುಕ್ತ(ret);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	ಯದಿ (ret_used <= 2) {
		ಮುಕ್ತ(ret);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	ವರ್ಣ *temp = realloc(ret, ret_used);
	ಯದಿ (temp != ರಿಕ್ತ) {
		ret = temp;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ಸ್ಥಿರ ವರ್ಣ *
get_rfc2965_cookie(ಸಂರಚನಾ cookie_internal_data **matched_cookies,
		   ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ,
		   ಪೂರ್ಣಾಙ್ಕ version)
{
	ಪೂರ್ಣಾಙ್ಕ ret_alloc = 4096; /* return string allocation */
	ಪೂರ್ಣಾಙ್ಕ ret_used = 1; /* return sting buffer used */
	ವರ್ಣ *ret; /* return string buffer */
	ಪೂರ್ಣಾಙ್ಕ midx; /* index into matched cookies */
	ಪೂರ್ಣಾಙ್ಕ outl;

	ret = malloc(ret_alloc);
	ಯದಿ (ret == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	ಯದಿ (outl >= ret_alloc - ret_used) {
		/* overflow */
		ಮುಕ್ತ(ret);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ret_used += outl;

	ಪ್ರತಿ (midx = 0; midx < ಎಣಿಕೆ; midx++) {
		ಯದಿ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			ಮುಕ್ತ(ret);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	/* shrink the output buffer to the required size */
	ವರ್ಣ *temp = realloc(ret, ret_used);
	ಯದಿ (temp != ರಿಕ್ತ) {
		ret = temp;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
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
urldb_delete_cookie_paths(ಧ್ರುವ ವರ್ಣ *domain,
			  ಧ್ರುವ ವರ್ಣ *path,
			  ಧ್ರುವ ವರ್ಣ *name,
			  ಸಂರಚನಾ path_data *parent)
{
	ಸಂರಚನಾ cookie_internal_data *c;
	ಸಂರಚನಾ path_data *p = parent;

	ಪ್ರತಿಜ್ಞಾ(parent);

	ಕುರು {
		ಪ್ರತಿ (c = p->cookies; c; c = c->next) {
			ಯದಿ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				ಯದಿ (c->prev) {
					c->prev->next = c->next;
				} ಅನ್ಯಥಾ {
					p->cookies = c->next;
				}

				ಯದಿ (c->next) {
					c->next->prev = c->prev;
				} ಅನ್ಯಥಾ {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				ಪ್ರತ್ಯಾಗಮ;
			}
		}

		ಯದಿ (p->children) {
			p = p->children;
		} ಅನ್ಯಥಾ {
			ಯಾವತ್ (p != parent) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					p = p->next;
					ಭಙ್ಗ;
				}

				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != parent);
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
urldb_delete_cookie_hosts(ಧ್ರುವ ವರ್ಣ *domain,
			  ಧ್ರುವ ವರ್ಣ *path,
			  ಧ್ರುವ ವರ್ಣ *name,
			  ಸಂರಚನಾ host_part *parent)
{
	ಸಂರಚನಾ host_part *h;
	ಪ್ರತಿಜ್ಞಾ(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	ಪ್ರತಿ (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_cookie_paths(FILE *fp, ಸಂರಚನಾ path_data *parent)
{
	ಸಂರಚನಾ path_data *p = parent;
	time_t now = time(ರಿಕ್ತ);

	ಪ್ರತಿಜ್ಞಾ(fp && parent);

	ಕುರು {
		ಯದಿ (p->cookies != ರಿಕ್ತ) {
			ಸಂರಚನಾ cookie_internal_data *c;

			ಪ್ರತಿ (c = p->cookies; c != ರಿಕ್ತ; c = c->next) {
				ಯದಿ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ಅನುವರ್ತ;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(ಪೂರ್ಣಾಙ್ಕ)c->expires, (ಪೂರ್ಣಾಙ್ಕ)c->last_used,
					c->no_destroy, c->name, c->ಮೌಲ್ಯ,
					c->ಮೌಲ್ಯ_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ಯದಿ (p->children != ರಿಕ್ತ) {
			p = p->children;
		} ಅನ್ಯಥಾ {
			ಯಾವತ್ (p != parent) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					p = p->next;
					ಭಙ್ಗ;
				}

				p = p->parent;
			}
		}
	} ಯಾವತ್ (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_save_cookie_hosts(FILE *fp, ಸಂರಚನಾ host_part *parent)
{
	ಸಂರಚನಾ host_part *h;
	ಪ್ರತಿಜ್ಞಾ(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	ಪ್ರತಿ (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_cookie(ಸಂರಚನಾ cookie_internal_data *c)
{
	ಮುಕ್ತ(c->name);
	ಮುಕ್ತ(c->ಮೌಲ್ಯ);
	ಮುಕ್ತ(c->comment);
	ಮುಕ್ತ(c->domain);
	ಮುಕ್ತ(c->path);

	ಮುಕ್ತ(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_path_ಗಂಟು_content(ಸಂರಚನಾ path_data *ಗಂಟು)
{
	ಸಂರಚನಾ cookie_internal_data *a, *b;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (ಗಂಟು->url != ರಿಕ್ತ) {
		nsurl_unref(ಗಂಟು->url);
	}

	lwc_string_unref(ಗಂಟು->scheme);

	ಮುಕ್ತ(ಗಂಟು->segment);
	ಪ್ರತಿ (i = 0; i < ಗಂಟು->frag_cnt; i++)
		ಮುಕ್ತ(ಗಂಟು->fragment[i]);
	ಮುಕ್ತ(ಗಂಟು->fragment);

	ಮುಕ್ತ(ಗಂಟು->urld.title);

	ಪ್ರತಿ (a = ಗಂಟು->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_prot_space(ಸಂರಚನಾ prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	ಮುಕ್ತ(space->realm);
	ಮುಕ್ತ(space->auth);

	ಮುಕ್ತ(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_path_ವೃಕ್ಷ(ಸಂರಚನಾ path_data *ಬೇರು)
{
	ಸಂರಚನಾ path_data *p = ಬೇರು;

	ಕುರು {
		ಯದಿ (p->children != ರಿಕ್ತ) {
			p = p->children;
		} ಅನ್ಯಥಾ {
			ಸಂರಚನಾ path_data *q = p;

			ಯಾವತ್ (p != ಬೇರು) {
				ಯದಿ (p->next != ರಿಕ್ತ) {
					p = p->next;
					ಭಙ್ಗ;
				}

				p = p->parent;

				urldb_destroy_path_ಗಂಟು_content(q);
				ಮುಕ್ತ(q);

				q = p;
			}

			urldb_destroy_path_ಗಂಟು_content(q);
			ಮುಕ್ತ(q);
		}
	} ಯಾವತ್ (p != ಬೇರು);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_host_ವೃಕ್ಷ(ಸಂರಚನಾ host_part *ಬೇರು)
{
	ಸಂರಚನಾ host_part *a, *b;
	ಸಂರಚನಾ path_data *p, *q;
	ಸಂರಚನಾ prot_space_data *s, *t;

	/* Destroy children */
	ಪ್ರತಿ (a = ಬೇರು->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_ವೃಕ್ಷ(a);
	}

	/* Now clean up paths */
	ಪ್ರತಿ (p = ಬೇರು->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_ವೃಕ್ಷ(p);
	}

	/* Root path */
	urldb_destroy_path_ಗಂಟು_content(&ಬೇರು->paths);

	/* Proctection space data */
	ಪ್ರತಿ (s = ಬೇರು->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	ಮುಕ್ತ(ಬೇರು->part);
	ಮುಕ್ತ(ಬೇರು);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ಸ್ಥಿರ ಶೂನ್ಯ urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ಬೇರು)
{
	/* Destroy children */
	ಯದಿ (ಬೇರು->ಎಡ != &empty)
		urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಬೇರು->ಎಡ);
	ಯದಿ (ಬೇರು->ಬಲ != &empty)
		urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಬೇರು->ಬಲ);

	/* And destroy ourselves */
	ಮುಕ್ತ(ಬೇರು);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_destroy(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ host_part *a, *b;
	ಪೂರ್ಣಾಙ್ಕ i;

	/* Clean up search trees */
	ಪ್ರತಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಯದಿ (ಹುಡುಕು_trees[i] != &empty) {
			urldb_destroy_ಹುಡುಕು_ವೃಕ್ಷ(ಹುಡುಕು_trees[i]);
			ಹುಡುಕು_trees[i] = &empty;
		}
	}

	/* And database */
	ಪ್ರತಿ (a = db_ಬೇರು.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_ವೃಕ್ಷ(a);
	}
	memset(&db_ಬೇರು, 0, ಪರಿಮಾಣ(db_ಬೇರು));

	/* And the bloom filter */
	ಯದಿ (url_bloom != ರಿಕ್ತ) {
		bloom_destroy(url_bloom);
		url_bloom = ರಿಕ್ತ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(ಧ್ರುವ ವರ್ಣ *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	ವರ್ಣ s[MAXIMUM_URL_LENGTH];
	ವರ್ಣ host[256];
	ಸಂರಚನಾ host_part *h;
	ಪೂರ್ಣಾಙ್ಕ urls;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ version;
	ಪೂರ್ಣಾಙ್ಕ length;
	FILE *fp;

	ಪ್ರತಿಜ್ಞಾ(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ಯದಿ (url_bloom == ರಿಕ್ತ)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	ಯದಿ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NEED_DATA;
	}

	version = atoi(s);
	ಯದಿ (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}
	ಯದಿ (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	ಯಾವತ್ (fgets(host, ಪರಿಮಾಣ host, fp)) {
		time_t hsts_expiry = 0;
		ಪೂರ್ಣಾಙ್ಕ hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		ಯದಿ (length == 0) {
			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;
			urls = atoi(s);
			/* Eight fields/url */
			ಪ್ರತಿ (i = 0; i < (8 * urls); i++) {
				ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					ಭಙ್ಗ;
			}
			ಅನುವರ್ತ;
		}

		ಯದಿ (version >= 107) {
			ವರ್ಣ *p = host;
			ಯಾವತ್ (*p && *p != ' ') p++;
			ಯಾವತ್ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ಯಾವತ್ (*p && *p != ' ') p++;
			ಯಾವತ್ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		ಯದಿ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			ಭಙ್ಗ;
		urls = atoi(s);

		/* no URLs => try next host */
		ಯದಿ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ಅನುವರ್ತ;
		}

		/* load the non-corrupt data */
		ಪ್ರತಿ (i = 0; i < urls; i++) {
			ಸಂರಚನಾ path_data *p = ರಿಕ್ತ;
			ವರ್ಣ scheme[64], ports[10];
			ವರ್ಣ url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned ಪೂರ್ಣಾಙ್ಕ port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			ವರ್ಣ *path_query;
			size_t len;

			ಯದಿ (!fgets(scheme, ಪರಿಮಾಣ scheme, fp))
				ಭಙ್ಗ;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			ಯದಿ (!fgets(ports, ಪರಿಮಾಣ ports, fp))
				ಭಙ್ಗ;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;
			length = strlen(s) - 1;
			s[length] = '\0';

			ಯದಿ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, ಪರಿಮಾಣ url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (port ? ":" : ""),
				 (port ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			ಯದಿ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
			}

			ಯದಿ (url_bloom != ರಿಕ್ತ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ಸೇರಿಸು_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ಯದಿ (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			ಯದಿ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;
			ಯದಿ (p)
				p->urld.visits = (unsigned ಪೂರ್ಣಾಙ್ಕ)atoi(s);

			/* entry last use time */
			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				ಭಙ್ಗ;
			}
			ಯದಿ (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;
			ಯದಿ (p)
				p->urld.type = (content_type)atoi(s);

			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;


			ಯದಿ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ಭಙ್ಗ;
			length = strlen(s) - 1;
			ಯದಿ (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				ಯದಿ (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ಧ್ರುವ ವರ್ಣ *filename)
{
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "w");
	ಯದಿ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ಪ್ರತ್ಯಾಗಮ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	ಪ್ರತಿ (i = 0; i != ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		urldb_save_ಹುಡುಕು_ವೃಕ್ಷ(ಹುಡುಕು_trees[i], fp);
	}

	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	ಸಂರಚನಾ path_data *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	ಸಂರಚನಾ host_part *h;
	ಸಂರಚನಾ path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	ಧ್ರುವ ವರ್ಣ *host_str;
	ವರ್ಣ *path_query = ರಿಕ್ತ;
	size_t len;
	bool match;
	unsigned ಪೂರ್ಣಾಙ್ಕ port_int;

	ಪ್ರತಿಜ್ಞಾ(url);

	ಯದಿ (url_bloom == ರಿಕ್ತ)
		url_bloom = bloom_create(BLOOM_SIZE);

	ಯದಿ (url_bloom != ರಿಕ್ತ) {
		uint32_t hash = nsurl_hash(url);
		bloom_ಸೇರಿಸು_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ಯದಿ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತಿಜ್ಞಾ(path_query != ರಿಕ್ತ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಯದಿ (scheme == ರಿಕ್ತ) {
		ಮುಕ್ತ(path_query);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (host != ರಿಕ್ತ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ಅನ್ಯಥಾ ಯದಿ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ಅನ್ಯಥಾ {
		lwc_string_unref(scheme);
		ಮುಕ್ತ(path_query);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	ಯದಿ (port != ರಿಕ್ತ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ಅನ್ಯಥಾ {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	ಯದಿ (h != ರಿಕ್ತ) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} ಅನ್ಯಥಾ {
		p = ರಿಕ್ತ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ಪ್ರತ್ಯಾಗಮ (p != ರಿಕ್ತ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, ಧ್ರುವ ವರ್ಣ *title)
{
	ಸಂರಚನಾ path_data *p;
	ವರ್ಣ *temp;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (p == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	ಯದಿ (title != ರಿಕ್ತ) {
		temp = strdup(title);
		ಯದಿ (temp == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	} ಅನ್ಯಥಾ {
		temp = ರಿಕ್ತ;
	}

	ಮುಕ್ತ(p->urld.title);
	p->urld.title = temp;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	ಸಂರಚನಾ path_data *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	ಸಂರಚನಾ path_data *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(ರಿಕ್ತ);
	p->urld.visits++;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_reset_url_visit_data(nsurl *url)
{
	ಸಂರಚನಾ path_data *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ಧ್ರುವ ಸಂರಚನಾ url_data *urldb_get_url_data(nsurl *url)
{
	ಸಂರಚನಾ path_data *p;
	ಸಂರಚನಾ url_internal_data *u;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	u = &p->urld;

	ಪ್ರತ್ಯಾಗಮ (ಧ್ರುವ ಸಂರಚನಾ url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ಸಂರಚನಾ path_data *p;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ p->url;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_set_auth_details(nsurl *url, ಧ್ರುವ ವರ್ಣ *realm, ಧ್ರುವ ವರ್ಣ *auth)
{
	ಸಂರಚನಾ path_data *p, *pi;
	ಸಂರಚನಾ host_part *h;
	ಸಂರಚನಾ prot_space_data *space, *space_alloc;
	ವರ್ಣ *realm_alloc, *auth_alloc;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ;

	/* Search for host_part */
	ಪ್ರತಿ (pi = p; pi->parent != ರಿಕ್ತ; pi = pi->parent)
		;
	h = (ಸಂರಚನಾ host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	ಪ್ರತಿ (space = h->prot_space; space; space = space->next) {
		ಯದಿ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			ಭಙ್ಗ;
	}

	ಯದಿ (space != ರಿಕ್ತ) {
		/* Overrule existing auth. */
		ಮುಕ್ತ(space->auth);
		space->auth = strdup(auth);
	} ಅನ್ಯಥಾ {
		/* Create a new protection space. */
		space = space_alloc = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ಯದಿ (!space_alloc || !realm_alloc || !auth_alloc) {
			ಮುಕ್ತ(space_alloc);
			ಮುಕ್ತ(realm_alloc);
			ಮುಕ್ತ(auth_alloc);
			ಪ್ರತ್ಯಾಗಮ;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->port = p->port;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->next = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ಧ್ರುವ ವರ್ಣ *urldb_get_auth_details(nsurl *url, ಧ್ರುವ ವರ್ಣ *realm)
{
	ಸಂರಚನಾ path_data *p, *p_cur, *p_ತುದಿ;

	ಪ್ರತಿಜ್ಞಾ(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	ಪ್ರತಿ (p_cur = p; p_cur != ರಿಕ್ತ; p_ತುದಿ = p_cur, p_cur = p_cur->parent) {
		ಯದಿ (p_cur->prot_space) {
			ಪ್ರತ್ಯಾಗಮ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ಯದಿ (realm != ರಿಕ್ತ) {
		ಧ್ರುವ ಸಂರಚನಾ host_part *h = (ಧ್ರುವ ಸಂರಚನಾ host_part *)p_ತುದಿ;
		ಧ್ರುವ ಸಂರಚನಾ prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		ಪ್ರತಿ (space = h->prot_space; space != ರಿಕ್ತ;
		     space = space->next) {
			ಯದಿ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				ಪ್ರತ್ಯಾಗಮ p->prot_space->auth;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_set_cert_permissions(nsurl *url, bool permit)
{
	ಸಂರಚನಾ path_data *p;
	ಸಂರಚನಾ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ;

	ಪ್ರತಿ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಸಂರಚನಾ host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ಸಂರಚನಾ path_data *p;
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಧ್ರುವ ಸಂರಚನಾ host_part *)p;

	ಪ್ರತ್ಯಾಗಮ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(ಸಂರಚನಾ nsurl *url, ಧ್ರುವ ವರ್ಣ *header)
{
	ಸಂರಚನಾ path_data *p;
	ಸಂರಚನಾ host_part *h;
	lwc_string *host;
	time_t now = time(ರಿಕ್ತ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (host != ರಿಕ್ತ) {
		ಯದಿ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ಪ್ರತ್ಯಾಗಮ true;
		} ಅನ್ಯಥಾ ಯದಿ (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		lwc_string_unref(host);
	} ಅನ್ಯಥಾ {
		/* No host part: ignore */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಸಂರಚನಾ host_part *)p;
	ಯದಿ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	ಯದಿ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ಯದಿ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ಅನ್ಯಥಾ ಯದಿ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ಸಂರಚನಾ nsurl *url)
{
	ಸಂರಚನಾ path_data *p;
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	lwc_string *host;
	time_t now = time(ರಿಕ್ತ);

	ಪ್ರತಿಜ್ಞಾ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (host != ರಿಕ್ತ) {
		ಯದಿ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ಪ್ರತ್ಯಾಗಮ false;
		} ಅನ್ಯಥಾ ಯದಿ (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		lwc_string_unref(host);
	} ಅನ್ಯಥಾ {
		/* No host part: not enabled */
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ಪ್ರತಿಜ್ಞಾ(p);

	h = (ಧ್ರುವ ಸಂರಚನಾ host_part *)p;

	/* Consult record for this host */
	ಯದಿ (h->hsts.expires > now) {
		/* Not expired */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Consult parent domains */
	ಪ್ರತಿ (h = h->parent; h && h != &db_ಬೇರು; h = h->parent) {
		ಯದಿ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ
urldb_iterate_partial(ಧ್ರುವ ವರ್ಣ *prefix,
		      bool (*callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data))
{
	ವರ್ಣ host[256];
	ವರ್ಣ buf[260]; /* max domain + "www." */
	ಧ್ರುವ ವರ್ಣ *slash, *scheme_sep;
	ಸಂರಚನಾ ಹುಡುಕು_ಗಂಟು *ವೃಕ್ಷ;
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;

	ಪ್ರತಿಜ್ಞಾ(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	ಯದಿ (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	ವೃಕ್ಷ = urldb_get_ಹುಡುಕು_ವೃಕ್ಷ(prefix);

	ಯದಿ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, ಪರಿಮಾಣ host, "%.*s",
			 (ಪೂರ್ಣಾಙ್ಕ) (slash - prefix), prefix);

		h = urldb_ಹುಡುಕು_find(ವೃಕ್ಷ, host);
		ಯದಿ (!h) {
			ಪೂರ್ಣಾಙ್ಕ len = slash - prefix;

			ಯದಿ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, ಪರಿಮಾಣ buf, "www.%s", host);
				h = urldb_ಹುಡುಕು_find(
					ಹುಡುಕು_trees[ST_DN + 'w' - 'a'],
					buf);
				ಯದಿ (!h)
					ಪ್ರತ್ಯಾಗಮ;
			} ಅನ್ಯಥಾ
				ಪ್ರತ್ಯಾಗಮ;
		}

		ಯದಿ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ len = strlen(prefix);

		/* looking for hosts */
		ಯದಿ (!urldb_iterate_partial_host(ವೃಕ್ಷ, prefix, callback))
			ಪ್ರತ್ಯಾಗಮ;

		ಯದಿ (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, ಪರಿಮಾಣ buf, "www.%s", prefix);
			ಯದಿ(!urldb_iterate_partial_host(
				   ಹುಡುಕು_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ಪ್ರತ್ಯಾಗಮ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ
urldb_iterate_entries(bool (*callback)(nsurl *url, ಧ್ರುವ ಸಂರಚನಾ url_data *data))
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿಜ್ಞಾ(callback);

	ಪ್ರತಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಯದಿ (!urldb_iterate_entries_host(ಹುಡುಕು_trees[i],
						callback,
						ರಿಕ್ತ)) {
			ಭಙ್ಗ;
		}
	}
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_iterate_cookies(bool (*callback)(ಧ್ರುವ ಸಂರಚನಾ cookie_data *data))
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿಜ್ಞಾ(callback);

	ಪ್ರತಿ (i = 0; i < ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		ಯದಿ (!urldb_iterate_entries_host(ಹುಡುಕು_trees[i],
						ರಿಕ್ತ, callback))
			ಭಙ್ಗ;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(ಧ್ರುವ ವರ್ಣ *header, nsurl *url, nsurl *referer)
{
	ಧ್ರುವ ವರ್ಣ *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(url && header);

	/* Get defragmented URL, as 'urlt' */
	ಯದಿ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಯದಿ (scheme == ರಿಕ್ತ) {
		nsurl_unref(urlt);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	ಯದಿ (path == ರಿಕ್ತ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (host == ರಿಕ್ತ) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ಯದಿ (rhost == ರಿಕ್ತ) {
			ಗಚ್ಛ error;
		}

		/* Domain match host names */
		ಯದಿ (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			ಧ್ರುವ ವರ್ಣ *hptr;
			ಧ್ರುವ ವರ್ಣ *rptr;
			ಧ್ರುವ ವರ್ಣ *dot;
			ಧ್ರುವ ವರ್ಣ *host_data = lwc_string_data(host);
			ಧ್ರುವ ವರ್ಣ *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ಯದಿ (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				ಗಚ್ಛ error;
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

			hptr = host_data + lwc_string_length(host) - 1;
			rptr = rhost_data + lwc_string_length(rhost) - 1;

			/* 1 */
			ಯಾವತ್ (hptr >= host_data && rptr >= rhost_data) {
				ಯದಿ (*hptr != *rptr)
					ಭಙ್ಗ;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ಯಾವತ್ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ಯದಿ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ರಿಕ್ತ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				ಗಚ್ಛ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	ಕುರು {
		ಸಂರಚನಾ cookie_internal_data *c;
		ವರ್ಣ *dot;
		size_t len;
#ifdef WITH_NSPSL
		ಧ್ರುವ ವರ್ಣ *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		ಯದಿ (!c) {
			/* failed => stop parsing */
			ಗಚ್ಛ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ಯದಿ (!c->name || !c->ಮೌಲ್ಯ) {
			urldb_free_cookie(c);
			ಗಚ್ಛ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		ಯದಿ (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			ಗಚ್ಛ error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ಯದಿ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ಯದಿ (suffix == ರಿಕ್ತ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			ಗಚ್ಛ error;
		}
#ಅನ್ಯಥಾ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ಯದಿ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			ಗಚ್ಛ error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		ಯದಿ (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			ಪೂರ್ಣಾಙ್ಕ hlen, dlen;
			ವರ್ಣ *domain = c->domain;

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
			ಯದಿ (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				ಗಚ್ಛ error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			ಯದಿ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				ಗಚ್ಛ error;
			}

			ಯದಿ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ಯದಿ (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				ಗಚ್ಛ error;
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
		ಯದಿ (!urldb_ಸೇರಿಸು_cookie(c, scheme, urlt))
			ಗಚ್ಛ error;
	} ಯಾವತ್ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ಪ್ರತ್ಯಾಗಮ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in content/urldb.h */
ವರ್ಣ *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ಧ್ರುವ ಸಂರಚನಾ path_data *p, *q;
	ಧ್ರುವ ಸಂರಚನಾ host_part *h;
	lwc_string *path_lwc;
	ಸಂರಚನಾ cookie_internal_data *c;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0, version = COOKIE_RFC2965;
	ಸಂರಚನಾ cookie_internal_data **matched_cookies;
	ಪೂರ್ಣಾಙ್ಕ matched_cookies_size = 20;
	ಧ್ರುವ ವರ್ಣ *path;
	ವರ್ಣ *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	ಪ್ರತಿಜ್ಞಾ(url != ರಿಕ್ತ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 ಪರಿಮಾಣ(ಸಂರಚನಾ cookie_internal_data *));
	ಯದಿ (!matched_cookies)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

#define GROW_MATCHED_COOKIES						\
	ಕುರು {								\
		ಯದಿ (ಎಣಿಕೆ == matched_cookies_size) {			\
			ಸಂರಚನಾ cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       ಪರಿಮಾಣ(ಸಂರಚನಾ cookie_internal_data *)); \
									\
			ಯದಿ (temp == ರಿಕ್ತ) {				\
				ಮುಕ್ತ(matched_cookies);			\
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ಯಾವತ್(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	ಯದಿ (path_lwc == ರಿಕ್ತ) {
		ಮುಕ್ತ(matched_cookies);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(ರಿಕ್ತ);

	ಯದಿ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		ಪ್ರತಿ (q = p->parent->children; q; q = q->next) {
			ಯದಿ (strcmp(q->segment, p->segment))
				ಅನುವರ್ತ;

			/* Consider all cookies associated with
			 * this exact path */
			ಪ್ರತಿ (c = q->cookies; c; c = c->next) {
				ಯದಿ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ಅನುವರ್ತ;

				ಯದಿ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					ಅನುವರ್ತ;

				ಯದಿ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ಅನುವರ್ತ;

				matched_cookies[ಎಣಿಕೆ++] = c;

				GROW_MATCHED_COOKIES;

				ಯದಿ (c->version < (unsigned ಪೂರ್ಣಾಙ್ಕ)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	ಪ್ರತಿ (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		ಪ್ರತಿ (q = p->children; q; q = q->next) {
			ಯದಿ (*(q->segment) != '\0')
				ಅನುವರ್ತ;

			ಪ್ರತಿ (c = q->cookies; c; c = c->next) {
				ಯದಿ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ಅನುವರ್ತ;

				ಯದಿ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ಅನುವರ್ತ;

				matched_cookies[ಎಣಿಕೆ++] = c;

				GROW_MATCHED_COOKIES;

				ಯದಿ (c->version < (unsigned ಪೂರ್ಣಾಙ್ಕ) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
			}
		}

		ಯದಿ (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ಭಙ್ಗ;
		}

		/* Consider p itself - may be the result of Path=/foo */
		ಪ್ರತಿ (c = p->cookies; c; c = c->next) {
			ಯದಿ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ಅನುವರ್ತ;

			/* Ensure cookie path is a prefix of the resource */
			ಯದಿ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ಅನುವರ್ತ;

			ಯದಿ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ಅನುವರ್ತ;

			matched_cookies[ಎಣಿಕೆ++] = c;

			GROW_MATCHED_COOKIES;

			ಯದಿ (c->version < (unsigned ಪೂರ್ಣಾಙ್ಕ) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	ಪ್ರತಿ (h = (ಧ್ರುವ ಸಂರಚನಾ host_part *)p; h && h != &db_ಬೇರು;
	     h = h->parent) {
		ಪ್ರತಿ (c = h->paths.cookies; c; c = c->next) {
			ಯದಿ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ಅನುವರ್ತ;

			/* Ensure cookie path is a prefix of the resource */
			ಯದಿ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ಅನುವರ್ತ;

			ಯದಿ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				ಅನುವರ್ತ;

			matched_cookies[ಎಣಿಕೆ++] = c;

			GROW_MATCHED_COOKIES;

			ಯದಿ (c->version < (unsigned ಪೂರ್ಣಾಙ್ಕ)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ಸಂರಚನಾ cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	ಯದಿ (ಎಣಿಕೆ == 0) {
		/* No cookies found */
		ಮುಕ್ತ(matched_cookies);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, ಎಣಿಕೆ);
	} ಅನ್ಯಥಾ {
		ret = get_rfc2965_cookie(matched_cookies, ಎಣಿಕೆ, version);
	}
	ಮುಕ್ತ(matched_cookies);

	ಪ್ರತ್ಯಾಗಮ ret;
}


/* exported interface documented in content/urldb.h */
ಶೂನ್ಯ urldb_delete_cookie(ಧ್ರುವ ವರ್ಣ *domain, ಧ್ರುವ ವರ್ಣ *path,
			 ಧ್ರುವ ವರ್ಣ *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_ಬೇರು);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(ಧ್ರುವ ವರ್ಣ *filename)
{
	FILE *fp;
	ವರ್ಣ s[16*1024];

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "r");
	ಯದಿ (!fp)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;

#define FIND_T {				\
		ಪ್ರತಿ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ಯದಿ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ಅನುವರ್ತ;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		ಪ್ರತಿ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ಯದಿ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ಅನುವರ್ತ;		\
		}				\
	}

	ಯಾವತ್ (fgets(s, ಪರಿಮಾಣ s, fp)) {
		ವರ್ಣ *p = s, *end = 0,
			*domain, *path, *name, *ಮೌಲ್ಯ, *scheme, *url,
			*comment;
		ಪೂರ್ಣಾಙ್ಕ version, domain_specified, path_specified,
			secure, http_only, no_destroy, ಮೌಲ್ಯ_quoted;
		time_t expires, last_used;
		ಸಂರಚನಾ cookie_internal_data *c;

		ಯದಿ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ಅನುವರ್ತ;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ಯದಿ (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			ಯದಿ (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ಭಙ್ಗ;
			}

			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			ಅನುವರ್ತ;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		ಯದಿ (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} ಅನ್ಯಥಾ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; ಮೌಲ್ಯ = p; FIND_T;
		ಯದಿ (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	ಮೌಲ್ಯ_quoted = atoi(p); FIND_T;
		} ಅನ್ಯಥಾ {
			ಮೌಲ್ಯ_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		ಪ್ರತಿ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ಪ್ರತಿಜ್ಞಾ(p <= end);

		/* Now create cookie */
		c = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ cookie_internal_data));
		ಯದಿ (!c)
			ಭಙ್ಗ;

		c->name = strdup(name);
		c->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
		c->ಮೌಲ್ಯ_was_quoted = ಮೌಲ್ಯ_quoted;
		c->comment = strdup(comment);
		c->domain_from_set = domain_specified;
		c->domain = strdup(domain);
		c->path_from_set = path_specified;
		c->path = strdup(path);
		c->expires = expires;
		c->last_used = last_used;
		c->secure = secure;
		c->http_only = http_only;
		c->version = version;
		c->no_destroy = no_destroy;

		ಯದಿ (!(c->name && c->ಮೌಲ್ಯ && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			ಭಙ್ಗ;
		}

		ಯದಿ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ರಿಕ್ತ;
			nsurl *url_nsurl = ರಿಕ್ತ;

			ಪ್ರತಿಜ್ಞಾ(scheme[0] != 'u');

			ಯದಿ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ಭಙ್ಗ;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ಯದಿ (!urldb_ಸೇರಿಸು_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ಭಙ್ಗ;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ಅನ್ಯಥಾ {
			ಯದಿ (!urldb_ಸೇರಿಸು_cookie(c, ರಿಕ್ತ, ರಿಕ್ತ)) {
				/* Cookie freed for us */
				ಭಙ್ಗ;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ಧ್ರುವ ವರ್ಣ *filename)
{
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	ಪ್ರತಿಜ್ಞಾ(filename);

	fp = fopen(filename, "w");
	ಯದಿ (!fp)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;

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
		cookie_file_version);
	fprintf(fp, "Version:\t%d\n", cookie_file_version);

	urldb_save_cookie_hosts(fp, &db_ಬೇರು);

	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
ಶೂನ್ಯ urldb_dump(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	urldb_dump_hosts(&db_ಬೇರು);

	ಪ್ರತಿ (i = 0; i != ಅಂಕೆ_ಹುಡುಕು_TREES; i++) {
		urldb_dump_ಹುಡುಕು(ಹುಡುಕು_trees[i], 0);
	}
}
