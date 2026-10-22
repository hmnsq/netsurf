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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <time.h>
#ifdef WITH_NSPSL
#അന്തര്ഭാവ <nspsl.h>
#endif

#അന്തര്ഭാവ "utils/inet.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/url.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/bloom.h"
#അന്തര്ഭാവ "utils/time.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "desktop/cookie_manager.h"

#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
ഘടന cookie_internal_data {
	ഘടന cookie_internal_data *prev;	/**< Previous in list */
	ഘടന cookie_internal_data *next;	/**< Next in list */

	അക്ഷരം *name;		/**< Cookie name */
	അക്ഷരം *മൂല്യം;		/**< Cookie value */
	bool മൂല്യം_was_quoted;	/**< Value was quoted in Set-Cookie: */
	അക്ഷരം *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	അക്ഷരം *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	അക്ഷരം *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	എണ്ണൽ cookie_version version;	/**< Specification compliance */
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
ഘടന prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned പൂർണ്ണസംഖ്യ port;
	/** Protection realm */
	അക്ഷരം *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	അക്ഷരം *auth;
	/** Next sibling */
	ഘടന prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ഘടന url_internal_data {
	അക്ഷരം *title;		/**< Resource title */
	unsigned പൂർണ്ണസംഖ്യ visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ഘടന path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned പൂർണ്ണസംഖ്യ port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	അക്ഷരം *segment;		/**< Path segment for this node */
	unsigned പൂർണ്ണസംഖ്യ frag_cnt;	/**< Number of entries in path_data::fragment */
	അക്ഷരം **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ഘടന url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	സ്ഥിരം ഘടന prot_space_data *prot_space;
	/** Cookies associated with resource */
	ഘടന cookie_internal_data *cookies;
	/** Last cookie in list */
	ഘടന cookie_internal_data *cookies_end;

	ഘടന path_data *next;	/**< Next sibling */
	ഘടന path_data *prev;	/**< Previous sibling */
	ഘടന path_data *parent; /**< Parent path segment */
	ഘടന path_data *children; /**< Child path segments */
	ഘടന path_data *last; /**< Last child */
};

ഘടന hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ഘടന host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ഘടന path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ഘടന hsts_data hsts;

	/**
	 * Part of host string
	 */
	അക്ഷരം *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ഘടന prot_space_data *prot_space;

	ഘടന host_part *next;	/**< Next sibling */
	ഘടന host_part *prev;	/**< Previous sibling */
	ഘടന host_part *parent; /**< Parent host part */
	ഘടന host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ഘടന തിരയൽ_കണ്ണി {
	സ്ഥിരം ഘടന host_part *data;	/**< Host tree entry */

	unsigned പൂർണ്ണസംഖ്യ level;		/**< Node level */

	ഘടന തിരയൽ_കണ്ണി *ഇടത്;	/**< Left subtree */
	ഘടന തിരയൽ_കണ്ണി *വലത്;	/**< Right subtree */
};

/** Root database handle */
സ്ഥിതം ഘടന host_part db_വേര്;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define അക്കം_തിരയൽ_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി empty = { 0, 0, &empty, &empty };
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *തിരയൽ_trees[അക്കം_തിരയൽ_TREES] = {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ loaded_cookie_file_version;

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
സ്ഥിതം ഘടന bloom_filter *url_bloom;
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
സ്ഥിതം nserror urldb_write_timet(FILE *fp, time_t val)
{
	പൂർണ്ണസംഖ്യ use;
	അക്ഷരം op[32];

	use = nsc_sntimet(op, 32, &val);
	എങ്കിൽ (use == 0) {
		fprintf(fp, "%i\n", (പൂർണ്ണസംഖ്യ)val);
	} അല്ലെങ്കിൽ {
		fprintf(fp, "%.*s\n", use, op);
	}
	തിരികെ NSERROR_OK;
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
സ്ഥിതം ശൂന്യം
urldb_write_paths(സ്ഥിരം ഘടന path_data *parent,
		  സ്ഥിരം അക്ഷരം *host,
		  FILE *fp,
		  അക്ഷരം **path,
		  പൂർണ്ണസംഖ്യ *path_alloc,
		  പൂർണ്ണസംഖ്യ *path_used,
		  time_t expiry)
{
	സ്ഥിരം ഘടന path_data *p = parent;
	പൂർണ്ണസംഖ്യ i;

	ചെയ്യുക {
		പൂർണ്ണസംഖ്യ seglen = p->segment != ശൂന്യ ? strlen(p->segment) : 0;
		പൂർണ്ണസംഖ്യ len = *path_used + seglen + 1;

		എങ്കിൽ (*path_alloc < len) {
			അക്ഷരം *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			എങ്കിൽ (!temp) {
				തിരികെ;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		എങ്കിൽ (p->segment != ശൂന്യ) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		എങ്കിൽ (p->children != ശൂന്യ) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} അല്ലെങ്കിൽ {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		എങ്കിൽ (p->children != ശൂന്യ) {
			/* Drill down into children */
			p = p->children;
		} അല്ലെങ്കിൽ {
			/* leaf node */
			എങ്കിൽ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				എങ്കിൽ (p->port) {
					fprintf(fp,"%d\n", p->port);
				} അല്ലെങ്കിൽ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (പൂർണ്ണസംഖ്യ)p->urld.type);

				fprintf(fp, "\n");

				എങ്കിൽ (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					വേണ്ടി (i = 0; s[i] != '\0'; i++)
						എങ്കിൽ (s[i] < 32)
							s[i] = ' ';
					വേണ്ടി (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} അല്ലെങ്കിൽ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ആകുമ്പോൾ (p != parent) {
				പൂർണ്ണസംഖ്യ seglen = p->segment != ശൂന്യ
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				എങ്കിൽ (p->next != ശൂന്യ) {
					/* Have a sibling, process that */
					p = p->next;
					നിർത്തുക;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
സ്ഥിതം ശൂന്യം
urldb_എണ്ണം_urls(സ്ഥിരം ഘടന path_data *വേര്,
		 time_t expiry,
		 unsigned പൂർണ്ണസംഖ്യ *എണ്ണം)
{
	സ്ഥിരം ഘടന path_data *p = വേര്;

	ചെയ്യുക {
		എങ്കിൽ (p->children != ശൂന്യ) {
			/* Drill down into children */
			p = p->children;
		} അല്ലെങ്കിൽ {
			/* No more children, increment count if required */
			എങ്കിൽ (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*എണ്ണം)++;
			}

			/* Now, find next node to process. */
			ആകുമ്പോൾ (p != വേര്) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					/* Have a sibling, process that */
					p = p->next;
					നിർത്തുക;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != വേര്);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
സ്ഥിതം ശൂന്യം urldb_save_തിരയൽ_വൃക്ഷം(ഘടന തിരയൽ_കണ്ണി *parent, FILE *fp)
{
	അക്ഷരം host[256];
	സ്ഥിരം ഘടന host_part *h;
	unsigned പൂർണ്ണസംഖ്യ path_എണ്ണം = 0;
	അക്ഷരം *path, *p, *end;
	പൂർണ്ണസംഖ്യ path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	പൂർണ്ണസംഖ്യ hsts_include_subdomains = 0;

	expiry = time(ശൂന്യ) - ((60 * 60 * 24) * nsoption_int(expire_url));

	എങ്കിൽ (parent == &empty)
		തിരികെ;

	urldb_save_തിരയൽ_വൃക്ഷം(parent->ഇടത്, fp);

	path = malloc(path_alloc);
	എങ്കിൽ (!path)
		തിരികെ;

	path[0] = '\0';

	വേണ്ടി (h = parent->data, p = host, end = host + വലുപ്പം host;
	     h && h != &db_വേര് && p < end; h = h->parent) {
		പൂർണ്ണസംഖ്യ written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		എങ്കിൽ (written < 0) {
			സ്വതന്ത്രം(path);
			തിരികെ;
		}
		p += written;
	}

	h = parent->data;
	എങ്കിൽ (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_എണ്ണം_urls(&parent->data->paths, expiry, &path_എണ്ണം);

	എങ്കിൽ (path_എണ്ണം > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_എണ്ണം);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} അല്ലെങ്കിൽ എങ്കിൽ (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	സ്വതന്ത്രം(path);

	urldb_save_തിരയൽ_വൃക്ഷം(parent->വലത്, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
സ്ഥിതം bool
urldb_iterate_entries_path(സ്ഥിരം ഘടന path_data *parent,
		bool (*url_callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data),
		bool (*cookie_callback)(സ്ഥിരം ഘടന cookie_data *data))
{
	സ്ഥിരം ഘടന path_data *p = parent;
	സ്ഥിരം ഘടന cookie_data *c;

	ചെയ്യുക {
		എങ്കിൽ (p->children != ശൂന്യ) {
			/* Drill down into children */
			p = p->children;
		} അല്ലെങ്കിൽ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			പ്രതിജ്ഞാ(url_callback || cookie_callback);

			/** \todo handle fragments? */
			എങ്കിൽ (url_callback) {
				സ്ഥിരം ഘടന url_internal_data *u = &p->urld;

				പ്രതിജ്ഞാ(p->url);

				എങ്കിൽ (!url_callback(p->url,
						  (സ്ഥിരം ഘടന url_data *) u))
					തിരികെ false;
			} അല്ലെങ്കിൽ {
				c = (സ്ഥിരം ഘടന cookie_data *)p->cookies;
				വേണ്ടി (; c != ശൂന്യ; c = c->next) {
					എങ്കിൽ (!cookie_callback(c))
						തിരികെ false;
				}
			}

			/* Now, find next node to process. */
			ആകുമ്പോൾ (p != parent) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					/* Have a sibling, process that */
					p = p->next;
					നിർത്തുക;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != parent);

	തിരികെ true;
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
സ്ഥിതം bool urldb__host_is_ip_address(സ്ഥിരം അക്ഷരം *host)
{
	ഘടന in_addr ipv4;
	size_t host_len = strlen(host);
	സ്ഥിരം അക്ഷരം *sane_host;
	സ്ഥിരം അക്ഷരം *slash;
#ifndef NO_IPV6
	ഘടന in6_addr ipv6;
	അക്ഷരം ipv6_addr[64];
	unsigned പൂർണ്ണസംഖ്യ ipv6_addr_len;
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
	എങ്കിൽ (slash == ശൂന്യ) {
		sane_host = host;
	} അല്ലെങ്കിൽ {
		അക്ഷരം *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	എങ്കിൽ (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		പോകുക out_false;

	എങ്കിൽ (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		പൂർണ്ണസംഖ്യ അക്കം_dots = 0;
		size_t index;

		വേണ്ടി (index = 0; index < host_len; index++) {
			എങ്കിൽ (sane_host[index] == '.')
				അക്കം_dots++;
		}

		എങ്കിൽ (അക്കം_dots == 3)
			പോകുക out_true;
		അല്ലെങ്കിൽ
			പോകുക out_false;
	}

#ifndef NO_IPV6
	എങ്കിൽ ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		പോകുക out_false;
	}

	ipv6_addr_len = host_len - 2;
	എങ്കിൽ (ipv6_addr_len >= വലുപ്പം(ipv6_addr)) {
		ipv6_addr_len = വലുപ്പം(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	എങ്കിൽ (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		പോകുക out_true;
#endif

out_false:
	എങ്കിൽ (slash != ശൂന്യ) സ്വതന്ത്രം((ശൂന്യം *)sane_host);
	തിരികെ false;

out_true:
	എങ്കിൽ (slash != ശൂന്യ) സ്വതന്ത്രം((ശൂന്യം *)sane_host);
	തിരികെ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ urldb_തിരയൽ_match_prefix(സ്ഥിരം ഘടന host_part *a, സ്ഥിരം അക്ഷരം *b)
{
	സ്ഥിരം അക്ഷരം *end, *dot;
	പൂർണ്ണസംഖ്യ plen, ret;

	പ്രതിജ്ഞാ(a && a != &db_വേര് && b);

	എങ്കിൽ (urldb__host_is_ip_address(b)) {
		/* IP address */
		തിരികെ strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ആകുമ്പോൾ (b < end && a && a != &db_വേര്) {
		dot = strchr(b, '.');
		എങ്കിൽ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		എങ്കിൽ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			തിരികെ ret;

		/* The strings matched */
		എങ്കിൽ (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			എങ്കിൽ (plen > dot - b) {
				/* len(a) > len(b) */
				തിരികെ 1;
			} അല്ലെങ്കിൽ എങ്കിൽ (plen < dot - b) {
				/* len(a) < len(b) */
				തിരികെ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	എങ്കിൽ (a && a != &db_വേര് && b >= end) {
		/* len(a) > len(b) => prefix matches */
		തിരികെ 0;
	} അല്ലെങ്കിൽ എങ്കിൽ ((!a || a == &db_വേര്) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		തിരികെ -1;
	}

	/* Identical */
	തിരികെ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
സ്ഥിതം bool
urldb_iterate_partial_host(ഘടന തിരയൽ_കണ്ണി *വേര്,
		സ്ഥിരം അക്ഷരം *prefix,
		bool (*callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data))
{
	പൂർണ്ണസംഖ്യ c;

	പ്രതിജ്ഞാ(വേര് && prefix && callback);

	എങ്കിൽ (വേര് == &empty)
		തിരികെ true;

	c = urldb_തിരയൽ_match_prefix(വേര്->data, prefix);

	എങ്കിൽ (c > 0) {
		/* No match => look in left subtree */
		തിരികെ urldb_iterate_partial_host(വേര്->ഇടത്,
						  prefix,
						  callback);
	} അല്ലെങ്കിൽ എങ്കിൽ (c < 0) {
		/* No match => look in right subtree */
		തിരികെ urldb_iterate_partial_host(വേര്->വലത്,
						  prefix,
						  callback);
	} അല്ലെങ്കിൽ {
		/* Match => iterate over l/r subtrees & process this node */
		എങ്കിൽ (!urldb_iterate_partial_host(വേര്->ഇടത്,
						prefix,
						callback)) {
			തിരികെ false;
		}

		എങ്കിൽ (വേര്->data->paths.children) {
			/* and extract all paths attached to this host */
			എങ്കിൽ (!urldb_iterate_entries_path(&വേര്->data->paths,
							callback,
							ശൂന്യ)) {
				തിരികെ false;
			}
		}

		എങ്കിൽ (!urldb_iterate_partial_host(വേര്->വലത്,
						prefix,
						callback)) {
			തിരികെ false;
		}
	}

	തിരികെ true;
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
സ്ഥിതം bool
urldb_iterate_partial_path(സ്ഥിരം ഘടന path_data *parent,
		സ്ഥിരം അക്ഷരം *prefix,
		bool (*callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data))
{
	സ്ഥിരം ഘടന path_data *p = parent->children;
	സ്ഥിരം അക്ഷരം *slash, *end = prefix + strlen(prefix);

	ചെയ്യുക {
		slash = strchr(prefix, '/');
		എങ്കിൽ (!slash) {
			slash = end;
		}

		എങ്കിൽ (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			തുടരുക;
		}

		എങ്കിൽ (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			എങ്കിൽ (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				എങ്കിൽ (!urldb_iterate_entries_path(p,
								callback,
								ശൂന്യ)) {
					തിരികെ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} അല്ലെങ്കിൽ {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} അല്ലെങ്കിൽ {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ആകുമ്പോൾ (p != ശൂന്യ);

	തിരികെ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
സ്ഥിതം bool
urldb_iterate_entries_host(ഘടന തിരയൽ_കണ്ണി *parent,
		bool (*url_callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data),
		bool (*cookie_callback)(സ്ഥിരം ഘടന cookie_data *data))
{
	എങ്കിൽ (parent == &empty) {
		തിരികെ true;
	}

	എങ്കിൽ (!urldb_iterate_entries_host(parent->ഇടത്,
					url_callback,
					cookie_callback)) {
		തിരികെ false;
	}

	എങ്കിൽ ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		എങ്കിൽ (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			തിരികെ false;
		}
	}

	എങ്കിൽ (!urldb_iterate_entries_host(parent->വലത്,
					url_callback,
					cookie_callback)) {
		തിരികെ false;
	}

	തിരികെ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
സ്ഥിതം ഘടന host_part *
urldb_add_host_കണ്ണി(സ്ഥിരം അക്ഷരം *part, ഘടന host_part *parent)
{
	ഘടന host_part *d;

	പ്രതിജ്ഞാ(part && parent);

	d = calloc(1, വലുപ്പം(ഘടന host_part));
	എങ്കിൽ (!d) {
		തിരികെ ശൂന്യ;
	}

	d->part = strdup(part);
	എങ്കിൽ (!d->part) {
		സ്വതന്ത്രം(d);
		തിരികെ ശൂന്യ;
	}

	d->next = parent->children;
	എങ്കിൽ (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	തിരികെ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ urldb_add_path_fragment_cmp(സ്ഥിരം ശൂന്യം *a, സ്ഥിരം ശൂന്യം *b)
{
	തിരികെ strcasecmp(*((സ്ഥിരം അക്ഷരം **) a), *((സ്ഥിരം അക്ഷരം **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
സ്ഥിതം ഘടന path_data *
urldb_add_path_fragment(ഘടന path_data *segment, lwc_string *fragment)
{
	അക്ഷരം **temp;

	പ്രതിജ്ഞാ(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	എങ്കിൽ (!fragment)
		തിരികെ segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * വലുപ്പം(അക്ഷരം *));
	എങ്കിൽ (!temp)
		തിരികെ ശൂന്യ;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	എങ്കിൽ (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		തിരികെ ശൂന്യ;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      വലുപ്പം (അക്ഷരം *),
	      urldb_add_path_fragment_cmp);

	തിരികെ segment;
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
സ്ഥിതം ഘടന path_data *
urldb_add_path_കണ്ണി(lwc_string *scheme,
		    unsigned പൂർണ്ണസംഖ്യ port,
		    സ്ഥിരം അക്ഷരം *segment,
		    lwc_string *fragment,
		    ഘടന path_data *parent)
{
	ഘടന path_data *d, *e;

	പ്രതിജ്ഞാ(scheme && segment && parent);

	d = calloc(1, വലുപ്പം(ഘടന path_data));
	എങ്കിൽ (!d)
		തിരികെ ശൂന്യ;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	എങ്കിൽ (!d->segment) {
		lwc_string_unref(d->scheme);
		സ്വതന്ത്രം(d);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (fragment) {
		എങ്കിൽ (!urldb_add_path_fragment(d, fragment)) {
			സ്വതന്ത്രം(d->segment);
			lwc_string_unref(d->scheme);
			സ്വതന്ത്രം(d);
			തിരികെ ശൂന്യ;
		}
	}

	വേണ്ടി (e = parent->children; e; e = e->next) {
		എങ്കിൽ (strcmp(e->segment, d->segment) > 0)
			നിർത്തുക;
	}

	എങ്കിൽ (e) {
		d->prev = e->prev;
		d->next = e;
		എങ്കിൽ (e->prev)
			e->prev->next = d;
		അല്ലെങ്കിൽ
			parent->children = d;
		e->prev = d;
	} അല്ലെങ്കിൽ എങ്കിൽ (!parent->children) {
		d->prev = d->next = ശൂന്യ;
		parent->children = parent->last = d;
	} അല്ലെങ്കിൽ {
		d->next = ശൂന്യ;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	തിരികെ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി **urldb_get_തിരയൽ_വൃക്ഷം_direct(സ്ഥിരം അക്ഷരം *host)
{
	പ്രതിജ്ഞാ(host);

	എങ്കിൽ (urldb__host_is_ip_address(host)) {
		തിരികെ &തിരയൽ_trees[ST_IP];
	} അല്ലെങ്കിൽ എങ്കിൽ (ascii_is_alpha(*host)) {
		തിരികെ &തിരയൽ_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	തിരികെ &തിരയൽ_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *urldb_get_തിരയൽ_വൃക്ഷം(സ്ഥിരം അക്ഷരം *host)
{
	തിരികെ *urldb_get_തിരയൽ_വൃക്ഷം_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ urldb_തിരയൽ_match_string(സ്ഥിരം ഘടന host_part *a, സ്ഥിരം അക്ഷരം *b)
{
	സ്ഥിരം അക്ഷരം *end, *dot;
	പൂർണ്ണസംഖ്യ plen, ret;

	പ്രതിജ്ഞാ(a && a != &db_വേര് && b);

	എങ്കിൽ (urldb__host_is_ip_address(b)) {
		/* IP address */
		തിരികെ strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ആകുമ്പോൾ (b < end && a && a != &db_വേര്) {
		dot = strchr(b, '.');
		എങ്കിൽ (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		എങ്കിൽ ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			തിരികെ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		എങ്കിൽ (plen > dot - b) {
			/* len(a) > len(b) */
			തിരികെ 1;
		} അല്ലെങ്കിൽ എങ്കിൽ (plen < dot - b) {
			/* len(a) < len(b) */
			തിരികെ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	എങ്കിൽ (a && a != &db_വേര് && b >= end) {
		/* len(a) > len(b) */
		തിരികെ 1;
	} അല്ലെങ്കിൽ എങ്കിൽ ((!a || a == &db_വേര്) && b < end) {
		/* len(a) < len(b) */
		തിരികെ -1;
	}

	/* Identical */
	തിരികെ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
സ്ഥിതം സ്ഥിരം ഘടന host_part *
urldb_തിരയൽ_find(ഘടന തിരയൽ_കണ്ണി *വേര്, സ്ഥിരം അക്ഷരം *host)
{
	പൂർണ്ണസംഖ്യ c;

	പ്രതിജ്ഞാ(വേര് && host);

	എങ്കിൽ (വേര് == &empty) {
		തിരികെ ശൂന്യ;
	}

	c = urldb_തിരയൽ_match_string(വേര്->data, host);

	എങ്കിൽ (c > 0) {
		തിരികെ urldb_തിരയൽ_find(വേര്->ഇടത്, host);
	} അല്ലെങ്കിൽ എങ്കിൽ (c < 0) {
		തിരികെ urldb_തിരയൽ_find(വേര്->വലത്, host);
	}

	തിരികെ വേര്->data;
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
സ്ഥിതം ഘടന path_data *
urldb_match_path(സ്ഥിരം ഘടന path_data *parent,
		 സ്ഥിരം അക്ഷരം *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	സ്ഥിരം ഘടന path_data *p;
	സ്ഥിരം അക്ഷരം *slash;
	bool match;

	പ്രതിജ്ഞാ(parent != ശൂന്യ);
	പ്രതിജ്ഞാ(parent->segment == ശൂന്യ);

	എങ്കിൽ (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	പ്രതിജ്ഞാ(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ആകുമ്പോൾ (p != ശൂന്യ) {
		slash = strchr(path + 1, '/');
		എങ്കിൽ (!slash) {
			slash = path + strlen(path);
		}

		എങ്കിൽ (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			എങ്കിൽ (*slash == '\0') {
				/* Complete match */
				തിരികെ (ഘടന path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} അല്ലെങ്കിൽ {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	തിരികെ ശൂന്യ;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
സ്ഥിതം ഘടന path_data *urldb_find_url(nsurl *url)
{
	സ്ഥിരം ഘടന host_part *h;
	ഘടന path_data *p;
	ഘടന തിരയൽ_കണ്ണി *വൃക്ഷം;
	അക്ഷരം *plq;
	സ്ഥിരം അക്ഷരം *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned പൂർണ്ണസംഖ്യ port_int;
	bool match;

	പ്രതിജ്ഞാ(url);

	എങ്കിൽ (url_bloom != ശൂന്യ) {
		എങ്കിൽ (bloom_തിരയൽ_hash(url_bloom, nsurl_hash(url)) == false) {
			തിരികെ ശൂന്യ;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	എങ്കിൽ (scheme == ശൂന്യ)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		തിരികെ ശൂന്യ;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (host != ശൂന്യ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} അല്ലെങ്കിൽ എങ്കിൽ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} അല്ലെങ്കിൽ {
		lwc_string_unref(scheme);
		തിരികെ ശൂന്യ;
	}

	വൃക്ഷം = urldb_get_തിരയൽ_വൃക്ഷം(host_str);
	h = urldb_തിരയൽ_find(വൃക്ഷം, host_str);
	എങ്കിൽ (!h) {
		lwc_string_unref(scheme);
		തിരികെ ശൂന്യ;
	}

	/* generate plq (path, leaf, query) */
	എങ്കിൽ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		തിരികെ ശൂന്യ;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	എങ്കിൽ (port != ശൂന്യ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} അല്ലെങ്കിൽ {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	സ്വതന്ത്രം(plq);
	lwc_string_unref(scheme);

	തിരികെ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
സ്ഥിതം ശൂന്യം urldb_dump_paths(ഘടന path_data *parent)
{
	സ്ഥിരം ഘടന path_data *p = parent;
	unsigned പൂർണ്ണസംഖ്യ i;

	ചെയ്യുക {
		എങ്കിൽ (p->segment != ശൂന്യ) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			വേണ്ടി (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		എങ്കിൽ (p->children != ശൂന്യ) {
			p = p->children;
		} അല്ലെങ്കിൽ {
			ആകുമ്പോൾ (p != parent) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					p = p->next;
					നിർത്തുക;
				}

				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
സ്ഥിതം ശൂന്യം urldb_dump_hosts(ഘടന host_part *parent)
{
	ഘടന host_part *h;

	എങ്കിൽ (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	വേണ്ടി (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
സ്ഥിതം ശൂന്യം urldb_dump_തിരയൽ(ഘടന തിരയൽ_കണ്ണി *parent, പൂർണ്ണസംഖ്യ depth)
{
	സ്ഥിരം ഘടന host_part *h;
	പൂർണ്ണസംഖ്യ i; /* index into string */
	അക്ഷരം s[1024];
	പൂർണ്ണസംഖ്യ r;
	പൂർണ്ണസംഖ്യ sl = വലുപ്പം(s) - 2;

	എങ്കിൽ (parent == &empty)
		തിരികെ;

	urldb_dump_തിരയൽ(parent->ഇടത്, depth + 1);

	വേണ്ടി (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	വേണ്ടി (h = parent->data; h; h = h->parent) {
		എങ്കിൽ (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			എങ്കിൽ (r < 0) {
				നിർത്തുക;
			}
			എങ്കിൽ ((i + r) >= sl) {
				നിർത്തുക;
			}
			i += r;
		}

		എങ്കിൽ (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_തിരയൽ(parent->വലത്, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
urldb_തിരയൽ_match_host(സ്ഥിരം ഘടന host_part *a, സ്ഥിരം ഘടന host_part *b)
{
	പൂർണ്ണസംഖ്യ ret;

	പ്രതിജ്ഞാ(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	വേണ്ടി (; a && a != &db_വേര് && b && b != &db_വേര്;
	     a = a->parent, b = b->parent) {
		എങ്കിൽ ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			തിരികെ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	എങ്കിൽ (a && a != &db_വേര് && (!b || b == &db_വേര്)) {
		/* len(a) > len(b) */
		തിരികെ 1;
	} അല്ലെങ്കിൽ എങ്കിൽ ((!a || a == &db_വേര്) && b && b != &db_വേര്) {
		/* len(a) < len(b) */
		തിരികെ -1;
	}

	/* identical */
	തിരികെ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *urldb_തിരയൽ_skew(ഘടന തിരയൽ_കണ്ണി *വേര്)
{
	പ്രതിജ്ഞാ(വേര്);

	എങ്കിൽ (വേര്->ഇടത്->level == വേര്->level) {
		ഘടന തിരയൽ_കണ്ണി *temp;

		temp = വേര്->ഇടത്;
		വേര്->ഇടത് = temp->വലത്;
		temp->വലത് = വേര്;
		വേര് = temp;
	}

	തിരികെ വേര്;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *urldb_തിരയൽ_split(ഘടന തിരയൽ_കണ്ണി *വേര്)
{
	പ്രതിജ്ഞാ(വേര്);

	എങ്കിൽ (വേര്->വലത്->വലത്->level == വേര്->level) {
		ഘടന തിരയൽ_കണ്ണി *temp;

		temp = വേര്->വലത്;
		വേര്->വലത് = temp->ഇടത്;
		temp->ഇടത് = വേര്;
		വേര് = temp;

		വേര്->level++;
	}

	തിരികെ വേര്;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *
urldb_തിരയൽ_തിരുകുക_internal(ഘടന തിരയൽ_കണ്ണി *വേര്, ഘടന തിരയൽ_കണ്ണി *n)
{
	പ്രതിജ്ഞാ(വേര് && n);

	എങ്കിൽ (വേര് == &empty) {
		വേര് = n;
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ c = urldb_തിരയൽ_match_host(വേര്->data, n->data);

		എങ്കിൽ (c > 0) {
			വേര്->ഇടത് = urldb_തിരയൽ_തിരുകുക_internal(
				വേര്->ഇടത്, n);
		} അല്ലെങ്കിൽ എങ്കിൽ (c < 0) {
			വേര്->വലത് = urldb_തിരയൽ_തിരുകുക_internal(
				വേര്->വലത്, n);
		} അല്ലെങ്കിൽ {
			/* exact match */
			സ്വതന്ത്രം(n);
			തിരികെ വേര്;
		}

		വേര് = urldb_തിരയൽ_skew(വേര്);
		വേര് = urldb_തിരയൽ_split(വേര്);
	}

	തിരികെ വേര്;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
സ്ഥിതം ഘടന തിരയൽ_കണ്ണി *
urldb_തിരയൽ_തിരുകുക(ഘടന തിരയൽ_കണ്ണി *വേര്, സ്ഥിരം ഘടന host_part *data)
{
	ഘടന തിരയൽ_കണ്ണി *n;

	പ്രതിജ്ഞാ(വേര് && data);

	n = malloc(വലുപ്പം(ഘടന തിരയൽ_കണ്ണി));
	എങ്കിൽ (!n)
		തിരികെ ശൂന്യ;

	n->level = 1;
	n->data = data;
	n->ഇടത് = n->വലത് = &empty;

	വേര് = urldb_തിരയൽ_തിരുകുക_internal(വേര്, n);

	തിരികെ വേര്;
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
സ്ഥിതം bool
urldb_parse_avpair(ഘടന cookie_internal_data *c,
		   അക്ഷരം *n,
		   അക്ഷരം *v,
		   bool was_quoted)
{
	പൂർണ്ണസംഖ്യ vlen;

	പ്രതിജ്ഞാ(c && n && v);

	/* Strip whitespace from start of name */
	വേണ്ടി (; *n; n++) {
		എങ്കിൽ (*n != ' ' && *n != '\t')
			നിർത്തുക;
	}

	/* Strip whitespace from end of name */
	വേണ്ടി (vlen = strlen(n); vlen; vlen--) {
		എങ്കിൽ (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		അല്ലെങ്കിൽ
			നിർത്തുക;
	}

	/* Strip whitespace from start of value */
	വേണ്ടി (; *v; v++) {
		എങ്കിൽ (*v != ' ' && *v != '\t')
			നിർത്തുക;
	}

	/* Strip whitespace from end of value */
	വേണ്ടി (vlen = strlen(v); vlen; vlen--) {
		എങ്കിൽ (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		അല്ലെങ്കിൽ
			നിർത്തുക;
	}

	എങ്കിൽ (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		എങ്കിൽ (!c->comment)
			തിരികെ false;
	} അല്ലെങ്കിൽ എങ്കിൽ (!c->domain && strcasecmp(n, "Domain") == 0) {
		എങ്കിൽ (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			എങ്കിൽ (!c->domain)
				തിരികെ false;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasecmp(n, "Max-Age") == 0) {
		പൂർണ്ണസംഖ്യ temp = atoi(v);
		എങ്കിൽ (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		അല്ലെങ്കിൽ
			c->expires = time(ശൂന്യ) + temp;
	} അല്ലെങ്കിൽ എങ്കിൽ (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		എങ്കിൽ (!c->path)
			തിരികെ false;
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasecmp(n, "Expires") == 0) {
		അക്ഷരം *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		വേണ്ടി (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		എങ്കിൽ (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} അല്ലെങ്കിൽ എങ്കിൽ (!c->name) {
		c->name = strdup(n);
		c->മൂല്യം = strdup(v);
		c->മൂല്യം_was_quoted = was_quoted;
		എങ്കിൽ (!c->name || !c->മൂല്യം) {
			തിരികെ false;
		}
	}

	തിരികെ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
സ്ഥിതം ശൂന്യം urldb_free_cookie(ഘടന cookie_internal_data *c)
{
	പ്രതിജ്ഞാ(c);

	സ്വതന്ത്രം(c->comment);
	സ്വതന്ത്രം(c->domain);
	സ്വതന്ത്രം(c->path);
	സ്വതന്ത്രം(c->name);
	സ്വതന്ത്രം(c->മൂല്യം);
	സ്വതന്ത്രം(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
സ്ഥിതം ഘടന cookie_internal_data *
urldb_parse_cookie(nsurl *url, സ്ഥിരം അക്ഷരം **cookie)
{
	ഘടന cookie_internal_data *c;
	സ്ഥിരം അക്ഷരം *cur;
	അക്ഷരം name[1024], മൂല്യം[4096];
	അക്ഷരം *n = name, *v = മൂല്യം;
	bool in_മൂല്യം = false;
	bool had_മൂല്യം_data = false;
	bool മൂല്യം_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	പ്രതിജ്ഞാ(url && cookie && *cookie);

	c = calloc(1, വലുപ്പം(ഘടന cookie_internal_data));
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	c->expires = -1;

	name[0] = '\0';
	മൂല്യം[0] = '\0';

	വേണ്ടി (cur = *cookie; *cur; cur++) {
		എങ്കിൽ (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			എങ്കിൽ (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				മൂല്യം[0] = '\0';

			}

			നിർത്തുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (*cur == '\r') {
			/* Spurious linefeed */
			തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (*cur == '\n') {
			/* Spurious newline */
			തുടരുക;
		}

		എങ്കിൽ (in_മൂല്യം && !had_മൂല്യം_data) {
			എങ്കിൽ (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				തുടരുക;
			} അല്ലെങ്കിൽ {
				had_മൂല്യം_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				എങ്കിൽ (*cur != '"') {
					മൂല്യം_verbatim = true;
				}
			}
		}

		എങ്കിൽ (in_മൂല്യം && !മൂല്യം_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			എങ്കിൽ (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				തുടരുക;
			}
		}

		എങ്കിൽ (!quoted && !in_മൂല്യം && *cur == '=') {
			/* First equals => attr-value separator */
			in_മൂല്യം = true;
			തുടരുക;
		}

		എങ്കിൽ (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			എങ്കിൽ (!urldb_parse_avpair(c, name, മൂല്യം, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				തിരികെ ശൂന്യ;
			}

			/* And reset to start */
			n = name;
			v = മൂല്യം;
			in_മൂല്യം = false;
			had_മൂല്യം_data = false;
			മൂല്യം_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			എങ്കിൽ (*cur != ';') {
				cur--;
			}

			തുടരുക;
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
		എങ്കിൽ (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			സ്ഥിരം അക്ഷരം *p;
			സ്ഥിരം അക്ഷരം *semi = strchr(cur + 1, ';');
			എങ്കിൽ (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			വേണ്ടി (p = cur + 1; p < semi; p++)
				എങ്കിൽ (*p == '=')
					നിർത്തുക;

			എങ്കിൽ (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} അല്ലെങ്കിൽ {
				/* found one => comma marks end of cookie */
				cur++;
				നിർത്തുക;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		എങ്കിൽ (!in_മൂല്യം) {
			എങ്കിൽ (n < name + (വലുപ്പം(name) - 1))
				*n++ = *cur;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (v < മൂല്യം + (വലുപ്പം(മൂല്യം) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	എങ്കിൽ (!urldb_parse_avpair(c, name, മൂല്യം, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		തിരികെ ശൂന്യ;
	}

	/* Now fix-up default values */
	എങ്കിൽ (c->domain == ശൂന്യ) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		എങ്കിൽ (host == ശൂന്യ) {
			urldb_free_cookie(c);
			തിരികെ ശൂന്യ;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	എങ്കിൽ (c->path == ശൂന്യ) {
		സ്ഥിരം അക്ഷരം *path_data;
		അക്ഷരം *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		എങ്കിൽ (path_lwc == ശൂന്യ) {
			urldb_free_cookie(c);
			തിരികെ ശൂന്യ;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		എങ്കിൽ (slash != ശൂന്യ) {
			/* Special case: retain first slash in path */
			എങ്കിൽ (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			എങ്കിൽ (slash == ശൂന്യ) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				തിരികെ ശൂന്യ;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} അല്ലെങ്കിൽ {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			എങ്കിൽ (path == ശൂന്യ) {
				urldb_free_cookie(c);
				തിരികെ ശൂന്യ;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	തിരികെ c;
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
സ്ഥിതം ഘടന path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned പൂർണ്ണസംഖ്യ port,
	       സ്ഥിരം ഘടന host_part *host,
	       അക്ഷരം *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ഘടന path_data *d, *e;
	അക്ഷരം *buf = path_query;
	അക്ഷരം *segment, *slash;
	bool match;

	പ്രതിജ്ഞാ(scheme && host && url);

	d = (ഘടന path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	എങ്കിൽ (*segment == '/')
		segment++;

	/* Process path segments */
	ചെയ്യുക {
		slash = strchr(segment, '/');
		എങ്കിൽ (!slash) {
			/* last segment */
			/* look for existing entry */
			വേണ്ടി (e = d->children; e; e = e->next)
				എങ്കിൽ (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					നിർത്തുക;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_കണ്ണി(scheme, port,
						    segment, fragment, d);
			നിർത്തുക;
		}

		*slash = '\0';

		/* look for existing entry */
		വേണ്ടി (e = d->children; e; e = e->next)
			എങ്കിൽ (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				നിർത്തുക;

		d = e ? e : urldb_add_path_കണ്ണി(scheme, port, segment, ശൂന്യ, d);
		എങ്കിൽ (!d)
			നിർത്തുക;

		segment = slash + 1;
	} ആകുമ്പോൾ (1);

	സ്വതന്ത്രം(path_query);

	എങ്കിൽ (d && !d->url) {
		/* Insert defragmented URL */
		എങ്കിൽ (nsurl_defragment(url, &d->url) != NSERROR_OK)
			തിരികെ ശൂന്യ;
	}

	തിരികെ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
സ്ഥിതം ഘടന host_part *urldb_add_host(സ്ഥിരം അക്ഷരം *host)
{
	ഘടന host_part *d = (ഘടന host_part *) &db_വേര്, *e;
	ഘടന തിരയൽ_കണ്ണി *s;
	അക്ഷരം buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	അക്ഷരം *part;

	പ്രതിജ്ഞാ(host);

	എങ്കിൽ (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		വേണ്ടി (e = d->children; e; e = e->next)
			എങ്കിൽ (strcasecmp(host, e->part) == 0)
				/* found => return it */
				തിരികെ e;

		d = urldb_add_host_കണ്ണി(host, d);

		s = urldb_തിരയൽ_തിരുകുക(തിരയൽ_trees[ST_IP], d);
		എങ്കിൽ (!s) {
			/* failed */
			d = ശൂന്യ;
		} അല്ലെങ്കിൽ {
			തിരയൽ_trees[ST_IP] = s;
		}

		തിരികെ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, വലുപ്പം buf);
	buf[വലുപ്പം buf - 1] = '\0';

	/* Process FQDN segments backwards */
	ചെയ്യുക {
		part = strrchr(buf, '.');
		എങ്കിൽ (!part) {
			/* last segment */
			/* Check for existing entry */
			വേണ്ടി (e = d->children; e; e = e->next)
				എങ്കിൽ (strcasecmp(buf, e->part) == 0)
					നിർത്തുക;

			എങ്കിൽ (e) {
				d = e;
			} അല്ലെങ്കിൽ {
				d = urldb_add_host_കണ്ണി(buf, d);
			}

			/* And insert into search tree */
			എങ്കിൽ (d) {
				ഘടന തിരയൽ_കണ്ണി **r;

				r = urldb_get_തിരയൽ_വൃക്ഷം_direct(buf);
				s = urldb_തിരയൽ_തിരുകുക(*r, d);
				എങ്കിൽ (!s) {
					/* failed */
					d = ശൂന്യ;
				} അല്ലെങ്കിൽ {
					*r = s;
				}
			}
			നിർത്തുക;
		}

		/* Check for existing entry */
		വേണ്ടി (e = d->children; e; e = e->next)
			എങ്കിൽ (strcasecmp(part + 1, e->part) == 0)
				നിർത്തുക;

		d = e ? e : urldb_add_host_കണ്ണി(part + 1, d);
		എങ്കിൽ (!d)
			നിർത്തുക;

		*part = '\0';
	} ആകുമ്പോൾ (1);

	തിരികെ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
സ്ഥിതം bool
urldb_തിരുകുക_cookie(ഘടന cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ഘടന cookie_internal_data *d;
	സ്ഥിരം ഘടന host_part *h;
	ഘടന path_data *p;
	time_t now = time(ശൂന്യ);

	പ്രതിജ്ഞാ(c);

	എങ്കിൽ (c->domain[0] == '.') {
		h = urldb_തിരയൽ_find(
			urldb_get_തിരയൽ_വൃക്ഷം(&(c->domain[1])),
			c->domain + 1);
		എങ്കിൽ (!h) {
			h = urldb_add_host(c->domain + 1);
			എങ്കിൽ (!h) {
				urldb_free_cookie(c);
				തിരികെ false;
			}
		}

		p = (ഘടന path_data *) &h->paths;
	} അല്ലെങ്കിൽ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		പ്രതിജ്ഞാ(url != ശൂന്യ);
		പ്രതിജ്ഞാ(scheme != ശൂന്യ);

		h = urldb_തിരയൽ_find(
			urldb_get_തിരയൽ_വൃക്ഷം(c->domain),
			c->domain);

		എങ്കിൽ (!h) {
			h = urldb_add_host(c->domain);
			എങ്കിൽ (!h) {
				urldb_free_cookie(c);
				തിരികെ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), ശൂന്യ, url);
		എങ്കിൽ (!p) {
			urldb_free_cookie(c);
			തിരികെ false;
		}
	}

	/* add cookie */
	വേണ്ടി (d = p->cookies; d; d = d->next) {
		എങ്കിൽ (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			നിർത്തുക;
	}

	എങ്കിൽ (d) {
		എങ്കിൽ (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			എങ്കിൽ (d->next)
				d->next->prev = d->prev;
			അല്ലെങ്കിൽ
				p->cookies_end = d->prev;
			എങ്കിൽ (d->prev)
				d->prev->next = d->next;
			അല്ലെങ്കിൽ
				p->cookies = d->next;

			cookie_manager_remove((ഘടന cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} അല്ലെങ്കിൽ {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			എങ്കിൽ (c->next)
				c->next->prev = c;
			അല്ലെങ്കിൽ
				p->cookies_end = c;
			എങ്കിൽ (c->prev)
				c->prev->next = c;
			അല്ലെങ്കിൽ
				p->cookies = c;

			cookie_manager_remove((ഘടന cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((ഘടന cookie_data *)c);
		}
	} അല്ലെങ്കിൽ {
		c->prev = p->cookies_end;
		c->next = ശൂന്യ;
		എങ്കിൽ (p->cookies_end)
			p->cookies_end->next = c;
		അല്ലെങ്കിൽ
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((ഘടന cookie_data *)c);
	}

	തിരികെ true;
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
സ്ഥിതം bool
concat_netscape_cookie(ഘടന cookie_internal_data *c,
		    പൂർണ്ണസംഖ്യ *used,
		    പൂർണ്ണസംഖ്യ *alloc,
		    അക്ഷരം **buf)
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
	പൂർണ്ണസംഖ്യ max_len;
	പൂർണ്ണസംഖ്യ outl; /* output length */

	പ്രതിജ്ഞാ(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->മൂല്യം) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	എങ്കിൽ (max_len >= *alloc) {
		അക്ഷരം *temp = realloc(*buf, max_len);
		എങ്കിൽ (!temp) {
			തിരികെ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	എങ്കിൽ (outl >= (*alloc - *used)) {
		/* output truncated */
		തിരികെ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	എങ്കിൽ (c->മൂല്യം_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->മൂല്യം);
		എങ്കിൽ (outl >= (*alloc - *used)) {
			/* output truncated */
			തിരികെ false;
		}
		*used += outl;
	} അല്ലെങ്കിൽ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->മൂല്യം);
		എങ്കിൽ (outl >= (*alloc - *used)) {
			/* output truncated */
			തിരികെ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	തിരികെ true;
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
സ്ഥിതം bool
concat_rfc2965_cookie(ഘടന cookie_internal_data *c,
		      പൂർണ്ണസംഖ്യ *used,
		      പൂർണ്ണസംഖ്യ *alloc,
		      അക്ഷരം **buf)
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
	സ്ഥിരം അക്ഷരം * സ്ഥിരം separators = "()<>@,;:\\\"/[]?={} \t";
	പൂർണ്ണസംഖ്യ max_len;
	പൂർണ്ണസംഖ്യ outl; /* output length */

	പ്രതിജ്ഞാ(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->മൂല്യം) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	എങ്കിൽ (max_len >= *alloc) {
		അക്ഷരം *temp = realloc(*buf, max_len);
		എങ്കിൽ (!temp) {
			തിരികെ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	എങ്കിൽ (outl >= (*alloc - *used)) {
		/* output truncated */
		തിരികെ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	എങ്കിൽ (c->മൂല്യം_was_quoted ||
	    strpbrk(c->മൂല്യം, separators) != ശൂന്യ) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->മൂല്യം);

	} അല്ലെങ്കിൽ {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->മൂല്യം);
	}
	എങ്കിൽ (outl >= (*alloc - *used)) {
		/* output truncated */
		തിരികെ false;
	}
	*used += outl;

	എങ്കിൽ (c->path_from_set) {
		/* Path, quoted if necessary */

		എങ്കിൽ (strpbrk(c->path, separators) != ശൂന്യ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} അല്ലെങ്കിൽ {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		എങ്കിൽ (outl >= (*alloc - *used)) {
			/* output truncated */
			തിരികെ false;
		}
		*used += outl;
	}

	എങ്കിൽ (c->domain_from_set) {
		/* Domain, quoted if necessary */
		എങ്കിൽ (strpbrk(c->domain, separators) != ശൂന്യ) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} അല്ലെങ്കിൽ {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		എങ്കിൽ (outl >= (*alloc - *used)) {
			/* output truncated */
			തിരികെ false;
		}
		*used += outl;
	}

	തിരികെ true;
}


/**
 * build output cookie string in netscape format
 */
സ്ഥിതം അക്ഷരം *
get_netscape_cookie(ഘടന cookie_internal_data **matched_cookies, പൂർണ്ണസംഖ്യ എണ്ണം)
{
	പൂർണ്ണസംഖ്യ ret_alloc = 4096, ret_used = 1;
	അക്ഷരം *ret;
	പൂർണ്ണസംഖ്യ midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	എങ്കിൽ (ret == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}
	ret[0] = '\0';

	വേണ്ടി (midx = 0; midx < എണ്ണം; midx++) {
		എങ്കിൽ (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			സ്വതന്ത്രം(ret);
			തിരികെ ശൂന്യ;
		}
	}

	എങ്കിൽ (ret_used <= 2) {
		സ്വതന്ത്രം(ret);
		തിരികെ ശൂന്യ;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	അക്ഷരം *temp = realloc(ret, ret_used);
	എങ്കിൽ (temp != ശൂന്യ) {
		ret = temp;
	}

	തിരികെ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
സ്ഥിതം അക്ഷരം *
get_rfc2965_cookie(ഘടന cookie_internal_data **matched_cookies,
		   പൂർണ്ണസംഖ്യ എണ്ണം,
		   പൂർണ്ണസംഖ്യ version)
{
	പൂർണ്ണസംഖ്യ ret_alloc = 4096; /* return string allocation */
	പൂർണ്ണസംഖ്യ ret_used = 1; /* return sting buffer used */
	അക്ഷരം *ret; /* return string buffer */
	പൂർണ്ണസംഖ്യ midx; /* index into matched cookies */
	പൂർണ്ണസംഖ്യ outl;

	ret = malloc(ret_alloc);
	എങ്കിൽ (ret == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	എങ്കിൽ (outl >= ret_alloc - ret_used) {
		/* overflow */
		സ്വതന്ത്രം(ret);
		തിരികെ ശൂന്യ;
	}
	ret_used += outl;

	വേണ്ടി (midx = 0; midx < എണ്ണം; midx++) {
		എങ്കിൽ (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			സ്വതന്ത്രം(ret);
			തിരികെ ശൂന്യ;
		}
	}

	/* shrink the output buffer to the required size */
	അക്ഷരം *temp = realloc(ret, ret_used);
	എങ്കിൽ (temp != ശൂന്യ) {
		ret = temp;
	}

	തിരികെ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
സ്ഥിതം ശൂന്യം
urldb_delete_cookie_paths(സ്ഥിരം അക്ഷരം *domain,
			  സ്ഥിരം അക്ഷരം *path,
			  സ്ഥിരം അക്ഷരം *name,
			  ഘടന path_data *parent)
{
	ഘടന cookie_internal_data *c;
	ഘടന path_data *p = parent;

	പ്രതിജ്ഞാ(parent);

	ചെയ്യുക {
		വേണ്ടി (c = p->cookies; c; c = c->next) {
			എങ്കിൽ (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				എങ്കിൽ (c->prev) {
					c->prev->next = c->next;
				} അല്ലെങ്കിൽ {
					p->cookies = c->next;
				}

				എങ്കിൽ (c->next) {
					c->next->prev = c->prev;
				} അല്ലെങ്കിൽ {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				തിരികെ;
			}
		}

		എങ്കിൽ (p->children) {
			p = p->children;
		} അല്ലെങ്കിൽ {
			ആകുമ്പോൾ (p != parent) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					p = p->next;
					നിർത്തുക;
				}

				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
സ്ഥിതം ശൂന്യം
urldb_delete_cookie_hosts(സ്ഥിരം അക്ഷരം *domain,
			  സ്ഥിരം അക്ഷരം *path,
			  സ്ഥിരം അക്ഷരം *name,
			  ഘടന host_part *parent)
{
	ഘടന host_part *h;
	പ്രതിജ്ഞാ(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	വേണ്ടി (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
സ്ഥിതം ശൂന്യം urldb_save_cookie_paths(FILE *fp, ഘടന path_data *parent)
{
	ഘടന path_data *p = parent;
	time_t now = time(ശൂന്യ);

	പ്രതിജ്ഞാ(fp && parent);

	ചെയ്യുക {
		എങ്കിൽ (p->cookies != ശൂന്യ) {
			ഘടന cookie_internal_data *c;

			വേണ്ടി (c = p->cookies; c != ശൂന്യ; c = c->next) {
				എങ്കിൽ (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					തുടരുക;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(പൂർണ്ണസംഖ്യ)c->expires, (പൂർണ്ണസംഖ്യ)c->last_used,
					c->no_destroy, c->name, c->മൂല്യം,
					c->മൂല്യം_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		എങ്കിൽ (p->children != ശൂന്യ) {
			p = p->children;
		} അല്ലെങ്കിൽ {
			ആകുമ്പോൾ (p != parent) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					p = p->next;
					നിർത്തുക;
				}

				p = p->parent;
			}
		}
	} ആകുമ്പോൾ (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
സ്ഥിതം ശൂന്യം urldb_save_cookie_hosts(FILE *fp, ഘടന host_part *parent)
{
	ഘടന host_part *h;
	പ്രതിജ്ഞാ(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	വേണ്ടി (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
സ്ഥിതം ശൂന്യം urldb_destroy_cookie(ഘടന cookie_internal_data *c)
{
	സ്വതന്ത്രം(c->name);
	സ്വതന്ത്രം(c->മൂല്യം);
	സ്വതന്ത്രം(c->comment);
	സ്വതന്ത്രം(c->domain);
	സ്വതന്ത്രം(c->path);

	സ്വതന്ത്രം(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
സ്ഥിതം ശൂന്യം urldb_destroy_path_കണ്ണി_content(ഘടന path_data *കണ്ണി)
{
	ഘടന cookie_internal_data *a, *b;
	unsigned പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (കണ്ണി->url != ശൂന്യ) {
		nsurl_unref(കണ്ണി->url);
	}

	lwc_string_unref(കണ്ണി->scheme);

	സ്വതന്ത്രം(കണ്ണി->segment);
	വേണ്ടി (i = 0; i < കണ്ണി->frag_cnt; i++)
		സ്വതന്ത്രം(കണ്ണി->fragment[i]);
	സ്വതന്ത്രം(കണ്ണി->fragment);

	സ്വതന്ത്രം(കണ്ണി->urld.title);

	വേണ്ടി (a = കണ്ണി->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
സ്ഥിതം ശൂന്യം urldb_destroy_prot_space(ഘടന prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	സ്വതന്ത്രം(space->realm);
	സ്വതന്ത്രം(space->auth);

	സ്വതന്ത്രം(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
സ്ഥിതം ശൂന്യം urldb_destroy_path_വൃക്ഷം(ഘടന path_data *വേര്)
{
	ഘടന path_data *p = വേര്;

	ചെയ്യുക {
		എങ്കിൽ (p->children != ശൂന്യ) {
			p = p->children;
		} അല്ലെങ്കിൽ {
			ഘടന path_data *q = p;

			ആകുമ്പോൾ (p != വേര്) {
				എങ്കിൽ (p->next != ശൂന്യ) {
					p = p->next;
					നിർത്തുക;
				}

				p = p->parent;

				urldb_destroy_path_കണ്ണി_content(q);
				സ്വതന്ത്രം(q);

				q = p;
			}

			urldb_destroy_path_കണ്ണി_content(q);
			സ്വതന്ത്രം(q);
		}
	} ആകുമ്പോൾ (p != വേര്);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
സ്ഥിതം ശൂന്യം urldb_destroy_host_വൃക്ഷം(ഘടന host_part *വേര്)
{
	ഘടന host_part *a, *b;
	ഘടന path_data *p, *q;
	ഘടന prot_space_data *s, *t;

	/* Destroy children */
	വേണ്ടി (a = വേര്->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_വൃക്ഷം(a);
	}

	/* Now clean up paths */
	വേണ്ടി (p = വേര്->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_വൃക്ഷം(p);
	}

	/* Root path */
	urldb_destroy_path_കണ്ണി_content(&വേര്->paths);

	/* Proctection space data */
	വേണ്ടി (s = വേര്->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	സ്വതന്ത്രം(വേര്->part);
	സ്വതന്ത്രം(വേര്);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
സ്ഥിതം ശൂന്യം urldb_destroy_തിരയൽ_വൃക്ഷം(ഘടന തിരയൽ_കണ്ണി *വേര്)
{
	/* Destroy children */
	എങ്കിൽ (വേര്->ഇടത് != &empty)
		urldb_destroy_തിരയൽ_വൃക്ഷം(വേര്->ഇടത്);
	എങ്കിൽ (വേര്->വലത് != &empty)
		urldb_destroy_തിരയൽ_വൃക്ഷം(വേര്->വലത്);

	/* And destroy ourselves */
	സ്വതന്ത്രം(വേര്);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
ശൂന്യം urldb_destroy(ശൂന്യം)
{
	ഘടന host_part *a, *b;
	പൂർണ്ണസംഖ്യ i;

	/* Clean up search trees */
	വേണ്ടി (i = 0; i < അക്കം_തിരയൽ_TREES; i++) {
		എങ്കിൽ (തിരയൽ_trees[i] != &empty) {
			urldb_destroy_തിരയൽ_വൃക്ഷം(തിരയൽ_trees[i]);
			തിരയൽ_trees[i] = &empty;
		}
	}

	/* And database */
	വേണ്ടി (a = db_വേര്.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_വൃക്ഷം(a);
	}
	memset(&db_വേര്, 0, വലുപ്പം(db_വേര്));

	/* And the bloom filter */
	എങ്കിൽ (url_bloom != ശൂന്യ) {
		bloom_destroy(url_bloom);
		url_bloom = ശൂന്യ;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(സ്ഥിരം അക്ഷരം *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	അക്ഷരം s[MAXIMUM_URL_LENGTH];
	അക്ഷരം host[256];
	ഘടന host_part *h;
	പൂർണ്ണസംഖ്യ urls;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ version;
	പൂർണ്ണസംഖ്യ length;
	FILE *fp;

	പ്രതിജ്ഞാ(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	എങ്കിൽ (url_bloom == ശൂന്യ)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	എങ്കിൽ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		തിരികെ NSERROR_NOT_FOUND;
	}

	എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		തിരികെ NSERROR_NEED_DATA;
	}

	version = atoi(s);
	എങ്കിൽ (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		തിരികെ NSERROR_INVALID;
	}
	എങ്കിൽ (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		തിരികെ NSERROR_INVALID;
	}

	ആകുമ്പോൾ (fgets(host, വലുപ്പം host, fp)) {
		time_t hsts_expiry = 0;
		പൂർണ്ണസംഖ്യ hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		എങ്കിൽ (length == 0) {
			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;
			urls = atoi(s);
			/* Eight fields/url */
			വേണ്ടി (i = 0; i < (8 * urls); i++) {
				എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					നിർത്തുക;
			}
			തുടരുക;
		}

		എങ്കിൽ (version >= 107) {
			അക്ഷരം *p = host;
			ആകുമ്പോൾ (*p && *p != ' ') p++;
			ആകുമ്പോൾ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ആകുമ്പോൾ (*p && *p != ' ') p++;
			ആകുമ്പോൾ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		എങ്കിൽ (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			തിരികെ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			നിർത്തുക;
		urls = atoi(s);

		/* no URLs => try next host */
		എങ്കിൽ (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			തുടരുക;
		}

		/* load the non-corrupt data */
		വേണ്ടി (i = 0; i < urls; i++) {
			ഘടന path_data *p = ശൂന്യ;
			അക്ഷരം scheme[64], ports[10];
			അക്ഷരം url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned പൂർണ്ണസംഖ്യ port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			അക്ഷരം *path_query;
			size_t len;

			എങ്കിൽ (!fgets(scheme, വലുപ്പം scheme, fp))
				നിർത്തുക;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			എങ്കിൽ (!fgets(ports, വലുപ്പം ports, fp))
				നിർത്തുക;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;
			length = strlen(s) - 1;
			s[length] = '\0';

			എങ്കിൽ (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, വലുപ്പം url, "%s://%s%s%s%s",
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
			എങ്കിൽ (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				തിരികെ NSERROR_NOMEM;
			}

			എങ്കിൽ (url_bloom != ശൂന്യ) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_തിരുകുക_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			എങ്കിൽ (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				തിരികെ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			എങ്കിൽ (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				തിരികെ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;
			എങ്കിൽ (p)
				p->urld.visits = (unsigned പൂർണ്ണസംഖ്യ)atoi(s);

			/* entry last use time */
			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				നിർത്തുക;
			}
			എങ്കിൽ (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;
			എങ്കിൽ (p)
				p->urld.type = (content_type)atoi(s);

			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;


			എങ്കിൽ (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				നിർത്തുക;
			length = strlen(s) - 1;
			എങ്കിൽ (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				എങ്കിൽ (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	തിരികെ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(സ്ഥിരം അക്ഷരം *filename)
{
	FILE *fp;
	പൂർണ്ണസംഖ്യ i;

	പ്രതിജ്ഞാ(filename);

	fp = fopen(filename, "w");
	എങ്കിൽ (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		തിരികെ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	വേണ്ടി (i = 0; i != അക്കം_തിരയൽ_TREES; i++) {
		urldb_save_തിരയൽ_വൃക്ഷം(തിരയൽ_trees[i], fp);
	}

	fclose(fp);

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	ഘടന path_data *p;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p) {
		തിരികെ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	ഘടന host_part *h;
	ഘടന path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	സ്ഥിരം അക്ഷരം *host_str;
	അക്ഷരം *path_query = ശൂന്യ;
	size_t len;
	bool match;
	unsigned പൂർണ്ണസംഖ്യ port_int;

	പ്രതിജ്ഞാ(url);

	എങ്കിൽ (url_bloom == ശൂന്യ)
		url_bloom = bloom_create(BLOOM_SIZE);

	എങ്കിൽ (url_bloom != ശൂന്യ) {
		uint32_t hash = nsurl_hash(url);
		bloom_തിരുകുക_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	എങ്കിൽ (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		തിരികെ false;
	}
	പ്രതിജ്ഞാ(path_query != ശൂന്യ);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	എങ്കിൽ (scheme == ശൂന്യ) {
		സ്വതന്ത്രം(path_query);
		തിരികെ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (host != ശൂന്യ) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} അല്ലെങ്കിൽ എങ്കിൽ (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} അല്ലെങ്കിൽ {
		lwc_string_unref(scheme);
		സ്വതന്ത്രം(path_query);
		തിരികെ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	എങ്കിൽ (port != ശൂന്യ) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} അല്ലെങ്കിൽ {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	എങ്കിൽ (h != ശൂന്യ) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} അല്ലെങ്കിൽ {
		p = ശൂന്യ;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	തിരികെ (p != ശൂന്യ);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, സ്ഥിരം അക്ഷരം *title)
{
	ഘടന path_data *p;
	അക്ഷരം *temp;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (p == ശൂന്യ) {
		തിരികെ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	എങ്കിൽ (title != ശൂന്യ) {
		temp = strdup(title);
		എങ്കിൽ (temp == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
	} അല്ലെങ്കിൽ {
		temp = ശൂന്യ;
	}

	സ്വതന്ത്രം(p->urld.title);
	p->urld.title = temp;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	ഘടന path_data *p;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p) {
		തിരികെ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	ഘടന path_data *p;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p) {
		തിരികെ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(ശൂന്യ);
	p->urld.visits++;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
ശൂന്യം urldb_reset_url_visit_data(nsurl *url)
{
	ഘടന path_data *p;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
സ്ഥിരം ഘടന url_data *urldb_get_url_data(nsurl *url)
{
	ഘടന path_data *p;
	ഘടന url_internal_data *u;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ ശൂന്യ;

	u = &p->urld;

	തിരികെ (സ്ഥിരം ഘടന url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ഘടന path_data *p;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ ശൂന്യ;

	തിരികെ p->url;
}


/* exported interface documented in netsurf/url_db.h */
ശൂന്യം urldb_set_auth_details(nsurl *url, സ്ഥിരം അക്ഷരം *realm, സ്ഥിരം അക്ഷരം *auth)
{
	ഘടന path_data *p, *pi;
	ഘടന host_part *h;
	ഘടന prot_space_data *space, *space_alloc;
	അക്ഷരം *realm_alloc, *auth_alloc;
	bool match;

	പ്രതിജ്ഞാ(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	എങ്കിൽ (!p)
		തിരികെ;

	/* Search for host_part */
	വേണ്ടി (pi = p; pi->parent != ശൂന്യ; pi = pi->parent)
		;
	h = (ഘടന host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	വേണ്ടി (space = h->prot_space; space; space = space->next) {
		എങ്കിൽ (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			നിർത്തുക;
	}

	എങ്കിൽ (space != ശൂന്യ) {
		/* Overrule existing auth. */
		സ്വതന്ത്രം(space->auth);
		space->auth = strdup(auth);
	} അല്ലെങ്കിൽ {
		/* Create a new protection space. */
		space = space_alloc = malloc(വലുപ്പം(ഘടന prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		എങ്കിൽ (!space_alloc || !realm_alloc || !auth_alloc) {
			സ്വതന്ത്രം(space_alloc);
			സ്വതന്ത്രം(realm_alloc);
			സ്വതന്ത്രം(auth_alloc);
			തിരികെ;
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
സ്ഥിരം അക്ഷരം *urldb_get_auth_details(nsurl *url, സ്ഥിരം അക്ഷരം *realm)
{
	ഘടന path_data *p, *p_cur, *p_മുകൾ;

	പ്രതിജ്ഞാ(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ ശൂന്യ;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	വേണ്ടി (p_cur = p; p_cur != ശൂന്യ; p_മുകൾ = p_cur, p_cur = p_cur->parent) {
		എങ്കിൽ (p_cur->prot_space) {
			തിരികെ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	എങ്കിൽ (realm != ശൂന്യ) {
		സ്ഥിരം ഘടന host_part *h = (സ്ഥിരം ഘടന host_part *)p_മുകൾ;
		സ്ഥിരം ഘടന prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		വേണ്ടി (space = h->prot_space; space != ശൂന്യ;
		     space = space->next) {
			എങ്കിൽ (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				തിരികെ p->prot_space->auth;
			}
		}
	}

	തിരികെ ശൂന്യ;
}


/* exported interface documented in netsurf/url_db.h */
ശൂന്യം urldb_set_cert_permissions(nsurl *url, bool permit)
{
	ഘടന path_data *p;
	ഘടന host_part *h;

	പ്രതിജ്ഞാ(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ;

	വേണ്ടി (; p && p->parent; p = p->parent)
		/* do nothing */;
	പ്രതിജ്ഞാ(p);

	h = (ഘടന host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ഘടന path_data *p;
	സ്ഥിരം ഘടന host_part *h;

	പ്രതിജ്ഞാ(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ false;

	വേണ്ടി (; p && p->parent; p = p->parent)
		/* do nothing */;
	പ്രതിജ്ഞാ(p);

	h = (സ്ഥിരം ഘടന host_part *)p;

	തിരികെ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(ഘടന nsurl *url, സ്ഥിരം അക്ഷരം *header)
{
	ഘടന path_data *p;
	ഘടന host_part *h;
	lwc_string *host;
	time_t now = time(ശൂന്യ);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	പ്രതിജ്ഞാ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (host != ശൂന്യ) {
		എങ്കിൽ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			തിരികെ true;
		} അല്ലെങ്കിൽ എങ്കിൽ (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			തിരികെ true;
		}

		lwc_string_unref(host);
	} അല്ലെങ്കിൽ {
		/* No host part: ignore */
		തിരികെ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ false;

	വേണ്ടി (; p && p->parent; p = p->parent)
		/* do nothing */;
	പ്രതിജ്ഞാ(p);

	h = (ഘടന host_part *)p;
	എങ്കിൽ (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		തിരികെ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	എങ്കിൽ (error != NSERROR_OK) {
		/* Parse failed: ignore */
		തിരികെ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	എങ്കിൽ (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} അല്ലെങ്കിൽ എങ്കിൽ ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	തിരികെ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ഘടന nsurl *url)
{
	ഘടന path_data *p;
	സ്ഥിരം ഘടന host_part *h;
	lwc_string *host;
	time_t now = time(ശൂന്യ);

	പ്രതിജ്ഞാ(url);

	host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (host != ശൂന്യ) {
		എങ്കിൽ (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			തിരികെ false;
		} അല്ലെങ്കിൽ എങ്കിൽ (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			തിരികെ false;
		}

		lwc_string_unref(host);
	} അല്ലെങ്കിൽ {
		/* No host part: not enabled */
		തിരികെ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ false;

	വേണ്ടി (; p && p->parent; p = p->parent)
		/* do nothing */;
	പ്രതിജ്ഞാ(p);

	h = (സ്ഥിരം ഘടന host_part *)p;

	/* Consult record for this host */
	എങ്കിൽ (h->hsts.expires > now) {
		/* Not expired */
		തിരികെ true;
	}

	/* Consult parent domains */
	വേണ്ടി (h = h->parent; h && h != &db_വേര്; h = h->parent) {
		എങ്കിൽ (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			തിരികെ true;
		}
	}

	തിരികെ false;
}


/* exported interface documented in netsurf/url_db.h */
ശൂന്യം
urldb_iterate_partial(സ്ഥിരം അക്ഷരം *prefix,
		      bool (*callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data))
{
	അക്ഷരം host[256];
	അക്ഷരം buf[260]; /* max domain + "www." */
	സ്ഥിരം അക്ഷരം *slash, *scheme_sep;
	ഘടന തിരയൽ_കണ്ണി *വൃക്ഷം;
	സ്ഥിരം ഘടന host_part *h;

	പ്രതിജ്ഞാ(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	എങ്കിൽ (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	വൃക്ഷം = urldb_get_തിരയൽ_വൃക്ഷം(prefix);

	എങ്കിൽ (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, വലുപ്പം host, "%.*s",
			 (പൂർണ്ണസംഖ്യ) (slash - prefix), prefix);

		h = urldb_തിരയൽ_find(വൃക്ഷം, host);
		എങ്കിൽ (!h) {
			പൂർണ്ണസംഖ്യ len = slash - prefix;

			എങ്കിൽ (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, വലുപ്പം buf, "www.%s", host);
				h = urldb_തിരയൽ_find(
					തിരയൽ_trees[ST_DN + 'w' - 'a'],
					buf);
				എങ്കിൽ (!h)
					തിരികെ;
			} അല്ലെങ്കിൽ
				തിരികെ;
		}

		എങ്കിൽ (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ len = strlen(prefix);

		/* looking for hosts */
		എങ്കിൽ (!urldb_iterate_partial_host(വൃക്ഷം, prefix, callback))
			തിരികെ;

		എങ്കിൽ (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, വലുപ്പം buf, "www.%s", prefix);
			എങ്കിൽ(!urldb_iterate_partial_host(
				   തിരയൽ_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				തിരികെ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
ശൂന്യം
urldb_iterate_entries(bool (*callback)(nsurl *url, സ്ഥിരം ഘടന url_data *data))
{
	പൂർണ്ണസംഖ്യ i;

	പ്രതിജ്ഞാ(callback);

	വേണ്ടി (i = 0; i < അക്കം_തിരയൽ_TREES; i++) {
		എങ്കിൽ (!urldb_iterate_entries_host(തിരയൽ_trees[i],
						callback,
						ശൂന്യ)) {
			നിർത്തുക;
		}
	}
}


/* exported interface documented in content/urldb.h */
ശൂന്യം urldb_iterate_cookies(bool (*callback)(സ്ഥിരം ഘടന cookie_data *data))
{
	പൂർണ്ണസംഖ്യ i;

	പ്രതിജ്ഞാ(callback);

	വേണ്ടി (i = 0; i < അക്കം_തിരയൽ_TREES; i++) {
		എങ്കിൽ (!urldb_iterate_entries_host(തിരയൽ_trees[i],
						ശൂന്യ, callback))
			നിർത്തുക;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(സ്ഥിരം അക്ഷരം *header, nsurl *url, nsurl *referer)
{
	സ്ഥിരം അക്ഷരം *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	പ്രതിജ്ഞാ(url && header);

	/* Get defragmented URL, as 'urlt' */
	എങ്കിൽ (nsurl_defragment(url, &urlt) != NSERROR_OK)
		തിരികെ ശൂന്യ;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	എങ്കിൽ (scheme == ശൂന്യ) {
		nsurl_unref(urlt);
		തിരികെ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	എങ്കിൽ (path == ശൂന്യ) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		തിരികെ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (host == ശൂന്യ) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		തിരികെ false;
	}

	എങ്കിൽ (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		എങ്കിൽ (rhost == ശൂന്യ) {
			പോകുക error;
		}

		/* Domain match host names */
		എങ്കിൽ (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			സ്ഥിരം അക്ഷരം *hptr;
			സ്ഥിരം അക്ഷരം *rptr;
			സ്ഥിരം അക്ഷരം *dot;
			സ്ഥിരം അക്ഷരം *host_data = lwc_string_data(host);
			സ്ഥിരം അക്ഷരം *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			എങ്കിൽ (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				പോകുക error;
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
			ആകുമ്പോൾ (hptr >= host_data && rptr >= rhost_data) {
				എങ്കിൽ (*hptr != *rptr)
					നിർത്തുക;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ആകുമ്പോൾ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			എങ്കിൽ (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ശൂന്യ ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				പോകുക error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	ചെയ്യുക {
		ഘടന cookie_internal_data *c;
		അക്ഷരം *dot;
		size_t len;
#ifdef WITH_NSPSL
		സ്ഥിരം അക്ഷരം *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		എങ്കിൽ (!c) {
			/* failed => stop parsing */
			പോകുക error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		എങ്കിൽ (!c->name || !c->മൂല്യം) {
			urldb_free_cookie(c);
			പോകുക error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		എങ്കിൽ (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			പോകുക error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		എങ്കിൽ (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		എങ്കിൽ (suffix == ശൂന്യ) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			പോകുക error;
		}
#അല്ലെങ്കിൽ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		എങ്കിൽ (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			പോകുക error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		എങ്കിൽ (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			പൂർണ്ണസംഖ്യ hlen, dlen;
			അക്ഷരം *domain = c->domain;

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
			പ്രതിജ്ഞാ(c->domain[0] == '.');

			/* 4.3.2:iii */
			എങ്കിൽ (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				പോകുക error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			എങ്കിൽ (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				പോകുക error;
			}

			എങ്കിൽ (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			എങ്കിൽ (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				പോകുക error;
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
		എങ്കിൽ (!urldb_തിരുകുക_cookie(c, scheme, urlt))
			പോകുക error;
	} ആകുമ്പോൾ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	തിരികെ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	തിരികെ false;
}


/* exported interface documented in content/urldb.h */
അക്ഷരം *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	സ്ഥിരം ഘടന path_data *p, *q;
	സ്ഥിരം ഘടന host_part *h;
	lwc_string *path_lwc;
	ഘടന cookie_internal_data *c;
	പൂർണ്ണസംഖ്യ എണ്ണം = 0, version = COOKIE_RFC2965;
	ഘടന cookie_internal_data **matched_cookies;
	പൂർണ്ണസംഖ്യ matched_cookies_size = 20;
	സ്ഥിരം അക്ഷരം *path;
	അക്ഷരം *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	പ്രതിജ്ഞാ(url != ശൂന്യ);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	എങ്കിൽ (!p)
		തിരികെ ശൂന്യ;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 വലുപ്പം(ഘടന cookie_internal_data *));
	എങ്കിൽ (!matched_cookies)
		തിരികെ ശൂന്യ;

#define GROW_MATCHED_COOKIES						\
	ചെയ്യുക {								\
		എങ്കിൽ (എണ്ണം == matched_cookies_size) {			\
			ഘടന cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       വലുപ്പം(ഘടന cookie_internal_data *)); \
									\
			എങ്കിൽ (temp == ശൂന്യ) {				\
				സ്വതന്ത്രം(matched_cookies);			\
				തിരികെ ശൂന്യ;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ആകുമ്പോൾ(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	എങ്കിൽ (path_lwc == ശൂന്യ) {
		സ്വതന്ത്രം(matched_cookies);
		തിരികെ ശൂന്യ;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(ശൂന്യ);

	എങ്കിൽ (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		വേണ്ടി (q = p->parent->children; q; q = q->next) {
			എങ്കിൽ (strcmp(q->segment, p->segment))
				തുടരുക;

			/* Consider all cookies associated with
			 * this exact path */
			വേണ്ടി (c = q->cookies; c; c = c->next) {
				എങ്കിൽ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					തുടരുക;

				എങ്കിൽ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					തുടരുക;

				എങ്കിൽ (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					തുടരുക;

				matched_cookies[എണ്ണം++] = c;

				GROW_MATCHED_COOKIES;

				എങ്കിൽ (c->version < (unsigned പൂർണ്ണസംഖ്യ)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ഘടന cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	വേണ്ടി (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		വേണ്ടി (q = p->children; q; q = q->next) {
			എങ്കിൽ (*(q->segment) != '\0')
				തുടരുക;

			വേണ്ടി (c = q->cookies; c; c = c->next) {
				എങ്കിൽ (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					തുടരുക;

				എങ്കിൽ (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					തുടരുക;

				matched_cookies[എണ്ണം++] = c;

				GROW_MATCHED_COOKIES;

				എങ്കിൽ (c->version < (unsigned പൂർണ്ണസംഖ്യ) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ഘടന cookie_data *)c);
			}
		}

		എങ്കിൽ (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			നിർത്തുക;
		}

		/* Consider p itself - may be the result of Path=/foo */
		വേണ്ടി (c = p->cookies; c; c = c->next) {
			എങ്കിൽ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				തുടരുക;

			/* Ensure cookie path is a prefix of the resource */
			എങ്കിൽ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				തുടരുക;

			എങ്കിൽ (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				തുടരുക;

			matched_cookies[എണ്ണം++] = c;

			GROW_MATCHED_COOKIES;

			എങ്കിൽ (c->version < (unsigned പൂർണ്ണസംഖ്യ) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ഘടന cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	വേണ്ടി (h = (സ്ഥിരം ഘടന host_part *)p; h && h != &db_വേര്;
	     h = h->parent) {
		വേണ്ടി (c = h->paths.cookies; c; c = c->next) {
			എങ്കിൽ (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				തുടരുക;

			/* Ensure cookie path is a prefix of the resource */
			എങ്കിൽ (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				തുടരുക;

			എങ്കിൽ (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				തുടരുക;

			matched_cookies[എണ്ണം++] = c;

			GROW_MATCHED_COOKIES;

			എങ്കിൽ (c->version < (unsigned പൂർണ്ണസംഖ്യ)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ഘടന cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	എങ്കിൽ (എണ്ണം == 0) {
		/* No cookies found */
		സ്വതന്ത്രം(matched_cookies);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, എണ്ണം);
	} അല്ലെങ്കിൽ {
		ret = get_rfc2965_cookie(matched_cookies, എണ്ണം, version);
	}
	സ്വതന്ത്രം(matched_cookies);

	തിരികെ ret;
}


/* exported interface documented in content/urldb.h */
ശൂന്യം urldb_delete_cookie(സ്ഥിരം അക്ഷരം *domain, സ്ഥിരം അക്ഷരം *path,
			 സ്ഥിരം അക്ഷരം *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_വേര്);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(സ്ഥിരം അക്ഷരം *filename)
{
	FILE *fp;
	അക്ഷരം s[16*1024];

	പ്രതിജ്ഞാ(filename);

	fp = fopen(filename, "r");
	എങ്കിൽ (!fp)
		തിരികെ NSERROR_NOT_FOUND;

#define FIND_T {				\
		വേണ്ടി (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		എങ്കിൽ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			തുടരുക;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		വേണ്ടി (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		എങ്കിൽ (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			തുടരുക;		\
		}				\
	}

	ആകുമ്പോൾ (fgets(s, വലുപ്പം s, fp)) {
		അക്ഷരം *p = s, *end = 0,
			*domain, *path, *name, *മൂല്യം, *scheme, *url,
			*comment;
		പൂർണ്ണസംഖ്യ version, domain_specified, path_specified,
			secure, http_only, no_destroy, മൂല്യം_quoted;
		time_t expires, last_used;
		ഘടന cookie_internal_data *c;

		എങ്കിൽ(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			തുടരുക;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		എങ്കിൽ (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			എങ്കിൽ (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				നിർത്തുക;
			}

			തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			തുടരുക;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		എങ്കിൽ (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} അല്ലെങ്കിൽ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; മൂല്യം = p; FIND_T;
		എങ്കിൽ (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	മൂല്യം_quoted = atoi(p); FIND_T;
		} അല്ലെങ്കിൽ {
			മൂല്യം_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		വേണ്ടി (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		പ്രതിജ്ഞാ(p <= end);

		/* Now create cookie */
		c = malloc(വലുപ്പം(ഘടന cookie_internal_data));
		എങ്കിൽ (!c)
			നിർത്തുക;

		c->name = strdup(name);
		c->മൂല്യം = strdup(മൂല്യം);
		c->മൂല്യം_was_quoted = മൂല്യം_quoted;
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

		എങ്കിൽ (!(c->name && c->മൂല്യം && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			നിർത്തുക;
		}

		എങ്കിൽ (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ശൂന്യ;
			nsurl *url_nsurl = ശൂന്യ;

			പ്രതിജ്ഞാ(scheme[0] != 'u');

			എങ്കിൽ (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				നിർത്തുക;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			എങ്കിൽ (!urldb_തിരുകുക_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				നിർത്തുക;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} അല്ലെങ്കിൽ {
			എങ്കിൽ (!urldb_തിരുകുക_cookie(c, ശൂന്യ, ശൂന്യ)) {
				/* Cookie freed for us */
				നിർത്തുക;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	തിരികെ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(സ്ഥിരം അക്ഷരം *filename)
{
	FILE *fp;
	പൂർണ്ണസംഖ്യ cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	പ്രതിജ്ഞാ(filename);

	fp = fopen(filename, "w");
	എങ്കിൽ (!fp)
		തിരികെ NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_വേര്);

	fclose(fp);

	തിരികെ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
ശൂന്യം urldb_dump(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;

	urldb_dump_hosts(&db_വേര്);

	വേണ്ടി (i = 0; i != അക്കം_തിരയൽ_TREES; i++) {
		urldb_dump_തിരയൽ(തിരയൽ_trees[i], 0);
	}
}
