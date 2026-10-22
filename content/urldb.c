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

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ifdef WITH_NSPSL
#ضمن <nspsl.h>
#endif

#ضمن "utils/inet.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/url.h"
#ضمن "utils/utils.h"
#ضمن "utils/bloom.h"
#ضمن "utils/time.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/ascii.h"
#ضمن "utils/http.h"
#ضمن "netsurf/bitmap.h"
#ضمن "desktop/cookie_manager.h"

#ضمن "content/content.h"
#ضمن "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
بنية cookie_internal_data {
	بنية cookie_internal_data *prev;	/**< Previous in list */
	بنية cookie_internal_data *next;	/**< Next in list */

	حرف *name;		/**< Cookie name */
	حرف *value;		/**< Cookie value */
	bool value_was_quoted;	/**< Value was quoted in Set-Cookie: */
	حرف *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	حرف *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	حرف *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	تعداد cookie_version version;	/**< Specification compliance */
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
بنية prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned صحيح port;
	/** Protection realm */
	حرف *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	حرف *auth;
	/** Next sibling */
	بنية prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
بنية url_internal_data {
	حرف *title;		/**< Resource title */
	unsigned صحيح visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
بنية path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned صحيح port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	حرف *segment;		/**< Path segment for this node */
	unsigned صحيح frag_cnt;	/**< Number of entries in path_data::fragment */
	حرف **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	بنية url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ثابت بنية prot_space_data *prot_space;
	/** Cookies associated with resource */
	بنية cookie_internal_data *cookies;
	/** Last cookie in list */
	بنية cookie_internal_data *cookies_end;

	بنية path_data *next;	/**< Next sibling */
	بنية path_data *prev;	/**< Previous sibling */
	بنية path_data *parent; /**< Parent path segment */
	بنية path_data *children; /**< Child path segments */
	بنية path_data *last; /**< Last child */
};

بنية hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

بنية host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	بنية path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	بنية hsts_data hsts;

	/**
	 * Part of host string
	 */
	حرف *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	بنية prot_space_data *prot_space;

	بنية host_part *next;	/**< Next sibling */
	بنية host_part *prev;	/**< Previous sibling */
	بنية host_part *parent; /**< Parent host part */
	بنية host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
بنية search_node {
	ثابت بنية host_part *data;	/**< Host tree entry */

	unsigned صحيح level;		/**< Node level */

	بنية search_node *left;	/**< Left subtree */
	بنية search_node *right;	/**< Right subtree */
};

/** Root database handle */
ساكن بنية host_part db_root;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define NUM_SEARCH_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
ساكن بنية search_node empty = { 0, 0, &empty, &empty };
ساكن بنية search_node *search_trees[NUM_SEARCH_TREES] = {
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
ساكن صحيح loaded_cookie_file_version;

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
ساكن بنية bloom_filter *url_bloom;
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
ساكن nserror urldb_write_timet(FILE *fp, time_t val)
{
	صحيح use;
	حرف op[32];

	use = nsc_sntimet(op, 32, &val);
	אם (use == 0) {
		fprintf(fp, "%i\n", (صحيح)val);
	} אחרת {
		fprintf(fp, "%.*s\n", use, op);
	}
	החזר NSERROR_OK;
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
ساكن فراغ
urldb_write_paths(ثابت بنية path_data *parent,
		  ثابت حرف *host,
		  FILE *fp,
		  حرف **path,
		  صحيح *path_alloc,
		  صحيح *path_used,
		  time_t expiry)
{
	ثابت بنية path_data *p = parent;
	صحيح i;

	עשה {
		صحيح seglen = p->segment != عدم ? strlen(p->segment) : 0;
		صحيح len = *path_used + seglen + 1;

		אם (*path_alloc < len) {
			حرف *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			אם (!temp) {
				החזר;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		אם (p->segment != عدم) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		אם (p->children != عدم) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} אחרת {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		אם (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} אחרת {
			/* leaf node */
			אם (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				אם (p->port) {
					fprintf(fp,"%d\n", p->port);
				} אחרת {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (صحيح)p->urld.type);

				fprintf(fp, "\n");

				אם (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					עבור (i = 0; s[i] != '\0'; i++)
						אם (s[i] < 32)
							s[i] = ' ';
					עבור (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} אחרת {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			כלעוד (p != parent) {
				صحيح seglen = p->segment != عدم
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				אם (p->next != عدم) {
					/* Have a sibling, process that */
					p = p->next;
					اكسر;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} כלעוד (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ساكن فراغ
urldb_count_urls(ثابت بنية path_data *root,
		 time_t expiry,
		 unsigned صحيح *count)
{
	ثابت بنية path_data *p = root;

	עשה {
		אם (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} אחרת {
			/* No more children, increment count if required */
			אם (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*count)++;
			}

			/* Now, find next node to process. */
			כלעוד (p != root) {
				אם (p->next != عدم) {
					/* Have a sibling, process that */
					p = p->next;
					اكسر;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} כלעוד (p != root);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ساكن فراغ urldb_save_search_tree(بنية search_node *parent, FILE *fp)
{
	حرف host[256];
	ثابت بنية host_part *h;
	unsigned صحيح path_count = 0;
	حرف *path, *p, *end;
	صحيح path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	صحيح hsts_include_subdomains = 0;

	expiry = time(عدم) - ((60 * 60 * 24) * nsoption_int(expire_url));

	אם (parent == &empty)
		החזר;

	urldb_save_search_tree(parent->left, fp);

	path = malloc(path_alloc);
	אם (!path)
		החזר;

	path[0] = '\0';

	עבור (h = parent->data, p = host, end = host + حجم host;
	     h && h != &db_root && p < end; h = h->parent) {
		صحيح written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		אם (written < 0) {
			حرر(path);
			החזר;
		}
		p += written;
	}

	h = parent->data;
	אם (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_count_urls(&parent->data->paths, expiry, &path_count);

	אם (path_count > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_count);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} אחרת אם (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	حرر(path);

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
ساكن bool
urldb_iterate_entries_path(ثابت بنية path_data *parent,
		bool (*url_callback)(nsurl *url, ثابت بنية url_data *data),
		bool (*cookie_callback)(ثابت بنية cookie_data *data))
{
	ثابت بنية path_data *p = parent;
	ثابت بنية cookie_data *c;

	עשה {
		אם (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} אחרת {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			أكد(url_callback || cookie_callback);

			/** \todo handle fragments? */
			אם (url_callback) {
				ثابت بنية url_internal_data *u = &p->urld;

				أكد(p->url);

				אם (!url_callback(p->url,
						  (ثابت بنية url_data *) u))
					החזר false;
			} אחרת {
				c = (ثابت بنية cookie_data *)p->cookies;
				עבור (; c != عدم; c = c->next) {
					אם (!cookie_callback(c))
						החזר false;
				}
			}

			/* Now, find next node to process. */
			כלעוד (p != parent) {
				אם (p->next != عدم) {
					/* Have a sibling, process that */
					p = p->next;
					اكسر;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} כלעוד (p != parent);

	החזר true;
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
ساكن bool urldb__host_is_ip_address(ثابت حرف *host)
{
	بنية in_addr ipv4;
	size_t host_len = strlen(host);
	ثابت حرف *sane_host;
	ثابت حرف *slash;
#ifndef NO_IPV6
	بنية in6_addr ipv6;
	حرف ipv6_addr[64];
	unsigned صحيح ipv6_addr_len;
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
	אם (slash == عدم) {
		sane_host = host;
	} אחרת {
		حرف *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	אם (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		اذهب out_false;

	אם (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		صحيح num_dots = 0;
		size_t index;

		עבור (index = 0; index < host_len; index++) {
			אם (sane_host[index] == '.')
				num_dots++;
		}

		אם (num_dots == 3)
			اذهب out_true;
		אחרת
			اذهب out_false;
	}

#ifndef NO_IPV6
	אם ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		اذهب out_false;
	}

	ipv6_addr_len = host_len - 2;
	אם (ipv6_addr_len >= حجم(ipv6_addr)) {
		ipv6_addr_len = حجم(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	אם (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		اذهب out_true;
#endif

out_false:
	אם (slash != عدم) حرر((فراغ *)sane_host);
	החזר false;

out_true:
	אם (slash != عدم) حرر((فراغ *)sane_host);
	החזר true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ساكن صحيح urldb_search_match_prefix(ثابت بنية host_part *a, ثابت حرف *b)
{
	ثابت حرف *end, *dot;
	صحيح plen, ret;

	أكد(a && a != &db_root && b);

	אם (urldb__host_is_ip_address(b)) {
		/* IP address */
		החזר strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	כלעוד (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		אם (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		אם ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			החזר ret;

		/* The strings matched */
		אם (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			אם (plen > dot - b) {
				/* len(a) > len(b) */
				החזר 1;
			} אחרת אם (plen < dot - b) {
				/* len(a) < len(b) */
				החזר -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	אם (a && a != &db_root && b >= end) {
		/* len(a) > len(b) => prefix matches */
		החזר 0;
	} אחרת אם ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		החזר -1;
	}

	/* Identical */
	החזר 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ساكن bool
urldb_iterate_partial_host(بنية search_node *root,
		ثابت حرف *prefix,
		bool (*callback)(nsurl *url, ثابت بنية url_data *data))
{
	صحيح c;

	أكد(root && prefix && callback);

	אם (root == &empty)
		החזר true;

	c = urldb_search_match_prefix(root->data, prefix);

	אם (c > 0) {
		/* No match => look in left subtree */
		החזר urldb_iterate_partial_host(root->left,
						  prefix,
						  callback);
	} אחרת אם (c < 0) {
		/* No match => look in right subtree */
		החזר urldb_iterate_partial_host(root->right,
						  prefix,
						  callback);
	} אחרת {
		/* Match => iterate over l/r subtrees & process this node */
		אם (!urldb_iterate_partial_host(root->left,
						prefix,
						callback)) {
			החזר false;
		}

		אם (root->data->paths.children) {
			/* and extract all paths attached to this host */
			אם (!urldb_iterate_entries_path(&root->data->paths,
							callback,
							عدم)) {
				החזר false;
			}
		}

		אם (!urldb_iterate_partial_host(root->right,
						prefix,
						callback)) {
			החזר false;
		}
	}

	החזר true;
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
ساكن bool
urldb_iterate_partial_path(ثابت بنية path_data *parent,
		ثابت حرف *prefix,
		bool (*callback)(nsurl *url, ثابت بنية url_data *data))
{
	ثابت بنية path_data *p = parent->children;
	ثابت حرف *slash, *end = prefix + strlen(prefix);

	עשה {
		slash = strchr(prefix, '/');
		אם (!slash) {
			slash = end;
		}

		אם (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			استمر;
		}

		אם (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			אם (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				אם (!urldb_iterate_entries_path(p,
								callback,
								عدم)) {
					החזר false;
				}

				/* Progress to next sibling */
				p = p->next;
			} אחרת {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} אחרת {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} כלעוד (p != عدم);

	החזר true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ساكن bool
urldb_iterate_entries_host(بنية search_node *parent,
		bool (*url_callback)(nsurl *url, ثابت بنية url_data *data),
		bool (*cookie_callback)(ثابت بنية cookie_data *data))
{
	אם (parent == &empty) {
		החזר true;
	}

	אם (!urldb_iterate_entries_host(parent->left,
					url_callback,
					cookie_callback)) {
		החזר false;
	}

	אם ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		אם (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			החזר false;
		}
	}

	אם (!urldb_iterate_entries_host(parent->right,
					url_callback,
					cookie_callback)) {
		החזר false;
	}

	החזר true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ساكن بنية host_part *
urldb_add_host_node(ثابت حرف *part, بنية host_part *parent)
{
	بنية host_part *d;

	أكد(part && parent);

	d = calloc(1, حجم(بنية host_part));
	אם (!d) {
		החזר عدم;
	}

	d->part = strdup(part);
	אם (!d->part) {
		حرر(d);
		החזר عدم;
	}

	d->next = parent->children;
	אם (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	החזר d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ساكن صحيح urldb_add_path_fragment_cmp(ثابت فراغ *a, ثابت فراغ *b)
{
	החזר strcasecmp(*((ثابت حرف **) a), *((ثابت حرف **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ساكن بنية path_data *
urldb_add_path_fragment(بنية path_data *segment, lwc_string *fragment)
{
	حرف **temp;

	أكد(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	אם (!fragment)
		החזר segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * حجم(حرف *));
	אם (!temp)
		החזר عدم;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	אם (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		החזר عدم;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      حجم (حرف *),
	      urldb_add_path_fragment_cmp);

	החזר segment;
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
ساكن بنية path_data *
urldb_add_path_node(lwc_string *scheme,
		    unsigned صحيح port,
		    ثابت حرف *segment,
		    lwc_string *fragment,
		    بنية path_data *parent)
{
	بنية path_data *d, *e;

	أكد(scheme && segment && parent);

	d = calloc(1, حجم(بنية path_data));
	אם (!d)
		החזר عدم;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	אם (!d->segment) {
		lwc_string_unref(d->scheme);
		حرر(d);
		החזר عدم;
	}

	אם (fragment) {
		אם (!urldb_add_path_fragment(d, fragment)) {
			حرر(d->segment);
			lwc_string_unref(d->scheme);
			حرر(d);
			החזר عدم;
		}
	}

	עבור (e = parent->children; e; e = e->next) {
		אם (strcmp(e->segment, d->segment) > 0)
			اكسر;
	}

	אם (e) {
		d->prev = e->prev;
		d->next = e;
		אם (e->prev)
			e->prev->next = d;
		אחרת
			parent->children = d;
		e->prev = d;
	} אחרת אם (!parent->children) {
		d->prev = d->next = عدم;
		parent->children = parent->last = d;
	} אחרת {
		d->next = عدم;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	החזר d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساكن بنية search_node **urldb_get_search_tree_direct(ثابت حرف *host)
{
	أكد(host);

	אם (urldb__host_is_ip_address(host)) {
		החזר &search_trees[ST_IP];
	} אחרת אם (ascii_is_alpha(*host)) {
		החזר &search_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	החזר &search_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساكن بنية search_node *urldb_get_search_tree(ثابت حرف *host)
{
	החזר *urldb_get_search_tree_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ساكن صحيح urldb_search_match_string(ثابت بنية host_part *a, ثابت حرف *b)
{
	ثابت حرف *end, *dot;
	صحيح plen, ret;

	أكد(a && a != &db_root && b);

	אם (urldb__host_is_ip_address(b)) {
		/* IP address */
		החזר strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	כלעוד (b < end && a && a != &db_root) {
		dot = strchr(b, '.');
		אם (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		אם ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			החזר ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		אם (plen > dot - b) {
			/* len(a) > len(b) */
			החזר 1;
		} אחרת אם (plen < dot - b) {
			/* len(a) < len(b) */
			החזר -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	אם (a && a != &db_root && b >= end) {
		/* len(a) > len(b) */
		החזר 1;
	} אחרת אם ((!a || a == &db_root) && b < end) {
		/* len(a) < len(b) */
		החזר -1;
	}

	/* Identical */
	החזר 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ساكن ثابت بنية host_part *
urldb_search_find(بنية search_node *root, ثابت حرف *host)
{
	صحيح c;

	أكد(root && host);

	אם (root == &empty) {
		החזר عدم;
	}

	c = urldb_search_match_string(root->data, host);

	אם (c > 0) {
		החזר urldb_search_find(root->left, host);
	} אחרת אם (c < 0) {
		החזר urldb_search_find(root->right, host);
	}

	החזר root->data;
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
ساكن بنية path_data *
urldb_match_path(ثابت بنية path_data *parent,
		 ثابت حرف *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	ثابت بنية path_data *p;
	ثابت حرف *slash;
	bool match;

	أكد(parent != عدم);
	أكد(parent->segment == عدم);

	אם (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	أكد(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	כלעוד (p != عدم) {
		slash = strchr(path + 1, '/');
		אם (!slash) {
			slash = path + strlen(path);
		}

		אם (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			אם (*slash == '\0') {
				/* Complete match */
				החזר (بنية path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} אחרת {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	החזר عدم;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ساكن بنية path_data *urldb_find_url(nsurl *url)
{
	ثابت بنية host_part *h;
	بنية path_data *p;
	بنية search_node *tree;
	حرف *plq;
	ثابت حرف *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned صحيح port_int;
	bool match;

	أكد(url);

	אם (url_bloom != عدم) {
		אם (bloom_search_hash(url_bloom, nsurl_hash(url)) == false) {
			החזר عدم;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	אם (scheme == عدم)
		החזר عدم;

	אם (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		החזר عدم;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	אם (host != عدم) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} אחרת אם (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} אחרת {
		lwc_string_unref(scheme);
		החזר عدم;
	}

	tree = urldb_get_search_tree(host_str);
	h = urldb_search_find(tree, host_str);
	אם (!h) {
		lwc_string_unref(scheme);
		החזר عدم;
	}

	/* generate plq (path, leaf, query) */
	אם (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		החזר عدم;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	אם (port != عدم) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} אחרת {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	حرر(plq);
	lwc_string_unref(scheme);

	החזר p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساكن فراغ urldb_dump_paths(بنية path_data *parent)
{
	ثابت بنية path_data *p = parent;
	unsigned صحيح i;

	עשה {
		אם (p->segment != عدم) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			עבור (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		אם (p->children != عدم) {
			p = p->children;
		} אחרת {
			כלעוד (p != parent) {
				אם (p->next != عدم) {
					p = p->next;
					اكسر;
				}

				p = p->parent;
			}
		}
	} כלעוד (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساكن فراغ urldb_dump_hosts(بنية host_part *parent)
{
	بنية host_part *h;

	אם (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	עבור (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ساكن فراغ urldb_dump_search(بنية search_node *parent, صحيح depth)
{
	ثابت بنية host_part *h;
	صحيح i; /* index into string */
	حرف s[1024];
	صحيح r;
	صحيح sl = حجم(s) - 2;

	אם (parent == &empty)
		החזר;

	urldb_dump_search(parent->left, depth + 1);

	עבור (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	עבור (h = parent->data; h; h = h->parent) {
		אם (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			אם (r < 0) {
				اكسر;
			}
			אם ((i + r) >= sl) {
				اكسر;
			}
			i += r;
		}

		אם (h->parent && h->parent->parent) {
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
ساكن صحيح
urldb_search_match_host(ثابت بنية host_part *a, ثابت بنية host_part *b)
{
	صحيح ret;

	أكد(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	עבור (; a && a != &db_root && b && b != &db_root;
	     a = a->parent, b = b->parent) {
		אם ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			החזר ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	אם (a && a != &db_root && (!b || b == &db_root)) {
		/* len(a) > len(b) */
		החזר 1;
	} אחרת אם ((!a || a == &db_root) && b && b != &db_root) {
		/* len(a) < len(b) */
		החזר -1;
	}

	/* identical */
	החזר 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ساكن بنية search_node *urldb_search_skew(بنية search_node *root)
{
	أكد(root);

	אם (root->left->level == root->level) {
		بنية search_node *temp;

		temp = root->left;
		root->left = temp->right;
		temp->right = root;
		root = temp;
	}

	החזר root;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ساكن بنية search_node *urldb_search_split(بنية search_node *root)
{
	أكد(root);

	אם (root->right->right->level == root->level) {
		بنية search_node *temp;

		temp = root->right;
		root->right = temp->left;
		temp->left = root;
		root = temp;

		root->level++;
	}

	החזר root;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ساكن بنية search_node *
urldb_search_insert_internal(بنية search_node *root, بنية search_node *n)
{
	أكد(root && n);

	אם (root == &empty) {
		root = n;
	} אחרת {
		صحيح c = urldb_search_match_host(root->data, n->data);

		אם (c > 0) {
			root->left = urldb_search_insert_internal(
				root->left, n);
		} אחרת אם (c < 0) {
			root->right = urldb_search_insert_internal(
				root->right, n);
		} אחרת {
			/* exact match */
			حرر(n);
			החזר root;
		}

		root = urldb_search_skew(root);
		root = urldb_search_split(root);
	}

	החזר root;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ساكن بنية search_node *
urldb_search_insert(بنية search_node *root, ثابت بنية host_part *data)
{
	بنية search_node *n;

	أكد(root && data);

	n = malloc(حجم(بنية search_node));
	אם (!n)
		החזר عدم;

	n->level = 1;
	n->data = data;
	n->left = n->right = &empty;

	root = urldb_search_insert_internal(root, n);

	החזר root;
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
ساكن bool
urldb_parse_avpair(بنية cookie_internal_data *c,
		   حرف *n,
		   حرف *v,
		   bool was_quoted)
{
	صحيح vlen;

	أكد(c && n && v);

	/* Strip whitespace from start of name */
	עבור (; *n; n++) {
		אם (*n != ' ' && *n != '\t')
			اكسر;
	}

	/* Strip whitespace from end of name */
	עבור (vlen = strlen(n); vlen; vlen--) {
		אם (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		אחרת
			اكسر;
	}

	/* Strip whitespace from start of value */
	עבור (; *v; v++) {
		אם (*v != ' ' && *v != '\t')
			اكسر;
	}

	/* Strip whitespace from end of value */
	עבור (vlen = strlen(v); vlen; vlen--) {
		אם (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		אחרת
			اكسر;
	}

	אם (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		אם (!c->comment)
			החזר false;
	} אחרת אם (!c->domain && strcasecmp(n, "Domain") == 0) {
		אם (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			אם (!c->domain)
				החזר false;
		}
	} אחרת אם (strcasecmp(n, "Max-Age") == 0) {
		صحيح temp = atoi(v);
		אם (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		אחרת
			c->expires = time(عدم) + temp;
	} אחרת אם (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		אם (!c->path)
			החזר false;
	} אחרת אם (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} אחרת אם (strcasecmp(n, "Expires") == 0) {
		حرف *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		עבור (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		אם (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} אחרת אם (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} אחרת אם (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} אחרת אם (!c->name) {
		c->name = strdup(n);
		c->value = strdup(v);
		c->value_was_quoted = was_quoted;
		אם (!c->name || !c->value) {
			החזר false;
		}
	}

	החזר true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ساكن فراغ urldb_free_cookie(بنية cookie_internal_data *c)
{
	أكد(c);

	حرر(c->comment);
	حرر(c->domain);
	حرر(c->path);
	حرر(c->name);
	حرر(c->value);
	حرر(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ساكن بنية cookie_internal_data *
urldb_parse_cookie(nsurl *url, ثابت حرف **cookie)
{
	بنية cookie_internal_data *c;
	ثابت حرف *cur;
	حرف name[1024], value[4096];
	حرف *n = name, *v = value;
	bool in_value = false;
	bool had_value_data = false;
	bool value_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	أكد(url && cookie && *cookie);

	c = calloc(1, حجم(بنية cookie_internal_data));
	אם (c == عدم)
		החזר عدم;

	c->expires = -1;

	name[0] = '\0';
	value[0] = '\0';

	עבור (cur = *cookie; *cur; cur++) {
		אם (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			אם (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				value[0] = '\0';

			}

			اكسر;
		} אחרת אם (*cur == '\r') {
			/* Spurious linefeed */
			استمر;
		} אחרת אם (*cur == '\n') {
			/* Spurious newline */
			استمر;
		}

		אם (in_value && !had_value_data) {
			אם (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				استمر;
			} אחרת {
				had_value_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				אם (*cur != '"') {
					value_verbatim = true;
				}
			}
		}

		אם (in_value && !value_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			אם (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				استمر;
			}
		}

		אם (!quoted && !in_value && *cur == '=') {
			/* First equals => attr-value separator */
			in_value = true;
			استمر;
		}

		אם (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			אם (!urldb_parse_avpair(c, name, value, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				החזר عدم;
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
			אם (*cur != ';') {
				cur--;
			}

			استمر;
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
		אם (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ثابت حرف *p;
			ثابت حرف *semi = strchr(cur + 1, ';');
			אם (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			עבור (p = cur + 1; p < semi; p++)
				אם (*p == '=')
					اكسر;

			אם (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} אחרת {
				/* found one => comma marks end of cookie */
				cur++;
				اكسر;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		אם (!in_value) {
			אם (n < name + (حجم(name) - 1))
				*n++ = *cur;
		} אחרת {
			אם (v < value + (حجم(value) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	אם (!urldb_parse_avpair(c, name, value, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		החזר عدم;
	}

	/* Now fix-up default values */
	אם (c->domain == عدم) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		אם (host == عدم) {
			urldb_free_cookie(c);
			החזר عدم;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	אם (c->path == عدم) {
		ثابت حرف *path_data;
		حرف *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		אם (path_lwc == عدم) {
			urldb_free_cookie(c);
			החזר عدم;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		אם (slash != عدم) {
			/* Special case: retain first slash in path */
			אם (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			אם (slash == عدم) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				החזר عدم;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} אחרת {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			אם (path == عدم) {
				urldb_free_cookie(c);
				החזר عدم;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	החזר c;
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
ساكن بنية path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned صحيح port,
	       ثابت بنية host_part *host,
	       حرف *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	بنية path_data *d, *e;
	حرف *buf = path_query;
	حرف *segment, *slash;
	bool match;

	أكد(scheme && host && url);

	d = (بنية path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	אם (*segment == '/')
		segment++;

	/* Process path segments */
	עשה {
		slash = strchr(segment, '/');
		אם (!slash) {
			/* last segment */
			/* look for existing entry */
			עבור (e = d->children; e; e = e->next)
				אם (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					اكسر;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_node(scheme, port,
						    segment, fragment, d);
			اكسر;
		}

		*slash = '\0';

		/* look for existing entry */
		עבור (e = d->children; e; e = e->next)
			אם (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				اكسر;

		d = e ? e : urldb_add_path_node(scheme, port, segment, عدم, d);
		אם (!d)
			اكسر;

		segment = slash + 1;
	} כלעוד (1);

	حرر(path_query);

	אם (d && !d->url) {
		/* Insert defragmented URL */
		אם (nsurl_defragment(url, &d->url) != NSERROR_OK)
			החזר عدم;
	}

	החזר d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ساكن بنية host_part *urldb_add_host(ثابت حرف *host)
{
	بنية host_part *d = (بنية host_part *) &db_root, *e;
	بنية search_node *s;
	حرف buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	حرف *part;

	أكد(host);

	אם (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		עבור (e = d->children; e; e = e->next)
			אם (strcasecmp(host, e->part) == 0)
				/* found => return it */
				החזר e;

		d = urldb_add_host_node(host, d);

		s = urldb_search_insert(search_trees[ST_IP], d);
		אם (!s) {
			/* failed */
			d = عدم;
		} אחרת {
			search_trees[ST_IP] = s;
		}

		החזר d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, حجم buf);
	buf[حجم buf - 1] = '\0';

	/* Process FQDN segments backwards */
	עשה {
		part = strrchr(buf, '.');
		אם (!part) {
			/* last segment */
			/* Check for existing entry */
			עבור (e = d->children; e; e = e->next)
				אם (strcasecmp(buf, e->part) == 0)
					اكسر;

			אם (e) {
				d = e;
			} אחרת {
				d = urldb_add_host_node(buf, d);
			}

			/* And insert into search tree */
			אם (d) {
				بنية search_node **r;

				r = urldb_get_search_tree_direct(buf);
				s = urldb_search_insert(*r, d);
				אם (!s) {
					/* failed */
					d = عدم;
				} אחרת {
					*r = s;
				}
			}
			اكسر;
		}

		/* Check for existing entry */
		עבור (e = d->children; e; e = e->next)
			אם (strcasecmp(part + 1, e->part) == 0)
				اكسر;

		d = e ? e : urldb_add_host_node(part + 1, d);
		אם (!d)
			اكسر;

		*part = '\0';
	} כלעוד (1);

	החזר d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ساكن bool
urldb_insert_cookie(بنية cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	بنية cookie_internal_data *d;
	ثابت بنية host_part *h;
	بنية path_data *p;
	time_t now = time(عدم);

	أكد(c);

	אם (c->domain[0] == '.') {
		h = urldb_search_find(
			urldb_get_search_tree(&(c->domain[1])),
			c->domain + 1);
		אם (!h) {
			h = urldb_add_host(c->domain + 1);
			אם (!h) {
				urldb_free_cookie(c);
				החזר false;
			}
		}

		p = (بنية path_data *) &h->paths;
	} אחרת {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		أكد(url != عدم);
		أكد(scheme != عدم);

		h = urldb_search_find(
			urldb_get_search_tree(c->domain),
			c->domain);

		אם (!h) {
			h = urldb_add_host(c->domain);
			אם (!h) {
				urldb_free_cookie(c);
				החזר false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), عدم, url);
		אם (!p) {
			urldb_free_cookie(c);
			החזר false;
		}
	}

	/* add cookie */
	עבור (d = p->cookies; d; d = d->next) {
		אם (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			اكسر;
	}

	אם (d) {
		אם (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			אם (d->next)
				d->next->prev = d->prev;
			אחרת
				p->cookies_end = d->prev;
			אם (d->prev)
				d->prev->next = d->next;
			אחרת
				p->cookies = d->next;

			cookie_manager_remove((بنية cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} אחרת {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			אם (c->next)
				c->next->prev = c;
			אחרת
				p->cookies_end = c;
			אם (c->prev)
				c->prev->next = c;
			אחרת
				p->cookies = c;

			cookie_manager_remove((بنية cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((بنية cookie_data *)c);
		}
	} אחרת {
		c->prev = p->cookies_end;
		c->next = عدم;
		אם (p->cookies_end)
			p->cookies_end->next = c;
		אחרת
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((بنية cookie_data *)c);
	}

	החזר true;
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
ساكن bool
concat_netscape_cookie(بنية cookie_internal_data *c,
		    صحيح *used,
		    صحيح *alloc,
		    حرف **buf)
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
	صحيح max_len;
	صحيح outl; /* output length */

	أكد(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->value) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	אם (max_len >= *alloc) {
		حرف *temp = realloc(*buf, max_len);
		אם (!temp) {
			החזר false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	אם (outl >= (*alloc - *used)) {
		/* output truncated */
		החזר false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	אם (c->value_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);
		אם (outl >= (*alloc - *used)) {
			/* output truncated */
			החזר false;
		}
		*used += outl;
	} אחרת {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->value);
		אם (outl >= (*alloc - *used)) {
			/* output truncated */
			החזר false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	החזר true;
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
ساكن bool
concat_rfc2965_cookie(بنية cookie_internal_data *c,
		      صحيح *used,
		      صحيح *alloc,
		      حرف **buf)
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
	ثابت حرف * ثابت separators = "()<>@,;:\\\"/[]?={} \t";
	صحيح max_len;
	صحيح outl; /* output length */

	أكد(c && used && alloc && buf && *buf);

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

	אם (max_len >= *alloc) {
		حرف *temp = realloc(*buf, max_len);
		אם (!temp) {
			החזר false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	אם (outl >= (*alloc - *used)) {
		/* output truncated */
		החזר false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	אם (c->value_was_quoted ||
	    strpbrk(c->value, separators) != عدم) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->value);

	} אחרת {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->value);
	}
	אם (outl >= (*alloc - *used)) {
		/* output truncated */
		החזר false;
	}
	*used += outl;

	אם (c->path_from_set) {
		/* Path, quoted if necessary */

		אם (strpbrk(c->path, separators) != عدم) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} אחרת {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		אם (outl >= (*alloc - *used)) {
			/* output truncated */
			החזר false;
		}
		*used += outl;
	}

	אם (c->domain_from_set) {
		/* Domain, quoted if necessary */
		אם (strpbrk(c->domain, separators) != عدم) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} אחרת {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		אם (outl >= (*alloc - *used)) {
			/* output truncated */
			החזר false;
		}
		*used += outl;
	}

	החזר true;
}


/**
 * build output cookie string in netscape format
 */
ساكن حرف *
get_netscape_cookie(بنية cookie_internal_data **matched_cookies, صحيح count)
{
	صحيح ret_alloc = 4096, ret_used = 1;
	حرف *ret;
	صحيح midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	אם (ret == عدم) {
		החזר عدم;
	}
	ret[0] = '\0';

	עבור (midx = 0; midx < count; midx++) {
		אם (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			حرر(ret);
			החזר عدم;
		}
	}

	אם (ret_used <= 2) {
		حرر(ret);
		החזר عدم;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	حرف *temp = realloc(ret, ret_used);
	אם (temp != عدم) {
		ret = temp;
	}

	החזר ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ساكن حرف *
get_rfc2965_cookie(بنية cookie_internal_data **matched_cookies,
		   صحيح count,
		   صحيح version)
{
	صحيح ret_alloc = 4096; /* return string allocation */
	صحيح ret_used = 1; /* return sting buffer used */
	حرف *ret; /* return string buffer */
	صحيح midx; /* index into matched cookies */
	صحيح outl;

	ret = malloc(ret_alloc);
	אם (ret == عدم) {
		החזר عدم;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	אם (outl >= ret_alloc - ret_used) {
		/* overflow */
		حرر(ret);
		החזר عدم;
	}
	ret_used += outl;

	עבור (midx = 0; midx < count; midx++) {
		אם (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			حرر(ret);
			החזר عدم;
		}
	}

	/* shrink the output buffer to the required size */
	حرف *temp = realloc(ret, ret_used);
	אם (temp != عدم) {
		ret = temp;
	}

	החזר ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ساكن فراغ
urldb_delete_cookie_paths(ثابت حرف *domain,
			  ثابت حرف *path,
			  ثابت حرف *name,
			  بنية path_data *parent)
{
	بنية cookie_internal_data *c;
	بنية path_data *p = parent;

	أكد(parent);

	עשה {
		עבור (c = p->cookies; c; c = c->next) {
			אם (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				אם (c->prev) {
					c->prev->next = c->next;
				} אחרת {
					p->cookies = c->next;
				}

				אם (c->next) {
					c->next->prev = c->prev;
				} אחרת {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				החזר;
			}
		}

		אם (p->children) {
			p = p->children;
		} אחרת {
			כלעוד (p != parent) {
				אם (p->next != عدم) {
					p = p->next;
					اكسر;
				}

				p = p->parent;
			}
		}
	} כלעוד (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ساكن فراغ
urldb_delete_cookie_hosts(ثابت حرف *domain,
			  ثابت حرف *path,
			  ثابت حرف *name,
			  بنية host_part *parent)
{
	بنية host_part *h;
	أكد(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	עבור (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ساكن فراغ urldb_save_cookie_paths(FILE *fp, بنية path_data *parent)
{
	بنية path_data *p = parent;
	time_t now = time(عدم);

	أكد(fp && parent);

	עשה {
		אם (p->cookies != عدم) {
			بنية cookie_internal_data *c;

			עבור (c = p->cookies; c != عدم; c = c->next) {
				אם (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					استمر;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(صحيح)c->expires, (صحيح)c->last_used,
					c->no_destroy, c->name, c->value,
					c->value_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		אם (p->children != عدم) {
			p = p->children;
		} אחרת {
			כלעוד (p != parent) {
				אם (p->next != عدم) {
					p = p->next;
					اكسر;
				}

				p = p->parent;
			}
		}
	} כלעוד (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ساكن فراغ urldb_save_cookie_hosts(FILE *fp, بنية host_part *parent)
{
	بنية host_part *h;
	أكد(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	עבור (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ساكن فراغ urldb_destroy_cookie(بنية cookie_internal_data *c)
{
	حرر(c->name);
	حرر(c->value);
	حرر(c->comment);
	حرر(c->domain);
	حرر(c->path);

	حرر(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ساكن فراغ urldb_destroy_path_node_content(بنية path_data *node)
{
	بنية cookie_internal_data *a, *b;
	unsigned صحيح i;

	אם (node->url != عدم) {
		nsurl_unref(node->url);
	}

	lwc_string_unref(node->scheme);

	حرر(node->segment);
	עבור (i = 0; i < node->frag_cnt; i++)
		حرر(node->fragment[i]);
	حرر(node->fragment);

	حرر(node->urld.title);

	עבור (a = node->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ساكن فراغ urldb_destroy_prot_space(بنية prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	حرر(space->realm);
	حرر(space->auth);

	حرر(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ساكن فراغ urldb_destroy_path_tree(بنية path_data *root)
{
	بنية path_data *p = root;

	עשה {
		אם (p->children != عدم) {
			p = p->children;
		} אחרת {
			بنية path_data *q = p;

			כלעוד (p != root) {
				אם (p->next != عدم) {
					p = p->next;
					اكسر;
				}

				p = p->parent;

				urldb_destroy_path_node_content(q);
				حرر(q);

				q = p;
			}

			urldb_destroy_path_node_content(q);
			حرر(q);
		}
	} כלעוד (p != root);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ساكن فراغ urldb_destroy_host_tree(بنية host_part *root)
{
	بنية host_part *a, *b;
	بنية path_data *p, *q;
	بنية prot_space_data *s, *t;

	/* Destroy children */
	עבור (a = root->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}

	/* Now clean up paths */
	עבור (p = root->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_tree(p);
	}

	/* Root path */
	urldb_destroy_path_node_content(&root->paths);

	/* Proctection space data */
	עבור (s = root->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	حرر(root->part);
	حرر(root);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ساكن فراغ urldb_destroy_search_tree(بنية search_node *root)
{
	/* Destroy children */
	אם (root->left != &empty)
		urldb_destroy_search_tree(root->left);
	אם (root->right != &empty)
		urldb_destroy_search_tree(root->right);

	/* And destroy ourselves */
	حرر(root);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
فراغ urldb_destroy(فراغ)
{
	بنية host_part *a, *b;
	صحيح i;

	/* Clean up search trees */
	עבור (i = 0; i < NUM_SEARCH_TREES; i++) {
		אם (search_trees[i] != &empty) {
			urldb_destroy_search_tree(search_trees[i]);
			search_trees[i] = &empty;
		}
	}

	/* And database */
	עבור (a = db_root.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_tree(a);
	}
	memset(&db_root, 0, حجم(db_root));

	/* And the bloom filter */
	אם (url_bloom != عدم) {
		bloom_destroy(url_bloom);
		url_bloom = عدم;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(ثابت حرف *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	حرف s[MAXIMUM_URL_LENGTH];
	حرف host[256];
	بنية host_part *h;
	صحيح urls;
	صحيح i;
	صحيح version;
	صحيح length;
	FILE *fp;

	أكد(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	אם (url_bloom == عدم)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	אם (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		החזר NSERROR_NOT_FOUND;
	}

	אם (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		החזר NSERROR_NEED_DATA;
	}

	version = atoi(s);
	אם (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		החזר NSERROR_INVALID;
	}
	אם (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		החזר NSERROR_INVALID;
	}

	כלעוד (fgets(host, حجم host, fp)) {
		time_t hsts_expiry = 0;
		صحيح hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		אם (length == 0) {
			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;
			urls = atoi(s);
			/* Eight fields/url */
			עבור (i = 0; i < (8 * urls); i++) {
				אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					اكسر;
			}
			استمر;
		}

		אם (version >= 107) {
			حرف *p = host;
			כלעוד (*p && *p != ' ') p++;
			כלעוד (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			כלעוד (*p && *p != ' ') p++;
			כלעוד (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		אם (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			החזר NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			اكسر;
		urls = atoi(s);

		/* no URLs => try next host */
		אם (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			استمر;
		}

		/* load the non-corrupt data */
		עבור (i = 0; i < urls; i++) {
			بنية path_data *p = عدم;
			حرف scheme[64], ports[10];
			حرف url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned صحيح port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			حرف *path_query;
			size_t len;

			אם (!fgets(scheme, حجم scheme, fp))
				اكسر;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			אם (!fgets(ports, حجم ports, fp))
				اكسر;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;
			length = strlen(s) - 1;
			s[length] = '\0';

			אם (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, حجم url, "%s://%s%s%s%s",
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
			אם (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				החזר NSERROR_NOMEM;
			}

			אם (url_bloom != عدم) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_insert_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			אם (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				החזר NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			אם (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				החזר NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;
			אם (p)
				p->urld.visits = (unsigned صحيح)atoi(s);

			/* entry last use time */
			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				اكسر;
			}
			אם (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;
			אם (p)
				p->urld.type = (content_type)atoi(s);

			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;


			אם (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				اكسر;
			length = strlen(s) - 1;
			אם (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				אם (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	החזר NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ثابت حرف *filename)
{
	FILE *fp;
	صحيح i;

	أكد(filename);

	fp = fopen(filename, "w");
	אם (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		החזר NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	עבור (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_save_search_tree(search_trees[i], fp);
	}

	fclose(fp);

	החזר NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	بنية path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p) {
		החזר NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	החזר NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	بنية host_part *h;
	بنية path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	ثابت حرف *host_str;
	حرف *path_query = عدم;
	size_t len;
	bool match;
	unsigned صحيح port_int;

	أكد(url);

	אם (url_bloom == عدم)
		url_bloom = bloom_create(BLOOM_SIZE);

	אם (url_bloom != عدم) {
		uint32_t hash = nsurl_hash(url);
		bloom_insert_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	אם (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		החזר false;
	}
	أكد(path_query != عدم);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	אם (scheme == عدم) {
		حرر(path_query);
		החזר false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	אם (host != عدم) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} אחרת אם (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} אחרת {
		lwc_string_unref(scheme);
		حرر(path_query);
		החזר false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	אם (port != عدم) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} אחרת {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	אם (h != عدم) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} אחרת {
		p = عدم;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	החזר (p != عدم);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, ثابت حرف *title)
{
	بنية path_data *p;
	حرف *temp;

	أكد(url);

	p = urldb_find_url(url);
	אם (p == عدم) {
		החזר NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	אם (title != عدم) {
		temp = strdup(title);
		אם (temp == عدم) {
			החזר NSERROR_NOMEM;
		}
	} אחרת {
		temp = عدم;
	}

	حرر(p->urld.title);
	p->urld.title = temp;

	החזר NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	بنية path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p) {
		החזר NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	החזר NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	بنية path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p) {
		החזר NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(عدم);
	p->urld.visits++;

	החזר NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
فراغ urldb_reset_url_visit_data(nsurl *url)
{
	بنية path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ثابت بنية url_data *urldb_get_url_data(nsurl *url)
{
	بنية path_data *p;
	بنية url_internal_data *u;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר عدم;

	u = &p->urld;

	החזר (ثابت بنية url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	بنية path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר عدم;

	החזר p->url;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_set_auth_details(nsurl *url, ثابت حرف *realm, ثابت حرف *auth)
{
	بنية path_data *p, *pi;
	بنية host_part *h;
	بنية prot_space_data *space, *space_alloc;
	حرف *realm_alloc, *auth_alloc;
	bool match;

	أكد(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	אם (!p)
		החזר;

	/* Search for host_part */
	עבור (pi = p; pi->parent != عدم; pi = pi->parent)
		;
	h = (بنية host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	עבור (space = h->prot_space; space; space = space->next) {
		אם (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			اكسر;
	}

	אם (space != عدم) {
		/* Overrule existing auth. */
		حرر(space->auth);
		space->auth = strdup(auth);
	} אחרת {
		/* Create a new protection space. */
		space = space_alloc = malloc(حجم(بنية prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		אם (!space_alloc || !realm_alloc || !auth_alloc) {
			حرر(space_alloc);
			حرر(realm_alloc);
			حرر(auth_alloc);
			החזר;
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
ثابت حرف *urldb_get_auth_details(nsurl *url, ثابت حرف *realm)
{
	بنية path_data *p, *p_cur, *p_top;

	أكد(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר عدم;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	עבור (p_cur = p; p_cur != عدم; p_top = p_cur, p_cur = p_cur->parent) {
		אם (p_cur->prot_space) {
			החזר p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	אם (realm != عدم) {
		ثابت بنية host_part *h = (ثابت بنية host_part *)p_top;
		ثابت بنية prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		עבור (space = h->prot_space; space != عدم;
		     space = space->next) {
			אם (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				החזר p->prot_space->auth;
			}
		}
	}

	החזר عدم;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_set_cert_permissions(nsurl *url, bool permit)
{
	بنية path_data *p;
	بنية host_part *h;

	أكد(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר;

	עבור (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (بنية host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	بنية path_data *p;
	ثابت بنية host_part *h;

	أكد(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר false;

	עבור (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ثابت بنية host_part *)p;

	החזר h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(بنية nsurl *url, ثابت حرف *header)
{
	بنية path_data *p;
	بنية host_part *h;
	lwc_string *host;
	time_t now = time(عدم);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	אם (host != عدم) {
		אם (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			החזר true;
		} אחרת אם (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			החזר true;
		}

		lwc_string_unref(host);
	} אחרת {
		/* No host part: ignore */
		החזר true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר false;

	עבור (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (بنية host_part *)p;
	אם (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		החזר true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	אם (error != NSERROR_OK) {
		/* Parse failed: ignore */
		החזר true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	אם (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} אחרת אם ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	החזר true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(بنية nsurl *url)
{
	بنية path_data *p;
	ثابت بنية host_part *h;
	lwc_string *host;
	time_t now = time(عدم);

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	אם (host != عدم) {
		אם (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			החזר false;
		} אחרת אם (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			החזר false;
		}

		lwc_string_unref(host);
	} אחרת {
		/* No host part: not enabled */
		החזר false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר false;

	עבור (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ثابت بنية host_part *)p;

	/* Consult record for this host */
	אם (h->hsts.expires > now) {
		/* Not expired */
		החזר true;
	}

	/* Consult parent domains */
	עבור (h = h->parent; h && h != &db_root; h = h->parent) {
		אם (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			החזר true;
		}
	}

	החזר false;
}


/* exported interface documented in netsurf/url_db.h */
فراغ
urldb_iterate_partial(ثابت حرف *prefix,
		      bool (*callback)(nsurl *url, ثابت بنية url_data *data))
{
	حرف host[256];
	حرف buf[260]; /* max domain + "www." */
	ثابت حرف *slash, *scheme_sep;
	بنية search_node *tree;
	ثابت بنية host_part *h;

	أكد(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	אם (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	tree = urldb_get_search_tree(prefix);

	אם (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, حجم host, "%.*s",
			 (صحيح) (slash - prefix), prefix);

		h = urldb_search_find(tree, host);
		אם (!h) {
			صحيح len = slash - prefix;

			אם (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, حجم buf, "www.%s", host);
				h = urldb_search_find(
					search_trees[ST_DN + 'w' - 'a'],
					buf);
				אם (!h)
					החזר;
			} אחרת
				החזר;
		}

		אם (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} אחרת {
		صحيح len = strlen(prefix);

		/* looking for hosts */
		אם (!urldb_iterate_partial_host(tree, prefix, callback))
			החזר;

		אם (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, حجم buf, "www.%s", prefix);
			אם(!urldb_iterate_partial_host(
				   search_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				החזר;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
فراغ
urldb_iterate_entries(bool (*callback)(nsurl *url, ثابت بنية url_data *data))
{
	صحيح i;

	أكد(callback);

	עבור (i = 0; i < NUM_SEARCH_TREES; i++) {
		אם (!urldb_iterate_entries_host(search_trees[i],
						callback,
						عدم)) {
			اكسر;
		}
	}
}


/* exported interface documented in content/urldb.h */
فراغ urldb_iterate_cookies(bool (*callback)(ثابت بنية cookie_data *data))
{
	صحيح i;

	أكد(callback);

	עבור (i = 0; i < NUM_SEARCH_TREES; i++) {
		אם (!urldb_iterate_entries_host(search_trees[i],
						عدم, callback))
			اكسر;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(ثابت حرف *header, nsurl *url, nsurl *referer)
{
	ثابت حرف *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	أكد(url && header);

	/* Get defragmented URL, as 'urlt' */
	אם (nsurl_defragment(url, &urlt) != NSERROR_OK)
		החזר عدم;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	אם (scheme == عدم) {
		nsurl_unref(urlt);
		החזר false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	אם (path == عدم) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		החזר false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	אם (host == عدم) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		החזר false;
	}

	אם (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		אם (rhost == عدم) {
			اذهب error;
		}

		/* Domain match host names */
		אם (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			ثابت حرف *hptr;
			ثابت حرف *rptr;
			ثابت حرف *dot;
			ثابت حرف *host_data = lwc_string_data(host);
			ثابت حرف *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			אם (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				اذهب error;
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
			כלעוד (hptr >= host_data && rptr >= rhost_data) {
				אם (*hptr != *rptr)
					اكسر;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			כלעוד (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			אם (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == عدم ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				اذهب error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	עשה {
		بنية cookie_internal_data *c;
		حرف *dot;
		size_t len;
#ifdef WITH_NSPSL
		ثابت حرف *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		אם (!c) {
			/* failed => stop parsing */
			اذهب error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		אם (!c->name || !c->value) {
			urldb_free_cookie(c);
			اذهب error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		אם (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			اذهب error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		אם (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		אם (suffix == عدم) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			اذهب error;
		}
#אחרת
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		אם (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			اذهب error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		אם (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			صحيح hlen, dlen;
			حرف *domain = c->domain;

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
			أكد(c->domain[0] == '.');

			/* 4.3.2:iii */
			אם (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				اذهب error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			אם (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				اذهب error;
			}

			אם (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			אם (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				اذهب error;
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
		אם (!urldb_insert_cookie(c, scheme, urlt))
			اذهب error;
	} כלעוד (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	החזר true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	החזר false;
}


/* exported interface documented in content/urldb.h */
حرف *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ثابت بنية path_data *p, *q;
	ثابت بنية host_part *h;
	lwc_string *path_lwc;
	بنية cookie_internal_data *c;
	صحيح count = 0, version = COOKIE_RFC2965;
	بنية cookie_internal_data **matched_cookies;
	صحيح matched_cookies_size = 20;
	ثابت حرف *path;
	حرف *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	أكد(url != عدم);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	אם (!p)
		החזר عدم;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 حجم(بنية cookie_internal_data *));
	אם (!matched_cookies)
		החזר عدم;

#define GROW_MATCHED_COOKIES						\
	עשה {								\
		אם (count == matched_cookies_size) {			\
			بنية cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       حجم(بنية cookie_internal_data *)); \
									\
			אם (temp == عدم) {				\
				حرر(matched_cookies);			\
				החזר عدم;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} כלעוד(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	אם (path_lwc == عدم) {
		حرر(matched_cookies);
		החזר عدم;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(عدم);

	אם (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		עבור (q = p->parent->children; q; q = q->next) {
			אם (strcmp(q->segment, p->segment))
				استمر;

			/* Consider all cookies associated with
			 * this exact path */
			עבור (c = q->cookies; c; c = c->next) {
				אם (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					استمر;

				אם (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					استمر;

				אם (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					استمر;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				אם (c->version < (unsigned صحيح)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((بنية cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	עבור (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		עבור (q = p->children; q; q = q->next) {
			אם (*(q->segment) != '\0')
				استمر;

			עבור (c = q->cookies; c; c = c->next) {
				אם (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					استمر;

				אם (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					استمر;

				matched_cookies[count++] = c;

				GROW_MATCHED_COOKIES;

				אם (c->version < (unsigned صحيح) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((بنية cookie_data *)c);
			}
		}

		אם (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			اكسر;
		}

		/* Consider p itself - may be the result of Path=/foo */
		עבור (c = p->cookies; c; c = c->next) {
			אם (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				استمر;

			/* Ensure cookie path is a prefix of the resource */
			אם (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				استمر;

			אם (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				استمر;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			אם (c->version < (unsigned صحيح) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((بنية cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	עבור (h = (ثابت بنية host_part *)p; h && h != &db_root;
	     h = h->parent) {
		עבור (c = h->paths.cookies; c; c = c->next) {
			אם (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				استمر;

			/* Ensure cookie path is a prefix of the resource */
			אם (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				استمر;

			אם (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				استمر;

			matched_cookies[count++] = c;

			GROW_MATCHED_COOKIES;

			אם (c->version < (unsigned صحيح)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((بنية cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	אם (count == 0) {
		/* No cookies found */
		حرر(matched_cookies);
		החזר عدم;
	}

	אם (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, count);
	} אחרת {
		ret = get_rfc2965_cookie(matched_cookies, count, version);
	}
	حرر(matched_cookies);

	החזר ret;
}


/* exported interface documented in content/urldb.h */
فراغ urldb_delete_cookie(ثابت حرف *domain, ثابت حرف *path,
			 ثابت حرف *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_root);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(ثابت حرف *filename)
{
	FILE *fp;
	حرف s[16*1024];

	أكد(filename);

	fp = fopen(filename, "r");
	אם (!fp)
		החזר NSERROR_NOT_FOUND;

#define FIND_T {				\
		עבור (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		אם (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			استمر;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		עבור (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		אם (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			استمر;		\
		}				\
	}

	כלעוד (fgets(s, حجم s, fp)) {
		حرف *p = s, *end = 0,
			*domain, *path, *name, *value, *scheme, *url,
			*comment;
		صحيح version, domain_specified, path_specified,
			secure, http_only, no_destroy, value_quoted;
		time_t expires, last_used;
		بنية cookie_internal_data *c;

		אם(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			استمر;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		אם (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			אם (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				اكسر;
			}

			استمر;
		} אחרת אם (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			استمر;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		אם (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} אחרת {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; value = p; FIND_T;
		אם (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	value_quoted = atoi(p); FIND_T;
		} אחרת {
			value_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		עבור (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		أكد(p <= end);

		/* Now create cookie */
		c = malloc(حجم(بنية cookie_internal_data));
		אם (!c)
			اكسر;

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

		אם (!(c->name && c->value && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			اكسر;
		}

		אם (c->domain[0] != '.') {
			lwc_string *scheme_lwc = عدم;
			nsurl *url_nsurl = عدم;

			أكد(scheme[0] != 'u');

			אם (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				اكسر;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			אם (!urldb_insert_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				اكسر;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} אחרת {
			אם (!urldb_insert_cookie(c, عدم, عدم)) {
				/* Cookie freed for us */
				اكسر;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	החזר NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ثابت حرف *filename)
{
	FILE *fp;
	صحيح cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	أكد(filename);

	fp = fopen(filename, "w");
	אם (!fp)
		החזר NSERROR_NOT_FOUND;

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

	החזר NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_dump(فراغ)
{
	صحيح i;

	urldb_dump_hosts(&db_root);

	עבור (i = 0; i != NUM_SEARCH_TREES; i++) {
		urldb_dump_search(search_trees[i], 0);
	}
}
