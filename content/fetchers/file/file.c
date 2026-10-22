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

#समावेश "utils/config.h"

#समावेश <stdlib.h>
#समावेश <ctype.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <fcntl.h>
#समावेश <unistd.h>
#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <time.h>
#समावेश <stdio.h>
#समावेश <stdarg.h>
#अगर_घोषित HAVE_MMAP
#समावेश <sys/mman.h>
#पूर्ण_अगर
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/dirent.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/time.h"
#समावेश "utils/ring.h"
#समावेश "utils/file.h"
#समावेश "netsurf/fetch.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"
#समावेश "dirlist.h"
#समावेश "file.h"

/* Maximum size of read buffer */
#घोषणा FETCH_FILE_MAX_BUF_आकार (1024 * 1024)

/** Context for a fetch */
काष्ठा fetch_file_context {
	काष्ठा fetch_file_context *r_अग्र, *r_पूर्व;

	काष्ठा fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	अक्षर *path; /**< The actual path to be used with open() */

	समय_प्रकार file_etag; /**< Request etag for file (previous st.m_time) */
};

अटल काष्ठा fetch_file_context *ring = शून्य;

/** issue fetch callbacks with locking */
अटल अंतरभूत bool fetch_file_send_callback(स्थिर fetch_संदेश *संदेश,
		काष्ठा fetch_file_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(संदेश, ctx->fetchh);
	ctx->locked = false;

	वापस ctx->aborted;
}

अटल bool fetch_file_send_header(काष्ठा fetch_file_context *ctx,
		स्थिर अक्षर *fmt, ...)
{
	fetch_संदेश संदेश;
	अक्षर header[64];
	बहु_सूची ap;
	पूर्णांक len;

	बहु_शुरू(ap, fmt);
	len = vsnprintf(header, माप header, fmt, ap);
	बहु_पूर्ण(ap);

	अगर (len >= (पूर्णांक)माप(header) || len < 0) {
		वापस false;
	}

	संदेश.type = FETCH_HEADER;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) header;
	संदेश.तथ्य.header_or_तथ्य.len = len;

	वापस fetch_file_send_callback(&संदेश, ctx);
}

/** callback to initialise the file fetcher. */
अटल bool fetch_file_initialise(lwc_string *scheme)
{
	वापस true;
}

/** callback to initialise the file fetcher. */
अटल व्योम fetch_file_finalise(lwc_string *scheme)
{
}

अटल bool fetch_file_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}

/** callback to set up a file fetch context. */
अटल व्योम *
fetch_file_setup(काष्ठा fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 स्थिर अक्षर *post_urlenc,
		 स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		 स्थिर अक्षर **headers)
{
	काष्ठा fetch_file_context *ctx;
	पूर्णांक i;
	nserror ret;

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य)
		वापस शून्य;

	ret = guit->file->nsurl_to_path(url, &ctx->path);
	अगर (ret != NSERROR_OK) {
		मुक्त(ctx);
		वापस शून्य;
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	क्रम (i = 0; headers[i] != शून्य; i++) {
		अगर (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			जारी;
		}

		/* If-None-Match: "12345678" */
		स्थिर अक्षर *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		जबतक (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		अगर (*d != '\0') {
			ret = nsc_snptimet(d, म_माप(d), &ctx->file_etag);
			अगर (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_जोड़ो(ring, ctx);

	वापस ctx;
}

/** callback to free a file fetch */
अटल व्योम fetch_file_free(व्योम *ctx)
{
	काष्ठा fetch_file_context *c = ctx;
	nsurl_unref(c->url);
	मुक्त(c->path);
	मुक्त(ctx);
}

/** callback to start a file fetch */
अटल bool fetch_file_start(व्योम *ctx)
{
	वापस true;
}

/** callback to abort a file fetch */
अटल व्योम fetch_file_abort(व्योम *ctx)
{
	काष्ठा fetch_file_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

अटल पूर्णांक fetch_file_errno_to_http_code(पूर्णांक error_no)
{
	चयन (error_no) {
	हाल ENAMETOOLONG:
		वापस 400;
	हाल EACCES:
		वापस 403;
	हाल ENOENT:
		वापस 404;
	शेष:
		अवरोध;
	}

	वापस 500;
}

अटल व्योम fetch_file_process_error(काष्ठा fetch_file_context *ctx, पूर्णांक code)
{
	fetch_संदेश संदेश;
	अक्षर बफर[1024];
	स्थिर अक्षर *title;
	अक्षर कुंजी[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	अगर (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		जाओ fetch_file_process_error_aborted;

	snprintf(कुंजी, माप कुंजी, "HTTP%03d", code);
	title = messages_get(कुंजी);

	snprintf(बफर, माप बफर,
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

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) बफर;
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_file_send_callback(&संदेश, ctx))
		जाओ fetch_file_process_error_aborted;

	संदेश.type = FETCH_FINISHED;
	fetch_file_send_callback(&संदेश, ctx);

fetch_file_process_error_aborted:
	वापस;
}


/** Process object as a regular file */
अटल व्योम fetch_file_process_plain(काष्ठा fetch_file_context *ctx,
				     काष्ठा stat *fdstat)
{
#अगर_घोषित HAVE_MMAP
	fetch_संदेश संदेश;
	अक्षर *buf = शून्य;
	माप_प्रकार buf_आकार;

	पूर्णांक fd; /**< The file descriptor of the object */

	/* Check if we can just return not modified */
	अगर (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		संदेश.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&संदेश, ctx);
		वापस;
	}

	fd = खोलो(ctx->path, O_RDONLY);
	अगर (fd < 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(त्रुटि_सं));
		वापस;
	}

	/* set buffer size */
	buf_आकार = fdstat->st_size;

	/* allocate the buffer storage */
	अगर (buf_आकार > 0) {
		buf = mmap(शून्य, buf_आकार, PROT_READ, MAP_SHARED, fd, 0);
		अगर (buf == MAP_FAILED) {
			संदेश.type = FETCH_ERROR;
			संदेश.तथ्य.error = "Unable to map memory for file data buffer";
			fetch_file_send_callback(&संदेश, ctx);
			बंद(fd);
			वापस;
		}
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	अगर (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		जाओ fetch_file_process_aborted;
	}

	/* content length */
	अगर (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		जाओ fetch_file_process_aborted;
	}

	/* create etag */
	अगर (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				   (int64_t) fdstat->st_mtime)) {
		जाओ fetch_file_process_aborted;
	}

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) buf;
	संदेश.तथ्य.header_or_तथ्य.len = buf_आकार;
	fetch_file_send_callback(&संदेश, ctx);

	अगर (ctx->aborted == false) {
		संदेश.type = FETCH_FINISHED;
		fetch_file_send_callback(&संदेश, ctx);
	}

fetch_file_process_aborted:

	अगर (buf != शून्य)
		munmap(buf, buf_आकार);
	बंद(fd);
#अन्यथा
	fetch_संदेश संदेश;
	अक्षर *buf;
	माप_प्रकार buf_आकार;

	ssize_t tot_read = 0;
	ssize_t res;

	खाता *infile;

	/* Check if we can just return not modified */
	अगर (ctx->file_etag != 0 && ctx->file_etag == fdstat->st_mtime) {
		fetch_set_http_code(ctx->fetchh, 304);
		संदेश.type = FETCH_NOTMODIFIED;
		fetch_file_send_callback(&संदेश, ctx);
		वापस;
	}

	infile = ख_खोलो(ctx->path, "rb");
	अगर (infile == शून्य) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(त्रुटि_सं));
		वापस;
	}

	/* set buffer size */
	buf_आकार = fdstat->st_size;
	अगर (buf_आकार > FETCH_FILE_MAX_BUF_आकार)
		buf_आकार = FETCH_FILE_MAX_BUF_आकार;

	/* allocate the buffer storage */
	buf = दो_स्मृति(buf_आकार);
	अगर (buf == शून्य) {
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error =
			"Unable to allocate memory for file data buffer";
		fetch_file_send_callback(&संदेश, ctx);
		ख_बंद(infile);
		वापस;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	अगर (fetch_file_send_header(ctx, "Content-Type: %s", 
				   guit->fetch->filetype(ctx->path))) {
		जाओ fetch_file_process_aborted;
	}

	/* content length */
	अगर (fetch_file_send_header(ctx, "Content-Length: %" PRIsizet,
				   fdstat->st_size)) {
		जाओ fetch_file_process_aborted;
	}

	/* create etag */
	अगर (fetch_file_send_header(ctx, "ETag: \"%10" PRId64 "\"", 
				   (int64_t) fdstat->st_mtime)) {
		जाओ fetch_file_process_aborted;
	}

	/* main data loop */
	जबतक (tot_read < fdstat->st_size) {
		res = ख_पढ़ो(buf, 1, buf_आकार, infile);
		अगर (res == 0) {
			अगर (ख_पूर्ण(infile)) {
				संदेश.type = FETCH_ERROR;
				संदेश.तथ्य.error = "Unexpected EOF reading file";
				fetch_file_send_callback(&संदेश, ctx);
				जाओ fetch_file_process_aborted;
			} अन्यथा {
				संदेश.type = FETCH_ERROR;
				संदेश.तथ्य.error = "Error reading file";
				fetch_file_send_callback(&संदेश, ctx);
				जाओ fetch_file_process_aborted;
			}
		}
		tot_read += res;

		संदेश.type = FETCH_तथ्य;
		संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) buf;
		संदेश.तथ्य.header_or_तथ्य.len = res;
		अगर (fetch_file_send_callback(&संदेश, ctx))
			अवरोध;
	}

	अगर (ctx->aborted == false) {
		संदेश.type = FETCH_FINISHED;
		fetch_file_send_callback(&संदेश, ctx);
	}

fetch_file_process_aborted:

	ख_बंद(infile);
	मुक्त(buf);
#पूर्ण_अगर
	वापस;
}

अटल अक्षर *gen_nice_title(अक्षर *path)
{
	अक्षर *nice_path, *cnv, *tmp;
	अक्षर *title;
	पूर्णांक title_लंबाई;

	/* Convert path for display */
	nice_path = दो_स्मृति(म_माप(path) * SLEN("&amp;") + 1);
	अगर (nice_path == शून्य) {
		वापस शून्य;
	}

	/* Escape special HTML characters */
	क्रम (cnv = nice_path, tmp = path; *tmp != '\0'; tmp++) {
		अगर (*tmp == '<') {
			*cnv++ = '&';
			*cnv++ = 'l';
			*cnv++ = 't';
			*cnv++ = ';';
		} अन्यथा अगर (*tmp == '>') {
			*cnv++ = '&';
			*cnv++ = 'g';
			*cnv++ = 't';
			*cnv++ = ';';
		} अन्यथा अगर (*tmp == '&') {
			*cnv++ = '&';
			*cnv++ = 'a';
			*cnv++ = 'm';
			*cnv++ = 'p';
			*cnv++ = ';';
		} अन्यथा {
			*cnv++ = *tmp;
		}
	}
	*cnv = '\0';

	/* Construct a localised title string */
	title_लंबाई = (cnv - nice_path) + म_माप(messages_get("FileIndex"));
	title = दो_स्मृति(title_लंबाई + 1);

	अगर (title == शून्य) {
		मुक्त(nice_path);
		वापस शून्य;
	}

	/* Set title to localised "Index of <nice_path>" */
	snprintf(title, title_लंबाई, messages_get("FileIndex"), nice_path);

	मुक्त(nice_path);

	वापस title;
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
अटल nserror
process_dir_ent(काष्ठा fetch_file_context *ctx,
		 काष्ठा dirent *ent,
		 bool even,
		 अक्षर *बफर,
		 माप_प्रकार बफर_len)
{
	nserror ret;
	अक्षर *urlpath = शून्य; /* buffer for leaf entry path */
	काष्ठा stat ent_stat; /* stat result of leaf entry */
	अक्षर datebuf[64]; /* buffer for date text */
	अक्षर timebuf[64]; /* buffer for time text */
	nsurl *url;

	/* skip hidden files */
	अगर (ent->d_name[0] == '.') {
		वापस NSERROR_BAD_PARAMETER;
	}

	ret = netsurf_mkpath(&urlpath, शून्य, 2, ctx->path, ent->d_name);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	अगर (stat(urlpath, &ent_stat) != 0) {
		ent_stat.st_mode = 0;
		datebuf[0] = 0;
		timebuf[0] = 0;
	} अन्यथा {
		/* Get date in output format. a (day of week) and b
		 * (month) are both affected by the locale
		 */
		अगर (स_माला((अक्षर *)&datebuf, माप datebuf, "%a %d %b %Y",
			     स_स्थानीय(&ent_stat.st_mtime)) == 0) {
			datebuf[0] = '-';
			datebuf[1] = 0;
		}

		/* Get time in output format */
		अगर (स_माला((अक्षर *)&timebuf, माप timebuf, "%H:%M",
			     स_स्थानीय(&ent_stat.st_mtime)) == 0) {
			timebuf[0] = '-';
			timebuf[1] = 0;
		}
	}

	ret = guit->file->path_to_nsurl(urlpath, &url);
	अगर (ret != NSERROR_OK) {
		मुक्त(urlpath);
		वापस ret;
	}

	अगर (S_ISREG(ent_stat.st_mode)) {
		/* regular file */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     guit->fetch->filetype(urlpath),
				     ent_stat.st_size,
				     datebuf, timebuf,
				     बफर, बफर_len);
	} अन्यथा अगर (S_ISDIR(ent_stat.st_mode)) {
		/* directory */
		dirlist_generate_row(even,
				     true,
				     url,
				     ent->d_name,
				     messages_get("FileDirectory"),
				     -1,
				     datebuf, timebuf,
				     बफर, बफर_len);
	} अन्यथा {
		/* something else */
		dirlist_generate_row(even,
				     false,
				     url,
				     ent->d_name,
				     "",
				     -1,
				     datebuf, timebuf,
				     बफर, बफर_len);
	}

	nsurl_unref(url);
	मुक्त(urlpath);

	वापस NSERROR_OK;
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
अटल पूर्णांक dir_sort_alpha(स्थिर काष्ठा dirent **d1, स्थिर काष्ठा dirent **d2)
{
	स्थिर अक्षर *s1 = (*d1)->d_name;
	स्थिर अक्षर *s2 = (*d2)->d_name;

	जबतक (*s1 != '\0' && *s2 != '\0') {
		अगर ((*s1 >= '0' && *s1 <= '9') &&
				(*s2 >= '0' && *s2 <= '9')) {
			पूर्णांक n1 = 0,  n2 = 0;
			जबतक (*s1 >= '0' && *s1 <= '9') {
				n1 = n1 * 10 + (*s1) - '0';
				s1++;
			}
			जबतक (*s2 >= '0' && *s2 <= '9') {
				n2 = n2 * 10 + (*s2) - '0';
				s2++;
			}
			अगर (n1 != n2) {
				वापस n1 - n2;
			}
			अगर (*s1 == '\0' || *s2 == '\0')
				अवरोध;
		}
		अगर (छोटे(*s1) != छोटे(*s2))
			अवरोध;

		s1++;
		s2++;
	}

	वापस छोटे(*s1) - छोटे(*s2);
}

अटल व्योम fetch_file_process_dir(काष्ठा fetch_file_context *ctx,
				   काष्ठा stat *fdstat)
{
	fetch_संदेश संदेश;
	अक्षर बफर[1024]; /* Output buffer */
	bool even = false; /* formatting flag */
	अक्षर *title; /* pretty printed title */
	nserror err; /* result from url routines */
	nsurl *up; /* url of parent */

	काष्ठा dirent **listing = शून्य; /* directory entry listing */
	पूर्णांक i; /* directory entry index */
	पूर्णांक n; /* number of directory entries */

	n = scandir(ctx->path, &listing, 0, dir_sort_alpha);
	अगर (n < 0) {
		fetch_file_process_error(ctx,
			fetch_file_errno_to_http_code(त्रुटि_सं));
		वापस;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* force no-cache */
	अगर (fetch_file_send_header(ctx, "Cache-Control: no-cache"))
		जाओ fetch_file_process_dir_aborted;

	/* content type */
	अगर (fetch_file_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		जाओ fetch_file_process_dir_aborted;

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) बफर;

	/* directory listing top */
	dirlist_generate_शीर्ष(बफर, माप बफर);
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_file_send_callback(&संदेश, ctx))
		जाओ fetch_file_process_dir_aborted;

	/* directory listing title */
	title = gen_nice_title(ctx->path);
	dirlist_generate_title(title, बफर, माप बफर);
	मुक्त(title);
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_file_send_callback(&संदेश, ctx))
		जाओ fetch_file_process_dir_aborted;

	/* Print parent directory link */
	err = nsurl_parent(ctx->url, &up);
	अगर (err == NSERROR_OK) {
		अगर (nsurl_compare(ctx->url, up, NSURL_COMPLETE) == false) {
			/* different URL; have parent */
			dirlist_generate_parent_link(nsurl_access(up),
					बफर, माप बफर);

			संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
			fetch_file_send_callback(&संदेश, ctx);
		}
		nsurl_unref(up);

		अगर (ctx->aborted)
			जाओ fetch_file_process_dir_aborted;

	}

	/* directory list headings */
	dirlist_generate_headings(बफर, माप बफर);
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_file_send_callback(&संदेश, ctx))
		जाओ fetch_file_process_dir_aborted;

	क्रम (i = 0; i < n; i++) {

		err = process_dir_ent(ctx, listing[i], even, बफर,
				       माप(बफर));

		अगर (err == NSERROR_OK) {
			संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
			अगर (fetch_file_send_callback(&संदेश, ctx))
				जाओ fetch_file_process_dir_aborted;

			even = !even;
		}
	}

	/* directory listing bottom */
	dirlist_generate_bottom(बफर, माप बफर);
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_file_send_callback(&संदेश, ctx))
		जाओ fetch_file_process_dir_aborted;

	संदेश.type = FETCH_FINISHED;
	fetch_file_send_callback(&संदेश, ctx);

fetch_file_process_dir_aborted:

	अगर (listing != शून्य) {
		क्रम (i = 0; i < n; i++) {
			मुक्त(listing[i]);
		}
		मुक्त(listing);
	}
}


/* process a file fetch */
अटल व्योम fetch_file_process(काष्ठा fetch_file_context *ctx)
{
	काष्ठा stat fdstat; /**< The objects stat */

	अगर (stat(ctx->path, &fdstat) != 0) {
		/* process errors as appropriate */
		fetch_file_process_error(ctx,
				fetch_file_errno_to_http_code(त्रुटि_सं));
		वापस;
	}

	अगर (S_ISDIR(fdstat.st_mode)) {
		/* directory listing */
		fetch_file_process_dir(ctx, &fdstat);
		वापस;
	} अन्यथा अगर (S_ISREG(fdstat.st_mode)) {
		/* regular file */
		fetch_file_process_plain(ctx, &fdstat);
		वापस;
	} अन्यथा {
		/* unhandled type of file */
		fetch_file_process_error(ctx, 501);
	}

	वापस;
}

/** callback to poll for additional file fetch contents */
अटल व्योम fetch_file_poll(lwc_string *scheme)
{
	काष्ठा fetch_file_context *c, *save_ring = शून्य;

	जबतक (ring != शून्य) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		अगर (c->locked == true) {
			RING_जोड़ो(save_ring, c);
			जारी;
		}

		/* Only process non-aborted fetches */
		अगर (c->aborted == false) {
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

nserror fetch_file_register(व्योम)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_file);
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
		.initialise = fetch_file_initialise,
		.acceptable = fetch_file_can_fetch,
		.setup = fetch_file_setup,
		.start = fetch_file_start,
		.पात = fetch_file_abort,
		.मुक्त = fetch_file_free,
		.poll = fetch_file_poll,
		.finalise = fetch_file_finalise
	};

	वापस fetcher_add(scheme, &fetcher_ops);
}
