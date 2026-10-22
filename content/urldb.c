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
#यदि_परिभाषितम् WITH_NSPSL
#अन्तर्भाव <nspsl.h>
#यदि_अन्तः

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
संरचना cookie_आन्तरिकम्_दत्त {
	संरचना cookie_आन्तरिकम्_दत्त *पूर्व;	/**< Previous in list */
	संरचना cookie_आन्तरिकम्_दत्त *अग्र;	/**< Next in list */

	वर्ण *name;		/**< Cookie name */
	वर्ण *मानम्;		/**< Cookie value */
	bool मानम्_was_quoted;	/**< Value was quoted in Set-Cookie: */
	वर्ण *comment;		/**< Cookie comment */
	bool domain_from_समुच्चयः;	/**< Domain came from Set-Cookie: header */
	वर्ण *domain;		/**< Domain */
	bool मार्गः_from_समुच्चयः;	/**< Path came from Set-Cookie: header */
	वर्ण *मार्गः;		/**< Path */
	time_t expires;		/**< Expiry timestamp, or -1 for session */
	time_t चरमः_प्रयुक्तम्;	/**< Last used time */
	bool secure;		/**< Only send for HTTPS requests */
	bool http_only;		/**< Only expose to HTTP(S) requests */
	गणना cookie_संस्करणम् संस्करणम्;	/**< Specification compliance */
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
संरचना prot_space_दत्त {
	/**
	 * URL scheme of canonical hostname of this protection space.
	 */
	lwc_string *scheme;
	/**
	 * Port number of canonical hostname of this protection
	 * space. When 0, it means the default port for given scheme,
	 * i.e. 80 (http), 443 (https).
	 */
	अचिह्नित पूर्णाङ्क पत्तनम्;
	/** Protection realm */
	वर्ण *realm;

	/**
	 * Authentication details for this protection space in form
	 * username:password
	 */
	वर्ण *auth;
	/** Next sibling */
	संरचना prot_space_दत्त *अग्र;
};


/**
 * meta data about a url
 *
 * \warning must be kept in sync with url_data structure in netsurf/url_db.h
 */
संरचना url_आन्तरिकम्_दत्त {
	वर्ण *title;		/**< Resource title */
	अचिह्नित पूर्णाङ्क visits;	/**< Visit count */
	time_t चरमः_दर्शय;	/**< Last visit time */
	content_type type;	/**< Type of resource */
};


/**
 * data entry for url
 */
संरचना मार्गः_दत्त {
	nsurl *url;		/**< Full URL */
	lwc_string *scheme;	/**< URL scheme for data */
	अचिह्नित पूर्णाङ्क पत्तनम्;	/**< Port number for data. When 0, it means
				 * the default port for given scheme, i.e.
				 * 80 (http), 443 (https). */
	वर्ण *segment;		/**< Path segment for this node */
	अचिह्नित पूर्णाङ्क frag_cnt;	/**< Number of entries in path_data::fragment */
	वर्ण **fragment;	/**< Array of fragments */
	bool persistent;	/**< This entry should persist */

	संरचना url_आन्तरिकम्_दत्त urld;	/**< URL data for resource */

	/**
	 * Protection space to which this resource belongs too. Can be
	 * NULL when it does not belong to a protection space or when
	 * it is not known. No ownership (is with struct host_part::prot_space).
	 */
	ध्रुव संरचना prot_space_दत्त *prot_space;
	/** Cookies associated with resource */
	संरचना cookie_आन्तरिकम्_दत्त *cookies;
	/** Last cookie in list */
	संरचना cookie_आन्तरिकम्_दत्त *cookies_अन्तः;

	संरचना मार्गः_दत्त *अग्र;	/**< Next sibling */
	संरचना मार्गः_दत्त *पूर्व;	/**< Previous sibling */
	संरचना मार्गः_दत्त *parent; /**< Parent path segment */
	संरचना मार्गः_दत्त *children; /**< Child path segments */
	संरचना मार्गः_दत्त *चरमः; /**< Last child */
};

संरचना hsts_दत्त {
	time_t expires; /**< Expiry time */
	bool include_sub_domains; /**< Whether to include subdomains */
};

संरचना host_part {
	/**
	 * Known paths on this host. This _must_ be first so that
	 * struct host_part *h = (struct host_part *)mypath; works
	 */
	संरचना मार्गः_दत्त paths;
	/**
	 * Allow access to SSL protected resources on this host
	 * without verifying certificate authenticity
	 */
	bool permit_invalid_certs;
	/* HSTS data */
	संरचना hsts_दत्त hsts;

	/**
	 * Part of host string
	 */
	वर्ण *part;

	/**
	 * Linked list of all known proctection spaces known for this
	 * host and all its schems and ports.
	 */
	संरचना prot_space_दत्त *prot_space;

	संरचना host_part *अग्र;	/**< Next sibling */
	संरचना host_part *पूर्व;	/**< Previous sibling */
	संरचना host_part *parent; /**< Parent host part */
	संरचना host_part *children; /**< Child host parts */
};


/**
 * search index node
 */
संरचना अन्वेषणम्_ग्रन्थिः {
	ध्रुव संरचना host_part *दत्त;	/**< Host tree entry */

	अचिह्नित पूर्णाङ्क level;		/**< Node level */

	संरचना अन्वेषणम्_ग्रन्थिः *वामः;	/**< Left subtree */
	संरचना अन्वेषणम्_ग्रन्थिः *दक्षिणः;	/**< Right subtree */
};

/** Root database handle */
स्थिर संरचना host_part db_मूलम्;

/** Search trees - one per letter + 1 for IPs + 1 for Everything Else */
#परिभाषय अङ्कः_अन्वेषणम्_TREES 28
#परिभाषय ST_IP 0
#परिभाषय ST_EE 1
#परिभाषय ST_DN 2
स्थिर संरचना अन्वेषणम्_ग्रन्थिः रिक्तानि = { 0, 0, &रिक्तानि, &रिक्तानि };
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *अन्वेषणम्_trees[अङ्कः_अन्वेषणम्_TREES] = {
	&रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि,
	&रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि,
	&रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि,
	&रिक्तानि, &रिक्तानि, &रिक्तानि, &रिक्तानि
};

/** Minimum cookie database file version */
#परिभाषय MIN_COOKIE_FILE_संस्करणम् 100
/** Current cookie database file version */
#परिभाषय COOKIE_FILE_संस्करणम् 102
/** loaded cookie file version */
स्थिर पूर्णाङ्क loaded_cookie_file_संस्करणम्;

/** Minimum URL database file version */
#परिभाषय MIN_URL_FILE_संस्करणम् 106
/** Current URL database file version */
#परिभाषय URL_FILE_संस्करणम् 107

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
#परिभाषय BLOOM_आकार (1024 * 32)


/**
 * write a time_t to a file portably
 *
 * \param fp File to write to
 * \param val the unix time value to output
 * \return NSERROR_OK on success
 */
स्थिर nserror urldb_write_timet(FILE *fp, time_t val)
{
	पूर्णाङ्क use;
	वर्ण op[32];

	use = nsc_sntimet(op, 32, &val);
	यदि (use == 0) {
		fprintf(fp, "%i\n", (पूर्णाङ्क)val);
	} अन्यथा {
		fprintf(fp, "%.*s\n", use, op);
	}
	प्रत्यागम NSERROR_OK;
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
urldb_write_paths(ध्रुव संरचना मार्गः_दत्त *parent,
		  ध्रुव वर्ण *host,
		  FILE *fp,
		  वर्ण **मार्गः,
		  पूर्णाङ्क *मार्गः_alloc,
		  पूर्णाङ्क *मार्गः_प्रयुक्तम्,
		  time_t expiry)
{
	ध्रुव संरचना मार्गः_दत्त *p = parent;
	पूर्णाङ्क i;

	कुरु {
		पूर्णाङ्क seglen = p->segment != रिक्त ? वाक्यदैर्घ्यम्(p->segment) : 0;
		पूर्णाङ्क len = *मार्गः_प्रयुक्तम् + seglen + 1;

		यदि (*मार्गः_alloc < len) {
			वर्ण *अस्थायी;
			अस्थायी = realloc(*मार्गः,
				       (len > 64) ? len : *मार्गः_alloc + 64);
			यदि (!अस्थायी) {
				प्रत्यागम;
			}
			*मार्गः = अस्थायी;
			*मार्गः_alloc = (len > 64) ? len : *मार्गः_alloc + 64;
		}

		यदि (p->segment != रिक्त) {
			स्मृतिं_प्रतिलिख(*मार्गः + *मार्गः_प्रयुक्तम् - 1, p->segment, seglen);
		}

		यदि (p->children != रिक्त) {
			(*मार्गः)[*मार्गः_प्रयुक्तम् + seglen - 1] = '/';
			(*मार्गः)[*मार्गः_प्रयुक्तम् + seglen] = '\0';
		} अन्यथा {
			(*मार्गः)[*मार्गः_प्रयुक्तम् + seglen - 1] = '\0';
			len -= 1;
		}

		*मार्गः_प्रयुक्तम् = len;

		यदि (p->children != रिक्त) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* leaf node */
			यदि (p->persistent ||
			    ((p->urld.चरमः_दर्शय > expiry) &&
			     (p->urld.visits > 0))) {
				fprintf(fp, "%s\n", lwc_string_दत्त(p->scheme));

				यदि (p->पत्तनम्) {
					fprintf(fp,"%d\n", p->पत्तनम्);
				} अन्यथा {
					fprintf(fp, "\n");
				}

				fprintf(fp, "%s\n", *मार्गः);

				/** \todo handle fragments? */

				/* number of visits */
				fprintf(fp, "%i\n", p->urld.visits);

				/* time entry was last used */
				urldb_write_timet(fp, p->urld.चरमः_दर्शय);

				/* entry type */
				fprintf(fp, "%i\n", (पूर्णाङ्क)p->urld.type);

				fprintf(fp, "\n");

				यदि (p->urld.title) {
					uint8_t *s = (uint8_t *) p->urld.title;

					प्रति (i = 0; s[i] != '\0'; i++)
						यदि (s[i] < 32)
							s[i] = ' ';
					प्रति (--i; ((i > 0) && (s[i] == ' '));
					     i--)
						s[i] = '\0';
					fprintf(fp, "%s\n", p->urld.title);
				} अन्यथा {
					fprintf(fp, "\n");
				}
			}

			/* Now, find next node to process. */
			यावत् (p != parent) {
				पूर्णाङ्क seglen = p->segment != रिक्त
					? वाक्यदैर्घ्यम्(p->segment) : 0;

				/* Remove our segment from the path */
				*मार्गः_प्रयुक्तम् -= seglen;
				(*मार्गः)[*मार्गः_प्रयुक्तम् - 1] = '\0';

				यदि (p->अग्र != रिक्त) {
					/* Have a sibling, process that */
					p = p->अग्र;
					भङ्ग;
				}

				/* Going up, so remove '/' */
				*मार्गः_प्रयुक्तम् -= 1;
				(*मार्गः)[*मार्गः_प्रयुक्तम् - 1] = '\0';

				/* Ascend tree */
				p = p->parent;
			}
		}
	} यावत् (p != parent);
}


/**
 * Count number of URLs associated with a host
 *
 * \param root Root of path data tree
 * \param expiry Expiry time for URLs
 * \param count Pointer to count
 */
स्थिर शून्य
urldb_गणनम्_urls(ध्रुव संरचना मार्गः_दत्त *मूलम्,
		 time_t expiry,
		 अचिह्नित पूर्णाङ्क *गणनम्)
{
	ध्रुव संरचना मार्गः_दत्त *p = मूलम्;

	कुरु {
		यदि (p->children != रिक्त) {
			/* Drill down into children */
			p = p->children;
		} अन्यथा {
			/* No more children, increment count if required */
			यदि (p->persistent ||
			    ((p->urld.चरमः_दर्शय > expiry) &&
			     (p->urld.visits > 0))) {
				(*गणनम्)++;
			}

			/* Now, find next node to process. */
			यावत् (p != मूलम्) {
				यदि (p->अग्र != रिक्त) {
					/* Have a sibling, process that */
					p = p->अग्र;
					भङ्ग;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} यावत् (p != मूलम्);
}


/**
 * Save a search (sub)tree
 *
 * \param parent root node of search tree to save.
 * \param fp File to write to
 */
स्थिर शून्य urldb_save_अन्वेषणम्_वृक्षः(संरचना अन्वेषणम्_ग्रन्थिः *parent, FILE *fp)
{
	वर्ण host[256];
	ध्रुव संरचना host_part *h;
	अचिह्नित पूर्णाङ्क मार्गः_गणनम् = 0;
	वर्ण *मार्गः, *p, *अन्तः;
	पूर्णाङ्क मार्गः_alloc = 64, मार्गः_प्रयुक्तम् = 1;
	time_t expiry, hsts_expiry = 0;
	पूर्णाङ्क hsts_include_subdomains = 0;

	expiry = time(रिक्त) - ((60 * 60 * 24) * nsoption_int(expire_url));

	यदि (parent == &रिक्तानि)
		प्रत्यागम;

	urldb_save_अन्वेषणम्_वृक्षः(parent->वामः, fp);

	मार्गः = स्मृतिं_देहि(मार्गः_alloc);
	यदि (!मार्गः)
		प्रत्यागम;

	मार्गः[0] = '\0';

	प्रति (h = parent->दत्त, p = host, अन्तः = host + परिमाण host;
	     h && h != &db_मूलम् && p < अन्तः; h = h->parent) {
		पूर्णाङ्क written = snprintf(p, अन्तः - p, "%s%s", h->part,
				       (h->parent && h->parent->parent) ? "." : "");
		यदि (written < 0) {
			मुक्त(मार्गः);
			प्रत्यागम;
		}
		p += written;
	}

	h = parent->दत्त;
	यदि (h && h->hsts.expires > expiry) {
		hsts_expiry = h->hsts.expires;
		hsts_include_subdomains = h->hsts.include_sub_domains;
	}

	urldb_गणनम्_urls(&parent->दत्त->paths, expiry, &मार्गः_गणनम्);

	यदि (मार्गः_गणनम् > 0) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "%i\n", मार्गः_गणनम्);

		urldb_write_paths(&parent->दत्त->paths, host, fp,
				  &मार्गः, &मार्गः_alloc, &मार्गः_प्रयुक्तम्, expiry);
	} अन्यथा यदि (hsts_expiry) {
		fprintf(fp, "%s %i ", host, hsts_include_subdomains);
		urldb_write_timet(fp, hsts_expiry);
		fprintf(fp, "0\n");
	}

	मुक्त(मार्गः);

	urldb_save_अन्वेषणम्_वृक्षः(parent->दक्षिणः, fp);
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
urldb_iterate_entries_मार्गः(ध्रुव संरचना मार्गः_दत्त *parent,
		bool (*url_callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त),
		bool (*cookie_callback)(ध्रुव संरचना cookie_दत्त *दत्त))
{
	ध्रुव संरचना मार्गः_दत्त *p = parent;
	ध्रुव संरचना cookie_दत्त *c;

	कुरु {
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
				ध्रुव संरचना url_आन्तरिकम्_दत्त *u = &p->urld;

				प्रतिज्ञा(p->url);

				यदि (!url_callback(p->url,
						  (ध्रुव संरचना url_दत्त *) u))
					प्रत्यागम false;
			} अन्यथा {
				c = (ध्रुव संरचना cookie_दत्त *)p->cookies;
				प्रति (; c != रिक्त; c = c->अग्र) {
					यदि (!cookie_callback(c))
						प्रत्यागम false;
				}
			}

			/* Now, find next node to process. */
			यावत् (p != parent) {
				यदि (p->अग्र != रिक्त) {
					/* Have a sibling, process that */
					p = p->अग्र;
					भङ्ग;
				}

				/* Ascend tree */
				p = p->parent;
			}
		}
	} यावत् (p != parent);

	प्रत्यागम true;
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
स्थिर bool urldb_host_is_ip_स्थानाङ्कः(ध्रुव वर्ण *host)
{
	संरचना अन्तर्_addr ipv4;
	size_t host_len = वाक्यदैर्घ्यम्(host);
	ध्रुव वर्ण *sane_host;
	ध्रुव वर्ण *slash;
#यदि_अपरिभाषितम् NO_IPV6
	संरचना in6_addr ipv6;
	वर्ण ipv6_addr[64];
	अचिह्नित पूर्णाङ्क ipv6_addr_len;
#यदि_अन्तः
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

	यदि (strspn(sane_host, "0123456789abcdefABCDEF[].:") < host_len)
		गच्छ बहिर्गमनम्_false;

	यदि (inet_aton(sane_host, &ipv4) != 0) {
		/* This can only be a sane IPv4 address if it contains 3 dots.
		 * Helpfully, inet_aton is happy to treat "a", "a.b", "a.b.c",
		 * and "a.b.c.d" as valid IPv4 address strings where we only
		 * support the full, dotted-quad, form.
		 */
		पूर्णाङ्क अङ्कः_dots = 0;
		size_t index;

		प्रति (index = 0; index < host_len; index++) {
			यदि (sane_host[index] == '.')
				अङ्कः_dots++;
		}

		यदि (अङ्कः_dots == 3)
			गच्छ बहिर्गमनम्_true;
		अन्यथा
			गच्छ बहिर्गमनम्_false;
	}

#यदि_अपरिभाषितम् NO_IPV6
	यदि ((host_len < 6) ||
	    (sane_host[0] != '[') ||
	    (sane_host[host_len - 1] != ']')) {
		गच्छ बहिर्गमनम्_false;
	}

	ipv6_addr_len = host_len - 2;
	यदि (ipv6_addr_len >= परिमाण(ipv6_addr)) {
		ipv6_addr_len = परिमाण(ipv6_addr) - 1;
	}
	strncpy(ipv6_addr, sane_host + 1, ipv6_addr_len);
	ipv6_addr[ipv6_addr_len] = '\0';

	यदि (inet_pton(AF_INET6, ipv6_addr, &ipv6) == 1)
		गच्छ बहिर्गमनम्_true;
#यदि_अन्तः

बहिर्गमनम्_false:
	यदि (slash != रिक्त) मुक्त((शून्य *)sane_host);
	प्रत्यागम false;

बहिर्गमनम्_true:
	यदि (slash != रिक्त) मुक्त((शून्य *)sane_host);
	प्रत्यागम true;
}


/**
 * Compare host_part with prefix
 *
 * \param a host part
 * \param b prefix
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क urldb_अन्वेषणम्_मेलनम्_उपसर्गः(ध्रुव संरचना host_part *a, ध्रुव वर्ण *b)
{
	ध्रुव वर्ण *अन्तः, *dot;
	पूर्णाङ्क plen, ret;

	प्रतिज्ञा(a && a != &db_मूलम् && b);

	यदि (urldb_host_is_ip_स्थानाङ्कः(b)) {
		/* IP address */
		प्रत्यागम strncasecmp(a->part, b, वाक्यदैर्घ्यम्(b));
	}

	अन्तः = b + वाक्यदैर्घ्यम्(b) + 1;

	यावत् (b < अन्तः && a && a != &db_मूलम्) {
		dot = strchr(b, '.');
		यदि (!dot) {
			/* last segment */
			dot = अन्तः - 1;
		}

		/* Compare strings (length limited) */
		यदि ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			प्रत्यागम ret;

		/* The strings matched */
		यदि (dot < अन्तः - 1) {
			/* Consider segment lengths only in the case
			 * where the prefix contains segments */
			plen = वाक्यदैर्घ्यम्(a->part);
			यदि (plen > dot - b) {
				/* len(a) > len(b) */
				प्रत्यागम 1;
			} अन्यथा यदि (plen < dot - b) {
				/* len(a) < len(b) */
				प्रत्यागम -1;
			}
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_मूलम् && b >= अन्तः) {
		/* len(a) > len(b) => prefix matches */
		प्रत्यागम 0;
	} अन्यथा यदि ((!a || a == &db_मूलम्) && b < अन्तः) {
		/* len(a) < len(b) => prefix does not match */
		प्रत्यागम -1;
	}

	/* Identical */
	प्रत्यागम 0;
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
urldb_iterate_partial_host(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्,
		ध्रुव वर्ण *उपसर्गः,
		bool (*callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त))
{
	पूर्णाङ्क c;

	प्रतिज्ञा(मूलम् && उपसर्गः && callback);

	यदि (मूलम् == &रिक्तानि)
		प्रत्यागम true;

	c = urldb_अन्वेषणम्_मेलनम्_उपसर्गः(मूलम्->दत्त, उपसर्गः);

	यदि (c > 0) {
		/* No match => look in left subtree */
		प्रत्यागम urldb_iterate_partial_host(मूलम्->वामः,
						  उपसर्गः,
						  callback);
	} अन्यथा यदि (c < 0) {
		/* No match => look in right subtree */
		प्रत्यागम urldb_iterate_partial_host(मूलम्->दक्षिणः,
						  उपसर्गः,
						  callback);
	} अन्यथा {
		/* Match => iterate over l/r subtrees & process this node */
		यदि (!urldb_iterate_partial_host(मूलम्->वामः,
						उपसर्गः,
						callback)) {
			प्रत्यागम false;
		}

		यदि (मूलम्->दत्त->paths.children) {
			/* and extract all paths attached to this host */
			यदि (!urldb_iterate_entries_मार्गः(&मूलम्->दत्त->paths,
							callback,
							रिक्त)) {
				प्रत्यागम false;
			}
		}

		यदि (!urldb_iterate_partial_host(मूलम्->दक्षिणः,
						उपसर्गः,
						callback)) {
			प्रत्यागम false;
		}
	}

	प्रत्यागम true;
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
urldb_iterate_partial_मार्गः(ध्रुव संरचना मार्गः_दत्त *parent,
		ध्रुव वर्ण *उपसर्गः,
		bool (*callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त))
{
	ध्रुव संरचना मार्गः_दत्त *p = parent->children;
	ध्रुव वर्ण *slash, *अन्तः = उपसर्गः + वाक्यदैर्घ्यम्(उपसर्गः);

	कुरु {
		slash = strchr(उपसर्गः, '/');
		यदि (!slash) {
			slash = अन्तः;
		}

		यदि (slash == उपसर्गः && *उपसर्गः == '/') {
			/* Ignore "//" */
			उपसर्गः++;
			अनुवर्त;
		}

		यदि (strncasecmp(p->segment, उपसर्गः, slash - उपसर्गः) == 0) {
			/* prefix matches so far */
			यदि (slash == अन्तः) {
				/* we've run out of prefix, so all
				 * paths below this one match */
				यदि (!urldb_iterate_entries_मार्गः(p,
								callback,
								रिक्त)) {
					प्रत्यागम false;
				}

				/* Progress to next sibling */
				p = p->अग्र;
			} अन्यथा {
				/* Skip over this segment */
				उपसर्गः = slash + 1;

				p = p->children;
			}
		} अन्यथा {
			/* Doesn't match this segment, try next sibling */
			p = p->अग्र;
		}
	} यावत् (p != रिक्त);

	प्रत्यागम true;
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
urldb_iterate_entries_host(संरचना अन्वेषणम्_ग्रन्थिः *parent,
		bool (*url_callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त),
		bool (*cookie_callback)(ध्रुव संरचना cookie_दत्त *दत्त))
{
	यदि (parent == &रिक्तानि) {
		प्रत्यागम true;
	}

	यदि (!urldb_iterate_entries_host(parent->वामः,
					url_callback,
					cookie_callback)) {
		प्रत्यागम false;
	}

	यदि ((parent->दत्त->paths.children) ||
	    ((cookie_callback) &&
	     (parent->दत्त->paths.cookies))) {
		/* We have paths (or domain cookies), so iterate them */
		यदि (!urldb_iterate_entries_मार्गः(&parent->दत्त->paths,
						url_callback,
						cookie_callback)) {
			प्रत्यागम false;
		}
	}

	यदि (!urldb_iterate_entries_host(parent->दक्षिणः,
					url_callback,
					cookie_callback)) {
		प्रत्यागम false;
	}

	प्रत्यागम true;
}


/**
 * Add a host node to the tree
 *
 * \param part Host segment to add (or whole IP address) (copied)
 * \param parent Parent node to add to
 * \return Pointer to added node, or NULL on memory exhaustion
 */
स्थिर संरचना host_part *
urldb_सङ्कलय_host_ग्रन्थिः(ध्रुव वर्ण *part, संरचना host_part *parent)
{
	संरचना host_part *d;

	प्रतिज्ञा(part && parent);

	d = शून्यस्मृतिं_देहि(1, परिमाण(संरचना host_part));
	यदि (!d) {
		प्रत्यागम रिक्त;
	}

	d->part = strdup(part);
	यदि (!d->part) {
		मुक्त(d);
		प्रत्यागम रिक्त;
	}

	d->अग्र = parent->children;
	यदि (parent->children) {
		parent->children->पूर्व = d;
	}
	d->parent = parent;
	parent->children = d;

	प्रत्यागम d;
}


/**
 * Fragment comparator callback for qsort
 *
 * \param a first value
 * \param b second value
 * \return 0 for equal else positive or negative value on comparison
 */
स्थिर पूर्णाङ्क urldb_सङ्कलय_मार्गः_fragment_cmp(ध्रुव शून्य *a, ध्रुव शून्य *b)
{
	प्रत्यागम strcasecmp(*((ध्रुव वर्ण **) a), *((ध्रुव वर्ण **) b));
}


/**
 * Add a fragment to a path segment
 *
 * \param segment Path segment to add to
 * \param fragment Fragment to add (copied), or NULL
 * \return segment or NULL on memory exhaustion
 */
स्थिर संरचना मार्गः_दत्त *
urldb_सङ्कलय_मार्गः_fragment(संरचना मार्गः_दत्त *segment, lwc_string *fragment)
{
	वर्ण **अस्थायी;

	प्रतिज्ञा(segment);

	/* If no fragment, this function is a NOP
	 * This may seem strange, but it makes the rest
	 * of the code cleaner */
	यदि (!fragment)
		प्रत्यागम segment;

	अस्थायी = realloc(segment->fragment,
		       (segment->frag_cnt + 1) * परिमाण(वर्ण *));
	यदि (!अस्थायी)
		प्रत्यागम रिक्त;

	segment->fragment = अस्थायी;
	segment->fragment[segment->frag_cnt] =
		strdup(lwc_string_दत्त(fragment));
	यदि (!segment->fragment[segment->frag_cnt]) {
		/* Don't free temp - it's now our buffer */
		प्रत्यागम रिक्त;
	}

	segment->frag_cnt++;

	/* We want fragments in alphabetical order, so sort them
	 * It may prove better to insert in alphabetical order instead */
	शीघ्रं_क्रमय(segment->fragment,
	      segment->frag_cnt,
	      परिमाण (वर्ण *),
	      urldb_सङ्कलय_मार्गः_fragment_cmp);

	प्रत्यागम segment;
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
स्थिर संरचना मार्गः_दत्त *
urldb_सङ्कलय_मार्गः_ग्रन्थिः(lwc_string *scheme,
		    अचिह्नित पूर्णाङ्क पत्तनम्,
		    ध्रुव वर्ण *segment,
		    lwc_string *fragment,
		    संरचना मार्गः_दत्त *parent)
{
	संरचना मार्गः_दत्त *d, *e;

	प्रतिज्ञा(scheme && segment && parent);

	d = शून्यस्मृतिं_देहि(1, परिमाण(संरचना मार्गः_दत्त));
	यदि (!d)
		प्रत्यागम रिक्त;

	d->scheme = lwc_string_ref(scheme);

	d->पत्तनम् = पत्तनम्;

	d->segment = strdup(segment);
	यदि (!d->segment) {
		lwc_string_unref(d->scheme);
		मुक्त(d);
		प्रत्यागम रिक्त;
	}

	यदि (fragment) {
		यदि (!urldb_सङ्कलय_मार्गः_fragment(d, fragment)) {
			मुक्त(d->segment);
			lwc_string_unref(d->scheme);
			मुक्त(d);
			प्रत्यागम रिक्त;
		}
	}

	प्रति (e = parent->children; e; e = e->अग्र) {
		यदि (वाक्यं_तुलय(e->segment, d->segment) > 0)
			भङ्ग;
	}

	यदि (e) {
		d->पूर्व = e->पूर्व;
		d->अग्र = e;
		यदि (e->पूर्व)
			e->पूर्व->अग्र = d;
		अन्यथा
			parent->children = d;
		e->पूर्व = d;
	} अन्यथा यदि (!parent->children) {
		d->पूर्व = d->अग्र = रिक्त;
		parent->children = parent->चरमः = d;
	} अन्यथा {
		d->अग्र = रिक्त;
		d->पूर्व = parent->चरमः;
		parent->चरमः->अग्र = d;
		parent->चरमः = d;
	}
	d->parent = parent;

	प्रत्यागम d;
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः **urldb_get_अन्वेषणम्_वृक्षः_प्रत्यक्षम्(ध्रुव वर्ण *host)
{
	प्रतिज्ञा(host);

	यदि (urldb_host_is_ip_स्थानाङ्कः(host)) {
		प्रत्यागम &अन्वेषणम्_trees[ST_IP];
	} अन्यथा यदि (ascii_is_alpha(*host)) {
		प्रत्यागम &अन्वेषणम्_trees[ST_DN + ascii_गन्तव्यम्_अधस्तनम्(*host) - 'a'];
	}
	प्रत्यागम &अन्वेषणम्_trees[ST_EE];
}


/**
 * Get the search tree for a particular host
 *
 * \param host the host to lookup
 * \return the corresponding search tree
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *urldb_get_अन्वेषणम्_वृक्षः(ध्रुव वर्ण *host)
{
	प्रत्यागम *urldb_get_अन्वेषणम्_वृक्षः_प्रत्यक्षम्(host);
}


/**
 * Compare host part with a string
 *
 * \param a host part
 * \param b string to compare
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क urldb_अन्वेषणम्_मेलनम्_string(ध्रुव संरचना host_part *a, ध्रुव वर्ण *b)
{
	ध्रुव वर्ण *अन्तः, *dot;
	पूर्णाङ्क plen, ret;

	प्रतिज्ञा(a && a != &db_मूलम् && b);

	यदि (urldb_host_is_ip_स्थानाङ्कः(b)) {
		/* IP address */
		प्रत्यागम strcasecmp(a->part, b);
	}

	अन्तः = b + वाक्यदैर्घ्यम्(b) + 1;

	यावत् (b < अन्तः && a && a != &db_मूलम्) {
		dot = strchr(b, '.');
		यदि (!dot) {
			/* last segment */
			dot = अन्तः - 1;
		}

		/* Compare strings (length limited) */
		यदि ((ret = strncasecmp(a->part, b, dot - b)) != 0)
			/* didn't match => return difference */
			प्रत्यागम ret;

		/* The strings matched, now check that the lengths do, too */
		plen = वाक्यदैर्घ्यम्(a->part);

		यदि (plen > dot - b) {
			/* len(a) > len(b) */
			प्रत्यागम 1;
		} अन्यथा यदि (plen < dot - b) {
			/* len(a) < len(b) */
			प्रत्यागम -1;
		}

		b = dot + 1;
		a = a->parent;
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_मूलम् && b >= अन्तः) {
		/* len(a) > len(b) */
		प्रत्यागम 1;
	} अन्यथा यदि ((!a || a == &db_मूलम्) && b < अन्तः) {
		/* len(a) < len(b) */
		प्रत्यागम -1;
	}

	/* Identical */
	प्रत्यागम 0;
}


/**
 * Find a node in a search tree
 *
 * \param root Tree to look in
 * \param host Host to find
 * \return Pointer to host tree node, or NULL if not found
 */
स्थिर ध्रुव संरचना host_part *
urldb_अन्वेषणम्_find(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्, ध्रुव वर्ण *host)
{
	पूर्णाङ्क c;

	प्रतिज्ञा(मूलम् && host);

	यदि (मूलम् == &रिक्तानि) {
		प्रत्यागम रिक्त;
	}

	c = urldb_अन्वेषणम्_मेलनम्_string(मूलम्->दत्त, host);

	यदि (c > 0) {
		प्रत्यागम urldb_अन्वेषणम्_find(मूलम्->वामः, host);
	} अन्यथा यदि (c < 0) {
		प्रत्यागम urldb_अन्वेषणम्_find(मूलम्->दक्षिणः, host);
	}

	प्रत्यागम मूलम्->दत्त;
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
स्थिर संरचना मार्गः_दत्त *
urldb_मेलनम्_मार्गः(ध्रुव संरचना मार्गः_दत्त *parent,
		 ध्रुव वर्ण *मार्गः,
		 lwc_string *scheme,
		 अचिह्नित ह्रस्व पत्तनम्)
{
	ध्रुव संरचना मार्गः_दत्त *p;
	ध्रुव वर्ण *slash;
	bool मेलनम्;

	प्रतिज्ञा(parent != रिक्त);
	प्रतिज्ञा(parent->segment == रिक्त);

	यदि (मार्गः[0] != '/') {
		NSLOG(netsurf, INFO, "path is %s", मार्गः);
	}

	प्रतिज्ञा(मार्गः[0] == '/');

	/* Start with children, as parent has no segment */
	p = parent->children;

	यावत् (p != रिक्त) {
		slash = strchr(मार्गः + 1, '/');
		यदि (!slash) {
			slash = मार्गः + वाक्यदैर्घ्यम्(मार्गः);
		}

		यदि (वाक्ये_तुलय(p->segment, मार्गः + 1, slash - मार्गः - 1) == 0 &&
		    lwc_string_isequal(p->scheme, scheme, &मेलनम्) == lwc_error_ok &&
		    मेलनम् == true &&
		    p->पत्तनम् == पत्तनम्) {
			यदि (*slash == '\0') {
				/* Complete match */
				प्रत्यागम (संरचना मार्गः_दत्त *) p;
			}

			/* Match so far, go down tree */
			p = p->children;

			मार्गः = slash;
		} अन्यथा {
			/* No match, try next sibling */
			p = p->अग्र;
		}
	}

	प्रत्यागम रिक्त;
}


/**
 * Find an URL in the database
 *
 * \param url Absolute URL to find
 * \return Pointer to path data, or NULL if not found
 */
स्थिर संरचना मार्गः_दत्त *urldb_find_url(nsurl *url)
{
	ध्रुव संरचना host_part *h;
	संरचना मार्गः_दत्त *p;
	संरचना अन्वेषणम्_ग्रन्थिः *वृक्षः;
	वर्ण *plq;
	ध्रुव वर्ण *host_str;
	lwc_string *scheme, *host, *पत्तनम्;
	size_t len = 0;
	अचिह्नित पूर्णाङ्क पत्तनम्_int;
	bool मेलनम्;

	प्रतिज्ञा(url);

	यदि (url_bloom != रिक्त) {
		यदि (bloom_अन्वेषणम्_hash(url_bloom, nsurl_hash(url)) == false) {
			प्रत्यागम रिक्त;
		}
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त)
		प्रत्यागम रिक्त;

	यदि (lwc_string_isequal(scheme, corestring_lwc_mailto, &मेलनम्) ==
	    lwc_error_ok && मेलनम् == true) {
		lwc_string_unref(scheme);
		प्रत्यागम रिक्त;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		host_str = lwc_string_दत्त(host);
		lwc_string_unref(host);

	} अन्यथा यदि (lwc_string_isequal(scheme, corestring_lwc_file, &मेलनम्) ==
		   lwc_error_ok && मेलनम् == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		प्रत्यागम रिक्त;
	}

	वृक्षः = urldb_get_अन्वेषणम्_वृक्षः(host_str);
	h = urldb_अन्वेषणम्_find(वृक्षः, host_str);
	यदि (!h) {
		lwc_string_unref(scheme);
		प्रत्यागम रिक्त;
	}

	/* generate plq (path, leaf, query) */
	यदि (nsurl_get(url, NSURL_मार्गः | NSURL_QUERY, &plq, &len) != NSERROR_OK) {
		lwc_string_unref(scheme);
		प्रत्यागम रिक्त;
	}

	/* Get port */
	पत्तनम् = nsurl_get_component(url, NSURL_पत्तनम्);
	यदि (पत्तनम् != रिक्त) {
		पत्तनम्_int = atoi(lwc_string_दत्त(पत्तनम्));
		lwc_string_unref(पत्तनम्);
	} अन्यथा {
		पत्तनम्_int = 0;
	}

	p = urldb_मेलनम्_मार्गः(&h->paths, plq, scheme, पत्तनम्_int);

	मुक्त(plq);
	lwc_string_unref(scheme);

	प्रत्यागम p;
}


/**
 * Dump URL database paths to stderr
 *
 * \param parent Parent node of tree to dump
 */
स्थिर शून्य urldb_dump_paths(संरचना मार्गः_दत्त *parent)
{
	ध्रुव संरचना मार्गः_दत्त *p = parent;
	अचिह्नित पूर्णाङ्क i;

	कुरु {
		यदि (p->segment != रिक्त) {
			NSLOG(netsurf, INFO, "\t%s : %u",
			      lwc_string_दत्त(p->scheme), p->पत्तनम्);

			NSLOG(netsurf, INFO, "\t\t'%s'", p->segment);

			प्रति (i = 0; i != p->frag_cnt; i++) {
				NSLOG(netsurf, INFO, "\t\t\t#%s",
				      p->fragment[i]);
			}
		}

		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			यावत् (p != parent) {
				यदि (p->अग्र != रिक्त) {
					p = p->अग्र;
					भङ्ग;
				}

				p = p->parent;
			}
		}
	} यावत् (p != parent);
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
	प्रति (h = parent->children; h; h = h->अग्र) {
		urldb_dump_hosts(h);
	}
}


/**
 * Dump search tree
 *
 * \param parent Parent node of tree to dump
 * \param depth Tree depth
 */
स्थिर शून्य urldb_dump_अन्वेषणम्(संरचना अन्वेषणम्_ग्रन्थिः *parent, पूर्णाङ्क गभीरता)
{
	ध्रुव संरचना host_part *h;
	पूर्णाङ्क i; /* index into string */
	वर्ण s[1024];
	पूर्णाङ्क r;
	पूर्णाङ्क sl = परिमाण(s) - 2;

	यदि (parent == &रिक्तानि)
		प्रत्यागम;

	urldb_dump_अन्वेषणम्(parent->वामः, गभीरता + 1);

	प्रति (i = 0; i != गभीरता; i++) {
		s[i] = ' ';
	}

	प्रति (h = parent->दत्त; h; h = h->parent) {
		यदि (h->part) {
			r = snprintf(&s[i], sl - i, "%s", h->part);
			यदि (r < 0) {
				भङ्ग;
			}
			यदि ((i + r) >= sl) {
				भङ्ग;
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

	urldb_dump_अन्वेषणम्(parent->दक्षिणः, गभीरता + 1);
}


/**
 * Compare a pair of host parts
 *
 * \param a first host part
 * \param b second host part
 * \return 0 if match, non-zero, otherwise
 */
स्थिर पूर्णाङ्क
urldb_अन्वेषणम्_मेलनम्_host(ध्रुव संरचना host_part *a, ध्रुव संरचना host_part *b)
{
	पूर्णाङ्क ret;

	प्रतिज्ञा(a && b);

	/* traverse up tree to root, comparing parts as we go. */
	प्रति (; a && a != &db_मूलम् && b && b != &db_मूलम्;
	     a = a->parent, b = b->parent) {
		यदि ((ret = strcasecmp(a->part, b->part)) != 0) {
			/* They differ => return the difference here */
			प्रत्यागम ret;
		}
	}

	/* If we get here then either:
	 *    a) The path lengths differ
	 * or b) The hosts are identical
	 */
	यदि (a && a != &db_मूलम् && (!b || b == &db_मूलम्)) {
		/* len(a) > len(b) */
		प्रत्यागम 1;
	} अन्यथा यदि ((!a || a == &db_मूलम्) && b && b != &db_मूलम्) {
		/* len(a) < len(b) */
		प्रत्यागम -1;
	}

	/* identical */
	प्रत्यागम 0;
}


/**
 * Rotate a subtree right
 *
 * \param root Root of subtree to rotate
 * \return new root of subtree
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *urldb_अन्वेषणम्_skew(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्)
{
	प्रतिज्ञा(मूलम्);

	यदि (मूलम्->वामः->level == मूलम्->level) {
		संरचना अन्वेषणम्_ग्रन्थिः *अस्थायी;

		अस्थायी = मूलम्->वामः;
		मूलम्->वामः = अस्थायी->दक्षिणः;
		अस्थायी->दक्षिणः = मूलम्;
		मूलम् = अस्थायी;
	}

	प्रत्यागम मूलम्;
}


/**
 * Rotate a node left, increasing the parent's level
 *
 * \param root Root of subtree to rotate
 * \return New root of subtree
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *urldb_अन्वेषणम्_विभज(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्)
{
	प्रतिज्ञा(मूलम्);

	यदि (मूलम्->दक्षिणः->दक्षिणः->level == मूलम्->level) {
		संरचना अन्वेषणम्_ग्रन्थिः *अस्थायी;

		अस्थायी = मूलम्->दक्षिणः;
		मूलम्->दक्षिणः = अस्थायी->वामः;
		अस्थायी->वामः = मूलम्;
		मूलम् = अस्थायी;

		मूलम्->level++;
	}

	प्रत्यागम मूलम्;
}


/**
 * Insert node into search tree
 *
 * \param root Root of (sub)tree to insert into
 * \param n Node to insert
 * \return Pointer to updated root
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *
urldb_अन्वेषणम्_निवेशय_आन्तरिकम्(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्, संरचना अन्वेषणम्_ग्रन्थिः *n)
{
	प्रतिज्ञा(मूलम् && n);

	यदि (मूलम् == &रिक्तानि) {
		मूलम् = n;
	} अन्यथा {
		पूर्णाङ्क c = urldb_अन्वेषणम्_मेलनम्_host(मूलम्->दत्त, n->दत्त);

		यदि (c > 0) {
			मूलम्->वामः = urldb_अन्वेषणम्_निवेशय_आन्तरिकम्(
				मूलम्->वामः, n);
		} अन्यथा यदि (c < 0) {
			मूलम्->दक्षिणः = urldb_अन्वेषणम्_निवेशय_आन्तरिकम्(
				मूलम्->दक्षिणः, n);
		} अन्यथा {
			/* exact match */
			मुक्त(n);
			प्रत्यागम मूलम्;
		}

		मूलम् = urldb_अन्वेषणम्_skew(मूलम्);
		मूलम् = urldb_अन्वेषणम्_विभज(मूलम्);
	}

	प्रत्यागम मूलम्;
}


/**
 * Insert a node into the search tree
 *
 * \param root Root of tree to insert into
 * \param data User data to insert
 * \return Pointer to updated root, or NULL if failed
 */
स्थिर संरचना अन्वेषणम्_ग्रन्थिः *
urldb_अन्वेषणम्_निवेशय(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्, ध्रुव संरचना host_part *दत्त)
{
	संरचना अन्वेषणम्_ग्रन्थिः *n;

	प्रतिज्ञा(मूलम् && दत्त);

	n = स्मृतिं_देहि(परिमाण(संरचना अन्वेषणम्_ग्रन्थिः));
	यदि (!n)
		प्रत्यागम रिक्त;

	n->level = 1;
	n->दत्त = दत्त;
	n->वामः = n->दक्षिणः = &रिक्तानि;

	मूलम् = urldb_अन्वेषणम्_निवेशय_आन्तरिकम्(मूलम्, n);

	प्रत्यागम मूलम्;
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
urldb_parse_avpair(संरचना cookie_आन्तरिकम्_दत्त *c,
		   वर्ण *n,
		   वर्ण *v,
		   bool was_quoted)
{
	पूर्णाङ्क vlen;

	प्रतिज्ञा(c && n && v);

	/* Strip whitespace from start of name */
	प्रति (; *n; n++) {
		यदि (*n != ' ' && *n != '\t')
			भङ्ग;
	}

	/* Strip whitespace from end of name */
	प्रति (vlen = वाक्यदैर्घ्यम्(n); vlen; vlen--) {
		यदि (n[vlen] == ' ' || n[vlen] == '\t')
			n[vlen] = '\0';
		अन्यथा
			भङ्ग;
	}

	/* Strip whitespace from start of value */
	प्रति (; *v; v++) {
		यदि (*v != ' ' && *v != '\t')
			भङ्ग;
	}

	/* Strip whitespace from end of value */
	प्रति (vlen = वाक्यदैर्घ्यम्(v); vlen; vlen--) {
		यदि (v[vlen] == ' ' || v[vlen] == '\t')
			v[vlen] = '\0';
		अन्यथा
			भङ्ग;
	}

	यदि (!c->comment && strcasecmp(n, "Comment") == 0) {
		c->comment = strdup(v);
		यदि (!c->comment)
			प्रत्यागम false;
	} अन्यथा यदि (!c->domain && strcasecmp(n, "Domain") == 0) {
		यदि (v[0] == '.') {
			/* Domain must start with a dot */
			c->domain_from_समुच्चयः = true;
			c->domain = strdup(v);
			यदि (!c->domain)
				प्रत्यागम false;
		}
	} अन्यथा यदि (strcasecmp(n, "Max-Age") == 0) {
		पूर्णाङ्क अस्थायी = atoi(v);
		यदि (अस्थायी == 0)
			/* Special case - 0 means delete */
			c->expires = 0;
		अन्यथा
			c->expires = time(रिक्त) + अस्थायी;
	} अन्यथा यदि (!c->मार्गः && strcasecmp(n, "Path") == 0) {
		c->मार्गः_from_समुच्चयः = true;
		c->मार्गः = strdup(v);
		यदि (!c->मार्गः)
			प्रत्यागम false;
	} अन्यथा यदि (strcasecmp(n, "Version") == 0) {
		c->संस्करणम् = atoi(v);
	} अन्यथा यदि (strcasecmp(n, "Expires") == 0) {
		वर्ण *datenoday;
		time_t expires;
		nserror res;

		/* Strip dayname from date (these are hugely variable
		 * and liable to break the parser.  They also serve no
		 * useful purpose) */
		प्रति (datenoday = v;
		     *datenoday && !ascii_is_digit(*datenoday);
		     datenoday++) {
			/* do nothing */
		}

		res = nsc_strntimet(datenoday, वाक्यदैर्घ्यम्(datenoday), &expires);
		यदि (res != NSERROR_OK) {
			/* assume we have an unrepresentable date =>
			 * force it to the maximum possible value of a
			 * 32bit time_t (this may break in 2038. We'll
			 * deal with that once we come to it) */
			expires = (time_t)0x7fffffff;
		}
		c->expires = expires;
	} अन्यथा यदि (strcasecmp(n, "Secure") == 0) {
		c->secure = true;
	} अन्यथा यदि (strcasecmp(n, "HttpOnly") == 0) {
		c->http_only = true;
	} अन्यथा यदि (!c->name) {
		c->name = strdup(n);
		c->मानम् = strdup(v);
		c->मानम्_was_quoted = was_quoted;
		यदि (!c->name || !c->मानम्) {
			प्रत्यागम false;
		}
	}

	प्रत्यागम true;
}


/**
 * Free a cookie
 *
 * \param c The cookie to free
 */
स्थिर शून्य urldb_free_cookie(संरचना cookie_आन्तरिकम्_दत्त *c)
{
	प्रतिज्ञा(c);

	मुक्त(c->comment);
	मुक्त(c->domain);
	मुक्त(c->मार्गः);
	मुक्त(c->name);
	मुक्त(c->मानम्);
	मुक्त(c);
}


/**
 * Parse a cookie
 *
 * \param url URL being fetched
 * \param cookie Pointer to cookie string (updated on exit)
 * \return Pointer to cookie structure (on heap, caller frees) or NULL
 */
स्थिर संरचना cookie_आन्तरिकम्_दत्त *
urldb_parse_cookie(nsurl *url, ध्रुव वर्ण **cookie)
{
	संरचना cookie_आन्तरिकम्_दत्त *c;
	ध्रुव वर्ण *cur;
	वर्ण name[1024], मानम्[4096];
	वर्ण *n = name, *v = मानम्;
	bool अन्तर्_मानम् = false;
	bool had_मानम्_दत्त = false;
	bool मानम्_verbatim = false;
	bool quoted = false;
	bool was_quoted = false;

	प्रतिज्ञा(url && cookie && *cookie);

	c = शून्यस्मृतिं_देहि(1, परिमाण(संरचना cookie_आन्तरिकम्_दत्त));
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	c->expires = -1;

	name[0] = '\0';
	मानम्[0] = '\0';

	प्रति (cur = *cookie; *cur; cur++) {
		यदि (*cur == '\r' && *(cur + 1) == '\n') {
			/* End of header */
			यदि (quoted) {
				/* Unmatched quote encountered */

				/* Match Firefox 2.0.0.11 */
				मानम्[0] = '\0';

			}

			भङ्ग;
		} अन्यथा यदि (*cur == '\r') {
			/* Spurious linefeed */
			अनुवर्त;
		} अन्यथा यदि (*cur == '\n') {
			/* Spurious newline */
			अनुवर्त;
		}

		यदि (अन्तर्_मानम् && !had_मानम्_दत्त) {
			यदि (*cur == ' ' || *cur == '\t') {
				/* Strip leading whitespace from value */
				अनुवर्त;
			} अन्यथा {
				had_मानम्_दत्त = true;

				/* Value is taken verbatim if first non-space
				 * character is not a " */
				यदि (*cur != '"') {
					मानम्_verbatim = true;
				}
			}
		}

		यदि (अन्तर्_मानम् && !मानम्_verbatim && (*cur == '"')) {
			/* Only non-verbatim values may be quoted */
			यदि (cur == *cookie || *(cur - 1) != '\\') {
				/* Only unescaped quotes count */
				was_quoted = quoted;
				quoted = !quoted;

				अनुवर्त;
			}
		}

		यदि (!quoted && !अन्तर्_मानम् && *cur == '=') {
			/* First equals => attr-value separator */
			अन्तर्_मानम् = true;
			अनुवर्त;
		}

		यदि (!quoted && (was_quoted || *cur == ';')) {
			/* Semicolon or after quoted value
			 * => end of current avpair */

			/* NUL-terminate tokens */
			*n = '\0';
			*v = '\0';

			यदि (!urldb_parse_avpair(c, name, मानम्, was_quoted)) {
				/* Memory exhausted */
				urldb_free_cookie(c);
				प्रत्यागम रिक्त;
			}

			/* And reset to start */
			n = name;
			v = मानम्;
			अन्तर्_मानम् = false;
			had_मानम्_दत्त = false;
			मानम्_verbatim = false;
			was_quoted = false;

			/* Now, if the current input is anything other than a
			 * semicolon, we must be sure to reprocess it */
			यदि (*cur != ';') {
				cur--;
			}

			अनुवर्त;
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
			ध्रुव वर्ण *p;
			ध्रुव वर्ण *semi = strchr(cur + 1, ';');
			यदि (!semi)
				semi = cur + वाक्यदैर्घ्यम्(cur) - 2 /* CRLF */;

			/* Look for equals sign between comma and semi */
			प्रति (p = cur + 1; p < semi; p++)
				यदि (*p == '=')
					भङ्ग;

			यदि (p == semi) {
				/* none found => comma internal to value */
				/* do nothing */
			} अन्यथा {
				/* found one => comma marks end of cookie */
				cur++;
				भङ्ग;
			}
		}

		/* Accumulate into buffers, always leaving space for a NUL */
		/** \todo is silently truncating overlong names/values wise? */
		यदि (!अन्तर्_मानम्) {
			यदि (n < name + (परिमाण(name) - 1))
				*n++ = *cur;
		} अन्यथा {
			यदि (v < मानम् + (परिमाण(मानम्) - 1))
				*v++ = *cur;
		}
	}

	/* Parse final avpair */
	*n = '\0';
	*v = '\0';

	यदि (!urldb_parse_avpair(c, name, मानम्, was_quoted)) {
		/* Memory exhausted */
		urldb_free_cookie(c);
		प्रत्यागम रिक्त;
	}

	/* Now fix-up default values */
	यदि (c->domain == रिक्त) {
		lwc_string *host = nsurl_get_component(url, NSURL_HOST);
		यदि (host == रिक्त) {
			urldb_free_cookie(c);
			प्रत्यागम रिक्त;
		}
		c->domain = strdup(lwc_string_दत्त(host));
		lwc_string_unref(host);
	}

	यदि (c->मार्गः == रिक्त) {
		ध्रुव वर्ण *मार्गः_दत्त;
		वर्ण *मार्गः, *slash;
		lwc_string *मार्गः_lwc;

		मार्गः_lwc = nsurl_get_component(url, NSURL_मार्गः);
		यदि (मार्गः_lwc == रिक्त) {
			urldb_free_cookie(c);
			प्रत्यागम रिक्त;
		}
		मार्गः_दत्त = lwc_string_दत्त(मार्गः_lwc);

		/* Strip leafname and trailing slash (4.3.1) */
		slash = strrchr(मार्गः_दत्त, '/');
		यदि (slash != रिक्त) {
			/* Special case: retain first slash in path */
			यदि (slash == मार्गः_दत्त)
				slash++;

			slash = strndup(मार्गः_दत्त, slash - मार्गः_दत्त);
			यदि (slash == रिक्त) {
				lwc_string_unref(मार्गः_lwc);
				urldb_free_cookie(c);
				प्रत्यागम रिक्त;
			}

			मार्गः = slash;
			lwc_string_unref(मार्गः_lwc);
		} अन्यथा {
			मार्गः = strdup(lwc_string_दत्त(मार्गः_lwc));
			lwc_string_unref(मार्गः_lwc);
			यदि (मार्गः == रिक्त) {
				urldb_free_cookie(c);
				प्रत्यागम रिक्त;
			}
		}

		c->मार्गः = मार्गः;
	}

	/* Write back current position */
	*cookie = cur;

	प्रत्यागम c;
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
स्थिर संरचना मार्गः_दत्त *
urldb_सङ्कलय_मार्गः(lwc_string *scheme,
	       अचिह्नित पूर्णाङ्क पत्तनम्,
	       ध्रुव संरचना host_part *host,
	       वर्ण *मार्गः_query,
	       lwc_string *fragment,
	       nsurl *url)
{
	संरचना मार्गः_दत्त *d, *e;
	वर्ण *buf = मार्गः_query;
	वर्ण *segment, *slash;
	bool मेलनम्;

	प्रतिज्ञा(scheme && host && url);

	d = (संरचना मार्गः_दत्त *) &host->paths;

	/* skip leading '/' */
	segment = buf;
	यदि (*segment == '/')
		segment++;

	/* Process path segments */
	कुरु {
		slash = strchr(segment, '/');
		यदि (!slash) {
			/* last segment */
			/* look for existing entry */
			प्रति (e = d->children; e; e = e->अग्र)
				यदि (वाक्यं_तुलय(segment, e->segment) == 0 &&
				    lwc_string_isequal(scheme,
						       e->scheme, &मेलनम्) ==
				    lwc_error_ok &&
				    मेलनम् == true &&
				    e->पत्तनम् == पत्तनम्)
					भङ्ग;

			d = e ? urldb_सङ्कलय_मार्गः_fragment(e, fragment) :
				urldb_सङ्कलय_मार्गः_ग्रन्थिः(scheme, पत्तनम्,
						    segment, fragment, d);
			भङ्ग;
		}

		*slash = '\0';

		/* look for existing entry */
		प्रति (e = d->children; e; e = e->अग्र)
			यदि (वाक्यं_तुलय(segment, e->segment) == 0 &&
			    lwc_string_isequal(scheme, e->scheme,
					       &मेलनम्) == lwc_error_ok &&
			    मेलनम् == true &&
			    e->पत्तनम् == पत्तनम्)
				भङ्ग;

		d = e ? e : urldb_सङ्कलय_मार्गः_ग्रन्थिः(scheme, पत्तनम्, segment, रिक्त, d);
		यदि (!d)
			भङ्ग;

		segment = slash + 1;
	} यावत् (1);

	मुक्त(मार्गः_query);

	यदि (d && !d->url) {
		/* Insert defragmented URL */
		यदि (nsurl_defragment(url, &d->url) != NSERROR_OK)
			प्रत्यागम रिक्त;
	}

	प्रत्यागम d;
}


/**
 * Add a host to the database, creating any intermediate entries
 *
 * \param host Hostname to add
 * \return Pointer to leaf node, or NULL on memory exhaustion
 */
स्थिर संरचना host_part *urldb_सङ्कलय_host(ध्रुव वर्ण *host)
{
	संरचना host_part *d = (संरचना host_part *) &db_मूलम्, *e;
	संरचना अन्वेषणम्_ग्रन्थिः *s;
	वर्ण buf[256]; /* 256 bytes is sufficient - domain names are
			* limited to 255 chars. */
	वर्ण *part;

	प्रतिज्ञा(host);

	यदि (urldb_host_is_ip_स्थानाङ्कः(host)) {
		/* Host is an IP, so simply add as TLD */

		/* Check for existing entry */
		प्रति (e = d->children; e; e = e->अग्र)
			यदि (strcasecmp(host, e->part) == 0)
				/* found => return it */
				प्रत्यागम e;

		d = urldb_सङ्कलय_host_ग्रन्थिः(host, d);

		s = urldb_अन्वेषणम्_निवेशय(अन्वेषणम्_trees[ST_IP], d);
		यदि (!s) {
			/* failed */
			d = रिक्त;
		} अन्यथा {
			अन्वेषणम्_trees[ST_IP] = s;
		}

		प्रत्यागम d;
	}

	/* Copy host string, so we can corrupt it */
	strncpy(buf, host, परिमाण buf);
	buf[परिमाण buf - 1] = '\0';

	/* Process FQDN segments backwards */
	कुरु {
		part = strrchr(buf, '.');
		यदि (!part) {
			/* last segment */
			/* Check for existing entry */
			प्रति (e = d->children; e; e = e->अग्र)
				यदि (strcasecmp(buf, e->part) == 0)
					भङ्ग;

			यदि (e) {
				d = e;
			} अन्यथा {
				d = urldb_सङ्कलय_host_ग्रन्थिः(buf, d);
			}

			/* And insert into search tree */
			यदि (d) {
				संरचना अन्वेषणम्_ग्रन्थिः **r;

				r = urldb_get_अन्वेषणम्_वृक्षः_प्रत्यक्षम्(buf);
				s = urldb_अन्वेषणम्_निवेशय(*r, d);
				यदि (!s) {
					/* failed */
					d = रिक्त;
				} अन्यथा {
					*r = s;
				}
			}
			भङ्ग;
		}

		/* Check for existing entry */
		प्रति (e = d->children; e; e = e->अग्र)
			यदि (strcasecmp(part + 1, e->part) == 0)
				भङ्ग;

		d = e ? e : urldb_सङ्कलय_host_ग्रन्थिः(part + 1, d);
		यदि (!d)
			भङ्ग;

		*part = '\0';
	} यावत् (1);

	प्रत्यागम d;
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
urldb_निवेशय_cookie(संरचना cookie_आन्तरिकम्_दत्त *c,
		    lwc_string *scheme,
		    nsurl *url)
{
	संरचना cookie_आन्तरिकम्_दत्त *d;
	ध्रुव संरचना host_part *h;
	संरचना मार्गः_दत्त *p;
	time_t now = time(रिक्त);

	प्रतिज्ञा(c);

	यदि (c->domain[0] == '.') {
		h = urldb_अन्वेषणम्_find(
			urldb_get_अन्वेषणम्_वृक्षः(&(c->domain[1])),
			c->domain + 1);
		यदि (!h) {
			h = urldb_सङ्कलय_host(c->domain + 1);
			यदि (!h) {
				urldb_free_cookie(c);
				प्रत्यागम false;
			}
		}

		p = (संरचना मार्गः_दत्त *) &h->paths;
	} अन्यथा {
		/* Need to have a URL and scheme, if it's not a domain cookie */
		प्रतिज्ञा(url != रिक्त);
		प्रतिज्ञा(scheme != रिक्त);

		h = urldb_अन्वेषणम्_find(
			urldb_get_अन्वेषणम्_वृक्षः(c->domain),
			c->domain);

		यदि (!h) {
			h = urldb_सङ्कलय_host(c->domain);
			यदि (!h) {
				urldb_free_cookie(c);
				प्रत्यागम false;
			}
		}

		/* find path */
		p = urldb_सङ्कलय_मार्गः(scheme, 0, h,
				   strdup(c->मार्गः), रिक्त, url);
		यदि (!p) {
			urldb_free_cookie(c);
			प्रत्यागम false;
		}
	}

	/* add cookie */
	प्रति (d = p->cookies; d; d = d->अग्र) {
		यदि (!वाक्यं_तुलय(d->domain, c->domain) &&
		    !वाक्यं_तुलय(d->मार्गः, c->मार्गः) &&
		    !वाक्यं_तुलय(d->name, c->name))
			भङ्ग;
	}

	यदि (d) {
		यदि (c->expires != -1 && c->expires < now) {
			/* remove cookie */
			यदि (d->अग्र)
				d->अग्र->पूर्व = d->पूर्व;
			अन्यथा
				p->cookies_अन्तः = d->पूर्व;
			यदि (d->पूर्व)
				d->पूर्व->अग्र = d->अग्र;
			अन्यथा
				p->cookies = d->अग्र;

			cookie_manager_remove((संरचना cookie_दत्त *)d);

			urldb_free_cookie(d);
			urldb_free_cookie(c);
		} अन्यथा {
			/* replace d with c */
			c->पूर्व = d->पूर्व;
			c->अग्र = d->अग्र;
			यदि (c->अग्र)
				c->अग्र->पूर्व = c;
			अन्यथा
				p->cookies_अन्तः = c;
			यदि (c->पूर्व)
				c->पूर्व->अग्र = c;
			अन्यथा
				p->cookies = c;

			cookie_manager_remove((संरचना cookie_दत्त *)d);
			urldb_free_cookie(d);

			cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
		}
	} अन्यथा {
		c->पूर्व = p->cookies_अन्तः;
		c->अग्र = रिक्त;
		यदि (p->cookies_अन्तः)
			p->cookies_अन्तः->अग्र = c;
		अन्यथा
			p->cookies = c;
		p->cookies_अन्तः = c;

		cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
	}

	प्रत्यागम true;
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
concat_netscape_cookie(संरचना cookie_आन्तरिकम्_दत्त *c,
		    पूर्णाङ्क *प्रयुक्तम्,
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

	प्रतिज्ञा(c && प्रयुक्तम् && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *प्रयुक्तम् + 2 +
		वाक्यदैर्घ्यम्(c->name) + 1 +
		वाक्यदैर्घ्यम्(c->मानम्) + 2 +
		(c->मार्गः_from_समुच्चयः ? 8 + वाक्यदैर्घ्यम्(c->मार्गः) + 2 : 0) +
		(c->domain_from_समुच्चयः ? 10 + वाक्यदैर्घ्यम्(c->domain) + 2 : 0);
	यदि (max_len >= *alloc) {
		वर्ण *अस्थायी = realloc(*buf, max_len);
		यदि (!अस्थायी) {
			प्रत्यागम false;
		}
		*buf = अस्थायी;
		*alloc = max_len;
	}

	/* Original Netscape cookie */
	outl = snprintf(*buf + *प्रयुक्तम् - 1, *alloc - *प्रयुक्तम्, "; %s=", c->name);
	यदि (outl >= (*alloc - *प्रयुक्तम्)) {
		/* output truncated */
		प्रत्यागम false;
	}
	*प्रयुक्तम् += outl;

	/* The Netscape spec doesn't mention quoting of cookie values.
	 * RFC 2109 $10.1.3 indicates that values must not be quoted.
	 *
	 * However, other browsers preserve quoting, so we should, too
	 */
	यदि (c->मानम्_was_quoted) {
		outl = snprintf(*buf + *प्रयुक्तम् - 1,
				*alloc - *प्रयुक्तम्,
				"\"%s\"", c->मानम्);
		यदि (outl >= (*alloc - *प्रयुक्तम्)) {
			/* output truncated */
			प्रत्यागम false;
		}
		*प्रयुक्तम् += outl;
	} अन्यथा {
		/** \todo should we %XX-encode [;HT,SP] ? */
		/** \todo Should we strip escaping backslashes? */
		outl = snprintf(*buf + *प्रयुक्तम् - 1,*alloc - *प्रयुक्तम्, "%s", c->मानम्);
		यदि (outl >= (*alloc - *प्रयुक्तम्)) {
			/* output truncated */
			प्रत्यागम false;
		}
		*प्रयुक्तम् += outl;
	}

	/* We don't send path/domain information -- that's what the
	 * Netscape spec suggests we should do, anyway. */

	प्रत्यागम true;
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
concat_rfc2965_cookie(संरचना cookie_आन्तरिकम्_दत्त *c,
		      पूर्णाङ्क *प्रयुक्तम्,
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
	ध्रुव वर्ण * ध्रुव separators = "()<>@,;:\\\"/[]?={} \t";
	पूर्णाङ्क max_len;
	पूर्णाङ्क outl; /* output length */

	प्रतिज्ञा(c && प्रयुक्तम् && alloc && buf && *buf);

	/* "; " cookie-value
	 * We allow for the possibility that values are quoted
	 */
	max_len = *प्रयुक्तम् + 2 +
		वाक्यदैर्घ्यम्(c->name) + 1 +
		वाक्यदैर्घ्यम्(c->मानम्) + 2 +
		(c->मार्गः_from_समुच्चयः ?
		 8 + वाक्यदैर्घ्यम्(c->मार्गः) + 2 : 0) +
		(c->domain_from_समुच्चयः ?
		 10 + वाक्यदैर्घ्यम्(c->domain) + 2 : 0);

	यदि (max_len >= *alloc) {
		वर्ण *अस्थायी = realloc(*buf, max_len);
		यदि (!अस्थायी) {
			प्रत्यागम false;
		}
		*buf = अस्थायी;
		*alloc = max_len;
	}

	/* RFC2109 or RFC2965 cookie */
	outl = snprintf(*buf + *प्रयुक्तम् - 1,
			*alloc - *प्रयुक्तम्,
			"; %s=", c->name);
	यदि (outl >= (*alloc - *प्रयुक्तम्)) {
		/* output truncated */
		प्रत्यागम false;
	}
	*प्रयुक्तम् += outl;

	/* Value needs quoting if it contains any separator or if
	 * it needs preserving from the Set-Cookie header */
	यदि (c->मानम्_was_quoted ||
	    strpbrk(c->मानम्, separators) != रिक्त) {
		outl = snprintf(*buf + *प्रयुक्तम् - 1,
				*alloc - *प्रयुक्तम्,
				"\"%s\"", c->मानम्);

	} अन्यथा {
		outl = snprintf(*buf + *प्रयुक्तम् - 1,
				*alloc - *प्रयुक्तम्,
				"%s", c->मानम्);
	}
	यदि (outl >= (*alloc - *प्रयुक्तम्)) {
		/* output truncated */
		प्रत्यागम false;
	}
	*प्रयुक्तम् += outl;

	यदि (c->मार्गः_from_समुच्चयः) {
		/* Path, quoted if necessary */

		यदि (strpbrk(c->मार्गः, separators) != रिक्त) {
			outl = snprintf(*buf + *प्रयुक्तम् - 1,
					*alloc - *प्रयुक्तम्,
					"; $Path=\"%s\"", c->मार्गः);

		} अन्यथा {
			outl = snprintf(*buf + *प्रयुक्तम् - 1,
				       *alloc - *प्रयुक्तम्,
				       "; $Path=%s", c->मार्गः);
		}
		यदि (outl >= (*alloc - *प्रयुक्तम्)) {
			/* output truncated */
			प्रत्यागम false;
		}
		*प्रयुक्तम् += outl;
	}

	यदि (c->domain_from_समुच्चयः) {
		/* Domain, quoted if necessary */
		यदि (strpbrk(c->domain, separators) != रिक्त) {
			outl = snprintf(*buf + *प्रयुक्तम् - 1,
					*alloc - *प्रयुक्तम्,
					"; $Domain=\"%s\"", c->domain);
		} अन्यथा {
			outl = snprintf(*buf + *प्रयुक्तम् - 1,
					*alloc - *प्रयुक्तम्,
					"; $Domain=%s", c->domain);
		}
		यदि (outl >= (*alloc - *प्रयुक्तम्)) {
			/* output truncated */
			प्रत्यागम false;
		}
		*प्रयुक्तम् += outl;
	}

	प्रत्यागम true;
}


/**
 * build output cookie string in netscape format
 */
स्थिर वर्ण *
get_netscape_cookie(संरचना cookie_आन्तरिकम्_दत्त **matched_cookies, पूर्णाङ्क गणनम्)
{
	पूर्णाङ्क ret_alloc = 4096, ret_प्रयुक्तम् = 1;
	वर्ण *ret;
	पूर्णाङ्क midx; /* index into matched cookies */

	ret = स्मृतिं_देहि(ret_alloc);
	यदि (ret == रिक्त) {
		प्रत्यागम रिक्त;
	}
	ret[0] = '\0';

	प्रति (midx = 0; midx < गणनम्; midx++) {
		यदि (!concat_netscape_cookie(matched_cookies[midx],
					    &ret_प्रयुक्तम्, &ret_alloc, &ret)) {
			मुक्त(ret);
			प्रत्यागम रिक्त;
		}
	}

	यदि (ret_प्रयुक्तम् <= 2) {
		मुक्त(ret);
		प्रत्यागम रिक्त;
	}

	/* Old-style cookies => no version & skip "; " */
	memmove(ret, ret + 2, ret_प्रयुक्तम् - 2);
	ret_प्रयुक्तम् -= 2;

	/* shrink the output buffer to the required size */
	वर्ण *अस्थायी = realloc(ret, ret_प्रयुक्तम्);
	यदि (अस्थायी != रिक्त) {
		ret = अस्थायी;
	}

	प्रत्यागम ret;
}


/**
 * build output cookie string in RFC2965 format
 */
स्थिर वर्ण *
get_rfc2965_cookie(संरचना cookie_आन्तरिकम्_दत्त **matched_cookies,
		   पूर्णाङ्क गणनम्,
		   पूर्णाङ्क संस्करणम्)
{
	पूर्णाङ्क ret_alloc = 4096; /* return string allocation */
	पूर्णाङ्क ret_प्रयुक्तम् = 1; /* return sting buffer used */
	वर्ण *ret; /* return string buffer */
	पूर्णाङ्क midx; /* index into matched cookies */
	पूर्णाङ्क outl;

	ret = स्मृतिं_देहि(ret_alloc);
	यदि (ret == रिक्त) {
		प्रत्यागम रिक्त;
	}
	ret[0] = '\0';

	outl = snprintf(ret,ret_alloc - ret_प्रयुक्तम्, "$Version=%d", संस्करणम्);
	यदि (outl >= ret_alloc - ret_प्रयुक्तम्) {
		/*अतिप्रवाहः*/
		मुक्त(ret);
		प्रत्यागम रिक्त;
	}
	ret_प्रयुक्तम् += outl;

	प्रति (midx = 0; midx < गणनम्; midx++) {
		यदि (!concat_rfc2965_cookie(matched_cookies[midx],
					   &ret_प्रयुक्तम्, &ret_alloc, &ret)) {
			मुक्त(ret);
			प्रत्यागम रिक्त;
		}
	}

	/* shrink the output buffer to the required size */
	वर्ण *अस्थायी = realloc(ret, ret_प्रयुक्तम्);
	यदि (अस्थायी != रिक्त) {
		ret = अस्थायी;
	}

	प्रत्यागम ret;
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
urldb_delete_cookie_paths(ध्रुव वर्ण *domain,
			  ध्रुव वर्ण *मार्गः,
			  ध्रुव वर्ण *name,
			  संरचना मार्गः_दत्त *parent)
{
	संरचना cookie_आन्तरिकम्_दत्त *c;
	संरचना मार्गः_दत्त *p = parent;

	प्रतिज्ञा(parent);

	कुरु {
		प्रति (c = p->cookies; c; c = c->अग्र) {
			यदि (वाक्यं_तुलय(c->domain, domain) == 0 &&
			    वाक्यं_तुलय(c->मार्गः, मार्गः) == 0 &&
			    वाक्यं_तुलय(c->name, name) == 0) {
				यदि (c->पूर्व) {
					c->पूर्व->अग्र = c->अग्र;
				} अन्यथा {
					p->cookies = c->अग्र;
				}

				यदि (c->अग्र) {
					c->अग्र->पूर्व = c->पूर्व;
				} अन्यथा {
					p->cookies_अन्तः = c->पूर्व;
				}

				urldb_free_cookie(c);

				प्रत्यागम;
			}
		}

		यदि (p->children) {
			p = p->children;
		} अन्यथा {
			यावत् (p != parent) {
				यदि (p->अग्र != रिक्त) {
					p = p->अग्र;
					भङ्ग;
				}

				p = p->parent;
			}
		}
	} यावत् (p != parent);
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
urldb_delete_cookie_hosts(ध्रुव वर्ण *domain,
			  ध्रुव वर्ण *मार्गः,
			  ध्रुव वर्ण *name,
			  संरचना host_part *parent)
{
	संरचना host_part *h;
	प्रतिज्ञा(parent);

	urldb_delete_cookie_paths(domain, मार्गः, name, &parent->paths);

	प्रति (h = parent->children; h; h = h->अग्र) {
		urldb_delete_cookie_hosts(domain, मार्गः, name, h);
	}
}


/**
 * Save a path subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent path
 */
स्थिर शून्य urldb_save_cookie_paths(FILE *fp, संरचना मार्गः_दत्त *parent)
{
	संरचना मार्गः_दत्त *p = parent;
	time_t now = time(रिक्त);

	प्रतिज्ञा(fp && parent);

	कुरु {
		यदि (p->cookies != रिक्त) {
			संरचना cookie_आन्तरिकम्_दत्त *c;

			प्रति (c = p->cookies; c != रिक्त; c = c->अग्र) {
				यदि (c->expires == -1 || c->expires < now) {
					/* Skip expired & session cookies */
					अनुवर्त;
				}

				fprintf(fp,
					"%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t"
					"%s\t%s\t%d\t%s\t%s\t%s\n",
					c->संस्करणम्, c->domain,
					c->domain_from_समुच्चयः, c->मार्गः,
					c->मार्गः_from_समुच्चयः, c->secure,
					c->http_only,
					(पूर्णाङ्क)c->expires, (पूर्णाङ्क)c->चरमः_प्रयुक्तम्,
					c->no_destroy, c->name, c->मानम्,
					c->मानम्_was_quoted,
					p->scheme ? lwc_string_दत्त(p->scheme) :
					"unused",
					p->url ? nsurl_access(p->url) :
					"unused",
					c->comment ? c->comment : "");
			}
		}

		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			यावत् (p != parent) {
				यदि (p->अग्र != रिक्त) {
					p = p->अग्र;
					भङ्ग;
				}

				p = p->parent;
			}
		}
	} यावत् (p != parent);
}


/**
 * Save a host subtree's cookies
 *
 * \param fp File pointer to write to
 * \param parent Parent host
 */
स्थिर शून्य urldb_save_cookie_hosts(FILE *fp, संरचना host_part *parent)
{
	संरचना host_part *h;
	प्रतिज्ञा(fp && parent);

	urldb_save_cookie_paths(fp, &parent->paths);

	प्रति (h = parent->children; h; h = h->अग्र)
		urldb_save_cookie_hosts(fp, h);
}


/**
 * Destroy a cookie node
 *
 * \param c Cookie to destroy
 */
स्थिर शून्य urldb_destroy_cookie(संरचना cookie_आन्तरिकम्_दत्त *c)
{
	मुक्त(c->name);
	मुक्त(c->मानम्);
	मुक्त(c->comment);
	मुक्त(c->domain);
	मुक्त(c->मार्गः);

	मुक्त(c);
}


/**
 * Destroy the contents of a path node
 *
 * \param node Node to destroy contents of (does not destroy node)
 */
स्थिर शून्य urldb_destroy_मार्गः_ग्रन्थिः_content(संरचना मार्गः_दत्त *ग्रन्थिः)
{
	संरचना cookie_आन्तरिकम्_दत्त *a, *b;
	अचिह्नित पूर्णाङ्क i;

	यदि (ग्रन्थिः->url != रिक्त) {
		nsurl_unref(ग्रन्थिः->url);
	}

	lwc_string_unref(ग्रन्थिः->scheme);

	मुक्त(ग्रन्थिः->segment);
	प्रति (i = 0; i < ग्रन्थिः->frag_cnt; i++)
		मुक्त(ग्रन्थिः->fragment[i]);
	मुक्त(ग्रन्थिः->fragment);

	मुक्त(ग्रन्थिः->urld.title);

	प्रति (a = ग्रन्थिः->cookies; a; a = b) {
		b = a->अग्र;
		urldb_destroy_cookie(a);
	}
}


/**
 * Destroy protection space data
 *
 * \param space Protection space to destroy
 */
स्थिर शून्य urldb_destroy_prot_space(संरचना prot_space_दत्त *space)
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
स्थिर शून्य urldb_destroy_मार्गः_वृक्षः(संरचना मार्गः_दत्त *मूलम्)
{
	संरचना मार्गः_दत्त *p = मूलम्;

	कुरु {
		यदि (p->children != रिक्त) {
			p = p->children;
		} अन्यथा {
			संरचना मार्गः_दत्त *q = p;

			यावत् (p != मूलम्) {
				यदि (p->अग्र != रिक्त) {
					p = p->अग्र;
					भङ्ग;
				}

				p = p->parent;

				urldb_destroy_मार्गः_ग्रन्थिः_content(q);
				मुक्त(q);

				q = p;
			}

			urldb_destroy_मार्गः_ग्रन्थिः_content(q);
			मुक्त(q);
		}
	} यावत् (p != मूलम्);
}


/**
 * Destroy a host tree
 *
 * \param root Root node of tree to destroy
 */
स्थिर शून्य urldb_destroy_host_वृक्षः(संरचना host_part *मूलम्)
{
	संरचना host_part *a, *b;
	संरचना मार्गः_दत्त *p, *q;
	संरचना prot_space_दत्त *s, *t;

	/* Destroy children */
	प्रति (a = मूलम्->children; a; a = b) {
		b = a->अग्र;
		urldb_destroy_host_वृक्षः(a);
	}

	/* Now clean up paths */
	प्रति (p = मूलम्->paths.children; p; p = q) {
		q = p->अग्र;
		urldb_destroy_मार्गः_वृक्षः(p);
	}

	/* Root path */
	urldb_destroy_मार्गः_ग्रन्थिः_content(&मूलम्->paths);

	/* Proctection space data */
	प्रति (s = मूलम्->prot_space; s; s = t) {
		t = s->अग्र;
		urldb_destroy_prot_space(s);
	}

	/* And ourselves */
	मुक्त(मूलम्->part);
	मुक्त(मूलम्);
}


/**
 * Destroy a search tree
 *
 * \param root Root node of tree to destroy
 */
स्थिर शून्य urldb_destroy_अन्वेषणम्_वृक्षः(संरचना अन्वेषणम्_ग्रन्थिः *मूलम्)
{
	/* Destroy children */
	यदि (मूलम्->वामः != &रिक्तानि)
		urldb_destroy_अन्वेषणम्_वृक्षः(मूलम्->वामः);
	यदि (मूलम्->दक्षिणः != &रिक्तानि)
		urldb_destroy_अन्वेषणम्_वृक्षः(मूलम्->दक्षिणः);

	/* And destroy ourselves */
	मुक्त(मूलम्);
}


/*************** External interface ***************/


/* exported interface documented in content/urldb.h */
शून्य urldb_destroy(शून्य)
{
	संरचना host_part *a, *b;
	पूर्णाङ्क i;

	/* Clean up search trees */
	प्रति (i = 0; i < अङ्कः_अन्वेषणम्_TREES; i++) {
		यदि (अन्वेषणम्_trees[i] != &रिक्तानि) {
			urldb_destroy_अन्वेषणम्_वृक्षः(अन्वेषणम्_trees[i]);
			अन्वेषणम्_trees[i] = &रिक्तानि;
		}
	}

	/* And database */
	प्रति (a = db_मूलम्.children; a; a = b) {
		b = a->अग्र;
		urldb_destroy_host_वृक्षः(a);
	}
	स्मृतिं_पूरय(&db_मूलम्, 0, परिमाण(db_मूलम्));

	/* And the bloom filter */
	यदि (url_bloom != रिक्त) {
		bloom_destroy(url_bloom);
		url_bloom = रिक्त;
	}
}


/* exported interface documented in netsurf/url_db.h */
nserror urldb_आरोपय(ध्रुव वर्ण *filename)
{
#परिभाषय अधिकतमम्_URL_दीर्घता 4096
	वर्ण s[अधिकतमम्_URL_दीर्घता];
	वर्ण host[256];
	संरचना host_part *h;
	पूर्णाङ्क urls;
	पूर्णाङ्क i;
	पूर्णाङ्क संस्करणम्;
	पूर्णाङ्क दीर्घता;
	FILE *fp;

	प्रतिज्ञा(filename);

	NSLOG(netsurf, INFO, "Loading URL file %s", filename);

	यदि (url_bloom == रिक्त)
		url_bloom = bloom_create(BLOOM_आकार);

	fp = fopen(filename, "r");
	यदि (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for reading",
		      filename);
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp)) {
		fclose(fp);
		प्रत्यागम NSERROR_NEED_दत्त;
	}

	संस्करणम् = atoi(s);
	यदि (संस्करणम् < MIN_URL_FILE_संस्करणम्) {
		NSLOG(netsurf, INFO, "Unsupported URL file version.");
		fclose(fp);
		प्रत्यागम NSERROR_INVALID;
	}
	यदि (संस्करणम् > URL_FILE_संस्करणम्) {
		NSLOG(netsurf, INFO, "Unknown URL file version.");
		fclose(fp);
		प्रत्यागम NSERROR_INVALID;
	}

	यावत् (fgets(host, परिमाण host, fp)) {
		time_t hsts_expiry = 0;
		पूर्णाङ्क hsts_include_sub_domains = 0;

		/* get the hostname */
		दीर्घता = वाक्यदैर्घ्यम्(host) - 1;
		host[दीर्घता] = '\0';

		/* skip data that has ended up with a host of '' */
		यदि (दीर्घता == 0) {
			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;
			urls = atoi(s);
			/* Eight fields/url */
			प्रति (i = 0; i < (8 * urls); i++) {
				यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
					भङ्ग;
			}
			अनुवर्त;
		}

		यदि (संस्करणम् >= 107) {
			वर्ण *p = host;
			यावत् (*p && *p != ' ') p++;
			यावत् (*p && *p == ' ') { *p = '\0'; p++; }
			hsts_include_sub_domains = (*p == '1');
			यावत् (*p && *p != ' ') p++;
			यावत् (*p && *p == ' ') p++;
			nsc_snptimet(p, वाक्यदैर्घ्यम्(p), &hsts_expiry);
		}

		h = urldb_सङ्कलय_host(host);
		यदि (!h) {
			NSLOG(netsurf, INFO, "Failed adding host: '%s'", host);
			fclose(fp);
			प्रत्यागम NSERROR_NOMEM;
		}
		h->hsts.expires = hsts_expiry;
		h->hsts.include_sub_domains = hsts_include_sub_domains;

		/* read number of URLs */
		यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
			भङ्ग;
		urls = atoi(s);

		/* no URLs => try next host */
		यदि (urls == 0) {
			NSLOG(netsurf, INFO, "No URLs for '%s'", host);
			अनुवर्त;
		}

		/* load the non-corrupt data */
		प्रति (i = 0; i < urls; i++) {
			संरचना मार्गः_दत्त *p = रिक्त;
			वर्ण scheme[64], ports[10];
			वर्ण url[64 + 3 + 256 + 6 + 4096 + 1 + 1];
			अचिह्नित पूर्णाङ्क पत्तनम्;
			bool is_file = false;
			nsurl *nsurl;
			lwc_string *scheme_lwc, *fragment_lwc;
			वर्ण *मार्गः_query;
			size_t len;

			यदि (!fgets(scheme, परिमाण scheme, fp))
				भङ्ग;
			दीर्घता = वाक्यदैर्घ्यम्(scheme) - 1;
			scheme[दीर्घता] = '\0';

			यदि (!fgets(ports, परिमाण ports, fp))
				भङ्ग;
			दीर्घता = वाक्यदैर्घ्यम्(ports) - 1;
			ports[दीर्घता] = '\0';
			पत्तनम् = atoi(ports);

			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;
			दीर्घता = वाक्यदैर्घ्यम्(s) - 1;
			s[दीर्घता] = '\0';

			यदि (!strcasecmp(host, "localhost") &&
			    !strcasecmp(scheme, "file"))
				is_file = true;

			snprintf(url, परिमाण url, "%s://%s%s%s%s",
				 scheme,
				 /* file URLs have no host */
				 (is_file ? "" : host),
				 (पत्तनम् ? ":" : ""),
				 (पत्तनम् ? ports : ""),
				 s);

			/* TODO: store URLs in pre-parsed state, and make
			 *       a nsurl_load to generate the nsurl more
			 *       swiftly.
			 *       Need a nsurl_save too.
			 */
			यदि (nsurl_create(url, &nsurl) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				प्रत्यागम NSERROR_NOMEM;
			}

			यदि (url_bloom != रिक्त) {
				uint32_t hash = nsurl_hash(nsurl);
				bloom_निवेशय_hash(url_bloom, hash);
			}

			/* Copy and merge path/query strings */
			यदि (nsurl_get(nsurl, NSURL_मार्गः | NSURL_QUERY,
				      &मार्गः_query, &len) != NSERROR_OK) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				प्रत्यागम NSERROR_NOMEM;
			}

			scheme_lwc = nsurl_get_component(nsurl, NSURL_SCHEME);
			fragment_lwc = nsurl_get_component(nsurl,
							   NSURL_FRAGMENT);
			p = urldb_सङ्कलय_मार्गः(scheme_lwc, पत्तनम्, h, मार्गः_query,
					   fragment_lwc, nsurl);
			यदि (!p) {
				NSLOG(netsurf, INFO, "Failed inserting '%s'",
				      url);
				fclose(fp);
				प्रत्यागम NSERROR_NOMEM;
			}
			nsurl_unref(nsurl);
			lwc_string_unref(scheme_lwc);
			lwc_string_unref(fragment_lwc);

			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;
			यदि (p)
				p->urld.visits = (अचिह्नित पूर्णाङ्क)atoi(s);

			/* entry last use time */
			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp)) {
				भङ्ग;
			}
			यदि (p) {
				nsc_snptimet(s, वाक्यदैर्घ्यम्(s) - 1, &p->urld.चरमः_दर्शय);
			}

			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;
			यदि (p)
				p->urld.type = (content_type)atoi(s);

			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;


			यदि (!fgets(s, अधिकतमम्_URL_दीर्घता, fp))
				भङ्ग;
			दीर्घता = वाक्यदैर्घ्यम्(s) - 1;
			यदि (p && दीर्घता > 0) {
				s[दीर्घता] = '\0';
				p->urld.title = स्मृतिं_देहि(दीर्घता + 1);
				यदि (p->urld.title)
					स्मृतिं_प्रतिलिख(p->urld.title, s, दीर्घता + 1);
			}
		}
	}

	fclose(fp);
	NSLOG(netsurf, INFO, "Successfully loaded URL file");
#अपरिभाषय अधिकतमम्_URL_दीर्घता

	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in netsurf/url_db.h */
nserror urldb_save(ध्रुव वर्ण *filename)
{
	FILE *fp;
	पूर्णाङ्क i;

	प्रतिज्ञा(filename);

	fp = fopen(filename, "w");
	यदि (!fp) {
		NSLOG(netsurf, INFO, "Failed to open file '%s' for writing",
		      filename);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	/* file format version number */
	fprintf(fp, "%d\n", URL_FILE_संस्करणम्);

	प्रति (i = 0; i != अङ्कः_अन्वेषणम्_TREES; i++) {
		urldb_save_अन्वेषणम्_वृक्षः(अन्वेषणम्_trees[i], fp);
	}

	fclose(fp);

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_समुच्चयः_url_persistence(nsurl *url, bool persist)
{
	संरचना मार्गः_दत्त *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	p->persistent = persist;

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
bool urldb_सङ्कलय_url(nsurl *url)
{
	संरचना host_part *h;
	संरचना मार्गः_दत्त *p;
	lwc_string *scheme;
	lwc_string *पत्तनम्;
	lwc_string *host;
	lwc_string *fragment;
	ध्रुव वर्ण *host_str;
	वर्ण *मार्गः_query = रिक्त;
	size_t len;
	bool मेलनम्;
	अचिह्नित पूर्णाङ्क पत्तनम्_int;

	प्रतिज्ञा(url);

	यदि (url_bloom == रिक्त)
		url_bloom = bloom_create(BLOOM_आकार);

	यदि (url_bloom != रिक्त) {
		uint32_t hash = nsurl_hash(url);
		bloom_निवेशय_hash(url_bloom, hash);
	}

	/* Copy and merge path/query strings */
	यदि (nsurl_get(url, NSURL_मार्गः | NSURL_QUERY, &मार्गः_query, &len) !=
	    NSERROR_OK) {
		प्रत्यागम false;
	}
	प्रतिज्ञा(मार्गः_query != रिक्त);

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त) {
		मुक्त(मार्गः_query);
		प्रत्यागम false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		host_str = lwc_string_दत्त(host);
		lwc_string_unref(host);

	} अन्यथा यदि (lwc_string_isequal(scheme, corestring_lwc_file, &मेलनम्) ==
		   lwc_error_ok && मेलनम् == true) {
		host_str = "localhost";

	} अन्यथा {
		lwc_string_unref(scheme);
		मुक्त(मार्गः_query);
		प्रत्यागम false;
	}

	fragment = nsurl_get_component(url, NSURL_FRAGMENT);

	पत्तनम् = nsurl_get_component(url, NSURL_पत्तनम्);
	यदि (पत्तनम् != रिक्त) {
		पत्तनम्_int = atoi(lwc_string_दत्त(पत्तनम्));
		lwc_string_unref(पत्तनम्);
	} अन्यथा {
		पत्तनम्_int = 0;
	}

	/* Get host entry */
	h = urldb_सङ्कलय_host(host_str);

	/* Get path entry */
	यदि (h != रिक्त) {
		p = urldb_सङ्कलय_मार्गः(scheme,
				   पत्तनम्_int,
				   h,
				   मार्गः_query,
				   fragment,
				   url);
	} अन्यथा {
		p = रिक्त;
	}

	lwc_string_unref(scheme);
	lwc_string_unref(fragment);

	प्रत्यागम (p != रिक्त);
}


/* exported interface documented in content/urldb.h */
nserror urldb_समुच्चयः_url_title(nsurl *url, ध्रुव वर्ण *title)
{
	संरचना मार्गः_दत्त *p;
	वर्ण *अस्थायी;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (p == रिक्त) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	/* copy the parameter if necessary */
	यदि (title != रिक्त) {
		अस्थायी = strdup(title);
		यदि (अस्थायी == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	} अन्यथा {
		अस्थायी = रिक्त;
	}

	मुक्त(p->urld.title);
	p->urld.title = अस्थायी;

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_समुच्चयः_url_content_type(nsurl *url, content_type type)
{
	संरचना मार्गः_दत्त *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	p->urld.type = type;

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
nserror urldb_update_url_दर्शय_दत्त(nsurl *url)
{
	संरचना मार्गः_दत्त *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	p->urld.चरमः_दर्शय = time(रिक्त);
	p->urld.visits++;

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/urldb.h */
शून्य urldb_reset_url_दर्शय_दत्त(nsurl *url)
{
	संरचना मार्गः_दत्त *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम;

	p->urld.चरमः_दर्शय = (time_t)0;
	p->urld.visits = 0;
}


/* exported interface documented in netsurf/url_db.h */
ध्रुव संरचना url_दत्त *urldb_get_url_दत्त(nsurl *url)
{
	संरचना मार्गः_दत्त *p;
	संरचना url_आन्तरिकम्_दत्त *u;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम रिक्त;

	u = &p->urld;

	प्रत्यागम (ध्रुव संरचना url_दत्त *) u;
}


/* exported interface documented in content/urldb.h */
nsurl *urldb_get_url(nsurl *url)
{
	संरचना मार्गः_दत्त *p;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम रिक्त;

	प्रत्यागम p->url;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_समुच्चयः_auth_details(nsurl *url, ध्रुव वर्ण *realm, ध्रुव वर्ण *auth)
{
	संरचना मार्गः_दत्त *p, *pi;
	संरचना host_part *h;
	संरचना prot_space_दत्त *space, *space_alloc;
	वर्ण *realm_alloc, *auth_alloc;
	bool मेलनम्;

	प्रतिज्ञा(url && realm && auth);

	/* add url, in case it's missing */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);

	यदि (!p)
		प्रत्यागम;

	/* Search for host_part */
	प्रति (pi = p; pi->parent != रिक्त; pi = pi->parent)
		;
	h = (संरचना host_part *)pi;

	/* Search if given URL belongs to a protection space we already know of. */
	प्रति (space = h->prot_space; space; space = space->अग्र) {
		यदि (!वाक्यं_तुलय(space->realm, realm) &&
		    lwc_string_isequal(space->scheme, p->scheme,
				       &मेलनम्) == lwc_error_ok &&
		    मेलनम् == true &&
		    space->पत्तनम् == p->पत्तनम्)
			भङ्ग;
	}

	यदि (space != रिक्त) {
		/* Overrule existing auth. */
		मुक्त(space->auth);
		space->auth = strdup(auth);
	} अन्यथा {
		/* Create a new protection space. */
		space = space_alloc = स्मृतिं_देहि(परिमाण(संरचना prot_space_दत्त));
		realm_alloc = strdup(realm);
		auth_alloc = strdup(auth);

		यदि (!space_alloc || !realm_alloc || !auth_alloc) {
			मुक्त(space_alloc);
			मुक्त(realm_alloc);
			मुक्त(auth_alloc);
			प्रत्यागम;
		}

		space->scheme = lwc_string_ref(p->scheme);
		space->पत्तनम् = p->पत्तनम्;
		space->realm = realm_alloc;
		space->auth = auth_alloc;
		space->अग्र = h->prot_space;
		h->prot_space = space;
	}

	p->prot_space = space;
}


/* exported interface documented in netsurf/url_db.h */
ध्रुव वर्ण *urldb_get_auth_details(nsurl *url, ध्रुव वर्ण *realm)
{
	संरचना मार्गः_दत्त *p, *p_cur, *p_शिखरम्;

	प्रतिज्ञा(url);

	/* add to the db, so our lookup will work */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम रिक्त;

	/* Check for any auth details attached to the path_data node or any of
	 * its parents.
	 */
	प्रति (p_cur = p; p_cur != रिक्त; p_शिखरम् = p_cur, p_cur = p_cur->parent) {
		यदि (p_cur->prot_space) {
			प्रत्यागम p_cur->prot_space->auth;
		}
	}

	/* Only when we have a realm (and canonical root of given URL), we can
	 * uniquely locate the protection space.
	 */
	यदि (realm != रिक्त) {
		ध्रुव संरचना host_part *h = (ध्रुव संरचना host_part *)p_शिखरम्;
		ध्रुव संरचना prot_space_दत्त *space;
		bool मेलनम्;

		/* Search for a possible matching protection space. */
		प्रति (space = h->prot_space; space != रिक्त;
		     space = space->अग्र) {
			यदि (!वाक्यं_तुलय(space->realm, realm) &&
			    lwc_string_isequal(space->scheme,
					       p->scheme, &मेलनम्) ==
			    lwc_error_ok &&
			    मेलनम् == true &&
			    space->पत्तनम् == p->पत्तनम्) {
				p->prot_space = space;
				प्रत्यागम p->prot_space->auth;
			}
		}
	}

	प्रत्यागम रिक्त;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_समुच्चयः_cert_permissions(nsurl *url, bool permit)
{
	संरचना मार्गः_दत्त *p;
	संरचना host_part *h;

	प्रतिज्ञा(url);

	/* add url, in case it's missing */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम;

	प्रति (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (संरचना host_part *)p;

	h->permit_invalid_certs = permit;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_cert_permissions(nsurl *url)
{
	संरचना मार्गः_दत्त *p;
	ध्रुव संरचना host_part *h;

	प्रतिज्ञा(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम false;

	प्रति (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (ध्रुव संरचना host_part *)p;

	प्रत्यागम h->permit_invalid_certs;
}


/* exported interface documented in content/urldb.h */
bool urldb_समुच्चयः_hsts_policy(संरचना nsurl *url, ध्रुव वर्ण *शीर्षकम्)
{
	संरचना मार्गः_दत्त *p;
	संरचना host_part *h;
	lwc_string *host;
	time_t now = time(रिक्त);
	http_strict_transport_security *sts;
	uint32_t max_age = 0;
	nserror error;

	प्रतिज्ञा(url);

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		यदि (urldb_host_is_ip_स्थानाङ्कः(lwc_string_दत्त(host))) {
			/* Host is IP: ignore */
			lwc_string_unref(host);
			प्रत्यागम true;
		} अन्यथा यदि (lwc_string_दीर्घता(host) == 0) {
			/* Host is blank: ignore */
			lwc_string_unref(host);
			प्रत्यागम true;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: ignore */
		प्रत्यागम true;
	}

	/* add url, in case it's missing */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम false;

	प्रति (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (संरचना host_part *)p;
	यदि (h->permit_invalid_certs) {
		/* Transport is tainted: ignore */
		प्रत्यागम true;
	}

	error = http_parse_strict_transport_security(शीर्षकम्, &sts);
	यदि (error != NSERROR_OK) {
		/* Parse failed: ignore */
		प्रत्यागम true;
	}

	h->hsts.include_sub_domains =
		http_strict_transport_security_include_subdomains(sts);

	max_age = http_strict_transport_security_max_age(sts);
	यदि (max_age == 0) {
		h->hsts.expires = 0;
		h->hsts.include_sub_domains = false;
	} अन्यथा यदि ((time_t) (now + max_age) > h->hsts.expires) {
		h->hsts.expires = now + max_age;
	}

	http_strict_transport_security_destroy(sts);

	प्रत्यागम true;
}


/* exported interface documented in content/urldb.h */
bool urldb_get_hsts_enabled(संरचना nsurl *url)
{
	संरचना मार्गः_दत्त *p;
	ध्रुव संरचना host_part *h;
	lwc_string *host;
	time_t now = time(रिक्त);

	प्रतिज्ञा(url);

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host != रिक्त) {
		यदि (urldb_host_is_ip_स्थानाङ्कः(lwc_string_दत्त(host))) {
			/* Host is IP: not enabled */
			lwc_string_unref(host);
			प्रत्यागम false;
		} अन्यथा यदि (lwc_string_दीर्घता(host) == 0) {
			/* Host is blank: not enabled */
			lwc_string_unref(host);
			प्रत्यागम false;
		}

		lwc_string_unref(host);
	} अन्यथा {
		/* No host part: not enabled */
		प्रत्यागम false;
	}

	/* The URL must exist in the db in order to find HSTS policy, since
	 * we search up the tree from the URL node, and policy from further
	 * up may also apply. */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम false;

	प्रति (; p && p->parent; p = p->parent)
		/* do nothing */;
	प्रतिज्ञा(p);

	h = (ध्रुव संरचना host_part *)p;

	/* Consult record for this host */
	यदि (h->hsts.expires > now) {
		/* Not expired */
		प्रत्यागम true;
	}

	/* Consult parent domains */
	प्रति (h = h->parent; h && h != &db_मूलम्; h = h->parent) {
		यदि (h->hsts.expires > now && h->hsts.include_sub_domains) {
			/* Not expired and subdomains included */
			प्रत्यागम true;
		}
	}

	प्रत्यागम false;
}


/* exported interface documented in netsurf/url_db.h */
शून्य
urldb_iterate_partial(ध्रुव वर्ण *उपसर्गः,
		      bool (*callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त))
{
	वर्ण host[256];
	वर्ण buf[260]; /* max domain + "www." */
	ध्रुव वर्ण *slash, *scheme_sep;
	संरचना अन्वेषणम्_ग्रन्थिः *वृक्षः;
	ध्रुव संरचना host_part *h;

	प्रतिज्ञा(उपसर्गः && callback);

	/* strip scheme */
	scheme_sep = वाक्ये_अन्विष(उपसर्गः, "://");
	यदि (scheme_sep)
		उपसर्गः = scheme_sep + 3;

	slash = strchr(उपसर्गः, '/');
	वृक्षः = urldb_get_अन्वेषणम्_वृक्षः(उपसर्गः);

	यदि (slash) {
		/* if there's a slash in the input, then we can
		 * assume that we're looking for a path */
		snprintf(host, परिमाण host, "%.*s",
			 (पूर्णाङ्क) (slash - उपसर्गः), उपसर्गः);

		h = urldb_अन्वेषणम्_find(वृक्षः, host);
		यदि (!h) {
			पूर्णाङ्क len = slash - उपसर्गः;

			यदि (len <= 3 || strncasecmp(host, "www.", 4) != 0) {
				snprintf(buf, परिमाण buf, "www.%s", host);
				h = urldb_अन्वेषणम्_find(
					अन्वेषणम्_trees[ST_DN + 'w' - 'a'],
					buf);
				यदि (!h)
					प्रत्यागम;
			} अन्यथा
				प्रत्यागम;
		}

		यदि (h->paths.children) {
			/* Have paths, iterate them */
			urldb_iterate_partial_मार्गः(&h->paths, slash + 1,
						   callback);
		}

	} अन्यथा {
		पूर्णाङ्क len = वाक्यदैर्घ्यम्(उपसर्गः);

		/* looking for hosts */
		यदि (!urldb_iterate_partial_host(वृक्षः, उपसर्गः, callback))
			प्रत्यागम;

		यदि (len <= 3 || strncasecmp(उपसर्गः, "www.", 4) != 0) {
			/* now look for www.prefix */
			snprintf(buf, परिमाण buf, "www.%s", उपसर्गः);
			यदि(!urldb_iterate_partial_host(
				   अन्वेषणम्_trees[ST_DN + 'w' - 'a'],
				   buf, callback))
				प्रत्यागम;
		}
	}
}


/* exported interface documented in netsurf/url_db.h */
शून्य
urldb_iterate_entries(bool (*callback)(nsurl *url, ध्रुव संरचना url_दत्त *दत्त))
{
	पूर्णाङ्क i;

	प्रतिज्ञा(callback);

	प्रति (i = 0; i < अङ्कः_अन्वेषणम्_TREES; i++) {
		यदि (!urldb_iterate_entries_host(अन्वेषणम्_trees[i],
						callback,
						रिक्त)) {
			भङ्ग;
		}
	}
}


/* exported interface documented in content/urldb.h */
शून्य urldb_iterate_cookies(bool (*callback)(ध्रुव संरचना cookie_दत्त *दत्त))
{
	पूर्णाङ्क i;

	प्रतिज्ञा(callback);

	प्रति (i = 0; i < अङ्कः_अन्वेषणम्_TREES; i++) {
		यदि (!urldb_iterate_entries_host(अन्वेषणम्_trees[i],
						रिक्त, callback))
			भङ्ग;
	}
}


/* exported interface documented in content/urldb.h */
bool urldb_समुच्चयः_cookie(ध्रुव वर्ण *शीर्षकम्, nsurl *url, nsurl *referer)
{
	ध्रुव वर्ण *cur = शीर्षकम्, *अन्तः;
	lwc_string *मार्गः, *host, *scheme;
	nsurl *urlt;
	bool मेलनम्;

	प्रतिज्ञा(url && शीर्षकम्);

	/* Get defragmented URL, as 'urlt' */
	यदि (nsurl_defragment(url, &urlt) != NSERROR_OK)
		प्रत्यागम रिक्त;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त) {
		nsurl_unref(urlt);
		प्रत्यागम false;
	}

	मार्गः = nsurl_get_component(url, NSURL_मार्गः);
	यदि (मार्गः == रिक्त) {
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		प्रत्यागम false;
	}

	host = nsurl_get_component(url, NSURL_HOST);
	यदि (host == रिक्त) {
		lwc_string_unref(मार्गः);
		lwc_string_unref(scheme);
		nsurl_unref(urlt);
		प्रत्यागम false;
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
		यदि (lwc_string_isequal(host, rhost, &मेलनम्) == lwc_error_ok &&
		    मेलनम् == false) {
			ध्रुव वर्ण *hptr;
			ध्रुव वर्ण *rptr;
			ध्रुव वर्ण *dot;
			ध्रुव वर्ण *host_दत्त = lwc_string_दत्त(host);
			ध्रुव वर्ण *rhost_दत्त = lwc_string_दत्त(rhost);

			/* Ensure neither host nor rhost are IP addresses */
			यदि (urldb_host_is_ip_स्थानाङ्कः(host_दत्त) ||
			    urldb_host_is_ip_स्थानाङ्कः(rhost_दत्त)) {
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

			hptr = host_दत्त + lwc_string_दीर्घता(host) - 1;
			rptr = rhost_दत्त + lwc_string_दीर्घता(rhost) - 1;

			/* 1 */
			यावत् (hptr >= host_दत्त && rptr >= rhost_दत्त) {
				यदि (*hptr != *rptr)
					भङ्ग;
				hptr--;
				rptr--;
			}
			/* Ensure we end up pointing at the start of the
			 * common suffix. The above loop will exit pointing
			 * to the byte before the start of the suffix. */
			hptr++;

			/* 2 */
			यावत् (*hptr != '\0' && *hptr != '.')
				hptr++;

			/* 3 */
			यदि (*hptr == '\0' ||
			    (dot = strchr(hptr + 1, '.')) == रिक्त ||
			    *(dot + 1) == '\0') {
				lwc_string_unref(rhost);
				गच्छ error;
			}
		}

		lwc_string_unref(rhost);
	}

	अन्तः = cur + वाक्यदैर्घ्यम्(cur) - 2 /* Trailing CRLF */;

	कुरु {
		संरचना cookie_आन्तरिकम्_दत्त *c;
		वर्ण *dot;
		size_t len;
#यदि_परिभाषितम् WITH_NSPSL
		ध्रुव वर्ण *suffix;
#यदि_अन्तः

		c = urldb_parse_cookie(url, &cur);
		यदि (!c) {
			/* failed => stop parsing */
			गच्छ error;
		}

		/* validate cookie */

		/* 4.2.2:i Cookie must have NAME and VALUE */
		यदि (!c->name || !c->मानम्) {
			urldb_free_cookie(c);
			गच्छ error;
		}

		/* 4.3.2:i Cookie path must be a prefix of URL path */
		len = वाक्यदैर्घ्यम्(c->मार्गः);
		यदि (len > lwc_string_दीर्घता(मार्गः) ||
		    वाक्ये_तुलय(c->मार्गः, lwc_string_दत्त(मार्गः),
			    len) != 0) {
			urldb_free_cookie(c);
			गच्छ error;
		}

#यदि_परिभाषितम् WITH_NSPSL
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
		dot = strchr(c->domain + 1, '.');
		यदि (!dot || *(dot + 1) == '\0') {
			/* no embedded dots */
			urldb_free_cookie(c);
			गच्छ error;
		}
#यदि_अन्तः

		/* Domain match fetch host with cookie domain */
		यदि (strcasecmp(lwc_string_दत्त(host), c->domain) != 0) {
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
			यदि (urldb_host_is_ip_स्थानाङ्कः(lwc_string_दत्त(host))) {
				/* IP address, so no partial match */
				urldb_free_cookie(c);
				गच्छ error;
			}

			hlen = lwc_string_दीर्घता(host);
			dlen = वाक्यदैर्घ्यम्(c->domain);

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

			यदि (strcasecmp(lwc_string_दत्त(host) + (hlen - dlen),
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
		यदि (!urldb_निवेशय_cookie(c, scheme, urlt))
			गच्छ error;
	} यावत् (cur < अन्तः);

	lwc_string_unref(host);
	lwc_string_unref(मार्गः);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	प्रत्यागम true;

error:
	lwc_string_unref(host);
	lwc_string_unref(मार्गः);
	lwc_string_unref(scheme);
	nsurl_unref(urlt);

	प्रत्यागम false;
}


/* exported interface documented in content/urldb.h */
वर्ण *urldb_get_cookie(nsurl *url, bool include_http_only)
{
	ध्रुव संरचना मार्गः_दत्त *p, *q;
	ध्रुव संरचना host_part *h;
	lwc_string *मार्गः_lwc;
	संरचना cookie_आन्तरिकम्_दत्त *c;
	पूर्णाङ्क गणनम् = 0, संस्करणम् = COOKIE_RFC2965;
	संरचना cookie_आन्तरिकम्_दत्त **matched_cookies;
	पूर्णाङ्क matched_cookies_आकार = 20;
	ध्रुव वर्ण *मार्गः;
	वर्ण *ret;
	lwc_string *scheme;
	time_t now;
	bool मेलनम्;

	प्रतिज्ञा(url != रिक्त);

	/* The URL must exist in the db in order to find relevant cookies, since
	 * we search up the tree from the URL node, and cookies from further
	 * up also apply. */
	urldb_सङ्कलय_url(url);

	p = urldb_find_url(url);
	यदि (!p)
		प्रत्यागम रिक्त;

	scheme = p->scheme;

	matched_cookies = स्मृतिं_देहि(matched_cookies_आकार *
				 परिमाण(संरचना cookie_आन्तरिकम्_दत्त *));
	यदि (!matched_cookies)
		प्रत्यागम रिक्त;

#परिभाषय GROW_MATCHED_COOKIES						\
	कुरु {								\
		यदि (गणनम् == matched_cookies_आकार) {			\
			संरचना cookie_आन्तरिकम्_दत्त **अस्थायी;		\
			अस्थायी = realloc(matched_cookies,			\
				       (matched_cookies_आकार + 20) *	\
				       परिमाण(संरचना cookie_आन्तरिकम्_दत्त *)); \
									\
			यदि (अस्थायी == रिक्त) {				\
				मुक्त(matched_cookies);			\
				प्रत्यागम रिक्त;				\
			}						\
									\
			matched_cookies = अस्थायी;				\
			matched_cookies_आकार += 20;			\
		}							\
	} यावत्(0)

	मार्गः_lwc = nsurl_get_component(url, NSURL_मार्गः);
	यदि (मार्गः_lwc == रिक्त) {
		मुक्त(matched_cookies);
		प्रत्यागम रिक्त;
	}
	मार्गः = lwc_string_दत्त(मार्गः_lwc);
	lwc_string_unref(मार्गः_lwc);

	now = time(रिक्त);

	यदि (*(p->segment) != '\0') {
		/* Match exact path, unless directory, when prefix matching
		 * will handle this case for us. */
		प्रति (q = p->parent->children; q; q = q->अग्र) {
			यदि (वाक्यं_तुलय(q->segment, p->segment))
				अनुवर्त;

			/* Consider all cookies associated with
			 * this exact path */
			प्रति (c = q->cookies; c; c = c->अग्र) {
				यदि (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					अनुवर्त;

				यदि (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &मेलनम्) &&
				    मेलनम् == false)
					/* secure cookie for insecure host.
					 * ignore */
					अनुवर्त;

				यदि (c->http_only && !include_http_only)
					/* Ignore HttpOnly */
					अनुवर्त;

				matched_cookies[गणनम्++] = c;

				GROW_MATCHED_COOKIES;

				यदि (c->संस्करणम् < (अचिह्नित पूर्णाङ्क)संस्करणम्)
					संस्करणम् = c->संस्करणम्;

				c->चरमः_प्रयुक्तम् = now;

				cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
			}
		}
	}

	/* Now consider cookies whose paths prefix-match ours */
	प्रति (p = p->parent; p; p = p->parent) {
		/* Find directory's path entry(ies) */
		/* There are potentially multiple due to differing schemes */
		प्रति (q = p->children; q; q = q->अग्र) {
			यदि (*(q->segment) != '\0')
				अनुवर्त;

			प्रति (c = q->cookies; c; c = c->अग्र) {
				यदि (c->expires != -1 && c->expires < now)
					/* cookie has expired => ignore */
					अनुवर्त;

				यदि (c->secure && lwc_string_isequal(
					    q->scheme,
					    corestring_lwc_https,
					    &मेलनम्) &&
				    मेलनम् == false)
					/* Secure cookie for insecure server
					 * => ignore */
					अनुवर्त;

				matched_cookies[गणनम्++] = c;

				GROW_MATCHED_COOKIES;

				यदि (c->संस्करणम् < (अचिह्नित पूर्णाङ्क) संस्करणम्)
					संस्करणम् = c->संस्करणम्;

				c->चरमः_प्रयुक्तम् = now;

				cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
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
			भङ्ग;
		}

		/* Consider p itself - may be the result of Path=/foo */
		प्रति (c = p->cookies; c; c = c->अग्र) {
			यदि (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				अनुवर्त;

			/* Ensure cookie path is a prefix of the resource */
			यदि (वाक्ये_तुलय(c->मार्गः, मार्गः, वाक्यदैर्घ्यम्(c->मार्गः)) != 0)
				/* paths don't match => ignore */
				अनुवर्त;

			यदि (c->secure && lwc_string_isequal(p->scheme,
							    corestring_lwc_https,
							    &मेलनम्) &&
			    मेलनम् == false)
				/* Secure cookie for insecure server
				 * => ignore */
				अनुवर्त;

			matched_cookies[गणनम्++] = c;

			GROW_MATCHED_COOKIES;

			यदि (c->संस्करणम् < (अचिह्नित पूर्णाङ्क) संस्करणम्)
				संस्करणम् = c->संस्करणम्;

			c->चरमः_प्रयुक्तम् = now;

			cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
		}

	}

	/* Finally consider domain cookies for hosts which domain match ours */
	प्रति (h = (ध्रुव संरचना host_part *)p; h && h != &db_मूलम्;
	     h = h->parent) {
		प्रति (c = h->paths.cookies; c; c = c->अग्र) {
			यदि (c->expires != -1 && c->expires < now)
				/* cookie has expired => ignore */
				अनुवर्त;

			/* Ensure cookie path is a prefix of the resource */
			यदि (वाक्ये_तुलय(c->मार्गः, मार्गः, वाक्यदैर्घ्यम्(c->मार्गः)) != 0)
				/* paths don't match => ignore */
				अनुवर्त;

			यदि (c->secure && lwc_string_isequal(scheme,
							    corestring_lwc_https,
							    &मेलनम्) &&
			    मेलनम् == false)
				/* secure cookie for insecure host. ignore */
				अनुवर्त;

			matched_cookies[गणनम्++] = c;

			GROW_MATCHED_COOKIES;

			यदि (c->संस्करणम् < (अचिह्नित पूर्णाङ्क)संस्करणम्)
				संस्करणम् = c->संस्करणम्;

			c->चरमः_प्रयुक्तम् = now;

			cookie_manager_सङ्कलय((संरचना cookie_दत्त *)c);
		}
	}
#अपरिभाषय GROW_MATCHED_COOKIES

	यदि (गणनम् == 0) {
		/* No cookies found */
		मुक्त(matched_cookies);
		प्रत्यागम रिक्त;
	}

	यदि (संस्करणम् == COOKIE_NETSCAPE) {
		ret = get_netscape_cookie(matched_cookies, गणनम्);
	} अन्यथा {
		ret = get_rfc2965_cookie(matched_cookies, गणनम्, संस्करणम्);
	}
	मुक्त(matched_cookies);

	प्रत्यागम ret;
}


/* exported interface documented in content/urldb.h */
शून्य urldb_delete_cookie(ध्रुव वर्ण *domain, ध्रुव वर्ण *मार्गः,
			 ध्रुव वर्ण *name)
{
	urldb_delete_cookie_hosts(domain, मार्गः, name, &db_मूलम्);
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_आरोपय_cookies(ध्रुव वर्ण *filename)
{
	FILE *fp;
	वर्ण s[16*1024];

	प्रतिज्ञा(filename);

	fp = fopen(filename, "r");
	यदि (!fp)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

#परिभाषय FIND_T {				\
		प्रति (; *p && *p != '\t'; p++)	\
			; /* do nothing */	\
		यदि (p >= अन्तः) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			अनुवर्त;		\
		}				\
		*p++ = '\0';			\
	}

#परिभाषय SKIP_T {				\
		प्रति (; *p && *p == '\t'; p++)	\
			; /* do nothing */	\
		यदि (p >= अन्तः) {			\
			NSLOG(netsurf, INFO, "Overran input");	\
			अनुवर्त;		\
		}				\
	}

	यावत् (fgets(s, परिमाण s, fp)) {
		वर्ण *p = s, *अन्तः = 0,
			*domain, *मार्गः, *name, *मानम्, *scheme, *url,
			*comment;
		पूर्णाङ्क संस्करणम्, domain_specified, मार्गः_specified,
			secure, http_only, no_destroy, मानम्_quoted;
		time_t expires, चरमः_प्रयुक्तम्;
		संरचना cookie_आन्तरिकम्_दत्त *c;

		यदि(s[0] == 0 || s[0] == '#')
			/* Skip blank lines or comments */
			अनुवर्त;

		s[वाक्यदैर्घ्यम्(s) - 1] = '\0'; /* lose terminating newline */
		अन्तः = s + वाक्यदैर्घ्यम्(s);

		/* Look for file version first
		 * (all input is ignored until this is read)
		 */
		यदि (strncasecmp(s, "Version:", 8) == 0) {
			FIND_T; SKIP_T; loaded_cookie_file_संस्करणम् = atoi(p);

			यदि (loaded_cookie_file_संस्करणम् <
			    MIN_COOKIE_FILE_संस्करणम्) {
				NSLOG(netsurf, INFO,
				      "Unsupported Cookie file version");
				भङ्ग;
			}

			अनुवर्त;
		} अन्यथा यदि (loaded_cookie_file_संस्करणम् == 0) {
			/* Haven't yet seen version; skip this input */
			अनुवर्त;
		}

		/* One cookie/line */

		/* Parse input */
		FIND_T; संस्करणम् = atoi(s);
		SKIP_T; domain = p; FIND_T;
		SKIP_T; domain_specified = atoi(p); FIND_T;
		SKIP_T; मार्गः = p; FIND_T;
		SKIP_T; मार्गः_specified = atoi(p); FIND_T;
		SKIP_T; secure = atoi(p); FIND_T;
		यदि (loaded_cookie_file_संस्करणम् > 101) {
			/* Introduced in version 1.02 */
			SKIP_T; http_only = atoi(p); FIND_T;
		} अन्यथा {
			http_only = 0;
		}
		SKIP_T; expires = (time_t)atoi(p); FIND_T;
		SKIP_T; चरमः_प्रयुक्तम् = (time_t)atoi(p); FIND_T;
		SKIP_T; no_destroy = atoi(p); FIND_T;
		SKIP_T; name = p; FIND_T;
		SKIP_T; मानम् = p; FIND_T;
		यदि (loaded_cookie_file_संस्करणम् > 100) {
			/* Introduced in version 1.01 */
			SKIP_T;	मानम्_quoted = atoi(p); FIND_T;
		} अन्यथा {
			मानम्_quoted = 0;
		}
		SKIP_T; scheme = p; FIND_T;
		SKIP_T; url = p; FIND_T;

		/* Comment may have no content, so don't
		 * use macros as they'll break */
		प्रति (; *p && *p == '\t'; p++)
			; /* do nothing */
		comment = p;

		प्रतिज्ञा(p <= अन्तः);

		/* Now create cookie */
		c = स्मृतिं_देहि(परिमाण(संरचना cookie_आन्तरिकम्_दत्त));
		यदि (!c)
			भङ्ग;

		c->name = strdup(name);
		c->मानम् = strdup(मानम्);
		c->मानम्_was_quoted = मानम्_quoted;
		c->comment = strdup(comment);
		c->domain_from_समुच्चयः = domain_specified;
		c->domain = strdup(domain);
		c->मार्गः_from_समुच्चयः = मार्गः_specified;
		c->मार्गः = strdup(मार्गः);
		c->expires = expires;
		c->चरमः_प्रयुक्तम् = चरमः_प्रयुक्तम्;
		c->secure = secure;
		c->http_only = http_only;
		c->संस्करणम् = संस्करणम्;
		c->no_destroy = no_destroy;

		यदि (!(c->name && c->मानम् && c->comment &&
		      c->domain && c->मार्गः)) {
			urldb_free_cookie(c);
			भङ्ग;
		}

		यदि (c->domain[0] != '.') {
			lwc_string *scheme_lwc = रिक्त;
			nsurl *url_nsurl = रिक्त;

			प्रतिज्ञा(scheme[0] != 'u');

			यदि (nsurl_create(url, &url_nsurl) != NSERROR_OK) {
				urldb_free_cookie(c);
				भङ्ग;
			}
			scheme_lwc = nsurl_get_component(url_nsurl,
							 NSURL_SCHEME);

			/* And insert it into database */
			यदि (!urldb_निवेशय_cookie(c, scheme_lwc, url_nsurl)) {
				/* Cookie freed for us */
				nsurl_unref(url_nsurl);
				lwc_string_unref(scheme_lwc);
				भङ्ग;
			}
			nsurl_unref(url_nsurl);
			lwc_string_unref(scheme_lwc);

		} अन्यथा {
			यदि (!urldb_निवेशय_cookie(c, रिक्त, रिक्त)) {
				/* Cookie freed for us */
				भङ्ग;
			}
		}
	}

#अपरिभाषय SKIP_T
#अपरिभाषय FIND_T

	fclose(fp);

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in netsurf/cookie_db.h */
nserror urldb_save_cookies(ध्रुव वर्ण *filename)
{
	FILE *fp;
	पूर्णाङ्क cookie_file_संस्करणम् = max(loaded_cookie_file_संस्करणम्,
				      COOKIE_FILE_संस्करणम्);

	प्रतिज्ञा(filename);

	fp = fopen(filename, "w");
	यदि (!fp)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

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
		cookie_file_संस्करणम्);
	fprintf(fp, "Version:\t%d\n", cookie_file_संस्करणम्);

	urldb_save_cookie_hosts(fp, &db_मूलम्);

	fclose(fp);

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in netsurf/url_db.h */
शून्य urldb_dump(शून्य)
{
	पूर्णाङ्क i;

	urldb_dump_hosts(&db_मूलम्);

	प्रति (i = 0; i != अङ्कः_अन्वेषणम्_TREES; i++) {
		urldb_dump_अन्वेषणम्(अन्वेषणम्_trees[i], 0);
	}
}
