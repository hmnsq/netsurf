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

#شامل "utils/config.h"

#شامل <stdlib.h>
#شامل <ctype.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <fcntl.h>
#شامل <unistd.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#شامل <stdio.h>
#شامل <stdarg.h>
#ifdef HAVE_MMAP
#شامل <sys/mman.h>
#endif
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/nsurl.h"
#شامل "utils/dirent.h"
#شامل "utils/corestrings.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/time.h"
#شامل "utils/ring.h"
#شامل "utils/file.h"
#شامل "netsurf/fetch.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "dirlist.h"
#شامل "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
ساختار fetch_file_context {
	ساختار fetch_file_context *r_next, *r_prev;

	ساختار fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	نویسه *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ایستا ساختار fetch_file_context *ring = پوچ;

/** issue fetch callbacks with locking */
ایستا درونخطی bool fetch_file_send_callback(ثابت fetch_پیام *پیام,
		ساختار fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	بازگشت ctx->aborted;
}

ایستا bool fetch_file_send_header(ساختار fetch_file_context *ctx,
		ثابت نویسه *fmt, ...)
{
	fetch_پیام پیام;
	نویسه header[64];
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(header, اندازه header, fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)اندازه(header) || len < 0) {
		بازگشت false;
	}

	پیام.type = FETCH_HEADER;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) header;
	پیام.data.header_or_data.len = len;

	بازگشت fetch_file_send_callback(&پیام, ctx);
}

/** callback to initialise the file fetcher. */
ایستا bool fetch_file_initialise(lwc_string *scheme)
{
	بازگشت true;
}

/** callback to initialise the file fetcher. */
ایستا تهی fetch_file_finalise(lwc_string *scheme)
{
}

ایستا bool fetch_file_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}

/** callback to set up a file fetch context. */
ایستا تهی *
fetch_file_setup(ساختار fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت نویسه *post_urlenc,
		 ثابت ساختار fetch_multipart_data *post_multipart,
		 ثابت نویسه **headers)
{
	ساختار fetch_file_context *ctx;
	صحیح i;
	nserror ret;

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ)
		بازگشت پوچ;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	اگر (ret != NSERROR_OK) {
		آزاد(ctx);
		بازگشت پوچ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	برای (i = 0; headers[i] != پوچ; i++) {
		اگر (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ادامه;
		}

		/* If-None-Match: "12345678" */
		ثابت نویسه *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		وقتی (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		اگر (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			اگر (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_درج(ring, ctx);

	بازگشت ctx;
}

/** callback to free a file fetch */
ایستا تهی fetch_file_free(تهی *ctx)
{
	ساختار fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	آزاد(c->path);
	آزاد(ctx);
}

/** callback to start a file fetch */
ایستا bool fetch_file_start(تهی *ctx)
{
	بازگشت true;
}

/** callback to abort a file fetch */
ایستا تهی fetch_file_abort(تهی *ctx)
{
	ساختار fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ایستا صحیح fetch_file_errno_to_http_code(صحیح error_no)
{
	انتخاب (error_no) {
	حالت ENAMETOOLONG:
		بازگشت 400;
	حالت EACCES:
		بازگشت 403;
	حالت ENOENT:
		بازگشت 404;
	پیشفرض:
		بشکن;
	}

	بازگشت 500;
}

ایستا تهی fetch_file_process_error(ساختار fetch_file_context *ctx, صحیح code)
{
	fetch_پیام پیام;
	نویسه buffer[1024];
	ثابت نویسه *title;
	نویسه کلید[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	اگر (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		برو fetch_file_process_error_aborted;

	snprintf(کلید, اندازه کلید, "HTTP%03d", code);
	title = messages_get(کلید);

	snprintf(buffer, اندازه buffer,
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

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_file_send_callback(&پیام, ctx))
		برو fetch_file_process_error_aborted;

	پیام.type = FETCH_FINISHED;
	fetch_file_send_callback(&پیام, ctx);

fetch_file_process_error_aborted:
	بازگشت;
}


/** Process object as a regular file */
ایستا تهی fetch_file_process_plain(ساختار fetch_file_context *ctx,
				     ساختار stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_پیام پیام;
	نویسه *buf = پوچ;
	size_t buf_size;

	صحیح fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	اگر (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیام, ctx);
		بازگشت;
	}

	fd = open(ctx->path, O_RDONLY);
	اگر (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		بازگشت;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	اگر (buf_size > 0) {
		buf = mmap(پوچ, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		اگر (buf == MAP_FAILED) {
			پیام.type = FETCH_ERROR;
			پیام.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&پیام, ctx);
			close(fd);
			بازگشت;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		برو fetch_file_process_aborted;
	}

	/* content length */
	اگر (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		برو fetch_file_process_aborted;
	}

	/* create etag */
	اگر (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		برو fetch_file_process_aborted;
	}

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) buf;
	پیام.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&پیام, ctx);

	اگر (ctx->aborted == false) {
		پیام.type = FETCH_FINISHED;
		fetch_file_send_callback(&پیام, ctx);
	}

fetch_file_process_aborted:

	اگر (buf != پوچ)
		munmap(buf, buf_size);
	close(fd);
#وگرنه
	fetch_پیام پیام;
	نویسه *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	اگر (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیام, ctx);
		بازگشت;
	}

	infile = fopen(ctx->path, "rb");
	اگر (infile == پوچ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		بازگشت;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	اگر (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	اگر (buf == پوچ) {
		پیام.type = FETCH_ERROR;
		پیام.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&پیام, ctx);
		fclose(infile);
		بازگشت;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		برو fetch_file_process_aborted;
	}

	/* content length */
	اگر (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		برو fetch_file_process_aborted;
	}

	/* create etag */
	اگر (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		برو fetch_file_process_aborted;
	}

	/* main data loop */
	وقتی (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		اگر (res == 0) {
			اگر (feof(infile)) {
				پیام.type = FETCH_ERROR;
				پیام.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&پیام, ctx);
				برو fetch_file_process_aborted;
			} وگرنه {
				پیام.type = FETCH_ERROR;
				پیام.data.error = "Error reading file";
				fetch_file_send_callback(&پیام, ctx);
				برو fetch_file_process_aborted;
			}
		}
		tot_read += res;

		پیام.type = FETCH_DATA;
		پیام.data.header_or_data.buf = (ثابت uint8_t *) buf;
		پیام.data.header_or_data.len = res;
		اگر (fetch_file_send_callback(&پیام, ctx))
			بشکن;
	}

	اگر (ctx->aborted == false) {
		پیام.type = FETCH_FINISHED;
		fetch_file_send_callback(&پیام, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	آزاد(buf);
#endif
	بازگشت;
}

ایستا نویسه *gen_nice_title(نویسه *path)
{
	نویسه *nice_path, *cnv, *tmp;
	نویسه *title;
	صحیح title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	اگر (nice_path == پوچ) {
		بازگشت پوچ;
	}

	/* Escape special HTML characters */
	برای (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		اگر (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} وگرنه اگر (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} وگرنه اگر (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} وگرنه {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	اگر (title == پوچ) {
		آزاد(nice_path);
		بازگشت پوچ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	آزاد(nice_path);

	بازگشت title;
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
ایستا nserror
process_dir_ent(ساختار fetch_file_context *ctx,
		 ساختار dirent *ent,
		 bool even,
		 نویسه *buffer,
		 size_t buffer_len)
{
	nserror ret;
	نویسه *urlpath = پوچ; /* buffer for leaf entry path */
	ساختار stat ent_stat; /* stat result of leaf entry */
	نویسه datebuf[64]; /* buffer for date text */
	نویسه timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	اگر (ent->d_name[0] == '.') {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, پوچ, 2, ctx->path, ent->d_name);
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}

	اگر (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} وگرنه {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		اگر (strftime((نویسه *)&datebuf, اندازه datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		اگر (strftime((نویسه *)&timebuf, اندازه timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	اگر (ret != NSERROR_OK) {
		آزاد(urlpath);
		بازگشت ret;
	}

	اگر (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} وگرنه اگر (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} وگرنه {
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
	آزاد(urlpath);

	بازگشت NSERROR_OK;
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
ایستا صحیح dir_sort_alpha(ثابت ساختار dirent **d1, ثابت ساختار dirent **d2)
{
	ثابت نویسه *s1 = (*d1)->d_name;
	ثابت نویسه *s2 = (*d2)->d_name;

	وقتی (*s1 != '\0' && *s2 != '\0') {
		اگر ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			صحیح n1 = 0,  n2 = 0;
			وقتی (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			وقتی (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			اگر (n1 != n2) {
				بازگشت n1 - n2;
			}
			اگر (*s1 == '\0' || *s2 == '\0')
				بشکن;
		}
		اگر (tolower(*s1) != tolower(*s2))
			بشکن;

		s1++;
		s2++;
	}

	بازگشت tolower(*s1) - tolower(*s2);
}

ایستا تهی fetch_file_process_dir(ساختار fetch_file_context *ctx,
				   ساختار stat *fdstat)
{
	fetch_پیام پیام;
	نویسه buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	نویسه *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	ساختار dirent **listing = پوچ; /* directory entry listing */
	صحیح i; /* directory entry index */
	صحیح n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	اگر (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		بازگشت;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	اگر (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		برو fetch_file_process_dir_aborted;

	/* content type */
	اگر (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		برو fetch_file_process_dir_aborted;

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_بالاترین(buffer, اندازه buffer);
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_file_send_callback(&پیام, ctx))
		برو fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, اندازه buffer);
	آزاد(title);
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_file_send_callback(&پیام, ctx))
		برو fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	اگر (err == NSERROR_OK) {
		اگر (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, اندازه buffer);

			پیام.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&پیام, ctx);
		}
		nsurl_unref(up);

		اگر (ctx->aborted)
			برو fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, اندازه buffer);
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_file_send_callback(&پیام, ctx))
		برو fetch_file_process_dir_aborted;

	برای (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       اندازه(buffer));

		اگر (err == NSERROR_OK) {
			پیام.data.header_or_data.len = strlen(buffer);
			اگر (fetch_file_send_callback(&پیام, ctx))
				برو fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, اندازه buffer);
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_file_send_callback(&پیام, ctx))
		برو fetch_file_process_dir_aborted;

	پیام.type = FETCH_FINISHED;
	fetch_file_send_callback(&پیام, ctx);

fetch_file_process_dir_aborted:

	اگر (listing != پوچ) {
		برای (i = 0; i < n; i++) {
			آزاد(listing[i]);
		}
		آزاد(listing);
	}
}


/* process a file fetch */
ایستا تهی fetch_file_process(ساختار fetch_file_context *ctx)
{
	ساختار stat fdstat; /**< The objects stat */

	اگر (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		بازگشت;
	}

	اگر (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		بازگشت;
	} وگرنه اگر (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		بازگشت;
	} وگرنه {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	بازگشت;
}

/** callback to poll for additional file fetch contents */
ایستا تهی fetch_file_poll(lwc_string *scheme)
{
	ساختار fetch_file_context *c, *save_ring = پوچ;

	وقتی (ring != پوچ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		اگر (c->locked == true) {
			RING_درج(save_ring, c);
			ادامه;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false) {
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

nserror fetch_file_register(تهی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.آزاد = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	بازگشت fetcher_add(scheme, &fetcher_ops);
}
