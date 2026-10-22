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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MMAP
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/dirent.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/time.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/file.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dirlist.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "file.h"

/* Maximum size of read buffer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FETCH_FILE_MAX_BUF_ਮਾਪ (1024 * 1024)

/** Context for a fetch */
ਬਣਤਰ fetch_file_context {
	ਬਣਤਰ fetch_file_context *r_ਅਗਲਾ, *r_prev;

	ਬਣਤਰ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	ਅੱਖਰ *ਰਸਤਾ; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

ਸਥਿਰ ਬਣਤਰ fetch_file_context *ring = ਨਿਰਾ;

/** issue fetch callbacks with locking */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool fetch_file_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ,
		ਬਣਤਰ fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, ctx->fetchh);
	ctx->locked = false;

	ਵਾਪਸ ctx->aborted;
}

ਸਥਿਰ bool fetch_file_send_ਸਿਰਲੇਖ(ਬਣਤਰ fetch_file_context *ctx,
		ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ ਸਿਰਲੇਖ[64];
	va_list ap;
	ਪੂਰਨ len;

	va_start(ap, fmt);
	len = vsnprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ, fmt, ap);
	va_end(ap);

	ਜੇ (len >= (ਪੂਰਨ)ਆਕਾਰ(ਸਿਰਲੇਖ) || len < 0) {
		ਵਾਪਸ false;
	}

	ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = len;

	ਵਾਪਸ fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
}

/** callback to initialise the file fetcher. */
ਸਥਿਰ bool fetch_file_initialise(lwc_string *scheme)
{
	ਵਾਪਸ true;
}

/** callback to initialise the file fetcher. */
ਸਥਿਰ ਖਾਲੀ fetch_file_finalise(lwc_string *scheme)
{
}

ਸਥਿਰ bool fetch_file_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}

/** callback to set up a file fetch context. */
ਸਥਿਰ ਖਾਲੀ *
fetch_file_ਤਿਆਰੀ(ਬਣਤਰ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ਅਚਲ ਅੱਖਰ *post_urlenc,
		 ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		 ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ fetch_file_context *ctx;
	ਪੂਰਨ i;
	nserror ret;

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ret = guit->file->nsurl_ਮੰਜ਼ਿਲ_ਰਸਤਾ(url, &ctx->ਰਸਤਾ);
	ਜੇ (ret != NSERROR_OK) {
		ਮੁਕਤ(ctx);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ਲਈ (i = 0; ਸਿਰਲੇਖਾਂ[i] != ਨਿਰਾ; i++) {
		ਜੇ (strncasecmp(ਸਿਰਲੇਖਾਂ[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ਜਾਰੀ;
		}

		/* If-None-Match: "12345678" */
		ਅਚਲ ਅੱਖਰ *d = ਸਿਰਲੇਖਾਂ[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ਜਦੋਂ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ਜੇ (*d != '\0') {
			ret = nsc_snptimet(d, ਲੜੀ_ਲੰਬਾਈ(d), &ctx->file_etag);
			ਜੇ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ਪਾਓ(ring, ctx);

	ਵਾਪਸ ctx;
}

/** callback to free a file fetch */
ਸਥਿਰ ਖਾਲੀ fetch_file_free(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	ਮੁਕਤ(c->ਰਸਤਾ);
	ਮੁਕਤ(ctx);
}

/** callback to start a file fetch */
ਸਥਿਰ bool fetch_file_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}

/** callback to abort a file fetch */
ਸਥਿਰ ਖਾਲੀ fetch_file_abort(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ਸਥਿਰ ਪੂਰਨ fetch_file_errno_ਮੰਜ਼ਿਲ_http_ਕੋਡ(ਪੂਰਨ error_no)
{
	ਚੋਣ (error_no) {
	ਹਾਲਤ ENAMETOOLONG:
		ਵਾਪਸ 400;
	ਹਾਲਤ EACCES:
		ਵਾਪਸ 403;
	ਹਾਲਤ ENOENT:
		ਵਾਪਸ 404;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ 500;
}

ਸਥਿਰ ਖਾਲੀ fetch_file_process_error(ਬਣਤਰ fetch_file_context *ctx, ਪੂਰਨ ਕੋਡ)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ ਬਫਰ[1024];
	ਅਚਲ ਅੱਖਰ *title;
	ਅੱਖਰ ਕੁੰਜੀ[8];

	/* content is going to return error code */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, ਕੋਡ);

	/* content type */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html; charset=utf-8"))
		ਜਾਓ fetch_file_process_error_aborted;

	snprintf(ਕੁੰਜੀ, ਆਕਾਰ ਕੁੰਜੀ, "HTTP%03d", ਕੋਡ);
	title = messages_get(ਕੁੰਜੀ);

	snprintf(ਬਫਰ, ਆਕਾਰ ਬਫਰ,
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
		 messages_get("FetchErrorCode"), ਕੋਡ,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਬਫਰ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_file_process_error_aborted;

	ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
	fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);

fetch_file_process_error_aborted:
	ਵਾਪਸ;
}


/** Process object as a regular file */
ਸਥਿਰ ਖਾਲੀ fetch_file_process_plain(ਬਣਤਰ fetch_file_context *ctx,
				     ਬਣਤਰ stat *fdstat)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MMAP
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ *buf = ਨਿਰਾ;
	size_t buf_ਮਾਪ;

	ਪੂਰਨ fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	ਜੇ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 304);
		ਸੁਨੇਹਾ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
		ਵਾਪਸ;
	}

	fd = open(ctx->ਰਸਤਾ, O_RDONLY);
	ਜੇ (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ਮੰਜ਼ਿਲ_http_ਕੋਡ(errno));
		ਵਾਪਸ;
	}

	/* set buffer size */
	buf_ਮਾਪ = fdstat->st_size;

	/* allocate the buffer storage */
	ਜੇ (buf_ਮਾਪ > 0) {
		buf = mmap(ਨਿਰਾ, buf_ਮਾਪ, PROT_READ, MAP_SHARED, fd, 0);
		ਜੇ (buf == MAP_FAILED) {
			ਸੁਨੇਹਾ.type = FETCH_ERROR;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
			close(fd);
			ਵਾਪਸ;
		}
	}

	/* fetch is going to be successful */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->ਰਸਤਾ))) {
		ਜਾਓ fetch_file_process_aborted;
	}

	/* content length */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ਜਾਓ fetch_file_process_aborted;
	}

	/* create etag */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		ਜਾਓ fetch_file_process_aborted;
	}

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) buf;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = buf_ਮਾਪ;
	fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);

	ਜੇ (ctx->aborted == false) {
		ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
		fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
	}

fetch_file_process_aborted:

	ਜੇ (buf != ਨਿਰਾ)
		munmap(buf, buf_ਮਾਪ);
	close(fd);
#ਨਹੀਂਤਾਂ
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ *buf;
	size_t buf_ਮਾਪ;

	ssize_t tot_read = 0;
	ssize_t res;

	FILE *infile;

	/* Check if we can just return not modified */
	ਜੇ (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 304);
		ਸੁਨੇਹਾ.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
		ਵਾਪਸ;
	}

	infile = fopen(ctx->ਰਸਤਾ, "rb");
	ਜੇ (infile == ਨਿਰਾ) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ਮੰਜ਼ਿਲ_http_ਕੋਡ(errno));
		ਵਾਪਸ;
	}

	/* set buffer size */
	buf_ਮਾਪ = fdstat->st_size;
	ਜੇ (buf_ਮਾਪ > FETCH_FILE_MAX_BUF_ਮਾਪ)
		buf_ਮਾਪ = FETCH_FILE_MAX_BUF_ਮਾਪ;

	/* allocate the buffer storage */
	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(buf_ਮਾਪ);
	ਜੇ (buf == ਨਿਰਾ) {
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
		fclose(infile);
		ਵਾਪਸ;
	}

	/* fetch is going to be successful */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->ਰਸਤਾ))) {
		ਜਾਓ fetch_file_process_aborted;
	}

	/* content length */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		ਜਾਓ fetch_file_process_aborted;
	}

	/* create etag */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		ਜਾਓ fetch_file_process_aborted;
	}

	/* main data loop */
	ਜਦੋਂ (tot_read < fdstat->st_size) {
		res = fread(buf, 1, buf_ਮਾਪ, infile);
		ਜੇ (res == 0) {
			ਜੇ (feof(infile)) {
				ਸੁਨੇਹਾ.type = FETCH_ERROR;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
				ਜਾਓ fetch_file_process_aborted;
			} ਨਹੀਂਤਾਂ {
				ਸੁਨੇਹਾ.type = FETCH_ERROR;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Error reading file";
				fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
				ਜਾਓ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) buf;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = res;
		ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
			ਰੋਕੋ;
	}

	ਜੇ (ctx->aborted == false) {
		ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
		fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
	}

fetch_file_process_aborted:

	fclose(infile);
	ਮੁਕਤ(buf);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ;
}

ਸਥਿਰ ਅੱਖਰ *gen_nice_title(ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *nice_ਰਸਤਾ, *cnv, *tmp;
	ਅੱਖਰ *title;
	ਪੂਰਨ title_ਲੰਮਾਈ;

	/* Convert path for display */
	nice_ਰਸਤਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ) * SLEN("&amp;") + 1);
	ਜੇ (nice_ਰਸਤਾ == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Escape special HTML characters */
	ਲਈ (cnv = nice_ਰਸਤਾ, tmp = ਰਸਤਾ; *tmp != '\0'; tmp++) {
		ਜੇ (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} ਨਹੀਂਤਾਂ ਜੇ (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} ਨਹੀਂਤਾਂ ਜੇ (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} ਨਹੀਂਤਾਂ {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_ਲੰਮਾਈ = (cnv - nice_ਰਸਤਾ) + ਲੜੀ_ਲੰਬਾਈ(messages_get("FileIndex"));
	title = ਸਮ੍ਰਿਤੀ_ਦਿਓ(title_ਲੰਮਾਈ + 1);

	ਜੇ (title == ਨਿਰਾ) {
		ਮੁਕਤ(nice_ਰਸਤਾ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_ਲੰਮਾਈ, messages_get("FileIndex"), nice_ਰਸਤਾ);

	ਮੁਕਤ(nice_ਰਸਤਾ);

	ਵਾਪਸ title;
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
ਸਥਿਰ nserror
process_dir_ent(ਬਣਤਰ fetch_file_context *ctx,
		 ਬਣਤਰ dirent *ent,
		 bool even,
		 ਅੱਖਰ *ਬਫਰ,
		 size_t ਬਫਰ_len)
{
	nserror ret;
	ਅੱਖਰ *urlpath = ਨਿਰਾ; /* buffer for leaf entry path */
	ਬਣਤਰ stat ent_stat; /* stat result of leaf entry */
	ਅੱਖਰ datebuf[64]; /* buffer for date text */
	ਅੱਖਰ timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	ਜੇ (ent->d_name[0] == '.') {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, ਨਿਰਾ, 2, ctx->ਰਸਤਾ, ent->d_name);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ਜੇ (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} ਨਹੀਂਤਾਂ {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		ਜੇ (strftime((ਅੱਖਰ *)&datebuf, ਆਕਾਰ datebuf, "%a %d %b %Y",
			     localtime(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		ਜੇ (strftime((ਅੱਖਰ *)&timebuf, ਆਕਾਰ timebuf, "%H:%M",
			     localtime(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->ਰਸਤਾ_ਮੰਜ਼ਿਲ_nsurl(urlpath, &url);
	ਜੇ (ret != NSERROR_OK) {
		ਮੁਕਤ(urlpath);
		ਵਾਪਸ ret;
	}

	ਜੇ (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_ਪੰਕਤੀ(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     ਬਫਰ, ਬਫਰ_len);
	} ਨਹੀਂਤਾਂ ਜੇ (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_ਪੰਕਤੀ(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     ਬਫਰ, ਬਫਰ_len);
	} ਨਹੀਂਤਾਂ {
		/* something else */
		dirlist_generate_ਪੰਕਤੀ(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     ਬਫਰ, ਬਫਰ_len);
	}

	nsurl_unref(url);
	ਮੁਕਤ(urlpath);

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ ਪੂਰਨ dir_ਛਾਂਟੋ_alpha(ਅਚਲ ਬਣਤਰ dirent **d1, ਅਚਲ ਬਣਤਰ dirent **d2)
{
	ਅਚਲ ਅੱਖਰ *s1 = (*d1)->d_name;
	ਅਚਲ ਅੱਖਰ *s2 = (*d2)->d_name;

	ਜਦੋਂ (*s1 != '\0' && *s2 != '\0') {
		ਜੇ ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			ਪੂਰਨ n1 = 0,  n2 = 0;
			ਜਦੋਂ (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			ਜਦੋਂ (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			ਜੇ (n1 != n2) {
				ਵਾਪਸ n1 - n2;
			}
			ਜੇ (*s1 == '\0' || *s2 == '\0')
				ਰੋਕੋ;
		}
		ਜੇ (tolower(*s1) != tolower(*s2))
			ਰੋਕੋ;

		s1++;
		s2++;
	}

	ਵਾਪਸ tolower(*s1) - tolower(*s2);
}

ਸਥਿਰ ਖਾਲੀ fetch_file_process_dir(ਬਣਤਰ fetch_file_context *ctx,
				   ਬਣਤਰ stat *fdstat)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ ਬਫਰ[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	ਅੱਖਰ *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *ਉੱਪਰ; /* url of parent */

	ਬਣਤਰ dirent **listing = ਨਿਰਾ; /* directory entry listing */
	ਪੂਰਨ i; /* directory entry index */
	ਪੂਰਨ n; /* number of directory entries */

	n = scandir(ctx->ਰਸਤਾ, &listing, 0, dir_ਛਾਂਟੋ_alpha);
	ਜੇ (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_ਮੰਜ਼ਿਲ_http_ਕੋਡ(errno));
		ਵਾਪਸ;
	}

	/* fetch is going to be successful */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 200);

	/* force no-cache */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Cache-Control: no-cache"))
		ਜਾਓ fetch_file_process_dir_aborted;

	/* content type */
	ਜੇ (fetch_file_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html; charset=utf-8"))
		ਜਾਓ fetch_file_process_dir_aborted;

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਬਫਰ;

	/* directory listing top */
	dirlist_generate_ਸਿਖਰ(ਬਫਰ, ਆਕਾਰ ਬਫਰ);
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->ਰਸਤਾ);
	dirlist_generate_title(title, ਬਫਰ, ਆਕਾਰ ਬਫਰ);
	ਮੁਕਤ(title);
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_ਮਾਪਾ(ctx->url, &ਉੱਪਰ);
	ਜੇ (err == NSERROR_OK) {
		ਜੇ (nsurl_compare(ctx->url, ਉੱਪਰ, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_ਮਾਪਾ_link(nsurl_access(ਉੱਪਰ),
					ਬਫਰ, ਆਕਾਰ ਬਫਰ);

			ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
			fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);
		}
		nsurl_unref(ਉੱਪਰ);

		ਜੇ (ctx->aborted)
			ਜਾਓ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(ਬਫਰ, ਆਕਾਰ ਬਫਰ);
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_file_process_dir_aborted;

	ਲਈ (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, ਬਫਰ,
				       ਆਕਾਰ(ਬਫਰ));

		ਜੇ (err == NSERROR_OK) {
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
			ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
				ਜਾਓ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(ਬਫਰ, ਆਕਾਰ ਬਫਰ);
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_file_process_dir_aborted;

	ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
	fetch_file_send_callback(&ਸੁਨੇਹਾ, ctx);

fetch_file_process_dir_aborted:

	ਜੇ (listing != ਨਿਰਾ) {
		ਲਈ (i = 0; i < n; i++) {
			ਮੁਕਤ(listing[i]);
		}
		ਮੁਕਤ(listing);
	}
}


/* process a file fetch */
ਸਥਿਰ ਖਾਲੀ fetch_file_process(ਬਣਤਰ fetch_file_context *ctx)
{
	ਬਣਤਰ stat fdstat; /**< The objects stat */

	ਜੇ (stat(ctx->ਰਸਤਾ, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_ਮੰਜ਼ਿਲ_http_ਕੋਡ(errno));
		ਵਾਪਸ;
	}

	ਜੇ (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		ਵਾਪਸ;
	} ਨਹੀਂਤਾਂ ਜੇ (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		ਵਾਪਸ;
	} ਨਹੀਂਤਾਂ {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	ਵਾਪਸ;
}

/** callback to poll for additional file fetch contents */
ਸਥਿਰ ਖਾਲੀ fetch_file_poll(lwc_string *scheme)
{
	ਬਣਤਰ fetch_file_context *c, *save_ring = ਨਿਰਾ;

	ਜਦੋਂ (ring != ਨਿਰਾ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ਜੇ (c->locked == true) {
			RING_ਪਾਓ(save_ring, c);
			ਜਾਰੀ;
		}

		/* Only process non-aborted fetches */
		ਜੇ (c->aborted == false) {
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

nserror fetch_file_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.ਤਿਆਰੀ = fetch_file_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_file_ਸ਼ੁਰੂ,
		.abort = fetch_file_abort,
		.ਮੁਕਤ = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(scheme, &fetcher_ops);
}
