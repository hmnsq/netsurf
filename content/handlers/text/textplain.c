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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <parserutils/input/inputstream.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "content/handlers/css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "text/textplain.h"

ಸಂರಚನಾ textplain_ಸಾಲು {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ textplain_content {
	ಸಂರಚನಾ content base;

	lwc_string *encoding;
	ಶೂನ್ಯ *inputstream;
	ವರ್ಣ *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_ಸಾಲು_ಎಣಿಕೆ;
	ಸಂರಚನಾ textplain_ಸಾಲು *physical_ಸಾಲು;
	ಪೂರ್ಣಾಙ್ಕ formatted_width;
	ಸಂರಚನಾ browser_window *bw;

	ಸಂರಚನಾ selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ಸ್ಥಿರ plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ textplain_tab_width = 256;  /* try for a sensible default */

ಸ್ಥಿರ lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ಸ್ಥಿರ ಶೂನ್ಯ textplain_fini(ಶೂನ್ಯ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ರಿಕ್ತ;
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
ಸ್ಥಿರ parserutils_error
textplain_charset_hack(ಧ್ರುವ uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	ಪ್ರತ್ಯಾಗಮ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ಸ್ಥಿರ nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	ವರ್ಣ *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	ಯದಿ (utf8_data == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	ಯದಿ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	ಯದಿ (error != PARSERUTILS_OK) {
		ಮುಕ್ತ(utf8_data);
		ಗಚ್ಛ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_ಸಾಲು = 0;
	c->physical_ಸಾಲು_ಎಣಿಕೆ = 0;
	c->formatted_width = 0;
	c->bw = ರಿಕ್ತ;
	c->sel = selection_create((ಸಂರಚನಾ content *)c);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ಸ್ಥಿರ nserror
textplain_create(ಧ್ರುವ content_handler *handler,
		 lwc_string *imime_type,
		 ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		 llcache_handle *llcache,
		 ಧ್ರುವ ವರ್ಣ *fallback_charset,
		 bool quirks,
		 ಸಂರಚನಾ content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, ಪರಿಮಾಣ(textplain_content));
	ಯದಿ (text == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(text);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	ಯದಿ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	ಯದಿ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		ಮುಕ್ತ(text);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	lwc_string_unref(encoding);

	*c = (ಸಂರಚನಾ content *) text;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ಸ್ಥಿರ bool
textplain_copy_utf8_data(textplain_content *c, ಧ್ರುವ uint8_t *buf, size_t len)
{
	ಯದಿ (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		ವರ್ಣ *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		ಯದಿ (utf8_data == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * drain input
 */
ಸ್ಥಿರ bool
textplain_drain_ಒಳಹರಿವು(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	ಸ್ಥಿರ ಧ್ರುವ uint8_t *u_fffd = (ಧ್ರುವ uint8_t *) "\xef\xbf\xbd";
	ಧ್ರುವ uint8_t *ch;
	size_t chlen, offset = 0;

	ಯಾವತ್ (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ಯದಿ (chlen == 1 && *ch == 0) {
			ಯದಿ (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ಯದಿ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ಪ್ರತ್ಯಾಗಮ false;
			}

			/* Emit U+FFFD */
			ಯದಿ (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				ಪ್ರತ್ಯಾಗಮ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} ಅನ್ಯಥಾ {
			/* Accumulate input */
			offset += chlen;

			ಯದಿ (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ಯದಿ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ಪ್ರತ್ಯಾಗಮ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	ಯದಿ (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		ಯದಿ (textplain_copy_utf8_data(c, ch, offset) == false)
			ಪ್ರತ್ಯಾಗಮ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ಸ್ಥಿರ bool
textplain_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data, unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ಧ್ರುವ uint8_t *) data, size);
	ಯದಿ (error != PARSERUTILS_OK) {
		ಗಚ್ಛ no_memory;
	}

	ಯದಿ (textplain_drain_ಒಳಹರಿವು(text, stream, PARSERUTILS_NEEDDATA) == false)
		ಗಚ್ಛ no_memory;

	ಪ್ರತ್ಯಾಗಮ true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ಸ್ಥಿರ bool textplain_convert(ಸಂರಚನಾ content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, ರಿಕ್ತ, 0);
	ಯದಿ (error != PARSERUTILS_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (textplain_drain_ಒಳಹರಿವು(text, stream, PARSERUTILS_EOF) == false)
		ಪ್ರತ್ಯಾಗಮ false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = ರಿಕ್ತ;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ಸ್ಥಿರ ಭಿನ್ನ textplain_ಸಾಲು_height(ಶೂನ್ಯ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ಪ್ರತ್ಯಾಗಮ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ಸ್ಥಿರ ಶೂನ್ಯ textplain_reformat(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	textplain_content *text = (textplain_content *) c;
	ವರ್ಣ *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long ಸಾಲು_ಎಣಿಕೆ = 0;
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು = text->physical_ಸಾಲು;
	ಸಂರಚನಾ textplain_ಸಾಲು *line1;
	size_t i, space, col;
	size_t columns = 80;
	ಪೂರ್ಣಾಙ್ಕ character_width;
	size_t ಸಾಲು_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_ಸಾಲು_ಎಣಿಕೆ = 0;

	ಯದಿ (!ಸಾಲು) {
		text->physical_ಸಾಲು = ಸಾಲು =
			malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ textplain_ಸಾಲು) * (1024 + 3));
		ಯದಿ (!ಸಾಲು)
			ಗಚ್ಛ no_memory;
	}

	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ++].start = ಸಾಲು_start = 0;
	space = 0;
	i = 0;
	col = 0;
	ಯಾವತ್ (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ಧ್ರುವ uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		ಯದಿ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		ಯದಿ (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		ಯದಿ (term || next_col >= columns) {
			ಯದಿ (ಸಾಲು_ಎಣಿಕೆ % 1024 == 0) {
				line1 = realloc(ಸಾಲು,
						ಪರಿಮಾಣ(ಸಂರಚನಾ textplain_ಸಾಲು) *
						(ಸಾಲು_ಎಣಿಕೆ + 1024 + 3));
				ಯದಿ (!line1)
					ಗಚ್ಛ no_memory;
				text->physical_ಸಾಲು = ಸಾಲು = line1;
			}

			ಯದಿ (term) {
				ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].length = i - ಸಾಲು_start;

				/* skip second char of CR/LF or LF/CR pair */
				ಯದಿ (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} ಅನ್ಯಥಾ {
				ಯದಿ (space) {
					/* break at last space in line */
					i = space;
					ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].length = (i + 1) - ಸಾಲು_start;
				} ಅನ್ಯಥಾ
					ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].length = i - ಸಾಲು_start;
			}

			ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ++].start = ಸಾಲು_start = i + 1;
			col = 0;
			space = 0;
		} ಅನ್ಯಥಾ {
			col++;
			ಯದಿ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].length = i - ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].start;
	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ].start = utf8_data_size;

	text->physical_ಸಾಲು_ಎಣಿಕೆ = ಸಾಲು_ಎಣಿಕೆ;
	c->width = width;
	c->height = ಸಾಲು_ಎಣಿಕೆ * textplain_ಸಾಲು_height() + MARGIN + MARGIN;

	ಪ್ರತ್ಯಾಗಮ;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", ಸಾಲು_ಎಣಿಕೆ);
	ಪ್ರತ್ಯಾಗಮ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ textplain_destroy(ಸಂರಚನಾ content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	ಯದಿ (text->inputstream != ರಿಕ್ತ) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	ಯದಿ (text->physical_ಸಾಲು != ರಿಕ್ತ) {
		ಮುಕ್ತ(text->physical_ಸಾಲು);
	}

	ಯದಿ (text->utf8_data != ರಿಕ್ತ) {
		ಮುಕ್ತ(text->utf8_data);
	}

	ಯದಿ (text->sel != ರಿಕ್ತ) {
		selection_destroy(text->sel);
	}
}


ಸ್ಥಿರ nserror textplain_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	ಧ್ರುವ textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ಧ್ರುವ uint8_t *data;
	size_t size;

	text = calloc(1, ಪರಿಮಾಣ(textplain_content));
	ಯದಿ (text == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&text->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&text->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	data = content__get_source_data(&text->base, &size);
	ಯದಿ (size > 0) {
		ಯದಿ (textplain_process_data(&text->base,
					   (ಧ್ರುವ ವರ್ಣ *)data,
					   size) == false) {
			content_destroy(&text->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ content_type textplain_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_TEXTPLAIN;
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
ಸ್ಥಿರ size_t
textplain_offset_from_coords(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ dir)
{
	textplain_content *textc = (textplain_content *) c;
	ಭಿನ್ನ ಸಾಲು_height = textplain_ಸಾಲು_height();
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು;
	ಧ್ರುವ ವರ್ಣ *text;
	unsigned nlines;
	size_t length;
	ಪೂರ್ಣಾಙ್ಕ idx;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	y = (ಪೂರ್ಣಾಙ್ಕ)((ಭಿನ್ನ)(y - MARGIN) / ಸಾಲು_height);
	x -= MARGIN;

	nlines = textc->physical_ಸಾಲು_ಎಣಿಕೆ;
	ಯದಿ (!nlines)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (y <= 0) y = 0;
	ಅನ್ಯಥಾ ಯದಿ ((unsigned)y >= nlines)
		y = nlines - 1;

	ಸಾಲು = &textc->physical_ಸಾಲು[y];
	text = textc->utf8_data + ಸಾಲು->start;
	length = ಸಾಲು->length;
	idx = 0;

	ಯಾವತ್ (x > 0) {
		size_t next_offset = 0;
		ಪೂರ್ಣಾಙ್ಕ width = INT_MAX;

		ಯಾವತ್ (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		ಯದಿ (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		ಯದಿ (x <= width) {
			ಪೂರ್ಣಾಙ್ಕ pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			ಭಙ್ಗ;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		ಯದಿ (x <= width) ಭಙ್ಗ;

		x -= width;
		length--;
		text++;
		idx++;
	}

	ಪ್ರತ್ಯಾಗಮ ಸಾಲು->start + idx;
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
ಸ್ಥಿರ nserror
textplain_mouse_action(ಸಂರಚನಾ content *c,
		       ಸಂರಚನಾ browser_window *bw,
		       browser_mouse_state mouse,
		       ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಧ್ರುವ ವರ್ಣ *status = 0;
	size_t idx;
	ಪೂರ್ಣಾಙ್ಕ dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, ರಿಕ್ತ);

	idx = textplain_offset_from_coords(c, x, y, dir);
	ಯದಿ (selection_click(text->sel, text->bw, mouse, idx)) {

		ಯದಿ (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, ರಿಕ್ತ);
			status = messages_get("Selecting");
		}

	} ಅನ್ಯಥಾ {
		ಯದಿ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	ಸಂದೇಶ_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_ಸಂದೇಶ_STATUS, &ಸಂದೇಶ_data);

	ಸಂದೇಶ_data.pointer = pointer;
	content_broadcast(c, CONTENT_ಸಂದೇಶ_POINTER, &ಸಂದೇಶ_data);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ nserror
textplain_mouse_track(ಸಂರಚನಾ content *c,
		      ಸಂರಚನಾ browser_window *bw,
		      browser_mouse_state mouse,
		      ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	textplain_content *text = (textplain_content *) c;

	ಯದಿ (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		ಪೂರ್ಣಾಙ್ಕ dir = -1;
		size_t idx;

		ಯದಿ (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, ರಿಕ್ತ);
	}

	ವಿಕಲ್ಪ (browser_window_get_drag_type(bw)) {

	ಪಕ್ಷ DRAGGING_SELECTION: {
		ಪೂರ್ಣಾಙ್ಕ dir = -1;
		size_t idx;

		ಯದಿ (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ಸ್ಥಿರ bool textplain_keypress(ಸಂರಚನಾ content *c, uint32_t ಕೀಲಿ)
{
	textplain_content *text = (textplain_content *) c;
	ಸಂರಚನಾ selection *sel = text->sel;

	ವಿಕಲ್ಪ (ಕೀಲಿ) {
	ಪಕ್ಷ NS_ಕೀಲಿ_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_SELECT_ALL:
		selection_select_all(sel);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ಪ್ರತ್ಯಾಗಮ selection_clear(sel, true);
	}

	ಪ್ರತ್ಯಾಗಮ false;
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
ಸ್ಥಿರ bool
text_draw(ಧ್ರುವ ವರ್ಣ *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    ಪೂರ್ಣಾಙ್ಕ x,
	    ಪೂರ್ಣಾಙ್ಕ y,
	    ಧ್ರುವ ಸಂರಚನಾ rect *clip,
	    ಪೂರ್ಣಾಙ್ಕ height,
	    ಭಿನ್ನ scale,
	    textplain_content *text,
	    ಧ್ರುವ ಸಂರಚನಾ selection *sel,
	    ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ಯದಿ (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ಯದಿ (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ಯದಿ (!highlighted &&
		    (text->base.textsearch.context != ರಿಕ್ತ) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ಯದಿ (highlighted) {
			ಸಂರಚನಾ rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			ಪೂರ್ಣಾಙ್ಕ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ಯದಿ (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ಪ್ರತಿಜ್ಞಾ(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			ಯದಿ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			ಯದಿ (res != NSERROR_OK) {
				endx = 0;
			}

			ಯದಿ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ಯದಿ (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				ಯದಿ (res != NSERROR_OK) {
					ಪ್ರತ್ಯಾಗಮ false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (start_idx > 0) {
				ಪೂರ್ಣಾಙ್ಕ px0 = max(x + startx, clip->x0);
				ಪೂರ್ಣಾಙ್ಕ px1 = min(x + endx, clip->x1);

				ಯದಿ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}

					clip_changed = true;
				} ಅನ್ಯಥಾ {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ಯದಿ (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			/* draw any text succeeding highlighted portion */
			ಯದಿ (endtxt_idx < utf8_len) {
				ಪೂರ್ಣಾಙ್ಕ px0 = max(x + endx, clip->x0);
				ಯದಿ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}
				}
			}

			ಯದಿ (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}

	ಯದಿ (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (ಪೂರ್ಣಾಙ್ಕ) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}
	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool
textplain_redraw(ಸಂರಚನಾ content *c,
		 ಸಂರಚನಾ content_redraw_data *data,
		 ಧ್ರುವ ಸಂರಚನಾ rect *clip,
		 ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	ಸಂರಚನಾ browser_window *bw = text->bw;
	ವರ್ಣ *utf8_data = text->utf8_data;
	long lineno;
	ಪೂರ್ಣಾಙ್ಕ x = data->x;
	ಪೂರ್ಣಾಙ್ಕ y = data->y;
	unsigned long ಸಾಲು_ಎಣಿಕೆ = text->physical_ಸಾಲು_ಎಣಿಕೆ;
	ಭಿನ್ನ ಸಾಲು_height = textplain_ಸಾಲು_height();
	ಭಿನ್ನ scaled_ಸಾಲು_height = ಸಾಲು_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_ಸಾಲು_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_ಸಾಲು_height + 1;
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು = text->physical_ಸಾಲು;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	ಯದಿ (line0 < 0)
		line0 = 0;
	ಯದಿ (line1 < 0)
		line1 = 0;
	ಯದಿ (ಸಾಲು_ಎಣಿಕೆ < (unsigned long) line0)
		line0 = ಸಾಲು_ಎಣಿಕೆ;
	ಯದಿ (ಸಾಲು_ಎಣಿಕೆ < (unsigned long) line1)
		line1 = ಸಾಲು_ಎಣಿಕೆ;
	ಯದಿ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!ಸಾಲು)
		ಪ್ರತ್ಯಾಗಮ true;

	/* choose a suitable background colour for any highlighted text */
	ಯದಿ ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ಅನ್ಯಥಾ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	ಪ್ರತಿ (lineno = line0; lineno != line1; lineno++) {
		ಧ್ರುವ ವರ್ಣ *text_d = utf8_data + ಸಾಲು[lineno].start;
		ಪೂರ್ಣಾಙ್ಕ tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		ಪೂರ್ಣಾಙ್ಕ tx = x;

		ಯದಿ (!tab_width) tab_width = 1;

		length = ಸಾಲು[lineno].length;
		ಯದಿ (!length)
			ಅನುವರ್ತ;

		ಯಾವತ್ (offset < length) {
			size_t next_offset = offset;
			ಪೂರ್ಣಾಙ್ಕ width;
			ಪೂರ್ಣಾಙ್ಕ ntx;
			nserror res;

			ಯಾವತ್ ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			ಯದಿ (!text_draw(text_d + offset,
				       next_offset - offset,
				       ಸಾಲು[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_ಸಾಲು_height),
				       clip,
				       ಸಾಲು_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (next_offset >= length)
				ಭಙ್ಗ;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			ಯದಿ (res == NSERROR_OK) {
				tx += (ಪೂರ್ಣಾಙ್ಕ)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ಯದಿ (bw) {
				unsigned tab_ofst = ಸಾಲು[lineno].start + next_offset;
				ಸಂರಚನಾ selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				ಯದಿ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				ಯದಿ (!highlighted &&
				    (c->textsearch.context != ರಿಕ್ತ)) {
					unsigned start_idx, end_idx;
					ಯದಿ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				ಯದಿ (highlighted) {
					ಸಂರಚನಾ rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_ಸಾಲು_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_ಸಾಲು_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ಸ್ಥಿರ nserror
textplain_open(ಸಂರಚನಾ content *c,
	       ಸಂರಚನಾ browser_window *bw,
	       ಸಂರಚನಾ content *page,
	       ಸಂರಚನಾ object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ಸ್ಥಿರ nserror textplain_close(ಸಂರಚನಾ content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ಸ್ಥಿರ ವರ್ಣ *textplain_get_selection(ಸಂರಚನಾ content *c)
{
	textplain_content *text = (textplain_content *) c;

	ಪ್ರತ್ಯಾಗಮ selection_get_copy(text->sel);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
textplain_coord_from_offset(ಧ್ರುವ ವರ್ಣ *text, size_t offset, size_t length)
{
	ಪೂರ್ಣಾಙ್ಕ x = 0;

	ಯಾವತ್ (offset > 0) {
		size_t next_offset = 0;
		ಪೂರ್ಣಾಙ್ಕ tx;

		ಯಾವತ್ (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		ಯದಿ (next_offset >= offset)
			ಭಙ್ಗ;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	ಪ್ರತ್ಯಾಗಮ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ಸ್ಥಿರ unsigned long textplain_ಸಾಲು_ಎಣಿಕೆ(ಸಂರಚನಾ content *c)
{
	textplain_content *text = (textplain_content *) c;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ text->physical_ಸಾಲು_ಎಣಿಕೆ;
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
ಸ್ಥಿರ ವರ್ಣ *
textplain_get_ಸಾಲು(ಸಂರಚನಾ content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (lineno >= text->physical_ಸಾಲು_ಎಣಿಕೆ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಸಾಲು = &text->physical_ಸಾಲು[lineno];

	*poffset = ಸಾಲು->start;
	*plen = ಸಾಲು->length;
	ಪ್ರತ್ಯಾಗಮ text->utf8_data + ಸಾಲು->start;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ textplain_find_ಸಾಲು(ಸಂರಚನಾ content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು;
	ಪೂರ್ಣಾಙ್ಕ nlines;
	ಪೂರ್ಣಾಙ್ಕ lineno = 0;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಸಾಲು = text->physical_ಸಾಲು;
	nlines = text->physical_ಸಾಲು_ಎಣಿಕೆ;

	ಯದಿ (offset > text->utf8_data_size) {
		ಪ್ರತ್ಯಾಗಮ -1;
	}

/* \todo - implement binary search here */
	ಯಾವತ್ (lineno < nlines && ಸಾಲು[lineno].start < offset) {
		lineno++;
	}
	ಯದಿ (ಸಾಲು[lineno].start > offset) {
		lineno--;
	}

	ಪ್ರತ್ಯಾಗಮ lineno;
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
ಸ್ಥಿರ nserror
textplain_textsearch_find(ಸಂರಚನಾ content *c,
			  ಸಂರಚನಾ textsearch_context *context,
			  ಧ್ರುವ ವರ್ಣ *pattern,
			  ಪೂರ್ಣಾಙ್ಕ p_len,
			  bool case_sens)
{
	ಪೂರ್ಣಾಙ್ಕ nlines = textplain_ಸಾಲು_ಎಣಿಕೆ(c);
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು;
	nserror res = NSERROR_OK;

	ಪ್ರತಿ(ಸಾಲು = 0; ಸಾಲು < nlines; ಸಾಲು++) {
		size_t offset, length;
		ಧ್ರುವ ವರ್ಣ *text;

		text = textplain_get_ಸಾಲು(c, ಸಾಲು, &offset, &length);
		ಯದಿ (text) {
			ಯಾವತ್ (length > 0) {
				unsigned match_length;
				size_t start_idx;
				ಧ್ರುವ ವರ್ಣ *new_text;
				ಧ್ರುವ ವರ್ಣ *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				ಯದಿ (!pos)
					ಭಙ್ಗ;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						ರಿಕ್ತ,
						ರಿಕ್ತ);
				ಯದಿ (res != NSERROR_OK) {
					ಪ್ರತ್ಯಾಗಮ res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ ಶೂನ್ಯ
textplain_coords_from_range(ಸಂರಚನಾ content *c,
			    unsigned start,
			    unsigned end,
			    ಸಂರಚನಾ rect *r)
{
	textplain_content *text = (textplain_content *) c;
	ಭಿನ್ನ ಸಾಲು_height = textplain_ಸಾಲು_height();
	ವರ್ಣ *utf8_data;
	ಸಂರಚನಾ textplain_ಸಾಲು *ಸಾಲು;
	unsigned lineno = 0;
	unsigned nlines;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(start <= end);
	ಪ್ರತಿಜ್ಞಾ(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_ಸಾಲು_ಎಣಿಕೆ;
	ಸಾಲು = text->physical_ಸಾಲು;

	/* find start */
	lineno = textplain_find_ಸಾಲು(c, start);

	r->y0 = (ಪೂರ್ಣಾಙ್ಕ)(MARGIN + lineno * ಸಾಲು_height);

	ಯದಿ (lineno + 1 <= nlines || ಸಾಲು[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_ಸಾಲು(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} ಅನ್ಯಥಾ {
		/* single line */
		ಧ್ರುವ ವರ್ಣ *text = utf8_data + ಸಾಲು[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - ಸಾಲು[lineno].start,
						    ಸಾಲು[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - ಸಾಲು[lineno].start,
						    ಸಾಲು[lineno].length);
	}

	r->y1 = (ಪೂರ್ಣಾಙ್ಕ)(MARGIN + (lineno + 1) * ಸಾಲು_height);
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
ಸ್ಥಿರ ವರ್ಣ *
textplain_get_raw_data(ಸಂರಚನಾ content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	ಯದಿ (!utf8_size) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* clamp to valid offset range */
	ಯದಿ (start >= utf8_size) start = utf8_size;
	ಯದಿ (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	ಪ್ರತ್ಯಾಗಮ text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
ಸ್ಥಿರ nserror
textplain_textsearch_bounds(ಸಂರಚನಾ content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    ಸಂರಚನಾ box *start_box,
			    ಸಂರಚನಾ box *end_box,
			    ಸಂರಚನಾ rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ಸ್ಥಿರ nserror
textplain_textselection_redraw(ಸಂರಚನಾ content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	ಸಂರಚನಾ rect r;

	ಯದಿ (end_idx <= start_idx) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ nserror
textplain_textselection_copy(ಸಂರಚನಾ content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     ಸಂರಚನಾ selection_string *selstr)
{
	ಧ್ರುವ ವರ್ಣ *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	ಯದಿ (text != ರಿಕ್ತ) {
		res = selection_string_append(text, length, false, ರಿಕ್ತ, selstr);
	}
	ಯದಿ (res == false) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ಸ್ಥಿರ nserror
textplain_textselection_get_end(ಸಂರಚನಾ content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * plain text content handler table
 */
ಸ್ಥಿರ ಧ್ರುವ content_handler textplain_content_handler = {
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
nserror textplain_init(ಶೂನ್ಯ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ಯದಿ (lerror != lwc_error_ok) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	ಯದಿ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	ಯದಿ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ಪ್ರತ್ಯಾಗಮ error;
}




