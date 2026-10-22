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
	unsigned ចំនួនគត់ port;
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
	unsigned ចំនួនគត់ visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
sa_nracanaa path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned ចំនួនគត់ port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	var_na *segment;		/**< Path segment for this node */
	unsigned ចំនួនគត់ frag_cnt;	/**< Number of entries in path_data::fragment */
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

	unsigned ចំនួនគត់ level;		/**< Node level */

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
static ចំនួនគត់ loaded_cookie_file_version;

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
	ចំនួនគត់ use;
	var_na op[32];

	use = nsc_sntimet(op, 32, &val);
	ប្រសិនបើ (use == 0) {
		fprintf(fp, "%i\n", (ចំនួនគត់)val);
	} បើមិនដូច្នេះ {
		fprintf(fp, "%.*s\n", use, op);
	}
	ត្រឡប់ NSERROR_OK;
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
		  ចំនួនគត់ *path_alloc,
		  ចំនួនគត់ *path_used,
		  time_t expiry)
{
	const sa_nracanaa path_data *p = parent;
	ចំនួនគត់ i;

	do {
		ចំនួនគត់ seglen = p->segment != NULL ? strlen(p->segment) : 0;
		ចំនួនគត់ len = *path_used + seglen + 1;

		ប្រសិនបើ (*path_alloc < len) {
			var_na *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			ប្រសិនបើ (!temp) {
				ត្រឡប់;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		ប្រសិនបើ (p->segment != NULL) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		ប្រសិនបើ (p->children != NULL) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} បើមិនដូច្នេះ {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		ប្រសិនបើ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} បើមិនដូច្នេះ {
			/* leaf node */
			ប្រសិនបើ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				ប្រសិនបើ (p->port) {
					fprintf(fp,"%d\n", p->port);
				} បើមិនដូច្នេះ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (ចំនួនគត់)p->urld.type);

				fprintf(fp, "\n");

				ប្រសិនបើ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					សម្រាប់ (i = 0; s[i] != '\0'; i++)
						ប្រសិនបើ (s[i] < 32)
							s[i] = ' ';
					សម្រាប់ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} បើមិនដូច្នេះ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ខណៈពេល (p != parent) {
				ចំនួនគត់ seglen = p->segment != NULL
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				ប្រសិនបើ (p->next != NULL) {
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
	} ខណៈពេល (p != parent);
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
		 unsigned ចំនួនគត់ *count)
{
	const sa_nracanaa path_data *p = root;

	do {
		ប្រសិនបើ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} បើមិនដូច្នេះ {
			/* No more children, increment count if required */
			ប្រសិនបើ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			ខណៈពេល (p != root) {
				ប្រសិនបើ (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ខណៈពេល (p != root);
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
	unsigned ចំនួនគត់ path_count = 0;
	var_na *path, *p, *end;
	ចំនួនគត់ path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	ចំនួនគត់ hsts_include_subdomains = 0;

	expiry = time(NULL) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ប្រសិនបើ (parent == &empty)
		ត្រឡប់;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	ប្រសិនបើ (!path)
		ត្រឡប់;

	path[0] = '\0';

	សម្រាប់ (h = parent->data, p = host, end = host + parimaa_na host;
	     h && h != &db_root && p < end; h = h->parent) {
		ចំនួនគត់ written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		ប្រសិនបើ (written < 0) {
			free(path);
			ត្រឡប់;
		}
		p += written;
	}

	h = parent->data;
	ប្រសិនបើ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	ប្រសិនបើ (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} បើមិនដូច្នេះ ប្រសិនបើ (hsts_expiry) {
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
		ប្រសិនបើ (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} បើមិនដូច្នេះ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			assert(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ប្រសិនបើ (url_callback) {
				const sa_nracanaa url_internal_data *u = &p->urld;

				assert(p->url);

				ប្រសិនបើ (!url_callback(p->url,
						  (const sa_nracanaa url_data *) u))
					ត្រឡប់ false;
			} បើមិនដូច្នេះ {
				c = (const sa_nracanaa cookie_data *)p->cookies;
				សម្រាប់ (; c != NULL; c = c->next) {
					ប្រសិនបើ (!cookie_callback(c))
						ត្រឡប់ false;
				}
			}

			/* Now, find next node to process. */
			ខណៈពេល (p != parent) {
				ប្រសិនបើ (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					bha_nga;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ខណៈពេល (p != parent);

	ត្រឡប់ true;
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
	unsigned ចំនួនគត់ ipv6_addr_len;
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
	ប្រសិនបើ (slash == NULL) {
		sane_host = host;
	} បើមិនដូច្នេះ {
		var_na *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ប្រសិនបើ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		goto out_false;

	ប្រសិនបើ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		ចំនួនគត់ num_dots = 0;
		size_t index;

		សម្រាប់ (index = 0; index < host_len; index++) {
			ប្រសិនបើ (sane_host[index] == '.')
				num_dots++;
		}

		ប្រសិនបើ (num_dots == 3)
			goto out_true;
		បើមិនដូច្នេះ
			goto out_false;
	}

#ifndef NO_IPV6
	ប្រសិនបើ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		goto out_false;
	}

	ipv6_addr_len = host_len - 2;
	ប្រសិនបើ (ipv6_addr_len >= parimaa_na(ipv6_addr)) {
		ipv6_addr_len = parimaa_na(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ប្រសិនបើ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		goto out_true;
#endif

out_false:
	ប្រសិនបើ (slash != NULL) free((void *)sane_host);
	ត្រឡប់ false;

out_true:
	ប្រសិនបើ (slash != NULL) free((void *)sane_host);
	ត្រឡប់ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
static ចំនួនគត់ urldb_search_match_prefix(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	ចំនួនគត់ plen, ret;

	assert(a && a != &db_root && b);

	ប្រសិនបើ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ត្រឡប់ strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ខណៈពេល (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ប្រសិនបើ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ប្រសិនបើ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ត្រឡប់ ret;

		/* The strings matched */
		ប្រសិនបើ (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			ប្រសិនបើ (plen > dot - b) {
				/* len(a) > len(b) */
				ត្រឡប់ 1;
			} បើមិនដូច្នេះ ប្រសិនបើ (plen < dot - b) {
				/* len(a) < len(b) */
				ត្រឡប់ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ប្រសិនបើ (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		ត្រឡប់ 0;
	} បើមិនដូច្នេះ ប្រសិនបើ ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		ត្រឡប់ -1;
	}

	/* Identical */
	ត្រឡប់ 0;
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
	ចំនួនគត់ c;

	assert(root && prefix && callback);

	ប្រសិនបើ (root == &empty)
		ត្រឡប់ true;

	c = urldb_search_match_prefix(root->data, prefix);

	ប្រសិនបើ (c > 0) {
		/* No match => look in left subtree */
		ត្រឡប់ urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} បើមិនដូច្នេះ ប្រសិនបើ (c < 0) {
		/* No match => look in right subtree */
		ត្រឡប់ urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} បើមិនដូច្នេះ {
		/* Match => iterate over l/r subtrees & process this node */
		ប្រសិនបើ (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			ត្រឡប់ false;
		}

		ប្រសិនបើ (root->data->paths.children) {
			/* and extract all paths attached to this host */
			ប្រសិនបើ (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							NULL)) {
				ត្រឡប់ false;
			}
		}

		ប្រសិនបើ (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			ត្រឡប់ false;
		}
	}

	ត្រឡប់ true;
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
		ប្រសិនបើ (!slash) {
			slash = end;
		}

		ប្រសិនបើ (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			continue;
		}

		ប្រសិនបើ (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			ប្រសិនបើ (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ប្រសិនបើ (!urldb_iterate_entries_path(p,
								callback,
								NULL)) {
					ត្រឡប់ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} បើមិនដូច្នេះ {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} បើមិនដូច្នេះ {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ខណៈពេល (p != NULL);

	ត្រឡប់ true;
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
	ប្រសិនបើ (parent == &empty) {
		ត្រឡប់ true;
	}

	ប្រសិនបើ (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		ត្រឡប់ false;
	}

	ប្រសិនបើ ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ប្រសិនបើ (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			ត្រឡប់ false;
		}
	}

	ប្រសិនបើ (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		ត្រឡប់ false;
	}

	ត្រឡប់ true;
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
	ប្រសិនបើ (!d) {
		ត្រឡប់ NULL;
	}

	d->part = strdup(part);
	ប្រសិនបើ (!d->part) {
		free(d);
		ត្រឡប់ NULL;
	}

	d->next = parent->children;
	ប្រសិនបើ (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	ត្រឡប់ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
static ចំនួនគត់ urldb_add_path_fragment_cmp(const void *a, const void *b)
{
	ត្រឡប់ strcasecmp(*((const var_na **) a), *((const var_na **) b));
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
	ប្រសិនបើ (!fragment)
		ត្រឡប់ segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * parimaa_na(var_na *));
	ប្រសិនបើ (!temp)
		ត្រឡប់ NULL;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	ប្រសិនបើ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ត្រឡប់ NULL;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      parimaa_na (var_na *),
	      urldb_add_path_fragment_cmp);

	ត្រឡប់ segment;
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
		    unsigned ចំនួនគត់ port,
		    const var_na *segment,
		    lwc_string *fragment,
		    sa_nracanaa path_data *parent)
{
	sa_nracanaa path_data *d, *e;

	assert(scheme && segment && parent);

	d = calloc(1, parimaa_na(sa_nracanaa path_data));
	ប្រសិនបើ (!d)
		ត្រឡប់ NULL;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	ប្រសិនបើ (!d->segment) {
		lwc_string_unref(d->scheme);
		free(d);
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ (fragment) {
		ប្រសិនបើ (!urldb_add_path_fragment(d, fragment)) {
			free(d->segment);
			lwc_string_unref(d->scheme);
			free(d);
			ត្រឡប់ NULL;
		}
	}

	សម្រាប់ (e = parent->children; e; e = e->next) {
		ប្រសិនបើ (strcmp(e->segment, d->segment) > 0)
			bha_nga;
	}

	ប្រសិនបើ (e) {
		d->prev = e->prev;
		d->next = e;
		ប្រសិនបើ (e->prev)
			e->prev->next = d;
		បើមិនដូច្នេះ
			parent->children = d;
		e->prev = d;
	} បើមិនដូច្នេះ ប្រសិនបើ (!parent->children) {
		d->prev = d->next = NULL;
		parent->children = parent->last = d;
	} បើមិនដូច្នេះ {
		d->next = NULL;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	ត្រឡប់ d;
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

	ប្រសិនបើ (urldb__host_is_ip_address(host)) {
		ត្រឡប់ &search_trees[ST_IP];
	} បើមិនដូច្នេះ ប្រសិនបើ (ascii_is_alpha(*host)) {
		ត្រឡប់ &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	ត្រឡប់ &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static sa_nracanaa search_node *urldb_get_search_tree(const var_na *host)
{
	ត្រឡប់ *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
static ចំនួនគត់ urldb_search_match_string(const sa_nracanaa host_part *a, const var_na *b)
{
	const var_na *end, *dot;
	ចំនួនគត់ plen, ret;

	assert(a && a != &db_root && b);

	ប្រសិនបើ (urldb__host_is_ip_address(b)) {
		/* IP address */
		ត្រឡប់ strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ខណៈពេល (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		ប្រសិនបើ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ប្រសិនបើ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ត្រឡប់ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		ប្រសិនបើ (plen > dot - b) {
			/* len(a) > len(b) */
			ត្រឡប់ 1;
		} បើមិនដូច្នេះ ប្រសិនបើ (plen < dot - b) {
			/* len(a) < len(b) */
			ត្រឡប់ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ប្រសិនបើ (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		ត្រឡប់ 1;
	} បើមិនដូច្នេះ ប្រសិនបើ ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		ត្រឡប់ -1;
	}

	/* Identical */
	ត្រឡប់ 0;
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
	ចំនួនគត់ c;

	assert(root && host);

	ប្រសិនបើ (root == &empty) {
		ត្រឡប់ NULL;
	}

	c = urldb_search_match_string(root->data, host);

	ប្រសិនបើ (c > 0) {
		ត្រឡប់ urldb_search_find(root->left, host);
	} បើមិនដូច្នេះ ប្រសិនបើ (c < 0) {
		ត្រឡប់ urldb_search_find(root->right, host);
	}

	ត្រឡប់ root->data;
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

	ប្រសិនបើ (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	assert(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ខណៈពេល (p != NULL) {
		slash = strchr(path + 1, '/');
		ប្រសិនបើ (!slash) {
			slash = path + strlen(path);
		}

		ប្រសិនបើ (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			ប្រសិនបើ (*slash == '\0') {
				/* Complete match */
				ត្រឡប់ (sa_nracanaa path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} បើមិនដូច្នេះ {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	ត្រឡប់ NULL;
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
	unsigned ចំនួនគត់ port_int;
	bool match;

	assert(url);

	ប្រសិនបើ (url_bloom != NULL) {
		ប្រសិនបើ (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			ត្រឡប់ NULL;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ប្រសិនបើ (scheme == NULL)
		ត្រឡប់ NULL;

	ប្រសិនបើ (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		ត្រឡប់ NULL;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ប្រសិនបើ (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} បើមិនដូច្នេះ ប្រសិនបើ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} បើមិនដូច្នេះ {
		lwc_string_unref(scheme);
		ត្រឡប់ NULL;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	ប្រសិនបើ (!h) {
		lwc_string_unref(scheme);
		ត្រឡប់ NULL;
	}

	/* generate plq (path, leaf, query) */
	ប្រសិនបើ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ត្រឡប់ NULL;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	ប្រសិនបើ (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} បើមិនដូច្នេះ {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	free(plq);
	lwc_string_unref(scheme);

	ត្រឡប់ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_paths(sa_nracanaa path_data *parent)
{
	const sa_nracanaa path_data *p = parent;
	unsigned ចំនួនគត់ i;

	do {
		ប្រសិនបើ (p->segment != NULL) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			សម្រាប់ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ប្រសិនបើ (p->children != NULL) {
			p = p->children;
		} បើមិនដូច្នេះ {
			ខណៈពេល (p != parent) {
				ប្រសិនបើ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ខណៈពេល (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_hosts(sa_nracanaa host_part *parent)
{
	sa_nracanaa host_part *h;

	ប្រសិនបើ (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	សម្រាប់ (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
static void urldb_dump_search(sa_nracanaa search_node *parent, ចំនួនគត់ depth)
{
	const sa_nracanaa host_part *h;
	ចំនួនគត់ i; /* index into string */
	var_na s[1024];
	ចំនួនគត់ r;
	ចំនួនគត់ sl = parimaa_na(s) - 2;

	ប្រសិនបើ (parent == &empty)
		ត្រឡប់;

	urldb_dump_search(parent->left, depth + 1);

	សម្រាប់ (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	សម្រាប់ (h = parent->data; h; h = h->parent) {
		ប្រសិនបើ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ប្រសិនបើ (r < 0) {
				bha_nga;
			}
			ប្រសិនបើ ((i + r) >= sl) {
				bha_nga;
			}
			i += r;
		}

		ប្រសិនបើ (h->parent && h->parent->parent) {
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
static ចំនួនគត់
urldb_search_match_host(const sa_nracanaa host_part *a, const sa_nracanaa host_part *b)
{
	ចំនួនគត់ ret;

	assert(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	សម្រាប់ (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		ប្រសិនបើ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ត្រឡប់ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ប្រសិនបើ (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		ត្រឡប់ 1;
	} បើមិនដូច្នេះ ប្រសិនបើ ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		ត្រឡប់ -1;
	}

	/* identical */
	ត្រឡប់ 0;
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

	ប្រសិនបើ (root->left->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	ត្រឡប់ root;
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

	ប្រសិនបើ (root->right->right->level == root->level) {
		sa_nracanaa search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	ត្រឡប់ root;
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

	ប្រសិនបើ (root == &empty) {
		root = n;
	} បើមិនដូច្នេះ {
		ចំនួនគត់ c = urldb_search_match_host(root->data, n->data);

		ប្រសិនបើ (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} បើមិនដូច្នេះ ប្រសិនបើ (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} បើមិនដូច្នេះ {
			/* exact match */
			free(n);
			ត្រឡប់ root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	ត្រឡប់ root;
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
	ប្រសិនបើ (!n)
		ត្រឡប់ NULL;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	ត្រឡប់ root;
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
	ចំនួនគត់ vlen;

	assert(c && n && v);

	/* Strip whitespace from start of name */
	សម្រាប់ (; *n; n++) {
		ប្រសិនបើ (*n != ' ' && *n != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of name */
	សម្រាប់ (vlen = strlen(n); vlen; vlen--) {
		ប្រសិនបើ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		បើមិនដូច្នេះ
			bha_nga;
	}

	/* Strip whitespace from start of value */
	សម្រាប់ (; *v; v++) {
		ប្រសិនបើ (*v != ' ' && *v != '\t')
			bha_nga;
	}

	/* Strip whitespace from end of value */
	សម្រាប់ (vlen = strlen(v); vlen; vlen--) {
		ប្រសិនបើ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		បើមិនដូច្នេះ
			bha_nga;
	}

	ប្រសិនបើ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ប្រសិនបើ (!c->comment)
			ត្រឡប់ false;
	} បើមិនដូច្នេះ ប្រសិនបើ (!c->domain && strcasecmp(n, "Domain") == 0) {
		ប្រសិនបើ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			ប្រសិនបើ (!c->domain)
				ត្រឡប់ false;
		}
	} បើមិនដូច្នេះ ប្រសិនបើ (strcasecmp(n, "Max-Age") == 0) {
		ចំនួនគត់ temp = atoi(v);
		ប្រសិនបើ (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		បើមិនដូច្នេះ
			c->expires = time(NULL) + temp;
	} បើមិនដូច្នេះ ប្រសិនបើ (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		ប្រសិនបើ (!c->path)
			ត្រឡប់ false;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} បើមិនដូច្នេះ ប្រសិនបើ (strcasecmp(n, "Expires") == 0) {
		var_na *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		សម្រាប់ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		ប្រសិនបើ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} បើមិនដូច្នេះ ប្រសិនបើ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} បើមិនដូច្នេះ ប្រសិនបើ (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		ប្រសិនបើ (!c->name || !c->value) {
			ត្រឡប់ false;
		}
	}

	ត្រឡប់ true;
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
	ប្រសិនបើ (c == NULL)
		ត្រឡប់ NULL;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	សម្រាប់ (cur = *cookie; *cur; cur++) {
		ប្រសិនបើ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ប្រសិនបើ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			bha_nga;
		} បើមិនដូច្នេះ ប្រសិនបើ (*cur == '\r') {
			/* Spurious linefeed */
			continue;
		} បើមិនដូច្នេះ ប្រសិនបើ (*cur == '\n') {
			/* Spurious newline */
			continue;
		}

		ប្រសិនបើ (in_value && !had_value_data) {
			ប្រសិនបើ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				continue;
			} បើមិនដូច្នេះ {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ប្រសិនបើ (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		ប្រសិនបើ (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ប្រសិនបើ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				continue;
			}
		}

		ប្រសិនបើ (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			continue;
		}

		ប្រសិនបើ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ប្រសិនបើ (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ត្រឡប់ NULL;
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
			ប្រសិនបើ (*cur != ';') {
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
		ប្រសិនបើ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			const var_na *p;
			const var_na *semi = strchr(cur + 1, ';');
			ប្រសិនបើ (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			សម្រាប់ (p = cur + 1; p < semi; p++)
				ប្រសិនបើ (*p == '=')
					bha_nga;

			ប្រសិនបើ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} បើមិនដូច្នេះ {
				/* found one => comma marks end of cookie */
				cur++;
				bha_nga;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ប្រសិនបើ (!in_value) {
			ប្រសិនបើ (n < name + (parimaa_na(name) - 1))
				*n++ = *cur;
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (v < value + (parimaa_na(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ប្រសិនបើ (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ត្រឡប់ NULL;
	}

	/* Now fix-up default values */
	ប្រសិនបើ (c->domain == NULL) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ប្រសិនបើ (host == NULL) {
			urldb_free_cookie(c);
			ត្រឡប់ NULL;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	ប្រសិនបើ (c->path == NULL) {
		const var_na *path_data;
		var_na *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		ប្រសិនបើ (path_lwc == NULL) {
			urldb_free_cookie(c);
			ត្រឡប់ NULL;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		ប្រសិនបើ (slash != NULL) {
			/* Special case: retain first slash in path */
			ប្រសិនបើ (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			ប្រសិនបើ (slash == NULL) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				ត្រឡប់ NULL;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} បើមិនដូច្នេះ {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			ប្រសិនបើ (path == NULL) {
				urldb_free_cookie(c);
				ត្រឡប់ NULL;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	ត្រឡប់ c;
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
	       unsigned ចំនួនគត់ port,
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
	ប្រសិនបើ (*segment == '/')
		segment++;

	/* Process path segments */
	do {
		slash = strchr(segment, '/');
		ប្រសិនបើ (!slash) {
			/* last segment */
			/* look for existing entry */
			សម្រាប់ (e = d->children; e; e = e->next)
				ប្រសិនបើ (strcmp(segment, e->segment) == 0 &&
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
		សម្រាប់ (e = d->children; e; e = e->next)
			ប្រសិនបើ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				bha_nga;

		d = e ? e : urldb_add_path_node(scheme, port, segment, NULL, d);
		ប្រសិនបើ (!d)
			bha_nga;

		segment = slash + 1;
	} ខណៈពេល (1);

	free(path_query);

	ប្រសិនបើ (d && !d->url) {
		/* Insert defragmented URL */
		ប្រសិនបើ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ត្រឡប់ NULL;
	}

	ត្រឡប់ d;
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

	ប្រសិនបើ (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		សម្រាប់ (e = d->children; e; e = e->next)
			ប្រសិនបើ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ត្រឡប់ e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		ប្រសិនបើ (!s) {
			/* failed */
			d = NULL;
		} បើមិនដូច្នេះ {
			search_trees[ST_IP] = s;
		}

		ត្រឡប់ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, parimaa_na buf);
	buf[parimaa_na buf - 1] = '\0';

	/* Process FQDN segments backwards */
	do {
		part = strrchr(buf, '.');
		ប្រសិនបើ (!part) {
			/* last segment */
			/* Check for existing entry */
			សម្រាប់ (e = d->children; e; e = e->next)
				ប្រសិនបើ (strcasecmp(buf, e->part) == 0)
					bha_nga;

			ប្រសិនបើ (e) {
				d = e;
			} បើមិនដូច្នេះ {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			ប្រសិនបើ (d) {
				sa_nracanaa search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				ប្រសិនបើ (!s) {
					/* failed */
					d = NULL;
				} បើមិនដូច្នេះ {
					*r = s;
				}
			}
			bha_nga;
		}

		/* Check for existing entry */
		សម្រាប់ (e = d->children; e; e = e->next)
			ប្រសិនបើ (strcasecmp(part + 1, e->part) == 0)
				bha_nga;

		d = e ? e : urldb_add_host_node(part + 1, d);
		ប្រសិនបើ (!d)
			bha_nga;

		*part = '\0';
	} ខណៈពេល (1);

	ត្រឡប់ d;
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

	ប្រសិនបើ (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		ប្រសិនបើ (!h) {
			h = urldb_add_host(c->domain + 1);
			ប្រសិនបើ (!h) {
				urldb_free_cookie(c);
				ត្រឡប់ false;
			}
		}

		p = (sa_nracanaa path_data *) &h->paths;
	} បើមិនដូច្នេះ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		assert(url != NULL);
		assert(scheme != NULL);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		ប្រសិនបើ (!h) {
			h = urldb_add_host(c->domain);
			ប្រសិនបើ (!h) {
				urldb_free_cookie(c);
				ត្រឡប់ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), NULL, url);
		ប្រសិនបើ (!p) {
			urldb_free_cookie(c);
			ត្រឡប់ false;
		}
	}

	/* add cookie */
	សម្រាប់ (d = p->cookies; d; d = d->next) {
		ប្រសិនបើ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			bha_nga;
	}

	ប្រសិនបើ (d) {
		ប្រសិនបើ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ប្រសិនបើ (d->next)
				d->next->prev = d->prev;
			បើមិនដូច្នេះ
				p->cookies_end = d->prev;
			ប្រសិនបើ (d->prev)
				d->prev->next = d->next;
			បើមិនដូច្នេះ
				p->cookies = d->next;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} បើមិនដូច្នេះ {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			ប្រសិនបើ (c->next)
				c->next->prev = c;
			បើមិនដូច្នេះ
				p->cookies_end = c;
			ប្រសិនបើ (c->prev)
				c->prev->next = c;
			បើមិនដូច្នេះ
				p->cookies = c;

			cookie_manager_remove((sa_nracanaa cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	} បើមិនដូច្នេះ {
		c->prev = p->cookies_end;
		c->next = NULL;
		ប្រសិនបើ (p->cookies_end)
			p->cookies_end->next = c;
		បើមិនដូច្នេះ
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((sa_nracanaa cookie_data *)c);
	}

	ត្រឡប់ true;
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
		    ចំនួនគត់ *used,
		    ចំនួនគត់ *alloc,
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
	ចំនួនគត់ max_len;
	ចំនួនគត់ outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	ប្រសិនបើ (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ប្រសិនបើ (!temp) {
			ត្រឡប់ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	ប្រសិនបើ (outl >= (*alloc - *used)) {
		/* output truncated */
		ត្រឡប់ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ប្រសិនបើ (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		ប្រសិនបើ (outl >= (*alloc - *used)) {
			/* output truncated */
			ត្រឡប់ false;
		}
		*used += outl;
	} បើមិនដូច្នេះ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		ប្រសិនបើ (outl >= (*alloc - *used)) {
			/* output truncated */
			ត្រឡប់ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ត្រឡប់ true;
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
		      ចំនួនគត់ *used,
		      ចំនួនគត់ *alloc,
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
	ចំនួនគត់ max_len;
	ចំនួនគត់ outl; /* output length */

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

	ប្រសិនបើ (max_len >= *alloc) {
		var_na *temp = realloc(*buf, max_len);
		ប្រសិនបើ (!temp) {
			ត្រឡប់ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	ប្រសិនបើ (outl >= (*alloc - *used)) {
		/* output truncated */
		ត្រឡប់ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ប្រសិនបើ (c->value_was_quoted ||
	    strpbrk(c->value, separators) != NULL) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} បើមិនដូច្នេះ {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	ប្រសិនបើ (outl >= (*alloc - *used)) {
		/* output truncated */
		ត្រឡប់ false;
	}
	*used += outl;

	ប្រសិនបើ (c->path_from_set) {
		/* Path, quoted if necessary */

		ប្រសិនបើ (strpbrk(c->path, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} បើមិនដូច្នេះ {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		ប្រសិនបើ (outl >= (*alloc - *used)) {
			/* output truncated */
			ត្រឡប់ false;
		}
		*used += outl;
	}

	ប្រសិនបើ (c->domain_from_set) {
		/* Domain, quoted if necessary */
		ប្រសិនបើ (strpbrk(c->domain, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} បើមិនដូច្នេះ {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		ប្រសិនបើ (outl >= (*alloc - *used)) {
			/* output truncated */
			ត្រឡប់ false;
		}
		*used += outl;
	}

	ត្រឡប់ true;
}


/**
 * build output cookie string in netscape format
 */
static var_na *
get_netscape_cookie(sa_nracanaa cookie_internal_data **matched_cookies, ចំនួនគត់ count)
{
	ចំនួនគត់ ret_alloc = 4096, ret_used = 1;
	var_na *ret;
	ចំនួនគត់ midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	ប្រសិនបើ (ret == NULL) {
		ត្រឡប់ NULL;
	}
	ret[0] = '\0';

	សម្រាប់ (midx = 0; midx < count; midx++) {
		ប្រសិនបើ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			free(ret);
			ត្រឡប់ NULL;
		}
	}

	ប្រសិនបើ (ret_used <= 2) {
		free(ret);
		ត្រឡប់ NULL;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ប្រសិនបើ (temp != NULL) {
		ret = temp;
	}

	ត្រឡប់ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
static var_na *
get_rfc2965_cookie(sa_nracanaa cookie_internal_data **matched_cookies,
		   ចំនួនគត់ count,
		   ចំនួនគត់ version)
{
	ចំនួនគត់ ret_alloc = 4096; /* return string allocation */
	ចំនួនគត់ ret_used = 1; /* return sting buffer used */
	var_na *ret; /* return string buffer */
	ចំនួនគត់ midx; /* index into matched cookies */
	ចំនួនគត់ outl;

	ret = malloc(ret_alloc);
	ប្រសិនបើ (ret == NULL) {
		ត្រឡប់ NULL;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	ប្រសិនបើ (outl >= ret_alloc - ret_used) {
		/* overflow */
		free(ret);
		ត្រឡប់ NULL;
	}
	ret_used += outl;

	សម្រាប់ (midx = 0; midx < count; midx++) {
		ប្រសិនបើ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			free(ret);
			ត្រឡប់ NULL;
		}
	}

	/* shrink the output buffer to the required size */
	var_na *temp = realloc(ret, ret_used);
	ប្រសិនបើ (temp != NULL) {
		ret = temp;
	}

	ត្រឡប់ ret;
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
		សម្រាប់ (c = p->cookies; c; c = c->next) {
			ប្រសិនបើ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				ប្រសិនបើ (c->prev) {
					c->prev->next = c->next;
				} បើមិនដូច្នេះ {
					p->cookies = c->next;
				}

				ប្រសិនបើ (c->next) {
					c->next->prev = c->prev;
				} បើមិនដូច្នេះ {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				ត្រឡប់;
			}
		}

		ប្រសិនបើ (p->children) {
			p = p->children;
		} បើមិនដូច្នេះ {
			ខណៈពេល (p != parent) {
				ប្រសិនបើ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ខណៈពេល (p != parent);
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

	សម្រាប់ (h = parent->children; h; h = h->next) {
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
		ប្រសិនបើ (p->cookies != NULL) {
			sa_nracanaa cookie_internal_data *c;

			សម្រាប់ (c = p->cookies; c != NULL; c = c->next) {
				ប្រសិនបើ (c->expires == -1 || c->expires < now) {
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
					(ចំនួនគត់)c->expires, (ចំនួនគត់)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ប្រសិនបើ (p->children != NULL) {
			p = p->children;
		} បើមិនដូច្នេះ {
			ខណៈពេល (p != parent) {
				ប្រសិនបើ (p->next != NULL) {
					p = p->next;
					bha_nga;
				}

				p = p->parent;
			}
		}
	} ខណៈពេល (p != parent);
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

	សម្រាប់ (h = parent->children; h; h = h->next)
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
	unsigned ចំនួនគត់ i;

	ប្រសិនបើ (node->url != NULL) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	free(node->segment);
	សម្រាប់ (i = 0; i < node->frag_cnt; i++)
		free(node->fragment[i]);
	free(node->fragment);

	free(node->urld.title);

	សម្រាប់ (a = node->cookies; a; a = b) {
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
		ប្រសិនបើ (p->children != NULL) {
			p = p->children;
		} បើមិនដូច្នេះ {
			sa_nracanaa path_data *q = p;

			ខណៈពេល (p != root) {
				ប្រសិនបើ (p->next != NULL) {
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
	} ខណៈពេល (p != root);
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
	សម្រាប់ (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	សម្រាប់ (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	សម្រាប់ (s = root->prot_space; s; s = t) {
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
	ប្រសិនបើ (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	ប្រសិនបើ (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	free(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
void urldb_destroy(void)
{
	sa_nracanaa host_part *a, *b;
	ចំនួនគត់ i;

	/* Clean up search trees */
	សម្រាប់ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ប្រសិនបើ (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	សម្រាប់ (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, parimaa_na(db_root));

	/* And the bloom filter */
	ប្រសិនបើ (url_bloom != NULL) {
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
	ចំនួនគត់ urls;
	ចំនួនគត់ i;
	ចំនួនគត់ version;
	ចំនួនគត់ length;
	FILE *fp;

	assert(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ប្រសិនបើ (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	ប្រសិនបើ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ត្រឡប់ NSERROR_NOT_FOUND;
	}

	ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		ត្រឡប់ NSERROR_NEED_DATA;
	}

	version = atoi(s);
	ប្រសិនបើ (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ត្រឡប់ NSERROR_INVALID;
	}
	ប្រសិនបើ (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ត្រឡប់ NSERROR_INVALID;
	}

	ខណៈពេល (fgets(host, parimaa_na host, fp)) {
		time_t hsts_expiry = 0;
		ចំនួនគត់ hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		ប្រសិនបើ (length == 0) {
			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			urls = atoi(s);
			/* Eight fields/url */
			សម្រាប់ (i = 0; i < (8 * urls); i++) {
				ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					bha_nga;
			}
			continue;
		}

		ប្រសិនបើ (version >= 107) {
			var_na *p = host;
			ខណៈពេល (*p && *p != ' ') p++;
			ខណៈពេល (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ខណៈពេល (*p && *p != ' ') p++;
			ខណៈពេល (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		ប្រសិនបើ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ត្រឡប់ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			bha_nga;
		urls = atoi(s);

		/* no URLs => try next host */
		ប្រសិនបើ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			continue;
		}

		/* load the non-corrupt data */
		សម្រាប់ (i = 0; i < urls; i++) {
			sa_nracanaa path_data *p = NULL;
			var_na scheme[64], ports[10];
			var_na url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned ចំនួនគត់ port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			var_na *path_query;
			size_t len;

			ប្រសិនបើ (!fgets(scheme, parimaa_na scheme, fp))
				bha_nga;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			ប្រសិនបើ (!fgets(ports, parimaa_na ports, fp))
				bha_nga;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			s[length] = '\0';

			ប្រសិនបើ (!strcasecmp(host, "localhost") &&
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
			ប្រសិនបើ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ត្រឡប់ NSERROR_NOMEM;
			}

			ប្រសិនបើ (url_bloom != NULL) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ប្រសិនបើ (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ត្រឡប់ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			ប្រសិនបើ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ត្រឡប់ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ប្រសិនបើ (p)
				p->urld.visits = (unsigned ចំនួនគត់)atoi(s);

			/* entry last use time */
			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				bha_nga;
			}
			ប្រសិនបើ (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			ប្រសិនបើ (p)
				p->urld.type = (content_type)atoi(s);

			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;


			ប្រសិនបើ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				bha_nga;
			length = strlen(s) - 1;
			ប្រសិនបើ (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				ប្រសិនបើ (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	ត្រឡប់ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(const var_na *filename)
{
	FILE *fp;
	ចំនួនគត់ i;

	assert(filename);

	fp = fopen(filename, "w");
	ប្រសិនបើ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ត្រឡប់ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	សម្រាប់ (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p) {
		ត្រឡប់ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	ត្រឡប់ NSERROR_OK;
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
	unsigned ចំនួនគត់ port_int;

	assert(url);

	ប្រសិនបើ (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	ប្រសិនបើ (url_bloom != NULL) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ប្រសិនបើ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		ត្រឡប់ false;
	}
	assert(path_query != NULL);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ប្រសិនបើ (scheme == NULL) {
		free(path_query);
		ត្រឡប់ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ប្រសិនបើ (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} បើមិនដូច្នេះ ប្រសិនបើ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} បើមិនដូច្នេះ {
		lwc_string_unref(scheme);
		free(path_query);
		ត្រឡប់ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	ប្រសិនបើ (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} បើមិនដូច្នេះ {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	ប្រសិនបើ (h != NULL) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} បើមិនដូច្នេះ {
		p = NULL;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ត្រឡប់ (p != NULL);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, const var_na *title)
{
	sa_nracanaa path_data *p;
	var_na *temp;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (p == NULL) {
		ត្រឡប់ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	ប្រសិនបើ (title != NULL) {
		temp = strdup(title);
		ប្រសិនបើ (temp == NULL) {
			ត្រឡប់ NSERROR_NOMEM;
		}
	} បើមិនដូច្នេះ {
		temp = NULL;
	}

	free(p->urld.title);
	p->urld.title = temp;

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p) {
		ត្រឡប់ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p) {
		ត្រឡប់ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(NULL);
	p->urld.visits++;

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
void urldb_reset_url_visit_data(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p)
		ត្រឡប់;

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
	ប្រសិនបើ (!p)
		ត្រឡប់ NULL;

	u = &p->urld;

	ត្រឡប់ (const sa_nracanaa url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	sa_nracanaa path_data *p;

	assert(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p)
		ត្រឡប់ NULL;

	ត្រឡប់ p->url;
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

	ប្រសិនបើ (!p)
		ត្រឡប់;

	/* Search for host_part */
	សម្រាប់ (pi = p; pi->parent != NULL; pi = pi->parent)
		;
	h = (sa_nracanaa host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	សម្រាប់ (space = h->prot_space; space; space = space->next) {
		ប្រសិនបើ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			bha_nga;
	}

	ប្រសិនបើ (space != NULL) {
		/* Overrule existing auth. */
		free(space->auth);
		space->auth = strdup(auth);
	} បើមិនដូច្នេះ {
		/* Create a new protection space. */
		space = space_alloc = malloc(parimaa_na(sa_nracanaa prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ប្រសិនបើ (!space_alloc || !realm_alloc || !auth_alloc) {
			free(space_alloc);
			free(realm_alloc);
			free(auth_alloc);
			ត្រឡប់;
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
	ប្រសិនបើ (!p)
		ត្រឡប់ NULL;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	សម្រាប់ (p_cur = p; p_cur != NULL; p_top = p_cur, p_cur = p_cur->parent) {
		ប្រសិនបើ (p_cur->prot_space) {
			ត្រឡប់ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ប្រសិនបើ (realm != NULL) {
		const sa_nracanaa host_part *h = (const sa_nracanaa host_part *)p_top;
		const sa_nracanaa prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		សម្រាប់ (space = h->prot_space; space != NULL;
		     space = space->next) {
			ប្រសិនបើ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				ត្រឡប់ p->prot_space->auth;
			}
		}
	}

	ត្រឡប់ NULL;
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
	ប្រសិនបើ (!p)
		ត្រឡប់;

	សម្រាប់ (; p && p->parent; p = p->parent)
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
	ប្រសិនបើ (!p)
		ត្រឡប់ false;

	សម្រាប់ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	ត្រឡប់ h->permit_invalid_certs;
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
	ប្រសិនបើ (host != NULL) {
		ប្រសិនបើ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ត្រឡប់ true;
		} បើមិនដូច្នេះ ប្រសិនបើ (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ត្រឡប់ true;
		}

		lwc_string_unref(host);
	} បើមិនដូច្នេះ {
		/* No host part: ignore */
		ត្រឡប់ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p)
		ត្រឡប់ false;

	សម្រាប់ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (sa_nracanaa host_part *)p;
	ប្រសិនបើ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ត្រឡប់ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	ប្រសិនបើ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ត្រឡប់ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ប្រសិនបើ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} បើមិនដូច្នេះ ប្រសិនបើ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ត្រឡប់ true;
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
	ប្រសិនបើ (host != NULL) {
		ប្រសិនបើ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ត្រឡប់ false;
		} បើមិនដូច្នេះ ប្រសិនបើ (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ត្រឡប់ false;
		}

		lwc_string_unref(host);
	} បើមិនដូច្នេះ {
		/* No host part: not enabled */
		ត្រឡប់ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ប្រសិនបើ (!p)
		ត្រឡប់ false;

	សម្រាប់ (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const sa_nracanaa host_part *)p;

	/* Consult record for this host */
	ប្រសិនបើ (h->hsts.expires > now) {
		/* Not expired */
		ត្រឡប់ true;
	}

	/* Consult parent domains */
	សម្រាប់ (h = h->parent; h && h != &db_root; h = h->parent) {
		ប្រសិនបើ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ត្រឡប់ true;
		}
	}

	ត្រឡប់ false;
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
	ប្រសិនបើ (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	ប្រសិនបើ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, parimaa_na host, "%.*s",
			 (ចំនួនគត់) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		ប្រសិនបើ (!h) {
			ចំនួនគត់ len = slash - prefix;

			ប្រសិនបើ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, parimaa_na buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				ប្រសិនបើ (!h)
					ត្រឡប់;
			} បើមិនដូច្នេះ
				ត្រឡប់;
		}

		ប្រសិនបើ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} បើមិនដូច្នេះ {
		ចំនួនគត់ len = strlen(prefix);

		/* looking for hosts */
		ប្រសិនបើ (!urldb_iterate_partial_host(tree, prefix, callback))
			ត្រឡប់;

		ប្រសិនបើ (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, parimaa_na buf, "www.%s", prefix);
			ប្រសិនបើ(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ត្រឡប់;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_entries(bool (*callback)(nsurl *url, const sa_nracanaa url_data *data))
{
	ចំនួនគត់ i;

	assert(callback);

	សម្រាប់ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ប្រសិនបើ (!urldb_iterate_entries_host(search_trees[i],
						callback,
						NULL)) {
			bha_nga;
		}
	}
}


/* exported interface documented in content/urldb.h */
void urldb_iterate_cookies(bool (*callback)(const sa_nracanaa cookie_data *data))
{
	ចំនួនគត់ i;

	assert(callback);

	សម្រាប់ (i = 0; i < NUM_SEARCH_TREES; i++) {
		ប្រសិនបើ (!urldb_iterate_entries_host(search_trees[i],
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
	ប្រសិនបើ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ត្រឡប់ NULL;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ប្រសិនបើ (scheme == NULL) {
		nsurl_unref(urlt);
		ត្រឡប់ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	ប្រសិនបើ (path == NULL) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ត្រឡប់ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ប្រសិនបើ (host == NULL) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ត្រឡប់ false;
	}

	ប្រសិនបើ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ប្រសិនបើ (rhost == NULL) {
			goto error;
		}

		/* Domain match host names */
		ប្រសិនបើ (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			const var_na *hptr;
			const var_na *rptr;
			const var_na *dot;
			const var_na *host_data = lwc_string_data(host);
			const var_na *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ប្រសិនបើ (urldb__host_is_ip_address(host_data) ||
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
			ខណៈពេល (hptr >= host_data && rptr >= rhost_data) {
				ប្រសិនបើ (*hptr != *rptr)
					bha_nga;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ខណៈពេល (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ប្រសិនបើ (*hptr == '\0' ||
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
		ប្រសិនបើ (!c) {
			/* failed => stop parsing */
			goto error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ប្រសិនបើ (!c->name || !c->value) {
			urldb_free_cookie(c);
			goto error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		ប្រសិនបើ (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			goto error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ប្រសិនបើ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ប្រសិនបើ (suffix == NULL) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			goto error;
		}
#បើមិនដូច្នេះ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ប្រសិនបើ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			goto error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		ប្រសិនបើ (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			ចំនួនគត់ hlen, dlen;
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
			ប្រសិនបើ (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				goto error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			ប្រសិនបើ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				goto error;
			}

			ប្រសិនបើ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ប្រសិនបើ (strcasecmp(lwc_string_data(host) + (hlen - dlen),
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
		ប្រសិនបើ (!urldb_insert_cookie(c, scheme, urlt))
			goto error;
	} ខណៈពេល (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ត្រឡប់ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ត្រឡប់ false;
}


/* exported interface documented in content/urldb.h */
var_na *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	const sa_nracanaa path_data *p, *q;
	const sa_nracanaa host_part *h;
	lwc_string *path_lwc;
	sa_nracanaa cookie_internal_data *c;
	ចំនួនគត់ count = 0, version = COOKIE_RFC2965;
	sa_nracanaa cookie_internal_data **matched_cookies;
	ចំនួនគត់ matched_cookies_size = 20;
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
	ប្រសិនបើ (!p)
		ត្រឡប់ NULL;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 parimaa_na(sa_nracanaa cookie_internal_data *));
	ប្រសិនបើ (!matched_cookies)
		ត្រឡប់ NULL;

#define GROW_MATCHED_COOKIES						\
	do {								\
		ប្រសិនបើ (count == matched_cookies_size) {			\
			sa_nracanaa cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       parimaa_na(sa_nracanaa cookie_internal_data *)); \
									\
			ប្រសិនបើ (temp == NULL) {				\
				free(matched_cookies);			\
				ត្រឡប់ NULL;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ខណៈពេល(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	ប្រសិនបើ (path_lwc == NULL) {
		free(matched_cookies);
		ត្រឡប់ NULL;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(NULL);

	ប្រសិនបើ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		សម្រាប់ (q = p->parent->children; q; q = q->next) {
			ប្រសិនបើ (strcmp(q->segment, p->segment))
				continue;

			/* Consider all cookies associated with
			 * this exact path */
			សម្រាប់ (c = q->cookies; c; c = c->next) {
				ប្រសិនបើ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ប្រសិនបើ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					continue;

				ប្រសិនបើ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ប្រសិនបើ (c->version < (unsigned ចំនួនគត់)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	សម្រាប់ (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		សម្រាប់ (q = p->children; q; q = q->next) {
			ប្រសិនបើ (*(q->segment) != '\0')
				continue;

			សម្រាប់ (c = q->cookies; c; c = c->next) {
				ប្រសិនបើ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				ប្រសិនបើ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				ប្រសិនបើ (c->version < (unsigned ចំនួនគត់) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((sa_nracanaa cookie_data *)c);
			}
		}

		ប្រសិនបើ (!p->parent) {
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
		សម្រាប់ (c = p->cookies; c; c = c->next) {
			ប្រសិនបើ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ប្រសិនបើ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ប្រសិនបើ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ប្រសិនបើ (c->version < (unsigned ចំនួនគត់) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	សម្រាប់ (h = (const sa_nracanaa host_part *)p; h && h != &db_root;
	     h = h->parent) {
		សម្រាប់ (c = h->paths.cookies; c; c = c->next) {
			ប្រសិនបើ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			ប្រសិនបើ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			ប្រសិនបើ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			ប្រសិនបើ (c->version < (unsigned ចំនួនគត់)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((sa_nracanaa cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	ប្រសិនបើ (count == 0) {
		/* No cookies found */
		free(matched_cookies);
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} បើមិនដូច្នេះ {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	free(matched_cookies);

	ត្រឡប់ ret;
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
	ប្រសិនបើ (!fp)
		ត្រឡប់ NSERROR_NOT_FOUND;

#define FIND_T {				\
		សម្រាប់ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ប្រសិនបើ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		សម្រាប់ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ប្រសិនបើ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
	}

	ខណៈពេល (fgets(s, parimaa_na s, fp)) {
		var_na *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		ចំនួនគត់ version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		sa_nracanaa cookie_internal_data *c;

		ប្រសិនបើ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			continue;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ប្រសិនបើ (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			ប្រសិនបើ (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				bha_nga;
			}

			continue;
		} បើមិនដូច្នេះ ប្រសិនបើ (loaded_cookie_file_version == 0) {
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
		ប្រសិនបើ (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} បើមិនដូច្នេះ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		ប្រសិនបើ (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} បើមិនដូច្នេះ {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		សម្រាប់ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		assert(p <= end);

		/* Now create cookie */
		c = malloc(parimaa_na(sa_nracanaa cookie_internal_data));
		ប្រសិនបើ (!c)
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

		ប្រសិនបើ (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			bha_nga;
		}

		ប្រសិនបើ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = NULL;
			nsurl *url_nsurl = NULL;

			assert(scheme[0] != 'u');

			ប្រសិនបើ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				bha_nga;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ប្រសិនបើ (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				bha_nga;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} បើមិនដូច្នេះ {
			ប្រសិនបើ (!urldb_insert_cookie(c, NULL, NULL)) {
				/* Cookie freed for us */
				bha_nga;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(const var_na *filename)
{
	FILE *fp;
	ចំនួនគត់ cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	assert(filename);

	fp = fopen(filename, "w");
	ប្រសិនបើ (!fp)
		ត្រឡប់ NSERROR_NOT_FOUND;

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

	ត្រឡប់ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_dump(void)
{
	ចំនួនគត់ i;

	urldb_dump_hosts(&db_root);

	សម្រាប់ (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
