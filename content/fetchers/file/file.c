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

#অন্তর্ভাব "utils/config.h"

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdarg.h>
#ifdef HAVE_MMAP
#অন্তর্ভাব <sys/mman.h>
#endif
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/dirent.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/time.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "utils/file.h"
#অন্তর্ভাব "netsurf/fetch.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"
#অন্তর্ভাব "dirlist.h"
#অন্তর্ভাব "file.h"

/* Maximum size of read buffer */
#define FETCH_FILE_MAX_BUF_আকার (1024 * 1024)

/** Context for a fetch */
কাঠামো fetch_file_context {
	কাঠামো fetch_file_context *r_পরবর্তী, *r_পূর্ব;

	কাঠামো fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	অক্ষর *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

স্থির কাঠামো fetch_file_context *ring = ফাঁকা;

/** issue fetch callbacks with locking */
স্থির অন্তঃরেখ bool fetch_file_send_callback(ধ্রুবক fetch_বার্তা *বার্তা,
		কাঠামো fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বার্তা, ctx->fetchh);
	ctx->locked = false;

	ফেরত ctx->aborted;
}

স্থির bool fetch_file_send_header(কাঠামো fetch_file_context *ctx,
		ধ্রুবক অক্ষর *fmt, ...)
{
	fetch_বার্তা বার্তা;
	অক্ষর header[64];
	va_list ap;
	পূর্ণসংখ্যা len;

	va_start(ap, fmt);
	len = vsnprintf(header, আকার header, fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণসংখ্যা)আকার(header) || len < 0) {
		ফেরত false;
	}

	বার্তা.type = FETCH_HEADER;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) header;
	বার্তা.তথ্য.header_or_তথ্য.len = len;

	ফেরত fetch_file_send_callback(&বার্তা, ctx);
}

/** callback to initialise the file fetcher. */
স্থির bool fetch_file_initialise(lwc_string *scheme)
{
	ফেরত true;
}

/** callback to initialise the file fetcher. */
স্থির শূন্য fetch_file_finalise(lwc_string *scheme)
{
}

স্থির bool fetch_file_can_fetch(ধ্রুবক nsurl *url)
{
	ফেরত true;
}

/** callback to set up a file fetch context. */
স্থির শূন্য *
fetch_file_setup(কাঠামো fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ধ্রুবক অক্ষর *post_urlenc,
		 ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		 ধ্রুবক অক্ষর **headers)
{
	কাঠামো fetch_file_context *ctx;
	পূর্ণসংখ্যা i;
	nserror ret;

	ctx = calloc(1, আকার(*ctx));
	যদি (ctx == ফাঁকা)
		ফেরত ফাঁকা;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	যদি (ret != NSERROR_OK) {
		মুক্ত(ctx);
		ফেরত ফাঁকা;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	জন্য (i = 0; headers[i] != ফাঁকা; i++) {
		যদি (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			চালু;
		}

		/* If-None-Match: "12345678" */
		ধ্রুবক অক্ষর *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		যখন (*d != '\0' && (*d < '0' || '9' < *d))
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

	ফেরত ctx;
}

/** callback to free a file fetch */
স্থির শূন্য fetch_file_free(শূন্য *ctx)
{
	কাঠামো fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	মুক্ত(c->path);
	মুক্ত(ctx);
}

/** callback to start a file fetch */
স্থির bool fetch_file_start(শূন্য *ctx)
{
	ফেরত true;
}

/** callback to abort a file fetch */
স্থির শূন্য fetch_file_abort(শূন্য *ctx)
{
	কাঠামো fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

স্থির পূর্ণসংখ্যা fetch_file_errno_to_http_code(পূর্ণসংখ্যা error_no)
{
	নির্বাচন (error_no) {
	ক্ষেত্র ENAMETOOLONG:
		ফেরত 400;
	ক্ষেত্র EACCES:
		ফেরত 403;
	ক্ষেত্র ENOENT:
		ফেরত 404;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	ফেরত 500;
}

স্থির শূন্য fetch_file_process_error(কাঠামো fetch_file_context *ctx, পূর্ণসংখ্যা code)
{
	fetch_বার্তা বার্তা;
	অক্ষর বাফার[1024];
	ধ্রুবক অক্ষর *title;
	অক্ষর চাবি[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		যাও fetch_file_process_error_aborted;

	snprintf(চাবি, আকার চাবি, "HTTP%03d", code);
	title = messages_get(চাবি);

	snprintf(বাফার, আকার বাফার,
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

	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) বাফার;
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_file_send_callback(&বার্তা, ctx))
		যাও fetch_file_process_error_aborted;

	বার্তা.type = FETCH_FINISHED;
	fetch_file_send_callback(&বার্তা, ctx);

fetch_file_process_error_aborted:
	ফেরত;
}


/** Process object as a regular file */
স্থির শূন্য fetch_file_process_plain(কাঠামো fetch_file_context *ctx,
				     কাঠামো stat *fdstat)
{
#ifdef HAVE_MMAP
	fetch_বার্তা বার্তা;
	অক্ষর *buf = ফাঁকা;
	size_t buf_আকার;

	পূর্ণসংখ্যা fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	যদি (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		বার্তা.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&বার্তা, ctx);
		ফেরত;
	}

	fd = open(ctx->path, O_RDONLY);
	যদি (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ফেরত;
	}

	/* set buffer size */
	buf_আকার = fdstat->st_size;

	/* allocate the buffer storage */
	যদি (buf_আকার > 0) {
		buf = mmap(ফাঁকা, buf_আকার, PROT_READ, MAP_SHARED, fd, 0);
		যদি (buf == MAP_FAILED) {
			বার্তা.type = FETCH_ERROR;
			বার্তা.তথ্য.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&বার্তা, ctx);
			close(fd);
			ফেরত;
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
		যাও fetch_file_process_aborted;
	}

	/* content length */
	যদি (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		যাও fetch_file_process_aborted;
	}

	/* create etag */
	যদি (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		যাও fetch_file_process_aborted;
	}

	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) buf;
	বার্তা.তথ্য.header_or_তথ্য.len = buf_আকার;
	fetch_file_send_callback(&বার্তা, ctx);

	যদি (ctx->aborted == false) {
		বার্তা.type = FETCH_FINISHED;
		fetch_file_send_callback(&বার্তা, ctx);
	}

fetch_file_process_aborted:

	যদি (buf != ফাঁকা)
		munmap(buf, buf_আকার);
	close(fd);
#অন্যথা
	fetch_বার্তা বার্তা;
	অক্ষর *buf;
	size_t buf_আকার;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	যদি (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		বার্তা.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&বার্তা, ctx);
		ফেরত;
	}

	infile = fopen(ctx->path, "rb");
	যদি (infile == ফাঁকা) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ফেরত;
	}

	/* set buffer size */
	buf_আকার = fdstat->st_size;
	যদি (buf_আকার > FETCH_FILE_MAX_BUF_আকার)
		buf_আকার = FETCH_FILE_MAX_BUF_আকার;

	/* allocate the buffer storage */
	buf = malloc(buf_আকার);
	যদি (buf == ফাঁকা) {
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&বার্তা, ctx);
		fclose(infile);
		ফেরত;
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
		যাও fetch_file_process_aborted;
	}

	/* content length */
	যদি (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		যাও fetch_file_process_aborted;
	}

	/* create etag */
	যদি (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		যাও fetch_file_process_aborted;
	}

	/* main data loop */
	যখন (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_আকার, infile);
		যদি (res == 0) {
			যদি (feof(infile)) {
				বার্তা.type = FETCH_ERROR;
				বার্তা.তথ্য.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&বার্তা, ctx);
				যাও fetch_file_process_aborted;
			} অন্যথা {
				বার্তা.type = FETCH_ERROR;
				বার্তা.তথ্য.error = "Error reading file";
				fetch_file_send_callback(&বার্তা, ctx);
				যাও fetch_file_process_aborted;
			}
		}
		tot_read += res;

		বার্তা.type = FETCH_তথ্য;
		বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) buf;
		বার্তা.তথ্য.header_or_তথ্য.len = res;
		যদি (fetch_file_send_callback(&বার্তা, ctx))
			বিরতি;
	}

	যদি (ctx->aborted == false) {
		বার্তা.type = FETCH_FINISHED;
		fetch_file_send_callback(&বার্তা, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	মুক্ত(buf);
#endif
	ফেরত;
}

স্থির অক্ষর *gen_nice_title(অক্ষর *path)
{
	অক্ষর *nice_path, *cnv, *tmp;
	অক্ষর *title;
	পূর্ণসংখ্যা title_দৈর্ঘ্য;

	/* Convert path for display */
	nice_path = malloc(strlen(path) * SLEN("&amp;") + 1);
	যদি (nice_path == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	/* Escape special HTML characters */
	জন্য (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
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
	title_দৈর্ঘ্য = (cnv - nice_path) + strlen(messages_get("FileIndex"));
	title = malloc(title_দৈর্ঘ্য + 1);

	যদি (title == ফাঁকা) {
		মুক্ত(nice_path);
		ফেরত ফাঁকা;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_দৈর্ঘ্য, messages_get("FileIndex"), nice_path);

	মুক্ত(nice_path);

	ফেরত title;
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
process_dir_ent(কাঠামো fetch_file_context *ctx,
		 কাঠামো dirent *ent,
		 bool even,
		 অক্ষর *বাফার,
		 size_t বাফার_len)
{
	nserror ret;
	অক্ষর *urlpath = ফাঁকা; /* buffer for leaf entry path */
	কাঠামো stat ent_stat; /* stat result of leaf entry */
	অক্ষর datebuf[64]; /* buffer for date text */
	অক্ষর timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	যদি (ent->d_name[0] == '.') {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ফাঁকা, 2, ctx->path, ent->d_name);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	যদি (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} অন্যথা {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		যদি (strftime((অক্ষর *)&datebuf, আকার datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		যদি (strftime((অক্ষর *)&timebuf, আকার timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	যদি (ret != NSERROR_OK) {
		মুক্ত(urlpath);
		ফেরত ret;
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
				     বাফার, বাফার_len);
	} অন্যথা যদি (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     বাফার, বাফার_len);
	} অন্যথা {
		/* something else */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     বাফার, বাফার_len);
	}

	nsurl_unref(url);
	মুক্ত(urlpath);

	ফেরত NSERROR_OK;
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
স্থির পূর্ণসংখ্যা dir_sort_alpha(ধ্রুবক কাঠামো dirent **d1, ধ্রুবক কাঠামো dirent **d2)
{
	ধ্রুবক অক্ষর *s1 = (*d1)->d_name;
	ধ্রুবক অক্ষর *s2 = (*d2)->d_name;

	যখন (*s1 != '\0' && *s2 != '\0') {
		যদি ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			পূর্ণসংখ্যা n1 = 0,  n2 = 0;
			যখন (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			যখন (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			যদি (n1 != n2) {
				ফেরত n1 - n2;
			}
			যদি (*s1 == '\0' || *s2 == '\0')
				বিরতি;
		}
		যদি (tolower(*s1) != tolower(*s2))
			বিরতি;

		s1++;
		s2++;
	}

	ফেরত tolower(*s1) - tolower(*s2);
}

স্থির শূন্য fetch_file_process_dir(কাঠামো fetch_file_context *ctx,
				   কাঠামো stat *fdstat)
{
	fetch_বার্তা বার্তা;
	অক্ষর বাফার[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	অক্ষর *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	কাঠামো dirent **listing = ফাঁকা; /* directory entry listing */
	পূর্ণসংখ্যা i; /* directory entry index */
	পূর্ণসংখ্যা n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	যদি (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(errno));
		ফেরত;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	যদি (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		যাও fetch_file_process_dir_aborted;

	/* content type */
	যদি (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		যাও fetch_file_process_dir_aborted;

	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) বাফার;

	/* directory listing top */
	dirlist_generate_শীর্ষ(বাফার, আকার বাফার);
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_file_send_callback(&বার্তা, ctx))
		যাও fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, বাফার, আকার বাফার);
	মুক্ত(title);
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_file_send_callback(&বার্তা, ctx))
		যাও fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	যদি (err == NSERROR_OK) {
		যদি (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					বাফার, আকার বাফার);

			বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
			fetch_file_send_callback(&বার্তা, ctx);
		}
		nsurl_unref(up);

		যদি (ctx->aborted)
			যাও fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(বাফার, আকার বাফার);
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_file_send_callback(&বার্তা, ctx))
		যাও fetch_file_process_dir_aborted;

	জন্য (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, বাফার,
				       আকার(বাফার));

		যদি (err == NSERROR_OK) {
			বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
			যদি (fetch_file_send_callback(&বার্তা, ctx))
				যাও fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(বাফার, আকার বাফার);
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_file_send_callback(&বার্তা, ctx))
		যাও fetch_file_process_dir_aborted;

	বার্তা.type = FETCH_FINISHED;
	fetch_file_send_callback(&বার্তা, ctx);

fetch_file_process_dir_aborted:

	যদি (listing != ফাঁকা) {
		জন্য (i = 0; i < n; i++) {
			মুক্ত(listing[i]);
		}
		মুক্ত(listing);
	}
}


/* process a file fetch */
স্থির শূন্য fetch_file_process(কাঠামো fetch_file_context *ctx)
{
	কাঠামো stat fdstat; /**< The objects stat */

	যদি (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(errno));
		ফেরত;
	}

	যদি (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ফেরত;
	} অন্যথা যদি (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ফেরত;
	} অন্যথা {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ফেরত;
}

/** callback to poll for additional file fetch contents */
স্থির শূন্য fetch_file_poll(lwc_string *scheme)
{
	কাঠামো fetch_file_context *c, *save_ring = ফাঁকা;

	যখন (ring != ফাঁকা) {
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
			চালু;
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
	ধ্রুবক কাঠামো fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.abort = fetch_file_abort,
		.মুক্ত = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ফেরত fetcher_add(scheme, &fetcher_ops);
}
