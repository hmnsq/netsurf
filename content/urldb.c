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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <time.h>
#ifdef WITH_NSPSL
#अन्तर्भाव <nspsl.h>
#endif

#अन्तर्भाव "utils/inet.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/url.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/bloom.h"
#अन्तर्भाव "utils/time.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "desktop/cookie_manager.h"

#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
संरचना cookie_internal_data {
	संरचना cookie_internal_data *prev;	/**< Previous in list */
	संरचना cookie_internal_data *next;	/**< Next in list */

	वर्ण *name;		/**< Cookie name */
	वर्ण *मान;		/**< Cookie value */
	bool मान_was_quoted;	/**< Value was quoted in Set-Cookie: */
	वर्ण *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	वर्ण *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	वर्ण *path;		/**< Path */
	समय_प्रकार expires;		/**< Expiry timestamp, or -1 for session */
	समय_प्रकार last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	गणना cookie_version version;	/**< Specification compliance */
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
संरचना prot_space_data {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	अचिन्हित पूर्णाङ्क port;
	/** Protection realm */
	वर्ण *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	वर्ण *auth;
	/** Next sibling */
	संरचना prot_space_data *next;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
संरचना url_internal_data {
	वर्ण *title;		/**< Resource title */
	अचिन्हित पूर्णाङ्क visits;	/**< Visit count */
	समय_प्रकार last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
संरचना path_data {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	अचिन्हित पूर्णाङ्क port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	वर्ण *segment;		/**< Path segment for this node */
	अचिन्हित पूर्णाङ्क frag_cnt;	/**< Number of entries in path_data::fragment */
	वर्ण **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	संरचना url_internal_data urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	अचर संरचना prot_space_data *prot_space;
	/** Cookies associated with resource */
	संरचना cookie_internal_data *cookies;
	/** Last cookie in list */
	संरचना cookie_internal_data *cookies_end;

	संरचना path_data *next;	/**< Next sibling */
	संरचना path_data *prev;	/**< Previous sibling */
	संरचना path_data *parent; /**< Parent path segment */
	संरचना path_data *children; /**< Child path segments */
	संरचना path_data *last; /**< Last child */
};

संरचना hsts_data {
	समय_प्रकार expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

संरचना host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	संरचना path_data paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	संरचना hsts_data hsts;

	/**
	 * Part of host string
	 */
	वर्ण *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	संरचना prot_space_data *prot_space;

	संरचना host_part *next;	/**< Next sibling */
	संरचना host_part *prev;	/**< Previous sibling */
	संरचना host_part *parent; /**< Parent host part */
	संरचना host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
संरचना खोज_गाँठो {
	अचर संरचना host_part *data;	/**< Host tree entry */

	अचिन्हित पूर्णाङ्क level;		/**< Node level */

	संरचना खोज_गाँठो *बायाँ;	/**< Left subtree */
	संरचना खोज_गाँठो *दायाँ;	/**< Right subtree */
};

/** Root database handle */
स्थिर संरचना host_part db_जरा;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#define अङ्क_खोज_TREES 28
#define ST_IP 0
#define ST_EE 1
#define ST_DN 2
स्थिर संरचना खोज_गाँठो empty = { 0, 0, &empty, &empty };
स्थिर संरचना खोज_गाँठो *खोज_trees[अङ्क_खोज_TREES] = {
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
स्थिर पूर्णाङ्क loaded_cookie_file_version;

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
स्थिर संरचना bloom_filter *url_bloom;
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
स्थिर nserror urldb_write_timet(खाता *fp, समय_प्रकार val)
{
	पूर्णाङ्क use;
	वर्ण op[32];

	use = nsc_sntimet(op, 32, &val);
	यदि (use == 0) {
		ख_लिखो(fp, "%i\n", (पूर्णाङ्क)val);
	} अन्यथा {
		ख_लिखो(fp, "%.*s\n", use, op);
	}
	फर्काऊ NSERROR_OK;
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
स्थिर शून्य
urldb_write_paths(अचर संरचना path_data *parent,
		  अचर वर्ण *host,
		  खाता *fp,
		  वर्ण **path,
		  पूर्णाङ्क *path_alloc,
		  पूर्णाङ्क *path_used,
		  समय_प्रकार expiry)
{
	अचर संरचना path_data *p = parent;
	पूर्णाङ्क i;

	गर {
		पूर्णाङ्क seglen = p->segment != रिक्त ? म_माप(p->segment) : 0;
		पूर्णाङ्क len = *path_used + seglen + 1;

		यदि (*path_alloc < len) {
			वर्ण *temp;
			temp = पुनः_स्मृति(*path,
				       (len > 64) ? len : *path_alloc + 64);
			यदि (!temp) {
				फर्काऊ;
			}
			*path = temp;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		यदि (p->segment != रिक्त) {
			स_नकल(*path + *path_used - 1, p->segment, seglen);
		}

		यदि (p->children != रिक्त) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} अन्यथा {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		यदि (p->children != रिक्त) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* leaf node */
			यदि (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				ख_लिखो(fp, "%s\n", lwc_string_data(p->scheme));

				यदि (p->port) {
					ख_लिखो(fp,"%d\n", p->port);
				} अन्यथा {
					ख_लिखो(fp, "\n");
				}

				ख_लिखो(fp, "%s\n", *path);

				/** \todo handle fragments? */

				/* number of visits */
				ख_लिखो(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.last_visit);

				/* entry type */
				ख_लिखो(fp, "%i\n", (पूर्णाङ्क)p->urld.type);

				ख_लिखो(fp, "\n");

				यदि (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					लागि (i = 0; s[i] != '\0'; i++)
						यदि (s[i] < 32)
							s[i] = ' ';
					लागि (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					ख_लिखो(fp, "%s\n", p->urld.title);
				} अन्यथा {
					ख_लिखो(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			जबसम्म (p != parent) {
				पूर्णाङ्क seglen = p->segment != रिक्त
					? म_माप(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				यदि (p->next != रिक्त) {
					/* Have a sibling, process that */
					p = p->next;
					रोक;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबसम्म (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
स्थिर शून्य
urldb_गन्ती_urls(अचर संरचना path_data *जरा,
		 समय_प्रकार expiry,
		 अचिन्हित पूर्णाङ्क *गन्ती)
{
	अचर संरचना path_data *p = जरा;

	गर {
		यदि (p->children != रिक्त) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* No more children, increment count if required */
			यदि (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*गन्ती)++;
			}

			/* Now, find next node to process. */
			जबसम्म (p != जरा) {
				यदि (p->next != रिक्त) {
					/* Have a sibling, process that */
					p = p->next;
					रोक;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबसम्म (p != जरा);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
स्थिर शून्य urldb_save_खोज_रूख(संरचना खोज_गाँठो *parent, खाता *fp)
{
	वर्ण host[256];
	अचर संरचना host_part *h;
	अचिन्हित पूर्णाङ्क path_गन्ती = 0;
	वर्ण *path, *p, *end;
	पूर्णाङ्क path_alloc = 64, path_used = 1;
	समय_प्रकार expiry, hsts_expiry = 0;
	पूर्णाङ्क hsts_include_subdomains = 0;

	expiry = समय(रिक्त) - ((60 * 60 * 24) * nsoption_int(expire_url));

	यदि (parent == &empty)
		फर्काऊ;

	urldb_save_खोज_रूख(parent->बायाँ, fp);

	path = दो_स्मृति(path_alloc);
	यदि (!path)
		फर्काऊ;

	path[0] = '\0';

	लागि (h = parent->data, p = host, end = host + परिमाण host;
	     h && h != &db_जरा && p < end; h = h->parent) {
		पूर्णाङ्क written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		यदि (written < 0) {
			मुक्त(path);
			फर्काऊ;
		}
		p += written;
	}

	h = parent->data;
	यदि (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_गन्ती_urls(&parent->data->paths, expiry, &path_गन्ती);

	यदि (path_गन्ती > 0) {
		ख_लिखो(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		ख_लिखो(fp, "%i\n", path_गन्ती);

		urldb_write_paths(&parent->data->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} अन्यथा यदि (hsts_expiry) {
		ख_लिखो(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		ख_लिखो(fp, "0\n");
	}

	मुक्त(path);

	urldb_save_खोज_रूख(parent->दायाँ, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
स्थिर bool
urldb_iterate_entries_path(अचर संरचना path_data *parent,
		bool (*url_callback)(nsurl *url, अचर संरचना url_data *data),
		bool (*cookie_callback)(अचर संरचना cookie_data *data))
{
	अचर संरचना path_data *p = parent;
	अचर संरचना cookie_data *c;

	गर {
		यदि (p->children != रिक्त) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			प्रतिज्ञा(url_callback || cookie_callback);

			/** \todo handle fragments? */
			यदि (url_callback) {
				अचर संरचना url_internal_data *u = &p->urld;

				प्रतिज्ञा(p->url);

				यदि (!url_callback(p->url,
						  (अचर संरचना url_data *) u))
					फर्काऊ false;
			} अन्यथा {
				c = (अचर संरचना cookie_data *)p->cookies;
				लागि (; c != रिक्त; c = c->next) {
					यदि (!cookie_callback(c))
						फर्काऊ false;
				}
			}

			/* Now, find next node to process. */
			जबसम्म (p != parent) {
				यदि (p->next != रिक्त) {
					/* Have a sibling, process that */
					p = p->next;
					रोक;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबसम्म (p != parent);

	फर्काऊ true;
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
स्थिर bool urldb__host_is_ip_address(अचर वर्ण *host)
{
	संरचना in_addr ipv4;
	माप_प्रकार host_len = म_माप(host);
	अचर वर्ण *sane_host;
	अचर वर्ण *slash;
#ifndef NO_IPV6
	संरचना in6_addr ipv6;
	वर्ण ipv6_addr[64];
	अचिन्हित पूर्णाङ्क ipv6_addr_len;
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

	slash = म_अक्षर(host, '/');
	यदि (slash == रिक्त) {
		sane_host = host;
	} अन्यथा {
		वर्ण *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	यदि (म_अखोज(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		गच्छ out_false;

	यदि (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		पूर्णाङ्क अङ्क_dots = 0;
		माप_प्रकार index;

		लागि (index = 0; index < host_len; index++) {
			यदि (sane_host[index] == '.')
				अङ्क_dots++;
		}

		यदि (अङ्क_dots == 3)
			गच्छ out_true;
		अन्यथा
			गच्छ out_false;
	}

#ifndef NO_IPV6
	यदि ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		गच्छ out_false;
	}

	ipv6_addr_len = host_len - 2;
	यदि (ipv6_addr_len >= परिमाण(ipv6_addr)) {
		ipv6_addr_len = परिमाण(ipv6_addr) - 1;
	}
	म_नकलन(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	यदि (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		गच्छ out_true;
#endif

out_false:
	यदि (slash != रिक्त) मुक्त((शून्य *)sane_host);
	फर्काऊ false;

out_true:
	यदि (slash != रिक्त) मुक्त((शून्य *)sane_host);
	फर्काऊ true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क urldb_खोज_match_prefix(अचर संरचना host_part *a, अचर वर्ण *b)
{
	अचर वर्ण *end, *dot;
	पूर्णाङ्क plen, ret;

	प्रतिज्ञा(a && a != &db_जरा && b);

	यदि (urldb__host_is_ip_address(b)) {
		/* IP address */
		फर्काऊ strncasecmp(a->part, b, म_माप(b));
	}

	end = b + म_माप(b) + 1;

	जबसम्म (b < end && a && a != &db_जरा) {
		dot = म_अक्षर(b, '.');
		यदि (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		यदि ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			फर्काऊ ret;

		/* The strings matched */
		यदि (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = म_माप(a->part);
			यदि (plen > dot - b) {
				/* len(a) > len(b) */
				फर्काऊ 1;
			} अन्यथा यदि (plen < dot - b) {
				/* len(a) < len(b) */
				फर्काऊ -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_जरा && b >= end) {
		/* len(a) > len(b) => prefix matches */
		फर्काऊ 0;
	} अन्यथा यदि ((!a || a == &db_जरा) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		फर्काऊ -1;
	}

	/* Identical */
	फर्काऊ 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
स्थिर bool
urldb_iterate_partial_host(संरचना खोज_गाँठो *जरा,
		अचर वर्ण *prefix,
		bool (*callback)(nsurl *url, अचर संरचना url_data *data))
{
	पूर्णाङ्क c;

	प्रतिज्ञा(जरा && prefix && callback);

	यदि (जरा == &empty)
		फर्काऊ true;

	c = urldb_खोज_match_prefix(जरा->data, prefix);

	यदि (c > 0) {
		/* No match => look in left subtree */
		फर्काऊ urldb_iterate_partial_host(जरा->बायाँ,
						  prefix,
						  callback);
	} अन्यथा यदि (c < 0) {
		/* No match => look in right subtree */
		फर्काऊ urldb_iterate_partial_host(जरा->दायाँ,
						  prefix,
						  callback);
	} अन्यथा {
		/* Match => iterate over l/r subtrees & process this node */
		यदि (!urldb_iterate_partial_host(जरा->बायाँ,
						prefix,
						callback)) {
			फर्काऊ false;
		}

		यदि (जरा->data->paths.children) {
			/* and extract all paths attached to this host */
			यदि (!urldb_iterate_entries_path(&जरा->data->paths,
							callback,
							रिक्त)) {
				फर्काऊ false;
			}
		}

		यदि (!urldb_iterate_partial_host(जरा->दायाँ,
						prefix,
						callback)) {
			फर्काऊ false;
		}
	}

	फर्काऊ true;
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
स्थिर bool
urldb_iterate_partial_path(अचर संरचना path_data *parent,
		अचर वर्ण *prefix,
		bool (*callback)(nsurl *url, अचर संरचना url_data *data))
{
	अचर संरचना path_data *p = parent->children;
	अचर वर्ण *slash, *end = prefix + म_माप(prefix);

	गर {
		slash = म_अक्षर(prefix, '/');
		यदि (!slash) {
			slash = end;
		}

		यदि (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			जारी;
		}

		यदि (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			यदि (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				यदि (!urldb_iterate_entries_path(p,
								callback,
								रिक्त)) {
					फर्काऊ false;
				}

				/* Progress to next sibling */
				p = p->next;
			} अन्यथा {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} अन्यथा {
			/* Doesn't match this segment, try next sibling */
			p = p->next;
		}
	} जबसम्म (p != रिक्त);

	फर्काऊ true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
स्थिर bool
urldb_iterate_entries_host(संरचना खोज_गाँठो *parent,
		bool (*url_callback)(nsurl *url, अचर संरचना url_data *data),
		bool (*cookie_callback)(अचर संरचना cookie_data *data))
{
	यदि (parent == &empty) {
		फर्काऊ true;
	}

	यदि (!urldb_iterate_entries_host(parent->बायाँ,
					url_callback,
					cookie_callback)) {
		फर्काऊ false;
	}

	यदि ((parent->data->paths.children) ||
	    ((cookie_callback) &&
	     (parent->data->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		यदि (!urldb_iterate_entries_path(&parent->data->paths,
						url_callback,
						cookie_callback)) {
			फर्काऊ false;
		}
	}

	यदि (!urldb_iterate_entries_host(parent->दायाँ,
					url_callback,
					cookie_callback)) {
		फर्काऊ false;
	}

	फर्काऊ true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
स्थिर संरचना host_part *
urldb_add_host_गाँठो(अचर वर्ण *part, संरचना host_part *parent)
{
	संरचना host_part *d;

	प्रतिज्ञा(part && parent);

	d = सुस्मृति(1, परिमाण(संरचना host_part));
	यदि (!d) {
		फर्काऊ रिक्त;
	}

	d->part = strdup(part);
	यदि (!d->part) {
		मुक्त(d);
		फर्काऊ रिक्त;
	}

	d->next = parent->children;
	यदि (parent->children) {
		parent->children->prev = d;
	}
	d->parent = parent;
	parent->children = d;

	फर्काऊ d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
स्थिर पूर्णाङ्क urldb_add_path_fragment_cmp(अचर शून्य *a, अचर शून्य *b)
{
	फर्काऊ strcasecmp(*((अचर वर्ण **) a), *((अचर वर्ण **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
स्थिर संरचना path_data *
urldb_add_path_fragment(संरचना path_data *segment, lwc_string *fragment)
{
	वर्ण **temp;

	प्रतिज्ञा(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	यदि (!fragment)
		फर्काऊ segment;

	temp = पुनः_स्मृति(segment->fragment,
		       (segment->frag_cnt + 1) * परिमाण(वर्ण *));
	यदि (!temp)
		फर्काऊ रिक्त;

	segment->fragment = temp;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_data(fragment));
	यदि (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		फर्काऊ रिक्त;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	क्विक(segment->fragment,
	      segment->frag_cnt,
	      परिमाण (वर्ण *),
	      urldb_add_path_fragment_cmp);

	फर्काऊ segment;
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
स्थिर संरचना path_data *
urldb_add_path_गाँठो(lwc_string *scheme,
		    अचिन्हित पूर्णाङ्क port,
		    अचर वर्ण *segment,
		    lwc_string *fragment,
		    संरचना path_data *parent)
{
	संरचना path_data *d, *e;

	प्रतिज्ञा(scheme && segment && parent);

	d = सुस्मृति(1, परिमाण(संरचना path_data));
	यदि (!d)
		फर्काऊ रिक्त;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	यदि (!d->segment) {
		lwc_string_unref(d->scheme);
		मुक्त(d);
		फर्काऊ रिक्त;
	}

	यदि (fragment) {
		यदि (!urldb_add_path_fragment(d, fragment)) {
			मुक्त(d->segment);
			lwc_string_unref(d->scheme);
			मुक्त(d);
			फर्काऊ रिक्त;
		}
	}

	लागि (e = parent->children; e; e = e->next) {
		यदि (म_भेद(e->segment, d->segment) > 0)
			रोक;
	}

	यदि (e) {
		d->prev = e->prev;
		d->next = e;
		यदि (e->prev)
			e->prev->next = d;
		अन्यथा
			parent->children = d;
		e->prev = d;
	} अन्यथा यदि (!parent->children) {
		d->prev = d->next = रिक्त;
		parent->children = parent->last = d;
	} अन्यथा {
		d->next = रिक्त;
		d->prev = parent->last;
		parent->last->next = d;
		parent->last = d;
	}
	d->parent = parent;

	फर्काऊ d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
स्थिर संरचना खोज_गाँठो **urldb_get_खोज_रूख_direct(अचर वर्ण *host)
{
	प्रतिज्ञा(host);

	यदि (urldb__host_is_ip_address(host)) {
		फर्काऊ &खोज_trees[ST_IP];
	} अन्यथा यदि (ascii_is_alpha(*host)) {
		फर्काऊ &खोज_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	फर्काऊ &खोज_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
स्थिर संरचना खोज_गाँठो *urldb_get_खोज_रूख(अचर वर्ण *host)
{
	फर्काऊ *urldb_get_खोज_रूख_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क urldb_खोज_match_string(अचर संरचना host_part *a, अचर वर्ण *b)
{
	अचर वर्ण *end, *dot;
	पूर्णाङ्क plen, ret;

	प्रतिज्ञा(a && a != &db_जरा && b);

	यदि (urldb__host_is_ip_address(b)) {
		/* IP address */
		फर्काऊ strcasecmp(a->part, b);
	}

	end = b + म_माप(b) + 1;

	जबसम्म (b < end && a && a != &db_जरा) {
		dot = म_अक्षर(b, '.');
		यदि (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		यदि ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			फर्काऊ ret;

		/* The strings matched, now check that the lengths do, too */
		plen = म_माप(a->part);

		यदि (plen > dot - b) {
			/* len(a) > len(b) */
			फर्काऊ 1;
		} अन्यथा यदि (plen < dot - b) {
			/* len(a) < len(b) */
			फर्काऊ -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_जरा && b >= end) {
		/* len(a) > len(b) */
		फर्काऊ 1;
	} अन्यथा यदि ((!a || a == &db_जरा) && b < end) {
		/* len(a) < len(b) */
		फर्काऊ -1;
	}

	/* Identical */
	फर्काऊ 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
स्थिर अचर संरचना host_part *
urldb_खोज_find(संरचना खोज_गाँठो *जरा, अचर वर्ण *host)
{
	पूर्णाङ्क c;

	प्रतिज्ञा(जरा && host);

	यदि (जरा == &empty) {
		फर्काऊ रिक्त;
	}

	c = urldb_खोज_match_string(जरा->data, host);

	यदि (c > 0) {
		फर्काऊ urldb_खोज_find(जरा->बायाँ, host);
	} अन्यथा यदि (c < 0) {
		फर्काऊ urldb_खोज_find(जरा->दायाँ, host);
	}

	फर्काऊ जरा->data;
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
स्थिर संरचना path_data *
urldb_match_path(अचर संरचना path_data *parent,
		 अचर वर्ण *path,
		 lwc_string *scheme,
		 अचिन्हित लघु port)
{
	अचर संरचना path_data *p;
	अचर वर्ण *slash;
	bool match;

	प्रतिज्ञा(parent != रिक्त);
	प्रतिज्ञा(parent->segment == रिक्त);

	यदि (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	प्रतिज्ञा(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	जबसम्म (p != रिक्त) {
		slash = म_अक्षर(path + 1, '/');
		यदि (!slash) {
			slash = path + म_माप(path);
		}

		यदि (म_भेदन(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			यदि (*slash == '\0') {
				/* Complete match */
				फर्काऊ (संरचना path_data *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} अन्यथा {
			/* No match, try next sibling */
			p = p->next;
		}
	}

	फर्काऊ रिक्त;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
स्थिर संरचना path_data *urldb_find_url(nsurl *url)
{
	अचर संरचना host_part *h;
	संरचना path_data *p;
	संरचना खोज_गाँठो *रूख;
	वर्ण *plq;
	अचर वर्ण *host_str;
	lwc_string *scheme, *host, *port;
	माप_प्रकार len = 0;
	अचिन्हित पूर्णाङ्क port_int;
	bool match;

	प्रतिज्ञा(url);

	यदि (url_bloom != रिक्त) {
		यदि (bloom_खोज_hash(url_bloom, nsurl_hash(url)) == false) {
			फर्काऊ रिक्त;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त)
		फर्काऊ रिक्त;

	यदि (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		फर्काऊ रिक्त;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} अन्यथा यदि (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		फर्काऊ रिक्त;
	}

	रूख = urldb_get_खोज_रूख(host_str);
	h = urldb_खोज_find(रूख, host_str);
	यदि (!h) {
		lwc_string_unref(scheme);
		फर्काऊ रिक्त;
	}

	/* generate plq (path, leaf, query) */
	यदि (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		फर्काऊ रिक्त;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	यदि (port != रिक्त) {
		port_int = म_से_प(lwc_string_data(port));
		lwc_string_unref(port);
	} अन्यथा {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	मुक्त(plq);
	lwc_string_unref(scheme);

	फर्काऊ p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
स्थिर शून्य urldb_dump_paths(संरचना path_data *parent)
{
	अचर संरचना path_data *p = parent;
	अचिन्हित पूर्णाङ्क i;

	गर {
		यदि (p->segment != रिक्त) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_data(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			लागि (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			जबसम्म (p != parent) {
				यदि (p->next != रिक्त) {
					p = p->next;
					रोक;
				}

				p = p->parent;
			}
		}
	} जबसम्म (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
स्थिर शून्य urldb_dump_hosts(संरचना host_part *parent)
{
	संरचना host_part *h;

	यदि (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	लागि (h = parent->children; h; h = h->next) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
स्थिर शून्य urldb_dump_खोज(संरचना खोज_गाँठो *parent, पूर्णाङ्क depth)
{
	अचर संरचना host_part *h;
	पूर्णाङ्क i; /* index into string */
	वर्ण s[1024];
	पूर्णाङ्क r;
	पूर्णाङ्क sl = परिमाण(s) - 2;

	यदि (parent == &empty)
		फर्काऊ;

	urldb_dump_खोज(parent->बायाँ, depth + 1);

	लागि (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	लागि (h = parent->data; h; h = h->parent) {
		यदि (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			यदि (r < 0) {
				रोक;
			}
			यदि ((i + r) >= sl) {
				रोक;
			}
			i += r;
		}

		यदि (h->parent && h->parent->parent) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_खोज(parent->दायाँ, depth + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क
urldb_खोज_match_host(अचर संरचना host_part *a, अचर संरचना host_part *b)
{
	पूर्णाङ्क ret;

	प्रतिज्ञा(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	लागि (; a && a != &db_जरा && b && b != &db_जरा;
	     a = a->parent, b = b->parent) {
		यदि ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			फर्काऊ ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_जरा && (!b || b == &db_जरा)) {
		/* len(a) > len(b) */
		फर्काऊ 1;
	} अन्यथा यदि ((!a || a == &db_जरा) && b && b != &db_जरा) {
		/* len(a) < len(b) */
		फर्काऊ -1;
	}

	/* identical */
	फर्काऊ 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
स्थिर संरचना खोज_गाँठो *urldb_खोज_skew(संरचना खोज_गाँठो *जरा)
{
	प्रतिज्ञा(जरा);

	यदि (जरा->बायाँ->level == जरा->level) {
		संरचना खोज_गाँठो *temp;

		temp = जरा->बायाँ;
		जरा->बायाँ = temp->दायाँ;
		temp->दायाँ = जरा;
		जरा = temp;
	}

	फर्काऊ जरा;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
स्थिर संरचना खोज_गाँठो *urldb_खोज_split(संरचना खोज_गाँठो *जरा)
{
	प्रतिज्ञा(जरा);

	यदि (जरा->दायाँ->दायाँ->level == जरा->level) {
		संरचना खोज_गाँठो *temp;

		temp = जरा->दायाँ;
		जरा->दायाँ = temp->बायाँ;
		temp->बायाँ = जरा;
		जरा = temp;

		जरा->level++;
	}

	फर्काऊ जरा;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
स्थिर संरचना खोज_गाँठो *
urldb_खोज_घुसाउ_internal(संरचना खोज_गाँठो *जरा, संरचना खोज_गाँठो *n)
{
	प्रतिज्ञा(जरा && n);

	यदि (जरा == &empty) {
		जरा = n;
	} अन्यथा {
		पूर्णाङ्क c = urldb_खोज_match_host(जरा->data, n->data);

		यदि (c > 0) {
			जरा->बायाँ = urldb_खोज_घुसाउ_internal(
				जरा->बायाँ, n);
		} अन्यथा यदि (c < 0) {
			जरा->दायाँ = urldb_खोज_घुसाउ_internal(
				जरा->दायाँ, n);
		} अन्यथा {
			/* exact match */
			मुक्त(n);
			फर्काऊ जरा;
		}

		जरा = urldb_खोज_skew(जरा);
		जरा = urldb_खोज_split(जरा);
	}

	फर्काऊ जरा;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
स्थिर संरचना खोज_गाँठो *
urldb_खोज_घुसाउ(संरचना खोज_गाँठो *जरा, अचर संरचना host_part *data)
{
	संरचना खोज_गाँठो *n;

	प्रतिज्ञा(जरा && data);

	n = दो_स्मृति(परिमाण(संरचना खोज_गाँठो));
	यदि (!n)
		फर्काऊ रिक्त;

	n->level = 1;
	n->data = data;
	n->बायाँ = n->दायाँ = &empty;

	जरा = urldb_खोज_घुसाउ_internal(जरा, n);

	फर्काऊ जरा;
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
स्थिर bool
urldb_parse_avpair(संरचना cookie_internal_data *c,
		   वर्ण *n,
		   वर्ण *v,
		   bool was_quoted)
{
	पूर्णाङ्क vlen;

	प्रतिज्ञा(c && n && v);

	/* Strip whitespace from start of name */
	लागि (; *n; n++) {
		यदि (*n != ' ' && *n != '\t')
			रोक;
	}

	/* Strip whitespace from end of name */
	लागि (vlen = म_माप(n); vlen; vlen--) {
		यदि (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		अन्यथा
			रोक;
	}

	/* Strip whitespace from start of value */
	लागि (; *v; v++) {
		यदि (*v != ' ' && *v != '\t')
			रोक;
	}

	/* Strip whitespace from end of value */
	लागि (vlen = म_माप(v); vlen; vlen--) {
		यदि (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		अन्यथा
			रोक;
	}

	यदि (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		यदि (!c->comment)
			फर्काऊ false;
	} अन्यथा यदि (!c->domain && strcasecmp(n, "Domain") == 0) {
		यदि (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			यदि (!c->domain)
				फर्काऊ false;
		}
	} अन्यथा यदि (strcasecmp(n, "Max-Age") == 0) {
		पूर्णाङ्क temp = म_से_प(v);
		यदि (temp == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		अन्यथा
			c->expires = समय(रिक्त) + temp;
	} अन्यथा यदि (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		यदि (!c->path)
			फर्काऊ false;
	} अन्यथा यदि (strcasecmp(n, "Version") == 0) {
		c->version = म_से_प(v);
	} अन्यथा यदि (strcasecmp(n, "Expires") == 0) {
		वर्ण *datenoday;
		समय_प्रकार expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		लागि (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, म_माप(datenoday), &expires);
		यदि (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (समय_प्रकार)0x7fffffff;
		}
		c->expires = expires;
	} अन्यथा यदि (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} अन्यथा यदि (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} अन्यथा यदि (!c->name) {
		c->name = strdup(n);
		c->मान = strdup(v);
		c->मान_was_quoted = was_quoted;
		यदि (!c->name || !c->मान) {
			फर्काऊ false;
		}
	}

	फर्काऊ true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
स्थिर शून्य urldb_free_cookie(संरचना cookie_internal_data *c)
{
	प्रतिज्ञा(c);

	मुक्त(c->comment);
	मुक्त(c->domain);
	मुक्त(c->path);
	मुक्त(c->name);
	मुक्त(c->मान);
	मुक्त(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
स्थिर संरचना cookie_internal_data *
urldb_parse_cookie(nsurl *url, अचर वर्ण **cookie)
{
	संरचना cookie_internal_data *c;
	अचर वर्ण *cur;
	वर्ण name[1024], मान[4096];
	वर्ण *n = name, *v = मान;
	bool in_मान = false;
	bool had_मान_data = false;
	bool मान_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	प्रतिज्ञा(url && cookie && *cookie);

	c = सुस्मृति(1, परिमाण(संरचना cookie_internal_data));
	यदि (c == रिक्त)
		फर्काऊ रिक्त;

	c->expires = -1;

	name[0] = '\0';
	मान[0] = '\0';

	लागि (cur = *cookie; *cur; cur++) {
		यदि (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			यदि (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				मान[0] = '\0';

			}

			रोक;
		} अन्यथा यदि (*cur == '\r') {
			/* Spurious linefeed */
			जारी;
		} अन्यथा यदि (*cur == '\n') {
			/* Spurious newline */
			जारी;
		}

		यदि (in_मान && !had_मान_data) {
			यदि (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				जारी;
			} अन्यथा {
				had_मान_data = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				यदि (*cur != '"') {
					मान_verbatim = true;
				}
			}
		}

		यदि (in_मान && !मान_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			यदि (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				जारी;
			}
		}

		यदि (!quoted && !in_मान && *cur == '=') {
			/* First equals => attr-value separator */
			in_मान = true;
			जारी;
		}

		यदि (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			यदि (!urldb_parse_avpair(c, name, मान, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				फर्काऊ रिक्त;
			}

			/* And reset to start */
			n = name;
			v = मान;
			in_मान = false;
			had_मान_data = false;
			मान_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			यदि (*cur != ';') {
				cur--;
			}

			जारी;
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
		यदि (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			अचर वर्ण *p;
			अचर वर्ण *semi = म_अक्षर(cur + 1, ';');
			यदि (!semi)
				semi = cur + म_माप(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			लागि (p = cur + 1; p < semi; p++)
				यदि (*p == '=')
					रोक;

			यदि (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} अन्यथा {
				/* found one => comma marks end of cookie */
				cur++;
				रोक;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		यदि (!in_मान) {
			यदि (n < name + (परिमाण(name) - 1))
				*n++ = *cur;
		} अन्यथा {
			यदि (v < मान + (परिमाण(मान) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	यदि (!urldb_parse_avpair(c, name, मान, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		फर्काऊ रिक्त;
	}

	/* Now fix-up default values */
	यदि (c->domain == रिक्त) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		यदि (host == रिक्त) {
			urldb_free_cookie(c);
			फर्काऊ रिक्त;
		}
		c->domain = strdup(lwc_string_data(host));
		lwc_string_unref(host);
	}

	यदि (c->path == रिक्त) {
		अचर वर्ण *path_data;
		वर्ण *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		यदि (path_lwc == रिक्त) {
			urldb_free_cookie(c);
			फर्काऊ रिक्त;
		}
		path_data = lwc_string_data(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = म_खोजप(path_data, '/');
		यदि (slash != रिक्त) {
			/* Special case: retain first slash in path */
			यदि (slash == path_data)
				slash++;

			slash = strndup(path_data, slash - path_data);
			यदि (slash == रिक्त) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				फर्काऊ रिक्त;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} अन्यथा {
			path = strdup(lwc_string_data(path_lwc));
			lwc_string_unref(path_lwc);
			यदि (path == रिक्त) {
				urldb_free_cookie(c);
				फर्काऊ रिक्त;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	फर्काऊ c;
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
स्थिर संरचना path_data *
urldb_add_path(lwc_string *scheme,
	       अचिन्हित पूर्णाङ्क port,
	       अचर संरचना host_part *host,
	       वर्ण *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	संरचना path_data *d, *e;
	वर्ण *buf = path_query;
	वर्ण *segment, *slash;
	bool match;

	प्रतिज्ञा(scheme && host && url);

	d = (संरचना path_data *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	यदि (*segment == '/')
		segment++;

	/* Process path segments */
	गर {
		slash = म_अक्षर(segment, '/');
		यदि (!slash) {
			/* last segment */
			/* look for existing entry */
			लागि (e = d->children; e; e = e->next)
				यदि (म_भेद(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					रोक;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_गाँठो(scheme, port,
						    segment, fragment, d);
			रोक;
		}

		*slash = '\0';

		/* look for existing entry */
		लागि (e = d->children; e; e = e->next)
			यदि (म_भेद(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				रोक;

		d = e ? e : urldb_add_path_गाँठो(scheme, port, segment, रिक्त, d);
		यदि (!d)
			रोक;

		segment = slash + 1;
	} जबसम्म (1);

	मुक्त(path_query);

	यदि (d && !d->url) {
		/* Insert defragmented URL */
		यदि (nsurl_defragment(url, &d->url) != NSERROR_OK)
			फर्काऊ रिक्त;
	}

	फर्काऊ d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
स्थिर संरचना host_part *urldb_add_host(अचर वर्ण *host)
{
	संरचना host_part *d = (संरचना host_part *) &db_जरा, *e;
	संरचना खोज_गाँठो *s;
	वर्ण buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	वर्ण *part;

	प्रतिज्ञा(host);

	यदि (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		लागि (e = d->children; e; e = e->next)
			यदि (strcasecmp(host, e->part) == 0)
				/* found => return it */
				फर्काऊ e;

		d = urldb_add_host_गाँठो(host, d);

		s = urldb_खोज_घुसाउ(खोज_trees[ST_IP], d);
		यदि (!s) {
			/* failed */
			d = रिक्त;
		} अन्यथा {
			खोज_trees[ST_IP] = s;
		}

		फर्काऊ d;
	}

	/* Copy host string, so we can corrupt it */
	म_नकलन(buf, host, परिमाण buf);
	buf[परिमाण buf - 1] = '\0';

	/* Process FQDN segments backwards */
	गर {
		part = म_खोजप(buf, '.');
		यदि (!part) {
			/* last segment */
			/* Check for existing entry */
			लागि (e = d->children; e; e = e->next)
				यदि (strcasecmp(buf, e->part) == 0)
					रोक;

			यदि (e) {
				d = e;
			} अन्यथा {
				d = urldb_add_host_गाँठो(buf, d);
			}

			/* And insert into search tree */
			यदि (d) {
				संरचना खोज_गाँठो **r;

				r = urldb_get_खोज_रूख_direct(buf);
				s = urldb_खोज_घुसाउ(*r, d);
				यदि (!s) {
					/* failed */
					d = रिक्त;
				} अन्यथा {
					*r = s;
				}
			}
			रोक;
		}

		/* Check for existing entry */
		लागि (e = d->children; e; e = e->next)
			यदि (strcasecmp(part + 1, e->part) == 0)
				रोक;

		d = e ? e : urldb_add_host_गाँठो(part + 1, d);
		यदि (!d)
			रोक;

		*part = '\0';
	} जबसम्म (1);

	फर्काऊ d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
स्थिर bool
urldb_घुसाउ_cookie(संरचना cookie_internal_data *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	संरचना cookie_internal_data *d;
	अचर संरचना host_part *h;
	संरचना path_data *p;
	समय_प्रकार now = समय(रिक्त);

	प्रतिज्ञा(c);

	यदि (c->domain[0] == '.') {
		h = urldb_खोज_find(
			urldb_get_खोज_रूख(&(c->domain[1])),
			c->domain + 1);
		यदि (!h) {
			h = urldb_add_host(c->domain + 1);
			यदि (!h) {
				urldb_free_cookie(c);
				फर्काऊ false;
			}
		}

		p = (संरचना path_data *) &h->paths;
	} अन्यथा {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		प्रतिज्ञा(url != रिक्त);
		प्रतिज्ञा(scheme != रिक्त);

		h = urldb_खोज_find(
			urldb_get_खोज_रूख(c->domain),
			c->domain);

		यदि (!h) {
			h = urldb_add_host(c->domain);
			यदि (!h) {
				urldb_free_cookie(c);
				फर्काऊ false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), रिक्त, url);
		यदि (!p) {
			urldb_free_cookie(c);
			फर्काऊ false;
		}
	}

	/* add cookie */
	लागि (d = p->cookies; d; d = d->next) {
		यदि (!म_भेद(d->domain, c->domain) &&
		    !म_भेद(d->path, c->path) &&
		    !म_भेद(d->name, c->name))
			रोक;
	}

	यदि (d) {
		यदि (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			यदि (d->next)
				d->next->prev = d->prev;
			अन्यथा
				p->cookies_end = d->prev;
			यदि (d->prev)
				d->prev->next = d->next;
			अन्यथा
				p->cookies = d->next;

			cookie_manager_remove((संरचना cookie_data *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} अन्यथा {
			/* replace d with c */
			c->prev = d->prev;
			c->next = d->next;
			यदि (c->next)
				c->next->prev = c;
			अन्यथा
				p->cookies_end = c;
			यदि (c->prev)
				c->prev->next = c;
			अन्यथा
				p->cookies = c;

			cookie_manager_remove((संरचना cookie_data *)d);
			urldb_free_cookie(d);

			cookie_manager_add((संरचना cookie_data *)c);
		}
	} अन्यथा {
		c->prev = p->cookies_end;
		c->next = रिक्त;
		यदि (p->cookies_end)
			p->cookies_end->next = c;
		अन्यथा
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((संरचना cookie_data *)c);
	}

	फर्काऊ true;
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
स्थिर bool
concat_netscape_cookie(संरचना cookie_internal_data *c,
		    पूर्णाङ्क *used,
		    पूर्णाङ्क *alloc,
		    वर्ण **buf)
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
	पूर्णाङ्क max_len;
	पूर्णाङ्क outl; /* output length */

	प्रतिज्ञा(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		म_माप(c->name) + 1 +
		म_माप(c->मान) + 2 +
		(c->path_from_set ? 8 + म_माप(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + म_माप(c->domain) + 2 : 0);
	यदि (max_len >= *alloc) {
		वर्ण *temp = पुनः_स्मृति(*buf, max_len);
		यदि (!temp) {
			फर्काऊ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	यदि (outl >= (*alloc - *used)) {
		/* output truncated */
		फर्काऊ false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	यदि (c->मान_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->मान);
		यदि (outl >= (*alloc - *used)) {
			/* output truncated */
			फर्काऊ false;
		}
		*used += outl;
	} अन्यथा {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->मान);
		यदि (outl >= (*alloc - *used)) {
			/* output truncated */
			फर्काऊ false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	फर्काऊ true;
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
स्थिर bool
concat_rfc2965_cookie(संरचना cookie_internal_data *c,
		      पूर्णाङ्क *used,
		      पूर्णाङ्क *alloc,
		      वर्ण **buf)
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
	अचर वर्ण * अचर separators = "()<>@,;:\\\"/[]?={} \t";
	पूर्णाङ्क max_len;
	पूर्णाङ्क outl; /* output length */

	प्रतिज्ञा(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		म_माप(c->name) + 1 +
		म_माप(c->मान) + 2 +
		(c->path_from_set ?
		 8 + म_माप(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + म_माप(c->domain) + 2 : 0);

	यदि (max_len >= *alloc) {
		वर्ण *temp = पुनः_स्मृति(*buf, max_len);
		यदि (!temp) {
			फर्काऊ false;
		}
		*buf = temp;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	यदि (outl >= (*alloc - *used)) {
		/* output truncated */
		फर्काऊ false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	यदि (c->मान_was_quoted ||
	    strpbrk(c->मान, separators) != रिक्त) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->मान);

	} अन्यथा {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->मान);
	}
	यदि (outl >= (*alloc - *used)) {
		/* output truncated */
		फर्काऊ false;
	}
	*used += outl;

	यदि (c->path_from_set) {
		/* Path, quoted if necessary */

		यदि (strpbrk(c->path, separators) != रिक्त) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} अन्यथा {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		यदि (outl >= (*alloc - *used)) {
			/* output truncated */
			फर्काऊ false;
		}
		*used += outl;
	}

	यदि (c->domain_from_set) {
		/* Domain, quoted if necessary */
		यदि (strpbrk(c->domain, separators) != रिक्त) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} अन्यथा {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		यदि (outl >= (*alloc - *used)) {
			/* output truncated */
			फर्काऊ false;
		}
		*used += outl;
	}

	फर्काऊ true;
}


/**
 * build output cookie string in netscape format
 */
स्थिर वर्ण *
get_netscape_cookie(संरचना cookie_internal_data **matched_cookies, पूर्णाङ्क गन्ती)
{
	पूर्णाङ्क ret_alloc = 4096, ret_used = 1;
	वर्ण *ret;
	पूर्णाङ्क midx; /* index into matched cookies */

	ret = दो_स्मृति(ret_alloc);
	यदि (ret == रिक्त) {
		फर्काऊ रिक्त;
	}
	ret[0] = '\0';

	लागि (midx = 0; midx < गन्ती; midx++) {
		यदि (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			मुक्त(ret);
			फर्काऊ रिक्त;
		}
	}

	यदि (ret_used <= 2) {
		मुक्त(ret);
		फर्काऊ रिक्त;
	}

	/* Old-style cookies => no version & skip "; " */
	स_हटाओ(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	वर्ण *temp = पुनः_स्मृति(ret, ret_used);
	यदि (temp != रिक्त) {
		ret = temp;
	}

	फर्काऊ ret;
}


/**
 * build output cookie string in RFC2965 format
 */
स्थिर वर्ण *
get_rfc2965_cookie(संरचना cookie_internal_data **matched_cookies,
		   पूर्णाङ्क गन्ती,
		   पूर्णाङ्क version)
{
	पूर्णाङ्क ret_alloc = 4096; /* return string allocation */
	पूर्णाङ्क ret_used = 1; /* return sting buffer used */
	वर्ण *ret; /* return string buffer */
	पूर्णाङ्क midx; /* index into matched cookies */
	पूर्णाङ्क outl;

	ret = दो_स्मृति(ret_alloc);
	यदि (ret == रिक्त) {
		फर्काऊ रिक्त;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	यदि (outl >= ret_alloc - ret_used) {
		/* overflow */
		मुक्त(ret);
		फर्काऊ रिक्त;
	}
	ret_used += outl;

	लागि (midx = 0; midx < गन्ती; midx++) {
		यदि (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			मुक्त(ret);
			फर्काऊ रिक्त;
		}
	}

	/* shrink the output buffer to the required size */
	वर्ण *temp = पुनः_स्मृति(ret, ret_used);
	यदि (temp != रिक्त) {
		ret = temp;
	}

	फर्काऊ ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
स्थिर शून्य
urldb_delete_cookie_paths(अचर वर्ण *domain,
			  अचर वर्ण *path,
			  अचर वर्ण *name,
			  संरचना path_data *parent)
{
	संरचना cookie_internal_data *c;
	संरचना path_data *p = parent;

	प्रतिज्ञा(parent);

	गर {
		लागि (c = p->cookies; c; c = c->next) {
			यदि (म_भेद(c->domain, domain) == 0 &&
			    म_भेद(c->path, path) == 0 &&
			    म_भेद(c->name, name) == 0) {
				यदि (c->prev) {
					c->prev->next = c->next;
				} अन्यथा {
					p->cookies = c->next;
				}

				यदि (c->next) {
					c->next->prev = c->prev;
				} अन्यथा {
					p->cookies_end = c->prev;
				}

				urldb_free_cookie(c);

				फर्काऊ;
			}
		}

		यदि (p->children) {
			p = p->children;
		} अन्यथा {
			जबसम्म (p != parent) {
				यदि (p->next != रिक्त) {
					p = p->next;
					रोक;
				}

				p = p->parent;
			}
		}
	} जबसम्म (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
स्थिर शून्य
urldb_delete_cookie_hosts(अचर वर्ण *domain,
			  अचर वर्ण *path,
			  अचर वर्ण *name,
			  संरचना host_part *parent)
{
	संरचना host_part *h;
	प्रतिज्ञा(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	लागि (h = parent->children; h; h = h->next) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
स्थिर शून्य urldb_save_cookie_paths(खाता *fp, संरचना path_data *parent)
{
	संरचना path_data *p = parent;
	समय_प्रकार now = समय(रिक्त);

	प्रतिज्ञा(fp && parent);

	गर {
		यदि (p->cookies != रिक्त) {
			संरचना cookie_internal_data *c;

			लागि (c = p->cookies; c != रिक्त; c = c->next) {
				यदि (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					जारी;
				}

				ख_लिखो(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->version, c->domain,
					c->domain_from_set, c->path,
					c->path_from_set, c->secure,
					c->http_only,
					(पूर्णाङ्क)c->expires, (पूर्णाङ्क)c->last_used,
					c->no_destroy, c->name, c->मान,
					c->मान_was_quoted,
					p->scheme ? lwc_string_data(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			जबसम्म (p != parent) {
				यदि (p->next != रिक्त) {
					p = p->next;
					रोक;
				}

				p = p->parent;
			}
		}
	} जबसम्म (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
स्थिर शून्य urldb_save_cookie_hosts(खाता *fp, संरचना host_part *parent)
{
	संरचना host_part *h;
	प्रतिज्ञा(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	लागि (h = parent->children; h; h = h->next)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
स्थिर शून्य urldb_destroy_cookie(संरचना cookie_internal_data *c)
{
	मुक्त(c->name);
	मुक्त(c->मान);
	मुक्त(c->comment);
	मुक्त(c->domain);
	मुक्त(c->path);

	मुक्त(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
स्थिर शून्य urldb_destroy_path_गाँठो_content(संरचना path_data *गाँठो)
{
	संरचना cookie_internal_data *a, *b;
	अचिन्हित पूर्णाङ्क i;

	यदि (गाँठो->url != रिक्त) {
		nsurl_unref(गाँठो->url);
	}

	lwc_string_unref(गाँठो->scheme);

	मुक्त(गाँठो->segment);
	लागि (i = 0; i < गाँठो->frag_cnt; i++)
		मुक्त(गाँठो->fragment[i]);
	मुक्त(गाँठो->fragment);

	मुक्त(गाँठो->urld.title);

	लागि (a = गाँठो->cookies; a; a = b) {
		b = a->next;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
स्थिर शून्य urldb_destroy_prot_space(संरचना prot_space_data *space)
{
	lwc_string_unref(space->scheme);
	मुक्त(space->realm);
	मुक्त(space->auth);

	मुक्त(space);
}


/**
 * Destroy a path tree
 *
 * \param root Root node of tree to destroy
 */
स्थिर शून्य urldb_destroy_path_रूख(संरचना path_data *जरा)
{
	संरचना path_data *p = जरा;

	गर {
		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			संरचना path_data *q = p;

			जबसम्म (p != जरा) {
				यदि (p->next != रिक्त) {
					p = p->next;
					रोक;
				}

				p = p->parent;

				urldb_destroy_path_गाँठो_content(q);
				मुक्त(q);

				q = p;
			}

			urldb_destroy_path_गाँठो_content(q);
			मुक्त(q);
		}
	} जबसम्म (p != जरा);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
स्थिर शून्य urldb_destroy_host_रूख(संरचना host_part *जरा)
{
	संरचना host_part *a, *b;
	संरचना path_data *p, *q;
	संरचना prot_space_data *s, *t;

	/* Destroy children */
	लागि (a = जरा->children; a; a = b) {
		b = a->next;
		urldb_destroy_host_रूख(a);
	}

	/* Now clean up paths */
	लागि (p = जरा->paths.children; p; p = q) {
		q = p->next;
		urldb_destroy_path_रूख(p);
	}

	/* Root path */
	urldb_destroy_path_गाँठो_content(&जरा->paths);

	/* Proctection space data */
	लागि (s = जरा->prot_space; s; s = t) {
		t = s->next;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	मुक्त(जरा->part);
	मुक्त(जरा);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
स्थिर शून्य urldb_destroy_खोज_रूख(संरचना खोज_गाँठो *जरा)
{
	/* Destroy children */
	यदि (जरा->बायाँ != &empty)
		urldb_destroy_खोज_रूख(जरा->बायाँ);
	यदि (जरा->दायाँ != &empty)
		urldb_destroy_खोज_रूख(जरा->दायाँ);

	/* And destroy ourselves */
	मुक्त(जरा);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
शून्य urldb_destroy(शून्य)
{
	संरचना host_part *a, *b;
	पूर्णाङ्क i;

	/* Clean up search trees */
	लागि (i = 0; i < अङ्क_खोज_TREES; i++) {
		यदि (खोज_trees[i] != &empty) {
			urldb_destroy_खोज_रूख(खोज_trees[i]);
			खोज_trees[i] = &empty;
		}
	}

	/* And database */
	लागि (a = db_जरा.children; a; a = b) {
		b = a->next;
		urldb_destroy_host_रूख(a);
	}
	स_रखो(&db_जरा, 0, परिमाण(db_जरा));

	/* And the bloom filter */
	यदि (url_bloom != रिक्त) {
		bloom_destroy(url_bloom);
		url_bloom = रिक्त;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(अचर वर्ण *filename)
{
#define MAXIMUM_URL_LENGTH 4096
	वर्ण s[MAXIMUM_URL_LENGTH];
	वर्ण host[256];
	संरचना host_part *h;
	पूर्णाङ्क urls;
	पूर्णाङ्क i;
	पूर्णाङ्क version;
	पूर्णाङ्क length;
	खाता *fp;

	प्रतिज्ञा(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	यदि (url_bloom == रिक्त)
		url_bloom = bloom_create(BLOOM_SIZE);

	fp = ख_खोलो(filename, "r");
	यदि (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		फर्काऊ NSERROR_NOT_FOUND;
	}

	यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp)) {
		ख_बंद(fp);
		फर्काऊ NSERROR_NEED_DATA;
	}

	version = म_से_प(s);
	यदि (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		ख_बंद(fp);
		फर्काऊ NSERROR_INVALID;
	}
	यदि (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		ख_बंद(fp);
		फर्काऊ NSERROR_INVALID;
	}

	जबसम्म (ख_माला_लो(host, परिमाण host, fp)) {
		समय_प्रकार hsts_expiry = 0;
		पूर्णाङ्क hsts_include_sub_domains = 0;

		/* get the hostname */
		length = म_माप(host) - 1;
		host[length] = '\0';

		/* skip data that has ended up with a host of '' */
		यदि (length == 0) {
			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;
			urls = म_से_प(s);
			/* Eight fields/url */
			लागि (i = 0; i < (8 * urls); i++) {
				यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
					रोक;
			}
			जारी;
		}

		यदि (version >= 107) {
			वर्ण *p = host;
			जबसम्म (*p && *p != ' ') p++;
			जबसम्म (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			जबसम्म (*p && *p != ' ') p++;
			जबसम्म (*p && *p == ' ') p++;
			nsc_snptimet(p, म_माप(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		यदि (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			ख_बंद(fp);
			फर्काऊ NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
			रोक;
		urls = म_से_प(s);

		/* no URLs => try next host */
		यदि (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			जारी;
		}

		/* load the non-corrupt data */
		लागि (i = 0; i < urls; i++) {
			संरचना path_data *p = रिक्त;
			वर्ण scheme[64], ports[10];
			वर्ण url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			अचिन्हित पूर्णाङ्क port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			वर्ण *path_query;
			माप_प्रकार len;

			यदि (!ख_माला_लो(scheme, परिमाण scheme, fp))
				रोक;
			length = म_माप(scheme) - 1;
			scheme[length] = '\0';

			यदि (!ख_माला_लो(ports, परिमाण ports, fp))
				रोक;
			length = म_माप(ports) - 1;
			ports[length] = '\0';
			port = म_से_प(ports);

			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;
			length = म_माप(s) - 1;
			s[length] = '\0';

			यदि (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, परिमाण url, "%s://%s%s%s%s",
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
			यदि (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				फर्काऊ NSERROR_NOMEM;
			}

			यदि (url_bloom != रिक्त) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_घुसाउ_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			यदि (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				फर्काऊ NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			यदि (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				फर्काऊ NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;
			यदि (p)
				p->urld.visits = (अचिन्हित पूर्णाङ्क)म_से_प(s);

			/* entry last use time */
			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp)) {
				रोक;
			}
			यदि (p) {
				nsc_snptimet(s, म_माप(s) - 1, &p->urld.last_visit);
			}

			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;
			यदि (p)
				p->urld.type = (content_type)म_से_प(s);

			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;


			यदि (!ख_माला_लो(s, MAXIMUM_URL_LENGTH, fp))
				रोक;
			length = म_माप(s) - 1;
			यदि (p && length > 0) {
				s[length] = '\0';
				p->urld.title = दो_स्मृति(length + 1);
				यदि (p->urld.title)
					स_नकल(p->urld.title, s, length + 1);
			}
		}
	}

	ख_बंद(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#undef MAXIMUM_URL_LENGTH

	फर्काऊ NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(अचर वर्ण *filename)
{
	खाता *fp;
	पूर्णाङ्क i;

	प्रतिज्ञा(filename);

	fp = ख_खोलो(filename, "w");
	यदि (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		फर्काऊ NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	ख_लिखो(fp, "%d\n", URL_FILE_VERSION);

	लागि (i = 0; i != अङ्क_खोज_TREES; i++) {
		urldb_save_खोज_रूख(खोज_trees[i], fp);
	}

	ख_बंद(fp);

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	संरचना path_data *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		फर्काऊ NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	संरचना host_part *h;
	संरचना path_data *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	अचर वर्ण *host_str;
	वर्ण *path_query = रिक्त;
	माप_प्रकार len;
	bool match;
	अचिन्हित पूर्णाङ्क port_int;

	प्रतिज्ञा(url);

	यदि (url_bloom == रिक्त)
		url_bloom = bloom_create(BLOOM_SIZE);

	यदि (url_bloom != रिक्त) {
		uint32_t hash = nsurl_hash(url);
		bloom_घुसाउ_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	यदि (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		फर्काऊ false;
	}
	प्रतिज्ञा(path_query != रिक्त);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त) {
		मुक्त(path_query);
		फर्काऊ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		host_str = lwc_string_data(host);
		lwc_string_unref(host);

	} अन्यथा यदि (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		मुक्त(path_query);
		फर्काऊ false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	यदि (port != रिक्त) {
		port_int = म_से_प(lwc_string_data(port));
		lwc_string_unref(port);
	} अन्यथा {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	यदि (h != रिक्त) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} अन्यथा {
		p = रिक्त;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	फर्काऊ (p != रिक्त);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, अचर वर्ण *title)
{
	संरचना path_data *p;
	वर्ण *temp;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (p == रिक्त) {
		फर्काऊ NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	यदि (title != रिक्त) {
		temp = strdup(title);
		यदि (temp == रिक्त) {
			फर्काऊ NSERROR_NOMEM;
		}
	} अन्यथा {
		temp = रिक्त;
	}

	मुक्त(p->urld.title);
	p->urld.title = temp;

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	संरचना path_data *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		फर्काऊ NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_data(nsurl *url)
{
	संरचना path_data *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		फर्काऊ NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = समय(रिक्त);
	p->urld.visits++;

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
शून्य urldb_reset_url_visit_data(nsurl *url)
{
	संरचना path_data *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ;

	p->urld.last_visit = (समय_प्रकार)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
अचर संरचना url_data *urldb_get_url_data(nsurl *url)
{
	संरचना path_data *p;
	संरचना url_internal_data *u;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ रिक्त;

	u = &p->urld;

	फर्काऊ (अचर संरचना url_data *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	संरचना path_data *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ रिक्त;

	फर्काऊ p->url;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_set_auth_details(nsurl *url, अचर वर्ण *realm, अचर वर्ण *auth)
{
	संरचना path_data *p, *pi;
	संरचना host_part *h;
	संरचना prot_space_data *space, *space_alloc;
	वर्ण *realm_alloc, *auth_alloc;
	bool match;

	प्रतिज्ञा(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	यदि (!p)
		फर्काऊ;

	/* Search for host_part */
	लागि (pi = p; pi->parent != रिक्त; pi = pi->parent)
		;
	h = (संरचना host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	लागि (space = h->prot_space; space; space = space->next) {
		यदि (!म_भेद(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			रोक;
	}

	यदि (space != रिक्त) {
		/* Overrule existing auth. */
		मुक्त(space->auth);
		space->auth = strdup(auth);
	} अन्यथा {
		/* Create a new protection space. */
		space = space_alloc = दो_स्मृति(परिमाण(संरचना prot_space_data));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		यदि (!space_alloc || !realm_alloc || !auth_alloc) {
			मुक्त(space_alloc);
			मुक्त(realm_alloc);
			मुक्त(auth_alloc);
			फर्काऊ;
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
अचर वर्ण *urldb_get_auth_details(nsurl *url, अचर वर्ण *realm)
{
	संरचना path_data *p, *p_cur, *p_शीर्ष;

	प्रतिज्ञा(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ रिक्त;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	लागि (p_cur = p; p_cur != रिक्त; p_शीर्ष = p_cur, p_cur = p_cur->parent) {
		यदि (p_cur->prot_space) {
			फर्काऊ p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	यदि (realm != रिक्त) {
		अचर संरचना host_part *h = (अचर संरचना host_part *)p_शीर्ष;
		अचर संरचना prot_space_data *space;
		bool match;

		/* Search for a possible matching protection space. */
		लागि (space = h->prot_space; space != रिक्त;
		     space = space->next) {
			यदि (!म_भेद(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				फर्काऊ p->prot_space->auth;
			}
		}
	}

	फर्काऊ रिक्त;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_set_cert_permissions(nsurl *url, bool permit)
{
	संरचना path_data *p;
	संरचना host_part *h;

	प्रतिज्ञा(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ;

	लागि (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (संरचना host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	संरचना path_data *p;
	अचर संरचना host_part *h;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ false;

	लागि (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (अचर संरचना host_part *)p;

	फर्काऊ h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(संरचना nsurl *url, अचर वर्ण *header)
{
	संरचना path_data *p;
	संरचना host_part *h;
	lwc_string *host;
	समय_प्रकार now = समय(रिक्त);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	प्रतिज्ञा(url);

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		यदि (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			फर्काऊ true;
		} अन्यथा यदि (lwc_string_length(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			फर्काऊ true;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: ignore */
		फर्काऊ true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ false;

	लागि (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (संरचना host_part *)p;
	यदि (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		फर्काऊ true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	यदि (error != NSERROR_OK) {
		/* Parse failed: ignore */
		फर्काऊ true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	यदि (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} अन्यथा यदि ((समय_प्रकार) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	फर्काऊ true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(संरचना nsurl *url)
{
	संरचना path_data *p;
	अचर संरचना host_part *h;
	lwc_string *host;
	समय_प्रकार now = समय(रिक्त);

	प्रतिज्ञा(url);

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		यदि (urldb__host_is_ip_address(lwc_string_data(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			फर्काऊ false;
		} अन्यथा यदि (lwc_string_length(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			फर्काऊ false;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: not enabled */
		फर्काऊ false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ false;

	लागि (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (अचर संरचना host_part *)p;

	/* Consult record for this host */
	यदि (h->hsts.expires > now) {
		/* Not expired */
		फर्काऊ true;
	}

	/* Consult parent domains */
	लागि (h = h->parent; h && h != &db_जरा; h = h->parent) {
		यदि (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			फर्काऊ true;
		}
	}

	फर्काऊ false;
}


/* exported interface documented in netsurf/url_db.h */
शून्य
urldb_iterate_partial(अचर वर्ण *prefix,
		      bool (*callback)(nsurl *url, अचर संरचना url_data *data))
{
	वर्ण host[256];
	वर्ण buf[260]; /* max domain + "www." */
	अचर वर्ण *slash, *scheme_sep;
	संरचना खोज_गाँठो *रूख;
	अचर संरचना host_part *h;

	प्रतिज्ञा(prefix && callback);

	/* strip scheme */
	scheme_sep = म_माला(prefix, "://");
	यदि (scheme_sep)
		prefix = scheme_sep + 3;

	slash = म_अक्षर(prefix, '/');
	रूख = urldb_get_खोज_रूख(prefix);

	यदि (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, परिमाण host, "%.*s",
			 (पूर्णाङ्क) (slash - prefix), prefix);

		h = urldb_खोज_find(रूख, host);
		यदि (!h) {
			पूर्णाङ्क len = slash - prefix;

			यदि (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, परिमाण buf, "www.%s", host);
				h = urldb_खोज_find(
					खोज_trees[ST_DN + 'w' - 'a'],
					buf);
				यदि (!h)
					फर्काऊ;
			} अन्यथा
				फर्काऊ;
		}

		यदि (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} अन्यथा {
		पूर्णाङ्क len = म_माप(prefix);

		/* looking for hosts */
		यदि (!urldb_iterate_partial_host(रूख, prefix, callback))
			फर्काऊ;

		यदि (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, परिमाण buf, "www.%s", prefix);
			यदि(!urldb_iterate_partial_host(
				   खोज_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				फर्काऊ;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
शून्य
urldb_iterate_entries(bool (*callback)(nsurl *url, अचर संरचना url_data *data))
{
	पूर्णाङ्क i;

	प्रतिज्ञा(callback);

	लागि (i = 0; i < अङ्क_खोज_TREES; i++) {
		यदि (!urldb_iterate_entries_host(खोज_trees[i],
						callback,
						रिक्त)) {
			रोक;
		}
	}
}


/* exported interface documented in content/urldb.h */
शून्य urldb_iterate_cookies(bool (*callback)(अचर संरचना cookie_data *data))
{
	पूर्णाङ्क i;

	प्रतिज्ञा(callback);

	लागि (i = 0; i < अङ्क_खोज_TREES; i++) {
		यदि (!urldb_iterate_entries_host(खोज_trees[i],
						रिक्त, callback))
			रोक;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(अचर वर्ण *header, nsurl *url, nsurl *referer)
{
	अचर वर्ण *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	प्रतिज्ञा(url && header);

	/* Get defragmented URL, as 'urlt' */
	यदि (nsurl_defragment(url, &urlt) != NSERROR_OK)
		फर्काऊ रिक्त;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त) {
		nsurl_unref(urlt);
		फर्काऊ false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	यदि (path == रिक्त) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		फर्काऊ false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host == रिक्त) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		फर्काऊ false;
	}

	यदि (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		यदि (rhost == रिक्त) {
			गच्छ error;
		}

		/* Domain match host names */
		यदि (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			अचर वर्ण *hptr;
			अचर वर्ण *rptr;
			अचर वर्ण *dot;
			अचर वर्ण *host_data = lwc_string_data(host);
			अचर वर्ण *rhost_data = lwc_string_data(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			यदि (urldb__host_is_ip_address(host_data) ||
			    urldb__host_is_ip_address(rhost_data)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				गच्छ error;
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
			जबसम्म (hptr >= host_data && rptr >= rhost_data) {
				यदि (*hptr != *rptr)
					रोक;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			जबसम्म (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			यदि (*hptr == '\0' ||
			    (dot = म_अक्षर(hptr + 1, '.')) == रिक्त ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				गच्छ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + म_माप(cur) - 2 /* Trailing CRLF */;

	गर {
		संरचना cookie_internal_data *c;
		वर्ण *dot;
		माप_प्रकार len;
#ifdef WITH_NSPSL
		अचर वर्ण *suffix;
#endif

		c = urldb_parse_cookie(url, &cur);
		यदि (!c) {
			/* failed => stop parsing */
			गच्छ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		यदि (!c->name || !c->मान) {
			urldb_free_cookie(c);
			गच्छ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = म_माप(c->path);
		यदि (len > lwc_string_length(path) ||
		    म_भेदन(c->path, lwc_string_data(path),
			    len) != 0) {
			urldb_free_cookie(c);
			गच्छ error;
		}

#ifdef WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		यदि (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		यदि (suffix == रिक्त) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			गच्छ error;
		}
#अन्यथा
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = म_अक्षर(c->domain + 1, '.');
		यदि (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			गच्छ error;
		}
#endif

		/* Domain match fetch host with cookie domain */
		यदि (strcasecmp(lwc_string_data(host), c->domain) != 0) {
			पूर्णाङ्क hlen, dlen;
			वर्ण *domain = c->domain;

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
			प्रतिज्ञा(c->domain[0] == '.');

			/* 4.3.2:iii */
			यदि (urldb__host_is_ip_address(lwc_string_data(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				गच्छ error;
			}

			hlen = lwc_string_length(host);
			dlen = म_माप(c->domain);

			यदि (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				गच्छ error;
			}

			यदि (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			यदि (strcasecmp(lwc_string_data(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				गच्छ error;
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
		यदि (!urldb_घुसाउ_cookie(c, scheme, urlt))
			गच्छ error;
	} जबसम्म (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	फर्काऊ true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	फर्काऊ false;
}


/* exported interface documented in content/urldb.h */
वर्ण *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	अचर संरचना path_data *p, *q;
	अचर संरचना host_part *h;
	lwc_string *path_lwc;
	संरचना cookie_internal_data *c;
	पूर्णाङ्क गन्ती = 0, version = COOKIE_RFC2965;
	संरचना cookie_internal_data **matched_cookies;
	पूर्णाङ्क matched_cookies_size = 20;
	अचर वर्ण *path;
	वर्ण *ret;
	lwc_string *scheme;
	समय_प्रकार now;
	bool match;

	प्रतिज्ञा(url != रिक्त);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		फर्काऊ रिक्त;

	scheme = p->scheme;

	matched_cookies = दो_स्मृति(matched_cookies_size *
				 परिमाण(संरचना cookie_internal_data *));
	यदि (!matched_cookies)
		फर्काऊ रिक्त;

#define GROW_MATCHED_COOKIES						\
	गर {								\
		यदि (गन्ती == matched_cookies_size) {			\
			संरचना cookie_internal_data **temp;		\
			temp = पुनः_स्मृति(matched_cookies,			\
				       (matched_cookies_size + 20) *	\
				       परिमाण(संरचना cookie_internal_data *)); \
									\
			यदि (temp == रिक्त) {				\
				मुक्त(matched_cookies);			\
				फर्काऊ रिक्त;				\
			}						\
									\
			matched_cookies = temp;				\
			matched_cookies_size += 20;			\
		}							\
	} जबसम्म(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	यदि (path_lwc == रिक्त) {
		मुक्त(matched_cookies);
		फर्काऊ रिक्त;
	}
	path = lwc_string_data(path_lwc);
	lwc_string_unref(path_lwc);

	now = समय(रिक्त);

	यदि (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		लागि (q = p->parent->children; q; q = q->next) {
			यदि (म_भेद(q->segment, p->segment))
				जारी;

			/* Consider all cookies associated with
			 * this exact path */
			लागि (c = q->cookies; c; c = c->next) {
				यदि (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					जारी;

				यदि (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					जारी;

				यदि (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					जारी;

				matched_cookies[गन्ती++] = c;

				GROW_MATCHED_COOKIES;

				यदि (c->version < (अचिन्हित पूर्णाङ्क)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((संरचना cookie_data *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	लागि (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		लागि (q = p->children; q; q = q->next) {
			यदि (*(q->segment) != '\0')
				जारी;

			लागि (c = q->cookies; c; c = c->next) {
				यदि (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					जारी;

				यदि (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					जारी;

				matched_cookies[गन्ती++] = c;

				GROW_MATCHED_COOKIES;

				यदि (c->version < (अचिन्हित पूर्णाङ्क) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((संरचना cookie_data *)c);
			}
		}

		यदि (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			रोक;
		}

		/* Consider p itself - may be the result of Path=/foo */
		लागि (c = p->cookies; c; c = c->next) {
			यदि (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				जारी;

			/* Ensure cookie path is a prefix of the resource */
			यदि (म_भेदन(c->path, path, म_माप(c->path)) != 0)
				/* paths don't match => ignore */
				जारी;

			यदि (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				जारी;

			matched_cookies[गन्ती++] = c;

			GROW_MATCHED_COOKIES;

			यदि (c->version < (अचिन्हित पूर्णाङ्क) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((संरचना cookie_data *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	लागि (h = (अचर संरचना host_part *)p; h && h != &db_जरा;
	     h = h->parent) {
		लागि (c = h->paths.cookies; c; c = c->next) {
			यदि (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				जारी;

			/* Ensure cookie path is a prefix of the resource */
			यदि (म_भेदन(c->path, path, म_माप(c->path)) != 0)
				/* paths don't match => ignore */
				जारी;

			यदि (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				जारी;

			matched_cookies[गन्ती++] = c;

			GROW_MATCHED_COOKIES;

			यदि (c->version < (अचिन्हित पूर्णाङ्क)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((संरचना cookie_data *)c);
		}
	}
#undef GROW_MATCHED_COOKIES

	यदि (गन्ती == 0) {
		/* No cookies found */
		मुक्त(matched_cookies);
		फर्काऊ रिक्त;
	}

	यदि (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, गन्ती);
	} अन्यथा {
		ret = get_rfc2965_cookie(matched_cookies, गन्ती, version);
	}
	मुक्त(matched_cookies);

	फर्काऊ ret;
}


/* exported interface documented in content/urldb.h */
शून्य urldb_delete_cookie(अचर वर्ण *domain, अचर वर्ण *path,
			 अचर वर्ण *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_जरा);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(अचर वर्ण *filename)
{
	खाता *fp;
	वर्ण s[16*1024];

	प्रतिज्ञा(filename);

	fp = ख_खोलो(filename, "r");
	यदि (!fp)
		फर्काऊ NSERROR_NOT_FOUND;

#define FIND_T {				\
		लागि (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		यदि (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			जारी;		\
		}				\
		*p++ = '\0';			\
	}

#define SKIP_T {				\
		लागि (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		यदि (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			जारी;		\
		}				\
	}

	जबसम्म (ख_माला_लो(s, परिमाण s, fp)) {
		वर्ण *p = s, *end = 0,
			*domain, *path, *name, *मान, *scheme, *url,
			*comment;
		पूर्णाङ्क version, domain_specified, path_specified,
			secure, http_only, no_destroy, मान_quoted;
		समय_प्रकार expires, last_used;
		संरचना cookie_internal_data *c;

		यदि(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			जारी;

		s[म_माप(s) - 1] = '\0'; /* lose terminating newline */
		end = s + म_माप(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		यदि (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = म_से_प(p);

			यदि (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				रोक;
			}

			जारी;
		} अन्यथा यदि (loaded_cookie_file_version == 0) {
			/* Haven't yet seen version; skip this input */
			जारी;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; version = म_से_प(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = म_से_प(p); FIND_T;
		SKIP_T; path = p; FIND_T;
		SKIP_T; path_specified = म_से_प(p); FIND_T;
		SKIP_T; secure = म_से_प(p); FIND_T;
		यदि (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = म_से_प(p); FIND_T;
		} अन्यथा {
			http_only = 0;
		}
		SKIP_T; expires = (समय_प्रकार)म_से_प(p); FIND_T;
		SKIP_T; last_used = (समय_प्रकार)म_से_प(p); FIND_T;
		SKIP_T; no_destroy = म_से_प(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; मान = p; FIND_T;
		यदि (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	मान_quoted = म_से_प(p); FIND_T;
		} अन्यथा {
			मान_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		लागि (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		प्रतिज्ञा(p <= end);

		/* Now create cookie */
		c = दो_स्मृति(परिमाण(संरचना cookie_internal_data));
		यदि (!c)
			रोक;

		c->name = strdup(name);
		c->मान = strdup(मान);
		c->मान_was_quoted = मान_quoted;
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

		यदि (!(c->name && c->मान && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			रोक;
		}

		यदि (c->domain[0] != '.') {
			lwc_string *scheme_lwc = रिक्त;
			nsurl *url_nsurl = रिक्त;

			प्रतिज्ञा(scheme[0] != 'u');

			यदि (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				रोक;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			यदि (!urldb_घुसाउ_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				रोक;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} अन्यथा {
			यदि (!urldb_घुसाउ_cookie(c, रिक्त, रिक्त)) {
				/* Cookie freed for us */
				रोक;
			}
		}
	}

#undef SKIP_T
#undef FIND_T

	ख_बंद(fp);

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(अचर वर्ण *filename)
{
	खाता *fp;
	पूर्णाङ्क cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	प्रतिज्ञा(filename);

	fp = ख_खोलो(filename, "w");
	यदि (!fp)
		फर्काऊ NSERROR_NOT_FOUND;

	ख_लिखो(fp, "# NetSurf cookies file.\n"
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
	ख_लिखो(fp, "Version:\t%d\n", cookie_file_version);

	urldb_save_cookie_hosts(fp, &db_जरा);

	ख_बंद(fp);

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_dump(शून्य)
{
	पूर्णाङ्क i;

	urldb_dump_hosts(&db_जरा);

	लागि (i = 0; i != अङ्क_खोज_TREES; i++) {
		urldb_dump_खोज(खोज_trees[i], 0);
	}
}
