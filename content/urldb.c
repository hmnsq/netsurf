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
#اگر_تعریف_شده WITH_NSPSL
#ضمن <nspsl.h>
#پایان_اگر

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
بنية cookie_داخلی_داده {
	بنية cookie_داخلی_داده *prev;	/**< Previous in list */
	بنية cookie_داخلی_داده *بعدی;	/**< Next in list */

	حرف *name;		/**< Cookie name */
	حرف *مقدار;		/**< Cookie value */
	bool مقدار_was_quoted;	/**< Value was quoted in Set-Cookie: */
	حرف *comment;		/**< Cookie comment */
	bool domain_from_مجموعه;	/**< Domain came from Set-Cookie: header */
	حرف *domain;		/**< Domain */
	bool مسیر_from_مجموعه;	/**< Path came from Set-Cookie: header */
	حرف *مسیر;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t آخرین_استفاده_شده;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	تعداد cookie_نسخه نسخه;	/**< Specification compliance */
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
بنية prot_space_داده {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	بیعلامت صحيح پورت;
	/** Protection realm */
	حرف *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	حرف *auth;
	/** Next sibling */
	بنية prot_space_داده *بعدی;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
بنية url_داخلی_داده {
	حرف *title;		/**< Resource title */
	بیعلامت صحيح visits;	/**< Visit count */
	time_t آخرین_بازدید;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
بنية مسیر_داده {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	بیعلامت صحيح پورت;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	حرف *segment;		/**< Path segment for this node */
	بیعلامت صحيح frag_cnt;	/**< Number of entries in path_data::fragment */
	حرف **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	بنية url_داخلی_داده urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ثابت بنية prot_space_داده *prot_space;
	/** Cookies associated with resource */
	بنية cookie_داخلی_داده *cookies;
	/** Last cookie in list */
	بنية cookie_داخلی_داده *cookies_انتها;

	بنية مسیر_داده *بعدی;	/**< Next sibling */
	بنية مسیر_داده *prev;	/**< Previous sibling */
	بنية مسیر_داده *والد; /**< Parent path segment */
	بنية مسیر_داده *children; /**< Child path segments */
	بنية مسیر_داده *آخرین; /**< Last child */
};

بنية hsts_داده {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

بنية host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	بنية مسیر_داده paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	بنية hsts_داده hsts;

	/**
	 * Part of host string
	 */
	حرف *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	بنية prot_space_داده *prot_space;

	بنية host_part *بعدی;	/**< Next sibling */
	بنية host_part *prev;	/**< Previous sibling */
	بنية host_part *والد; /**< Parent host part */
	بنية host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
بنية جستجو_گره {
	ثابت بنية host_part *داده;	/**< Host tree entry */

	بیعلامت صحيح level;		/**< Node level */

	بنية جستجو_گره *چپ;	/**< Left subtree */
	بنية جستجو_گره *راست;	/**< Right subtree */
};

/** Root database handle */
ساكن بنية host_part db_ریشه;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#تعریف_کن رقم_جستجو_TREES 28
#تعریف_کن ST_IP 0
#تعریف_کن ST_EE 1
#تعریف_کن ST_DN 2
ساكن بنية جستجو_گره خالی = { 0, 0, &خالی, &خالی };
ساكن بنية جستجو_گره *جستجو_trees[رقم_جستجو_TREES] = {
	&خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی,
	&خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی,
	&خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی, &خالی,
	&خالی, &خالی, &خالی, &خالی
};

/** Minimum cookie database file version */
#تعریف_کن MIN_COOKIE_FILE_نسخه 100
/** Current cookie database file version */
#تعریف_کن COOKIE_FILE_نسخه 102
/** loaded cookie file version */
ساكن صحيح loaded_cookie_file_نسخه;

/** Minimum URL database file version */
#تعریف_کن MIN_URL_FILE_نسخه 106
/** Current URL database file version */
#تعریف_کن URL_FILE_نسخه 107

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
#تعریف_کن BLOOM_اندازه (1024 * 32)


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
	إذا (use == 0) {
		fprintf(fp, "%i\n", (صحيح)val);
	} وإلا {
		fprintf(fp, "%.*s\n", use, op);
	}
	أرجع NSERROR_OK;
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
urldb_write_paths(ثابت بنية مسیر_داده *والد,
		  ثابت حرف *host,
		  FILE *fp,
		  حرف **مسیر,
		  صحيح *مسیر_alloc,
		  صحيح *مسیر_استفاده_شده,
		  time_t expiry)
{
	ثابت بنية مسیر_داده *p = والد;
	صحيح i;

	افعل {
		صحيح seglen = p->segment != عدم ? طول_رشته(p->segment) : 0;
		صحيح len = *مسیر_استفاده_شده + seglen + 1;

		إذا (*مسیر_alloc < len) {
			حرف *موقت;
			موقت = realloc(*مسیر,
				       (len > 64) ? len : *مسیر_alloc + 64);
			إذا (!موقت) {
				أرجع;
			}
			*مسیر = موقت;
			*مسیر_alloc = (len > 64) ? len : *مسیر_alloc + 64;
		}

		إذا (p->segment != عدم) {
			کپی_حافظه(*مسیر + *مسیر_استفاده_شده - 1, p->segment, seglen);
		}

		إذا (p->children != عدم) {
			(*مسیر)[*مسیر_استفاده_شده + seglen - 1] = '/';
			(*مسیر)[*مسیر_استفاده_شده + seglen] = '\0';
		} وإلا {
			(*مسیر)[*مسیر_استفاده_شده + seglen - 1] = '\0';
			len -= 1;
		}

		*مسیر_استفاده_شده = len;

		إذا (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} وإلا {
			/* leaf node */
			إذا (p->persistent ||
			    ((p->urld.آخرین_بازدید > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_داده(p->scheme));

				إذا (p->پورت) {
					fprintf(fp,"%d\n", p->پورت);
				} وإلا {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *مسیر);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.آخرین_بازدید);

				/* entry type */
				fprintf(fp, "%i\n", (صحيح)p->urld.type);

				fprintf(fp, "\n");

				إذا (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					لكل (i = 0; s[i] != '\0'; i++)
						إذا (s[i] < 32)
							s[i] = ' ';
					لكل (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} وإلا {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			طالما (p != والد) {
				صحيح seglen = p->segment != عدم
					? طول_رشته(p->segment) : 0;

				/* Remove our segment from the path */
				*مسیر_استفاده_شده -= seglen;
				(*مسیر)[*مسیر_استفاده_شده - 1] = '\0';

				إذا (p->بعدی != عدم) {
					/* Have a sibling, process that */
					p = p->بعدی;
					اكسر;
				}

				/* Going up, so remove '/' */
				*مسیر_استفاده_شده -= 1;
				(*مسیر)[*مسیر_استفاده_شده - 1] = '\0';

				/* Ascend tree */
				p = p->والد;
			}
		}
	} طالما (p != والد);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ساكن فراغ
urldb_شمار_urls(ثابت بنية مسیر_داده *ریشه,
		 time_t expiry,
		 بیعلامت صحيح *شمار)
{
	ثابت بنية مسیر_داده *p = ریشه;

	افعل {
		إذا (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} وإلا {
			/* No more children, increment count if required */
			إذا (p->persistent ||
			    ((p->urld.آخرین_بازدید > expiry) &&
			     (p->urld.visits > 0))) {
				(*شمار)++;
			}

			/* Now, find next node to process. */
			طالما (p != ریشه) {
				إذا (p->بعدی != عدم) {
					/* Have a sibling, process that */
					p = p->بعدی;
					اكسر;
				}

				/* Ascend tree */
				p = p->والد;
			}
		}
	} طالما (p != ریشه);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ساكن فراغ urldb_save_جستجو_درخت(بنية جستجو_گره *والد, FILE *fp)
{
	حرف host[256];
	ثابت بنية host_part *h;
	بیعلامت صحيح مسیر_شمار = 0;
	حرف *مسیر, *p, *انتها;
	صحيح مسیر_alloc = 64, مسیر_استفاده_شده = 1;
	time_t expiry, hsts_expiry = 0;
	صحيح hsts_include_subdomains = 0;

	expiry = time(عدم) - ((60 * 60 * 24) * nsoption_int(expire_url));

	إذا (والد == &خالی)
		أرجع;

	urldb_save_جستجو_درخت(والد->چپ, fp);

	مسیر = حافظه_بده(مسیر_alloc);
	إذا (!مسیر)
		أرجع;

	مسیر[0] = '\0';

	لكل (h = والد->داده, p = host, انتها = host + حجم host;
	     h && h != &db_ریشه && p < انتها; h = h->والد) {
		صحيح written = snprintf(p, انتها - p, "%s%s", h->part,
				       (h->والد && h->والد->والد) ? "." : "");
		إذا (written < 0) {
			حرر(مسیر);
			أرجع;
		}
		p += written;
	}

	h = والد->داده;
	إذا (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_شمار_urls(&والد->داده->paths, expiry, &مسیر_شمار);

	إذا (مسیر_شمار > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", مسیر_شمار);

		urldb_write_paths(&والد->داده->paths, host, fp,
				  &مسیر, &مسیر_alloc, &مسیر_استفاده_شده, expiry);
	} وإلا إذا (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	حرر(مسیر);

	urldb_save_جستجو_درخت(والد->راست, fp);
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
urldb_iterate_entries_مسیر(ثابت بنية مسیر_داده *والد,
		bool (*url_callback)(nsurl *url, ثابت بنية url_داده *داده),
		bool (*cookie_callback)(ثابت بنية cookie_داده *داده))
{
	ثابت بنية مسیر_داده *p = والد;
	ثابت بنية cookie_داده *c;

	افعل {
		إذا (p->children != عدم) {
			/* Drill down into children */
			p = p->children;
		} وإلا {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			أكد(url_callback || cookie_callback);

			/** \todo handle fragments? */
			إذا (url_callback) {
				ثابت بنية url_داخلی_داده *u = &p->urld;

				أكد(p->url);

				إذا (!url_callback(p->url,
						  (ثابت بنية url_داده *) u))
					أرجع false;
			} وإلا {
				c = (ثابت بنية cookie_داده *)p->cookies;
				لكل (; c != عدم; c = c->بعدی) {
					إذا (!cookie_callback(c))
						أرجع false;
				}
			}

			/* Now, find next node to process. */
			طالما (p != والد) {
				إذا (p->بعدی != عدم) {
					/* Have a sibling, process that */
					p = p->بعدی;
					اكسر;
				}

				/* Ascend tree */
				p = p->والد;
			}
		}
	} طالما (p != والد);

	أرجع true;
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
ساكن bool urldb_host_is_ip_آدرس(ثابت حرف *host)
{
	بنية درون_addr ipv4;
	size_t host_len = طول_رشته(host);
	ثابت حرف *sane_host;
	ثابت حرف *slash;
#اگر_تعریف_نشده NO_IPV6
	بنية in6_addr ipv6;
	حرف ipv6_addr[64];
	بیعلامت صحيح ipv6_addr_len;
#پایان_اگر
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
	إذا (slash == عدم) {
		sane_host = host;
	} وإلا {
		حرف *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	إذا (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		اذهب بیرون_false;

	إذا (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		صحيح رقم_dots = 0;
		size_t index;

		لكل (index = 0; index < host_len; index++) {
			إذا (sane_host[index] == '.')
				رقم_dots++;
		}

		إذا (رقم_dots == 3)
			اذهب بیرون_true;
		وإلا
			اذهب بیرون_false;
	}

#اگر_تعریف_نشده NO_IPV6
	إذا ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		اذهب بیرون_false;
	}

	ipv6_addr_len = host_len - 2;
	إذا (ipv6_addr_len >= حجم(ipv6_addr)) {
		ipv6_addr_len = حجم(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	إذا (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		اذهب بیرون_true;
#پایان_اگر

بیرون_false:
	إذا (slash != عدم) حرر((فراغ *)sane_host);
	أرجع false;

بیرون_true:
	إذا (slash != عدم) حرر((فراغ *)sane_host);
	أرجع true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ساكن صحيح urldb_جستجو_تطابق_پیشوند(ثابت بنية host_part *a, ثابت حرف *b)
{
	ثابت حرف *انتها, *dot;
	صحيح plen, ret;

	أكد(a && a != &db_ریشه && b);

	إذا (urldb_host_is_ip_آدرس(b)) {
		/* IP address */
		أرجع strncasecmp(a->part, b, طول_رشته(b));
	}

	انتها = b + طول_رشته(b) + 1;

	طالما (b < انتها && a && a != &db_ریشه) {
		dot = strchr(b, '.');
		إذا (!dot) {
			/* last segment */
			dot = انتها - 1;
		}

		/* Compare strings (length limited) */
		إذا ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			أرجع ret;

		/* The strings matched */
		إذا (dot < انتها - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = طول_رشته(a->part);
			إذا (plen > dot - b) {
				/* len(a) > len(b) */
				أرجع 1;
			} وإلا إذا (plen < dot - b) {
				/* len(a) < len(b) */
				أرجع -1;
			}
		}

		b = dot + 1;
		a = a->والد;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	إذا (a && a != &db_ریشه && b >= انتها) {
		/* len(a) > len(b) => prefix matches */
		أرجع 0;
	} وإلا إذا ((!a || a == &db_ریشه) && b < انتها) {
		/* len(a) < len(b) => prefix does not match */
		أرجع -1;
	}

	/* Identical */
	أرجع 0;
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
urldb_iterate_partial_host(بنية جستجو_گره *ریشه,
		ثابت حرف *پیشوند,
		bool (*callback)(nsurl *url, ثابت بنية url_داده *داده))
{
	صحيح c;

	أكد(ریشه && پیشوند && callback);

	إذا (ریشه == &خالی)
		أرجع true;

	c = urldb_جستجو_تطابق_پیشوند(ریشه->داده, پیشوند);

	إذا (c > 0) {
		/* No match => look in left subtree */
		أرجع urldb_iterate_partial_host(ریشه->چپ,
						  پیشوند,
						  callback);
	} وإلا إذا (c < 0) {
		/* No match => look in right subtree */
		أرجع urldb_iterate_partial_host(ریشه->راست,
						  پیشوند,
						  callback);
	} وإلا {
		/* Match => iterate over l/r subtrees & process this node */
		إذا (!urldb_iterate_partial_host(ریشه->چپ,
						پیشوند,
						callback)) {
			أرجع false;
		}

		إذا (ریشه->داده->paths.children) {
			/* and extract all paths attached to this host */
			إذا (!urldb_iterate_entries_مسیر(&ریشه->داده->paths,
							callback,
							عدم)) {
				أرجع false;
			}
		}

		إذا (!urldb_iterate_partial_host(ریشه->راست,
						پیشوند,
						callback)) {
			أرجع false;
		}
	}

	أرجع true;
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
urldb_iterate_partial_مسیر(ثابت بنية مسیر_داده *والد,
		ثابت حرف *پیشوند,
		bool (*callback)(nsurl *url, ثابت بنية url_داده *داده))
{
	ثابت بنية مسیر_داده *p = والد->children;
	ثابت حرف *slash, *انتها = پیشوند + طول_رشته(پیشوند);

	افعل {
		slash = strchr(پیشوند, '/');
		إذا (!slash) {
			slash = انتها;
		}

		إذا (slash == پیشوند && *پیشوند == '/') {
			/* Ignore "//" */
			پیشوند++;
			استمر;
		}

		إذا (strncasecmp(p->segment, پیشوند, slash - پیشوند) == 0) {
			/* prefix matches so far */
			إذا (slash == انتها) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				إذا (!urldb_iterate_entries_مسیر(p,
								callback,
								عدم)) {
					أرجع false;
				}

				/* Progress to next sibling */
				p = p->بعدی;
			} وإلا {
				/* Skip over this segment */
				پیشوند = slash + 1;

				p = p->children;
			}
		} وإلا {
			/* Doesn't match this segment, try next sibling */
			p = p->بعدی;
		}
	} طالما (p != عدم);

	أرجع true;
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
urldb_iterate_entries_host(بنية جستجو_گره *والد,
		bool (*url_callback)(nsurl *url, ثابت بنية url_داده *داده),
		bool (*cookie_callback)(ثابت بنية cookie_داده *داده))
{
	إذا (والد == &خالی) {
		أرجع true;
	}

	إذا (!urldb_iterate_entries_host(والد->چپ,
					url_callback,
					cookie_callback)) {
		أرجع false;
	}

	إذا ((والد->داده->paths.children) ||
	    ((cookie_callback) &&
	     (والد->داده->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		إذا (!urldb_iterate_entries_مسیر(&والد->داده->paths,
						url_callback,
						cookie_callback)) {
			أرجع false;
		}
	}

	إذا (!urldb_iterate_entries_host(والد->راست,
					url_callback,
					cookie_callback)) {
		أرجع false;
	}

	أرجع true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ساكن بنية host_part *
urldb_جمع_کن_host_گره(ثابت حرف *part, بنية host_part *والد)
{
	بنية host_part *d;

	أكد(part && والد);

	d = حافظه_صفر_بده(1, حجم(بنية host_part));
	إذا (!d) {
		أرجع عدم;
	}

	d->part = strdup(part);
	إذا (!d->part) {
		حرر(d);
		أرجع عدم;
	}

	d->بعدی = والد->children;
	إذا (والد->children) {
		والد->children->prev = d;
	}
	d->والد = والد;
	والد->children = d;

	أرجع d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ساكن صحيح urldb_جمع_کن_مسیر_fragment_cmp(ثابت فراغ *a, ثابت فراغ *b)
{
	أرجع strcasecmp(*((ثابت حرف **) a), *((ثابت حرف **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ساكن بنية مسیر_داده *
urldb_جمع_کن_مسیر_fragment(بنية مسیر_داده *segment, lwc_string *fragment)
{
	حرف **موقت;

	أكد(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	إذا (!fragment)
		أرجع segment;

	موقت = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * حجم(حرف *));
	إذا (!موقت)
		أرجع عدم;

	segment->fragment = موقت;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_داده(fragment));
	إذا (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		أرجع عدم;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      حجم (حرف *),
	      urldb_جمع_کن_مسیر_fragment_cmp);

	أرجع segment;
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
ساكن بنية مسیر_داده *
urldb_جمع_کن_مسیر_گره(lwc_string *scheme,
		    بیعلامت صحيح پورت,
		    ثابت حرف *segment,
		    lwc_string *fragment,
		    بنية مسیر_داده *والد)
{
	بنية مسیر_داده *d, *e;

	أكد(scheme && segment && والد);

	d = حافظه_صفر_بده(1, حجم(بنية مسیر_داده));
	إذا (!d)
		أرجع عدم;

	d->scheme = lwc_string_ref(scheme);

	d->پورت = پورت;

	d->segment = strdup(segment);
	إذا (!d->segment) {
		lwc_string_unref(d->scheme);
		حرر(d);
		أرجع عدم;
	}

	إذا (fragment) {
		إذا (!urldb_جمع_کن_مسیر_fragment(d, fragment)) {
			حرر(d->segment);
			lwc_string_unref(d->scheme);
			حرر(d);
			أرجع عدم;
		}
	}

	لكل (e = والد->children; e; e = e->بعدی) {
		إذا (strcmp(e->segment, d->segment) > 0)
			اكسر;
	}

	إذا (e) {
		d->prev = e->prev;
		d->بعدی = e;
		إذا (e->prev)
			e->prev->بعدی = d;
		وإلا
			والد->children = d;
		e->prev = d;
	} وإلا إذا (!والد->children) {
		d->prev = d->بعدی = عدم;
		والد->children = والد->آخرین = d;
	} وإلا {
		d->بعدی = عدم;
		d->prev = والد->آخرین;
		والد->آخرین->بعدی = d;
		والد->آخرین = d;
	}
	d->والد = والد;

	أرجع d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساكن بنية جستجو_گره **urldb_get_جستجو_درخت_مستقیم(ثابت حرف *host)
{
	أكد(host);

	إذا (urldb_host_is_ip_آدرس(host)) {
		أرجع &جستجو_trees[ST_IP];
	} وإلا إذا (ascii_is_alpha(*host)) {
		أرجع &جستجو_trees[ST_DN + ascii_مقصد_پایینی(*host) - 'a'];
	}
	أرجع &جستجو_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساكن بنية جستجو_گره *urldb_get_جستجو_درخت(ثابت حرف *host)
{
	أرجع *urldb_get_جستجو_درخت_مستقیم(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ساكن صحيح urldb_جستجو_تطابق_string(ثابت بنية host_part *a, ثابت حرف *b)
{
	ثابت حرف *انتها, *dot;
	صحيح plen, ret;

	أكد(a && a != &db_ریشه && b);

	إذا (urldb_host_is_ip_آدرس(b)) {
		/* IP address */
		أرجع strcasecmp(a->part, b);
	}

	انتها = b + طول_رشته(b) + 1;

	طالما (b < انتها && a && a != &db_ریشه) {
		dot = strchr(b, '.');
		إذا (!dot) {
			/* last segment */
			dot = انتها - 1;
		}

		/* Compare strings (length limited) */
		إذا ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			أرجع ret;

		/* The strings matched, now check that the lengths do, too */
		plen = طول_رشته(a->part);

		إذا (plen > dot - b) {
			/* len(a) > len(b) */
			أرجع 1;
		} وإلا إذا (plen < dot - b) {
			/* len(a) < len(b) */
			أرجع -1;
		}

		b = dot + 1;
		a = a->والد;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	إذا (a && a != &db_ریشه && b >= انتها) {
		/* len(a) > len(b) */
		أرجع 1;
	} وإلا إذا ((!a || a == &db_ریشه) && b < انتها) {
		/* len(a) < len(b) */
		أرجع -1;
	}

	/* Identical */
	أرجع 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ساكن ثابت بنية host_part *
urldb_جستجو_پیدا_کن(بنية جستجو_گره *ریشه, ثابت حرف *host)
{
	صحيح c;

	أكد(ریشه && host);

	إذا (ریشه == &خالی) {
		أرجع عدم;
	}

	c = urldb_جستجو_تطابق_string(ریشه->داده, host);

	إذا (c > 0) {
		أرجع urldb_جستجو_پیدا_کن(ریشه->چپ, host);
	} وإلا إذا (c < 0) {
		أرجع urldb_جستجو_پیدا_کن(ریشه->راست, host);
	}

	أرجع ریشه->داده;
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
ساكن بنية مسیر_داده *
urldb_تطابق_مسیر(ثابت بنية مسیر_داده *والد,
		 ثابت حرف *مسیر,
		 lwc_string *scheme,
		 بیعلامت کوتاه پورت)
{
	ثابت بنية مسیر_داده *p;
	ثابت حرف *slash;
	bool تطابق;

	أكد(والد != عدم);
	أكد(والد->segment == عدم);

	إذا (مسیر[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", مسیر);
	}

	أكد(مسیر[0] == '/');

	/* Start with children, as parent has no segment */
	p = والد->children;

	طالما (p != عدم) {
		slash = strchr(مسیر + 1, '/');
		إذا (!slash) {
			slash = مسیر + طول_رشته(مسیر);
		}

		إذا (مقایسه_رشته(p->segment, مسیر + 1, slash - مسیر - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &تطابق) == lwc_error_ok &&
		    تطابق == true &&
		    p->پورت == پورت) {
			إذا (*slash == '\0') {
				/* Complete match */
				أرجع (بنية مسیر_داده *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			مسیر = slash;
		} وإلا {
			/* No match, try next sibling */
			p = p->بعدی;
		}
	}

	أرجع عدم;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ساكن بنية مسیر_داده *urldb_پیدا_کن_url(nsurl *url)
{
	ثابت بنية host_part *h;
	بنية مسیر_داده *p;
	بنية جستجو_گره *درخت;
	حرف *plq;
	ثابت حرف *host_str;
	lwc_string *scheme, *host, *پورت;
	size_t len = 0;
	بیعلامت صحيح پورت_int;
	bool تطابق;

	أكد(url);

	إذا (url_bloom != عدم) {
		إذا (bloom_جستجو_hash(url_bloom, nsurl_hash(url)) == false) {
			أرجع عدم;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	إذا (scheme == عدم)
		أرجع عدم;

	إذا (lwc_string_isequal(scheme, corestring_lwc_mailto, &تطابق) ==
	    lwc_error_ok && تطابق == true) {
		lwc_string_unref(scheme);
		أرجع عدم;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	إذا (host != عدم) {
		host_str = lwc_string_داده(host);
		lwc_string_unref(host);

	} وإلا إذا (lwc_string_isequal(scheme, corestring_lwc_file, &تطابق) ==
		   lwc_error_ok && تطابق == true) {
		host_str = "localhost";

	} وإلا {
		lwc_string_unref(scheme);
		أرجع عدم;
	}

	درخت = urldb_get_جستجو_درخت(host_str);
	h = urldb_جستجو_پیدا_کن(درخت, host_str);
	إذا (!h) {
		lwc_string_unref(scheme);
		أرجع عدم;
	}

	/* generate plq (path, leaf, query) */
	إذا (nsurl_get(url, NSURL_مسیر | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		أرجع عدم;
	}

	/* Get port */
	پورت = nsurl_get_component(url, NSURL_پورت);
	إذا (پورت != عدم) {
		پورت_int = atoi(lwc_string_داده(پورت));
		lwc_string_unref(پورت);
	} وإلا {
		پورت_int = 0;
	}

	p = urldb_تطابق_مسیر(&h->paths, plq, scheme, پورت_int);

	حرر(plq);
	lwc_string_unref(scheme);

	أرجع p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساكن فراغ urldb_dump_paths(بنية مسیر_داده *والد)
{
	ثابت بنية مسیر_داده *p = والد;
	بیعلامت صحيح i;

	افعل {
		إذا (p->segment != عدم) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_داده(p->scheme), p->پورت);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			لكل (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		إذا (p->children != عدم) {
			p = p->children;
		} وإلا {
			طالما (p != والد) {
				إذا (p->بعدی != عدم) {
					p = p->بعدی;
					اكسر;
				}

				p = p->والد;
			}
		}
	} طالما (p != والد);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساكن فراغ urldb_dump_hosts(بنية host_part *والد)
{
	بنية host_part *h;

	إذا (والد->part) {
		NSLOG(netsurf, INFO, "%s", والد->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      والد->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&والد->paths);

	/* and recurse */
	لكل (h = والد->children; h; h = h->بعدی) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ساكن فراغ urldb_dump_جستجو(بنية جستجو_گره *والد, صحيح عمق)
{
	ثابت بنية host_part *h;
	صحيح i; /* index into string */
	حرف s[1024];
	صحيح r;
	صحيح sl = حجم(s) - 2;

	إذا (والد == &خالی)
		أرجع;

	urldb_dump_جستجو(والد->چپ, عمق + 1);

	لكل (i = 0; i != عمق; i++) {
		s[i] = ' ';
	}

	لكل (h = والد->داده; h; h = h->والد) {
		إذا (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			إذا (r < 0) {
				اكسر;
			}
			إذا ((i + r) >= sl) {
				اكسر;
			}
			i += r;
		}

		إذا (h->والد && h->والد->والد) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_جستجو(والد->راست, عمق + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ساكن صحيح
urldb_جستجو_تطابق_host(ثابت بنية host_part *a, ثابت بنية host_part *b)
{
	صحيح ret;

	أكد(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	لكل (; a && a != &db_ریشه && b && b != &db_ریشه;
	     a = a->والد, b = b->والد) {
		إذا ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			أرجع ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	إذا (a && a != &db_ریشه && (!b || b == &db_ریشه)) {
		/* len(a) > len(b) */
		أرجع 1;
	} وإلا إذا ((!a || a == &db_ریشه) && b && b != &db_ریشه) {
		/* len(a) < len(b) */
		أرجع -1;
	}

	/* identical */
	أرجع 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ساكن بنية جستجو_گره *urldb_جستجو_skew(بنية جستجو_گره *ریشه)
{
	أكد(ریشه);

	إذا (ریشه->چپ->level == ریشه->level) {
		بنية جستجو_گره *موقت;

		موقت = ریشه->چپ;
		ریشه->چپ = موقت->راست;
		موقت->راست = ریشه;
		ریشه = موقت;
	}

	أرجع ریشه;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ساكن بنية جستجو_گره *urldb_جستجو_تقسیم(بنية جستجو_گره *ریشه)
{
	أكد(ریشه);

	إذا (ریشه->راست->راست->level == ریشه->level) {
		بنية جستجو_گره *موقت;

		موقت = ریشه->راست;
		ریشه->راست = موقت->چپ;
		موقت->چپ = ریشه;
		ریشه = موقت;

		ریشه->level++;
	}

	أرجع ریشه;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ساكن بنية جستجو_گره *
urldb_جستجو_درج_داخلی(بنية جستجو_گره *ریشه, بنية جستجو_گره *n)
{
	أكد(ریشه && n);

	إذا (ریشه == &خالی) {
		ریشه = n;
	} وإلا {
		صحيح c = urldb_جستجو_تطابق_host(ریشه->داده, n->داده);

		إذا (c > 0) {
			ریشه->چپ = urldb_جستجو_درج_داخلی(
				ریشه->چپ, n);
		} وإلا إذا (c < 0) {
			ریشه->راست = urldb_جستجو_درج_داخلی(
				ریشه->راست, n);
		} وإلا {
			/* exact match */
			حرر(n);
			أرجع ریشه;
		}

		ریشه = urldb_جستجو_skew(ریشه);
		ریشه = urldb_جستجو_تقسیم(ریشه);
	}

	أرجع ریشه;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ساكن بنية جستجو_گره *
urldb_جستجو_درج(بنية جستجو_گره *ریشه, ثابت بنية host_part *داده)
{
	بنية جستجو_گره *n;

	أكد(ریشه && داده);

	n = حافظه_بده(حجم(بنية جستجو_گره));
	إذا (!n)
		أرجع عدم;

	n->level = 1;
	n->داده = داده;
	n->چپ = n->راست = &خالی;

	ریشه = urldb_جستجو_درج_داخلی(ریشه, n);

	أرجع ریشه;
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
urldb_parse_avpair(بنية cookie_داخلی_داده *c,
		   حرف *n,
		   حرف *v,
		   bool was_quoted)
{
	صحيح vlen;

	أكد(c && n && v);

	/* Strip whitespace from start of name */
	لكل (; *n; n++) {
		إذا (*n != ' ' && *n != '\t')
			اكسر;
	}

	/* Strip whitespace from end of name */
	لكل (vlen = طول_رشته(n); vlen; vlen--) {
		إذا (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		وإلا
			اكسر;
	}

	/* Strip whitespace from start of value */
	لكل (; *v; v++) {
		إذا (*v != ' ' && *v != '\t')
			اكسر;
	}

	/* Strip whitespace from end of value */
	لكل (vlen = طول_رشته(v); vlen; vlen--) {
		إذا (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		وإلا
			اكسر;
	}

	إذا (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		إذا (!c->comment)
			أرجع false;
	} وإلا إذا (!c->domain && strcasecmp(n, "Domain") == 0) {
		إذا (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_مجموعه = true;
			c->domain = strdup(v);
			إذا (!c->domain)
				أرجع false;
		}
	} وإلا إذا (strcasecmp(n, "Max-Age") == 0) {
		صحيح موقت = atoi(v);
		إذا (موقت == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		وإلا
			c->expires = time(عدم) + موقت;
	} وإلا إذا (!c->مسیر && strcasecmp(n, "Path") == 0) {
		c->مسیر_from_مجموعه = true;
		c->مسیر = strdup(v);
		إذا (!c->مسیر)
			أرجع false;
	} وإلا إذا (strcasecmp(n, "Version") == 0) {
		c->نسخه = atoi(v);
	} وإلا إذا (strcasecmp(n, "Expires") == 0) {
		حرف *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		لكل (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, طول_رشته(datenoday), &expires);
		إذا (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} وإلا إذا (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} وإلا إذا (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} وإلا إذا (!c->name) {
		c->name = strdup(n);
		c->مقدار = strdup(v);
		c->مقدار_was_quoted = was_quoted;
		إذا (!c->name || !c->مقدار) {
			أرجع false;
		}
	}

	أرجع true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ساكن فراغ urldb_free_cookie(بنية cookie_داخلی_داده *c)
{
	أكد(c);

	حرر(c->comment);
	حرر(c->domain);
	حرر(c->مسیر);
	حرر(c->name);
	حرر(c->مقدار);
	حرر(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ساكن بنية cookie_داخلی_داده *
urldb_parse_cookie(nsurl *url, ثابت حرف **cookie)
{
	بنية cookie_داخلی_داده *c;
	ثابت حرف *cur;
	حرف name[1024], مقدار[4096];
	حرف *n = name, *v = مقدار;
	bool درون_مقدار = false;
	bool had_مقدار_داده = false;
	bool مقدار_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	أكد(url && cookie && *cookie);

	c = حافظه_صفر_بده(1, حجم(بنية cookie_داخلی_داده));
	إذا (c == عدم)
		أرجع عدم;

	c->expires = -1;

	name[0] = '\0';
	مقدار[0] = '\0';

	لكل (cur = *cookie; *cur; cur++) {
		إذا (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			إذا (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				مقدار[0] = '\0';

			}

			اكسر;
		} وإلا إذا (*cur == '\r') {
			/* Spurious linefeed */
			استمر;
		} وإلا إذا (*cur == '\n') {
			/* Spurious newline */
			استمر;
		}

		إذا (درون_مقدار && !had_مقدار_داده) {
			إذا (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				استمر;
			} وإلا {
				had_مقدار_داده = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				إذا (*cur != '"') {
					مقدار_verbatim = true;
				}
			}
		}

		إذا (درون_مقدار && !مقدار_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			إذا (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				استمر;
			}
		}

		إذا (!quoted && !درون_مقدار && *cur == '=') {
			/* First equals => attr-value separator */
			درون_مقدار = true;
			استمر;
		}

		إذا (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			إذا (!urldb_parse_avpair(c, name, مقدار, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				أرجع عدم;
			}

			/* And reset to start */
			n = name;
			v = مقدار;
			درون_مقدار = false;
			had_مقدار_داده = false;
			مقدار_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			إذا (*cur != ';') {
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
		إذا (!quoted && *cur == ',') {
			/* Find semi-colon, if any */
			ثابت حرف *p;
			ثابت حرف *semi = strchr(cur + 1, ';');
			إذا (!semi)
				semi = cur + طول_رشته(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			لكل (p = cur + 1; p < semi; p++)
				إذا (*p == '=')
					اكسر;

			إذا (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} وإلا {
				/* found one => comma marks end of cookie */
				cur++;
				اكسر;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		إذا (!درون_مقدار) {
			إذا (n < name + (حجم(name) - 1))
				*n++ = *cur;
		} وإلا {
			إذا (v < مقدار + (حجم(مقدار) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	إذا (!urldb_parse_avpair(c, name, مقدار, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		أرجع عدم;
	}

	/* Now fix-up default values */
	إذا (c->domain == عدم) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		إذا (host == عدم) {
			urldb_free_cookie(c);
			أرجع عدم;
		}
		c->domain = strdup(lwc_string_داده(host));
		lwc_string_unref(host);
	}

	إذا (c->مسیر == عدم) {
		ثابت حرف *مسیر_داده;
		حرف *مسیر, *slash;
		lwc_string *مسیر_lwc;

		مسیر_lwc = nsurl_get_component(url, NSURL_مسیر);
		إذا (مسیر_lwc == عدم) {
			urldb_free_cookie(c);
			أرجع عدم;
		}
		مسیر_داده = lwc_string_داده(مسیر_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(مسیر_داده, '/');
		إذا (slash != عدم) {
			/* Special case: retain first slash in path */
			إذا (slash == مسیر_داده)
				slash++;

			slash = strndup(مسیر_داده, slash - مسیر_داده);
			إذا (slash == عدم) {
				lwc_string_unref(مسیر_lwc);
				urldb_free_cookie(c);
				أرجع عدم;
			}

			مسیر = slash;
			lwc_string_unref(مسیر_lwc);
		} وإلا {
			مسیر = strdup(lwc_string_داده(مسیر_lwc));
			lwc_string_unref(مسیر_lwc);
			إذا (مسیر == عدم) {
				urldb_free_cookie(c);
				أرجع عدم;
			}
		}

		c->مسیر = مسیر;
	}

	/* Write back current position */
	*cookie = cur;

	أرجع c;
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
ساكن بنية مسیر_داده *
urldb_جمع_کن_مسیر(lwc_string *scheme,
	       بیعلامت صحيح پورت,
	       ثابت بنية host_part *host,
	       حرف *مسیر_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	بنية مسیر_داده *d, *e;
	حرف *buf = مسیر_query;
	حرف *segment, *slash;
	bool تطابق;

	أكد(scheme && host && url);

	d = (بنية مسیر_داده *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	إذا (*segment == '/')
		segment++;

	/* Process path segments */
	افعل {
		slash = strchr(segment, '/');
		إذا (!slash) {
			/* last segment */
			/* look for existing entry */
			لكل (e = d->children; e; e = e->بعدی)
				إذا (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &تطابق) ==
				    lwc_error_ok &&
				    تطابق == true &&
				    e->پورت == پورت)
					اكسر;

			d = e ? urldb_جمع_کن_مسیر_fragment(e, fragment) :
				urldb_جمع_کن_مسیر_گره(scheme, پورت,
						    segment, fragment, d);
			اكسر;
		}

		*slash = '\0';

		/* look for existing entry */
		لكل (e = d->children; e; e = e->بعدی)
			إذا (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &تطابق) == lwc_error_ok &&
			    تطابق == true &&
			    e->پورت == پورت)
				اكسر;

		d = e ? e : urldb_جمع_کن_مسیر_گره(scheme, پورت, segment, عدم, d);
		إذا (!d)
			اكسر;

		segment = slash + 1;
	} طالما (1);

	حرر(مسیر_query);

	إذا (d && !d->url) {
		/* Insert defragmented URL */
		إذا (nsurl_defragment(url, &d->url) != NSERROR_OK)
			أرجع عدم;
	}

	أرجع d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ساكن بنية host_part *urldb_جمع_کن_host(ثابت حرف *host)
{
	بنية host_part *d = (بنية host_part *) &db_ریشه, *e;
	بنية جستجو_گره *s;
	حرف buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	حرف *part;

	أكد(host);

	إذا (urldb_host_is_ip_آدرس(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		لكل (e = d->children; e; e = e->بعدی)
			إذا (strcasecmp(host, e->part) == 0)
				/* found => return it */
				أرجع e;

		d = urldb_جمع_کن_host_گره(host, d);

		s = urldb_جستجو_درج(جستجو_trees[ST_IP], d);
		إذا (!s) {
			/* failed */
			d = عدم;
		} وإلا {
			جستجو_trees[ST_IP] = s;
		}

		أرجع d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, حجم buf);
	buf[حجم buf - 1] = '\0';

	/* Process FQDN segments backwards */
	افعل {
		part = strrchr(buf, '.');
		إذا (!part) {
			/* last segment */
			/* Check for existing entry */
			لكل (e = d->children; e; e = e->بعدی)
				إذا (strcasecmp(buf, e->part) == 0)
					اكسر;

			إذا (e) {
				d = e;
			} وإلا {
				d = urldb_جمع_کن_host_گره(buf, d);
			}

			/* And insert into search tree */
			إذا (d) {
				بنية جستجو_گره **r;

				r = urldb_get_جستجو_درخت_مستقیم(buf);
				s = urldb_جستجو_درج(*r, d);
				إذا (!s) {
					/* failed */
					d = عدم;
				} وإلا {
					*r = s;
				}
			}
			اكسر;
		}

		/* Check for existing entry */
		لكل (e = d->children; e; e = e->بعدی)
			إذا (strcasecmp(part + 1, e->part) == 0)
				اكسر;

		d = e ? e : urldb_جمع_کن_host_گره(part + 1, d);
		إذا (!d)
			اكسر;

		*part = '\0';
	} طالما (1);

	أرجع d;
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
urldb_درج_cookie(بنية cookie_داخلی_داده *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	بنية cookie_داخلی_داده *d;
	ثابت بنية host_part *h;
	بنية مسیر_داده *p;
	time_t now = time(عدم);

	أكد(c);

	إذا (c->domain[0] == '.') {
		h = urldb_جستجو_پیدا_کن(
			urldb_get_جستجو_درخت(&(c->domain[1])),
			c->domain + 1);
		إذا (!h) {
			h = urldb_جمع_کن_host(c->domain + 1);
			إذا (!h) {
				urldb_free_cookie(c);
				أرجع false;
			}
		}

		p = (بنية مسیر_داده *) &h->paths;
	} وإلا {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		أكد(url != عدم);
		أكد(scheme != عدم);

		h = urldb_جستجو_پیدا_کن(
			urldb_get_جستجو_درخت(c->domain),
			c->domain);

		إذا (!h) {
			h = urldb_جمع_کن_host(c->domain);
			إذا (!h) {
				urldb_free_cookie(c);
				أرجع false;
			}
		}

		/* find path */
		p = urldb_جمع_کن_مسیر(scheme, 0, h,
				   strdup(c->مسیر), عدم, url);
		إذا (!p) {
			urldb_free_cookie(c);
			أرجع false;
		}
	}

	/* add cookie */
	لكل (d = p->cookies; d; d = d->بعدی) {
		إذا (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->مسیر, c->مسیر) &&
		    !strcmp(d->name, c->name))
			اكسر;
	}

	إذا (d) {
		إذا (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			إذا (d->بعدی)
				d->بعدی->prev = d->prev;
			وإلا
				p->cookies_انتها = d->prev;
			إذا (d->prev)
				d->prev->بعدی = d->بعدی;
			وإلا
				p->cookies = d->بعدی;

			cookie_manager_remove((بنية cookie_داده *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} وإلا {
			/* replace d with c */
			c->prev = d->prev;
			c->بعدی = d->بعدی;
			إذا (c->بعدی)
				c->بعدی->prev = c;
			وإلا
				p->cookies_انتها = c;
			إذا (c->prev)
				c->prev->بعدی = c;
			وإلا
				p->cookies = c;

			cookie_manager_remove((بنية cookie_داده *)d);
			urldb_free_cookie(d);

			cookie_manager_جمع_کن((بنية cookie_داده *)c);
		}
	} وإلا {
		c->prev = p->cookies_انتها;
		c->بعدی = عدم;
		إذا (p->cookies_انتها)
			p->cookies_انتها->بعدی = c;
		وإلا
			p->cookies = c;
		p->cookies_انتها = c;

		cookie_manager_جمع_کن((بنية cookie_داده *)c);
	}

	أرجع true;
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
concat_netscape_cookie(بنية cookie_داخلی_داده *c,
		    صحيح *استفاده_شده,
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

	أكد(c && استفاده_شده && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *استفاده_شده + 2 +
		طول_رشته(c->name) + 1 +
		طول_رشته(c->مقدار) + 2 +
		(c->مسیر_from_مجموعه ? 8 + طول_رشته(c->مسیر) + 2 : 0) +
		(c->domain_from_مجموعه ? 10 + طول_رشته(c->domain) + 2 : 0);
	إذا (max_len >= *alloc) {
		حرف *موقت = realloc(*buf, max_len);
		إذا (!موقت) {
			أرجع false;
		}
		*buf = موقت;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *استفاده_شده - 1, *alloc - *استفاده_شده, "; %s=", c->name);
	إذا (outl >= (*alloc - *استفاده_شده)) {
		/* output truncated */
		أرجع false;
	}
	*استفاده_شده += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	إذا (c->مقدار_was_quoted) {
		outl = snprintf(*buf + *استفاده_شده - 1,
				*alloc - *استفاده_شده,
				"\"%s\"", c->مقدار);
		إذا (outl >= (*alloc - *استفاده_شده)) {
			/* output truncated */
			أرجع false;
		}
		*استفاده_شده += outl;
	} وإلا {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *استفاده_شده - 1,*alloc - *استفاده_شده, "%s", c->مقدار);
		إذا (outl >= (*alloc - *استفاده_شده)) {
			/* output truncated */
			أرجع false;
		}
		*استفاده_شده += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	أرجع true;
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
concat_rfc2965_cookie(بنية cookie_داخلی_داده *c,
		      صحيح *استفاده_شده,
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

	أكد(c && استفاده_شده && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *استفاده_شده + 2 +
		طول_رشته(c->name) + 1 +
		طول_رشته(c->مقدار) + 2 +
		(c->مسیر_from_مجموعه ?
		 8 + طول_رشته(c->مسیر) + 2 : 0) +
		(c->domain_from_مجموعه ?
		 10 + طول_رشته(c->domain) + 2 : 0);

	إذا (max_len >= *alloc) {
		حرف *موقت = realloc(*buf, max_len);
		إذا (!موقت) {
			أرجع false;
		}
		*buf = موقت;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *استفاده_شده - 1,
			*alloc - *استفاده_شده,
			"; %s=", c->name);
	إذا (outl >= (*alloc - *استفاده_شده)) {
		/* output truncated */
		أرجع false;
	}
	*استفاده_شده += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	إذا (c->مقدار_was_quoted ||
	    strpbrk(c->مقدار, separators) != عدم) {
		outl = snprintf(*buf + *استفاده_شده - 1,
				*alloc - *استفاده_شده,
				"\"%s\"", c->مقدار);

	} وإلا {
		outl = snprintf(*buf + *استفاده_شده - 1,
				*alloc - *استفاده_شده,
				"%s", c->مقدار);
	}
	إذا (outl >= (*alloc - *استفاده_شده)) {
		/* output truncated */
		أرجع false;
	}
	*استفاده_شده += outl;

	إذا (c->مسیر_from_مجموعه) {
		/* Path, quoted if necessary */

		إذا (strpbrk(c->مسیر, separators) != عدم) {
			outl = snprintf(*buf + *استفاده_شده - 1,
					*alloc - *استفاده_شده,
					"; $Path=\"%s\"", c->مسیر);

		} وإلا {
			outl = snprintf(*buf + *استفاده_شده - 1,
				       *alloc - *استفاده_شده,
				       "; $Path=%s", c->مسیر);
		}
		إذا (outl >= (*alloc - *استفاده_شده)) {
			/* output truncated */
			أرجع false;
		}
		*استفاده_شده += outl;
	}

	إذا (c->domain_from_مجموعه) {
		/* Domain, quoted if necessary */
		إذا (strpbrk(c->domain, separators) != عدم) {
			outl = snprintf(*buf + *استفاده_شده - 1,
					*alloc - *استفاده_شده,
					"; $Domain=\"%s\"", c->domain);
		} وإلا {
			outl = snprintf(*buf + *استفاده_شده - 1,
					*alloc - *استفاده_شده,
					"; $Domain=%s", c->domain);
		}
		إذا (outl >= (*alloc - *استفاده_شده)) {
			/* output truncated */
			أرجع false;
		}
		*استفاده_شده += outl;
	}

	أرجع true;
}


/**
 * build output cookie string in netscape format
 */
ساكن حرف *
get_netscape_cookie(بنية cookie_داخلی_داده **matched_cookies, صحيح شمار)
{
	صحيح ret_alloc = 4096, ret_استفاده_شده = 1;
	حرف *ret;
	صحيح midx; /* index into matched cookies */

	ret = حافظه_بده(ret_alloc);
	إذا (ret == عدم) {
		أرجع عدم;
	}
	ret[0] = '\0';

	لكل (midx = 0; midx < شمار; midx++) {
		إذا (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_استفاده_شده, &ret_alloc, &ret)) {
			حرر(ret);
			أرجع عدم;
		}
	}

	إذا (ret_استفاده_شده <= 2) {
		حرر(ret);
		أرجع عدم;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_استفاده_شده - 2);
	ret_استفاده_شده -= 2;

	/* shrink the output buffer to the required size */
	حرف *موقت = realloc(ret, ret_استفاده_شده);
	إذا (موقت != عدم) {
		ret = موقت;
	}

	أرجع ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ساكن حرف *
get_rfc2965_cookie(بنية cookie_داخلی_داده **matched_cookies,
		   صحيح شمار,
		   صحيح نسخه)
{
	صحيح ret_alloc = 4096; /* return string allocation */
	صحيح ret_استفاده_شده = 1; /* return sting buffer used */
	حرف *ret; /* return string buffer */
	صحيح midx; /* index into matched cookies */
	صحيح outl;

	ret = حافظه_بده(ret_alloc);
	إذا (ret == عدم) {
		أرجع عدم;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_استفاده_شده, "$Version=%d", نسخه);
	إذا (outl >= ret_alloc - ret_استفاده_شده) {
		/*سرریز*/
		حرر(ret);
		أرجع عدم;
	}
	ret_استفاده_شده += outl;

	لكل (midx = 0; midx < شمار; midx++) {
		إذا (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_استفاده_شده, &ret_alloc, &ret)) {
			حرر(ret);
			أرجع عدم;
		}
	}

	/* shrink the output buffer to the required size */
	حرف *موقت = realloc(ret, ret_استفاده_شده);
	إذا (موقت != عدم) {
		ret = موقت;
	}

	أرجع ret;
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
			  ثابت حرف *مسیر,
			  ثابت حرف *name,
			  بنية مسیر_داده *والد)
{
	بنية cookie_داخلی_داده *c;
	بنية مسیر_داده *p = والد;

	أكد(والد);

	افعل {
		لكل (c = p->cookies; c; c = c->بعدی) {
			إذا (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->مسیر, مسیر) == 0 &&
			    strcmp(c->name, name) == 0) {
				إذا (c->prev) {
					c->prev->بعدی = c->بعدی;
				} وإلا {
					p->cookies = c->بعدی;
				}

				إذا (c->بعدی) {
					c->بعدی->prev = c->prev;
				} وإلا {
					p->cookies_انتها = c->prev;
				}

				urldb_free_cookie(c);

				أرجع;
			}
		}

		إذا (p->children) {
			p = p->children;
		} وإلا {
			طالما (p != والد) {
				إذا (p->بعدی != عدم) {
					p = p->بعدی;
					اكسر;
				}

				p = p->والد;
			}
		}
	} طالما (p != والد);
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
			  ثابت حرف *مسیر,
			  ثابت حرف *name,
			  بنية host_part *والد)
{
	بنية host_part *h;
	أكد(والد);

	urldb_delete_cookie_paths(domain, مسیر, name, &والد->paths);

	لكل (h = والد->children; h; h = h->بعدی) {
		urldb_delete_cookie_hosts(domain, مسیر, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ساكن فراغ urldb_save_cookie_paths(FILE *fp, بنية مسیر_داده *والد)
{
	بنية مسیر_داده *p = والد;
	time_t now = time(عدم);

	أكد(fp && والد);

	افعل {
		إذا (p->cookies != عدم) {
			بنية cookie_داخلی_داده *c;

			لكل (c = p->cookies; c != عدم; c = c->بعدی) {
				إذا (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					استمر;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->نسخه, c->domain,
					c->domain_from_مجموعه, c->مسیر,
					c->مسیر_from_مجموعه, c->secure,
					c->http_only,
					(صحيح)c->expires, (صحيح)c->آخرین_استفاده_شده,
					c->no_destroy, c->name, c->مقدار,
					c->مقدار_was_quoted,
					p->scheme ? lwc_string_داده(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		إذا (p->children != عدم) {
			p = p->children;
		} وإلا {
			طالما (p != والد) {
				إذا (p->بعدی != عدم) {
					p = p->بعدی;
					اكسر;
				}

				p = p->والد;
			}
		}
	} طالما (p != والد);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ساكن فراغ urldb_save_cookie_hosts(FILE *fp, بنية host_part *والد)
{
	بنية host_part *h;
	أكد(fp && والد);

	urldb_save_cookie_paths(fp, &والد->paths);

	لكل (h = والد->children; h; h = h->بعدی)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ساكن فراغ urldb_destroy_cookie(بنية cookie_داخلی_داده *c)
{
	حرر(c->name);
	حرر(c->مقدار);
	حرر(c->comment);
	حرر(c->domain);
	حرر(c->مسیر);

	حرر(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ساكن فراغ urldb_destroy_مسیر_گره_content(بنية مسیر_داده *گره)
{
	بنية cookie_داخلی_داده *a, *b;
	بیعلامت صحيح i;

	إذا (گره->url != عدم) {
		nsurl_unref(گره->url);
	}

	lwc_string_unref(گره->scheme);

	حرر(گره->segment);
	لكل (i = 0; i < گره->frag_cnt; i++)
		حرر(گره->fragment[i]);
	حرر(گره->fragment);

	حرر(گره->urld.title);

	لكل (a = گره->cookies; a; a = b) {
		b = a->بعدی;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ساكن فراغ urldb_destroy_prot_space(بنية prot_space_داده *space)
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
ساكن فراغ urldb_destroy_مسیر_درخت(بنية مسیر_داده *ریشه)
{
	بنية مسیر_داده *p = ریشه;

	افعل {
		إذا (p->children != عدم) {
			p = p->children;
		} وإلا {
			بنية مسیر_داده *q = p;

			طالما (p != ریشه) {
				إذا (p->بعدی != عدم) {
					p = p->بعدی;
					اكسر;
				}

				p = p->والد;

				urldb_destroy_مسیر_گره_content(q);
				حرر(q);

				q = p;
			}

			urldb_destroy_مسیر_گره_content(q);
			حرر(q);
		}
	} طالما (p != ریشه);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ساكن فراغ urldb_destroy_host_درخت(بنية host_part *ریشه)
{
	بنية host_part *a, *b;
	بنية مسیر_داده *p, *q;
	بنية prot_space_داده *s, *t;

	/* Destroy children */
	لكل (a = ریشه->children; a; a = b) {
		b = a->بعدی;
		urldb_destroy_host_درخت(a);
	}

	/* Now clean up paths */
	لكل (p = ریشه->paths.children; p; p = q) {
		q = p->بعدی;
		urldb_destroy_مسیر_درخت(p);
	}

	/* Root path */
	urldb_destroy_مسیر_گره_content(&ریشه->paths);

	/* Proctection space data */
	لكل (s = ریشه->prot_space; s; s = t) {
		t = s->بعدی;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	حرر(ریشه->part);
	حرر(ریشه);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ساكن فراغ urldb_destroy_جستجو_درخت(بنية جستجو_گره *ریشه)
{
	/* Destroy children */
	إذا (ریشه->چپ != &خالی)
		urldb_destroy_جستجو_درخت(ریشه->چپ);
	إذا (ریشه->راست != &خالی)
		urldb_destroy_جستجو_درخت(ریشه->راست);

	/* And destroy ourselves */
	حرر(ریشه);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
فراغ urldb_destroy(فراغ)
{
	بنية host_part *a, *b;
	صحيح i;

	/* Clean up search trees */
	لكل (i = 0; i < رقم_جستجو_TREES; i++) {
		إذا (جستجو_trees[i] != &خالی) {
			urldb_destroy_جستجو_درخت(جستجو_trees[i]);
			جستجو_trees[i] = &خالی;
		}
	}

	/* And database */
	لكل (a = db_ریشه.children; a; a = b) {
		b = a->بعدی;
		urldb_destroy_host_درخت(a);
	}
	memset(&db_ریشه, 0, حجم(db_ریشه));

	/* And the bloom filter */
	إذا (url_bloom != عدم) {
		bloom_destroy(url_bloom);
		url_bloom = عدم;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_بارگیری(ثابت حرف *filename)
{
#تعریف_کن بیشینه_URL_درازی 4096
	حرف s[بیشینه_URL_درازی];
	حرف host[256];
	بنية host_part *h;
	صحيح urls;
	صحيح i;
	صحيح نسخه;
	صحيح درازی;
	FILE *fp;

	أكد(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	إذا (url_bloom == عدم)
		url_bloom = bloom_create(BLOOM_اندازه);

	fp = fopen(filename, "r");
	إذا (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		أرجع NSERROR_NOT_یافت;
	}

	إذا (!fgets(s, بیشینه_URL_درازی, fp)) {
		fclose(fp);
		أرجع NSERROR_NEED_داده;
	}

	نسخه = atoi(s);
	إذا (نسخه < MIN_URL_FILE_نسخه) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		أرجع NSERROR_INVALID;
	}
	إذا (نسخه > URL_FILE_نسخه) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		أرجع NSERROR_INVALID;
	}

	طالما (fgets(host, حجم host, fp)) {
		time_t hsts_expiry = 0;
		صحيح hsts_include_sub_domains = 0;

		/* get the hostname */
		درازی = طول_رشته(host) - 1;
		host[درازی] = '\0';

		/* skip data that has ended up with a host of '' */
		إذا (درازی == 0) {
			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;
			urls = atoi(s);
			/* Eight fields/url */
			لكل (i = 0; i < (8 * urls); i++) {
				إذا (!fgets(s, بیشینه_URL_درازی, fp))
					اكسر;
			}
			استمر;
		}

		إذا (نسخه >= 107) {
			حرف *p = host;
			طالما (*p && *p != ' ') p++;
			طالما (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			طالما (*p && *p != ' ') p++;
			طالما (*p && *p == ' ') p++;
			nsc_snptimet(p, طول_رشته(p), &hsts_expiry);
		}

		h = urldb_جمع_کن_host(host);
		إذا (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			أرجع NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		إذا (!fgets(s, بیشینه_URL_درازی, fp))
			اكسر;
		urls = atoi(s);

		/* no URLs => try next host */
		إذا (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			استمر;
		}

		/* load the non-corrupt data */
		لكل (i = 0; i < urls; i++) {
			بنية مسیر_داده *p = عدم;
			حرف scheme[64], ports[10];
			حرف url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			بیعلامت صحيح پورت;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			حرف *مسیر_query;
			size_t len;

			إذا (!fgets(scheme, حجم scheme, fp))
				اكسر;
			درازی = طول_رشته(scheme) - 1;
			scheme[درازی] = '\0';

			إذا (!fgets(ports, حجم ports, fp))
				اكسر;
			درازی = طول_رشته(ports) - 1;
			ports[درازی] = '\0';
			پورت = atoi(ports);

			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;
			درازی = طول_رشته(s) - 1;
			s[درازی] = '\0';

			إذا (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, حجم url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (پورت ? ":" : ""),
				 (پورت ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			إذا (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				أرجع NSERROR_NOMEM;
			}

			إذا (url_bloom != عدم) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_درج_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			إذا (nsurl_get(nsurl, NSURL_مسیر | NSURL_QUERY,
				      &مسیر_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				أرجع NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_جمع_کن_مسیر(scheme_lwc, پورت, h, مسیر_query,
					   fragment_lwc, nsurl);
			إذا (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				أرجع NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;
			إذا (p)
				p->urld.visits = (بیعلامت صحيح)atoi(s);

			/* entry last use time */
			إذا (!fgets(s, بیشینه_URL_درازی, fp)) {
				اكسر;
			}
			إذا (p) {
				nsc_snptimet(s, طول_رشته(s) - 1, &p->urld.آخرین_بازدید);
			}

			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;
			إذا (p)
				p->urld.type = (content_type)atoi(s);

			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;


			إذا (!fgets(s, بیشینه_URL_درازی, fp))
				اكسر;
			درازی = طول_رشته(s) - 1;
			إذا (p && درازی > 0) {
				s[درازی] = '\0';
				p->urld.title = حافظه_بده(درازی + 1);
				إذا (p->urld.title)
					کپی_حافظه(p->urld.title, s, درازی + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#لغو_تعریف بیشینه_URL_درازی

	أرجع NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ثابت حرف *filename)
{
	FILE *fp;
	صحيح i;

	أكد(filename);

	fp = fopen(filename, "w");
	إذا (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		أرجع NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_نسخه);

	لكل (i = 0; i != رقم_جستجو_TREES; i++) {
		urldb_save_جستجو_درخت(جستجو_trees[i], fp);
	}

	fclose(fp);

	أرجع NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_مجموعه_url_persistence(nsurl *url, bool persist)
{
	بنية مسیر_داده *p;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p) {
		أرجع NSERROR_NOT_یافت;
	}

	p->persistent = persist;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_جمع_کن_url(nsurl *url)
{
	بنية host_part *h;
	بنية مسیر_داده *p;
	lwc_string *scheme;
	lwc_string *پورت;
	lwc_string *host;
	lwc_string *fragment;
	ثابت حرف *host_str;
	حرف *مسیر_query = عدم;
	size_t len;
	bool تطابق;
	بیعلامت صحيح پورت_int;

	أكد(url);

	إذا (url_bloom == عدم)
		url_bloom = bloom_create(BLOOM_اندازه);

	إذا (url_bloom != عدم) {
		uint32_t hash = nsurl_hash(url);
		bloom_درج_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	إذا (nsurl_get(url, NSURL_مسیر | NSURL_QUERY, &مسیر_query, &len) !=
	    NSERROR_OK) {
		أرجع false;
	}
	أكد(مسیر_query != عدم);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	إذا (scheme == عدم) {
		حرر(مسیر_query);
		أرجع false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	إذا (host != عدم) {
		host_str = lwc_string_داده(host);
		lwc_string_unref(host);

	} وإلا إذا (lwc_string_isequal(scheme, corestring_lwc_file, &تطابق) ==
		   lwc_error_ok && تطابق == true) {
		host_str = "localhost";

	} وإلا {
		lwc_string_unref(scheme);
		حرر(مسیر_query);
		أرجع false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	پورت = nsurl_get_component(url, NSURL_پورت);
	إذا (پورت != عدم) {
		پورت_int = atoi(lwc_string_داده(پورت));
		lwc_string_unref(پورت);
	} وإلا {
		پورت_int = 0;
	}

	/* Get host entry */
	h = urldb_جمع_کن_host(host_str);

	/* Get path entry */
	إذا (h != عدم) {
		p = urldb_جمع_کن_مسیر(scheme,
				   پورت_int,
				   h,
				   مسیر_query,
				   fragment,
				   url);
	} وإلا {
		p = عدم;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	أرجع (p != عدم);
}


/* exported interface documented in content/urldb.h */
nserror urldb_مجموعه_url_title(nsurl *url, ثابت حرف *title)
{
	بنية مسیر_داده *p;
	حرف *موقت;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (p == عدم) {
		أرجع NSERROR_NOT_یافت;
	}

	/* copy the parameter if necessary */
	إذا (title != عدم) {
		موقت = strdup(title);
		إذا (موقت == عدم) {
			أرجع NSERROR_NOMEM;
		}
	} وإلا {
		موقت = عدم;
	}

	حرر(p->urld.title);
	p->urld.title = موقت;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_مجموعه_url_content_type(nsurl *url, content_type type)
{
	بنية مسیر_داده *p;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p) {
		أرجع NSERROR_NOT_یافت;
	}

	p->urld.type = type;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_بازدید_داده(nsurl *url)
{
	بنية مسیر_داده *p;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p) {
		أرجع NSERROR_NOT_یافت;
	}

	p->urld.آخرین_بازدید = time(عدم);
	p->urld.visits++;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
فراغ urldb_reset_url_بازدید_داده(nsurl *url)
{
	بنية مسیر_داده *p;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع;

	p->urld.آخرین_بازدید = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ثابت بنية url_داده *urldb_get_url_داده(nsurl *url)
{
	بنية مسیر_داده *p;
	بنية url_داخلی_داده *u;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع عدم;

	u = &p->urld;

	أرجع (ثابت بنية url_داده *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	بنية مسیر_داده *p;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع عدم;

	أرجع p->url;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_مجموعه_auth_details(nsurl *url, ثابت حرف *realm, ثابت حرف *auth)
{
	بنية مسیر_داده *p, *pi;
	بنية host_part *h;
	بنية prot_space_داده *space, *space_alloc;
	حرف *realm_alloc, *auth_alloc;
	bool تطابق;

	أكد(url && realm && auth);

	/* add url, in case it's missing */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);

	إذا (!p)
		أرجع;

	/* Search for host_part */
	لكل (pi = p; pi->والد != عدم; pi = pi->والد)
		;
	h = (بنية host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	لكل (space = h->prot_space; space; space = space->بعدی) {
		إذا (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &تطابق) == lwc_error_ok &&
		    تطابق == true &&
		    space->پورت == p->پورت)
			اكسر;
	}

	إذا (space != عدم) {
		/* Overrule existing auth. */
		حرر(space->auth);
		space->auth = strdup(auth);
	} وإلا {
		/* Create a new protection space. */
		space = space_alloc = حافظه_بده(حجم(بنية prot_space_داده));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		إذا (!space_alloc || !realm_alloc || !auth_alloc) {
			حرر(space_alloc);
			حرر(realm_alloc);
			حرر(auth_alloc);
			أرجع;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->پورت = p->پورت;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->بعدی = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ثابت حرف *urldb_get_auth_details(nsurl *url, ثابت حرف *realm)
{
	بنية مسیر_داده *p, *p_cur, *p_بالاترین;

	أكد(url);

	/* add to the db, so our lookup will work */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع عدم;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	لكل (p_cur = p; p_cur != عدم; p_بالاترین = p_cur, p_cur = p_cur->والد) {
		إذا (p_cur->prot_space) {
			أرجع p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	إذا (realm != عدم) {
		ثابت بنية host_part *h = (ثابت بنية host_part *)p_بالاترین;
		ثابت بنية prot_space_داده *space;
		bool تطابق;

		/* Search for a possible matching protection space. */
		لكل (space = h->prot_space; space != عدم;
		     space = space->بعدی) {
			إذا (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &تطابق) ==
			    lwc_error_ok &&
			    تطابق == true &&
			    space->پورت == p->پورت) {
				p->prot_space = space;
				أرجع p->prot_space->auth;
			}
		}
	}

	أرجع عدم;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_مجموعه_cert_permissions(nsurl *url, bool permit)
{
	بنية مسیر_داده *p;
	بنية host_part *h;

	أكد(url);

	/* add url, in case it's missing */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع;

	لكل (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (بنية host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	بنية مسیر_داده *p;
	ثابت بنية host_part *h;

	أكد(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع false;

	لكل (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ثابت بنية host_part *)p;

	أرجع h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_مجموعه_hsts_policy(بنية nsurl *url, ثابت حرف *سرآیند)
{
	بنية مسیر_داده *p;
	بنية host_part *h;
	lwc_string *host;
	time_t now = time(عدم);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	إذا (host != عدم) {
		إذا (urldb_host_is_ip_آدرس(lwc_string_داده(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			أرجع true;
		} وإلا إذا (lwc_string_درازی(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			أرجع true;
		}

		lwc_string_unref(host);
	} وإلا {
		/* No host part: ignore */
		أرجع true;
	}

	/* add url, in case it's missing */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع false;

	لكل (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (بنية host_part *)p;
	إذا (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		أرجع true;
	}

	error = http_parse_strict_transport_security(سرآیند, &sts);
	إذا (error != NSERROR_OK) {
		/* Parse failed: ignore */
		أرجع true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	إذا (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} وإلا إذا ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	أرجع true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(بنية nsurl *url)
{
	بنية مسیر_داده *p;
	ثابت بنية host_part *h;
	lwc_string *host;
	time_t now = time(عدم);

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	إذا (host != عدم) {
		إذا (urldb_host_is_ip_آدرس(lwc_string_داده(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			أرجع false;
		} وإلا إذا (lwc_string_درازی(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			أرجع false;
		}

		lwc_string_unref(host);
	} وإلا {
		/* No host part: not enabled */
		أرجع false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع false;

	لكل (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ثابت بنية host_part *)p;

	/* Consult record for this host */
	إذا (h->hsts.expires > now) {
		/* Not expired */
		أرجع true;
	}

	/* Consult parent domains */
	لكل (h = h->والد; h && h != &db_ریشه; h = h->والد) {
		إذا (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			أرجع true;
		}
	}

	أرجع false;
}


/* exported interface documented in netsurf/url_db.h */
فراغ
urldb_iterate_partial(ثابت حرف *پیشوند,
		      bool (*callback)(nsurl *url, ثابت بنية url_داده *داده))
{
	حرف host[256];
	حرف buf[260]; /* max domain + "www." */
	ثابت حرف *slash, *scheme_sep;
	بنية جستجو_گره *درخت;
	ثابت بنية host_part *h;

	أكد(پیشوند && callback);

	/* strip scheme */
	scheme_sep = جستجوی_رشته(پیشوند, "://");
	إذا (scheme_sep)
		پیشوند = scheme_sep + 3;

	slash = strchr(پیشوند, '/');
	درخت = urldb_get_جستجو_درخت(پیشوند);

	إذا (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, حجم host, "%.*s",
			 (صحيح) (slash - پیشوند), پیشوند);

		h = urldb_جستجو_پیدا_کن(درخت, host);
		إذا (!h) {
			صحيح len = slash - پیشوند;

			إذا (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, حجم buf, "www.%s", host);
				h = urldb_جستجو_پیدا_کن(
					جستجو_trees[ST_DN + 'w' - 'a'],
					buf);
				إذا (!h)
					أرجع;
			} وإلا
				أرجع;
		}

		إذا (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_مسیر(&h->paths, slash + 1,
						   callback);
		}

	} وإلا {
		صحيح len = طول_رشته(پیشوند);

		/* looking for hosts */
		إذا (!urldb_iterate_partial_host(درخت, پیشوند, callback))
			أرجع;

		إذا (len <= 3 || strncasecmp(پیشوند, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, حجم buf, "www.%s", پیشوند);
			إذا(!urldb_iterate_partial_host(
				   جستجو_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				أرجع;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
فراغ
urldb_iterate_entries(bool (*callback)(nsurl *url, ثابت بنية url_داده *داده))
{
	صحيح i;

	أكد(callback);

	لكل (i = 0; i < رقم_جستجو_TREES; i++) {
		إذا (!urldb_iterate_entries_host(جستجو_trees[i],
						callback,
						عدم)) {
			اكسر;
		}
	}
}


/* exported interface documented in content/urldb.h */
فراغ urldb_iterate_cookies(bool (*callback)(ثابت بنية cookie_داده *داده))
{
	صحيح i;

	أكد(callback);

	لكل (i = 0; i < رقم_جستجو_TREES; i++) {
		إذا (!urldb_iterate_entries_host(جستجو_trees[i],
						عدم, callback))
			اكسر;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_مجموعه_cookie(ثابت حرف *سرآیند, nsurl *url, nsurl *referer)
{
	ثابت حرف *cur = سرآیند, *انتها;
	lwc_string *مسیر, *host, *scheme;
	nsurl *urlt;
	bool تطابق;

	أكد(url && سرآیند);

	/* Get defragmented URL, as 'urlt' */
	إذا (nsurl_defragment(url, &urlt) != NSERROR_OK)
		أرجع عدم;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	إذا (scheme == عدم) {
		nsurl_unref(urlt);
		أرجع false;
	}

	مسیر = nsurl_get_component(url, NSURL_مسیر);
	إذا (مسیر == عدم) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		أرجع false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	إذا (host == عدم) {
		lwc_string_unref(مسیر);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		أرجع false;
	}

	إذا (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		إذا (rhost == عدم) {
			اذهب error;
		}

		/* Domain match host names */
		إذا (lwc_string_isequal(host, rhost, &تطابق) == lwc_error_ok &&
		    تطابق == false) {
			ثابت حرف *hptr;
			ثابت حرف *rptr;
			ثابت حرف *dot;
			ثابت حرف *host_داده = lwc_string_داده(host);
			ثابت حرف *rhost_داده = lwc_string_داده(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			إذا (urldb_host_is_ip_آدرس(host_داده) ||
			    urldb_host_is_ip_آدرس(rhost_داده)) {
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

			hptr = host_داده + lwc_string_درازی(host) - 1;
			rptr = rhost_داده + lwc_string_درازی(rhost) - 1;

			/* 1 */
			طالما (hptr >= host_داده && rptr >= rhost_داده) {
				إذا (*hptr != *rptr)
					اكسر;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			طالما (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			إذا (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == عدم ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				اذهب error;
			}
		}

		lwc_string_unref(rhost);
	}

	انتها = cur + طول_رشته(cur) - 2 /* Trailing CRLF */;

	افعل {
		بنية cookie_داخلی_داده *c;
		حرف *dot;
		size_t len;
#اگر_تعریف_شده WITH_NSPSL
		ثابت حرف *suffix;
#پایان_اگر

		c = urldb_parse_cookie(url, &cur);
		إذا (!c) {
			/* failed => stop parsing */
			اذهب error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		إذا (!c->name || !c->مقدار) {
			urldb_free_cookie(c);
			اذهب error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = طول_رشته(c->مسیر);
		إذا (len > lwc_string_درازی(مسیر) ||
		    مقایسه_رشته(c->مسیر, lwc_string_داده(مسیر),
			    len) != 0) {
			urldb_free_cookie(c);
			اذهب error;
		}

#اگر_تعریف_شده WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		إذا (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		إذا (suffix == عدم) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			اذهب error;
		}
#وإلا
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		إذا (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			اذهب error;
		}
#پایان_اگر

		/* Domain match fetch host with cookie domain */
		إذا (strcasecmp(lwc_string_داده(host), c->domain) != 0) {
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
			إذا (urldb_host_is_ip_آدرس(lwc_string_داده(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				اذهب error;
			}

			hlen = lwc_string_درازی(host);
			dlen = طول_رشته(c->domain);

			إذا (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				اذهب error;
			}

			إذا (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			إذا (strcasecmp(lwc_string_داده(host) + (hlen - dlen),
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
		إذا (!urldb_درج_cookie(c, scheme, urlt))
			اذهب error;
	} طالما (cur < انتها);

	lwc_string_unref(host);
	lwc_string_unref(مسیر);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	أرجع true;

error:
	lwc_string_unref(host);
	lwc_string_unref(مسیر);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	أرجع false;
}


/* exported interface documented in content/urldb.h */
حرف *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ثابت بنية مسیر_داده *p, *q;
	ثابت بنية host_part *h;
	lwc_string *مسیر_lwc;
	بنية cookie_داخلی_داده *c;
	صحيح شمار = 0, نسخه = COOKIE_RFC2965;
	بنية cookie_داخلی_داده **matched_cookies;
	صحيح matched_cookies_اندازه = 20;
	ثابت حرف *مسیر;
	حرف *ret;
	lwc_string *scheme;
	time_t now;
	bool تطابق;

	أكد(url != عدم);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_جمع_کن_url(url);

	p = urldb_پیدا_کن_url(url);
	إذا (!p)
		أرجع عدم;

	scheme = p->scheme;

	matched_cookies = حافظه_بده(matched_cookies_اندازه *
				 حجم(بنية cookie_داخلی_داده *));
	إذا (!matched_cookies)
		أرجع عدم;

#تعریف_کن GROW_MATCHED_COOKIES						\
	افعل {								\
		إذا (شمار == matched_cookies_اندازه) {			\
			بنية cookie_داخلی_داده **موقت;		\
			موقت = realloc(matched_cookies,			\
				       (matched_cookies_اندازه + 20) *	\
				       حجم(بنية cookie_داخلی_داده *)); \
									\
			إذا (موقت == عدم) {				\
				حرر(matched_cookies);			\
				أرجع عدم;				\
			}						\
									\
			matched_cookies = موقت;				\
			matched_cookies_اندازه += 20;			\
		}							\
	} طالما(0)

	مسیر_lwc = nsurl_get_component(url, NSURL_مسیر);
	إذا (مسیر_lwc == عدم) {
		حرر(matched_cookies);
		أرجع عدم;
	}
	مسیر = lwc_string_داده(مسیر_lwc);
	lwc_string_unref(مسیر_lwc);

	now = time(عدم);

	إذا (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		لكل (q = p->والد->children; q; q = q->بعدی) {
			إذا (strcmp(q->segment, p->segment))
				استمر;

			/* Consider all cookies associated with
			 * this exact path */
			لكل (c = q->cookies; c; c = c->بعدی) {
				إذا (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					استمر;

				إذا (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &تطابق) &&
				    تطابق == false)
					/* secure cookie for insecure host.
					 * ignore */
					استمر;

				إذا (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					استمر;

				matched_cookies[شمار++] = c;

				GROW_MATCHED_COOKIES;

				إذا (c->نسخه < (بیعلامت صحيح)نسخه)
					نسخه = c->نسخه;

				c->آخرین_استفاده_شده = now;

				cookie_manager_جمع_کن((بنية cookie_داده *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	لكل (p = p->والد; p; p = p->والد) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		لكل (q = p->children; q; q = q->بعدی) {
			إذا (*(q->segment) != '\0')
				استمر;

			لكل (c = q->cookies; c; c = c->بعدی) {
				إذا (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					استمر;

				إذا (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &تطابق) &&
				    تطابق == false)
					/* Secure cookie for insecure server
					 * => ignore */
					استمر;

				matched_cookies[شمار++] = c;

				GROW_MATCHED_COOKIES;

				إذا (c->نسخه < (بیعلامت صحيح) نسخه)
					نسخه = c->نسخه;

				c->آخرین_استفاده_شده = now;

				cookie_manager_جمع_کن((بنية cookie_داده *)c);
			}
		}

		إذا (!p->والد) {
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
		لكل (c = p->cookies; c; c = c->بعدی) {
			إذا (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				استمر;

			/* Ensure cookie path is a prefix of the resource */
			إذا (مقایسه_رشته(c->مسیر, مسیر, طول_رشته(c->مسیر)) != 0)
				/* paths don't match => ignore */
				استمر;

			إذا (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &تطابق) &&
			    تطابق == false)
				/* Secure cookie for insecure server
				 * => ignore */
				استمر;

			matched_cookies[شمار++] = c;

			GROW_MATCHED_COOKIES;

			إذا (c->نسخه < (بیعلامت صحيح) نسخه)
				نسخه = c->نسخه;

			c->آخرین_استفاده_شده = now;

			cookie_manager_جمع_کن((بنية cookie_داده *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	لكل (h = (ثابت بنية host_part *)p; h && h != &db_ریشه;
	     h = h->والد) {
		لكل (c = h->paths.cookies; c; c = c->بعدی) {
			إذا (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				استمر;

			/* Ensure cookie path is a prefix of the resource */
			إذا (مقایسه_رشته(c->مسیر, مسیر, طول_رشته(c->مسیر)) != 0)
				/* paths don't match => ignore */
				استمر;

			إذا (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &تطابق) &&
			    تطابق == false)
				/* secure cookie for insecure host. ignore */
				استمر;

			matched_cookies[شمار++] = c;

			GROW_MATCHED_COOKIES;

			إذا (c->نسخه < (بیعلامت صحيح)نسخه)
				نسخه = c->نسخه;

			c->آخرین_استفاده_شده = now;

			cookie_manager_جمع_کن((بنية cookie_داده *)c);
		}
	}
#لغو_تعریف GROW_MATCHED_COOKIES

	إذا (شمار == 0) {
		/* No cookies found */
		حرر(matched_cookies);
		أرجع عدم;
	}

	إذا (نسخه == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, شمار);
	} وإلا {
		ret = get_rfc2965_cookie(matched_cookies, شمار, نسخه);
	}
	حرر(matched_cookies);

	أرجع ret;
}


/* exported interface documented in content/urldb.h */
فراغ urldb_delete_cookie(ثابت حرف *domain, ثابت حرف *مسیر,
			 ثابت حرف *name)
{
	urldb_delete_cookie_hosts(domain, مسیر, name, &db_ریشه);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_بارگیری_cookies(ثابت حرف *filename)
{
	FILE *fp;
	حرف s[16*1024];

	أكد(filename);

	fp = fopen(filename, "r");
	إذا (!fp)
		أرجع NSERROR_NOT_یافت;

#تعریف_کن پیدا_کن_T {				\
		لكل (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		إذا (p >= انتها) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			استمر;		\
		}				\
		*p++ = '\0';			\
	}

#تعریف_کن SKIP_T {				\
		لكل (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		إذا (p >= انتها) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			استمر;		\
		}				\
	}

	طالما (fgets(s, حجم s, fp)) {
		حرف *p = s, *انتها = 0,
			*domain, *مسیر, *name, *مقدار, *scheme, *url,
			*comment;
		صحيح نسخه, domain_specified, مسیر_specified,
			secure, http_only, no_destroy, مقدار_quoted;
		time_t expires, آخرین_استفاده_شده;
		بنية cookie_داخلی_داده *c;

		إذا(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			استمر;

		s[طول_رشته(s) - 1] = '\0'; /* lose terminating newline */
		انتها = s + طول_رشته(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		إذا (strncasecmp(s, "Version:", 8) == 0) {
			پیدا_کن_T; SKIP_T; loaded_cookie_file_نسخه = atoi(p);

			إذا (loaded_cookie_file_نسخه <
			    MIN_COOKIE_FILE_نسخه) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				اكسر;
			}

			استمر;
		} وإلا إذا (loaded_cookie_file_نسخه == 0) {
			/* Haven't yet seen version; skip this input */
			استمر;
		}

		/* One cookie/line */

		/* Parse input */
		پیدا_کن_T; نسخه = atoi(s);
		SKIP_T; domain = p; پیدا_کن_T;
		SKIP_T; domain_specified = atoi(p); پیدا_کن_T;
		SKIP_T; مسیر = p; پیدا_کن_T;
		SKIP_T; مسیر_specified = atoi(p); پیدا_کن_T;
		SKIP_T; secure = atoi(p); پیدا_کن_T;
		إذا (loaded_cookie_file_نسخه > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); پیدا_کن_T;
		} وإلا {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); پیدا_کن_T;
		SKIP_T; آخرین_استفاده_شده = (time_t)atoi(p); پیدا_کن_T;
		SKIP_T; no_destroy = atoi(p); پیدا_کن_T;
		SKIP_T; name = p; پیدا_کن_T;
		SKIP_T; مقدار = p; پیدا_کن_T;
		إذا (loaded_cookie_file_نسخه > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	مقدار_quoted = atoi(p); پیدا_کن_T;
		} وإلا {
			مقدار_quoted = 0;
		}
		SKIP_T; scheme = p; پیدا_کن_T;
		SKIP_T; url = p; پیدا_کن_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		لكل (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		أكد(p <= انتها);

		/* Now create cookie */
		c = حافظه_بده(حجم(بنية cookie_داخلی_داده));
		إذا (!c)
			اكسر;

		c->name = strdup(name);
		c->مقدار = strdup(مقدار);
		c->مقدار_was_quoted = مقدار_quoted;
		c->comment = strdup(comment);
		c->domain_from_مجموعه = domain_specified;
		c->domain = strdup(domain);
		c->مسیر_from_مجموعه = مسیر_specified;
		c->مسیر = strdup(مسیر);
		c->expires = expires;
		c->آخرین_استفاده_شده = آخرین_استفاده_شده;
		c->secure = secure;
		c->http_only = http_only;
		c->نسخه = نسخه;
		c->no_destroy = no_destroy;

		إذا (!(c->name && c->مقدار && c->comment &&
		      c->domain && c->مسیر)) {
			urldb_free_cookie(c);
			اكسر;
		}

		إذا (c->domain[0] != '.') {
			lwc_string *scheme_lwc = عدم;
			nsurl *url_nsurl = عدم;

			أكد(scheme[0] != 'u');

			إذا (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				اكسر;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			إذا (!urldb_درج_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				اكسر;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} وإلا {
			إذا (!urldb_درج_cookie(c, عدم, عدم)) {
				/* Cookie freed for us */
				اكسر;
			}
		}
	}

#لغو_تعریف SKIP_T
#لغو_تعریف پیدا_کن_T

	fclose(fp);

	أرجع NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ثابت حرف *filename)
{
	FILE *fp;
	صحيح cookie_file_نسخه = max(loaded_cookie_file_نسخه,
				      COOKIE_FILE_نسخه);

	أكد(filename);

	fp = fopen(filename, "w");
	إذا (!fp)
		أرجع NSERROR_NOT_یافت;

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
		cookie_file_نسخه);
	fprintf(fp, "Version:\t%d\n", cookie_file_نسخه);

	urldb_save_cookie_hosts(fp, &db_ریشه);

	fclose(fp);

	أرجع NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
فراغ urldb_dump(فراغ)
{
	صحيح i;

	urldb_dump_hosts(&db_ریشه);

	لكل (i = 0; i != رقم_جستجو_TREES; i++) {
		urldb_dump_جستجو(جستجو_trees[i], 0);
	}
}
