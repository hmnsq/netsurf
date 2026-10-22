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
struct cookie_internal_data {
	struct cookie_internal_data *prev;	/**< Previous in list */
	struct cookie_internal_data *next;	/**< Next in list */

	char *name;		/**< Cookie name */
	char *value;		/**< Cookie value */
	bool value_was_quoted;	/**< Value was quoted in Set-Cookie: */
	char *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	char *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	char *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	enum cookie_version version;	/**< Specification compliance */
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
struct prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned số_nguyên port;
	/** Protection realm */
	char *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	char *auth;
	/** Next sibling */
	struct prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
struct url_internal_data {
	char *title;		/**< Resource title */
	unsigned số_nguyên visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
struct path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned số_nguyên port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	char *segment;		/**< Path segment for this node */
	unsigned số_nguyên frag_cnt;	/**< Number of entries in path_data::fragment */
	char **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	struct url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	const struct prot_space_data *prot_space;
	/** Cookies associated with resource */
	struct cookie_internal_data *cookies;
	/** Last cookie in list */
	struct cookie_internal_data *cookies_end;

	struct path_data *next;	/**< Next sibling */
	struct path_data *prev;	/**< Previous sibling */
	struct path_data *parent; /**< Parent path segment */
	struct path_data *children; /**< Child path segments */
	struct path_data *last; /**< Last child */
};

struct hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

struct host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	struct path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	struct hsts_data hsts;

	/**
	 * Part of host string
	 */
	char *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	struct prot_space_data *prot_space;

	struct host_part *next;	/**< Next sibling */
	struct host_part *prev;	/**< Previous sibling */
	struct host_part *parent; /**< Parent host part */
	struct host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
struct search_node {
	const struct host_part *data;	/**< Host tree entry */

	unsigned số_nguyên level;		/**< Node level */

	struct search_node *left;	/**< Left subtree */
	struct search_node *right;	/**< Right subtree */
};

/** Root database handle */
static struct host_part db_root;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define NUM_SEARCH_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
static struct search_node empty = { 0, 0, &empty, &empty };
static struct search_node *search_trees[NUM_SEARCH_TREES] = {
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
static số_nguyên loaded_cookie_file_version;

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
static struct bloom_filter *url_bloom;
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
	số_nguyên use;
	char op[32];

	use = nsc_sntimet(op, 32, &val);
	nếu (use == 0) {
		fprintf(fp, "%i\n", (số_nguyên)val);
	} không_thì {
		fprintf(fp, "%.*s\n", use, op);
	}
	trả_về NSERROR_OK;
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
urldb_write_paths(const struct path_data *parent,
		  const char *host,
		  FILE *fp,
		  char **path,
		  số_nguyên *path_alloc,
		  số_nguyên *path_used,
		  time_t expiry)
{
	const struct path_data *p = parent;
	số_nguyên i;

	do {
		số_nguyên seglen = p->segment != NULL ? strlen(p->segment) : 0;
		số_nguyên len = *path_used + seglen + 1;

		nếu (*path_alloc < len) {
			char *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			nếu (!temp) {
				trả_về;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		nếu (p->segment != NULL) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		nếu (p->children != NULL) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} không_thì {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		nếu (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} không_thì {
			/* leaf node */
			nếu (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				nếu (p->port) {
					fprintf(fp,"%d\n", p->port);
				} không_thì {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (số_nguyên)p->urld.type);

				fprintf(fp, "\n");

				nếu (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					cho (i = 0; s[i] != '\0'; i++)
						nếu (s[i] < 32)
							s[i] = ' ';
					cho (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} không_thì {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			trong_khi (p != parent) {
				số_nguyên seglen = p->segment != NULL
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				nếu (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					break;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} trong_khi (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
static void
urldb_count_urls(const struct path_data *root,
		 time_t expiry,
		 unsigned số_nguyên *count)
{
	const struct path_data *p = root;

	do {
		nếu (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} không_thì {
			/* No more children, increment count if required */
			nếu (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			trong_khi (p != root) {
				nếu (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					break;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} trong_khi (p != root);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
static void urldb_save_search_tree(struct search_node *parent, FILE *fp)
{
	char host[256];
	const struct host_part *h;
	unsigned số_nguyên path_count = 0;
	char *path, *p, *end;
	số_nguyên path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	số_nguyên hsts_include_subdomains = 0;

	expiry = time(NULL) - ((60 * 60 * 24) * nsoption_int(expire_url));

	nếu (parent == &empty)
		trả_về;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	nếu (!path)
		trả_về;

	path[0] = '\0';

	cho (h = parent->data, p = host, end = host + sizeof host;
	     h && h != &db_root && p < end; h = h->parent) {
		số_nguyên written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		nếu (written < 0) {
			free(path);
			trả_về;
		}
		p += written;
	}

	h = parent->data;
	nếu (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	nếu (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} không_thì nếu (hsts_expiry) {
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
urldb_iterate_entries_path(const struct path_data *parent,
		bool (*url_callback)(nsurl *url, const struct url_data *data),
		bool (*cookie_callback)(const struct cookie_data *data))
{
	const struct path_data *p = parent;
	const struct cookie_data *c;

	do {
		nếu (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} không_thì {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			assert(url_callback || cookie_callback);

			/** \todo handle fragments? */
			nếu (url_callback) {
				const struct url_internal_data *u = &p->urld;

				assert(p->url);

				nếu (!url_callback(p->url,
						  (const struct url_data *) u))
					trả_về false;
			} không_thì {
				c = (const struct cookie_data *)p->cookies;
				cho (; c != NULL; c = c->next) {
					nếu (!cookie_callback(c))
						trả_về false;
				}
			}

			/* Now, find next node to process. */
			trong_khi (p != parent) {
				nếu (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					break;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} trong_khi (p != parent);

	trả_về true;
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
static bool urldb__host_is_ip_address(const char *host)
{
	struct in_addr ipv4;
	size_t host_len = strlen(host);
	const char *sane_host;
	const char *slash;
#ifndef NO_IPV6
	struct in6_addr ipv6;
	char ipv6_addr[64];
	unsigned số_nguyên ipv6_addr_len;
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
	nếu (slash == NULL) {
		sane_host = host;
	} không_thì {
		char *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	nếu (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		goto out_false;

	nếu (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		số_nguyên num_dots = 0;
		size_t index;

		cho (index = 0; index < host_len; index++) {
			nếu (sane_host[index] == '.')
				num_dots++;
		}

		nếu (num_dots == 3)
			goto out_true;
		không_thì
			goto out_false;
	}

#ifndef NO_IPV6
	nếu ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		goto out_false;
	}

	ipv6_addr_len = host_len - 2;
	nếu (ipv6_addr_len >= sizeof(ipv6_addr)) {
		ipv6_addr_len = sizeof(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	nếu (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		goto out_true;
#endif

out_false:
	nếu (slash != NULL) free((void *)sane_host);
	trả_về false;

out_true:
	nếu (slash != NULL) free((void *)sane_host);
	trả_về true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
static số_nguyên urldb_search_match_prefix(const struct host_part *a, const char *b)
{
	const char *end, *dot;
	số_nguyên plen, ret;

	assert(a && a != &db_root && b);

	nếu (urldb__host_is_ip_address(b)) {
		/* IP address */
		trả_về strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	trong_khi (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		nếu (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		nếu ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			trả_về ret;

		/* The strings matched */
		nếu (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			nếu (plen > dot - b) {
				/* len(a) > len(b) */
				trả_về 1;
			} không_thì nếu (plen < dot - b) {
				/* len(a) < len(b) */
				trả_về -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	nếu (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		trả_về 0;
	} không_thì nếu ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		trả_về -1;
	}

	/* Identical */
	trả_về 0;
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
urldb_iterate_partial_host(struct search_node *root,
		const char *prefix,
		bool (*callback)(nsurl *url, const struct url_data *data))
{
	số_nguyên c;

	assert(root && prefix && callback);

	nếu (root == &empty)
		trả_về true;

	c = urldb_search_match_prefix(root->data, prefix);

	nếu (c > 0) {
		/* No match => look in left subtree */
		trả_về urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} không_thì nếu (c < 0) {
		/* No match => look in right subtree */
		trả_về urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} không_thì {
		/* Match => iterate over l/r subtrees & process this node */
		nếu (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			trả_về false;
		}

		nếu (root->data->paths.children) {
			/* and extract all paths attached to this host */
			nếu (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							NULL)) {
				trả_về false;
			}
		}

		nếu (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			trả_về false;
		}
	}

	trả_về true;
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
urldb_iterate_partial_path(const struct path_data *parent,
		const char *prefix,
		bool (*callback)(nsurl *url, const struct url_data *data))
{
	const struct path_data *p = parent->children;
	const char *slash, *end = prefix + strlen(prefix);

	do {
		slash = strchr(prefix, '/');
		nếu (!slash) {
			slash = end;
		}

		nếu (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			continue;
		}

		nếu (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			nếu (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				nếu (!urldb_iterate_entries_path(p,
								callback,
								NULL)) {
					trả_về false;
				}

				/* Progress to next sibling */
				p = p->next;
			} không_thì {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} không_thì {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} trong_khi (p != NULL);

	trả_về true;
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
urldb_iterate_entries_host(struct search_node *parent,
		bool (*url_callback)(nsurl *url, const struct url_data *data),
		bool (*cookie_callback)(const struct cookie_data *data))
{
	nếu (parent == &empty) {
		trả_về true;
	}

	nếu (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		trả_về false;
	}

	nếu ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		nếu (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			trả_về false;
		}
	}

	nếu (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		trả_về false;
	}

	trả_về true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
static struct host_part *
urldb_add_host_node(const char *part, struct host_part *parent)
{
	struct host_part *d;

	assert(part && parent);

	d = calloc(1, sizeof(struct host_part));
	nếu (!d) {
		trả_về NULL;
	}

	d->part = strdup(part);
	nếu (!d->part) {
		free(d);
		trả_về NULL;
	}

	d->next = parent->children;
	nếu (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	trả_về d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
static số_nguyên urldb_add_path_fragment_cmp(const void *a, const void *b)
{
	trả_về strcasecmp(*((const char **) a), *((const char **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
static struct path_data *
urldb_add_path_fragment(struct path_data *segment, lwc_string *fragment)
{
	char **temp;

	assert(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	nếu (!fragment)
		trả_về segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * sizeof(char *));
	nếu (!temp)
		trả_về NULL;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	nếu (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		trả_về NULL;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      sizeof (char *),
	      urldb_add_path_fragment_cmp);

	trả_về segment;
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
static struct path_data *
urldb_add_path_node(lwc_string *scheme,
		    unsigned số_nguyên port,
		    const char *segment,
		    lwc_string *fragment,
		    struct path_data *parent)
{
	struct path_data *d, *e;

	assert(scheme && segment && parent);

	d = calloc(1, sizeof(struct path_data));
	nếu (!d)
		trả_về NULL;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	nếu (!d->segment) {
		lwc_string_unref(d->scheme);
		free(d);
		trả_về NULL;
	}

	nếu (fragment) {
		nếu (!urldb_add_path_fragment(d, fragment)) {
			free(d->segment);
			lwc_string_unref(d->scheme);
			free(d);
			trả_về NULL;
		}
	}

	cho (e = parent->children; e; e = e->next) {
		nếu (strcmp(e->segment, d->segment) > 0)
			break;
	}

	nếu (e) {
		d->prev = e->prev;
		d->next = e;
		nếu (e->prev)
			e->prev->next = d;
		không_thì
			parent->children = d;
		e->prev = d;
	} không_thì nếu (!parent->children) {
		d->prev = d->next = NULL;
		parent->children = parent->last = d;
	} không_thì {
		d->next = NULL;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	trả_về d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static struct search_node **urldb_get_search_tree_direct(const char *host)
{
	assert(host);

	nếu (urldb__host_is_ip_address(host)) {
		trả_về &search_trees[ST_IP];
	} không_thì nếu (ascii_is_alpha(*host)) {
		trả_về &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	trả_về &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static struct search_node *urldb_get_search_tree(const char *host)
{
	trả_về *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
static số_nguyên urldb_search_match_string(const struct host_part *a, const char *b)
{
	const char *end, *dot;
	số_nguyên plen, ret;

	assert(a && a != &db_root && b);

	nếu (urldb__host_is_ip_address(b)) {
		/* IP address */
		trả_về strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	trong_khi (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		nếu (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		nếu ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			trả_về ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		nếu (plen > dot - b) {
			/* len(a) > len(b) */
			trả_về 1;
		} không_thì nếu (plen < dot - b) {
			/* len(a) < len(b) */
			trả_về -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	nếu (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		trả_về 1;
	} không_thì nếu ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		trả_về -1;
	}

	/* Identical */
	trả_về 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
static const struct host_part *
urldb_search_find(struct search_node *root, const char *host)
{
	số_nguyên c;

	assert(root && host);

	nếu (root == &empty) {
		trả_về NULL;
	}

	c = urldb_search_match_string(root->data, host);

	nếu (c > 0) {
		trả_về urldb_search_find(root->left, host);
	} không_thì nếu (c < 0) {
		trả_về urldb_search_find(root->right, host);
	}

	trả_về root->data;
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
static struct path_data *
urldb_match_path(const struct path_data *parent,
		 const char *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	const struct path_data *p;
	const char *slash;
	bool match;

	assert(parent != NULL);
	assert(parent->segment == NULL);

	nếu (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	assert(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	trong_khi (p != NULL) {
		slash = strchr(path + 1, '/');
		nếu (!slash) {
			slash = path + strlen(path);
		}

		nếu (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			nếu (*slash == '\0') {
				/* Complete match */
				trả_về (struct path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} không_thì {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	trả_về NULL;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
static struct path_data *urldb_find_url(nsurl *url)
{
	const struct host_part *h;
	struct path_data *p;
	struct search_node *tree;
	char *plq;
	const char *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned số_nguyên port_int;
	bool match;

	assert(url);

	nếu (url_bloom != NULL) {
		nếu (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			trả_về NULL;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	nếu (scheme == NULL)
		trả_về NULL;

	nếu (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		trả_về NULL;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	nếu (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} không_thì nếu (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} không_thì {
		lwc_string_unref(scheme);
		trả_về NULL;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	nếu (!h) {
		lwc_string_unref(scheme);
		trả_về NULL;
	}

	/* generate plq (path, leaf, query) */
	nếu (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		trả_về NULL;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	nếu (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} không_thì {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	free(plq);
	lwc_string_unref(scheme);

	trả_về p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_paths(struct path_data *parent)
{
	const struct path_data *p = parent;
	unsigned số_nguyên i;

	do {
		nếu (p->segment != NULL) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			cho (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		nếu (p->children != NULL) {
			p = p->children;
		} không_thì {
			trong_khi (p != parent) {
				nếu (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} trong_khi (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_hosts(struct host_part *parent)
{
	struct host_part *h;

	nếu (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	cho (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
static void urldb_dump_search(struct search_node *parent, số_nguyên depth)
{
	const struct host_part *h;
	số_nguyên i; /* index into string */
	char s[1024];
	số_nguyên r;
	số_nguyên sl = sizeof(s) - 2;

	nếu (parent == &empty)
		trả_về;

	urldb_dump_search(parent->left, depth + 1);

	cho (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	cho (h = parent->data; h; h = h->parent) {
		nếu (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			nếu (r < 0) {
				break;
			}
			nếu ((i + r) >= sl) {
				break;
			}
			i += r;
		}

		nếu (h->parent && h->parent->parent) {
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
static số_nguyên
urldb_search_match_host(const struct host_part *a, const struct host_part *b)
{
	số_nguyên ret;

	assert(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	cho (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		nếu ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			trả_về ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	nếu (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		trả_về 1;
	} không_thì nếu ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		trả_về -1;
	}

	/* identical */
	trả_về 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
static struct search_node *urldb_search_skew(struct search_node *root)
{
	assert(root);

	nếu (root->left->level == root->level) {
		struct search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	trả_về root;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
static struct search_node *urldb_search_split(struct search_node *root)
{
	assert(root);

	nếu (root->right->right->level == root->level) {
		struct search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	trả_về root;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
static struct search_node *
urldb_search_insert_internal(struct search_node *root, struct search_node *n)
{
	assert(root && n);

	nếu (root == &empty) {
		root = n;
	} không_thì {
		số_nguyên c = urldb_search_match_host(root->data, n->data);

		nếu (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} không_thì nếu (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} không_thì {
			/* exact match */
			free(n);
			trả_về root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	trả_về root;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
static struct search_node *
urldb_search_insert(struct search_node *root, const struct host_part *data)
{
	struct search_node *n;

	assert(root && data);

	n = malloc(sizeof(struct search_node));
	nếu (!n)
		trả_về NULL;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	trả_về root;
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
urldb_parse_avpair(struct cookie_internal_data *c,
		   char *n,
		   char *v,
		   bool was_quoted)
{
	số_nguyên vlen;

	assert(c && n && v);

	/* Strip whitespace from start of name */
	cho (; *n; n++) {
		nếu (*n != ' ' && *n != '\t')
			break;
	}

	/* Strip whitespace from end of name */
	cho (vlen = strlen(n); vlen; vlen--) {
		nếu (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		không_thì
			break;
	}

	/* Strip whitespace from start of value */
	cho (; *v; v++) {
		nếu (*v != ' ' && *v != '\t')
			break;
	}

	/* Strip whitespace from end of value */
	cho (vlen = strlen(v); vlen; vlen--) {
		nếu (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		không_thì
			break;
	}

	nếu (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		nếu (!c->comment)
			trả_về false;
	} không_thì nếu (!c->domain && strcasecmp(n, "Domain") == 0) {
		nếu (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			nếu (!c->domain)
				trả_về false;
		}
	} không_thì nếu (strcasecmp(n, "Max-Age") == 0) {
		số_nguyên temp = atoi(v);
		nếu (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		không_thì
			c->expires = time(NULL) + temp;
	} không_thì nếu (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		nếu (!c->path)
			trả_về false;
	} không_thì nếu (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} không_thì nếu (strcasecmp(n, "Expires") == 0) {
		char *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		cho (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		nếu (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} không_thì nếu (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} không_thì nếu (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} không_thì nếu (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		nếu (!c->name || !c->value) {
			trả_về false;
		}
	}

	trả_về true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
static void urldb_free_cookie(struct cookie_internal_data *c)
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
static struct cookie_internal_data *
urldb_parse_cookie(nsurl *url, const char **cookie)
{
	struct cookie_internal_data *c;
	const char *cur;
	char name[1024], value[4096];
	char *n = name, *v = value;
	bool in_value = false;
	bool had_value_data = false;
	bool value_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	assert(url && cookie && *cookie);

	c = calloc(1, sizeof(struct cookie_internal_data));
	nếu (c == NULL)
		trả_về NULL;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	cho (cur = *cookie; *cur; cur++) {
		nếu (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			nếu (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			break;
		} không_thì nếu (*cur == '\r') {
			/* Spurious linefeed */
			continue;
		} không_thì nếu (*cur == '\n') {
			/* Spurious newline */
			continue;
		}

		nếu (in_value && !had_value_data) {
			nếu (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				continue;
			} không_thì {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				nếu (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		nếu (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			nếu (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				continue;
			}
		}

		nếu (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			continue;
		}

		nếu (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			nếu (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				trả_về NULL;
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
			nếu (*cur != ';') {
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
		nếu (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			const char *p;
			const char *semi = strchr(cur + 1, ';');
			nếu (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			cho (p = cur + 1; p < semi; p++)
				nếu (*p == '=')
					break;

			nếu (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} không_thì {
				/* found one => comma marks end of cookie */
				cur++;
				break;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		nếu (!in_value) {
			nếu (n < name + (sizeof(name) - 1))
				*n++ = *cur;
		} không_thì {
			nếu (v < value + (sizeof(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	nếu (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		trả_về NULL;
	}

	/* Now fix-up default values */
	nếu (c->domain == NULL) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		nếu (host == NULL) {
			urldb_free_cookie(c);
			trả_về NULL;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	nếu (c->path == NULL) {
		const char *path_data;
		char *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		nếu (path_lwc == NULL) {
			urldb_free_cookie(c);
			trả_về NULL;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		nếu (slash != NULL) {
			/* Special case: retain first slash in path */
			nếu (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			nếu (slash == NULL) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				trả_về NULL;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} không_thì {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			nếu (path == NULL) {
				urldb_free_cookie(c);
				trả_về NULL;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	trả_về c;
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
static struct path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned số_nguyên port,
	       const struct host_part *host,
	       char *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	struct path_data *d, *e;
	char *buf = path_query;
	char *segment, *slash;
	bool match;

	assert(scheme && host && url);

	d = (struct path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	nếu (*segment == '/')
		segment++;

	/* Process path segments */
	do {
		slash = strchr(segment, '/');
		nếu (!slash) {
			/* last segment */
			/* look for existing entry */
			cho (e = d->children; e; e = e->next)
				nếu (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					break;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_node(scheme, port,
						    segment, fragment, d);
			break;
		}

		*slash = '\0';

		/* look for existing entry */
		cho (e = d->children; e; e = e->next)
			nếu (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				break;

		d = e ? e : urldb_add_path_node(scheme, port, segment, NULL, d);
		nếu (!d)
			break;

		segment = slash + 1;
	} trong_khi (1);

	free(path_query);

	nếu (d && !d->url) {
		/* Insert defragmented URL */
		nếu (nsurl_defragment(url, &d->url) != NSERROR_OK)
			trả_về NULL;
	}

	trả_về d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
static struct host_part *urldb_add_host(const char *host)
{
	struct host_part *d = (struct host_part *) &db_root, *e;
	struct search_node *s;
	char buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	char *part;

	assert(host);

	nếu (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		cho (e = d->children; e; e = e->next)
			nếu (strcasecmp(host, e->part) == 0)
				/* found => return it */
				trả_về e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		nếu (!s) {
			/* failed */
			d = NULL;
		} không_thì {
			search_trees[ST_IP] = s;
		}

		trả_về d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, sizeof buf);
	buf[sizeof buf - 1] = '\0';

	/* Process FQDN segments backwards */
	do {
		part = strrchr(buf, '.');
		nếu (!part) {
			/* last segment */
			/* Check for existing entry */
			cho (e = d->children; e; e = e->next)
				nếu (strcasecmp(buf, e->part) == 0)
					break;

			nếu (e) {
				d = e;
			} không_thì {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			nếu (d) {
				struct search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				nếu (!s) {
					/* failed */
					d = NULL;
				} không_thì {
					*r = s;
				}
			}
			break;
		}

		/* Check for existing entry */
		cho (e = d->children; e; e = e->next)
			nếu (strcasecmp(part + 1, e->part) == 0)
				break;

		d = e ? e : urldb_add_host_node(part + 1, d);
		nếu (!d)
			break;

		*part = '\0';
	} trong_khi (1);

	trả_về d;
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
urldb_insert_cookie(struct cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	struct cookie_internal_data *d;
	const struct host_part *h;
	struct path_data *p;
	time_t now = time(NULL);

	assert(c);

	nếu (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		nếu (!h) {
			h = urldb_add_host(c->domain + 1);
			nếu (!h) {
				urldb_free_cookie(c);
				trả_về false;
			}
		}

		p = (struct path_data *) &h->paths;
	} không_thì {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		assert(url != NULL);
		assert(scheme != NULL);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		nếu (!h) {
			h = urldb_add_host(c->domain);
			nếu (!h) {
				urldb_free_cookie(c);
				trả_về false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), NULL, url);
		nếu (!p) {
			urldb_free_cookie(c);
			trả_về false;
		}
	}

	/* add cookie */
	cho (d = p->cookies; d; d = d->next) {
		nếu (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			break;
	}

	nếu (d) {
		nếu (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			nếu (d->next)
				d->next->prev = d->prev;
			không_thì
				p->cookies_end = d->prev;
			nếu (d->prev)
				d->prev->next = d->next;
			không_thì
				p->cookies = d->next;

			cookie_manager_remove((struct cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} không_thì {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			nếu (c->next)
				c->next->prev = c;
			không_thì
				p->cookies_end = c;
			nếu (c->prev)
				c->prev->next = c;
			không_thì
				p->cookies = c;

			cookie_manager_remove((struct cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((struct cookie_data *)c);
		}
	} không_thì {
		c->prev = p->cookies_end;
		c->next = NULL;
		nếu (p->cookies_end)
			p->cookies_end->next = c;
		không_thì
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((struct cookie_data *)c);
	}

	trả_về true;
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
concat_netscape_cookie(struct cookie_internal_data *c,
		    số_nguyên *used,
		    số_nguyên *alloc,
		    char **buf)
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
	số_nguyên max_len;
	số_nguyên outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	nếu (max_len >= *alloc) {
		char *temp = realloc(*buf, max_len);
		nếu (!temp) {
			trả_về false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	nếu (outl >= (*alloc - *used)) {
		/* output truncated */
		trả_về false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	nếu (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		nếu (outl >= (*alloc - *used)) {
			/* output truncated */
			trả_về false;
		}
		*used += outl;
	} không_thì {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		nếu (outl >= (*alloc - *used)) {
			/* output truncated */
			trả_về false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	trả_về true;
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
concat_rfc2965_cookie(struct cookie_internal_data *c,
		      số_nguyên *used,
		      số_nguyên *alloc,
		      char **buf)
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
	const char * const separators = "()<>@,;:\\\"/[]?={} \t";
	số_nguyên max_len;
	số_nguyên outl; /* output length */

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

	nếu (max_len >= *alloc) {
		char *temp = realloc(*buf, max_len);
		nếu (!temp) {
			trả_về false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	nếu (outl >= (*alloc - *used)) {
		/* output truncated */
		trả_về false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	nếu (c->value_was_quoted ||
	    strpbrk(c->value, separators) != NULL) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} không_thì {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	nếu (outl >= (*alloc - *used)) {
		/* output truncated */
		trả_về false;
	}
	*used += outl;

	nếu (c->path_from_set) {
		/* Path, quoted if necessary */

		nếu (strpbrk(c->path, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} không_thì {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		nếu (outl >= (*alloc - *used)) {
			/* output truncated */
			trả_về false;
		}
		*used += outl;
	}

	nếu (c->domain_from_set) {
		/* Domain, quoted if necessary */
		nếu (strpbrk(c->domain, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} không_thì {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		nếu (outl >= (*alloc - *used)) {
			/* output truncated */
			trả_về false;
		}
		*used += outl;
	}

	trả_về true;
}


/**
 * build output cookie string in netscape format
 */
static char *
get_netscape_cookie(struct cookie_internal_data **matched_cookies, số_nguyên count)
{
	số_nguyên ret_alloc = 4096, ret_used = 1;
	char *ret;
	số_nguyên midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	nếu (ret == NULL) {
		trả_về NULL;
	}
	ret[0] = '\0';

	cho (midx = 0; midx < count; midx++) {
		nếu (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			free(ret);
			trả_về NULL;
		}
	}

	nếu (ret_used <= 2) {
		free(ret);
		trả_về NULL;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	char *temp = realloc(ret, ret_used);
	nếu (temp != NULL) {
		ret = temp;
	}

	trả_về ret;
}


/**
 * build output cookie string in RFC2965 format
 */
static char *
get_rfc2965_cookie(struct cookie_internal_data **matched_cookies,
		   số_nguyên count,
		   số_nguyên version)
{
	số_nguyên ret_alloc = 4096; /* return string allocation */
	số_nguyên ret_used = 1; /* return sting buffer used */
	char *ret; /* return string buffer */
	số_nguyên midx; /* index into matched cookies */
	số_nguyên outl;

	ret = malloc(ret_alloc);
	nếu (ret == NULL) {
		trả_về NULL;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	nếu (outl >= ret_alloc - ret_used) {
		/* overflow */
		free(ret);
		trả_về NULL;
	}
	ret_used += outl;

	cho (midx = 0; midx < count; midx++) {
		nếu (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			free(ret);
			trả_về NULL;
		}
	}

	/* shrink the output buffer to the required size */
	char *temp = realloc(ret, ret_used);
	nếu (temp != NULL) {
		ret = temp;
	}

	trả_về ret;
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
urldb_delete_cookie_paths(const char *domain,
			  const char *path,
			  const char *name,
			  struct path_data *parent)
{
	struct cookie_internal_data *c;
	struct path_data *p = parent;

	assert(parent);

	do {
		cho (c = p->cookies; c; c = c->next) {
			nếu (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				nếu (c->prev) {
					c->prev->next = c->next;
				} không_thì {
					p->cookies = c->next;
				}

				nếu (c->next) {
					c->next->prev = c->prev;
				} không_thì {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				trả_về;
			}
		}

		nếu (p->children) {
			p = p->children;
		} không_thì {
			trong_khi (p != parent) {
				nếu (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} trong_khi (p != parent);
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
urldb_delete_cookie_hosts(const char *domain,
			  const char *path,
			  const char *name,
			  struct host_part *parent)
{
	struct host_part *h;
	assert(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	cho (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
static void urldb_save_cookie_paths(FILE *fp, struct path_data *parent)
{
	struct path_data *p = parent;
	time_t now = time(NULL);

	assert(fp && parent);

	do {
		nếu (p->cookies != NULL) {
			struct cookie_internal_data *c;

			cho (c = p->cookies; c != NULL; c = c->next) {
				nếu (c->expires == -1 || c->expires < now) {
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
					(số_nguyên)c->expires, (số_nguyên)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		nếu (p->children != NULL) {
			p = p->children;
		} không_thì {
			trong_khi (p != parent) {
				nếu (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} trong_khi (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
static void urldb_save_cookie_hosts(FILE *fp, struct host_part *parent)
{
	struct host_part *h;
	assert(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	cho (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
static void urldb_destroy_cookie(struct cookie_internal_data *c)
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
static void urldb_destroy_path_node_content(struct path_data *node)
{
	struct cookie_internal_data *a, *b;
	unsigned số_nguyên i;

	nếu (node->url != NULL) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	free(node->segment);
	cho (i = 0; i < node->frag_cnt; i++)
		free(node->fragment[i]);
	free(node->fragment);

	free(node->urld.title);

	cho (a = node->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
static void urldb_destroy_prot_space(struct prot_space_data *space)
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
static void urldb_destroy_path_tree(struct path_data *root)
{
	struct path_data *p = root;

	do {
		nếu (p->children != NULL) {
			p = p->children;
		} không_thì {
			struct path_data *q = p;

			trong_khi (p != root) {
				nếu (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;

				urldb_destroy_path_node_content(q);
				free(q);

				q = p;
			}

			urldb_destroy_path_node_content(q);
			free(q);
		}
	} trong_khi (p != root);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
static void urldb_destroy_host_tree(struct host_part *root)
{
	struct host_part *a, *b;
	struct path_data *p, *q;
	struct prot_space_data *s, *t;

	/* Destroy children */
	cho (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	cho (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	cho (s = root->prot_space; s; s = t) {
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
static void urldb_destroy_search_tree(struct search_node *root)
{
	/* Destroy children */
	nếu (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	nếu (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	free(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
void urldb_destroy(void)
{
	struct host_part *a, *b;
	số_nguyên i;

	/* Clean up search trees */
	cho (i = 0; i < NUM_SEARCH_TREES; i++) {
		nếu (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	cho (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, sizeof(db_root));

	/* And the bloom filter */
	nếu (url_bloom != NULL) {
		bloom_destroy(url_bloom);
		url_bloom = NULL;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(const char *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	char s[MAXIMUM_URL_LENGTH];
	char host[256];
	struct host_part *h;
	số_nguyên urls;
	số_nguyên i;
	số_nguyên version;
	số_nguyên length;
	FILE *fp;

	assert(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	nếu (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	nếu (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		trả_về NSERROR_NOT_FOUND;
	}

	nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		trả_về NSERROR_NEED_DATA;
	}

	version = atoi(s);
	nếu (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		trả_về NSERROR_INVALID;
	}
	nếu (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		trả_về NSERROR_INVALID;
	}

	trong_khi (fgets(host, sizeof host, fp)) {
		time_t hsts_expiry = 0;
		số_nguyên hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		nếu (length == 0) {
			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			urls = atoi(s);
			/* Eight fields/url */
			cho (i = 0; i < (8 * urls); i++) {
				nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					break;
			}
			continue;
		}

		nếu (version >= 107) {
			char *p = host;
			trong_khi (*p && *p != ' ') p++;
			trong_khi (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			trong_khi (*p && *p != ' ') p++;
			trong_khi (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		nếu (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			trả_về NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			break;
		urls = atoi(s);

		/* no URLs => try next host */
		nếu (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			continue;
		}

		/* load the non-corrupt data */
		cho (i = 0; i < urls; i++) {
			struct path_data *p = NULL;
			char scheme[64], ports[10];
			char url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned số_nguyên port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			char *path_query;
			size_t len;

			nếu (!fgets(scheme, sizeof scheme, fp))
				break;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			nếu (!fgets(ports, sizeof ports, fp))
				break;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			length = strlen(s) - 1;
			s[length] = '\0';

			nếu (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, sizeof url, "%s://%s%s%s%s",
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
			nếu (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				trả_về NSERROR_NOMEM;
			}

			nếu (url_bloom != NULL) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			nếu (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				trả_về NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			nếu (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				trả_về NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			nếu (p)
				p->urld.visits = (unsigned số_nguyên)atoi(s);

			/* entry last use time */
			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				break;
			}
			nếu (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			nếu (p)
				p->urld.type = (content_type)atoi(s);

			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;


			nếu (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			length = strlen(s) - 1;
			nếu (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				nếu (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	trả_về NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(const char *filename)
{
	FILE *fp;
	số_nguyên i;

	assert(filename);

	fp = fopen(filename, "w");
	nếu (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		trả_về NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	cho (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	trả_về NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p) {
		trả_về NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	trả_về NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	struct host_part *h;
	struct path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	const char *host_str;
	char *path_query = NULL;
	size_t len;
	bool match;
	unsigned số_nguyên port_int;

	assert(url);

	nếu (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	nếu (url_bloom != NULL) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	nếu (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		trả_về false;
	}
	assert(path_query != NULL);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	nếu (scheme == NULL) {
		free(path_query);
		trả_về false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	nếu (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} không_thì nếu (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} không_thì {
		lwc_string_unref(scheme);
		free(path_query);
		trả_về false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	nếu (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} không_thì {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	nếu (h != NULL) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} không_thì {
		p = NULL;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	trả_về (p != NULL);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, const char *title)
{
	struct path_data *p;
	char *temp;

	assert(url);

	p = urldb_find_url(url);
	nếu (p == NULL) {
		trả_về NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	nếu (title != NULL) {
		temp = strdup(title);
		nếu (temp == NULL) {
			trả_về NSERROR_NOMEM;
		}
	} không_thì {
		temp = NULL;
	}

	free(p->urld.title);
	p->urld.title = temp;

	trả_về NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p) {
		trả_về NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	trả_về NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p) {
		trả_về NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(NULL);
	p->urld.visits++;

	trả_về NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
void urldb_reset_url_visit_data(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
const struct url_data *urldb_get_url_data(nsurl *url)
{
	struct path_data *p;
	struct url_internal_data *u;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về NULL;

	u = &p->urld;

	trả_về (const struct url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về NULL;

	trả_về p->url;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_set_auth_details(nsurl *url, const char *realm, const char *auth)
{
	struct path_data *p, *pi;
	struct host_part *h;
	struct prot_space_data *space, *space_alloc;
	char *realm_alloc, *auth_alloc;
	bool match;

	assert(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	nếu (!p)
		trả_về;

	/* Search for host_part */
	cho (pi = p; pi->parent != NULL; pi = pi->parent)
		;
	h = (struct host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	cho (space = h->prot_space; space; space = space->next) {
		nếu (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			break;
	}

	nếu (space != NULL) {
		/* Overrule existing auth. */
		free(space->auth);
		space->auth = strdup(auth);
	} không_thì {
		/* Create a new protection space. */
		space = space_alloc = malloc(sizeof(struct prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		nếu (!space_alloc || !realm_alloc || !auth_alloc) {
			free(space_alloc);
			free(realm_alloc);
			free(auth_alloc);
			trả_về;
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
const char *urldb_get_auth_details(nsurl *url, const char *realm)
{
	struct path_data *p, *p_cur, *p_top;

	assert(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về NULL;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	cho (p_cur = p; p_cur != NULL; p_top = p_cur, p_cur = p_cur->parent) {
		nếu (p_cur->prot_space) {
			trả_về p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	nếu (realm != NULL) {
		const struct host_part *h = (const struct host_part *)p_top;
		const struct prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		cho (space = h->prot_space; space != NULL;
		     space = space->next) {
			nếu (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				trả_về p->prot_space->auth;
			}
		}
	}

	trả_về NULL;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_set_cert_permissions(nsurl *url, bool permit)
{
	struct path_data *p;
	struct host_part *h;

	assert(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về;

	cho (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (struct host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	struct path_data *p;
	const struct host_part *h;

	assert(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về false;

	cho (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const struct host_part *)p;

	trả_về h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(struct nsurl *url, const char *header)
{
	struct path_data *p;
	struct host_part *h;
	lwc_string *host;
	time_t now = time(NULL);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	assert(url);

	host = nsurl_get_component(url, NSURL_HOST);
	nếu (host != NULL) {
		nếu (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			trả_về true;
		} không_thì nếu (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			trả_về true;
		}

		lwc_string_unref(host);
	} không_thì {
		/* No host part: ignore */
		trả_về true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về false;

	cho (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (struct host_part *)p;
	nếu (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		trả_về true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	nếu (error != NSERROR_OK) {
		/* Parse failed: ignore */
		trả_về true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	nếu (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} không_thì nếu ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	trả_về true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(struct nsurl *url)
{
	struct path_data *p;
	const struct host_part *h;
	lwc_string *host;
	time_t now = time(NULL);

	assert(url);

	host = nsurl_get_component(url, NSURL_HOST);
	nếu (host != NULL) {
		nếu (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			trả_về false;
		} không_thì nếu (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			trả_về false;
		}

		lwc_string_unref(host);
	} không_thì {
		/* No host part: not enabled */
		trả_về false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về false;

	cho (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const struct host_part *)p;

	/* Consult record for this host */
	nếu (h->hsts.expires > now) {
		/* Not expired */
		trả_về true;
	}

	/* Consult parent domains */
	cho (h = h->parent; h && h != &db_root; h = h->parent) {
		nếu (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			trả_về true;
		}
	}

	trả_về false;
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_partial(const char *prefix,
		      bool (*callback)(nsurl *url, const struct url_data *data))
{
	char host[256];
	char buf[260]; /* max domain + "www." */
	const char *slash, *scheme_sep;
	struct search_node *tree;
	const struct host_part *h;

	assert(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	nếu (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	nếu (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, sizeof host, "%.*s",
			 (số_nguyên) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		nếu (!h) {
			số_nguyên len = slash - prefix;

			nếu (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, sizeof buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				nếu (!h)
					trả_về;
			} không_thì
				trả_về;
		}

		nếu (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} không_thì {
		số_nguyên len = strlen(prefix);

		/* looking for hosts */
		nếu (!urldb_iterate_partial_host(tree, prefix, callback))
			trả_về;

		nếu (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, sizeof buf, "www.%s", prefix);
			nếu(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				trả_về;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_entries(bool (*callback)(nsurl *url, const struct url_data *data))
{
	số_nguyên i;

	assert(callback);

	cho (i = 0; i < NUM_SEARCH_TREES; i++) {
		nếu (!urldb_iterate_entries_host(search_trees[i],
						callback,
						NULL)) {
			break;
		}
	}
}


/* exported interface documented in content/urldb.h */
void urldb_iterate_cookies(bool (*callback)(const struct cookie_data *data))
{
	số_nguyên i;

	assert(callback);

	cho (i = 0; i < NUM_SEARCH_TREES; i++) {
		nếu (!urldb_iterate_entries_host(search_trees[i],
						NULL, callback))
			break;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(const char *header, nsurl *url, nsurl *referer)
{
	const char *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	assert(url && header);

	/* Get defragmented URL, as 'urlt' */
	nếu (nsurl_defragment(url, &urlt) != NSERROR_OK)
		trả_về NULL;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	nếu (scheme == NULL) {
		nsurl_unref(urlt);
		trả_về false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	nếu (path == NULL) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		trả_về false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	nếu (host == NULL) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		trả_về false;
	}

	nếu (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		nếu (rhost == NULL) {
			goto error;
		}

		/* Domain match host names */
		nếu (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			const char *hptr;
			const char *rptr;
			const char *dot;
			const char *host_data = lwc_string_data(host);
			const char *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			nếu (urldb__host_is_ip_address(host_data) ||
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
			trong_khi (hptr >= host_data && rptr >= rhost_data) {
				nếu (*hptr != *rptr)
					break;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			trong_khi (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			nếu (*hptr == '\0' ||
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
		struct cookie_internal_data *c;
		char *dot;
		size_t len;
#ifdef WITH_NSPSL
		const char *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		nếu (!c) {
			/* failed => stop parsing */
			goto error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		nếu (!c->name || !c->value) {
			urldb_free_cookie(c);
			goto error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		nếu (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			goto error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		nếu (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		nếu (suffix == NULL) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			goto error;
		}
#không_thì
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		nếu (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			goto error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		nếu (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			số_nguyên hlen, dlen;
			char *domain = c->domain;

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
			nếu (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				goto error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			nếu (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				goto error;
			}

			nếu (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			nếu (strcasecmp(lwc_string_data(host) + (hlen - dlen),
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
		nếu (!urldb_insert_cookie(c, scheme, urlt))
			goto error;
	} trong_khi (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	trả_về true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	trả_về false;
}


/* exported interface documented in content/urldb.h */
char *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	const struct path_data *p, *q;
	const struct host_part *h;
	lwc_string *path_lwc;
	struct cookie_internal_data *c;
	số_nguyên count = 0, version = COOKIE_RFC2965;
	struct cookie_internal_data **matched_cookies;
	số_nguyên matched_cookies_size = 20;
	const char *path;
	char *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	assert(url != NULL);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	nếu (!p)
		trả_về NULL;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 sizeof(struct cookie_internal_data *));
	nếu (!matched_cookies)
		trả_về NULL;

#define GROW_MATCHED_COOKIES						\
	do {								\
		nếu (count == matched_cookies_size) {			\
			struct cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       sizeof(struct cookie_internal_data *)); \
									\
			nếu (temp == NULL) {				\
				free(matched_cookies);			\
				trả_về NULL;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} trong_khi(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	nếu (path_lwc == NULL) {
		free(matched_cookies);
		trả_về NULL;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(NULL);

	nếu (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		cho (q = p->parent->children; q; q = q->next) {
			nếu (strcmp(q->segment, p->segment))
				continue;

			/* Consider all cookies associated with
			 * this exact path */
			cho (c = q->cookies; c; c = c->next) {
				nếu (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				nếu (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					continue;

				nếu (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				nếu (c->version < (unsigned số_nguyên)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((struct cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	cho (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		cho (q = p->children; q; q = q->next) {
			nếu (*(q->segment) != '\0')
				continue;

			cho (c = q->cookies; c; c = c->next) {
				nếu (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				nếu (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				nếu (c->version < (unsigned số_nguyên) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((struct cookie_data *)c);
			}
		}

		nếu (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			break;
		}

		/* Consider p itself - may be the result of Path=/foo */
		cho (c = p->cookies; c; c = c->next) {
			nếu (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			nếu (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			nếu (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			nếu (c->version < (unsigned số_nguyên) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((struct cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	cho (h = (const struct host_part *)p; h && h != &db_root;
	     h = h->parent) {
		cho (c = h->paths.cookies; c; c = c->next) {
			nếu (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			nếu (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			nếu (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			nếu (c->version < (unsigned số_nguyên)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((struct cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	nếu (count == 0) {
		/* No cookies found */
		free(matched_cookies);
		trả_về NULL;
	}

	nếu (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} không_thì {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	free(matched_cookies);

	trả_về ret;
}


/* exported interface documented in content/urldb.h */
void urldb_delete_cookie(const char *domain, const char *path,
			 const char *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_root);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(const char *filename)
{
	FILE *fp;
	char s[16*1024];

	assert(filename);

	fp = fopen(filename, "r");
	nếu (!fp)
		trả_về NSERROR_NOT_FOUND;

#define FIND_T {				\
		cho (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		nếu (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		cho (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		nếu (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
	}

	trong_khi (fgets(s, sizeof s, fp)) {
		char *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		số_nguyên version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		struct cookie_internal_data *c;

		nếu(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			continue;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		nếu (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			nếu (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				break;
			}

			continue;
		} không_thì nếu (loaded_cookie_file_version == 0) {
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
		nếu (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} không_thì {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		nếu (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} không_thì {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		cho (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		assert(p <= end);

		/* Now create cookie */
		c = malloc(sizeof(struct cookie_internal_data));
		nếu (!c)
			break;

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

		nếu (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			break;
		}

		nếu (c->domain[0] != '.') {
			lwc_string *scheme_lwc = NULL;
			nsurl *url_nsurl = NULL;

			assert(scheme[0] != 'u');

			nếu (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				break;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			nếu (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				break;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} không_thì {
			nếu (!urldb_insert_cookie(c, NULL, NULL)) {
				/* Cookie freed for us */
				break;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	trả_về NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(const char *filename)
{
	FILE *fp;
	số_nguyên cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	assert(filename);

	fp = fopen(filename, "w");
	nếu (!fp)
		trả_về NSERROR_NOT_FOUND;

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

	trả_về NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_dump(void)
{
	số_nguyên i;

	urldb_dump_hosts(&db_root);

	cho (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
