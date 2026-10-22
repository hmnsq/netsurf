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
	unsigned จำนวนเต็ม port;
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
	unsigned จำนวนเต็ม visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
sa_nracanaa path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned จำนวนเต็ม port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	var_na *segment;		/**< Path segment for this node */
	unsigned จำนวนเต็ม frag_cnt;	/**< Number of entries in path_data::fragment */
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

	unsigned จำนวนเต็ม level;		/**< Node level */

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
static จำนวนเต็ม loaded_cookie_file_version;

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
	จำนวนเต็ม use;
	var_na op[32];

	use = nsc_sntimet(op, 32, &val);
	ถ้า (use == 0) {
		fprintf(fp, "%i\n", (จำนวนเต็ม)val);
	} มิฉะนั้น {
		fprintf(fp, "%.*s\n", use, op);
	}
	คืน NSERROR_OK;
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
		  จำนวนเต็ม *path_alloc,
		  จำนวนเต็ม *path_used,
		  time_t expiry)
{
	const sa_nracanaa path_data *p = parent;
	จำนวนเต็ม i;

	do {
		จำนวนเต็ม seglen = p->segment != NULL ? strlen(p->segment) : 0;
		จำนวนเต็ม len = *path_used + seglen + 1;

		ถ้า (*path_alloc < len) {
			var_na *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			ถ้า (!temp) {
				คืน;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		ถ้า (p->segment != NULL) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		ถ้า (p->children != NULL) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} มิฉะนั้น {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		ถ้า (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} มิฉะนั้น {
			/* leaf node */
			ถ้า (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				ถ้า (p->port) {
					fprintf(fp,"%d\n", p->port);
				} มิฉะนั้น {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (จำนวนเต็ม)p->urld.type);

				fprintf(fp, "\n");

				ถ้า (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					สำหรับ (i = 0; s[i] != '\0'; i++)
						ถ้า (s[i] < 32)
							s[i] = ' ';
					สำหรับ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} มิฉะนั้น {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ขณะ (p != parent) {
				จำนวนเต็ม seglen = p->segment != NULL
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				ถ้า (p->next != NULL) {
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
	} ขณะ (p != parent);
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
		 unsigned จำนวนเต็ม *count)
{
	const sa_nracanaa path_data *p = root;

	do {
		ถ้า (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} มิฉะนั้น {
			/* No more children, increment count if required */
			ถ้า (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			ขณะ (p != root) {
				ถ้า (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ขณะ (p != root);
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
	unsigned จำนวนเต็ม path_count = 0;
	var_na *path, *p, *end;
	จำนวนเต็ม path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	จำนวนเต็ม hsts_include_subdomains = 0;

	expiry = time(NULL) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ถ้า (parent == &empty)
		คืน;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	ถ้า (!path)
		คืน;

	path[0] = '\0';

	สำหรับ (h = parent->data, p = host, end = host + parimaa_na host;
	     h && h != &db_root && p < end; h = h->parent) {
		จำนวนเต็ม written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		ถ้า (written < 0) {
			free(path);
			คืน;
		}
		p += written;
	}

	h = parent->data;
	ถ้า (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	ถ้า (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} มิฉะนั้น ถ้า (hsts_expiry) {
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
		ถ้า (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} มิฉะนั้น {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			assert(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ถ้า (url_callback) {
				const sa_nracanaa url_internal_data *u = &p->urld;

				assert(p->url);

				ถ้า (!url_callback(p->url,
						  (const sa_nracanaa url_data *) u))
					คืน false;
			} มิฉะนั้น {
				c = (const sa_nracanaa cookie_data *)p->cookies;
				สำหรับ (; c != NULL; c = c->next) {
					ถ้า (!cookie_callback(c))
						คืน false;
				}
			}

			/* Now, find next node to process. */
			ขณะ (p != parent) {
				ถ้า (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ขณะ (p != parent);

	คืน true;
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
	unsigned จำนวนเต็ม ipv6_addr_len;
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
	ถ้า (slash == NULL) {
		sane_host = host;
	} มิฉะนั้น {
		var_na *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ถ้า (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		goto out_false;

	ถ้า (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		จำนวนเต็ม num_dots = 0;
		size_t index;

		สำหรับ (index = 0; index < host_len; index++) {
			ถ้า (sane_host[index] == '.')
				num_dots++;
		}

		ถ้า (num_dots == 3)
			goto out_true;
		มิฉะนั้น
			goto out_false;
	}

#ifndef NO_IPV6
	ถ้า ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		goto out_false;
	}

	ipv6_addr_len = host_len - 2;
	ถ้า (ipv6_addr_len >= parimaa_na(ipv6_addr)) {
		ipv6_addr_len = parimaa_na(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ถ้า (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		goto out_true;
#endif

out_false:
	ถ้า (slash != NULL) free((void *)sane_host);
	คืน false;

out_true:
	ถ้า (slash != NULL) free((void *)sane_host);
	คืน true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
static จำนวนเต็ม urldb_search_match_prefix(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	จำนวนเต็ม plen, ret;

	assert(a && a != &db_root && b);

	ถ้า (urldb__host_is_ip_address(b)) {
		/* IP address */
		คืน strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ขณะ (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ถ้า (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ถ้า ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			คืน ret;

		/* The strings matched */
		ถ้า (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			ถ้า (plen > dot - b) {
				/* len(a) > len(b) */
				คืน 1;
			} มิฉะนั้น ถ้า (plen < dot - b) {
				/* len(a) < len(b) */
				คืน -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ถ้า (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		คืน 0;
	} มิฉะนั้น ถ้า ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		คืน -1;
	}

	/* Identical */
	คืน 0;
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
	จำนวนเต็ม c;

	assert(root && prefix && callback);

	ถ้า (root == &empty)
		คืน true;

	c = urldb_search_match_prefix(root->data, prefix);

	ถ้า (c > 0) {
		/* No match => look in left subtree */
		คืน urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} มิฉะนั้น ถ้า (c < 0) {
		/* No match => look in right subtree */
		คืน urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} มิฉะนั้น {
		/* Match => iterate over l/r subtrees & process this node */
		ถ้า (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			คืน false;
		}

		ถ้า (root->data->paths.children) {
			/* and extract all paths attached to this host */
			ถ้า (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							NULL)) {
				คืน false;
			}
		}

		ถ้า (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			คืน false;
		}
	}

	คืน true;
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
		ถ้า (!slash) {
			slash = end;
		}

		ถ้า (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			continue;
		}

		ถ้า (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			ถ้า (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ถ้า (!urldb_iterate_entries_path(p,
								callback,
								NULL)) {
					คืน false;
				}

				/* Progress to next sibling */
				p = p->next;
			} มิฉะนั้น {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} มิฉะนั้น {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ขณะ (p != NULL);

	คืน true;
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
	ถ้า (parent == &empty) {
		คืน true;
	}

	ถ้า (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		คืน false;
	}

	ถ้า ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ถ้า (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			คืน false;
		}
	}

	ถ้า (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		คืน false;
	}

	คืน true;
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
	ถ้า (!d) {
		คืน NULL;
	}

	d->part = strdup(part);
	ถ้า (!d->part) {
		free(d);
		คืน NULL;
	}

	d->next = parent->children;
	ถ้า (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	คืน d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
static จำนวนเต็ม urldb_add_path_fragment_cmp(const void *a, const void *b)
{
	คืน strcasecmp(*((const var_na **) a), *((const var_na **) b));
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
	ถ้า (!fragment)
		คืน segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * parimaa_na(var_na *));
	ถ้า (!temp)
		คืน NULL;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	ถ้า (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		คืน NULL;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      parimaa_na (var_na *),
	      urldb_add_path_fragment_cmp);

	คืน segment;
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
		    unsigned จำนวนเต็ม port,
		    const var_na *segment,
		    lwc_string *fragment,
		    sa_nracanaa path_data *parent)
{
	sa_nracanaa path_data *d, *e;

	assert(scheme && segment && parent);

	d = calloc(1, parimaa_na(sa_nracanaa path_data));
	ถ้า (!d)
		คืน NULL;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	ถ้า (!d->segment) {
		lwc_string_unref(d->scheme);
		free(d);
		คืน NULL;
	}

	ถ้า (fragment) {
		ถ้า (!urldb_add_path_fragment(d, fragment)) {
			free(d->segment);
			lwc_string_unref(d->scheme);
			free(d);
			คืน NULL;
		}
	}

	สำหรับ (e = parent->children; e; e = e->next) {
		ถ้า (strcmp(e->segment, d->segment) > 0)
			bha_nga;
	}

	ถ้า (e) {
		d->prev = e->prev;
		d->next = e;
		ถ้า (e->prev)
			e->prev->next = d;
		มิฉะนั้น
			parent->children = d;
		e->prev = d;
	} มิฉะนั้น ถ้า (!parent->children) {
		d->prev = d->next = NULL;
		parent->children = parent->last = d;
	} มิฉะนั้น {
		d->next = NULL;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	คืน d;
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

	ถ้า (urldb__host_is_ip_address(host)) {
		คืน &search_trees[ST_IP];
	} มิฉะนั้น ถ้า (ascii_is_alpha(*host)) {
		คืน &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	คืน &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static sa_nracanaa search_node *urldb_get_search_tree(const var_na *host)
{
	คืน *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
static จำนวนเต็ม urldb_search_match_string(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	จำนวนเต็ม plen, ret;

	assert(a && a != &db_root && b);

	ถ้า (urldb__host_is_ip_address(b)) {
		/* IP address */
		คืน strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ขณะ (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ถ้า (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ถ้า ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			คืน ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		ถ้า (plen > dot - b) {
			/* len(a) > len(b) */
			คืน 1;
		} มิฉะนั้น ถ้า (plen < dot - b) {
			/* len(a) < len(b) */
			คืน -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ถ้า (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		คืน 1;
	} มิฉะนั้น ถ้า ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		คืน -1;
	}

	/* Identical */
	คืน 0;
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
	จำนวนเต็ม c;

	assert(root && host);

	ถ้า (root == &empty) {
		คืน NULL;
	}

	c = urldb_search_match_string(root->data, host);

	ถ้า (c > 0) {
		คืน urldb_search_find(root->left, host);
	} มิฉะนั้น ถ้า (c < 0) {
		คืน urldb_search_find(root->right, host);
	}

	คืน root->data;
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

	ถ้า (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	assert(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ขณะ (p != NULL) {
		slash = strchr(path + 1, '/');
		ถ้า (!slash) {
			slash = path + strlen(path);
		}

		ถ้า (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			ถ้า (*slash == '\0') {
				/* Complete match */
				คืน (sa_nracanaa path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} มิฉะนั้น {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	คืน NULL;
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
	unsigned จำนวนเต็ม port_int;
	bool match;

	assert(url);

	ถ้า (url_bloom != NULL) {
		ถ้า (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			คืน NULL;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ถ้า (scheme == NULL)
		คืน NULL;

	ถ้า (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		คืน NULL;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ถ้า (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} มิฉะนั้น ถ้า (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} มิฉะนั้น {
		lwc_string_unref(scheme);
		คืน NULL;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	ถ้า (!h) {
		lwc_string_unref(scheme);
		คืน NULL;
	}

	/* generate plq (path, leaf, query) */
	ถ้า (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		คืน NULL;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	ถ้า (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} มิฉะนั้น {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	free(plq);
	lwc_string_unref(scheme);

	คืน p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_paths(sa_nracanaa path_data *parent)
{
	const sa_nracanaa path_data *p = parent;
	unsigned จำนวนเต็ม i;

	do {
		ถ้า (p->segment != NULL) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			สำหรับ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ถ้า (p->children != NULL) {
			p = p->children;
		} มิฉะนั้น {
			ขณะ (p != parent) {
				ถ้า (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ขณะ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_hosts(sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *h;

	ถ้า (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	สำหรับ (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
static void urldb_dump_search(sa_nracanaa search_node *parent, จำนวนเต็ม depth)
{
	const sa_nracanaa host_part *h;
	จำนวนเต็ม i; /* index into string */
	var_na s[1024];
	จำนวนเต็ม r;
	จำนวนเต็ม sl = parimaa_na(s) - 2;

	ถ้า (parent == &empty)
		คืน;

	urldb_dump_search(parent->left, depth + 1);

	สำหรับ (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	สำหรับ (h = parent->data; h; h = h->parent) {
		ถ้า (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ถ้า (r < 0) {
				bha_nga;
			}
			ถ้า ((i + r) >= sl) {
				bha_nga;
			}
			i += r;
		}

		ถ้า (h->parent && h->parent->parent) {
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
static จำนวนเต็ม
urldb_search_match_host(const sa_nracanaa host_part *a, const sa_nracanaa host_part *b)
{
	จำนวนเต็ม ret;

	assert(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	สำหรับ (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		ถ้า ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			คืน ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ถ้า (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		คืน 1;
	} มิฉะนั้น ถ้า ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		คืน -1;
	}

	/* identical */
	คืน 0;
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

	ถ้า (root->left->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	คืน root;
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

	ถ้า (root->right->right->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	คืน root;
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

	ถ้า (root == &empty) {
		root = n;
	} มิฉะนั้น {
		จำนวนเต็ม c = urldb_search_match_host(root->data, n->data);

		ถ้า (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} มิฉะนั้น ถ้า (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} มิฉะนั้น {
			/* exact match */
			free(n);
			คืน root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	คืน root;
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
	ถ้า (!n)
		คืน NULL;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	คืน root;
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
	จำนวนเต็ม vlen;

	assert(c && n && v);

	/* Strip whitespace from start of name */
	สำหรับ (; *n; n++) {
		ถ้า (*n != ' ' && *n != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of name */
	สำหรับ (vlen = strlen(n); vlen; vlen--) {
		ถ้า (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		มิฉะนั้น
			bha_nga;
	}

	/* Strip whitespace from start of value */
	สำหรับ (; *v; v++) {
		ถ้า (*v != ' ' && *v != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of value */
	สำหรับ (vlen = strlen(v); vlen; vlen--) {
		ถ้า (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		มิฉะนั้น
			bha_nga;
	}

	ถ้า (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ถ้า (!c->comment)
			คืน false;
	} มิฉะนั้น ถ้า (!c->domain && strcasecmp(n, "Domain") == 0) {
		ถ้า (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			ถ้า (!c->domain)
				คืน false;
		}
	} มิฉะนั้น ถ้า (strcasecmp(n, "Max-Age") == 0) {
		จำนวนเต็ม temp = atoi(v);
		ถ้า (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		มิฉะนั้น
			c->expires = time(NULL) + temp;
	} มิฉะนั้น ถ้า (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		ถ้า (!c->path)
			คืน false;
	} มิฉะนั้น ถ้า (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} มิฉะนั้น ถ้า (strcasecmp(n, "Expires") == 0) {
		var_na *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		สำหรับ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		ถ้า (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} มิฉะนั้น ถ้า (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} มิฉะนั้น ถ้า (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} มิฉะนั้น ถ้า (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		ถ้า (!c->name || !c->value) {
			คืน false;
		}
	}

	คืน true;
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
	ถ้า (c == NULL)
		คืน NULL;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	สำหรับ (cur = *cookie; *cur; cur++) {
		ถ้า (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ถ้า (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			bha_nga;
		} มิฉะนั้น ถ้า (*cur == '\r') {
			/* Spurious linefeed */
			continue;
		} มิฉะนั้น ถ้า (*cur == '\n') {
			/* Spurious newline */
			continue;
		}

		ถ้า (in_value && !had_value_data) {
			ถ้า (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				continue;
			} มิฉะนั้น {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ถ้า (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		ถ้า (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ถ้า (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				continue;
			}
		}

		ถ้า (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			continue;
		}

		ถ้า (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ถ้า (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				คืน NULL;
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
			ถ้า (*cur != ';') {
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
		ถ้า (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			const var_na *p;
			const var_na *semi = strchr(cur + 1, ';');
			ถ้า (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			สำหรับ (p = cur + 1; p < semi; p++)
				ถ้า (*p == '=')
					bha_nga;

			ถ้า (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} มิฉะนั้น {
				/* found one => comma marks end of cookie */
				cur++;
				bha_nga;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ถ้า (!in_value) {
			ถ้า (n < name + (parimaa_na(name) - 1))
				*n++ = *cur;
		} มิฉะนั้น {
			ถ้า (v < value + (parimaa_na(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ถ้า (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		คืน NULL;
	}

	/* Now fix-up default values */
	ถ้า (c->domain == NULL) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ถ้า (host == NULL) {
			urldb_free_cookie(c);
			คืน NULL;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	ถ้า (c->path == NULL) {
		const var_na *path_data;
		var_na *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		ถ้า (path_lwc == NULL) {
			urldb_free_cookie(c);
			คืน NULL;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		ถ้า (slash != NULL) {
			/* Special case: retain first slash in path */
			ถ้า (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			ถ้า (slash == NULL) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				คืน NULL;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} มิฉะนั้น {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			ถ้า (path == NULL) {
				urldb_free_cookie(c);
				คืน NULL;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	คืน c;
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
	       unsigned จำนวนเต็ม port,
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
	ถ้า (*segment == '/')
		segment++;

	/* Process path segments */
	do {
		slash = strchr(segment, '/');
		ถ้า (!slash) {
			/* last segment */
			/* look for existing entry */
			สำหรับ (e = d->children; e; e = e->next)
				ถ้า (strcmp(segment, e->segment) == 0 &&
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
		สำหรับ (e = d->children; e; e = e->next)
			ถ้า (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				bha_nga;

		d = e ? e : urldb_add_path_node(scheme, port, segment, NULL, d);
		ถ้า (!d)
			bha_nga;

		segment = slash + 1;
	} ขณะ (1);

	free(path_query);

	ถ้า (d && !d->url) {
		/* Insert defragmented URL */
		ถ้า (nsurl_defragment(url, &d->url) != NSERROR_OK)
			คืน NULL;
	}

	คืน d;
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

	ถ้า (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		สำหรับ (e = d->children; e; e = e->next)
			ถ้า (strcasecmp(host, e->part) == 0)
				/* found => return it */
				คืน e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		ถ้า (!s) {
			/* failed */
			d = NULL;
		} มิฉะนั้น {
			search_trees[ST_IP] = s;
		}

		คืน d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, parimaa_na buf);
	buf[parimaa_na buf - 1] = '\0';

	/* Process FQDN segments backwards */
	do {
		part = strrchr(buf, '.');
		ถ้า (!part) {
			/* last segment */
			/* Check for existing entry */
			สำหรับ (e = d->children; e; e = e->next)
				ถ้า (strcasecmp(buf, e->part) == 0)
					bha_nga;

			ถ้า (e) {
				d = e;
			} มิฉะนั้น {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			ถ้า (d) {
				sa_nracanaa search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				ถ้า (!s) {
					/* failed */
					d = NULL;
				} มิฉะนั้น {
					*r = s;
				}
			}
			bha_nga;
		}

		/* Check for existing entry */
		สำหรับ (e = d->children; e; e = e->next)
			ถ้า (strcasecmp(part + 1, e->part) == 0)
				bha_nga;

		d = e ? e : urldb_add_host_node(part + 1, d);
		ถ้า (!d)
			bha_nga;

		*part = '\0';
	} ขณะ (1);

	คืน d;
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

	ถ้า (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		ถ้า (!h) {
			h = urldb_add_host(c->domain + 1);
			ถ้า (!h) {
				urldb_free_cookie(c);
				คืน false;
			}
		}

		p = (sa_nracanaa path_data *) &h->paths;
	} มิฉะนั้น {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		assert(url != NULL);
		assert(scheme != NULL);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		ถ้า (!h) {
			h = urldb_add_host(c->domain);
			ถ้า (!h) {
				urldb_free_cookie(c);
				คืน false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), NULL, url);
		ถ้า (!p) {
			urldb_free_cookie(c);
			คืน false;
		}
	}

	/* add cookie */
	สำหรับ (d = p->cookies; d; d = d->next) {
		ถ้า (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			bha_nga;
	}

	ถ้า (d) {
		ถ้า (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ถ้า (d->next)
				d->next->prev = d->prev;
			มิฉะนั้น
				p->cookies_end = d->prev;
			ถ้า (d->prev)
				d->prev->next = d->next;
			มิฉะนั้น
				p->cookies = d->next;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} มิฉะนั้น {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			ถ้า (c->next)
				c->next->prev = c;
			มิฉะนั้น
				p->cookies_end = c;
			ถ้า (c->prev)
				c->prev->next = c;
			มิฉะนั้น
				p->cookies = c;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	} มิฉะนั้น {
		c->prev = p->cookies_end;
		c->next = NULL;
		ถ้า (p->cookies_end)
			p->cookies_end->next = c;
		มิฉะนั้น
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((sa_nracanaa cookie_data *)c);
	}

	คืน true;
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
		    จำนวนเต็ม *used,
		    จำนวนเต็ม *alloc,
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
	จำนวนเต็ม max_len;
	จำนวนเต็ม outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	ถ้า (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ถ้า (!temp) {
			คืน false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	ถ้า (outl >= (*alloc - *used)) {
		/* output truncated */
		คืน false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ถ้า (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		ถ้า (outl >= (*alloc - *used)) {
			/* output truncated */
			คืน false;
		}
		*used += outl;
	} มิฉะนั้น {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		ถ้า (outl >= (*alloc - *used)) {
			/* output truncated */
			คืน false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	คืน true;
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
		      จำนวนเต็ม *used,
		      จำนวนเต็ม *alloc,
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
	จำนวนเต็ม max_len;
	จำนวนเต็ม outl; /* output length */

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

	ถ้า (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ถ้า (!temp) {
			คืน false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	ถ้า (outl >= (*alloc - *used)) {
		/* output truncated */
		คืน false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ถ้า (c->value_was_quoted ||
	    strpbrk(c->value, separators) != NULL) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} มิฉะนั้น {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	ถ้า (outl >= (*alloc - *used)) {
		/* output truncated */
		คืน false;
	}
	*used += outl;

	ถ้า (c->path_from_set) {
		/* Path, quoted if necessary */

		ถ้า (strpbrk(c->path, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} มิฉะนั้น {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		ถ้า (outl >= (*alloc - *used)) {
			/* output truncated */
			คืน false;
		}
		*used += outl;
	}

	ถ้า (c->domain_from_set) {
		/* Domain, quoted if necessary */
		ถ้า (strpbrk(c->domain, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} มิฉะนั้น {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		ถ้า (outl >= (*alloc - *used)) {
			/* output truncated */
			คืน false;
		}
		*used += outl;
	}

	คืน true;
}


/**
 * build output cookie string in netscape format
 */
static var_na *
get_netscape_cookie(sa_nracanaa cookie_internal_data **matched_cookies, จำนวนเต็ม count)
{
	จำนวนเต็ม ret_alloc = 4096, ret_used = 1;
	var_na *ret;
	จำนวนเต็ม midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	ถ้า (ret == NULL) {
		คืน NULL;
	}
	ret[0] = '\0';

	สำหรับ (midx = 0; midx < count; midx++) {
		ถ้า (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			free(ret);
			คืน NULL;
		}
	}

	ถ้า (ret_used <= 2) {
		free(ret);
		คืน NULL;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ถ้า (temp != NULL) {
		ret = temp;
	}

	คืน ret;
}


/**
 * build output cookie string in RFC2965 format
 */
static var_na *
get_rfc2965_cookie(sa_nracanaa cookie_internal_data **matched_cookies,
		   จำนวนเต็ม count,
		   จำนวนเต็ม version)
{
	จำนวนเต็ม ret_alloc = 4096; /* return string allocation */
	จำนวนเต็ม ret_used = 1; /* return sting buffer used */
	var_na *ret; /* return string buffer */
	จำนวนเต็ม midx; /* index into matched cookies */
	จำนวนเต็ม outl;

	ret = malloc(ret_alloc);
	ถ้า (ret == NULL) {
		คืน NULL;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	ถ้า (outl >= ret_alloc - ret_used) {
		/* overflow */
		free(ret);
		คืน NULL;
	}
	ret_used += outl;

	สำหรับ (midx = 0; midx < count; midx++) {
		ถ้า (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			free(ret);
			คืน NULL;
		}
	}

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ถ้า (temp != NULL) {
		ret = temp;
	}

	คืน ret;
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
		สำหรับ (c = p->cookies; c; c = c->next) {
			ถ้า (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				ถ้า (c->prev) {
					c->prev->next = c->next;
				} มิฉะนั้น {
					p->cookies = c->next;
				}

				ถ้า (c->next) {
					c->next->prev = c->prev;
				} มิฉะนั้น {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				คืน;
			}
		}

		ถ้า (p->children) {
			p = p->children;
		} มิฉะนั้น {
			ขณะ (p != parent) {
				ถ้า (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ขณะ (p != parent);
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

	สำหรับ (h = parent->children; h; h = h->next) {
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
		ถ้า (p->cookies != NULL) {
			sa_nracanaa cookie_internal_data *c;

			สำหรับ (c = p->cookies; c != NULL; c = c->next) {
				ถ้า (c->expires == -1 || c->expires < now) {
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
					(จำนวนเต็ม)c->expires, (จำนวนเต็ม)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ถ้า (p->children != NULL) {
			p = p->children;
		} มิฉะนั้น {
			ขณะ (p != parent) {
				ถ้า (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ขณะ (p != parent);
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

	สำหรับ (h = parent->children; h; h = h->next)
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
	unsigned จำนวนเต็ม i;

	ถ้า (node->url != NULL) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	free(node->segment);
	สำหรับ (i = 0; i < node->frag_cnt; i++)
		free(node->fragment[i]);
	free(node->fragment);

	free(node->urld.title);

	สำหรับ (a = node->cookies; a; a = b) {
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
		ถ้า (p->children != NULL) {
			p = p->children;
		} มิฉะนั้น {
			sa_nracanaa path_data *q = p;

			ขณะ (p != root) {
				ถ้า (p->next != NULL) {
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
	} ขณะ (p != root);
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
	สำหรับ (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	สำหรับ (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	สำหรับ (s = root->prot_space; s; s = t) {
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
	ถ้า (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	ถ้า (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	free(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
void urldb_destroy(void)
{
	sa_nracanaa host_part *a, *b;
	จำนวนเต็ม i;

	/* Clean up search trees */
	สำหรับ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ถ้า (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	สำหรับ (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, parimaa_na(db_root));

	/* And the bloom filter */
	ถ้า (url_bloom != NULL) {
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
	จำนวนเต็ม urls;
	จำนวนเต็ม i;
	จำนวนเต็ม version;
	จำนวนเต็ม length;
	FILE *fp;

	assert(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ถ้า (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	ถ้า (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		คืน NSERROR_NOT_FOUND;
	}

	ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		คืน NSERROR_NEED_DATA;
	}

	version = atoi(s);
	ถ้า (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		คืน NSERROR_INVALID;
	}
	ถ้า (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		คืน NSERROR_INVALID;
	}

	ขณะ (fgets(host, parimaa_na host, fp)) {
		time_t hsts_expiry = 0;
		จำนวนเต็ม hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		ถ้า (length == 0) {
			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			urls = atoi(s);
			/* Eight fields/url */
			สำหรับ (i = 0; i < (8 * urls); i++) {
				ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					bha_nga;
			}
			continue;
		}

		ถ้า (version >= 107) {
			var_na *p = host;
			ขณะ (*p && *p != ' ') p++;
			ขณะ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ขณะ (*p && *p != ' ') p++;
			ขณะ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		ถ้า (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			คืน NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			bha_nga;
		urls = atoi(s);

		/* no URLs => try next host */
		ถ้า (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			continue;
		}

		/* load the non-corrupt data */
		สำหรับ (i = 0; i < urls; i++) {
			sa_nracanaa path_data *p = NULL;
			var_na scheme[64], ports[10];
			var_na url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned จำนวนเต็ม port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			var_na *path_query;
			size_t len;

			ถ้า (!fgets(scheme, parimaa_na scheme, fp))
				bha_nga;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			ถ้า (!fgets(ports, parimaa_na ports, fp))
				bha_nga;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			s[length] = '\0';

			ถ้า (!strcasecmp(host, "localhost") &&
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
			ถ้า (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				คืน NSERROR_NOMEM;
			}

			ถ้า (url_bloom != NULL) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ถ้า (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				คืน NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			ถ้า (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				คืน NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ถ้า (p)
				p->urld.visits = (unsigned จำนวนเต็ม)atoi(s);

			/* entry last use time */
			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				bha_nga;
			}
			ถ้า (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ถ้า (p)
				p->urld.type = (content_type)atoi(s);

			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;


			ถ้า (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			ถ้า (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				ถ้า (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	คืน NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(const var_na *filename)
{
	FILE *fp;
	จำนวนเต็ม i;

	assert(filename);

	fp = fopen(filename, "w");
	ถ้า (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		คืน NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	สำหรับ (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	คืน NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (!p) {
		คืน NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	คืน NSERROR_OK;
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
	unsigned จำนวนเต็ม port_int;

	assert(url);

	ถ้า (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	ถ้า (url_bloom != NULL) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ถ้า (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		คืน false;
	}
	assert(path_query != NULL);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ถ้า (scheme == NULL) {
		free(path_query);
		คืน false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ถ้า (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} มิฉะนั้น ถ้า (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} มิฉะนั้น {
		lwc_string_unref(scheme);
		free(path_query);
		คืน false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	ถ้า (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} มิฉะนั้น {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	ถ้า (h != NULL) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} มิฉะนั้น {
		p = NULL;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	คืน (p != NULL);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, const var_na *title)
{
	sa_nracanaa path_data *p;
	var_na *temp;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (p == NULL) {
		คืน NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	ถ้า (title != NULL) {
		temp = strdup(title);
		ถ้า (temp == NULL) {
			คืน NSERROR_NOMEM;
		}
	} มิฉะนั้น {
		temp = NULL;
	}

	free(p->urld.title);
	p->urld.title = temp;

	คืน NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (!p) {
		คืน NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	คืน NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (!p) {
		คืน NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(NULL);
	p->urld.visits++;

	คืน NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
void urldb_reset_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (!p)
		คืน;

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
	ถ้า (!p)
		คืน NULL;

	u = &p->urld;

	คืน (const sa_nracanaa url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ถ้า (!p)
		คืน NULL;

	คืน p->url;
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

	ถ้า (!p)
		คืน;

	/* Search for host_part */
	สำหรับ (pi = p; pi->parent != NULL; pi = pi->parent)
		;
	h = (sa_nracanaa host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	สำหรับ (space = h->prot_space; space; space = space->next) {
		ถ้า (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			bha_nga;
	}

	ถ้า (space != NULL) {
		/* Overrule existing auth. */
		free(space->auth);
		space->auth = strdup(auth);
	} มิฉะนั้น {
		/* Create a new protection space. */
		space = space_alloc = malloc(parimaa_na(sa_nracanaa prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ถ้า (!space_alloc || !realm_alloc || !auth_alloc) {
			free(space_alloc);
			free(realm_alloc);
			free(auth_alloc);
			คืน;
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
	ถ้า (!p)
		คืน NULL;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	สำหรับ (p_cur = p; p_cur != NULL; p_top = p_cur, p_cur = p_cur->parent) {
		ถ้า (p_cur->prot_space) {
			คืน p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ถ้า (realm != NULL) {
		const sa_nracanaa host_part *h = (const sa_nracanaa host_part *)p_top;
		const sa_nracanaa prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		สำหรับ (space = h->prot_space; space != NULL;
		     space = space->next) {
			ถ้า (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				คืน p->prot_space->auth;
			}
		}
	}

	คืน NULL;
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
	ถ้า (!p)
		คืน;

	สำหรับ (; p && p->parent; p = p->parent)
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
	ถ้า (!p)
		คืน false;

	สำหรับ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	คืน h->permit_invalid_certs;
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
	ถ้า (host != NULL) {
		ถ้า (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			คืน true;
		} มิฉะนั้น ถ้า (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			คืน true;
		}

		lwc_string_unref(host);
	} มิฉะนั้น {
		/* No host part: ignore */
		คืน true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ถ้า (!p)
		คืน false;

	สำหรับ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (sa_nracanaa host_part *)p;
	ถ้า (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		คืน true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	ถ้า (error != NSERROR_OK) {
		/* Parse failed: ignore */
		คืน true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ถ้า (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} มิฉะนั้น ถ้า ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	คืน true;
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
	ถ้า (host != NULL) {
		ถ้า (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			คืน false;
		} มิฉะนั้น ถ้า (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			คืน false;
		}

		lwc_string_unref(host);
	} มิฉะนั้น {
		/* No host part: not enabled */
		คืน false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ถ้า (!p)
		คืน false;

	สำหรับ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	/* Consult record for this host */
	ถ้า (h->hsts.expires > now) {
		/* Not expired */
		คืน true;
	}

	/* Consult parent domains */
	สำหรับ (h = h->parent; h && h != &db_root; h = h->parent) {
		ถ้า (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			คืน true;
		}
	}

	คืน false;
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
	ถ้า (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	ถ้า (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, parimaa_na host, "%.*s",
			 (จำนวนเต็ม) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		ถ้า (!h) {
			จำนวนเต็ม len = slash - prefix;

			ถ้า (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, parimaa_na buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				ถ้า (!h)
					คืน;
			} มิฉะนั้น
				คืน;
		}

		ถ้า (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} มิฉะนั้น {
		จำนวนเต็ม len = strlen(prefix);

		/* looking for hosts */
		ถ้า (!urldb_iterate_partial_host(tree, prefix, callback))
			คืน;

		ถ้า (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, parimaa_na buf, "www.%s", prefix);
			ถ้า(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				คืน;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_entries(bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	จำนวนเต็ม i;

	assert(callback);

	สำหรับ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ถ้า (!urldb_iterate_entries_host(search_trees[i],
						callback,
						NULL)) {
			bha_nga;
		}
	}
}


/* exported interface documented in content/urldb.h */
void urldb_iterate_cookies(bool (*callback)(const sa_nracanaa cookie_data *data))
{
	จำนวนเต็ม i;

	assert(callback);

	สำหรับ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ถ้า (!urldb_iterate_entries_host(search_trees[i],
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
	ถ้า (nsurl_defragment(url, &urlt) != NSERROR_OK)
		คืน NULL;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ถ้า (scheme == NULL) {
		nsurl_unref(urlt);
		คืน false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	ถ้า (path == NULL) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		คืน false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ถ้า (host == NULL) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		คืน false;
	}

	ถ้า (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ถ้า (rhost == NULL) {
			goto error;
		}

		/* Domain match host names */
		ถ้า (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			const var_na *hptr;
			const var_na *rptr;
			const var_na *dot;
			const var_na *host_data = lwc_string_data(host);
			const var_na *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ถ้า (urldb__host_is_ip_address(host_data) ||
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
			ขณะ (hptr >= host_data && rptr >= rhost_data) {
				ถ้า (*hptr != *rptr)
					bha_nga;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ขณะ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ถ้า (*hptr == '\0' ||
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
		ถ้า (!c) {
			/* failed => stop parsing */
			goto error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ถ้า (!c->name || !c->value) {
			urldb_free_cookie(c);
			goto error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		ถ้า (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			goto error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ถ้า (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ถ้า (suffix == NULL) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			goto error;
		}
#มิฉะนั้น
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ถ้า (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			goto error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		ถ้า (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			จำนวนเต็ม hlen, dlen;
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
			ถ้า (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				goto error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			ถ้า (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				goto error;
			}

			ถ้า (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ถ้า (strcasecmp(lwc_string_data(host) + (hlen - dlen),
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
		ถ้า (!urldb_insert_cookie(c, scheme, urlt))
			goto error;
	} ขณะ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	คืน true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	คืน false;
}


/* exported interface documented in content/urldb.h */
var_na *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	const sa_nracanaa path_data *p, *q;
	const sa_nracanaa host_part *h;
	lwc_string *path_lwc;
	sa_nracanaa cookie_internal_data *c;
	จำนวนเต็ม count = 0, version = COOKIE_RFC2965;
	sa_nracanaa cookie_internal_data **matched_cookies;
	จำนวนเต็ม matched_cookies_size = 20;
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
	ถ้า (!p)
		คืน NULL;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 parimaa_na(sa_nracanaa cookie_internal_data *));
	ถ้า (!matched_cookies)
		คืน NULL;

#define GROW_MATCHED_COOKIES						\
	do {								\
		ถ้า (count == matched_cookies_size) {			\
			sa_nracanaa cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       parimaa_na(sa_nracanaa cookie_internal_data *)); \
									\
			ถ้า (temp == NULL) {				\
				free(matched_cookies);			\
				คืน NULL;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ขณะ(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	ถ้า (path_lwc == NULL) {
		free(matched_cookies);
		คืน NULL;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(NULL);

	ถ้า (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		สำหรับ (q = p->parent->children; q; q = q->next) {
			ถ้า (strcmp(q->segment, p->segment))
				continue;

			/* Consider all cookies associated with
			 * this exact path */
			สำหรับ (c = q->cookies; c; c = c->next) {
				ถ้า (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ถ้า (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					continue;

				ถ้า (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ถ้า (c->version < (unsigned จำนวนเต็ม)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	สำหรับ (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		สำหรับ (q = p->children; q; q = q->next) {
			ถ้า (*(q->segment) != '\0')
				continue;

			สำหรับ (c = q->cookies; c; c = c->next) {
				ถ้า (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ถ้า (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ถ้า (c->version < (unsigned จำนวนเต็ม) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}

		ถ้า (!p->parent) {
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
		สำหรับ (c = p->cookies; c; c = c->next) {
			ถ้า (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ถ้า (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ถ้า (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ถ้า (c->version < (unsigned จำนวนเต็ม) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	สำหรับ (h = (const sa_nracanaa host_part *)p; h && h != &db_root;
	     h = h->parent) {
		สำหรับ (c = h->paths.cookies; c; c = c->next) {
			ถ้า (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ถ้า (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ถ้า (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ถ้า (c->version < (unsigned จำนวนเต็ม)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	ถ้า (count == 0) {
		/* No cookies found */
		free(matched_cookies);
		คืน NULL;
	}

	ถ้า (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} มิฉะนั้น {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	free(matched_cookies);

	คืน ret;
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
	ถ้า (!fp)
		คืน NSERROR_NOT_FOUND;

#define FIND_T {				\
		สำหรับ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ถ้า (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		สำหรับ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ถ้า (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
	}

	ขณะ (fgets(s, parimaa_na s, fp)) {
		var_na *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		จำนวนเต็ม version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		sa_nracanaa cookie_internal_data *c;

		ถ้า(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			continue;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ถ้า (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			ถ้า (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				bha_nga;
			}

			continue;
		} มิฉะนั้น ถ้า (loaded_cookie_file_version == 0) {
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
		ถ้า (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} มิฉะนั้น {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		ถ้า (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} มิฉะนั้น {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		สำหรับ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		assert(p <= end);

		/* Now create cookie */
		c = malloc(parimaa_na(sa_nracanaa cookie_internal_data));
		ถ้า (!c)
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

		ถ้า (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			bha_nga;
		}

		ถ้า (c->domain[0] != '.') {
			lwc_string *scheme_lwc = NULL;
			nsurl *url_nsurl = NULL;

			assert(scheme[0] != 'u');

			ถ้า (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				bha_nga;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ถ้า (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				bha_nga;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} มิฉะนั้น {
			ถ้า (!urldb_insert_cookie(c, NULL, NULL)) {
				/* Cookie freed for us */
				bha_nga;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	คืน NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(const var_na *filename)
{
	FILE *fp;
	จำนวนเต็ม cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	assert(filename);

	fp = fopen(filename, "w");
	ถ้า (!fp)
		คืน NSERROR_NOT_FOUND;

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

	คืน NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_dump(void)
{
	จำนวนเต็ม i;

	urldb_dump_hosts(&db_root);

	สำหรับ (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
