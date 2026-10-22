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

#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <time.h>
#अगर_घोषित WITH_NSPSL
#समावेश <nspsl.h>
#पूर्ण_अगर

#समावेश "utils/inet.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/log.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/url.h"
#समावेश "utils/utils.h"
#समावेश "utils/bloom.h"
#समावेश "utils/time.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/ascii.h"
#समावेश "utils/http.h"
#समावेश "netsurf/bitmap.h"
#समावेश "desktop/cookie_manager.h"

#समावेश "content/content.h"
#समावेश "content/urldb.h"

/**
 * cookie entry.
 *
 * \warning This *must* be kept in sync with the public interface in
 *   netsurf/cookie_db.h
 */
काष्ठा cookie_internal_तथ्य {
	काष्ठा cookie_internal_तथ्य *पूर्व;	/**< Previous in list */
	काष्ठा cookie_internal_तथ्य *अग्र;	/**< Next in list */

	अक्षर *name;		/**< Cookie name */
	अक्षर *मूल्य;		/**< Cookie value */
	bool मूल्य_was_quoted;	/**< Value was quoted in Set-Cookie: */
	अक्षर *comment;		/**< Cookie comment */
	bool domain_from_set;	/**< Domain came from Set-Cookie: header */
	अक्षर *domain;		/**< Domain */
	bool path_from_set;	/**< Path came from Set-Cookie: header */
	अक्षर *path;		/**< Path */
	समय_प्रकार expires;		/**< Expiry timestamp, or -1 for session */
	समय_प्रकार last_used;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	क्रमागत cookie_version version;	/**< Specification compliance */
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
काष्ठा prot_space_तथ्य {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	अचिन्हित पूर्णांक port;
	/** Protection realm */
	अक्षर *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	अक्षर *auth;
	/** Next sibling */
	काष्ठा prot_space_तथ्य *अग्र;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
काष्ठा url_internal_तथ्य {
	अक्षर *title;		/**< Resource title */
	अचिन्हित पूर्णांक visits;	/**< Visit count */
	समय_प्रकार last_visit;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
काष्ठा path_तथ्य {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	अचिन्हित पूर्णांक port;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	अक्षर *segment;		/**< Path segment for this node */
	अचिन्हित पूर्णांक frag_cnt;	/**< Number of entries in path_data::fragment */
	अक्षर **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	काष्ठा url_internal_तथ्य urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	स्थिर काष्ठा prot_space_तथ्य *prot_space;
	/** Cookies associated with resource */
	काष्ठा cookie_internal_तथ्य *cookies;
	/** Last cookie in list */
	काष्ठा cookie_internal_तथ्य *cookies_end;

	काष्ठा path_तथ्य *अग्र;	/**< Next sibling */
	काष्ठा path_तथ्य *पूर्व;	/**< Previous sibling */
	काष्ठा path_तथ्य *parent; /**< Parent path segment */
	काष्ठा path_तथ्य *children; /**< Child path segments */
	काष्ठा path_तथ्य *last; /**< Last child */
};

काष्ठा hsts_तथ्य {
	समय_प्रकार expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

काष्ठा host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	काष्ठा path_तथ्य paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	काष्ठा hsts_तथ्य hsts;

	/**
	 * Part of host string
	 */
	अक्षर *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	काष्ठा prot_space_तथ्य *prot_space;

	काष्ठा host_part *अग्र;	/**< Next sibling */
	काष्ठा host_part *पूर्व;	/**< Previous sibling */
	काष्ठा host_part *parent; /**< Parent host part */
	काष्ठा host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
काष्ठा खोज_गाँठ {
	स्थिर काष्ठा host_part *तथ्य;	/**< Host tree entry */

	अचिन्हित पूर्णांक level;		/**< Node level */

	काष्ठा खोज_गाँठ *बायाँ;	/**< Left subtree */
	काष्ठा खोज_गाँठ *दायाँ;	/**< Right subtree */
};

/** Root database handle */
अटल काष्ठा host_part db_मूल;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#घोषणा अंक_खोज_TREES 28
#घोषणा ST_IP 0
#घोषणा ST_EE 1
#घोषणा ST_DN 2
अटल काष्ठा खोज_गाँठ empty = { 0, 0, &empty, &empty };
अटल काष्ठा खोज_गाँठ *खोज_trees[अंक_खोज_TREES] = {
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty, &empty, &empty, &empty, &empty,
	&empty, &empty, &empty, &empty
};

/** Minimum cookie database file version */
#घोषणा MIN_COOKIE_FILE_VERSION 100
/** Current cookie database file version */
#घोषणा COOKIE_FILE_VERSION 102
/** loaded cookie file version */
अटल पूर्णांक loaded_cookie_file_version;

/** Minimum URL database file version */
#घोषणा MIN_URL_FILE_VERSION 106
/** Current URL database file version */
#घोषणा URL_FILE_VERSION 107

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
अटल काष्ठा bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#घोषणा BLOOM_आकार (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
अटल nserror urldb_write_timet(खाता *fp, समय_प्रकार val)
{
	पूर्णांक use;
	अक्षर op[32];

	use = nsc_sntimet(op, 32, &val);
	अगर (use == 0) {
		ख_लिखो(fp, "%i\n", (पूर्णांक)val);
	} अन्यथा {
		ख_लिखो(fp, "%.*s\n", use, op);
	}
	वापस NSERROR_OK;
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
अटल व्योम
urldb_write_paths(स्थिर काष्ठा path_तथ्य *parent,
		  स्थिर अक्षर *host,
		  खाता *fp,
		  अक्षर **path,
		  पूर्णांक *path_alloc,
		  पूर्णांक *path_used,
		  समय_प्रकार expiry)
{
	स्थिर काष्ठा path_तथ्य *p = parent;
	पूर्णांक i;

	करो {
		पूर्णांक seglen = p->segment != शून्य ? म_माप(p->segment) : 0;
		पूर्णांक len = *path_used + seglen + 1;

		अगर (*path_alloc < len) {
			अक्षर *अस्थायी;
			अस्थायी = पुनः_स्मृति(*path,
				       (len > 64) ? len : *path_alloc + 64);
			अगर (!अस्थायी) {
				वापस;
			}
			*path = अस्थायी;
			*path_alloc = (len > 64) ? len : *path_alloc + 64;
		}

		अगर (p->segment != शून्य) {
			स_नकल(*path + *path_used - 1, p->segment, seglen);
		}

		अगर (p->children != शून्य) {
			(*path)[*path_used + seglen - 1] = '/';
			(*path)[*path_used + seglen] = '\0';
		} अन्यथा {
			(*path)[*path_used + seglen - 1] = '\0';
			len -= 1;
		}

		*path_used = len;

		अगर (p->children != शून्य) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* leaf node */
			अगर (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				ख_लिखो(fp, "%s\n", lwc_string_तथ्य(p->scheme));

				अगर (p->port) {
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
				ख_लिखो(fp, "%i\n", (पूर्णांक)p->urld.type);

				ख_लिखो(fp, "\n");

				अगर (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					क्रम (i = 0; s[i] != '\0'; i++)
						अगर (s[i] < 32)
							s[i] = ' ';
					क्रम (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					ख_लिखो(fp, "%s\n", p->urld.title);
				} अन्यथा {
					ख_लिखो(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			जबतक (p != parent) {
				पूर्णांक seglen = p->segment != शून्य
					? म_माप(p->segment) : 0;

				/* Remove our segment from the path */
				*path_used -= seglen;
				(*path)[*path_used - 1] = '\0';

				अगर (p->अग्र != शून्य) {
					/* Have a sibling, process that */
					p = p->अग्र;
					अवरोध;
				}

				/* Going up, so remove '/' */
				*path_used -= 1;
				(*path)[*path_used - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबतक (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
अटल व्योम
urldb_गिनती_urls(स्थिर काष्ठा path_तथ्य *मूल,
		 समय_प्रकार expiry,
		 अचिन्हित पूर्णांक *गिनती)
{
	स्थिर काष्ठा path_तथ्य *p = मूल;

	करो {
		अगर (p->children != शून्य) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* No more children, increment count if required */
			अगर (p->persistent ||
			    ((p->urld.last_visit > expiry) &&
			     (p->urld.visits > 0))) {
				(*गिनती)++;
			}

			/* Now, find next node to process. */
			जबतक (p != मूल) {
				अगर (p->अग्र != शून्य) {
					/* Have a sibling, process that */
					p = p->अग्र;
					अवरोध;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबतक (p != मूल);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
अटल व्योम urldb_save_खोज_वृक्ष(काष्ठा खोज_गाँठ *parent, खाता *fp)
{
	अक्षर host[256];
	स्थिर काष्ठा host_part *h;
	अचिन्हित पूर्णांक path_गिनती = 0;
	अक्षर *path, *p, *end;
	पूर्णांक path_alloc = 64, path_used = 1;
	समय_प्रकार expiry, hsts_expiry = 0;
	पूर्णांक hsts_include_subdomains = 0;

	expiry = समय(शून्य) - ((60 * 60 * 24) * nsoption_int(expire_url));

	अगर (parent == &empty)
		वापस;

	urldb_save_खोज_वृक्ष(parent->बायाँ, fp);

	path = दो_स्मृति(path_alloc);
	अगर (!path)
		वापस;

	path[0] = '\0';

	क्रम (h = parent->तथ्य, p = host, end = host + माप host;
	     h && h != &db_मूल && p < end; h = h->parent) {
		पूर्णांक written = snprintf(p, end - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		अगर (written < 0) {
			मुक्त(path);
			वापस;
		}
		p += written;
	}

	h = parent->तथ्य;
	अगर (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_गिनती_urls(&parent->तथ्य->paths, expiry, &path_गिनती);

	अगर (path_गिनती > 0) {
		ख_लिखो(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		ख_लिखो(fp, "%i\n", path_गिनती);

		urldb_write_paths(&parent->तथ्य->paths, host, fp,
				  &path, &path_alloc, &path_used, expiry);
	} अन्यथा अगर (hsts_expiry) {
		ख_लिखो(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		ख_लिखो(fp, "0\n");
	}

	मुक्त(path);

	urldb_save_खोज_वृक्ष(parent->दायाँ, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
अटल bool
urldb_iterate_entries_path(स्थिर काष्ठा path_तथ्य *parent,
		bool (*url_callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य),
		bool (*cookie_callback)(स्थिर काष्ठा cookie_तथ्य *तथ्य))
{
	स्थिर काष्ठा path_तथ्य *p = parent;
	स्थिर काष्ठा cookie_तथ्य *c;

	करो {
		अगर (p->children != शून्य) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			निश्चित(url_callback || cookie_callback);

			/** \todo handle fragments? */
			अगर (url_callback) {
				स्थिर काष्ठा url_internal_तथ्य *u = &p->urld;

				निश्चित(p->url);

				अगर (!url_callback(p->url,
						  (स्थिर काष्ठा url_तथ्य *) u))
					वापस false;
			} अन्यथा {
				c = (स्थिर काष्ठा cookie_तथ्य *)p->cookies;
				क्रम (; c != शून्य; c = c->अग्र) {
					अगर (!cookie_callback(c))
						वापस false;
				}
			}

			/* Now, find next node to process. */
			जबतक (p != parent) {
				अगर (p->अग्र != शून्य) {
					/* Have a sibling, process that */
					p = p->अग्र;
					अवरोध;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} जबतक (p != parent);

	वापस true;
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
अटल bool urldb__host_is_ip_address(स्थिर अक्षर *host)
{
	काष्ठा in_addr ipv4;
	माप_प्रकार host_len = म_माप(host);
	स्थिर अक्षर *sane_host;
	स्थिर अक्षर *slash;
#अगर_अघोषित NO_IPV6
	काष्ठा in6_addr ipv6;
	अक्षर ipv6_addr[64];
	अचिन्हित पूर्णांक ipv6_addr_len;
#पूर्ण_अगर
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
	अगर (slash == शून्य) {
		sane_host = host;
	} अन्यथा {
		अक्षर *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	अगर (म_अखोज(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		जाओ out_false;

	अगर (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		पूर्णांक अंक_dots = 0;
		माप_प्रकार index;

		क्रम (index = 0; index < host_len; index++) {
			अगर (sane_host[index] == '.')
				अंक_dots++;
		}

		अगर (अंक_dots == 3)
			जाओ out_true;
		अन्यथा
			जाओ out_false;
	}

#अगर_अघोषित NO_IPV6
	अगर ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		जाओ out_false;
	}

	ipv6_addr_len = host_len - 2;
	अगर (ipv6_addr_len >= माप(ipv6_addr)) {
		ipv6_addr_len = माप(ipv6_addr) - 1;
	}
	म_नकलन(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	अगर (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		जाओ out_true;
#पूर्ण_अगर

out_false:
	अगर (slash != शून्य) मुक्त((व्योम *)sane_host);
	वापस false;

out_true:
	अगर (slash != शून्य) मुक्त((व्योम *)sane_host);
	वापस true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
अटल पूर्णांक urldb_खोज_match_prefix(स्थिर काष्ठा host_part *a, स्थिर अक्षर *b)
{
	स्थिर अक्षर *end, *dot;
	पूर्णांक plen, ret;

	निश्चित(a && a != &db_मूल && b);

	अगर (urldb__host_is_ip_address(b)) {
		/* IP address */
		वापस strncasecmp(a->part, b, म_माप(b));
	}

	end = b + म_माप(b) + 1;

	जबतक (b < end && a && a != &db_मूल) {
		dot = म_अक्षर(b, '.');
		अगर (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		अगर ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			वापस ret;

		/* The strings matched */
		अगर (dot < end - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = म_माप(a->part);
			अगर (plen > dot - b) {
				/* len(a) > len(b) */
				वापस 1;
			} अन्यथा अगर (plen < dot - b) {
				/* len(a) < len(b) */
				वापस -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	अगर (a && a != &db_मूल && b >= end) {
		/* len(a) > len(b) => prefix matches */
		वापस 0;
	} अन्यथा अगर ((!a || a == &db_मूल) && b < end) {
		/* len(a) < len(b) => prefix does not match */
		वापस -1;
	}

	/* Identical */
	वापस 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
अटल bool
urldb_iterate_partial_host(काष्ठा खोज_गाँठ *मूल,
		स्थिर अक्षर *prefix,
		bool (*callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य))
{
	पूर्णांक c;

	निश्चित(मूल && prefix && callback);

	अगर (मूल == &empty)
		वापस true;

	c = urldb_खोज_match_prefix(मूल->तथ्य, prefix);

	अगर (c > 0) {
		/* No match => look in left subtree */
		वापस urldb_iterate_partial_host(मूल->बायाँ,
						  prefix,
						  callback);
	} अन्यथा अगर (c < 0) {
		/* No match => look in right subtree */
		वापस urldb_iterate_partial_host(मूल->दायाँ,
						  prefix,
						  callback);
	} अन्यथा {
		/* Match => iterate over l/r subtrees & process this node */
		अगर (!urldb_iterate_partial_host(मूल->बायाँ,
						prefix,
						callback)) {
			वापस false;
		}

		अगर (मूल->तथ्य->paths.children) {
			/* and extract all paths attached to this host */
			अगर (!urldb_iterate_entries_path(&मूल->तथ्य->paths,
							callback,
							शून्य)) {
				वापस false;
			}
		}

		अगर (!urldb_iterate_partial_host(मूल->दायाँ,
						prefix,
						callback)) {
			वापस false;
		}
	}

	वापस true;
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
अटल bool
urldb_iterate_partial_path(स्थिर काष्ठा path_तथ्य *parent,
		स्थिर अक्षर *prefix,
		bool (*callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य))
{
	स्थिर काष्ठा path_तथ्य *p = parent->children;
	स्थिर अक्षर *slash, *end = prefix + म_माप(prefix);

	करो {
		slash = म_अक्षर(prefix, '/');
		अगर (!slash) {
			slash = end;
		}

		अगर (slash == prefix && *prefix == '/') {
			/* Ignore "//" */
			prefix++;
			जारी;
		}

		अगर (strncasecmp(p->segment, prefix, slash - prefix) == 0) {
			/* prefix matches so far */
			अगर (slash == end) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				अगर (!urldb_iterate_entries_path(p,
								callback,
								शून्य)) {
					वापस false;
				}

				/* Progress to next sibling */
				p = p->अग्र;
			} अन्यथा {
				/* Skip over this segment */
				prefix = slash + 1;

				p = p->children;
			}
		} अन्यथा {
			/* Doesn't match this segment, try next sibling */
			p = p->अग्र;
		}
	} जबतक (p != शून्य);

	वापस true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
अटल bool
urldb_iterate_entries_host(काष्ठा खोज_गाँठ *parent,
		bool (*url_callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य),
		bool (*cookie_callback)(स्थिर काष्ठा cookie_तथ्य *तथ्य))
{
	अगर (parent == &empty) {
		वापस true;
	}

	अगर (!urldb_iterate_entries_host(parent->बायाँ,
					url_callback,
					cookie_callback)) {
		वापस false;
	}

	अगर ((parent->तथ्य->paths.children) ||
	    ((cookie_callback) &&
	     (parent->तथ्य->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		अगर (!urldb_iterate_entries_path(&parent->तथ्य->paths,
						url_callback,
						cookie_callback)) {
			वापस false;
		}
	}

	अगर (!urldb_iterate_entries_host(parent->दायाँ,
					url_callback,
					cookie_callback)) {
		वापस false;
	}

	वापस true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
अटल काष्ठा host_part *
urldb_add_host_गाँठ(स्थिर अक्षर *part, काष्ठा host_part *parent)
{
	काष्ठा host_part *d;

	निश्चित(part && parent);

	d = सुस्मृति(1, माप(काष्ठा host_part));
	अगर (!d) {
		वापस शून्य;
	}

	d->part = strdup(part);
	अगर (!d->part) {
		मुक्त(d);
		वापस शून्य;
	}

	d->अग्र = parent->children;
	अगर (parent->children) {
		parent->children->पूर्व = d;
	}
	d->parent = parent;
	parent->children = d;

	वापस d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
अटल पूर्णांक urldb_add_path_fragment_cmp(स्थिर व्योम *a, स्थिर व्योम *b)
{
	वापस strcasecmp(*((स्थिर अक्षर **) a), *((स्थिर अक्षर **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
अटल काष्ठा path_तथ्य *
urldb_add_path_fragment(काष्ठा path_तथ्य *segment, lwc_string *fragment)
{
	अक्षर **अस्थायी;

	निश्चित(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	अगर (!fragment)
		वापस segment;

	अस्थायी = पुनः_स्मृति(segment->fragment,
		       (segment->frag_cnt + 1) * माप(अक्षर *));
	अगर (!अस्थायी)
		वापस शून्य;

	segment->fragment = अस्थायी;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_तथ्य(fragment));
	अगर (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		वापस शून्य;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	क्विक(segment->fragment,
	      segment->frag_cnt,
	      माप (अक्षर *),
	      urldb_add_path_fragment_cmp);

	वापस segment;
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
अटल काष्ठा path_तथ्य *
urldb_add_path_गाँठ(lwc_string *scheme,
		    अचिन्हित पूर्णांक port,
		    स्थिर अक्षर *segment,
		    lwc_string *fragment,
		    काष्ठा path_तथ्य *parent)
{
	काष्ठा path_तथ्य *d, *e;

	निश्चित(scheme && segment && parent);

	d = सुस्मृति(1, माप(काष्ठा path_तथ्य));
	अगर (!d)
		वापस शून्य;

	d->scheme = lwc_string_ref(scheme);

	d->port = port;

	d->segment = strdup(segment);
	अगर (!d->segment) {
		lwc_string_unref(d->scheme);
		मुक्त(d);
		वापस शून्य;
	}

	अगर (fragment) {
		अगर (!urldb_add_path_fragment(d, fragment)) {
			मुक्त(d->segment);
			lwc_string_unref(d->scheme);
			मुक्त(d);
			वापस शून्य;
		}
	}

	क्रम (e = parent->children; e; e = e->अग्र) {
		अगर (म_भेद(e->segment, d->segment) > 0)
			अवरोध;
	}

	अगर (e) {
		d->पूर्व = e->पूर्व;
		d->अग्र = e;
		अगर (e->पूर्व)
			e->पूर्व->अग्र = d;
		अन्यथा
			parent->children = d;
		e->पूर्व = d;
	} अन्यथा अगर (!parent->children) {
		d->पूर्व = d->अग्र = शून्य;
		parent->children = parent->last = d;
	} अन्यथा {
		d->अग्र = शून्य;
		d->पूर्व = parent->last;
		parent->last->अग्र = d;
		parent->last = d;
	}
	d->parent = parent;

	वापस d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
अटल काष्ठा खोज_गाँठ **urldb_get_खोज_वृक्ष_direct(स्थिर अक्षर *host)
{
	निश्चित(host);

	अगर (urldb__host_is_ip_address(host)) {
		वापस &खोज_trees[ST_IP];
	} अन्यथा अगर (ascii_is_alpha(*host)) {
		वापस &खोज_trees[ST_DN + ascii_to_lower(*host) - 'a'];
	}
	वापस &खोज_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
अटल काष्ठा खोज_गाँठ *urldb_get_खोज_वृक्ष(स्थिर अक्षर *host)
{
	वापस *urldb_get_खोज_वृक्ष_direct(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
अटल पूर्णांक urldb_खोज_match_string(स्थिर काष्ठा host_part *a, स्थिर अक्षर *b)
{
	स्थिर अक्षर *end, *dot;
	पूर्णांक plen, ret;

	निश्चित(a && a != &db_मूल && b);

	अगर (urldb__host_is_ip_address(b)) {
		/* IP address */
		वापस strcasecmp(a->part, b);
	}

	end = b + म_माप(b) + 1;

	जबतक (b < end && a && a != &db_मूल) {
		dot = म_अक्षर(b, '.');
		अगर (!dot) {
			/* last segment */
			dot = end - 1;
		}

		/* Compare strings (length limited) */
		अगर ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			वापस ret;

		/* The strings matched, now check that the lengths do, too */
		plen = म_माप(a->part);

		अगर (plen > dot - b) {
			/* len(a) > len(b) */
			वापस 1;
		} अन्यथा अगर (plen < dot - b) {
			/* len(a) < len(b) */
			वापस -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	अगर (a && a != &db_मूल && b >= end) {
		/* len(a) > len(b) */
		वापस 1;
	} अन्यथा अगर ((!a || a == &db_मूल) && b < end) {
		/* len(a) < len(b) */
		वापस -1;
	}

	/* Identical */
	वापस 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
अटल स्थिर काष्ठा host_part *
urldb_खोज_find(काष्ठा खोज_गाँठ *मूल, स्थिर अक्षर *host)
{
	पूर्णांक c;

	निश्चित(मूल && host);

	अगर (मूल == &empty) {
		वापस शून्य;
	}

	c = urldb_खोज_match_string(मूल->तथ्य, host);

	अगर (c > 0) {
		वापस urldb_खोज_find(मूल->बायाँ, host);
	} अन्यथा अगर (c < 0) {
		वापस urldb_खोज_find(मूल->दायाँ, host);
	}

	वापस मूल->तथ्य;
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
अटल काष्ठा path_तथ्य *
urldb_match_path(स्थिर काष्ठा path_तथ्य *parent,
		 स्थिर अक्षर *path,
		 lwc_string *scheme,
		 अचिन्हित लघु port)
{
	स्थिर काष्ठा path_तथ्य *p;
	स्थिर अक्षर *slash;
	bool match;

	निश्चित(parent != शून्य);
	निश्चित(parent->segment == शून्य);

	अगर (path[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", path);
	}

	निश्चित(path[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	जबतक (p != शून्य) {
		slash = म_अक्षर(path + 1, '/');
		अगर (!slash) {
			slash = path + म_माप(path);
		}

		अगर (म_भेदन(p->segment, path + 1, slash - path - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &match) == lwc_error_ok &&
		    match == true &&
		    p->port == port) {
			अगर (*slash == '\0') {
				/* Complete match */
				वापस (काष्ठा path_तथ्य *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			path = slash;
		} अन्यथा {
			/* No match, try next sibling */
			p = p->अग्र;
		}
	}

	वापस शून्य;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
अटल काष्ठा path_तथ्य *urldb_find_url(nsurl *url)
{
	स्थिर काष्ठा host_part *h;
	काष्ठा path_तथ्य *p;
	काष्ठा खोज_गाँठ *वृक्ष;
	अक्षर *plq;
	स्थिर अक्षर *host_str;
	lwc_string *scheme, *host, *port;
	माप_प्रकार len = 0;
	अचिन्हित पूर्णांक port_int;
	bool match;

	निश्चित(url);

	अगर (url_bloom != शून्य) {
		अगर (bloom_खोज_hash(url_bloom, nsurl_hash(url)) == false) {
			वापस शून्य;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (scheme == शून्य)
		वापस शून्य;

	अगर (lwc_string_isequal(scheme, corestring_lwc_mailto, &match) ==
	    lwc_error_ok && match == true) {
		lwc_string_unref(scheme);
		वापस शून्य;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	अगर (host != शून्य) {
		host_str = lwc_string_तथ्य(host);
		lwc_string_unref(host);

	} अन्यथा अगर (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		वापस शून्य;
	}

	वृक्ष = urldb_get_खोज_वृक्ष(host_str);
	h = urldb_खोज_find(वृक्ष, host_str);
	अगर (!h) {
		lwc_string_unref(scheme);
		वापस शून्य;
	}

	/* generate plq (path, leaf, query) */
	अगर (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		वापस शून्य;
	}

	/* Get port */
	port = nsurl_get_component(url, NSURL_PORT);
	अगर (port != शून्य) {
		port_int = म_से_प(lwc_string_तथ्य(port));
		lwc_string_unref(port);
	} अन्यथा {
		port_int = 0;
	}

	p = urldb_match_path(&h->paths, plq, scheme, port_int);

	मुक्त(plq);
	lwc_string_unref(scheme);

	वापस p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
अटल व्योम urldb_dump_paths(काष्ठा path_तथ्य *parent)
{
	स्थिर काष्ठा path_तथ्य *p = parent;
	अचिन्हित पूर्णांक i;

	करो {
		अगर (p->segment != शून्य) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_तथ्य(p->scheme), p->port);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			क्रम (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		अगर (p->children != शून्य) {
			p = p->children;
		} अन्यथा {
			जबतक (p != parent) {
				अगर (p->अग्र != शून्य) {
					p = p->अग्र;
					अवरोध;
				}

				p = p->parent;
			}
		}
	} जबतक (p != parent);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
अटल व्योम urldb_dump_hosts(काष्ठा host_part *parent)
{
	काष्ठा host_part *h;

	अगर (parent->part) {
		NSLOG(netsurf, INFO, "%s", parent->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      parent->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&parent->paths);

	/* and recurse */
	क्रम (h = parent->children; h; h = h->अग्र) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
अटल व्योम urldb_dump_खोज(काष्ठा खोज_गाँठ *parent, पूर्णांक depth)
{
	स्थिर काष्ठा host_part *h;
	पूर्णांक i; /* index into string */
	अक्षर s[1024];
	पूर्णांक r;
	पूर्णांक sl = माप(s) - 2;

	अगर (parent == &empty)
		वापस;

	urldb_dump_खोज(parent->बायाँ, depth + 1);

	क्रम (i = 0; i != depth; i++) {
		s[i] = ' ';
	}

	क्रम (h = parent->तथ्य; h; h = h->parent) {
		अगर (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			अगर (r < 0) {
				अवरोध;
			}
			अगर ((i + r) >= sl) {
				अवरोध;
			}
			i += r;
		}

		अगर (h->parent && h->parent->parent) {
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
अटल पूर्णांक
urldb_खोज_match_host(स्थिर काष्ठा host_part *a, स्थिर काष्ठा host_part *b)
{
	पूर्णांक ret;

	निश्चित(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	क्रम (; a && a != &db_मूल && b && b != &db_मूल;
	     a = a->parent, b = b->parent) {
		अगर ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			वापस ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	अगर (a && a != &db_मूल && (!b || b == &db_मूल)) {
		/* len(a) > len(b) */
		वापस 1;
	} अन्यथा अगर ((!a || a == &db_मूल) && b && b != &db_मूल) {
		/* len(a) < len(b) */
		वापस -1;
	}

	/* identical */
	वापस 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
अटल काष्ठा खोज_गाँठ *urldb_खोज_skew(काष्ठा खोज_गाँठ *मूल)
{
	निश्चित(मूल);

	अगर (मूल->बायाँ->level == मूल->level) {
		काष्ठा खोज_गाँठ *अस्थायी;

		अस्थायी = मूल->बायाँ;
		मूल->बायाँ = अस्थायी->दायाँ;
		अस्थायी->दायाँ = मूल;
		मूल = अस्थायी;
	}

	वापस मूल;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
अटल काष्ठा खोज_गाँठ *urldb_खोज_split(काष्ठा खोज_गाँठ *मूल)
{
	निश्चित(मूल);

	अगर (मूल->दायाँ->दायाँ->level == मूल->level) {
		काष्ठा खोज_गाँठ *अस्थायी;

		अस्थायी = मूल->दायाँ;
		मूल->दायाँ = अस्थायी->बायाँ;
		अस्थायी->बायाँ = मूल;
		मूल = अस्थायी;

		मूल->level++;
	}

	वापस मूल;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
अटल काष्ठा खोज_गाँठ *
urldb_खोज_जोड़ो_internal(काष्ठा खोज_गाँठ *मूल, काष्ठा खोज_गाँठ *n)
{
	निश्चित(मूल && n);

	अगर (मूल == &empty) {
		मूल = n;
	} अन्यथा {
		पूर्णांक c = urldb_खोज_match_host(मूल->तथ्य, n->तथ्य);

		अगर (c > 0) {
			मूल->बायाँ = urldb_खोज_जोड़ो_internal(
				मूल->बायाँ, n);
		} अन्यथा अगर (c < 0) {
			मूल->दायाँ = urldb_खोज_जोड़ो_internal(
				मूल->दायाँ, n);
		} अन्यथा {
			/* exact match */
			मुक्त(n);
			वापस मूल;
		}

		मूल = urldb_खोज_skew(मूल);
		मूल = urldb_खोज_split(मूल);
	}

	वापस मूल;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
अटल काष्ठा खोज_गाँठ *
urldb_खोज_जोड़ो(काष्ठा खोज_गाँठ *मूल, स्थिर काष्ठा host_part *तथ्य)
{
	काष्ठा खोज_गाँठ *n;

	निश्चित(मूल && तथ्य);

	n = दो_स्मृति(माप(काष्ठा खोज_गाँठ));
	अगर (!n)
		वापस शून्य;

	n->level = 1;
	n->तथ्य = तथ्य;
	n->बायाँ = n->दायाँ = &empty;

	मूल = urldb_खोज_जोड़ो_internal(मूल, n);

	वापस मूल;
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
अटल bool
urldb_parse_avpair(काष्ठा cookie_internal_तथ्य *c,
		   अक्षर *n,
		   अक्षर *v,
		   bool was_quoted)
{
	पूर्णांक vlen;

	निश्चित(c && n && v);

	/* Strip whitespace from start of name */
	क्रम (; *n; n++) {
		अगर (*n != ' ' && *n != '\t')
			अवरोध;
	}

	/* Strip whitespace from end of name */
	क्रम (vlen = म_माप(n); vlen; vlen--) {
		अगर (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		अन्यथा
			अवरोध;
	}

	/* Strip whitespace from start of value */
	क्रम (; *v; v++) {
		अगर (*v != ' ' && *v != '\t')
			अवरोध;
	}

	/* Strip whitespace from end of value */
	क्रम (vlen = म_माप(v); vlen; vlen--) {
		अगर (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		अन्यथा
			अवरोध;
	}

	अगर (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		अगर (!c->comment)
			वापस false;
	} अन्यथा अगर (!c->domain && strcasecmp(n, "Domain") == 0) {
		अगर (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_set = true;
			c->domain = strdup(v);
			अगर (!c->domain)
				वापस false;
		}
	} अन्यथा अगर (strcasecmp(n, "Max-Age") == 0) {
		पूर्णांक अस्थायी = म_से_प(v);
		अगर (अस्थायी == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		अन्यथा
			c->expires = समय(शून्य) + अस्थायी;
	} अन्यथा अगर (!c->path && strcasecmp(n, "Path") == 0) {
		c->path_from_set = true;
		c->path = strdup(v);
		अगर (!c->path)
			वापस false;
	} अन्यथा अगर (strcasecmp(n, "Version") == 0) {
		c->version = म_से_प(v);
	} अन्यथा अगर (strcasecmp(n, "Expires") == 0) {
		अक्षर *datenoday;
		समय_प्रकार expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		क्रम (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, म_माप(datenoday), &expires);
		अगर (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (समय_प्रकार)0x7fffffff;
		}
		c->expires = expires;
	} अन्यथा अगर (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} अन्यथा अगर (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} अन्यथा अगर (!c->name) {
		c->name = strdup(n);
		c->मूल्य = strdup(v);
		c->मूल्य_was_quoted = was_quoted;
		अगर (!c->name || !c->मूल्य) {
			वापस false;
		}
	}

	वापस true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
अटल व्योम urldb_free_cookie(काष्ठा cookie_internal_तथ्य *c)
{
	निश्चित(c);

	मुक्त(c->comment);
	मुक्त(c->domain);
	मुक्त(c->path);
	मुक्त(c->name);
	मुक्त(c->मूल्य);
	मुक्त(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
अटल काष्ठा cookie_internal_तथ्य *
urldb_parse_cookie(nsurl *url, स्थिर अक्षर **cookie)
{
	काष्ठा cookie_internal_तथ्य *c;
	स्थिर अक्षर *cur;
	अक्षर name[1024], मूल्य[4096];
	अक्षर *n = name, *v = मूल्य;
	bool in_मूल्य = false;
	bool had_मूल्य_तथ्य = false;
	bool मूल्य_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	निश्चित(url && cookie && *cookie);

	c = सुस्मृति(1, माप(काष्ठा cookie_internal_तथ्य));
	अगर (c == शून्य)
		वापस शून्य;

	c->expires = -1;

	name[0] = '\0';
	मूल्य[0] = '\0';

	क्रम (cur = *cookie; *cur; cur++) {
		अगर (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			अगर (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				मूल्य[0] = '\0';

			}

			अवरोध;
		} अन्यथा अगर (*cur == '\r') {
			/* Spurious linefeed */
			जारी;
		} अन्यथा अगर (*cur == '\n') {
			/* Spurious newline */
			जारी;
		}

		अगर (in_मूल्य && !had_मूल्य_तथ्य) {
			अगर (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				जारी;
			} अन्यथा {
				had_मूल्य_तथ्य = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				अगर (*cur != '"') {
					मूल्य_verbatim = true;
				}
			}
		}

		अगर (in_मूल्य && !मूल्य_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			अगर (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				जारी;
			}
		}

		अगर (!quoted && !in_मूल्य && *cur == '=') {
			/* First equals => attr-value separator */
			in_मूल्य = true;
			जारी;
		}

		अगर (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			अगर (!urldb_parse_avpair(c, name, मूल्य, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				वापस शून्य;
			}

			/* And reset to start */
			n = name;
			v = मूल्य;
			in_मूल्य = false;
			had_मूल्य_तथ्य = false;
			मूल्य_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			अगर (*cur != ';') {
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
		अगर (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			स्थिर अक्षर *p;
			स्थिर अक्षर *semi = म_अक्षर(cur + 1, ';');
			अगर (!semi)
				semi = cur + म_माप(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			क्रम (p = cur + 1; p < semi; p++)
				अगर (*p == '=')
					अवरोध;

			अगर (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} अन्यथा {
				/* found one => comma marks end of cookie */
				cur++;
				अवरोध;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		अगर (!in_मूल्य) {
			अगर (n < name + (माप(name) - 1))
				*n++ = *cur;
		} अन्यथा {
			अगर (v < मूल्य + (माप(मूल्य) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	अगर (!urldb_parse_avpair(c, name, मूल्य, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		वापस शून्य;
	}

	/* Now fix-up default values */
	अगर (c->domain == शून्य) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		अगर (host == शून्य) {
			urldb_free_cookie(c);
			वापस शून्य;
		}
		c->domain = strdup(lwc_string_तथ्य(host));
		lwc_string_unref(host);
	}

	अगर (c->path == शून्य) {
		स्थिर अक्षर *path_तथ्य;
		अक्षर *path, *slash;
		lwc_string *path_lwc;

		path_lwc = nsurl_get_component(url, NSURL_PATH);
		अगर (path_lwc == शून्य) {
			urldb_free_cookie(c);
			वापस शून्य;
		}
		path_तथ्य = lwc_string_तथ्य(path_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = म_खोजप(path_तथ्य, '/');
		अगर (slash != शून्य) {
			/* Special case: retain first slash in path */
			अगर (slash == path_तथ्य)
				slash++;

			slash = strndup(path_तथ्य, slash - path_तथ्य);
			अगर (slash == शून्य) {
				lwc_string_unref(path_lwc);
				urldb_free_cookie(c);
				वापस शून्य;
			}

			path = slash;
			lwc_string_unref(path_lwc);
		} अन्यथा {
			path = strdup(lwc_string_तथ्य(path_lwc));
			lwc_string_unref(path_lwc);
			अगर (path == शून्य) {
				urldb_free_cookie(c);
				वापस शून्य;
			}
		}

		c->path = path;
	}

	/* Write back current position */
	*cookie = cur;

	वापस c;
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
अटल काष्ठा path_तथ्य *
urldb_add_path(lwc_string *scheme,
	       अचिन्हित पूर्णांक port,
	       स्थिर काष्ठा host_part *host,
	       अक्षर *path_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	काष्ठा path_तथ्य *d, *e;
	अक्षर *buf = path_query;
	अक्षर *segment, *slash;
	bool match;

	निश्चित(scheme && host && url);

	d = (काष्ठा path_तथ्य *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	अगर (*segment == '/')
		segment++;

	/* Process path segments */
	करो {
		slash = म_अक्षर(segment, '/');
		अगर (!slash) {
			/* last segment */
			/* look for existing entry */
			क्रम (e = d->children; e; e = e->अग्र)
				अगर (म_भेद(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &match) ==
				    lwc_error_ok &&
				    match == true &&
				    e->port == port)
					अवरोध;

			d = e ? urldb_add_path_fragment(e, fragment) :
				urldb_add_path_गाँठ(scheme, port,
						    segment, fragment, d);
			अवरोध;
		}

		*slash = '\0';

		/* look for existing entry */
		क्रम (e = d->children; e; e = e->अग्र)
			अगर (म_भेद(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &match) == lwc_error_ok &&
			    match == true &&
			    e->port == port)
				अवरोध;

		d = e ? e : urldb_add_path_गाँठ(scheme, port, segment, शून्य, d);
		अगर (!d)
			अवरोध;

		segment = slash + 1;
	} जबतक (1);

	मुक्त(path_query);

	अगर (d && !d->url) {
		/* Insert defragmented URL */
		अगर (nsurl_defragment(url, &d->url) != NSERROR_OK)
			वापस शून्य;
	}

	वापस d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
अटल काष्ठा host_part *urldb_add_host(स्थिर अक्षर *host)
{
	काष्ठा host_part *d = (काष्ठा host_part *) &db_मूल, *e;
	काष्ठा खोज_गाँठ *s;
	अक्षर buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	अक्षर *part;

	निश्चित(host);

	अगर (urldb__host_is_ip_address(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		क्रम (e = d->children; e; e = e->अग्र)
			अगर (strcasecmp(host, e->part) == 0)
				/* found => return it */
				वापस e;

		d = urldb_add_host_गाँठ(host, d);

		s = urldb_खोज_जोड़ो(खोज_trees[ST_IP], d);
		अगर (!s) {
			/* failed */
			d = शून्य;
		} अन्यथा {
			खोज_trees[ST_IP] = s;
		}

		वापस d;
	}

	/* Copy host string, so we can corrupt it */
	म_नकलन(buf, host, माप buf);
	buf[माप buf - 1] = '\0';

	/* Process FQDN segments backwards */
	करो {
		part = म_खोजप(buf, '.');
		अगर (!part) {
			/* last segment */
			/* Check for existing entry */
			क्रम (e = d->children; e; e = e->अग्र)
				अगर (strcasecmp(buf, e->part) == 0)
					अवरोध;

			अगर (e) {
				d = e;
			} अन्यथा {
				d = urldb_add_host_गाँठ(buf, d);
			}

			/* And insert into search tree */
			अगर (d) {
				काष्ठा खोज_गाँठ **r;

				r = urldb_get_खोज_वृक्ष_direct(buf);
				s = urldb_खोज_जोड़ो(*r, d);
				अगर (!s) {
					/* failed */
					d = शून्य;
				} अन्यथा {
					*r = s;
				}
			}
			अवरोध;
		}

		/* Check for existing entry */
		क्रम (e = d->children; e; e = e->अग्र)
			अगर (strcasecmp(part + 1, e->part) == 0)
				अवरोध;

		d = e ? e : urldb_add_host_गाँठ(part + 1, d);
		अगर (!d)
			अवरोध;

		*part = '\0';
	} जबतक (1);

	वापस d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
अटल bool
urldb_जोड़ो_cookie(काष्ठा cookie_internal_तथ्य *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	काष्ठा cookie_internal_तथ्य *d;
	स्थिर काष्ठा host_part *h;
	काष्ठा path_तथ्य *p;
	समय_प्रकार now = समय(शून्य);

	निश्चित(c);

	अगर (c->domain[0] == '.') {
		h = urldb_खोज_find(
			urldb_get_खोज_वृक्ष(&(c->domain[1])),
			c->domain + 1);
		अगर (!h) {
			h = urldb_add_host(c->domain + 1);
			अगर (!h) {
				urldb_free_cookie(c);
				वापस false;
			}
		}

		p = (काष्ठा path_तथ्य *) &h->paths;
	} अन्यथा {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		निश्चित(url != शून्य);
		निश्चित(scheme != शून्य);

		h = urldb_खोज_find(
			urldb_get_खोज_वृक्ष(c->domain),
			c->domain);

		अगर (!h) {
			h = urldb_add_host(c->domain);
			अगर (!h) {
				urldb_free_cookie(c);
				वापस false;
			}
		}

		/* find path */
		p = urldb_add_path(scheme, 0, h,
				   strdup(c->path), शून्य, url);
		अगर (!p) {
			urldb_free_cookie(c);
			वापस false;
		}
	}

	/* add cookie */
	क्रम (d = p->cookies; d; d = d->अग्र) {
		अगर (!म_भेद(d->domain, c->domain) &&
		    !म_भेद(d->path, c->path) &&
		    !म_भेद(d->name, c->name))
			अवरोध;
	}

	अगर (d) {
		अगर (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			अगर (d->अग्र)
				d->अग्र->पूर्व = d->पूर्व;
			अन्यथा
				p->cookies_end = d->पूर्व;
			अगर (d->पूर्व)
				d->पूर्व->अग्र = d->अग्र;
			अन्यथा
				p->cookies = d->अग्र;

			cookie_manager_remove((काष्ठा cookie_तथ्य *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} अन्यथा {
			/* replace d with c */
			c->पूर्व = d->पूर्व;
			c->अग्र = d->अग्र;
			अगर (c->अग्र)
				c->अग्र->पूर्व = c;
			अन्यथा
				p->cookies_end = c;
			अगर (c->पूर्व)
				c->पूर्व->अग्र = c;
			अन्यथा
				p->cookies = c;

			cookie_manager_remove((काष्ठा cookie_तथ्य *)d);
			urldb_free_cookie(d);

			cookie_manager_add((काष्ठा cookie_तथ्य *)c);
		}
	} अन्यथा {
		c->पूर्व = p->cookies_end;
		c->अग्र = शून्य;
		अगर (p->cookies_end)
			p->cookies_end->अग्र = c;
		अन्यथा
			p->cookies = c;
		p->cookies_end = c;

		cookie_manager_add((काष्ठा cookie_तथ्य *)c);
	}

	वापस true;
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
अटल bool
concat_netscape_cookie(काष्ठा cookie_internal_तथ्य *c,
		    पूर्णांक *used,
		    पूर्णांक *alloc,
		    अक्षर **buf)
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
	पूर्णांक max_len;
	पूर्णांक outl; /* output length */

	निश्चित(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		म_माप(c->name) + 1 +
		म_माप(c->मूल्य) + 2 +
		(c->path_from_set ? 8 + म_माप(c->path) + 2 : 0) +
		(c->domain_from_set ? 10 + म_माप(c->domain) + 2 : 0);
	अगर (max_len >= *alloc) {
		अक्षर *अस्थायी = पुनः_स्मृति(*buf, max_len);
		अगर (!अस्थायी) {
			वापस false;
		}
		*buf = अस्थायी;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *used - 1, *alloc - *used, "; %s=", c->name);
	अगर (outl >= (*alloc - *used)) {
		/* output truncated */
		वापस false;
	}
	*used += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	अगर (c->मूल्य_was_quoted) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->मूल्य);
		अगर (outl >= (*alloc - *used)) {
			/* output truncated */
			वापस false;
		}
		*used += outl;
	} अन्यथा {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *used - 1,*alloc - *used, "%s", c->मूल्य);
		अगर (outl >= (*alloc - *used)) {
			/* output truncated */
			वापस false;
		}
		*used += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	वापस true;
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
अटल bool
concat_rfc2965_cookie(काष्ठा cookie_internal_तथ्य *c,
		      पूर्णांक *used,
		      पूर्णांक *alloc,
		      अक्षर **buf)
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
	स्थिर अक्षर * स्थिर separators = "()<>@,;:\\\"/[]?={} \t";
	पूर्णांक max_len;
	पूर्णांक outl; /* output length */

	निश्चित(c && used && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *used + 2 +
		म_माप(c->name) + 1 +
		म_माप(c->मूल्य) + 2 +
		(c->path_from_set ?
		 8 + म_माप(c->path) + 2 : 0) +
		(c->domain_from_set ?
		 10 + म_माप(c->domain) + 2 : 0);

	अगर (max_len >= *alloc) {
		अक्षर *अस्थायी = पुनः_स्मृति(*buf, max_len);
		अगर (!अस्थायी) {
			वापस false;
		}
		*buf = अस्थायी;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *used - 1,
			*alloc - *used,
			"; %s=", c->name);
	अगर (outl >= (*alloc - *used)) {
		/* output truncated */
		वापस false;
	}
	*used += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	अगर (c->मूल्य_was_quoted ||
	    strpbrk(c->मूल्य, separators) != शून्य) {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"\"%s\"", c->मूल्य);

	} अन्यथा {
		outl = snprintf(*buf + *used - 1,
				*alloc - *used,
				"%s", c->मूल्य);
	}
	अगर (outl >= (*alloc - *used)) {
		/* output truncated */
		वापस false;
	}
	*used += outl;

	अगर (c->path_from_set) {
		/* Path, quoted if necessary */

		अगर (strpbrk(c->path, separators) != शून्य) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Path=\"%s\"", c->path);

		} अन्यथा {
			outl = snprintf(*buf + *used - 1,
				       *alloc - *used,
				       "; $Path=%s", c->path);
		}
		अगर (outl >= (*alloc - *used)) {
			/* output truncated */
			वापस false;
		}
		*used += outl;
	}

	अगर (c->domain_from_set) {
		/* Domain, quoted if necessary */
		अगर (strpbrk(c->domain, separators) != शून्य) {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=\"%s\"", c->domain);
		} अन्यथा {
			outl = snprintf(*buf + *used - 1,
					*alloc - *used,
					"; $Domain=%s", c->domain);
		}
		अगर (outl >= (*alloc - *used)) {
			/* output truncated */
			वापस false;
		}
		*used += outl;
	}

	वापस true;
}


/**
 * build output cookie string in netscape format
 */
अटल अक्षर *
get_netscape_cookie(काष्ठा cookie_internal_तथ्य **matched_cookies, पूर्णांक गिनती)
{
	पूर्णांक ret_alloc = 4096, ret_used = 1;
	अक्षर *ret;
	पूर्णांक midx; /* index into matched cookies */

	ret = दो_स्मृति(ret_alloc);
	अगर (ret == शून्य) {
		वापस शून्य;
	}
	ret[0] = '\0';

	क्रम (midx = 0; midx < गिनती; midx++) {
		अगर (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_used, &ret_alloc, &ret)) {
			मुक्त(ret);
			वापस शून्य;
		}
	}

	अगर (ret_used <= 2) {
		मुक्त(ret);
		वापस शून्य;
	}

	/* Old-style cookies => no version & skip "; " */
	स_हटाओ(ret, ret + 2, ret_used - 2);
	ret_used -= 2;

	/* shrink the output buffer to the required size */
	अक्षर *अस्थायी = पुनः_स्मृति(ret, ret_used);
	अगर (अस्थायी != शून्य) {
		ret = अस्थायी;
	}

	वापस ret;
}


/**
 * build output cookie string in RFC2965 format
 */
अटल अक्षर *
get_rfc2965_cookie(काष्ठा cookie_internal_तथ्य **matched_cookies,
		   पूर्णांक गिनती,
		   पूर्णांक version)
{
	पूर्णांक ret_alloc = 4096; /* return string allocation */
	पूर्णांक ret_used = 1; /* return sting buffer used */
	अक्षर *ret; /* return string buffer */
	पूर्णांक midx; /* index into matched cookies */
	पूर्णांक outl;

	ret = दो_स्मृति(ret_alloc);
	अगर (ret == शून्य) {
		वापस शून्य;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_used, "$Version=%d", version);
	अगर (outl >= ret_alloc - ret_used) {
		/* overflow */
		मुक्त(ret);
		वापस शून्य;
	}
	ret_used += outl;

	क्रम (midx = 0; midx < गिनती; midx++) {
		अगर (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_used, &ret_alloc, &ret)) {
			मुक्त(ret);
			वापस शून्य;
		}
	}

	/* shrink the output buffer to the required size */
	अक्षर *अस्थायी = पुनः_स्मृति(ret, ret_used);
	अगर (अस्थायी != शून्य) {
		ret = अस्थायी;
	}

	वापस ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
अटल व्योम
urldb_delete_cookie_paths(स्थिर अक्षर *domain,
			  स्थिर अक्षर *path,
			  स्थिर अक्षर *name,
			  काष्ठा path_तथ्य *parent)
{
	काष्ठा cookie_internal_तथ्य *c;
	काष्ठा path_तथ्य *p = parent;

	निश्चित(parent);

	करो {
		क्रम (c = p->cookies; c; c = c->अग्र) {
			अगर (म_भेद(c->domain, domain) == 0 &&
			    म_भेद(c->path, path) == 0 &&
			    म_भेद(c->name, name) == 0) {
				अगर (c->पूर्व) {
					c->पूर्व->अग्र = c->अग्र;
				} अन्यथा {
					p->cookies = c->अग्र;
				}

				अगर (c->अग्र) {
					c->अग्र->पूर्व = c->पूर्व;
				} अन्यथा {
					p->cookies_end = c->पूर्व;
				}

				urldb_free_cookie(c);

				वापस;
			}
		}

		अगर (p->children) {
			p = p->children;
		} अन्यथा {
			जबतक (p != parent) {
				अगर (p->अग्र != शून्य) {
					p = p->अग्र;
					अवरोध;
				}

				p = p->parent;
			}
		}
	} जबतक (p != parent);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
अटल व्योम
urldb_delete_cookie_hosts(स्थिर अक्षर *domain,
			  स्थिर अक्षर *path,
			  स्थिर अक्षर *name,
			  काष्ठा host_part *parent)
{
	काष्ठा host_part *h;
	निश्चित(parent);

	urldb_delete_cookie_paths(domain, path, name, &parent->paths);

	क्रम (h = parent->children; h; h = h->अग्र) {
		urldb_delete_cookie_hosts(domain, path, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
अटल व्योम urldb_save_cookie_paths(खाता *fp, काष्ठा path_तथ्य *parent)
{
	काष्ठा path_तथ्य *p = parent;
	समय_प्रकार now = समय(शून्य);

	निश्चित(fp && parent);

	करो {
		अगर (p->cookies != शून्य) {
			काष्ठा cookie_internal_तथ्य *c;

			क्रम (c = p->cookies; c != शून्य; c = c->अग्र) {
				अगर (c->expires == -1 || c->expires < now) {
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
					(पूर्णांक)c->expires, (पूर्णांक)c->last_used,
					c->no_destroy, c->name, c->मूल्य,
					c->मूल्य_was_quoted,
					p->scheme ? lwc_string_तथ्य(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		अगर (p->children != शून्य) {
			p = p->children;
		} अन्यथा {
			जबतक (p != parent) {
				अगर (p->अग्र != शून्य) {
					p = p->अग्र;
					अवरोध;
				}

				p = p->parent;
			}
		}
	} जबतक (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
अटल व्योम urldb_save_cookie_hosts(खाता *fp, काष्ठा host_part *parent)
{
	काष्ठा host_part *h;
	निश्चित(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	क्रम (h = parent->children; h; h = h->अग्र)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
अटल व्योम urldb_destroy_cookie(काष्ठा cookie_internal_तथ्य *c)
{
	मुक्त(c->name);
	मुक्त(c->मूल्य);
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
अटल व्योम urldb_destroy_path_गाँठ_content(काष्ठा path_तथ्य *गाँठ)
{
	काष्ठा cookie_internal_तथ्य *a, *b;
	अचिन्हित पूर्णांक i;

	अगर (गाँठ->url != शून्य) {
		nsurl_unref(गाँठ->url);
	}

	lwc_string_unref(गाँठ->scheme);

	मुक्त(गाँठ->segment);
	क्रम (i = 0; i < गाँठ->frag_cnt; i++)
		मुक्त(गाँठ->fragment[i]);
	मुक्त(गाँठ->fragment);

	मुक्त(गाँठ->urld.title);

	क्रम (a = गाँठ->cookies; a; a = b) {
		b = a->अग्र;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
अटल व्योम urldb_destroy_prot_space(काष्ठा prot_space_तथ्य *space)
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
अटल व्योम urldb_destroy_path_वृक्ष(काष्ठा path_तथ्य *मूल)
{
	काष्ठा path_तथ्य *p = मूल;

	करो {
		अगर (p->children != शून्य) {
			p = p->children;
		} अन्यथा {
			काष्ठा path_तथ्य *q = p;

			जबतक (p != मूल) {
				अगर (p->अग्र != शून्य) {
					p = p->अग्र;
					अवरोध;
				}

				p = p->parent;

				urldb_destroy_path_गाँठ_content(q);
				मुक्त(q);

				q = p;
			}

			urldb_destroy_path_गाँठ_content(q);
			मुक्त(q);
		}
	} जबतक (p != मूल);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
अटल व्योम urldb_destroy_host_वृक्ष(काष्ठा host_part *मूल)
{
	काष्ठा host_part *a, *b;
	काष्ठा path_तथ्य *p, *q;
	काष्ठा prot_space_तथ्य *s, *t;

	/* Destroy children */
	क्रम (a = मूल->children; a; a = b) {
		b = a->अग्र;
		urldb_destroy_host_वृक्ष(a);
	}

	/* Now clean up paths */
	क्रम (p = मूल->paths.children; p; p = q) {
		q = p->अग्र;
		urldb_destroy_path_वृक्ष(p);
	}

	/* Root path */
	urldb_destroy_path_गाँठ_content(&मूल->paths);

	/* Proctection space data */
	क्रम (s = मूल->prot_space; s; s = t) {
		t = s->अग्र;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	मुक्त(मूल->part);
	मुक्त(मूल);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
अटल व्योम urldb_destroy_खोज_वृक्ष(काष्ठा खोज_गाँठ *मूल)
{
	/* Destroy children */
	अगर (मूल->बायाँ != &empty)
		urldb_destroy_खोज_वृक्ष(मूल->बायाँ);
	अगर (मूल->दायाँ != &empty)
		urldb_destroy_खोज_वृक्ष(मूल->दायाँ);

	/* And destroy ourselves */
	मुक्त(मूल);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
व्योम urldb_destroy(व्योम)
{
	काष्ठा host_part *a, *b;
	पूर्णांक i;

	/* Clean up search trees */
	क्रम (i = 0; i < अंक_खोज_TREES; i++) {
		अगर (खोज_trees[i] != &empty) {
			urldb_destroy_खोज_वृक्ष(खोज_trees[i]);
			खोज_trees[i] = &empty;
		}
	}

	/* And database */
	क्रम (a = db_मूल.children; a; a = b) {
		b = a->अग्र;
		urldb_destroy_host_वृक्ष(a);
	}
	स_रखो(&db_मूल, 0, माप(db_मूल));

	/* And the bloom filter */
	अगर (url_bloom != शून्य) {
		bloom_destroy(url_bloom);
		url_bloom = शून्य;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_load(स्थिर अक्षर *filename)
{
#घोषणा MAXIMUM_URL_लंबाई 4096
	अक्षर s[MAXIMUM_URL_लंबाई];
	अक्षर host[256];
	काष्ठा host_part *h;
	पूर्णांक urls;
	पूर्णांक i;
	पूर्णांक version;
	पूर्णांक लंबाई;
	खाता *fp;

	निश्चित(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	अगर (url_bloom == शून्य)
		url_bloom = bloom_create(BLOOM_आकार);

	fp = ख_खोलो(filename, "r");
	अगर (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		वापस NSERROR_NOT_FOUND;
	}

	अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp)) {
		ख_बंद(fp);
		वापस NSERROR_NEED_तथ्य;
	}

	version = म_से_प(s);
	अगर (version < MIN_URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		ख_बंद(fp);
		वापस NSERROR_INVALID;
	}
	अगर (version > URL_FILE_VERSION) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		ख_बंद(fp);
		वापस NSERROR_INVALID;
	}

	जबतक (ख_माला_लो(host, माप host, fp)) {
		समय_प्रकार hsts_expiry = 0;
		पूर्णांक hsts_include_sub_domains = 0;

		/* get the hostname */
		लंबाई = म_माप(host) - 1;
		host[लंबाई] = '\0';

		/* skip data that has ended up with a host of '' */
		अगर (लंबाई == 0) {
			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;
			urls = म_से_प(s);
			/* Eight fields/url */
			क्रम (i = 0; i < (8 * urls); i++) {
				अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
					अवरोध;
			}
			जारी;
		}

		अगर (version >= 107) {
			अक्षर *p = host;
			जबतक (*p && *p != ' ') p++;
			जबतक (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			जबतक (*p && *p != ' ') p++;
			जबतक (*p && *p == ' ') p++;
			nsc_snptimet(p, म_माप(p), &hsts_expiry);
		}

		h = urldb_add_host(host);
		अगर (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			ख_बंद(fp);
			वापस NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
			अवरोध;
		urls = म_से_प(s);

		/* no URLs => try next host */
		अगर (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			जारी;
		}

		/* load the non-corrupt data */
		क्रम (i = 0; i < urls; i++) {
			काष्ठा path_तथ्य *p = शून्य;
			अक्षर scheme[64], ports[10];
			अक्षर url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			अचिन्हित पूर्णांक port;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			अक्षर *path_query;
			माप_प्रकार len;

			अगर (!ख_माला_लो(scheme, माप scheme, fp))
				अवरोध;
			लंबाई = म_माप(scheme) - 1;
			scheme[लंबाई] = '\0';

			अगर (!ख_माला_लो(ports, माप ports, fp))
				अवरोध;
			लंबाई = म_माप(ports) - 1;
			ports[लंबाई] = '\0';
			port = म_से_प(ports);

			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;
			लंबाई = म_माप(s) - 1;
			s[लंबाई] = '\0';

			अगर (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, माप url, "%s://%s%s%s%s",
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
			अगर (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				वापस NSERROR_NOMEM;
			}

			अगर (url_bloom != शून्य) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_जोड़ो_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			अगर (nsurl_get(nsurl, NSURL_PATH | NSURL_QUERY,
				      &path_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				वापस NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_add_path(scheme_lwc, port, h, path_query,
					   fragment_lwc, nsurl);
			अगर (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				ख_बंद(fp);
				वापस NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;
			अगर (p)
				p->urld.visits = (अचिन्हित पूर्णांक)म_से_प(s);

			/* entry last use time */
			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp)) {
				अवरोध;
			}
			अगर (p) {
				nsc_snptimet(s, म_माप(s) - 1, &p->urld.last_visit);
			}

			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;
			अगर (p)
				p->urld.type = (content_type)म_से_प(s);

			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;


			अगर (!ख_माला_लो(s, MAXIMUM_URL_लंबाई, fp))
				अवरोध;
			लंबाई = म_माप(s) - 1;
			अगर (p && लंबाई > 0) {
				s[लंबाई] = '\0';
				p->urld.title = दो_स्मृति(लंबाई + 1);
				अगर (p->urld.title)
					स_नकल(p->urld.title, s, लंबाई + 1);
			}
		}
	}

	ख_बंद(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#अघोषित MAXIMUM_URL_लंबाई

	वापस NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(स्थिर अक्षर *filename)
{
	खाता *fp;
	पूर्णांक i;

	निश्चित(filename);

	fp = ख_खोलो(filename, "w");
	अगर (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		वापस NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	ख_लिखो(fp, "%d\n", URL_FILE_VERSION);

	क्रम (i = 0; i != अंक_खोज_TREES; i++) {
		urldb_save_खोज_वृक्ष(खोज_trees[i], fp);
	}

	ख_बंद(fp);

	वापस NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_persistence(nsurl *url, bool persist)
{
	काष्ठा path_तथ्य *p;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p) {
		वापस NSERROR_NOT_FOUND;
	}

	p->persistent = persist;

	वापस NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_add_url(nsurl *url)
{
	काष्ठा host_part *h;
	काष्ठा path_तथ्य *p;
	lwc_string *scheme;
	lwc_string *port;
	lwc_string *host;
	lwc_string *fragment;
	स्थिर अक्षर *host_str;
	अक्षर *path_query = शून्य;
	माप_प्रकार len;
	bool match;
	अचिन्हित पूर्णांक port_int;

	निश्चित(url);

	अगर (url_bloom == शून्य)
		url_bloom = bloom_create(BLOOM_आकार);

	अगर (url_bloom != शून्य) {
		uint32_t hash = nsurl_hash(url);
		bloom_जोड़ो_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	अगर (nsurl_get(url, NSURL_PATH | NSURL_QUERY, &path_query, &len) !=
	    NSERROR_OK) {
		वापस false;
	}
	निश्चित(path_query != शून्य);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (scheme == शून्य) {
		मुक्त(path_query);
		वापस false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	अगर (host != शून्य) {
		host_str = lwc_string_तथ्य(host);
		lwc_string_unref(host);

	} अन्यथा अगर (lwc_string_isequal(scheme, corestring_lwc_file, &match) ==
		   lwc_error_ok && match == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		मुक्त(path_query);
		वापस false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	port = nsurl_get_component(url, NSURL_PORT);
	अगर (port != शून्य) {
		port_int = म_से_प(lwc_string_तथ्य(port));
		lwc_string_unref(port);
	} अन्यथा {
		port_int = 0;
	}

	/* Get host entry */
	h = urldb_add_host(host_str);

	/* Get path entry */
	अगर (h != शून्य) {
		p = urldb_add_path(scheme,
				   port_int,
				   h,
				   path_query,
				   fragment,
				   url);
	} अन्यथा {
		p = शून्य;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	वापस (p != शून्य);
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_title(nsurl *url, स्थिर अक्षर *title)
{
	काष्ठा path_तथ्य *p;
	अक्षर *अस्थायी;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (p == शून्य) {
		वापस NSERROR_NOT_FOUND;
	}

	/* copy the parameter if necessary */
	अगर (title != शून्य) {
		अस्थायी = strdup(title);
		अगर (अस्थायी == शून्य) {
			वापस NSERROR_NOMEM;
		}
	} अन्यथा {
		अस्थायी = शून्य;
	}

	मुक्त(p->urld.title);
	p->urld.title = अस्थायी;

	वापस NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_set_url_content_type(nsurl *url, content_type type)
{
	काष्ठा path_तथ्य *p;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p) {
		वापस NSERROR_NOT_FOUND;
	}

	p->urld.type = type;

	वापस NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_visit_तथ्य(nsurl *url)
{
	काष्ठा path_तथ्य *p;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p) {
		वापस NSERROR_NOT_FOUND;
	}

	p->urld.last_visit = समय(शून्य);
	p->urld.visits++;

	वापस NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
व्योम urldb_reset_url_visit_तथ्य(nsurl *url)
{
	काष्ठा path_तथ्य *p;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस;

	p->urld.last_visit = (समय_प्रकार)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
स्थिर काष्ठा url_तथ्य *urldb_get_url_तथ्य(nsurl *url)
{
	काष्ठा path_तथ्य *p;
	काष्ठा url_internal_तथ्य *u;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस शून्य;

	u = &p->urld;

	वापस (स्थिर काष्ठा url_तथ्य *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	काष्ठा path_तथ्य *p;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस शून्य;

	वापस p->url;
}


/* exported interface documented in netsurf/url_db.h */
व्योम urldb_set_auth_details(nsurl *url, स्थिर अक्षर *realm, स्थिर अक्षर *auth)
{
	काष्ठा path_तथ्य *p, *pi;
	काष्ठा host_part *h;
	काष्ठा prot_space_तथ्य *space, *space_alloc;
	अक्षर *realm_alloc, *auth_alloc;
	bool match;

	निश्चित(url && realm && auth);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);

	अगर (!p)
		वापस;

	/* Search for host_part */
	क्रम (pi = p; pi->parent != शून्य; pi = pi->parent)
		;
	h = (काष्ठा host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	क्रम (space = h->prot_space; space; space = space->अग्र) {
		अगर (!म_भेद(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &match) == lwc_error_ok &&
		    match == true &&
		    space->port == p->port)
			अवरोध;
	}

	अगर (space != शून्य) {
		/* Overrule existing auth. */
		मुक्त(space->auth);
		space->auth = strdup(auth);
	} अन्यथा {
		/* Create a new protection space. */
		space = space_alloc = दो_स्मृति(माप(काष्ठा prot_space_तथ्य));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		अगर (!space_alloc || !realm_alloc || !auth_alloc) {
			मुक्त(space_alloc);
			मुक्त(realm_alloc);
			मुक्त(auth_alloc);
			वापस;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->port = p->port;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->अग्र = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
स्थिर अक्षर *urldb_get_auth_details(nsurl *url, स्थिर अक्षर *realm)
{
	काष्ठा path_तथ्य *p, *p_cur, *p_शीर्ष;

	निश्चित(url);

	/* add to the db, so our lookup will work */
	urldb_add_url(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस शून्य;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	क्रम (p_cur = p; p_cur != शून्य; p_शीर्ष = p_cur, p_cur = p_cur->parent) {
		अगर (p_cur->prot_space) {
			वापस p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	अगर (realm != शून्य) {
		स्थिर काष्ठा host_part *h = (स्थिर काष्ठा host_part *)p_शीर्ष;
		स्थिर काष्ठा prot_space_तथ्य *space;
		bool match;

		/* Search for a possible matching protection space. */
		क्रम (space = h->prot_space; space != शून्य;
		     space = space->अग्र) {
			अगर (!म_भेद(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &match) ==
			    lwc_error_ok &&
			    match == true &&
			    space->port == p->port) {
				p->prot_space = space;
				वापस p->prot_space->auth;
			}
		}
	}

	वापस शून्य;
}


/* exported interface documented in netsurf/url_db.h */
व्योम urldb_set_cert_permissions(nsurl *url, bool permit)
{
	काष्ठा path_तथ्य *p;
	काष्ठा host_part *h;

	निश्चित(url);

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस;

	क्रम (; p && p->parent; p = p->parent)
		/* do nothing */;
	निश्चित(p);

	h = (काष्ठा host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	काष्ठा path_तथ्य *p;
	स्थिर काष्ठा host_part *h;

	निश्चित(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस false;

	क्रम (; p && p->parent; p = p->parent)
		/* do nothing */;
	निश्चित(p);

	h = (स्थिर काष्ठा host_part *)p;

	वापस h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_set_hsts_policy(काष्ठा nsurl *url, स्थिर अक्षर *header)
{
	काष्ठा path_तथ्य *p;
	काष्ठा host_part *h;
	lwc_string *host;
	समय_प्रकार now = समय(शून्य);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	निश्चित(url);

	host = nsurl_get_component(url, NSURL_HOST);
	अगर (host != शून्य) {
		अगर (urldb__host_is_ip_address(lwc_string_तथ्य(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			वापस true;
		} अन्यथा अगर (lwc_string_लंबाई(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			वापस true;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: ignore */
		वापस true;
	}

	/* add url, in case it's missing */
	urldb_add_url(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस false;

	क्रम (; p && p->parent; p = p->parent)
		/* do nothing */;
	निश्चित(p);

	h = (काष्ठा host_part *)p;
	अगर (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		वापस true;
	}

	error = http_parse_strict_transport_security(header, &sts);
	अगर (error != NSERROR_OK) {
		/* Parse failed: ignore */
		वापस true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	अगर (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} अन्यथा अगर ((समय_प्रकार) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	वापस true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(काष्ठा nsurl *url)
{
	काष्ठा path_तथ्य *p;
	स्थिर काष्ठा host_part *h;
	lwc_string *host;
	समय_प्रकार now = समय(शून्य);

	निश्चित(url);

	host = nsurl_get_component(url, NSURL_HOST);
	अगर (host != शून्य) {
		अगर (urldb__host_is_ip_address(lwc_string_तथ्य(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			वापस false;
		} अन्यथा अगर (lwc_string_लंबाई(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			वापस false;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: not enabled */
		वापस false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस false;

	क्रम (; p && p->parent; p = p->parent)
		/* do nothing */;
	निश्चित(p);

	h = (स्थिर काष्ठा host_part *)p;

	/* Consult record for this host */
	अगर (h->hsts.expires > now) {
		/* Not expired */
		वापस true;
	}

	/* Consult parent domains */
	क्रम (h = h->parent; h && h != &db_मूल; h = h->parent) {
		अगर (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			वापस true;
		}
	}

	वापस false;
}


/* exported interface documented in netsurf/url_db.h */
व्योम
urldb_iterate_partial(स्थिर अक्षर *prefix,
		      bool (*callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य))
{
	अक्षर host[256];
	अक्षर buf[260]; /* max domain + "www." */
	स्थिर अक्षर *slash, *scheme_sep;
	काष्ठा खोज_गाँठ *वृक्ष;
	स्थिर काष्ठा host_part *h;

	निश्चित(prefix && callback);

	/* strip scheme */
	scheme_sep = म_माला(prefix, "://");
	अगर (scheme_sep)
		prefix = scheme_sep + 3;

	slash = म_अक्षर(prefix, '/');
	वृक्ष = urldb_get_खोज_वृक्ष(prefix);

	अगर (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, माप host, "%.*s",
			 (पूर्णांक) (slash - prefix), prefix);

		h = urldb_खोज_find(वृक्ष, host);
		अगर (!h) {
			पूर्णांक len = slash - prefix;

			अगर (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, माप buf, "www.%s", host);
				h = urldb_खोज_find(
					खोज_trees[ST_DN + 'w' - 'a'],
					buf);
				अगर (!h)
					वापस;
			} अन्यथा
				वापस;
		}

		अगर (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_path(&h->paths, slash + 1,
						   callback);
		}

	} अन्यथा {
		पूर्णांक len = म_माप(prefix);

		/* looking for hosts */
		अगर (!urldb_iterate_partial_host(वृक्ष, prefix, callback))
			वापस;

		अगर (len <= 3 || strncasecmp(prefix, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, माप buf, "www.%s", prefix);
			अगर(!urldb_iterate_partial_host(
				   खोज_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				वापस;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
व्योम
urldb_iterate_entries(bool (*callback)(nsurl *url, स्थिर काष्ठा url_तथ्य *तथ्य))
{
	पूर्णांक i;

	निश्चित(callback);

	क्रम (i = 0; i < अंक_खोज_TREES; i++) {
		अगर (!urldb_iterate_entries_host(खोज_trees[i],
						callback,
						शून्य)) {
			अवरोध;
		}
	}
}


/* exported interface documented in content/urldb.h */
व्योम urldb_iterate_cookies(bool (*callback)(स्थिर काष्ठा cookie_तथ्य *तथ्य))
{
	पूर्णांक i;

	निश्चित(callback);

	क्रम (i = 0; i < अंक_खोज_TREES; i++) {
		अगर (!urldb_iterate_entries_host(खोज_trees[i],
						शून्य, callback))
			अवरोध;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_set_cookie(स्थिर अक्षर *header, nsurl *url, nsurl *referer)
{
	स्थिर अक्षर *cur = header, *end;
	lwc_string *path, *host, *scheme;
	nsurl *urlt;
	bool match;

	निश्चित(url && header);

	/* Get defragmented URL, as 'urlt' */
	अगर (nsurl_defragment(url, &urlt) != NSERROR_OK)
		वापस शून्य;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (scheme == शून्य) {
		nsurl_unref(urlt);
		वापस false;
	}

	path = nsurl_get_component(url, NSURL_PATH);
	अगर (path == शून्य) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		वापस false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	अगर (host == शून्य) {
		lwc_string_unref(path);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		वापस false;
	}

	अगर (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		अगर (rhost == शून्य) {
			जाओ error;
		}

		/* Domain match host names */
		अगर (lwc_string_isequal(host, rhost, &match) == lwc_error_ok &&
		    match == false) {
			स्थिर अक्षर *hptr;
			स्थिर अक्षर *rptr;
			स्थिर अक्षर *dot;
			स्थिर अक्षर *host_तथ्य = lwc_string_तथ्य(host);
			स्थिर अक्षर *rhost_तथ्य = lwc_string_तथ्य(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			अगर (urldb__host_is_ip_address(host_तथ्य) ||
			    urldb__host_is_ip_address(rhost_तथ्य)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				जाओ error;
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

			hptr = host_तथ्य + lwc_string_लंबाई(host) - 1;
			rptr = rhost_तथ्य + lwc_string_लंबाई(rhost) - 1;

			/* 1 */
			जबतक (hptr >= host_तथ्य && rptr >= rhost_तथ्य) {
				अगर (*hptr != *rptr)
					अवरोध;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			जबतक (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			अगर (*hptr == '\0' ||
			    (dot = म_अक्षर(hptr + 1, '.')) == शून्य ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				जाओ error;
			}
		}

		lwc_string_unref(rhost);
	}

	end = cur + म_माप(cur) - 2 /* Trailing CRLF */;

	करो {
		काष्ठा cookie_internal_तथ्य *c;
		अक्षर *dot;
		माप_प्रकार len;
#अगर_घोषित WITH_NSPSL
		स्थिर अक्षर *suffix;
#पूर्ण_अगर

		c = urldb_parse_cookie(url, &cur);
		अगर (!c) {
			/* failed => stop parsing */
			जाओ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		अगर (!c->name || !c->मूल्य) {
			urldb_free_cookie(c);
			जाओ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = म_माप(c->path);
		अगर (len > lwc_string_लंबाई(path) ||
		    म_भेदन(c->path, lwc_string_तथ्य(path),
			    len) != 0) {
			urldb_free_cookie(c);
			जाओ error;
		}

#अगर_घोषित WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		अगर (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		अगर (suffix == शून्य) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			जाओ error;
		}
#अन्यथा
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = म_अक्षर(c->domain + 1, '.');
		अगर (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			जाओ error;
		}
#पूर्ण_अगर

		/* Domain match fetch host with cookie domain */
		अगर (strcasecmp(lwc_string_तथ्य(host), c->domain) != 0) {
			पूर्णांक hlen, dlen;
			अक्षर *domain = c->domain;

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
			निश्चित(c->domain[0] == '.');

			/* 4.3.2:iii */
			अगर (urldb__host_is_ip_address(lwc_string_तथ्य(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				जाओ error;
			}

			hlen = lwc_string_लंबाई(host);
			dlen = म_माप(c->domain);

			अगर (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				जाओ error;
			}

			अगर (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			अगर (strcasecmp(lwc_string_तथ्य(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				जाओ error;
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
		अगर (!urldb_जोड़ो_cookie(c, scheme, urlt))
			जाओ error;
	} जबतक (cur < end);

	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	वापस true;

error:
	lwc_string_unref(host);
	lwc_string_unref(path);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	वापस false;
}


/* exported interface documented in content/urldb.h */
अक्षर *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	स्थिर काष्ठा path_तथ्य *p, *q;
	स्थिर काष्ठा host_part *h;
	lwc_string *path_lwc;
	काष्ठा cookie_internal_तथ्य *c;
	पूर्णांक गिनती = 0, version = COOKIE_RFC2965;
	काष्ठा cookie_internal_तथ्य **matched_cookies;
	पूर्णांक matched_cookies_आकार = 20;
	स्थिर अक्षर *path;
	अक्षर *ret;
	lwc_string *scheme;
	समय_प्रकार now;
	bool match;

	निश्चित(url != शून्य);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_add_url(url);

	p = urldb_find_url(url);
	अगर (!p)
		वापस शून्य;

	scheme = p->scheme;

	matched_cookies = दो_स्मृति(matched_cookies_आकार *
				 माप(काष्ठा cookie_internal_तथ्य *));
	अगर (!matched_cookies)
		वापस शून्य;

#घोषणा GROW_MATCHED_COOKIES						\
	करो {								\
		अगर (गिनती == matched_cookies_आकार) {			\
			काष्ठा cookie_internal_तथ्य **अस्थायी;		\
			अस्थायी = पुनः_स्मृति(matched_cookies,			\
				       (matched_cookies_आकार + 20) *	\
				       माप(काष्ठा cookie_internal_तथ्य *)); \
									\
			अगर (अस्थायी == शून्य) {				\
				मुक्त(matched_cookies);			\
				वापस शून्य;				\
			}						\
									\
			matched_cookies = अस्थायी;				\
			matched_cookies_आकार += 20;			\
		}							\
	} जबतक(0)

	path_lwc = nsurl_get_component(url, NSURL_PATH);
	अगर (path_lwc == शून्य) {
		मुक्त(matched_cookies);
		वापस शून्य;
	}
	path = lwc_string_तथ्य(path_lwc);
	lwc_string_unref(path_lwc);

	now = समय(शून्य);

	अगर (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		क्रम (q = p->parent->children; q; q = q->अग्र) {
			अगर (म_भेद(q->segment, p->segment))
				जारी;

			/* Consider all cookies associated with
			 * this exact path */
			क्रम (c = q->cookies; c; c = c->अग्र) {
				अगर (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					जारी;

				अगर (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* secure cookie for insecure host.
					 * ignore */
					जारी;

				अगर (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					जारी;

				matched_cookies[गिनती++] = c;

				GROW_MATCHED_COOKIES;

				अगर (c->version < (अचिन्हित पूर्णांक)version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((काष्ठा cookie_तथ्य *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	क्रम (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		क्रम (q = p->children; q; q = q->अग्र) {
			अगर (*(q->segment) != '\0')
				जारी;

			क्रम (c = q->cookies; c; c = c->अग्र) {
				अगर (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					जारी;

				अगर (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &match) &&
				    match == false)
					/* Secure cookie for insecure server
					 * => ignore */
					जारी;

				matched_cookies[गिनती++] = c;

				GROW_MATCHED_COOKIES;

				अगर (c->version < (अचिन्हित पूर्णांक) version)
					version = c->version;

				c->last_used = now;

				cookie_manager_add((काष्ठा cookie_तथ्य *)c);
			}
		}

		अगर (!p->parent) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			अवरोध;
		}

		/* Consider p itself - may be the result of Path=/foo */
		क्रम (c = p->cookies; c; c = c->अग्र) {
			अगर (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				जारी;

			/* Ensure cookie path is a prefix of the resource */
			अगर (म_भेदन(c->path, path, म_माप(c->path)) != 0)
				/* paths don't match => ignore */
				जारी;

			अगर (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* Secure cookie for insecure server
				 * => ignore */
				जारी;

			matched_cookies[गिनती++] = c;

			GROW_MATCHED_COOKIES;

			अगर (c->version < (अचिन्हित पूर्णांक) version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((काष्ठा cookie_तथ्य *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	क्रम (h = (स्थिर काष्ठा host_part *)p; h && h != &db_मूल;
	     h = h->parent) {
		क्रम (c = h->paths.cookies; c; c = c->अग्र) {
			अगर (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				जारी;

			/* Ensure cookie path is a prefix of the resource */
			अगर (म_भेदन(c->path, path, म_माप(c->path)) != 0)
				/* paths don't match => ignore */
				जारी;

			अगर (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &match) &&
			    match == false)
				/* secure cookie for insecure host. ignore */
				जारी;

			matched_cookies[गिनती++] = c;

			GROW_MATCHED_COOKIES;

			अगर (c->version < (अचिन्हित पूर्णांक)version)
				version = c->version;

			c->last_used = now;

			cookie_manager_add((काष्ठा cookie_तथ्य *)c);
		}
	}
#अघोषित GROW_MATCHED_COOKIES

	अगर (गिनती == 0) {
		/* No cookies found */
		मुक्त(matched_cookies);
		वापस शून्य;
	}

	अगर (version == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, गिनती);
	} अन्यथा {
		ret = get_rfc2965_cookie(matched_cookies, गिनती, version);
	}
	मुक्त(matched_cookies);

	वापस ret;
}


/* exported interface documented in content/urldb.h */
व्योम urldb_delete_cookie(स्थिर अक्षर *domain, स्थिर अक्षर *path,
			 स्थिर अक्षर *name)
{
	urldb_delete_cookie_hosts(domain, path, name, &db_मूल);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_load_cookies(स्थिर अक्षर *filename)
{
	खाता *fp;
	अक्षर s[16*1024];

	निश्चित(filename);

	fp = ख_खोलो(filename, "r");
	अगर (!fp)
		वापस NSERROR_NOT_FOUND;

#घोषणा FIND_T {				\
		क्रम (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		अगर (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			जारी;		\
		}				\
		*p++ = '\0';			\
	}

#घोषणा SKIP_T {				\
		क्रम (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		अगर (p >= end) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			जारी;		\
		}				\
	}

	जबतक (ख_माला_लो(s, माप s, fp)) {
		अक्षर *p = s, *end = 0,
			*domain, *path, *name, *मूल्य, *scheme, *url,
			*comment;
		पूर्णांक version, domain_specified, path_specified,
			secure, http_only, no_destroy, मूल्य_quoted;
		समय_प्रकार expires, last_used;
		काष्ठा cookie_internal_तथ्य *c;

		अगर(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			जारी;

		s[म_माप(s) - 1] = '\0'; /* lose terminating newline */
		end = s + म_माप(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		अगर (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_version = म_से_प(p);

			अगर (loaded_cookie_file_version <
			    MIN_COOKIE_FILE_VERSION) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				अवरोध;
			}

			जारी;
		} अन्यथा अगर (loaded_cookie_file_version == 0) {
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
		अगर (loaded_cookie_file_version > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = म_से_प(p); FIND_T;
		} अन्यथा {
			http_only = 0;
		}
		SKIP_T; expires = (समय_प्रकार)म_से_प(p); FIND_T;
		SKIP_T; last_used = (समय_प्रकार)म_से_प(p); FIND_T;
		SKIP_T; no_destroy = म_से_प(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; मूल्य = p; FIND_T;
		अगर (loaded_cookie_file_version > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	मूल्य_quoted = म_से_प(p); FIND_T;
		} अन्यथा {
			मूल्य_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		क्रम (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		निश्चित(p <= end);

		/* Now create cookie */
		c = दो_स्मृति(माप(काष्ठा cookie_internal_तथ्य));
		अगर (!c)
			अवरोध;

		c->name = strdup(name);
		c->मूल्य = strdup(मूल्य);
		c->मूल्य_was_quoted = मूल्य_quoted;
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

		अगर (!(c->name && c->मूल्य && c->comment &&
		      c->domain && c->path)) {
			urldb_free_cookie(c);
			अवरोध;
		}

		अगर (c->domain[0] != '.') {
			lwc_string *scheme_lwc = शून्य;
			nsurl *url_nsurl = शून्य;

			निश्चित(scheme[0] != 'u');

			अगर (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				अवरोध;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			अगर (!urldb_जोड़ो_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				अवरोध;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} अन्यथा {
			अगर (!urldb_जोड़ो_cookie(c, शून्य, शून्य)) {
				/* Cookie freed for us */
				अवरोध;
			}
		}
	}

#अघोषित SKIP_T
#अघोषित FIND_T

	ख_बंद(fp);

	वापस NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(स्थिर अक्षर *filename)
{
	खाता *fp;
	पूर्णांक cookie_file_version = max(loaded_cookie_file_version,
				      COOKIE_FILE_VERSION);

	निश्चित(filename);

	fp = ख_खोलो(filename, "w");
	अगर (!fp)
		वापस NSERROR_NOT_FOUND;

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

	urldb_save_cookie_hosts(fp, &db_मूल);

	ख_बंद(fp);

	वापस NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
व्योम urldb_dump(व्योम)
{
	पूर्णांक i;

	urldb_dump_hosts(&db_मूल);

	क्रम (i = 0; i != अंक_खोज_TREES; i++) {
		urldb_dump_खोज(खोज_trees[i], 0);
	}
}
