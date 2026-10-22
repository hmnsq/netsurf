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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#ifdef WITH_NSPSL
#شامل <nspsl.h>
#endif

#شامل "utils/inet.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/corestrings.h"
#شامل "utils/url.h"
#شامل "utils/utils.h"
#شامل "utils/bloom.h"
#شامل "utils/time.h"
#شامل "utils/nsurl.h"
#شامل "utils/ascii.h"
#شامل "utils/http.h"
#شامل "netsurf/bitmap.h"
#شامل "desktop/cookie_manager.h"

#شامل "content/content.h"
#شامل "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
ساختار cookie_internal_data {
	ساختار cookie_internal_data *prev;	/**< Previous in list */
	ساختار cookie_internal_data *next;	/**< Next in list */

	نویسه *name;		/**< Cookie name */
	نویسه *مقدار;		/**< Cookie value */
	bool مقدار_was_quoted;	/**< Value was quoted in Set-Cookie: */
	نویسه *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	نویسه *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	نویسه *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	شمارش cookie_version version;	/**< Specification compliance */
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
ساختار prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned صحیح port;
	/** Protection realm */
	نویسه *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	نویسه *auth;
	/** Next sibling */
	ساختار prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ساختار url_internal_data {
	نویسه *title;		/**< Resource title */
	unsigned صحیح visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ساختار path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned صحیح port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	نویسه *segment;		/**< Path segment for this node */
	unsigned صحیح frag_cnt;	/**< Number of entries in path_data::fragment */
	نویسه **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ساختار url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ثابت ساختار prot_space_data *prot_space;
	/** Cookies associated with resource */
	ساختار cookie_internal_data *cookies;
	/** Last cookie in list */
	ساختار cookie_internal_data *cookies_end;

	ساختار path_data *next;	/**< Next sibling */
	ساختار path_data *prev;	/**< Previous sibling */
	ساختار path_data *parent; /**< Parent path segment */
	ساختار path_data *children; /**< Child path segments */
	ساختار path_data *last; /**< Last child */
};

ساختار hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ساختار host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ساختار path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ساختار hsts_data hsts;

	/**
	 * Part of host string
	 */
	نویسه *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ساختار prot_space_data *prot_space;

	ساختار host_part *next;	/**< Next sibling */
	ساختار host_part *prev;	/**< Previous sibling */
	ساختار host_part *parent; /**< Parent host part */
	ساختار host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ساختار جستجو_گره {
	ثابت ساختار host_part *data;	/**< Host tree entry */

	unsigned صحیح level;		/**< Node level */

	ساختار جستجو_گره *چپ;	/**< Left subtree */
	ساختار جستجو_گره *راست;	/**< Right subtree */
};

/** Root database handle */
ایستا ساختار host_part db_ریشه;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define رقم_جستجو_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
ایستا ساختار جستجو_گره empty = { 0, 0, &empty, &empty };
ایستا ساختار جستجو_گره *جستجو_trees[رقم_جستجو_TREES] = {
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
ایستا صحیح loaded_cookie_file_version;

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
ایستا ساختار bloom_filter *url_bloom;
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
ایستا nserror urldb_write_timet(FILE *fp, time_t val)
{
	صحیح use;
	نویسه op[32];

	use = nsc_sntimet(op, 32, &val);
	اگر (use == 0) {
		fprintf(fp, "%i\n", (صحیح)val);
	} وگرنه {
		fprintf(fp, "%.*s\n", use, op);
	}
	بازگشت NSERROR_OK;
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
ایستا تهی
urldb_write_paths(ثابت ساختار path_data *parent,
		  ثابت نویسه *host,
		  FILE *fp,
		  نویسه **path,
		  صحیح *path_alloc,
		  صحیح *path_used,
		  time_t expiry)
{
	ثابت ساختار path_data *p = parent;
	صحیح i;

	انجامبده {
		صحیح seglen = p->segment != پوچ ? strlen(p->segment) : 0;
		صحیح len = *path_used + seglen + 1;

		اگر (*path_alloc < len) {
			نویسه *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			اگر (!temp) {
				بازگشت;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		اگر (p->segment != پوچ) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		اگر (p->children != پوچ) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} وگرنه {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		اگر (p->children != پوچ) {
			/* Drill down into children */
			p = p->children;
		} وگرنه {
			/* leaf node */
			اگر (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				اگر (p->port) {
					fprintf(fp,"%d\n", p->port);
				} وگرنه {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (صحیح)p->urld.type);

				fprintf(fp, "\n");

				اگر (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					برای (i = 0; s[i] != '\0'; i++)
						اگر (s[i] < 32)
							s[i] = ' ';
					برای (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} وگرنه {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			وقتی (p != parent) {
				صحیح seglen = p->segment != پوچ
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				اگر (p->next != پوچ) {
					/* Have a sibling, process that */
					p = p->next;
					بشکن;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} وقتی (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ایستا تهی
urldb_شمار_urls(ثابت ساختار path_data *ریشه,
		 time_t expiry,
		 unsigned صحیح *شمار)
{
	ثابت ساختار path_data *p = ریشه;

	انجامبده {
		اگر (p->children != پوچ) {
			/* Drill down into children */
			p = p->children;
		} وگرنه {
			/* No more children, increment count if required */
			اگر (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*شمار)++;
			}

			/* Now, find next node to process. */
			وقتی (p != ریشه) {
				اگر (p->next != پوچ) {
					/* Have a sibling, process that */
					p = p->next;
					بشکن;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} وقتی (p != ریشه);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ایستا تهی urldb_save_جستجو_درخت(ساختار جستجو_گره *parent, FILE *fp)
{
	نویسه host[256];
	ثابت ساختار host_part *h;
	unsigned صحیح path_شمار = 0;
	نویسه *path, *p, *end;
	صحیح path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	صحیح hsts_include_subdomains = 0;

	expiry = time(پوچ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	اگر (parent == &empty)
		بازگشت;

	urldb_save_جستجو_درخت(parent->چپ, fp);

	path = malloc(path_alloc);
	اگر (!path)
		بازگشت;

	path[0] = '\0';

	برای (h = parent->data, p = host, end = host + اندازه host;
	     h && h != &db_ریشه && p < end; h = h->parent) {
		صحیح written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		اگر (written < 0) {
			آزاد(path);
			بازگشت;
		}
		p += written;
	}

	h = parent->data;
	اگر (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_شمار_urls(&parent->data->paths, expiry, &path_شمار);

	اگر (path_شمار > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_شمار);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} وگرنه اگر (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	آزاد(path);

	urldb_save_جستجو_درخت(parent->راست, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ایستا bool
urldb_iterate_entries_path(ثابت ساختار path_data *parent,
		bool (*url_callback)(nsurl *url, ثابت ساختار url_data *data),
		bool (*cookie_callback)(ثابت ساختار cookie_data *data))
{
	ثابت ساختار path_data *p = parent;
	ثابت ساختار cookie_data *c;

	انجامبده {
		اگر (p->children != پوچ) {
			/* Drill down into children */
			p = p->children;
		} وگرنه {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			أكد(url_callback || cookie_callback);

			/** \todo handle fragments? */
			اگر (url_callback) {
				ثابت ساختار url_internal_data *u = &p->urld;

				أكد(p->url);

				اگر (!url_callback(p->url,
						  (ثابت ساختار url_data *) u))
					بازگشت false;
			} وگرنه {
				c = (ثابت ساختار cookie_data *)p->cookies;
				برای (; c != پوچ; c = c->next) {
					اگر (!cookie_callback(c))
						بازگشت false;
				}
			}

			/* Now, find next node to process. */
			وقتی (p != parent) {
				اگر (p->next != پوچ) {
					/* Have a sibling, process that */
					p = p->next;
					بشکن;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} وقتی (p != parent);

	بازگشت true;
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
ایستا bool urldb__host_is_ip_address(ثابت نویسه *host)
{
	ساختار in_addr ipv4;
	size_t host_len = strlen(host);
	ثابت نویسه *sane_host;
	ثابت نویسه *slash;
#ifndef NO_IPV6
	ساختار in6_addr ipv6;
	نویسه ipv6_addr[64];
	unsigned صحیح ipv6_addr_len;
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
	اگر (slash == پوچ) {
		sane_host = host;
	} وگرنه {
		نویسه *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	اگر (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		برو out_false;

	اگر (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		صحیح رقم_dots = 0;
		size_t index;

		برای (index = 0; index < host_len; index++) {
			اگر (sane_host[index] == '.')
				رقم_dots++;
		}

		اگر (رقم_dots == 3)
			برو out_true;
		وگرنه
			برو out_false;
	}

#ifndef NO_IPV6
	اگر ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		برو out_false;
	}

	ipv6_addr_len = host_len - 2;
	اگر (ipv6_addr_len >= اندازه(ipv6_addr)) {
		ipv6_addr_len = اندازه(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	اگر (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		برو out_true;
#endif

out_false:
	اگر (slash != پوچ) آزاد((تهی *)sane_host);
	بازگشت false;

out_true:
	اگر (slash != پوچ) آزاد((تهی *)sane_host);
	بازگشت true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ایستا صحیح urldb_جستجو_match_prefix(ثابت ساختار host_part *a, ثابت نویسه *b)
{
	ثابت نویسه *end, *dot;
	صحیح plen, ret;

	أكد(a && a != &db_ریشه && b);

	اگر (urldb__host_is_ip_address(b)) {
		/* IP address */
		بازگشت strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	وقتی (b < end && a && a != &db_ریشه) {
		dot = strchr(b, '.');
		اگر (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		اگر ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			بازگشت ret;

		/* The strings matched */
		اگر (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			اگر (plen > dot - b) {
				/* len(a) > len(b) */
				بازگشت 1;
			} وگرنه اگر (plen < dot - b) {
				/* len(a) < len(b) */
				بازگشت -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_ریشه && b >= end) {
		/* len(a) > len(b) => prefix matches */
		بازگشت 0;
	} وگرنه اگر ((!a || a == &db_ریشه) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		بازگشت -1;
	}

	/* Identical */
	بازگشت 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ایستا bool
urldb_iterate_partial_host(ساختار جستجو_گره *ریشه,
		ثابت نویسه *prefix,
		bool (*callback)(nsurl *url, ثابت ساختار url_data *data))
{
	صحیح c;

	أكد(ریشه && prefix && callback);

	اگر (ریشه == &empty)
		بازگشت true;

	c = urldb_جستجو_match_prefix(ریشه->data, prefix);

	اگر (c > 0) {
		/* No match => look in left subtree */
		بازگشت urldb_iterate_partial_host(ریشه->چپ,
						  prefix,
						  callback);
	} وگرنه اگر (c < 0) {
		/* No match => look in right subtree */
		بازگشت urldb_iterate_partial_host(ریشه->راست,
						  prefix,
						  callback);
	} وگرنه {
		/* Match => iterate over l/r subtrees & process this node */
		اگر (!urldb_iterate_partial_host(ریشه->چپ,
						prefix,
						callback)) {
			بازگشت false;
		}

		اگر (ریشه->data->paths.children) {
			/* and extract all paths attached to this host */
			اگر (!urldb_iterate_entries_path(&ریشه->data->paths,
							callback,
							پوچ)) {
				بازگشت false;
			}
		}

		اگر (!urldb_iterate_partial_host(ریشه->راست,
						prefix,
						callback)) {
			بازگشت false;
		}
	}

	بازگشت true;
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
ایستا bool
urldb_iterate_partial_path(ثابت ساختار path_data *parent,
		ثابت نویسه *prefix,
		bool (*callback)(nsurl *url, ثابت ساختار url_data *data))
{
	ثابت ساختار path_data *p = parent->children;
	ثابت نویسه *slash, *end = prefix + strlen(prefix);

	انجامبده {
		slash = strchr(prefix, '/');
		اگر (!slash) {
			slash = end;
		}

		اگر (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			ادامه;
		}

		اگر (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			اگر (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				اگر (!urldb_iterate_entries_path(p,
								callback,
								پوچ)) {
					بازگشت false;
				}

				/* Progress to next sibling */
				p = p->next;
			} وگرنه {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} وگرنه {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} وقتی (p != پوچ);

	بازگشت true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ایستا bool
urldb_iterate_entries_host(ساختار جستجو_گره *parent,
		bool (*url_callback)(nsurl *url, ثابت ساختار url_data *data),
		bool (*cookie_callback)(ثابت ساختار cookie_data *data))
{
	اگر (parent == &empty) {
		بازگشت true;
	}

	اگر (!urldb_iterate_entries_host(parent->چپ,
					url_callback,
					cookie_callback)) {
		بازگشت false;
	}

	اگر ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		اگر (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			بازگشت false;
		}
	}

	اگر (!urldb_iterate_entries_host(parent->راست,
					url_callback,
					cookie_callback)) {
		بازگشت false;
	}

	بازگشت true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ایستا ساختار host_part *
urldb_add_host_گره(ثابت نویسه *part, ساختار host_part *parent)
{
	ساختار host_part *d;

	أكد(part && parent);

	d = calloc(1, اندازه(ساختار host_part));
	اگر (!d) {
		بازگشت پوچ;
	}

	d->part = strdup(part);
	اگر (!d->part) {
		آزاد(d);
		بازگشت پوچ;
	}

	d->next = parent->children;
	اگر (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	بازگشت d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ایستا صحیح urldb_add_path_fragment_cmp(ثابت تهی *a, ثابت تهی *b)
{
	بازگشت strcasecmp(*((ثابت نویسه **) a), *((ثابت نویسه **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ایستا ساختار path_data *
urldb_add_path_fragment(ساختار path_data *segment, lwc_string *fragment)
{
	نویسه **temp;

	أكد(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	اگر (!fragment)
		بازگشت segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * اندازه(نویسه *));
	اگر (!temp)
		بازگشت پوچ;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	اگر (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		بازگشت پوچ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      اندازه (نویسه *),
	      urldb_add_path_fragment_cmp);

	بازگشت segment;
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
ایستا ساختار path_data *
urldb_add_path_گره(lwc_string *scheme,
		    unsigned صحیح port,
		    ثابت نویسه *segment,
		    lwc_string *fragment,
		    ساختار path_data *parent)
{
	ساختار path_data *d, *e;

	أكد(scheme && segment && parent);

	d = calloc(1, اندازه(ساختار path_data));
	اگر (!d)
		بازگشت پوچ;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	اگر (!d->segment) {
		lwc_string_unref(d->scheme);
		آزاد(d);
		بازگشت پوچ;
	}

	اگر (fragment) {
		اگر (!urldb_add_path_fragment(d, fragment)) {
			آزاد(d->segment);
			lwc_string_unref(d->scheme);
			آزاد(d);
			بازگشت پوچ;
		}
	}

	برای (e = parent->children; e; e = e->next) {
		اگر (strcmp(e->segment, d->segment) > 0)
			بشکن;
	}

	اگر (e) {
		d->prev = e->prev;
		d->next = e;
		اگر (e->prev)
			e->prev->next = d;
		وگرنه
			parent->children = d;
		e->prev = d;
	} وگرنه اگر (!parent->children) {
		d->prev = d->next = پوچ;
		parent->children = parent->last = d;
	} وگرنه {
		d->next = پوچ;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	بازگشت d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ایستا ساختار جستجو_گره **urldb_get_جستجو_درخت_direct(ثابت نویسه *host)
{
	أكد(host);

	اگر (urldb__host_is_ip_address(host)) {
		بازگشت &جستجو_trees[ST_IP];
	} وگرنه اگر (ascii_is_alpha(*host)) {
		بازگشت &جستجو_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	بازگشت &جستجو_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ایستا ساختار جستجو_گره *urldb_get_جستجو_درخت(ثابت نویسه *host)
{
	بازگشت *urldb_get_جستجو_درخت_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ایستا صحیح urldb_جستجو_match_string(ثابت ساختار host_part *a, ثابت نویسه *b)
{
	ثابت نویسه *end, *dot;
	صحیح plen, ret;

	أكد(a && a != &db_ریشه && b);

	اگر (urldb__host_is_ip_address(b)) {
		/* IP address */
		بازگشت strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	وقتی (b < end && a && a != &db_ریشه) {
		dot = strchr(b, '.');
		اگر (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		اگر ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			بازگشت ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		اگر (plen > dot - b) {
			/* len(a) > len(b) */
			بازگشت 1;
		} وگرنه اگر (plen < dot - b) {
			/* len(a) < len(b) */
			بازگشت -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_ریشه && b >= end) {
		/* len(a) > len(b) */
		بازگشت 1;
	} وگرنه اگر ((!a || a == &db_ریشه) && b < end) {
		/* len(a) < len(b) */
		بازگشت -1;
	}

	/* Identical */
	بازگشت 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ایستا ثابت ساختار host_part *
urldb_جستجو_find(ساختار جستجو_گره *ریشه, ثابت نویسه *host)
{
	صحیح c;

	أكد(ریشه && host);

	اگر (ریشه == &empty) {
		بازگشت پوچ;
	}

	c = urldb_جستجو_match_string(ریشه->data, host);

	اگر (c > 0) {
		بازگشت urldb_جستجو_find(ریشه->چپ, host);
	} وگرنه اگر (c < 0) {
		بازگشت urldb_جستجو_find(ریشه->راست, host);
	}

	بازگشت ریشه->data;
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
ایستا ساختار path_data *
urldb_match_path(ثابت ساختار path_data *parent,
		 ثابت نویسه *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	ثابت ساختار path_data *p;
	ثابت نویسه *slash;
	bool match;

	أكد(parent != پوچ);
	أكد(parent->segment == پوچ);

	اگر (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	أكد(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	وقتی (p != پوچ) {
		slash = strchr(path + 1, '/');
		اگر (!slash) {
			slash = path + strlen(path);
		}

		اگر (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			اگر (*slash == '\0') {
				/* Complete match */
				بازگشت (ساختار path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} وگرنه {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	بازگشت پوچ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ایستا ساختار path_data *urldb_find_url(nsurl *url)
{
	ثابت ساختار host_part *h;
	ساختار path_data *p;
	ساختار جستجو_گره *درخت;
	نویسه *plq;
	ثابت نویسه *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned صحیح port_int;
	bool match;

	أكد(url);

	اگر (url_bloom != پوچ) {
		اگر (bloom_جستجو_hash(url_bloom, nsurl_hash(url)) == false) {
			بازگشت پوچ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == پوچ)
		بازگشت پوچ;

	اگر (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		بازگشت پوچ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != پوچ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} وگرنه اگر (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} وگرنه {
		lwc_string_unref(scheme);
		بازگشت پوچ;
	}

	درخت = urldb_get_جستجو_درخت(host_str);
	h = urldb_جستجو_find(درخت, host_str);
	اگر (!h) {
		lwc_string_unref(scheme);
		بازگشت پوچ;
	}

	/* generate plq (path, leaf, query) */
	اگر (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		بازگشت پوچ;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	اگر (port != پوچ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} وگرنه {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	آزاد(plq);
	lwc_string_unref(scheme);

	بازگشت p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ایستا تهی urldb_dump_paths(ساختار path_data *parent)
{
	ثابت ساختار path_data *p = parent;
	unsigned صحیح i;

	انجامبده {
		اگر (p->segment != پوچ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			برای (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		اگر (p->children != پوچ) {
			p = p->children;
		} وگرنه {
			وقتی (p != parent) {
				اگر (p->next != پوچ) {
					p = p->next;
					بشکن;
				}

				p = p->parent;
			}
		}
	} وقتی (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ایستا تهی urldb_dump_hosts(ساختار host_part *parent)
{
	ساختار host_part *h;

	اگر (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	برای (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ایستا تهی urldb_dump_جستجو(ساختار جستجو_گره *parent, صحیح depth)
{
	ثابت ساختار host_part *h;
	صحیح i; /* index into string */
	نویسه s[1024];
	صحیح r;
	صحیح sl = اندازه(s) - 2;

	اگر (parent == &empty)
		بازگشت;

	urldb_dump_جستجو(parent->چپ, depth + 1);

	برای (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	برای (h = parent->data; h; h = h->parent) {
		اگر (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			اگر (r < 0) {
				بشکن;
			}
			اگر ((i + r) >= sl) {
				بشکن;
			}
			i += r;
		}

		اگر (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_جستجو(parent->راست, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ایستا صحیح
urldb_جستجو_match_host(ثابت ساختار host_part *a, ثابت ساختار host_part *b)
{
	صحیح ret;

	أكد(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	برای (; a && a != &db_ریشه && b && b != &db_ریشه;
	     a = a->parent, b = b->parent) {
		اگر ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			بازگشت ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_ریشه && (!b || b == &db_ریشه)) {
		/* len(a) > len(b) */
		بازگشت 1;
	} وگرنه اگر ((!a || a == &db_ریشه) && b && b != &db_ریشه) {
		/* len(a) < len(b) */
		بازگشت -1;
	}

	/* identical */
	بازگشت 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ایستا ساختار جستجو_گره *urldb_جستجو_skew(ساختار جستجو_گره *ریشه)
{
	أكد(ریشه);

	اگر (ریشه->چپ->level == ریشه->level) {
		ساختار جستجو_گره *temp;

		temp = ریشه->چپ;
		ریشه->چپ = temp->راست;
		temp->راست = ریشه;
		ریشه = temp;
	}

	بازگشت ریشه;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ایستا ساختار جستجو_گره *urldb_جستجو_split(ساختار جستجو_گره *ریشه)
{
	أكد(ریشه);

	اگر (ریشه->راست->راست->level == ریشه->level) {
		ساختار جستجو_گره *temp;

		temp = ریشه->راست;
		ریشه->راست = temp->چپ;
		temp->چپ = ریشه;
		ریشه = temp;

		ریشه->level++;
	}

	بازگشت ریشه;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ایستا ساختار جستجو_گره *
urldb_جستجو_درج_internal(ساختار جستجو_گره *ریشه, ساختار جستجو_گره *n)
{
	أكد(ریشه && n);

	اگر (ریشه == &empty) {
		ریشه = n;
	} وگرنه {
		صحیح c = urldb_جستجو_match_host(ریشه->data, n->data);

		اگر (c > 0) {
			ریشه->چپ = urldb_جستجو_درج_internal(
				ریشه->چپ, n);
		} وگرنه اگر (c < 0) {
			ریشه->راست = urldb_جستجو_درج_internal(
				ریشه->راست, n);
		} وگرنه {
			/* exact match */
			آزاد(n);
			بازگشت ریشه;
		}

		ریشه = urldb_جستجو_skew(ریشه);
		ریشه = urldb_جستجو_split(ریشه);
	}

	بازگشت ریشه;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ایستا ساختار جستجو_گره *
urldb_جستجو_درج(ساختار جستجو_گره *ریشه, ثابت ساختار host_part *data)
{
	ساختار جستجو_گره *n;

	أكد(ریشه && data);

	n = malloc(اندازه(ساختار جستجو_گره));
	اگر (!n)
		بازگشت پوچ;

	n->level = 1;
	n->data = data;
	n->چپ = n->راست = &empty;

	ریشه = urldb_جستجو_درج_internal(ریشه, n);

	بازگشت ریشه;
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
ایستا bool
urldb_parse_avpair(ساختار cookie_internal_data *c,
		   نویسه *n,
		   نویسه *v,
		   bool was_quoted)
{
	صحیح vlen;

	أكد(c && n && v);

	/* Strip whitespace from start of name */
	برای (; *n; n++) {
		اگر (*n != ' ' && *n != '\t')
			بشکن;
	}

	/* Strip whitespace from end of name */
	برای (vlen = strlen(n); vlen; vlen--) {
		اگر (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		وگرنه
			بشکن;
	}

	/* Strip whitespace from start of value */
	برای (; *v; v++) {
		اگر (*v != ' ' && *v != '\t')
			بشکن;
	}

	/* Strip whitespace from end of value */
	برای (vlen = strlen(v); vlen; vlen--) {
		اگر (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		وگرنه
			بشکن;
	}

	اگر (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		اگر (!c->comment)
			بازگشت false;
	} وگرنه اگر (!c->domain && strcasecmp(n, "Domain") == 0) {
		اگر (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			اگر (!c->domain)
				بازگشت false;
		}
	} وگرنه اگر (strcasecmp(n, "Max-Age") == 0) {
		صحیح temp = atoi(v);
		اگر (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		وگرنه
			c->expires = time(پوچ) + temp;
	} وگرنه اگر (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		اگر (!c->path)
			بازگشت false;
	} وگرنه اگر (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} وگرنه اگر (strcasecmp(n, "Expires") == 0) {
		نویسه *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		برای (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		اگر (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} وگرنه اگر (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} وگرنه اگر (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} وگرنه اگر (!c->name) {
		c->name = strdup(n);
		c->مقدار = strdup(v);
		c->مقدار_was_quoted = was_quoted;
		اگر (!c->name || !c->مقدار) {
			بازگشت false;
		}
	}

	بازگشت true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ایستا تهی urldb_free_cookie(ساختار cookie_internal_data *c)
{
	أكد(c);

	آزاد(c->comment);
	آزاد(c->domain);
	آزاد(c->path);
	آزاد(c->name);
	آزاد(c->مقدار);
	آزاد(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ایستا ساختار cookie_internal_data *
urldb_parse_cookie(nsurl *url, ثابت نویسه **cookie)
{
	ساختار cookie_internal_data *c;
	ثابت نویسه *cur;
	نویسه name[1024], مقدار[4096];
	نویسه *n = name, *v = مقدار;
	bool in_مقدار = false;
	bool had_مقدار_data = false;
	bool مقدار_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	أكد(url && cookie && *cookie);

	c = calloc(1, اندازه(ساختار cookie_internal_data));
	اگر (c == پوچ)
		بازگشت پوچ;

	c->expires = -1;

	name[0] = '\0';
	مقدار[0] = '\0';

	برای (cur = *cookie; *cur; cur++) {
		اگر (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			اگر (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				مقدار[0] = '\0';

			}

			بشکن;
		} وگرنه اگر (*cur == '\r') {
			/* Spurious linefeed */
			ادامه;
		} وگرنه اگر (*cur == '\n') {
			/* Spurious newline */
			ادامه;
		}

		اگر (in_مقدار && !had_مقدار_data) {
			اگر (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ادامه;
			} وگرنه {
				had_مقدار_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				اگر (*cur != '"') {
					مقدار_verbatim = true;
				}
			}
		}

		اگر (in_مقدار && !مقدار_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			اگر (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ادامه;
			}
		}

		اگر (!quoted && !in_مقدار && *cur == '=') {
			/* First equals => attr-value separator */
			in_مقدار = true;
			ادامه;
		}

		اگر (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			اگر (!urldb_parse_avpair(c, name, مقدار, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				بازگشت پوچ;
			}

			/* And reset to start */
			n = name;
			v = مقدار;
			in_مقدار = false;
			had_مقدار_data = false;
			مقدار_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			اگر (*cur != ';') {
				cur--;
			}

			ادامه;
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
		اگر (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ثابت نویسه *p;
			ثابت نویسه *semi = strchr(cur + 1, ';');
			اگر (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			برای (p = cur + 1; p < semi; p++)
				اگر (*p == '=')
					بشکن;

			اگر (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} وگرنه {
				/* found one => comma marks end of cookie */
				cur++;
				بشکن;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		اگر (!in_مقدار) {
			اگر (n < name + (اندازه(name) - 1))
				*n++ = *cur;
		} وگرنه {
			اگر (v < مقدار + (اندازه(مقدار) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	اگر (!urldb_parse_avpair(c, name, مقدار, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		بازگشت پوچ;
	}

	/* Now fix-up default values */
	اگر (c->domain == پوچ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		اگر (host == پوچ) {
			urldb_free_cookie(c);
			بازگشت پوچ;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	اگر (c->path == پوچ) {
		ثابت نویسه *path_data;
		نویسه *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		اگر (path_lwc == پوچ) {
			urldb_free_cookie(c);
			بازگشت پوچ;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		اگر (slash != پوچ) {
			/* Special case: retain first slash in path */
			اگر (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			اگر (slash == پوچ) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				بازگشت پوچ;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} وگرنه {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			اگر (path == پوچ) {
				urldb_free_cookie(c);
				بازگشت پوچ;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	بازگشت c;
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
ایستا ساختار path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned صحیح port,
	       ثابت ساختار host_part *host,
	       نویسه *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ساختار path_data *d, *e;
	نویسه *buf = path_query;
	نویسه *segment, *slash;
	bool match;

	أكد(scheme && host && url);

	d = (ساختار path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	اگر (*segment == '/')
		segment++;

	/* Process path segments */
	انجامبده {
		slash = strchr(segment, '/');
		اگر (!slash) {
			/* last segment */
			/* look for existing entry */
			برای (e = d->children; e; e = e->next)
				اگر (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					بشکن;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_گره(scheme, port,
						    segment, fragment, d);
			بشکن;
		}

		*slash = '\0';

		/* look for existing entry */
		برای (e = d->children; e; e = e->next)
			اگر (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				بشکن;

		d = e ? e : urldb_add_path_گره(scheme, port, segment, پوچ, d);
		اگر (!d)
			بشکن;

		segment = slash + 1;
	} وقتی (1);

	آزاد(path_query);

	اگر (d && !d->url) {
		/* Insert defragmented URL */
		اگر (nsurl_defragment(url, &d->url) != NSERROR_OK)
			بازگشت پوچ;
	}

	بازگشت d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ایستا ساختار host_part *urldb_add_host(ثابت نویسه *host)
{
	ساختار host_part *d = (ساختار host_part *) &db_ریشه, *e;
	ساختار جستجو_گره *s;
	نویسه buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	نویسه *part;

	أكد(host);

	اگر (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		برای (e = d->children; e; e = e->next)
			اگر (strcasecmp(host, e->part) == 0)
				/* found => return it */
				بازگشت e;

		d = urldb_add_host_گره(host, d);

		s = urldb_جستجو_درج(جستجو_trees[ST_IP], d);
		اگر (!s) {
			/* failed */
			d = پوچ;
		} وگرنه {
			جستجو_trees[ST_IP] = s;
		}

		بازگشت d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, اندازه buf);
	buf[اندازه buf - 1] = '\0';

	/* Process FQDN segments backwards */
	انجامبده {
		part = strrchr(buf, '.');
		اگر (!part) {
			/* last segment */
			/* Check for existing entry */
			برای (e = d->children; e; e = e->next)
				اگر (strcasecmp(buf, e->part) == 0)
					بشکن;

			اگر (e) {
				d = e;
			} وگرنه {
				d = urldb_add_host_گره(buf, d);
			}

			/* And insert into search tree */
			اگر (d) {
				ساختار جستجو_گره **r;

				r = urldb_get_جستجو_درخت_direct(buf);
				s = urldb_جستجو_درج(*r, d);
				اگر (!s) {
					/* failed */
					d = پوچ;
				} وگرنه {
					*r = s;
				}
			}
			بشکن;
		}

		/* Check for existing entry */
		برای (e = d->children; e; e = e->next)
			اگر (strcasecmp(part + 1, e->part) == 0)
				بشکن;

		d = e ? e : urldb_add_host_گره(part + 1, d);
		اگر (!d)
			بشکن;

		*part = '\0';
	} وقتی (1);

	بازگشت d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ایستا bool
urldb_درج_cookie(ساختار cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ساختار cookie_internal_data *d;
	ثابت ساختار host_part *h;
	ساختار path_data *p;
	time_t now = time(پوچ);

	أكد(c);

	اگر (c->domain[0] == '.') {
		h = urldb_جستجو_find(
			urldb_get_جستجو_درخت(&(c->domain[1])),
			c->domain + 1);
		اگر (!h) {
			h = urldb_add_host(c->domain + 1);
			اگر (!h) {
				urldb_free_cookie(c);
				بازگشت false;
			}
		}

		p = (ساختار path_data *) &h->paths;
	} وگرنه {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		أكد(url != پوچ);
		أكد(scheme != پوچ);

		h = urldb_جستجو_find(
			urldb_get_جستجو_درخت(c->domain),
			c->domain);

		اگر (!h) {
			h = urldb_add_host(c->domain);
			اگر (!h) {
				urldb_free_cookie(c);
				بازگشت false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), پوچ, url);
		اگر (!p) {
			urldb_free_cookie(c);
			بازگشت false;
		}
	}

	/* add cookie */
	برای (d = p->cookies; d; d = d->next) {
		اگر (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			بشکن;
	}

	اگر (d) {
		اگر (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			اگر (d->next)
				d->next->prev = d->prev;
			وگرنه
				p->cookies_end = d->prev;
			اگر (d->prev)
				d->prev->next = d->next;
			وگرنه
				p->cookies = d->next;

			cookie_manager_remove((ساختار cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} وگرنه {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			اگر (c->next)
				c->next->prev = c;
			وگرنه
				p->cookies_end = c;
			اگر (c->prev)
				c->prev->next = c;
			وگرنه
				p->cookies = c;

			cookie_manager_remove((ساختار cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((ساختار cookie_data *)c);
		}
	} وگرنه {
		c->prev = p->cookies_end;
		c->next = پوچ;
		اگر (p->cookies_end)
			p->cookies_end->next = c;
		وگرنه
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((ساختار cookie_data *)c);
	}

	بازگشت true;
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
ایستا bool
concat_netscape_cookie(ساختار cookie_internal_data *c,
		    صحیح *used,
		    صحیح *alloc,
		    نویسه **buf)
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
	صحیح max_len;
	صحیح outl; /* output length */

	أكد(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->مقدار) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	اگر (max_len >= *alloc) {
		نویسه *temp = realloc(*buf, max_len);
		اگر (!temp) {
			بازگشت false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	اگر (outl >= (*alloc - *used)) {
		/* output truncated */
		بازگشت false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	اگر (c->مقدار_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->مقدار);
		اگر (outl >= (*alloc - *used)) {
			/* output truncated */
			بازگشت false;
		}
		*used += outl;
	} وگرنه {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->مقدار);
		اگر (outl >= (*alloc - *used)) {
			/* output truncated */
			بازگشت false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	بازگشت true;
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
ایستا bool
concat_rfc2965_cookie(ساختار cookie_internal_data *c,
		      صحیح *used,
		      صحیح *alloc,
		      نویسه **buf)
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
	ثابت نویسه * ثابت separators = "()<>@,;:\\\"/[]?={} \t";
	صحیح max_len;
	صحیح outl; /* output length */

	أكد(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->مقدار) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	اگر (max_len >= *alloc) {
		نویسه *temp = realloc(*buf, max_len);
		اگر (!temp) {
			بازگشت false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	اگر (outl >= (*alloc - *used)) {
		/* output truncated */
		بازگشت false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	اگر (c->مقدار_was_quoted ||
	    strpbrk(c->مقدار, separators) != پوچ) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->مقدار);

	} وگرنه {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->مقدار);
	}
	اگر (outl >= (*alloc - *used)) {
		/* output truncated */
		بازگشت false;
	}
	*used += outl;

	اگر (c->path_from_set) {
		/* Path, quoted if necessary */

		اگر (strpbrk(c->path, separators) != پوچ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} وگرنه {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		اگر (outl >= (*alloc - *used)) {
			/* output truncated */
			بازگشت false;
		}
		*used += outl;
	}

	اگر (c->domain_from_set) {
		/* Domain, quoted if necessary */
		اگر (strpbrk(c->domain, separators) != پوچ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} وگرنه {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		اگر (outl >= (*alloc - *used)) {
			/* output truncated */
			بازگشت false;
		}
		*used += outl;
	}

	بازگشت true;
}


/**
 * build output cookie string in netscape format
 */
ایستا نویسه *
get_netscape_cookie(ساختار cookie_internal_data **matched_cookies, صحیح شمار)
{
	صحیح ret_alloc = 4096, ret_used = 1;
	نویسه *ret;
	صحیح midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	اگر (ret == پوچ) {
		بازگشت پوچ;
	}
	ret[0] = '\0';

	برای (midx = 0; midx < شمار; midx++) {
		اگر (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			آزاد(ret);
			بازگشت پوچ;
		}
	}

	اگر (ret_used <= 2) {
		آزاد(ret);
		بازگشت پوچ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	نویسه *temp = realloc(ret, ret_used);
	اگر (temp != پوچ) {
		ret = temp;
	}

	بازگشت ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ایستا نویسه *
get_rfc2965_cookie(ساختار cookie_internal_data **matched_cookies,
		   صحیح شمار,
		   صحیح version)
{
	صحیح ret_alloc = 4096; /* return string allocation */
	صحیح ret_used = 1; /* return sting buffer used */
	نویسه *ret; /* return string buffer */
	صحیح midx; /* index into matched cookies */
	صحیح outl;

	ret = malloc(ret_alloc);
	اگر (ret == پوچ) {
		بازگشت پوچ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	اگر (outl >= ret_alloc - ret_used) {
		/* overflow */
		آزاد(ret);
		بازگشت پوچ;
	}
	ret_used += outl;

	برای (midx = 0; midx < شمار; midx++) {
		اگر (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			آزاد(ret);
			بازگشت پوچ;
		}
	}

	/* shrink the output buffer to the required size */
	نویسه *temp = realloc(ret, ret_used);
	اگر (temp != پوچ) {
		ret = temp;
	}

	بازگشت ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ایستا تهی
urldb_delete_cookie_paths(ثابت نویسه *domain,
			  ثابت نویسه *path,
			  ثابت نویسه *name,
			  ساختار path_data *parent)
{
	ساختار cookie_internal_data *c;
	ساختار path_data *p = parent;

	أكد(parent);

	انجامبده {
		برای (c = p->cookies; c; c = c->next) {
			اگر (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				اگر (c->prev) {
					c->prev->next = c->next;
				} وگرنه {
					p->cookies = c->next;
				}

				اگر (c->next) {
					c->next->prev = c->prev;
				} وگرنه {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				بازگشت;
			}
		}

		اگر (p->children) {
			p = p->children;
		} وگرنه {
			وقتی (p != parent) {
				اگر (p->next != پوچ) {
					p = p->next;
					بشکن;
				}

				p = p->parent;
			}
		}
	} وقتی (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ایستا تهی
urldb_delete_cookie_hosts(ثابت نویسه *domain,
			  ثابت نویسه *path,
			  ثابت نویسه *name,
			  ساختار host_part *parent)
{
	ساختار host_part *h;
	أكد(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	برای (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ایستا تهی urldb_save_cookie_paths(FILE *fp, ساختار path_data *parent)
{
	ساختار path_data *p = parent;
	time_t now = time(پوچ);

	أكد(fp && parent);

	انجامبده {
		اگر (p->cookies != پوچ) {
			ساختار cookie_internal_data *c;

			برای (c = p->cookies; c != پوچ; c = c->next) {
				اگر (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ادامه;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(صحیح)c->expires, (صحیح)c->last_used,
					c->no_destroy, c->name, c->مقدار,
					c->مقدار_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		اگر (p->children != پوچ) {
			p = p->children;
		} وگرنه {
			وقتی (p != parent) {
				اگر (p->next != پوچ) {
					p = p->next;
					بشکن;
				}

				p = p->parent;
			}
		}
	} وقتی (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ایستا تهی urldb_save_cookie_hosts(FILE *fp, ساختار host_part *parent)
{
	ساختار host_part *h;
	أكد(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	برای (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ایستا تهی urldb_destroy_cookie(ساختار cookie_internal_data *c)
{
	آزاد(c->name);
	آزاد(c->مقدار);
	آزاد(c->comment);
	آزاد(c->domain);
	آزاد(c->path);

	آزاد(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ایستا تهی urldb_destroy_path_گره_content(ساختار path_data *گره)
{
	ساختار cookie_internal_data *a, *b;
	unsigned صحیح i;

	اگر (گره->url != پوچ) {
		nsurl_unref(گره->url);
	}

	lwc_string_unref(گره->scheme);

	آزاد(گره->segment);
	برای (i = 0; i < گره->frag_cnt; i++)
		آزاد(گره->fragment[i]);
	آزاد(گره->fragment);

	آزاد(گره->urld.title);

	برای (a = گره->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ایستا تهی urldb_destroy_prot_space(ساختار prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	آزاد(space->realm);
	آزاد(space->auth);

	آزاد(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ایستا تهی urldb_destroy_path_درخت(ساختار path_data *ریشه)
{
	ساختار path_data *p = ریشه;

	انجامبده {
		اگر (p->children != پوچ) {
			p = p->children;
		} وگرنه {
			ساختار path_data *q = p;

			وقتی (p != ریشه) {
				اگر (p->next != پوچ) {
					p = p->next;
					بشکن;
				}

				p = p->parent;

				urldb_destroy_path_گره_content(q);
				آزاد(q);

				q = p;
			}

			urldb_destroy_path_گره_content(q);
			آزاد(q);
		}
	} وقتی (p != ریشه);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ایستا تهی urldb_destroy_host_درخت(ساختار host_part *ریشه)
{
	ساختار host_part *a, *b;
	ساختار path_data *p, *q;
	ساختار prot_space_data *s, *t;

	/* Destroy children */
	برای (a = ریشه->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_درخت(a);
	}

	/* Now clean up paths */
	برای (p = ریشه->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_درخت(p);
	}

	/* Root path */
	urldb_destroy_path_گره_content(&ریشه->paths);

	/* Proctection space data */
	برای (s = ریشه->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	آزاد(ریشه->part);
	آزاد(ریشه);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ایستا تهی urldb_destroy_جستجو_درخت(ساختار جستجو_گره *ریشه)
{
	/* Destroy children */
	اگر (ریشه->چپ != &empty)
		urldb_destroy_جستجو_درخت(ریشه->چپ);
	اگر (ریشه->راست != &empty)
		urldb_destroy_جستجو_درخت(ریشه->راست);

	/* And destroy ourselves */
	آزاد(ریشه);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
تهی urldb_destroy(تهی)
{
	ساختار host_part *a, *b;
	صحیح i;

	/* Clean up search trees */
	برای (i = 0; i < رقم_جستجو_TREES; i++) {
		اگر (جستجو_trees[i] != &empty) {
			urldb_destroy_جستجو_درخت(جستجو_trees[i]);
			جستجو_trees[i] = &empty;
		}
	}

	/* And database */
	برای (a = db_ریشه.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_درخت(a);
	}
	memset(&db_ریشه, 0, اندازه(db_ریشه));

	/* And the bloom filter */
	اگر (url_bloom != پوچ) {
		bloom_destroy(url_bloom);
		url_bloom = پوچ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(ثابت نویسه *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	نویسه s[MAXIMUM_URL_LENGTH];
	نویسه host[256];
	ساختار host_part *h;
	صحیح urls;
	صحیح i;
	صحیح version;
	صحیح length;
	FILE *fp;

	أكد(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	اگر (url_bloom == پوچ)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	اگر (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		بازگشت NSERROR_NOT_FOUND;
	}

	اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		بازگشت NSERROR_NEED_DATA;
	}

	version = atoi(s);
	اگر (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		بازگشت NSERROR_INVALID;
	}
	اگر (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		بازگشت NSERROR_INVALID;
	}

	وقتی (fgets(host, اندازه host, fp)) {
		time_t hsts_expiry = 0;
		صحیح hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		اگر (length == 0) {
			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;
			urls = atoi(s);
			/* Eight fields/url */
			برای (i = 0; i < (8 * urls); i++) {
				اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					بشکن;
			}
			ادامه;
		}

		اگر (version >= 107) {
			نویسه *p = host;
			وقتی (*p && *p != ' ') p++;
			وقتی (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			وقتی (*p && *p != ' ') p++;
			وقتی (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		اگر (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			بازگشت NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			بشکن;
		urls = atoi(s);

		/* no URLs => try next host */
		اگر (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ادامه;
		}

		/* load the non-corrupt data */
		برای (i = 0; i < urls; i++) {
			ساختار path_data *p = پوچ;
			نویسه scheme[64], ports[10];
			نویسه url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned صحیح port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			نویسه *path_query;
			size_t len;

			اگر (!fgets(scheme, اندازه scheme, fp))
				بشکن;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			اگر (!fgets(ports, اندازه ports, fp))
				بشکن;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;
			length = strlen(s) - 1;
			s[length] = '\0';

			اگر (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, اندازه url, "%s://%s%s%s%s",
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
			اگر (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				بازگشت NSERROR_NOMEM;
			}

			اگر (url_bloom != پوچ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_درج_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			اگر (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				بازگشت NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			اگر (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				بازگشت NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;
			اگر (p)
				p->urld.visits = (unsigned صحیح)atoi(s);

			/* entry last use time */
			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				بشکن;
			}
			اگر (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;
			اگر (p)
				p->urld.type = (content_type)atoi(s);

			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;


			اگر (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				بشکن;
			length = strlen(s) - 1;
			اگر (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				اگر (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	بازگشت NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ثابت نویسه *filename)
{
	FILE *fp;
	صحیح i;

	أكد(filename);

	fp = fopen(filename, "w");
	اگر (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		بازگشت NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	برای (i = 0; i != رقم_جستجو_TREES; i++) {
		urldb_save_جستجو_درخت(جستجو_trees[i], fp);
	}

	fclose(fp);

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	ساختار path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p) {
		بازگشت NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	ساختار host_part *h;
	ساختار path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	ثابت نویسه *host_str;
	نویسه *path_query = پوچ;
	size_t len;
	bool match;
	unsigned صحیح port_int;

	أكد(url);

	اگر (url_bloom == پوچ)
		url_bloom = bloom_create(BLOOM_SIZE);

	اگر (url_bloom != پوچ) {
		uint32_t hash = nsurl_hash(url);
		bloom_درج_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	اگر (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		بازگشت false;
	}
	أكد(path_query != پوچ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == پوچ) {
		آزاد(path_query);
		بازگشت false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != پوچ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} وگرنه اگر (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} وگرنه {
		lwc_string_unref(scheme);
		آزاد(path_query);
		بازگشت false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	اگر (port != پوچ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} وگرنه {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	اگر (h != پوچ) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} وگرنه {
		p = پوچ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	بازگشت (p != پوچ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, ثابت نویسه *title)
{
	ساختار path_data *p;
	نویسه *temp;

	أكد(url);

	p = urldb_find_url(url);
	اگر (p == پوچ) {
		بازگشت NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	اگر (title != پوچ) {
		temp = strdup(title);
		اگر (temp == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
	} وگرنه {
		temp = پوچ;
	}

	آزاد(p->urld.title);
	p->urld.title = temp;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	ساختار path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p) {
		بازگشت NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	ساختار path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p) {
		بازگشت NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(پوچ);
	p->urld.visits++;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
تهی urldb_reset_url_visit_data(nsurl *url)
{
	ساختار path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ثابت ساختار url_data *urldb_get_url_data(nsurl *url)
{
	ساختار path_data *p;
	ساختار url_internal_data *u;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت پوچ;

	u = &p->urld;

	بازگشت (ثابت ساختار url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ساختار path_data *p;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت پوچ;

	بازگشت p->url;
}


/* exported interface documented in netsurf/url_db.h */
تهی urldb_set_auth_details(nsurl *url, ثابت نویسه *realm, ثابت نویسه *auth)
{
	ساختار path_data *p, *pi;
	ساختار host_part *h;
	ساختار prot_space_data *space, *space_alloc;
	نویسه *realm_alloc, *auth_alloc;
	bool match;

	أكد(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	اگر (!p)
		بازگشت;

	/* Search for host_part */
	برای (pi = p; pi->parent != پوچ; pi = pi->parent)
		;
	h = (ساختار host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	برای (space = h->prot_space; space; space = space->next) {
		اگر (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			بشکن;
	}

	اگر (space != پوچ) {
		/* Overrule existing auth. */
		آزاد(space->auth);
		space->auth = strdup(auth);
	} وگرنه {
		/* Create a new protection space. */
		space = space_alloc = malloc(اندازه(ساختار prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		اگر (!space_alloc || !realm_alloc || !auth_alloc) {
			آزاد(space_alloc);
			آزاد(realm_alloc);
			آزاد(auth_alloc);
			بازگشت;
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
ثابت نویسه *urldb_get_auth_details(nsurl *url, ثابت نویسه *realm)
{
	ساختار path_data *p, *p_cur, *p_بالاترین;

	أكد(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت پوچ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	برای (p_cur = p; p_cur != پوچ; p_بالاترین = p_cur, p_cur = p_cur->parent) {
		اگر (p_cur->prot_space) {
			بازگشت p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	اگر (realm != پوچ) {
		ثابت ساختار host_part *h = (ثابت ساختار host_part *)p_بالاترین;
		ثابت ساختار prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		برای (space = h->prot_space; space != پوچ;
		     space = space->next) {
			اگر (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				بازگشت p->prot_space->auth;
			}
		}
	}

	بازگشت پوچ;
}


/* exported interface documented in netsurf/url_db.h */
تهی urldb_set_cert_permissions(nsurl *url, bool permit)
{
	ساختار path_data *p;
	ساختار host_part *h;

	أكد(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت;

	برای (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ساختار host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ساختار path_data *p;
	ثابت ساختار host_part *h;

	أكد(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت false;

	برای (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ثابت ساختار host_part *)p;

	بازگشت h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(ساختار nsurl *url, ثابت نویسه *header)
{
	ساختار path_data *p;
	ساختار host_part *h;
	lwc_string *host;
	time_t now = time(پوچ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != پوچ) {
		اگر (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			بازگشت true;
		} وگرنه اگر (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			بازگشت true;
		}

		lwc_string_unref(host);
	} وگرنه {
		/* No host part: ignore */
		بازگشت true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت false;

	برای (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ساختار host_part *)p;
	اگر (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		بازگشت true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	اگر (error != NSERROR_OK) {
		/* Parse failed: ignore */
		بازگشت true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	اگر (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} وگرنه اگر ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	بازگشت true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ساختار nsurl *url)
{
	ساختار path_data *p;
	ثابت ساختار host_part *h;
	lwc_string *host;
	time_t now = time(پوچ);

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != پوچ) {
		اگر (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			بازگشت false;
		} وگرنه اگر (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			بازگشت false;
		}

		lwc_string_unref(host);
	} وگرنه {
		/* No host part: not enabled */
		بازگشت false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت false;

	برای (; p && p->parent; p = p->parent)
		/* do nothing */;
	أكد(p);

	h = (ثابت ساختار host_part *)p;

	/* Consult record for this host */
	اگر (h->hsts.expires > now) {
		/* Not expired */
		بازگشت true;
	}

	/* Consult parent domains */
	برای (h = h->parent; h && h != &db_ریشه; h = h->parent) {
		اگر (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			بازگشت true;
		}
	}

	بازگشت false;
}


/* exported interface documented in netsurf/url_db.h */
تهی
urldb_iterate_partial(ثابت نویسه *prefix,
		      bool (*callback)(nsurl *url, ثابت ساختار url_data *data))
{
	نویسه host[256];
	نویسه buf[260]; /* max domain + "www." */
	ثابت نویسه *slash, *scheme_sep;
	ساختار جستجو_گره *درخت;
	ثابت ساختار host_part *h;

	أكد(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	اگر (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	درخت = urldb_get_جستجو_درخت(prefix);

	اگر (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, اندازه host, "%.*s",
			 (صحیح) (slash - prefix), prefix);

		h = urldb_جستجو_find(درخت, host);
		اگر (!h) {
			صحیح len = slash - prefix;

			اگر (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, اندازه buf, "www.%s", host);
				h = urldb_جستجو_find(
					جستجو_trees[ST_DN + 'w' - 'a'],
					buf);
				اگر (!h)
					بازگشت;
			} وگرنه
				بازگشت;
		}

		اگر (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} وگرنه {
		صحیح len = strlen(prefix);

		/* looking for hosts */
		اگر (!urldb_iterate_partial_host(درخت, prefix, callback))
			بازگشت;

		اگر (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, اندازه buf, "www.%s", prefix);
			اگر(!urldb_iterate_partial_host(
				   جستجو_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				بازگشت;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
تهی
urldb_iterate_entries(bool (*callback)(nsurl *url, ثابت ساختار url_data *data))
{
	صحیح i;

	أكد(callback);

	برای (i = 0; i < رقم_جستجو_TREES; i++) {
		اگر (!urldb_iterate_entries_host(جستجو_trees[i],
						callback,
						پوچ)) {
			بشکن;
		}
	}
}


/* exported interface documented in content/urldb.h */
تهی urldb_iterate_cookies(bool (*callback)(ثابت ساختار cookie_data *data))
{
	صحیح i;

	أكد(callback);

	برای (i = 0; i < رقم_جستجو_TREES; i++) {
		اگر (!urldb_iterate_entries_host(جستجو_trees[i],
						پوچ, callback))
			بشکن;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(ثابت نویسه *header, nsurl *url, nsurl *referer)
{
	ثابت نویسه *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	أكد(url && header);

	/* Get defragmented URL, as 'urlt' */
	اگر (nsurl_defragment(url, &urlt) != NSERROR_OK)
		بازگشت پوچ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == پوچ) {
		nsurl_unref(urlt);
		بازگشت false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	اگر (path == پوچ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		بازگشت false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host == پوچ) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		بازگشت false;
	}

	اگر (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		اگر (rhost == پوچ) {
			برو error;
		}

		/* Domain match host names */
		اگر (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			ثابت نویسه *hptr;
			ثابت نویسه *rptr;
			ثابت نویسه *dot;
			ثابت نویسه *host_data = lwc_string_data(host);
			ثابت نویسه *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			اگر (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				برو error;
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
			وقتی (hptr >= host_data && rptr >= rhost_data) {
				اگر (*hptr != *rptr)
					بشکن;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			وقتی (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			اگر (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == پوچ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				برو error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	انجامبده {
		ساختار cookie_internal_data *c;
		نویسه *dot;
		size_t len;
#ifdef WITH_NSPSL
		ثابت نویسه *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		اگر (!c) {
			/* failed => stop parsing */
			برو error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		اگر (!c->name || !c->مقدار) {
			urldb_free_cookie(c);
			برو error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		اگر (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			برو error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		اگر (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		اگر (suffix == پوچ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			برو error;
		}
#وگرنه
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		اگر (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			برو error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		اگر (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			صحیح hlen, dlen;
			نویسه *domain = c->domain;

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
			اگر (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				برو error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			اگر (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				برو error;
			}

			اگر (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			اگر (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				برو error;
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
		اگر (!urldb_درج_cookie(c, scheme, urlt))
			برو error;
	} وقتی (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	بازگشت true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	بازگشت false;
}


/* exported interface documented in content/urldb.h */
نویسه *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ثابت ساختار path_data *p, *q;
	ثابت ساختار host_part *h;
	lwc_string *path_lwc;
	ساختار cookie_internal_data *c;
	صحیح شمار = 0, version = COOKIE_RFC2965;
	ساختار cookie_internal_data **matched_cookies;
	صحیح matched_cookies_size = 20;
	ثابت نویسه *path;
	نویسه *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	أكد(url != پوچ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	اگر (!p)
		بازگشت پوچ;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 اندازه(ساختار cookie_internal_data *));
	اگر (!matched_cookies)
		بازگشت پوچ;

#define GROW_MATCHED_COOKIES						\
	انجامبده {								\
		اگر (شمار == matched_cookies_size) {			\
			ساختار cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       اندازه(ساختار cookie_internal_data *)); \
									\
			اگر (temp == پوچ) {				\
				آزاد(matched_cookies);			\
				بازگشت پوچ;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} وقتی(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	اگر (path_lwc == پوچ) {
		آزاد(matched_cookies);
		بازگشت پوچ;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(پوچ);

	اگر (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		برای (q = p->parent->children; q; q = q->next) {
			اگر (strcmp(q->segment, p->segment))
				ادامه;

			/* Consider all cookies associated with
			 * this exact path */
			برای (c = q->cookies; c; c = c->next) {
				اگر (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ادامه;

				اگر (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					ادامه;

				اگر (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ادامه;

				matched_cookies[شمار++] = c;

				GROW_MATCHED_COOKIES;

				اگر (c->version < (unsigned صحیح)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ساختار cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	برای (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		برای (q = p->children; q; q = q->next) {
			اگر (*(q->segment) != '\0')
				ادامه;

			برای (c = q->cookies; c; c = c->next) {
				اگر (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ادامه;

				اگر (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ادامه;

				matched_cookies[شمار++] = c;

				GROW_MATCHED_COOKIES;

				اگر (c->version < (unsigned صحیح) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ساختار cookie_data *)c);
			}
		}

		اگر (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			بشکن;
		}

		/* Consider p itself - may be the result of Path=/foo */
		برای (c = p->cookies; c; c = c->next) {
			اگر (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ادامه;

			/* Ensure cookie path is a prefix of the resource */
			اگر (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ادامه;

			اگر (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ادامه;

			matched_cookies[شمار++] = c;

			GROW_MATCHED_COOKIES;

			اگر (c->version < (unsigned صحیح) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ساختار cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	برای (h = (ثابت ساختار host_part *)p; h && h != &db_ریشه;
	     h = h->parent) {
		برای (c = h->paths.cookies; c; c = c->next) {
			اگر (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ادامه;

			/* Ensure cookie path is a prefix of the resource */
			اگر (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ادامه;

			اگر (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				ادامه;

			matched_cookies[شمار++] = c;

			GROW_MATCHED_COOKIES;

			اگر (c->version < (unsigned صحیح)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ساختار cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	اگر (شمار == 0) {
		/* No cookies found */
		آزاد(matched_cookies);
		بازگشت پوچ;
	}

	اگر (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, شمار);
	} وگرنه {
		ret = get_rfc2965_cookie(matched_cookies, شمار, version);
	}
	آزاد(matched_cookies);

	بازگشت ret;
}


/* exported interface documented in content/urldb.h */
تهی urldb_delete_cookie(ثابت نویسه *domain, ثابت نویسه *path,
			 ثابت نویسه *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_ریشه);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(ثابت نویسه *filename)
{
	FILE *fp;
	نویسه s[16*1024];

	أكد(filename);

	fp = fopen(filename, "r");
	اگر (!fp)
		بازگشت NSERROR_NOT_FOUND;

#define FIND_T {				\
		برای (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		اگر (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ادامه;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		برای (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		اگر (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ادامه;		\
		}				\
	}

	وقتی (fgets(s, اندازه s, fp)) {
		نویسه *p = s, *end = 0,
			*domain, *path, *name, *مقدار, *scheme, *url,
			*comment;
		صحیح version, domain_specified, path_specified,
			secure, http_only, no_destroy, مقدار_quoted;
		time_t expires, last_used;
		ساختار cookie_internal_data *c;

		اگر(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ادامه;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		اگر (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			اگر (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				بشکن;
			}

			ادامه;
		} وگرنه اگر (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			ادامه;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		اگر (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} وگرنه {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; مقدار = p; FIND_T;
		اگر (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	مقدار_quoted = atoi(p); FIND_T;
		} وگرنه {
			مقدار_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		برای (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		أكد(p <= end);

		/* Now create cookie */
		c = malloc(اندازه(ساختار cookie_internal_data));
		اگر (!c)
			بشکن;

		c->name = strdup(name);
		c->مقدار = strdup(مقدار);
		c->مقدار_was_quoted = مقدار_quoted;
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

		اگر (!(c->name && c->مقدار && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			بشکن;
		}

		اگر (c->domain[0] != '.') {
			lwc_string *scheme_lwc = پوچ;
			nsurl *url_nsurl = پوچ;

			أكد(scheme[0] != 'u');

			اگر (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				بشکن;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			اگر (!urldb_درج_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				بشکن;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} وگرنه {
			اگر (!urldb_درج_cookie(c, پوچ, پوچ)) {
				/* Cookie freed for us */
				بشکن;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	بازگشت NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ثابت نویسه *filename)
{
	FILE *fp;
	صحیح cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	أكد(filename);

	fp = fopen(filename, "w");
	اگر (!fp)
		بازگشت NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_ریشه);

	fclose(fp);

	بازگشت NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
تهی urldb_dump(تهی)
{
	صحیح i;

	urldb_dump_hosts(&db_ریشه);

	برای (i = 0; i != رقم_جستجو_TREES; i++) {
		urldb_dump_جستجو(جستجو_trees[i], 0);
	}
}
