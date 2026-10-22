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
	unsigned 整数 port;
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
	unsigned 整数 visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
struct path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned 整数 port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	char *segment;		/**< Path segment for this node */
	unsigned 整数 frag_cnt;	/**< Number of entries in path_data::fragment */
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

	unsigned 整数 level;		/**< Node level */

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
static 整数 loaded_cookie_file_version;

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
	整数 use;
	char op[32];

	use = nsc_sntimet(op, 32, &val);
	如果 (use == 0) {
		fprintf(fp, "%i\n", (整数)val);
	} 否则 {
		fprintf(fp, "%.*s\n", use, op);
	}
	返回 NSERROR_OK;
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
		  整数 *path_alloc,
		  整数 *path_used,
		  time_t expiry)
{
	const struct path_data *p = parent;
	整数 i;

	do {
		整数 seglen = p->segment != NULL ? strlen(p->segment) : 0;
		整数 len = *path_used + seglen + 1;

		如果 (*path_alloc < len) {
			char *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			如果 (!temp) {
				返回;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		如果 (p->segment != NULL) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		如果 (p->children != NULL) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} 否则 {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		如果 (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} 否则 {
			/* leaf node */
			如果 (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				如果 (p->port) {
					fprintf(fp,"%d\n", p->port);
				} 否则 {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (整数)p->urld.type);

				fprintf(fp, "\n");

				如果 (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					为 (i = 0; s[i] != '\0'; i++)
						如果 (s[i] < 32)
							s[i] = ' ';
					为 (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} 否则 {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			当 (p != parent) {
				整数 seglen = p->segment != NULL
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				如果 (p->next != NULL) {
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
	} 当 (p != parent);
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
		 unsigned 整数 *count)
{
	const struct path_data *p = root;

	do {
		如果 (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} 否则 {
			/* No more children, increment count if required */
			如果 (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			当 (p != root) {
				如果 (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					break;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} 当 (p != root);
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
	unsigned 整数 path_count = 0;
	char *path, *p, *end;
	整数 path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	整数 hsts_include_subdomains = 0;

	expiry = time(NULL) - ((60 * 60 * 24) * nsoption_int(expire_url));

	如果 (parent == &empty)
		返回;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	如果 (!path)
		返回;

	path[0] = '\0';

	为 (h = parent->data, p = host, end = host + sizeof host;
	     h && h != &db_root && p < end; h = h->parent) {
		整数 written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		如果 (written < 0) {
			free(path);
			返回;
		}
		p += written;
	}

	h = parent->data;
	如果 (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	如果 (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} 否则 如果 (hsts_expiry) {
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
		如果 (p->children != NULL) {
			/* Drill down into children */
			p = p->children;
		} 否则 {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			assert(url_callback || cookie_callback);

			/** \todo handle fragments? */
			如果 (url_callback) {
				const struct url_internal_data *u = &p->urld;

				assert(p->url);

				如果 (!url_callback(p->url,
						  (const struct url_data *) u))
					返回 false;
			} 否则 {
				c = (const struct cookie_data *)p->cookies;
				为 (; c != NULL; c = c->next) {
					如果 (!cookie_callback(c))
						返回 false;
				}
			}

			/* Now, find next node to process. */
			当 (p != parent) {
				如果 (p->next != NULL) {
					/* Have a sibling, process that */
					p = p->next;
					break;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} 当 (p != parent);

	返回 true;
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
	unsigned 整数 ipv6_addr_len;
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
	如果 (slash == NULL) {
		sane_host = host;
	} 否则 {
		char *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	如果 (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		goto out_false;

	如果 (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		整数 num_dots = 0;
		size_t index;

		为 (index = 0; index < host_len; index++) {
			如果 (sane_host[index] == '.')
				num_dots++;
		}

		如果 (num_dots == 3)
			goto out_true;
		否则
			goto out_false;
	}

#ifndef NO_IPV6
	如果 ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		goto out_false;
	}

	ipv6_addr_len = host_len - 2;
	如果 (ipv6_addr_len >= sizeof(ipv6_addr)) {
		ipv6_addr_len = sizeof(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	如果 (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		goto out_true;
#endif

out_false:
	如果 (slash != NULL) free((void *)sane_host);
	返回 false;

out_true:
	如果 (slash != NULL) free((void *)sane_host);
	返回 true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
static 整数 urldb_search_match_prefix(const struct host_part *a, const char *b)
{
	const char *end, *dot;
	整数 plen, ret;

	assert(a && a != &db_root && b);

	如果 (urldb__host_is_ip_address(b)) {
		/* IP address */
		返回 strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	当 (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		如果 (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		如果 ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			返回 ret;

		/* The strings matched */
		如果 (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			如果 (plen > dot - b) {
				/* len(a) > len(b) */
				返回 1;
			} 否则 如果 (plen < dot - b) {
				/* len(a) < len(b) */
				返回 -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	如果 (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		返回 0;
	} 否则 如果 ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		返回 -1;
	}

	/* Identical */
	返回 0;
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
	整数 c;

	assert(root && prefix && callback);

	如果 (root == &empty)
		返回 true;

	c = urldb_search_match_prefix(root->data, prefix);

	如果 (c > 0) {
		/* No match => look in left subtree */
		返回 urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} 否则 如果 (c < 0) {
		/* No match => look in right subtree */
		返回 urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} 否则 {
		/* Match => iterate over l/r subtrees & process this node */
		如果 (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			返回 false;
		}

		如果 (root->data->paths.children) {
			/* and extract all paths attached to this host */
			如果 (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							NULL)) {
				返回 false;
			}
		}

		如果 (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			返回 false;
		}
	}

	返回 true;
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
		如果 (!slash) {
			slash = end;
		}

		如果 (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			continue;
		}

		如果 (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			如果 (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				如果 (!urldb_iterate_entries_path(p,
								callback,
								NULL)) {
					返回 false;
				}

				/* Progress to next sibling */
				p = p->next;
			} 否则 {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} 否则 {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} 当 (p != NULL);

	返回 true;
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
	如果 (parent == &empty) {
		返回 true;
	}

	如果 (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		返回 false;
	}

	如果 ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		如果 (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			返回 false;
		}
	}

	如果 (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		返回 false;
	}

	返回 true;
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
	如果 (!d) {
		返回 NULL;
	}

	d->part = strdup(part);
	如果 (!d->part) {
		free(d);
		返回 NULL;
	}

	d->next = parent->children;
	如果 (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	返回 d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
static 整数 urldb_add_path_fragment_cmp(const void *a, const void *b)
{
	返回 strcasecmp(*((const char **) a), *((const char **) b));
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
	如果 (!fragment)
		返回 segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * sizeof(char *));
	如果 (!temp)
		返回 NULL;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	如果 (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		返回 NULL;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      sizeof (char *),
	      urldb_add_path_fragment_cmp);

	返回 segment;
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
		    unsigned 整数 port,
		    const char *segment,
		    lwc_string *fragment,
		    struct path_data *parent)
{
	struct path_data *d, *e;

	assert(scheme && segment && parent);

	d = calloc(1, sizeof(struct path_data));
	如果 (!d)
		返回 NULL;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	如果 (!d->segment) {
		lwc_string_unref(d->scheme);
		free(d);
		返回 NULL;
	}

	如果 (fragment) {
		如果 (!urldb_add_path_fragment(d, fragment)) {
			free(d->segment);
			lwc_string_unref(d->scheme);
			free(d);
			返回 NULL;
		}
	}

	为 (e = parent->children; e; e = e->next) {
		如果 (strcmp(e->segment, d->segment) > 0)
			break;
	}

	如果 (e) {
		d->prev = e->prev;
		d->next = e;
		如果 (e->prev)
			e->prev->next = d;
		否则
			parent->children = d;
		e->prev = d;
	} 否则 如果 (!parent->children) {
		d->prev = d->next = NULL;
		parent->children = parent->last = d;
	} 否则 {
		d->next = NULL;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	返回 d;
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

	如果 (urldb__host_is_ip_address(host)) {
		返回 &search_trees[ST_IP];
	} 否则 如果 (ascii_is_alpha(*host)) {
		返回 &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	返回 &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
static struct search_node *urldb_get_search_tree(const char *host)
{
	返回 *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
static 整数 urldb_search_match_string(const struct host_part *a, const char *b)
{
	const char *end, *dot;
	整数 plen, ret;

	assert(a && a != &db_root && b);

	如果 (urldb__host_is_ip_address(b)) {
		/* IP address */
		返回 strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	当 (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		如果 (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		如果 ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			返回 ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		如果 (plen > dot - b) {
			/* len(a) > len(b) */
			返回 1;
		} 否则 如果 (plen < dot - b) {
			/* len(a) < len(b) */
			返回 -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	如果 (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		返回 1;
	} 否则 如果 ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		返回 -1;
	}

	/* Identical */
	返回 0;
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
	整数 c;

	assert(root && host);

	如果 (root == &empty) {
		返回 NULL;
	}

	c = urldb_search_match_string(root->data, host);

	如果 (c > 0) {
		返回 urldb_search_find(root->left, host);
	} 否则 如果 (c < 0) {
		返回 urldb_search_find(root->right, host);
	}

	返回 root->data;
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

	如果 (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	assert(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	当 (p != NULL) {
		slash = strchr(path + 1, '/');
		如果 (!slash) {
			slash = path + strlen(path);
		}

		如果 (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			如果 (*slash == '\0') {
				/* Complete match */
				返回 (struct path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} 否则 {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	返回 NULL;
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
	unsigned 整数 port_int;
	bool match;

	assert(url);

	如果 (url_bloom != NULL) {
		如果 (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			返回 NULL;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	如果 (scheme == NULL)
		返回 NULL;

	如果 (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		返回 NULL;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	如果 (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} 否则 如果 (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} 否则 {
		lwc_string_unref(scheme);
		返回 NULL;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	如果 (!h) {
		lwc_string_unref(scheme);
		返回 NULL;
	}

	/* generate plq (path, leaf, query) */
	如果 (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		返回 NULL;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	如果 (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} 否则 {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	free(plq);
	lwc_string_unref(scheme);

	返回 p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_paths(struct path_data *parent)
{
	const struct path_data *p = parent;
	unsigned 整数 i;

	do {
		如果 (p->segment != NULL) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			为 (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		如果 (p->children != NULL) {
			p = p->children;
		} 否则 {
			当 (p != parent) {
				如果 (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} 当 (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
static void urldb_dump_hosts(struct host_part *parent)
{
	struct host_part *h;

	如果 (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	为 (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
static void urldb_dump_search(struct search_node *parent, 整数 depth)
{
	const struct host_part *h;
	整数 i; /* index into string */
	char s[1024];
	整数 r;
	整数 sl = sizeof(s) - 2;

	如果 (parent == &empty)
		返回;

	urldb_dump_search(parent->left, depth + 1);

	为 (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	为 (h = parent->data; h; h = h->parent) {
		如果 (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			如果 (r < 0) {
				break;
			}
			如果 ((i + r) >= sl) {
				break;
			}
			i += r;
		}

		如果 (h->parent && h->parent->parent) {
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
static 整数
urldb_search_match_host(const struct host_part *a, const struct host_part *b)
{
	整数 ret;

	assert(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	为 (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		如果 ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			返回 ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	如果 (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		返回 1;
	} 否则 如果 ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		返回 -1;
	}

	/* identical */
	返回 0;
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

	如果 (root->left->level == root->level) {
		struct search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	返回 root;
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

	如果 (root->right->right->level == root->level) {
		struct search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	返回 root;
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

	如果 (root == &empty) {
		root = n;
	} 否则 {
		整数 c = urldb_search_match_host(root->data, n->data);

		如果 (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} 否则 如果 (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} 否则 {
			/* exact match */
			free(n);
			返回 root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	返回 root;
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
	如果 (!n)
		返回 NULL;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	返回 root;
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
	整数 vlen;

	assert(c && n && v);

	/* Strip whitespace from start of name */
	为 (; *n; n++) {
		如果 (*n != ' ' && *n != '\t')
			break;
	}

	/* Strip whitespace from end of name */
	为 (vlen = strlen(n); vlen; vlen--) {
		如果 (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		否则
			break;
	}

	/* Strip whitespace from start of value */
	为 (; *v; v++) {
		如果 (*v != ' ' && *v != '\t')
			break;
	}

	/* Strip whitespace from end of value */
	为 (vlen = strlen(v); vlen; vlen--) {
		如果 (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		否则
			break;
	}

	如果 (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		如果 (!c->comment)
			返回 false;
	} 否则 如果 (!c->domain && strcasecmp(n, "Domain") == 0) {
		如果 (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			如果 (!c->domain)
				返回 false;
		}
	} 否则 如果 (strcasecmp(n, "Max-Age") == 0) {
		整数 temp = atoi(v);
		如果 (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		否则
			c->expires = time(NULL) + temp;
	} 否则 如果 (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		如果 (!c->path)
			返回 false;
	} 否则 如果 (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} 否则 如果 (strcasecmp(n, "Expires") == 0) {
		char *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		为 (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		如果 (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} 否则 如果 (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} 否则 如果 (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} 否则 如果 (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		如果 (!c->name || !c->value) {
			返回 false;
		}
	}

	返回 true;
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
	如果 (c == NULL)
		返回 NULL;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	为 (cur = *cookie; *cur; cur++) {
		如果 (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			如果 (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			break;
		} 否则 如果 (*cur == '\r') {
			/* Spurious linefeed */
			continue;
		} 否则 如果 (*cur == '\n') {
			/* Spurious newline */
			continue;
		}

		如果 (in_value && !had_value_data) {
			如果 (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				continue;
			} 否则 {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				如果 (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		如果 (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			如果 (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				continue;
			}
		}

		如果 (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			continue;
		}

		如果 (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			如果 (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				返回 NULL;
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
			如果 (*cur != ';') {
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
		如果 (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			const char *p;
			const char *semi = strchr(cur + 1, ';');
			如果 (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			为 (p = cur + 1; p < semi; p++)
				如果 (*p == '=')
					break;

			如果 (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} 否则 {
				/* found one => comma marks end of cookie */
				cur++;
				break;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		如果 (!in_value) {
			如果 (n < name + (sizeof(name) - 1))
				*n++ = *cur;
		} 否则 {
			如果 (v < value + (sizeof(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	如果 (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		返回 NULL;
	}

	/* Now fix-up default values */
	如果 (c->domain == NULL) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		如果 (host == NULL) {
			urldb_free_cookie(c);
			返回 NULL;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	如果 (c->path == NULL) {
		const char *path_data;
		char *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		如果 (path_lwc == NULL) {
			urldb_free_cookie(c);
			返回 NULL;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		如果 (slash != NULL) {
			/* Special case: retain first slash in path */
			如果 (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			如果 (slash == NULL) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				返回 NULL;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} 否则 {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			如果 (path == NULL) {
				urldb_free_cookie(c);
				返回 NULL;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	返回 c;
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
	       unsigned 整数 port,
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
	如果 (*segment == '/')
		segment++;

	/* Process path segments */
	do {
		slash = strchr(segment, '/');
		如果 (!slash) {
			/* last segment */
			/* look for existing entry */
			为 (e = d->children; e; e = e->next)
				如果 (strcmp(segment, e->segment) == 0 &&
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
		为 (e = d->children; e; e = e->next)
			如果 (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				break;

		d = e ? e : urldb_add_path_node(scheme, port, segment, NULL, d);
		如果 (!d)
			break;

		segment = slash + 1;
	} 当 (1);

	free(path_query);

	如果 (d && !d->url) {
		/* Insert defragmented URL */
		如果 (nsurl_defragment(url, &d->url) != NSERROR_OK)
			返回 NULL;
	}

	返回 d;
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

	如果 (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		为 (e = d->children; e; e = e->next)
			如果 (strcasecmp(host, e->part) == 0)
				/* found => return it */
				返回 e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		如果 (!s) {
			/* failed */
			d = NULL;
		} 否则 {
			search_trees[ST_IP] = s;
		}

		返回 d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, sizeof buf);
	buf[sizeof buf - 1] = '\0';

	/* Process FQDN segments backwards */
	do {
		part = strrchr(buf, '.');
		如果 (!part) {
			/* last segment */
			/* Check for existing entry */
			为 (e = d->children; e; e = e->next)
				如果 (strcasecmp(buf, e->part) == 0)
					break;

			如果 (e) {
				d = e;
			} 否则 {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			如果 (d) {
				struct search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				如果 (!s) {
					/* failed */
					d = NULL;
				} 否则 {
					*r = s;
				}
			}
			break;
		}

		/* Check for existing entry */
		为 (e = d->children; e; e = e->next)
			如果 (strcasecmp(part + 1, e->part) == 0)
				break;

		d = e ? e : urldb_add_host_node(part + 1, d);
		如果 (!d)
			break;

		*part = '\0';
	} 当 (1);

	返回 d;
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

	如果 (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		如果 (!h) {
			h = urldb_add_host(c->domain + 1);
			如果 (!h) {
				urldb_free_cookie(c);
				返回 false;
			}
		}

		p = (struct path_data *) &h->paths;
	} 否则 {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		assert(url != NULL);
		assert(scheme != NULL);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		如果 (!h) {
			h = urldb_add_host(c->domain);
			如果 (!h) {
				urldb_free_cookie(c);
				返回 false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), NULL, url);
		如果 (!p) {
			urldb_free_cookie(c);
			返回 false;
		}
	}

	/* add cookie */
	为 (d = p->cookies; d; d = d->next) {
		如果 (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			break;
	}

	如果 (d) {
		如果 (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			如果 (d->next)
				d->next->prev = d->prev;
			否则
				p->cookies_end = d->prev;
			如果 (d->prev)
				d->prev->next = d->next;
			否则
				p->cookies = d->next;

			cookie_manager_remove((struct cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} 否则 {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			如果 (c->next)
				c->next->prev = c;
			否则
				p->cookies_end = c;
			如果 (c->prev)
				c->prev->next = c;
			否则
				p->cookies = c;

			cookie_manager_remove((struct cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((struct cookie_data *)c);
		}
	} 否则 {
		c->prev = p->cookies_end;
		c->next = NULL;
		如果 (p->cookies_end)
			p->cookies_end->next = c;
		否则
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((struct cookie_data *)c);
	}

	返回 true;
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
		    整数 *used,
		    整数 *alloc,
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
	整数 max_len;
	整数 outl; /* output length */

	assert(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	如果 (max_len >= *alloc) {
		char *temp = realloc(*buf, max_len);
		如果 (!temp) {
			返回 false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	如果 (outl >= (*alloc - *used)) {
		/* output truncated */
		返回 false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	如果 (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		如果 (outl >= (*alloc - *used)) {
			/* output truncated */
			返回 false;
		}
		*used += outl;
	} 否则 {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		如果 (outl >= (*alloc - *used)) {
			/* output truncated */
			返回 false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	返回 true;
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
		      整数 *used,
		      整数 *alloc,
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
	整数 max_len;
	整数 outl; /* output length */

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

	如果 (max_len >= *alloc) {
		char *temp = realloc(*buf, max_len);
		如果 (!temp) {
			返回 false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	如果 (outl >= (*alloc - *used)) {
		/* output truncated */
		返回 false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	如果 (c->value_was_quoted ||
	    strpbrk(c->value, separators) != NULL) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} 否则 {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	如果 (outl >= (*alloc - *used)) {
		/* output truncated */
		返回 false;
	}
	*used += outl;

	如果 (c->path_from_set) {
		/* Path, quoted if necessary */

		如果 (strpbrk(c->path, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} 否则 {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		如果 (outl >= (*alloc - *used)) {
			/* output truncated */
			返回 false;
		}
		*used += outl;
	}

	如果 (c->domain_from_set) {
		/* Domain, quoted if necessary */
		如果 (strpbrk(c->domain, separators) != NULL) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} 否则 {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		如果 (outl >= (*alloc - *used)) {
			/* output truncated */
			返回 false;
		}
		*used += outl;
	}

	返回 true;
}


/**
 * build output cookie string in netscape format
 */
static char *
get_netscape_cookie(struct cookie_internal_data **matched_cookies, 整数 count)
{
	整数 ret_alloc = 4096, ret_used = 1;
	char *ret;
	整数 midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	如果 (ret == NULL) {
		返回 NULL;
	}
	ret[0] = '\0';

	为 (midx = 0; midx < count; midx++) {
		如果 (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			free(ret);
			返回 NULL;
		}
	}

	如果 (ret_used <= 2) {
		free(ret);
		返回 NULL;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	char *temp = realloc(ret, ret_used);
	如果 (temp != NULL) {
		ret = temp;
	}

	返回 ret;
}


/**
 * build output cookie string in RFC2965 format
 */
static char *
get_rfc2965_cookie(struct cookie_internal_data **matched_cookies,
		   整数 count,
		   整数 version)
{
	整数 ret_alloc = 4096; /* return string allocation */
	整数 ret_used = 1; /* return sting buffer used */
	char *ret; /* return string buffer */
	整数 midx; /* index into matched cookies */
	整数 outl;

	ret = malloc(ret_alloc);
	如果 (ret == NULL) {
		返回 NULL;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	如果 (outl >= ret_alloc - ret_used) {
		/* overflow */
		free(ret);
		返回 NULL;
	}
	ret_used += outl;

	为 (midx = 0; midx < count; midx++) {
		如果 (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			free(ret);
			返回 NULL;
		}
	}

	/* shrink the output buffer to the required size */
	char *temp = realloc(ret, ret_used);
	如果 (temp != NULL) {
		ret = temp;
	}

	返回 ret;
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
		为 (c = p->cookies; c; c = c->next) {
			如果 (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				如果 (c->prev) {
					c->prev->next = c->next;
				} 否则 {
					p->cookies = c->next;
				}

				如果 (c->next) {
					c->next->prev = c->prev;
				} 否则 {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				返回;
			}
		}

		如果 (p->children) {
			p = p->children;
		} 否则 {
			当 (p != parent) {
				如果 (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} 当 (p != parent);
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

	为 (h = parent->children; h; h = h->next) {
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
		如果 (p->cookies != NULL) {
			struct cookie_internal_data *c;

			为 (c = p->cookies; c != NULL; c = c->next) {
				如果 (c->expires == -1 || c->expires < now) {
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
					(整数)c->expires, (整数)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		如果 (p->children != NULL) {
			p = p->children;
		} 否则 {
			当 (p != parent) {
				如果 (p->next != NULL) {
					p = p->next;
					break;
				}

				p = p->parent;
			}
		}
	} 当 (p != parent);
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

	为 (h = parent->children; h; h = h->next)
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
	unsigned 整数 i;

	如果 (node->url != NULL) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	free(node->segment);
	为 (i = 0; i < node->frag_cnt; i++)
		free(node->fragment[i]);
	free(node->fragment);

	free(node->urld.title);

	为 (a = node->cookies; a; a = b) {
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
		如果 (p->children != NULL) {
			p = p->children;
		} 否则 {
			struct path_data *q = p;

			当 (p != root) {
				如果 (p->next != NULL) {
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
	} 当 (p != root);
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
	为 (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	为 (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	为 (s = root->prot_space; s; s = t) {
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
	如果 (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	如果 (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	free(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
void urldb_destroy(void)
{
	struct host_part *a, *b;
	整数 i;

	/* Clean up search trees */
	为 (i = 0; i < NUM_SEARCH_TREES; i++) {
		如果 (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	为 (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, sizeof(db_root));

	/* And the bloom filter */
	如果 (url_bloom != NULL) {
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
	整数 urls;
	整数 i;
	整数 version;
	整数 length;
	FILE *fp;

	assert(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	如果 (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	如果 (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		返回 NSERROR_NOT_FOUND;
	}

	如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		返回 NSERROR_NEED_DATA;
	}

	version = atoi(s);
	如果 (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		返回 NSERROR_INVALID;
	}
	如果 (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		返回 NSERROR_INVALID;
	}

	当 (fgets(host, sizeof host, fp)) {
		time_t hsts_expiry = 0;
		整数 hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		如果 (length == 0) {
			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			urls = atoi(s);
			/* Eight fields/url */
			为 (i = 0; i < (8 * urls); i++) {
				如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					break;
			}
			continue;
		}

		如果 (version >= 107) {
			char *p = host;
			当 (*p && *p != ' ') p++;
			当 (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			当 (*p && *p != ' ') p++;
			当 (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		如果 (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			返回 NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			break;
		urls = atoi(s);

		/* no URLs => try next host */
		如果 (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			continue;
		}

		/* load the non-corrupt data */
		为 (i = 0; i < urls; i++) {
			struct path_data *p = NULL;
			char scheme[64], ports[10];
			char url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned 整数 port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			char *path_query;
			size_t len;

			如果 (!fgets(scheme, sizeof scheme, fp))
				break;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			如果 (!fgets(ports, sizeof ports, fp))
				break;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			length = strlen(s) - 1;
			s[length] = '\0';

			如果 (!strcasecmp(host, "localhost") &&
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
			如果 (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				返回 NSERROR_NOMEM;
			}

			如果 (url_bloom != NULL) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			如果 (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				返回 NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			如果 (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				返回 NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			如果 (p)
				p->urld.visits = (unsigned 整数)atoi(s);

			/* entry last use time */
			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				break;
			}
			如果 (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			如果 (p)
				p->urld.type = (content_type)atoi(s);

			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;


			如果 (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				break;
			length = strlen(s) - 1;
			如果 (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				如果 (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	返回 NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(const char *filename)
{
	FILE *fp;
	整数 i;

	assert(filename);

	fp = fopen(filename, "w");
	如果 (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		返回 NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	为 (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	返回 NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	如果 (!p) {
		返回 NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	返回 NSERROR_OK;
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
	unsigned 整数 port_int;

	assert(url);

	如果 (url_bloom == NULL)
		url_bloom = bloom_create(BLOOM_SIZE);

	如果 (url_bloom != NULL) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	如果 (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		返回 false;
	}
	assert(path_query != NULL);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	如果 (scheme == NULL) {
		free(path_query);
		返回 false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	如果 (host != NULL) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} 否则 如果 (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} 否则 {
		lwc_string_unref(scheme);
		free(path_query);
		返回 false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	如果 (port != NULL) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} 否则 {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	如果 (h != NULL) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} 否则 {
		p = NULL;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	返回 (p != NULL);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, const char *title)
{
	struct path_data *p;
	char *temp;

	assert(url);

	p = urldb_find_url(url);
	如果 (p == NULL) {
		返回 NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	如果 (title != NULL) {
		temp = strdup(title);
		如果 (temp == NULL) {
			返回 NSERROR_NOMEM;
		}
	} 否则 {
		temp = NULL;
	}

	free(p->urld.title);
	p->urld.title = temp;

	返回 NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	如果 (!p) {
		返回 NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	返回 NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	如果 (!p) {
		返回 NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(NULL);
	p->urld.visits++;

	返回 NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
void urldb_reset_url_visit_data(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	如果 (!p)
		返回;

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
	如果 (!p)
		返回 NULL;

	u = &p->urld;

	返回 (const struct url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	struct path_data *p;

	assert(url);

	p = urldb_find_url(url);
	如果 (!p)
		返回 NULL;

	返回 p->url;
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

	如果 (!p)
		返回;

	/* Search for host_part */
	为 (pi = p; pi->parent != NULL; pi = pi->parent)
		;
	h = (struct host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	为 (space = h->prot_space; space; space = space->next) {
		如果 (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			break;
	}

	如果 (space != NULL) {
		/* Overrule existing auth. */
		free(space->auth);
		space->auth = strdup(auth);
	} 否则 {
		/* Create a new protection space. */
		space = space_alloc = malloc(sizeof(struct prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		如果 (!space_alloc || !realm_alloc || !auth_alloc) {
			free(space_alloc);
			free(realm_alloc);
			free(auth_alloc);
			返回;
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
	如果 (!p)
		返回 NULL;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	为 (p_cur = p; p_cur != NULL; p_top = p_cur, p_cur = p_cur->parent) {
		如果 (p_cur->prot_space) {
			返回 p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	如果 (realm != NULL) {
		const struct host_part *h = (const struct host_part *)p_top;
		const struct prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		为 (space = h->prot_space; space != NULL;
		     space = space->next) {
			如果 (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				返回 p->prot_space->auth;
			}
		}
	}

	返回 NULL;
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
	如果 (!p)
		返回;

	为 (; p && p->parent; p = p->parent)
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
	如果 (!p)
		返回 false;

	为 (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const struct host_part *)p;

	返回 h->permit_invalid_certs;
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
	如果 (host != NULL) {
		如果 (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			返回 true;
		} 否则 如果 (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			返回 true;
		}

		lwc_string_unref(host);
	} 否则 {
		/* No host part: ignore */
		返回 true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	如果 (!p)
		返回 false;

	为 (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (struct host_part *)p;
	如果 (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		返回 true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	如果 (error != NSERROR_OK) {
		/* Parse failed: ignore */
		返回 true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	如果 (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} 否则 如果 ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	返回 true;
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
	如果 (host != NULL) {
		如果 (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			返回 false;
		} 否则 如果 (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			返回 false;
		}

		lwc_string_unref(host);
	} 否则 {
		/* No host part: not enabled */
		返回 false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	如果 (!p)
		返回 false;

	为 (; p && p->parent; p = p->parent)
		/* do nothing */;
	assert(p);

	h = (const struct host_part *)p;

	/* Consult record for this host */
	如果 (h->hsts.expires > now) {
		/* Not expired */
		返回 true;
	}

	/* Consult parent domains */
	为 (h = h->parent; h && h != &db_root; h = h->parent) {
		如果 (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			返回 true;
		}
	}

	返回 false;
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
	如果 (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	如果 (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, sizeof host, "%.*s",
			 (整数) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		如果 (!h) {
			整数 len = slash - prefix;

			如果 (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, sizeof buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				如果 (!h)
					返回;
			} 否则
				返回;
		}

		如果 (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} 否则 {
		整数 len = strlen(prefix);

		/* looking for hosts */
		如果 (!urldb_iterate_partial_host(tree, prefix, callback))
			返回;

		如果 (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, sizeof buf, "www.%s", prefix);
			如果(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				返回;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
void
urldb_iterate_entries(bool (*callback)(nsurl *url, const struct url_data *data))
{
	整数 i;

	assert(callback);

	为 (i = 0; i < NUM_SEARCH_TREES; i++) {
		如果 (!urldb_iterate_entries_host(search_trees[i],
						callback,
						NULL)) {
			break;
		}
	}
}


/* exported interface documented in content/urldb.h */
void urldb_iterate_cookies(bool (*callback)(const struct cookie_data *data))
{
	整数 i;

	assert(callback);

	为 (i = 0; i < NUM_SEARCH_TREES; i++) {
		如果 (!urldb_iterate_entries_host(search_trees[i],
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
	如果 (nsurl_defragment(url, &urlt) != NSERROR_OK)
		返回 NULL;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	如果 (scheme == NULL) {
		nsurl_unref(urlt);
		返回 false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	如果 (path == NULL) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		返回 false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	如果 (host == NULL) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		返回 false;
	}

	如果 (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		如果 (rhost == NULL) {
			goto error;
		}

		/* Domain match host names */
		如果 (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			const char *hptr;
			const char *rptr;
			const char *dot;
			const char *host_data = lwc_string_data(host);
			const char *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			如果 (urldb__host_is_ip_address(host_data) ||
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
			当 (hptr >= host_data && rptr >= rhost_data) {
				如果 (*hptr != *rptr)
					break;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			当 (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			如果 (*hptr == '\0' ||
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
		如果 (!c) {
			/* failed => stop parsing */
			goto error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		如果 (!c->name || !c->value) {
			urldb_free_cookie(c);
			goto error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		如果 (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			goto error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		如果 (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		如果 (suffix == NULL) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			goto error;
		}
#否则
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		如果 (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			goto error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		如果 (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			整数 hlen, dlen;
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
			如果 (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				goto error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			如果 (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				goto error;
			}

			如果 (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			如果 (strcasecmp(lwc_string_data(host) + (hlen - dlen),
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
		如果 (!urldb_insert_cookie(c, scheme, urlt))
			goto error;
	} 当 (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	返回 true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	返回 false;
}


/* exported interface documented in content/urldb.h */
char *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	const struct path_data *p, *q;
	const struct host_part *h;
	lwc_string *path_lwc;
	struct cookie_internal_data *c;
	整数 count = 0, version = COOKIE_RFC2965;
	struct cookie_internal_data **matched_cookies;
	整数 matched_cookies_size = 20;
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
	如果 (!p)
		返回 NULL;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 sizeof(struct cookie_internal_data *));
	如果 (!matched_cookies)
		返回 NULL;

#define GROW_MATCHED_COOKIES						\
	do {								\
		如果 (count == matched_cookies_size) {			\
			struct cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       sizeof(struct cookie_internal_data *)); \
									\
			如果 (temp == NULL) {				\
				free(matched_cookies);			\
				返回 NULL;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} 当(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	如果 (path_lwc == NULL) {
		free(matched_cookies);
		返回 NULL;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(NULL);

	如果 (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		为 (q = p->parent->children; q; q = q->next) {
			如果 (strcmp(q->segment, p->segment))
				continue;

			/* Consider all cookies associated with
			 * this exact path */
			为 (c = q->cookies; c; c = c->next) {
				如果 (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				如果 (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					continue;

				如果 (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				如果 (c->version < (unsigned 整数)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((struct cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	为 (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		为 (q = p->children; q; q = q->next) {
			如果 (*(q->segment) != '\0')
				continue;

			为 (c = q->cookies; c; c = c->next) {
				如果 (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					continue;

				如果 (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					continue;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				如果 (c->version < (unsigned 整数) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((struct cookie_data *)c);
			}
		}

		如果 (!p->parent) {
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
		为 (c = p->cookies; c; c = c->next) {
			如果 (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			如果 (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			如果 (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			如果 (c->version < (unsigned 整数) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((struct cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	为 (h = (const struct host_part *)p; h && h != &db_root;
	     h = h->parent) {
		为 (c = h->paths.cookies; c; c = c->next) {
			如果 (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				continue;

			/* Ensure cookie path is a prefix of the resource */
			如果 (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				continue;

			如果 (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				continue;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			如果 (c->version < (unsigned 整数)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((struct cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	如果 (count == 0) {
		/* No cookies found */
		free(matched_cookies);
		返回 NULL;
	}

	如果 (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} 否则 {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	free(matched_cookies);

	返回 ret;
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
	如果 (!fp)
		返回 NSERROR_NOT_FOUND;

#define FIND_T {				\
		为 (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		如果 (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		为 (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		如果 (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			continue;		\
		}				\
	}

	当 (fgets(s, sizeof s, fp)) {
		char *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		整数 version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		struct cookie_internal_data *c;

		如果(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			continue;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		如果 (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			如果 (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				break;
			}

			continue;
		} 否则 如果 (loaded_cookie_file_version == 0) {
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
		如果 (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} 否则 {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		如果 (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} 否则 {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		为 (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		assert(p <= end);

		/* Now create cookie */
		c = malloc(sizeof(struct cookie_internal_data));
		如果 (!c)
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

		如果 (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			break;
		}

		如果 (c->domain[0] != '.') {
			lwc_string *scheme_lwc = NULL;
			nsurl *url_nsurl = NULL;

			assert(scheme[0] != 'u');

			如果 (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				break;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			如果 (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				break;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} 否则 {
			如果 (!urldb_insert_cookie(c, NULL, NULL)) {
				/* Cookie freed for us */
				break;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	返回 NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(const char *filename)
{
	FILE *fp;
	整数 cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	assert(filename);

	fp = fopen(filename, "w");
	如果 (!fp)
		返回 NSERROR_NOT_FOUND;

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

	返回 NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
void urldb_dump(void)
{
	整数 i;

	urldb_dump_hosts(&db_root);

	为 (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
