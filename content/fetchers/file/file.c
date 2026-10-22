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

#அந்தர்பாவ "utils/config.h"

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdarg.h>
#ifdef HAVE_MMAP
#அந்தர்பாவ <sys/mman.h>
#endif
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/dirent.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/time.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "utils/file.h"
#அந்தர்பாவ "netsurf/fetch.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"
#அந்தர்பாவ "dirlist.h"
#அந்தர்பாவ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
கட்டமைப்பு fetch_file_context {
	கட்டமைப்பு fetch_file_context *r_next, *r_prev;

	கட்டமைப்பு fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	எழுத்து *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

நிலையான கட்டமைப்பு fetch_file_context *ring = வெற்று;

/** issue fetch callbacks with locking */
நிலையான உள்வரி bool fetch_file_send_callback(மாறிலி fetch_செய்தி *செய்தி,
		கட்டமைப்பு fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(செய்தி, ctx->fetchh);
	ctx->locked = false;

	திரும்பு ctx->aborted;
}

நிலையான bool fetch_file_send_header(கட்டமைப்பு fetch_file_context *ctx,
		மாறிலி எழுத்து *fmt, ...)
{
	fetch_செய்தி செய்தி;
	எழுத்து header[64];
	va_list ap;
	முழு len;

	va_start(ap, fmt);
	len = vsnprintf(header, அளவு header, fmt, ap);
	va_end(ap);

	எனில் (len >= (முழு)அளவு(header) || len < 0) {
		திரும்பு false;
	}

	செய்தி.type = FETCH_HEADER;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) header;
	செய்தி.data.header_or_data.len = len;

	திரும்பு fetch_file_send_callback(&செய்தி, ctx);
}

/** callback to initialise the file fetcher. */
நிலையான bool fetch_file_initialise(lwc_string *scheme)
{
	திரும்பு true;
}

/** callback to initialise the file fetcher. */
நிலையான வெறுமை fetch_file_finalise(lwc_string *scheme)
{
}

நிலையான bool fetch_file_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}

/** callback to set up a file fetch context. */
நிலையான வெறுமை *
fetch_file_setup(கட்டமைப்பு fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 மாறிலி எழுத்து *post_urlenc,
		 மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		 மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு fetch_file_context *ctx;
	முழு i;
	nserror ret;

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று)
		திரும்பு வெற்று;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	எனில் (ret != NSERROR_OK) {
		விடுவி(ctx);
		திரும்பு வெற்று;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	க்கு (i = 0; headers[i] != வெற்று; i++) {
		எனில் (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			தொடர்;
		}

		/* If-None-Match: "12345678" */
		மாறிலி எழுத்து *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		போது (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		எனில் (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			எனில் (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_செருகு(ring, ctx);

	திரும்பு ctx;
}

/** callback to free a file fetch */
நிலையான வெறுமை fetch_file_free(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	விடுவி(c->path);
	விடுவி(ctx);
}

/** callback to start a file fetch */
நிலையான bool fetch_file_start(வெறுமை *ctx)
{
	திரும்பு true;
}

/** callback to abort a file fetch */
நிலையான வெறுமை fetch_file_abort(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

நிலையான முழு fetch_file_errno_to_http_code(முழு error_no)
{
	தேர்வு (error_no) {
	நிலை ENAMETOOLONG:
		திரும்பு 400;
	நிலை EACCES:
		திரும்பு 403;
	நிலை ENOENT:
		திரும்பு 404;
	இயல்பு:
		நிறுத்து;
	}

	திரும்பு 500;
}

நிலையான வெறுமை fetch_file_process_error(கட்டமைப்பு fetch_file_context *ctx, முழு code)
{
	fetch_செய்தி செய்தி;
	எழுத்து buffer[1024];
	மாறிலி எழுத்து *title;
	எழுத்து சாவி[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	எனில் (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		செல் fetch_file_process_error_aborted;

	snprintf(சாவி, அளவு சாவி, "HTTP%03d", code);
	title = messages_get(சாவி);

	snprintf(buffer, அளவு buffer,
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

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buffer;
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_file_send_callback(&செய்தி, ctx))
		செல் fetch_file_process_error_aborted;

	செய்தி.type = FETCH_FINISHED;
	fetch_file_send_callback(&செய்தி, ctx);

fetch_file_process_error_aborted:
	திரும்பு;
}


/** Process object as a regular file */
நிலையான வெறுமை fetch_file_process_plain(கட்டமைப்பு fetch_file_context *ctx,
				     கட்டமைப்பு stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_செய்தி செய்தி;
	எழுத்து *buf = வெற்று;
	size_t buf_size;

	முழு fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	எனில் (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		செய்தி.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&செய்தி, ctx);
		திரும்பு;
	}

	fd = open(ctx->path, O_RDONLY);
	எனில் (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		திரும்பு;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	எனில் (buf_size > 0) {
		buf = mmap(வெற்று, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		எனில் (buf == MAP_FAILED) {
			செய்தி.type = FETCH_ERROR;
			செய்தி.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&செய்தி, ctx);
			close(fd);
			திரும்பு;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	எனில் (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		செல் fetch_file_process_aborted;
	}

	/* content length */
	எனில் (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		செல் fetch_file_process_aborted;
	}

	/* create etag */
	எனில் (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		செல் fetch_file_process_aborted;
	}

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buf;
	செய்தி.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&செய்தி, ctx);

	எனில் (ctx->aborted == false) {
		செய்தி.type = FETCH_FINISHED;
		fetch_file_send_callback(&செய்தி, ctx);
	}

fetch_file_process_aborted:

	எனில் (buf != வெற்று)
		munmap(buf, buf_size);
	close(fd);
#இல்லையெனில்
	fetch_செய்தி செய்தி;
	எழுத்து *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	எனில் (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		செய்தி.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&செய்தி, ctx);
		திரும்பு;
	}

	infile = fopen(ctx->path, "rb");
	எனில் (infile == வெற்று) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		திரும்பு;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	எனில் (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	எனில் (buf == வெற்று) {
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&செய்தி, ctx);
		fclose(infile);
		திரும்பு;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	எனில் (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		செல் fetch_file_process_aborted;
	}

	/* content length */
	எனில் (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		செல் fetch_file_process_aborted;
	}

	/* create etag */
	எனில் (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		செல் fetch_file_process_aborted;
	}

	/* main data loop */
	போது (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		எனில் (res == 0) {
			எனில் (feof(infile)) {
				செய்தி.type = FETCH_ERROR;
				செய்தி.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&செய்தி, ctx);
				செல் fetch_file_process_aborted;
			} இல்லையெனில் {
				செய்தி.type = FETCH_ERROR;
				செய்தி.data.error = "Error reading file";
				fetch_file_send_callback(&செய்தி, ctx);
				செல் fetch_file_process_aborted;
			}
		}
		tot_read += res;

		செய்தி.type = FETCH_DATA;
		செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buf;
		செய்தி.data.header_or_data.len = res;
		எனில் (fetch_file_send_callback(&செய்தி, ctx))
			நிறுத்து;
	}

	எனில் (ctx->aborted == false) {
		செய்தி.type = FETCH_FINISHED;
		fetch_file_send_callback(&செய்தி, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	விடுவி(buf);
#endif
	திரும்பு;
}

நிலையான எழுத்து *gen_nice_title(எழுத்து *path)
{
	எழுத்து *nice_path, *cnv, *tmp;
	எழுத்து *title;
	முழு title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	எனில் (nice_path == வெற்று) {
		திரும்பு வெற்று;
	}

	/* Escape special HTML characters */
	க்கு (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		எனில் (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} இல்லையெனில் எனில் (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} இல்லையெனில் எனில் (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} இல்லையெனில் {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	எனில் (title == வெற்று) {
		விடுவி(nice_path);
		திரும்பு வெற்று;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	விடுவி(nice_path);

	திரும்பு title;
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
நிலையான nserror
process_dir_ent(கட்டமைப்பு fetch_file_context *ctx,
		 கட்டமைப்பு dirent *ent,
		 bool even,
		 எழுத்து *buffer,
		 size_t buffer_len)
{
	nserror ret;
	எழுத்து *urlpath = வெற்று; /* buffer for leaf entry path */
	கட்டமைப்பு stat ent_stat; /* stat result of leaf entry */
	எழுத்து datebuf[64]; /* buffer for date text */
	எழுத்து timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	எனில் (ent->d_name[0] == '.') {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, வெற்று, 2, ctx->path, ent->d_name);
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}

	எனில் (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} இல்லையெனில் {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		எனில் (strftime((எழுத்து *)&datebuf, அளவு datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		எனில் (strftime((எழுத்து *)&timebuf, அளவு timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	எனில் (ret != NSERROR_OK) {
		விடுவி(urlpath);
		திரும்பு ret;
	}

	எனில் (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} இல்லையெனில் எனில் (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} இல்லையெனில் {
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
	விடுவி(urlpath);

	திரும்பு NSERROR_OK;
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
நிலையான முழு dir_sort_alpha(மாறிலி கட்டமைப்பு dirent **d1, மாறிலி கட்டமைப்பு dirent **d2)
{
	மாறிலி எழுத்து *s1 = (*d1)->d_name;
	மாறிலி எழுத்து *s2 = (*d2)->d_name;

	போது (*s1 != '\0' && *s2 != '\0') {
		எனில் ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			முழு n1 = 0,  n2 = 0;
			போது (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			போது (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			எனில் (n1 != n2) {
				திரும்பு n1 - n2;
			}
			எனில் (*s1 == '\0' || *s2 == '\0')
				நிறுத்து;
		}
		எனில் (tolower(*s1) != tolower(*s2))
			நிறுத்து;

		s1++;
		s2++;
	}

	திரும்பு tolower(*s1) - tolower(*s2);
}

நிலையான வெறுமை fetch_file_process_dir(கட்டமைப்பு fetch_file_context *ctx,
				   கட்டமைப்பு stat *fdstat)
{
	fetch_செய்தி செய்தி;
	எழுத்து buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	எழுத்து *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	கட்டமைப்பு dirent **listing = வெற்று; /* directory entry listing */
	முழு i; /* directory entry index */
	முழு n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	எனில் (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		திரும்பு;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	எனில் (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		செல் fetch_file_process_dir_aborted;

	/* content type */
	எனில் (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		செல் fetch_file_process_dir_aborted;

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_உச்சி(buffer, அளவு buffer);
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_file_send_callback(&செய்தி, ctx))
		செல் fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, அளவு buffer);
	விடுவி(title);
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_file_send_callback(&செய்தி, ctx))
		செல் fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	எனில் (err == NSERROR_OK) {
		எனில் (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, அளவு buffer);

			செய்தி.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&செய்தி, ctx);
		}
		nsurl_unref(up);

		எனில் (ctx->aborted)
			செல் fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, அளவு buffer);
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_file_send_callback(&செய்தி, ctx))
		செல் fetch_file_process_dir_aborted;

	க்கு (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       அளவு(buffer));

		எனில் (err == NSERROR_OK) {
			செய்தி.data.header_or_data.len = strlen(buffer);
			எனில் (fetch_file_send_callback(&செய்தி, ctx))
				செல் fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, அளவு buffer);
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_file_send_callback(&செய்தி, ctx))
		செல் fetch_file_process_dir_aborted;

	செய்தி.type = FETCH_FINISHED;
	fetch_file_send_callback(&செய்தி, ctx);

fetch_file_process_dir_aborted:

	எனில் (listing != வெற்று) {
		க்கு (i = 0; i < n; i++) {
			விடுவி(listing[i]);
		}
		விடுவி(listing);
	}
}


/* process a file fetch */
நிலையான வெறுமை fetch_file_process(கட்டமைப்பு fetch_file_context *ctx)
{
	கட்டமைப்பு stat fdstat; /**< The objects stat */

	எனில் (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		திரும்பு;
	}

	எனில் (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		திரும்பு;
	} இல்லையெனில் எனில் (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		திரும்பு;
	} இல்லையெனில் {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	திரும்பு;
}

/** callback to poll for additional file fetch contents */
நிலையான வெறுமை fetch_file_poll(lwc_string *scheme)
{
	கட்டமைப்பு fetch_file_context *c, *save_ring = வெற்று;

	போது (ring != வெற்று) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		எனில் (c->locked == true) {
			RING_செருகு(save_ring, c);
			தொடர்;
		}

		/* Only process non-aborted fetches */
		எனில் (c->aborted == false) {
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

nserror fetch_file_register(வெறுமை)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.விடுவி = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	திரும்பு fetcher_add(scheme, &fetcher_ops);
}
