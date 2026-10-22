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

#ଅନ୍ତର୍ଭାଵ "utils/config.h"

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ifdef HAVE_MMAP
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#endif
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/dirent.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/time.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "utils/file.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/fetch.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"
#ଅନ୍ତର୍ଭାଵ "dirlist.h"
#ଅନ୍ତର୍ଭାଵ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
ସଂରଚନା fetch_file_context {
	ସଂରଚନା fetch_file_context *r_next, *r_prev;

	ସଂରଚନା fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	ଅକ୍ଷର *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ସ୍ଥିର ସଂରଚନା fetch_file_context *ring = ଖାଲି;

/** issue fetch callbacks with locking */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool fetch_file_send_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ,
		ସଂରଚନା fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ସନ୍ଦେଶ, ctx->fetchh);
	ctx->locked = false;

	ଫେରନ୍ତୁ ctx->aborted;
}

ସ୍ଥିର bool fetch_file_send_header(ସଂରଚନା fetch_file_context *ctx,
		ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର header[64];
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	va_start(ap, fmt);
	len = vsnprintf(header, ଆକାର header, fmt, ap);
	va_end(ap);

	ଯଦି (len >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(header) || len < 0) {
		ଫେରନ୍ତୁ false;
	}

	ସନ୍ଦେଶ.type = FETCH_HEADER;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) header;
	ସନ୍ଦେଶ.data.header_or_data.len = len;

	ଫେରନ୍ତୁ fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
}

/** callback to initialise the file fetcher. */
ସ୍ଥିର bool fetch_file_initialise(lwc_string *scheme)
{
	ଫେରନ୍ତୁ true;
}

/** callback to initialise the file fetcher. */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_finalise(lwc_string *scheme)
{
}

ସ୍ଥିର bool fetch_file_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ true;
}

/** callback to set up a file fetch context. */
ସ୍ଥିର ଶୂନ୍ୟ *
fetch_file_setup(ସଂରଚନା fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ଅଚଳ ଅକ୍ଷର *post_urlenc,
		 ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		 ଅଚଳ ଅକ୍ଷର **headers)
{
	ସଂରଚନା fetch_file_context *ctx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	nserror ret;

	ctx = calloc(1, ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	ଯଦି (ret != NSERROR_OK) {
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ପାଇଁ (i = 0; headers[i] != ଖାଲି; i++) {
		ଯଦି (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ଜାରି;
		}

		/* If-None-Match: "12345678" */
		ଅଚଳ ଅକ୍ଷର *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ଯେତେବେଳେ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ଯଦି (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			ଯଦି (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ଭର୍ତ୍ତି(ring, ctx);

	ଫେରନ୍ତୁ ctx;
}

/** callback to free a file fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_free(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	ମୁକ୍ତ(c->path);
	ମୁକ୍ତ(ctx);
}

/** callback to start a file fetch */
ସ୍ଥିର bool fetch_file_start(ଶୂନ୍ୟ *ctx)
{
	ଫେରନ୍ତୁ true;
}

/** callback to abort a file fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_abort(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fetch_file_errno_to_http_code(ପୂର୍ଣ୍ଣସଂଖ୍ୟା error_no)
{
	ବାଛ (error_no) {
	ସ୍ଥିତି ENAMETOOLONG:
		ଫେରନ୍ତୁ 400;
	ସ୍ଥିତି EACCES:
		ଫେରନ୍ତୁ 403;
	ସ୍ଥିତି ENOENT:
		ଫେରନ୍ତୁ 404;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ 500;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_file_process_error(ସଂରଚନା fetch_file_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା code)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର buffer[1024];
	ଅଚଳ ଅକ୍ଷର *title;
	ଅକ୍ଷର ଚାବି[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	ଯଦି (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ଯାଅ fetch_file_process_error_aborted;

	snprintf(ଚାବି, ଆକାର ଚାବି, "HTTP%03d", code);
	title = messages_get(ଚାବି);

	snprintf(buffer, ଆକାର buffer,
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

	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) buffer;
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_file_process_error_aborted;

	ସନ୍ଦେଶ.type = FETCH_FINISHED;
	fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);

fetch_file_process_error_aborted:
	ଫେରନ୍ତୁ;
}


/** Process object as a regular file */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_process_plain(ସଂରଚନା fetch_file_context *ctx,
				     ସଂରଚନା stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର *buf = ଖାଲି;
	size_t buf_size;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ଯଦି (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		ସନ୍ଦେଶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
		ଫେରନ୍ତୁ;
	}

	fd = open(ctx->path, O_RDONLY);
	ଯଦି (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ଫେରନ୍ତୁ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	ଯଦି (buf_size > 0) {
		buf = mmap(ଖାଲି, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		ଯଦି (buf == MAP_FAILED) {
			ସନ୍ଦେଶ.type = FETCH_ERROR;
			ସନ୍ଦେଶ.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
			close(fd);
			ଫେରନ୍ତୁ;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ଯଦି (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		ଯାଅ fetch_file_process_aborted;
	}

	/* content length */
	ଯଦି (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ଯାଅ fetch_file_process_aborted;
	}

	/* create etag */
	ଯଦି (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		ଯାଅ fetch_file_process_aborted;
	}

	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) buf;
	ସନ୍ଦେଶ.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);

	ଯଦି (ctx->aborted == false) {
		ସନ୍ଦେଶ.type = FETCH_FINISHED;
		fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
	}

fetch_file_process_aborted:

	ଯଦି (buf != ଖାଲି)
		munmap(buf, buf_size);
	close(fd);
#ଅନ୍ୟଥା
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ଯଦି (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		ସନ୍ଦେଶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
		ଫେରନ୍ତୁ;
	}

	infile = fopen(ctx->path, "rb");
	ଯଦି (infile == ଖାଲି) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ଫେରନ୍ତୁ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	ଯଦି (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	ଯଦି (buf == ଖାଲି) {
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
		fclose(infile);
		ଫେରନ୍ତୁ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ଯଦି (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		ଯାଅ fetch_file_process_aborted;
	}

	/* content length */
	ଯଦି (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ଯାଅ fetch_file_process_aborted;
	}

	/* create etag */
	ଯଦି (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		ଯାଅ fetch_file_process_aborted;
	}

	/* main data loop */
	ଯେତେବେଳେ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		ଯଦି (res == 0) {
			ଯଦି (feof(infile)) {
				ସନ୍ଦେଶ.type = FETCH_ERROR;
				ସନ୍ଦେଶ.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
				ଯାଅ fetch_file_process_aborted;
			} ଅନ୍ୟଥା {
				ସନ୍ଦେଶ.type = FETCH_ERROR;
				ସନ୍ଦେଶ.data.error = "Error reading file";
				fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
				ଯାଅ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		ସନ୍ଦେଶ.type = FETCH_DATA;
		ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) buf;
		ସନ୍ଦେଶ.data.header_or_data.len = res;
		ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
			ଭାଙ୍ଗ;
	}

	ଯଦି (ctx->aborted == false) {
		ସନ୍ଦେଶ.type = FETCH_FINISHED;
		fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	ମୁକ୍ତ(buf);
#endif
	ଫେରନ୍ତୁ;
}

ସ୍ଥିର ଅକ୍ଷର *gen_nice_title(ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *nice_path, *cnv, *tmp;
	ଅକ୍ଷର *title;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	ଯଦି (nice_path == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Escape special HTML characters */
	ପାଇଁ (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		ଯଦି (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ଅନ୍ୟଥା ଯଦି (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ଅନ୍ୟଥା ଯଦି (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ଅନ୍ୟଥା {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	ଯଦି (title == ଖାଲି) {
		ମୁକ୍ତ(nice_path);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	ମୁକ୍ତ(nice_path);

	ଫେରନ୍ତୁ title;
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
ସ୍ଥିର nserror
process_dir_ent(ସଂରଚନା fetch_file_context *ctx,
		 ସଂରଚନା dirent *ent,
		 bool even,
		 ଅକ୍ଷର *buffer,
		 size_t buffer_len)
{
	nserror ret;
	ଅକ୍ଷର *urlpath = ଖାଲି; /* buffer for leaf entry path */
	ସଂରଚନା stat ent_stat; /* stat result of leaf entry */
	ଅକ୍ଷର datebuf[64]; /* buffer for date text */
	ଅକ୍ଷର timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ଯଦି (ent->d_name[0] == '.') {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ଖାଲି, 2, ctx->path, ent->d_name);
	ଯଦି (ret != NSERROR_OK) {
		ଫେରନ୍ତୁ ret;
	}

	ଯଦି (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ଅନ୍ୟଥା {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ଯଦି (strftime((ଅକ୍ଷର *)&datebuf, ଆକାର datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ଯଦି (strftime((ଅକ୍ଷର *)&timebuf, ଆକାର timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	ଯଦି (ret != NSERROR_OK) {
		ମୁକ୍ତ(urlpath);
		ଫେରନ୍ତୁ ret;
	}

	ଯଦି (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ଅନ୍ୟଥା ଯଦି (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ଅନ୍ୟଥା {
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
	ମୁକ୍ତ(urlpath);

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir_sort_alpha(ଅଚଳ ସଂରଚନା dirent **d1, ଅଚଳ ସଂରଚନା dirent **d2)
{
	ଅଚଳ ଅକ୍ଷର *s1 = (*d1)->d_name;
	ଅଚଳ ଅକ୍ଷର *s2 = (*d2)->d_name;

	ଯେତେବେଳେ (*s1 != '\0' && *s2 != '\0') {
		ଯଦି ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା n1 = 0,  n2 = 0;
			ଯେତେବେଳେ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ଯେତେବେଳେ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ଯଦି (n1 != n2) {
				ଫେରନ୍ତୁ n1 - n2;
			}
			ଯଦି (*s1 == '\0' || *s2 == '\0')
				ଭାଙ୍ଗ;
		}
		ଯଦି (tolower(*s1) != tolower(*s2))
			ଭାଙ୍ଗ;

		s1++;
		s2++;
	}

	ଫେରନ୍ତୁ tolower(*s1) - tolower(*s2);
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_file_process_dir(ସଂରଚନା fetch_file_context *ctx,
				   ସଂରଚନା stat *fdstat)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	ଅକ୍ଷର *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	ସଂରଚନା dirent **listing = ଖାଲି; /* directory entry listing */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i; /* directory entry index */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	ଯଦି (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		ଫେରନ୍ତୁ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	ଯଦି (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		ଯାଅ fetch_file_process_dir_aborted;

	/* content type */
	ଯଦି (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ଯାଅ fetch_file_process_dir_aborted;

	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_ଶୀର୍ଷ(buffer, ଆକାର buffer);
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, ଆକାର buffer);
	ମୁକ୍ତ(title);
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	ଯଦି (err == NSERROR_OK) {
		ଯଦି (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, ଆକାର buffer);

			ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);
		}
		nsurl_unref(up);

		ଯଦି (ctx->aborted)
			ଯାଅ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, ଆକାର buffer);
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_file_process_dir_aborted;

	ପାଇଁ (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       ଆକାର(buffer));

		ଯଦି (err == NSERROR_OK) {
			ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
			ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
				ଯାଅ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, ଆକାର buffer);
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_file_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_file_process_dir_aborted;

	ସନ୍ଦେଶ.type = FETCH_FINISHED;
	fetch_file_send_callback(&ସନ୍ଦେଶ, ctx);

fetch_file_process_dir_aborted:

	ଯଦି (listing != ଖାଲି) {
		ପାଇଁ (i = 0; i < n; i++) {
			ମୁକ୍ତ(listing[i]);
		}
		ମୁକ୍ତ(listing);
	}
}


/* process a file fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_process(ସଂରଚନା fetch_file_context *ctx)
{
	ସଂରଚନା stat fdstat; /**< The objects stat */

	ଯଦି (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ଫେରନ୍ତୁ;
	}

	ଯଦି (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ଫେରନ୍ତୁ;
	} ଅନ୍ୟଥା ଯଦି (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ଫେରନ୍ତୁ;
	} ଅନ୍ୟଥା {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ଫେରନ୍ତୁ;
}

/** callback to poll for additional file fetch contents */
ସ୍ଥିର ଶୂନ୍ୟ fetch_file_poll(lwc_string *scheme)
{
	ସଂରଚନା fetch_file_context *c, *save_ring = ଖାଲି;

	ଯେତେବେଳେ (ring != ଖାଲି) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ଯଦି (c->locked == true) {
			RING_ଭର୍ତ୍ତି(save_ring, c);
			ଜାରି;
		}

		/* Only process non-aborted fetches */
		ଯଦି (c->aborted == false) {
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

nserror fetch_file_register(ଶୂନ୍ୟ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ଅଚଳ ସଂରଚନା fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.ମୁକ୍ତ = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ଫେରନ୍ତୁ fetcher_add(scheme, &fetcher_ops);
}
