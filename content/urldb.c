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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <time.h>
#ifdef WITH_NSPSL
#અન્તર્ભાવ <nspsl.h>
#endif

#અન્તર્ભાવ "utils/inet.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/url.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/bloom.h"
#અન્તર્ભાવ "utils/time.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "utils/http.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "desktop/cookie_manager.h"

#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
સંરચના cookie_internal_data {
	સંરચના cookie_internal_data *prev;	/**< Previous in list */
	સંરચના cookie_internal_data *next;	/**< Next in list */

	અક્ષર *name;		/**< Cookie name */
	અક્ષર *કિંમત;		/**< Cookie value */
	bool કિંમત_was_quoted;	/**< Value was quoted in Set-Cookie: */
	અક્ષર *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	અક્ષર *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	અક્ષર *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ગણના cookie_version version;	/**< Specification compliance */
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
સંરચના prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned પૂર્ણાંક port;
	/** Protection realm */
	અક્ષર *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	અક્ષર *auth;
	/** Next sibling */
	સંરચના prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
સંરચના url_internal_data {
	અક્ષર *title;		/**< Resource title */
	unsigned પૂર્ણાંક visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
સંરચના path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned પૂર્ણાંક port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	અક્ષર *segment;		/**< Path segment for this node */
	unsigned પૂર્ણાંક frag_cnt;	/**< Number of entries in path_data::fragment */
	અક્ષર **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	સંરચના url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	અચલ સંરચના prot_space_data *prot_space;
	/** Cookies associated with resource */
	સંરચના cookie_internal_data *cookies;
	/** Last cookie in list */
	સંરચના cookie_internal_data *cookies_end;

	સંરચના path_data *next;	/**< Next sibling */
	સંરચના path_data *prev;	/**< Previous sibling */
	સંરચના path_data *parent; /**< Parent path segment */
	સંરચના path_data *children; /**< Child path segments */
	સંરચના path_data *last; /**< Last child */
};

સંરચના hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

સંરચના host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	સંરચના path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	સંરચના hsts_data hsts;

	/**
	 * Part of host string
	 */
	અક્ષર *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	સંરચના prot_space_data *prot_space;

	સંરચના host_part *next;	/**< Next sibling */
	સંરચના host_part *prev;	/**< Previous sibling */
	સંરચના host_part *parent; /**< Parent host part */
	સંરચના host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
સંરચના શોધ_ગાંઠ {
	અચલ સંરચના host_part *data;	/**< Host tree entry */

	unsigned પૂર્ણાંક level;		/**< Node level */

	સંરચના શોધ_ગાંઠ *ડાબું;	/**< Left subtree */
	સંરચના શોધ_ગાંઠ *જમણું;	/**< Right subtree */
};

/** Root database handle */
સ્થિર સંરચના host_part db_મૂળ;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define અંક_શોધ_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
સ્થિર સંરચના શોધ_ગાંઠ empty = { 0, 0, &empty, &empty };
સ્થિર સંરચના શોધ_ગાંઠ *શોધ_trees[અંક_શોધ_TREES] = {
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
સ્થિર પૂર્ણાંક loaded_cookie_file_version;

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
સ્થિર સંરચના bloom_filter *url_bloom;
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
સ્થિર nserror urldb_write_timet(FILE *fp, time_t val)
{
	પૂર્ણાંક use;
	અક્ષર op[32];

	use = nsc_sntimet(op, 32, &val);
	જો (use == 0) {
		fprintf(fp, "%i\n", (પૂર્ણાંક)val);
	} નહીંતર {
		fprintf(fp, "%.*s\n", use, op);
	}
	પાછા NSERROR_OK;
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
સ્થિર શૂન્ય
urldb_write_paths(અચલ સંરચના path_data *parent,
		  અચલ અક્ષર *host,
		  FILE *fp,
		  અક્ષર **path,
		  પૂર્ણાંક *path_alloc,
		  પૂર્ણાંક *path_used,
		  time_t expiry)
{
	અચલ સંરચના path_data *p = parent;
	પૂર્ણાંક i;

	કરો {
		પૂર્ણાંક seglen = p->segment != ખાલી ? strlen(p->segment) : 0;
		પૂર્ણાંક len = *path_used + seglen + 1;

		જો (*path_alloc < len) {
			અક્ષર *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			જો (!temp) {
				પાછા;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		જો (p->segment != ખાલી) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		જો (p->children != ખાલી) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} નહીંતર {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		જો (p->children != ખાલી) {
			/* Drill down into children */
			p = p->children;
		} નહીંતર {
			/* leaf node */
			જો (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				જો (p->port) {
					fprintf(fp,"%d\n", p->port);
				} નહીંતર {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (પૂર્ણાંક)p->urld.type);

				fprintf(fp, "\n");

				જો (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					માટે (i = 0; s[i] != '\0'; i++)
						જો (s[i] < 32)
							s[i] = ' ';
					માટે (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} નહીંતર {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			જ્યારે (p != parent) {
				પૂર્ણાંક seglen = p->segment != ખાલી
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				જો (p->next != ખાલી) {
					/* Have a sibling, process that */
					p = p->next;
					તોડો;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} જ્યારે (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
સ્થિર શૂન્ય
urldb_ગણતરી_urls(અચલ સંરચના path_data *મૂળ,
		 time_t expiry,
		 unsigned પૂર્ણાંક *ગણતરી)
{
	અચલ સંરચના path_data *p = મૂળ;

	કરો {
		જો (p->children != ખાલી) {
			/* Drill down into children */
			p = p->children;
		} નહીંતર {
			/* No more children, increment count if required */
			જો (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*ગણતરી)++;
			}

			/* Now, find next node to process. */
			જ્યારે (p != મૂળ) {
				જો (p->next != ખાલી) {
					/* Have a sibling, process that */
					p = p->next;
					તોડો;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} જ્યારે (p != મૂળ);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
સ્થિર શૂન્ય urldb_save_શોધ_વૃક્ષ(સંરચના શોધ_ગાંઠ *parent, FILE *fp)
{
	અક્ષર host[256];
	અચલ સંરચના host_part *h;
	unsigned પૂર્ણાંક path_ગણતરી = 0;
	અક્ષર *path, *p, *end;
	પૂર્ણાંક path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	પૂર્ણાંક hsts_include_subdomains = 0;

	expiry = time(ખાલી) - ((60 * 60 * 24) * nsoption_int(expire_url));

	જો (parent == &empty)
		પાછા;

	urldb_save_શોધ_વૃક્ષ(parent->ડાબું, fp);

	path = malloc(path_alloc);
	જો (!path)
		પાછા;

	path[0] = '\0';

	માટે (h = parent->data, p = host, end = host + કદ host;
	     h && h != &db_મૂળ && p < end; h = h->parent) {
		પૂર્ણાંક written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		જો (written < 0) {
			મુક્ત(path);
			પાછા;
		}
		p += written;
	}

	h = parent->data;
	જો (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_ગણતરી_urls(&parent->data->paths, expiry, &path_ગણતરી);

	જો (path_ગણતરી > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_ગણતરી);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} નહીંતર જો (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	મુક્ત(path);

	urldb_save_શોધ_વૃક્ષ(parent->જમણું, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
સ્થિર bool
urldb_iterate_entries_path(અચલ સંરચના path_data *parent,
		bool (*url_callback)(nsurl *url, અચલ સંરચના url_data *data),
		bool (*cookie_callback)(અચલ સંરચના cookie_data *data))
{
	અચલ સંરચના path_data *p = parent;
	અચલ સંરચના cookie_data *c;

	કરો {
		જો (p->children != ખાલી) {
			/* Drill down into children */
			p = p->children;
		} નહીંતર {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			પ્રતિજ્ઞા(url_callback || cookie_callback);

			/** \todo handle fragments? */
			જો (url_callback) {
				અચલ સંરચના url_internal_data *u = &p->urld;

				પ્રતિજ્ઞા(p->url);

				જો (!url_callback(p->url,
						  (અચલ સંરચના url_data *) u))
					પાછા false;
			} નહીંતર {
				c = (અચલ સંરચના cookie_data *)p->cookies;
				માટે (; c != ખાલી; c = c->next) {
					જો (!cookie_callback(c))
						પાછા false;
				}
			}

			/* Now, find next node to process. */
			જ્યારે (p != parent) {
				જો (p->next != ખાલી) {
					/* Have a sibling, process that */
					p = p->next;
					તોડો;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} જ્યારે (p != parent);

	પાછા true;
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
સ્થિર bool urldb__host_is_ip_address(અચલ અક્ષર *host)
{
	સંરચના in_addr ipv4;
	size_t host_len = strlen(host);
	અચલ અક્ષર *sane_host;
	અચલ અક્ષર *slash;
#ifndef NO_IPV6
	સંરચના in6_addr ipv6;
	અક્ષર ipv6_addr[64];
	unsigned પૂર્ણાંક ipv6_addr_len;
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
	જો (slash == ખાલી) {
		sane_host = host;
	} નહીંતર {
		અક્ષર *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	જો (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		જાઓ out_false;

	જો (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		પૂર્ણાંક અંક_dots = 0;
		size_t index;

		માટે (index = 0; index < host_len; index++) {
			જો (sane_host[index] == '.')
				અંક_dots++;
		}

		જો (અંક_dots == 3)
			જાઓ out_true;
		નહીંતર
			જાઓ out_false;
	}

#ifndef NO_IPV6
	જો ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		જાઓ out_false;
	}

	ipv6_addr_len = host_len - 2;
	જો (ipv6_addr_len >= કદ(ipv6_addr)) {
		ipv6_addr_len = કદ(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	જો (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		જાઓ out_true;
#endif

out_false:
	જો (slash != ખાલી) મુક્ત((શૂન્ય *)sane_host);
	પાછા false;

out_true:
	જો (slash != ખાલી) મુક્ત((શૂન્ય *)sane_host);
	પાછા true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
સ્થિર પૂર્ણાંક urldb_શોધ_match_prefix(અચલ સંરચના host_part *a, અચલ અક્ષર *b)
{
	અચલ અક્ષર *end, *dot;
	પૂર્ણાંક plen, ret;

	પ્રતિજ્ઞા(a && a != &db_મૂળ && b);

	જો (urldb__host_is_ip_address(b)) {
		/* IP address */
		પાછા strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	જ્યારે (b < end && a && a != &db_મૂળ) {
		dot = strchr(b, '.');
		જો (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		જો ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			પાછા ret;

		/* The strings matched */
		જો (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			જો (plen > dot - b) {
				/* len(a) > len(b) */
				પાછા 1;
			} નહીંતર જો (plen < dot - b) {
				/* len(a) < len(b) */
				પાછા -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	જો (a && a != &db_મૂળ && b >= end) {
		/* len(a) > len(b) => prefix matches */
		પાછા 0;
	} નહીંતર જો ((!a || a == &db_મૂળ) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		પાછા -1;
	}

	/* Identical */
	પાછા 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
સ્થિર bool
urldb_iterate_partial_host(સંરચના શોધ_ગાંઠ *મૂળ,
		અચલ અક્ષર *prefix,
		bool (*callback)(nsurl *url, અચલ સંરચના url_data *data))
{
	પૂર્ણાંક c;

	પ્રતિજ્ઞા(મૂળ && prefix && callback);

	જો (મૂળ == &empty)
		પાછા true;

	c = urldb_શોધ_match_prefix(મૂળ->data, prefix);

	જો (c > 0) {
		/* No match => look in left subtree */
		પાછા urldb_iterate_partial_host(મૂળ->ડાબું,
						  prefix,
						  callback);
	} નહીંતર જો (c < 0) {
		/* No match => look in right subtree */
		પાછા urldb_iterate_partial_host(મૂળ->જમણું,
						  prefix,
						  callback);
	} નહીંતર {
		/* Match => iterate over l/r subtrees & process this node */
		જો (!urldb_iterate_partial_host(મૂળ->ડાબું,
						prefix,
						callback)) {
			પાછા false;
		}

		જો (મૂળ->data->paths.children) {
			/* and extract all paths attached to this host */
			જો (!urldb_iterate_entries_path(&મૂળ->data->paths,
							callback,
							ખાલી)) {
				પાછા false;
			}
		}

		જો (!urldb_iterate_partial_host(મૂળ->જમણું,
						prefix,
						callback)) {
			પાછા false;
		}
	}

	પાછા true;
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
સ્થિર bool
urldb_iterate_partial_path(અચલ સંરચના path_data *parent,
		અચલ અક્ષર *prefix,
		bool (*callback)(nsurl *url, અચલ સંરચના url_data *data))
{
	અચલ સંરચના path_data *p = parent->children;
	અચલ અક્ષર *slash, *end = prefix + strlen(prefix);

	કરો {
		slash = strchr(prefix, '/');
		જો (!slash) {
			slash = end;
		}

		જો (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			ચાલુ;
		}

		જો (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			જો (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				જો (!urldb_iterate_entries_path(p,
								callback,
								ખાલી)) {
					પાછા false;
				}

				/* Progress to next sibling */
				p = p->next;
			} નહીંતર {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} નહીંતર {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} જ્યારે (p != ખાલી);

	પાછા true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
સ્થિર bool
urldb_iterate_entries_host(સંરચના શોધ_ગાંઠ *parent,
		bool (*url_callback)(nsurl *url, અચલ સંરચના url_data *data),
		bool (*cookie_callback)(અચલ સંરચના cookie_data *data))
{
	જો (parent == &empty) {
		પાછા true;
	}

	જો (!urldb_iterate_entries_host(parent->ડાબું,
					url_callback,
					cookie_callback)) {
		પાછા false;
	}

	જો ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		જો (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			પાછા false;
		}
	}

	જો (!urldb_iterate_entries_host(parent->જમણું,
					url_callback,
					cookie_callback)) {
		પાછા false;
	}

	પાછા true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
સ્થિર સંરચના host_part *
urldb_add_host_ગાંઠ(અચલ અક્ષર *part, સંરચના host_part *parent)
{
	સંરચના host_part *d;

	પ્રતિજ્ઞા(part && parent);

	d = calloc(1, કદ(સંરચના host_part));
	જો (!d) {
		પાછા ખાલી;
	}

	d->part = strdup(part);
	જો (!d->part) {
		મુક્ત(d);
		પાછા ખાલી;
	}

	d->next = parent->children;
	જો (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	પાછા d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
સ્થિર પૂર્ણાંક urldb_add_path_fragment_cmp(અચલ શૂન્ય *a, અચલ શૂન્ય *b)
{
	પાછા strcasecmp(*((અચલ અક્ષર **) a), *((અચલ અક્ષર **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
સ્થિર સંરચના path_data *
urldb_add_path_fragment(સંરચના path_data *segment, lwc_string *fragment)
{
	અક્ષર **temp;

	પ્રતિજ્ઞા(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	જો (!fragment)
		પાછા segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * કદ(અક્ષર *));
	જો (!temp)
		પાછા ખાલી;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	જો (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		પાછા ખાલી;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      કદ (અક્ષર *),
	      urldb_add_path_fragment_cmp);

	પાછા segment;
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
સ્થિર સંરચના path_data *
urldb_add_path_ગાંઠ(lwc_string *scheme,
		    unsigned પૂર્ણાંક port,
		    અચલ અક્ષર *segment,
		    lwc_string *fragment,
		    સંરચના path_data *parent)
{
	સંરચના path_data *d, *e;

	પ્રતિજ્ઞા(scheme && segment && parent);

	d = calloc(1, કદ(સંરચના path_data));
	જો (!d)
		પાછા ખાલી;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	જો (!d->segment) {
		lwc_string_unref(d->scheme);
		મુક્ત(d);
		પાછા ખાલી;
	}

	જો (fragment) {
		જો (!urldb_add_path_fragment(d, fragment)) {
			મુક્ત(d->segment);
			lwc_string_unref(d->scheme);
			મુક્ત(d);
			પાછા ખાલી;
		}
	}

	માટે (e = parent->children; e; e = e->next) {
		જો (strcmp(e->segment, d->segment) > 0)
			તોડો;
	}

	જો (e) {
		d->prev = e->prev;
		d->next = e;
		જો (e->prev)
			e->prev->next = d;
		નહીંતર
			parent->children = d;
		e->prev = d;
	} નહીંતર જો (!parent->children) {
		d->prev = d->next = ખાલી;
		parent->children = parent->last = d;
	} નહીંતર {
		d->next = ખાલી;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	પાછા d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
સ્થિર સંરચના શોધ_ગાંઠ **urldb_get_શોધ_વૃક્ષ_direct(અચલ અક્ષર *host)
{
	પ્રતિજ્ઞા(host);

	જો (urldb__host_is_ip_address(host)) {
		પાછા &શોધ_trees[ST_IP];
	} નહીંતર જો (ascii_is_alpha(*host)) {
		પાછા &શોધ_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	પાછા &શોધ_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
સ્થિર સંરચના શોધ_ગાંઠ *urldb_get_શોધ_વૃક્ષ(અચલ અક્ષર *host)
{
	પાછા *urldb_get_શોધ_વૃક્ષ_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
સ્થિર પૂર્ણાંક urldb_શોધ_match_string(અચલ સંરચના host_part *a, અચલ અક્ષર *b)
{
	અચલ અક્ષર *end, *dot;
	પૂર્ણાંક plen, ret;

	પ્રતિજ્ઞા(a && a != &db_મૂળ && b);

	જો (urldb__host_is_ip_address(b)) {
		/* IP address */
		પાછા strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	જ્યારે (b < end && a && a != &db_મૂળ) {
		dot = strchr(b, '.');
		જો (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		જો ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			પાછા ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		જો (plen > dot - b) {
			/* len(a) > len(b) */
			પાછા 1;
		} નહીંતર જો (plen < dot - b) {
			/* len(a) < len(b) */
			પાછા -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	જો (a && a != &db_મૂળ && b >= end) {
		/* len(a) > len(b) */
		પાછા 1;
	} નહીંતર જો ((!a || a == &db_મૂળ) && b < end) {
		/* len(a) < len(b) */
		પાછા -1;
	}

	/* Identical */
	પાછા 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
સ્થિર અચલ સંરચના host_part *
urldb_શોધ_find(સંરચના શોધ_ગાંઠ *મૂળ, અચલ અક્ષર *host)
{
	પૂર્ણાંક c;

	પ્રતિજ્ઞા(મૂળ && host);

	જો (મૂળ == &empty) {
		પાછા ખાલી;
	}

	c = urldb_શોધ_match_string(મૂળ->data, host);

	જો (c > 0) {
		પાછા urldb_શોધ_find(મૂળ->ડાબું, host);
	} નહીંતર જો (c < 0) {
		પાછા urldb_શોધ_find(મૂળ->જમણું, host);
	}

	પાછા મૂળ->data;
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
સ્થિર સંરચના path_data *
urldb_match_path(અચલ સંરચના path_data *parent,
		 અચલ અક્ષર *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	અચલ સંરચના path_data *p;
	અચલ અક્ષર *slash;
	bool match;

	પ્રતિજ્ઞા(parent != ખાલી);
	પ્રતિજ્ઞા(parent->segment == ખાલી);

	જો (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	પ્રતિજ્ઞા(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	જ્યારે (p != ખાલી) {
		slash = strchr(path + 1, '/');
		જો (!slash) {
			slash = path + strlen(path);
		}

		જો (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			જો (*slash == '\0') {
				/* Complete match */
				પાછા (સંરચના path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} નહીંતર {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	પાછા ખાલી;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
સ્થિર સંરચના path_data *urldb_find_url(nsurl *url)
{
	અચલ સંરચના host_part *h;
	સંરચના path_data *p;
	સંરચના શોધ_ગાંઠ *વૃક્ષ;
	અક્ષર *plq;
	અચલ અક્ષર *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned પૂર્ણાંક port_int;
	bool match;

	પ્રતિજ્ઞા(url);

	જો (url_bloom != ખાલી) {
		જો (bloom_શોધ_hash(url_bloom, nsurl_hash(url)) == false) {
			પાછા ખાલી;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	જો (scheme == ખાલી)
		પાછા ખાલી;

	જો (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		પાછા ખાલી;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	જો (host != ખાલી) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} નહીંતર જો (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} નહીંતર {
		lwc_string_unref(scheme);
		પાછા ખાલી;
	}

	વૃક્ષ = urldb_get_શોધ_વૃક્ષ(host_str);
	h = urldb_શોધ_find(વૃક્ષ, host_str);
	જો (!h) {
		lwc_string_unref(scheme);
		પાછા ખાલી;
	}

	/* generate plq (path, leaf, query) */
	જો (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		પાછા ખાલી;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	જો (port != ખાલી) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} નહીંતર {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	મુક્ત(plq);
	lwc_string_unref(scheme);

	પાછા p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
સ્થિર શૂન્ય urldb_dump_paths(સંરચના path_data *parent)
{
	અચલ સંરચના path_data *p = parent;
	unsigned પૂર્ણાંક i;

	કરો {
		જો (p->segment != ખાલી) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			માટે (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		જો (p->children != ખાલી) {
			p = p->children;
		} નહીંતર {
			જ્યારે (p != parent) {
				જો (p->next != ખાલી) {
					p = p->next;
					તોડો;
				}

				p = p->parent;
			}
		}
	} જ્યારે (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
સ્થિર શૂન્ય urldb_dump_hosts(સંરચના host_part *parent)
{
	સંરચના host_part *h;

	જો (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	માટે (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
સ્થિર શૂન્ય urldb_dump_શોધ(સંરચના શોધ_ગાંઠ *parent, પૂર્ણાંક depth)
{
	અચલ સંરચના host_part *h;
	પૂર્ણાંક i; /* index into string */
	અક્ષર s[1024];
	પૂર્ણાંક r;
	પૂર્ણાંક sl = કદ(s) - 2;

	જો (parent == &empty)
		પાછા;

	urldb_dump_શોધ(parent->ડાબું, depth + 1);

	માટે (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	માટે (h = parent->data; h; h = h->parent) {
		જો (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			જો (r < 0) {
				તોડો;
			}
			જો ((i + r) >= sl) {
				તોડો;
			}
			i += r;
		}

		જો (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_શોધ(parent->જમણું, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
સ્થિર પૂર્ણાંક
urldb_શોધ_match_host(અચલ સંરચના host_part *a, અચલ સંરચના host_part *b)
{
	પૂર્ણાંક ret;

	પ્રતિજ્ઞા(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	માટે (; a && a != &db_મૂળ && b && b != &db_મૂળ;
	     a = a->parent, b = b->parent) {
		જો ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			પાછા ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	જો (a && a != &db_મૂળ && (!b || b == &db_મૂળ)) {
		/* len(a) > len(b) */
		પાછા 1;
	} નહીંતર જો ((!a || a == &db_મૂળ) && b && b != &db_મૂળ) {
		/* len(a) < len(b) */
		પાછા -1;
	}

	/* identical */
	પાછા 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
સ્થિર સંરચના શોધ_ગાંઠ *urldb_શોધ_skew(સંરચના શોધ_ગાંઠ *મૂળ)
{
	પ્રતિજ્ઞા(મૂળ);

	જો (મૂળ->ડાબું->level == મૂળ->level) {
		સંરચના શોધ_ગાંઠ *temp;

		temp = મૂળ->ડાબું;
		મૂળ->ડાબું = temp->જમણું;
		temp->જમણું = મૂળ;
		મૂળ = temp;
	}

	પાછા મૂળ;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
સ્થિર સંરચના શોધ_ગાંઠ *urldb_શોધ_split(સંરચના શોધ_ગાંઠ *મૂળ)
{
	પ્રતિજ્ઞા(મૂળ);

	જો (મૂળ->જમણું->જમણું->level == મૂળ->level) {
		સંરચના શોધ_ગાંઠ *temp;

		temp = મૂળ->જમણું;
		મૂળ->જમણું = temp->ડાબું;
		temp->ડાબું = મૂળ;
		મૂળ = temp;

		મૂળ->level++;
	}

	પાછા મૂળ;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
સ્થિર સંરચના શોધ_ગાંઠ *
urldb_શોધ_ઉમેરો_internal(સંરચના શોધ_ગાંઠ *મૂળ, સંરચના શોધ_ગાંઠ *n)
{
	પ્રતિજ્ઞા(મૂળ && n);

	જો (મૂળ == &empty) {
		મૂળ = n;
	} નહીંતર {
		પૂર્ણાંક c = urldb_શોધ_match_host(મૂળ->data, n->data);

		જો (c > 0) {
			મૂળ->ડાબું = urldb_શોધ_ઉમેરો_internal(
				મૂળ->ડાબું, n);
		} નહીંતર જો (c < 0) {
			મૂળ->જમણું = urldb_શોધ_ઉમેરો_internal(
				મૂળ->જમણું, n);
		} નહીંતર {
			/* exact match */
			મુક્ત(n);
			પાછા મૂળ;
		}

		મૂળ = urldb_શોધ_skew(મૂળ);
		મૂળ = urldb_શોધ_split(મૂળ);
	}

	પાછા મૂળ;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
સ્થિર સંરચના શોધ_ગાંઠ *
urldb_શોધ_ઉમેરો(સંરચના શોધ_ગાંઠ *મૂળ, અચલ સંરચના host_part *data)
{
	સંરચના શોધ_ગાંઠ *n;

	પ્રતિજ્ઞા(મૂળ && data);

	n = malloc(કદ(સંરચના શોધ_ગાંઠ));
	જો (!n)
		પાછા ખાલી;

	n->level = 1;
	n->data = data;
	n->ડાબું = n->જમણું = &empty;

	મૂળ = urldb_શોધ_ઉમેરો_internal(મૂળ, n);

	પાછા મૂળ;
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
સ્થિર bool
urldb_parse_avpair(સંરચના cookie_internal_data *c,
		   અક્ષર *n,
		   અક્ષર *v,
		   bool was_quoted)
{
	પૂર્ણાંક vlen;

	પ્રતિજ્ઞા(c && n && v);

	/* Strip whitespace from start of name */
	માટે (; *n; n++) {
		જો (*n != ' ' && *n != '\t')
			તોડો;
	}

	/* Strip whitespace from end of name */
	માટે (vlen = strlen(n); vlen; vlen--) {
		જો (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		નહીંતર
			તોડો;
	}

	/* Strip whitespace from start of value */
	માટે (; *v; v++) {
		જો (*v != ' ' && *v != '\t')
			તોડો;
	}

	/* Strip whitespace from end of value */
	માટે (vlen = strlen(v); vlen; vlen--) {
		જો (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		નહીંતર
			તોડો;
	}

	જો (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		જો (!c->comment)
			પાછા false;
	} નહીંતર જો (!c->domain && strcasecmp(n, "Domain") == 0) {
		જો (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			જો (!c->domain)
				પાછા false;
		}
	} નહીંતર જો (strcasecmp(n, "Max-Age") == 0) {
		પૂર્ણાંક temp = atoi(v);
		જો (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		નહીંતર
			c->expires = time(ખાલી) + temp;
	} નહીંતર જો (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		જો (!c->path)
			પાછા false;
	} નહીંતર જો (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} નહીંતર જો (strcasecmp(n, "Expires") == 0) {
		અક્ષર *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		માટે (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		જો (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} નહીંતર જો (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} નહીંતર જો (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} નહીંતર જો (!c->name) {
		c->name = strdup(n);
		c->કિંમત = strdup(v);
		c->કિંમત_was_quoted = was_quoted;
		જો (!c->name || !c->કિંમત) {
			પાછા false;
		}
	}

	પાછા true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
સ્થિર શૂન્ય urldb_free_cookie(સંરચના cookie_internal_data *c)
{
	પ્રતિજ્ઞા(c);

	મુક્ત(c->comment);
	મુક્ત(c->domain);
	મુક્ત(c->path);
	મુક્ત(c->name);
	મુક્ત(c->કિંમત);
	મુક્ત(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
સ્થિર સંરચના cookie_internal_data *
urldb_parse_cookie(nsurl *url, અચલ અક્ષર **cookie)
{
	સંરચના cookie_internal_data *c;
	અચલ અક્ષર *cur;
	અક્ષર name[1024], કિંમત[4096];
	અક્ષર *n = name, *v = કિંમત;
	bool in_કિંમત = false;
	bool had_કિંમત_data = false;
	bool કિંમત_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	પ્રતિજ્ઞા(url && cookie && *cookie);

	c = calloc(1, કદ(સંરચના cookie_internal_data));
	જો (c == ખાલી)
		પાછા ખાલી;

	c->expires = -1;

	name[0] = '\0';
	કિંમત[0] = '\0';

	માટે (cur = *cookie; *cur; cur++) {
		જો (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			જો (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				કિંમત[0] = '\0';

			}

			તોડો;
		} નહીંતર જો (*cur == '\r') {
			/* Spurious linefeed */
			ચાલુ;
		} નહીંતર જો (*cur == '\n') {
			/* Spurious newline */
			ચાલુ;
		}

		જો (in_કિંમત && !had_કિંમત_data) {
			જો (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ચાલુ;
			} નહીંતર {
				had_કિંમત_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				જો (*cur != '"') {
					કિંમત_verbatim = true;
				}
			}
		}

		જો (in_કિંમત && !કિંમત_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			જો (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ચાલુ;
			}
		}

		જો (!quoted && !in_કિંમત && *cur == '=') {
			/* First equals => attr-value separator */
			in_કિંમત = true;
			ચાલુ;
		}

		જો (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			જો (!urldb_parse_avpair(c, name, કિંમત, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				પાછા ખાલી;
			}

			/* And reset to start */
			n = name;
			v = કિંમત;
			in_કિંમત = false;
			had_કિંમત_data = false;
			કિંમત_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			જો (*cur != ';') {
				cur--;
			}

			ચાલુ;
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
		જો (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			અચલ અક્ષર *p;
			અચલ અક્ષર *semi = strchr(cur + 1, ';');
			જો (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			માટે (p = cur + 1; p < semi; p++)
				જો (*p == '=')
					તોડો;

			જો (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} નહીંતર {
				/* found one => comma marks end of cookie */
				cur++;
				તોડો;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		જો (!in_કિંમત) {
			જો (n < name + (કદ(name) - 1))
				*n++ = *cur;
		} નહીંતર {
			જો (v < કિંમત + (કદ(કિંમત) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	જો (!urldb_parse_avpair(c, name, કિંમત, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		પાછા ખાલી;
	}

	/* Now fix-up default values */
	જો (c->domain == ખાલી) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		જો (host == ખાલી) {
			urldb_free_cookie(c);
			પાછા ખાલી;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	જો (c->path == ખાલી) {
		અચલ અક્ષર *path_data;
		અક્ષર *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		જો (path_lwc == ખાલી) {
			urldb_free_cookie(c);
			પાછા ખાલી;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		જો (slash != ખાલી) {
			/* Special case: retain first slash in path */
			જો (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			જો (slash == ખાલી) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				પાછા ખાલી;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} નહીંતર {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			જો (path == ખાલી) {
				urldb_free_cookie(c);
				પાછા ખાલી;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	પાછા c;
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
સ્થિર સંરચના path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned પૂર્ણાંક port,
	       અચલ સંરચના host_part *host,
	       અક્ષર *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	સંરચના path_data *d, *e;
	અક્ષર *buf = path_query;
	અક્ષર *segment, *slash;
	bool match;

	પ્રતિજ્ઞા(scheme && host && url);

	d = (સંરચના path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	જો (*segment == '/')
		segment++;

	/* Process path segments */
	કરો {
		slash = strchr(segment, '/');
		જો (!slash) {
			/* last segment */
			/* look for existing entry */
			માટે (e = d->children; e; e = e->next)
				જો (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					તોડો;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_ગાંઠ(scheme, port,
						    segment, fragment, d);
			તોડો;
		}

		*slash = '\0';

		/* look for existing entry */
		માટે (e = d->children; e; e = e->next)
			જો (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				તોડો;

		d = e ? e : urldb_add_path_ગાંઠ(scheme, port, segment, ખાલી, d);
		જો (!d)
			તોડો;

		segment = slash + 1;
	} જ્યારે (1);

	મુક્ત(path_query);

	જો (d && !d->url) {
		/* Insert defragmented URL */
		જો (nsurl_defragment(url, &d->url) != NSERROR_OK)
			પાછા ખાલી;
	}

	પાછા d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
સ્થિર સંરચના host_part *urldb_add_host(અચલ અક્ષર *host)
{
	સંરચના host_part *d = (સંરચના host_part *) &db_મૂળ, *e;
	સંરચના શોધ_ગાંઠ *s;
	અક્ષર buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	અક્ષર *part;

	પ્રતિજ્ઞા(host);

	જો (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		માટે (e = d->children; e; e = e->next)
			જો (strcasecmp(host, e->part) == 0)
				/* found => return it */
				પાછા e;

		d = urldb_add_host_ગાંઠ(host, d);

		s = urldb_શોધ_ઉમેરો(શોધ_trees[ST_IP], d);
		જો (!s) {
			/* failed */
			d = ખાલી;
		} નહીંતર {
			શોધ_trees[ST_IP] = s;
		}

		પાછા d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, કદ buf);
	buf[કદ buf - 1] = '\0';

	/* Process FQDN segments backwards */
	કરો {
		part = strrchr(buf, '.');
		જો (!part) {
			/* last segment */
			/* Check for existing entry */
			માટે (e = d->children; e; e = e->next)
				જો (strcasecmp(buf, e->part) == 0)
					તોડો;

			જો (e) {
				d = e;
			} નહીંતર {
				d = urldb_add_host_ગાંઠ(buf, d);
			}

			/* And insert into search tree */
			જો (d) {
				સંરચના શોધ_ગાંઠ **r;

				r = urldb_get_શોધ_વૃક્ષ_direct(buf);
				s = urldb_શોધ_ઉમેરો(*r, d);
				જો (!s) {
					/* failed */
					d = ખાલી;
				} નહીંતર {
					*r = s;
				}
			}
			તોડો;
		}

		/* Check for existing entry */
		માટે (e = d->children; e; e = e->next)
			જો (strcasecmp(part + 1, e->part) == 0)
				તોડો;

		d = e ? e : urldb_add_host_ગાંઠ(part + 1, d);
		જો (!d)
			તોડો;

		*part = '\0';
	} જ્યારે (1);

	પાછા d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
સ્થિર bool
urldb_ઉમેરો_cookie(સંરચના cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	સંરચના cookie_internal_data *d;
	અચલ સંરચના host_part *h;
	સંરચના path_data *p;
	time_t now = time(ખાલી);

	પ્રતિજ્ઞા(c);

	જો (c->domain[0] == '.') {
		h = urldb_શોધ_find(
			urldb_get_શોધ_વૃક્ષ(&(c->domain[1])),
			c->domain + 1);
		જો (!h) {
			h = urldb_add_host(c->domain + 1);
			જો (!h) {
				urldb_free_cookie(c);
				પાછા false;
			}
		}

		p = (સંરચના path_data *) &h->paths;
	} નહીંતર {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		પ્રતિજ્ઞા(url != ખાલી);
		પ્રતિજ્ઞા(scheme != ખાલી);

		h = urldb_શોધ_find(
			urldb_get_શોધ_વૃક્ષ(c->domain),
			c->domain);

		જો (!h) {
			h = urldb_add_host(c->domain);
			જો (!h) {
				urldb_free_cookie(c);
				પાછા false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), ખાલી, url);
		જો (!p) {
			urldb_free_cookie(c);
			પાછા false;
		}
	}

	/* add cookie */
	માટે (d = p->cookies; d; d = d->next) {
		જો (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			તોડો;
	}

	જો (d) {
		જો (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			જો (d->next)
				d->next->prev = d->prev;
			નહીંતર
				p->cookies_end = d->prev;
			જો (d->prev)
				d->prev->next = d->next;
			નહીંતર
				p->cookies = d->next;

			cookie_manager_remove((સંરચના cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} નહીંતર {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			જો (c->next)
				c->next->prev = c;
			નહીંતર
				p->cookies_end = c;
			જો (c->prev)
				c->prev->next = c;
			નહીંતર
				p->cookies = c;

			cookie_manager_remove((સંરચના cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((સંરચના cookie_data *)c);
		}
	} નહીંતર {
		c->prev = p->cookies_end;
		c->next = ખાલી;
		જો (p->cookies_end)
			p->cookies_end->next = c;
		નહીંતર
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((સંરચના cookie_data *)c);
	}

	પાછા true;
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
સ્થિર bool
concat_netscape_cookie(સંરચના cookie_internal_data *c,
		    પૂર્ણાંક *used,
		    પૂર્ણાંક *alloc,
		    અક્ષર **buf)
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
	પૂર્ણાંક max_len;
	પૂર્ણાંક outl; /* output length */

	પ્રતિજ્ઞા(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->કિંમત) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	જો (max_len >= *alloc) {
		અક્ષર *temp = realloc(*buf, max_len);
		જો (!temp) {
			પાછા false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	જો (outl >= (*alloc - *used)) {
		/* output truncated */
		પાછા false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	જો (c->કિંમત_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->કિંમત);
		જો (outl >= (*alloc - *used)) {
			/* output truncated */
			પાછા false;
		}
		*used += outl;
	} નહીંતર {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->કિંમત);
		જો (outl >= (*alloc - *used)) {
			/* output truncated */
			પાછા false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	પાછા true;
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
સ્થિર bool
concat_rfc2965_cookie(સંરચના cookie_internal_data *c,
		      પૂર્ણાંક *used,
		      પૂર્ણાંક *alloc,
		      અક્ષર **buf)
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
	અચલ અક્ષર * અચલ separators = "()<>@,;:\\\"/[]?={} \t";
	પૂર્ણાંક max_len;
	પૂર્ણાંક outl; /* output length */

	પ્રતિજ્ઞા(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->કિંમત) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	જો (max_len >= *alloc) {
		અક્ષર *temp = realloc(*buf, max_len);
		જો (!temp) {
			પાછા false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	જો (outl >= (*alloc - *used)) {
		/* output truncated */
		પાછા false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	જો (c->કિંમત_was_quoted ||
	    strpbrk(c->કિંમત, separators) != ખાલી) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->કિંમત);

	} નહીંતર {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->કિંમત);
	}
	જો (outl >= (*alloc - *used)) {
		/* output truncated */
		પાછા false;
	}
	*used += outl;

	જો (c->path_from_set) {
		/* Path, quoted if necessary */

		જો (strpbrk(c->path, separators) != ખાલી) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} નહીંતર {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		જો (outl >= (*alloc - *used)) {
			/* output truncated */
			પાછા false;
		}
		*used += outl;
	}

	જો (c->domain_from_set) {
		/* Domain, quoted if necessary */
		જો (strpbrk(c->domain, separators) != ખાલી) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} નહીંતર {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		જો (outl >= (*alloc - *used)) {
			/* output truncated */
			પાછા false;
		}
		*used += outl;
	}

	પાછા true;
}


/**
 * build output cookie string in netscape format
 */
સ્થિર અક્ષર *
get_netscape_cookie(સંરચના cookie_internal_data **matched_cookies, પૂર્ણાંક ગણતરી)
{
	પૂર્ણાંક ret_alloc = 4096, ret_used = 1;
	અક્ષર *ret;
	પૂર્ણાંક midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	જો (ret == ખાલી) {
		પાછા ખાલી;
	}
	ret[0] = '\0';

	માટે (midx = 0; midx < ગણતરી; midx++) {
		જો (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			મુક્ત(ret);
			પાછા ખાલી;
		}
	}

	જો (ret_used <= 2) {
		મુક્ત(ret);
		પાછા ખાલી;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	અક્ષર *temp = realloc(ret, ret_used);
	જો (temp != ખાલી) {
		ret = temp;
	}

	પાછા ret;
}


/**
 * build output cookie string in RFC2965 format
 */
સ્થિર અક્ષર *
get_rfc2965_cookie(સંરચના cookie_internal_data **matched_cookies,
		   પૂર્ણાંક ગણતરી,
		   પૂર્ણાંક version)
{
	પૂર્ણાંક ret_alloc = 4096; /* return string allocation */
	પૂર્ણાંક ret_used = 1; /* return sting buffer used */
	અક્ષર *ret; /* return string buffer */
	પૂર્ણાંક midx; /* index into matched cookies */
	પૂર્ણાંક outl;

	ret = malloc(ret_alloc);
	જો (ret == ખાલી) {
		પાછા ખાલી;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	જો (outl >= ret_alloc - ret_used) {
		/* overflow */
		મુક્ત(ret);
		પાછા ખાલી;
	}
	ret_used += outl;

	માટે (midx = 0; midx < ગણતરી; midx++) {
		જો (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			મુક્ત(ret);
			પાછા ખાલી;
		}
	}

	/* shrink the output buffer to the required size */
	અક્ષર *temp = realloc(ret, ret_used);
	જો (temp != ખાલી) {
		ret = temp;
	}

	પાછા ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
સ્થિર શૂન્ય
urldb_delete_cookie_paths(અચલ અક્ષર *domain,
			  અચલ અક્ષર *path,
			  અચલ અક્ષર *name,
			  સંરચના path_data *parent)
{
	સંરચના cookie_internal_data *c;
	સંરચના path_data *p = parent;

	પ્રતિજ્ઞા(parent);

	કરો {
		માટે (c = p->cookies; c; c = c->next) {
			જો (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				જો (c->prev) {
					c->prev->next = c->next;
				} નહીંતર {
					p->cookies = c->next;
				}

				જો (c->next) {
					c->next->prev = c->prev;
				} નહીંતર {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				પાછા;
			}
		}

		જો (p->children) {
			p = p->children;
		} નહીંતર {
			જ્યારે (p != parent) {
				જો (p->next != ખાલી) {
					p = p->next;
					તોડો;
				}

				p = p->parent;
			}
		}
	} જ્યારે (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
સ્થિર શૂન્ય
urldb_delete_cookie_hosts(અચલ અક્ષર *domain,
			  અચલ અક્ષર *path,
			  અચલ અક્ષર *name,
			  સંરચના host_part *parent)
{
	સંરચના host_part *h;
	પ્રતિજ્ઞા(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	માટે (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
સ્થિર શૂન્ય urldb_save_cookie_paths(FILE *fp, સંરચના path_data *parent)
{
	સંરચના path_data *p = parent;
	time_t now = time(ખાલી);

	પ્રતિજ્ઞા(fp && parent);

	કરો {
		જો (p->cookies != ખાલી) {
			સંરચના cookie_internal_data *c;

			માટે (c = p->cookies; c != ખાલી; c = c->next) {
				જો (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ચાલુ;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(પૂર્ણાંક)c->expires, (પૂર્ણાંક)c->last_used,
					c->no_destroy, c->name, c->કિંમત,
					c->કિંમત_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		જો (p->children != ખાલી) {
			p = p->children;
		} નહીંતર {
			જ્યારે (p != parent) {
				જો (p->next != ખાલી) {
					p = p->next;
					તોડો;
				}

				p = p->parent;
			}
		}
	} જ્યારે (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
સ્થિર શૂન્ય urldb_save_cookie_hosts(FILE *fp, સંરચના host_part *parent)
{
	સંરચના host_part *h;
	પ્રતિજ્ઞા(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	માટે (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
સ્થિર શૂન્ય urldb_destroy_cookie(સંરચના cookie_internal_data *c)
{
	મુક્ત(c->name);
	મુક્ત(c->કિંમત);
	મુક્ત(c->comment);
	મુક્ત(c->domain);
	મુક્ત(c->path);

	મુક્ત(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
સ્થિર શૂન્ય urldb_destroy_path_ગાંઠ_content(સંરચના path_data *ગાંઠ)
{
	સંરચના cookie_internal_data *a, *b;
	unsigned પૂર્ણાંક i;

	જો (ગાંઠ->url != ખાલી) {
		nsurl_unref(ગાંઠ->url);
	}

	lwc_string_unref(ગાંઠ->scheme);

	મુક્ત(ગાંઠ->segment);
	માટે (i = 0; i < ગાંઠ->frag_cnt; i++)
		મુક્ત(ગાંઠ->fragment[i]);
	મુક્ત(ગાંઠ->fragment);

	મુક્ત(ગાંઠ->urld.title);

	માટે (a = ગાંઠ->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
સ્થિર શૂન્ય urldb_destroy_prot_space(સંરચના prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	મુક્ત(space->realm);
	મુક્ત(space->auth);

	મુક્ત(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
સ્થિર શૂન્ય urldb_destroy_path_વૃક્ષ(સંરચના path_data *મૂળ)
{
	સંરચના path_data *p = મૂળ;

	કરો {
		જો (p->children != ખાલી) {
			p = p->children;
		} નહીંતર {
			સંરચના path_data *q = p;

			જ્યારે (p != મૂળ) {
				જો (p->next != ખાલી) {
					p = p->next;
					તોડો;
				}

				p = p->parent;

				urldb_destroy_path_ગાંઠ_content(q);
				મુક્ત(q);

				q = p;
			}

			urldb_destroy_path_ગાંઠ_content(q);
			મુક્ત(q);
		}
	} જ્યારે (p != મૂળ);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
સ્થિર શૂન્ય urldb_destroy_host_વૃક્ષ(સંરચના host_part *મૂળ)
{
	સંરચના host_part *a, *b;
	સંરચના path_data *p, *q;
	સંરચના prot_space_data *s, *t;

	/* Destroy children */
	માટે (a = મૂળ->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_વૃક્ષ(a);
	}

	/* Now clean up paths */
	માટે (p = મૂળ->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_વૃક્ષ(p);
	}

	/* Root path */
	urldb_destroy_path_ગાંઠ_content(&મૂળ->paths);

	/* Proctection space data */
	માટે (s = મૂળ->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	મુક્ત(મૂળ->part);
	મુક્ત(મૂળ);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
સ્થિર શૂન્ય urldb_destroy_શોધ_વૃક્ષ(સંરચના શોધ_ગાંઠ *મૂળ)
{
	/* Destroy children */
	જો (મૂળ->ડાબું != &empty)
		urldb_destroy_શોધ_વૃક્ષ(મૂળ->ડાબું);
	જો (મૂળ->જમણું != &empty)
		urldb_destroy_શોધ_વૃક્ષ(મૂળ->જમણું);

	/* And destroy ourselves */
	મુક્ત(મૂળ);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
શૂન્ય urldb_destroy(શૂન્ય)
{
	સંરચના host_part *a, *b;
	પૂર્ણાંક i;

	/* Clean up search trees */
	માટે (i = 0; i < અંક_શોધ_TREES; i++) {
		જો (શોધ_trees[i] != &empty) {
			urldb_destroy_શોધ_વૃક્ષ(શોધ_trees[i]);
			શોધ_trees[i] = &empty;
		}
	}

	/* And database */
	માટે (a = db_મૂળ.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_વૃક્ષ(a);
	}
	memset(&db_મૂળ, 0, કદ(db_મૂળ));

	/* And the bloom filter */
	જો (url_bloom != ખાલી) {
		bloom_destroy(url_bloom);
		url_bloom = ખાલી;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(અચલ અક્ષર *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	અક્ષર s[MAXIMUM_URL_LENGTH];
	અક્ષર host[256];
	સંરચના host_part *h;
	પૂર્ણાંક urls;
	પૂર્ણાંક i;
	પૂર્ણાંક version;
	પૂર્ણાંક length;
	FILE *fp;

	પ્રતિજ્ઞા(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	જો (url_bloom == ખાલી)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	જો (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		પાછા NSERROR_NOT_FOUND;
	}

	જો (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		પાછા NSERROR_NEED_DATA;
	}

	version = atoi(s);
	જો (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		પાછા NSERROR_INVALID;
	}
	જો (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		પાછા NSERROR_INVALID;
	}

	જ્યારે (fgets(host, કદ host, fp)) {
		time_t hsts_expiry = 0;
		પૂર્ણાંક hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		જો (length == 0) {
			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;
			urls = atoi(s);
			/* Eight fields/url */
			માટે (i = 0; i < (8 * urls); i++) {
				જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					તોડો;
			}
			ચાલુ;
		}

		જો (version >= 107) {
			અક્ષર *p = host;
			જ્યારે (*p && *p != ' ') p++;
			જ્યારે (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			જ્યારે (*p && *p != ' ') p++;
			જ્યારે (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		જો (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			પાછા NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			તોડો;
		urls = atoi(s);

		/* no URLs => try next host */
		જો (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ચાલુ;
		}

		/* load the non-corrupt data */
		માટે (i = 0; i < urls; i++) {
			સંરચના path_data *p = ખાલી;
			અક્ષર scheme[64], ports[10];
			અક્ષર url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned પૂર્ણાંક port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			અક્ષર *path_query;
			size_t len;

			જો (!fgets(scheme, કદ scheme, fp))
				તોડો;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			જો (!fgets(ports, કદ ports, fp))
				તોડો;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;
			length = strlen(s) - 1;
			s[length] = '\0';

			જો (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, કદ url, "%s://%s%s%s%s",
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
			જો (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				પાછા NSERROR_NOMEM;
			}

			જો (url_bloom != ખાલી) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ઉમેરો_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			જો (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				પાછા NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			જો (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				પાછા NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;
			જો (p)
				p->urld.visits = (unsigned પૂર્ણાંક)atoi(s);

			/* entry last use time */
			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				તોડો;
			}
			જો (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;
			જો (p)
				p->urld.type = (content_type)atoi(s);

			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;


			જો (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				તોડો;
			length = strlen(s) - 1;
			જો (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				જો (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	પાછા NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(અચલ અક્ષર *filename)
{
	FILE *fp;
	પૂર્ણાંક i;

	પ્રતિજ્ઞા(filename);

	fp = fopen(filename, "w");
	જો (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		પાછા NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	માટે (i = 0; i != અંક_શોધ_TREES; i++) {
		urldb_save_શોધ_વૃક્ષ(શોધ_trees[i], fp);
	}

	fclose(fp);

	પાછા NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	સંરચના path_data *p;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p) {
		પાછા NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	સંરચના host_part *h;
	સંરચના path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	અચલ અક્ષર *host_str;
	અક્ષર *path_query = ખાલી;
	size_t len;
	bool match;
	unsigned પૂર્ણાંક port_int;

	પ્રતિજ્ઞા(url);

	જો (url_bloom == ખાલી)
		url_bloom = bloom_create(BLOOM_SIZE);

	જો (url_bloom != ખાલી) {
		uint32_t hash = nsurl_hash(url);
		bloom_ઉમેરો_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	જો (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		પાછા false;
	}
	પ્રતિજ્ઞા(path_query != ખાલી);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	જો (scheme == ખાલી) {
		મુક્ત(path_query);
		પાછા false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	જો (host != ખાલી) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} નહીંતર જો (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} નહીંતર {
		lwc_string_unref(scheme);
		મુક્ત(path_query);
		પાછા false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	જો (port != ખાલી) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} નહીંતર {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	જો (h != ખાલી) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} નહીંતર {
		p = ખાલી;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	પાછા (p != ખાલી);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, અચલ અક્ષર *title)
{
	સંરચના path_data *p;
	અક્ષર *temp;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (p == ખાલી) {
		પાછા NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	જો (title != ખાલી) {
		temp = strdup(title);
		જો (temp == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
	} નહીંતર {
		temp = ખાલી;
	}

	મુક્ત(p->urld.title);
	p->urld.title = temp;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	સંરચના path_data *p;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p) {
		પાછા NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	સંરચના path_data *p;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p) {
		પાછા NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(ખાલી);
	p->urld.visits++;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
શૂન્ય urldb_reset_url_visit_data(nsurl *url)
{
	સંરચના path_data *p;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
અચલ સંરચના url_data *urldb_get_url_data(nsurl *url)
{
	સંરચના path_data *p;
	સંરચના url_internal_data *u;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા ખાલી;

	u = &p->urld;

	પાછા (અચલ સંરચના url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	સંરચના path_data *p;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા ખાલી;

	પાછા p->url;
}


/* exported interface documented in netsurf/url_db.h */
શૂન્ય urldb_set_auth_details(nsurl *url, અચલ અક્ષર *realm, અચલ અક્ષર *auth)
{
	સંરચના path_data *p, *pi;
	સંરચના host_part *h;
	સંરચના prot_space_data *space, *space_alloc;
	અક્ષર *realm_alloc, *auth_alloc;
	bool match;

	પ્રતિજ્ઞા(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	જો (!p)
		પાછા;

	/* Search for host_part */
	માટે (pi = p; pi->parent != ખાલી; pi = pi->parent)
		;
	h = (સંરચના host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	માટે (space = h->prot_space; space; space = space->next) {
		જો (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			તોડો;
	}

	જો (space != ખાલી) {
		/* Overrule existing auth. */
		મુક્ત(space->auth);
		space->auth = strdup(auth);
	} નહીંતર {
		/* Create a new protection space. */
		space = space_alloc = malloc(કદ(સંરચના prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		જો (!space_alloc || !realm_alloc || !auth_alloc) {
			મુક્ત(space_alloc);
			મુક્ત(realm_alloc);
			મુક્ત(auth_alloc);
			પાછા;
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
અચલ અક્ષર *urldb_get_auth_details(nsurl *url, અચલ અક્ષર *realm)
{
	સંરચના path_data *p, *p_cur, *p_ટોચ;

	પ્રતિજ્ઞા(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા ખાલી;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	માટે (p_cur = p; p_cur != ખાલી; p_ટોચ = p_cur, p_cur = p_cur->parent) {
		જો (p_cur->prot_space) {
			પાછા p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	જો (realm != ખાલી) {
		અચલ સંરચના host_part *h = (અચલ સંરચના host_part *)p_ટોચ;
		અચલ સંરચના prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		માટે (space = h->prot_space; space != ખાલી;
		     space = space->next) {
			જો (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				પાછા p->prot_space->auth;
			}
		}
	}

	પાછા ખાલી;
}


/* exported interface documented in netsurf/url_db.h */
શૂન્ય urldb_set_cert_permissions(nsurl *url, bool permit)
{
	સંરચના path_data *p;
	સંરચના host_part *h;

	પ્રતિજ્ઞા(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા;

	માટે (; p && p->parent; p = p->parent)
		/* do nothing */;
	પ્રતિજ્ઞા(p);

	h = (સંરચના host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	સંરચના path_data *p;
	અચલ સંરચના host_part *h;

	પ્રતિજ્ઞા(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા false;

	માટે (; p && p->parent; p = p->parent)
		/* do nothing */;
	પ્રતિજ્ઞા(p);

	h = (અચલ સંરચના host_part *)p;

	પાછા h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(સંરચના nsurl *url, અચલ અક્ષર *header)
{
	સંરચના path_data *p;
	સંરચના host_part *h;
	lwc_string *host;
	time_t now = time(ખાલી);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	પ્રતિજ્ઞા(url);

	host = nsurl_get_component(url, NSURL_HOST);
	જો (host != ખાલી) {
		જો (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			પાછા true;
		} નહીંતર જો (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			પાછા true;
		}

		lwc_string_unref(host);
	} નહીંતર {
		/* No host part: ignore */
		પાછા true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા false;

	માટે (; p && p->parent; p = p->parent)
		/* do nothing */;
	પ્રતિજ્ઞા(p);

	h = (સંરચના host_part *)p;
	જો (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		પાછા true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	જો (error != NSERROR_OK) {
		/* Parse failed: ignore */
		પાછા true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	જો (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} નહીંતર જો ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	પાછા true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(સંરચના nsurl *url)
{
	સંરચના path_data *p;
	અચલ સંરચના host_part *h;
	lwc_string *host;
	time_t now = time(ખાલી);

	પ્રતિજ્ઞા(url);

	host = nsurl_get_component(url, NSURL_HOST);
	જો (host != ખાલી) {
		જો (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			પાછા false;
		} નહીંતર જો (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			પાછા false;
		}

		lwc_string_unref(host);
	} નહીંતર {
		/* No host part: not enabled */
		પાછા false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા false;

	માટે (; p && p->parent; p = p->parent)
		/* do nothing */;
	પ્રતિજ્ઞા(p);

	h = (અચલ સંરચના host_part *)p;

	/* Consult record for this host */
	જો (h->hsts.expires > now) {
		/* Not expired */
		પાછા true;
	}

	/* Consult parent domains */
	માટે (h = h->parent; h && h != &db_મૂળ; h = h->parent) {
		જો (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			પાછા true;
		}
	}

	પાછા false;
}


/* exported interface documented in netsurf/url_db.h */
શૂન્ય
urldb_iterate_partial(અચલ અક્ષર *prefix,
		      bool (*callback)(nsurl *url, અચલ સંરચના url_data *data))
{
	અક્ષર host[256];
	અક્ષર buf[260]; /* max domain + "www." */
	અચલ અક્ષર *slash, *scheme_sep;
	સંરચના શોધ_ગાંઠ *વૃક્ષ;
	અચલ સંરચના host_part *h;

	પ્રતિજ્ઞા(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	જો (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	વૃક્ષ = urldb_get_શોધ_વૃક્ષ(prefix);

	જો (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, કદ host, "%.*s",
			 (પૂર્ણાંક) (slash - prefix), prefix);

		h = urldb_શોધ_find(વૃક્ષ, host);
		જો (!h) {
			પૂર્ણાંક len = slash - prefix;

			જો (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, કદ buf, "www.%s", host);
				h = urldb_શોધ_find(
					શોધ_trees[ST_DN + 'w' - 'a'],
					buf);
				જો (!h)
					પાછા;
			} નહીંતર
				પાછા;
		}

		જો (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} નહીંતર {
		પૂર્ણાંક len = strlen(prefix);

		/* looking for hosts */
		જો (!urldb_iterate_partial_host(વૃક્ષ, prefix, callback))
			પાછા;

		જો (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, કદ buf, "www.%s", prefix);
			જો(!urldb_iterate_partial_host(
				   શોધ_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				પાછા;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
શૂન્ય
urldb_iterate_entries(bool (*callback)(nsurl *url, અચલ સંરચના url_data *data))
{
	પૂર્ણાંક i;

	પ્રતિજ્ઞા(callback);

	માટે (i = 0; i < અંક_શોધ_TREES; i++) {
		જો (!urldb_iterate_entries_host(શોધ_trees[i],
						callback,
						ખાલી)) {
			તોડો;
		}
	}
}


/* exported interface documented in content/urldb.h */
શૂન્ય urldb_iterate_cookies(bool (*callback)(અચલ સંરચના cookie_data *data))
{
	પૂર્ણાંક i;

	પ્રતિજ્ઞા(callback);

	માટે (i = 0; i < અંક_શોધ_TREES; i++) {
		જો (!urldb_iterate_entries_host(શોધ_trees[i],
						ખાલી, callback))
			તોડો;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(અચલ અક્ષર *header, nsurl *url, nsurl *referer)
{
	અચલ અક્ષર *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	પ્રતિજ્ઞા(url && header);

	/* Get defragmented URL, as 'urlt' */
	જો (nsurl_defragment(url, &urlt) != NSERROR_OK)
		પાછા ખાલી;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	જો (scheme == ખાલી) {
		nsurl_unref(urlt);
		પાછા false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	જો (path == ખાલી) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		પાછા false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	જો (host == ખાલી) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		પાછા false;
	}

	જો (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		જો (rhost == ખાલી) {
			જાઓ error;
		}

		/* Domain match host names */
		જો (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			અચલ અક્ષર *hptr;
			અચલ અક્ષર *rptr;
			અચલ અક્ષર *dot;
			અચલ અક્ષર *host_data = lwc_string_data(host);
			અચલ અક્ષર *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			જો (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				જાઓ error;
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
			જ્યારે (hptr >= host_data && rptr >= rhost_data) {
				જો (*hptr != *rptr)
					તોડો;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			જ્યારે (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			જો (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ખાલી ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				જાઓ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	કરો {
		સંરચના cookie_internal_data *c;
		અક્ષર *dot;
		size_t len;
#ifdef WITH_NSPSL
		અચલ અક્ષર *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		જો (!c) {
			/* failed => stop parsing */
			જાઓ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		જો (!c->name || !c->કિંમત) {
			urldb_free_cookie(c);
			જાઓ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		જો (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			જાઓ error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		જો (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		જો (suffix == ખાલી) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			જાઓ error;
		}
#નહીંતર
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		જો (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			જાઓ error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		જો (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			પૂર્ણાંક hlen, dlen;
			અક્ષર *domain = c->domain;

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
			પ્રતિજ્ઞા(c->domain[0] == '.');

			/* 4.3.2:iii */
			જો (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				જાઓ error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			જો (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				જાઓ error;
			}

			જો (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			જો (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				જાઓ error;
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
		જો (!urldb_ઉમેરો_cookie(c, scheme, urlt))
			જાઓ error;
	} જ્યારે (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	પાછા true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	પાછા false;
}


/* exported interface documented in content/urldb.h */
અક્ષર *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	અચલ સંરચના path_data *p, *q;
	અચલ સંરચના host_part *h;
	lwc_string *path_lwc;
	સંરચના cookie_internal_data *c;
	પૂર્ણાંક ગણતરી = 0, version = COOKIE_RFC2965;
	સંરચના cookie_internal_data **matched_cookies;
	પૂર્ણાંક matched_cookies_size = 20;
	અચલ અક્ષર *path;
	અક્ષર *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	પ્રતિજ્ઞા(url != ખાલી);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	જો (!p)
		પાછા ખાલી;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 કદ(સંરચના cookie_internal_data *));
	જો (!matched_cookies)
		પાછા ખાલી;

#define GROW_MATCHED_COOKIES						\
	કરો {								\
		જો (ગણતરી == matched_cookies_size) {			\
			સંરચના cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       કદ(સંરચના cookie_internal_data *)); \
									\
			જો (temp == ખાલી) {				\
				મુક્ત(matched_cookies);			\
				પાછા ખાલી;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} જ્યારે(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	જો (path_lwc == ખાલી) {
		મુક્ત(matched_cookies);
		પાછા ખાલી;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(ખાલી);

	જો (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		માટે (q = p->parent->children; q; q = q->next) {
			જો (strcmp(q->segment, p->segment))
				ચાલુ;

			/* Consider all cookies associated with
			 * this exact path */
			માટે (c = q->cookies; c; c = c->next) {
				જો (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ચાલુ;

				જો (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					ચાલુ;

				જો (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ચાલુ;

				matched_cookies[ગણતરી++] = c;

				GROW_MATCHED_COOKIES;

				જો (c->version < (unsigned પૂર્ણાંક)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((સંરચના cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	માટે (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		માટે (q = p->children; q; q = q->next) {
			જો (*(q->segment) != '\0')
				ચાલુ;

			માટે (c = q->cookies; c; c = c->next) {
				જો (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ચાલુ;

				જો (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ચાલુ;

				matched_cookies[ગણતરી++] = c;

				GROW_MATCHED_COOKIES;

				જો (c->version < (unsigned પૂર્ણાંક) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((સંરચના cookie_data *)c);
			}
		}

		જો (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			તોડો;
		}

		/* Consider p itself - may be the result of Path=/foo */
		માટે (c = p->cookies; c; c = c->next) {
			જો (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ચાલુ;

			/* Ensure cookie path is a prefix of the resource */
			જો (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ચાલુ;

			જો (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ચાલુ;

			matched_cookies[ગણતરી++] = c;

			GROW_MATCHED_COOKIES;

			જો (c->version < (unsigned પૂર્ણાંક) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((સંરચના cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	માટે (h = (અચલ સંરચના host_part *)p; h && h != &db_મૂળ;
	     h = h->parent) {
		માટે (c = h->paths.cookies; c; c = c->next) {
			જો (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ચાલુ;

			/* Ensure cookie path is a prefix of the resource */
			જો (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ચાલુ;

			જો (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				ચાલુ;

			matched_cookies[ગણતરી++] = c;

			GROW_MATCHED_COOKIES;

			જો (c->version < (unsigned પૂર્ણાંક)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((સંરચના cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	જો (ગણતરી == 0) {
		/* No cookies found */
		મુક્ત(matched_cookies);
		પાછા ખાલી;
	}

	જો (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, ગણતરી);
	} નહીંતર {
		ret = get_rfc2965_cookie(matched_cookies, ગણતરી, version);
	}
	મુક્ત(matched_cookies);

	પાછા ret;
}


/* exported interface documented in content/urldb.h */
શૂન્ય urldb_delete_cookie(અચલ અક્ષર *domain, અચલ અક્ષર *path,
			 અચલ અક્ષર *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_મૂળ);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(અચલ અક્ષર *filename)
{
	FILE *fp;
	અક્ષર s[16*1024];

	પ્રતિજ્ઞા(filename);

	fp = fopen(filename, "r");
	જો (!fp)
		પાછા NSERROR_NOT_FOUND;

#define FIND_T {				\
		માટે (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		જો (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ચાલુ;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		માટે (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		જો (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ચાલુ;		\
		}				\
	}

	જ્યારે (fgets(s, કદ s, fp)) {
		અક્ષર *p = s, *end = 0,
			*domain, *path, *name, *કિંમત, *scheme, *url,
			*comment;
		પૂર્ણાંક version, domain_specified, path_specified,
			secure, http_only, no_destroy, કિંમત_quoted;
		time_t expires, last_used;
		સંરચના cookie_internal_data *c;

		જો(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ચાલુ;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		જો (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			જો (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				તોડો;
			}

			ચાલુ;
		} નહીંતર જો (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			ચાલુ;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		જો (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} નહીંતર {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; કિંમત = p; FIND_T;
		જો (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	કિંમત_quoted = atoi(p); FIND_T;
		} નહીંતર {
			કિંમત_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		માટે (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		પ્રતિજ્ઞા(p <= end);

		/* Now create cookie */
		c = malloc(કદ(સંરચના cookie_internal_data));
		જો (!c)
			તોડો;

		c->name = strdup(name);
		c->કિંમત = strdup(કિંમત);
		c->કિંમત_was_quoted = કિંમત_quoted;
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

		જો (!(c->name && c->કિંમત && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			તોડો;
		}

		જો (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ખાલી;
			nsurl *url_nsurl = ખાલી;

			પ્રતિજ્ઞા(scheme[0] != 'u');

			જો (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				તોડો;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			જો (!urldb_ઉમેરો_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				તોડો;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} નહીંતર {
			જો (!urldb_ઉમેરો_cookie(c, ખાલી, ખાલી)) {
				/* Cookie freed for us */
				તોડો;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	પાછા NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(અચલ અક્ષર *filename)
{
	FILE *fp;
	પૂર્ણાંક cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	પ્રતિજ્ઞા(filename);

	fp = fopen(filename, "w");
	જો (!fp)
		પાછા NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_મૂળ);

	fclose(fp);

	પાછા NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
શૂન્ય urldb_dump(શૂન્ય)
{
	પૂર્ણાંક i;

	urldb_dump_hosts(&db_મૂળ);

	માટે (i = 0; i != અંક_શોધ_TREES; i++) {
		urldb_dump_શોધ(શોધ_trees[i], 0);
	}
}
