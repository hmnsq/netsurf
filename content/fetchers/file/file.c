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
ساكن مضمن bool fetch_file_send_callback(ثابت fetch_پەیام *پەیام,
		بنية fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پەیام, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

ساكن bool fetch_file_send_header(بنية fetch_file_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پەیام پەیام;
	حرف header[64];
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(header, حجم header, fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(header) || len < 0) {
		أرجع false;
	}

	پەیام.type = FETCH_HEADER;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *) header;
	پەیام.data.header_or_data.len = len;

	أرجع fetch_file_send_callback(&پەیام, ctx);
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
	إذا (ctx == عدم)
		أرجع عدم;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	إذا (ret != NSERROR_OK) {
		حرر(ctx);
		أرجع عدم;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	لكل (i = 0; headers[i] != عدم; i++) {
		إذا (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			استمر;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		طالما (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		إذا (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			إذا (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_زیادکردن(ring, ctx);

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
	fetch_پەیام پەیام;
	حرف buffer[1024];
	ثابت حرف *title;
	حرف کلیل[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	إذا (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_error_aborted;

	snprintf(کلیل, حجم کلیل, "HTTP%03d", code);
	title = messages_get(کلیل);

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

	پەیام.type = FETCH_DATA;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;
	پەیام.data.header_or_data.len = strlen(buffer);
	إذا (fetch_file_send_callback(&پەیام, ctx))
		اذهب fetch_file_process_error_aborted;

	پەیام.type = FETCH_FINISHED;
	fetch_file_send_callback(&پەیام, ctx);

fetch_file_process_error_aborted:
	أرجع;
}


/** Process object as a regular file */
ساكن فراغ fetch_file_process_plain(بنية fetch_file_context *ctx,
				     بنية stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_پەیام پەیام;
	حرف *buf = عدم;
	size_t buf_size;

	صحيح fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	إذا (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		پەیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پەیام, ctx);
		أرجع;
	}

	fd = open(ctx->path, O_RDONLY);
	إذا (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	إذا (buf_size > 0) {
		buf = mmap(عدم, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		إذا (buf == MAP_FAILED) {
			پەیام.type = FETCH_ERROR;
			پەیام.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&پەیام, ctx);
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
	إذا (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	إذا (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	إذا (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	پەیام.type = FETCH_DATA;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *) buf;
	پەیام.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&پەیام, ctx);

	إذا (ctx->aborted == false) {
		پەیام.type = FETCH_FINISHED;
		fetch_file_send_callback(&پەیام, ctx);
	}

fetch_file_process_aborted:

	إذا (buf != عدم)
		munmap(buf, buf_size);
	close(fd);
#وإلا
	fetch_پەیام پەیام;
	حرف *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	إذا (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		پەیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پەیام, ctx);
		أرجع;
	}

	infile = fopen(ctx->path, "rb");
	إذا (infile == عدم) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	إذا (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	إذا (buf == عدم) {
		پەیام.type = FETCH_ERROR;
		پەیام.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&پەیام, ctx);
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
	إذا (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	إذا (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	إذا (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	/* main data loop */
	طالما (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		إذا (res == 0) {
			إذا (feof(infile)) {
				پەیام.type = FETCH_ERROR;
				پەیام.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&پەیام, ctx);
				اذهب fetch_file_process_aborted;
			} وإلا {
				پەیام.type = FETCH_ERROR;
				پەیام.data.error = "Error reading file";
				fetch_file_send_callback(&پەیام, ctx);
				اذهب fetch_file_process_aborted;
			}
		}
		tot_read += res;

		پەیام.type = FETCH_DATA;
		پەیام.data.header_or_data.buf = (ثابت uint8_t *) buf;
		پەیام.data.header_or_data.len = res;
		إذا (fetch_file_send_callback(&پەیام, ctx))
			اكسر;
	}

	إذا (ctx->aborted == false) {
		پەیام.type = FETCH_FINISHED;
		fetch_file_send_callback(&پەیام, ctx);
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
	إذا (nice_path == عدم) {
		أرجع عدم;
	}

	/* Escape special HTML characters */
	لكل (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		إذا (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} وإلا إذا (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} وإلا إذا (*tmp == '&') {
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

	إذا (title == عدم) {
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
	إذا (ent->d_name[0] == '.') {
		أرجع NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, عدم, 2, ctx->path, ent->d_name);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	إذا (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} وإلا {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		إذا (strftime((حرف *)&datebuf, حجم datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		إذا (strftime((حرف *)&timebuf, حجم timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	إذا (ret != NSERROR_OK) {
		حرر(urlpath);
		أرجع ret;
	}

	إذا (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} وإلا إذا (S_ISDIR(ent_stat.st_mode)) {
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
		إذا ((*s1 >= '0' && *s1 <= '9') &&
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
			إذا (n1 != n2) {
				أرجع n1 - n2;
			}
			إذا (*s1 == '\0' || *s2 == '\0')
				اكسر;
		}
		إذا (tolower(*s1) != tolower(*s2))
			اكسر;

		s1++;
		s2++;
	}

	أرجع tolower(*s1) - tolower(*s2);
}

ساكن فراغ fetch_file_process_dir(بنية fetch_file_context *ctx,
				   بنية stat *fdstat)
{
	fetch_پەیام پەیام;
	حرف buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	حرف *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	بنية dirent **listing = عدم; /* directory entry listing */
	صحيح i; /* directory entry index */
	صحيح n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	إذا (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	إذا (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		اذهب fetch_file_process_dir_aborted;

	/* content type */
	إذا (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_dir_aborted;

	پەیام.type = FETCH_DATA;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_لووتکە(buffer, حجم buffer);
	پەیام.data.header_or_data.len = strlen(buffer);
	إذا (fetch_file_send_callback(&پەیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, حجم buffer);
	حرر(title);
	پەیام.data.header_or_data.len = strlen(buffer);
	إذا (fetch_file_send_callback(&پەیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	إذا (err == NSERROR_OK) {
		إذا (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, حجم buffer);

			پەیام.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&پەیام, ctx);
		}
		nsurl_unref(up);

		إذا (ctx->aborted)
			اذهب fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, حجم buffer);
	پەیام.data.header_or_data.len = strlen(buffer);
	إذا (fetch_file_send_callback(&پەیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	لكل (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       حجم(buffer));

		إذا (err == NSERROR_OK) {
			پەیام.data.header_or_data.len = strlen(buffer);
			إذا (fetch_file_send_callback(&پەیام, ctx))
				اذهب fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, حجم buffer);
	پەیام.data.header_or_data.len = strlen(buffer);
	إذا (fetch_file_send_callback(&پەیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	پەیام.type = FETCH_FINISHED;
	fetch_file_send_callback(&پەیام, ctx);

fetch_file_process_dir_aborted:

	إذا (listing != عدم) {
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

	إذا (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		أرجع;
	}

	إذا (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		أرجع;
	} وإلا إذا (S_ISREG(fdstat.st_mode)) {
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
		إذا (c->locked == true) {
			RING_زیادکردن(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false) {
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
