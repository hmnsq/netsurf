/*
 * Copyright 2010 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf.
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
 *
 * file scheme URL handling. Based on the data fetcher by Rob Kendrick
 *
 * output dates and directory ordering are affected by the current locale
 */

#अन्तर्भाव "utils/config.h"

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdarg.h>
#यदि_परिभाषितम् HAVE_MMAP
#अन्तर्भाव <sys/mman.h>
#यदि_अन्तः
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/dirent.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/time.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/file.h"
#अन्तर्भाव "netsurf/fetch.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"
#अन्तर्भाव "dirlist.h"
#अन्तर्भाव "file.h"

/* Maximum size of read buffer */
#परिभाषय FETCH_FILE_MAX_BUF_आकार (1024 * 1024)

/** Context for a fetch */
संरचना fetch_file_context {
	संरचना fetch_file_context *r_अग्र, *r_पूर्व;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	वर्ण *मार्गः; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

स्थिर संरचना fetch_file_context *ring = रिक्त;

/** issue fetch callbacks with locking */
स्थिर अन्तःस्थ bool fetch_file_send_callback(ध्रुव fetch_सन्देशः *सन्देशः,
		संरचना fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(सन्देशः, ctx->fetchh);
	ctx->locked = false;

	प्रत्यागम ctx->aborted;
}

स्थिर bool fetch_file_send_शीर्षकम्(संरचना fetch_file_context *ctx,
		ध्रुव वर्ण *fmt, ...)
{
	fetch_सन्देशः सन्देशः;
	वर्ण शीर्षकम्[64];
	va_list ap;
	पूर्णाङ्क len;

	va_start(ap, fmt);
	len = vsnprintf(शीर्षकम्, परिमाण शीर्षकम्, fmt, ap);
	va_end(ap);

	यदि (len >= (पूर्णाङ्क)परिमाण(शीर्षकम्) || len < 0) {
		प्रत्यागम false;
	}

	सन्देशः.type = FETCH_शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = len;

	प्रत्यागम fetch_file_send_callback(&सन्देशः, ctx);
}

/** callback to initialise the file fetcher. */
स्थिर bool fetch_file_initialise(lwc_string *scheme)
{
	प्रत्यागम true;
}

/** callback to initialise the file fetcher. */
स्थिर शून्य fetch_file_finalise(lwc_string *scheme)
{
}

स्थिर bool fetch_file_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम true;
}

/** callback to set up a file fetch context. */
स्थिर शून्य *
fetch_file_सज्जता(संरचना fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		 ध्रुव वर्ण **शीर्षकाणि)
{
	संरचना fetch_file_context *ctx;
	पूर्णाङ्क i;
	nserror ret;

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	यदि (ctx == रिक्त)
		प्रत्यागम रिक्त;

	ret = guit->file->nsurl_गन्तव्यम्_मार्गः(url, &ctx->मार्गः);
	यदि (ret != NSERROR_OK) {
		मुक्त(ctx);
		प्रत्यागम रिक्त;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	प्रति (i = 0; शीर्षकाणि[i] != रिक्त; i++) {
		यदि (strncasecmp(शीर्षकाणि[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			अनुवर्त;
		}

		/* If-None-Match: "12345678" */
		ध्रुव वर्ण *d = शीर्षकाणि[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		यावत् (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		यदि (*d != '\0') {
			ret = nsc_snptimet(d, वाक्यदैर्घ्यम्(d), &ctx->file_etag);
			यदि (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_निवेशय(ring, ctx);

	प्रत्यागम ctx;
}

/** callback to free a file fetch */
स्थिर शून्य fetch_file_free(शून्य *ctx)
{
	संरचना fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	मुक्त(c->मार्गः);
	मुक्त(ctx);
}

/** callback to start a file fetch */
स्थिर bool fetch_file_start(शून्य *ctx)
{
	प्रत्यागम true;
}

/** callback to abort a file fetch */
स्थिर शून्य fetch_file_abort(शून्य *ctx)
{
	संरचना fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

स्थिर पूर्णाङ्क fetch_file_errno_गन्तव्यम्_http_संहिता(पूर्णाङ्क error_no)
{
	विकल्प (error_no) {
	पक्ष ENAMETOOLONG:
		प्रत्यागम 400;
	पक्ष EACCES:
		प्रत्यागम 403;
	पक्ष ENOENT:
		प्रत्यागम 404;
	स्वभाव:
		भङ्ग;
	}

	प्रत्यागम 500;
}

स्थिर शून्य fetch_file_process_error(संरचना fetch_file_context *ctx, पूर्णाङ्क संहिता)
{
	fetch_सन्देशः सन्देशः;
	वर्ण बफर[1024];
	ध्रुव वर्ण *title;
	वर्ण कुञ्चिका[8];

	/* content is going to return error code */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, संहिता);

	/* content type */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_file_process_error_aborted;

	snprintf(कुञ्चिका, परिमाण कुञ्चिका, "HTTP%03d", संहिता);
	title = messages_get(कुञ्चिका);

	snprintf(बफर, परिमाण बफर,
		 "<html><head>"
		 "<title>%s</title>"
		 "<link rel=\"stylesheet\" type=\"text/css\" "
		 "href=\"resource:internal.css\">\n"
		 "</head>"
		 "<body class=\"ns-even-bg ns-even-fg ns-border\" "
		 "id =\"fetcherror\">\n"
		 "<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n"
		 "<p>%s %d %s %s</p>\n"
		 "</body>\n</html>\n",
		 title, title,
		 messages_get("FetchErrorCode"), संहिता,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) बफर;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_error_aborted;

	सन्देशः.type = FETCH_परिसमाप्तम्;
	fetch_file_send_callback(&सन्देशः, ctx);

fetch_file_process_error_aborted:
	प्रत्यागम;
}


/** Process object as a regular file */
स्थिर शून्य fetch_file_process_plain(संरचना fetch_file_context *ctx,
				     संरचना stat *fdstat)
{
#यदि_परिभाषितम् HAVE_MMAP
	fetch_सन्देशः सन्देशः;
	वर्ण *buf = रिक्त;
	size_t buf_आकार;

	पूर्णाङ्क fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	यदि (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_समुच्चयः_http_संहिता(ctx->fetchh, 304);
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&सन्देशः, ctx);
		प्रत्यागम;
	}

	fd = उद्घाटय(ctx->मार्गः, O_RDONLY);
	यदि (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_गन्तव्यम्_http_संहिता(errno));
		प्रत्यागम;
	}

	/* set buffer size */
	buf_आकार = fdstat->st_size;

	/* allocate the buffer storage */
	यदि (buf_आकार > 0) {
		buf = mmap(रिक्त, buf_आकार, PROT_READ, MAP_SHARED, fd, 0);
		यदि (buf == MAP_FAILED) {
			सन्देशः.type = FETCH_ERROR;
			सन्देशः.दत्त.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&सन्देशः, ctx);
			close(fd);
			प्रत्यागम;
		}
	}

	/* fetch is going to be successful */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->मार्गः))) {
		गच्छ fetch_file_process_aborted;
	}

	/* content length */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		गच्छ fetch_file_process_aborted;
	}

	/* create etag */
	यदि (fetch_file_send_शीर्षकम्(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		गच्छ fetch_file_process_aborted;
	}

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) buf;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = buf_आकार;
	fetch_file_send_callback(&सन्देशः, ctx);

	यदि (ctx->aborted == false) {
		सन्देशः.type = FETCH_परिसमाप्तम्;
		fetch_file_send_callback(&सन्देशः, ctx);
	}

fetch_file_process_aborted:

	यदि (buf != रिक्त)
		munmap(buf, buf_आकार);
	close(fd);
#अन्यथा
	fetch_सन्देशः सन्देशः;
	वर्ण *buf;
	size_t buf_आकार;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	यदि (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_समुच्चयः_http_संहिता(ctx->fetchh, 304);
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&सन्देशः, ctx);
		प्रत्यागम;
	}

	infile = fopen(ctx->मार्गः, "rb");
	यदि (infile == रिक्त) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_गन्तव्यम्_http_संहिता(errno));
		प्रत्यागम;
	}

	/* set buffer size */
	buf_आकार = fdstat->st_size;
	यदि (buf_आकार > FETCH_FILE_MAX_BUF_आकार)
		buf_आकार = FETCH_FILE_MAX_BUF_आकार;

	/* allocate the buffer storage */
	buf = स्मृतिं_देहि(buf_आकार);
	यदि (buf == रिक्त) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&सन्देशः, ctx);
		fclose(infile);
		प्रत्यागम;
	}

	/* fetch is going to be successful */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->मार्गः))) {
		गच्छ fetch_file_process_aborted;
	}

	/* content length */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		गच्छ fetch_file_process_aborted;
	}

	/* create etag */
	यदि (fetch_file_send_शीर्षकम्(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		गच्छ fetch_file_process_aborted;
	}

	/* main data loop */
	यावत् (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_आकार, infile);
		यदि (res == 0) {
			यदि (feof(infile)) {
				सन्देशः.type = FETCH_ERROR;
				सन्देशः.दत्त.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&सन्देशः, ctx);
				गच्छ fetch_file_process_aborted;
			} अन्यथा {
				सन्देशः.type = FETCH_ERROR;
				सन्देशः.दत्त.error = "Error reading file";
				fetch_file_send_callback(&सन्देशः, ctx);
				गच्छ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		सन्देशः.type = FETCH_दत्त;
		सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) buf;
		सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = res;
		यदि (fetch_file_send_callback(&सन्देशः, ctx))
			भङ्ग;
	}

	यदि (ctx->aborted == false) {
		सन्देशः.type = FETCH_परिसमाप्तम्;
		fetch_file_send_callback(&सन्देशः, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	मुक्त(buf);
#यदि_अन्तः
	प्रत्यागम;
}

स्थिर वर्ण *gen_nice_title(वर्ण *मार्गः)
{
	वर्ण *nice_मार्गः, *cnv, *tmp;
	वर्ण *title;
	पूर्णाङ्क title_दीर्घता;

	/* Convert path for display */
	nice_मार्गः = स्मृतिं_देहि(वाक्यदैर्घ्यम्(मार्गः) * SLEN("&amp;") + 1);
	यदि (nice_मार्गः == रिक्त) {
		प्रत्यागम रिक्त;
	}

	/* Escape special HTML characters */
	प्रति (cnv = nice_मार्गः, tmp = मार्गः; *tmp != '\0'; tmp++) {
		यदि (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} अन्यथा यदि (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} अन्यथा यदि (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} अन्यथा {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_दीर्घता = (cnv - nice_मार्गः) + वाक्यदैर्घ्यम्(messages_get("FileIndex"));
	title = स्मृतिं_देहि(title_दीर्घता + 1);

	यदि (title == रिक्त) {
		मुक्त(nice_मार्गः);
		प्रत्यागम रिक्त;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_दीर्घता, messages_get("FileIndex"), nice_मार्गः);

	मुक्त(nice_मार्गः);

	प्रत्यागम title;
}

/**
 * Generate an output row of the directory listing.
 *
 * \param ctx The file fetching context.
 * \param ent current directory entry.
 * \param even is the row an even row.
 * \param buffer The output buffer.
 * \param buffer_len The space available in the output buffer.
 * \return NSERROR_OK or error code on faliure.
 */
स्थिर nserror
process_dir_ent(संरचना fetch_file_context *ctx,
		 संरचना dirent *ent,
		 bool even,
		 वर्ण *बफर,
		 size_t बफर_len)
{
	nserror ret;
	वर्ण *urlpath = रिक्त; /* buffer for leaf entry path */
	संरचना stat ent_stat; /* stat result of leaf entry */
	वर्ण datebuf[64]; /* buffer for date text */
	वर्ण timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	यदि (ent->d_name[0] == '.') {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, रिक्त, 2, ctx->मार्गः, ent->d_name);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	यदि (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} अन्यथा {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		यदि (strftime((वर्ण *)&datebuf, परिमाण datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		यदि (strftime((वर्ण *)&timebuf, परिमाण timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->मार्गः_गन्तव्यम्_nsurl(urlpath, &url);
	यदि (ret != NSERROR_OK) {
		मुक्त(urlpath);
		प्रत्यागम ret;
	}

	यदि (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     बफर, बफर_len);
	} अन्यथा यदि (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     बफर, बफर_len);
	} अन्यथा {
		/* something else */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     बफर, बफर_len);
	}

	nsurl_unref(url);
	मुक्त(urlpath);

	प्रत्यागम NSERROR_OK;
}

/**
 * Comparison function for sorting directories.
 *
 * Correctly orders non zero-padded numerical parts.
 * ie. produces "file1, file2, file10" rather than "file1, file10, file2".
 *
 * \param d1 first directory entry
 * \param d2 second directory entry
 */
स्थिर पूर्णाङ्क dir_क्रमय_alpha(ध्रुव संरचना dirent **d1, ध्रुव संरचना dirent **d2)
{
	ध्रुव वर्ण *s1 = (*d1)->d_name;
	ध्रुव वर्ण *s2 = (*d2)->d_name;

	यावत् (*s1 != '\0' && *s2 != '\0') {
		यदि ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			पूर्णाङ्क n1 = 0,  n2 = 0;
			यावत् (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			यावत् (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			यदि (n1 != n2) {
				प्रत्यागम n1 - n2;
			}
			यदि (*s1 == '\0' || *s2 == '\0')
				भङ्ग;
		}
		यदि (tolower(*s1) != tolower(*s2))
			भङ्ग;

		s1++;
		s2++;
	}

	प्रत्यागम tolower(*s1) - tolower(*s2);
}

स्थिर शून्य fetch_file_process_dir(संरचना fetch_file_context *ctx,
				   संरचना stat *fdstat)
{
	fetch_सन्देशः सन्देशः;
	वर्ण बफर[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	वर्ण *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *ऊर्ध्वम्; /* url of parent */

	संरचना dirent **listing = रिक्त; /* directory entry listing */
	पूर्णाङ्क i; /* directory entry index */
	पूर्णाङ्क n; /* number of directory entries */

	n = scandir(ctx->मार्गः, &listing, 0, dir_क्रमय_alpha);
	यदि (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_गन्तव्यम्_http_संहिता(errno));
		प्रत्यागम;
	}

	/* fetch is going to be successful */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, 200);

	/* force no-cache */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Cache-Control: no-cache"))
		गच्छ fetch_file_process_dir_aborted;

	/* content type */
	यदि (fetch_file_send_शीर्षकम्(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_file_process_dir_aborted;

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) बफर;

	/* directory listing top */
	dirlist_generate_शिखरम्(बफर, परिमाण बफर);
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->मार्गः);
	dirlist_generate_title(title, बफर, परिमाण बफर);
	मुक्त(title);
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &ऊर्ध्वम्);
	यदि (err == NSERROR_OK) {
		यदि (nsurl_compare(ctx->url, ऊर्ध्वम्, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(ऊर्ध्वम्),
					बफर, परिमाण बफर);

			सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
			fetch_file_send_callback(&सन्देशः, ctx);
		}
		nsurl_unref(ऊर्ध्वम्);

		यदि (ctx->aborted)
			गच्छ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(बफर, परिमाण बफर);
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	प्रति (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, बफर,
				       परिमाण(बफर));

		यदि (err == NSERROR_OK) {
			सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
			यदि (fetch_file_send_callback(&सन्देशः, ctx))
				गच्छ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(बफर, परिमाण बफर);
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	सन्देशः.type = FETCH_परिसमाप्तम्;
	fetch_file_send_callback(&सन्देशः, ctx);

fetch_file_process_dir_aborted:

	यदि (listing != रिक्त) {
		प्रति (i = 0; i < n; i++) {
			मुक्त(listing[i]);
		}
		मुक्त(listing);
	}
}


/* process a file fetch */
स्थिर शून्य fetch_file_process(संरचना fetch_file_context *ctx)
{
	संरचना stat fdstat; /**< The objects stat */

	यदि (stat(ctx->मार्गः, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_गन्तव्यम्_http_संहिता(errno));
		प्रत्यागम;
	}

	यदि (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		प्रत्यागम;
	} अन्यथा यदि (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		प्रत्यागम;
	} अन्यथा {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	प्रत्यागम;
}

/** callback to poll for additional file fetch contents */
स्थिर शून्य fetch_file_poll(lwc_string *scheme)
{
	संरचना fetch_file_context *c, *save_ring = रिक्त;

	यावत् (ring != रिक्त) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		यदि (c->locked == true) {
			RING_निवेशय(save_ring, c);
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		यदि (c->aborted == false) {
			/* file fetches can be processed in one go */
			fetch_file_process(c);
		}

		/* And now finish */
		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_file_पञ्जिकाकोशः(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.सज्जता = fetch_file_सज्जता,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.मुक्त = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	प्रत्यागम fetcher_सङ्कलय(scheme, &fetcher_ops);
}
