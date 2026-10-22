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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ifdef WITH_NSPSL
#ଅନ୍ତର୍ଭାଵ <nspsl.h>
#endif

#ଅନ୍ତର୍ଭାଵ "utils/inet.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/url.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/bloom.h"
#ଅନ୍ତର୍ଭାଵ "utils/time.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "desktop/cookie_manager.h"

#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
ସଂରଚନା cookie_internal_data {
	ସଂରଚନା cookie_internal_data *prev;	/**< Previous in list */
	ସଂରଚନା cookie_internal_data *next;	/**< Next in list */

	ଅକ୍ଷର *name;		/**< Cookie name */
	ଅକ୍ଷର *ମୂଲ୍ୟ;		/**< Cookie value */
	bool ମୂଲ୍ୟ_was_quoted;	/**< Value was quoted in Set-Cookie: */
	ଅକ୍ଷର *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	ଅକ୍ଷର *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	ଅକ୍ଷର *path;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	ଗଣନା cookie_version version;	/**< Specification compliance */
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
ସଂରଚନା prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port;
	/** Protection realm */
	ଅକ୍ଷର *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	ଅକ୍ଷର *auth;
	/** Next sibling */
	ସଂରଚନା prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ସଂରଚନା url_internal_data {
	ଅକ୍ଷର *title;		/**< Resource title */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା visits;	/**< Visit count */
	time_t last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ସଂରଚନା path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	ଅକ୍ଷର *segment;		/**< Path segment for this node */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା frag_cnt;	/**< Number of entries in path_data::fragment */
	ଅକ୍ଷର **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ସଂରଚନା url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ଅଚଳ ସଂରଚନା prot_space_data *prot_space;
	/** Cookies associated with resource */
	ସଂରଚନା cookie_internal_data *cookies;
	/** Last cookie in list */
	ସଂରଚନା cookie_internal_data *cookies_end;

	ସଂରଚନା path_data *next;	/**< Next sibling */
	ସଂରଚନା path_data *prev;	/**< Previous sibling */
	ସଂରଚନା path_data *parent; /**< Parent path segment */
	ସଂରଚନା path_data *children; /**< Child path segments */
	ସଂରଚନା path_data *last; /**< Last child */
};

ସଂରଚନା hsts_data {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ସଂରଚନା host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ସଂରଚନା path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ସଂରଚନା hsts_data hsts;

	/**
	 * Part of host string
	 */
	ଅକ୍ଷର *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ସଂରଚନା prot_space_data *prot_space;

	ସଂରଚନା host_part *next;	/**< Next sibling */
	ସଂରଚନା host_part *prev;	/**< Previous sibling */
	ସଂରଚନା host_part *parent; /**< Parent host part */
	ସଂରଚନା host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି {
	ଅଚଳ ସଂରଚନା host_part *data;	/**< Host tree entry */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା level;		/**< Node level */

	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ବାମ;	/**< Left subtree */
	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ଡାହାଣ;	/**< Right subtree */
};

/** Root database handle */
ସ୍ଥିର ସଂରଚନା host_part db_ମୂଳ;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define ଅଙ୍କ_ସନ୍ଧାନ_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି empty = { 0, 0, &empty, &empty };
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ସନ୍ଧାନ_trees[ଅଙ୍କ_ସନ୍ଧାନ_TREES] = {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loaded_cookie_file_version;

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
ସ୍ଥିର ସଂରଚନା bloom_filter *url_bloom;
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
ସ୍ଥିର nserror urldb_write_timet(FILE *fp, time_t val)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା use;
	ଅକ୍ଷର op[32];

	use = nsc_sntimet(op, 32, &val);
	ଯଦି (use == 0) {
		fprintf(fp, "%i\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)val);
	} ଅନ୍ୟଥା {
		fprintf(fp, "%.*s\n", use, op);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର ଶୂନ୍ୟ
urldb_write_paths(ଅଚଳ ସଂରଚନା path_data *parent,
		  ଅଚଳ ଅକ୍ଷର *host,
		  FILE *fp,
		  ଅକ୍ଷର **path,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *path_alloc,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *path_used,
		  time_t expiry)
{
	ଅଚଳ ସଂରଚନା path_data *p = parent;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	କର {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା seglen = p->segment != ଖାଲି ? strlen(p->segment) : 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = *path_used + seglen + 1;

		ଯଦି (*path_alloc < len) {
			ଅକ୍ଷର *temp;
			temp = realloc(*path,
				       (len > 64) ? len : *path_alloc + 64);
			ଯଦି (!temp) {
				ଫେରନ୍ତୁ;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		ଯଦି (p->segment != ଖାଲି) {
			memcpy(*path + *path_used - 1, p->segment, seglen);
		}

		ଯଦି (p->children != ଖାଲି) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} ଅନ୍ୟଥା {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		ଯଦି (p->children != ଖାଲି) {
			/* Drill down into children */
			p = p->children;
		} ଅନ୍ୟଥା {
			/* leaf node */
			ଯଦି (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_data(p->scheme));

				ଯଦି (p->port) {
					fprintf(fp,"%d\n", p->port);
				} ଅନ୍ୟଥା {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				fprintf(fp, "%i\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->urld.type);

				fprintf(fp, "\n");

				ଯଦି (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					ପାଇଁ (i = 0; s[i] != '\0'; i++)
						ଯଦି (s[i] < 32)
							s[i] = ' ';
					ପାଇଁ (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ଅନ୍ୟଥା {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			ଯେତେବେଳେ (p != parent) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା seglen = p->segment != ଖାଲି
					? strlen(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				ଯଦି (p->next != ଖାଲି) {
					/* Have a sibling, process that */
					p = p->next;
					ଭାଙ୍ଗ;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ସ୍ଥିର ଶୂନ୍ୟ
urldb_ଗଣନା_urls(ଅଚଳ ସଂରଚନା path_data *ମୂଳ,
		 time_t expiry,
		 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଗଣନା_2)
{
	ଅଚଳ ସଂରଚନା path_data *p = ମୂଳ;

	କର {
		ଯଦି (p->children != ଖାଲି) {
			/* Drill down into children */
			p = p->children;
		} ଅନ୍ୟଥା {
			/* No more children, increment count if required */
			ଯଦି (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*ଗଣନା_2)++;
			}

			/* Now, find next node to process. */
			ଯେତେବେଳେ (p != ମୂଳ) {
				ଯଦି (p->next != ଖାଲି) {
					/* Have a sibling, process that */
					p = p->next;
					ଭାଙ୍ଗ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != ମୂଳ);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_save_ସନ୍ଧାନ_ବୃକ୍ଷ(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *parent, FILE *fp)
{
	ଅକ୍ଷର host[256];
	ଅଚଳ ସଂରଚନା host_part *h;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା path_ଗଣନା = 0;
	ଅକ୍ଷର *path, *p, *end;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା path_alloc = 64, path_used = 1;
	time_t expiry, hsts_expiry = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hsts_include_subdomains = 0;

	expiry = time(ଖାଲି) - ((60 * 60 * 24) * nsoption_int(expire_url));

	ଯଦି (parent == &empty)
		ଫେରନ୍ତୁ;

	urldb_save_ସନ୍ଧାନ_ବୃକ୍ଷ(parent->ବାମ, fp);

	path = malloc(path_alloc);
	ଯଦି (!path)
		ଫେରନ୍ତୁ;

	path[0] = '\0';

	ପାଇଁ (h = parent->data, p = host, end = host + ଆକାର host;
	     h && h != &db_ମୂଳ && p < end; h = h->parent) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		ଯଦି (written < 0) {
			ମୁକ୍ତ(path);
			ଫେରନ୍ତୁ;
		}
		p += written;
	}

	h = parent->data;
	ଯଦି (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_ଗଣନା_urls(&parent->data->paths, expiry, &path_ଗଣନା);

	ଯଦି (path_ଗଣନା > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", path_ଗଣନା);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} ଅନ୍ୟଥା ଯଦି (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	ମୁକ୍ତ(path);

	urldb_save_ସନ୍ଧାନ_ବୃକ୍ଷ(parent->ଡାହାଣ, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ସ୍ଥିର bool
urldb_iterate_entries_path(ଅଚଳ ସଂରଚନା path_data *parent,
		bool (*url_callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data),
		bool (*cookie_callback)(ଅଚଳ ସଂରଚନା cookie_data *data))
{
	ଅଚଳ ସଂରଚନା path_data *p = parent;
	ଅଚଳ ସଂରଚନା cookie_data *c;

	କର {
		ଯଦି (p->children != ଖାଲି) {
			/* Drill down into children */
			p = p->children;
		} ଅନ୍ୟଥା {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			ପ୍ରତିଜ୍ଞା(url_callback || cookie_callback);

			/** \todo handle fragments? */
			ଯଦି (url_callback) {
				ଅଚଳ ସଂରଚନା url_internal_data *u = &p->urld;

				ପ୍ରତିଜ୍ଞା(p->url);

				ଯଦି (!url_callback(p->url,
						  (ଅଚଳ ସଂରଚନା url_data *) u))
					ଫେରନ୍ତୁ false;
			} ଅନ୍ୟଥା {
				c = (ଅଚଳ ସଂରଚନା cookie_data *)p->cookies;
				ପାଇଁ (; c != ଖାଲି; c = c->next) {
					ଯଦି (!cookie_callback(c))
						ଫେରନ୍ତୁ false;
				}
			}

			/* Now, find next node to process. */
			ଯେତେବେଳେ (p != parent) {
				ଯଦି (p->next != ଖାଲି) {
					/* Have a sibling, process that */
					p = p->next;
					ଭାଙ୍ଗ;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != parent);

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool urldb__host_is_ip_address(ଅଚଳ ଅକ୍ଷର *host)
{
	ସଂରଚନା in_addr ipv4;
	size_t host_len = strlen(host);
	ଅଚଳ ଅକ୍ଷର *sane_host;
	ଅଚଳ ଅକ୍ଷର *slash;
#ifndef NO_IPV6
	ସଂରଚନା in6_addr ipv6;
	ଅକ୍ଷର ipv6_addr[64];
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ipv6_addr_len;
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
	ଯଦି (slash == ଖାଲି) {
		sane_host = host;
	} ଅନ୍ୟଥା {
		ଅକ୍ଷର *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	ଯଦି (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		ଯାଅ out_false;

	ଯଦି (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_dots = 0;
		size_t index;

		ପାଇଁ (index = 0; index < host_len; index++) {
			ଯଦି (sane_host[index] == '.')
				ଅଙ୍କ_dots++;
		}

		ଯଦି (ଅଙ୍କ_dots == 3)
			ଯାଅ out_true;
		ଅନ୍ୟଥା
			ଯାଅ out_false;
	}

#ifndef NO_IPV6
	ଯଦି ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		ଯାଅ out_false;
	}

	ipv6_addr_len = host_len - 2;
	ଯଦି (ipv6_addr_len >= ଆକାର(ipv6_addr)) {
		ipv6_addr_len = ଆକାର(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	ଯଦି (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		ଯାଅ out_true;
#endif

out_false:
	ଯଦି (slash != ଖାଲି) ମୁକ୍ତ((ଶୂନ୍ୟ *)sane_host);
	ଫେରନ୍ତୁ false;

out_true:
	ଯଦି (slash != ଖାଲି) ମୁକ୍ତ((ଶୂନ୍ୟ *)sane_host);
	ଫେରନ୍ତୁ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା urldb_ସନ୍ଧାନ_match_prefix(ଅଚଳ ସଂରଚନା host_part *a, ଅଚଳ ଅକ୍ଷର *b)
{
	ଅଚଳ ଅକ୍ଷର *end, *dot;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା plen, ret;

	ପ୍ରତିଜ୍ଞା(a && a != &db_ମୂଳ && b);

	ଯଦି (urldb__host_is_ip_address(b)) {
		/* IP address */
		ଫେରନ୍ତୁ strncasecmp(a->part, b, strlen(b));
	}

	end = b + strlen(b) + 1;

	ଯେତେବେଳେ (b < end && a && a != &db_ମୂଳ) {
		dot = strchr(b, '.');
		ଯଦି (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ଯଦି ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ଫେରନ୍ତୁ ret;

		/* The strings matched */
		ଯଦି (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = strlen(a->part);
			ଯଦି (plen > dot - b) {
				/* len(a) > len(b) */
				ଫେରନ୍ତୁ 1;
			} ଅନ୍ୟଥା ଯଦି (plen < dot - b) {
				/* len(a) < len(b) */
				ଫେରନ୍ତୁ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ଯଦି (a && a != &db_ମୂଳ && b >= end) {
		/* len(a) > len(b) => prefix matches */
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା ଯଦି ((!a || a == &db_ମୂଳ) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		ଫେରନ୍ତୁ -1;
	}

	/* Identical */
	ଫେରନ୍ତୁ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ସ୍ଥିର bool
urldb_iterate_partial_host(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ,
		ଅଚଳ ଅକ୍ଷର *prefix,
		bool (*callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;

	ପ୍ରତିଜ୍ଞା(ମୂଳ && prefix && callback);

	ଯଦି (ମୂଳ == &empty)
		ଫେରନ୍ତୁ true;

	c = urldb_ସନ୍ଧାନ_match_prefix(ମୂଳ->data, prefix);

	ଯଦି (c > 0) {
		/* No match => look in left subtree */
		ଫେରନ୍ତୁ urldb_iterate_partial_host(ମୂଳ->ବାମ,
						  prefix,
						  callback);
	} ଅନ୍ୟଥା ଯଦି (c < 0) {
		/* No match => look in right subtree */
		ଫେରନ୍ତୁ urldb_iterate_partial_host(ମୂଳ->ଡାହାଣ,
						  prefix,
						  callback);
	} ଅନ୍ୟଥା {
		/* Match => iterate over l/r subtrees & process this node */
		ଯଦି (!urldb_iterate_partial_host(ମୂଳ->ବାମ,
						prefix,
						callback)) {
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (ମୂଳ->data->paths.children) {
			/* and extract all paths attached to this host */
			ଯଦି (!urldb_iterate_entries_path(&ମୂଳ->data->paths,
							callback,
							ଖାଲି)) {
				ଫେରନ୍ତୁ false;
			}
		}

		ଯଦି (!urldb_iterate_partial_host(ମୂଳ->ଡାହାଣ,
						prefix,
						callback)) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
urldb_iterate_partial_path(ଅଚଳ ସଂରଚନା path_data *parent,
		ଅଚଳ ଅକ୍ଷର *prefix,
		bool (*callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data))
{
	ଅଚଳ ସଂରଚନା path_data *p = parent->children;
	ଅଚଳ ଅକ୍ଷର *slash, *end = prefix + strlen(prefix);

	କର {
		slash = strchr(prefix, '/');
		ଯଦି (!slash) {
			slash = end;
		}

		ଯଦି (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			ଜାରି;
		}

		ଯଦି (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			ଯଦି (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				ଯଦି (!urldb_iterate_entries_path(p,
								callback,
								ଖାଲି)) {
					ଫେରନ୍ତୁ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} ଅନ୍ୟଥା {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} ଅନ୍ୟଥା {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} ଯେତେବେଳେ (p != ଖାଲି);

	ଫେରନ୍ତୁ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ସ୍ଥିର bool
urldb_iterate_entries_host(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *parent,
		bool (*url_callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data),
		bool (*cookie_callback)(ଅଚଳ ସଂରଚନା cookie_data *data))
{
	ଯଦି (parent == &empty) {
		ଫେରନ୍ତୁ true;
	}

	ଯଦି (!urldb_iterate_entries_host(parent->ବାମ,
					url_callback,
					cookie_callback)) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		ଯଦି (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଯଦି (!urldb_iterate_entries_host(parent->ଡାହାଣ,
					url_callback,
					cookie_callback)) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ସ୍ଥିର ସଂରଚନା host_part *
urldb_add_host_ଗଣ୍ଠି(ଅଚଳ ଅକ୍ଷର *part, ସଂରଚନା host_part *parent)
{
	ସଂରଚନା host_part *d;

	ପ୍ରତିଜ୍ଞା(part && parent);

	d = calloc(1, ଆକାର(ସଂରଚନା host_part));
	ଯଦି (!d) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	d->part = strdup(part);
	ଯଦି (!d->part) {
		ମୁକ୍ତ(d);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	d->next = parent->children;
	ଯଦି (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	ଫେରନ୍ତୁ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା urldb_add_path_fragment_cmp(ଅଚଳ ଶୂନ୍ୟ *a, ଅଚଳ ଶୂନ୍ୟ *b)
{
	ଫେରନ୍ତୁ strcasecmp(*((ଅଚଳ ଅକ୍ଷର **) a), *((ଅଚଳ ଅକ୍ଷର **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ସ୍ଥିର ସଂରଚନା path_data *
urldb_add_path_fragment(ସଂରଚନା path_data *segment, lwc_string *fragment)
{
	ଅକ୍ଷର **temp;

	ପ୍ରତିଜ୍ଞା(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	ଯଦି (!fragment)
		ଫେରନ୍ତୁ segment;

	temp = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * ଆକାର(ଅକ୍ଷର *));
	ଯଦି (!temp)
		ଫେରନ୍ତୁ ଖାଲି;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	ଯଦି (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		ଫେରନ୍ତୁ ଖାଲି;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      ଆକାର (ଅକ୍ଷର *),
	      urldb_add_path_fragment_cmp);

	ଫେରନ୍ତୁ segment;
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
ସ୍ଥିର ସଂରଚନା path_data *
urldb_add_path_ଗଣ୍ଠି(lwc_string *scheme,
		    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port,
		    ଅଚଳ ଅକ୍ଷର *segment,
		    lwc_string *fragment,
		    ସଂରଚନା path_data *parent)
{
	ସଂରଚନା path_data *d, *e;

	ପ୍ରତିଜ୍ଞା(scheme && segment && parent);

	d = calloc(1, ଆକାର(ସଂରଚନା path_data));
	ଯଦି (!d)
		ଫେରନ୍ତୁ ଖାଲି;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	ଯଦି (!d->segment) {
		lwc_string_unref(d->scheme);
		ମୁକ୍ତ(d);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (fragment) {
		ଯଦି (!urldb_add_path_fragment(d, fragment)) {
			ମୁକ୍ତ(d->segment);
			lwc_string_unref(d->scheme);
			ମୁକ୍ତ(d);
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	ପାଇଁ (e = parent->children; e; e = e->next) {
		ଯଦି (strcmp(e->segment, d->segment) > 0)
			ଭାଙ୍ଗ;
	}

	ଯଦି (e) {
		d->prev = e->prev;
		d->next = e;
		ଯଦି (e->prev)
			e->prev->next = d;
		ଅନ୍ୟଥା
			parent->children = d;
		e->prev = d;
	} ଅନ୍ୟଥା ଯଦି (!parent->children) {
		d->prev = d->next = ଖାଲି;
		parent->children = parent->last = d;
	} ଅନ୍ୟଥା {
		d->next = ଖାଲି;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	ଫେରନ୍ତୁ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି **urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ_direct(ଅଚଳ ଅକ୍ଷର *host)
{
	ପ୍ରତିଜ୍ଞା(host);

	ଯଦି (urldb__host_is_ip_address(host)) {
		ଫେରନ୍ତୁ &ସନ୍ଧାନ_trees[ST_IP];
	} ଅନ୍ୟଥା ଯଦି (ascii_is_alpha(*host)) {
		ଫେରନ୍ତୁ &ସନ୍ଧାନ_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	ଫେରନ୍ତୁ &ସନ୍ଧାନ_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ(ଅଚଳ ଅକ୍ଷର *host)
{
	ଫେରନ୍ତୁ *urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା urldb_ସନ୍ଧାନ_match_string(ଅଚଳ ସଂରଚନା host_part *a, ଅଚଳ ଅକ୍ଷର *b)
{
	ଅଚଳ ଅକ୍ଷର *end, *dot;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା plen, ret;

	ପ୍ରତିଜ୍ଞା(a && a != &db_ମୂଳ && b);

	ଯଦି (urldb__host_is_ip_address(b)) {
		/* IP address */
		ଫେରନ୍ତୁ strcasecmp(a->part, b);
	}

	end = b + strlen(b) + 1;

	ଯେତେବେଳେ (b < end && a && a != &db_ମୂଳ) {
		dot = strchr(b, '.');
		ଯଦି (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		ଯଦି ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			ଫେରନ୍ତୁ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = strlen(a->part);

		ଯଦି (plen > dot - b) {
			/* len(a) > len(b) */
			ଫେରନ୍ତୁ 1;
		} ଅନ୍ୟଥା ଯଦି (plen < dot - b) {
			/* len(a) < len(b) */
			ଫେରନ୍ତୁ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ଯଦି (a && a != &db_ମୂଳ && b >= end) {
		/* len(a) > len(b) */
		ଫେରନ୍ତୁ 1;
	} ଅନ୍ୟଥା ଯଦି ((!a || a == &db_ମୂଳ) && b < end) {
		/* len(a) < len(b) */
		ଫେରନ୍ତୁ -1;
	}

	/* Identical */
	ଫେରନ୍ତୁ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ସ୍ଥିର ଅଚଳ ସଂରଚନା host_part *
urldb_ସନ୍ଧାନ_find(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ, ଅଚଳ ଅକ୍ଷର *host)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;

	ପ୍ରତିଜ୍ଞା(ମୂଳ && host);

	ଯଦି (ମୂଳ == &empty) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	c = urldb_ସନ୍ଧାନ_match_string(ମୂଳ->data, host);

	ଯଦି (c > 0) {
		ଫେରନ୍ତୁ urldb_ସନ୍ଧାନ_find(ମୂଳ->ବାମ, host);
	} ଅନ୍ୟଥା ଯଦି (c < 0) {
		ଫେରନ୍ତୁ urldb_ସନ୍ଧାନ_find(ମୂଳ->ଡାହାଣ, host);
	}

	ଫେରନ୍ତୁ ମୂଳ->data;
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
ସ୍ଥିର ସଂରଚନା path_data *
urldb_match_path(ଅଚଳ ସଂରଚନା path_data *parent,
		 ଅଚଳ ଅକ୍ଷର *path,
		 lwc_string *scheme,
		 unsigned short port)
{
	ଅଚଳ ସଂରଚନା path_data *p;
	ଅଚଳ ଅକ୍ଷର *slash;
	bool match;

	ପ୍ରତିଜ୍ଞା(parent != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(parent->segment == ଖାଲି);

	ଯଦି (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	ପ୍ରତିଜ୍ଞା(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	ଯେତେବେଳେ (p != ଖାଲି) {
		slash = strchr(path + 1, '/');
		ଯଦି (!slash) {
			slash = path + strlen(path);
		}

		ଯଦି (strncmp(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			ଯଦି (*slash == '\0') {
				/* Complete match */
				ଫେରନ୍ତୁ (ସଂରଚନା path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} ଅନ୍ୟଥା {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ସ୍ଥିର ସଂରଚନା path_data *urldb_find_url(nsurl *url)
{
	ଅଚଳ ସଂରଚନା host_part *h;
	ସଂରଚନା path_data *p;
	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ବୃକ୍ଷ;
	ଅକ୍ଷର *plq;
	ଅଚଳ ଅକ୍ଷର *host_str;
	lwc_string *scheme, *host, *port;
	size_t len = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port_int;
	bool match;

	ପ୍ରତିଜ୍ଞା(url);

	ଯଦି (url_bloom != ଖାଲି) {
		ଯଦି (bloom_ସନ୍ଧାନ_hash(url_bloom, nsurl_hash(url)) == false) {
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (scheme == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (host != ଖାଲି) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ଅନ୍ୟଥା ଯଦି (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ଅନ୍ୟଥା {
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ବୃକ୍ଷ = urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ(host_str);
	h = urldb_ସନ୍ଧାନ_find(ବୃକ୍ଷ, host_str);
	ଯଦି (!h) {
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* generate plq (path, leaf, query) */
	ଯଦି (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	ଯଦି (port != ଖାଲି) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ଅନ୍ୟଥା {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	ମୁକ୍ତ(plq);
	lwc_string_unref(scheme);

	ଫେରନ୍ତୁ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_dump_paths(ସଂରଚନା path_data *parent)
{
	ଅଚଳ ସଂରଚନା path_data *p = parent;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	କର {
		ଯଦି (p->segment != ଖାଲି) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			ପାଇଁ (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		ଯଦି (p->children != ଖାଲି) {
			p = p->children;
		} ଅନ୍ୟଥା {
			ଯେତେବେଳେ (p != parent) {
				ଯଦି (p->next != ଖାଲି) {
					p = p->next;
					ଭାଙ୍ଗ;
				}

				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_dump_hosts(ସଂରଚନା host_part *parent)
{
	ସଂରଚନା host_part *h;

	ଯଦି (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	ପାଇଁ (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_dump_ସନ୍ଧାନ(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth)
{
	ଅଚଳ ସଂରଚନା host_part *h;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i; /* index into string */
	ଅକ୍ଷର s[1024];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sl = ଆକାର(s) - 2;

	ଯଦି (parent == &empty)
		ଫେରନ୍ତୁ;

	urldb_dump_ସନ୍ଧାନ(parent->ବାମ, depth + 1);

	ପାଇଁ (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	ପାଇଁ (h = parent->data; h; h = h->parent) {
		ଯଦି (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			ଯଦି (r < 0) {
				ଭାଙ୍ଗ;
			}
			ଯଦି ((i + r) >= sl) {
				ଭାଙ୍ଗ;
			}
			i += r;
		}

		ଯଦି (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_ସନ୍ଧାନ(parent->ଡାହାଣ, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
urldb_ସନ୍ଧାନ_match_host(ଅଚଳ ସଂରଚନା host_part *a, ଅଚଳ ସଂରଚନା host_part *b)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ପ୍ରତିଜ୍ଞା(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	ପାଇଁ (; a && a != &db_ମୂଳ && b && b != &db_ମୂଳ;
	     a = a->parent, b = b->parent) {
		ଯଦି ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			ଫେରନ୍ତୁ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	ଯଦି (a && a != &db_ମୂଳ && (!b || b == &db_ମୂଳ)) {
		/* len(a) > len(b) */
		ଫେରନ୍ତୁ 1;
	} ଅନ୍ୟଥା ଯଦି ((!a || a == &db_ମୂଳ) && b && b != &db_ମୂଳ) {
		/* len(a) < len(b) */
		ଫେରନ୍ତୁ -1;
	}

	/* identical */
	ଫେରନ୍ତୁ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *urldb_ସନ୍ଧାନ_skew(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ)
{
	ପ୍ରତିଜ୍ଞା(ମୂଳ);

	ଯଦି (ମୂଳ->ବାମ->level == ମୂଳ->level) {
		ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *temp;

		temp = ମୂଳ->ବାମ;
		ମୂଳ->ବାମ = temp->ଡାହାଣ;
		temp->ଡାହାଣ = ମୂଳ;
		ମୂଳ = temp;
	}

	ଫେରନ୍ତୁ ମୂଳ;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *urldb_ସନ୍ଧାନ_split(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ)
{
	ପ୍ରତିଜ୍ଞା(ମୂଳ);

	ଯଦି (ମୂଳ->ଡାହାଣ->ଡାହାଣ->level == ମୂଳ->level) {
		ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *temp;

		temp = ମୂଳ->ଡାହାଣ;
		ମୂଳ->ଡାହାଣ = temp->ବାମ;
		temp->ବାମ = ମୂଳ;
		ମୂଳ = temp;

		ମୂଳ->level++;
	}

	ଫେରନ୍ତୁ ମୂଳ;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *
urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି_internal(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ, ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *n)
{
	ପ୍ରତିଜ୍ଞା(ମୂଳ && n);

	ଯଦି (ମୂଳ == &empty) {
		ମୂଳ = n;
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = urldb_ସନ୍ଧାନ_match_host(ମୂଳ->data, n->data);

		ଯଦି (c > 0) {
			ମୂଳ->ବାମ = urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି_internal(
				ମୂଳ->ବାମ, n);
		} ଅନ୍ୟଥା ଯଦି (c < 0) {
			ମୂଳ->ଡାହାଣ = urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି_internal(
				ମୂଳ->ଡାହାଣ, n);
		} ଅନ୍ୟଥା {
			/* exact match */
			ମୁକ୍ତ(n);
			ଫେରନ୍ତୁ ମୂଳ;
		}

		ମୂଳ = urldb_ସନ୍ଧାନ_skew(ମୂଳ);
		ମୂଳ = urldb_ସନ୍ଧାନ_split(ମୂଳ);
	}

	ଫେରନ୍ତୁ ମୂଳ;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ସ୍ଥିର ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *
urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ, ଅଚଳ ସଂରଚନା host_part *data)
{
	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *n;

	ପ୍ରତିଜ୍ଞା(ମୂଳ && data);

	n = malloc(ଆକାର(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି));
	ଯଦି (!n)
		ଫେରନ୍ତୁ ଖାଲି;

	n->level = 1;
	n->data = data;
	n->ବାମ = n->ଡାହାଣ = &empty;

	ମୂଳ = urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି_internal(ମୂଳ, n);

	ଫେରନ୍ତୁ ମୂଳ;
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
ସ୍ଥିର bool
urldb_parse_avpair(ସଂରଚନା cookie_internal_data *c,
		   ଅକ୍ଷର *n,
		   ଅକ୍ଷର *v,
		   bool was_quoted)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlen;

	ପ୍ରତିଜ୍ଞା(c && n && v);

	/* Strip whitespace from start of name */
	ପାଇଁ (; *n; n++) {
		ଯଦି (*n != ' ' && *n != '\t')
			ଭାଙ୍ଗ;
	}

	/* Strip whitespace from end of name */
	ପାଇଁ (vlen = strlen(n); vlen; vlen--) {
		ଯଦି (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ଅନ୍ୟଥା
			ଭାଙ୍ଗ;
	}

	/* Strip whitespace from start of value */
	ପାଇଁ (; *v; v++) {
		ଯଦି (*v != ' ' && *v != '\t')
			ଭାଙ୍ଗ;
	}

	/* Strip whitespace from end of value */
	ପାଇଁ (vlen = strlen(v); vlen; vlen--) {
		ଯଦି (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ଅନ୍ୟଥା
			ଭାଙ୍ଗ;
	}

	ଯଦି (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		ଯଦି (!c->comment)
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା ଯଦି (!c->domain && strcasecmp(n, "Domain") == 0) {
		ଯଦି (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			ଯଦି (!c->domain)
				ଫେରନ୍ତୁ false;
		}
	} ଅନ୍ୟଥା ଯଦି (strcasecmp(n, "Max-Age") == 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା temp = atoi(v);
		ଯଦି (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ଅନ୍ୟଥା
			c->expires = time(ଖାଲି) + temp;
	} ଅନ୍ୟଥା ଯଦି (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		ଯଦି (!c->path)
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା ଯଦି (strcasecmp(n, "Version") == 0) {
		c->version = atoi(v);
	} ଅନ୍ୟଥା ଯଦି (strcasecmp(n, "Expires") == 0) {
		ଅକ୍ଷର *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		ପାଇଁ (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, strlen(datenoday), &expires);
		ଯଦି (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ଅନ୍ୟଥା ଯଦି (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ଅନ୍ୟଥା ଯଦି (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ଅନ୍ୟଥା ଯଦି (!c->name) {
		c->name = strdup(n);
		c->ମୂଲ୍ୟ = strdup(v);
		c->ମୂଲ୍ୟ_was_quoted = was_quoted;
		ଯଦି (!c->name || !c->ମୂଲ୍ୟ) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_free_cookie(ସଂରଚନା cookie_internal_data *c)
{
	ପ୍ରତିଜ୍ଞା(c);

	ମୁକ୍ତ(c->comment);
	ମୁକ୍ତ(c->domain);
	ମୁକ୍ତ(c->path);
	ମୁକ୍ତ(c->name);
	ମୁକ୍ତ(c->ମୂଲ୍ୟ);
	ମୁକ୍ତ(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ସ୍ଥିର ସଂରଚନା cookie_internal_data *
urldb_parse_cookie(nsurl *url, ଅଚଳ ଅକ୍ଷର **cookie)
{
	ସଂରଚନା cookie_internal_data *c;
	ଅଚଳ ଅକ୍ଷର *cur;
	ଅକ୍ଷର name[1024], ମୂଲ୍ୟ[4096];
	ଅକ୍ଷର *n = name, *v = ମୂଲ୍ୟ;
	bool in_ମୂଲ୍ୟ = false;
	bool had_ମୂଲ୍ୟ_data = false;
	bool ମୂଲ୍ୟ_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	ପ୍ରତିଜ୍ଞା(url && cookie && *cookie);

	c = calloc(1, ଆକାର(ସଂରଚନା cookie_internal_data));
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	c->expires = -1;

	name[0] = '\0';
	ମୂଲ୍ୟ[0] = '\0';

	ପାଇଁ (cur = *cookie; *cur; cur++) {
		ଯଦି (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			ଯଦି (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				ମୂଲ୍ୟ[0] = '\0';

			}

			ଭାଙ୍ଗ;
		} ଅନ୍ୟଥା ଯଦି (*cur == '\r') {
			/* Spurious linefeed */
			ଜାରି;
		} ଅନ୍ୟଥା ଯଦି (*cur == '\n') {
			/* Spurious newline */
			ଜାରି;
		}

		ଯଦି (in_ମୂଲ୍ୟ && !had_ମୂଲ୍ୟ_data) {
			ଯଦି (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				ଜାରି;
			} ଅନ୍ୟଥା {
				had_ମୂଲ୍ୟ_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				ଯଦି (*cur != '"') {
					ମୂଲ୍ୟ_verbatim = true;
				}
			}
		}

		ଯଦି (in_ମୂଲ୍ୟ && !ମୂଲ୍ୟ_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			ଯଦି (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				ଜାରି;
			}
		}

		ଯଦି (!quoted && !in_ମୂଲ୍ୟ && *cur == '=') {
			/* First equals => attr-value separator */
			in_ମୂଲ୍ୟ = true;
			ଜାରି;
		}

		ଯଦି (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			ଯଦି (!urldb_parse_avpair(c, name, ମୂଲ୍ୟ, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				ଫେରନ୍ତୁ ଖାଲି;
			}

			/* And reset to start */
			n = name;
			v = ମୂଲ୍ୟ;
			in_ମୂଲ୍ୟ = false;
			had_ମୂଲ୍ୟ_data = false;
			ମୂଲ୍ୟ_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			ଯଦି (*cur != ';') {
				cur--;
			}

			ଜାରି;
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
		ଯଦି (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ଅଚଳ ଅକ୍ଷର *p;
			ଅଚଳ ଅକ୍ଷର *semi = strchr(cur + 1, ';');
			ଯଦି (!semi)
				semi = cur + strlen(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			ପାଇଁ (p = cur + 1; p < semi; p++)
				ଯଦି (*p == '=')
					ଭାଙ୍ଗ;

			ଯଦି (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ଅନ୍ୟଥା {
				/* found one => comma marks end of cookie */
				cur++;
				ଭାଙ୍ଗ;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		ଯଦି (!in_ମୂଲ୍ୟ) {
			ଯଦି (n < name + (ଆକାର(name) - 1))
				*n++ = *cur;
		} ଅନ୍ୟଥା {
			ଯଦି (v < ମୂଲ୍ୟ + (ଆକାର(ମୂଲ୍ୟ) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	ଯଦି (!urldb_parse_avpair(c, name, ମୂଲ୍ୟ, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Now fix-up default values */
	ଯଦି (c->domain == ଖାଲି) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		ଯଦି (host == ଖାଲି) {
			urldb_free_cookie(c);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	ଯଦି (c->path == ଖାଲି) {
		ଅଚଳ ଅକ୍ଷର *path_data;
		ଅକ୍ଷର *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		ଯଦି (path_lwc == ଖାଲି) {
			urldb_free_cookie(c);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(path_data, '/');
		ଯଦି (slash != ଖାଲି) {
			/* Special case: retain first slash in path */
			ଯଦି (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			ଯଦି (slash == ଖାଲି) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				ଫେରନ୍ତୁ ଖାଲି;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} ଅନ୍ୟଥା {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			ଯଦି (path == ଖାଲି) {
				urldb_free_cookie(c);
				ଫେରନ୍ତୁ ଖାଲି;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	ଫେରନ୍ତୁ c;
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
ସ୍ଥିର ସଂରଚନା path_data *
urldb_add_path(lwc_string *scheme,
	       unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port,
	       ଅଚଳ ସଂରଚନା host_part *host,
	       ଅକ୍ଷର *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ସଂରଚନା path_data *d, *e;
	ଅକ୍ଷର *buf = path_query;
	ଅକ୍ଷର *segment, *slash;
	bool match;

	ପ୍ରତିଜ୍ଞା(scheme && host && url);

	d = (ସଂରଚନା path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	ଯଦି (*segment == '/')
		segment++;

	/* Process path segments */
	କର {
		slash = strchr(segment, '/');
		ଯଦି (!slash) {
			/* last segment */
			/* look for existing entry */
			ପାଇଁ (e = d->children; e; e = e->next)
				ଯଦି (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					ଭାଙ୍ଗ;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_ଗଣ୍ଠି(scheme, port,
						    segment, fragment, d);
			ଭାଙ୍ଗ;
		}

		*slash = '\0';

		/* look for existing entry */
		ପାଇଁ (e = d->children; e; e = e->next)
			ଯଦି (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				ଭାଙ୍ଗ;

		d = e ? e : urldb_add_path_ଗଣ୍ଠି(scheme, port, segment, ଖାଲି, d);
		ଯଦି (!d)
			ଭାଙ୍ଗ;

		segment = slash + 1;
	} ଯେତେବେଳେ (1);

	ମୁକ୍ତ(path_query);

	ଯଦି (d && !d->url) {
		/* Insert defragmented URL */
		ଯଦି (nsurl_defragment(url, &d->url) != NSERROR_OK)
			ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ସ୍ଥିର ସଂରଚନା host_part *urldb_add_host(ଅଚଳ ଅକ୍ଷର *host)
{
	ସଂରଚନା host_part *d = (ସଂରଚନା host_part *) &db_ମୂଳ, *e;
	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *s;
	ଅକ୍ଷର buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	ଅକ୍ଷର *part;

	ପ୍ରତିଜ୍ଞା(host);

	ଯଦି (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		ପାଇଁ (e = d->children; e; e = e->next)
			ଯଦି (strcasecmp(host, e->part) == 0)
				/* found => return it */
				ଫେରନ୍ତୁ e;

		d = urldb_add_host_ଗଣ୍ଠି(host, d);

		s = urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି(ସନ୍ଧାନ_trees[ST_IP], d);
		ଯଦି (!s) {
			/* failed */
			d = ଖାଲି;
		} ଅନ୍ୟଥା {
			ସନ୍ଧାନ_trees[ST_IP] = s;
		}

		ଫେରନ୍ତୁ d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, ଆକାର buf);
	buf[ଆକାର buf - 1] = '\0';

	/* Process FQDN segments backwards */
	କର {
		part = strrchr(buf, '.');
		ଯଦି (!part) {
			/* last segment */
			/* Check for existing entry */
			ପାଇଁ (e = d->children; e; e = e->next)
				ଯଦି (strcasecmp(buf, e->part) == 0)
					ଭାଙ୍ଗ;

			ଯଦି (e) {
				d = e;
			} ଅନ୍ୟଥା {
				d = urldb_add_host_ଗଣ୍ଠି(buf, d);
			}

			/* And insert into search tree */
			ଯଦି (d) {
				ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି **r;

				r = urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ_direct(buf);
				s = urldb_ସନ୍ଧାନ_ଭର୍ତ୍ତି(*r, d);
				ଯଦି (!s) {
					/* failed */
					d = ଖାଲି;
				} ଅନ୍ୟଥା {
					*r = s;
				}
			}
			ଭାଙ୍ଗ;
		}

		/* Check for existing entry */
		ପାଇଁ (e = d->children; e; e = e->next)
			ଯଦି (strcasecmp(part + 1, e->part) == 0)
				ଭାଙ୍ଗ;

		d = e ? e : urldb_add_host_ଗଣ୍ଠି(part + 1, d);
		ଯଦି (!d)
			ଭାଙ୍ଗ;

		*part = '\0';
	} ଯେତେବେଳେ (1);

	ଫେରନ୍ତୁ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ସ୍ଥିର bool
urldb_ଭର୍ତ୍ତି_cookie(ସଂରଚନା cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ସଂରଚନା cookie_internal_data *d;
	ଅଚଳ ସଂରଚନା host_part *h;
	ସଂରଚନା path_data *p;
	time_t now = time(ଖାଲି);

	ପ୍ରତିଜ୍ଞା(c);

	ଯଦି (c->domain[0] == '.') {
		h = urldb_ସନ୍ଧାନ_find(
			urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ(&(c->domain[1])),
			c->domain + 1);
		ଯଦି (!h) {
			h = urldb_add_host(c->domain + 1);
			ଯଦି (!h) {
				urldb_free_cookie(c);
				ଫେରନ୍ତୁ false;
			}
		}

		p = (ସଂରଚନା path_data *) &h->paths;
	} ଅନ୍ୟଥା {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		ପ୍ରତିଜ୍ଞା(url != ଖାଲି);
		ପ୍ରତିଜ୍ଞା(scheme != ଖାଲି);

		h = urldb_ସନ୍ଧାନ_find(
			urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ(c->domain),
			c->domain);

		ଯଦି (!h) {
			h = urldb_add_host(c->domain);
			ଯଦି (!h) {
				urldb_free_cookie(c);
				ଫେରନ୍ତୁ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), ଖାଲି, url);
		ଯଦି (!p) {
			urldb_free_cookie(c);
			ଫେରନ୍ତୁ false;
		}
	}

	/* add cookie */
	ପାଇଁ (d = p->cookies; d; d = d->next) {
		ଯଦି (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->path, c->path) &&
		    !strcmp(d->name, c->name))
			ଭାଙ୍ଗ;
	}

	ଯଦି (d) {
		ଯଦି (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			ଯଦି (d->next)
				d->next->prev = d->prev;
			ଅନ୍ୟଥା
				p->cookies_end = d->prev;
			ଯଦି (d->prev)
				d->prev->next = d->next;
			ଅନ୍ୟଥା
				p->cookies = d->next;

			cookie_manager_remove((ସଂରଚନା cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ଅନ୍ୟଥା {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			ଯଦି (c->next)
				c->next->prev = c;
			ଅନ୍ୟଥା
				p->cookies_end = c;
			ଯଦି (c->prev)
				c->prev->next = c;
			ଅନ୍ୟଥା
				p->cookies = c;

			cookie_manager_remove((ସଂରଚନା cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((ସଂରଚନା cookie_data *)c);
		}
	} ଅନ୍ୟଥା {
		c->prev = p->cookies_end;
		c->next = ଖାଲି;
		ଯଦି (p->cookies_end)
			p->cookies_end->next = c;
		ଅନ୍ୟଥା
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((ସଂରଚନା cookie_data *)c);
	}

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
concat_netscape_cookie(ସଂରଚନା cookie_internal_data *c,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *used,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *alloc,
		    ଅକ୍ଷର **buf)
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
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା outl; /* output length */

	ପ୍ରତିଜ୍ଞା(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->ମୂଲ୍ୟ) + 2 +
		(c->path_from_set ? 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + strlen(c->domain) + 2 : 0);
	ଯଦି (max_len >= *alloc) {
		ଅକ୍ଷର *temp = realloc(*buf, max_len);
		ଯଦି (!temp) {
			ଫେରନ୍ତୁ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	ଯଦି (outl >= (*alloc - *used)) {
		/* output truncated */
		ଫେରନ୍ତୁ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	ଯଦି (c->ମୂଲ୍ୟ_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->ମୂଲ୍ୟ);
		ଯଦି (outl >= (*alloc - *used)) {
			/* output truncated */
			ଫେରନ୍ତୁ false;
		}
		*used += outl;
	} ଅନ୍ୟଥା {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->ମୂଲ୍ୟ);
		ଯଦି (outl >= (*alloc - *used)) {
			/* output truncated */
			ଫେରନ୍ତୁ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
concat_rfc2965_cookie(ସଂରଚନା cookie_internal_data *c,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା *used,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା *alloc,
		      ଅକ୍ଷର **buf)
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
	ଅଚଳ ଅକ୍ଷର * ଅଚଳ separators = "()<>@,;:\\\"/[]?={} \t";
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା outl; /* output length */

	ପ୍ରତିଜ୍ଞା(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		strlen(c->name) + 1 +
		strlen(c->ମୂଲ୍ୟ) + 2 +
		(c->path_from_set ?
		 8 + strlen(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + strlen(c->domain) + 2 : 0);

	ଯଦି (max_len >= *alloc) {
		ଅକ୍ଷର *temp = realloc(*buf, max_len);
		ଯଦି (!temp) {
			ଫେରନ୍ତୁ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	ଯଦି (outl >= (*alloc - *used)) {
		/* output truncated */
		ଫେରନ୍ତୁ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	ଯଦି (c->ମୂଲ୍ୟ_was_quoted ||
	    strpbrk(c->ମୂଲ୍ୟ, separators) != ଖାଲି) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->ମୂଲ୍ୟ);

	} ଅନ୍ୟଥା {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->ମୂଲ୍ୟ);
	}
	ଯଦି (outl >= (*alloc - *used)) {
		/* output truncated */
		ଫେରନ୍ତୁ false;
	}
	*used += outl;

	ଯଦି (c->path_from_set) {
		/* Path, quoted if necessary */

		ଯଦି (strpbrk(c->path, separators) != ଖାଲି) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} ଅନ୍ୟଥା {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		ଯଦି (outl >= (*alloc - *used)) {
			/* output truncated */
			ଫେରନ୍ତୁ false;
		}
		*used += outl;
	}

	ଯଦି (c->domain_from_set) {
		/* Domain, quoted if necessary */
		ଯଦି (strpbrk(c->domain, separators) != ଖାଲି) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} ଅନ୍ୟଥା {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		ଯଦି (outl >= (*alloc - *used)) {
			/* output truncated */
			ଫେରନ୍ତୁ false;
		}
		*used += outl;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * build output cookie string in netscape format
 */
ସ୍ଥିର ଅକ୍ଷର *
get_netscape_cookie(ସଂରଚନା cookie_internal_data **matched_cookies, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_alloc = 4096, ret_used = 1;
	ଅକ୍ଷର *ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା midx; /* index into matched cookies */

	ret = malloc(ret_alloc);
	ଯଦି (ret == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ret[0] = '\0';

	ପାଇଁ (midx = 0; midx < ଗଣନା_2; midx++) {
		ଯଦି (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			ମୁକ୍ତ(ret);
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	ଯଦି (ret_used <= 2) {
		ମୁକ୍ତ(ret);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	ଅକ୍ଷର *temp = realloc(ret, ret_used);
	ଯଦି (temp != ଖାଲି) {
		ret = temp;
	}

	ଫେରନ୍ତୁ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ସ୍ଥିର ଅକ୍ଷର *
get_rfc2965_cookie(ସଂରଚନା cookie_internal_data **matched_cookies,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା version)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_alloc = 4096; /* return string allocation */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_used = 1; /* return sting buffer used */
	ଅକ୍ଷର *ret; /* return string buffer */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା midx; /* index into matched cookies */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା outl;

	ret = malloc(ret_alloc);
	ଯଦି (ret == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	ଯଦି (outl >= ret_alloc - ret_used) {
		/* overflow */
		ମୁକ୍ତ(ret);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ret_used += outl;

	ପାଇଁ (midx = 0; midx < ଗଣନା_2; midx++) {
		ଯଦି (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			ମୁକ୍ତ(ret);
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	/* shrink the output buffer to the required size */
	ଅକ୍ଷର *temp = realloc(ret, ret_used);
	ଯଦି (temp != ଖାଲି) {
		ret = temp;
	}

	ଫେରନ୍ତୁ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ସ୍ଥିର ଶୂନ୍ୟ
urldb_delete_cookie_paths(ଅଚଳ ଅକ୍ଷର *domain,
			  ଅଚଳ ଅକ୍ଷର *path,
			  ଅଚଳ ଅକ୍ଷର *name,
			  ସଂରଚନା path_data *parent)
{
	ସଂରଚନା cookie_internal_data *c;
	ସଂରଚନା path_data *p = parent;

	ପ୍ରତିଜ୍ଞା(parent);

	କର {
		ପାଇଁ (c = p->cookies; c; c = c->next) {
			ଯଦି (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->path, path) == 0 &&
			    strcmp(c->name, name) == 0) {
				ଯଦି (c->prev) {
					c->prev->next = c->next;
				} ଅନ୍ୟଥା {
					p->cookies = c->next;
				}

				ଯଦି (c->next) {
					c->next->prev = c->prev;
				} ଅନ୍ୟଥା {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				ଫେରନ୍ତୁ;
			}
		}

		ଯଦି (p->children) {
			p = p->children;
		} ଅନ୍ୟଥା {
			ଯେତେବେଳେ (p != parent) {
				ଯଦି (p->next != ଖାଲି) {
					p = p->next;
					ଭାଙ୍ଗ;
				}

				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ସ୍ଥିର ଶୂନ୍ୟ
urldb_delete_cookie_hosts(ଅଚଳ ଅକ୍ଷର *domain,
			  ଅଚଳ ଅକ୍ଷର *path,
			  ଅଚଳ ଅକ୍ଷର *name,
			  ସଂରଚନା host_part *parent)
{
	ସଂରଚନା host_part *h;
	ପ୍ରତିଜ୍ଞା(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	ପାଇଁ (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_save_cookie_paths(FILE *fp, ସଂରଚନା path_data *parent)
{
	ସଂରଚନା path_data *p = parent;
	time_t now = time(ଖାଲି);

	ପ୍ରତିଜ୍ଞା(fp && parent);

	କର {
		ଯଦି (p->cookies != ଖାଲି) {
			ସଂରଚନା cookie_internal_data *c;

			ପାଇଁ (c = p->cookies; c != ଖାଲି; c = c->next) {
				ଯଦି (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					ଜାରି;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)c->expires, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)c->last_used,
					c->no_destroy, c->name, c->ମୂଲ୍ୟ,
					c->ମୂଲ୍ୟ_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		ଯଦି (p->children != ଖାଲି) {
			p = p->children;
		} ଅନ୍ୟଥା {
			ଯେତେବେଳେ (p != parent) {
				ଯଦି (p->next != ଖାଲି) {
					p = p->next;
					ଭାଙ୍ଗ;
				}

				p = p->parent;
			}
		}
	} ଯେତେବେଳେ (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_save_cookie_hosts(FILE *fp, ସଂରଚନା host_part *parent)
{
	ସଂରଚନା host_part *h;
	ପ୍ରତିଜ୍ଞା(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	ପାଇଁ (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_cookie(ସଂରଚନା cookie_internal_data *c)
{
	ମୁକ୍ତ(c->name);
	ମୁକ୍ତ(c->ମୂଲ୍ୟ);
	ମୁକ୍ତ(c->comment);
	ମୁକ୍ତ(c->domain);
	ମୁକ୍ତ(c->path);

	ମୁକ୍ତ(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_path_ଗଣ୍ଠି_content(ସଂରଚନା path_data *ଗଣ୍ଠି)
{
	ସଂରଚନା cookie_internal_data *a, *b;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (ଗଣ୍ଠି->url != ଖାଲି) {
		nsurl_unref(ଗଣ୍ଠି->url);
	}

	lwc_string_unref(ଗଣ୍ଠି->scheme);

	ମୁକ୍ତ(ଗଣ୍ଠି->segment);
	ପାଇଁ (i = 0; i < ଗଣ୍ଠି->frag_cnt; i++)
		ମୁକ୍ତ(ଗଣ୍ଠି->fragment[i]);
	ମୁକ୍ତ(ଗଣ୍ଠି->fragment);

	ମୁକ୍ତ(ଗଣ୍ଠି->urld.title);

	ପାଇଁ (a = ଗଣ୍ଠି->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_prot_space(ସଂରଚନା prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	ମୁକ୍ତ(space->realm);
	ମୁକ୍ତ(space->auth);

	ମୁକ୍ତ(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_path_ବୃକ୍ଷ(ସଂରଚନା path_data *ମୂଳ)
{
	ସଂରଚନା path_data *p = ମୂଳ;

	କର {
		ଯଦି (p->children != ଖାଲି) {
			p = p->children;
		} ଅନ୍ୟଥା {
			ସଂରଚନା path_data *q = p;

			ଯେତେବେଳେ (p != ମୂଳ) {
				ଯଦି (p->next != ଖାଲି) {
					p = p->next;
					ଭାଙ୍ଗ;
				}

				p = p->parent;

				urldb_destroy_path_ଗଣ୍ଠି_content(q);
				ମୁକ୍ତ(q);

				q = p;
			}

			urldb_destroy_path_ଗଣ୍ଠି_content(q);
			ମୁକ୍ତ(q);
		}
	} ଯେତେବେଳେ (p != ମୂଳ);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_host_ବୃକ୍ଷ(ସଂରଚନା host_part *ମୂଳ)
{
	ସଂରଚନା host_part *a, *b;
	ସଂରଚନା path_data *p, *q;
	ସଂରଚନା prot_space_data *s, *t;

	/* Destroy children */
	ପାଇଁ (a = ମୂଳ->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_ବୃକ୍ଷ(a);
	}

	/* Now clean up paths */
	ପାଇଁ (p = ମୂଳ->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_ବୃକ୍ଷ(p);
	}

	/* Root path */
	urldb_destroy_path_ଗଣ୍ଠି_content(&ମୂଳ->paths);

	/* Proctection space data */
	ପାଇଁ (s = ମୂଳ->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	ମୁକ୍ତ(ମୂଳ->part);
	ମୁକ୍ତ(ମୂଳ);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ସ୍ଥିର ଶୂନ୍ୟ urldb_destroy_ସନ୍ଧାନ_ବୃକ୍ଷ(ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ମୂଳ)
{
	/* Destroy children */
	ଯଦି (ମୂଳ->ବାମ != &empty)
		urldb_destroy_ସନ୍ଧାନ_ବୃକ୍ଷ(ମୂଳ->ବାମ);
	ଯଦି (ମୂଳ->ଡାହାଣ != &empty)
		urldb_destroy_ସନ୍ଧାନ_ବୃକ୍ଷ(ମୂଳ->ଡାହାଣ);

	/* And destroy ourselves */
	ମୁକ୍ତ(ମୂଳ);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
ଶୂନ୍ୟ urldb_destroy(ଶୂନ୍ୟ)
{
	ସଂରଚନା host_part *a, *b;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* Clean up search trees */
	ପାଇଁ (i = 0; i < ଅଙ୍କ_ସନ୍ଧାନ_TREES; i++) {
		ଯଦି (ସନ୍ଧାନ_trees[i] != &empty) {
			urldb_destroy_ସନ୍ଧାନ_ବୃକ୍ଷ(ସନ୍ଧାନ_trees[i]);
			ସନ୍ଧାନ_trees[i] = &empty;
		}
	}

	/* And database */
	ପାଇଁ (a = db_ମୂଳ.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_ବୃକ୍ଷ(a);
	}
	memset(&db_ମୂଳ, 0, ଆକାର(db_ମୂଳ));

	/* And the bloom filter */
	ଯଦି (url_bloom != ଖାଲି) {
		bloom_destroy(url_bloom);
		url_bloom = ଖାଲି;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(ଅଚଳ ଅକ୍ଷର *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	ଅକ୍ଷର s[MAXIMUM_URL_LENGTH];
	ଅକ୍ଷର host[256];
	ସଂରଚନା host_part *h;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା urls;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା version;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;
	FILE *fp;

	ପ୍ରତିଜ୍ଞା(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	ଯଦି (url_bloom == ଖାଲି)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = fopen(filename, "r");
	ଯଦି (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
		fclose(fp);
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;
	}

	version = atoi(s);
	ଯଦି (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}
	ଯଦି (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	ଯେତେବେଳେ (fgets(host, ଆକାର host, fp)) {
		time_t hsts_expiry = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା hsts_include_sub_domains = 0;

		/* get the hostname */
		length = strlen(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		ଯଦି (length == 0) {
			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;
			urls = atoi(s);
			/* Eight fields/url */
			ପାଇଁ (i = 0; i < (8 * urls); i++) {
				ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
					ଭାଙ୍ଗ;
			}
			ଜାରି;
		}

		ଯଦି (version >= 107) {
			ଅକ୍ଷର *p = host;
			ଯେତେବେଳେ (*p && *p != ' ') p++;
			ଯେତେବେଳେ (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			ଯେତେବେଳେ (*p && *p != ' ') p++;
			ଯେତେବେଳେ (*p && *p == ' ') p++;
			nsc_snptimet(p, strlen(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		ଯଦି (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
			ଭାଙ୍ଗ;
		urls = atoi(s);

		/* no URLs => try next host */
		ଯଦି (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			ଜାରି;
		}

		/* load the non-corrupt data */
		ପାଇଁ (i = 0; i < urls; i++) {
			ସଂରଚନା path_data *p = ଖାଲି;
			ଅକ୍ଷର scheme[64], ports[10];
			ଅକ୍ଷର url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			ଅକ୍ଷର *path_query;
			size_t len;

			ଯଦି (!fgets(scheme, ଆକାର scheme, fp))
				ଭାଙ୍ଗ;
			length = strlen(scheme) - 1;
			scheme[length] = '\0';

			ଯଦି (!fgets(ports, ଆକାର ports, fp))
				ଭାଙ୍ଗ;
			length = strlen(ports) - 1;
			ports[length] = '\0';
			port = atoi(ports);

			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;
			length = strlen(s) - 1;
			s[length] = '\0';

			ଯଦି (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, ଆକାର url, "%s://%s%s%s%s",
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
			ଯଦି (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}

			ଯଦି (url_bloom != ଖାଲି) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_ଭର୍ତ୍ତି_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			ଯଦି (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			ଯଦି (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;
			ଯଦି (p)
				p->urld.visits = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)atoi(s);

			/* entry last use time */
			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp)) {
				ଭାଙ୍ଗ;
			}
			ଯଦି (p) {
				nsc_snptimet(s, strlen(s) - 1, &p->urld.last_visit);
			}

			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;
			ଯଦି (p)
				p->urld.type = (content_type)atoi(s);

			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;


			ଯଦି (!fgets(s, MAXIMUM_URL_LENGTH, fp))
				ଭାଙ୍ଗ;
			length = strlen(s) - 1;
			ଯଦି (p && length > 0) {
				s[length] = '\0';
				p->urld.title = malloc(length + 1);
				ଯଦି (p->urld.title)
					memcpy(p->urld.title, s, length + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ଅଚଳ ଅକ୍ଷର *filename)
{
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପ୍ରତିଜ୍ଞା(filename);

	fp = fopen(filename, "w");
	ଯଦି (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		ଫେରନ୍ତୁ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_VERSION);

	ପାଇଁ (i = 0; i != ଅଙ୍କ_ସନ୍ଧାନ_TREES; i++) {
		urldb_save_ସନ୍ଧାନ_ବୃକ୍ଷ(ସନ୍ଧାନ_trees[i], fp);
	}

	fclose(fp);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	ସଂରଚନା path_data *p;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p) {
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	ସଂରଚନା host_part *h;
	ସଂରଚନା path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	ଅଚଳ ଅକ୍ଷର *host_str;
	ଅକ୍ଷର *path_query = ଖାଲି;
	size_t len;
	bool match;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା port_int;

	ପ୍ରତିଜ୍ଞା(url);

	ଯଦି (url_bloom == ଖାଲି)
		url_bloom = bloom_create(BLOOM_SIZE);

	ଯଦି (url_bloom != ଖାଲି) {
		uint32_t hash = nsurl_hash(url);
		bloom_ଭର୍ତ୍ତି_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	ଯଦି (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}
	ପ୍ରତିଜ୍ଞା(path_query != ଖାଲି);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (scheme == ଖାଲି) {
		ମୁକ୍ତ(path_query);
		ଫେରନ୍ତୁ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (host != ଖାଲି) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} ଅନ୍ୟଥା ଯଦି (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} ଅନ୍ୟଥା {
		lwc_string_unref(scheme);
		ମୁକ୍ତ(path_query);
		ଫେରନ୍ତୁ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	ଯଦି (port != ଖାଲି) {
		port_int = atoi(lwc_string_data(port));
		lwc_string_unref(port);
	} ଅନ୍ୟଥା {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	ଯଦି (h != ଖାଲି) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} ଅନ୍ୟଥା {
		p = ଖାଲି;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	ଫେରନ୍ତୁ (p != ଖାଲି);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, ଅଚଳ ଅକ୍ଷର *title)
{
	ସଂରଚନା path_data *p;
	ଅକ୍ଷର *temp;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (p == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	ଯଦି (title != ଖାଲି) {
		temp = strdup(title);
		ଯଦି (temp == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	} ଅନ୍ୟଥା {
		temp = ଖାଲି;
	}

	ମୁକ୍ତ(p->urld.title);
	p->urld.title = temp;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	ସଂରଚନା path_data *p;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p) {
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	ସଂରଚନା path_data *p;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p) {
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = time(ଖାଲି);
	p->urld.visits++;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
ଶୂନ୍ୟ urldb_reset_url_visit_data(nsurl *url)
{
	ସଂରଚନା path_data *p;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ;

	p->urld.last_visit = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ଅଚଳ ସଂରଚନା url_data *urldb_get_url_data(nsurl *url)
{
	ସଂରଚନା path_data *p;
	ସଂରଚନା url_internal_data *u;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ ଖାଲି;

	u = &p->urld;

	ଫେରନ୍ତୁ (ଅଚଳ ସଂରଚନା url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ସଂରଚନା path_data *p;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ p->url;
}


/* exported interface documented in netsurf/url_db.h */
ଶୂନ୍ୟ urldb_set_auth_details(nsurl *url, ଅଚଳ ଅକ୍ଷର *realm, ଅଚଳ ଅକ୍ଷର *auth)
{
	ସଂରଚନା path_data *p, *pi;
	ସଂରଚନା host_part *h;
	ସଂରଚନା prot_space_data *space, *space_alloc;
	ଅକ୍ଷର *realm_alloc, *auth_alloc;
	bool match;

	ପ୍ରତିଜ୍ଞା(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	ଯଦି (!p)
		ଫେରନ୍ତୁ;

	/* Search for host_part */
	ପାଇଁ (pi = p; pi->parent != ଖାଲି; pi = pi->parent)
		;
	h = (ସଂରଚନା host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	ପାଇଁ (space = h->prot_space; space; space = space->next) {
		ଯଦି (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			ଭାଙ୍ଗ;
	}

	ଯଦି (space != ଖାଲି) {
		/* Overrule existing auth. */
		ମୁକ୍ତ(space->auth);
		space->auth = strdup(auth);
	} ଅନ୍ୟଥା {
		/* Create a new protection space. */
		space = space_alloc = malloc(ଆକାର(ସଂରଚନା prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		ଯଦି (!space_alloc || !realm_alloc || !auth_alloc) {
			ମୁକ୍ତ(space_alloc);
			ମୁକ୍ତ(realm_alloc);
			ମୁକ୍ତ(auth_alloc);
			ଫେରନ୍ତୁ;
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
ଅଚଳ ଅକ୍ଷର *urldb_get_auth_details(nsurl *url, ଅଚଳ ଅକ୍ଷର *realm)
{
	ସଂରଚନା path_data *p, *p_cur, *p_ଶୀର୍ଷ;

	ପ୍ରତିଜ୍ଞା(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ ଖାଲି;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	ପାଇଁ (p_cur = p; p_cur != ଖାଲି; p_ଶୀର୍ଷ = p_cur, p_cur = p_cur->parent) {
		ଯଦି (p_cur->prot_space) {
			ଫେରନ୍ତୁ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	ଯଦି (realm != ଖାଲି) {
		ଅଚଳ ସଂରଚନା host_part *h = (ଅଚଳ ସଂରଚନା host_part *)p_ଶୀର୍ଷ;
		ଅଚଳ ସଂରଚନା prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		ପାଇଁ (space = h->prot_space; space != ଖାଲି;
		     space = space->next) {
			ଯଦି (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				ଫେରନ୍ତୁ p->prot_space->auth;
			}
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}


/* exported interface documented in netsurf/url_db.h */
ଶୂନ୍ୟ urldb_set_cert_permissions(nsurl *url, bool permit)
{
	ସଂରଚନା path_data *p;
	ସଂରଚନା host_part *h;

	ପ୍ରତିଜ୍ଞା(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ;

	ପାଇଁ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ପ୍ରତିଜ୍ଞା(p);

	h = (ସଂରଚନା host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ସଂରଚନା path_data *p;
	ଅଚଳ ସଂରଚନା host_part *h;

	ପ୍ରତିଜ୍ଞା(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ପ୍ରତିଜ୍ଞା(p);

	h = (ଅଚଳ ସଂରଚନା host_part *)p;

	ଫେରନ୍ତୁ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(ସଂରଚନା nsurl *url, ଅଚଳ ଅକ୍ଷର *header)
{
	ସଂରଚନା path_data *p;
	ସଂରଚନା host_part *h;
	lwc_string *host;
	time_t now = time(ଖାଲି);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	ପ୍ରତିଜ୍ଞା(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (host != ଖାଲି) {
		ଯଦି (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			ଫେରନ୍ତୁ true;
		} ଅନ୍ୟଥା ଯଦି (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			ଫେରନ୍ତୁ true;
		}

		lwc_string_unref(host);
	} ଅନ୍ୟଥା {
		/* No host part: ignore */
		ଫେରନ୍ତୁ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ପ୍ରତିଜ୍ଞା(p);

	h = (ସଂରଚନା host_part *)p;
	ଯଦି (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		ଫେରନ୍ତୁ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	ଯଦି (error != NSERROR_OK) {
		/* Parse failed: ignore */
		ଫେରନ୍ତୁ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	ଯଦି (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ଅନ୍ୟଥା ଯଦି ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ସଂରଚନା nsurl *url)
{
	ସଂରଚନା path_data *p;
	ଅଚଳ ସଂରଚନା host_part *h;
	lwc_string *host;
	time_t now = time(ଖାଲି);

	ପ୍ରତିଜ୍ଞା(url);

	host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (host != ଖାଲି) {
		ଯଦି (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			ଫେରନ୍ତୁ false;
		} ଅନ୍ୟଥା ଯଦି (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			ଫେରନ୍ତୁ false;
		}

		lwc_string_unref(host);
	} ଅନ୍ୟଥା {
		/* No host part: not enabled */
		ଫେରନ୍ତୁ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (; p && p->parent; p = p->parent)
		/* do nothing */;
	ପ୍ରତିଜ୍ଞା(p);

	h = (ଅଚଳ ସଂରଚନା host_part *)p;

	/* Consult record for this host */
	ଯଦି (h->hsts.expires > now) {
		/* Not expired */
		ଫେରନ୍ତୁ true;
	}

	/* Consult parent domains */
	ପାଇଁ (h = h->parent; h && h != &db_ମୂଳ; h = h->parent) {
		ଯଦି (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			ଫେରନ୍ତୁ true;
		}
	}

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in netsurf/url_db.h */
ଶୂନ୍ୟ
urldb_iterate_partial(ଅଚଳ ଅକ୍ଷର *prefix,
		      bool (*callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data))
{
	ଅକ୍ଷର host[256];
	ଅକ୍ଷର buf[260]; /* max domain + "www." */
	ଅଚଳ ଅକ୍ଷର *slash, *scheme_sep;
	ସଂରଚନା ସନ୍ଧାନ_ଗଣ୍ଠି *ବୃକ୍ଷ;
	ଅଚଳ ସଂରଚନା host_part *h;

	ପ୍ରତିଜ୍ଞା(prefix && callback);

	/* strip scheme */
	scheme_sep = strstr(prefix, "://");
	ଯଦି (scheme_sep)
		prefix = scheme_sep + 3;

	slash = strchr(prefix, '/');
	ବୃକ୍ଷ = urldb_get_ସନ୍ଧାନ_ବୃକ୍ଷ(prefix);

	ଯଦି (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, ଆକାର host, "%.*s",
			 (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (slash - prefix), prefix);

		h = urldb_ସନ୍ଧାନ_find(ବୃକ୍ଷ, host);
		ଯଦି (!h) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = slash - prefix;

			ଯଦି (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, ଆକାର buf, "www.%s", host);
				h = urldb_ସନ୍ଧାନ_find(
					ସନ୍ଧାନ_trees[ST_DN + 'w' - 'a'],
					buf);
				ଯଦି (!h)
					ଫେରନ୍ତୁ;
			} ଅନ୍ୟଥା
				ଫେରନ୍ତୁ;
		}

		ଯଦି (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(prefix);

		/* looking for hosts */
		ଯଦି (!urldb_iterate_partial_host(ବୃକ୍ଷ, prefix, callback))
			ଫେରନ୍ତୁ;

		ଯଦି (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, ଆକାର buf, "www.%s", prefix);
			ଯଦି(!urldb_iterate_partial_host(
				   ସନ୍ଧାନ_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				ଫେରନ୍ତୁ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
ଶୂନ୍ୟ
urldb_iterate_entries(bool (*callback)(nsurl *url, ଅଚଳ ସଂରଚନା url_data *data))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପ୍ରତିଜ୍ଞା(callback);

	ପାଇଁ (i = 0; i < ଅଙ୍କ_ସନ୍ଧାନ_TREES; i++) {
		ଯଦି (!urldb_iterate_entries_host(ସନ୍ଧାନ_trees[i],
						callback,
						ଖାଲି)) {
			ଭାଙ୍ଗ;
		}
	}
}


/* exported interface documented in content/urldb.h */
ଶୂନ୍ୟ urldb_iterate_cookies(bool (*callback)(ଅଚଳ ସଂରଚନା cookie_data *data))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପ୍ରତିଜ୍ଞା(callback);

	ପାଇଁ (i = 0; i < ଅଙ୍କ_ସନ୍ଧାନ_TREES; i++) {
		ଯଦି (!urldb_iterate_entries_host(ସନ୍ଧାନ_trees[i],
						ଖାଲି, callback))
			ଭାଙ୍ଗ;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(ଅଚଳ ଅକ୍ଷର *header, nsurl *url, nsurl *referer)
{
	ଅଚଳ ଅକ୍ଷର *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	ପ୍ରତିଜ୍ଞା(url && header);

	/* Get defragmented URL, as 'urlt' */
	ଯଦି (nsurl_defragment(url, &urlt) != NSERROR_OK)
		ଫେରନ୍ତୁ ଖାଲି;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (scheme == ଖାଲି) {
		nsurl_unref(urlt);
		ଫେରନ୍ତୁ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	ଯଦି (path == ଖାଲି) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ଫେରନ୍ତୁ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (host == ଖାଲି) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		ଯଦି (rhost == ଖାଲି) {
			ଯାଅ error;
		}

		/* Domain match host names */
		ଯଦି (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			ଅଚଳ ଅକ୍ଷର *hptr;
			ଅଚଳ ଅକ୍ଷର *rptr;
			ଅଚଳ ଅକ୍ଷର *dot;
			ଅଚଳ ଅକ୍ଷର *host_data = lwc_string_data(host);
			ଅଚଳ ଅକ୍ଷର *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			ଯଦି (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				ଯାଅ error;
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
			ଯେତେବେଳେ (hptr >= host_data && rptr >= rhost_data) {
				ଯଦି (*hptr != *rptr)
					ଭାଙ୍ଗ;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			ଯେତେବେଳେ (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			ଯଦି (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == ଖାଲି ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				ଯାଅ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + strlen(cur) - 2 /* Trailing CRLF */;

	କର {
		ସଂରଚନା cookie_internal_data *c;
		ଅକ୍ଷର *dot;
		size_t len;
#ifdef WITH_NSPSL
		ଅଚଳ ଅକ୍ଷର *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		ଯଦି (!c) {
			/* failed => stop parsing */
			ଯାଅ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		ଯଦି (!c->name || !c->ମୂଲ୍ୟ) {
			urldb_free_cookie(c);
			ଯାଅ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = strlen(c->path);
		ଯଦି (len > lwc_string_length(path) ||
		    strncmp(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			ଯାଅ error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		ଯଦି (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		ଯଦି (suffix == ଖାଲି) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			ଯାଅ error;
		}
#ଅନ୍ୟଥା
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		ଯଦି (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			ଯାଅ error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		ଯଦି (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା hlen, dlen;
			ଅକ୍ଷର *domain = c->domain;

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
			ପ୍ରତିଜ୍ଞା(c->domain[0] == '.');

			/* 4.3.2:iii */
			ଯଦି (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				ଯାଅ error;
			}

			hlen = lwc_string_length(host);
			dlen = strlen(c->domain);

			ଯଦି (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				ଯାଅ error;
			}

			ଯଦି (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			ଯଦି (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				ଯାଅ error;
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
		ଯଦି (!urldb_ଭର୍ତ୍ତି_cookie(c, scheme, urlt))
			ଯାଅ error;
	} ଯେତେବେଳେ (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ଫେରନ୍ତୁ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in content/urldb.h */
ଅକ୍ଷର *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ଅଚଳ ସଂରଚନା path_data *p, *q;
	ଅଚଳ ସଂରଚନା host_part *h;
	lwc_string *path_lwc;
	ସଂରଚନା cookie_internal_data *c;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0, version = COOKIE_RFC2965;
	ସଂରଚନା cookie_internal_data **matched_cookies;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା matched_cookies_size = 20;
	ଅଚଳ ଅକ୍ଷର *path;
	ଅକ୍ଷର *ret;
	lwc_string *scheme;
	time_t now;
	bool match;

	ପ୍ରତିଜ୍ଞା(url != ଖାଲି);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	ଯଦି (!p)
		ଫେରନ୍ତୁ ଖାଲି;

	scheme = p->scheme;

	matched_cookies = malloc(matched_cookies_size *
				 ଆକାର(ସଂରଚନା cookie_internal_data *));
	ଯଦି (!matched_cookies)
		ଫେରନ୍ତୁ ଖାଲି;

#define GROW_MATCHED_COOKIES						\
	କର {								\
		ଯଦି (ଗଣନା_2 == matched_cookies_size) {			\
			ସଂରଚନା cookie_internal_data **temp;		\
			temp = realloc(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       ଆକାର(ସଂରଚନା cookie_internal_data *)); \
									\
			ଯଦି (temp == ଖାଲି) {				\
				ମୁକ୍ତ(matched_cookies);			\
				ଫେରନ୍ତୁ ଖାଲି;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} ଯେତେବେଳେ(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	ଯଦି (path_lwc == ଖାଲି) {
		ମୁକ୍ତ(matched_cookies);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = time(ଖାଲି);

	ଯଦି (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		ପାଇଁ (q = p->parent->children; q; q = q->next) {
			ଯଦି (strcmp(q->segment, p->segment))
				ଜାରି;

			/* Consider all cookies associated with
			 * this exact path */
			ପାଇଁ (c = q->cookies; c; c = c->next) {
				ଯଦି (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ଜାରି;

				ଯଦି (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					ଜାରି;

				ଯଦି (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					ଜାରି;

				matched_cookies[ଗଣନା_2++] = c;

				GROW_MATCHED_COOKIES;

				ଯଦି (c->version < (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ସଂରଚନା cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	ପାଇଁ (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		ପାଇଁ (q = p->children; q; q = q->next) {
			ଯଦି (*(q->segment) != '\0')
				ଜାରି;

			ପାଇଁ (c = q->cookies; c; c = c->next) {
				ଯଦି (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					ଜାରି;

				ଯଦି (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					ଜାରି;

				matched_cookies[ଗଣନା_2++] = c;

				GROW_MATCHED_COOKIES;

				ଯଦି (c->version < (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((ସଂରଚନା cookie_data *)c);
			}
		}

		ଯଦି (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			ଭାଙ୍ଗ;
		}

		/* Consider p itself - may be the result of Path=/foo */
		ପାଇଁ (c = p->cookies; c; c = c->next) {
			ଯଦି (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ଜାରି;

			/* Ensure cookie path is a prefix of the resource */
			ଯଦି (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ଜାରି;

			ଯଦି (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				ଜାରି;

			matched_cookies[ଗଣନା_2++] = c;

			GROW_MATCHED_COOKIES;

			ଯଦି (c->version < (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ସଂରଚନା cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	ପାଇଁ (h = (ଅଚଳ ସଂରଚନା host_part *)p; h && h != &db_ମୂଳ;
	     h = h->parent) {
		ପାଇଁ (c = h->paths.cookies; c; c = c->next) {
			ଯଦି (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				ଜାରି;

			/* Ensure cookie path is a prefix of the resource */
			ଯଦି (strncmp(c->path, path, strlen(c->path)) != 0)
				/* paths don't match => ignore */
				ଜାରି;

			ଯଦି (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				ଜାରି;

			matched_cookies[ଗଣନା_2++] = c;

			GROW_MATCHED_COOKIES;

			ଯଦି (c->version < (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((ସଂରଚନା cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	ଯଦି (ଗଣନା_2 == 0) {
		/* No cookies found */
		ମୁକ୍ତ(matched_cookies);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, ଗଣନା_2);
	} ଅନ୍ୟଥା {
		ret = get_rfc2965_cookie(matched_cookies, ଗଣନା_2, version);
	}
	ମୁକ୍ତ(matched_cookies);

	ଫେରନ୍ତୁ ret;
}


/* exported interface documented in content/urldb.h */
ଶୂନ୍ୟ urldb_delete_cookie(ଅଚଳ ଅକ୍ଷର *domain, ଅଚଳ ଅକ୍ଷର *path,
			 ଅଚଳ ଅକ୍ଷର *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_ମୂଳ);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(ଅଚଳ ଅକ୍ଷର *filename)
{
	FILE *fp;
	ଅକ୍ଷର s[16*1024];

	ପ୍ରତିଜ୍ଞା(filename);

	fp = fopen(filename, "r");
	ଯଦି (!fp)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

#define FIND_T {				\
		ପାଇଁ (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		ଯଦି (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ଜାରି;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		ପାଇଁ (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		ଯଦି (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			ଜାରି;		\
		}				\
	}

	ଯେତେବେଳେ (fgets(s, ଆକାର s, fp)) {
		ଅକ୍ଷର *p = s, *end = 0,
			*domain, *path, *name, *ମୂଲ୍ୟ, *scheme, *url,
			*comment;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା version, domain_specified, path_specified,
			secure, http_only, no_destroy, ମୂଲ୍ୟ_quoted;
		time_t expires, last_used;
		ସଂରଚନା cookie_internal_data *c;

		ଯଦି(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			ଜାରି;

		s[strlen(s) - 1] = '\0'; /* lose terminating newline */
		end = s + strlen(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		ଯଦି (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = atoi(p);

			ଯଦି (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				ଭାଙ୍ଗ;
			}

			ଜାରି;
		} ଅନ୍ୟଥା ଯଦି (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			ଜାରି;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		ଯଦି (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} ଅନ୍ୟଥା {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; last_used = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; ମୂଲ୍ୟ = p; FIND_T;
		ଯଦି (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	ମୂଲ୍ୟ_quoted = atoi(p); FIND_T;
		} ଅନ୍ୟଥା {
			ମୂଲ୍ୟ_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		ପାଇଁ (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		ପ୍ରତିଜ୍ଞା(p <= end);

		/* Now create cookie */
		c = malloc(ଆକାର(ସଂରଚନା cookie_internal_data));
		ଯଦି (!c)
			ଭାଙ୍ଗ;

		c->name = strdup(name);
		c->ମୂଲ୍ୟ = strdup(ମୂଲ୍ୟ);
		c->ମୂଲ୍ୟ_was_quoted = ମୂଲ୍ୟ_quoted;
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

		ଯଦି (!(c->name && c->ମୂଲ୍ୟ && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			ଭାଙ୍ଗ;
		}

		ଯଦି (c->domain[0] != '.') {
			lwc_string *scheme_lwc = ଖାଲି;
			nsurl *url_nsurl = ଖାଲି;

			ପ୍ରତିଜ୍ଞା(scheme[0] != 'u');

			ଯଦି (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				ଭାଙ୍ଗ;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			ଯଦି (!urldb_ଭର୍ତ୍ତି_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				ଭାଙ୍ଗ;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ଅନ୍ୟଥା {
			ଯଦି (!urldb_ଭର୍ତ୍ତି_cookie(c, ଖାଲି, ଖାଲି)) {
				/* Cookie freed for us */
				ଭାଙ୍ଗ;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	fclose(fp);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ଅଚଳ ଅକ୍ଷର *filename)
{
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	ପ୍ରତିଜ୍ଞା(filename);

	fp = fopen(filename, "w");
	ଯଦି (!fp)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_ମୂଳ);

	fclose(fp);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
ଶୂନ୍ୟ urldb_dump(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	urldb_dump_hosts(&db_ମୂଳ);

	ପାଇଁ (i = 0; i != ଅଙ୍କ_ସନ୍ଧାନ_TREES; i++) {
		urldb_dump_ସନ୍ଧାନ(ସନ୍ଧାନ_trees[i], 0);
	}
}
