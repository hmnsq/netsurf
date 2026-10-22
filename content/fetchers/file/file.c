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

#ಅನ್ತರ್ಭಾವ "utils/config.h"

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ifdef HAVE_MMAP
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#endif
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/dirent.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/time.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/file.h"
#ಅನ್ತರ್ಭಾವ "netsurf/fetch.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "dirlist.h"
#ಅನ್ತರ್ಭಾವ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
ಸಂರಚನಾ fetch_file_context {
	ಸಂರಚನಾ fetch_file_context *r_next, *r_prev;

	ಸಂರಚನಾ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	ವರ್ಣ *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch_file_context *ring = ರಿಕ್ತ;

/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool fetch_file_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ಸಂರಚನಾ fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಪ್ರತ್ಯಾಗಮ ctx->aborted;
}

ಸ್ಥಿರ bool fetch_file_send_header(ಸಂರಚನಾ fetch_file_context *ctx,
		ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ header[64];
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ len;

	va_start(ap, fmt);
	len = vsnprintf(header, ಪರಿಮಾಣ header, fmt, ap);
	va_end(ap);

	ಯದಿ (len >= (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(header) || len < 0) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಸಂದೇಶ.type = FETCH_HEADER;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) header;
	ಸಂದೇಶ.data.header_or_data.len = len;

	ಪ್ರತ್ಯಾಗಮ fetch_file_send_callback(&ಸಂದೇಶ, ctx);
}

/** callback to initialise the file fetcher. */
ಸ್ಥಿರ bool fetch_file_initialise(lwc_string *scheme)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to initialise the file fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_finalise(lwc_string *scheme)
{
}

ಸ್ಥಿರ bool fetch_file_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to set up a file fetch context. */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_file_setup(ಸಂರಚನಾ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಧ್ರುವ ವರ್ಣ *post_urlenc,
		 ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		 ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ fetch_file_context *ctx;
	ಪೂರ್ಣಾಙ್ಕ i;
	nserror ret;

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	ಯದಿ (ret != NSERROR_OK) {
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ಪ್ರತಿ (i = 0; headers[i] != ರಿಕ್ತ; i++) {
		ಯದಿ (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ಅನುವರ್ತ;
		}

		/* If-None-Match: "12345678" */
		ಧ್ರುವ ವರ್ಣ *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ಯಾವತ್ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ಯದಿ (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			ಯದಿ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಪ್ರತ್ಯಾಗಮ ctx;
}

/** callback to free a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_free(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	ಮುಕ್ತ(c->path);
	ಮುಕ್ತ(ctx);
}

/** callback to start a file fetch */
ಸ್ಥಿರ bool fetch_file_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to abort a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_abort(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fetch_file_errno_to_http_code(ಪೂರ್ಣಾಙ್ಕ error_no)
{
	ವಿಕಲ್ಪ (error_no) {
	ಪಕ್ಷ ENAMETOOLONG:
		ಪ್ರತ್ಯಾಗಮ 400;
	ಪಕ್ಷ EACCES:
		ಪ್ರತ್ಯಾಗಮ 403;
	ಪಕ್ಷ ENOENT:
		ಪ್ರತ್ಯಾಗಮ 404;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ 500;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_error(ಸಂರಚನಾ fetch_file_context *ctx, ಪೂರ್ಣಾಙ್ಕ code)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ buffer[1024];
	ಧ್ರುವ ವರ್ಣ *title;
	ವರ್ಣ ಕೀಲಿ[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ಗಚ್ಛ fetch_file_process_error_aborted;

	snprintf(ಕೀಲಿ, ಪರಿಮಾಣ ಕೀಲಿ, "HTTP%03d", code);
	title = messages_get(ಕೀಲಿ);

	snprintf(buffer, ಪರಿಮಾಣ buffer,
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

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buffer;
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_file_process_error_aborted;

	ಸಂದೇಶ.type = FETCH_FINISHED;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

fetch_file_process_error_aborted:
	ಪ್ರತ್ಯಾಗಮ;
}


/** Process object as a regular file */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_plain(ಸಂರಚನಾ fetch_file_context *ctx,
				     ಸಂರಚನಾ stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ *buf = ರಿಕ್ತ;
	size_t buf_size;

	ಪೂರ್ಣಾಙ್ಕ fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ಯದಿ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		ಪ್ರತ್ಯಾಗಮ;
	}

	fd = open(ctx->path, O_RDONLY);
	ಯದಿ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	ಯದಿ (buf_size > 0) {
		buf = mmap(ರಿಕ್ತ, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		ಯದಿ (buf == MAP_FAILED) {
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&ಸಂದೇಶ, ctx);
			close(fd);
			ಪ್ರತ್ಯಾಗಮ;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	/* content length */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	/* create etag */
	ಯದಿ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buf;
	ಸಂದೇಶ.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

	ಯದಿ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_FINISHED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_file_process_aborted:

	ಯದಿ (buf != ರಿಕ್ತ)
		munmap(buf, buf_size);
	close(fd);
#ಅನ್ಯಥಾ
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ಯದಿ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		ಪ್ರತ್ಯಾಗಮ;
	}

	infile = fopen(ctx->path, "rb");
	ಯದಿ (infile == ರಿಕ್ತ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	ಯದಿ (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	ಯದಿ (buf == ರಿಕ್ತ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		fclose(infile);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	/* content length */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	/* create etag */
	ಯದಿ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		ಗಚ್ಛ fetch_file_process_aborted;
	}

	/* main data loop */
	ಯಾವತ್ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		ಯದಿ (res == 0) {
			ಯದಿ (feof(infile)) {
				ಸಂದೇಶ.type = FETCH_ERROR;
				ಸಂದೇಶ.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&ಸಂದೇಶ, ctx);
				ಗಚ್ಛ fetch_file_process_aborted;
			} ಅನ್ಯಥಾ {
				ಸಂದೇಶ.type = FETCH_ERROR;
				ಸಂದೇಶ.data.error = "Error reading file";
				fetch_file_send_callback(&ಸಂದೇಶ, ctx);
				ಗಚ್ಛ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		ಸಂದೇಶ.type = FETCH_DATA;
		ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buf;
		ಸಂದೇಶ.data.header_or_data.len = res;
		ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
			ಭಙ್ಗ;
	}

	ಯದಿ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_FINISHED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	ಮುಕ್ತ(buf);
#endif
	ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ ವರ್ಣ *gen_nice_title(ವರ್ಣ *path)
{
	ವರ್ಣ *nice_path, *cnv, *tmp;
	ವರ್ಣ *title;
	ಪೂರ್ಣಾಙ್ಕ title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	ಯದಿ (nice_path == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Escape special HTML characters */
	ಪ್ರತಿ (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		ಯದಿ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ಅನ್ಯಥಾ ಯದಿ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ಅನ್ಯಥಾ ಯದಿ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ಅನ್ಯಥಾ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	ಯದಿ (title == ರಿಕ್ತ) {
		ಮುಕ್ತ(nice_path);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	ಮುಕ್ತ(nice_path);

	ಪ್ರತ್ಯಾಗಮ title;
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
ಸ್ಥಿರ nserror
process_dir_ent(ಸಂರಚನಾ fetch_file_context *ctx,
		 ಸಂರಚನಾ dirent *ent,
		 bool even,
		 ವರ್ಣ *buffer,
		 size_t buffer_len)
{
	nserror ret;
	ವರ್ಣ *urlpath = ರಿಕ್ತ; /* buffer for leaf entry path */
	ಸಂರಚನಾ stat ent_stat; /* stat result of leaf entry */
	ವರ್ಣ datebuf[64]; /* buffer for date text */
	ವರ್ಣ timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ಯದಿ (ent->d_name[0] == '.') {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ರಿಕ್ತ, 2, ctx->path, ent->d_name);
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಯದಿ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ಅನ್ಯಥಾ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ಯದಿ (strftime((ವರ್ಣ *)&datebuf, ಪರಿಮಾಣ datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ಯದಿ (strftime((ವರ್ಣ *)&timebuf, ಪರಿಮಾಣ timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	ಯದಿ (ret != NSERROR_OK) {
		ಮುಕ್ತ(urlpath);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಯದಿ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ಅನ್ಯಥಾ ಯದಿ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ಅನ್ಯಥಾ {
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
	ಮುಕ್ತ(urlpath);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dir_sort_alpha(ಧ್ರುವ ಸಂರಚನಾ dirent **d1, ಧ್ರುವ ಸಂರಚನಾ dirent **d2)
{
	ಧ್ರುವ ವರ್ಣ *s1 = (*d1)->d_name;
	ಧ್ರುವ ವರ್ಣ *s2 = (*d2)->d_name;

	ಯಾವತ್ (*s1 != '\0' && *s2 != '\0') {
		ಯದಿ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			ಪೂರ್ಣಾಙ್ಕ n1 = 0,  n2 = 0;
			ಯಾವತ್ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ಯಾವತ್ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ಯದಿ (n1 != n2) {
				ಪ್ರತ್ಯಾಗಮ n1 - n2;
			}
			ಯದಿ (*s1 == '\0' || *s2 == '\0')
				ಭಙ್ಗ;
		}
		ಯದಿ (tolower(*s1) != tolower(*s2))
			ಭಙ್ಗ;

		s1++;
		s2++;
	}

	ಪ್ರತ್ಯಾಗಮ tolower(*s1) - tolower(*s2);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_dir(ಸಂರಚನಾ fetch_file_context *ctx,
				   ಸಂರಚನಾ stat *fdstat)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	ವರ್ಣ *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	ಸಂರಚನಾ dirent **listing = ರಿಕ್ತ; /* directory entry listing */
	ಪೂರ್ಣಾಙ್ಕ i; /* directory entry index */
	ಪೂರ್ಣಾಙ್ಕ n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	ಯದಿ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	ಯದಿ (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	/* content type */
	ಯದಿ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_ತುದಿ(buffer, ಪರಿಮಾಣ buffer);
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, ಪರಿಮಾಣ buffer);
	ಮುಕ್ತ(title);
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	ಯದಿ (err == NSERROR_OK) {
		ಯದಿ (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, ಪರಿಮಾಣ buffer);

			ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		}
		nsurl_unref(up);

		ಯದಿ (ctx->aborted)
			ಗಚ್ಛ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, ಪರಿಮಾಣ buffer);
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	ಪ್ರತಿ (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       ಪರಿಮಾಣ(buffer));

		ಯದಿ (err == NSERROR_OK) {
			ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
			ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
				ಗಚ್ಛ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, ಪರಿಮಾಣ buffer);
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_file_process_dir_aborted;

	ಸಂದೇಶ.type = FETCH_FINISHED;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

fetch_file_process_dir_aborted:

	ಯದಿ (listing != ರಿಕ್ತ) {
		ಪ್ರತಿ (i = 0; i < n; i++) {
			ಮುಕ್ತ(listing[i]);
		}
		ಮುಕ್ತ(listing);
	}
}


/* process a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process(ಸಂರಚನಾ fetch_file_context *ctx)
{
	ಸಂರಚನಾ stat fdstat; /**< The objects stat */

	ಯದಿ (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ಪ್ರತ್ಯಾಗಮ;
	} ಅನ್ಯಥಾ ಯದಿ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ಪ್ರತ್ಯಾಗಮ;
	} ಅನ್ಯಥಾ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ಪ್ರತ್ಯಾಗಮ;
}

/** callback to poll for additional file fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_poll(lwc_string *scheme)
{
	ಸಂರಚನಾ fetch_file_context *c, *save_ring = ರಿಕ್ತ;

	ಯಾವತ್ (ring != ರಿಕ್ತ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಯದಿ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಅನುವರ್ತ;
		}

		/* Only process non-aborted fetches */
		ಯದಿ (c->aborted == false) {
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

nserror fetch_file_register(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.ಮುಕ್ತ = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(scheme, &fetcher_ops);
}
