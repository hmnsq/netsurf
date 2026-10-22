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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <time.h>
#ifdef WITH_NSPSL
#அந்தர்பாவ <nspsl.h>
#endif

#அந்தர்பாவ "utils/inet.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/url.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/bloom.h"
#அந்தர்பாவ "utils/time.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "desktop/cookie_manager.h"

#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
கட்டமைப்பு cookie_internal_data {
	கட்டமைப்பு cookie_internal_data *prev;	/**< Previous in list */
	கட்டமைப்பு cookie_internal_data *next;	/**< Next in list */

	எழுத்து *name;		/**< Cookie name */
	எழுத்து *மதிப்பு;		/**< Cookie value */
	bool மதிப்பு_was_quoted;	/**< Value was quoted in Set-Cookie: */
	எழுத்து *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	எழுத்து *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	எழுத்து *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	எண்ணிக்கை cookie_version version;	/**< Specification compliance */
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
கட்டமைப்பு prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned முழு port;
	/** Protection realm */
	எழுத்து *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	எழுத்து *auth;
	/** Next sibling */
	கட்டமைப்பு prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
கட்டமைப்பு url_internal_data {
	எழுத்து *title;		/**< Resource title */
	unsigned முழு visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
கட்டமைப்பு path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned முழு port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	எழுத்து *segment;		/**< Path segment for this node */
	unsigned முழு frag_cnt;	/**< Number of entries in path_data::fragment */
	எழுத்து **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	கட்டமைப்பு url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	மாறிலி கட்டமைப்பு prot_space_data *prot_space;
	/** Cookies associated with resource */
	கட்டமைப்பு cookie_internal_data *cookies;
	/** Last cookie in list */
	கட்டமைப்பு cookie_internal_data *cookies_end;

	கட்டமைப்பு path_data *next;	/**< Next sibling */
	கட்டமைப்பு path_data *prev;	/**< Previous sibling */
	கட்டமைப்பு path_data *parent; /**< Parent path segment */
	கட்டமைப்பு path_data *children; /**< Child path segments */
	கட்டமைப்பு path_data *last; /**< Last child */
};

கட்டமைப்பு hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

கட்டமைப்பு host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	கட்டமைப்பு path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	கட்டமைப்பு hsts_data hsts;

	/**
	 * Part of host string
	 */
	எழுத்து *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	கட்டமைப்பு prot_space_data *prot_space;

	கட்டமைப்பு host_part *next;	/**< Next sibling */
	கட்டமைப்பு host_part *prev;	/**< Previous sibling */
	கட்டமைப்பு host_part *parent; /**< Parent host part */
	கட்டமைப்பு host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
கட்டமைப்பு தேடல்_முனை {
	மாறிலி கட்டமைப்பு host_part *data;	/**< Host tree entry */

	unsigned முழு level;		/**< Node level */

	கட்டமைப்பு தேடல்_முனை *இடது;	/**< Left subtree */
	கட்டமைப்பு தேடல்_முனை *வலது;	/**< Right subtree */
};

/** Root database handle */
நிலையான கட்டமைப்பு host_part db_வேர்;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define இலக்கம்_தேடல்_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
நிலையான கட்டமைப்பு தேடல்_முனை empty = { 0, 0, &empty, &empty };
நிலையான கட்டமைப்பு தேடல்_முனை *தேடல்_trees[இலக்கம்_தேடல்_TREES] = {
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
நிலையான முழு loaded_cookie_file_version;

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
நிலையான கட்டமைப்பு bloom_filter *url_bloom;
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
நிலையான nserror urldb_write_timet(FILE *fp, time_t val)
{
	முழு use;
	எழுத்து op[32];

	use = nsc_sntimet(op, 32, &val);
	எனில் (use == 0) {
		fprintf(fp, "%i\n", (முழு)val);
	} இல்லையெனில் {
		fprintf(fp, "%.*s\n", use, op);
	}
	திரும்பு NSERROR_OK;
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
நிலையான வெறுமை
urldb_write_paths(மாறிலி கட்டமைப்பு path_data *parent,
		  மாறிலி எழுத்து *host,
		  FILE *fp,
		  எழுத்து **path,
		  முழு *path_alloc,
		  முழு *path_used,
		  time_t expiry)
{
	மாறிலி கட்டமைப்பு path_data *p = parent;
	முழு i;

	செய் {
		முழு seglen = p->segment != வெற்று ? strlen(p->segment) : 0;
		முழு len = *path_used + seglen + 1;

		எனில் (*path_alloc < len) {
			எழுத்து *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			எனில் (!temp) {
				திரும்பு;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		எனில் (p->segment != வெற்று) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		எனில் (p->children != வெற்று) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} இல்லையெனில் {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		எனில் (p->children != வெற்று) {
			/* Drill down into children */
			p = p->children;
		} இல்லையெனில் {
			/* leaf node */
			எனில் (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				எனில் (p->port) {
					fprintf(fp,"%d\n", p->port);
				} இல்லையெனில் {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (முழு)p->urld.type);

				fprintf(fp, "\n");

				எனில் (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					க்கு (i = 0; s[i] != '\0'; i++)
						எனில் (s[i] < 32)
							s[i] = ' ';
					க்கு (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} இல்லையெனில் {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			போது (p != parent) {
				முழு seglen = p->segment != வெற்று
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				எனில் (p->next != வெற்று) {
					/* Have a sibling, process that */
					p = p->next;
					நிறுத்து;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} போது (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
நிலையான வெறுமை
urldb_மொத்தம்_urls(மாறிலி கட்டமைப்பு path_data *வேர்,
		 time_t expiry,
		 unsigned முழு *மொத்தம்)
{
	மாறிலி கட்டமைப்பு path_data *p = வேர்;

	செய் {
		எனில் (p->children != வெற்று) {
			/* Drill down into children */
			p = p->children;
		} இல்லையெனில் {
			/* No more children, increment count if required */
			எனில் (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*மொத்தம்)++;
			}

			/* Now, find next node to process. */
			போது (p != வேர்) {
				எனில் (p->next != வெற்று) {
					/* Have a sibling, process that */
					p = p->next;
					நிறுத்து;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} போது (p != வேர்);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
நிலையான வெறுமை urldb_save_தேடல்_மரம்(கட்டமைப்பு தேடல்_முனை *parent, FILE *fp)
{
	எழுத்து host[256];
	மாறிலி கட்டமைப்பு host_part *h;
	unsigned முழு path_மொத்தம் = 0;
	எழுத்து *path, *p, *end;
	முழு path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	முழு hsts_include_subdomains = 0;

	expiry = time(வெற்று) - ((60 * 60 * 24) * nsoption_int(expire_url));

	எனில் (parent == &empty)
		திரும்பு;

	urldb_save_தேடல்_மரம்(parent->இடது, fp);

	path = malloc(path_alloc);
	எனில் (!path)
		திரும்பு;

	path[0] = '\0';

	க்கு (h = parent->data, p = host, end = host + அளவு host;
	     h && h != &db_வேர் && p < end; h = h->parent) {
		முழு written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		எனில் (written < 0) {
			விடுவி(path);
			திரும்பு;
		}
		p += written;
	}

	h = parent->data;
	எனில் (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_மொத்தம்_urls(&parent->data->paths, expiry, &path_மொத்தம்);

	எனில் (path_மொத்தம் > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_மொத்தம்);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} இல்லையெனில் எனில் (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	விடுவி(path);

	urldb_save_தேடல்_மரம்(parent->வலது, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
நிலையான bool
urldb_iterate_entries_path(மாறிலி கட்டமைப்பு path_data *parent,
		bool (*url_callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data),
		bool (*cookie_callback)(மாறிலி கட்டமைப்பு cookie_data *data))
{
	மாறிலி கட்டமைப்பு path_data *p = parent;
	மாறிலி கட்டமைப்பு cookie_data *c;

	செய் {
		எனில் (p->children != வெற்று) {
			/* Drill down into children */
			p = p->children;
		} இல்லையெனில் {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ப்ரதிஜ்ஞா(url_callback || cookie_callback);

			/** \todo handle fragments? */
			எனில் (url_callback) {
				மாறிலி கட்டமைப்பு url_internal_data *u = &p->urld;

				ப்ரதிஜ்ஞா(p->url);

				எனில் (!url_callback(p->url,
						  (மாறிலி கட்டமைப்பு url_data *) u))
					திரும்பு false;
			} இல்லையெனில் {
				c = (மாறிலி கட்டமைப்பு cookie_data *)p->cookies;
				க்கு (; c != வெற்று; c = c->next) {
					எனில் (!cookie_callback(c))
						திரும்பு false;
				}
			}

			/* Now, find next node to process. */
			போது (p != parent) {
				எனில் (p->next != வெற்று) {
					/* Have a sibling, process that */
					p = p->next;
					நிறுத்து;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} போது (p != parent);

	திரும்பு true;
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
நிலையான bool urldb__host_is_ip_address(மாறிலி எழுத்து *host)
{
	கட்டமைப்பு in_addr ipv4;
	size_t host_len = strlen(host);
	மாறிலி எழுத்து *sane_host;
	மாறிலி எழுத்து *slash;
#ifndef NO_IPV6
	கட்டமைப்பு in6_addr ipv6;
	எழுத்து ipv6_addr[64];
	unsigned முழு ipv6_addr_len;
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
	எனில் (slash == வெற்று) {
		sane_host = host;
	} இல்லையெனில் {
		எழுத்து *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	எனில் (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		செல் out_false;

	எனில் (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		முழு இலக்கம்_dots = 0;
		size_t index;

		க்கு (index = 0; index < host_len; index++) {
			எனில் (sane_host[index] == '.')
				இலக்கம்_dots++;
		}

		எனில் (இலக்கம்_dots == 3)
			செல் out_true;
		இல்லையெனில்
			செல் out_false;
	}

#ifndef NO_IPV6
	எனில் ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		செல் out_false;
	}

	ipv6_addr_len = host_len - 2;
	எனில் (ipv6_addr_len >= அளவு(ipv6_addr)) {
		ipv6_addr_len = அளவு(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	எனில் (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		செல் out_true;
#endif

out_false:
	எனில் (slash != வெற்று) விடுவி((வெறுமை *)sane_host);
	திரும்பு false;

out_true:
	எனில் (slash != வெற்று) விடுவி((வெறுமை *)sane_host);
	திரும்பு true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
நிலையான முழு urldb_தேடல்_match_prefix(மாறிலி கட்டமைப்பு host_part *a, மாறிலி எழுத்து *b)
{
	மாறிலி எழுத்து *end, *dot;
	முழு plen, ret;

	ப்ரதிஜ்ஞா(a && a != &db_வேர் && b);

	எனில் (urldb__host_is_ip_address(b)) {
		/* IP address */
		திரும்பு strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	போது (b < end && a && a != &db_வேர்) {
		dot = strchr(b, '.');
		எனில் (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		எனில் ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			திரும்பு ret;

		/* The strings matched */
		எனில் (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			எனில் (plen > dot - b) {
				/* len(a) > len(b) */
				திரும்பு 1;
			} இல்லையெனில் எனில் (plen < dot - b) {
				/* len(a) < len(b) */
				திரும்பு -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	எனில் (a && a != &db_வேர் && b >= end) {
		/* len(a) > len(b) => prefix matches */
		திரும்பு 0;
	} இல்லையெனில் எனில் ((!a || a == &db_வேர்) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		திரும்பு -1;
	}

	/* Identical */
	திரும்பு 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
நிலையான bool
urldb_iterate_partial_host(கட்டமைப்பு தேடல்_முனை *வேர்,
		மாறிலி எழுத்து *prefix,
		bool (*callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data))
{
	முழு c;

	ப்ரதிஜ்ஞா(வேர் && prefix && callback);

	எனில் (வேர் == &empty)
		திரும்பு true;

	c = urldb_தேடல்_match_prefix(வேர்->data, prefix);

	எனில் (c > 0) {
		/* No match => look in left subtree */
		திரும்பு urldb_iterate_partial_host(வேர்->இடது,
						  prefix,
						  callback);
	} இல்லையெனில் எனில் (c < 0) {
		/* No match => look in right subtree */
		திரும்பு urldb_iterate_partial_host(வேர்->வலது,
						  prefix,
						  callback);
	} இல்லையெனில் {
		/* Match => iterate over l/r subtrees & process this node */
		எனில் (!urldb_iterate_partial_host(வேர்->இடது,
						prefix,
						callback)) {
			திரும்பு false;
		}

		எனில் (வேர்->data->paths.children) {
			/* and extract all paths attached to this host */
			எனில் (!urldb_iterate_entries_path(&வேர்->data->paths,
							callback,
							வெற்று)) {
				திரும்பு false;
			}
		}

		எனில் (!urldb_iterate_partial_host(வேர்->வலது,
						prefix,
						callback)) {
			திரும்பு false;
		}
	}

	திரும்பு true;
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
நிலையான bool
urldb_iterate_partial_path(மாறிலி கட்டமைப்பு path_data *parent,
		மாறிலி எழுத்து *prefix,
		bool (*callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data))
{
	மாறிலி கட்டமைப்பு path_data *p = parent->children;
	மாறிலி எழுத்து *slash, *end = prefix + strlen(prefix);

	செய் {
		slash = strchr(prefix, '/');
		எனில் (!slash) {
			slash = end;
		}

		எனில் (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			தொடர்;
		}

		எனில் (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			எனில் (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				எனில் (!urldb_iterate_entries_path(p,
								callback,
								வெற்று)) {
					திரும்பு false;
				}

				/* Progress to next sibling */
				p = p->next;
			} இல்லையெனில் {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} இல்லையெனில் {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} போது (p != வெற்று);

	திரும்பு true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
நிலையான bool
urldb_iterate_entries_host(கட்டமைப்பு தேடல்_முனை *parent,
		bool (*url_callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data),
		bool (*cookie_callback)(மாறிலி கட்டமைப்பு cookie_data *data))
{
	எனில் (parent == &empty) {
		திரும்பு true;
	}

	எனில் (!urldb_iterate_entries_host(parent->இடது,
					url_callback,
					cookie_callback)) {
		திரும்பு false;
	}

	எனில் ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		எனில் (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			திரும்பு false;
		}
	}

	எனில் (!urldb_iterate_entries_host(parent->வலது,
					url_callback,
					cookie_callback)) {
		திரும்பு false;
	}

	திரும்பு true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
நிலையான கட்டமைப்பு host_part *
urldb_add_host_முனை(மாறிலி எழுத்து *part, கட்டமைப்பு host_part *parent)
{
	கட்டமைப்பு host_part *d;

	ப்ரதிஜ்ஞா(part && parent);

	d = calloc(1, அளவு(கட்டமைப்பு host_part));
	எனில் (!d) {
		திரும்பு வெற்று;
	}

	d->part = strdup(part);
	எனில் (!d->part) {
		விடுவி(d);
		திரும்பு வெற்று;
	}

	d->next = parent->children;
	எனில் (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	திரும்பு d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
நிலையான முழு urldb_add_path_fragment_cmp(மாறிலி வெறுமை *a, மாறிலி வெறுமை *b)
{
	திரும்பு strcasecmp(*((மாறிலி எழுத்து **) a), *((மாறிலி எழுத்து **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
நிலையான கட்டமைப்பு path_data *
urldb_add_path_fragment(கட்டமைப்பு path_data *segment, lwc_string *fragment)
{
	எழுத்து **temp;

	ப்ரதிஜ்ஞா(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	எனில் (!fragment)
		திரும்பு segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * அளவு(எழுத்து *));
	எனில் (!temp)
		திரும்பு வெற்று;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	எனில் (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		திரும்பு வெற்று;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      அளவு (எழுத்து *),
	      urldb_add_path_fragment_cmp);

	திரும்பு segment;
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
நிலையான கட்டமைப்பு path_data *
urldb_add_path_முனை(lwc_string *scheme,
		    unsigned முழு port,
		    மாறிலி எழுத்து *segment,
		    lwc_string *fragment,
		    கட்டமைப்பு path_data *parent)
{
	கட்டமைப்பு path_data *d, *e;

	ப்ரதிஜ்ஞா(scheme && segment && parent);

	d = calloc(1, அளவு(கட்டமைப்பு path_data));
	எனில் (!d)
		திரும்பு வெற்று;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	எனில் (!d->segment) {
		lwc_string_unref(d->scheme);
		விடுவி(d);
		திரும்பு வெற்று;
	}

	எனில் (fragment) {
		எனில் (!urldb_add_path_fragment(d, fragment)) {
			விடுவி(d->segment);
			lwc_string_unref(d->scheme);
			விடுவி(d);
			திரும்பு வெற்று;
		}
	}

	க்கு (e = parent->children; e; e = e->next) {
		எனில் (strcmp(e->segment, d->segment) > 0)
			நிறுத்து;
	}

	எனில் (e) {
		d->prev = e->prev;
		d->next = e;
		எனில் (e->prev)
			e->prev->next = d;
		இல்லையெனில்
			parent->children = d;
		e->prev = d;
	} இல்லையெனில் எனில் (!parent->children) {
		d->prev = d->next = வெற்று;
		parent->children = parent->last = d;
	} இல்லையெனில் {
		d->next = வெற்று;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	திரும்பு d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
நிலையான கட்டமைப்பு தேடல்_முனை **urldb_get_தேடல்_மரம்_direct(மாறிலி எழுத்து *host)
{
	ப்ரதிஜ்ஞா(host);

	எனில் (urldb__host_is_ip_address(host)) {
		திரும்பு &தேடல்_trees[ST_IP];
	} இல்லையெனில் எனில் (ascii_is_alpha(*host)) {
		திரும்பு &தேடல்_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	திரும்பு &தேடல்_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
நிலையான கட்டமைப்பு தேடல்_முனை *urldb_get_தேடல்_மரம்(மாறிலி எழுத்து *host)
{
	திரும்பு *urldb_get_தேடல்_மரம்_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
நிலையான முழு urldb_தேடல்_match_string(மாறிலி கட்டமைப்பு host_part *a, மாறிலி எழுத்து *b)
{
	மாறிலி எழுத்து *end, *dot;
	முழு plen, ret;

	ப்ரதிஜ்ஞா(a && a != &db_வேர் && b);

	எனில் (urldb__host_is_ip_address(b)) {
		/* IP address */
		திரும்பு strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	போது (b < end && a && a != &db_வேர்) {
		dot = strchr(b, '.');
		எனில் (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		எனில் ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			திரும்பு ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		எனில் (plen > dot - b) {
			/* len(a) > len(b) */
			திரும்பு 1;
		} இல்லையெனில் எனில் (plen < dot - b) {
			/* len(a) < len(b) */
			திரும்பு -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	எனில் (a && a != &db_வேர் && b >= end) {
		/* len(a) > len(b) */
		திரும்பு 1;
	} இல்லையெனில் எனில் ((!a || a == &db_வேர்) && b < end) {
		/* len(a) < len(b) */
		திரும்பு -1;
	}

	/* Identical */
	திரும்பு 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
நிலையான மாறிலி கட்டமைப்பு host_part *
urldb_தேடல்_find(கட்டமைப்பு தேடல்_முனை *வேர், மாறிலி எழுத்து *host)
{
	முழு c;

	ப்ரதிஜ்ஞா(வேர் && host);

	எனில் (வேர் == &empty) {
		திரும்பு வெற்று;
	}

	c = urldb_தேடல்_match_string(வேர்->data, host);

	எனில் (c > 0) {
		திரும்பு urldb_தேடல்_find(வேர்->இடது, host);
	} இல்லையெனில் எனில் (c < 0) {
		திரும்பு urldb_தேடல்_find(வேர்->வலது, host);
	}

	திரும்பு வேர்->data;
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
நிலையான கட்டமைப்பு path_data *
urldb_match_path(மாறிலி கட்டமைப்பு path_data *parent,
		 மாறிலி எழுத்து *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	மாறிலி கட்டமைப்பு path_data *p;
	மாறிலி எழுத்து *slash;
	bool match;

	ப்ரதிஜ்ஞா(parent != வெற்று);
	ப்ரதிஜ்ஞா(parent->segment == வெற்று);

	எனில் (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	ப்ரதிஜ்ஞா(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	போது (p != வெற்று) {
		slash = strchr(path + 1, '/');
		எனில் (!slash) {
			slash = path + strlen(path);
		}

		எனில் (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			எனில் (*slash == '\0') {
				/* Complete match */
				திரும்பு (கட்டமைப்பு path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} இல்லையெனில் {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	திரும்பு வெற்று;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
நிலையான கட்டமைப்பு path_data *urldb_find_url(nsurl *url)
{
	மாறிலி கட்டமைப்பு host_part *h;
	கட்டமைப்பு path_data *p;
	கட்டமைப்பு தேடல்_முனை *மரம்;
	எழுத்து *plq;
	மாறிலி எழுத்து *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned முழு port_int;
	bool match;

	ப்ரதிஜ்ஞா(url);

	எனில் (url_bloom != வெற்று) {
		எனில் (bloom_தேடல்_hash(url_bloom, nsurl_hash(url)) == false) {
			திரும்பு வெற்று;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	எனில் (scheme == வெற்று)
		திரும்பு வெற்று;

	எனில் (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		திரும்பு வெற்று;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	எனில் (host != வெற்று) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} இல்லையெனில் எனில் (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} இல்லையெனில் {
		lwc_string_unref(scheme);
		திரும்பு வெற்று;
	}

	மரம் = urldb_get_தேடல்_மரம்(host_str);
	h = urldb_தேடல்_find(மரம், host_str);
	எனில் (!h) {
		lwc_string_unref(scheme);
		திரும்பு வெற்று;
	}

	/* generate plq (path, leaf, query) */
	எனில் (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		திரும்பு வெற்று;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	எனில் (port != வெற்று) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} இல்லையெனில் {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	விடுவி(plq);
	lwc_string_unref(scheme);

	திரும்பு p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
நிலையான வெறுமை urldb_dump_paths(கட்டமைப்பு path_data *parent)
{
	மாறிலி கட்டமைப்பு path_data *p = parent;
	unsigned முழு i;

	செய் {
		எனில் (p->segment != வெற்று) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			க்கு (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		எனில் (p->children != வெற்று) {
			p = p->children;
		} இல்லையெனில் {
			போது (p != parent) {
				எனில் (p->next != வெற்று) {
					p = p->next;
					நிறுத்து;
				}

				p = p->parent;
			}
		}
	} போது (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
நிலையான வெறுமை urldb_dump_hosts(கட்டமைப்பு host_part *parent)
{
	கட்டமைப்பு host_part *h;

	எனில் (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	க்கு (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
நிலையான வெறுமை urldb_dump_தேடல்(கட்டமைப்பு தேடல்_முனை *parent, முழு depth)
{
	மாறிலி கட்டமைப்பு host_part *h;
	முழு i; /* index into string */
	எழுத்து s[1024];
	முழு r;
	முழு sl = அளவு(s) - 2;

	எனில் (parent == &empty)
		திரும்பு;

	urldb_dump_தேடல்(parent->இடது, depth + 1);

	க்கு (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	க்கு (h = parent->data; h; h = h->parent) {
		எனில் (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			எனில் (r < 0) {
				நிறுத்து;
			}
			எனில் ((i + r) >= sl) {
				நிறுத்து;
			}
			i += r;
		}

		எனில் (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_தேடல்(parent->வலது, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
நிலையான முழு
urldb_தேடல்_match_host(மாறிலி கட்டமைப்பு host_part *a, மாறிலி கட்டமைப்பு host_part *b)
{
	முழு ret;

	ப்ரதிஜ்ஞா(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	க்கு (; a && a != &db_வேர் && b && b != &db_வேர்;
	     a = a->parent, b = b->parent) {
		எனில் ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			திரும்பு ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	எனில் (a && a != &db_வேர் && (!b || b == &db_வேர்)) {
		/* len(a) > len(b) */
		திரும்பு 1;
	} இல்லையெனில் எனில் ((!a || a == &db_வேர்) && b && b != &db_வேர்) {
		/* len(a) < len(b) */
		திரும்பு -1;
	}

	/* identical */
	திரும்பு 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
நிலையான கட்டமைப்பு தேடல்_முனை *urldb_தேடல்_skew(கட்டமைப்பு தேடல்_முனை *வேர்)
{
	ப்ரதிஜ்ஞா(வேர்);

	எனில் (வேர்->இடது->level == வேர்->level) {
		கட்டமைப்பு தேடல்_முனை *temp;

		temp = வேர்->இடது;
		வேர்->இடது = temp->வலது;
		temp->வலது = வேர்;
		வேர் = temp;
	}

	திரும்பு வேர்;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
நிலையான கட்டமைப்பு தேடல்_முனை *urldb_தேடல்_split(கட்டமைப்பு தேடல்_முனை *வேர்)
{
	ப்ரதிஜ்ஞா(வேர்);

	எனில் (வேர்->வலது->வலது->level == வேர்->level) {
		கட்டமைப்பு தேடல்_முனை *temp;

		temp = வேர்->வலது;
		வேர்->வலது = temp->இடது;
		temp->இடது = வேர்;
		வேர் = temp;

		வேர்->level++;
	}

	திரும்பு வேர்;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
நிலையான கட்டமைப்பு தேடல்_முனை *
urldb_தேடல்_செருகு_internal(கட்டமைப்பு தேடல்_முனை *வேர், கட்டமைப்பு தேடல்_முனை *n)
{
	ப்ரதிஜ்ஞா(வேர் && n);

	எனில் (வேர் == &empty) {
		வேர் = n;
	} இல்லையெனில் {
		முழு c = urldb_தேடல்_match_host(வேர்->data, n->data);

		எனில் (c > 0) {
			வேர்->இடது = urldb_தேடல்_செருகு_internal(
				வேர்->இடது, n);
		} இல்லையெனில் எனில் (c < 0) {
			வேர்->வலது = urldb_தேடல்_செருகு_internal(
				வேர்->வலது, n);
		} இல்லையெனில் {
			/* exact match */
			விடுவி(n);
			திரும்பு வேர்;
		}

		வேர் = urldb_தேடல்_skew(வேர்);
		வேர் = urldb_தேடல்_split(வேர்);
	}

	திரும்பு வேர்;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
நிலையான கட்டமைப்பு தேடல்_முனை *
urldb_தேடல்_செருகு(கட்டமைப்பு தேடல்_முனை *வேர், மாறிலி கட்டமைப்பு host_part *data)
{
	கட்டமைப்பு தேடல்_முனை *n;

	ப்ரதிஜ்ஞா(வேர் && data);

	n = malloc(அளவு(கட்டமைப்பு தேடல்_முனை));
	எனில் (!n)
		திரும்பு வெற்று;

	n->level = 1;
	n->data = data;
	n->இடது = n->வலது = &empty;

	வேர் = urldb_தேடல்_செருகு_internal(வேர், n);

	திரும்பு வேர்;
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
நிலையான bool
urldb_parse_avpair(கட்டமைப்பு cookie_internal_data *c,
		   எழுத்து *n,
		   எழுத்து *v,
		   bool was_quoted)
{
	முழு vlen;

	ப்ரதிஜ்ஞா(c && n && v);

	/* Strip whitespace from start of name */
	க்கு (; *n; n++) {
		எனில் (*n != ' ' && *n != '\t')
			நிறுத்து;
	}

	/* Strip whitespace from end of name */
	க்கு (vlen = strlen(n); vlen; vlen--) {
		எனில் (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		இல்லையெனில்
			நிறுத்து;
	}

	/* Strip whitespace from start of value */
	க்கு (; *v; v++) {
		எனில் (*v != ' ' && *v != '\t')
			நிறுத்து;
	}

	/* Strip whitespace from end of value */
	க்கு (vlen = strlen(v); vlen; vlen--) {
		எனில் (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		இல்லையெனில்
			நிறுத்து;
	}

	எனில் (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		எனில் (!c->comment)
			திரும்பு false;
	} இல்லையெனில் எனில் (!c->domain && strcasecmp(n, "Domain") == 0) {
		எனில் (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			எனில் (!c->domain)
				திரும்பு false;
		}
	} இல்லையெனில் எனில் (strcasecmp(n, "Max-Age") == 0) {
		முழு temp = atoi(v);
		எனில் (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		இல்லையெனில்
			c->expires = time(வெற்று) + temp;
	} இல்லையெனில் எனில் (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		எனில் (!c->path)
			திரும்பு false;
	} இல்லையெனில் எனில் (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} இல்லையெனில் எனில் (strcasecmp(n, "Expires") == 0) {
		எழுத்து *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		க்கு (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		எனில் (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} இல்லையெனில் எனில் (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} இல்லையெனில் எனில் (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} இல்லையெனில் எனில் (!c->name) {
		c->name = strdup(n);
		c->மதிப்பு = strdup(v);
		c->மதிப்பு_was_quoted = was_quoted;
		எனில் (!c->name || !c->மதிப்பு) {
			திரும்பு false;
		}
	}

	திரும்பு true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
நிலையான வெறுமை urldb_free_cookie(கட்டமைப்பு cookie_internal_data *c)
{
	ப்ரதிஜ்ஞா(c);

	விடுவி(c->comment);
	விடுவி(c->domain);
	விடுவி(c->path);
	விடுவி(c->name);
	விடுவி(c->மதிப்பு);
	விடுவி(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
நிலையான கட்டமைப்பு cookie_internal_data *
urldb_parse_cookie(nsurl *url, மாறிலி எழுத்து **cookie)
{
	கட்டமைப்பு cookie_internal_data *c;
	மாறிலி எழுத்து *cur;
	எழுத்து name[1024], மதிப்பு[4096];
	எழுத்து *n = name, *v = மதிப்பு;
	bool in_மதிப்பு = false;
	bool had_மதிப்பு_data = false;
	bool மதிப்பு_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ப்ரதிஜ்ஞா(url && cookie && *cookie);

	c = calloc(1, அளவு(கட்டமைப்பு cookie_internal_data));
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	c->expires = -1;

	name[0] = '\0';
	மதிப்பு[0] = '\0';

	க்கு (cur = *cookie; *cur; cur++) {
		எனில் (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			எனில் (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				மதிப்பு[0] = '\0';

			}

			நிறுத்து;
		} இல்லையெனில் எனில் (*cur == '\r') {
			/* Spurious linefeed */
			தொடர்;
		} இல்லையெனில் எனில் (*cur == '\n') {
			/* Spurious newline */
			தொடர்;
		}

		எனில் (in_மதிப்பு && !had_மதிப்பு_data) {
			எனில் (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				தொடர்;
			} இல்லையெனில் {
				had_மதிப்பு_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				எனில் (*cur != '"') {
					மதிப்பு_verbatim = true;
				}
			}
		}

		எனில் (in_மதிப்பு && !மதிப்பு_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			எனில் (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				தொடர்;
			}
		}

		எனில் (!quoted && !in_மதிப்பு && *cur == '=') {
			/* First equals => attr-value separator */
			in_மதிப்பு = true;
			தொடர்;
		}

		எனில் (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			எனில் (!urldb_parse_avpair(c, name, மதிப்பு, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				திரும்பு வெற்று;
			}

			/* And reset to start */
			n = name;
			v = மதிப்பு;
			in_மதிப்பு = false;
			had_மதிப்பு_data = false;
			மதிப்பு_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			எனில் (*cur != ';') {
				cur--;
			}

			தொடர்;
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
		எனில் (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			மாறிலி எழுத்து *p;
			மாறிலி எழுத்து *semi = strchr(cur + 1, ';');
			எனில் (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			க்கு (p = cur + 1; p < semi; p++)
				எனில் (*p == '=')
					நிறுத்து;

			எனில் (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} இல்லையெனில் {
				/* found one => comma marks end of cookie */
				cur++;
				நிறுத்து;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		எனில் (!in_மதிப்பு) {
			எனில் (n < name + (அளவு(name) - 1))
				*n++ = *cur;
		} இல்லையெனில் {
			எனில் (v < மதிப்பு + (அளவு(மதிப்பு) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	எனில் (!urldb_parse_avpair(c, name, மதிப்பு, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		திரும்பு வெற்று;
	}

	/* Now fix-up default values */
	எனில் (c->domain == வெற்று) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		எனில் (host == வெற்று) {
			urldb_free_cookie(c);
			திரும்பு வெற்று;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	எனில் (c->path == வெற்று) {
		மாறிலி எழுத்து *path_data;
		எழுத்து *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		எனில் (path_lwc == வெற்று) {
			urldb_free_cookie(c);
			திரும்பு வெற்று;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		எனில் (slash != வெற்று) {
			/* Special case: retain first slash in path */
			எனில் (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			எனில் (slash == வெற்று) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				திரும்பு வெற்று;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} இல்லையெனில் {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			எனில் (path == வெற்று) {
				urldb_free_cookie(c);
				திரும்பு வெற்று;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	திரும்பு c;
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
நிலையான கட்டமைப்பு path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned முழு port,
	       மாறிலி கட்டமைப்பு host_part *host,
	       எழுத்து *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	கட்டமைப்பு path_data *d, *e;
	எழுத்து *buf = path_query;
	எழுத்து *segment, *slash;
	bool match;

	ப்ரதிஜ்ஞா(scheme && host && url);

	d = (கட்டமைப்பு path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	எனில் (*segment == '/')
		segment++;

	/* Process path segments */
	செய் {
		slash = strchr(segment, '/');
		எனில் (!slash) {
			/* last segment */
			/* look for existing entry */
			க்கு (e = d->children; e; e = e->next)
				எனில் (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					நிறுத்து;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_முனை(scheme, port,
						    segment, fragment, d);
			நிறுத்து;
		}

		*slash = '\0';

		/* look for existing entry */
		க்கு (e = d->children; e; e = e->next)
			எனில் (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				நிறுத்து;

		d = e ? e : urldb_add_path_முனை(scheme, port, segment, வெற்று, d);
		எனில் (!d)
			நிறுத்து;

		segment = slash + 1;
	} போது (1);

	விடுவி(path_query);

	எனில் (d && !d->url) {
		/* Insert defragmented URL */
		எனில் (nsurl_defragment(url, &d->url) != NSERROR_OK)
			திரும்பு வெற்று;
	}

	திரும்பு d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
நிலையான கட்டமைப்பு host_part *urldb_add_host(மாறிலி எழுத்து *host)
{
	கட்டமைப்பு host_part *d = (கட்டமைப்பு host_part *) &db_வேர், *e;
	கட்டமைப்பு தேடல்_முனை *s;
	எழுத்து buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	எழுத்து *part;

	ப்ரதிஜ்ஞா(host);

	எனில் (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		க்கு (e = d->children; e; e = e->next)
			எனில் (strcasecmp(host, e->part) == 0)
				/* found => return it */
				திரும்பு e;

		d = urldb_add_host_முனை(host, d);

		s = urldb_தேடல்_செருகு(தேடல்_trees[ST_IP], d);
		எனில் (!s) {
			/* failed */
			d = வெற்று;
		} இல்லையெனில் {
			தேடல்_trees[ST_IP] = s;
		}

		திரும்பு d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, அளவு buf);
	buf[அளவு buf - 1] = '\0';

	/* Process FQDN segments backwards */
	செய் {
		part = strrchr(buf, '.');
		எனில் (!part) {
			/* last segment */
			/* Check for existing entry */
			க்கு (e = d->children; e; e = e->next)
				எனில் (strcasecmp(buf, e->part) == 0)
					நிறுத்து;

			எனில் (e) {
				d = e;
			} இல்லையெனில் {
				d = urldb_add_host_முனை(buf, d);
			}

			/* And insert into search tree */
			எனில் (d) {
				கட்டமைப்பு தேடல்_முனை **r;

				r = urldb_get_தேடல்_மரம்_direct(buf);
				s = urldb_தேடல்_செருகு(*r, d);
				எனில் (!s) {
					/* failed */
					d = வெற்று;
				} இல்லையெனில் {
					*r = s;
				}
			}
			நிறுத்து;
		}

		/* Check for existing entry */
		க்கு (e = d->children; e; e = e->next)
			எனில் (strcasecmp(part + 1, e->part) == 0)
				நிறுத்து;

		d = e ? e : urldb_add_host_முனை(part + 1, d);
		எனில் (!d)
			நிறுத்து;

		*part = '\0';
	} போது (1);

	திரும்பு d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
நிலையான bool
urldb_செருகு_cookie(கட்டமைப்பு cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	கட்டமைப்பு cookie_internal_data *d;
	மாறிலி கட்டமைப்பு host_part *h;
	கட்டமைப்பு path_data *p;
	time_t now = time(வெற்று);

	ப்ரதிஜ்ஞா(c);

	எனில் (c->domain[0] == '.') {
		h = urldb_தேடல்_find(
			urldb_get_தேடல்_மரம்(&(c->domain[1])),
			c->domain + 1);
		எனில் (!h) {
			h = urldb_add_host(c->domain + 1);
			எனில் (!h) {
				urldb_free_cookie(c);
				திரும்பு false;
			}
		}

		p = (கட்டமைப்பு path_data *) &h->paths;
	} இல்லையெனில் {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ப்ரதிஜ்ஞா(url != வெற்று);
		ப்ரதிஜ்ஞா(scheme != வெற்று);

		h = urldb_தேடல்_find(
			urldb_get_தேடல்_மரம்(c->domain),
			c->domain);

		எனில் (!h) {
			h = urldb_add_host(c->domain);
			எனில் (!h) {
				urldb_free_cookie(c);
				திரும்பு false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), வெற்று, url);
		எனில் (!p) {
			urldb_free_cookie(c);
			திரும்பு false;
		}
	}

	/* add cookie */
	க்கு (d = p->cookies; d; d = d->next) {
		எனில் (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			நிறுத்து;
	}

	எனில் (d) {
		எனில் (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			எனில் (d->next)
				d->next->prev = d->prev;
			இல்லையெனில்
				p->cookies_end = d->prev;
			எனில் (d->prev)
				d->prev->next = d->next;
			இல்லையெனில்
				p->cookies = d->next;

			cookie_manager_remove((கட்டமைப்பு cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} இல்லையெனில் {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			எனில் (c->next)
				c->next->prev = c;
			இல்லையெனில்
				p->cookies_end = c;
			எனில் (c->prev)
				c->prev->next = c;
			இல்லையெனில்
				p->cookies = c;

			cookie_manager_remove((கட்டமைப்பு cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((கட்டமைப்பு cookie_data *)c);
		}
	} இல்லையெனில் {
		c->prev = p->cookies_end;
		c->next = வெற்று;
		எனில் (p->cookies_end)
			p->cookies_end->next = c;
		இல்லையெனில்
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((கட்டமைப்பு cookie_data *)c);
	}

	திரும்பு true;
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
நிலையான bool
concat_netscape_cookie(கட்டமைப்பு cookie_internal_data *c,
		    முழு *used,
		    முழு *alloc,
		    எழுத்து **buf)
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
	முழு max_len;
	முழு outl; /* output length */

	ப்ரதிஜ்ஞா(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->மதிப்பு) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	எனில் (max_len >= *alloc) {
		எழுத்து *temp = realloc(*buf, max_len);
		எனில் (!temp) {
			திரும்பு false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	எனில் (outl >= (*alloc - *used)) {
		/* output truncated */
		திரும்பு false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	எனில் (c->மதிப்பு_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->மதிப்பு);
		எனில் (outl >= (*alloc - *used)) {
			/* output truncated */
			திரும்பு false;
		}
		*used += outl;
	} இல்லையெனில் {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->மதிப்பு);
		எனில் (outl >= (*alloc - *used)) {
			/* output truncated */
			திரும்பு false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	திரும்பு true;
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
நிலையான bool
concat_rfc2965_cookie(கட்டமைப்பு cookie_internal_data *c,
		      முழு *used,
		      முழு *alloc,
		      எழுத்து **buf)
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
	மாறிலி எழுத்து * மாறிலி separators = "()<>@,;:\\\"/[]?={} \t";
	முழு max_len;
	முழு outl; /* output length */

	ப்ரதிஜ்ஞா(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->மதிப்பு) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	எனில் (max_len >= *alloc) {
		எழுத்து *temp = realloc(*buf, max_len);
		எனில் (!temp) {
			திரும்பு false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	எனில் (outl >= (*alloc - *used)) {
		/* output truncated */
		திரும்பு false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	எனில் (c->மதிப்பு_was_quoted ||
	    strpbrk(c->மதிப்பு, separators) != வெற்று) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->மதிப்பு);

	} இல்லையெனில் {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->மதிப்பு);
	}
	எனில் (outl >= (*alloc - *used)) {
		/* output truncated */
		திரும்பு false;
	}
	*used += outl;

	எனில் (c->path_from_set) {
		/* Path, quoted if necessary */

		எனில் (strpbrk(c->path, separators) != வெற்று) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} இல்லையெனில் {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		எனில் (outl >= (*alloc - *used)) {
			/* output truncated */
			திரும்பு false;
		}
		*used += outl;
	}

	எனில் (c->domain_from_set) {
		/* Domain, quoted if necessary */
		எனில் (strpbrk(c->domain, separators) != வெற்று) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} இல்லையெனில் {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		எனில் (outl >= (*alloc - *used)) {
			/* output truncated */
			திரும்பு false;
		}
		*used += outl;
	}

	திரும்பு true;
}


/**
 * build output cookie string in netscape format
 */
நிலையான எழுத்து *
get_netscape_cookie(கட்டமைப்பு cookie_internal_data **matched_cookies, முழு மொத்தம்)
{
	முழு ret_alloc = 4096, ret_used = 1;
	எழுத்து *ret;
	முழு midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	எனில் (ret == வெற்று) {
		திரும்பு வெற்று;
	}
	ret[0] = '\0';

	க்கு (midx = 0; midx < மொத்தம்; midx++) {
		எனில் (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			விடுவி(ret);
			திரும்பு வெற்று;
		}
	}

	எனில் (ret_used <= 2) {
		விடுவி(ret);
		திரும்பு வெற்று;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	எழுத்து *temp = realloc(ret, ret_used);
	எனில் (temp != வெற்று) {
		ret = temp;
	}

	திரும்பு ret;
}


/**
 * build output cookie string in RFC2965 format
 */
நிலையான எழுத்து *
get_rfc2965_cookie(கட்டமைப்பு cookie_internal_data **matched_cookies,
		   முழு மொத்தம்,
		   முழு version)
{
	முழு ret_alloc = 4096; /* return string allocation */
	முழு ret_used = 1; /* return sting buffer used */
	எழுத்து *ret; /* return string buffer */
	முழு midx; /* index into matched cookies */
	முழு outl;

	ret = malloc(ret_alloc);
	எனில் (ret == வெற்று) {
		திரும்பு வெற்று;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	எனில் (outl >= ret_alloc - ret_used) {
		/* overflow */
		விடுவி(ret);
		திரும்பு வெற்று;
	}
	ret_used += outl;

	க்கு (midx = 0; midx < மொத்தம்; midx++) {
		எனில் (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			விடுவி(ret);
			திரும்பு வெற்று;
		}
	}

	/* shrink the output buffer to the required size */
	எழுத்து *temp = realloc(ret, ret_used);
	எனில் (temp != வெற்று) {
		ret = temp;
	}

	திரும்பு ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
நிலையான வெறுமை
urldb_delete_cookie_paths(மாறிலி எழுத்து *domain,
			  மாறிலி எழுத்து *path,
			  மாறிலி எழுத்து *name,
			  கட்டமைப்பு path_data *parent)
{
	கட்டமைப்பு cookie_internal_data *c;
	கட்டமைப்பு path_data *p = parent;

	ப்ரதிஜ்ஞா(parent);

	செய் {
		க்கு (c = p->cookies; c; c = c->next) {
			எனில் (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				எனில் (c->prev) {
					c->prev->next = c->next;
				} இல்லையெனில் {
					p->cookies = c->next;
				}

				எனில் (c->next) {
					c->next->prev = c->prev;
				} இல்லையெனில் {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				திரும்பு;
			}
		}

		எனில் (p->children) {
			p = p->children;
		} இல்லையெனில் {
			போது (p != parent) {
				எனில் (p->next != வெற்று) {
					p = p->next;
					நிறுத்து;
				}

				p = p->parent;
			}
		}
	} போது (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
நிலையான வெறுமை
urldb_delete_cookie_hosts(மாறிலி எழுத்து *domain,
			  மாறிலி எழுத்து *path,
			  மாறிலி எழுத்து *name,
			  கட்டமைப்பு host_part *parent)
{
	கட்டமைப்பு host_part *h;
	ப்ரதிஜ்ஞா(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	க்கு (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
நிலையான வெறுமை urldb_save_cookie_paths(FILE *fp, கட்டமைப்பு path_data *parent)
{
	கட்டமைப்பு path_data *p = parent;
	time_t now = time(வெற்று);

	ப்ரதிஜ்ஞா(fp && parent);

	செய் {
		எனில் (p->cookies != வெற்று) {
			கட்டமைப்பு cookie_internal_data *c;

			க்கு (c = p->cookies; c != வெற்று; c = c->next) {
				எனில் (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					தொடர்;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(முழு)c->expires, (முழு)c->last_used,
					c->no_destroy, c->name, c->மதிப்பு,
					c->மதிப்பு_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		எனில் (p->children != வெற்று) {
			p = p->children;
		} இல்லையெனில் {
			போது (p != parent) {
				எனில் (p->next != வெற்று) {
					p = p->next;
					நிறுத்து;
				}

				p = p->parent;
			}
		}
	} போது (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
நிலையான வெறுமை urldb_save_cookie_hosts(FILE *fp, கட்டமைப்பு host_part *parent)
{
	கட்டமைப்பு host_part *h;
	ப்ரதிஜ்ஞா(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	க்கு (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
நிலையான வெறுமை urldb_destroy_cookie(கட்டமைப்பு cookie_internal_data *c)
{
	விடுவி(c->name);
	விடுவி(c->மதிப்பு);
	விடுவி(c->comment);
	விடுவி(c->domain);
	விடுவி(c->path);

	விடுவி(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
நிலையான வெறுமை urldb_destroy_path_முனை_content(கட்டமைப்பு path_data *முனை)
{
	கட்டமைப்பு cookie_internal_data *a, *b;
	unsigned முழு i;

	எனில் (முனை->url != வெற்று) {
		nsurl_unref(முனை->url);
	}

	lwc_string_unref(முனை->scheme);

	விடுவி(முனை->segment);
	க்கு (i = 0; i < முனை->frag_cnt; i++)
		விடுவி(முனை->fragment[i]);
	விடுவி(முனை->fragment);

	விடுவி(முனை->urld.title);

	க்கு (a = முனை->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
நிலையான வெறுமை urldb_destroy_prot_space(கட்டமைப்பு prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	விடுவி(space->realm);
	விடுவி(space->auth);

	விடுவி(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
நிலையான வெறுமை urldb_destroy_path_மரம்(கட்டமைப்பு path_data *வேர்)
{
	கட்டமைப்பு path_data *p = வேர்;

	செய் {
		எனில் (p->children != வெற்று) {
			p = p->children;
		} இல்லையெனில் {
			கட்டமைப்பு path_data *q = p;

			போது (p != வேர்) {
				எனில் (p->next != வெற்று) {
					p = p->next;
					நிறுத்து;
				}

				p = p->parent;

				urldb_destroy_path_முனை_content(q);
				விடுவி(q);

				q = p;
			}

			urldb_destroy_path_முனை_content(q);
			விடுவி(q);
		}
	} போது (p != வேர்);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
நிலையான வெறுமை urldb_destroy_host_மரம்(கட்டமைப்பு host_part *வேர்)
{
	கட்டமைப்பு host_part *a, *b;
	கட்டமைப்பு path_data *p, *q;
	கட்டமைப்பு prot_space_data *s, *t;

	/* Destroy children */
	க்கு (a = வேர்->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_மரம்(a);
	}

	/* Now clean up paths */
	க்கு (p = வேர்->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_மரம்(p);
	}

	/* Root path */
	urldb_destroy_path_முனை_content(&வேர்->paths);

	/* Proctection space data */
	க்கு (s = வேர்->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	விடுவி(வேர்->part);
	விடுவி(வேர்);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
நிலையான வெறுமை urldb_destroy_தேடல்_மரம்(கட்டமைப்பு தேடல்_முனை *வேர்)
{
	/* Destroy children */
	எனில் (வேர்->இடது != &empty)
		urldb_destroy_தேடல்_மரம்(வேர்->இடது);
	எனில் (வேர்->வலது != &empty)
		urldb_destroy_தேடல்_மரம்(வேர்->வலது);

	/* And destroy ourselves */
	விடுவி(வேர்);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
வெறுமை urldb_destroy(வெறுமை)
{
	கட்டமைப்பு host_part *a, *b;
	முழு i;

	/* Clean up search trees */
	க்கு (i = 0; i < இலக்கம்_தேடல்_TREES; i++) {
		எனில் (தேடல்_trees[i] != &empty) {
			urldb_destroy_தேடல்_மரம்(தேடல்_trees[i]);
			தேடல்_trees[i] = &empty;
		}
	}

	/* And database */
	க்கு (a = db_வேர்.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_மரம்(a);
	}
	memset(&db_வேர், 0, அளவு(db_வேர்));

	/* And the bloom filter */
	எனில் (url_bloom != வெற்று) {
		bloom_destroy(url_bloom);
		url_bloom = வெற்று;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(மாறிலி எழுத்து *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	எழுத்து s[MAXIMUM_URL_LENGTH];
	எழுத்து host[256];
	கட்டமைப்பு host_part *h;
	முழு urls;
	முழு i;
	முழு version;
	முழு length;
	FILE *fp;

	ப்ரதிஜ்ஞா(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	எனில் (url_bloom == வெற்று)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	எனில் (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		திரும்பு NSERROR_NOT_FOUND;
	}

	எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		திரும்பு NSERROR_NEED_DATA;
	}

	version = atoi(s);
	எனில் (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		திரும்பு NSERROR_INVALID;
	}
	எனில் (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		திரும்பு NSERROR_INVALID;
	}

	போது (fgets(host, அளவு host, fp)) {
		time_t hsts_expiry = 0;
		முழு hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		எனில் (length == 0) {
			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;
			urls = atoi(s);
			/* Eight fields/url */
			க்கு (i = 0; i < (8 * urls); i++) {
				எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					நிறுத்து;
			}
			தொடர்;
		}

		எனில் (version >= 107) {
			எழுத்து *p = host;
			போது (*p && *p != ' ') p++;
			போது (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			போது (*p && *p != ' ') p++;
			போது (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		எனில் (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			திரும்பு NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			நிறுத்து;
		urls = atoi(s);

		/* no URLs => try next host */
		எனில் (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			தொடர்;
		}

		/* load the non-corrupt data */
		க்கு (i = 0; i < urls; i++) {
			கட்டமைப்பு path_data *p = வெற்று;
			எழுத்து scheme[64], ports[10];
			எழுத்து url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned முழு port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			எழுத்து *path_query;
			size_t len;

			எனில் (!fgets(scheme, அளவு scheme, fp))
				நிறுத்து;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			எனில் (!fgets(ports, அளவு ports, fp))
				நிறுத்து;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;
			length = strlen(s) - 1;
			s[length] = '\0';

			எனில் (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, அளவு url, "%s://%s%s%s%s",
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
			எனில் (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				திரும்பு NSERROR_NOMEM;
			}

			எனில் (url_bloom != வெற்று) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_செருகு_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			எனில் (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				திரும்பு NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			எனில் (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				திரும்பு NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;
			எனில் (p)
				p->urld.visits = (unsigned முழு)atoi(s);

			/* entry last use time */
			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				நிறுத்து;
			}
			எனில் (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;
			எனில் (p)
				p->urld.type = (content_type)atoi(s);

			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;


			எனில் (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				நிறுத்து;
			length = strlen(s) - 1;
			எனில் (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				எனில் (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	திரும்பு NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(மாறிலி எழுத்து *filename)
{
	FILE *fp;
	முழு i;

	ப்ரதிஜ்ஞா(filename);

	fp = fopen(filename, "w");
	எனில் (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		திரும்பு NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	க்கு (i = 0; i != இலக்கம்_தேடல்_TREES; i++) {
		urldb_save_தேடல்_மரம்(தேடல்_trees[i], fp);
	}

	fclose(fp);

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	கட்டமைப்பு path_data *p;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p) {
		திரும்பு NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	கட்டமைப்பு host_part *h;
	கட்டமைப்பு path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	மாறிலி எழுத்து *host_str;
	எழுத்து *path_query = வெற்று;
	size_t len;
	bool match;
	unsigned முழு port_int;

	ப்ரதிஜ்ஞா(url);

	எனில் (url_bloom == வெற்று)
		url_bloom = bloom_create(BLOOM_SIZE);

	எனில் (url_bloom != வெற்று) {
		uint32_t hash = nsurl_hash(url);
		bloom_செருகு_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	எனில் (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		திரும்பு false;
	}
	ப்ரதிஜ்ஞா(path_query != வெற்று);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	எனில் (scheme == வெற்று) {
		விடுவி(path_query);
		திரும்பு false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	எனில் (host != வெற்று) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} இல்லையெனில் எனில் (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} இல்லையெனில் {
		lwc_string_unref(scheme);
		விடுவி(path_query);
		திரும்பு false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	எனில் (port != வெற்று) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} இல்லையெனில் {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	எனில் (h != வெற்று) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} இல்லையெனில் {
		p = வெற்று;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	திரும்பு (p != வெற்று);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, மாறிலி எழுத்து *title)
{
	கட்டமைப்பு path_data *p;
	எழுத்து *temp;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (p == வெற்று) {
		திரும்பு NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	எனில் (title != வெற்று) {
		temp = strdup(title);
		எனில் (temp == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
	} இல்லையெனில் {
		temp = வெற்று;
	}

	விடுவி(p->urld.title);
	p->urld.title = temp;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	கட்டமைப்பு path_data *p;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p) {
		திரும்பு NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	கட்டமைப்பு path_data *p;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p) {
		திரும்பு NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(வெற்று);
	p->urld.visits++;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
வெறுமை urldb_reset_url_visit_data(nsurl *url)
{
	கட்டமைப்பு path_data *p;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
மாறிலி கட்டமைப்பு url_data *urldb_get_url_data(nsurl *url)
{
	கட்டமைப்பு path_data *p;
	கட்டமைப்பு url_internal_data *u;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு வெற்று;

	u = &p->urld;

	திரும்பு (மாறிலி கட்டமைப்பு url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	கட்டமைப்பு path_data *p;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு வெற்று;

	திரும்பு p->url;
}


/* exported interface documented in netsurf/url_db.h */
வெறுமை urldb_set_auth_details(nsurl *url, மாறிலி எழுத்து *realm, மாறிலி எழுத்து *auth)
{
	கட்டமைப்பு path_data *p, *pi;
	கட்டமைப்பு host_part *h;
	கட்டமைப்பு prot_space_data *space, *space_alloc;
	எழுத்து *realm_alloc, *auth_alloc;
	bool match;

	ப்ரதிஜ்ஞா(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	எனில் (!p)
		திரும்பு;

	/* Search for host_part */
	க்கு (pi = p; pi->parent != வெற்று; pi = pi->parent)
		;
	h = (கட்டமைப்பு host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	க்கு (space = h->prot_space; space; space = space->next) {
		எனில் (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			நிறுத்து;
	}

	எனில் (space != வெற்று) {
		/* Overrule existing auth. */
		விடுவி(space->auth);
		space->auth = strdup(auth);
	} இல்லையெனில் {
		/* Create a new protection space. */
		space = space_alloc = malloc(அளவு(கட்டமைப்பு prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		எனில் (!space_alloc || !realm_alloc || !auth_alloc) {
			விடுவி(space_alloc);
			விடுவி(realm_alloc);
			விடுவி(auth_alloc);
			திரும்பு;
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
மாறிலி எழுத்து *urldb_get_auth_details(nsurl *url, மாறிலி எழுத்து *realm)
{
	கட்டமைப்பு path_data *p, *p_cur, *p_உச்சி;

	ப்ரதிஜ்ஞா(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு வெற்று;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	க்கு (p_cur = p; p_cur != வெற்று; p_உச்சி = p_cur, p_cur = p_cur->parent) {
		எனில் (p_cur->prot_space) {
			திரும்பு p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	எனில் (realm != வெற்று) {
		மாறிலி கட்டமைப்பு host_part *h = (மாறிலி கட்டமைப்பு host_part *)p_உச்சி;
		மாறிலி கட்டமைப்பு prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		க்கு (space = h->prot_space; space != வெற்று;
		     space = space->next) {
			எனில் (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				திரும்பு p->prot_space->auth;
			}
		}
	}

	திரும்பு வெற்று;
}


/* exported interface documented in netsurf/url_db.h */
வெறுமை urldb_set_cert_permissions(nsurl *url, bool permit)
{
	கட்டமைப்பு path_data *p;
	கட்டமைப்பு host_part *h;

	ப்ரதிஜ்ஞா(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு;

	க்கு (; p && p->parent; p = p->parent)
		/* do nothing */;
	ப்ரதிஜ்ஞா(p);

	h = (கட்டமைப்பு host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	கட்டமைப்பு path_data *p;
	மாறிலி கட்டமைப்பு host_part *h;

	ப்ரதிஜ்ஞா(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு false;

	க்கு (; p && p->parent; p = p->parent)
		/* do nothing */;
	ப்ரதிஜ்ஞா(p);

	h = (மாறிலி கட்டமைப்பு host_part *)p;

	திரும்பு h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(கட்டமைப்பு nsurl *url, மாறிலி எழுத்து *header)
{
	கட்டமைப்பு path_data *p;
	கட்டமைப்பு host_part *h;
	lwc_string *host;
	time_t now = time(வெற்று);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ப்ரதிஜ்ஞா(url);

	host = nsurl_get_component(url, NSURL_HOST);
	எனில் (host != வெற்று) {
		எனில் (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			திரும்பு true;
		} இல்லையெனில் எனில் (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			திரும்பு true;
		}

		lwc_string_unref(host);
	} இல்லையெனில் {
		/* No host part: ignore */
		திரும்பு true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு false;

	க்கு (; p && p->parent; p = p->parent)
		/* do nothing */;
	ப்ரதிஜ்ஞா(p);

	h = (கட்டமைப்பு host_part *)p;
	எனில் (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		திரும்பு true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	எனில் (error != NSERROR_OK) {
		/* Parse failed: ignore */
		திரும்பு true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	எனில் (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} இல்லையெனில் எனில் ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	திரும்பு true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(கட்டமைப்பு nsurl *url)
{
	கட்டமைப்பு path_data *p;
	மாறிலி கட்டமைப்பு host_part *h;
	lwc_string *host;
	time_t now = time(வெற்று);

	ப்ரதிஜ்ஞா(url);

	host = nsurl_get_component(url, NSURL_HOST);
	எனில் (host != வெற்று) {
		எனில் (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			திரும்பு false;
		} இல்லையெனில் எனில் (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			திரும்பு false;
		}

		lwc_string_unref(host);
	} இல்லையெனில் {
		/* No host part: not enabled */
		திரும்பு false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு false;

	க்கு (; p && p->parent; p = p->parent)
		/* do nothing */;
	ப்ரதிஜ்ஞா(p);

	h = (மாறிலி கட்டமைப்பு host_part *)p;

	/* Consult record for this host */
	எனில் (h->hsts.expires > now) {
		/* Not expired */
		திரும்பு true;
	}

	/* Consult parent domains */
	க்கு (h = h->parent; h && h != &db_வேர்; h = h->parent) {
		எனில் (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			திரும்பு true;
		}
	}

	திரும்பு false;
}


/* exported interface documented in netsurf/url_db.h */
வெறுமை
urldb_iterate_partial(மாறிலி எழுத்து *prefix,
		      bool (*callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data))
{
	எழுத்து host[256];
	எழுத்து buf[260]; /* max domain + "www." */
	மாறிலி எழுத்து *slash, *scheme_sep;
	கட்டமைப்பு தேடல்_முனை *மரம்;
	மாறிலி கட்டமைப்பு host_part *h;

	ப்ரதிஜ்ஞா(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	எனில் (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	மரம் = urldb_get_தேடல்_மரம்(prefix);

	எனில் (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, அளவு host, "%.*s",
			 (முழு) (slash - prefix), prefix);

		h = urldb_தேடல்_find(மரம், host);
		எனில் (!h) {
			முழு len = slash - prefix;

			எனில் (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, அளவு buf, "www.%s", host);
				h = urldb_தேடல்_find(
					தேடல்_trees[ST_DN + 'w' - 'a'],
					buf);
				எனில் (!h)
					திரும்பு;
			} இல்லையெனில்
				திரும்பு;
		}

		எனில் (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} இல்லையெனில் {
		முழு len = strlen(prefix);

		/* looking for hosts */
		எனில் (!urldb_iterate_partial_host(மரம், prefix, callback))
			திரும்பு;

		எனில் (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, அளவு buf, "www.%s", prefix);
			எனில்(!urldb_iterate_partial_host(
				   தேடல்_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				திரும்பு;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
வெறுமை
urldb_iterate_entries(bool (*callback)(nsurl *url, மாறிலி கட்டமைப்பு url_data *data))
{
	முழு i;

	ப்ரதிஜ்ஞா(callback);

	க்கு (i = 0; i < இலக்கம்_தேடல்_TREES; i++) {
		எனில் (!urldb_iterate_entries_host(தேடல்_trees[i],
						callback,
						வெற்று)) {
			நிறுத்து;
		}
	}
}


/* exported interface documented in content/urldb.h */
வெறுமை urldb_iterate_cookies(bool (*callback)(மாறிலி கட்டமைப்பு cookie_data *data))
{
	முழு i;

	ப்ரதிஜ்ஞா(callback);

	க்கு (i = 0; i < இலக்கம்_தேடல்_TREES; i++) {
		எனில் (!urldb_iterate_entries_host(தேடல்_trees[i],
						வெற்று, callback))
			நிறுத்து;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(மாறிலி எழுத்து *header, nsurl *url, nsurl *referer)
{
	மாறிலி எழுத்து *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	ப்ரதிஜ்ஞா(url && header);

	/* Get defragmented URL, as 'urlt' */
	எனில் (nsurl_defragment(url, &urlt) != NSERROR_OK)
		திரும்பு வெற்று;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	எனில் (scheme == வெற்று) {
		nsurl_unref(urlt);
		திரும்பு false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	எனில் (path == வெற்று) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		திரும்பு false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	எனில் (host == வெற்று) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		திரும்பு false;
	}

	எனில் (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		எனில் (rhost == வெற்று) {
			செல் error;
		}

		/* Domain match host names */
		எனில் (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			மாறிலி எழுத்து *hptr;
			மாறிலி எழுத்து *rptr;
			மாறிலி எழுத்து *dot;
			மாறிலி எழுத்து *host_data = lwc_string_data(host);
			மாறிலி எழுத்து *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			எனில் (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				செல் error;
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
			போது (hptr >= host_data && rptr >= rhost_data) {
				எனில் (*hptr != *rptr)
					நிறுத்து;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			போது (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			எனில் (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == வெற்று ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				செல் error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	செய் {
		கட்டமைப்பு cookie_internal_data *c;
		எழுத்து *dot;
		size_t len;
#ifdef WITH_NSPSL
		மாறிலி எழுத்து *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		எனில் (!c) {
			/* failed => stop parsing */
			செல் error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		எனில் (!c->name || !c->மதிப்பு) {
			urldb_free_cookie(c);
			செல் error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		எனில் (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			செல் error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		எனில் (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		எனில் (suffix == வெற்று) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			செல் error;
		}
#இல்லையெனில்
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		எனில் (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			செல் error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		எனில் (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			முழு hlen, dlen;
			எழுத்து *domain = c->domain;

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
			ப்ரதிஜ்ஞா(c->domain[0] == '.');

			/* 4.3.2:iii */
			எனில் (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				செல் error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			எனில் (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				செல் error;
			}

			எனில் (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			எனில் (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				செல் error;
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
		எனில் (!urldb_செருகு_cookie(c, scheme, urlt))
			செல் error;
	} போது (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	திரும்பு true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	திரும்பு false;
}


/* exported interface documented in content/urldb.h */
எழுத்து *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	மாறிலி கட்டமைப்பு path_data *p, *q;
	மாறிலி கட்டமைப்பு host_part *h;
	lwc_string *path_lwc;
	கட்டமைப்பு cookie_internal_data *c;
	முழு மொத்தம் = 0, version = COOKIE_RFC2965;
	கட்டமைப்பு cookie_internal_data **matched_cookies;
	முழு matched_cookies_size = 20;
	மாறிலி எழுத்து *path;
	எழுத்து *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	ப்ரதிஜ்ஞா(url != வெற்று);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	எனில் (!p)
		திரும்பு வெற்று;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 அளவு(கட்டமைப்பு cookie_internal_data *));
	எனில் (!matched_cookies)
		திரும்பு வெற்று;

#define GROW_MATCHED_COOKIES						\
	செய் {								\
		எனில் (மொத்தம் == matched_cookies_size) {			\
			கட்டமைப்பு cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       அளவு(கட்டமைப்பு cookie_internal_data *)); \
									\
			எனில் (temp == வெற்று) {				\
				விடுவி(matched_cookies);			\
				திரும்பு வெற்று;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} போது(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	எனில் (path_lwc == வெற்று) {
		விடுவி(matched_cookies);
		திரும்பு வெற்று;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(வெற்று);

	எனில் (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		க்கு (q = p->parent->children; q; q = q->next) {
			எனில் (strcmp(q->segment, p->segment))
				தொடர்;

			/* Consider all cookies associated with
			 * this exact path */
			க்கு (c = q->cookies; c; c = c->next) {
				எனில் (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					தொடர்;

				எனில் (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					தொடர்;

				எனில் (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					தொடர்;

				matched_cookies[மொத்தம்++] = c;

				GROW_MATCHED_COOKIES;

				எனில் (c->version < (unsigned முழு)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((கட்டமைப்பு cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	க்கு (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		க்கு (q = p->children; q; q = q->next) {
			எனில் (*(q->segment) != '\0')
				தொடர்;

			க்கு (c = q->cookies; c; c = c->next) {
				எனில் (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					தொடர்;

				எனில் (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					தொடர்;

				matched_cookies[மொத்தம்++] = c;

				GROW_MATCHED_COOKIES;

				எனில் (c->version < (unsigned முழு) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((கட்டமைப்பு cookie_data *)c);
			}
		}

		எனில் (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			நிறுத்து;
		}

		/* Consider p itself - may be the result of Path=/foo */
		க்கு (c = p->cookies; c; c = c->next) {
			எனில் (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				தொடர்;

			/* Ensure cookie path is a prefix of the resource */
			எனில் (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				தொடர்;

			எனில் (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				தொடர்;

			matched_cookies[மொத்தம்++] = c;

			GROW_MATCHED_COOKIES;

			எனில் (c->version < (unsigned முழு) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((கட்டமைப்பு cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	க்கு (h = (மாறிலி கட்டமைப்பு host_part *)p; h && h != &db_வேர்;
	     h = h->parent) {
		க்கு (c = h->paths.cookies; c; c = c->next) {
			எனில் (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				தொடர்;

			/* Ensure cookie path is a prefix of the resource */
			எனில் (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				தொடர்;

			எனில் (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				தொடர்;

			matched_cookies[மொத்தம்++] = c;

			GROW_MATCHED_COOKIES;

			எனில் (c->version < (unsigned முழு)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((கட்டமைப்பு cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	எனில் (மொத்தம் == 0) {
		/* No cookies found */
		விடுவி(matched_cookies);
		திரும்பு வெற்று;
	}

	எனில் (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, மொத்தம்);
	} இல்லையெனில் {
		ret = get_rfc2965_cookie(matched_cookies, மொத்தம், version);
	}
	விடுவி(matched_cookies);

	திரும்பு ret;
}


/* exported interface documented in content/urldb.h */
வெறுமை urldb_delete_cookie(மாறிலி எழுத்து *domain, மாறிலி எழுத்து *path,
			 மாறிலி எழுத்து *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_வேர்);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(மாறிலி எழுத்து *filename)
{
	FILE *fp;
	எழுத்து s[16*1024];

	ப்ரதிஜ்ஞா(filename);

	fp = fopen(filename, "r");
	எனில் (!fp)
		திரும்பு NSERROR_NOT_FOUND;

#define FIND_T {				\
		க்கு (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		எனில் (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			தொடர்;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		க்கு (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		எனில் (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			தொடர்;		\
		}				\
	}

	போது (fgets(s, அளவு s, fp)) {
		எழுத்து *p = s, *end = 0,
			*domain, *path, *name, *மதிப்பு, *scheme, *url,
			*comment;
		முழு version, domain_specified, path_specified,
			secure, http_only, no_destroy, மதிப்பு_quoted;
		time_t expires, last_used;
		கட்டமைப்பு cookie_internal_data *c;

		எனில்(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			தொடர்;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		எனில் (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			எனில் (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				நிறுத்து;
			}

			தொடர்;
		} இல்லையெனில் எனில் (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			தொடர்;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		எனில் (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} இல்லையெனில் {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; மதிப்பு = p; FIND_T;
		எனில் (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	மதிப்பு_quoted = atoi(p); FIND_T;
		} இல்லையெனில் {
			மதிப்பு_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		க்கு (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ப்ரதிஜ்ஞா(p <= end);

		/* Now create cookie */
		c = malloc(அளவு(கட்டமைப்பு cookie_internal_data));
		எனில் (!c)
			நிறுத்து;

		c->name = strdup(name);
		c->மதிப்பு = strdup(மதிப்பு);
		c->மதிப்பு_was_quoted = மதிப்பு_quoted;
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

		எனில் (!(c->name && c->மதிப்பு && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			நிறுத்து;
		}

		எனில் (c->domain[0] != '.') {
			lwc_string *scheme_lwc = வெற்று;
			nsurl *url_nsurl = வெற்று;

			ப்ரதிஜ்ஞா(scheme[0] != 'u');

			எனில் (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				நிறுத்து;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			எனில் (!urldb_செருகு_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				நிறுத்து;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} இல்லையெனில் {
			எனில் (!urldb_செருகு_cookie(c, வெற்று, வெற்று)) {
				/* Cookie freed for us */
				நிறுத்து;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	திரும்பு NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(மாறிலி எழுத்து *filename)
{
	FILE *fp;
	முழு cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	ப்ரதிஜ்ஞா(filename);

	fp = fopen(filename, "w");
	எனில் (!fp)
		திரும்பு NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_வேர்);

	fclose(fp);

	திரும்பு NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
வெறுமை urldb_dump(வெறுமை)
{
	முழு i;

	urldb_dump_hosts(&db_வேர்);

	க்கு (i = 0; i != இலக்கம்_தேடல்_TREES; i++) {
		urldb_dump_தேடல்(தேடல்_trees[i], 0);
	}
}
