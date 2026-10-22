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
#اگر_تعریف_شده HAVE_MMAP
#ضمن <sys/mman.h>
#پایان_اگر
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
#تعریف_کن FETCH_FILE_MAX_BUF_اندازه (1024 * 1024)

/** Context for a fetch */
بنية fetch_file_context {
	بنية fetch_file_context *r_بعدی, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	حرف *مسیر; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ساكن بنية fetch_file_context *ring = عدم;

/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_file_send_callback(ثابت fetch_پیام *پیام,
		بنية fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

ساكن bool fetch_file_send_سرآیند(بنية fetch_file_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پیام پیام;
	حرف سرآیند[64];
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرآیند, حجم سرآیند, fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرآیند) || len < 0) {
		أرجع false;
	}

	پیام.type = FETCH_سرآیند;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) سرآیند;
	پیام.داده.سرآیند_or_داده.len = len;

	أرجع fetch_file_send_callback(&پیام, ctx);
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
fetch_file_راهاندازی(بنية fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_داده *post_multipart,
		 ثابت حرف **سرآیندها)
{
	بنية fetch_file_context *ctx;
	صحيح i;
	nserror ret;

	ctx = حافظه_صفر_بده(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ret = guit->file->nsurl_مقصد_مسیر(url, &ctx->مسیر);
	إذا (ret != NSERROR_OK) {
		حرر(ctx);
		أرجع عدم;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	لكل (i = 0; سرآیندها[i] != عدم; i++) {
		إذا (strncasecmp(سرآیندها[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			استمر;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = سرآیندها[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		طالما (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		إذا (*d != '\0') {
			ret = nsc_snptimet(d, طول_رشته(d), &ctx->file_etag);
			إذا (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_درج(ring, ctx);

	أرجع ctx;
}

/** callback to free a file fetch */
ساكن فراغ fetch_file_free(فراغ *ctx)
{
	بنية fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	حرر(c->مسیر);
	حرر(ctx);
}

/** callback to start a file fetch */
ساكن bool fetch_file_شروع(فراغ *ctx)
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

ساكن صحيح fetch_file_errno_مقصد_http_کد(صحيح error_no)
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

ساكن فراغ fetch_file_process_error(بنية fetch_file_context *ctx, صحيح کد)
{
	fetch_پیام پیام;
	حرف میانگیر[1024];
	ثابت حرف *title;
	حرف کلید[8];

	/* content is going to return error code */
	fetch_مجموعه_http_کد(ctx->fetchh, کد);

	/* content type */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_error_aborted;

	snprintf(کلید, حجم کلید, "HTTP%03d", کد);
	title = messages_get(کلید);

	snprintf(میانگیر, حجم میانگیر,
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
		 messages_get("FetchErrorCode"), کد,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	پیام.type = FETCH_داده;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) میانگیر;
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_file_send_callback(&پیام, ctx))
		اذهب fetch_file_process_error_aborted;

	پیام.type = FETCH_پایانیافته;
	fetch_file_send_callback(&پیام, ctx);

fetch_file_process_error_aborted:
	أرجع;
}


/** Process object as a regular file */
ساكن فراغ fetch_file_process_plain(بنية fetch_file_context *ctx,
				     بنية stat *fdstat)
{
#اگر_تعریف_شده HAVE_MMAP
	fetch_پیام پیام;
	حرف *buf = عدم;
	size_t buf_اندازه;

	صحيح fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	إذا (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_مجموعه_http_کد(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیام, ctx);
		أرجع;
	}

	fd = open(ctx->مسیر, O_RDONLY);
	إذا (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_کد(errno));
		أرجع;
	}

	/* set buffer size */
	buf_اندازه = fdstat->st_size;

	/* allocate the buffer storage */
	إذا (buf_اندازه > 0) {
		buf = mmap(عدم, buf_اندازه, PROT_READ, MAP_SHARED, fd, 0);
		إذا (buf == MAP_FAILED) {
			پیام.type = FETCH_ERROR;
			پیام.داده.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&پیام, ctx);
			close(fd);
			أرجع;
		}
	}

	/* fetch is going to be successful */
	fetch_مجموعه_http_کد(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->مسیر))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	إذا (fetch_file_send_سرآیند(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	پیام.type = FETCH_داده;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) buf;
	پیام.داده.سرآیند_or_داده.len = buf_اندازه;
	fetch_file_send_callback(&پیام, ctx);

	إذا (ctx->aborted == false) {
		پیام.type = FETCH_پایانیافته;
		fetch_file_send_callback(&پیام, ctx);
	}

fetch_file_process_aborted:

	إذا (buf != عدم)
		munmap(buf, buf_اندازه);
	close(fd);
#وإلا
	fetch_پیام پیام;
	حرف *buf;
	size_t buf_اندازه;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	إذا (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_مجموعه_http_کد(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&پیام, ctx);
		أرجع;
	}

	infile = fopen(ctx->مسیر, "rb");
	إذا (infile == عدم) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_کد(errno));
		أرجع;
	}

	/* set buffer size */
	buf_اندازه = fdstat->st_size;
	إذا (buf_اندازه > FETCH_FILE_MAX_BUF_اندازه)
		buf_اندازه = FETCH_FILE_MAX_BUF_اندازه;

	/* allocate the buffer storage */
	buf = حافظه_بده(buf_اندازه);
	إذا (buf == عدم) {
		پیام.type = FETCH_ERROR;
		پیام.داده.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&پیام, ctx);
		fclose(infile);
		أرجع;
	}

	/* fetch is going to be successful */
	fetch_مجموعه_http_کد(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->مسیر))) {
		اذهب fetch_file_process_aborted;
	}

	/* content length */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		اذهب fetch_file_process_aborted;
	}

	/* create etag */
	إذا (fetch_file_send_سرآیند(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		اذهب fetch_file_process_aborted;
	}

	/* main data loop */
	طالما (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_اندازه, infile);
		إذا (res == 0) {
			إذا (feof(infile)) {
				پیام.type = FETCH_ERROR;
				پیام.داده.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&پیام, ctx);
				اذهب fetch_file_process_aborted;
			} وإلا {
				پیام.type = FETCH_ERROR;
				پیام.داده.error = "Error reading file";
				fetch_file_send_callback(&پیام, ctx);
				اذهب fetch_file_process_aborted;
			}
		}
		tot_read += res;

		پیام.type = FETCH_داده;
		پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) buf;
		پیام.داده.سرآیند_or_داده.len = res;
		إذا (fetch_file_send_callback(&پیام, ctx))
			اكسر;
	}

	إذا (ctx->aborted == false) {
		پیام.type = FETCH_پایانیافته;
		fetch_file_send_callback(&پیام, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	حرر(buf);
#پایان_اگر
	أرجع;
}

ساكن حرف *gen_nice_title(حرف *مسیر)
{
	حرف *nice_مسیر, *cnv, *tmp;
	حرف *title;
	صحيح title_درازی;

	/* Convert path for display */
	nice_مسیر = حافظه_بده(طول_رشته(مسیر) * SLEN("&amp;") + 1);
	إذا (nice_مسیر == عدم) {
		أرجع عدم;
	}

	/* Escape special HTML characters */
	لكل (cnv = nice_مسیر, tmp = مسیر; *tmp != '\0'; tmp++) {
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
	title_درازی = (cnv - nice_مسیر) + طول_رشته(messages_get("FileIndex"));
	title = حافظه_بده(title_درازی + 1);

	إذا (title == عدم) {
		حرر(nice_مسیر);
		أرجع عدم;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_درازی, messages_get("FileIndex"), nice_مسیر);

	حرر(nice_مسیر);

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
		 حرف *میانگیر,
		 size_t میانگیر_len)
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

	ret = netsurf_mkpath(&urlpath, عدم, 2, ctx->مسیر, ent->d_name);
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

	ret = guit->file->مسیر_مقصد_nsurl(urlpath, &url);
	إذا (ret != NSERROR_OK) {
		حرر(urlpath);
		أرجع ret;
	}

	إذا (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_ردیف(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     میانگیر, میانگیر_len);
	} وإلا إذا (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_ردیف(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     میانگیر, میانگیر_len);
	} وإلا {
		/* something else */
		dirlist_generate_ردیف(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     میانگیر, میانگیر_len);
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
ساكن صحيح dir_مرتب_سازی_alpha(ثابت بنية dirent **d1, ثابت بنية dirent **d2)
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
	fetch_پیام پیام;
	حرف میانگیر[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	حرف *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *به_بالا; /* url of parent */

	بنية dirent **listing = عدم; /* directory entry listing */
	صحيح i; /* directory entry index */
	صحيح n; /* number of directory entries */

	n = scandir(ctx->مسیر, &listing, 0, dir_مرتب_سازی_alpha);
	إذا (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_مقصد_http_کد(errno));
		أرجع;
	}

	/* fetch is going to be successful */
	fetch_مجموعه_http_کد(ctx->fetchh, 200);

	/* force no-cache */
	إذا (fetch_file_send_سرآیند(ctx, "Cache-Control: no-cache"))
		اذهب fetch_file_process_dir_aborted;

	/* content type */
	إذا (fetch_file_send_سرآیند(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_file_process_dir_aborted;

	پیام.type = FETCH_داده;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) میانگیر;

	/* directory listing top */
	dirlist_generate_بالاترین(میانگیر, حجم میانگیر);
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_file_send_callback(&پیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->مسیر);
	dirlist_generate_title(title, میانگیر, حجم میانگیر);
	حرر(title);
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_file_send_callback(&پیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_والد(ctx->url, &به_بالا);
	إذا (err == NSERROR_OK) {
		إذا (nsurl_compare(ctx->url, به_بالا, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_والد_link(nsurl_access(به_بالا),
					میانگیر, حجم میانگیر);

			پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
			fetch_file_send_callback(&پیام, ctx);
		}
		nsurl_unref(به_بالا);

		إذا (ctx->aborted)
			اذهب fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(میانگیر, حجم میانگیر);
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_file_send_callback(&پیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	لكل (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, میانگیر,
				       حجم(میانگیر));

		إذا (err == NSERROR_OK) {
			پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
			إذا (fetch_file_send_callback(&پیام, ctx))
				اذهب fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(میانگیر, حجم میانگیر);
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_file_send_callback(&پیام, ctx))
		اذهب fetch_file_process_dir_aborted;

	پیام.type = FETCH_پایانیافته;
	fetch_file_send_callback(&پیام, ctx);

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

	إذا (stat(ctx->مسیر, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_مقصد_http_کد(errno));
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
			RING_درج(save_ring, c);
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

nserror fetch_file_ثبات(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.راهاندازی = fetch_file_راهاندازی,
		.شروع = fetch_file_شروع,
		.abort = fetch_file_abort,
		.حرر = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	أرجع fetcher_جمع_کن(scheme, &fetcher_ops);
}
