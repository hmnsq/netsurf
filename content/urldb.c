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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <time.h>
#ifdef WITH_NSPSL
#অন্তর্ভাব <nspsl.h>
#endif

#অন্তর্ভাব "utils/inet.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/url.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/bloom.h"
#অন্তর্ভাব "utils/time.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/ascii.h"
#অন্তর্ভাব "utils/http.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "desktop/cookie_manager.h"

#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
সংরচনা cookie_internal_data {
	সংরচনা cookie_internal_data *prev;	/**< Previous in list */
	সংরচনা cookie_internal_data *next;	/**< Next in list */

	বর্ণ *name;		/**< Cookie name */
	বর্ণ *মান;		/**< Cookie value */
	bool মান_was_quoted;	/**< Value was quoted in Set-Cookie: */
	বর্ণ *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	বর্ণ *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	বর্ণ *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	গণনা cookie_version version;	/**< Specification compliance */
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
সংরচনা prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned পূর্ণাঙ্ক port;
	/** Protection realm */
	বর্ণ *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	বর্ণ *auth;
	/** Next sibling */
	সংরচনা prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
সংরচনা url_internal_data {
	বর্ণ *title;		/**< Resource title */
	unsigned পূর্ণাঙ্ক visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
সংরচনা path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned পূর্ণাঙ্ক port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	বর্ণ *segment;		/**< Path segment for this node */
	unsigned পূর্ণাঙ্ক frag_cnt;	/**< Number of entries in path_data::fragment */
	বর্ণ **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	সংরচনা url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ধ্রুব সংরচনা prot_space_data *prot_space;
	/** Cookies associated with resource */
	সংরচনা cookie_internal_data *cookies;
	/** Last cookie in list */
	সংরচনা cookie_internal_data *cookies_end;

	সংরচনা path_data *next;	/**< Next sibling */
	সংরচনা path_data *prev;	/**< Previous sibling */
	সংরচনা path_data *parent; /**< Parent path segment */
	সংরচনা path_data *children; /**< Child path segments */
	সংরচনা path_data *last; /**< Last child */
};

সংরচনা hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

সংরচনা host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	সংরচনা path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	সংরচনা hsts_data hsts;

	/**
	 * Part of host string
	 */
	বর্ণ *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	সংরচনা prot_space_data *prot_space;

	সংরচনা host_part *next;	/**< Next sibling */
	সংরচনা host_part *prev;	/**< Previous sibling */
	সংরচনা host_part *parent; /**< Parent host part */
	সংরচনা host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
সংরচনা অনুসন্ধান_গ্রন্থি {
	ধ্রুব সংরচনা host_part *data;	/**< Host tree entry */

	unsigned পূর্ণাঙ্ক level;		/**< Node level */

	সংরচনা অনুসন্ধান_গ্রন্থি *বাম;	/**< Left subtree */
	সংরচনা অনুসন্ধান_গ্রন্থি *ডান;	/**< Right subtree */
};

/** Root database handle */
স্থির সংরচনা host_part db_মূল;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define অঙ্ক_অনুসন্ধান_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
স্থির সংরচনা অনুসন্ধান_গ্রন্থি empty = { 0, 0, &empty, &empty };
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *অনুসন্ধান_trees[অঙ্ক_অনুসন্ধান_TREES] = {
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
স্থির পূর্ণাঙ্ক loaded_cookie_file_version;

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
স্থির সংরচনা bloom_filter *url_bloom;
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
স্থির nserror urldb_write_timet(FILE *fp, time_t val)
{
	পূর্ণাঙ্ক use;
	বর্ণ op[32];

	use = nsc_sntimet(op, 32, &val);
	যদি (use == 0) {
		fprintf(fp, "%i\n", (পূর্ণাঙ্ক)val);
	} অন্যথা {
		fprintf(fp, "%.*s\n", use, op);
	}
	প্রত্যাগম NSERROR_OK;
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
স্থির শূন্য
urldb_write_paths(ধ্রুব সংরচনা path_data *parent,
		  ধ্রুব বর্ণ *host,
		  FILE *fp,
		  বর্ণ **path,
		  পূর্ণাঙ্ক *path_alloc,
		  পূর্ণাঙ্ক *path_used,
		  time_t expiry)
{
	ধ্রুব সংরচনা path_data *p = parent;
	পূর্ণাঙ্ক i;

	কুরু {
		পূর্ণাঙ্ক seglen = p->segment != রিক্ত ? strlen(p->segment) : 0;
		পূর্ণাঙ্ক len = *path_used + seglen + 1;

		যদি (*path_alloc < len) {
			বর্ণ *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			যদি (!temp) {
				প্রত্যাগম;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		যদি (p->segment != রিক্ত) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		যদি (p->children != রিক্ত) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} অন্যথা {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		যদি (p->children != রিক্ত) {
			/* Drill down into children */
			p = p->children;
		} অন্যথা {
			/* leaf node */
			যদি (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				যদি (p->port) {
					fprintf(fp,"%d\n", p->port);
				} অন্যথা {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (পূর্ণাঙ্ক)p->urld.type);

				fprintf(fp, "\n");

				যদি (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					প্রতি (i = 0; s[i] != '\0'; i++)
						যদি (s[i] < 32)
							s[i] = ' ';
					প্রতি (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} অন্যথা {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			যাবত্ (p != parent) {
				পূর্ণাঙ্ক seglen = p->segment != রিক্ত
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				যদি (p->next != রিক্ত) {
					/* Have a sibling, process that */
					p = p->next;
					ভঙ্গ;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} যাবত্ (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
স্থির শূন্য
urldb_গুনতি_urls(ধ্রুব সংরচনা path_data *মূল,
		 time_t expiry,
		 unsigned পূর্ণাঙ্ক *গুনতি)
{
	ধ্রুব সংরচনা path_data *p = মূল;

	কুরু {
		যদি (p->children != রিক্ত) {
			/* Drill down into children */
			p = p->children;
		} অন্যথা {
			/* No more children, increment count if required */
			যদি (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*গুনতি)++;
			}

			/* Now, find next node to process. */
			যাবত্ (p != মূল) {
				যদি (p->next != রিক্ত) {
					/* Have a sibling, process that */
					p = p->next;
					ভঙ্গ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} যাবত্ (p != মূল);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
স্থির শূন্য urldb_save_অনুসন্ধান_বৃক্ষ(সংরচনা অনুসন্ধান_গ্রন্থি *parent, FILE *fp)
{
	বর্ণ host[256];
	ধ্রুব সংরচনা host_part *h;
	unsigned পূর্ণাঙ্ক path_গুনতি = 0;
	বর্ণ *path, *p, *end;
	পূর্ণাঙ্ক path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	পূর্ণাঙ্ক hsts_include_subdomains = 0;

	expiry = time(রিক্ত) - ((60 * 60 * 24) * nsoption_int(expire_url));

	যদি (parent == &empty)
		প্রত্যাগম;

	urldb_save_অনুসন্ধান_বৃক্ষ(parent->বাম, fp);

	path = malloc(path_alloc);
	যদি (!path)
		প্রত্যাগম;

	path[0] = '\0';

	প্রতি (h = parent->data, p = host, end = host + পরিমাণ host;
	     h && h != &db_মূল && p < end; h = h->parent) {
		পূর্ণাঙ্ক written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		যদি (written < 0) {
			মুক্ত(path);
			প্রত্যাগম;
		}
		p += written;
	}

	h = parent->data;
	যদি (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_গুনতি_urls(&parent->data->paths, expiry, &path_গুনতি);

	যদি (path_গুনতি > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_গুনতি);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} অন্যথা যদি (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	মুক্ত(path);

	urldb_save_অনুসন্ধান_বৃক্ষ(parent->ডান, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
স্থির bool
urldb_iterate_entries_path(ধ্রুব সংরচনা path_data *parent,
		bool (*url_callback)(nsurl *url, ধ্রুব সংরচনা url_data *data),
		bool (*cookie_callback)(ধ্রুব সংরচনা cookie_data *data))
{
	ধ্রুব সংরচনা path_data *p = parent;
	ধ্রুব সংরচনা cookie_data *c;

	কুরু {
		যদি (p->children != রিক্ত) {
			/* Drill down into children */
			p = p->children;
		} অন্যথা {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			প্রতিজ্ঞা(url_callback || cookie_callback);

			/** \todo handle fragments? */
			যদি (url_callback) {
				ধ্রুব সংরচনা url_internal_data *u = &p->urld;

				প্রতিজ্ঞা(p->url);

				যদি (!url_callback(p->url,
						  (ধ্রুব সংরচনা url_data *) u))
					প্রত্যাগম false;
			} অন্যথা {
				c = (ধ্রুব সংরচনা cookie_data *)p->cookies;
				প্রতি (; c != রিক্ত; c = c->next) {
					যদি (!cookie_callback(c))
						প্রত্যাগম false;
				}
			}

			/* Now, find next node to process. */
			যাবত্ (p != parent) {
				যদি (p->next != রিক্ত) {
					/* Have a sibling, process that */
					p = p->next;
					ভঙ্গ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} যাবত্ (p != parent);

	প্রত্যাগম true;
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
স্থির bool urldb__host_is_ip_address(ধ্রুব বর্ণ *host)
{
	সংরচনা in_addr ipv4;
	size_t host_len = strlen(host);
	ধ্রুব বর্ণ *sane_host;
	ধ্রুব বর্ণ *slash;
#ifndef NO_IPV6
	সংরচনা in6_addr ipv6;
	বর্ণ ipv6_addr[64];
	unsigned পূর্ণাঙ্ক ipv6_addr_len;
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
	যদি (slash == রিক্ত) {
		sane_host = host;
	} অন্যথা {
		বর্ণ *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	যদি (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		গচ্ছ out_false;

	যদি (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		পূর্ণাঙ্ক অঙ্ক_dots = 0;
		size_t index;

		প্রতি (index = 0; index < host_len; index++) {
			যদি (sane_host[index] == '.')
				অঙ্ক_dots++;
		}

		যদি (অঙ্ক_dots == 3)
			গচ্ছ out_true;
		অন্যথা
			গচ্ছ out_false;
	}

#ifndef NO_IPV6
	যদি ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		গচ্ছ out_false;
	}

	ipv6_addr_len = host_len - 2;
	যদি (ipv6_addr_len >= পরিমাণ(ipv6_addr)) {
		ipv6_addr_len = পরিমাণ(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	যদি (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		গচ্ছ out_true;
#endif

out_false:
	যদি (slash != রিক্ত) মুক্ত((শূন্য *)sane_host);
	প্রত্যাগম false;

out_true:
	যদি (slash != রিক্ত) মুক্ত((শূন্য *)sane_host);
	প্রত্যাগম true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
স্থির পূর্ণাঙ্ক urldb_অনুসন্ধান_match_prefix(ধ্রুব সংরচনা host_part *a, ধ্রুব বর্ণ *b)
{
	ধ্রুব বর্ণ *end, *dot;
	পূর্ণাঙ্ক plen, ret;

	প্রতিজ্ঞা(a && a != &db_মূল && b);

	যদি (urldb__host_is_ip_address(b)) {
		/* IP address */
		প্রত্যাগম strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	যাবত্ (b < end && a && a != &db_মূল) {
		dot = strchr(b, '.');
		যদি (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		যদি ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			প্রত্যাগম ret;

		/* The strings matched */
		যদি (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			যদি (plen > dot - b) {
				/* len(a) > len(b) */
				প্রত্যাগম 1;
			} অন্যথা যদি (plen < dot - b) {
				/* len(a) < len(b) */
				প্রত্যাগম -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	যদি (a && a != &db_মূল && b >= end) {
		/* len(a) > len(b) => prefix matches */
		প্রত্যাগম 0;
	} অন্যথা যদি ((!a || a == &db_মূল) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		প্রত্যাগম -1;
	}

	/* Identical */
	প্রত্যাগম 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
স্থির bool
urldb_iterate_partial_host(সংরচনা অনুসন্ধান_গ্রন্থি *মূল,
		ধ্রুব বর্ণ *prefix,
		bool (*callback)(nsurl *url, ধ্রুব সংরচনা url_data *data))
{
	পূর্ণাঙ্ক c;

	প্রতিজ্ঞা(মূল && prefix && callback);

	যদি (মূল == &empty)
		প্রত্যাগম true;

	c = urldb_অনুসন্ধান_match_prefix(মূল->data, prefix);

	যদি (c > 0) {
		/* No match => look in left subtree */
		প্রত্যাগম urldb_iterate_partial_host(মূল->বাম,
						  prefix,
						  callback);
	} অন্যথা যদি (c < 0) {
		/* No match => look in right subtree */
		প্রত্যাগম urldb_iterate_partial_host(মূল->ডান,
						  prefix,
						  callback);
	} অন্যথা {
		/* Match => iterate over l/r subtrees & process this node */
		যদি (!urldb_iterate_partial_host(মূল->বাম,
						prefix,
						callback)) {
			প্রত্যাগম false;
		}

		যদি (মূল->data->paths.children) {
			/* and extract all paths attached to this host */
			যদি (!urldb_iterate_entries_path(&মূল->data->paths,
							callback,
							রিক্ত)) {
				প্রত্যাগম false;
			}
		}

		যদি (!urldb_iterate_partial_host(মূল->ডান,
						prefix,
						callback)) {
			প্রত্যাগম false;
		}
	}

	প্রত্যাগম true;
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
স্থির bool
urldb_iterate_partial_path(ধ্রুব সংরচনা path_data *parent,
		ধ্রুব বর্ণ *prefix,
		bool (*callback)(nsurl *url, ধ্রুব সংরচনা url_data *data))
{
	ধ্রুব সংরচনা path_data *p = parent->children;
	ধ্রুব বর্ণ *slash, *end = prefix + strlen(prefix);

	কুরু {
		slash = strchr(prefix, '/');
		যদি (!slash) {
			slash = end;
		}

		যদি (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			অনুবর্ত;
		}

		যদি (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			যদি (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				যদি (!urldb_iterate_entries_path(p,
								callback,
								রিক্ত)) {
					প্রত্যাগম false;
				}

				/* Progress to next sibling */
				p = p->next;
			} অন্যথা {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} অন্যথা {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} যাবত্ (p != রিক্ত);

	প্রত্যাগম true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
স্থির bool
urldb_iterate_entries_host(সংরচনা অনুসন্ধান_গ্রন্থি *parent,
		bool (*url_callback)(nsurl *url, ধ্রুব সংরচনা url_data *data),
		bool (*cookie_callback)(ধ্রুব সংরচনা cookie_data *data))
{
	যদি (parent == &empty) {
		প্রত্যাগম true;
	}

	যদি (!urldb_iterate_entries_host(parent->বাম,
					url_callback,
					cookie_callback)) {
		প্রত্যাগম false;
	}

	যদি ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		যদি (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			প্রত্যাগম false;
		}
	}

	যদি (!urldb_iterate_entries_host(parent->ডান,
					url_callback,
					cookie_callback)) {
		প্রত্যাগম false;
	}

	প্রত্যাগম true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
স্থির সংরচনা host_part *
urldb_add_host_গ্রন্থি(ধ্রুব বর্ণ *part, সংরচনা host_part *parent)
{
	সংরচনা host_part *d;

	প্রতিজ্ঞা(part && parent);

	d = calloc(1, পরিমাণ(সংরচনা host_part));
	যদি (!d) {
		প্রত্যাগম রিক্ত;
	}

	d->part = strdup(part);
	যদি (!d->part) {
		মুক্ত(d);
		প্রত্যাগম রিক্ত;
	}

	d->next = parent->children;
	যদি (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	প্রত্যাগম d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
স্থির পূর্ণাঙ্ক urldb_add_path_fragment_cmp(ধ্রুব শূন্য *a, ধ্রুব শূন্য *b)
{
	প্রত্যাগম strcasecmp(*((ধ্রুব বর্ণ **) a), *((ধ্রুব বর্ণ **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
স্থির সংরচনা path_data *
urldb_add_path_fragment(সংরচনা path_data *segment, lwc_string *fragment)
{
	বর্ণ **temp;

	প্রতিজ্ঞা(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	যদি (!fragment)
		প্রত্যাগম segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * পরিমাণ(বর্ণ *));
	যদি (!temp)
		প্রত্যাগম রিক্ত;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	যদি (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		প্রত্যাগম রিক্ত;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      পরিমাণ (বর্ণ *),
	      urldb_add_path_fragment_cmp);

	প্রত্যাগম segment;
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
স্থির সংরচনা path_data *
urldb_add_path_গ্রন্থি(lwc_string *scheme,
		    unsigned পূর্ণাঙ্ক port,
		    ধ্রুব বর্ণ *segment,
		    lwc_string *fragment,
		    সংরচনা path_data *parent)
{
	সংরচনা path_data *d, *e;

	প্রতিজ্ঞা(scheme && segment && parent);

	d = calloc(1, পরিমাণ(সংরচনা path_data));
	যদি (!d)
		প্রত্যাগম রিক্ত;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	যদি (!d->segment) {
		lwc_string_unref(d->scheme);
		মুক্ত(d);
		প্রত্যাগম রিক্ত;
	}

	যদি (fragment) {
		যদি (!urldb_add_path_fragment(d, fragment)) {
			মুক্ত(d->segment);
			lwc_string_unref(d->scheme);
			মুক্ত(d);
			প্রত্যাগম রিক্ত;
		}
	}

	প্রতি (e = parent->children; e; e = e->next) {
		যদি (strcmp(e->segment, d->segment) > 0)
			ভঙ্গ;
	}

	যদি (e) {
		d->prev = e->prev;
		d->next = e;
		যদি (e->prev)
			e->prev->next = d;
		অন্যথা
			parent->children = d;
		e->prev = d;
	} অন্যথা যদি (!parent->children) {
		d->prev = d->next = রিক্ত;
		parent->children = parent->last = d;
	} অন্যথা {
		d->next = রিক্ত;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	প্রত্যাগম d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি **urldb_get_অনুসন্ধান_বৃক্ষ_direct(ধ্রুব বর্ণ *host)
{
	প্রতিজ্ঞা(host);

	যদি (urldb__host_is_ip_address(host)) {
		প্রত্যাগম &অনুসন্ধান_trees[ST_IP];
	} অন্যথা যদি (ascii_is_alpha(*host)) {
		প্রত্যাগম &অনুসন্ধান_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	প্রত্যাগম &অনুসন্ধান_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *urldb_get_অনুসন্ধান_বৃক্ষ(ধ্রুব বর্ণ *host)
{
	প্রত্যাগম *urldb_get_অনুসন্ধান_বৃক্ষ_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
স্থির পূর্ণাঙ্ক urldb_অনুসন্ধান_match_string(ধ্রুব সংরচনা host_part *a, ধ্রুব বর্ণ *b)
{
	ধ্রুব বর্ণ *end, *dot;
	পূর্ণাঙ্ক plen, ret;

	প্রতিজ্ঞা(a && a != &db_মূল && b);

	যদি (urldb__host_is_ip_address(b)) {
		/* IP address */
		প্রত্যাগম strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	যাবত্ (b < end && a && a != &db_মূল) {
		dot = strchr(b, '.');
		যদি (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		যদি ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			প্রত্যাগম ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		যদি (plen > dot - b) {
			/* len(a) > len(b) */
			প্রত্যাগম 1;
		} অন্যথা যদি (plen < dot - b) {
			/* len(a) < len(b) */
			প্রত্যাগম -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	যদি (a && a != &db_মূল && b >= end) {
		/* len(a) > len(b) */
		প্রত্যাগম 1;
	} অন্যথা যদি ((!a || a == &db_মূল) && b < end) {
		/* len(a) < len(b) */
		প্রত্যাগম -1;
	}

	/* Identical */
	প্রত্যাগম 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
স্থির ধ্রুব সংরচনা host_part *
urldb_অনুসন্ধান_find(সংরচনা অনুসন্ধান_গ্রন্থি *মূল, ধ্রুব বর্ণ *host)
{
	পূর্ণাঙ্ক c;

	প্রতিজ্ঞা(মূল && host);

	যদি (মূল == &empty) {
		প্রত্যাগম রিক্ত;
	}

	c = urldb_অনুসন্ধান_match_string(মূল->data, host);

	যদি (c > 0) {
		প্রত্যাগম urldb_অনুসন্ধান_find(মূল->বাম, host);
	} অন্যথা যদি (c < 0) {
		প্রত্যাগম urldb_অনুসন্ধান_find(মূল->ডান, host);
	}

	প্রত্যাগম মূল->data;
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
স্থির সংরচনা path_data *
urldb_match_path(ধ্রুব সংরচনা path_data *parent,
		 ধ্রুব বর্ণ *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	ধ্রুব সংরচনা path_data *p;
	ধ্রুব বর্ণ *slash;
	bool match;

	প্রতিজ্ঞা(parent != রিক্ত);
	প্রতিজ্ঞা(parent->segment == রিক্ত);

	যদি (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	প্রতিজ্ঞা(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	যাবত্ (p != রিক্ত) {
		slash = strchr(path + 1, '/');
		যদি (!slash) {
			slash = path + strlen(path);
		}

		যদি (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			যদি (*slash == '\0') {
				/* Complete match */
				প্রত্যাগম (সংরচনা path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} অন্যথা {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	প্রত্যাগম রিক্ত;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
স্থির সংরচনা path_data *urldb_find_url(nsurl *url)
{
	ধ্রুব সংরচনা host_part *h;
	সংরচনা path_data *p;
	সংরচনা অনুসন্ধান_গ্রন্থি *বৃক্ষ;
	বর্ণ *plq;
	ধ্রুব বর্ণ *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned পূর্ণাঙ্ক port_int;
	bool match;

	প্রতিজ্ঞা(url);

	যদি (url_bloom != রিক্ত) {
		যদি (bloom_অনুসন্ধান_hash(url_bloom, nsurl_hash(url)) == false) {
			প্রত্যাগম রিক্ত;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	যদি (scheme == রিক্ত)
		প্রত্যাগম রিক্ত;

	যদি (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		প্রত্যাগম রিক্ত;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	যদি (host != রিক্ত) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} অন্যথা যদি (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} অন্যথা {
		lwc_string_unref(scheme);
		প্রত্যাগম রিক্ত;
	}

	বৃক্ষ = urldb_get_অনুসন্ধান_বৃক্ষ(host_str);
	h = urldb_অনুসন্ধান_find(বৃক্ষ, host_str);
	যদি (!h) {
		lwc_string_unref(scheme);
		প্রত্যাগম রিক্ত;
	}

	/* generate plq (path, leaf, query) */
	যদি (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		প্রত্যাগম রিক্ত;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	যদি (port != রিক্ত) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} অন্যথা {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	মুক্ত(plq);
	lwc_string_unref(scheme);

	প্রত্যাগম p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
স্থির শূন্য urldb_dump_paths(সংরচনা path_data *parent)
{
	ধ্রুব সংরচনা path_data *p = parent;
	unsigned পূর্ণাঙ্ক i;

	কুরু {
		যদি (p->segment != রিক্ত) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			প্রতি (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		যদি (p->children != রিক্ত) {
			p = p->children;
		} অন্যথা {
			যাবত্ (p != parent) {
				যদি (p->next != রিক্ত) {
					p = p->next;
					ভঙ্গ;
				}

				p = p->parent;
			}
		}
	} যাবত্ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
স্থির শূন্য urldb_dump_hosts(সংরচনা host_part *parent)
{
	সংরচনা host_part *h;

	যদি (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	প্রতি (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
স্থির শূন্য urldb_dump_অনুসন্ধান(সংরচনা অনুসন্ধান_গ্রন্থি *parent, পূর্ণাঙ্ক depth)
{
	ধ্রুব সংরচনা host_part *h;
	পূর্ণাঙ্ক i; /* index into string */
	বর্ণ s[1024];
	পূর্ণাঙ্ক r;
	পূর্ণাঙ্ক sl = পরিমাণ(s) - 2;

	যদি (parent == &empty)
		প্রত্যাগম;

	urldb_dump_অনুসন্ধান(parent->বাম, depth + 1);

	প্রতি (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	প্রতি (h = parent->data; h; h = h->parent) {
		যদি (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			যদি (r < 0) {
				ভঙ্গ;
			}
			যদি ((i + r) >= sl) {
				ভঙ্গ;
			}
			i += r;
		}

		যদি (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_অনুসন্ধান(parent->ডান, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
স্থির পূর্ণাঙ্ক
urldb_অনুসন্ধান_match_host(ধ্রুব সংরচনা host_part *a, ধ্রুব সংরচনা host_part *b)
{
	পূর্ণাঙ্ক ret;

	প্রতিজ্ঞা(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	প্রতি (; a && a != &db_মূল && b && b != &db_মূল;
	     a = a->parent, b = b->parent) {
		যদি ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			প্রত্যাগম ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	যদি (a && a != &db_মূল && (!b || b == &db_মূল)) {
		/* len(a) > len(b) */
		প্রত্যাগম 1;
	} অন্যথা যদি ((!a || a == &db_মূল) && b && b != &db_মূল) {
		/* len(a) < len(b) */
		প্রত্যাগম -1;
	}

	/* identical */
	প্রত্যাগম 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *urldb_অনুসন্ধান_skew(সংরচনা অনুসন্ধান_গ্রন্থি *মূল)
{
	প্রতিজ্ঞা(মূল);

	যদি (মূল->বাম->level == মূল->level) {
		সংরচনা অনুসন্ধান_গ্রন্থি *temp;

		temp = মূল->বাম;
		মূল->বাম = temp->ডান;
		temp->ডান = মূল;
		মূল = temp;
	}

	প্রত্যাগম মূল;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *urldb_অনুসন্ধান_split(সংরচনা অনুসন্ধান_গ্রন্থি *মূল)
{
	প্রতিজ্ঞা(মূল);

	যদি (মূল->ডান->ডান->level == মূল->level) {
		সংরচনা অনুসন্ধান_গ্রন্থি *temp;

		temp = মূল->ডান;
		মূল->ডান = temp->বাম;
		temp->বাম = মূল;
		মূল = temp;

		মূল->level++;
	}

	প্রত্যাগম মূল;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *
urldb_অনুসন্ধান_ঢোকাও_internal(সংরচনা অনুসন্ধান_গ্রন্থি *মূল, সংরচনা অনুসন্ধান_গ্রন্থি *n)
{
	প্রতিজ্ঞা(মূল && n);

	যদি (মূল == &empty) {
		মূল = n;
	} অন্যথা {
		পূর্ণাঙ্ক c = urldb_অনুসন্ধান_match_host(মূল->data, n->data);

		যদি (c > 0) {
			মূল->বাম = urldb_অনুসন্ধান_ঢোকাও_internal(
				মূল->বাম, n);
		} অন্যথা যদি (c < 0) {
			মূল->ডান = urldb_অনুসন্ধান_ঢোকাও_internal(
				মূল->ডান, n);
		} অন্যথা {
			/* exact match */
			মুক্ত(n);
			প্রত্যাগম মূল;
		}

		মূল = urldb_অনুসন্ধান_skew(মূল);
		মূল = urldb_অনুসন্ধান_split(মূল);
	}

	প্রত্যাগম মূল;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
স্থির সংরচনা অনুসন্ধান_গ্রন্থি *
urldb_অনুসন্ধান_ঢোকাও(সংরচনা অনুসন্ধান_গ্রন্থি *মূল, ধ্রুব সংরচনা host_part *data)
{
	সংরচনা অনুসন্ধান_গ্রন্থি *n;

	প্রতিজ্ঞা(মূল && data);

	n = malloc(পরিমাণ(সংরচনা অনুসন্ধান_গ্রন্থি));
	যদি (!n)
		প্রত্যাগম রিক্ত;

	n->level = 1;
	n->data = data;
	n->বাম = n->ডান = &empty;

	মূল = urldb_অনুসন্ধান_ঢোকাও_internal(মূল, n);

	প্রত্যাগম মূল;
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
স্থির bool
urldb_parse_avpair(সংরচনা cookie_internal_data *c,
		   বর্ণ *n,
		   বর্ণ *v,
		   bool was_quoted)
{
	পূর্ণাঙ্ক vlen;

	প্রতিজ্ঞা(c && n && v);

	/* Strip whitespace from start of name */
	প্রতি (; *n; n++) {
		যদি (*n != ' ' && *n != '\t')
			ভঙ্গ;
	}

	/* Strip whitespace from end of name */
	প্রতি (vlen = strlen(n); vlen; vlen--) {
		যদি (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		অন্যথা
			ভঙ্গ;
	}

	/* Strip whitespace from start of value */
	প্রতি (; *v; v++) {
		যদি (*v != ' ' && *v != '\t')
			ভঙ্গ;
	}

	/* Strip whitespace from end of value */
	প্রতি (vlen = strlen(v); vlen; vlen--) {
		যদি (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		অন্যথা
			ভঙ্গ;
	}

	যদি (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		যদি (!c->comment)
			প্রত্যাগম false;
	} অন্যথা যদি (!c->domain && strcasecmp(n, "Domain") == 0) {
		যদি (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			যদি (!c->domain)
				প্রত্যাগম false;
		}
	} অন্যথা যদি (strcasecmp(n, "Max-Age") == 0) {
		পূর্ণাঙ্ক temp = atoi(v);
		যদি (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		অন্যথা
			c->expires = time(রিক্ত) + temp;
	} অন্যথা যদি (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		যদি (!c->path)
			প্রত্যাগম false;
	} অন্যথা যদি (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} অন্যথা যদি (strcasecmp(n, "Expires") == 0) {
		বর্ণ *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		প্রতি (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		যদি (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} অন্যথা যদি (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} অন্যথা যদি (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} অন্যথা যদি (!c->name) {
		c->name = strdup(n);
		c->মান = strdup(v);
		c->মান_was_quoted = was_quoted;
		যদি (!c->name || !c->মান) {
			প্রত্যাগম false;
		}
	}

	প্রত্যাগম true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
স্থির শূন্য urldb_free_cookie(সংরচনা cookie_internal_data *c)
{
	প্রতিজ্ঞা(c);

	মুক্ত(c->comment);
	মুক্ত(c->domain);
	মুক্ত(c->path);
	মুক্ত(c->name);
	মুক্ত(c->মান);
	মুক্ত(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
স্থির সংরচনা cookie_internal_data *
urldb_parse_cookie(nsurl *url, ধ্রুব বর্ণ **cookie)
{
	সংরচনা cookie_internal_data *c;
	ধ্রুব বর্ণ *cur;
	বর্ণ name[1024], মান[4096];
	বর্ণ *n = name, *v = মান;
	bool in_মান = false;
	bool had_মান_data = false;
	bool মান_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	প্রতিজ্ঞা(url && cookie && *cookie);

	c = calloc(1, পরিমাণ(সংরচনা cookie_internal_data));
	যদি (c == রিক্ত)
		প্রত্যাগম রিক্ত;

	c->expires = -1;

	name[0] = '\0';
	মান[0] = '\0';

	প্রতি (cur = *cookie; *cur; cur++) {
		যদি (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			যদি (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				মান[0] = '\0';

			}

			ভঙ্গ;
		} অন্যথা যদি (*cur == '\r') {
			/* Spurious linefeed */
			অনুবর্ত;
		} অন্যথা যদি (*cur == '\n') {
			/* Spurious newline */
			অনুবর্ত;
		}

		যদি (in_মান && !had_মান_data) {
			যদি (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				অনুবর্ত;
			} অন্যথা {
				had_মান_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				যদি (*cur != '"') {
					মান_verbatim = true;
				}
			}
		}

		যদি (in_মান && !মান_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			যদি (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				অনুবর্ত;
			}
		}

		যদি (!quoted && !in_মান && *cur == '=') {
			/* First equals => attr-value separator */
			in_মান = true;
			অনুবর্ত;
		}

		যদি (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			যদি (!urldb_parse_avpair(c, name, মান, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				প্রত্যাগম রিক্ত;
			}

			/* And reset to start */
			n = name;
			v = মান;
			in_মান = false;
			had_মান_data = false;
			মান_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			যদি (*cur != ';') {
				cur--;
			}

			অনুবর্ত;
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
		যদি (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ধ্রুব বর্ণ *p;
			ধ্রুব বর্ণ *semi = strchr(cur + 1, ';');
			যদি (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			প্রতি (p = cur + 1; p < semi; p++)
				যদি (*p == '=')
					ভঙ্গ;

			যদি (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} অন্যথা {
				/* found one => comma marks end of cookie */
				cur++;
				ভঙ্গ;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		যদি (!in_মান) {
			যদি (n < name + (পরিমাণ(name) - 1))
				*n++ = *cur;
		} অন্যথা {
			যদি (v < মান + (পরিমাণ(মান) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	যদি (!urldb_parse_avpair(c, name, মান, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		প্রত্যাগম রিক্ত;
	}

	/* Now fix-up default values */
	যদি (c->domain == রিক্ত) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		যদি (host == রিক্ত) {
			urldb_free_cookie(c);
			প্রত্যাগম রিক্ত;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	যদি (c->path == রিক্ত) {
		ধ্রুব বর্ণ *path_data;
		বর্ণ *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		যদি (path_lwc == রিক্ত) {
			urldb_free_cookie(c);
			প্রত্যাগম রিক্ত;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		যদি (slash != রিক্ত) {
			/* Special case: retain first slash in path */
			যদি (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			যদি (slash == রিক্ত) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				প্রত্যাগম রিক্ত;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} অন্যথা {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			যদি (path == রিক্ত) {
				urldb_free_cookie(c);
				প্রত্যাগম রিক্ত;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	প্রত্যাগম c;
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
স্থির সংরচনা path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned পূর্ণাঙ্ক port,
	       ধ্রুব সংরচনা host_part *host,
	       বর্ণ *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	সংরচনা path_data *d, *e;
	বর্ণ *buf = path_query;
	বর্ণ *segment, *slash;
	bool match;

	প্রতিজ্ঞা(scheme && host && url);

	d = (সংরচনা path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	যদি (*segment == '/')
		segment++;

	/* Process path segments */
	কুরু {
		slash = strchr(segment, '/');
		যদি (!slash) {
			/* last segment */
			/* look for existing entry */
			প্রতি (e = d->children; e; e = e->next)
				যদি (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					ভঙ্গ;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_গ্রন্থি(scheme, port,
						    segment, fragment, d);
			ভঙ্গ;
		}

		*slash = '\0';

		/* look for existing entry */
		প্রতি (e = d->children; e; e = e->next)
			যদি (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				ভঙ্গ;

		d = e ? e : urldb_add_path_গ্রন্থি(scheme, port, segment, রিক্ত, d);
		যদি (!d)
			ভঙ্গ;

		segment = slash + 1;
	} যাবত্ (1);

	মুক্ত(path_query);

	যদি (d && !d->url) {
		/* Insert defragmented URL */
		যদি (nsurl_defragment(url, &d->url) != NSERROR_OK)
			প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
স্থির সংরচনা host_part *urldb_add_host(ধ্রুব বর্ণ *host)
{
	সংরচনা host_part *d = (সংরচনা host_part *) &db_মূল, *e;
	সংরচনা অনুসন্ধান_গ্রন্থি *s;
	বর্ণ buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	বর্ণ *part;

	প্রতিজ্ঞা(host);

	যদি (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		প্রতি (e = d->children; e; e = e->next)
			যদি (strcasecmp(host, e->part) == 0)
				/* found => return it */
				প্রত্যাগম e;

		d = urldb_add_host_গ্রন্থি(host, d);

		s = urldb_অনুসন্ধান_ঢোকাও(অনুসন্ধান_trees[ST_IP], d);
		যদি (!s) {
			/* failed */
			d = রিক্ত;
		} অন্যথা {
			অনুসন্ধান_trees[ST_IP] = s;
		}

		প্রত্যাগম d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, পরিমাণ buf);
	buf[পরিমাণ buf - 1] = '\0';

	/* Process FQDN segments backwards */
	কুরু {
		part = strrchr(buf, '.');
		যদি (!part) {
			/* last segment */
			/* Check for existing entry */
			প্রতি (e = d->children; e; e = e->next)
				যদি (strcasecmp(buf, e->part) == 0)
					ভঙ্গ;

			যদি (e) {
				d = e;
			} অন্যথা {
				d = urldb_add_host_গ্রন্থি(buf, d);
			}

			/* And insert into search tree */
			যদি (d) {
				সংরচনা অনুসন্ধান_গ্রন্থি **r;

				r = urldb_get_অনুসন্ধান_বৃক্ষ_direct(buf);
				s = urldb_অনুসন্ধান_ঢোকাও(*r, d);
				যদি (!s) {
					/* failed */
					d = রিক্ত;
				} অন্যথা {
					*r = s;
				}
			}
			ভঙ্গ;
		}

		/* Check for existing entry */
		প্রতি (e = d->children; e; e = e->next)
			যদি (strcasecmp(part + 1, e->part) == 0)
				ভঙ্গ;

		d = e ? e : urldb_add_host_গ্রন্থি(part + 1, d);
		যদি (!d)
			ভঙ্গ;

		*part = '\0';
	} যাবত্ (1);

	প্রত্যাগম d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
স্থির bool
urldb_ঢোকাও_cookie(সংরচনা cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	সংরচনা cookie_internal_data *d;
	ধ্রুব সংরচনা host_part *h;
	সংরচনা path_data *p;
	time_t now = time(রিক্ত);

	প্রতিজ্ঞা(c);

	যদি (c->domain[0] == '.') {
		h = urldb_অনুসন্ধান_find(
			urldb_get_অনুসন্ধান_বৃক্ষ(&(c->domain[1])),
			c->domain + 1);
		যদি (!h) {
			h = urldb_add_host(c->domain + 1);
			যদি (!h) {
				urldb_free_cookie(c);
				প্রত্যাগম false;
			}
		}

		p = (সংরচনা path_data *) &h->paths;
	} অন্যথা {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		প্রতিজ্ঞা(url != রিক্ত);
		প্রতিজ্ঞা(scheme != রিক্ত);

		h = urldb_অনুসন্ধান_find(
			urldb_get_অনুসন্ধান_বৃক্ষ(c->domain),
			c->domain);

		যদি (!h) {
			h = urldb_add_host(c->domain);
			যদি (!h) {
				urldb_free_cookie(c);
				প্রত্যাগম false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), রিক্ত, url);
		যদি (!p) {
			urldb_free_cookie(c);
			প্রত্যাগম false;
		}
	}

	/* add cookie */
	প্রতি (d = p->cookies; d; d = d->next) {
		যদি (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			ভঙ্গ;
	}

	যদি (d) {
		যদি (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			যদি (d->next)
				d->next->prev = d->prev;
			অন্যথা
				p->cookies_end = d->prev;
			যদি (d->prev)
				d->prev->next = d->next;
			অন্যথা
				p->cookies = d->next;

			cookie_manager_remove((সংরচনা cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} অন্যথা {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			যদি (c->next)
				c->next->prev = c;
			অন্যথা
				p->cookies_end = c;
			যদি (c->prev)
				c->prev->next = c;
			অন্যথা
				p->cookies = c;

			cookie_manager_remove((সংরচনা cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((সংরচনা cookie_data *)c);
		}
	} অন্যথা {
		c->prev = p->cookies_end;
		c->next = রিক্ত;
		যদি (p->cookies_end)
			p->cookies_end->next = c;
		অন্যথা
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((সংরচনা cookie_data *)c);
	}

	প্রত্যাগম true;
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
স্থির bool
concat_netscape_cookie(সংরচনা cookie_internal_data *c,
		    পূর্ণাঙ্ক *used,
		    পূর্ণাঙ্ক *alloc,
		    বর্ণ **buf)
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
	পূর্ণাঙ্ক max_len;
	পূর্ণাঙ্ক outl; /* output length */

	প্রতিজ্ঞা(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->মান) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	যদি (max_len >= *alloc) {
		বর্ণ *temp = realloc(*buf, max_len);
		যদি (!temp) {
			প্রত্যাগম false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	যদি (outl >= (*alloc - *used)) {
		/* output truncated */
		প্রত্যাগম false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	যদি (c->মান_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->মান);
		যদি (outl >= (*alloc - *used)) {
			/* output truncated */
			প্রত্যাগম false;
		}
		*used += outl;
	} অন্যথা {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->মান);
		যদি (outl >= (*alloc - *used)) {
			/* output truncated */
			প্রত্যাগম false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	প্রত্যাগম true;
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
স্থির bool
concat_rfc2965_cookie(সংরচনা cookie_internal_data *c,
		      পূর্ণাঙ্ক *used,
		      পূর্ণাঙ্ক *alloc,
		      বর্ণ **buf)
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
	ধ্রুব বর্ণ * ধ্রুব separators = "()<>@,;:\\\"/[]?={} \t";
	পূর্ণাঙ্ক max_len;
	পূর্ণাঙ্ক outl; /* output length */

	প্রতিজ্ঞা(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->মান) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	যদি (max_len >= *alloc) {
		বর্ণ *temp = realloc(*buf, max_len);
		যদি (!temp) {
			প্রত্যাগম false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	যদি (outl >= (*alloc - *used)) {
		/* output truncated */
		প্রত্যাগম false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	যদি (c->মান_was_quoted ||
	    strpbrk(c->মান, separators) != রিক্ত) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->মান);

	} অন্যথা {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->মান);
	}
	যদি (outl >= (*alloc - *used)) {
		/* output truncated */
		প্রত্যাগম false;
	}
	*used += outl;

	যদি (c->path_from_set) {
		/* Path, quoted if necessary */

		যদি (strpbrk(c->path, separators) != রিক্ত) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} অন্যথা {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		যদি (outl >= (*alloc - *used)) {
			/* output truncated */
			প্রত্যাগম false;
		}
		*used += outl;
	}

	যদি (c->domain_from_set) {
		/* Domain, quoted if necessary */
		যদি (strpbrk(c->domain, separators) != রিক্ত) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} অন্যথা {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		যদি (outl >= (*alloc - *used)) {
			/* output truncated */
			প্রত্যাগম false;
		}
		*used += outl;
	}

	প্রত্যাগম true;
}


/**
 * build output cookie string in netscape format
 */
স্থির বর্ণ *
get_netscape_cookie(সংরচনা cookie_internal_data **matched_cookies, পূর্ণাঙ্ক গুনতি)
{
	পূর্ণাঙ্ক ret_alloc = 4096, ret_used = 1;
	বর্ণ *ret;
	পূর্ণাঙ্ক midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	যদি (ret == রিক্ত) {
		প্রত্যাগম রিক্ত;
	}
	ret[0] = '\0';

	প্রতি (midx = 0; midx < গুনতি; midx++) {
		যদি (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			মুক্ত(ret);
			প্রত্যাগম রিক্ত;
		}
	}

	যদি (ret_used <= 2) {
		মুক্ত(ret);
		প্রত্যাগম রিক্ত;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	বর্ণ *temp = realloc(ret, ret_used);
	যদি (temp != রিক্ত) {
		ret = temp;
	}

	প্রত্যাগম ret;
}


/**
 * build output cookie string in RFC2965 format
 */
স্থির বর্ণ *
get_rfc2965_cookie(সংরচনা cookie_internal_data **matched_cookies,
		   পূর্ণাঙ্ক গুনতি,
		   পূর্ণাঙ্ক version)
{
	পূর্ণাঙ্ক ret_alloc = 4096; /* return string allocation */
	পূর্ণাঙ্ক ret_used = 1; /* return sting buffer used */
	বর্ণ *ret; /* return string buffer */
	পূর্ণাঙ্ক midx; /* index into matched cookies */
	পূর্ণাঙ্ক outl;

	ret = malloc(ret_alloc);
	যদি (ret == রিক্ত) {
		প্রত্যাগম রিক্ত;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	যদি (outl >= ret_alloc - ret_used) {
		/* overflow */
		মুক্ত(ret);
		প্রত্যাগম রিক্ত;
	}
	ret_used += outl;

	প্রতি (midx = 0; midx < গুনতি; midx++) {
		যদি (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			মুক্ত(ret);
			প্রত্যাগম রিক্ত;
		}
	}

	/* shrink the output buffer to the required size */
	বর্ণ *temp = realloc(ret, ret_used);
	যদি (temp != রিক্ত) {
		ret = temp;
	}

	প্রত্যাগম ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
স্থির শূন্য
urldb_delete_cookie_paths(ধ্রুব বর্ণ *domain,
			  ধ্রুব বর্ণ *path,
			  ধ্রুব বর্ণ *name,
			  সংরচনা path_data *parent)
{
	সংরচনা cookie_internal_data *c;
	সংরচনা path_data *p = parent;

	প্রতিজ্ঞা(parent);

	কুরু {
		প্রতি (c = p->cookies; c; c = c->next) {
			যদি (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				যদি (c->prev) {
					c->prev->next = c->next;
				} অন্যথা {
					p->cookies = c->next;
				}

				যদি (c->next) {
					c->next->prev = c->prev;
				} অন্যথা {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				প্রত্যাগম;
			}
		}

		যদি (p->children) {
			p = p->children;
		} অন্যথা {
			যাবত্ (p != parent) {
				যদি (p->next != রিক্ত) {
					p = p->next;
					ভঙ্গ;
				}

				p = p->parent;
			}
		}
	} যাবত্ (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
স্থির শূন্য
urldb_delete_cookie_hosts(ধ্রুব বর্ণ *domain,
			  ধ্রুব বর্ণ *path,
			  ধ্রুব বর্ণ *name,
			  সংরচনা host_part *parent)
{
	সংরচনা host_part *h;
	প্রতিজ্ঞা(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	প্রতি (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
স্থির শূন্য urldb_save_cookie_paths(FILE *fp, সংরচনা path_data *parent)
{
	সংরচনা path_data *p = parent;
	time_t now = time(রিক্ত);

	প্রতিজ্ঞা(fp && parent);

	কুরু {
		যদি (p->cookies != রিক্ত) {
			সংরচনা cookie_internal_data *c;

			প্রতি (c = p->cookies; c != রিক্ত; c = c->next) {
				যদি (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					অনুবর্ত;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(পূর্ণাঙ্ক)c->expires, (পূর্ণাঙ্ক)c->last_used,
					c->no_destroy, c->name, c->মান,
					c->মান_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		যদি (p->children != রিক্ত) {
			p = p->children;
		} অন্যথা {
			যাবত্ (p != parent) {
				যদি (p->next != রিক্ত) {
					p = p->next;
					ভঙ্গ;
				}

				p = p->parent;
			}
		}
	} যাবত্ (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
স্থির শূন্য urldb_save_cookie_hosts(FILE *fp, সংরচনা host_part *parent)
{
	সংরচনা host_part *h;
	প্রতিজ্ঞা(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	প্রতি (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
স্থির শূন্য urldb_destroy_cookie(সংরচনা cookie_internal_data *c)
{
	মুক্ত(c->name);
	মুক্ত(c->মান);
	মুক্ত(c->comment);
	মুক্ত(c->domain);
	মুক্ত(c->path);

	মুক্ত(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
স্থির শূন্য urldb_destroy_path_গ্রন্থি_content(সংরচনা path_data *গ্রন্থি)
{
	সংরচনা cookie_internal_data *a, *b;
	unsigned পূর্ণাঙ্ক i;

	যদি (গ্রন্থি->url != রিক্ত) {
		nsurl_unref(গ্রন্থি->url);
	}

	lwc_string_unref(গ্রন্থি->scheme);

	মুক্ত(গ্রন্থি->segment);
	প্রতি (i = 0; i < গ্রন্থি->frag_cnt; i++)
		মুক্ত(গ্রন্থি->fragment[i]);
	মুক্ত(গ্রন্থি->fragment);

	মুক্ত(গ্রন্থি->urld.title);

	প্রতি (a = গ্রন্থি->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
স্থির শূন্য urldb_destroy_prot_space(সংরচনা prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	মুক্ত(space->realm);
	মুক্ত(space->auth);

	মুক্ত(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
স্থির শূন্য urldb_destroy_path_বৃক্ষ(সংরচনা path_data *মূল)
{
	সংরচনা path_data *p = মূল;

	কুরু {
		যদি (p->children != রিক্ত) {
			p = p->children;
		} অন্যথা {
			সংরচনা path_data *q = p;

			যাবত্ (p != মূল) {
				যদি (p->next != রিক্ত) {
					p = p->next;
					ভঙ্গ;
				}

				p = p->parent;

				urldb_destroy_path_গ্রন্থি_content(q);
				মুক্ত(q);

				q = p;
			}

			urldb_destroy_path_গ্রন্থি_content(q);
			মুক্ত(q);
		}
	} যাবত্ (p != মূল);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
স্থির শূন্য urldb_destroy_host_বৃক্ষ(সংরচনা host_part *মূল)
{
	সংরচনা host_part *a, *b;
	সংরচনা path_data *p, *q;
	সংরচনা prot_space_data *s, *t;

	/* Destroy children */
	প্রতি (a = মূল->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_বৃক্ষ(a);
	}

	/* Now clean up paths */
	প্রতি (p = মূল->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_বৃক্ষ(p);
	}

	/* Root path */
	urldb_destroy_path_গ্রন্থি_content(&মূল->paths);

	/* Proctection space data */
	প্রতি (s = মূল->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	মুক্ত(মূল->part);
	মুক্ত(মূল);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
স্থির শূন্য urldb_destroy_অনুসন্ধান_বৃক্ষ(সংরচনা অনুসন্ধান_গ্রন্থি *মূল)
{
	/* Destroy children */
	যদি (মূল->বাম != &empty)
		urldb_destroy_অনুসন্ধান_বৃক্ষ(মূল->বাম);
	যদি (মূল->ডান != &empty)
		urldb_destroy_অনুসন্ধান_বৃক্ষ(মূল->ডান);

	/* And destroy ourselves */
	মুক্ত(মূল);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
শূন্য urldb_destroy(শূন্য)
{
	সংরচনা host_part *a, *b;
	পূর্ণাঙ্ক i;

	/* Clean up search trees */
	প্রতি (i = 0; i < অঙ্ক_অনুসন্ধান_TREES; i++) {
		যদি (অনুসন্ধান_trees[i] != &empty) {
			urldb_destroy_অনুসন্ধান_বৃক্ষ(অনুসন্ধান_trees[i]);
			অনুসন্ধান_trees[i] = &empty;
		}
	}

	/* And database */
	প্রতি (a = db_মূল.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_বৃক্ষ(a);
	}
	memset(&db_মূল, 0, পরিমাণ(db_মূল));

	/* And the bloom filter */
	যদি (url_bloom != রিক্ত) {
		bloom_destroy(url_bloom);
		url_bloom = রিক্ত;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(ধ্রুব বর্ণ *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	বর্ণ s[MAXIMUM_URL_LENGTH];
	বর্ণ host[256];
	সংরচনা host_part *h;
	পূর্ণাঙ্ক urls;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক version;
	পূর্ণাঙ্ক length;
	FILE *fp;

	প্রতিজ্ঞা(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	যদি (url_bloom == রিক্ত)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	যদি (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		প্রত্যাগম NSERROR_NOT_FOUND;
	}

	যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		প্রত্যাগম NSERROR_NEED_DATA;
	}

	version = atoi(s);
	যদি (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		প্রত্যাগম NSERROR_INVALID;
	}
	যদি (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		প্রত্যাগম NSERROR_INVALID;
	}

	যাবত্ (fgets(host, পরিমাণ host, fp)) {
		time_t hsts_expiry = 0;
		পূর্ণাঙ্ক hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		যদি (length == 0) {
			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;
			urls = atoi(s);
			/* Eight fields/url */
			প্রতি (i = 0; i < (8 * urls); i++) {
				যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					ভঙ্গ;
			}
			অনুবর্ত;
		}

		যদি (version >= 107) {
			বর্ণ *p = host;
			যাবত্ (*p && *p != ' ') p++;
			যাবত্ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			যাবত্ (*p && *p != ' ') p++;
			যাবত্ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		যদি (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			প্রত্যাগম NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			ভঙ্গ;
		urls = atoi(s);

		/* no URLs => try next host */
		যদি (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			অনুবর্ত;
		}

		/* load the non-corrupt data */
		প্রতি (i = 0; i < urls; i++) {
			সংরচনা path_data *p = রিক্ত;
			বর্ণ scheme[64], ports[10];
			বর্ণ url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned পূর্ণাঙ্ক port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			বর্ণ *path_query;
			size_t len;

			যদি (!fgets(scheme, পরিমাণ scheme, fp))
				ভঙ্গ;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			যদি (!fgets(ports, পরিমাণ ports, fp))
				ভঙ্গ;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;
			length = strlen(s) - 1;
			s[length] = '\0';

			যদি (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, পরিমাণ url, "%s://%s%s%s%s",
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
			যদি (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				প্রত্যাগম NSERROR_NOMEM;
			}

			যদি (url_bloom != রিক্ত) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ঢোকাও_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			যদি (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				প্রত্যাগম NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			যদি (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				প্রত্যাগম NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;
			যদি (p)
				p->urld.visits = (unsigned পূর্ণাঙ্ক)atoi(s);

			/* entry last use time */
			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				ভঙ্গ;
			}
			যদি (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;
			যদি (p)
				p->urld.type = (content_type)atoi(s);

			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;


			যদি (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ভঙ্গ;
			length = strlen(s) - 1;
			যদি (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				যদি (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	প্রত্যাগম NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ধ্রুব বর্ণ *filename)
{
	FILE *fp;
	পূর্ণাঙ্ক i;

	প্রতিজ্ঞা(filename);

	fp = fopen(filename, "w");
	যদি (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		প্রত্যাগম NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	প্রতি (i = 0; i != অঙ্ক_অনুসন্ধান_TREES; i++) {
		urldb_save_অনুসন্ধান_বৃক্ষ(অনুসন্ধান_trees[i], fp);
	}

	fclose(fp);

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	সংরচনা path_data *p;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p) {
		প্রত্যাগম NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	সংরচনা host_part *h;
	সংরচনা path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	ধ্রুব বর্ণ *host_str;
	বর্ণ *path_query = রিক্ত;
	size_t len;
	bool match;
	unsigned পূর্ণাঙ্ক port_int;

	প্রতিজ্ঞা(url);

	যদি (url_bloom == রিক্ত)
		url_bloom = bloom_create(BLOOM_SIZE);

	যদি (url_bloom != রিক্ত) {
		uint32_t hash = nsurl_hash(url);
		bloom_ঢোকাও_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	যদি (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		প্রত্যাগম false;
	}
	প্রতিজ্ঞা(path_query != রিক্ত);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	যদি (scheme == রিক্ত) {
		মুক্ত(path_query);
		প্রত্যাগম false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	যদি (host != রিক্ত) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} অন্যথা যদি (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} অন্যথা {
		lwc_string_unref(scheme);
		মুক্ত(path_query);
		প্রত্যাগম false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	যদি (port != রিক্ত) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} অন্যথা {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	যদি (h != রিক্ত) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} অন্যথা {
		p = রিক্ত;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	প্রত্যাগম (p != রিক্ত);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, ধ্রুব বর্ণ *title)
{
	সংরচনা path_data *p;
	বর্ণ *temp;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (p == রিক্ত) {
		প্রত্যাগম NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	যদি (title != রিক্ত) {
		temp = strdup(title);
		যদি (temp == রিক্ত) {
			প্রত্যাগম NSERROR_NOMEM;
		}
	} অন্যথা {
		temp = রিক্ত;
	}

	মুক্ত(p->urld.title);
	p->urld.title = temp;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	সংরচনা path_data *p;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p) {
		প্রত্যাগম NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	সংরচনা path_data *p;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p) {
		প্রত্যাগম NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(রিক্ত);
	p->urld.visits++;

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
শূন্য urldb_reset_url_visit_data(nsurl *url)
{
	সংরচনা path_data *p;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ধ্রুব সংরচনা url_data *urldb_get_url_data(nsurl *url)
{
	সংরচনা path_data *p;
	সংরচনা url_internal_data *u;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম রিক্ত;

	u = &p->urld;

	প্রত্যাগম (ধ্রুব সংরচনা url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	সংরচনা path_data *p;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম রিক্ত;

	প্রত্যাগম p->url;
}


/* exported interface documented in netsurf/url_db.h */
শূন্য urldb_set_auth_details(nsurl *url, ধ্রুব বর্ণ *realm, ধ্রুব বর্ণ *auth)
{
	সংরচনা path_data *p, *pi;
	সংরচনা host_part *h;
	সংরচনা prot_space_data *space, *space_alloc;
	বর্ণ *realm_alloc, *auth_alloc;
	bool match;

	প্রতিজ্ঞা(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	যদি (!p)
		প্রত্যাগম;

	/* Search for host_part */
	প্রতি (pi = p; pi->parent != রিক্ত; pi = pi->parent)
		;
	h = (সংরচনা host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	প্রতি (space = h->prot_space; space; space = space->next) {
		যদি (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			ভঙ্গ;
	}

	যদি (space != রিক্ত) {
		/* Overrule existing auth. */
		মুক্ত(space->auth);
		space->auth = strdup(auth);
	} অন্যথা {
		/* Create a new protection space. */
		space = space_alloc = malloc(পরিমাণ(সংরচনা prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		যদি (!space_alloc || !realm_alloc || !auth_alloc) {
			মুক্ত(space_alloc);
			মুক্ত(realm_alloc);
			মুক্ত(auth_alloc);
			প্রত্যাগম;
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
ধ্রুব বর্ণ *urldb_get_auth_details(nsurl *url, ধ্রুব বর্ণ *realm)
{
	সংরচনা path_data *p, *p_cur, *p_শীর্ষ;

	প্রতিজ্ঞা(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম রিক্ত;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	প্রতি (p_cur = p; p_cur != রিক্ত; p_শীর্ষ = p_cur, p_cur = p_cur->parent) {
		যদি (p_cur->prot_space) {
			প্রত্যাগম p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	যদি (realm != রিক্ত) {
		ধ্রুব সংরচনা host_part *h = (ধ্রুব সংরচনা host_part *)p_শীর্ষ;
		ধ্রুব সংরচনা prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		প্রতি (space = h->prot_space; space != রিক্ত;
		     space = space->next) {
			যদি (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				প্রত্যাগম p->prot_space->auth;
			}
		}
	}

	প্রত্যাগম রিক্ত;
}


/* exported interface documented in netsurf/url_db.h */
শূন্য urldb_set_cert_permissions(nsurl *url, bool permit)
{
	সংরচনা path_data *p;
	সংরচনা host_part *h;

	প্রতিজ্ঞা(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম;

	প্রতি (; p && p->parent; p = p->parent)
		/* do nothing */;
	প্রতিজ্ঞা(p);

	h = (সংরচনা host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	সংরচনা path_data *p;
	ধ্রুব সংরচনা host_part *h;

	প্রতিজ্ঞা(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম false;

	প্রতি (; p && p->parent; p = p->parent)
		/* do nothing */;
	প্রতিজ্ঞা(p);

	h = (ধ্রুব সংরচনা host_part *)p;

	প্রত্যাগম h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(সংরচনা nsurl *url, ধ্রুব বর্ণ *header)
{
	সংরচনা path_data *p;
	সংরচনা host_part *h;
	lwc_string *host;
	time_t now = time(রিক্ত);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	প্রতিজ্ঞা(url);

	host = nsurl_get_component(url, NSURL_HOST);
	যদি (host != রিক্ত) {
		যদি (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			প্রত্যাগম true;
		} অন্যথা যদি (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			প্রত্যাগম true;
		}

		lwc_string_unref(host);
	} অন্যথা {
		/* No host part: ignore */
		প্রত্যাগম true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম false;

	প্রতি (; p && p->parent; p = p->parent)
		/* do nothing */;
	প্রতিজ্ঞা(p);

	h = (সংরচনা host_part *)p;
	যদি (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		প্রত্যাগম true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	যদি (error != NSERROR_OK) {
		/* Parse failed: ignore */
		প্রত্যাগম true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	যদি (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} অন্যথা যদি ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	প্রত্যাগম true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(সংরচনা nsurl *url)
{
	সংরচনা path_data *p;
	ধ্রুব সংরচনা host_part *h;
	lwc_string *host;
	time_t now = time(রিক্ত);

	প্রতিজ্ঞা(url);

	host = nsurl_get_component(url, NSURL_HOST);
	যদি (host != রিক্ত) {
		যদি (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			প্রত্যাগম false;
		} অন্যথা যদি (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			প্রত্যাগম false;
		}

		lwc_string_unref(host);
	} অন্যথা {
		/* No host part: not enabled */
		প্রত্যাগম false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম false;

	প্রতি (; p && p->parent; p = p->parent)
		/* do nothing */;
	প্রতিজ্ঞা(p);

	h = (ধ্রুব সংরচনা host_part *)p;

	/* Consult record for this host */
	যদি (h->hsts.expires > now) {
		/* Not expired */
		প্রত্যাগম true;
	}

	/* Consult parent domains */
	প্রতি (h = h->parent; h && h != &db_মূল; h = h->parent) {
		যদি (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			প্রত্যাগম true;
		}
	}

	প্রত্যাগম false;
}


/* exported interface documented in netsurf/url_db.h */
শূন্য
urldb_iterate_partial(ধ্রুব বর্ণ *prefix,
		      bool (*callback)(nsurl *url, ধ্রুব সংরচনা url_data *data))
{
	বর্ণ host[256];
	বর্ণ buf[260]; /* max domain + "www." */
	ধ্রুব বর্ণ *slash, *scheme_sep;
	সংরচনা অনুসন্ধান_গ্রন্থি *বৃক্ষ;
	ধ্রুব সংরচনা host_part *h;

	প্রতিজ্ঞা(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	যদি (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	বৃক্ষ = urldb_get_অনুসন্ধান_বৃক্ষ(prefix);

	যদি (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, পরিমাণ host, "%.*s",
			 (পূর্ণাঙ্ক) (slash - prefix), prefix);

		h = urldb_অনুসন্ধান_find(বৃক্ষ, host);
		যদি (!h) {
			পূর্ণাঙ্ক len = slash - prefix;

			যদি (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, পরিমাণ buf, "www.%s", host);
				h = urldb_অনুসন্ধান_find(
					অনুসন্ধান_trees[ST_DN + 'w' - 'a'],
					buf);
				যদি (!h)
					প্রত্যাগম;
			} অন্যথা
				প্রত্যাগম;
		}

		যদি (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} অন্যথা {
		পূর্ণাঙ্ক len = strlen(prefix);

		/* looking for hosts */
		যদি (!urldb_iterate_partial_host(বৃক্ষ, prefix, callback))
			প্রত্যাগম;

		যদি (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, পরিমাণ buf, "www.%s", prefix);
			যদি(!urldb_iterate_partial_host(
				   অনুসন্ধান_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				প্রত্যাগম;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
শূন্য
urldb_iterate_entries(bool (*callback)(nsurl *url, ধ্রুব সংরচনা url_data *data))
{
	পূর্ণাঙ্ক i;

	প্রতিজ্ঞা(callback);

	প্রতি (i = 0; i < অঙ্ক_অনুসন্ধান_TREES; i++) {
		যদি (!urldb_iterate_entries_host(অনুসন্ধান_trees[i],
						callback,
						রিক্ত)) {
			ভঙ্গ;
		}
	}
}


/* exported interface documented in content/urldb.h */
শূন্য urldb_iterate_cookies(bool (*callback)(ধ্রুব সংরচনা cookie_data *data))
{
	পূর্ণাঙ্ক i;

	প্রতিজ্ঞা(callback);

	প্রতি (i = 0; i < অঙ্ক_অনুসন্ধান_TREES; i++) {
		যদি (!urldb_iterate_entries_host(অনুসন্ধান_trees[i],
						রিক্ত, callback))
			ভঙ্গ;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(ধ্রুব বর্ণ *header, nsurl *url, nsurl *referer)
{
	ধ্রুব বর্ণ *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	প্রতিজ্ঞা(url && header);

	/* Get defragmented URL, as 'urlt' */
	যদি (nsurl_defragment(url, &urlt) != NSERROR_OK)
		প্রত্যাগম রিক্ত;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	যদি (scheme == রিক্ত) {
		nsurl_unref(urlt);
		প্রত্যাগম false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	যদি (path == রিক্ত) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		প্রত্যাগম false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	যদি (host == রিক্ত) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		প্রত্যাগম false;
	}

	যদি (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		যদি (rhost == রিক্ত) {
			গচ্ছ error;
		}

		/* Domain match host names */
		যদি (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			ধ্রুব বর্ণ *hptr;
			ধ্রুব বর্ণ *rptr;
			ধ্রুব বর্ণ *dot;
			ধ্রুব বর্ণ *host_data = lwc_string_data(host);
			ধ্রুব বর্ণ *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			যদি (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				গচ্ছ error;
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
			যাবত্ (hptr >= host_data && rptr >= rhost_data) {
				যদি (*hptr != *rptr)
					ভঙ্গ;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			যাবত্ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			যদি (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == রিক্ত ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				গচ্ছ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	কুরু {
		সংরচনা cookie_internal_data *c;
		বর্ণ *dot;
		size_t len;
#ifdef WITH_NSPSL
		ধ্রুব বর্ণ *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		যদি (!c) {
			/* failed => stop parsing */
			গচ্ছ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		যদি (!c->name || !c->মান) {
			urldb_free_cookie(c);
			গচ্ছ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		যদি (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			গচ্ছ error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		যদি (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		যদি (suffix == রিক্ত) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			গচ্ছ error;
		}
#অন্যথা
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		যদি (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			গচ্ছ error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		যদি (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			পূর্ণাঙ্ক hlen, dlen;
			বর্ণ *domain = c->domain;

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
			প্রতিজ্ঞা(c->domain[0] == '.');

			/* 4.3.2:iii */
			যদি (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				গচ্ছ error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			যদি (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				গচ্ছ error;
			}

			যদি (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			যদি (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				গচ্ছ error;
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
		যদি (!urldb_ঢোকাও_cookie(c, scheme, urlt))
			গচ্ছ error;
	} যাবত্ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	প্রত্যাগম true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	প্রত্যাগম false;
}


/* exported interface documented in content/urldb.h */
বর্ণ *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ধ্রুব সংরচনা path_data *p, *q;
	ধ্রুব সংরচনা host_part *h;
	lwc_string *path_lwc;
	সংরচনা cookie_internal_data *c;
	পূর্ণাঙ্ক গুনতি = 0, version = COOKIE_RFC2965;
	সংরচনা cookie_internal_data **matched_cookies;
	পূর্ণাঙ্ক matched_cookies_size = 20;
	ধ্রুব বর্ণ *path;
	বর্ণ *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	প্রতিজ্ঞা(url != রিক্ত);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	যদি (!p)
		প্রত্যাগম রিক্ত;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 পরিমাণ(সংরচনা cookie_internal_data *));
	যদি (!matched_cookies)
		প্রত্যাগম রিক্ত;

#define GROW_MATCHED_COOKIES						\
	কুরু {								\
		যদি (গুনতি == matched_cookies_size) {			\
			সংরচনা cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       পরিমাণ(সংরচনা cookie_internal_data *)); \
									\
			যদি (temp == রিক্ত) {				\
				মুক্ত(matched_cookies);			\
				প্রত্যাগম রিক্ত;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} যাবত্(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	যদি (path_lwc == রিক্ত) {
		মুক্ত(matched_cookies);
		প্রত্যাগম রিক্ত;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(রিক্ত);

	যদি (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		প্রতি (q = p->parent->children; q; q = q->next) {
			যদি (strcmp(q->segment, p->segment))
				অনুবর্ত;

			/* Consider all cookies associated with
			 * this exact path */
			প্রতি (c = q->cookies; c; c = c->next) {
				যদি (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					অনুবর্ত;

				যদি (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					অনুবর্ত;

				যদি (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					অনুবর্ত;

				matched_cookies[গুনতি++] = c;

				GROW_MATCHED_COOKIES;

				যদি (c->version < (unsigned পূর্ণাঙ্ক)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((সংরচনা cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	প্রতি (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		প্রতি (q = p->children; q; q = q->next) {
			যদি (*(q->segment) != '\0')
				অনুবর্ত;

			প্রতি (c = q->cookies; c; c = c->next) {
				যদি (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					অনুবর্ত;

				যদি (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					অনুবর্ত;

				matched_cookies[গুনতি++] = c;

				GROW_MATCHED_COOKIES;

				যদি (c->version < (unsigned পূর্ণাঙ্ক) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((সংরচনা cookie_data *)c);
			}
		}

		যদি (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ভঙ্গ;
		}

		/* Consider p itself - may be the result of Path=/foo */
		প্রতি (c = p->cookies; c; c = c->next) {
			যদি (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				অনুবর্ত;

			/* Ensure cookie path is a prefix of the resource */
			যদি (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				অনুবর্ত;

			যদি (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				অনুবর্ত;

			matched_cookies[গুনতি++] = c;

			GROW_MATCHED_COOKIES;

			যদি (c->version < (unsigned পূর্ণাঙ্ক) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((সংরচনা cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	প্রতি (h = (ধ্রুব সংরচনা host_part *)p; h && h != &db_মূল;
	     h = h->parent) {
		প্রতি (c = h->paths.cookies; c; c = c->next) {
			যদি (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				অনুবর্ত;

			/* Ensure cookie path is a prefix of the resource */
			যদি (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				অনুবর্ত;

			যদি (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				অনুবর্ত;

			matched_cookies[গুনতি++] = c;

			GROW_MATCHED_COOKIES;

			যদি (c->version < (unsigned পূর্ণাঙ্ক)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((সংরচনা cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	যদি (গুনতি == 0) {
		/* No cookies found */
		মুক্ত(matched_cookies);
		প্রত্যাগম রিক্ত;
	}

	যদি (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, গুনতি);
	} অন্যথা {
		ret = get_rfc2965_cookie(matched_cookies, গুনতি, version);
	}
	মুক্ত(matched_cookies);

	প্রত্যাগম ret;
}


/* exported interface documented in content/urldb.h */
শূন্য urldb_delete_cookie(ধ্রুব বর্ণ *domain, ধ্রুব বর্ণ *path,
			 ধ্রুব বর্ণ *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_মূল);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(ধ্রুব বর্ণ *filename)
{
	FILE *fp;
	বর্ণ s[16*1024];

	প্রতিজ্ঞা(filename);

	fp = fopen(filename, "r");
	যদি (!fp)
		প্রত্যাগম NSERROR_NOT_FOUND;

#define FIND_T {				\
		প্রতি (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		যদি (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			অনুবর্ত;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		প্রতি (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		যদি (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			অনুবর্ত;		\
		}				\
	}

	যাবত্ (fgets(s, পরিমাণ s, fp)) {
		বর্ণ *p = s, *end = 0,
			*domain, *path, *name, *মান, *scheme, *url,
			*comment;
		পূর্ণাঙ্ক version, domain_specified, path_specified,
			secure, http_only, no_destroy, মান_quoted;
		time_t expires, last_used;
		সংরচনা cookie_internal_data *c;

		যদি(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			অনুবর্ত;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		যদি (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			যদি (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ভঙ্গ;
			}

			অনুবর্ত;
		} অন্যথা যদি (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			অনুবর্ত;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		যদি (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} অন্যথা {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; মান = p; FIND_T;
		যদি (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	মান_quoted = atoi(p); FIND_T;
		} অন্যথা {
			মান_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		প্রতি (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		প্রতিজ্ঞা(p <= end);

		/* Now create cookie */
		c = malloc(পরিমাণ(সংরচনা cookie_internal_data));
		যদি (!c)
			ভঙ্গ;

		c->name = strdup(name);
		c->মান = strdup(মান);
		c->মান_was_quoted = মান_quoted;
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

		যদি (!(c->name && c->মান && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			ভঙ্গ;
		}

		যদি (c->domain[0] != '.') {
			lwc_string *scheme_lwc = রিক্ত;
			nsurl *url_nsurl = রিক্ত;

			প্রতিজ্ঞা(scheme[0] != 'u');

			যদি (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ভঙ্গ;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			যদি (!urldb_ঢোকাও_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ভঙ্গ;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} অন্যথা {
			যদি (!urldb_ঢোকাও_cookie(c, রিক্ত, রিক্ত)) {
				/* Cookie freed for us */
				ভঙ্গ;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ধ্রুব বর্ণ *filename)
{
	FILE *fp;
	পূর্ণাঙ্ক cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	প্রতিজ্ঞা(filename);

	fp = fopen(filename, "w");
	যদি (!fp)
		প্রত্যাগম NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_মূল);

	fclose(fp);

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
শূন্য urldb_dump(শূন্য)
{
	পূর্ণাঙ্ক i;

	urldb_dump_hosts(&db_মূল);

	প্রতি (i = 0; i != অঙ্ক_অনুসন্ধান_TREES; i++) {
		urldb_dump_অনুসন্ধান(অনুসন্ধান_trees[i], 0);
	}
}
