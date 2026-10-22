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
#اگر_تعریف_شدہ HAVE_MMAP
#شامل <sys/mman.h>
#اگر_ختم
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
#تعریف_کرو FETCH_FILE_MAX_BUF_سائز (1024 * 1024)

/** Context for a fetch */
ساخت fetch_file_context {
	ساخت fetch_file_context *r_اگلا, *r_پچھلا;

	ساخت fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	حرف *راستہ; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ساکن ساخت fetch_file_context *ring = تہی;

/** issue fetch callbacks with locking */
ساکن اندرونی bool fetch_file_send_callback(ثابت fetch_پیغام *پیغام,
		ساخت fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
	ctx->locked = false;

	واپس ctx->aborted;
}

ساکن bool fetch_file_send_سرخی(ساخت fetch_file_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پیغام پیغام;
	حرف سرخی[64];
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(سرخی, حجم سرخی, fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)حجم(سرخی) || len < 0) {
		واپس false;
	}

	پیغام.type = FETCH_سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = len;

	واپس fetch_file_send_callback(&پیغام, ctx);
}

/** callback to initialise the file fetcher. */
ساکن bool fetch_file_initialise(lwc_string *scheme)
{
	واپس true;
}

/** callback to initialise the file fetcher. */
ساکن خالی fetch_file_finalise(lwc_string *scheme)
{
}

ساکن bool fetch_file_can_fetch(ثابت nsurl *url)
{
	واپس true;
}

/** callback to set up a file fetch context. */
ساکن خالی *
fetch_file_تیاری(ساخت fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		 ثابت حرف **سرخیاں)
{
	ساخت fetch_file_context *ctx;
	صحیح i;
	nserror ret;

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی)
		واپس تہی;

	ret = guit->file->nsurl_مقصد_راستہ(url, &ctx->راستہ);
	اگر (ret != NSERROR_OK) {
		آزاد(ctx);
		واپس تہی;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	برائے (i = 0; سرخیاں[i] != تہی; i++) {
		اگر (strncasecmp(سرخیاں[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			جاری;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = سرخیاں[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		جب (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		اگر (*d != '\0') {
			ret = nsc_snptimet(d, لڑی_لمبائی(d), &ctx->file_etag);
			اگر (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_داخل_کرو(ring, ctx);

	واپس ctx;
}

/** callback to free a file fetch */
ساکن خالی fetch_file_free(خالی *ctx)
{
	ساخت fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	آزاد(c->راستہ);
	آزاد(ctx);
}

/** callback to start a file fetch */
ساکن bool fetch_file_آغاز(خالی *ctx)
{
	واپس true;
}

/** callback to abort a file fetch */
ساکن خالی fetch_file_abort(خالی *ctx)
{
	ساخت fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساکن صحیح fetch_file_errno_مقصد_http_رمز(صحیح error_no)
{
	انتخاب (error_no) {
	حالت ENAMETOOLONG:
		واپس 400;
	حالت EACCES:
		واپس 403;
	حالت ENOENT:
		واپس 404;
	طےشدہ:
		توڑو;
	}

	واپس 500;
}

ساکن خالی fetch_file_process_error(ساخت fetch_file_context *ctx, صحیح رمز)
{
	fetch_پیغام پیغام;
	حرف بفر[1024];
	ثابت حرف *title;
	حرف کلید[8];

	/* content is going to return error code */
	fetch_سیٹ_http_رمز(ctx->fetchh, رمز);

	/* content type */
	اگر (fetch_file_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8"))
		جاؤ fetch_file_process_error_aborted;

	snprintf(کلید, حجم کلید, "HTTP%03d", رمز);
	title = messages_get(کلید);

	snprintf(بفر, حجم بفر,
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
		 messages_get("FetchErrorCode"), رمز,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) بفر;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_file_send_callback(&پیغام, ctx))
		جاؤ fetch_file_process_error_aborted;

	پیغام.type = FETCH_ختم_شدہ;
	fetch_file_send_callback(&پیغام, ctx);

fetch_file_process_error_aborted:
	واپس;
}


/** Process object as a regular file */
ساکن خالی fetch_file_process_plain(ساخت fetch_file_context *ctx,
				     ساخت stat *fdstat)
{
#اگر_تعریف_شدہ HAVE_MMAP
	fetch_پیغام پیغام;
	حرف *buf = تہی;
	size_t buf_سائز;

	صحیح fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	اگر (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_سیٹ_http_رمز(ctx->fetchh, 304);
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیغام, ctx);
		واپس;
	}

	fd = open(ctx->راستہ, O_RDONLY);
	اگر (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_رمز(errno));
		واپس;
	}

	/* set buffer size */
	buf_سائز = fdstat->st_size;

	/* allocate the buffer storage */
	اگر (buf_سائز > 0) {
		buf = mmap(تہی, buf_سائز, PROT_READ, MAP_SHARED, fd, 0);
		اگر (buf == MAP_FAILED) {
			پیغام.type = FETCH_ERROR;
			پیغام.ڈیٹا.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&پیغام, ctx);
			close(fd);
			واپس;
		}
	}

	/* fetch is going to be successful */
	fetch_سیٹ_http_رمز(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_file_send_سرخی(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->راستہ))) {
		جاؤ fetch_file_process_aborted;
	}

	/* content length */
	اگر (fetch_file_send_سرخی(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		جاؤ fetch_file_process_aborted;
	}

	/* create etag */
	اگر (fetch_file_send_سرخی(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		جاؤ fetch_file_process_aborted;
	}

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) buf;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = buf_سائز;
	fetch_file_send_callback(&پیغام, ctx);

	اگر (ctx->aborted == false) {
		پیغام.type = FETCH_ختم_شدہ;
		fetch_file_send_callback(&پیغام, ctx);
	}

fetch_file_process_aborted:

	اگر (buf != تہی)
		munmap(buf, buf_سائز);
	close(fd);
#ورنہ
	fetch_پیغام پیغام;
	حرف *buf;
	size_t buf_سائز;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	اگر (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_سیٹ_http_رمز(ctx->fetchh, 304);
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیغام, ctx);
		واپس;
	}

	infile = fopen(ctx->راستہ, "rb");
	اگر (infile == تہی) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_رمز(errno));
		واپس;
	}

	/* set buffer size */
	buf_سائز = fdstat->st_size;
	اگر (buf_سائز > FETCH_FILE_MAX_BUF_سائز)
		buf_سائز = FETCH_FILE_MAX_BUF_سائز;

	/* allocate the buffer storage */
	buf = حافظہ_دو(buf_سائز);
	اگر (buf == تہی) {
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&پیغام, ctx);
		fclose(infile);
		واپس;
	}

	/* fetch is going to be successful */
	fetch_سیٹ_http_رمز(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_file_send_سرخی(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->راستہ))) {
		جاؤ fetch_file_process_aborted;
	}

	/* content length */
	اگر (fetch_file_send_سرخی(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		جاؤ fetch_file_process_aborted;
	}

	/* create etag */
	اگر (fetch_file_send_سرخی(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		جاؤ fetch_file_process_aborted;
	}

	/* main data loop */
	جب (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_سائز, infile);
		اگر (res == 0) {
			اگر (feof(infile)) {
				پیغام.type = FETCH_ERROR;
				پیغام.ڈیٹا.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&پیغام, ctx);
				جاؤ fetch_file_process_aborted;
			} ورنہ {
				پیغام.type = FETCH_ERROR;
				پیغام.ڈیٹا.error = "Error reading file";
				fetch_file_send_callback(&پیغام, ctx);
				جاؤ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		پیغام.type = FETCH_ڈیٹا;
		پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) buf;
		پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = res;
		اگر (fetch_file_send_callback(&پیغام, ctx))
			توڑو;
	}

	اگر (ctx->aborted == false) {
		پیغام.type = FETCH_ختم_شدہ;
		fetch_file_send_callback(&پیغام, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	آزاد(buf);
#اگر_ختم
	واپس;
}

ساکن حرف *gen_nice_title(حرف *راستہ)
{
	حرف *nice_راستہ, *cnv, *tmp;
	حرف *title;
	صحیح title_لمبائی;

	/* Convert path for display */
	nice_راستہ = حافظہ_دو(لڑی_لمبائی(راستہ) * SLEN("&amp;") + 1);
	اگر (nice_راستہ == تہی) {
		واپس تہی;
	}

	/* Escape special HTML characters */
	برائے (cnv = nice_راستہ, tmp = راستہ; *tmp != '\0'; tmp++) {
		اگر (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ورنہ اگر (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ورنہ اگر (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ورنہ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_لمبائی = (cnv - nice_راستہ) + لڑی_لمبائی(messages_get("FileIndex"));
	title = حافظہ_دو(title_لمبائی + 1);

	اگر (title == تہی) {
		آزاد(nice_راستہ);
		واپس تہی;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_لمبائی, messages_get("FileIndex"), nice_راستہ);

	آزاد(nice_راستہ);

	واپس title;
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
ساکن nserror
process_dir_ent(ساخت fetch_file_context *ctx,
		 ساخت dirent *ent,
		 bool even,
		 حرف *بفر,
		 size_t بفر_len)
{
	nserror ret;
	حرف *urlpath = تہی; /* buffer for leaf entry path */
	ساخت stat ent_stat; /* stat result of leaf entry */
	حرف datebuf[64]; /* buffer for date text */
	حرف timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	اگر (ent->d_name[0] == '.') {
		واپس NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, تہی, 2, ctx->راستہ, ent->d_name);
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}

	اگر (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ورنہ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		اگر (strftime((حرف *)&datebuf, حجم datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		اگر (strftime((حرف *)&timebuf, حجم timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->راستہ_مقصد_nsurl(urlpath, &url);
	اگر (ret != NSERROR_OK) {
		آزاد(urlpath);
		واپس ret;
	}

	اگر (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_صف(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     بفر, بفر_len);
	} ورنہ اگر (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_صف(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     بفر, بفر_len);
	} ورنہ {
		/* something else */
		dirlist_generate_صف(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     بفر, بفر_len);
	}

	nsurl_unref(url);
	آزاد(urlpath);

	واپس NSERROR_OK;
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
ساکن صحیح dir_ترتیب_دو_alpha(ثابت ساخت dirent **d1, ثابت ساخت dirent **d2)
{
	ثابت حرف *s1 = (*d1)->d_name;
	ثابت حرف *s2 = (*d2)->d_name;

	جب (*s1 != '\0' && *s2 != '\0') {
		اگر ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			صحیح n1 = 0,  n2 = 0;
			جب (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			جب (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			اگر (n1 != n2) {
				واپس n1 - n2;
			}
			اگر (*s1 == '\0' || *s2 == '\0')
				توڑو;
		}
		اگر (tolower(*s1) != tolower(*s2))
			توڑو;

		s1++;
		s2++;
	}

	واپس tolower(*s1) - tolower(*s2);
}

ساکن خالی fetch_file_process_dir(ساخت fetch_file_context *ctx,
				   ساخت stat *fdstat)
{
	fetch_پیغام پیغام;
	حرف بفر[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	حرف *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *اوپر; /* url of parent */

	ساخت dirent **listing = تہی; /* directory entry listing */
	صحیح i; /* directory entry index */
	صحیح n; /* number of directory entries */

	n = scandir(ctx->راستہ, &listing, 0, dir_ترتیب_دو_alpha);
	اگر (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_مقصد_http_رمز(errno));
		واپس;
	}

	/* fetch is going to be successful */
	fetch_سیٹ_http_رمز(ctx->fetchh, 200);

	/* force no-cache */
	اگر (fetch_file_send_سرخی(ctx, "Cache-Control: no-cache"))
		جاؤ fetch_file_process_dir_aborted;

	/* content type */
	اگر (fetch_file_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8"))
		جاؤ fetch_file_process_dir_aborted;

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) بفر;

	/* directory listing top */
	dirlist_generate_چوٹی(بفر, حجم بفر);
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_file_send_callback(&پیغام, ctx))
		جاؤ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->راستہ);
	dirlist_generate_title(title, بفر, حجم بفر);
	آزاد(title);
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_file_send_callback(&پیغام, ctx))
		جاؤ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_والد(ctx->url, &اوپر);
	اگر (err == NSERROR_OK) {
		اگر (nsurl_compare(ctx->url, اوپر, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_والد_link(nsurl_access(اوپر),
					بفر, حجم بفر);

			پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
			fetch_file_send_callback(&پیغام, ctx);
		}
		nsurl_unref(اوپر);

		اگر (ctx->aborted)
			جاؤ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(بفر, حجم بفر);
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_file_send_callback(&پیغام, ctx))
		جاؤ fetch_file_process_dir_aborted;

	برائے (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, بفر,
				       حجم(بفر));

		اگر (err == NSERROR_OK) {
			پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
			اگر (fetch_file_send_callback(&پیغام, ctx))
				جاؤ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(بفر, حجم بفر);
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_file_send_callback(&پیغام, ctx))
		جاؤ fetch_file_process_dir_aborted;

	پیغام.type = FETCH_ختم_شدہ;
	fetch_file_send_callback(&پیغام, ctx);

fetch_file_process_dir_aborted:

	اگر (listing != تہی) {
		برائے (i = 0; i < n; i++) {
			آزاد(listing[i]);
		}
		آزاد(listing);
	}
}


/* process a file fetch */
ساکن خالی fetch_file_process(ساخت fetch_file_context *ctx)
{
	ساخت stat fdstat; /**< The objects stat */

	اگر (stat(ctx->راستہ, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_رمز(errno));
		واپس;
	}

	اگر (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		واپس;
	} ورنہ اگر (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		واپس;
	} ورنہ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	واپس;
}

/** callback to poll for additional file fetch contents */
ساکن خالی fetch_file_poll(lwc_string *scheme)
{
	ساخت fetch_file_context *c, *save_ring = تہی;

	جب (ring != تہی) {
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
			RING_داخل_کرو(save_ring, c);
			جاری;
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

nserror fetch_file_رجسٹر(خالی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.تیاری = fetch_file_تیاری,
		.آغاز = fetch_file_آغاز,
		.abort = fetch_file_abort,
		.آزاد = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	واپس fetcher_جوڑو(scheme, &fetcher_ops);
}
