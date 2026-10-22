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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#ifdef WITH_NSPSL
#include <nspsl.h>
#endif

#include "utils/inet.h"
#include "utils/nsoption.h"
#include "utils/log.h"
#include "utils/corestrings.h"
#include "utils/url.h"
#include "utils/utils.h"
#include "utils/bloom.h"
#include "utils/time.h"
#include "utils/nsurl.h"
#include "utils/ascii.h"
#include "utils/http.h"
#include "netsurf/bitmap.h"
#include "desktop/cookie_manager.h"

#include "content/content.h"
#include "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
sa_nracanaa cookie_internal_data {
	sa_nracanaa cookie_internal_data *prev;	/**< Previous in list */
	sa_nracanaa cookie_internal_data *next;	/**< Next in list */

	var_na *name;		/**< Cookie name */
	var_na *value;		/**< Cookie value */
	bool value_was_quoted;	/**< Value was quoted in Set-Cookie: */
	var_na *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	var_na *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	var_na *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ga_nanaa cookie_version version;	/**< Specification compliance */
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
sa_nracanaa prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned puur_naa_nka port;
	/** Protection realm */
	var_na *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	var_na *auth;
	/** Next sibling */
	sa_nracanaa prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
sa_nracanaa url_internal_data {
	var_na *title;		/**< Resource title */
	unsigned puur_naa_nka visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
sa_nracanaa path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned puur_naa_nka port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	var_na *segment;		/**< Path segment for this node */
	unsigned puur_naa_nka frag_cnt;	/**< Number of entries in path_data::fragment */
	var_na **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	sa_nracanaa url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	const sa_nracanaa prot_space_data *prot_space;
	/** Cookies associated with resource */
	sa_nracanaa cookie_internal_data *cookies;
	/** Last cookie in list */
	sa_nracanaa cookie_internal_data *cookies_end;

	sa_nracanaa path_data *next;	/**< Next sibling */
	sa_nracanaa path_data *prev;	/**< Previous sibling */
	sa_nracanaa path_data *parent; /**< Parent path segment */
	sa_nracanaa path_data *children; /**< Child path segments */
	sa_nracanaa path_data *last; /**< Last child */
};

sa_nracanaa hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

sa_nracanaa host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	sa_nracanaa path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	sa_nracanaa hsts_data hsts;

	/**
	 * Part of host string
	 */
	var_na *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	sa_nracanaa prot_space_data *prot_space;

	sa_nracanaa host_part *next;	/**< Next sibling */
	sa_nracanaa host_part *prev;	/**< Previous sibling */
	sa_nracanaa host_part *parent; /**< Parent host part */
	sa_nracanaa host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
sa_nracanaa search_node {
	const sa_nracanaa host_part *data;	/**< Host tree entry */

	unsigned puur_naa_nka level;		/**< Node level */

	sa_nracanaa search_node *left;	/**< Left subtree */
	sa_nracanaa search_node *right;	/**< Right subtree */
};

/** Root database handle */
static sa_nracanaa host_part db_root;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define NUM_SEARCH_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
static sa_nracanaa search_node empty = { 0, 0, &empty, &empty };
static sa_nracanaa search_node *search_trees[NUM_SEARCH_TREES] = {
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
static puur_naa_nka loaded_cookie_file_version;

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
static sa_nracanaa bloom_filter *url_bloom;
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
static nserror urldb_write_timet(FILE *fp, time_t val)
{
	puur_naa_nka use;
	var_na op[32];

	use = nsc_sntimet(op, 32, &val);
	ᤀᤡ᤺ (use == 0) {
		fprintf(fp, "%i\n", (puur_naa_nka)val);
	} ᤀᤡ᤺ᤆᤡ {
		fprintf(fp, "%.*s\n", use, op);
	}
	ᤀᤡᤴ NSERROR_OK;
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
static void
urldb_write_paths(const sa_nracanaa path_data *parent,
		  const var_na *host,
		  FILE *fp,
		  var_na **path,
		  puur_naa_nka *path_alloc,
		  puur_naa_nka *path_used,
		  time_t expiry)
{
	const sa_nracanaa path_data *p = parent;
	puur_naa_nka i;

	do {
		puur_naa_nka seglen = p->segment != NULL ? strlen(p->segment) : 0;
		puur_naa_nka len = *path_used + seglen + 1;

		ᤀᤡ᤺ (*path_alloc < len) {
			var_na *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			ᤀᤡ᤺ (!temp) {
				ᤀᤡᤴ;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		ᤀᤡ᤺ (p->segment != NULL) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		ᤀᤡ᤺ (p->children != NULL) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} ᤀᤡ᤺ᤆᤡ {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		ᤀᤡ᤺ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			/* leaf node */
			ᤀᤡ᤺ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				ᤀᤡ᤺ (p->port) {
					fprintf(fp,"%d\n", p->port);
				} ᤀᤡ᤺ᤆᤡ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (puur_naa_nka)p->urld.type);

				fprintf(fp, "\n");

				ᤀᤡ᤺ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					for (i = 0; s[i] != '\0'; i++)
						ᤀᤡ᤺ (s[i] < 32)
							s[i] = ' ';
					for (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ᤀᤡ᤺ᤆᤡ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ᤀᤠ᤺ (p != parent) {
				puur_naa_nka seglen = p->segment != NULL
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				ᤀᤡ᤺ (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
static void
urldb_count_urls(const sa_nracanaa path_data *root,
		 time_t expiry,
		 unsigned puur_naa_nka *count)
{
	const sa_nracanaa path_data *p = root;

	do {
		ᤀᤡ᤺ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			/* No more children, increment count if required */
			ᤀᤡ᤺ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			ᤀᤠ᤺ (p != root) {
				ᤀᤡ᤺ (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != root);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
static void urldb_save_search_tree(sa_nracanaa search_node *parent, FILE *fp)
{
	var_na host[256];
	const sa_nracanaa host_part *h;
	unsigned puur_naa_nka path_count = 0;
	var_na *path, *p, *end;
	puur_naa_nka path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	puur_naa_nka hsts_include_subdomains = 0;

	expiry = time(NULL) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ᤀᤡ᤺ (parent == &empty)
		ᤀᤡᤴ;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	ᤀᤡ᤺ (!path)
		ᤀᤡᤴ;

	path[0] = '\0';

	for (h = parent->data, p = host, end = host + parimaa_na host;
	     h && h != &db_root && p < end; h = h->parent) {
		puur_naa_nka written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		ᤀᤡ᤺ (written < 0) {
			free(path);
			ᤀᤡᤴ;
		}
		p += written;
	}

	h = parent->data;
	ᤀᤡ᤺ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	ᤀᤡ᤺ (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	free(path);

	urldb_save_search_tree(parent->right, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
static bool
urldb_iterate_entries_path(const sa_nracanaa path_data *parent,
		bool (*url_callback)(nsurl *url, const sa_nracanaa url_data *data),
		bool (*cookie_callback)(const sa_nracanaa cookie_data *data))
{
	const sa_nracanaa path_data *p = parent;
	const sa_nracanaa cookie_data *c;

	do {
		ᤀᤡ᤺ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			assert(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ᤀᤡ᤺ (url_callback) {
				const sa_nracanaa url_internal_data *u = &p->urld;

				assert(p->url);

				ᤀᤡ᤺ (!url_callback(p->url,
						  (const sa_nracanaa url_data *) u))
					ᤀᤡᤴ false;
			} ᤀᤡ᤺ᤆᤡ {
				c = (const sa_nracanaa cookie_data *)p->cookies;
				for (; c != NULL; c = c->next) {
					ᤀᤡ᤺ (!cookie_callback(c))
						ᤀᤡᤴ false;
				}
			}

			/* Now, find next node to process. */
			ᤀᤠ᤺ (p != parent) {
				ᤀᤡ᤺ (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != parent);

	ᤀᤡᤴ true;
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
static bool urldb__host_is_ip_address(const var_na *host)
{
	sa_nracanaa in_addr ipv4;
	size_t host_len = strlen(host);
	const var_na *sane_host;
	const var_na *slash;
#ifndef NO_IPV6
	sa_nracanaa in6_addr ipv6;
	var_na ipv6_addr[64];
	unsigned puur_naa_nka ipv6_addr_len;
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
	ᤀᤡ᤺ (slash == NULL) {
		sane_host = host;
	} ᤀᤡ᤺ᤆᤡ {
		var_na *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ᤀᤡ᤺ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		goto out_false;

	ᤀᤡ᤺ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		puur_naa_nka num_dots = 0;
		size_t index;

		for (index = 0; index < host_len; index++) {
			ᤀᤡ᤺ (sane_host[index] == '.')
				num_dots++;
		}

		ᤀᤡ᤺ (num_dots == 3)
			goto out_true;
		ᤀᤡ᤺ᤆᤡ
			goto out_false;
	}

#ifndef NO_IPV6
	ᤀᤡ᤺ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		goto out_false;
	}

	ipv6_addr_len = host_len - 2;
	ᤀᤡ᤺ (ipv6_addr_len >= parimaa_na(ipv6_addr)) {
		ipv6_addr_len = parimaa_na(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ᤀᤡ᤺ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		goto out_true;
#endif

out_false:
	ᤀᤡ᤺ (slash != NULL) free((void *)sane_host);
	ᤀᤡᤴ false;

out_true:
	ᤀᤡ᤺ (slash != NULL) free((void *)sane_host);
	ᤀᤡᤴ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
static puur_naa_nka urldb_search_match_prefix(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	puur_naa_nka plen, ret;

	assert(a && a != &db_root && b);

	ᤀᤡ᤺ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ᤀᤡᤴ strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ᤀᤠ᤺ (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ᤀᤡ᤺ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ᤀᤡ᤺ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ᤀᤡᤴ ret;

		/* The strings matched */
		ᤀᤡ᤺ (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			ᤀᤡ᤺ (plen > dot - b) {
				/* len(a) > len(b) */
				ᤀᤡᤴ 1;
			} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (plen < dot - b) {
				/* len(a) < len(b) */
				ᤀᤡᤴ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ᤀᤡ᤺ (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		ᤀᤡᤴ 0;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		ᤀᤡᤴ -1;
	}

	/* Identical */
	ᤀᤡᤴ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
static bool
urldb_iterate_partial_host(sa_nracanaa search_node *root,
		const var_na *prefix,
		bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	puur_naa_nka c;

	assert(root && prefix && callback);

	ᤀᤡ᤺ (root == &empty)
		ᤀᤡᤴ true;

	c = urldb_search_match_prefix(root->data, prefix);

	ᤀᤡ᤺ (c > 0) {
		/* No match => look in left subtree */
		ᤀᤡᤴ urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (c < 0) {
		/* No match => look in right subtree */
		ᤀᤡᤴ urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} ᤀᤡ᤺ᤆᤡ {
		/* Match => iterate over l/r subtrees & process this node */
		ᤀᤡ᤺ (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			ᤀᤡᤴ false;
		}

		ᤀᤡ᤺ (root->data->paths.children) {
			/* and extract all paths attached to this host */
			ᤀᤡ᤺ (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							NULL)) {
				ᤀᤡᤴ false;
			}
		}

		ᤀᤡ᤺ (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			ᤀᤡᤴ false;
		}
	}

	ᤀᤡᤴ true;
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
static bool
urldb_iterate_partial_path(const sa_nracanaa path_data *parent,
		const var_na *prefix,
		bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	const sa_nracanaa path_data *p = parent->children;
	const var_na *slash, *end = prefix + strlen(prefix);

	do {
		slash = strchr(prefix, '/');
		ᤀᤡ᤺ (!slash) {
			slash = end;
		}

		ᤀᤡ᤺ (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			continue;
		}

		ᤀᤡ᤺ (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			ᤀᤡ᤺ (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ᤀᤡ᤺ (!urldb_iterate_entries_path(p,
								callback,
								NULL)) {
					ᤀᤡᤴ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} ᤀᤡ᤺ᤆᤡ {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} ᤀᤡ᤺ᤆᤡ {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ᤀᤠ᤺ (p != NULL);

	ᤀᤡᤴ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
static bool
urldb_iterate_entries_host(sa_nracanaa search_node *parent,
		bool (*url_callback)(nsurl *url, const sa_nracanaa url_data *data),
		bool (*cookie_callback)(const sa_nracanaa cookie_data *data))
{
	ᤀᤡ᤺ (parent == &empty) {
		ᤀᤡᤴ true;
	}

	ᤀᤡ᤺ (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		ᤀᤡᤴ false;
	}

	ᤀᤡ᤺ ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ᤀᤡ᤺ (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			ᤀᤡᤴ false;
		}
	}

	ᤀᤡ᤺ (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		ᤀᤡᤴ false;
	}

	ᤀᤡᤴ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
static sa_nracanaa host_part *
urldb_add_host_node(const var_na *part, sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *d;

	assert(part && parent);

	d = calloc(1, parimaa_na(sa_nracanaa host_part));
	ᤀᤡ᤺ (!d) {
		ᤀᤡᤴ NULL;
	}

	d->part = strdup(part);
	ᤀᤡ᤺ (!d->part) {
		free(d);
		ᤀᤡᤴ NULL;
	}

	d->next = parent->children;
	ᤀᤡ᤺ (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	ᤀᤡᤴ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
static puur_naa_nka urldb_add_path_fragment_cmp(const void *a, const void *b)
{
	ᤀᤡᤴ strcasecmp(*((const var_na **) a), *((const var_na **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
static sa_nracanaa path_data *
urldb_add_path_fragment(sa_nracanaa path_data *segment, lwc_string *fragment)
{
	var_na **temp;

	assert(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ᤀᤡ᤺ (!fragment)
		ᤀᤡᤴ segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * parimaa_na(var_na *));
	ᤀᤡ᤺ (!temp)
		ᤀᤡᤴ NULL;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	ᤀᤡ᤺ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ᤀᤡᤴ NULL;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      parimaa_na (var_na *),
	      urldb_add_path_fragment_cmp);

	ᤀᤡᤴ segment;
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
static sa_nracanaa path_data *
urldb_add_path_node(lwc_string *scheme,
		    unsigned puur_naa_nka port,
		    const var_na *segment,
		    lwc_string *fragment,
		    sa_nracanaa path_data *parent)
{
	sa_nracanaa path_data *d, *e;

	assert(scheme && segment && parent);

	d = calloc(1, parimaa_na(sa_nracanaa path_data));
	ᤀᤡ᤺ (!d)
		ᤀᤡᤴ NULL;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	ᤀᤡ᤺ (!d->segment) {
		lwc_string_unref(d->scheme);
		free(d);
		ᤀᤡᤴ NULL;
	}

	ᤀᤡ᤺ (fragment) {
		ᤀᤡ᤺ (!urldb_add_path_fragment(d, fragment)) {
			free(d->segment);
			lwc_string_unref(d->scheme);
			free(d);
			ᤀᤡᤴ NULL;
		}
	}

	for (e = parent->children; e; e = e->next) {
		ᤀᤡ᤺ (strcmp(e->segment, d->segment) > 0)
			bha_nga;
	}

	ᤀᤡ᤺ (e) {
		d->prev = e->prev;
		d->next = e;
		ᤀᤡ᤺ (e->prev)
			e->prev->next = d;
		ᤀᤡ᤺ᤆᤡ
			parent->children = d;
		e->prev = d;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (!parent->children) {
		d->prev = d->next = NULL;
		parent->children = parent->last = d;
	} ᤀᤡ᤺ᤆᤡ {
		d->next = NULL;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	ᤀᤡᤴ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static sa_nracanaa search_node **urldb_get_search_tree_direct(const var_na *host)
{
	assert(host);

	ᤀᤡ᤺ (urldb__host_is_ip_address(host)) {
		ᤀᤡᤴ &search_trees[ST_IP];
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (ascii_is_alpha(*host)) {
		ᤀᤡᤴ &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	ᤀᤡᤴ &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static sa_nracanaa search_node *urldb_get_search_tree(const var_na *host)
{
	ᤀᤡᤴ *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
static puur_naa_nka urldb_search_match_string(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	puur_naa_nka plen, ret;

	assert(a && a != &db_root && b);

	ᤀᤡ᤺ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ᤀᤡᤴ strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ᤀᤠ᤺ (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ᤀᤡ᤺ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ᤀᤡ᤺ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ᤀᤡᤴ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		ᤀᤡ᤺ (plen > dot - b) {
			/* len(a) > len(b) */
			ᤀᤡᤴ 1;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (plen < dot - b) {
			/* len(a) < len(b) */
			ᤀᤡᤴ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ᤀᤡ᤺ (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		ᤀᤡᤴ 1;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		ᤀᤡᤴ -1;
	}

	/* Identical */
	ᤀᤡᤴ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
static const sa_nracanaa host_part *
urldb_search_find(sa_nracanaa search_node *root, const var_na *host)
{
	puur_naa_nka c;

	assert(root && host);

	ᤀᤡ᤺ (root == &empty) {
		ᤀᤡᤴ NULL;
	}

	c = urldb_search_match_string(root->data, host);

	ᤀᤡ᤺ (c > 0) {
		ᤀᤡᤴ urldb_search_find(root->left, host);
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (c < 0) {
		ᤀᤡᤴ urldb_search_find(root->right, host);
	}

	ᤀᤡᤴ root->data;
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
static sa_nracanaa path_data *
urldb_match_path(const sa_nracanaa path_data *parent,
		 const var_na *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	const sa_nracanaa path_data *p;
	const var_na *slash;
	bool match;

	assert(parent != NULL);
	assert(parent->segment == NULL);

	ᤀᤡ᤺ (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	assert(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ᤀᤠ᤺ (p != NULL) {
		slash = strchr(path + 1, '/');
		ᤀᤡ᤺ (!slash) {
			slash = path + strlen(path);
		}

		ᤀᤡ᤺ (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			ᤀᤡ᤺ (*slash == '\0') {
				/* Complete match */
				ᤀᤡᤴ (sa_nracanaa path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} ᤀᤡ᤺ᤆᤡ {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	ᤀᤡᤴ NULL;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
static sa_nracanaa path_data *urldb_find_url(nsurl *url)
{
	const sa_nracanaa host_part *h;
	sa_nracanaa path_data *p;
	sa_nracanaa search_node *tree;
	var_na *plq;
	const var_na *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned puur_naa_nka port_int;
	bool match;

	assert(url);

	ᤀᤡ᤺ (url_bloom != NULL) {
		ᤀᤡ᤺ (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			ᤀᤡᤴ NULL;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ᤀᤡ᤺ (scheme == NULL)
		ᤀᤡᤴ NULL;

	ᤀᤡ᤺ (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		ᤀᤡᤴ NULL;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ᤀᤡ᤺ (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ᤀᤡ᤺ᤆᤡ {
		lwc_string_unref(scheme);
		ᤀᤡᤴ NULL;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	ᤀᤡ᤺ (!h) {
		lwc_string_unref(scheme);
		ᤀᤡᤴ NULL;
	}

	/* generate plq (path, leaf, query) */
	ᤀᤡ᤺ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ᤀᤡᤴ NULL;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	ᤀᤡ᤺ (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ᤀᤡ᤺ᤆᤡ {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	free(plq);
	lwc_string_unref(scheme);

	ᤀᤡᤴ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_paths(sa_nracanaa path_data *parent)
{
	const sa_nracanaa path_data *p = parent;
	unsigned puur_naa_nka i;

	do {
		ᤀᤡ᤺ (p->segment != NULL) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			for (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ᤀᤡ᤺ (p->children != NULL) {
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			ᤀᤠ᤺ (p != parent) {
				ᤀᤡ᤺ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_hosts(sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *h;

	ᤀᤡ᤺ (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	for (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
static void urldb_dump_search(sa_nracanaa search_node *parent, puur_naa_nka depth)
{
	const sa_nracanaa host_part *h;
	puur_naa_nka i; /* index into string */
	var_na s[1024];
	puur_naa_nka r;
	puur_naa_nka sl = parimaa_na(s) - 2;

	ᤀᤡ᤺ (parent == &empty)
		ᤀᤡᤴ;

	urldb_dump_search(parent->left, depth + 1);

	for (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	for (h = parent->data; h; h = h->parent) {
		ᤀᤡ᤺ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ᤀᤡ᤺ (r < 0) {
				bha_nga;
			}
			ᤀᤡ᤺ ((i + r) >= sl) {
				bha_nga;
			}
			i += r;
		}

		ᤀᤡ᤺ (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_search(parent->right, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
static puur_naa_nka
urldb_search_match_host(const sa_nracanaa host_part *a, const sa_nracanaa host_part *b)
{
	puur_naa_nka ret;

	assert(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	for (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		ᤀᤡ᤺ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ᤀᤡᤴ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ᤀᤡ᤺ (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		ᤀᤡᤴ 1;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		ᤀᤡᤴ -1;
	}

	/* identical */
	ᤀᤡᤴ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
static sa_nracanaa search_node *urldb_search_skew(sa_nracanaa search_node *root)
{
	assert(root);

	ᤀᤡ᤺ (root->left->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	ᤀᤡᤴ root;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
static sa_nracanaa search_node *urldb_search_split(sa_nracanaa search_node *root)
{
	assert(root);

	ᤀᤡ᤺ (root->right->right->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	ᤀᤡᤴ root;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
static sa_nracanaa search_node *
urldb_search_insert_internal(sa_nracanaa search_node *root, sa_nracanaa search_node *n)
{
	assert(root && n);

	ᤀᤡ᤺ (root == &empty) {
		root = n;
	} ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka c = urldb_search_match_host(root->data, n->data);

		ᤀᤡ᤺ (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} ᤀᤡ᤺ᤆᤡ {
			/* exact match */
			free(n);
			ᤀᤡᤴ root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	ᤀᤡᤴ root;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
static sa_nracanaa search_node *
urldb_search_insert(sa_nracanaa search_node *root, const sa_nracanaa host_part *data)
{
	sa_nracanaa search_node *n;

	assert(root && data);

	n = malloc(parimaa_na(sa_nracanaa search_node));
	ᤀᤡ᤺ (!n)
		ᤀᤡᤴ NULL;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	ᤀᤡᤴ root;
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
static bool
urldb_parse_avpair(sa_nracanaa cookie_internal_data *c,
		   var_na *n,
		   var_na *v,
		   bool was_quoted)
{
	puur_naa_nka vlen;

	assert(c && n && v);

	/* Strip whitespace from start of name */
	for (; *n; n++) {
		ᤀᤡ᤺ (*n != ' ' && *n != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of name */
	for (vlen = strlen(n); vlen; vlen--) {
		ᤀᤡ᤺ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ᤀᤡ᤺ᤆᤡ
			bha_nga;
	}

	/* Strip whitespace from start of value */
	for (; *v; v++) {
		ᤀᤡ᤺ (*v != ' ' && *v != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of value */
	for (vlen = strlen(v); vlen; vlen--) {
		ᤀᤡ᤺ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ᤀᤡ᤺ᤆᤡ
			bha_nga;
	}

	ᤀᤡ᤺ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ᤀᤡ᤺ (!c->comment)
			ᤀᤡᤴ false;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ᤀᤡ᤺ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			ᤀᤡ᤺ (!c->domain)
				ᤀᤡᤴ false;
		}
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcasecmp(n, "Max-Age") == 0) {
		puur_naa_nka temp = atoi(v);
		ᤀᤡ᤺ (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ᤀᤡ᤺ᤆᤡ
			c->expires = time(NULL) + temp;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		ᤀᤡ᤺ (!c->path)
			ᤀᤡᤴ false;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcasecmp(n, "Expires") == 0) {
		var_na *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		for (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		ᤀᤡ᤺ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		ᤀᤡ᤺ (!c->name || !c->value) {
			ᤀᤡᤴ false;
		}
	}

	ᤀᤡᤴ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
static void urldb_free_cookie(sa_nracanaa cookie_internal_data *c)
{
	assert(c);

	free(c->comment);
	free(c->domain);
	free(c->path);
	free(c->name);
	free(c->value);
	free(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
static sa_nracanaa cookie_internal_data *
urldb_parse_cookie(nsurl *url, const var_na **cookie)
{
	sa_nracanaa cookie_internal_data *c;
	const var_na *cur;
	var_na name[1024], value[4096];
	var_na *n = name, *v = value;
	bool in_value = false;
	bool had_value_data = false;
	bool value_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	assert(url && cookie && *cookie);

	c = calloc(1, parimaa_na(sa_nracanaa cookie_internal_data));
	ᤀᤡ᤺ (c == NULL)
		ᤀᤡᤴ NULL;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	for (cur = *cookie; *cur; cur++) {
		ᤀᤡ᤺ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ᤀᤡ᤺ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			bha_nga;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*cur == '\r') {
			/* Spurious linefeed */
			continue;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*cur == '\n') {
			/* Spurious newline */
			continue;
		}

		ᤀᤡ᤺ (in_value && !had_value_data) {
			ᤀᤡ᤺ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				continue;
			} ᤀᤡ᤺ᤆᤡ {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ᤀᤡ᤺ (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		ᤀᤡ᤺ (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ᤀᤡ᤺ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				continue;
			}
		}

		ᤀᤡ᤺ (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			continue;
		}

		ᤀᤡ᤺ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ᤀᤡ᤺ (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ᤀᤡᤴ NULL;
			}

			/* And reset to start */
			n = name;
			v = value;
			in_value = false;
			had_value_data = false;
			value_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ᤀᤡ᤺ (*cur != ';') {
				cur--;
			}

			continue;
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
		ᤀᤡ᤺ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			const var_na *p;
			const var_na *semi = strchr(cur + 1, ';');
			ᤀᤡ᤺ (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			for (p = cur + 1; p < semi; p++)
				ᤀᤡ᤺ (*p == '=')
					bha_nga;

			ᤀᤡ᤺ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ᤀᤡ᤺ᤆᤡ {
				/* found one => comma marks end of cookie */
				cur++;
				bha_nga;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ᤀᤡ᤺ (!in_value) {
			ᤀᤡ᤺ (n < name + (parimaa_na(name) - 1))
				*n++ = *cur;
		} ᤀᤡ᤺ᤆᤡ {
			ᤀᤡ᤺ (v < value + (parimaa_na(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ᤀᤡ᤺ (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ᤀᤡᤴ NULL;
	}

	/* Now fix-up default values */
	ᤀᤡ᤺ (c->domain == NULL) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ᤀᤡ᤺ (host == NULL) {
			urldb_free_cookie(c);
			ᤀᤡᤴ NULL;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	ᤀᤡ᤺ (c->path == NULL) {
		const var_na *path_data;
		var_na *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		ᤀᤡ᤺ (path_lwc == NULL) {
			urldb_free_cookie(c);
			ᤀᤡᤴ NULL;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		ᤀᤡ᤺ (slash != NULL) {
			/* Special case: retain first slash in path */
			ᤀᤡ᤺ (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			ᤀᤡ᤺ (slash == NULL) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				ᤀᤡᤴ NULL;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} ᤀᤡ᤺ᤆᤡ {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			ᤀᤡ᤺ (path == NULL) {
				urldb_free_cookie(c);
				ᤀᤡᤴ NULL;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	ᤀᤡᤴ c;
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
static sa_nracanaa path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned puur_naa_nka port,
	       const sa_nracanaa host_part *host,
	       var_na *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	sa_nracanaa path_data *d, *e;
	var_na *buf = path_query;
	var_na *segment, *slash;
	bool match;

	assert(scheme && host && url);

	d = (sa_nracanaa path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ᤀᤡ᤺ (*segment == '/')
		segment++;

	/* Process path segments */
	do {
		slash = strchr(segment, '/');
		ᤀᤡ᤺ (!slash) {
			/* last segment */
			/* look for existing entry */
			for (e = d->children; e; e = e->next)
				ᤀᤡ᤺ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					bha_nga;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_node(scheme, port,
						    segment, fragment, d);
			bha_nga;
		}

		*slash = '\0';

		/* look for existing entry */
		for (e = d->children; e; e = e->next)
			ᤀᤡ᤺ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				bha_nga;

		d = e ? e : urldb_add_path_node(scheme, port, segment, NULL, d);
		ᤀᤡ᤺ (!d)
			bha_nga;

		segment = slash + 1;
	} ᤀᤠ᤺ (1);

	free(path_query);

	ᤀᤡ᤺ (d && !d->url) {
		/* Insert defragmented URL */
		ᤀᤡ᤺ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ᤀᤡᤴ NULL;
	}

	ᤀᤡᤴ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
static sa_nracanaa host_part *urldb_add_host(const var_na *host)
{
	sa_nracanaa host_part *d = (sa_nracanaa host_part *) &db_root, *e;
	sa_nracanaa search_node *s;
	var_na buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	var_na *part;

	assert(host);

	ᤀᤡ᤺ (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		for (e = d->children; e; e = e->next)
			ᤀᤡ᤺ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ᤀᤡᤴ e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		ᤀᤡ᤺ (!s) {
			/* failed */
			d = NULL;
		} ᤀᤡ᤺ᤆᤡ {
			search_trees[ST_IP] = s;
		}

		ᤀᤡᤴ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, parimaa_na buf);
	buf[parimaa_na buf - 1] = '\0';

	/* Process FQDN segments backwards */
	do {
		part = strrchr(buf, '.');
		ᤀᤡ᤺ (!part) {
			/* last segment */
			/* Check for existing entry */
			for (e = d->children; e; e = e->next)
				ᤀᤡ᤺ (strcasecmp(buf, e->part) == 0)
					bha_nga;

			ᤀᤡ᤺ (e) {
				d = e;
			} ᤀᤡ᤺ᤆᤡ {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			ᤀᤡ᤺ (d) {
				sa_nracanaa search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				ᤀᤡ᤺ (!s) {
					/* failed */
					d = NULL;
				} ᤀᤡ᤺ᤆᤡ {
					*r = s;
				}
			}
			bha_nga;
		}

		/* Check for existing entry */
		for (e = d->children; e; e = e->next)
			ᤀᤡ᤺ (strcasecmp(part + 1, e->part) == 0)
				bha_nga;

		d = e ? e : urldb_add_host_node(part + 1, d);
		ᤀᤡ᤺ (!d)
			bha_nga;

		*part = '\0';
	} ᤀᤠ᤺ (1);

	ᤀᤡᤴ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
static bool
urldb_insert_cookie(sa_nracanaa cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	sa_nracanaa cookie_internal_data *d;
	const sa_nracanaa host_part *h;
	sa_nracanaa path_data *p;
	time_t now = time(NULL);

	assert(c);

	ᤀᤡ᤺ (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		ᤀᤡ᤺ (!h) {
			h = urldb_add_host(c->domain + 1);
			ᤀᤡ᤺ (!h) {
				urldb_free_cookie(c);
				ᤀᤡᤴ false;
			}
		}

		p = (sa_nracanaa path_data *) &h->paths;
	} ᤀᤡ᤺ᤆᤡ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		assert(url != NULL);
		assert(scheme != NULL);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		ᤀᤡ᤺ (!h) {
			h = urldb_add_host(c->domain);
			ᤀᤡ᤺ (!h) {
				urldb_free_cookie(c);
				ᤀᤡᤴ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), NULL, url);
		ᤀᤡ᤺ (!p) {
			urldb_free_cookie(c);
			ᤀᤡᤴ false;
		}
	}

	/* add cookie */
	for (d = p->cookies; d; d = d->next) {
		ᤀᤡ᤺ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			bha_nga;
	}

	ᤀᤡ᤺ (d) {
		ᤀᤡ᤺ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ᤀᤡ᤺ (d->next)
				d->next->prev = d->prev;
			ᤀᤡ᤺ᤆᤡ
				p->cookies_end = d->prev;
			ᤀᤡ᤺ (d->prev)
				d->prev->next = d->next;
			ᤀᤡ᤺ᤆᤡ
				p->cookies = d->next;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ᤀᤡ᤺ᤆᤡ {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			ᤀᤡ᤺ (c->next)
				c->next->prev = c;
			ᤀᤡ᤺ᤆᤡ
				p->cookies_end = c;
			ᤀᤡ᤺ (c->prev)
				c->prev->next = c;
			ᤀᤡ᤺ᤆᤡ
				p->cookies = c;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	} ᤀᤡ᤺ᤆᤡ {
		c->prev = p->cookies_end;
		c->next = NULL;
		ᤀᤡ᤺ (p->cookies_end)
			p->cookies_end->next = c;
		ᤀᤡ᤺ᤆᤡ
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((sa_nracanaa cookie_data *)c);
	}

	ᤀᤡᤴ true;
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
static bool
concat_netscape_cookie(sa_nracanaa cookie_internal_data *c,
		    puur_naa_nka *used,
		    puur_naa_nka *alloc,
		    var_na **buf)
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
	puur_naa_nka max_len;
	puur_naa_nka outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	ᤀᤡ᤺ (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ᤀᤡ᤺ (!temp) {
			ᤀᤡᤴ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	ᤀᤡ᤺ (outl >= (*alloc - *used)) {
		/* output truncated */
		ᤀᤡᤴ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ᤀᤡ᤺ (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		ᤀᤡ᤺ (outl >= (*alloc - *used)) {
			/* output truncated */
			ᤀᤡᤴ false;
		}
		*used += outl;
	} ᤀᤡ᤺ᤆᤡ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		ᤀᤡ᤺ (outl >= (*alloc - *used)) {
			/* output truncated */
			ᤀᤡᤴ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ᤀᤡᤴ true;
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
static bool
concat_rfc2965_cookie(sa_nracanaa cookie_internal_data *c,
		      puur_naa_nka *used,
		      puur_naa_nka *alloc,
		      var_na **buf)
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
	const var_na * const separators = "()<>@,;:\\\"/[]?={} \t";
	puur_naa_nka max_len;
	puur_naa_nka outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	ᤀᤡ᤺ (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ᤀᤡ᤺ (!temp) {
			ᤀᤡᤴ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	ᤀᤡ᤺ (outl >= (*alloc - *used)) {
		/* output truncated */
		ᤀᤡᤴ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ᤀᤡ᤺ (c->value_was_quoted ||
	    strpbrk(c->value, separators) != NULL) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} ᤀᤡ᤺ᤆᤡ {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	ᤀᤡ᤺ (outl >= (*alloc - *used)) {
		/* output truncated */
		ᤀᤡᤴ false;
	}
	*used += outl;

	ᤀᤡ᤺ (c->path_from_set) {
		/* Path, quoted if necessary */

		ᤀᤡ᤺ (strpbrk(c->path, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} ᤀᤡ᤺ᤆᤡ {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		ᤀᤡ᤺ (outl >= (*alloc - *used)) {
			/* output truncated */
			ᤀᤡᤴ false;
		}
		*used += outl;
	}

	ᤀᤡ᤺ (c->domain_from_set) {
		/* Domain, quoted if necessary */
		ᤀᤡ᤺ (strpbrk(c->domain, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} ᤀᤡ᤺ᤆᤡ {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		ᤀᤡ᤺ (outl >= (*alloc - *used)) {
			/* output truncated */
			ᤀᤡᤴ false;
		}
		*used += outl;
	}

	ᤀᤡᤴ true;
}


/**
 * build output cookie string in netscape format
 */
static var_na *
get_netscape_cookie(sa_nracanaa cookie_internal_data **matched_cookies, puur_naa_nka count)
{
	puur_naa_nka ret_alloc = 4096, ret_used = 1;
	var_na *ret;
	puur_naa_nka midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	ᤀᤡ᤺ (ret == NULL) {
		ᤀᤡᤴ NULL;
	}
	ret[0] = '\0';

	for (midx = 0; midx < count; midx++) {
		ᤀᤡ᤺ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			free(ret);
			ᤀᤡᤴ NULL;
		}
	}

	ᤀᤡ᤺ (ret_used <= 2) {
		free(ret);
		ᤀᤡᤴ NULL;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ᤀᤡ᤺ (temp != NULL) {
		ret = temp;
	}

	ᤀᤡᤴ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
static var_na *
get_rfc2965_cookie(sa_nracanaa cookie_internal_data **matched_cookies,
		   puur_naa_nka count,
		   puur_naa_nka version)
{
	puur_naa_nka ret_alloc = 4096; /* return string allocation */
	puur_naa_nka ret_used = 1; /* return sting buffer used */
	var_na *ret; /* return string buffer */
	puur_naa_nka midx; /* index into matched cookies */
	puur_naa_nka outl;

	ret = malloc(ret_alloc);
	ᤀᤡ᤺ (ret == NULL) {
		ᤀᤡᤴ NULL;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	ᤀᤡ᤺ (outl >= ret_alloc - ret_used) {
		/* overflow */
		free(ret);
		ᤀᤡᤴ NULL;
	}
	ret_used += outl;

	for (midx = 0; midx < count; midx++) {
		ᤀᤡ᤺ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			free(ret);
			ᤀᤡᤴ NULL;
		}
	}

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ᤀᤡ᤺ (temp != NULL) {
		ret = temp;
	}

	ᤀᤡᤴ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
static void
urldb_delete_cookie_paths(const var_na *domain,
			  const var_na *path,
			  const var_na *name,
			  sa_nracanaa path_data *parent)
{
	sa_nracanaa cookie_internal_data *c;
	sa_nracanaa path_data *p = parent;

	assert(parent);

	do {
		for (c = p->cookies; c; c = c->next) {
			ᤀᤡ᤺ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				ᤀᤡ᤺ (c->prev) {
					c->prev->next = c->next;
				} ᤀᤡ᤺ᤆᤡ {
					p->cookies = c->next;
				}

				ᤀᤡ᤺ (c->next) {
					c->next->prev = c->prev;
				} ᤀᤡ᤺ᤆᤡ {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				ᤀᤡᤴ;
			}
		}

		ᤀᤡ᤺ (p->children) {
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			ᤀᤠ᤺ (p != parent) {
				ᤀᤡ᤺ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
static void
urldb_delete_cookie_hosts(const var_na *domain,
			  const var_na *path,
			  const var_na *name,
			  sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *h;
	assert(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	for (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
static void urldb_save_cookie_paths(FILE *fp, sa_nracanaa path_data *parent)
{
	sa_nracanaa path_data *p = parent;
	time_t now = time(NULL);

	assert(fp && parent);

	do {
		ᤀᤡ᤺ (p->cookies != NULL) {
			sa_nracanaa cookie_internal_data *c;

			for (c = p->cookies; c != NULL; c = c->next) {
				ᤀᤡ᤺ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					continue;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(puur_naa_nka)c->expires, (puur_naa_nka)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ᤀᤡ᤺ (p->children != NULL) {
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			ᤀᤠ᤺ (p != parent) {
				ᤀᤡ᤺ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ᤀᤠ᤺ (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
static void urldb_save_cookie_hosts(FILE *fp, sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *h;
	assert(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	for (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
static void urldb_destroy_cookie(sa_nracanaa cookie_internal_data *c)
{
	free(c->name);
	free(c->value);
	free(c->comment);
	free(c->domain);
	free(c->path);

	free(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
static void urldb_destroy_path_node_content(sa_nracanaa path_data *node)
{
	sa_nracanaa cookie_internal_data *a, *b;
	unsigned puur_naa_nka i;

	ᤀᤡ᤺ (node->url != NULL) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	free(node->segment);
	for (i = 0; i < node->frag_cnt; i++)
		free(node->fragment[i]);
	free(node->fragment);

	free(node->urld.title);

	for (a = node->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
static void urldb_destroy_prot_space(sa_nracanaa prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	free(space->realm);
	free(space->auth);

	free(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
static void urldb_destroy_path_tree(sa_nracanaa path_data *root)
{
	sa_nracanaa path_data *p = root;

	do {
		ᤀᤡ᤺ (p->children != NULL) {
			p = p->children;
		} ᤀᤡ᤺ᤆᤡ {
			sa_nracanaa path_data *q = p;

			ᤀᤠ᤺ (p != root) {
				ᤀᤡ᤺ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;

				urldb_destroy_path_node_content(q);
				free(q);

				q = p;
			}

			urldb_destroy_path_node_content(q);
			free(q);
		}
	} ᤀᤠ᤺ (p != root);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
static void urldb_destroy_host_tree(sa_nracanaa host_part *root)
{
	sa_nracanaa host_part *a, *b;
	sa_nracanaa path_data *p, *q;
	sa_nracanaa prot_space_data *s, *t;

	/* Destroy children */
	for (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	for (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	for (s = root->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	free(root->part);
	free(root);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
static void urldb_destroy_search_tree(sa_nracanaa search_node *root)
{
	/* Destroy children */
	ᤀᤡ᤺ (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	ᤀᤡ᤺ (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	free(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
void urldb_destroy(void)
{
	sa_nracanaa host_part *a, *b;
	puur_naa_nka i;

	/* Clean up search trees */
	for (i = 0; i < NUM_SEARCH_TREES; i++) {
		ᤀᤡ᤺ (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	for (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, parimaa_na(db_root));

	/* And the bloom filter */
	ᤀᤡ᤺ (url_bloom != NULL) {
		bloom_destroy(url_bloom);
		url_bloom = NULL;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(const var_na *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	var_na s[MAXIMUM_URL_LENGTH];
	var_na host[256];
	sa_nracanaa host_part *h;
	puur_naa_nka urls;
	puur_naa_nka i;
	puur_naa_nka version;
	puur_naa_nka length;
	FILE *fp;

	assert(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ᤀᤡ᤺ (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	ᤀᤡ᤺ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ᤀᤡᤴ NSERROR_NOT_FOUND;
	}

	ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		ᤀᤡᤴ NSERROR_NEED_DATA;
	}

	version = atoi(s);
	ᤀᤡ᤺ (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ᤀᤡᤴ NSERROR_INVALID;
	}
	ᤀᤡ᤺ (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ᤀᤡᤴ NSERROR_INVALID;
	}

	ᤀᤠ᤺ (fgets(host, parimaa_na host, fp)) {
		time_t hsts_expiry = 0;
		puur_naa_nka hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		ᤀᤡ᤺ (length == 0) {
			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			urls = atoi(s);
			/* Eight fields/url */
			for (i = 0; i < (8 * urls); i++) {
				ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					bha_nga;
			}
			continue;
		}

		ᤀᤡ᤺ (version >= 107) {
			var_na *p = host;
			ᤀᤠ᤺ (*p && *p != ' ') p++;
			ᤀᤠ᤺ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ᤀᤠ᤺ (*p && *p != ' ') p++;
			ᤀᤠ᤺ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		ᤀᤡ᤺ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ᤀᤡᤴ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			bha_nga;
		urls = atoi(s);

		/* no URLs => try next host */
		ᤀᤡ᤺ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			continue;
		}

		/* load the non-corrupt data */
		for (i = 0; i < urls; i++) {
			sa_nracanaa path_data *p = NULL;
			var_na scheme[64], ports[10];
			var_na url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned puur_naa_nka port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			var_na *path_query;
			size_t len;

			ᤀᤡ᤺ (!fgets(scheme, parimaa_na scheme, fp))
				bha_nga;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			ᤀᤡ᤺ (!fgets(ports, parimaa_na ports, fp))
				bha_nga;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			s[length] = '\0';

			ᤀᤡ᤺ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, parimaa_na url, "%s://%s%s%s%s",
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
			ᤀᤡ᤺ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ᤀᤡᤴ NSERROR_NOMEM;
			}

			ᤀᤡ᤺ (url_bloom != NULL) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ᤀᤡ᤺ (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ᤀᤡᤴ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			ᤀᤡ᤺ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ᤀᤡᤴ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ᤀᤡ᤺ (p)
				p->urld.visits = (unsigned puur_naa_nka)atoi(s);

			/* entry last use time */
			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				bha_nga;
			}
			ᤀᤡ᤺ (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ᤀᤡ᤺ (p)
				p->urld.type = (content_type)atoi(s);

			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;


			ᤀᤡ᤺ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			ᤀᤡ᤺ (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				ᤀᤡ᤺ (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	ᤀᤡᤴ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(const var_na *filename)
{
	FILE *fp;
	puur_naa_nka i;

	assert(filename);

	fp = fopen(filename, "w");
	ᤀᤡ᤺ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ᤀᤡᤴ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	for (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p) {
		ᤀᤡᤴ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	sa_nracanaa host_part *h;
	sa_nracanaa path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	const var_na *host_str;
	var_na *path_query = NULL;
	size_t len;
	bool match;
	unsigned puur_naa_nka port_int;

	assert(url);

	ᤀᤡ᤺ (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	ᤀᤡ᤺ (url_bloom != NULL) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ᤀᤡ᤺ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		ᤀᤡᤴ false;
	}
	assert(path_query != NULL);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ᤀᤡ᤺ (scheme == NULL) {
		free(path_query);
		ᤀᤡᤴ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ᤀᤡ᤺ (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ᤀᤡ᤺ᤆᤡ {
		lwc_string_unref(scheme);
		free(path_query);
		ᤀᤡᤴ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	ᤀᤡ᤺ (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ᤀᤡ᤺ᤆᤡ {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	ᤀᤡ᤺ (h != NULL) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} ᤀᤡ᤺ᤆᤡ {
		p = NULL;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ᤀᤡᤴ (p != NULL);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, const var_na *title)
{
	sa_nracanaa path_data *p;
	var_na *temp;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (p == NULL) {
		ᤀᤡᤴ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	ᤀᤡ᤺ (title != NULL) {
		temp = strdup(title);
		ᤀᤡ᤺ (temp == NULL) {
			ᤀᤡᤴ NSERROR_NOMEM;
		}
	} ᤀᤡ᤺ᤆᤡ {
		temp = NULL;
	}

	free(p->urld.title);
	p->urld.title = temp;

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p) {
		ᤀᤡᤴ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p) {
		ᤀᤡᤴ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(NULL);
	p->urld.visits++;

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
void urldb_reset_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
const sa_nracanaa url_data *urldb_get_url_data(nsurl *url)
{
	sa_nracanaa path_data *p;
	sa_nracanaa url_internal_data *u;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ NULL;

	u = &p->urld;

	ᤀᤡᤴ (const sa_nracanaa url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ NULL;

	ᤀᤡᤴ p->url;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_set_auth_details(nsurl *url, const var_na *realm, const var_na *auth)
{
	sa_nracanaa path_data *p, *pi;
	sa_nracanaa host_part *h;
	sa_nracanaa prot_space_data *space, *space_alloc;
	var_na *realm_alloc, *auth_alloc;
	bool match;

	assert(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ;

	/* Search for host_part */
	for (pi = p; pi->parent != NULL; pi = pi->parent)
		;
	h = (sa_nracanaa host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	for (space = h->prot_space; space; space = space->next) {
		ᤀᤡ᤺ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			bha_nga;
	}

	ᤀᤡ᤺ (space != NULL) {
		/* Overrule existing auth. */
		free(space->auth);
		space->auth = strdup(auth);
	} ᤀᤡ᤺ᤆᤡ {
		/* Create a new protection space. */
		space = space_alloc = malloc(parimaa_na(sa_nracanaa prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ᤀᤡ᤺ (!space_alloc || !realm_alloc || !auth_alloc) {
			free(space_alloc);
			free(realm_alloc);
			free(auth_alloc);
			ᤀᤡᤴ;
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
const var_na *urldb_get_auth_details(nsurl *url, const var_na *realm)
{
	sa_nracanaa path_data *p, *p_cur, *p_top;

	assert(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ NULL;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	for (p_cur = p; p_cur != NULL; p_top = p_cur, p_cur = p_cur->parent) {
		ᤀᤡ᤺ (p_cur->prot_space) {
			ᤀᤡᤴ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ᤀᤡ᤺ (realm != NULL) {
		const sa_nracanaa host_part *h = (const sa_nracanaa host_part *)p_top;
		const sa_nracanaa prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		for (space = h->prot_space; space != NULL;
		     space = space->next) {
			ᤀᤡ᤺ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				ᤀᤡᤴ p->prot_space->auth;
			}
		}
	}

	ᤀᤡᤴ NULL;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_set_cert_permissions(nsurl *url, bool permit)
{
	sa_nracanaa path_data *p;
	sa_nracanaa host_part *h;

	assert(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ;

	for (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (sa_nracanaa host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	sa_nracanaa path_data *p;
	const sa_nracanaa host_part *h;

	assert(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ false;

	for (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	ᤀᤡᤴ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(sa_nracanaa nsurl *url, const var_na *header)
{
	sa_nracanaa path_data *p;
	sa_nracanaa host_part *h;
	lwc_string *host;
	time_t now = time(NULL);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	assert(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ᤀᤡ᤺ (host != NULL) {
		ᤀᤡ᤺ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ᤀᤡᤴ true;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ᤀᤡᤴ true;
		}

		lwc_string_unref(host);
	} ᤀᤡ᤺ᤆᤡ {
		/* No host part: ignore */
		ᤀᤡᤴ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ false;

	for (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (sa_nracanaa host_part *)p;
	ᤀᤡ᤺ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ᤀᤡᤴ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	ᤀᤡ᤺ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ᤀᤡᤴ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ᤀᤡ᤺ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ᤀᤡᤴ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(sa_nracanaa nsurl *url)
{
	sa_nracanaa path_data *p;
	const sa_nracanaa host_part *h;
	lwc_string *host;
	time_t now = time(NULL);

	assert(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ᤀᤡ᤺ (host != NULL) {
		ᤀᤡ᤺ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ᤀᤡᤴ false;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ᤀᤡᤴ false;
		}

		lwc_string_unref(host);
	} ᤀᤡ᤺ᤆᤡ {
		/* No host part: not enabled */
		ᤀᤡᤴ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ false;

	for (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	/* Consult record for this host */
	ᤀᤡ᤺ (h->hsts.expires > now) {
		/* Not expired */
		ᤀᤡᤴ true;
	}

	/* Consult parent domains */
	for (h = h->parent; h && h != &db_root; h = h->parent) {
		ᤀᤡ᤺ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ᤀᤡᤴ true;
		}
	}

	ᤀᤡᤴ false;
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_partial(const var_na *prefix,
		      bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	var_na host[256];
	var_na buf[260]; /* max domain + "www." */
	const var_na *slash, *scheme_sep;
	sa_nracanaa search_node *tree;
	const sa_nracanaa host_part *h;

	assert(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	ᤀᤡ᤺ (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	ᤀᤡ᤺ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, parimaa_na host, "%.*s",
			 (puur_naa_nka) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		ᤀᤡ᤺ (!h) {
			puur_naa_nka len = slash - prefix;

			ᤀᤡ᤺ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, parimaa_na buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				ᤀᤡ᤺ (!h)
					ᤀᤡᤴ;
			} ᤀᤡ᤺ᤆᤡ
				ᤀᤡᤴ;
		}

		ᤀᤡ᤺ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka len = strlen(prefix);

		/* looking for hosts */
		ᤀᤡ᤺ (!urldb_iterate_partial_host(tree, prefix, callback))
			ᤀᤡᤴ;

		ᤀᤡ᤺ (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, parimaa_na buf, "www.%s", prefix);
			ᤀᤡ᤺(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ᤀᤡᤴ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_entries(bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	puur_naa_nka i;

	assert(callback);

	for (i = 0; i < NUM_SEARCH_TREES; i++) {
		ᤀᤡ᤺ (!urldb_iterate_entries_host(search_trees[i],
						callback,
						NULL)) {
			bha_nga;
		}
	}
}


/* exported interface documented in content/urldb.h */
void urldb_iterate_cookies(bool (*callback)(const sa_nracanaa cookie_data *data))
{
	puur_naa_nka i;

	assert(callback);

	for (i = 0; i < NUM_SEARCH_TREES; i++) {
		ᤀᤡ᤺ (!urldb_iterate_entries_host(search_trees[i],
						NULL, callback))
			bha_nga;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(const var_na *header, nsurl *url, nsurl *referer)
{
	const var_na *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	assert(url && header);

	/* Get defragmented URL, as 'urlt' */
	ᤀᤡ᤺ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ᤀᤡᤴ NULL;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ᤀᤡ᤺ (scheme == NULL) {
		nsurl_unref(urlt);
		ᤀᤡᤴ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	ᤀᤡ᤺ (path == NULL) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ᤀᤡᤴ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ᤀᤡ᤺ (host == NULL) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ᤀᤡᤴ false;
	}

	ᤀᤡ᤺ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ᤀᤡ᤺ (rhost == NULL) {
			goto error;
		}

		/* Domain match host names */
		ᤀᤡ᤺ (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			const var_na *hptr;
			const var_na *rptr;
			const var_na *dot;
			const var_na *host_data = lwc_string_data(host);
			const var_na *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ᤀᤡ᤺ (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				goto error;
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
			ᤀᤠ᤺ (hptr >= host_data && rptr >= rhost_data) {
				ᤀᤡ᤺ (*hptr != *rptr)
					bha_nga;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ᤀᤠ᤺ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ᤀᤡ᤺ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == NULL ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				goto error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	do {
		sa_nracanaa cookie_internal_data *c;
		var_na *dot;
		size_t len;
#ifdef WITH_NSPSL
		const var_na *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		ᤀᤡ᤺ (!c) {
			/* failed => stop parsing */
			goto error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ᤀᤡ᤺ (!c->name || !c->value) {
			urldb_free_cookie(c);
			goto error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		ᤀᤡ᤺ (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			goto error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ᤀᤡ᤺ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ᤀᤡ᤺ (suffix == NULL) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			goto error;
		}
#ᤀᤡ᤺ᤆᤡ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ᤀᤡ᤺ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			goto error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		ᤀᤡ᤺ (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			puur_naa_nka hlen, dlen;
			var_na *domain = c->domain;

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
			assert(c->domain[0] == '.');

			/* 4.3.2:iii */
			ᤀᤡ᤺ (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				goto error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			ᤀᤡ᤺ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				goto error;
			}

			ᤀᤡ᤺ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ᤀᤡ᤺ (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				goto error;
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
		ᤀᤡ᤺ (!urldb_insert_cookie(c, scheme, urlt))
			goto error;
	} ᤀᤠ᤺ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ᤀᤡᤴ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ᤀᤡᤴ false;
}


/* exported interface documented in content/urldb.h */
var_na *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	const sa_nracanaa path_data *p, *q;
	const sa_nracanaa host_part *h;
	lwc_string *path_lwc;
	sa_nracanaa cookie_internal_data *c;
	puur_naa_nka count = 0, version = COOKIE_RFC2965;
	sa_nracanaa cookie_internal_data **matched_cookies;
	puur_naa_nka matched_cookies_size = 20;
	const var_na *path;
	var_na *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	assert(url != NULL);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ᤀᤡ᤺ (!p)
		ᤀᤡᤴ NULL;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 parimaa_na(sa_nracanaa cookie_internal_data *));
	ᤀᤡ᤺ (!matched_cookies)
		ᤀᤡᤴ NULL;

#define GROW_MATCHED_COOKIES						\
	do {								\
		ᤀᤡ᤺ (count == matched_cookies_size) {			\
			sa_nracanaa cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       parimaa_na(sa_nracanaa cookie_internal_data *)); \
									\
			ᤀᤡ᤺ (temp == NULL) {				\
				free(matched_cookies);			\
				ᤀᤡᤴ NULL;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ᤀᤠ᤺(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	ᤀᤡ᤺ (path_lwc == NULL) {
		free(matched_cookies);
		ᤀᤡᤴ NULL;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(NULL);

	ᤀᤡ᤺ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		for (q = p->parent->children; q; q = q->next) {
			ᤀᤡ᤺ (strcmp(q->segment, p->segment))
				continue;

			/* Consider all cookies associated with
			 * this exact path */
			for (c = q->cookies; c; c = c->next) {
				ᤀᤡ᤺ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ᤀᤡ᤺ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					continue;

				ᤀᤡ᤺ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ᤀᤡ᤺ (c->version < (unsigned puur_naa_nka)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	for (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		for (q = p->children; q; q = q->next) {
			ᤀᤡ᤺ (*(q->segment) != '\0')
				continue;

			for (c = q->cookies; c; c = c->next) {
				ᤀᤡ᤺ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ᤀᤡ᤺ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ᤀᤡ᤺ (c->version < (unsigned puur_naa_nka) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}

		ᤀᤡ᤺ (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			bha_nga;
		}

		/* Consider p itself - may be the result of Path=/foo */
		for (c = p->cookies; c; c = c->next) {
			ᤀᤡ᤺ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ᤀᤡ᤺ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ᤀᤡ᤺ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ᤀᤡ᤺ (c->version < (unsigned puur_naa_nka) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	for (h = (const sa_nracanaa host_part *)p; h && h != &db_root;
	     h = h->parent) {
		for (c = h->paths.cookies; c; c = c->next) {
			ᤀᤡ᤺ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ᤀᤡ᤺ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ᤀᤡ᤺ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ᤀᤡ᤺ (c->version < (unsigned puur_naa_nka)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	ᤀᤡ᤺ (count == 0) {
		/* No cookies found */
		free(matched_cookies);
		ᤀᤡᤴ NULL;
	}

	ᤀᤡ᤺ (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} ᤀᤡ᤺ᤆᤡ {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	free(matched_cookies);

	ᤀᤡᤴ ret;
}


/* exported interface documented in content/urldb.h */
void urldb_delete_cookie(const var_na *domain, const var_na *path,
			 const var_na *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_root);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(const var_na *filename)
{
	FILE *fp;
	var_na s[16*1024];

	assert(filename);

	fp = fopen(filename, "r");
	ᤀᤡ᤺ (!fp)
		ᤀᤡᤴ NSERROR_NOT_FOUND;

#define FIND_T {				\
		for (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ᤀᤡ᤺ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		for (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ᤀᤡ᤺ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
	}

	ᤀᤠ᤺ (fgets(s, parimaa_na s, fp)) {
		var_na *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		puur_naa_nka version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		sa_nracanaa cookie_internal_data *c;

		ᤀᤡ᤺(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			continue;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ᤀᤡ᤺ (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			ᤀᤡ᤺ (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				bha_nga;
			}

			continue;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			continue;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		ᤀᤡ᤺ (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} ᤀᤡ᤺ᤆᤡ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		ᤀᤡ᤺ (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} ᤀᤡ᤺ᤆᤡ {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		for (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		assert(p <= end);

		/* Now create cookie */
		c = malloc(parimaa_na(sa_nracanaa cookie_internal_data));
		ᤀᤡ᤺ (!c)
			bha_nga;

		c->name = strdup(name);
		c->value = strdup(value);
		c->value_was_quoted = value_quoted;
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

		ᤀᤡ᤺ (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			bha_nga;
		}

		ᤀᤡ᤺ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = NULL;
			nsurl *url_nsurl = NULL;

			assert(scheme[0] != 'u');

			ᤀᤡ᤺ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				bha_nga;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ᤀᤡ᤺ (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				bha_nga;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ᤀᤡ᤺ᤆᤡ {
			ᤀᤡ᤺ (!urldb_insert_cookie(c, NULL, NULL)) {
				/* Cookie freed for us */
				bha_nga;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(const var_na *filename)
{
	FILE *fp;
	puur_naa_nka cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	assert(filename);

	fp = fopen(filename, "w");
	ᤀᤡ᤺ (!fp)
		ᤀᤡᤴ NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_root);

	fclose(fp);

	ᤀᤡᤴ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_dump(void)
{
	puur_naa_nka i;

	urldb_dump_hosts(&db_root);

	for (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
