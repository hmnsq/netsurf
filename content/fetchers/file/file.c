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

#అన్తర్భావ "utils/config.h"

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdarg.h>
#నిర్వచితమైతే HAVE_MMAP
#అన్తర్భావ <sys/mman.h>
#అయితే_ముగింపు
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/dirent.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/time.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "utils/file.h"
#అన్తర్భావ "netsurf/fetch.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"
#అన్తర్భావ "dirlist.h"
#అన్తర్భావ "file.h"

/* Maximum size of read buffer */
#నిర్వచించు FETCH_FILE_MAX_BUF_కొలత (1024 * 1024)

/** Context for a fetch */
నిర్మాణం fetch_file_context {
	నిర్మాణం fetch_file_context *r_తదుపరి, *r_prev;

	నిర్మాణం fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	అక్షరం *మార్గం; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

స్థిర నిర్మాణం fetch_file_context *ring = ఖాళీ;

/** issue fetch callbacks with locking */
స్థిర అంతఃవరుస bool fetch_file_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం,
		నిర్మాణం fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(సందేశం, ctx->fetchh);
	ctx->locked = false;

	తిరిగి ctx->aborted;
}

స్థిర bool fetch_file_send_శీర్షిక(నిర్మాణం fetch_file_context *ctx,
		స్థిరరాశి అక్షరం *fmt, ...)
{
	fetch_సందేశం సందేశం;
	అక్షరం శీర్షిక[64];
	va_list ap;
	పూర్ణాంకం len;

	va_start(ap, fmt);
	len = vsnprintf(శీర్షిక, పరిమాణం శీర్షిక, fmt, ap);
	va_end(ap);

	ఒకవేళ (len >= (పూర్ణాంకం)పరిమాణం(శీర్షిక) || len < 0) {
		తిరిగి false;
	}

	సందేశం.type = FETCH_శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = len;

	తిరిగి fetch_file_send_callback(&సందేశం, ctx);
}

/** callback to initialise the file fetcher. */
స్థిర bool fetch_file_initialise(lwc_string *scheme)
{
	తిరిగి true;
}

/** callback to initialise the file fetcher. */
స్థిర శూన్య fetch_file_finalise(lwc_string *scheme)
{
}

స్థిర bool fetch_file_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}

/** callback to set up a file fetch context. */
స్థిర శూన్య *
fetch_file_అమరిక(నిర్మాణం fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 స్థిరరాశి అక్షరం *post_urlenc,
		 స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		 స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం fetch_file_context *ctx;
	పూర్ణాంకం i;
	nserror ret;

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి ఖాళీ;

	ret = guit->file->nsurl_గమ్యం_మార్గం(url, &ctx->మార్గం);
	ఒకవేళ (ret != NSERROR_OK) {
		విడుదల(ctx);
		తిరిగి ఖాళీ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	కొరకు (i = 0; శీర్షికలు[i] != ఖాళీ; i++) {
		ఒకవేళ (strncasecmp(శీర్షికలు[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			కొనసాగు;
		}

		/* If-None-Match: "12345678" */
		స్థిరరాశి అక్షరం *d = శీర్షికలు[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		అయితే (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ఒకవేళ (*d != '\0') {
			ret = nsc_snptimet(d, అక్షరశ్రేణి_పొడవు(d), &ctx->file_etag);
			ఒకవేళ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_చొప్పించు(ring, ctx);

	తిరిగి ctx;
}

/** callback to free a file fetch */
స్థిర శూన్య fetch_file_free(శూన్య *ctx)
{
	నిర్మాణం fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	విడుదల(c->మార్గం);
	విడుదల(ctx);
}

/** callback to start a file fetch */
స్థిర bool fetch_file_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}

/** callback to abort a file fetch */
స్థిర శూన్య fetch_file_abort(శూన్య *ctx)
{
	నిర్మాణం fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

స్థిర పూర్ణాంకం fetch_file_errno_గమ్యం_http_సంకేతం(పూర్ణాంకం error_no)
{
	ఎంపిక (error_no) {
	స్థితి ENAMETOOLONG:
		తిరిగి 400;
	స్థితి EACCES:
		తిరిగి 403;
	స్థితి ENOENT:
		తిరిగి 404;
	అప్రమేయ:
		ఆపు;
	}

	తిరిగి 500;
}

స్థిర శూన్య fetch_file_process_error(నిర్మాణం fetch_file_context *ctx, పూర్ణాంకం సంకేతం_2)
{
	fetch_సందేశం సందేశం;
	అక్షరం బఫరు[1024];
	స్థిరరాశి అక్షరం *title;
	అక్షరం కీలకం[8];

	/* content is going to return error code */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, సంకేతం_2);

	/* content type */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Type: text/html; charset=utf-8"))
		వెళ్ళు fetch_file_process_error_aborted;

	snprintf(కీలకం, పరిమాణం కీలకం, "HTTP%03d", సంకేతం_2);
	title = messages_get(కీలకం);

	snprintf(బఫరు, పరిమాణం బఫరు,
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
		 messages_get("FetchErrorCode"), సంకేతం_2,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) బఫరు;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_file_process_error_aborted;

	సందేశం.type = FETCH_ముగిసిన;
	fetch_file_send_callback(&సందేశం, ctx);

fetch_file_process_error_aborted:
	తిరిగి;
}


/** Process object as a regular file */
స్థిర శూన్య fetch_file_process_plain(నిర్మాణం fetch_file_context *ctx,
				     నిర్మాణం stat *fdstat)
{
#నిర్వచితమైతే HAVE_MMAP
	fetch_సందేశం సందేశం;
	అక్షరం *buf = ఖాళీ;
	size_t buf_కొలత;

	పూర్ణాంకం fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ఒకవేళ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_సమితి_http_సంకేతం(ctx->fetchh, 304);
		సందేశం.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&సందేశం, ctx);
		తిరిగి;
	}

	fd = open(ctx->మార్గం, O_RDONLY);
	ఒకవేళ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_గమ్యం_http_సంకేతం(errno));
		తిరిగి;
	}

	/* set buffer size */
	buf_కొలత = fdstat->st_size;

	/* allocate the buffer storage */
	ఒకవేళ (buf_కొలత > 0) {
		buf = mmap(ఖాళీ, buf_కొలత, PROT_READ, MAP_SHARED, fd, 0);
		ఒకవేళ (buf == MAP_FAILED) {
			సందేశం.type = FETCH_ERROR;
			సందేశం.దత్తాంశం.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&సందేశం, ctx);
			close(fd);
			తిరిగి;
		}
	}

	/* fetch is going to be successful */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->మార్గం))) {
		వెళ్ళు fetch_file_process_aborted;
	}

	/* content length */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		వెళ్ళు fetch_file_process_aborted;
	}

	/* create etag */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		వెళ్ళు fetch_file_process_aborted;
	}

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) buf;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = buf_కొలత;
	fetch_file_send_callback(&సందేశం, ctx);

	ఒకవేళ (ctx->aborted == false) {
		సందేశం.type = FETCH_ముగిసిన;
		fetch_file_send_callback(&సందేశం, ctx);
	}

fetch_file_process_aborted:

	ఒకవేళ (buf != ఖాళీ)
		munmap(buf, buf_కొలత);
	close(fd);
#లేకపోతే
	fetch_సందేశం సందేశం;
	అక్షరం *buf;
	size_t buf_కొలత;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ఒకవేళ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_సమితి_http_సంకేతం(ctx->fetchh, 304);
		సందేశం.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&సందేశం, ctx);
		తిరిగి;
	}

	infile = fopen(ctx->మార్గం, "rb");
	ఒకవేళ (infile == ఖాళీ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_గమ్యం_http_సంకేతం(errno));
		తిరిగి;
	}

	/* set buffer size */
	buf_కొలత = fdstat->st_size;
	ఒకవేళ (buf_కొలత > FETCH_FILE_MAX_BUF_కొలత)
		buf_కొలత = FETCH_FILE_MAX_BUF_కొలత;

	/* allocate the buffer storage */
	buf = జ్ఞాపకం_ఇవ్వు(buf_కొలత);
	ఒకవేళ (buf == ఖాళీ) {
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&సందేశం, ctx);
		fclose(infile);
		తిరిగి;
	}

	/* fetch is going to be successful */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->మార్గం))) {
		వెళ్ళు fetch_file_process_aborted;
	}

	/* content length */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		వెళ్ళు fetch_file_process_aborted;
	}

	/* create etag */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		వెళ్ళు fetch_file_process_aborted;
	}

	/* main data loop */
	అయితే (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_కొలత, infile);
		ఒకవేళ (res == 0) {
			ఒకవేళ (feof(infile)) {
				సందేశం.type = FETCH_ERROR;
				సందేశం.దత్తాంశం.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&సందేశం, ctx);
				వెళ్ళు fetch_file_process_aborted;
			} లేకపోతే {
				సందేశం.type = FETCH_ERROR;
				సందేశం.దత్తాంశం.error = "Error reading file";
				fetch_file_send_callback(&సందేశం, ctx);
				వెళ్ళు fetch_file_process_aborted;
			}
		}
		tot_read += res;

		సందేశం.type = FETCH_దత్తాంశం;
		సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) buf;
		సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = res;
		ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
			ఆపు;
	}

	ఒకవేళ (ctx->aborted == false) {
		సందేశం.type = FETCH_ముగిసిన;
		fetch_file_send_callback(&సందేశం, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	విడుదల(buf);
#అయితే_ముగింపు
	తిరిగి;
}

స్థిర అక్షరం *gen_nice_title(అక్షరం *మార్గం)
{
	అక్షరం *nice_మార్గం, *cnv, *tmp;
	అక్షరం *title;
	పూర్ణాంకం title_నిడివి;

	/* Convert path for display */
	nice_మార్గం = జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(మార్గం) * SLEN("&amp;") + 1);
	ఒకవేళ (nice_మార్గం == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	/* Escape special HTML characters */
	కొరకు (cnv = nice_మార్గం, tmp = మార్గం; *tmp != '\0'; tmp++) {
		ఒకవేళ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} లేకపోతే ఒకవేళ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} లేకపోతే ఒకవేళ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} లేకపోతే {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_నిడివి = (cnv - nice_మార్గం) + అక్షరశ్రేణి_పొడవు(messages_get("FileIndex"));
	title = జ్ఞాపకం_ఇవ్వు(title_నిడివి + 1);

	ఒకవేళ (title == ఖాళీ) {
		విడుదల(nice_మార్గం);
		తిరిగి ఖాళీ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_నిడివి, messages_get("FileIndex"), nice_మార్గం);

	విడుదల(nice_మార్గం);

	తిరిగి title;
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
స్థిర nserror
process_dir_ent(నిర్మాణం fetch_file_context *ctx,
		 నిర్మాణం dirent *ent,
		 bool even,
		 అక్షరం *బఫరు,
		 size_t బఫరు_len)
{
	nserror ret;
	అక్షరం *urlpath = ఖాళీ; /* buffer for leaf entry path */
	నిర్మాణం stat ent_stat; /* stat result of leaf entry */
	అక్షరం datebuf[64]; /* buffer for date text */
	అక్షరం timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ఒకవేళ (ent->d_name[0] == '.') {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ఖాళీ, 2, ctx->మార్గం, ent->d_name);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	ఒకవేళ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} లేకపోతే {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ఒకవేళ (strftime((అక్షరం *)&datebuf, పరిమాణం datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ఒకవేళ (strftime((అక్షరం *)&timebuf, పరిమాణం timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->మార్గం_గమ్యం_nsurl(urlpath, &url);
	ఒకవేళ (ret != NSERROR_OK) {
		విడుదల(urlpath);
		తిరిగి ret;
	}

	ఒకవేళ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_అడ్డువరుస(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     బఫరు, బఫరు_len);
	} లేకపోతే ఒకవేళ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_అడ్డువరుస(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     బఫరు, బఫరు_len);
	} లేకపోతే {
		/* something else */
		dirlist_generate_అడ్డువరుస(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     బఫరు, బఫరు_len);
	}

	nsurl_unref(url);
	విడుదల(urlpath);

	తిరిగి NSERROR_OK;
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
స్థిర పూర్ణాంకం dir_క్రమీకరించు_alpha(స్థిరరాశి నిర్మాణం dirent **d1, స్థిరరాశి నిర్మాణం dirent **d2)
{
	స్థిరరాశి అక్షరం *s1 = (*d1)->d_name;
	స్థిరరాశి అక్షరం *s2 = (*d2)->d_name;

	అయితే (*s1 != '\0' && *s2 != '\0') {
		ఒకవేళ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			పూర్ణాంకం n1 = 0,  n2 = 0;
			అయితే (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			అయితే (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ఒకవేళ (n1 != n2) {
				తిరిగి n1 - n2;
			}
			ఒకవేళ (*s1 == '\0' || *s2 == '\0')
				ఆపు;
		}
		ఒకవేళ (tolower(*s1) != tolower(*s2))
			ఆపు;

		s1++;
		s2++;
	}

	తిరిగి tolower(*s1) - tolower(*s2);
}

స్థిర శూన్య fetch_file_process_dir(నిర్మాణం fetch_file_context *ctx,
				   నిర్మాణం stat *fdstat)
{
	fetch_సందేశం సందేశం;
	అక్షరం బఫరు[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	అక్షరం *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *పైకి; /* url of parent */

	నిర్మాణం dirent **listing = ఖాళీ; /* directory entry listing */
	పూర్ణాంకం i; /* directory entry index */
	పూర్ణాంకం n; /* number of directory entries */

	n = scandir(ctx->మార్గం, &listing, 0, dir_క్రమీకరించు_alpha);
	ఒకవేళ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_గమ్యం_http_సంకేతం(errno));
		తిరిగి;
	}

	/* fetch is going to be successful */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, 200);

	/* force no-cache */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Cache-Control: no-cache"))
		వెళ్ళు fetch_file_process_dir_aborted;

	/* content type */
	ఒకవేళ (fetch_file_send_శీర్షిక(ctx, "Content-Type: text/html; charset=utf-8"))
		వెళ్ళు fetch_file_process_dir_aborted;

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) బఫరు;

	/* directory listing top */
	dirlist_generate_శిఖరం(బఫరు, పరిమాణం బఫరు);
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->మార్గం);
	dirlist_generate_title(title, బఫరు, పరిమాణం బఫరు);
	విడుదల(title);
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_మాతృ(ctx->url, &పైకి);
	ఒకవేళ (err == NSERROR_OK) {
		ఒకవేళ (nsurl_compare(ctx->url, పైకి, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_మాతృ_link(nsurl_access(పైకి),
					బఫరు, పరిమాణం బఫరు);

			సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
			fetch_file_send_callback(&సందేశం, ctx);
		}
		nsurl_unref(పైకి);

		ఒకవేళ (ctx->aborted)
			వెళ్ళు fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(బఫరు, పరిమాణం బఫరు);
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_file_process_dir_aborted;

	కొరకు (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, బఫరు,
				       పరిమాణం(బఫరు));

		ఒకవేళ (err == NSERROR_OK) {
			సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
			ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
				వెళ్ళు fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(బఫరు, పరిమాణం బఫరు);
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_file_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_file_process_dir_aborted;

	సందేశం.type = FETCH_ముగిసిన;
	fetch_file_send_callback(&సందేశం, ctx);

fetch_file_process_dir_aborted:

	ఒకవేళ (listing != ఖాళీ) {
		కొరకు (i = 0; i < n; i++) {
			విడుదల(listing[i]);
		}
		విడుదల(listing);
	}
}


/* process a file fetch */
స్థిర శూన్య fetch_file_process(నిర్మాణం fetch_file_context *ctx)
{
	నిర్మాణం stat fdstat; /**< The objects stat */

	ఒకవేళ (stat(ctx->మార్గం, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_గమ్యం_http_సంకేతం(errno));
		తిరిగి;
	}

	ఒకవేళ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		తిరిగి;
	} లేకపోతే ఒకవేళ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		తిరిగి;
	} లేకపోతే {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	తిరిగి;
}

/** callback to poll for additional file fetch contents */
స్థిర శూన్య fetch_file_poll(lwc_string *scheme)
{
	నిర్మాణం fetch_file_context *c, *save_ring = ఖాళీ;

	అయితే (ring != ఖాళీ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ఒకవేళ (c->locked == true) {
			RING_చొప్పించు(save_ring, c);
			కొనసాగు;
		}

		/* Only process non-aborted fetches */
		ఒకవేళ (c->aborted == false) {
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

nserror fetch_file_రిజిస్టరు(శూన్య)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.అమరిక = fetch_file_అమరిక,
		.ప్రారంభం = fetch_file_ప్రారంభం,
		.abort = fetch_file_abort,
		.విడుదల = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	తిరిగి fetcher_కూడు(scheme, &fetcher_ops);
}
