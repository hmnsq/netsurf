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

#અન્તર્ભાવ "utils/config.h"

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdarg.h>
#ifdef HAVE_MMAP
#અન્તર્ભાવ <sys/mman.h>
#endif
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/dirent.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/time.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "utils/file.h"
#અન્તર્ભાવ "netsurf/fetch.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"
#અન્તર્ભાવ "dirlist.h"
#અન્તર્ભાવ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
સંરચના fetch_file_context {
	સંરચના fetch_file_context *r_next, *r_prev;

	સંરચના fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	અક્ષર *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

સ્થિર સંરચના fetch_file_context *ring = ખાલી;

/** issue fetch callbacks with locking */
સ્થિર અંતઃરેખ bool fetch_file_send_callback(અચલ fetch_સંદેશ *સંદેશ,
		સંરચના fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(સંદેશ, ctx->fetchh);
	ctx->locked = false;

	પાછા ctx->aborted;
}

સ્થિર bool fetch_file_send_header(સંરચના fetch_file_context *ctx,
		અચલ અક્ષર *fmt, ...)
{
	fetch_સંદેશ સંદેશ;
	અક્ષર header[64];
	va_list ap;
	પૂર્ણાંક len;

	va_start(ap, fmt);
	len = vsnprintf(header, કદ header, fmt, ap);
	va_end(ap);

	જો (len >= (પૂર્ણાંક)કદ(header) || len < 0) {
		પાછા false;
	}

	સંદેશ.type = FETCH_HEADER;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) header;
	સંદેશ.data.header_or_data.len = len;

	પાછા fetch_file_send_callback(&સંદેશ, ctx);
}

/** callback to initialise the file fetcher. */
સ્થિર bool fetch_file_initialise(lwc_string *scheme)
{
	પાછા true;
}

/** callback to initialise the file fetcher. */
સ્થિર શૂન્ય fetch_file_finalise(lwc_string *scheme)
{
}

સ્થિર bool fetch_file_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}

/** callback to set up a file fetch context. */
સ્થિર શૂન્ય *
fetch_file_setup(સંરચના fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 અચલ અક્ષર *post_urlenc,
		 અચલ સંરચના fetch_multipart_data *post_multipart,
		 અચલ અક્ષર **headers)
{
	સંરચના fetch_file_context *ctx;
	પૂર્ણાંક i;
	nserror ret;

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી)
		પાછા ખાલી;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	જો (ret != NSERROR_OK) {
		મુક્ત(ctx);
		પાછા ખાલી;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	માટે (i = 0; headers[i] != ખાલી; i++) {
		જો (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ચાલુ;
		}

		/* If-None-Match: "12345678" */
		અચલ અક્ષર *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		જ્યારે (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		જો (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			જો (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ઉમેરો(ring, ctx);

	પાછા ctx;
}

/** callback to free a file fetch */
સ્થિર શૂન્ય fetch_file_free(શૂન્ય *ctx)
{
	સંરચના fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	મુક્ત(c->path);
	મુક્ત(ctx);
}

/** callback to start a file fetch */
સ્થિર bool fetch_file_start(શૂન્ય *ctx)
{
	પાછા true;
}

/** callback to abort a file fetch */
સ્થિર શૂન્ય fetch_file_abort(શૂન્ય *ctx)
{
	સંરચના fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

સ્થિર પૂર્ણાંક fetch_file_errno_to_http_code(પૂર્ણાંક error_no)
{
	પસંદગી (error_no) {
	સ્થિતિ ENAMETOOLONG:
		પાછા 400;
	સ્થિતિ EACCES:
		પાછા 403;
	સ્થિતિ ENOENT:
		પાછા 404;
	મૂળભૂત:
		તોડો;
	}

	પાછા 500;
}

સ્થિર શૂન્ય fetch_file_process_error(સંરચના fetch_file_context *ctx, પૂર્ણાંક code)
{
	fetch_સંદેશ સંદેશ;
	અક્ષર buffer[1024];
	અચલ અક્ષર *title;
	અક્ષર ચાવી[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	જો (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		જાઓ fetch_file_process_error_aborted;

	snprintf(ચાવી, કદ ચાવી, "HTTP%03d", code);
	title = messages_get(ચાવી);

	snprintf(buffer, કદ buffer,
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

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buffer;
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_file_send_callback(&સંદેશ, ctx))
		જાઓ fetch_file_process_error_aborted;

	સંદેશ.type = FETCH_FINISHED;
	fetch_file_send_callback(&સંદેશ, ctx);

fetch_file_process_error_aborted:
	પાછા;
}


/** Process object as a regular file */
સ્થિર શૂન્ય fetch_file_process_plain(સંરચના fetch_file_context *ctx,
				     સંરચના stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_સંદેશ સંદેશ;
	અક્ષર *buf = ખાલી;
	size_t buf_size;

	પૂર્ણાંક fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	જો (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		સંદેશ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&સંદેશ, ctx);
		પાછા;
	}

	fd = open(ctx->path, O_RDONLY);
	જો (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		પાછા;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	જો (buf_size > 0) {
		buf = mmap(ખાલી, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		જો (buf == MAP_FAILED) {
			સંદેશ.type = FETCH_ERROR;
			સંદેશ.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&સંદેશ, ctx);
			close(fd);
			પાછા;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	જો (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		જાઓ fetch_file_process_aborted;
	}

	/* content length */
	જો (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		જાઓ fetch_file_process_aborted;
	}

	/* create etag */
	જો (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		જાઓ fetch_file_process_aborted;
	}

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buf;
	સંદેશ.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&સંદેશ, ctx);

	જો (ctx->aborted == false) {
		સંદેશ.type = FETCH_FINISHED;
		fetch_file_send_callback(&સંદેશ, ctx);
	}

fetch_file_process_aborted:

	જો (buf != ખાલી)
		munmap(buf, buf_size);
	close(fd);
#નહીંતર
	fetch_સંદેશ સંદેશ;
	અક્ષર *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	જો (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		સંદેશ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&સંદેશ, ctx);
		પાછા;
	}

	infile = fopen(ctx->path, "rb");
	જો (infile == ખાલી) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		પાછા;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	જો (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	જો (buf == ખાલી) {
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&સંદેશ, ctx);
		fclose(infile);
		પાછા;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	જો (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		જાઓ fetch_file_process_aborted;
	}

	/* content length */
	જો (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		જાઓ fetch_file_process_aborted;
	}

	/* create etag */
	જો (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		જાઓ fetch_file_process_aborted;
	}

	/* main data loop */
	જ્યારે (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		જો (res == 0) {
			જો (feof(infile)) {
				સંદેશ.type = FETCH_ERROR;
				સંદેશ.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&સંદેશ, ctx);
				જાઓ fetch_file_process_aborted;
			} નહીંતર {
				સંદેશ.type = FETCH_ERROR;
				સંદેશ.data.error = "Error reading file";
				fetch_file_send_callback(&સંદેશ, ctx);
				જાઓ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		સંદેશ.type = FETCH_DATA;
		સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buf;
		સંદેશ.data.header_or_data.len = res;
		જો (fetch_file_send_callback(&સંદેશ, ctx))
			તોડો;
	}

	જો (ctx->aborted == false) {
		સંદેશ.type = FETCH_FINISHED;
		fetch_file_send_callback(&સંદેશ, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	મુક્ત(buf);
#endif
	પાછા;
}

સ્થિર અક્ષર *gen_nice_title(અક્ષર *path)
{
	અક્ષર *nice_path, *cnv, *tmp;
	અક્ષર *title;
	પૂર્ણાંક title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	જો (nice_path == ખાલી) {
		પાછા ખાલી;
	}

	/* Escape special HTML characters */
	માટે (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		જો (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} નહીંતર જો (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} નહીંતર જો (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} નહીંતર {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	જો (title == ખાલી) {
		મુક્ત(nice_path);
		પાછા ખાલી;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	મુક્ત(nice_path);

	પાછા title;
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
સ્થિર nserror
process_dir_ent(સંરચના fetch_file_context *ctx,
		 સંરચના dirent *ent,
		 bool even,
		 અક્ષર *buffer,
		 size_t buffer_len)
{
	nserror ret;
	અક્ષર *urlpath = ખાલી; /* buffer for leaf entry path */
	સંરચના stat ent_stat; /* stat result of leaf entry */
	અક્ષર datebuf[64]; /* buffer for date text */
	અક્ષર timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	જો (ent->d_name[0] == '.') {
		પાછા NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ખાલી, 2, ctx->path, ent->d_name);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	જો (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} નહીંતર {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		જો (strftime((અક્ષર *)&datebuf, કદ datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		જો (strftime((અક્ષર *)&timebuf, કદ timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	જો (ret != NSERROR_OK) {
		મુક્ત(urlpath);
		પાછા ret;
	}

	જો (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} નહીંતર જો (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} નહીંતર {
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
	મુક્ત(urlpath);

	પાછા NSERROR_OK;
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
સ્થિર પૂર્ણાંક dir_sort_alpha(અચલ સંરચના dirent **d1, અચલ સંરચના dirent **d2)
{
	અચલ અક્ષર *s1 = (*d1)->d_name;
	અચલ અક્ષર *s2 = (*d2)->d_name;

	જ્યારે (*s1 != '\0' && *s2 != '\0') {
		જો ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			પૂર્ણાંક n1 = 0,  n2 = 0;
			જ્યારે (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			જ્યારે (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			જો (n1 != n2) {
				પાછા n1 - n2;
			}
			જો (*s1 == '\0' || *s2 == '\0')
				તોડો;
		}
		જો (tolower(*s1) != tolower(*s2))
			તોડો;

		s1++;
		s2++;
	}

	પાછા tolower(*s1) - tolower(*s2);
}

સ્થિર શૂન્ય fetch_file_process_dir(સંરચના fetch_file_context *ctx,
				   સંરચના stat *fdstat)
{
	fetch_સંદેશ સંદેશ;
	અક્ષર buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	અક્ષર *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	સંરચના dirent **listing = ખાલી; /* directory entry listing */
	પૂર્ણાંક i; /* directory entry index */
	પૂર્ણાંક n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	જો (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		પાછા;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	જો (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		જાઓ fetch_file_process_dir_aborted;

	/* content type */
	જો (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		જાઓ fetch_file_process_dir_aborted;

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_ટોચ(buffer, કદ buffer);
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_file_send_callback(&સંદેશ, ctx))
		જાઓ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, કદ buffer);
	મુક્ત(title);
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_file_send_callback(&સંદેશ, ctx))
		જાઓ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	જો (err == NSERROR_OK) {
		જો (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, કદ buffer);

			સંદેશ.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&સંદેશ, ctx);
		}
		nsurl_unref(up);

		જો (ctx->aborted)
			જાઓ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, કદ buffer);
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_file_send_callback(&સંદેશ, ctx))
		જાઓ fetch_file_process_dir_aborted;

	માટે (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       કદ(buffer));

		જો (err == NSERROR_OK) {
			સંદેશ.data.header_or_data.len = strlen(buffer);
			જો (fetch_file_send_callback(&સંદેશ, ctx))
				જાઓ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, કદ buffer);
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_file_send_callback(&સંદેશ, ctx))
		જાઓ fetch_file_process_dir_aborted;

	સંદેશ.type = FETCH_FINISHED;
	fetch_file_send_callback(&સંદેશ, ctx);

fetch_file_process_dir_aborted:

	જો (listing != ખાલી) {
		માટે (i = 0; i < n; i++) {
			મુક્ત(listing[i]);
		}
		મુક્ત(listing);
	}
}


/* process a file fetch */
સ્થિર શૂન્ય fetch_file_process(સંરચના fetch_file_context *ctx)
{
	સંરચના stat fdstat; /**< The objects stat */

	જો (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		પાછા;
	}

	જો (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		પાછા;
	} નહીંતર જો (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		પાછા;
	} નહીંતર {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	પાછા;
}

/** callback to poll for additional file fetch contents */
સ્થિર શૂન્ય fetch_file_poll(lwc_string *scheme)
{
	સંરચના fetch_file_context *c, *save_ring = ખાલી;

	જ્યારે (ring != ખાલી) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		જો (c->locked == true) {
			RING_ઉમેરો(save_ring, c);
			ચાલુ;
		}

		/* Only process non-aborted fetches */
		જો (c->aborted == false) {
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

nserror fetch_file_register(શૂન્ય)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.મુક્ત = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	પાછા fetcher_add(scheme, &fetcher_ops);
}
