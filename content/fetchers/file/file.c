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

#অন্তর্ভাৱ "utils/config.h"

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdarg.h>
#ifdef HAVE_MMAP
#অন্তর্ভাৱ <sys/mman.h>
#endif
#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/dirent.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/time.h"
#অন্তর্ভাৱ "utils/ring.h"
#অন্তর্ভাৱ "utils/file.h"
#অন্তর্ভাৱ "netsurf/fetch.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"
#অন্তর্ভাৱ "dirlist.h"
#অন্তর্ভাৱ "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_SIZE (1024 * 1024)

/** Context for a fetch */
সংরচনা fetch_file_context {
	সংরচনা fetch_file_context *r_next, *r_prev;

	সংরচনা fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	ৱর্ণ *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

স্থির সংরচনা fetch_file_context *ring = রিক্ত;

/** issue fetch callbacks with locking */
স্থির অন্তঃস্থ bool fetch_file_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা,
		সংরচনা fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বাৰ্তা, ctx->fetchh);
	ctx->locked = false;

	ঘূৰাই ctx->aborted;
}

স্থির bool fetch_file_send_header(সংরচনা fetch_file_context *ctx,
		ধ্রুৱ ৱর্ণ *fmt, ...)
{
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ header[64];
	va_list ap;
	পূর্ণাঙ্ক len;

	va_start(ap, fmt);
	len = vsnprintf(header, পরিমাণ header, fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণাঙ্ক)পরিমাণ(header) || len < 0) {
		ঘূৰাই false;
	}

	বাৰ্তা.type = FETCH_HEADER;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) header;
	বাৰ্তা.data.header_or_data.len = len;

	ঘূৰাই fetch_file_send_callback(&বাৰ্তা, ctx);
}

/** callback to initialise the file fetcher. */
স্থির bool fetch_file_initialise(lwc_string *scheme)
{
	ঘূৰাই true;
}

/** callback to initialise the file fetcher. */
স্থির শূন্য fetch_file_finalise(lwc_string *scheme)
{
}

স্থির bool fetch_file_can_fetch(ধ্রুৱ nsurl *url)
{
	ঘূৰাই true;
}

/** callback to set up a file fetch context. */
স্থির শূন্য *
fetch_file_setup(সংরচনা fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ধ্রুৱ ৱর্ণ *post_urlenc,
		 ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		 ধ্রুৱ ৱর্ণ **headers)
{
	সংরচনা fetch_file_context *ctx;
	পূর্ণাঙ্ক i;
	nserror ret;

	ctx = calloc(1, পরিমাণ(*ctx));
	যদি (ctx == রিক্ত)
		ঘূৰাই রিক্ত;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	যদি (ret != NSERROR_OK) {
		মুক্ত(ctx);
		ঘূৰাই রিক্ত;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	বাবে (i = 0; headers[i] != রিক্ত; i++) {
		যদি (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			অনুৱর্ত;
		}

		/* If-None-Match: "12345678" */
		ধ্রুৱ ৱর্ণ *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		যেতিয়ালৈ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		যদি (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->file_etag);
			যদি (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ঢোকাও(ring, ctx);

	ঘূৰাই ctx;
}

/** callback to free a file fetch */
স্থির শূন্য fetch_file_free(শূন্য *ctx)
{
	সংরচনা fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	মুক্ত(c->path);
	মুক্ত(ctx);
}

/** callback to start a file fetch */
স্থির bool fetch_file_start(শূন্য *ctx)
{
	ঘূৰাই true;
}

/** callback to abort a file fetch */
স্থির শূন্য fetch_file_abort(শূন্য *ctx)
{
	সংরচনা fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

স্থির পূর্ণাঙ্ক fetch_file_errno_to_http_code(পূর্ণাঙ্ক error_no)
{
	ৱিকল্প (error_no) {
	পক্ষ ENAMETOOLONG:
		ঘূৰাই 400;
	পক্ষ EACCES:
		ঘূৰাই 403;
	পক্ষ ENOENT:
		ঘূৰাই 404;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	ঘূৰাই 500;
}

স্থির শূন্য fetch_file_process_error(সংরচনা fetch_file_context *ctx, পূর্ণাঙ্ক code)
{
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ buffer[1024];
	ধ্রুৱ ৱর্ণ *title;
	ৱর্ণ চাবি[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		গচ্ছ fetch_file_process_error_aborted;

	snprintf(চাবি, পরিমাণ চাবি, "HTTP%03d", code);
	title = messages_get(চাবি);

	snprintf(buffer, পরিমাণ buffer,
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

	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) buffer;
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_file_process_error_aborted;

	বাৰ্তা.type = FETCH_FINISHED;
	fetch_file_send_callback(&বাৰ্তা, ctx);

fetch_file_process_error_aborted:
	ঘূৰাই;
}


/** Process object as a regular file */
স্থির শূন্য fetch_file_process_plain(সংরচনা fetch_file_context *ctx,
				     সংরচনা stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ *buf = রিক্ত;
	size_t buf_size;

	পূর্ণাঙ্ক fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	যদি (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		বাৰ্তা.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&বাৰ্তা, ctx);
		ঘূৰাই;
	}

	fd = open(ctx->path, O_RDONLY);
	যদি (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ঘূৰাই;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;

	/* allocate the buffer storage */
	যদি (buf_size > 0) {
		buf = mmap(রিক্ত, buf_size, PROT_READ, MAP_SHARED, fd, 0);
		যদি (buf == MAP_FAILED) {
			বাৰ্তা.type = FETCH_ERROR;
			বাৰ্তা.data.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&বাৰ্তা, ctx);
			close(fd);
			ঘূৰাই;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		গচ্ছ fetch_file_process_aborted;
	}

	/* content length */
	যদি (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		গচ্ছ fetch_file_process_aborted;
	}

	/* create etag */
	যদি (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		গচ্ছ fetch_file_process_aborted;
	}

	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) buf;
	বাৰ্তা.data.header_or_data.len = buf_size;
	fetch_file_send_callback(&বাৰ্তা, ctx);

	যদি (ctx->aborted == false) {
		বাৰ্তা.type = FETCH_FINISHED;
		fetch_file_send_callback(&বাৰ্তা, ctx);
	}

fetch_file_process_aborted:

	যদি (buf != রিক্ত)
		munmap(buf, buf_size);
	close(fd);
#অন্যথা
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ *buf;
	size_t buf_size;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	যদি (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		বাৰ্তা.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&বাৰ্তা, ctx);
		ঘূৰাই;
	}

	infile = fopen(ctx->path, "rb");
	যদি (infile == রিক্ত) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ঘূৰাই;
	}

	/* set buffer size */
	buf_size = fdstat->st_size;
	যদি (buf_size > FETCH_FILE_MAX_BUF_SIZE)
		buf_size = FETCH_FILE_MAX_BUF_SIZE;

	/* allocate the buffer storage */
	buf = malloc(buf_size);
	যদি (buf == রিক্ত) {
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&বাৰ্তা, ctx);
		fclose(infile);
		ঘূৰাই;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		গচ্ছ fetch_file_process_aborted;
	}

	/* content length */
	যদি (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		গচ্ছ fetch_file_process_aborted;
	}

	/* create etag */
	যদি (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		গচ্ছ fetch_file_process_aborted;
	}

	/* main data loop */
	যেতিয়ালৈ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_size, infile);
		যদি (res == 0) {
			যদি (feof(infile)) {
				বাৰ্তা.type = FETCH_ERROR;
				বাৰ্তা.data.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&বাৰ্তা, ctx);
				গচ্ছ fetch_file_process_aborted;
			} অন্যথা {
				বাৰ্তা.type = FETCH_ERROR;
				বাৰ্তা.data.error = "Error reading file";
				fetch_file_send_callback(&বাৰ্তা, ctx);
				গচ্ছ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		বাৰ্তা.type = FETCH_DATA;
		বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) buf;
		বাৰ্তা.data.header_or_data.len = res;
		যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
			ভঙ্গ;
	}

	যদি (ctx->aborted == false) {
		বাৰ্তা.type = FETCH_FINISHED;
		fetch_file_send_callback(&বাৰ্তা, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	মুক্ত(buf);
#endif
	ঘূৰাই;
}

স্থির ৱর্ণ *gen_nice_title(ৱর্ণ *path)
{
	ৱর্ণ *nice_path, *cnv, *tmp;
	ৱর্ণ *title;
	পূর্ণাঙ্ক title_length;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	যদি (nice_path == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	/* Escape special HTML characters */
	বাবে (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		যদি (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} অন্যথা যদি (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} অন্যথা যদি (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} অন্যথা {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_length = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_length + 1);

	যদি (title == রিক্ত) {
		মুক্ত(nice_path);
		ঘূৰাই রিক্ত;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_length, messages_get("FileIndex"), nice_path);

	মুক্ত(nice_path);

	ঘূৰাই title;
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
স্থির nserror
process_dir_ent(সংরচনা fetch_file_context *ctx,
		 সংরচনা dirent *ent,
		 bool even,
		 ৱর্ণ *buffer,
		 size_t buffer_len)
{
	nserror ret;
	ৱর্ণ *urlpath = রিক্ত; /* buffer for leaf entry path */
	সংরচনা stat ent_stat; /* stat result of leaf entry */
	ৱর্ণ datebuf[64]; /* buffer for date text */
	ৱর্ণ timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	যদি (ent->d_name[0] == '.') {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, রিক্ত, 2, ctx->path, ent->d_name);
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}

	যদি (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} অন্যথা {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		যদি (strftime((ৱর্ণ *)&datebuf, পরিমাণ datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		যদি (strftime((ৱর্ণ *)&timebuf, পরিমাণ timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	যদি (ret != NSERROR_OK) {
		মুক্ত(urlpath);
		ঘূৰাই ret;
	}

	যদি (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} অন্যথা যদি (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     buffer, buffer_len);
	} অন্যথা {
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
	মুক্ত(urlpath);

	ঘূৰাই NSERROR_OK;
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
স্থির পূর্ণাঙ্ক dir_sort_alpha(ধ্রুৱ সংরচনা dirent **d1, ধ্রুৱ সংরচনা dirent **d2)
{
	ধ্রুৱ ৱর্ণ *s1 = (*d1)->d_name;
	ধ্রুৱ ৱর্ণ *s2 = (*d2)->d_name;

	যেতিয়ালৈ (*s1 != '\0' && *s2 != '\0') {
		যদি ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			পূর্ণাঙ্ক n1 = 0,  n2 = 0;
			যেতিয়ালৈ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			যেতিয়ালৈ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			যদি (n1 != n2) {
				ঘূৰাই n1 - n2;
			}
			যদি (*s1 == '\0' || *s2 == '\0')
				ভঙ্গ;
		}
		যদি (tolower(*s1) != tolower(*s2))
			ভঙ্গ;

		s1++;
		s2++;
	}

	ঘূৰাই tolower(*s1) - tolower(*s2);
}

স্থির শূন্য fetch_file_process_dir(সংরচনা fetch_file_context *ctx,
				   সংরচনা stat *fdstat)
{
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ buffer[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	ৱর্ণ *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	সংরচনা dirent **listing = রিক্ত; /* directory entry listing */
	পূর্ণাঙ্ক i; /* directory entry index */
	পূর্ণাঙ্ক n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	যদি (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		ঘূৰাই;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	যদি (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		গচ্ছ fetch_file_process_dir_aborted;

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		গচ্ছ fetch_file_process_dir_aborted;

	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) buffer;

	/* directory listing top */
	dirlist_generate_শীৰ্ষ(buffer, পরিমাণ buffer);
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, buffer, পরিমাণ buffer);
	মুক্ত(title);
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	যদি (err == NSERROR_OK) {
		যদি (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					buffer, পরিমাণ buffer);

			বাৰ্তা.data.header_or_data.len = strlen(buffer);
			fetch_file_send_callback(&বাৰ্তা, ctx);
		}
		nsurl_unref(up);

		যদি (ctx->aborted)
			গচ্ছ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(buffer, পরিমাণ buffer);
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_file_process_dir_aborted;

	বাবে (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, buffer,
				       পরিমাণ(buffer));

		যদি (err == NSERROR_OK) {
			বাৰ্তা.data.header_or_data.len = strlen(buffer);
			যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
				গচ্ছ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(buffer, পরিমাণ buffer);
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_file_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_file_process_dir_aborted;

	বাৰ্তা.type = FETCH_FINISHED;
	fetch_file_send_callback(&বাৰ্তা, ctx);

fetch_file_process_dir_aborted:

	যদি (listing != রিক্ত) {
		বাবে (i = 0; i < n; i++) {
			মুক্ত(listing[i]);
		}
		মুক্ত(listing);
	}
}


/* process a file fetch */
স্থির শূন্য fetch_file_process(সংরচনা fetch_file_context *ctx)
{
	সংরচনা stat fdstat; /**< The objects stat */

	যদি (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ঘূৰাই;
	}

	যদি (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ঘূৰাই;
	} অন্যথা যদি (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ঘূৰাই;
	} অন্যথা {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ঘূৰাই;
}

/** callback to poll for additional file fetch contents */
স্থির শূন্য fetch_file_poll(lwc_string *scheme)
{
	সংরচনা fetch_file_context *c, *save_ring = রিক্ত;

	যেতিয়ালৈ (ring != রিক্ত) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		যদি (c->locked == true) {
			RING_ঢোকাও(save_ring, c);
			অনুৱর্ত;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false) {
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

nserror fetch_file_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ধ্রুৱ সংরচনা fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.মুক্ত = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ঘূৰাই fetcher_add(scheme, &fetcher_ops);
}
