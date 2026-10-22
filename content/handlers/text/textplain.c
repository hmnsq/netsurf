/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2006 Adrian Lees <adrianl@users.sourceforge.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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
 * plain text content handling implementation.
 */

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <parserutils/input/inputstream.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/utf8.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/keypress.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/layout.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"
#ଅନ୍ତର୍ଭାଵ "content/handlers/css/utils.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "text/textplain.h"

ସଂରଚନା textplain_ଧାଡ଼ି {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା textplain_content {
	ସଂରଚନା content base;

	lwc_string *encoding;
	ଶୂନ୍ୟ *inputstream;
	ଅକ୍ଷର *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_ଧାଡ଼ି_ଗଣନା;
	ସଂରଚନା textplain_ଧାଡ଼ି *physical_ଧାଡ଼ି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା formatted_width;
	ସଂରଚନା browser_window *bw;

	ସଂରଚନା selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ସ୍ଥିର plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା textplain_tab_width = 256;  /* try for a sensible default */

ସ୍ଥିର lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ସ୍ଥିର ଶୂନ୍ୟ textplain_fini(ଶୂନ୍ୟ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ଖାଲି;
}


/**
 * Work around feature in libparserutils
 *
 * if the client provides an encoding up front, but does not provide a
 * charset detection callback, then libparserutils will replace the
 * provided encoding with UTF-8. This breaks our input handling.
 *
 * Avoid this by providing a callback that does precisely nothing,
 * thus preserving whatever charset information we decided on in
 * textplain_create.
 */
ସ୍ଥିର parserutils_error
textplain_charset_hack(ଅଚଳ uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	ଫେରନ୍ତୁ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ସ୍ଥିର nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	ଅକ୍ଷର *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	ଯଦି (utf8_data == ଖାଲି)
		ଯାଅ no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	ଯଦି (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	ଯଦି (error != PARSERUTILS_OK) {
		ମୁକ୍ତ(utf8_data);
		ଯାଅ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_ଧାଡ଼ି = 0;
	c->physical_ଧାଡ଼ି_ଗଣନା = 0;
	c->formatted_width = 0;
	c->bw = ଖାଲି;
	c->sel = selection_create((ସଂରଚନା content *)c);

	ଫେରନ୍ତୁ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);

	ଫେରନ୍ତୁ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ସ୍ଥିର nserror
textplain_create(ଅଚଳ content_handler *handler,
		 lwc_string *imime_type,
		 ଅଚଳ ସଂରଚନା http_parameter *params,
		 llcache_handle *llcache,
		 ଅଚଳ ଅକ୍ଷର *fallback_charset,
		 bool quirks,
		 ସଂରଚନା content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, ଆକାର(textplain_content));
	ଯଦି (text == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(text);
		ଫେରନ୍ତୁ error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	ଯଦି (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	ଯଦି (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		ମୁକ୍ତ(text);
		ଫେରନ୍ତୁ error;
	}

	lwc_string_unref(encoding);

	*c = (ସଂରଚନା content *) text;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ସ୍ଥିର bool
textplain_copy_utf8_data(textplain_content *c, ଅଚଳ uint8_t *buf, size_t len)
{
	ଯଦି (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		ଅକ୍ଷର *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		ଯଦି (utf8_data == ଖାଲି)
			ଫେରନ୍ତୁ false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	ଫେରନ୍ତୁ true;
}


/**
 * drain input
 */
ସ୍ଥିର bool
textplain_drain_ନିବେଶ(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	ସ୍ଥିର ଅଚଳ uint8_t *u_fffd = (ଅଚଳ uint8_t *) "\xef\xbf\xbd";
	ଅଚଳ uint8_t *ch;
	size_t chlen, offset = 0;

	ଯେତେବେଳେ (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ଯଦି (chlen == 1 && *ch == 0) {
			ଯଦି (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ଯଦି (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ଫେରନ୍ତୁ false;
			}

			/* Emit U+FFFD */
			ଯଦି (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				ଫେରନ୍ତୁ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} ଅନ୍ୟଥା {
			/* Accumulate input */
			offset += chlen;

			ଯଦି (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ଯଦି (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ଫେରନ୍ତୁ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	ଯଦି (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		ଯଦି (textplain_copy_utf8_data(c, ch, offset) == false)
			ଫେରନ୍ତୁ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ସ୍ଥିର bool
textplain_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ଅଚଳ uint8_t *) data, size);
	ଯଦି (error != PARSERUTILS_OK) {
		ଯାଅ no_memory;
	}

	ଯଦି (textplain_drain_ନିବେଶ(text, stream, PARSERUTILS_NEEDDATA) == false)
		ଯାଅ no_memory;

	ଫେରନ୍ତୁ true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
	ଫେରନ୍ତୁ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ସ୍ଥିର bool textplain_convert(ସଂରଚନା content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, ଖାଲି, 0);
	ଯଦି (error != PARSERUTILS_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (textplain_drain_ନିବେଶ(text, stream, PARSERUTILS_EOF) == false)
		ଫେରନ୍ତୁ false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = ଖାଲି;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	ଫେରନ୍ତୁ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ସ୍ଥିର ଦଶମିକ textplain_ଧାଡ଼ି_height(ଶୂନ୍ୟ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ଫେରନ୍ତୁ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ସ୍ଥିର ଶୂନ୍ୟ textplain_reformat(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	textplain_content *text = (textplain_content *) c;
	ଅକ୍ଷର *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long ଧାଡ଼ି_ଗଣନା = 0;
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି = text->physical_ଧାଡ଼ି;
	ସଂରଚନା textplain_ଧାଡ଼ି *line1;
	size_t i, space, col;
	size_t columns = 80;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା character_width;
	size_t ଧାଡ଼ି_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_ଧାଡ଼ି_ଗଣନା = 0;

	ଯଦି (!ଧାଡ଼ି) {
		text->physical_ଧାଡ଼ି = ଧାଡ଼ି =
			malloc(ଆକାର(ସଂରଚନା textplain_ଧାଡ଼ି) * (1024 + 3));
		ଯଦି (!ଧାଡ଼ି)
			ଯାଅ no_memory;
	}

	ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା++].start = ଧାଡ଼ି_start = 0;
	space = 0;
	i = 0;
	col = 0;
	ଯେତେବେଳେ (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ଅଚଳ uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		ଯଦି (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		ଯଦି (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		ଯଦି (term || next_col >= columns) {
			ଯଦି (ଧାଡ଼ି_ଗଣନା % 1024 == 0) {
				line1 = realloc(ଧାଡ଼ି,
						ଆକାର(ସଂରଚନା textplain_ଧାଡ଼ି) *
						(ଧାଡ଼ି_ଗଣନା + 1024 + 3));
				ଯଦି (!line1)
					ଯାଅ no_memory;
				text->physical_ଧାଡ଼ି = ଧାଡ଼ି = line1;
			}

			ଯଦି (term) {
				ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା-1].length = i - ଧାଡ଼ି_start;

				/* skip second char of CR/LF or LF/CR pair */
				ଯଦି (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} ଅନ୍ୟଥା {
				ଯଦି (space) {
					/* break at last space in line */
					i = space;
					ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା-1].length = (i + 1) - ଧାଡ଼ି_start;
				} ଅନ୍ୟଥା
					ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା-1].length = i - ଧାଡ଼ି_start;
			}

			ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା++].start = ଧାଡ଼ି_start = i + 1;
			col = 0;
			space = 0;
		} ଅନ୍ୟଥା {
			col++;
			ଯଦି (chr == ' ')
				space = i;
		}
		i += csize;
	}
	ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା-1].length = i - ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା-1].start;
	ଧାଡ଼ି[ଧାଡ଼ି_ଗଣନା].start = utf8_data_size;

	text->physical_ଧାଡ଼ି_ଗଣନା = ଧାଡ଼ି_ଗଣନା;
	c->width = width;
	c->height = ଧାଡ଼ି_ଗଣନା * textplain_ଧାଡ଼ି_height() + MARGIN + MARGIN;

	ଫେରନ୍ତୁ;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", ଧାଡ଼ି_ଗଣନା);
	ଫେରନ୍ତୁ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ସ୍ଥିର ଶୂନ୍ୟ textplain_destroy(ସଂରଚନା content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	ଯଦି (text->inputstream != ଖାଲି) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	ଯଦି (text->physical_ଧାଡ଼ି != ଖାଲି) {
		ମୁକ୍ତ(text->physical_ଧାଡ଼ି);
	}

	ଯଦି (text->utf8_data != ଖାଲି) {
		ମୁକ୍ତ(text->utf8_data);
	}

	ଯଦି (text->sel != ଖାଲି) {
		selection_destroy(text->sel);
	}
}


ସ୍ଥିର nserror textplain_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	ଅଚଳ textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ଅଚଳ uint8_t *data;
	size_t size;

	text = calloc(1, ଆକାର(textplain_content));
	ଯଦି (text == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&text->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&text->base);
		ଫେରନ୍ତୁ error;
	}

	data = content__get_source_data(&text->base, &size);
	ଯଦି (size > 0) {
		ଯଦି (textplain_process_data(&text->base,
					   (ଅଚଳ ଅକ୍ଷର *)data,
					   size) == false) {
			content_destroy(&text->base);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		ଯଦି (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର content_type textplain_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_TEXTPLAIN;
}


/**
 * Return byte offset within UTF8 textplain content.
 *
 * given the co-ordinates of a point within a textplain content. 'dir'
 * specifies the direction in which to search (-1 = above-left, +1 =
 * below-right) if the co-ordinates are not contained within a line.
 *
 * \param[in] c   content of type CONTENT_TEXTPLAIN
 * \param[in] x   x ordinate of point
 * \param[in] y   y ordinate of point
 * \param[in] dir direction of search if not within line
 * \return byte offset of character containing (or nearest to) point
 */
ସ୍ଥିର size_t
textplain_offset_from_coords(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir)
{
	textplain_content *textc = (textplain_content *) c;
	ଦଶମିକ ଧାଡ଼ି_height = textplain_ଧାଡ଼ି_height();
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି;
	ଅଚଳ ଅକ୍ଷର *text;
	unsigned nlines;
	size_t length;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	y = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((ଦଶମିକ)(y - MARGIN) / ଧାଡ଼ି_height);
	x -= MARGIN;

	nlines = textc->physical_ଧାଡ଼ି_ଗଣନା;
	ଯଦି (!nlines)
		ଫେରନ୍ତୁ 0;

	ଯଦି (y <= 0) y = 0;
	ଅନ୍ୟଥା ଯଦି ((unsigned)y >= nlines)
		y = nlines - 1;

	ଧାଡ଼ି = &textc->physical_ଧାଡ଼ି[y];
	text = textc->utf8_data + ଧାଡ଼ି->start;
	length = ଧାଡ଼ି->length;
	idx = 0;

	ଯେତେବେଳେ (x > 0) {
		size_t next_offset = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = INT_MAX;

		ଯେତେବେଳେ (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		ଯଦି (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		ଯଦି (x <= width) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			ଭାଙ୍ଗ;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		ଯଦି (x <= width) ଭାଙ୍ଗ;

		x -= width;
		length--;
		text++;
		idx++;
	}

	ଫେରନ୍ତୁ ଧାଡ଼ି->start + idx;
}


/**
 * Handle mouse clicks and movements in a TEXTPLAIN content window.
 *
 * \param c	  content of type textplain
 * \param bw	  browser window
 * \param mouse  mouse state on action
 * \param x	  coordinate of mouse
 * \param y	  coordinate of mouse
 */
ସ୍ଥିର nserror
textplain_mouse_action(ସଂରଚନା content *c,
		       ସଂରଚନା browser_window *bw,
		       browser_mouse_state mouse,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ଅଚଳ ଅକ୍ଷର *status = 0;
	size_t idx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, ଖାଲି);

	idx = textplain_offset_from_coords(c, x, y, dir);
	ଯଦି (selection_click(text->sel, text->bw, mouse, idx)) {

		ଯଦି (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, ଖାଲି);
			status = messages_get("Selecting");
		}

	} ଅନ୍ୟଥା {
		ଯଦି (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	ସନ୍ଦେଶ_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_STATUS, &ସନ୍ଦେଶ_data);

	ସନ୍ଦେଶ_data.pointer = pointer;
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_POINTER, &ସନ୍ଦେଶ_data);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Handle mouse tracking (including drags) in a TEXTPLAIN content window.
 *
 * \param  c	  content of type textplain
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
ସ୍ଥିର nserror
textplain_mouse_track(ସଂରଚନା content *c,
		      ସଂରଚନା browser_window *bw,
		      browser_mouse_state mouse,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	textplain_content *text = (textplain_content *) c;

	ଯଦି (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = -1;
		size_t idx;

		ଯଦି (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, ଖାଲି);
	}

	ବାଛ (browser_window_get_drag_type(bw)) {

	ସ୍ଥିତି DRAGGING_SELECTION: {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = -1;
		size_t idx;

		ଯଦି (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ସ୍ଥିର bool textplain_keypress(ସଂରଚନା content *c, uint32_t ଚାବି)
{
	textplain_content *text = (textplain_content *) c;
	ସଂରଚନା selection *sel = text->sel;

	ବାଛ (ଚାବି) {
	ସ୍ଥିତି NS_ଚାବି_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_CLEAR_SELECTION:
		selection_clear(sel, true);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_SELECT_ALL:
		selection_select_all(sel);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ଫେରନ୍ତୁ selection_clear(sel, true);
	}

	ଫେରନ୍ତୁ false;
}


/**
 * Redraw a text string with highlighting
 * (for selection/search)
 *
 * \param utf8_text pointer to UTF-8 text string
 * \param utf8_len  length of string, in bytes
 * \param offset    byte offset within textual representation
 * \param x         x ordinate at which to plot text
 * \param y         y ordinate at which to plot text
 * \param clip      pointer to current clip rectangle
 * \param height    height of text string
 * \param scale     current display scale (1.0 = 100%)
 * \param text      Content being redrawn.
 * \param sel       Selection context
 * \param search    Search context
 * \param ctx	    current redraw context
 * \return true iff successful and redraw should proceed
 */
ସ୍ଥିର bool
text_draw(ଅଚଳ ଅକ୍ଷର *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
	    ଅଚଳ ସଂରଚନା rect *clip,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
	    ଦଶମିକ scale,
	    textplain_content *text,
	    ଅଚଳ ସଂରଚନା selection *sel,
	    ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ଯଦି (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ଯଦି (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ଯଦି (!highlighted &&
		    (text->base.textsearch.context != ଖାଲି) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ଯଦି (highlighted) {
			ସଂରଚନା rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ଯଦି (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ପ୍ରତିଜ୍ଞା(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			ଯଦି (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			ଯଦି (res != NSERROR_OK) {
				endx = 0;
			}

			ଯଦି (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ଯଦି (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				ଯଦି (res != NSERROR_OK) {
					ଫେରନ୍ତୁ false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (start_idx > 0) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px0 = max(x + startx, clip->x0);
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px1 = min(x + endx, clip->x1);

				ଯଦି (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}

					clip_changed = true;
				} ଅନ୍ୟଥା {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ଯଦି (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ଫେରନ୍ତୁ false;
			}

			/* draw any text succeeding highlighted portion */
			ଯଦି (endtxt_idx < utf8_len) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px0 = max(x + endx, clip->x0);
				ଯଦି (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}
				}
			}

			ଯଦି (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ଫେରନ୍ତୁ false;
			}
		}
	}

	ଯଦି (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}
	ଫେରନ୍ତୁ true;
}


/**
 * Draw a CONTENT_TEXTPLAIN using the current set of plotters (plot).
 *
 * x, y, clip_[xy][01] are in target coordinates.
 *
 * \param  c	 content of type CONTENT_TEXTPLAIN
 * \param  data	 redraw data for this content redraw
 * \param  clip	 current clip region
 * \param  ctx	 current redraw context
 * \return true if successful, false otherwise
 */
ସ୍ଥିର bool
textplain_redraw(ସଂରଚନା content *c,
		 ସଂରଚନା content_redraw_data *data,
		 ଅଚଳ ସଂରଚନା rect *clip,
		 ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	ସଂରଚନା browser_window *bw = text->bw;
	ଅକ୍ଷର *utf8_data = text->utf8_data;
	long lineno;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = data->x;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y = data->y;
	unsigned long ଧାଡ଼ି_ଗଣନା = text->physical_ଧାଡ଼ି_ଗଣନା;
	ଦଶମିକ ଧାଡ଼ି_height = textplain_ଧାଡ଼ି_height();
	ଦଶମିକ scaled_ଧାଡ଼ି_height = ଧାଡ଼ି_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_ଧାଡ଼ି_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_ଧାଡ଼ି_height + 1;
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି = text->physical_ଧାଡ଼ି;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	ଯଦି (line0 < 0)
		line0 = 0;
	ଯଦି (line1 < 0)
		line1 = 0;
	ଯଦି (ଧାଡ଼ି_ଗଣନା < (unsigned long) line0)
		line0 = ଧାଡ଼ି_ଗଣନା;
	ଯଦି (ଧାଡ଼ି_ଗଣନା < (unsigned long) line1)
		line1 = ଧାଡ଼ି_ଗଣନା;
	ଯଦି (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!ଧାଡ଼ି)
		ଫେରନ୍ତୁ true;

	/* choose a suitable background colour for any highlighted text */
	ଯଦି ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ଅନ୍ୟଥା
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	ପାଇଁ (lineno = line0; lineno != line1; lineno++) {
		ଅଚଳ ଅକ୍ଷର *text_d = utf8_data + ଧାଡ଼ି[lineno].start;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା tx = x;

		ଯଦି (!tab_width) tab_width = 1;

		length = ଧାଡ଼ି[lineno].length;
		ଯଦି (!length)
			ଜାରି;

		ଯେତେବେଳେ (offset < length) {
			size_t next_offset = offset;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntx;
			nserror res;

			ଯେତେବେଳେ ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			ଯଦି (!text_draw(text_d + offset,
				       next_offset - offset,
				       ଧାଡ଼ି[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_ଧାଡ଼ି_height),
				       clip,
				       ଧାଡ଼ି_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (next_offset >= length)
				ଭାଙ୍ଗ;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			ଯଦି (res == NSERROR_OK) {
				tx += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ଯଦି (bw) {
				unsigned tab_ofst = ଧାଡ଼ି[lineno].start + next_offset;
				ସଂରଚନା selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				ଯଦି (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				ଯଦି (!highlighted &&
				    (c->textsearch.context != ଖାଲି)) {
					unsigned start_idx, end_idx;
					ଯଦି (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				ଯଦି (highlighted) {
					ସଂରଚନା rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_ଧାଡ଼ି_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_ଧାଡ଼ି_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ସ୍ଥିର nserror
textplain_open(ସଂରଚନା content *c,
	       ସଂରଚନା browser_window *bw,
	       ସଂରଚନା content *page,
	       ସଂରଚନା object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ସ୍ଥିର nserror textplain_close(ସଂରଚନା content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = ଖାଲି;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ସ୍ଥିର ଅକ୍ଷର *textplain_get_selection(ସଂରଚନା content *c)
{
	textplain_content *text = (textplain_content *) c;

	ଫେରନ୍ତୁ selection_get_copy(text->sel);
}


/**
 * Convert a character offset within a line of text into the
 * horizontal co-ordinate
 *
 * The conversion takes into account the font being used and any tabs
 * in the text
 *
 * \param  text    line of text
 * \param  offset  char offset within text
 * \param  length  line length
 * \return x ordinate
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
textplain_coord_from_offset(ଅଚଳ ଅକ୍ଷର *text, size_t offset, size_t length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = 0;

	ଯେତେବେଳେ (offset > 0) {
		size_t next_offset = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା tx;

		ଯେତେବେଳେ (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		ଯଦି (next_offset >= offset)
			ଭାଙ୍ଗ;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	ଫେରନ୍ତୁ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ସ୍ଥିର unsigned long textplain_ଧାଡ଼ି_ଗଣନା(ସଂରଚନା content *c)
{
	textplain_content *text = (textplain_content *) c;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ text->physical_ଧାଡ଼ି_ଗଣନା;
}


/**
 * Return a pointer to the requested line of text.
 *
 * \param[in] c        content of type CONTENT_TEXTPLAIN
 * \param[in] lineno   line number
 * \param[out] poffset receives byte offset of line start within text
 * \param[out] plen    receives length of returned line
 * \return pointer to text, or NULL if invalid line number
 */
ସ୍ଥିର ଅକ୍ଷର *
textplain_get_ଧାଡ଼ି(ସଂରଚନା content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (lineno >= text->physical_ଧାଡ଼ି_ଗଣନା)
		ଫେରନ୍ତୁ ଖାଲି;
	ଧାଡ଼ି = &text->physical_ଧାଡ଼ି[lineno];

	*poffset = ଧାଡ଼ି->start;
	*plen = ଧାଡ଼ି->length;
	ଫେରନ୍ତୁ text->utf8_data + ଧାଡ଼ି->start;
}


/**
 * Find line number of byte in text
 *
 * Given a byte offset within the text, return the line number
 * of the line containing that offset.
 *
 * \param[in] c       content of type CONTENT_TEXTPLAIN
 * \param[in] offset  byte offset within textual representation
 * \return line number, or -1 if offset invalid (larger than size)
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା textplain_find_ଧାଡ଼ି(ସଂରଚନା content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nlines;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno = 0;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଧାଡ଼ି = text->physical_ଧାଡ଼ି;
	nlines = text->physical_ଧାଡ଼ି_ଗଣନା;

	ଯଦି (offset > text->utf8_data_size) {
		ଫେରନ୍ତୁ -1;
	}

/* \todo - implement binary search here */
	ଯେତେବେଳେ (lineno < nlines && ଧାଡ଼ି[lineno].start < offset) {
		lineno++;
	}
	ଯଦି (ଧାଡ଼ି[lineno].start > offset) {
		lineno--;
	}

	ଫେରନ୍ତୁ lineno;
}


/**
 * Finds all occurrences of a given string in a textplain content
 *
 * \param c the content to be searched
 * \param context The search context to add the entry to.
 * \param pattern the string pattern to search for
 * \param p_len pattern length
 * \param case_sens whether to perform a case sensitive search
 * \return NSERROR_OK on success else error code on faliure
 */
ସ୍ଥିର nserror
textplain_textsearch_find(ସଂରଚନା content *c,
			  ସଂରଚନା textsearch_context *context,
			  ଅଚଳ ଅକ୍ଷର *pattern,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_len,
			  bool case_sens)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nlines = textplain_ଧାଡ଼ି_ଗଣନା(c);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି;
	nserror res = NSERROR_OK;

	ପାଇଁ(ଧାଡ଼ି = 0; ଧାଡ଼ି < nlines; ଧାଡ଼ି++) {
		size_t offset, length;
		ଅଚଳ ଅକ୍ଷର *text;

		text = textplain_get_ଧାଡ଼ି(c, ଧାଡ଼ି, &offset, &length);
		ଯଦି (text) {
			ଯେତେବେଳେ (length > 0) {
				unsigned match_length;
				size_t start_idx;
				ଅଚଳ ଅକ୍ଷର *new_text;
				ଅଚଳ ଅକ୍ଷର *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				ଯଦି (!pos)
					ଭାଙ୍ଗ;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						ଖାଲି,
						ଖାଲି);
				ଯଦି (res != NSERROR_OK) {
					ଫେରନ୍ତୁ res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	ଫେରନ୍ତୁ res;
}


/**
 * Given a range of byte offsets within a UTF8 textplain content,
 * return a box that fully encloses the text
 *
 * \param[in] c     content of type CONTENT_TEXTPLAIN
 * \param[in] start byte offset of start of text range
 * \param[in] end   byte offset of end
 * \param[out] r    rectangle to be completed
 */
ସ୍ଥିର ଶୂନ୍ୟ
textplain_coords_from_range(ସଂରଚନା content *c,
			    unsigned start,
			    unsigned end,
			    ସଂରଚନା rect *r)
{
	textplain_content *text = (textplain_content *) c;
	ଦଶମିକ ଧାଡ଼ି_height = textplain_ଧାଡ଼ି_height();
	ଅକ୍ଷର *utf8_data;
	ସଂରଚନା textplain_ଧାଡ଼ି *ଧାଡ଼ି;
	unsigned lineno = 0;
	unsigned nlines;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(start <= end);
	ପ୍ରତିଜ୍ଞା(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_ଧାଡ଼ି_ଗଣନା;
	ଧାଡ଼ି = text->physical_ଧାଡ଼ି;

	/* find start */
	lineno = textplain_find_ଧାଡ଼ି(c, start);

	r->y0 = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(MARGIN + lineno * ଧାଡ଼ି_height);

	ଯଦି (lineno + 1 <= nlines || ଧାଡ଼ି[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_ଧାଡ଼ି(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} ଅନ୍ୟଥା {
		/* single line */
		ଅଚଳ ଅକ୍ଷର *text = utf8_data + ଧାଡ଼ି[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - ଧାଡ଼ି[lineno].start,
						    ଧାଡ଼ି[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - ଧାଡ଼ି[lineno].start,
						    ଧାଡ଼ି[lineno].length);
	}

	r->y1 = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(MARGIN + (lineno + 1) * ଧାଡ଼ି_height);
}


/**
 * Return a pointer to the raw UTF-8 data, as opposed to the reformatted
 * text to fit the window width. Thus only hard newlines are preserved
 * in the saved/copied text of a selection.
 *
 * \param[in] c     content of type CONTENT_TEXTPLAIN
 * \param[in] start starting byte offset within UTF-8 text
 * \param[in] end   ending byte offset
 * \param[out] plen receives validated length
 * \return pointer to text, or NULL if no text
 */
ସ୍ଥିର ଅକ୍ଷର *
textplain_get_raw_data(ସଂରଚନା content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	ଯଦି (!utf8_size) ଫେରନ୍ତୁ ଖାଲି;

	/* clamp to valid offset range */
	ଯଦି (start >= utf8_size) start = utf8_size;
	ଯଦି (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	ଫେରନ୍ତୁ text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
ସ୍ଥିର nserror
textplain_textsearch_bounds(ସଂରଚନା content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    ସଂରଚନା box *start_box,
			    ସଂରଚନା box *end_box,
			    ସଂରଚନା rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ସ୍ଥିର nserror
textplain_textselection_redraw(ସଂରଚନା content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	ସଂରଚନା rect r;

	ଯଦି (end_idx <= start_idx) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror
textplain_textselection_copy(ସଂରଚନା content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     ସଂରଚନା selection_string *selstr)
{
	ଅଚଳ ଅକ୍ଷର *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	ଯଦି (text != ଖାଲି) {
		res = selection_string_append(text, length, false, ଖାଲି, selstr);
	}
	ଯଦି (res == false) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ସ୍ଥିର nserror
textplain_textselection_get_end(ସଂରଚନା content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * plain text content handler table
 */
ସ୍ଥିର ଅଚଳ content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_data = textplain_process_data,
	.data_complete = textplain_convert,
	.reformat = textplain_reformat,
	.destroy = textplain_destroy,
	.mouse_track = textplain_mouse_track,
	.mouse_action = textplain_mouse_action,
	.keypress = textplain_keypress,
	.redraw = textplain_redraw,
	.open = textplain_open,
	.close = textplain_close,
	.get_selection = textplain_get_selection,
	.clone = textplain_clone,
	.type = textplain_content_type,
	.textsearch_find = textplain_textsearch_find,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_end = textplain_textselection_get_end,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(ଶୂନ୍ୟ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ଯଦି (lerror != lwc_error_ok) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	ଯଦି (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	ଯଦି (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ଫେରନ୍ତୁ error;
}




