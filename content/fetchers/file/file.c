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

#include "utils/config.h"

#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#include <libwapcaplet/libwapcaplet.h>

#include "netsurf/inttypes.h"
#include "utils/nsurl.h"
#include "utils/dirent.h"
#include "utils/corestrings.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/log.h"
#include "utils/time.h"
#include "utils/ring.h"
#include "utils/file.h"
#include "netsurf/fetch.h"
#include "desktop/gui_internal.h"

#include "content/fetch.h"
#include "content/fetchers.h"
#include "dirlist.h"
#include "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
sa_nracanaa fetch_file_context {
	sa_nracanaa fetch_file_context *r_next, *r_prev;

	sa_nracanaa fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	var_na *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

static sa_nracanaa fetch_file_context *ring = NULL;

/** issue fetch callbacks with locking */
static anta_hstha bool fetch_file_send_callback(const fetch_msg *msg,
		sa_nracanaa fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(msg, ctx->fetchh);
	ctx->locked = false;

	ສົ່ງຄືນ ctx->aborted;
}

static bool fetch_file_send_header(sa_nracanaa fetch_file_context *ctx,
		const var_na *fmt, ...)
{
	fetch_msg msg;
	var_na header[64];
	va_list ap;
	ຈຳນວນເຕັມ len;

	va_start(ap, fmt);
	len = vsnprintf(header, parimaa_na header, fmt, ap);
	va_end(ap);

	ຖ້າ (len >= (ຈຳນວນເຕັມ)parimaa_na(header) || len < 0) {
		ສົ່ງຄືນ false;
	}

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.buf = (const uint8_t *) header;
	msg.data.header_or_data.len = len;

	ສົ່ງຄືນ fetch_file_send_callback(&msg, ctx);
}

/** callback to initialise the file fetcher. */
static bool fetch_file_initialise(lwc_string *scheme)
{
	ສົ່ງຄືນ true;
}

/** callback to initialise the file fetcher. */
static void fetch_file_finalise(lwc_string *scheme)
{
}

static bool fetch_file_can_fetch(const nsurl *url)
{
	ສົ່ງຄືນ true;
}

/** callback to set up a file fetch context. */
static void *
fetch_file_setup(sa_nracanaa fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 const var_na *post_urlenc,
		 const sa_nracanaa fetch_multipart_data *post_multipart,
		 const var_na **headers)
{
	sa_nracanaa fetch_file_context *ctx;
	ຈຳນວນເຕັມ i;
	nserror ret;

	ctx = calloc(1, parimaa_na(*ctx));
	ຖ້າ (ctx == NULL)
		ສົ່ງຄືນ NULL;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	ຖ້າ (ret != NSERROR_OK) {
		free(ctx);
		ສົ່ງຄືນ NULL;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ສຳລັບ (i = 0; headers[i] != NULL; i++) {
		ຖ້າ (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			continue;
		}

		/* If-None-Match: "12345678" */
		const var_na *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ໃນຂະນະທີ່ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ຖ້າ (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			ຖ້າ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_INSERT(ring, ctx);

	ສົ່ງຄືນ ctx;
}

/** callback to free a file fetch */
static void fetch_file_free(void *ctx)
{
	sa_nracanaa fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	free(c->path);
	free(ctx);
}

/** callback to start a file fetch */
static bool fetch_file_start(void *ctx)
{
	ສົ່ງຄືນ true;
}

/** callback to abort a file fetch */
static void fetch_file_abort(void *ctx)
{
	sa_nracanaa fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

static ຈຳນວນເຕັມ fetch_file_errno_to_http_code(ຈຳນວນເຕັມ error_no)
{
	switch (error_no) {
	pak_sa ENAMETOOLONG:
		ສົ່ງຄືນ 400;
	pak_sa EACCES:
		ສົ່ງຄືນ 403;
	pak_sa ENOENT:
		ສົ່ງຄືນ 404;
	default:
		bha_nga;
	}

	ສົ່ງຄືນ 500;
}

static void fetch_file_process_error(sa_nracanaa fetch_file_context *ctx, ຈຳນວນເຕັມ code)
{
	fetch_msg msg;
	var_na buffer[1024];
	const var_na *title;
	var_na key[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		goto fetch_file_process_error_aborted;

	snprintf(key, parimaa_na key, "HTTP%03d", code);
	title = messages_get(key);

	snprintf(buffer, parimaa_na buffer,
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
	msg.data.header_or_data.buf = (const uint8_t *) buffer;
	msg.data.header_or_data.len = strlen(buffer);
	ຖ້າ (fetch_file_send_callback(&msg, ctx))
		goto fetch_file_process_error_aborted;

	msg.type = FETCH_FINISHED;
	fetch_file_send_callback(&msg, ctx);

fetch_file_process_error_aborted:
	ສົ່ງຄືນ;
}


/** Process object as a regular file */
static void fetch_file_process_plain(sa_nracanaa fetch_file_context *ctx,
				     sa_nracanaa stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_msg msg;
	var_na *buf = NULL;
	size_t buf_size;

	ຈຳນວນເຕັມ fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ຖ້າ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		msg.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&msg, ctx);
		ສົ່ງຄືນ;
	}

	fd = open(ctx->path, O_RDONLY);
	ຖ້າ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ສົ່ງຄືນ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	ຖ້າ (buf_size > 0) {
		buf = mmap(NULL, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		ຖ້າ (buf == MAP_FAILED) {
			msg.type = FETCH_ERROR;
			msg.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&msg, ctx);
			close(fd);
			ສົ່ງຄືນ;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		goto fetch_file_process_aborted;
	}

	/* content length */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		goto fetch_file_process_aborted;
	}

	/* create etag */
	ຖ້າ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		goto fetch_file_process_aborted;
	}

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *) buf;
	msg.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&msg, ctx);

	ຖ້າ (ctx->aborted == false) {
		msg.type = FETCH_FINISHED;
		fetch_file_send_callback(&msg, ctx);
	}

fetch_file_process_aborted:

	ຖ້າ (buf != NULL)
		munmap(buf, buf_size);
	close(fd);
#ບໍ່ດັ່ງນັ້ນ
	fetch_msg msg;
	var_na *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ຖ້າ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		msg.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&msg, ctx);
		ສົ່ງຄືນ;
	}

	infile = fopen(ctx->path, "rb");
	ຖ້າ (infile == NULL) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ສົ່ງຄືນ;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	ຖ້າ (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	ຖ້າ (buf == NULL) {
		msg.type = FETCH_ERROR;
		msg.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&msg, ctx);
		fclose(infile);
		ສົ່ງຄືນ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		goto fetch_file_process_aborted;
	}

	/* content length */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		goto fetch_file_process_aborted;
	}

	/* create etag */
	ຖ້າ (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		goto fetch_file_process_aborted;
	}

	/* main data loop */
	ໃນຂະນະທີ່ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		ຖ້າ (res == 0) {
			ຖ້າ (feof(infile)) {
				msg.type = FETCH_ERROR;
				msg.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&msg, ctx);
				goto fetch_file_process_aborted;
			} ບໍ່ດັ່ງນັ້ນ {
				msg.type = FETCH_ERROR;
				msg.data.error = "Error reading file";
				fetch_file_send_callback(&msg, ctx);
				goto fetch_file_process_aborted;
			}
		}
		tot_read += res;

		msg.type = FETCH_DATA;
		msg.data.header_or_data.buf = (const uint8_t *) buf;
		msg.data.header_or_data.len = res;
		ຖ້າ (fetch_file_send_callback(&msg, ctx))
			bha_nga;
	}

	ຖ້າ (ctx->aborted == false) {
		msg.type = FETCH_FINISHED;
		fetch_file_send_callback(&msg, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	free(buf);
#endif
	ສົ່ງຄືນ;
}

static var_na *gen_nice_title(var_na *path)
{
	var_na *nice_path, *cnv, *tmp;
	var_na *title;
	ຈຳນວນເຕັມ title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	ຖ້າ (nice_path == NULL) {
		ສົ່ງຄືນ NULL;
	}

	/* Escape special HTML characters */
	ສຳລັບ (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		ຖ້າ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ບໍ່ດັ່ງນັ້ນ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	ຖ້າ (title == NULL) {
		free(nice_path);
		ສົ່ງຄືນ NULL;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	free(nice_path);

	ສົ່ງຄືນ title;
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
static nserror
process_dir_ent(sa_nracanaa fetch_file_context *ctx,
		 sa_nracanaa dirent *ent,
		 bool even,
		 var_na *buffer,
		 size_t buffer_len)
{
	nserror ret;
	var_na *urlpath = NULL; /* buffer for leaf entry path */
	sa_nracanaa stat ent_stat; /* stat result of leaf entry */
	var_na datebuf[64]; /* buffer for date text */
	var_na timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ຖ້າ (ent->d_name[0] == '.') {
		ສົ່ງຄືນ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, NULL, 2, ctx->path, ent->d_name);
	ຖ້າ (ret != NSERROR_OK) {
		ສົ່ງຄືນ ret;
	}

	ຖ້າ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ບໍ່ດັ່ງນັ້ນ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ຖ້າ (strftime((var_na *)&datebuf, parimaa_na datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ຖ້າ (strftime((var_na *)&timebuf, parimaa_na timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	ຖ້າ (ret != NSERROR_OK) {
		free(urlpath);
		ສົ່ງຄືນ ret;
	}

	ຖ້າ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} ບໍ່ດັ່ງນັ້ນ {
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
	free(urlpath);

	ສົ່ງຄືນ NSERROR_OK;
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
static ຈຳນວນເຕັມ dir_sort_alpha(const sa_nracanaa dirent **d1, const sa_nracanaa dirent **d2)
{
	const var_na *s1 = (*d1)->d_name;
	const var_na *s2 = (*d2)->d_name;

	ໃນຂະນະທີ່ (*s1 != '\0' && *s2 != '\0') {
		ຖ້າ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			ຈຳນວນເຕັມ n1 = 0,  n2 = 0;
			ໃນຂະນະທີ່ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ໃນຂະນະທີ່ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ຖ້າ (n1 != n2) {
				ສົ່ງຄືນ n1 - n2;
			}
			ຖ້າ (*s1 == '\0' || *s2 == '\0')
				bha_nga;
		}
		ຖ້າ (tolower(*s1) != tolower(*s2))
			bha_nga;

		s1++;
		s2++;
	}

	ສົ່ງຄືນ tolower(*s1) - tolower(*s2);
}

static void fetch_file_process_dir(sa_nracanaa fetch_file_context *ctx,
				   sa_nracanaa stat *fdstat)
{
	fetch_msg msg;
	var_na buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	var_na *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	sa_nracanaa dirent **listing = NULL; /* directory entry listing */
	ຈຳນວນເຕັມ i; /* directory entry index */
	ຈຳນວນເຕັມ n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	ຖ້າ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		ສົ່ງຄືນ;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	ຖ້າ (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		goto fetch_file_process_dir_aborted;

	/* content type */
	ຖ້າ (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		goto fetch_file_process_dir_aborted;

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_top(buffer, parimaa_na buffer);
	msg.data.header_or_data.len = strlen(buffer);
	ຖ້າ (fetch_file_send_callback(&msg, ctx))
		goto fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, parimaa_na buffer);
	free(title);
	msg.data.header_or_data.len = strlen(buffer);
	ຖ້າ (fetch_file_send_callback(&msg, ctx))
		goto fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	ຖ້າ (err == NSERROR_OK) {
		ຖ້າ (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, parimaa_na buffer);

			msg.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&msg, ctx);
		}
		nsurl_unref(up);

		ຖ້າ (ctx->aborted)
			goto fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, parimaa_na buffer);
	msg.data.header_or_data.len = strlen(buffer);
	ຖ້າ (fetch_file_send_callback(&msg, ctx))
		goto fetch_file_process_dir_aborted;

	ສຳລັບ (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       parimaa_na(buffer));

		ຖ້າ (err == NSERROR_OK) {
			msg.data.header_or_data.len = strlen(buffer);
			ຖ້າ (fetch_file_send_callback(&msg, ctx))
				goto fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, parimaa_na buffer);
	msg.data.header_or_data.len = strlen(buffer);
	ຖ້າ (fetch_file_send_callback(&msg, ctx))
		goto fetch_file_process_dir_aborted;

	msg.type = FETCH_FINISHED;
	fetch_file_send_callback(&msg, ctx);

fetch_file_process_dir_aborted:

	ຖ້າ (listing != NULL) {
		ສຳລັບ (i = 0; i < n; i++) {
			free(listing[i]);
		}
		free(listing);
	}
}


/* process a file fetch */
static void fetch_file_process(sa_nracanaa fetch_file_context *ctx)
{
	sa_nracanaa stat fdstat; /**< The objects stat */

	ຖ້າ (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ສົ່ງຄືນ;
	}

	ຖ້າ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ສົ່ງຄືນ;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ສົ່ງຄືນ;
	} ບໍ່ດັ່ງນັ້ນ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ສົ່ງຄືນ;
}

/** callback to poll for additional file fetch contents */
static void fetch_file_poll(lwc_string *scheme)
{
	sa_nracanaa fetch_file_context *c, *save_ring = NULL;

	ໃນຂະນະທີ່ (ring != NULL) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ຖ້າ (c->locked == true) {
			RING_INSERT(save_ring, c);
			continue;
		}

		/* Only process non-aborted fetches */
		ຖ້າ (c->aborted == false) {
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

nserror fetch_file_register(void)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	const sa_nracanaa fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.free = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ສົ່ງຄືນ fetcher_add(scheme, &fetcher_ops);
}
