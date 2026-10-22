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
#ifdef HAVE_MMAP
#अन्तर्भाव <sys/mman.h>
#endif
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
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
संरचना fetch_file_context {
	संरचना fetch_file_context *r_next, *r_prev;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	वर्ण *path; /**< The actual path to be used with open() */

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

	पटिनिवत्त ctx->aborted;
}

स्थिर bool fetch_file_send_header(संरचना fetch_file_context *ctx,
		ध्रुव वर्ण *fmt, ...)
{
	fetch_सन्देशः सन्देशः;
	वर्ण header[64];
	va_list ap;
	पूर्णाङ्क len;

	va_start(ap, fmt);
	len = vsnprintf(header, परिमाण header, fmt, ap);
	va_end(ap);

	सचे (len >= (पूर्णाङ्क)परिमाण(header) || len < 0) {
		पटिनिवत्त false;
	}

	सन्देशः.type = FETCH_HEADER;
	सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) header;
	सन्देशः.data.header_or_data.len = len;

	पटिनिवत्त fetch_file_send_callback(&सन्देशः, ctx);
}

/** callback to initialise the file fetcher. */
स्थिर bool fetch_file_initialise(lwc_string *scheme)
{
	पटिनिवत्त true;
}

/** callback to initialise the file fetcher. */
स्थिर शून्य fetch_file_finalise(lwc_string *scheme)
{
}

स्थिर bool fetch_file_can_fetch(ध्रुव nsurl *url)
{
	पटिनिवत्त true;
}

/** callback to set up a file fetch context. */
स्थिर शून्य *
fetch_file_setup(संरचना fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_data *post_multipart,
		 ध्रुव वर्ण **headers)
{
	संरचना fetch_file_context *ctx;
	पूर्णाङ्क i;
	nserror ret;

	ctx = calloc(1, परिमाण(*ctx));
	सचे (ctx == रिक्त)
		पटिनिवत्त रिक्त;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	सचे (ret != NSERROR_OK) {
		मुक्त(ctx);
		पटिनिवत्त रिक्त;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	अत्थाय (i = 0; headers[i] != रिक्त; i++) {
		सचे (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			अनुवर्त;
		}

		/* If-None-Match: "12345678" */
		ध्रुव वर्ण *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		याव (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		सचे (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			सचे (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_निवेशय(ring, ctx);

	पटिनिवत्त ctx;
}

/** callback to free a file fetch */
स्थिर शून्य fetch_file_free(शून्य *ctx)
{
	संरचना fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	मुक्त(c->path);
	मुक्त(ctx);
}

/** callback to start a file fetch */
स्थिर bool fetch_file_start(शून्य *ctx)
{
	पटिनिवत्त true;
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

स्थिर पूर्णाङ्क fetch_file_errno_to_http_code(पूर्णाङ्क error_no)
{
	विकल्प (error_no) {
	पक्ष ENAMETOOLONG:
		पटिनिवत्त 400;
	पक्ष EACCES:
		पटिनिवत्त 403;
	पक्ष ENOENT:
		पटिनिवत्त 404;
	स्वभाव:
		भङ्ग;
	}

	पटिनिवत्त 500;
}

स्थिर शून्य fetch_file_process_error(संरचना fetch_file_context *ctx, पूर्णाङ्क code)
{
	fetch_सन्देशः सन्देशः;
	वर्ण buffer[1024];
	ध्रुव वर्ण *title;
	वर्ण कुञ्चिका[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	सचे (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_file_process_error_aborted;

	snprintf(कुञ्चिका, परिमाण कुञ्चिका, "HTTP%03d", code);
	title = messages_get(कुञ्चिका);

	snprintf(buffer, परिमाण buffer,
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
		 messages_get("FetchErrorCode"), code,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	सन्देशः.type = FETCH_DATA;
	सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) buffer;
	सन्देशः.data.header_or_data.len = strlen(buffer);
	सचे (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_error_aborted;

	सन्देशः.type = FETCH_FINISHED;
	fetch_file_send_callback(&सन्देशः, ctx);

fetch_file_process_error_aborted:
	पटिनिवत्त;
}


/** Process object as a regular file */
स्थिर शून्य fetch_file_process_plain(संरचना fetch_file_context *ctx,
				     संरचना stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_सन्देशः सन्देशः;
	वर्ण *buf = रिक्त;
	size_t buf_size;

	पूर्णाङ्क fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	सचे (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&सन्देशः, ctx);
		पटिनिवत्त;
	}

	fd = open(ctx->path, O_RDONLY);
	सचे (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		पटिनिवत्त;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	सचे (buf_size > 0) {
		buf = mmap(रिक्त, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		सचे (buf == MAP_FAILED) {
			सन्देशः.type = FETCH_ERROR;
			सन्देशः.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&सन्देशः, ctx);
			close(fd);
			पटिनिवत्त;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	सचे (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		गच्छ fetch_file_process_aborted;
	}

	/* content length */
	सचे (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		गच्छ fetch_file_process_aborted;
	}

	/* create etag */
	सचे (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		गच्छ fetch_file_process_aborted;
	}

	सन्देशः.type = FETCH_DATA;
	सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) buf;
	सन्देशः.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&सन्देशः, ctx);

	सचे (ctx->aborted == false) {
		सन्देशः.type = FETCH_FINISHED;
		fetch_file_send_callback(&सन्देशः, ctx);
	}

fetch_file_process_aborted:

	सचे (buf != रिक्त)
		munmap(buf, buf_size);
	close(fd);
#अञ्ञथा
	fetch_सन्देशः सन्देशः;
	वर्ण *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	सचे (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&सन्देशः, ctx);
		पटिनिवत्त;
	}

	infile = fopen(ctx->path, "rb");
	सचे (infile == रिक्त) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		पटिनिवत्त;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	सचे (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	सचे (buf == रिक्त) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&सन्देशः, ctx);
		fclose(infile);
		पटिनिवत्त;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	सचे (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		गच्छ fetch_file_process_aborted;
	}

	/* content length */
	सचे (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		गच्छ fetch_file_process_aborted;
	}

	/* create etag */
	सचे (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		गच्छ fetch_file_process_aborted;
	}

	/* main data loop */
	याव (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		सचे (res == 0) {
			सचे (feof(infile)) {
				सन्देशः.type = FETCH_ERROR;
				सन्देशः.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&सन्देशः, ctx);
				गच्छ fetch_file_process_aborted;
			} अञ्ञथा {
				सन्देशः.type = FETCH_ERROR;
				सन्देशः.data.error = "Error reading file";
				fetch_file_send_callback(&सन्देशः, ctx);
				गच्छ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		सन्देशः.type = FETCH_DATA;
		सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) buf;
		सन्देशः.data.header_or_data.len = res;
		सचे (fetch_file_send_callback(&सन्देशः, ctx))
			भङ्ग;
	}

	सचे (ctx->aborted == false) {
		सन्देशः.type = FETCH_FINISHED;
		fetch_file_send_callback(&सन्देशः, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	मुक्त(buf);
#endif
	पटिनिवत्त;
}

स्थिर वर्ण *gen_nice_title(वर्ण *path)
{
	वर्ण *nice_path, *cnv, *tmp;
	वर्ण *title;
	पूर्णाङ्क title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	सचे (nice_path == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	/* Escape special HTML characters */
	अत्थाय (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		सचे (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} अञ्ञथा सचे (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} अञ्ञथा सचे (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} अञ्ञथा {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	सचे (title == रिक्त) {
		मुक्त(nice_path);
		पटिनिवत्त रिक्त;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	मुक्त(nice_path);

	पटिनिवत्त title;
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
		 वर्ण *buffer,
		 size_t buffer_len)
{
	nserror ret;
	वर्ण *urlpath = रिक्त; /* buffer for leaf entry path */
	संरचना stat ent_stat; /* stat result of leaf entry */
	वर्ण datebuf[64]; /* buffer for date text */
	वर्ण timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	सचे (ent->d_name[0] == '.') {
		पटिनिवत्त NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, रिक्त, 2, ctx->path, ent->d_name);
	सचे (ret != NSERROR_OK) {
		पटिनिवत्त ret;
	}

	सचे (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} अञ्ञथा {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		सचे (strftime((वर्ण *)&datebuf, परिमाण datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		सचे (strftime((वर्ण *)&timebuf, परिमाण timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	सचे (ret != NSERROR_OK) {
		मुक्त(urlpath);
		पटिनिवत्त ret;
	}

	सचे (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} अञ्ञथा सचे (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} अञ्ञथा {
		/* something else */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	}

	nsurl_unref(url);
	मुक्त(urlpath);

	पटिनिवत्त NSERROR_OK;
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
स्थिर पूर्णाङ्क dir_sort_alpha(ध्रुव संरचना dirent **d1, ध्रुव संरचना dirent **d2)
{
	ध्रुव वर्ण *s1 = (*d1)->d_name;
	ध्रुव वर्ण *s2 = (*d2)->d_name;

	याव (*s1 != '\0' && *s2 != '\0') {
		सचे ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			पूर्णाङ्क n1 = 0,  n2 = 0;
			याव (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			याव (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			सचे (n1 != n2) {
				पटिनिवत्त n1 - n2;
			}
			सचे (*s1 == '\0' || *s2 == '\0')
				भङ्ग;
		}
		सचे (tolower(*s1) != tolower(*s2))
			भङ्ग;

		s1++;
		s2++;
	}

	पटिनिवत्त tolower(*s1) - tolower(*s2);
}

स्थिर शून्य fetch_file_process_dir(संरचना fetch_file_context *ctx,
				   संरचना stat *fdstat)
{
	fetch_सन्देशः सन्देशः;
	वर्ण buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	वर्ण *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	संरचना dirent **listing = रिक्त; /* directory entry listing */
	पूर्णाङ्क i; /* directory entry index */
	पूर्णाङ्क n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	सचे (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		पटिनिवत्त;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	सचे (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		गच्छ fetch_file_process_dir_aborted;

	/* content type */
	सचे (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_file_process_dir_aborted;

	सन्देशः.type = FETCH_DATA;
	सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_शिखरम्(buffer, परिमाण buffer);
	सन्देशः.data.header_or_data.len = strlen(buffer);
	सचे (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, परिमाण buffer);
	मुक्त(title);
	सन्देशः.data.header_or_data.len = strlen(buffer);
	सचे (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	सचे (err == NSERROR_OK) {
		सचे (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, परिमाण buffer);

			सन्देशः.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&सन्देशः, ctx);
		}
		nsurl_unref(up);

		सचे (ctx->aborted)
			गच्छ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, परिमाण buffer);
	सन्देशः.data.header_or_data.len = strlen(buffer);
	सचे (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	अत्थाय (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       परिमाण(buffer));

		सचे (err == NSERROR_OK) {
			सन्देशः.data.header_or_data.len = strlen(buffer);
			सचे (fetch_file_send_callback(&सन्देशः, ctx))
				गच्छ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, परिमाण buffer);
	सन्देशः.data.header_or_data.len = strlen(buffer);
	सचे (fetch_file_send_callback(&सन्देशः, ctx))
		गच्छ fetch_file_process_dir_aborted;

	सन्देशः.type = FETCH_FINISHED;
	fetch_file_send_callback(&सन्देशः, ctx);

fetch_file_process_dir_aborted:

	सचे (listing != रिक्त) {
		अत्थाय (i = 0; i < n; i++) {
			मुक्त(listing[i]);
		}
		मुक्त(listing);
	}
}


/* process a file fetch */
स्थिर शून्य fetch_file_process(संरचना fetch_file_context *ctx)
{
	संरचना stat fdstat; /**< The objects stat */

	सचे (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		पटिनिवत्त;
	}

	सचे (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		पटिनिवत्त;
	} अञ्ञथा सचे (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		पटिनिवत्त;
	} अञ्ञथा {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	पटिनिवत्त;
}

/** callback to poll for additional file fetch contents */
स्थिर शून्य fetch_file_poll(lwc_string *scheme)
{
	संरचना fetch_file_context *c, *save_ring = रिक्त;

	याव (ring != रिक्त) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		सचे (c->locked == true) {
			RING_निवेशय(save_ring, c);
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		सचे (c->aborted == false) {
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

nserror fetch_file_register(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ध्रुव संरचना fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.मुक्त = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	पटिनिवत्त fetcher_add(scheme, &fetcher_ops);
}
