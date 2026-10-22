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

#ضمن "utils/config.h"

#ضمن <stdlib.h>
#ضمن <ctype.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <fcntl.h>
#ضمن <unistd.h>
#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ضمن <stdio.h>
#ضمن <stdarg.h>
#ifdef HAVE_MMAP
#ضمن <sys/mman.h>
#endif
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/dirent.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/time.h"
#ضمن "utils/ring.h"
#ضمن "utils/file.h"
#ضمن "netsurf/fetch.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "dirlist.h"
#ضمن "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
بنية fetch_file_context {
	بنية fetch_file_context *r_next, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	حرف *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ساكن بنية fetch_file_context *ring = عدم;

/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_file_send_callback(ثابت fetch_msg *msg,
		بنية fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(msg, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

ساكن bool fetch_file_send_header(بنية fetch_file_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_msg msg;
	حرف header[64];
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(header, حجم header, fmt, ap);
	va_end(ap);

	𐡄𐡍 (len >= (صحيح)حجم(header) || len < 0) {
		أرجع false;
	}

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.buf = (ثابت uint8_t *) header;
	msg.data.header_or_data.len = len;

	أرجع fetch_file_send_callback(&msg, ctx);
}

/** callback to initialise the file fetcher. */
ساكن bool fetch_file_initialise(lwc_string *scheme)
{
	أرجع true;
}

/** callback to initialise the file fetcher. */
ساكن فراغ fetch_file_finalise(lwc_string *scheme)
{
}

ساكن bool fetch_file_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

/** callback to set up a file fetch context. */
ساكن فراغ *
fetch_file_setup(بنية fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_data *post_multipart,
		 ثابت حرف **headers)
{
	بنية fetch_file_context *ctx;
	صحيح i;
	nserror ret;

	ctx = calloc(1, حجم(*ctx));
	𐡄𐡍 (ctx == عدم)
		أرجع عدم;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	𐡄𐡍 (ret != NSERROR_OK) {
		حرر(ctx);
		أرجع عدم;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	لكل (i = 0; headers[i] != عدم; i++) {
		𐡄𐡍 (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			استمر;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		طالما (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		𐡄𐡍 (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			𐡄𐡍 (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_INSERT(ring, ctx);

	أرجع ctx;
}

/** callback to free a file fetch */
ساكن فراغ fetch_file_free(فراغ *ctx)
{
	بنية fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	حرر(c->path);
	حرر(ctx);
}

/** callback to start a file fetch */
ساكن bool fetch_file_start(فراغ *ctx)
{
	أرجع true;
}

/** callback to abort a file fetch */
ساكن فراغ fetch_file_abort(فراغ *ctx)
{
	بنية fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن صحيح fetch_file_errno_to_http_code(صحيح error_no)
{
	بدل (error_no) {
	حالة ENAMETOOLONG:
		أرجع 400;
	حالة EACCES:
		أرجع 403;
	حالة ENOENT:
		أرجع 404;
	افتراضي:
		اكسر;
	}

	أرجع 500;
}

ساكن فراغ fetch_file_process_error(بنية fetch_file_context *ctx, صحيح code)
{
	fetch_msg msg;
	حرف buffer[1024];
	ثابت حرف *title;
	حرف key[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_error_aborted;

	snprintf(key, حجم key, "HTTP%03d", code);
	title = messages_get(key);

	snprintf(buffer, حجم buffer,
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

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (ثابت uint8_t *) buffer;
	msg.data.header_or_data.len = strlen(buffer);
	𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
		اذهب fetch_file_process_error_aborted;

	msg.type = FETCH_FINISHED;
	fetch_file_send_callback(&msg, ctx);

fetch_file_process_error_aborted:
	أرجع;
}


/** Process object as a regular file */
ساكن فراغ fetch_file_process_plain(بنية fetch_file_context *ctx,
				     بنية stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_msg msg;
	حرف *buf = عدم;
	size_t buf_size;

	صحيح fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	𐡄𐡍 (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		msg.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&msg, ctx);
		أرجع;
	}

	fd = open(ctx->path, O_RDONLY);
	𐡄𐡍 (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	𐡄𐡍 (buf_size > 0) {
		buf = mmap(عدم, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		𐡄𐡍 (buf == MAP_FAILED) {
			msg.type = FETCH_ERROR;
			msg.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&msg, ctx);
			close(fd);
			أرجع;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	𐡄𐡍 (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (ثابت uint8_t *) buf;
	msg.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&msg, ctx);

	𐡄𐡍 (ctx->aborted == false) {
		msg.type = FETCH_FINISHED;
		fetch_file_send_callback(&msg, ctx);
	}

fetch_file_process_aborted:

	𐡄𐡍 (buf != عدم)
		munmap(buf, buf_size);
	close(fd);
#وإلا
	fetch_msg msg;
	حرف *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	𐡄𐡍 (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		msg.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&msg, ctx);
		أرجع;
	}

	infile = fopen(ctx->path, "rb");
	𐡄𐡍 (infile == عدم) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	𐡄𐡍 (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	𐡄𐡍 (buf == عدم) {
		msg.type = FETCH_ERROR;
		msg.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&msg, ctx);
		fclose(infile);
		أرجع;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	𐡄𐡍 (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	/* main data loop */
	طالما (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		𐡄𐡍 (res == 0) {
			𐡄𐡍 (feof(infile)) {
				msg.type = FETCH_ERROR;
				msg.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&msg, ctx);
				اذهب fetch_file_process_aborted;
			} وإلا {
				msg.type = FETCH_ERROR;
				msg.data.error = "Error reading file";
				fetch_file_send_callback(&msg, ctx);
				اذهب fetch_file_process_aborted;
			}
		}
		tot_read += res;

		msg.type = FETCH_DATA;
		msg.data.header_or_data.buf = (ثابت uint8_t *) buf;
		msg.data.header_or_data.len = res;
		𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
			اكسر;
	}

	𐡄𐡍 (ctx->aborted == false) {
		msg.type = FETCH_FINISHED;
		fetch_file_send_callback(&msg, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	حرر(buf);
#endif
	أرجع;
}

ساكن حرف *gen_nice_title(حرف *path)
{
	حرف *nice_path, *cnv, *tmp;
	حرف *title;
	صحيح title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	𐡄𐡍 (nice_path == عدم) {
		أرجع عدم;
	}

	/* Escape special HTML characters */
	لكل (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		𐡄𐡍 (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} وإلا 𐡄𐡍 (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} وإلا 𐡄𐡍 (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} وإلا {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	𐡄𐡍 (title == عدم) {
		حرر(nice_path);
		أرجع عدم;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	حرر(nice_path);

	أرجع title;
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
ساكن nserror
process_dir_ent(بنية fetch_file_context *ctx,
		 بنية dirent *ent,
		 bool even,
		 حرف *buffer,
		 size_t buffer_len)
{
	nserror ret;
	حرف *urlpath = عدم; /* buffer for leaf entry path */
	بنية stat ent_stat; /* stat result of leaf entry */
	حرف datebuf[64]; /* buffer for date text */
	حرف timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	𐡄𐡍 (ent->d_name[0] == '.') {
		أرجع NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, عدم, 2, ctx->path, ent->d_name);
	𐡄𐡍 (ret != NSERROR_OK) {
		أرجع ret;
	}

	𐡄𐡍 (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} وإلا {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		𐡄𐡍 (strftime((حرف *)&datebuf, حجم datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		𐡄𐡍 (strftime((حرف *)&timebuf, حجم timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	𐡄𐡍 (ret != NSERROR_OK) {
		حرر(urlpath);
		أرجع ret;
	}

	𐡄𐡍 (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} وإلا 𐡄𐡍 (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} وإلا {
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
	حرر(urlpath);

	أرجع NSERROR_OK;
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
ساكن صحيح dir_sort_alpha(ثابت بنية dirent **d1, ثابت بنية dirent **d2)
{
	ثابت حرف *s1 = (*d1)->d_name;
	ثابت حرف *s2 = (*d2)->d_name;

	طالما (*s1 != '\0' && *s2 != '\0') {
		𐡄𐡍 ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			صحيح n1 = 0,  n2 = 0;
			طالما (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			طالما (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			𐡄𐡍 (n1 != n2) {
				أرجع n1 - n2;
			}
			𐡄𐡍 (*s1 == '\0' || *s2 == '\0')
				اكسر;
		}
		𐡄𐡍 (tolower(*s1) != tolower(*s2))
			اكسر;

		s1++;
		s2++;
	}

	أرجع tolower(*s1) - tolower(*s2);
}

ساكن فراغ fetch_file_process_dir(بنية fetch_file_context *ctx,
				   بنية stat *fdstat)
{
	fetch_msg msg;
	حرف buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	حرف *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	بنية dirent **listing = عدم; /* directory entry listing */
	صحيح i; /* directory entry index */
	صحيح n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	𐡄𐡍 (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	𐡄𐡍 (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		اذهب fetch_file_process_dir_aborted;

	/* content type */
	𐡄𐡍 (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_dir_aborted;

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (ثابت uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_top(buffer, حجم buffer);
	msg.data.header_or_data.len = strlen(buffer);
	𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, حجم buffer);
	حرر(title);
	msg.data.header_or_data.len = strlen(buffer);
	𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	𐡄𐡍 (err == NSERROR_OK) {
		𐡄𐡍 (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, حجم buffer);

			msg.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&msg, ctx);
		}
		nsurl_unref(up);

		𐡄𐡍 (ctx->aborted)
			اذهب fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, حجم buffer);
	msg.data.header_or_data.len = strlen(buffer);
	𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
		اذهب fetch_file_process_dir_aborted;

	لكل (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       حجم(buffer));

		𐡄𐡍 (err == NSERROR_OK) {
			msg.data.header_or_data.len = strlen(buffer);
			𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
				اذهب fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, حجم buffer);
	msg.data.header_or_data.len = strlen(buffer);
	𐡄𐡍 (fetch_file_send_callback(&msg, ctx))
		اذهب fetch_file_process_dir_aborted;

	msg.type = FETCH_FINISHED;
	fetch_file_send_callback(&msg, ctx);

fetch_file_process_dir_aborted:

	𐡄𐡍 (listing != عدم) {
		لكل (i = 0; i < n; i++) {
			حرر(listing[i]);
		}
		حرر(listing);
	}
}


/* process a file fetch */
ساكن فراغ fetch_file_process(بنية fetch_file_context *ctx)
{
	بنية stat fdstat; /**< The objects stat */

	𐡄𐡍 (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	𐡄𐡍 (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		أرجع;
	} وإلا 𐡄𐡍 (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		أرجع;
	} وإلا {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	أرجع;
}

/** callback to poll for additional file fetch contents */
ساكن فراغ fetch_file_poll(lwc_string *scheme)
{
	بنية fetch_file_context *c, *save_ring = عدم;

	طالما (ring != عدم) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		𐡄𐡍 (c->locked == true) {
			RING_INSERT(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		𐡄𐡍 (c->aborted == false) {
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

nserror fetch_file_register(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ثابت بنية fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.حرر = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	أرجع fetcher_add(scheme, &fetcher_ops);
}
