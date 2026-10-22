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
#اگر_تعریف_شدہ WITH_NSPSL
#شامل <nspsl.h>
#اگر_ختم

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
ساخت cookie_اندرونی_ڈیٹا {
	ساخت cookie_اندرونی_ڈیٹا *پچھلا;	/**< Previous in list */
	ساخت cookie_اندرونی_ڈیٹا *اگلا;	/**< Next in list */

	حرف *name;		/**< Cookie name */
	حرف *قدر;		/**< Cookie value */
	bool قدر_was_quoted;	/**< Value was quoted in Set-Cookie: */
	حرف *comment;		/**< Cookie comment */
	bool domain_from_سیٹ;	/**< Domain came from Set-Cookie: header */
	حرف *domain;		/**< Domain */
	bool راستہ_from_سیٹ;	/**< Path came from Set-Cookie: header */
	حرف *راستہ;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t آخری_مستعمل;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	شمار cookie_نسخہ نسخہ;	/**< Specification compliance */
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
ساخت prot_space_ڈیٹا {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	غیرعلامتی صحیح پورٹ;
	/** Protection realm */
	حرف *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	حرف *auth;
	/** Next sibling */
	ساخت prot_space_ڈیٹا *اگلا;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
ساخت url_اندرونی_ڈیٹا {
	حرف *title;		/**< Resource title */
	غیرعلامتی صحیح visits;	/**< Visit count */
	time_t آخری_دورہ;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
ساخت راستہ_ڈیٹا {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	غیرعلامتی صحیح پورٹ;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	حرف *segment;		/**< Path segment for this node */
	غیرعلامتی صحیح frag_cnt;	/**< Number of entries in path_data::fragment */
	حرف **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	ساخت url_اندرونی_ڈیٹا urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ثابت ساخت prot_space_ڈیٹا *prot_space;
	/** Cookies associated with resource */
	ساخت cookie_اندرونی_ڈیٹا *cookies;
	/** Last cookie in list */
	ساخت cookie_اندرونی_ڈیٹا *cookies_انتہا;

	ساخت راستہ_ڈیٹا *اگلا;	/**< Next sibling */
	ساخت راستہ_ڈیٹا *پچھلا;	/**< Previous sibling */
	ساخت راستہ_ڈیٹا *والد; /**< Parent path segment */
	ساخت راستہ_ڈیٹا *children; /**< Child path segments */
	ساخت راستہ_ڈیٹا *آخری; /**< Last child */
};

ساخت hsts_ڈیٹا {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

ساخت host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	ساخت راستہ_ڈیٹا paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	ساخت hsts_ڈیٹا hsts;

	/**
	 * Part of host string
	 */
	حرف *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	ساخت prot_space_ڈیٹا *prot_space;

	ساخت host_part *اگلا;	/**< Next sibling */
	ساخت host_part *پچھلا;	/**< Previous sibling */
	ساخت host_part *والد; /**< Parent host part */
	ساخت host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
ساخت تلاش_گرہ {
	ثابت ساخت host_part *ڈیٹا;	/**< Host tree entry */

	غیرعلامتی صحیح level;		/**< Node level */

	ساخت تلاش_گرہ *بایاں;	/**< Left subtree */
	ساخت تلاش_گرہ *دایاں;	/**< Right subtree */
};

/** Root database handle */
ساکن ساخت host_part db_جڑ;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#تعریف_کرو عدد_تلاش_TREES 28
#تعریف_کرو ST_IP 0
#تعریف_کرو ST_EE 1
#تعریف_کرو ST_DN 2
ساکن ساخت تلاش_گرہ خالی_خانے = { 0, 0, &خالی_خانے, &خالی_خانے };
ساکن ساخت تلاش_گرہ *تلاش_trees[عدد_تلاش_TREES] = {
	&خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے,
	&خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے,
	&خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے,
	&خالی_خانے, &خالی_خانے, &خالی_خانے, &خالی_خانے
};

/** Minimum cookie database file version */
#تعریف_کرو MIN_COOKIE_FILE_نسخہ 100
/** Current cookie database file version */
#تعریف_کرو COOKIE_FILE_نسخہ 102
/** loaded cookie file version */
ساکن صحیح loaded_cookie_file_نسخہ;

/** Minimum URL database file version */
#تعریف_کرو MIN_URL_FILE_نسخہ 106
/** Current URL database file version */
#تعریف_کرو URL_FILE_نسخہ 107

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
ساکن ساخت bloom_filter *url_bloom;
/**
 * Size of url filter
 */
#تعریف_کرو BLOOM_سائز (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
ساکن nserror urldb_write_timet(FILE *fp, time_t val)
{
	صحیح use;
	حرف op[32];

	use = nsc_sntimet(op, 32, &val);
	اگر (use == 0) {
		fprintf(fp, "%i\n", (صحیح)val);
	} ورنہ {
		fprintf(fp, "%.*s\n", use, op);
	}
	واپس NSERROR_OK;
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
ساکن خالی
urldb_write_paths(ثابت ساخت راستہ_ڈیٹا *والد,
		  ثابت حرف *host,
		  FILE *fp,
		  حرف **راستہ,
		  صحیح *راستہ_alloc,
		  صحیح *راستہ_مستعمل,
		  time_t expiry)
{
	ثابت ساخت راستہ_ڈیٹا *p = والد;
	صحیح i;

	کرو {
		صحیح seglen = p->segment != تہی ? لڑی_لمبائی(p->segment) : 0;
		صحیح len = *راستہ_مستعمل + seglen + 1;

		اگر (*راستہ_alloc < len) {
			حرف *عارضی;
			عارضی = realloc(*راستہ,
				       (len > 64) ? len : *راستہ_alloc + 64);
			اگر (!عارضی) {
				واپس;
			}
			*راستہ = عارضی;
			*راستہ_alloc = (len > 64) ? len : *راستہ_alloc + 64;
		}

		اگر (p->segment != تہی) {
			حافظہ_نقل(*راستہ + *راستہ_مستعمل - 1, p->segment, seglen);
		}

		اگر (p->children != تہی) {
			(*راستہ)[*راستہ_مستعمل + seglen - 1] = '/';
			(*راستہ)[*راستہ_مستعمل + seglen] = '\0';
		} ورنہ {
			(*راستہ)[*راستہ_مستعمل + seglen - 1] = '\0';
			len -= 1;
		}

		*راستہ_مستعمل = len;

		اگر (p->children != تہی) {
			/* Drill down into children */
			p = p->children;
		} ورنہ {
			/* leaf node */
			اگر (p->persistent ||
			    ((p->urld.آخری_دورہ > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_ڈیٹا(p->scheme));

				اگر (p->پورٹ) {
					fprintf(fp,"%d\n", p->پورٹ);
				} ورنہ {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *راستہ);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.آخری_دورہ);

				/* entry type */
				fprintf(fp, "%i\n", (صحیح)p->urld.type);

				fprintf(fp, "\n");

				اگر (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					برائے (i = 0; s[i] != '\0'; i++)
						اگر (s[i] < 32)
							s[i] = ' ';
					برائے (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} ورنہ {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			جب (p != والد) {
				صحیح seglen = p->segment != تہی
					? لڑی_لمبائی(p->segment) : 0;

				/* Remove our segment from the path */
				*راستہ_مستعمل -= seglen;
				(*راستہ)[*راستہ_مستعمل - 1] = '\0';

				اگر (p->اگلا != تہی) {
					/* Have a sibling, process that */
					p = p->اگلا;
					توڑو;
				}

				/* Going up, so remove '/' */
				*راستہ_مستعمل -= 1;
				(*راستہ)[*راستہ_مستعمل - 1] = '\0';

				/* Ascend tree */
				p = p->والد;
			}
		}
	} جب (p != والد);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
ساکن خالی
urldb_گنتی_urls(ثابت ساخت راستہ_ڈیٹا *جڑ,
		 time_t expiry,
		 غیرعلامتی صحیح *گنتی)
{
	ثابت ساخت راستہ_ڈیٹا *p = جڑ;

	کرو {
		اگر (p->children != تہی) {
			/* Drill down into children */
			p = p->children;
		} ورنہ {
			/* No more children, increment count if required */
			اگر (p->persistent ||
			    ((p->urld.آخری_دورہ > expiry) &&
			     (p->urld.visits > 0))) {
				(*گنتی)++;
			}

			/* Now, find next node to process. */
			جب (p != جڑ) {
				اگر (p->اگلا != تہی) {
					/* Have a sibling, process that */
					p = p->اگلا;
					توڑو;
				}

				/* Ascend tree */
				p = p->والد;
			}
		}
	} جب (p != جڑ);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
ساکن خالی urldb_save_تلاش_درخت(ساخت تلاش_گرہ *والد, FILE *fp)
{
	حرف host[256];
	ثابت ساخت host_part *h;
	غیرعلامتی صحیح راستہ_گنتی = 0;
	حرف *راستہ, *p, *انتہا;
	صحیح راستہ_alloc = 64, راستہ_مستعمل = 1;
	time_t expiry, hsts_expiry = 0;
	صحیح hsts_include_subdomains = 0;

	expiry = time(تہی) - ((60 * 60 * 24) * nsoption_int(expire_url));

	اگر (والد == &خالی_خانے)
		واپس;

	urldb_save_تلاش_درخت(والد->بایاں, fp);

	راستہ = حافظہ_دو(راستہ_alloc);
	اگر (!راستہ)
		واپس;

	راستہ[0] = '\0';

	برائے (h = والد->ڈیٹا, p = host, انتہا = host + حجم host;
	     h && h != &db_جڑ && p < انتہا; h = h->والد) {
		صحیح written = snprintf(p, انتہا - p, "%s%s", h->part,
				       (h->والد && h->والد->والد) ? "." : "");
		اگر (written < 0) {
			آزاد(راستہ);
			واپس;
		}
		p += written;
	}

	h = والد->ڈیٹا;
	اگر (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_گنتی_urls(&والد->ڈیٹا->paths, expiry, &راستہ_گنتی);

	اگر (راستہ_گنتی > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", راستہ_گنتی);

		urldb_write_paths(&والد->ڈیٹا->paths, host, fp,
				  &راستہ, &راستہ_alloc, &راستہ_مستعمل, expiry);
	} ورنہ اگر (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	آزاد(راستہ);

	urldb_save_تلاش_درخت(والد->دایاں, fp);
}


/**
 * Path data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ساکن bool
urldb_iterate_entries_راستہ(ثابت ساخت راستہ_ڈیٹا *والد,
		bool (*url_callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا),
		bool (*cookie_callback)(ثابت ساخت cookie_ڈیٹا *ڈیٹا))
{
	ثابت ساخت راستہ_ڈیٹا *p = والد;
	ثابت ساخت cookie_ڈیٹا *c;

	کرو {
		اگر (p->children != تہی) {
			/* Drill down into children */
			p = p->children;
		} ورنہ {
			/* All leaf nodes in the path tree should have an URL or
			 * cookies attached to them. If this is not the case, it
			 * indicates that there's a bug in the file loader/URL
			 * insertion code. Therefore, assert this here. */
			أكد(url_callback || cookie_callback);

			/** \todo handle fragments? */
			اگر (url_callback) {
				ثابت ساخت url_اندرونی_ڈیٹا *u = &p->urld;

				أكد(p->url);

				اگر (!url_callback(p->url,
						  (ثابت ساخت url_ڈیٹا *) u))
					واپس false;
			} ورنہ {
				c = (ثابت ساخت cookie_ڈیٹا *)p->cookies;
				برائے (; c != تہی; c = c->اگلا) {
					اگر (!cookie_callback(c))
						واپس false;
				}
			}

			/* Now, find next node to process. */
			جب (p != والد) {
				اگر (p->اگلا != تہی) {
					/* Have a sibling, process that */
					p = p->اگلا;
					توڑو;
				}

				/* Ascend tree */
				p = p->والد;
			}
		}
	} جب (p != والد);

	واپس true;
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
ساکن bool urldb_host_is_ip_پتہ(ثابت حرف *host)
{
	ساخت اندر_addr ipv4;
	size_t host_len = لڑی_لمبائی(host);
	ثابت حرف *sane_host;
	ثابت حرف *slash;
#اگر_غیر_تعریف_شدہ NO_IPV6
	ساخت in6_addr ipv6;
	حرف ipv6_addr[64];
	غیرعلامتی صحیح ipv6_addr_len;
#اگر_ختم
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
	اگر (slash == تہی) {
		sane_host = host;
	} ورنہ {
		حرف *c = strdup(host);
		c[slash - host] = '\0';
		sane_host = c;
		host_len = slash - host;
		NSLOG(netsurf, INFO, "WARNING: called with non-host '%s'",
		      host);
	}

	اگر (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		جاؤ برآمد_false;

	اگر (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		صحیح عدد_dots = 0;
		size_t index;

		برائے (index = 0; index < host_len; index++) {
			اگر (sane_host[index] == '.')
				عدد_dots++;
		}

		اگر (عدد_dots == 3)
			جاؤ برآمد_true;
		ورنہ
			جاؤ برآمد_false;
	}

#اگر_غیر_تعریف_شدہ NO_IPV6
	اگر ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		جاؤ برآمد_false;
	}

	ipv6_addr_len = host_len - 2;
	اگر (ipv6_addr_len >= حجم(ipv6_addr)) {
		ipv6_addr_len = حجم(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	اگر (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		جاؤ برآمد_true;
#اگر_ختم

برآمد_false:
	اگر (slash != تہی) آزاد((خالی *)sane_host);
	واپس false;

برآمد_true:
	اگر (slash != تہی) آزاد((خالی *)sane_host);
	واپس true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
ساکن صحیح urldb_تلاش_مطابقت_سابقہ(ثابت ساخت host_part *a, ثابت حرف *b)
{
	ثابت حرف *انتہا, *dot;
	صحیح plen, ret;

	أكد(a && a != &db_جڑ && b);

	اگر (urldb_host_is_ip_پتہ(b)) {
		/* IP address */
		واپس strncasecmp(a->part, b, لڑی_لمبائی(b));
	}

	انتہا = b + لڑی_لمبائی(b) + 1;

	جب (b < انتہا && a && a != &db_جڑ) {
		dot = strchr(b, '.');
		اگر (!dot) {
			/* last segment */
			dot = انتہا - 1;
		}

		/* Compare strings (length limited) */
		اگر ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			واپس ret;

		/* The strings matched */
		اگر (dot < انتہا - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = لڑی_لمبائی(a->part);
			اگر (plen > dot - b) {
				/* len(a) > len(b) */
				واپس 1;
			} ورنہ اگر (plen < dot - b) {
				/* len(a) < len(b) */
				واپس -1;
			}
		}

		b = dot + 1;
		a = a->والد;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_جڑ && b >= انتہا) {
		/* len(a) > len(b) => prefix matches */
		واپس 0;
	} ورنہ اگر ((!a || a == &db_جڑ) && b < انتہا) {
		/* len(a) < len(b) => prefix does not match */
		واپس -1;
	}

	/* Identical */
	واپس 0;
}


/**
 * Partial host iterator (internal)
 *
 * \param root Root of (sub)tree to traverse
 * \param prefix Prefix to match
 * \param callback Callback function
 * \return true to continue, false otherwise
 */
ساکن bool
urldb_iterate_partial_host(ساخت تلاش_گرہ *جڑ,
		ثابت حرف *سابقہ,
		bool (*callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا))
{
	صحیح c;

	أكد(جڑ && سابقہ && callback);

	اگر (جڑ == &خالی_خانے)
		واپس true;

	c = urldb_تلاش_مطابقت_سابقہ(جڑ->ڈیٹا, سابقہ);

	اگر (c > 0) {
		/* No match => look in left subtree */
		واپس urldb_iterate_partial_host(جڑ->بایاں,
						  سابقہ,
						  callback);
	} ورنہ اگر (c < 0) {
		/* No match => look in right subtree */
		واپس urldb_iterate_partial_host(جڑ->دایاں,
						  سابقہ,
						  callback);
	} ورنہ {
		/* Match => iterate over l/r subtrees & process this node */
		اگر (!urldb_iterate_partial_host(جڑ->بایاں,
						سابقہ,
						callback)) {
			واپس false;
		}

		اگر (جڑ->ڈیٹا->paths.children) {
			/* and extract all paths attached to this host */
			اگر (!urldb_iterate_entries_راستہ(&جڑ->ڈیٹا->paths,
							callback,
							تہی)) {
				واپس false;
			}
		}

		اگر (!urldb_iterate_partial_host(جڑ->دایاں,
						سابقہ,
						callback)) {
			واپس false;
		}
	}

	واپس true;
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
ساکن bool
urldb_iterate_partial_راستہ(ثابت ساخت راستہ_ڈیٹا *والد,
		ثابت حرف *سابقہ,
		bool (*callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا))
{
	ثابت ساخت راستہ_ڈیٹا *p = والد->children;
	ثابت حرف *slash, *انتہا = سابقہ + لڑی_لمبائی(سابقہ);

	کرو {
		slash = strchr(سابقہ, '/');
		اگر (!slash) {
			slash = انتہا;
		}

		اگر (slash == سابقہ && *سابقہ == '/') {
			/* Ignore "//" */
			سابقہ++;
			جاری;
		}

		اگر (strncasecmp(p->segment, سابقہ, slash - سابقہ) == 0) {
			/* prefix matches so far */
			اگر (slash == انتہا) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				اگر (!urldb_iterate_entries_راستہ(p,
								callback,
								تہی)) {
					واپس false;
				}

				/* Progress to next sibling */
				p = p->اگلا;
			} ورنہ {
				/* Skip over this segment */
				سابقہ = slash + 1;

				p = p->children;
			}
		} ورنہ {
			/* Doesn't match this segment, try next sibling */
			p = p->اگلا;
		}
	} جب (p != تہی);

	واپس true;
}


/**
 * Host data iterator (internal)
 *
 * \param parent Root of subtree to iterate over
 * \param url_callback Callback function
 * \param cookie_callback Callback function
 * \return true to continue, false otherwise
 */
ساکن bool
urldb_iterate_entries_host(ساخت تلاش_گرہ *والد,
		bool (*url_callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا),
		bool (*cookie_callback)(ثابت ساخت cookie_ڈیٹا *ڈیٹا))
{
	اگر (والد == &خالی_خانے) {
		واپس true;
	}

	اگر (!urldb_iterate_entries_host(والد->بایاں,
					url_callback,
					cookie_callback)) {
		واپس false;
	}

	اگر ((والد->ڈیٹا->paths.children) ||
	    ((cookie_callback) &&
	     (والد->ڈیٹا->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		اگر (!urldb_iterate_entries_راستہ(&والد->ڈیٹا->paths,
						url_callback,
						cookie_callback)) {
			واپس false;
		}
	}

	اگر (!urldb_iterate_entries_host(والد->دایاں,
					url_callback,
					cookie_callback)) {
		واپس false;
	}

	واپس true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
ساکن ساخت host_part *
urldb_جوڑو_host_گرہ(ثابت حرف *part, ساخت host_part *والد)
{
	ساخت host_part *d;

	أكد(part && والد);

	d = صفر_حافظہ_دو(1, حجم(ساخت host_part));
	اگر (!d) {
		واپس تہی;
	}

	d->part = strdup(part);
	اگر (!d->part) {
		آزاد(d);
		واپس تہی;
	}

	d->اگلا = والد->children;
	اگر (والد->children) {
		والد->children->پچھلا = d;
	}
	d->والد = والد;
	والد->children = d;

	واپس d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
ساکن صحیح urldb_جوڑو_راستہ_fragment_cmp(ثابت خالی *a, ثابت خالی *b)
{
	واپس strcasecmp(*((ثابت حرف **) a), *((ثابت حرف **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
ساکن ساخت راستہ_ڈیٹا *
urldb_جوڑو_راستہ_fragment(ساخت راستہ_ڈیٹا *segment, lwc_string *fragment)
{
	حرف **عارضی;

	أكد(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	اگر (!fragment)
		واپس segment;

	عارضی = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * حجم(حرف *));
	اگر (!عارضی)
		واپس تہی;

	segment->fragment = عارضی;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_ڈیٹا(fragment));
	اگر (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		واپس تہی;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	qsort(segment->fragment,
	      segment->frag_cnt,
	      حجم (حرف *),
	      urldb_جوڑو_راستہ_fragment_cmp);

	واپس segment;
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
ساکن ساخت راستہ_ڈیٹا *
urldb_جوڑو_راستہ_گرہ(lwc_string *scheme,
		    غیرعلامتی صحیح پورٹ,
		    ثابت حرف *segment,
		    lwc_string *fragment,
		    ساخت راستہ_ڈیٹا *والد)
{
	ساخت راستہ_ڈیٹا *d, *e;

	أكد(scheme && segment && والد);

	d = صفر_حافظہ_دو(1, حجم(ساخت راستہ_ڈیٹا));
	اگر (!d)
		واپس تہی;

	d->scheme = lwc_string_ref(scheme);

	d->پورٹ = پورٹ;

	d->segment = strdup(segment);
	اگر (!d->segment) {
		lwc_string_unref(d->scheme);
		آزاد(d);
		واپس تہی;
	}

	اگر (fragment) {
		اگر (!urldb_جوڑو_راستہ_fragment(d, fragment)) {
			آزاد(d->segment);
			lwc_string_unref(d->scheme);
			آزاد(d);
			واپس تہی;
		}
	}

	برائے (e = والد->children; e; e = e->اگلا) {
		اگر (strcmp(e->segment, d->segment) > 0)
			توڑو;
	}

	اگر (e) {
		d->پچھلا = e->پچھلا;
		d->اگلا = e;
		اگر (e->پچھلا)
			e->پچھلا->اگلا = d;
		ورنہ
			والد->children = d;
		e->پچھلا = d;
	} ورنہ اگر (!والد->children) {
		d->پچھلا = d->اگلا = تہی;
		والد->children = والد->آخری = d;
	} ورنہ {
		d->اگلا = تہی;
		d->پچھلا = والد->آخری;
		والد->آخری->اگلا = d;
		والد->آخری = d;
	}
	d->والد = والد;

	واپس d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساکن ساخت تلاش_گرہ **urldb_get_تلاش_درخت_براہ_راست(ثابت حرف *host)
{
	أكد(host);

	اگر (urldb_host_is_ip_پتہ(host)) {
		واپس &تلاش_trees[ST_IP];
	} ورنہ اگر (ascii_is_alpha(*host)) {
		واپس &تلاش_trees[ST_DN + ascii_مقصد_زیریں(*host) - 'a'];
	}
	واپس &تلاش_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
ساکن ساخت تلاش_گرہ *urldb_get_تلاش_درخت(ثابت حرف *host)
{
	واپس *urldb_get_تلاش_درخت_براہ_راست(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
ساکن صحیح urldb_تلاش_مطابقت_string(ثابت ساخت host_part *a, ثابت حرف *b)
{
	ثابت حرف *انتہا, *dot;
	صحیح plen, ret;

	أكد(a && a != &db_جڑ && b);

	اگر (urldb_host_is_ip_پتہ(b)) {
		/* IP address */
		واپس strcasecmp(a->part, b);
	}

	انتہا = b + لڑی_لمبائی(b) + 1;

	جب (b < انتہا && a && a != &db_جڑ) {
		dot = strchr(b, '.');
		اگر (!dot) {
			/* last segment */
			dot = انتہا - 1;
		}

		/* Compare strings (length limited) */
		اگر ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			واپس ret;

		/* The strings matched, now check that the lengths do, too */
		plen = لڑی_لمبائی(a->part);

		اگر (plen > dot - b) {
			/* len(a) > len(b) */
			واپس 1;
		} ورنہ اگر (plen < dot - b) {
			/* len(a) < len(b) */
			واپس -1;
		}

		b = dot + 1;
		a = a->والد;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_جڑ && b >= انتہا) {
		/* len(a) > len(b) */
		واپس 1;
	} ورنہ اگر ((!a || a == &db_جڑ) && b < انتہا) {
		/* len(a) < len(b) */
		واپس -1;
	}

	/* Identical */
	واپس 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
ساکن ثابت ساخت host_part *
urldb_تلاش_ڈھونڈو(ساخت تلاش_گرہ *جڑ, ثابت حرف *host)
{
	صحیح c;

	أكد(جڑ && host);

	اگر (جڑ == &خالی_خانے) {
		واپس تہی;
	}

	c = urldb_تلاش_مطابقت_string(جڑ->ڈیٹا, host);

	اگر (c > 0) {
		واپس urldb_تلاش_ڈھونڈو(جڑ->بایاں, host);
	} ورنہ اگر (c < 0) {
		واپس urldb_تلاش_ڈھونڈو(جڑ->دایاں, host);
	}

	واپس جڑ->ڈیٹا;
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
ساکن ساخت راستہ_ڈیٹا *
urldb_مطابقت_راستہ(ثابت ساخت راستہ_ڈیٹا *والد,
		 ثابت حرف *راستہ,
		 lwc_string *scheme,
		 غیرعلامتی مختصر پورٹ)
{
	ثابت ساخت راستہ_ڈیٹا *p;
	ثابت حرف *slash;
	bool مطابقت;

	أكد(والد != تہی);
	أكد(والد->segment == تہی);

	اگر (راستہ[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", راستہ);
	}

	أكد(راستہ[0] == '/');

	/* Start with children, as parent has no segment */
	p = والد->children;

	جب (p != تہی) {
		slash = strchr(راستہ + 1, '/');
		اگر (!slash) {
			slash = راستہ + لڑی_لمبائی(راستہ);
		}

		اگر (لڑی_موازنہ(p->segment, راستہ + 1, slash - راستہ - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &مطابقت) == lwc_error_ok &&
		    مطابقت == true &&
		    p->پورٹ == پورٹ) {
			اگر (*slash == '\0') {
				/* Complete match */
				واپس (ساخت راستہ_ڈیٹا *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			راستہ = slash;
		} ورنہ {
			/* No match, try next sibling */
			p = p->اگلا;
		}
	}

	واپس تہی;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
ساکن ساخت راستہ_ڈیٹا *urldb_ڈھونڈو_url(nsurl *url)
{
	ثابت ساخت host_part *h;
	ساخت راستہ_ڈیٹا *p;
	ساخت تلاش_گرہ *درخت;
	حرف *plq;
	ثابت حرف *host_str;
	lwc_string *scheme, *host, *پورٹ;
	size_t len = 0;
	غیرعلامتی صحیح پورٹ_int;
	bool مطابقت;

	أكد(url);

	اگر (url_bloom != تہی) {
		اگر (bloom_تلاش_hash(url_bloom, nsurl_hash(url)) == false) {
			واپس تہی;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == تہی)
		واپس تہی;

	اگر (lwc_string_isequal(scheme, corestring_lwc_mailto, &مطابقت) ==
	    lwc_error_ok && مطابقت == true) {
		lwc_string_unref(scheme);
		واپس تہی;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != تہی) {
		host_str = lwc_string_ڈیٹا(host);
		lwc_string_unref(host);

	} ورنہ اگر (lwc_string_isequal(scheme, corestring_lwc_file, &مطابقت) ==
		   lwc_error_ok && مطابقت == true) {
		host_str = "localhost";

	} ورنہ {
		lwc_string_unref(scheme);
		واپس تہی;
	}

	درخت = urldb_get_تلاش_درخت(host_str);
	h = urldb_تلاش_ڈھونڈو(درخت, host_str);
	اگر (!h) {
		lwc_string_unref(scheme);
		واپس تہی;
	}

	/* generate plq (path, leaf, query) */
	اگر (nsurl_get(url, NSURL_راستہ | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		واپس تہی;
	}

	/* Get port */
	پورٹ = nsurl_get_component(url, NSURL_پورٹ);
	اگر (پورٹ != تہی) {
		پورٹ_int = atoi(lwc_string_ڈیٹا(پورٹ));
		lwc_string_unref(پورٹ);
	} ورنہ {
		پورٹ_int = 0;
	}

	p = urldb_مطابقت_راستہ(&h->paths, plq, scheme, پورٹ_int);

	آزاد(plq);
	lwc_string_unref(scheme);

	واپس p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساکن خالی urldb_dump_paths(ساخت راستہ_ڈیٹا *والد)
{
	ثابت ساخت راستہ_ڈیٹا *p = والد;
	غیرعلامتی صحیح i;

	کرو {
		اگر (p->segment != تہی) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_ڈیٹا(p->scheme), p->پورٹ);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			برائے (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		اگر (p->children != تہی) {
			p = p->children;
		} ورنہ {
			جب (p != والد) {
				اگر (p->اگلا != تہی) {
					p = p->اگلا;
					توڑو;
				}

				p = p->والد;
			}
		}
	} جب (p != والد);
}


/**
 * Dump URL database hosts to stderr
 *
 * \param parent Parent node of tree to dump
 */
ساکن خالی urldb_dump_hosts(ساخت host_part *والد)
{
	ساخت host_part *h;

	اگر (والد->part) {
		NSLOG(netsurf, INFO, "%s", والد->part);

		NSLOG(netsurf, INFO, "\t%s invalid SSL certs",
		      والد->permit_invalid_certs ? "Permits" : "Denies");
	}

	/* Dump path data */
	urldb_dump_paths(&والد->paths);

	/* and recurse */
	برائے (h = والد->children; h; h = h->اگلا) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
ساکن خالی urldb_dump_تلاش(ساخت تلاش_گرہ *والد, صحیح گہرائی)
{
	ثابت ساخت host_part *h;
	صحیح i; /* index into string */
	حرف s[1024];
	صحیح r;
	صحیح sl = حجم(s) - 2;

	اگر (والد == &خالی_خانے)
		واپس;

	urldb_dump_تلاش(والد->بایاں, گہرائی + 1);

	برائے (i = 0; i != گہرائی; i++) {
		s[i] = ' ';
	}

	برائے (h = والد->ڈیٹا; h; h = h->والد) {
		اگر (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			اگر (r < 0) {
				توڑو;
			}
			اگر ((i + r) >= sl) {
				توڑو;
			}
			i += r;
		}

		اگر (h->والد && h->والد->والد) {
			s[i]='.';
			i++;
		}
	}
	s[i]= 0;

	NSLOG(netsurf, INFO, "%s", s);

	urldb_dump_تلاش(والد->دایاں, گہرائی + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
ساکن صحیح
urldb_تلاش_مطابقت_host(ثابت ساخت host_part *a, ثابت ساخت host_part *b)
{
	صحیح ret;

	أكد(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	برائے (; a && a != &db_جڑ && b && b != &db_جڑ;
	     a = a->والد, b = b->والد) {
		اگر ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			واپس ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	اگر (a && a != &db_جڑ && (!b || b == &db_جڑ)) {
		/* len(a) > len(b) */
		واپس 1;
	} ورنہ اگر ((!a || a == &db_جڑ) && b && b != &db_جڑ) {
		/* len(a) < len(b) */
		واپس -1;
	}

	/* identical */
	واپس 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
ساکن ساخت تلاش_گرہ *urldb_تلاش_skew(ساخت تلاش_گرہ *جڑ)
{
	أكد(جڑ);

	اگر (جڑ->بایاں->level == جڑ->level) {
		ساخت تلاش_گرہ *عارضی;

		عارضی = جڑ->بایاں;
		جڑ->بایاں = عارضی->دایاں;
		عارضی->دایاں = جڑ;
		جڑ = عارضی;
	}

	واپس جڑ;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
ساکن ساخت تلاش_گرہ *urldb_تلاش_بانٹو(ساخت تلاش_گرہ *جڑ)
{
	أكد(جڑ);

	اگر (جڑ->دایاں->دایاں->level == جڑ->level) {
		ساخت تلاش_گرہ *عارضی;

		عارضی = جڑ->دایاں;
		جڑ->دایاں = عارضی->بایاں;
		عارضی->بایاں = جڑ;
		جڑ = عارضی;

		جڑ->level++;
	}

	واپس جڑ;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
ساکن ساخت تلاش_گرہ *
urldb_تلاش_داخل_کرو_اندرونی(ساخت تلاش_گرہ *جڑ, ساخت تلاش_گرہ *n)
{
	أكد(جڑ && n);

	اگر (جڑ == &خالی_خانے) {
		جڑ = n;
	} ورنہ {
		صحیح c = urldb_تلاش_مطابقت_host(جڑ->ڈیٹا, n->ڈیٹا);

		اگر (c > 0) {
			جڑ->بایاں = urldb_تلاش_داخل_کرو_اندرونی(
				جڑ->بایاں, n);
		} ورنہ اگر (c < 0) {
			جڑ->دایاں = urldb_تلاش_داخل_کرو_اندرونی(
				جڑ->دایاں, n);
		} ورنہ {
			/* exact match */
			آزاد(n);
			واپس جڑ;
		}

		جڑ = urldb_تلاش_skew(جڑ);
		جڑ = urldb_تلاش_بانٹو(جڑ);
	}

	واپس جڑ;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
ساکن ساخت تلاش_گرہ *
urldb_تلاش_داخل_کرو(ساخت تلاش_گرہ *جڑ, ثابت ساخت host_part *ڈیٹا)
{
	ساخت تلاش_گرہ *n;

	أكد(جڑ && ڈیٹا);

	n = حافظہ_دو(حجم(ساخت تلاش_گرہ));
	اگر (!n)
		واپس تہی;

	n->level = 1;
	n->ڈیٹا = ڈیٹا;
	n->بایاں = n->دایاں = &خالی_خانے;

	جڑ = urldb_تلاش_داخل_کرو_اندرونی(جڑ, n);

	واپس جڑ;
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
ساکن bool
urldb_parse_avpair(ساخت cookie_اندرونی_ڈیٹا *c,
		   حرف *n,
		   حرف *v,
		   bool was_quoted)
{
	صحیح vlen;

	أكد(c && n && v);

	/* Strip whitespace from start of name */
	برائے (; *n; n++) {
		اگر (*n != ' ' && *n != '\t')
			توڑو;
	}

	/* Strip whitespace from end of name */
	برائے (vlen = لڑی_لمبائی(n); vlen; vlen--) {
		اگر (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		ورنہ
			توڑو;
	}

	/* Strip whitespace from start of value */
	برائے (; *v; v++) {
		اگر (*v != ' ' && *v != '\t')
			توڑو;
	}

	/* Strip whitespace from end of value */
	برائے (vlen = لڑی_لمبائی(v); vlen; vlen--) {
		اگر (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		ورنہ
			توڑو;
	}

	اگر (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		اگر (!c->comment)
			واپس false;
	} ورنہ اگر (!c->domain && strcasecmp(n, "Domain") == 0) {
		اگر (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_سیٹ = true;
			c->domain = strdup(v);
			اگر (!c->domain)
				واپس false;
		}
	} ورنہ اگر (strcasecmp(n, "Max-Age") == 0) {
		صحیح عارضی = atoi(v);
		اگر (عارضی == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		ورنہ
			c->expires = time(تہی) + عارضی;
	} ورنہ اگر (!c->راستہ && strcasecmp(n, "Path") == 0) {
		c->راستہ_from_سیٹ = true;
		c->راستہ = strdup(v);
		اگر (!c->راستہ)
			واپس false;
	} ورنہ اگر (strcasecmp(n, "Version") == 0) {
		c->نسخہ = atoi(v);
	} ورنہ اگر (strcasecmp(n, "Expires") == 0) {
		حرف *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		برائے (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, لڑی_لمبائی(datenoday), &expires);
		اگر (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} ورنہ اگر (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} ورنہ اگر (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} ورنہ اگر (!c->name) {
		c->name = strdup(n);
		c->قدر = strdup(v);
		c->قدر_was_quoted = was_quoted;
		اگر (!c->name || !c->قدر) {
			واپس false;
		}
	}

	واپس true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
ساکن خالی urldb_free_cookie(ساخت cookie_اندرونی_ڈیٹا *c)
{
	أكد(c);

	آزاد(c->comment);
	آزاد(c->domain);
	آزاد(c->راستہ);
	آزاد(c->name);
	آزاد(c->قدر);
	آزاد(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
ساکن ساخت cookie_اندرونی_ڈیٹا *
urldb_parse_cookie(nsurl *url, ثابت حرف **cookie)
{
	ساخت cookie_اندرونی_ڈیٹا *c;
	ثابت حرف *cur;
	حرف name[1024], قدر[4096];
	حرف *n = name, *v = قدر;
	bool اندر_قدر = false;
	bool had_قدر_ڈیٹا = false;
	bool قدر_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	أكد(url && cookie && *cookie);

	c = صفر_حافظہ_دو(1, حجم(ساخت cookie_اندرونی_ڈیٹا));
	اگر (c == تہی)
		واپس تہی;

	c->expires = -1;

	name[0] = '\0';
	قدر[0] = '\0';

	برائے (cur = *cookie; *cur; cur++) {
		اگر (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			اگر (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				قدر[0] = '\0';

			}

			توڑو;
		} ورنہ اگر (*cur == '\r') {
			/* Spurious linefeed */
			جاری;
		} ورنہ اگر (*cur == '\n') {
			/* Spurious newline */
			جاری;
		}

		اگر (اندر_قدر && !had_قدر_ڈیٹا) {
			اگر (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				جاری;
			} ورنہ {
				had_قدر_ڈیٹا = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				اگر (*cur != '"') {
					قدر_verbatim = true;
				}
			}
		}

		اگر (اندر_قدر && !قدر_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			اگر (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				جاری;
			}
		}

		اگر (!quoted && !اندر_قدر && *cur == '=') {
			/* First equals => attr-value separator */
			اندر_قدر = true;
			جاری;
		}

		اگر (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			اگر (!urldb_parse_avpair(c, name, قدر, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				واپس تہی;
			}

			/* And reset to start */
			n = name;
			v = قدر;
			اندر_قدر = false;
			had_قدر_ڈیٹا = false;
			قدر_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			اگر (*cur != ';') {
				cur--;
			}

			جاری;
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
			ثابت حرف *p;
			ثابت حرف *semi = strchr(cur + 1, ';');
			اگر (!semi)
				semi = cur + لڑی_لمبائی(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			برائے (p = cur + 1; p < semi; p++)
				اگر (*p == '=')
					توڑو;

			اگر (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} ورنہ {
				/* found one => comma marks end of cookie */
				cur++;
				توڑو;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		اگر (!اندر_قدر) {
			اگر (n < name + (حجم(name) - 1))
				*n++ = *cur;
		} ورنہ {
			اگر (v < قدر + (حجم(قدر) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	اگر (!urldb_parse_avpair(c, name, قدر, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		واپس تہی;
	}

	/* Now fix-up default values */
	اگر (c->domain == تہی) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		اگر (host == تہی) {
			urldb_free_cookie(c);
			واپس تہی;
		}
		c->domain = strdup(lwc_string_ڈیٹا(host));
		lwc_string_unref(host);
	}

	اگر (c->راستہ == تہی) {
		ثابت حرف *راستہ_ڈیٹا;
		حرف *راستہ, *slash;
		lwc_string *راستہ_lwc;

		راستہ_lwc = nsurl_get_component(url, NSURL_راستہ);
		اگر (راستہ_lwc == تہی) {
			urldb_free_cookie(c);
			واپس تہی;
		}
		راستہ_ڈیٹا = lwc_string_ڈیٹا(راستہ_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(راستہ_ڈیٹا, '/');
		اگر (slash != تہی) {
			/* Special case: retain first slash in path */
			اگر (slash == راستہ_ڈیٹا)
				slash++;

			slash = strndup(راستہ_ڈیٹا, slash - راستہ_ڈیٹا);
			اگر (slash == تہی) {
				lwc_string_unref(راستہ_lwc);
				urldb_free_cookie(c);
				واپس تہی;
			}

			راستہ = slash;
			lwc_string_unref(راستہ_lwc);
		} ورنہ {
			راستہ = strdup(lwc_string_ڈیٹا(راستہ_lwc));
			lwc_string_unref(راستہ_lwc);
			اگر (راستہ == تہی) {
				urldb_free_cookie(c);
				واپس تہی;
			}
		}

		c->راستہ = راستہ;
	}

	/* Write back current position */
	*cookie = cur;

	واپس c;
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
ساکن ساخت راستہ_ڈیٹا *
urldb_جوڑو_راستہ(lwc_string *scheme,
	       غیرعلامتی صحیح پورٹ,
	       ثابت ساخت host_part *host,
	       حرف *راستہ_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	ساخت راستہ_ڈیٹا *d, *e;
	حرف *buf = راستہ_query;
	حرف *segment, *slash;
	bool مطابقت;

	أكد(scheme && host && url);

	d = (ساخت راستہ_ڈیٹا *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	اگر (*segment == '/')
		segment++;

	/* Process path segments */
	کرو {
		slash = strchr(segment, '/');
		اگر (!slash) {
			/* last segment */
			/* look for existing entry */
			برائے (e = d->children; e; e = e->اگلا)
				اگر (strcmp(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &مطابقت) ==
				    lwc_error_ok &&
				    مطابقت == true &&
				    e->پورٹ == پورٹ)
					توڑو;

			d = e ? urldb_جوڑو_راستہ_fragment(e, fragment) :
				urldb_جوڑو_راستہ_گرہ(scheme, پورٹ,
						    segment, fragment, d);
			توڑو;
		}

		*slash = '\0';

		/* look for existing entry */
		برائے (e = d->children; e; e = e->اگلا)
			اگر (strcmp(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &مطابقت) == lwc_error_ok &&
			    مطابقت == true &&
			    e->پورٹ == پورٹ)
				توڑو;

		d = e ? e : urldb_جوڑو_راستہ_گرہ(scheme, پورٹ, segment, تہی, d);
		اگر (!d)
			توڑو;

		segment = slash + 1;
	} جب (1);

	آزاد(راستہ_query);

	اگر (d && !d->url) {
		/* Insert defragmented URL */
		اگر (nsurl_defragment(url, &d->url) != NSERROR_OK)
			واپس تہی;
	}

	واپس d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
ساکن ساخت host_part *urldb_جوڑو_host(ثابت حرف *host)
{
	ساخت host_part *d = (ساخت host_part *) &db_جڑ, *e;
	ساخت تلاش_گرہ *s;
	حرف buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	حرف *part;

	أكد(host);

	اگر (urldb_host_is_ip_پتہ(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		برائے (e = d->children; e; e = e->اگلا)
			اگر (strcasecmp(host, e->part) == 0)
				/* found => return it */
				واپس e;

		d = urldb_جوڑو_host_گرہ(host, d);

		s = urldb_تلاش_داخل_کرو(تلاش_trees[ST_IP], d);
		اگر (!s) {
			/* failed */
			d = تہی;
		} ورنہ {
			تلاش_trees[ST_IP] = s;
		}

		واپس d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, حجم buf);
	buf[حجم buf - 1] = '\0';

	/* Process FQDN segments backwards */
	کرو {
		part = strrchr(buf, '.');
		اگر (!part) {
			/* last segment */
			/* Check for existing entry */
			برائے (e = d->children; e; e = e->اگلا)
				اگر (strcasecmp(buf, e->part) == 0)
					توڑو;

			اگر (e) {
				d = e;
			} ورنہ {
				d = urldb_جوڑو_host_گرہ(buf, d);
			}

			/* And insert into search tree */
			اگر (d) {
				ساخت تلاش_گرہ **r;

				r = urldb_get_تلاش_درخت_براہ_راست(buf);
				s = urldb_تلاش_داخل_کرو(*r, d);
				اگر (!s) {
					/* failed */
					d = تہی;
				} ورنہ {
					*r = s;
				}
			}
			توڑو;
		}

		/* Check for existing entry */
		برائے (e = d->children; e; e = e->اگلا)
			اگر (strcasecmp(part + 1, e->part) == 0)
				توڑو;

		d = e ? e : urldb_جوڑو_host_گرہ(part + 1, d);
		اگر (!d)
			توڑو;

		*part = '\0';
	} جب (1);

	واپس d;
}


/**
 * Insert a cookie into the database
 *
 * \param c The cookie to insert
 * \param scheme URL scheme associated with cookie path
 * \param url URL (sans fragment) associated with cookie
 * \return true on success, false on memory exhaustion (c will be freed)
 */
ساکن bool
urldb_داخل_کرو_cookie(ساخت cookie_اندرونی_ڈیٹا *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	ساخت cookie_اندرونی_ڈیٹا *d;
	ثابت ساخت host_part *h;
	ساخت راستہ_ڈیٹا *p;
	time_t now = time(تہی);

	أكد(c);

	اگر (c->domain[0] == '.') {
		h = urldb_تلاش_ڈھونڈو(
			urldb_get_تلاش_درخت(&(c->domain[1])),
			c->domain + 1);
		اگر (!h) {
			h = urldb_جوڑو_host(c->domain + 1);
			اگر (!h) {
				urldb_free_cookie(c);
				واپس false;
			}
		}

		p = (ساخت راستہ_ڈیٹا *) &h->paths;
	} ورنہ {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		أكد(url != تہی);
		أكد(scheme != تہی);

		h = urldb_تلاش_ڈھونڈو(
			urldb_get_تلاش_درخت(c->domain),
			c->domain);

		اگر (!h) {
			h = urldb_جوڑو_host(c->domain);
			اگر (!h) {
				urldb_free_cookie(c);
				واپس false;
			}
		}

		/* find path */
		p = urldb_جوڑو_راستہ(scheme, 0, h,
				   strdup(c->راستہ), تہی, url);
		اگر (!p) {
			urldb_free_cookie(c);
			واپس false;
		}
	}

	/* add cookie */
	برائے (d = p->cookies; d; d = d->اگلا) {
		اگر (!strcmp(d->domain, c->domain) &&
		    !strcmp(d->راستہ, c->راستہ) &&
		    !strcmp(d->name, c->name))
			توڑو;
	}

	اگر (d) {
		اگر (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			اگر (d->اگلا)
				d->اگلا->پچھلا = d->پچھلا;
			ورنہ
				p->cookies_انتہا = d->پچھلا;
			اگر (d->پچھلا)
				d->پچھلا->اگلا = d->اگلا;
			ورنہ
				p->cookies = d->اگلا;

			cookie_manager_remove((ساخت cookie_ڈیٹا *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} ورنہ {
			/* replace d with c */
			c->پچھلا = d->پچھلا;
			c->اگلا = d->اگلا;
			اگر (c->اگلا)
				c->اگلا->پچھلا = c;
			ورنہ
				p->cookies_انتہا = c;
			اگر (c->پچھلا)
				c->پچھلا->اگلا = c;
			ورنہ
				p->cookies = c;

			cookie_manager_remove((ساخت cookie_ڈیٹا *)d);
			urldb_free_cookie(d);

			cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
		}
	} ورنہ {
		c->پچھلا = p->cookies_انتہا;
		c->اگلا = تہی;
		اگر (p->cookies_انتہا)
			p->cookies_انتہا->اگلا = c;
		ورنہ
			p->cookies = c;
		p->cookies_انتہا = c;

		cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
	}

	واپس true;
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
ساکن bool
concat_netscape_cookie(ساخت cookie_اندرونی_ڈیٹا *c,
		    صحیح *مستعمل,
		    صحیح *alloc,
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
	صحیح max_len;
	صحیح outl; /* output length */

	أكد(c && مستعمل && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *مستعمل + 2 +
		لڑی_لمبائی(c->name) + 1 +
		لڑی_لمبائی(c->قدر) + 2 +
		(c->راستہ_from_سیٹ ? 8 + لڑی_لمبائی(c->راستہ) + 2 : 0) +
		(c->domain_from_سیٹ ? 10 + لڑی_لمبائی(c->domain) + 2 : 0);
	اگر (max_len >= *alloc) {
		حرف *عارضی = realloc(*buf, max_len);
		اگر (!عارضی) {
			واپس false;
		}
		*buf = عارضی;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *مستعمل - 1, *alloc - *مستعمل, "; %s=", c->name);
	اگر (outl >= (*alloc - *مستعمل)) {
		/* output truncated */
		واپس false;
	}
	*مستعمل += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	اگر (c->قدر_was_quoted) {
		outl = snprintf(*buf + *مستعمل - 1,
				*alloc - *مستعمل,
				"\"%s\"", c->قدر);
		اگر (outl >= (*alloc - *مستعمل)) {
			/* output truncated */
			واپس false;
		}
		*مستعمل += outl;
	} ورنہ {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *مستعمل - 1,*alloc - *مستعمل, "%s", c->قدر);
		اگر (outl >= (*alloc - *مستعمل)) {
			/* output truncated */
			واپس false;
		}
		*مستعمل += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	واپس true;
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
ساکن bool
concat_rfc2965_cookie(ساخت cookie_اندرونی_ڈیٹا *c,
		      صحیح *مستعمل,
		      صحیح *alloc,
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
	صحیح max_len;
	صحیح outl; /* output length */

	أكد(c && مستعمل && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *مستعمل + 2 +
		لڑی_لمبائی(c->name) + 1 +
		لڑی_لمبائی(c->قدر) + 2 +
		(c->راستہ_from_سیٹ ?
		 8 + لڑی_لمبائی(c->راستہ) + 2 : 0) +
		(c->domain_from_سیٹ ?
		 10 + لڑی_لمبائی(c->domain) + 2 : 0);

	اگر (max_len >= *alloc) {
		حرف *عارضی = realloc(*buf, max_len);
		اگر (!عارضی) {
			واپس false;
		}
		*buf = عارضی;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *مستعمل - 1,
			*alloc - *مستعمل,
			"; %s=", c->name);
	اگر (outl >= (*alloc - *مستعمل)) {
		/* output truncated */
		واپس false;
	}
	*مستعمل += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	اگر (c->قدر_was_quoted ||
	    strpbrk(c->قدر, separators) != تہی) {
		outl = snprintf(*buf + *مستعمل - 1,
				*alloc - *مستعمل,
				"\"%s\"", c->قدر);

	} ورنہ {
		outl = snprintf(*buf + *مستعمل - 1,
				*alloc - *مستعمل,
				"%s", c->قدر);
	}
	اگر (outl >= (*alloc - *مستعمل)) {
		/* output truncated */
		واپس false;
	}
	*مستعمل += outl;

	اگر (c->راستہ_from_سیٹ) {
		/* Path, quoted if necessary */

		اگر (strpbrk(c->راستہ, separators) != تہی) {
			outl = snprintf(*buf + *مستعمل - 1,
					*alloc - *مستعمل,
					"; $Path=\"%s\"", c->راستہ);

		} ورنہ {
			outl = snprintf(*buf + *مستعمل - 1,
				       *alloc - *مستعمل,
				       "; $Path=%s", c->راستہ);
		}
		اگر (outl >= (*alloc - *مستعمل)) {
			/* output truncated */
			واپس false;
		}
		*مستعمل += outl;
	}

	اگر (c->domain_from_سیٹ) {
		/* Domain, quoted if necessary */
		اگر (strpbrk(c->domain, separators) != تہی) {
			outl = snprintf(*buf + *مستعمل - 1,
					*alloc - *مستعمل,
					"; $Domain=\"%s\"", c->domain);
		} ورنہ {
			outl = snprintf(*buf + *مستعمل - 1,
					*alloc - *مستعمل,
					"; $Domain=%s", c->domain);
		}
		اگر (outl >= (*alloc - *مستعمل)) {
			/* output truncated */
			واپس false;
		}
		*مستعمل += outl;
	}

	واپس true;
}


/**
 * build output cookie string in netscape format
 */
ساکن حرف *
get_netscape_cookie(ساخت cookie_اندرونی_ڈیٹا **matched_cookies, صحیح گنتی)
{
	صحیح ret_alloc = 4096, ret_مستعمل = 1;
	حرف *ret;
	صحیح midx; /* index into matched cookies */

	ret = حافظہ_دو(ret_alloc);
	اگر (ret == تہی) {
		واپس تہی;
	}
	ret[0] = '\0';

	برائے (midx = 0; midx < گنتی; midx++) {
		اگر (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_مستعمل, &ret_alloc, &ret)) {
			آزاد(ret);
			واپس تہی;
		}
	}

	اگر (ret_مستعمل <= 2) {
		آزاد(ret);
		واپس تہی;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_مستعمل - 2);
	ret_مستعمل -= 2;

	/* shrink the output buffer to the required size */
	حرف *عارضی = realloc(ret, ret_مستعمل);
	اگر (عارضی != تہی) {
		ret = عارضی;
	}

	واپس ret;
}


/**
 * build output cookie string in RFC2965 format
 */
ساکن حرف *
get_rfc2965_cookie(ساخت cookie_اندرونی_ڈیٹا **matched_cookies,
		   صحیح گنتی,
		   صحیح نسخہ)
{
	صحیح ret_alloc = 4096; /* return string allocation */
	صحیح ret_مستعمل = 1; /* return sting buffer used */
	حرف *ret; /* return string buffer */
	صحیح midx; /* index into matched cookies */
	صحیح outl;

	ret = حافظہ_دو(ret_alloc);
	اگر (ret == تہی) {
		واپس تہی;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_مستعمل, "$Version=%d", نسخہ);
	اگر (outl >= ret_alloc - ret_مستعمل) {
		/*حد_سے_تجاوز*/
		آزاد(ret);
		واپس تہی;
	}
	ret_مستعمل += outl;

	برائے (midx = 0; midx < گنتی; midx++) {
		اگر (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_مستعمل, &ret_alloc, &ret)) {
			آزاد(ret);
			واپس تہی;
		}
	}

	/* shrink the output buffer to the required size */
	حرف *عارضی = realloc(ret, ret_مستعمل);
	اگر (عارضی != تہی) {
		ret = عارضی;
	}

	واپس ret;
}


/**
 * deletes paths from a cookie.
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ساکن خالی
urldb_delete_cookie_paths(ثابت حرف *domain,
			  ثابت حرف *راستہ,
			  ثابت حرف *name,
			  ساخت راستہ_ڈیٹا *والد)
{
	ساخت cookie_اندرونی_ڈیٹا *c;
	ساخت راستہ_ڈیٹا *p = والد;

	أكد(والد);

	کرو {
		برائے (c = p->cookies; c; c = c->اگلا) {
			اگر (strcmp(c->domain, domain) == 0 &&
			    strcmp(c->راستہ, راستہ) == 0 &&
			    strcmp(c->name, name) == 0) {
				اگر (c->پچھلا) {
					c->پچھلا->اگلا = c->اگلا;
				} ورنہ {
					p->cookies = c->اگلا;
				}

				اگر (c->اگلا) {
					c->اگلا->پچھلا = c->پچھلا;
				} ورنہ {
					p->cookies_انتہا = c->پچھلا;
				}

				urldb_free_cookie(c);

				واپس;
			}
		}

		اگر (p->children) {
			p = p->children;
		} ورنہ {
			جب (p != والد) {
				اگر (p->اگلا != تہی) {
					p = p->اگلا;
					توڑو;
				}

				p = p->والد;
			}
		}
	} جب (p != والد);
}


/**
 * Deletes cookie hosts and their assoicated paths
 *
 * \param domain the cookie domain
 * \param path the cookie path
 * \param name The cookie name
 * \param parent The url data of the cookie
 */
ساکن خالی
urldb_delete_cookie_hosts(ثابت حرف *domain,
			  ثابت حرف *راستہ,
			  ثابت حرف *name,
			  ساخت host_part *والد)
{
	ساخت host_part *h;
	أكد(والد);

	urldb_delete_cookie_paths(domain, راستہ, name, &والد->paths);

	برائے (h = والد->children; h; h = h->اگلا) {
		urldb_delete_cookie_hosts(domain, راستہ, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
ساکن خالی urldb_save_cookie_paths(FILE *fp, ساخت راستہ_ڈیٹا *والد)
{
	ساخت راستہ_ڈیٹا *p = والد;
	time_t now = time(تہی);

	أكد(fp && والد);

	کرو {
		اگر (p->cookies != تہی) {
			ساخت cookie_اندرونی_ڈیٹا *c;

			برائے (c = p->cookies; c != تہی; c = c->اگلا) {
				اگر (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					جاری;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->نسخہ, c->domain,
					c->domain_from_سیٹ, c->راستہ,
					c->راستہ_from_سیٹ, c->secure,
					c->http_only,
					(صحیح)c->expires, (صحیح)c->آخری_مستعمل,
					c->no_destroy, c->name, c->قدر,
					c->قدر_was_quoted,
					p->scheme ? lwc_string_ڈیٹا(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		اگر (p->children != تہی) {
			p = p->children;
		} ورنہ {
			جب (p != والد) {
				اگر (p->اگلا != تہی) {
					p = p->اگلا;
					توڑو;
				}

				p = p->والد;
			}
		}
	} جب (p != والد);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
ساکن خالی urldb_save_cookie_hosts(FILE *fp, ساخت host_part *والد)
{
	ساخت host_part *h;
	أكد(fp && والد);

	urldb_save_cookie_paths(fp, &والد->paths);

	برائے (h = والد->children; h; h = h->اگلا)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
ساکن خالی urldb_destroy_cookie(ساخت cookie_اندرونی_ڈیٹا *c)
{
	آزاد(c->name);
	آزاد(c->قدر);
	آزاد(c->comment);
	آزاد(c->domain);
	آزاد(c->راستہ);

	آزاد(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
ساکن خالی urldb_destroy_راستہ_گرہ_content(ساخت راستہ_ڈیٹا *گرہ)
{
	ساخت cookie_اندرونی_ڈیٹا *a, *b;
	غیرعلامتی صحیح i;

	اگر (گرہ->url != تہی) {
		nsurl_unref(گرہ->url);
	}

	lwc_string_unref(گرہ->scheme);

	آزاد(گرہ->segment);
	برائے (i = 0; i < گرہ->frag_cnt; i++)
		آزاد(گرہ->fragment[i]);
	آزاد(گرہ->fragment);

	آزاد(گرہ->urld.title);

	برائے (a = گرہ->cookies; a; a = b) {
		b = a->اگلا;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
ساکن خالی urldb_destroy_prot_space(ساخت prot_space_ڈیٹا *space)
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
ساکن خالی urldb_destroy_راستہ_درخت(ساخت راستہ_ڈیٹا *جڑ)
{
	ساخت راستہ_ڈیٹا *p = جڑ;

	کرو {
		اگر (p->children != تہی) {
			p = p->children;
		} ورنہ {
			ساخت راستہ_ڈیٹا *q = p;

			جب (p != جڑ) {
				اگر (p->اگلا != تہی) {
					p = p->اگلا;
					توڑو;
				}

				p = p->والد;

				urldb_destroy_راستہ_گرہ_content(q);
				آزاد(q);

				q = p;
			}

			urldb_destroy_راستہ_گرہ_content(q);
			آزاد(q);
		}
	} جب (p != جڑ);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
ساکن خالی urldb_destroy_host_درخت(ساخت host_part *جڑ)
{
	ساخت host_part *a, *b;
	ساخت راستہ_ڈیٹا *p, *q;
	ساخت prot_space_ڈیٹا *s, *t;

	/* Destroy children */
	برائے (a = جڑ->children; a; a = b) {
		b = a->اگلا;
		urldb_destroy_host_درخت(a);
	}

	/* Now clean up paths */
	برائے (p = جڑ->paths.children; p; p = q) {
		q = p->اگلا;
		urldb_destroy_راستہ_درخت(p);
	}

	/* Root path */
	urldb_destroy_راستہ_گرہ_content(&جڑ->paths);

	/* Proctection space data */
	برائے (s = جڑ->prot_space; s; s = t) {
		t = s->اگلا;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	آزاد(جڑ->part);
	آزاد(جڑ);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
ساکن خالی urldb_destroy_تلاش_درخت(ساخت تلاش_گرہ *جڑ)
{
	/* Destroy children */
	اگر (جڑ->بایاں != &خالی_خانے)
		urldb_destroy_تلاش_درخت(جڑ->بایاں);
	اگر (جڑ->دایاں != &خالی_خانے)
		urldb_destroy_تلاش_درخت(جڑ->دایاں);

	/* And destroy ourselves */
	آزاد(جڑ);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
خالی urldb_destroy(خالی)
{
	ساخت host_part *a, *b;
	صحیح i;

	/* Clean up search trees */
	برائے (i = 0; i < عدد_تلاش_TREES; i++) {
		اگر (تلاش_trees[i] != &خالی_خانے) {
			urldb_destroy_تلاش_درخت(تلاش_trees[i]);
			تلاش_trees[i] = &خالی_خانے;
		}
	}

	/* And database */
	برائے (a = db_جڑ.children; a; a = b) {
		b = a->اگلا;
		urldb_destroy_host_درخت(a);
	}
	memset(&db_جڑ, 0, حجم(db_جڑ));

	/* And the bloom filter */
	اگر (url_bloom != تہی) {
		bloom_destroy(url_bloom);
		url_bloom = تہی;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_بھرو(ثابت حرف *filename)
{
#تعریف_کرو زیادہ_سے_زیادہ_URL_لمبائی 4096
	حرف s[زیادہ_سے_زیادہ_URL_لمبائی];
	حرف host[256];
	ساخت host_part *h;
	صحیح urls;
	صحیح i;
	صحیح نسخہ;
	صحیح لمبائی;
	FILE *fp;

	أكد(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	اگر (url_bloom == تہی)
		url_bloom = bloom_create(BLOOM_سائز);

	fp = fopen(filename, "r");
	اگر (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		واپس NSERROR_NOT_ملا;
	}

	اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp)) {
		fclose(fp);
		واپس NSERROR_NEED_ڈیٹا;
	}

	نسخہ = atoi(s);
	اگر (نسخہ < MIN_URL_FILE_نسخہ) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		واپس NSERROR_INVALID;
	}
	اگر (نسخہ > URL_FILE_نسخہ) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		واپس NSERROR_INVALID;
	}

	جب (fgets(host, حجم host, fp)) {
		time_t hsts_expiry = 0;
		صحیح hsts_include_sub_domains = 0;

		/* get the hostname */
		لمبائی = لڑی_لمبائی(host) - 1;
		host[لمبائی] = '\0';

		/* skip data that has ended up with a host of '' */
		اگر (لمبائی == 0) {
			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;
			urls = atoi(s);
			/* Eight fields/url */
			برائے (i = 0; i < (8 * urls); i++) {
				اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
					توڑو;
			}
			جاری;
		}

		اگر (نسخہ >= 107) {
			حرف *p = host;
			جب (*p && *p != ' ') p++;
			جب (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			جب (*p && *p != ' ') p++;
			جب (*p && *p == ' ') p++;
			nsc_snptimet(p, لڑی_لمبائی(p), &hsts_expiry);
		}

		h = urldb_جوڑو_host(host);
		اگر (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			واپس NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
			توڑو;
		urls = atoi(s);

		/* no URLs => try next host */
		اگر (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			جاری;
		}

		/* load the non-corrupt data */
		برائے (i = 0; i < urls; i++) {
			ساخت راستہ_ڈیٹا *p = تہی;
			حرف scheme[64], ports[10];
			حرف url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			غیرعلامتی صحیح پورٹ;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			حرف *راستہ_query;
			size_t len;

			اگر (!fgets(scheme, حجم scheme, fp))
				توڑو;
			لمبائی = لڑی_لمبائی(scheme) - 1;
			scheme[لمبائی] = '\0';

			اگر (!fgets(ports, حجم ports, fp))
				توڑو;
			لمبائی = لڑی_لمبائی(ports) - 1;
			ports[لمبائی] = '\0';
			پورٹ = atoi(ports);

			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;
			لمبائی = لڑی_لمبائی(s) - 1;
			s[لمبائی] = '\0';

			اگر (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, حجم url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (پورٹ ? ":" : ""),
				 (پورٹ ? ports : ""),
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
				واپس NSERROR_NOMEM;
			}

			اگر (url_bloom != تہی) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_داخل_کرو_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			اگر (nsurl_get(nsurl, NSURL_راستہ | NSURL_QUERY,
				      &راستہ_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				واپس NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_جوڑو_راستہ(scheme_lwc, پورٹ, h, راستہ_query,
					   fragment_lwc, nsurl);
			اگر (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				واپس NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;
			اگر (p)
				p->urld.visits = (غیرعلامتی صحیح)atoi(s);

			/* entry last use time */
			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp)) {
				توڑو;
			}
			اگر (p) {
				nsc_snptimet(s, لڑی_لمبائی(s) - 1, &p->urld.آخری_دورہ);
			}

			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;
			اگر (p)
				p->urld.type = (content_type)atoi(s);

			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;


			اگر (!fgets(s, زیادہ_سے_زیادہ_URL_لمبائی, fp))
				توڑو;
			لمبائی = لڑی_لمبائی(s) - 1;
			اگر (p && لمبائی > 0) {
				s[لمبائی] = '\0';
				p->urld.title = حافظہ_دو(لمبائی + 1);
				اگر (p->urld.title)
					حافظہ_نقل(p->urld.title, s, لمبائی + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#تعریف_منسوخ زیادہ_سے_زیادہ_URL_لمبائی

	واپس NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ثابت حرف *filename)
{
	FILE *fp;
	صحیح i;

	أكد(filename);

	fp = fopen(filename, "w");
	اگر (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		واپس NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_نسخہ);

	برائے (i = 0; i != عدد_تلاش_TREES; i++) {
		urldb_save_تلاش_درخت(تلاش_trees[i], fp);
	}

	fclose(fp);

	واپس NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_سیٹ_url_persistence(nsurl *url, bool persist)
{
	ساخت راستہ_ڈیٹا *p;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p) {
		واپس NSERROR_NOT_ملا;
	}

	p->persistent = persist;

	واپس NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_جوڑو_url(nsurl *url)
{
	ساخت host_part *h;
	ساخت راستہ_ڈیٹا *p;
	lwc_string *scheme;
	lwc_string *پورٹ;
	lwc_string *host;
	lwc_string *fragment;
	ثابت حرف *host_str;
	حرف *راستہ_query = تہی;
	size_t len;
	bool مطابقت;
	غیرعلامتی صحیح پورٹ_int;

	أكد(url);

	اگر (url_bloom == تہی)
		url_bloom = bloom_create(BLOOM_سائز);

	اگر (url_bloom != تہی) {
		uint32_t hash = nsurl_hash(url);
		bloom_داخل_کرو_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	اگر (nsurl_get(url, NSURL_راستہ | NSURL_QUERY, &راستہ_query, &len) !=
	    NSERROR_OK) {
		واپس false;
	}
	أكد(راستہ_query != تہی);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == تہی) {
		آزاد(راستہ_query);
		واپس false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != تہی) {
		host_str = lwc_string_ڈیٹا(host);
		lwc_string_unref(host);

	} ورنہ اگر (lwc_string_isequal(scheme, corestring_lwc_file, &مطابقت) ==
		   lwc_error_ok && مطابقت == true) {
		host_str = "localhost";

	} ورنہ {
		lwc_string_unref(scheme);
		آزاد(راستہ_query);
		واپس false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	پورٹ = nsurl_get_component(url, NSURL_پورٹ);
	اگر (پورٹ != تہی) {
		پورٹ_int = atoi(lwc_string_ڈیٹا(پورٹ));
		lwc_string_unref(پورٹ);
	} ورنہ {
		پورٹ_int = 0;
	}

	/* Get host entry */
	h = urldb_جوڑو_host(host_str);

	/* Get path entry */
	اگر (h != تہی) {
		p = urldb_جوڑو_راستہ(scheme,
				   پورٹ_int,
				   h,
				   راستہ_query,
				   fragment,
				   url);
	} ورنہ {
		p = تہی;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	واپس (p != تہی);
}


/* exported interface documented in content/urldb.h */
nserror urldb_سیٹ_url_title(nsurl *url, ثابت حرف *title)
{
	ساخت راستہ_ڈیٹا *p;
	حرف *عارضی;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (p == تہی) {
		واپس NSERROR_NOT_ملا;
	}

	/* copy the parameter if necessary */
	اگر (title != تہی) {
		عارضی = strdup(title);
		اگر (عارضی == تہی) {
			واپس NSERROR_NOMEM;
		}
	} ورنہ {
		عارضی = تہی;
	}

	آزاد(p->urld.title);
	p->urld.title = عارضی;

	واپس NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_سیٹ_url_content_type(nsurl *url, content_type type)
{
	ساخت راستہ_ڈیٹا *p;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p) {
		واپس NSERROR_NOT_ملا;
	}

	p->urld.type = type;

	واپس NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_دورہ_ڈیٹا(nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p) {
		واپس NSERROR_NOT_ملا;
	}

	p->urld.آخری_دورہ = time(تہی);
	p->urld.visits++;

	واپس NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
خالی urldb_reset_url_دورہ_ڈیٹا(nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس;

	p->urld.آخری_دورہ = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ثابت ساخت url_ڈیٹا *urldb_get_url_ڈیٹا(nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;
	ساخت url_اندرونی_ڈیٹا *u;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس تہی;

	u = &p->urld;

	واپس (ثابت ساخت url_ڈیٹا *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس تہی;

	واپس p->url;
}


/* exported interface documented in netsurf/url_db.h */
خالی urldb_سیٹ_auth_details(nsurl *url, ثابت حرف *realm, ثابت حرف *auth)
{
	ساخت راستہ_ڈیٹا *p, *pi;
	ساخت host_part *h;
	ساخت prot_space_ڈیٹا *space, *space_alloc;
	حرف *realm_alloc, *auth_alloc;
	bool مطابقت;

	أكد(url && realm && auth);

	/* add url, in case it's missing */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);

	اگر (!p)
		واپس;

	/* Search for host_part */
	برائے (pi = p; pi->والد != تہی; pi = pi->والد)
		;
	h = (ساخت host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	برائے (space = h->prot_space; space; space = space->اگلا) {
		اگر (!strcmp(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &مطابقت) == lwc_error_ok &&
		    مطابقت == true &&
		    space->پورٹ == p->پورٹ)
			توڑو;
	}

	اگر (space != تہی) {
		/* Overrule existing auth. */
		آزاد(space->auth);
		space->auth = strdup(auth);
	} ورنہ {
		/* Create a new protection space. */
		space = space_alloc = حافظہ_دو(حجم(ساخت prot_space_ڈیٹا));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		اگر (!space_alloc || !realm_alloc || !auth_alloc) {
			آزاد(space_alloc);
			آزاد(realm_alloc);
			آزاد(auth_alloc);
			واپس;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->پورٹ = p->پورٹ;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->اگلا = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ثابت حرف *urldb_get_auth_details(nsurl *url, ثابت حرف *realm)
{
	ساخت راستہ_ڈیٹا *p, *p_cur, *p_چوٹی;

	أكد(url);

	/* add to the db, so our lookup will work */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس تہی;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	برائے (p_cur = p; p_cur != تہی; p_چوٹی = p_cur, p_cur = p_cur->والد) {
		اگر (p_cur->prot_space) {
			واپس p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	اگر (realm != تہی) {
		ثابت ساخت host_part *h = (ثابت ساخت host_part *)p_چوٹی;
		ثابت ساخت prot_space_ڈیٹا *space;
		bool مطابقت;

		/* Search for a possible matching protection space. */
		برائے (space = h->prot_space; space != تہی;
		     space = space->اگلا) {
			اگر (!strcmp(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &مطابقت) ==
			    lwc_error_ok &&
			    مطابقت == true &&
			    space->پورٹ == p->پورٹ) {
				p->prot_space = space;
				واپس p->prot_space->auth;
			}
		}
	}

	واپس تہی;
}


/* exported interface documented in netsurf/url_db.h */
خالی urldb_سیٹ_cert_permissions(nsurl *url, bool permit)
{
	ساخت راستہ_ڈیٹا *p;
	ساخت host_part *h;

	أكد(url);

	/* add url, in case it's missing */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس;

	برائے (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ساخت host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;
	ثابت ساخت host_part *h;

	أكد(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس false;

	برائے (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ثابت ساخت host_part *)p;

	واپس h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_سیٹ_hsts_policy(ساخت nsurl *url, ثابت حرف *سرخی)
{
	ساخت راستہ_ڈیٹا *p;
	ساخت host_part *h;
	lwc_string *host;
	time_t now = time(تہی);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != تہی) {
		اگر (urldb_host_is_ip_پتہ(lwc_string_ڈیٹا(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			واپس true;
		} ورنہ اگر (lwc_string_لمبائی(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			واپس true;
		}

		lwc_string_unref(host);
	} ورنہ {
		/* No host part: ignore */
		واپس true;
	}

	/* add url, in case it's missing */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس false;

	برائے (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ساخت host_part *)p;
	اگر (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		واپس true;
	}

	error = http_parse_strict_transport_security(سرخی, &sts);
	اگر (error != NSERROR_OK) {
		/* Parse failed: ignore */
		واپس true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	اگر (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} ورنہ اگر ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	واپس true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(ساخت nsurl *url)
{
	ساخت راستہ_ڈیٹا *p;
	ثابت ساخت host_part *h;
	lwc_string *host;
	time_t now = time(تہی);

	أكد(url);

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host != تہی) {
		اگر (urldb_host_is_ip_پتہ(lwc_string_ڈیٹا(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			واپس false;
		} ورنہ اگر (lwc_string_لمبائی(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			واپس false;
		}

		lwc_string_unref(host);
	} ورنہ {
		/* No host part: not enabled */
		واپس false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس false;

	برائے (; p && p->والد; p = p->والد)
		/* do nothing */;
	أكد(p);

	h = (ثابت ساخت host_part *)p;

	/* Consult record for this host */
	اگر (h->hsts.expires > now) {
		/* Not expired */
		واپس true;
	}

	/* Consult parent domains */
	برائے (h = h->والد; h && h != &db_جڑ; h = h->والد) {
		اگر (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			واپس true;
		}
	}

	واپس false;
}


/* exported interface documented in netsurf/url_db.h */
خالی
urldb_iterate_partial(ثابت حرف *سابقہ,
		      bool (*callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا))
{
	حرف host[256];
	حرف buf[260]; /* max domain + "www." */
	ثابت حرف *slash, *scheme_sep;
	ساخت تلاش_گرہ *درخت;
	ثابت ساخت host_part *h;

	أكد(سابقہ && callback);

	/* strip scheme */
	scheme_sep = لڑی_تلاش(سابقہ, "://");
	اگر (scheme_sep)
		سابقہ = scheme_sep + 3;

	slash = strchr(سابقہ, '/');
	درخت = urldb_get_تلاش_درخت(سابقہ);

	اگر (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, حجم host, "%.*s",
			 (صحیح) (slash - سابقہ), سابقہ);

		h = urldb_تلاش_ڈھونڈو(درخت, host);
		اگر (!h) {
			صحیح len = slash - سابقہ;

			اگر (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, حجم buf, "www.%s", host);
				h = urldb_تلاش_ڈھونڈو(
					تلاش_trees[ST_DN + 'w' - 'a'],
					buf);
				اگر (!h)
					واپس;
			} ورنہ
				واپس;
		}

		اگر (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_راستہ(&h->paths, slash + 1,
						   callback);
		}

	} ورنہ {
		صحیح len = لڑی_لمبائی(سابقہ);

		/* looking for hosts */
		اگر (!urldb_iterate_partial_host(درخت, سابقہ, callback))
			واپس;

		اگر (len <= 3 || strncasecmp(سابقہ, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, حجم buf, "www.%s", سابقہ);
			اگر(!urldb_iterate_partial_host(
				   تلاش_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				واپس;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
خالی
urldb_iterate_entries(bool (*callback)(nsurl *url, ثابت ساخت url_ڈیٹا *ڈیٹا))
{
	صحیح i;

	أكد(callback);

	برائے (i = 0; i < عدد_تلاش_TREES; i++) {
		اگر (!urldb_iterate_entries_host(تلاش_trees[i],
						callback,
						تہی)) {
			توڑو;
		}
	}
}


/* exported interface documented in content/urldb.h */
خالی urldb_iterate_cookies(bool (*callback)(ثابت ساخت cookie_ڈیٹا *ڈیٹا))
{
	صحیح i;

	أكد(callback);

	برائے (i = 0; i < عدد_تلاش_TREES; i++) {
		اگر (!urldb_iterate_entries_host(تلاش_trees[i],
						تہی, callback))
			توڑو;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_سیٹ_cookie(ثابت حرف *سرخی, nsurl *url, nsurl *referer)
{
	ثابت حرف *cur = سرخی, *انتہا;
	lwc_string *راستہ, *host, *scheme;
	nsurl *urlt;
	bool مطابقت;

	أكد(url && سرخی);

	/* Get defragmented URL, as 'urlt' */
	اگر (nsurl_defragment(url, &urlt) != NSERROR_OK)
		واپس تہی;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == تہی) {
		nsurl_unref(urlt);
		واپس false;
	}

	راستہ = nsurl_get_component(url, NSURL_راستہ);
	اگر (راستہ == تہی) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		واپس false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	اگر (host == تہی) {
		lwc_string_unref(راستہ);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		واپس false;
	}

	اگر (referer) {
		lwc_string *rhost;

		/* Ensure that url's host name domain matches
		 * referer's (4.3.5) */
		rhost = nsurl_get_component(referer, NSURL_HOST);
		اگر (rhost == تہی) {
			جاؤ error;
		}

		/* Domain match host names */
		اگر (lwc_string_isequal(host, rhost, &مطابقت) == lwc_error_ok &&
		    مطابقت == false) {
			ثابت حرف *hptr;
			ثابت حرف *rptr;
			ثابت حرف *dot;
			ثابت حرف *host_ڈیٹا = lwc_string_ڈیٹا(host);
			ثابت حرف *rhost_ڈیٹا = lwc_string_ڈیٹا(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			اگر (urldb_host_is_ip_پتہ(host_ڈیٹا) ||
			    urldb_host_is_ip_پتہ(rhost_ڈیٹا)) {
				/* IP address, so no partial match */
				lwc_string_unref(rhost);
				جاؤ error;
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

			hptr = host_ڈیٹا + lwc_string_لمبائی(host) - 1;
			rptr = rhost_ڈیٹا + lwc_string_لمبائی(rhost) - 1;

			/* 1 */
			جب (hptr >= host_ڈیٹا && rptr >= rhost_ڈیٹا) {
				اگر (*hptr != *rptr)
					توڑو;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			جب (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			اگر (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == تہی ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				جاؤ error;
			}
		}

		lwc_string_unref(rhost);
	}

	انتہا = cur + لڑی_لمبائی(cur) - 2 /* Trailing CRLF */;

	کرو {
		ساخت cookie_اندرونی_ڈیٹا *c;
		حرف *dot;
		size_t len;
#اگر_تعریف_شدہ WITH_NSPSL
		ثابت حرف *suffix;
#اگر_ختم

		c = urldb_parse_cookie(url, &cur);
		اگر (!c) {
			/* failed => stop parsing */
			جاؤ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		اگر (!c->name || !c->قدر) {
			urldb_free_cookie(c);
			جاؤ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = لڑی_لمبائی(c->راستہ);
		اگر (len > lwc_string_لمبائی(راستہ) ||
		    لڑی_موازنہ(c->راستہ, lwc_string_ڈیٹا(راستہ),
			    len) != 0) {
			urldb_free_cookie(c);
			جاؤ error;
		}

#اگر_تعریف_شدہ WITH_NSPSL
		/* check domain is not a public suffix */
		dot = c->domain;
		اگر (*dot == '.') {
			dot++;
		}
		suffix = nspsl_getpublicsuffix(dot);
		اگر (suffix == تہی) {
			NSLOG(netsurf, INFO,
			      "domain %s was a public suffix domain", dot);
			urldb_free_cookie(c);
			جاؤ error;
		}
#ورنہ
		/* 4.3.2:ii Cookie domain must contain embedded dots */
		dot = strchr(c->domain + 1, '.');
		اگر (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			جاؤ error;
		}
#اگر_ختم

		/* Domain match fetch host with cookie domain */
		اگر (strcasecmp(lwc_string_ڈیٹا(host), c->domain) != 0) {
			صحیح hlen, dlen;
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
			اگر (urldb_host_is_ip_پتہ(lwc_string_ڈیٹا(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				جاؤ error;
			}

			hlen = lwc_string_لمبائی(host);
			dlen = لڑی_لمبائی(c->domain);

			اگر (hlen <= dlen && hlen != dlen - 1) {
				/* Partial match not possible */
				urldb_free_cookie(c);
				جاؤ error;
			}

			اگر (hlen == dlen - 1) {
				/* Relax matching to allow
				 * host a.com to match .a.com */
				domain++;
				dlen--;
			}

			اگر (strcasecmp(lwc_string_ڈیٹا(host) + (hlen - dlen),
				       domain)) {
				urldb_free_cookie(c);
				جاؤ error;
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
		اگر (!urldb_داخل_کرو_cookie(c, scheme, urlt))
			جاؤ error;
	} جب (cur < انتہا);

	lwc_string_unref(host);
	lwc_string_unref(راستہ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	واپس true;

error:
	lwc_string_unref(host);
	lwc_string_unref(راستہ);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	واپس false;
}


/* exported interface documented in content/urldb.h */
حرف *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ثابت ساخت راستہ_ڈیٹا *p, *q;
	ثابت ساخت host_part *h;
	lwc_string *راستہ_lwc;
	ساخت cookie_اندرونی_ڈیٹا *c;
	صحیح گنتی = 0, نسخہ = COOKIE_RFC2965;
	ساخت cookie_اندرونی_ڈیٹا **matched_cookies;
	صحیح matched_cookies_سائز = 20;
	ثابت حرف *راستہ;
	حرف *ret;
	lwc_string *scheme;
	time_t now;
	bool مطابقت;

	أكد(url != تہی);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_جوڑو_url(url);

	p = urldb_ڈھونڈو_url(url);
	اگر (!p)
		واپس تہی;

	scheme = p->scheme;

	matched_cookies = حافظہ_دو(matched_cookies_سائز *
				 حجم(ساخت cookie_اندرونی_ڈیٹا *));
	اگر (!matched_cookies)
		واپس تہی;

#تعریف_کرو GROW_MATCHED_COOKIES						\
	کرو {								\
		اگر (گنتی == matched_cookies_سائز) {			\
			ساخت cookie_اندرونی_ڈیٹا **عارضی;		\
			عارضی = realloc(matched_cookies,			\
				       (matched_cookies_سائز + 20) *	\
				       حجم(ساخت cookie_اندرونی_ڈیٹا *)); \
									\
			اگر (عارضی == تہی) {				\
				آزاد(matched_cookies);			\
				واپس تہی;				\
			}						\
									\
			matched_cookies = عارضی;				\
			matched_cookies_سائز += 20;			\
		}							\
	} جب(0)

	راستہ_lwc = nsurl_get_component(url, NSURL_راستہ);
	اگر (راستہ_lwc == تہی) {
		آزاد(matched_cookies);
		واپس تہی;
	}
	راستہ = lwc_string_ڈیٹا(راستہ_lwc);
	lwc_string_unref(راستہ_lwc);

	now = time(تہی);

	اگر (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		برائے (q = p->والد->children; q; q = q->اگلا) {
			اگر (strcmp(q->segment, p->segment))
				جاری;

			/* Consider all cookies associated with
			 * this exact path */
			برائے (c = q->cookies; c; c = c->اگلا) {
				اگر (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					جاری;

				اگر (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &مطابقت) &&
				    مطابقت == false)
					/* secure cookie for insecure host.
					 * ignore */
					جاری;

				اگر (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					جاری;

				matched_cookies[گنتی++] = c;

				GROW_MATCHED_COOKIES;

				اگر (c->نسخہ < (غیرعلامتی صحیح)نسخہ)
					نسخہ = c->نسخہ;

				c->آخری_مستعمل = now;

				cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	برائے (p = p->والد; p; p = p->والد) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		برائے (q = p->children; q; q = q->اگلا) {
			اگر (*(q->segment) != '\0')
				جاری;

			برائے (c = q->cookies; c; c = c->اگلا) {
				اگر (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					جاری;

				اگر (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &مطابقت) &&
				    مطابقت == false)
					/* Secure cookie for insecure server
					 * => ignore */
					جاری;

				matched_cookies[گنتی++] = c;

				GROW_MATCHED_COOKIES;

				اگر (c->نسخہ < (غیرعلامتی صحیح) نسخہ)
					نسخہ = c->نسخہ;

				c->آخری_مستعمل = now;

				cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
			}
		}

		اگر (!p->والد) {
			/* No parent, so bail here. This can't go in
			 * the loop exit condition as we also want to
			 * process the top-level node.
			 *
			 * If p->parent is NULL then p->cookies are
			 * the domain cookies and thus we don't even
			 * try matching against them.
			 */
			توڑو;
		}

		/* Consider p itself - may be the result of Path=/foo */
		برائے (c = p->cookies; c; c = c->اگلا) {
			اگر (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				جاری;

			/* Ensure cookie path is a prefix of the resource */
			اگر (لڑی_موازنہ(c->راستہ, راستہ, لڑی_لمبائی(c->راستہ)) != 0)
				/* paths don't match => ignore */
				جاری;

			اگر (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &مطابقت) &&
			    مطابقت == false)
				/* Secure cookie for insecure server
				 * => ignore */
				جاری;

			matched_cookies[گنتی++] = c;

			GROW_MATCHED_COOKIES;

			اگر (c->نسخہ < (غیرعلامتی صحیح) نسخہ)
				نسخہ = c->نسخہ;

			c->آخری_مستعمل = now;

			cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	برائے (h = (ثابت ساخت host_part *)p; h && h != &db_جڑ;
	     h = h->والد) {
		برائے (c = h->paths.cookies; c; c = c->اگلا) {
			اگر (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				جاری;

			/* Ensure cookie path is a prefix of the resource */
			اگر (لڑی_موازنہ(c->راستہ, راستہ, لڑی_لمبائی(c->راستہ)) != 0)
				/* paths don't match => ignore */
				جاری;

			اگر (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &مطابقت) &&
			    مطابقت == false)
				/* secure cookie for insecure host. ignore */
				جاری;

			matched_cookies[گنتی++] = c;

			GROW_MATCHED_COOKIES;

			اگر (c->نسخہ < (غیرعلامتی صحیح)نسخہ)
				نسخہ = c->نسخہ;

			c->آخری_مستعمل = now;

			cookie_manager_جوڑو((ساخت cookie_ڈیٹا *)c);
		}
	}
#تعریف_منسوخ GROW_MATCHED_COOKIES

	اگر (گنتی == 0) {
		/* No cookies found */
		آزاد(matched_cookies);
		واپس تہی;
	}

	اگر (نسخہ == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, گنتی);
	} ورنہ {
		ret = get_rfc2965_cookie(matched_cookies, گنتی, نسخہ);
	}
	آزاد(matched_cookies);

	واپس ret;
}


/* exported interface documented in content/urldb.h */
خالی urldb_delete_cookie(ثابت حرف *domain, ثابت حرف *راستہ,
			 ثابت حرف *name)
{
	urldb_delete_cookie_hosts(domain, راستہ, name, &db_جڑ);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_بھرو_cookies(ثابت حرف *filename)
{
	FILE *fp;
	حرف s[16*1024];

	أكد(filename);

	fp = fopen(filename, "r");
	اگر (!fp)
		واپس NSERROR_NOT_ملا;

#تعریف_کرو ڈھونڈو_T {				\
		برائے (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		اگر (p >= انتہا) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			جاری;		\
		}				\
		*p++ = '\0';			\
	}

#تعریف_کرو SKIP_T {				\
		برائے (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		اگر (p >= انتہا) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			جاری;		\
		}				\
	}

	جب (fgets(s, حجم s, fp)) {
		حرف *p = s, *انتہا = 0,
			*domain, *راستہ, *name, *قدر, *scheme, *url,
			*comment;
		صحیح نسخہ, domain_specified, راستہ_specified,
			secure, http_only, no_destroy, قدر_quoted;
		time_t expires, آخری_مستعمل;
		ساخت cookie_اندرونی_ڈیٹا *c;

		اگر(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			جاری;

		s[لڑی_لمبائی(s) - 1] = '\0'; /* lose terminating newline */
		انتہا = s + لڑی_لمبائی(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		اگر (strncasecmp(s, "Version:", 8) == 0) {
			ڈھونڈو_T; SKIP_T; loaded_cookie_file_نسخہ = atoi(p);

			اگر (loaded_cookie_file_نسخہ <
			    MIN_COOKIE_FILE_نسخہ) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				توڑو;
			}

			جاری;
		} ورنہ اگر (loaded_cookie_file_نسخہ == 0) {
			/* Haven't yet seen version; skip this input */
			جاری;
		}

		/* One cookie/line */

		/* Parse input */
		ڈھونڈو_T; نسخہ = atoi(s);
		SKIP_T; domain = p; ڈھونڈو_T;
		SKIP_T; domain_specified = atoi(p); ڈھونڈو_T;
		SKIP_T; راستہ = p; ڈھونڈو_T;
		SKIP_T; راستہ_specified = atoi(p); ڈھونڈو_T;
		SKIP_T; secure = atoi(p); ڈھونڈو_T;
		اگر (loaded_cookie_file_نسخہ > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); ڈھونڈو_T;
		} ورنہ {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); ڈھونڈو_T;
		SKIP_T; آخری_مستعمل = (time_t)atoi(p); ڈھونڈو_T;
		SKIP_T; no_destroy = atoi(p); ڈھونڈو_T;
		SKIP_T; name = p; ڈھونڈو_T;
		SKIP_T; قدر = p; ڈھونڈو_T;
		اگر (loaded_cookie_file_نسخہ > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	قدر_quoted = atoi(p); ڈھونڈو_T;
		} ورنہ {
			قدر_quoted = 0;
		}
		SKIP_T; scheme = p; ڈھونڈو_T;
		SKIP_T; url = p; ڈھونڈو_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		برائے (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		أكد(p <= انتہا);

		/* Now create cookie */
		c = حافظہ_دو(حجم(ساخت cookie_اندرونی_ڈیٹا));
		اگر (!c)
			توڑو;

		c->name = strdup(name);
		c->قدر = strdup(قدر);
		c->قدر_was_quoted = قدر_quoted;
		c->comment = strdup(comment);
		c->domain_from_سیٹ = domain_specified;
		c->domain = strdup(domain);
		c->راستہ_from_سیٹ = راستہ_specified;
		c->راستہ = strdup(راستہ);
		c->expires = expires;
		c->آخری_مستعمل = آخری_مستعمل;
		c->secure = secure;
		c->http_only = http_only;
		c->نسخہ = نسخہ;
		c->no_destroy = no_destroy;

		اگر (!(c->name && c->قدر && c->comment &&
		      c->domain && c->راستہ)) {
			urldb_free_cookie(c);
			توڑو;
		}

		اگر (c->domain[0] != '.') {
			lwc_string *scheme_lwc = تہی;
			nsurl *url_nsurl = تہی;

			أكد(scheme[0] != 'u');

			اگر (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				توڑو;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			اگر (!urldb_داخل_کرو_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				توڑو;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} ورنہ {
			اگر (!urldb_داخل_کرو_cookie(c, تہی, تہی)) {
				/* Cookie freed for us */
				توڑو;
			}
		}
	}

#تعریف_منسوخ SKIP_T
#تعریف_منسوخ ڈھونڈو_T

	fclose(fp);

	واپس NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ثابت حرف *filename)
{
	FILE *fp;
	صحیح cookie_file_نسخہ = max(loaded_cookie_file_نسخہ,
				      COOKIE_FILE_نسخہ);

	أكد(filename);

	fp = fopen(filename, "w");
	اگر (!fp)
		واپس NSERROR_NOT_ملا;

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
		cookie_file_نسخہ);
	fprintf(fp, "Version:\t%d\n", cookie_file_نسخہ);

	urldb_save_cookie_hosts(fp, &db_جڑ);

	fclose(fp);

	واپس NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
خالی urldb_dump(خالی)
{
	صحیح i;

	urldb_dump_hosts(&db_جڑ);

	برائے (i = 0; i != عدد_تلاش_TREES; i++) {
		urldb_dump_تلاش(تلاش_trees[i], 0);
	}
}
