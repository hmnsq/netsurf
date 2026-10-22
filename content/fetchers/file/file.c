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

#ಅನ್ತರ್ಭಾವ "utils/config.h"

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MMAP
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/dirent.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/time.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/file.h"
#ಅನ್ತರ್ಭಾವ "netsurf/fetch.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "dirlist.h"
#ಅನ್ತರ್ಭಾವ "file.h"

/* Maximum size of read buffer */
#ವ್ಯಾಖ್ಯಾನಿಸು FETCH_FILE_MAX_BUF_ಗಾತ್ರ (1024 * 1024)

/** Context for a fetch */
ರಚನೆ fetch_file_context {
	ರಚನೆ fetch_file_context *r_ಮುಂದಿನ, *r_prev;

	ರಚನೆ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	ಅಕ್ಷರ *ಮಾರ್ಗ; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ಸ್ಥಿರ ರಚನೆ fetch_file_context *ring = ಖಾಲಿ;

/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool fetch_file_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ರಚನೆ fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಹಿಂತಿರುಗಿ ctx->aborted;
}

ಸ್ಥಿರ bool fetch_file_send_ಶೀರ್ಷಿಕೆ(ರಚನೆ fetch_file_context *ctx,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[64];
	va_list ap;
	ಪೂರ್ಣಾಂಕ len;

	va_start(ap, fmt);
	len = vsnprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ, fmt, ap);
	va_end(ap);

	ಆದರೆ (len >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ) || len < 0) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = len;

	ಹಿಂತಿರುಗಿ fetch_file_send_callback(&ಸಂದೇಶ, ctx);
}

/** callback to initialise the file fetcher. */
ಸ್ಥಿರ bool fetch_file_initialise(lwc_string *scheme)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to initialise the file fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_finalise(lwc_string *scheme)
{
}

ಸ್ಥಿರ bool fetch_file_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to set up a file fetch context. */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_file_ಸಿದ್ಧತೆ(ರಚನೆ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ fetch_file_context *ctx;
	ಪೂರ್ಣಾಂಕ i;
	nserror ret;

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ret = guit->file->nsurl_ಗಮ್ಯ_ಮಾರ್ಗ(url, &ctx->ಮಾರ್ಗ);
	ಆದರೆ (ret != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ಗಾಗಿ (i = 0; ಶೀರ್ಷಿಕೆಗಳು[i] != ಖಾಲಿ; i++) {
		ಆದರೆ (strncasecmp(ಶೀರ್ಷಿಕೆಗಳು[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ಮುಂದುವರಿ;
		}

		/* If-None-Match: "12345678" */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *d = ಶೀರ್ಷಿಕೆಗಳು[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ಆಗ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ಆದರೆ (*d != '\0') {
			ret = nsc_snptimet(d, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(d), &ctx->file_etag);
			ಆದರೆ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಹಿಂತಿರುಗಿ ctx;
}

/** callback to free a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_free(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	ಬಿಡುಗಡೆ(c->ಮಾರ್ಗ);
	ಬಿಡುಗಡೆ(ctx);
}

/** callback to start a file fetch */
ಸ್ಥಿರ bool fetch_file_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to abort a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_abort(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fetch_file_errno_ಗಮ್ಯ_http_ಸಂಕೇತ(ಪೂರ್ಣಾಂಕ error_no)
{
	ಆಯ್ಕೆ (error_no) {
	ಸ್ಥಿತಿ ENAMETOOLONG:
		ಹಿಂತಿರುಗಿ 400;
	ಸ್ಥಿತಿ EACCES:
		ಹಿಂತಿರುಗಿ 403;
	ಸ್ಥಿತಿ ENOENT:
		ಹಿಂತಿರುಗಿ 404;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ 500;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_error(ರಚನೆ fetch_file_context *ctx, ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ ಬಫರ್[1024];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;
	ಅಕ್ಷರ ಕೀಲಿ[8];

	/* content is going to return error code */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, ಸಂಕೇತ_2);

	/* content type */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html; charset=utf-8"))
		ಹೋಗು fetch_file_process_error_aborted;

	snprintf(ಕೀಲಿ, ಗಾತ್ರ ಕೀಲಿ, "HTTP%03d", ಸಂಕೇತ_2);
	title = messages_get(ಕೀಲಿ);

	snprintf(ಬಫರ್, ಗಾತ್ರ ಬಫರ್,
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
		 messages_get("FetchErrorCode"), ಸಂಕೇತ_2,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಬಫರ್;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_file_process_error_aborted;

	ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

fetch_file_process_error_aborted:
	ಹಿಂತಿರುಗಿ;
}


/** Process object as a regular file */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_plain(ರಚನೆ fetch_file_context *ctx,
				     ರಚನೆ stat *fdstat)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MMAP
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ *buf = ಖಾಲಿ;
	size_t buf_ಗಾತ್ರ;

	ಪೂರ್ಣಾಂಕ fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ಆದರೆ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		ಹಿಂತಿರುಗಿ;
	}

	fd = open(ctx->ಮಾರ್ಗ, O_RDONLY);
	ಆದರೆ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ಗಮ್ಯ_http_ಸಂಕೇತ(errno));
		ಹಿಂತಿರುಗಿ;
	}

	/* set buffer size */
	buf_ಗಾತ್ರ = fdstat->st_size;

	/* allocate the buffer storage */
	ಆದರೆ (buf_ಗಾತ್ರ > 0) {
		buf = mmap(ಖಾಲಿ, buf_ಗಾತ್ರ, PROT_READ, MAP_SHARED, fd, 0);
		ಆದರೆ (buf == MAP_FAILED) {
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&ಸಂದೇಶ, ctx);
			close(fd);
			ಹಿಂತಿರುಗಿ;
		}
	}

	/* fetch is going to be successful */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->ಮಾರ್ಗ))) {
		ಹೋಗು fetch_file_process_aborted;
	}

	/* content length */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ಹೋಗು fetch_file_process_aborted;
	}

	/* create etag */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		ಹೋಗು fetch_file_process_aborted;
	}

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) buf;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = buf_ಗಾತ್ರ;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

	ಆದರೆ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_file_process_aborted:

	ಆದರೆ (buf != ಖಾಲಿ)
		munmap(buf, buf_ಗಾತ್ರ);
	close(fd);
#ಇಲ್ಲದಿದ್ದರೆ
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ *buf;
	size_t buf_ಗಾತ್ರ;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ಆದರೆ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		ಹಿಂತಿರುಗಿ;
	}

	infile = fopen(ctx->ಮಾರ್ಗ, "rb");
	ಆದರೆ (infile == ಖಾಲಿ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ಗಮ್ಯ_http_ಸಂಕೇತ(errno));
		ಹಿಂತಿರುಗಿ;
	}

	/* set buffer size */
	buf_ಗಾತ್ರ = fdstat->st_size;
	ಆದರೆ (buf_ಗಾತ್ರ > FETCH_FILE_MAX_BUF_ಗಾತ್ರ)
		buf_ಗಾತ್ರ = FETCH_FILE_MAX_BUF_ಗಾತ್ರ;

	/* allocate the buffer storage */
	buf = ಸ್ಮರಣೆ_ಕೊಡು(buf_ಗಾತ್ರ);
	ಆದರೆ (buf == ಖಾಲಿ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		fclose(infile);
		ಹಿಂತಿರುಗಿ;
	}

	/* fetch is going to be successful */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->ಮಾರ್ಗ))) {
		ಹೋಗು fetch_file_process_aborted;
	}

	/* content length */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ಹೋಗು fetch_file_process_aborted;
	}

	/* create etag */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		ಹೋಗು fetch_file_process_aborted;
	}

	/* main data loop */
	ಆಗ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_ಗಾತ್ರ, infile);
		ಆದರೆ (res == 0) {
			ಆದರೆ (feof(infile)) {
				ಸಂದೇಶ.type = FETCH_ERROR;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&ಸಂದೇಶ, ctx);
				ಹೋಗು fetch_file_process_aborted;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಸಂದೇಶ.type = FETCH_ERROR;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Error reading file";
				fetch_file_send_callback(&ಸಂದೇಶ, ctx);
				ಹೋಗು fetch_file_process_aborted;
			}
		}
		tot_read += res;

		ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) buf;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = res;
		ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
		fetch_file_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	ಬಿಡುಗಡೆ(buf);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಅಕ್ಷರ *gen_nice_title(ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *nice_ಮಾರ್ಗ, *cnv, *tmp;
	ಅಕ್ಷರ *title;
	ಪೂರ್ಣಾಂಕ title_ಉದ್ದಳತೆ;

	/* Convert path for display */
	nice_ಮಾರ್ಗ = ಸ್ಮರಣೆ_ಕೊಡು(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ) * SLEN("&amp;") + 1);
	ಆದರೆ (nice_ಮಾರ್ಗ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Escape special HTML characters */
	ಗಾಗಿ (cnv = nice_ಮಾರ್ಗ, tmp = ಮಾರ್ಗ; *tmp != '\0'; tmp++) {
		ಆದರೆ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ಇಲ್ಲದಿದ್ದರೆ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_ಉದ್ದಳತೆ = (cnv - nice_ಮಾರ್ಗ) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(messages_get("FileIndex"));
	title = ಸ್ಮರಣೆ_ಕೊಡು(title_ಉದ್ದಳತೆ + 1);

	ಆದರೆ (title == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(nice_ಮಾರ್ಗ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_ಉದ್ದಳತೆ, messages_get("FileIndex"), nice_ಮಾರ್ಗ);

	ಬಿಡುಗಡೆ(nice_ಮಾರ್ಗ);

	ಹಿಂತಿರುಗಿ title;
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
ಸ್ಥಿರ nserror
process_dir_ent(ರಚನೆ fetch_file_context *ctx,
		 ರಚನೆ dirent *ent,
		 bool even,
		 ಅಕ್ಷರ *ಬಫರ್,
		 size_t ಬಫರ್_len)
{
	nserror ret;
	ಅಕ್ಷರ *urlpath = ಖಾಲಿ; /* buffer for leaf entry path */
	ರಚನೆ stat ent_stat; /* stat result of leaf entry */
	ಅಕ್ಷರ datebuf[64]; /* buffer for date text */
	ಅಕ್ಷರ timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ಆದರೆ (ent->d_name[0] == '.') {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ಖಾಲಿ, 2, ctx->ಮಾರ್ಗ, ent->d_name);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ಆದರೆ (strftime((ಅಕ್ಷರ *)&datebuf, ಗಾತ್ರ datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ಆದರೆ (strftime((ಅಕ್ಷರ *)&timebuf, ಗಾತ್ರ timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->ಮಾರ್ಗ_ಗಮ್ಯ_nsurl(urlpath, &url);
	ಆದರೆ (ret != NSERROR_OK) {
		ಬಿಡುಗಡೆ(urlpath);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_ಅಡ್ಡಸಾಲು(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     ಬಫರ್, ಬಫರ್_len);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_ಅಡ್ಡಸಾಲು(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     ಬಫರ್, ಬಫರ್_len);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* something else */
		dirlist_generate_ಅಡ್ಡಸಾಲು(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     ಬಫರ್, ಬಫರ್_len);
	}

	nsurl_unref(url);
	ಬಿಡುಗಡೆ(urlpath);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dir_ವಿಂಗಡಿಸು_alpha(ಸ್ಥಿರಾಂಕ ರಚನೆ dirent **d1, ಸ್ಥಿರಾಂಕ ರಚನೆ dirent **d2)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = (*d1)->d_name;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2 = (*d2)->d_name;

	ಆಗ (*s1 != '\0' && *s2 != '\0') {
		ಆದರೆ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			ಪೂರ್ಣಾಂಕ n1 = 0,  n2 = 0;
			ಆಗ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ಆಗ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ಆದರೆ (n1 != n2) {
				ಹಿಂತಿರುಗಿ n1 - n2;
			}
			ಆದರೆ (*s1 == '\0' || *s2 == '\0')
				ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (tolower(*s1) != tolower(*s2))
			ನಿಲ್ಲಿಸು;

		s1++;
		s2++;
	}

	ಹಿಂತಿರುಗಿ tolower(*s1) - tolower(*s2);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process_dir(ರಚನೆ fetch_file_context *ctx,
				   ರಚನೆ stat *fdstat)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ ಬಫರ್[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	ಅಕ್ಷರ *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *ಮೇಲೆ; /* url of parent */

	ರಚನೆ dirent **listing = ಖಾಲಿ; /* directory entry listing */
	ಪೂರ್ಣಾಂಕ i; /* directory entry index */
	ಪೂರ್ಣಾಂಕ n; /* number of directory entries */

	n = scandir(ctx->ಮಾರ್ಗ, &listing, 0, dir_ವಿಂಗಡಿಸು_alpha);
	ಆದರೆ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_ಗಮ್ಯ_http_ಸಂಕೇತ(errno));
		ಹಿಂತಿರುಗಿ;
	}

	/* fetch is going to be successful */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 200);

	/* force no-cache */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Cache-Control: no-cache"))
		ಹೋಗು fetch_file_process_dir_aborted;

	/* content type */
	ಆದರೆ (fetch_file_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html; charset=utf-8"))
		ಹೋಗು fetch_file_process_dir_aborted;

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಬಫರ್;

	/* directory listing top */
	dirlist_generate_ತುದಿ(ಬಫರ್, ಗಾತ್ರ ಬಫರ್);
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->ಮಾರ್ಗ);
	dirlist_generate_title(title, ಬಫರ್, ಗಾತ್ರ ಬಫರ್);
	ಬಿಡುಗಡೆ(title);
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_ಪೋಷಕ(ctx->url, &ಮೇಲೆ);
	ಆದರೆ (err == NSERROR_OK) {
		ಆದರೆ (nsurl_compare(ctx->url, ಮೇಲೆ, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_ಪೋಷಕ_link(nsurl_access(ಮೇಲೆ),
					ಬಫರ್, ಗಾತ್ರ ಬಫರ್);

			ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
			fetch_file_send_callback(&ಸಂದೇಶ, ctx);
		}
		nsurl_unref(ಮೇಲೆ);

		ಆದರೆ (ctx->aborted)
			ಹೋಗು fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(ಬಫರ್, ಗಾತ್ರ ಬಫರ್);
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_file_process_dir_aborted;

	ಗಾಗಿ (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, ಬಫರ್,
				       ಗಾತ್ರ(ಬಫರ್));

		ಆದರೆ (err == NSERROR_OK) {
			ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
			ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
				ಹೋಗು fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(ಬಫರ್, ಗಾತ್ರ ಬಫರ್);
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_file_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_file_process_dir_aborted;

	ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
	fetch_file_send_callback(&ಸಂದೇಶ, ctx);

fetch_file_process_dir_aborted:

	ಆದರೆ (listing != ಖಾಲಿ) {
		ಗಾಗಿ (i = 0; i < n; i++) {
			ಬಿಡುಗಡೆ(listing[i]);
		}
		ಬಿಡುಗಡೆ(listing);
	}
}


/* process a file fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_process(ರಚನೆ fetch_file_context *ctx)
{
	ರಚನೆ stat fdstat; /**< The objects stat */

	ಆದರೆ (stat(ctx->ಮಾರ್ಗ, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ಗಮ್ಯ_http_ಸಂಕೇತ(errno));
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ಹಿಂತಿರುಗಿ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ಹಿಂತಿರುಗಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ಹಿಂತಿರುಗಿ;
}

/** callback to poll for additional file fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_file_poll(lwc_string *scheme)
{
	ರಚನೆ fetch_file_context *c, *save_ring = ಖಾಲಿ;

	ಆಗ (ring != ಖಾಲಿ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಆದರೆ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಮುಂದುವರಿ;
		}

		/* Only process non-aborted fetches */
		ಆದರೆ (c->aborted == false) {
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

nserror fetch_file_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_file_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_file_ಆರಂಭ,
		.abort = fetch_file_abort,
		.ಬಿಡುಗಡೆ = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops);
}
