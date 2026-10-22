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

#അന്തര്ഭാവ "utils/config.h"

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdarg.h>
#ifdef HAVE_MMAP
#അന്തര്ഭാവ <sys/mman.h>
#endif
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/dirent.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/time.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "utils/file.h"
#അന്തര്ഭാവ "netsurf/fetch.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"
#അന്തര്ഭാവ "dirlist.h"
#അന്തര്ഭാവ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
ഘടന fetch_file_context {
	ഘടന fetch_file_context *r_next, *r_prev;

	ഘടന fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	അക്ഷരം *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

സ്ഥിതം ഘടന fetch_file_context *ring = ശൂന്യ;

/** issue fetch callbacks with locking */
സ്ഥിതം വരിയിൽ bool fetch_file_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം,
		ഘടന fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(സന്ദേശം, ctx->fetchh);
	ctx->locked = false;

	തിരികെ ctx->aborted;
}

സ്ഥിതം bool fetch_file_send_header(ഘടന fetch_file_context *ctx,
		സ്ഥിരം അക്ഷരം *fmt, ...)
{
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം header[64];
	va_list ap;
	പൂർണ്ണസംഖ്യ len;

	va_start(ap, fmt);
	len = vsnprintf(header, വലുപ്പം header, fmt, ap);
	va_end(ap);

	എങ്കിൽ (len >= (പൂർണ്ണസംഖ്യ)വലുപ്പം(header) || len < 0) {
		തിരികെ false;
	}

	സന്ദേശം.type = FETCH_HEADER;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) header;
	സന്ദേശം.data.header_or_data.len = len;

	തിരികെ fetch_file_send_callback(&സന്ദേശം, ctx);
}

/** callback to initialise the file fetcher. */
സ്ഥിതം bool fetch_file_initialise(lwc_string *scheme)
{
	തിരികെ true;
}

/** callback to initialise the file fetcher. */
സ്ഥിതം ശൂന്യം fetch_file_finalise(lwc_string *scheme)
{
}

സ്ഥിതം bool fetch_file_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ true;
}

/** callback to set up a file fetch context. */
സ്ഥിതം ശൂന്യം *
fetch_file_setup(ഘടന fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 സ്ഥിരം അക്ഷരം *post_urlenc,
		 സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		 സ്ഥിരം അക്ഷരം **headers)
{
	ഘടന fetch_file_context *ctx;
	പൂർണ്ണസംഖ്യ i;
	nserror ret;

	ctx = calloc(1, വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ)
		തിരികെ ശൂന്യ;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	എങ്കിൽ (ret != NSERROR_OK) {
		സ്വതന്ത്രം(ctx);
		തിരികെ ശൂന്യ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	വേണ്ടി (i = 0; headers[i] != ശൂന്യ; i++) {
		എങ്കിൽ (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			തുടരുക;
		}

		/* If-None-Match: "12345678" */
		സ്ഥിരം അക്ഷരം *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ആകുമ്പോൾ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		എങ്കിൽ (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			എങ്കിൽ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_തിരുകുക(ring, ctx);

	തിരികെ ctx;
}

/** callback to free a file fetch */
സ്ഥിതം ശൂന്യം fetch_file_free(ശൂന്യം *ctx)
{
	ഘടന fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	സ്വതന്ത്രം(c->path);
	സ്വതന്ത്രം(ctx);
}

/** callback to start a file fetch */
സ്ഥിതം bool fetch_file_start(ശൂന്യം *ctx)
{
	തിരികെ true;
}

/** callback to abort a file fetch */
സ്ഥിതം ശൂന്യം fetch_file_abort(ശൂന്യം *ctx)
{
	ഘടന fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fetch_file_errno_to_http_code(പൂർണ്ണസംഖ്യ error_no)
{
	തിരഞ്ഞെടുപ്പ് (error_no) {
	അവസ്ഥ ENAMETOOLONG:
		തിരികെ 400;
	അവസ്ഥ EACCES:
		തിരികെ 403;
	അവസ്ഥ ENOENT:
		തിരികെ 404;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ 500;
}

സ്ഥിതം ശൂന്യം fetch_file_process_error(ഘടന fetch_file_context *ctx, പൂർണ്ണസംഖ്യ code)
{
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം buffer[1024];
	സ്ഥിരം അക്ഷരം *title;
	അക്ഷരം താക്കോൽ[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		പോകുക fetch_file_process_error_aborted;

	snprintf(താക്കോൽ, വലുപ്പം താക്കോൽ, "HTTP%03d", code);
	title = messages_get(താക്കോൽ);

	snprintf(buffer, വലുപ്പം buffer,
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

	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) buffer;
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_file_process_error_aborted;

	സന്ദേശം.type = FETCH_FINISHED;
	fetch_file_send_callback(&സന്ദേശം, ctx);

fetch_file_process_error_aborted:
	തിരികെ;
}


/** Process object as a regular file */
സ്ഥിതം ശൂന്യം fetch_file_process_plain(ഘടന fetch_file_context *ctx,
				     ഘടന stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം *buf = ശൂന്യ;
	size_t buf_size;

	പൂർണ്ണസംഖ്യ fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	എങ്കിൽ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		സന്ദേശം.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&സന്ദേശം, ctx);
		തിരികെ;
	}

	fd = open(ctx->path, O_RDONLY);
	എങ്കിൽ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		തിരികെ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	എങ്കിൽ (buf_size > 0) {
		buf = mmap(ശൂന്യ, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		എങ്കിൽ (buf == MAP_FAILED) {
			സന്ദേശം.type = FETCH_ERROR;
			സന്ദേശം.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&സന്ദേശം, ctx);
			close(fd);
			തിരികെ;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		പോകുക fetch_file_process_aborted;
	}

	/* content length */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		പോകുക fetch_file_process_aborted;
	}

	/* create etag */
	എങ്കിൽ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		പോകുക fetch_file_process_aborted;
	}

	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) buf;
	സന്ദേശം.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&സന്ദേശം, ctx);

	എങ്കിൽ (ctx->aborted == false) {
		സന്ദേശം.type = FETCH_FINISHED;
		fetch_file_send_callback(&സന്ദേശം, ctx);
	}

fetch_file_process_aborted:

	എങ്കിൽ (buf != ശൂന്യ)
		munmap(buf, buf_size);
	close(fd);
#അല്ലെങ്കിൽ
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	എങ്കിൽ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		സന്ദേശം.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&സന്ദേശം, ctx);
		തിരികെ;
	}

	infile = fopen(ctx->path, "rb");
	എങ്കിൽ (infile == ശൂന്യ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		തിരികെ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	എങ്കിൽ (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	എങ്കിൽ (buf == ശൂന്യ) {
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&സന്ദേശം, ctx);
		fclose(infile);
		തിരികെ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		പോകുക fetch_file_process_aborted;
	}

	/* content length */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		പോകുക fetch_file_process_aborted;
	}

	/* create etag */
	എങ്കിൽ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		പോകുക fetch_file_process_aborted;
	}

	/* main data loop */
	ആകുമ്പോൾ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		എങ്കിൽ (res == 0) {
			എങ്കിൽ (feof(infile)) {
				സന്ദേശം.type = FETCH_ERROR;
				സന്ദേശം.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&സന്ദേശം, ctx);
				പോകുക fetch_file_process_aborted;
			} അല്ലെങ്കിൽ {
				സന്ദേശം.type = FETCH_ERROR;
				സന്ദേശം.data.error = "Error reading file";
				fetch_file_send_callback(&സന്ദേശം, ctx);
				പോകുക fetch_file_process_aborted;
			}
		}
		tot_read += res;

		സന്ദേശം.type = FETCH_DATA;
		സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) buf;
		സന്ദേശം.data.header_or_data.len = res;
		എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
			നിർത്തുക;
	}

	എങ്കിൽ (ctx->aborted == false) {
		സന്ദേശം.type = FETCH_FINISHED;
		fetch_file_send_callback(&സന്ദേശം, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	സ്വതന്ത്രം(buf);
#endif
	തിരികെ;
}

സ്ഥിതം അക്ഷരം *gen_nice_title(അക്ഷരം *path)
{
	അക്ഷരം *nice_path, *cnv, *tmp;
	അക്ഷരം *title;
	പൂർണ്ണസംഖ്യ title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	എങ്കിൽ (nice_path == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	/* Escape special HTML characters */
	വേണ്ടി (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		എങ്കിൽ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} അല്ലെങ്കിൽ എങ്കിൽ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} അല്ലെങ്കിൽ എങ്കിൽ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} അല്ലെങ്കിൽ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	എങ്കിൽ (title == ശൂന്യ) {
		സ്വതന്ത്രം(nice_path);
		തിരികെ ശൂന്യ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	സ്വതന്ത്രം(nice_path);

	തിരികെ title;
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
സ്ഥിതം nserror
process_dir_ent(ഘടന fetch_file_context *ctx,
		 ഘടന dirent *ent,
		 bool even,
		 അക്ഷരം *buffer,
		 size_t buffer_len)
{
	nserror ret;
	അക്ഷരം *urlpath = ശൂന്യ; /* buffer for leaf entry path */
	ഘടന stat ent_stat; /* stat result of leaf entry */
	അക്ഷരം datebuf[64]; /* buffer for date text */
	അക്ഷരം timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	എങ്കിൽ (ent->d_name[0] == '.') {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ശൂന്യ, 2, ctx->path, ent->d_name);
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}

	എങ്കിൽ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} അല്ലെങ്കിൽ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		എങ്കിൽ (strftime((അക്ഷരം *)&datebuf, വലുപ്പം datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		എങ്കിൽ (strftime((അക്ഷരം *)&timebuf, വലുപ്പം timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	എങ്കിൽ (ret != NSERROR_OK) {
		സ്വതന്ത്രം(urlpath);
		തിരികെ ret;
	}

	എങ്കിൽ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} അല്ലെങ്കിൽ എങ്കിൽ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} അല്ലെങ്കിൽ {
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
	സ്വതന്ത്രം(urlpath);

	തിരികെ NSERROR_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ dir_sort_alpha(സ്ഥിരം ഘടന dirent **d1, സ്ഥിരം ഘടന dirent **d2)
{
	സ്ഥിരം അക്ഷരം *s1 = (*d1)->d_name;
	സ്ഥിരം അക്ഷരം *s2 = (*d2)->d_name;

	ആകുമ്പോൾ (*s1 != '\0' && *s2 != '\0') {
		എങ്കിൽ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			പൂർണ്ണസംഖ്യ n1 = 0,  n2 = 0;
			ആകുമ്പോൾ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ആകുമ്പോൾ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			എങ്കിൽ (n1 != n2) {
				തിരികെ n1 - n2;
			}
			എങ്കിൽ (*s1 == '\0' || *s2 == '\0')
				നിർത്തുക;
		}
		എങ്കിൽ (tolower(*s1) != tolower(*s2))
			നിർത്തുക;

		s1++;
		s2++;
	}

	തിരികെ tolower(*s1) - tolower(*s2);
}

സ്ഥിതം ശൂന്യം fetch_file_process_dir(ഘടന fetch_file_context *ctx,
				   ഘടന stat *fdstat)
{
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	അക്ഷരം *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	ഘടന dirent **listing = ശൂന്യ; /* directory entry listing */
	പൂർണ്ണസംഖ്യ i; /* directory entry index */
	പൂർണ്ണസംഖ്യ n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	എങ്കിൽ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		തിരികെ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	എങ്കിൽ (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		പോകുക fetch_file_process_dir_aborted;

	/* content type */
	എങ്കിൽ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		പോകുക fetch_file_process_dir_aborted;

	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_മുകൾ(buffer, വലുപ്പം buffer);
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, വലുപ്പം buffer);
	സ്വതന്ത്രം(title);
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	എങ്കിൽ (err == NSERROR_OK) {
		എങ്കിൽ (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, വലുപ്പം buffer);

			സന്ദേശം.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&സന്ദേശം, ctx);
		}
		nsurl_unref(up);

		എങ്കിൽ (ctx->aborted)
			പോകുക fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, വലുപ്പം buffer);
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_file_process_dir_aborted;

	വേണ്ടി (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       വലുപ്പം(buffer));

		എങ്കിൽ (err == NSERROR_OK) {
			സന്ദേശം.data.header_or_data.len = strlen(buffer);
			എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
				പോകുക fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, വലുപ്പം buffer);
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_file_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_file_process_dir_aborted;

	സന്ദേശം.type = FETCH_FINISHED;
	fetch_file_send_callback(&സന്ദേശം, ctx);

fetch_file_process_dir_aborted:

	എങ്കിൽ (listing != ശൂന്യ) {
		വേണ്ടി (i = 0; i < n; i++) {
			സ്വതന്ത്രം(listing[i]);
		}
		സ്വതന്ത്രം(listing);
	}
}


/* process a file fetch */
സ്ഥിതം ശൂന്യം fetch_file_process(ഘടന fetch_file_context *ctx)
{
	ഘടന stat fdstat; /**< The objects stat */

	എങ്കിൽ (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		തിരികെ;
	}

	എങ്കിൽ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		തിരികെ;
	} അല്ലെങ്കിൽ എങ്കിൽ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		തിരികെ;
	} അല്ലെങ്കിൽ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	തിരികെ;
}

/** callback to poll for additional file fetch contents */
സ്ഥിതം ശൂന്യം fetch_file_poll(lwc_string *scheme)
{
	ഘടന fetch_file_context *c, *save_ring = ശൂന്യ;

	ആകുമ്പോൾ (ring != ശൂന്യ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		എങ്കിൽ (c->locked == true) {
			RING_തിരുകുക(save_ring, c);
			തുടരുക;
		}

		/* Only process non-aborted fetches */
		എങ്കിൽ (c->aborted == false) {
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

nserror fetch_file_register(ശൂന്യം)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	സ്ഥിരം ഘടന fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.സ്വതന്ത്രം = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	തിരികെ fetcher_add(scheme, &fetcher_ops);
}
