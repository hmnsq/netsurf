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

#include <string.h>
#include <parserutils/input/inputstream.h>

#include "utils/errors.h"
#include "utils/corestrings.h"
#include "utils/http.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/utf8.h"
#include "utils/nsoption.h"
#include "netsurf/content.h"
#include "netsurf/keypress.h"
#include "netsurf/browser_window.h"
#include "netsurf/plotters.h"
#include "netsurf/layout.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "content/hlcache.h"
#include "content/textsearch.h"
#include "content/handlers/css/utils.h"
#include "desktop/selection.h"
#include "desktop/gui_internal.h"

#include "text/textplain.h"

sa_nracanaa textplain_line {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
typedef sa_nracanaa textplain_content {
	sa_nracanaa content base;

	lwc_string *encoding;
	void *inputstream;
	var_na *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_line_count;
	sa_nracanaa textplain_line *physical_line;
	puur_naa_nka formatted_width;
	sa_nracanaa browser_window *bw;

	sa_nracanaa selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

static plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

static puur_naa_nka textplain_tab_width = 256;  /* try for a sensible default */

static lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
static void textplain_fini(void)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = NULL;
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
static parserutils_error
textplain_charset_hack(const uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	ᬯᬮᬶ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
static nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	var_na *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	ᬬᬾᬦ᭄ (utf8_data == NULL)
		goto no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	ᬬᬾᬦ᭄ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	ᬬᬾᬦ᭄ (error != PARSERUTILS_OK) {
		free(utf8_data);
		goto no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_line = 0;
	c->physical_line_count = 0;
	c->formatted_width = 0;
	c->bw = NULL;
	c->sel = selection_create((sa_nracanaa content *)c);

	ᬯᬮᬶ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);

	ᬯᬮᬶ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
static nserror
textplain_create(const content_handler *handler,
		 lwc_string *imime_type,
		 const sa_nracanaa http_parameter *params,
		 llcache_handle *llcache,
		 const var_na *fallback_charset,
		 bool quirks,
		 sa_nracanaa content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, parimaa_na(textplain_content));
	ᬬᬾᬦ᭄ (text == NULL) {
		ᬯᬮᬶ NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(text);
		ᬯᬮᬶ error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		free(text);
		ᬯᬮᬶ error;
	}

	lwc_string_unref(encoding);

	*c = (sa_nracanaa content *) text;

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
static bool
textplain_copy_utf8_data(textplain_content *c, const uint8_t *buf, size_t len)
{
	ᬬᬾᬦ᭄ (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		var_na *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		ᬬᬾᬦ᭄ (utf8_data == NULL)
			ᬯᬮᬶ false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	ᬯᬮᬶ true;
}


/**
 * drain input
 */
static bool
textplain_drain_input(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	static const uint8_t *u_fffd = (const uint8_t *) "\xef\xbf\xbd";
	const uint8_t *ch;
	size_t chlen, offset = 0;

	ᬲᭂᬤᭂᬮ (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ᬬᬾᬦ᭄ (chlen == 1 && *ch == 0) {
			ᬬᬾᬦ᭄ (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ᬬᬾᬦ᭄ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ᬯᬮᬶ false;
			}

			/* Emit U+FFFD */
			ᬬᬾᬦ᭄ (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				ᬯᬮᬶ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} ᬮᬶᬬ {
			/* Accumulate input */
			offset += chlen;

			ᬬᬾᬦ᭄ (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ᬬᬾᬦ᭄ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ᬯᬮᬶ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	ᬬᬾᬦ᭄ (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		ᬬᬾᬦ᭄ (textplain_copy_utf8_data(c, ch, offset) == false)
			ᬯᬮᬶ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	ᬯᬮᬶ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
static bool
textplain_process_data(sa_nracanaa content *c, const var_na *data, unsigned puur_naa_nka size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (const uint8_t *) data, size);
	ᬬᬾᬦ᭄ (error != PARSERUTILS_OK) {
		goto no_memory;
	}

	ᬬᬾᬦ᭄ (textplain_drain_input(text, stream, PARSERUTILS_NEEDDATA) == false)
		goto no_memory;

	ᬯᬮᬶ true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, NULL);
	ᬯᬮᬶ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
static bool textplain_convert(sa_nracanaa content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, NULL, 0);
	ᬬᬾᬦ᭄ (error != PARSERUTILS_OK) {
		ᬯᬮᬶ false;
	}

	ᬬᬾᬦ᭄ (textplain_drain_input(text, stream, PARSERUTILS_EOF) == false)
		ᬯᬮᬶ false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = NULL;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	ᬯᬮᬶ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
static float textplain_line_height(void)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ᬯᬮᬶ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
static void textplain_reformat(sa_nracanaa content *c, puur_naa_nka width, puur_naa_nka height)
{
	textplain_content *text = (textplain_content *) c;
	var_na *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long line_count = 0;
	sa_nracanaa textplain_line *line = text->physical_line;
	sa_nracanaa textplain_line *line1;
	size_t i, space, col;
	size_t columns = 80;
	puur_naa_nka character_width;
	size_t line_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_line_count = 0;

	ᬬᬾᬦ᭄ (!line) {
		text->physical_line = line =
			malloc(parimaa_na(sa_nracanaa textplain_line) * (1024 + 3));
		ᬬᬾᬦ᭄ (!line)
			goto no_memory;
	}

	line[line_count++].start = line_start = 0;
	space = 0;
	i = 0;
	col = 0;
	ᬲᭂᬤᭂᬮ (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((const uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		ᬬᬾᬦ᭄ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		ᬬᬾᬦ᭄ (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		ᬬᬾᬦ᭄ (term || next_col >= columns) {
			ᬬᬾᬦ᭄ (line_count % 1024 == 0) {
				line1 = realloc(line,
						parimaa_na(sa_nracanaa textplain_line) *
						(line_count + 1024 + 3));
				ᬬᬾᬦ᭄ (!line1)
					goto no_memory;
				text->physical_line = line = line1;
			}

			ᬬᬾᬦ᭄ (term) {
				line[line_count-1].length = i - line_start;

				/* skip second char of CR/LF or LF/CR pair */
				ᬬᬾᬦ᭄ (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} ᬮᬶᬬ {
				ᬬᬾᬦ᭄ (space) {
					/* break at last space in line */
					i = space;
					line[line_count-1].length = (i + 1) - line_start;
				} ᬮᬶᬬ
					line[line_count-1].length = i - line_start;
			}

			line[line_count++].start = line_start = i + 1;
			col = 0;
			space = 0;
		} ᬮᬶᬬ {
			col++;
			ᬬᬾᬦ᭄ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	line[line_count-1].length = i - line[line_count-1].start;
	line[line_count].start = utf8_data_size;

	text->physical_line_count = line_count;
	c->width = width;
	c->height = line_count * textplain_line_height() + MARGIN + MARGIN;

	ᬯᬮᬶ;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", line_count);
	ᬯᬮᬶ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

static void textplain_destroy(sa_nracanaa content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	ᬬᬾᬦ᭄ (text->inputstream != NULL) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	ᬬᬾᬦ᭄ (text->physical_line != NULL) {
		free(text->physical_line);
	}

	ᬬᬾᬦ᭄ (text->utf8_data != NULL) {
		free(text->utf8_data);
	}

	ᬬᬾᬦ᭄ (text->sel != NULL) {
		selection_destroy(text->sel);
	}
}


static nserror textplain_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	const textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	const uint8_t *data;
	size_t size;

	text = calloc(1, parimaa_na(textplain_content));
	ᬬᬾᬦ᭄ (text == NULL)
		ᬯᬮᬶ NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&text->base);
		ᬯᬮᬶ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&text->base);
		ᬯᬮᬶ error;
	}

	data = content__get_source_data(&text->base, &size);
	ᬬᬾᬦ᭄ (size > 0) {
		ᬬᬾᬦ᭄ (textplain_process_data(&text->base,
					   (const var_na *)data,
					   size) == false) {
			content_destroy(&text->base);
			ᬯᬮᬶ NSERROR_NOMEM;
		}
	}

	ᬬᬾᬦ᭄ (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		ᬬᬾᬦ᭄ (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			ᬯᬮᬶ NSERROR_CLONE_FAILED;
		}
	}

	ᬯᬮᬶ NSERROR_OK;
}


static content_type textplain_content_type(void)
{
	ᬯᬮᬶ CONTENT_TEXTPLAIN;
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
static size_t
textplain_offset_from_coords(sa_nracanaa content *c, puur_naa_nka x, puur_naa_nka y, puur_naa_nka dir)
{
	textplain_content *textc = (textplain_content *) c;
	float line_height = textplain_line_height();
	sa_nracanaa textplain_line *line;
	const var_na *text;
	unsigned nlines;
	size_t length;
	puur_naa_nka idx;

	assert(c != NULL);

	y = (puur_naa_nka)((float)(y - MARGIN) / line_height);
	x -= MARGIN;

	nlines = textc->physical_line_count;
	ᬬᬾᬦ᭄ (!nlines)
		ᬯᬮᬶ 0;

	ᬬᬾᬦ᭄ (y <= 0) y = 0;
	ᬮᬶᬬ ᬬᬾᬦ᭄ ((unsigned)y >= nlines)
		y = nlines - 1;

	line = &textc->physical_line[y];
	text = textc->utf8_data + line->start;
	length = line->length;
	idx = 0;

	ᬲᭂᬤᭂᬮ (x > 0) {
		size_t next_offset = 0;
		puur_naa_nka width = INT_MAX;

		ᬲᭂᬤᭂᬮ (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		ᬬᬾᬦ᭄ (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		ᬬᬾᬦ᭄ (x <= width) {
			puur_naa_nka pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			bha_nga;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		ᬬᬾᬦ᭄ (x <= width) bha_nga;

		x -= width;
		length--;
		text++;
		idx++;
	}

	ᬯᬮᬶ line->start + idx;
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
static nserror
textplain_mouse_action(sa_nracanaa content *c,
		       sa_nracanaa browser_window *bw,
		       browser_mouse_state mouse,
		       puur_naa_nka x, puur_naa_nka y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	sa_ngha content_msg_data msg_data;
	const var_na *status = 0;
	size_t idx;
	puur_naa_nka dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, NULL);

	idx = textplain_offset_from_coords(c, x, y, dir);
	ᬬᬾᬦ᭄ (selection_click(text->sel, text->bw, mouse, idx)) {

		ᬬᬾᬦ᭄ (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, NULL);
			status = messages_get("Selecting");
		}

	} ᬮᬶᬬ {
		ᬬᬾᬦ᭄ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	msg_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

	msg_data.pointer = pointer;
	content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);

	ᬯᬮᬶ NSERROR_OK;
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
static nserror
textplain_mouse_track(sa_nracanaa content *c,
		      sa_nracanaa browser_window *bw,
		      browser_mouse_state mouse,
		      puur_naa_nka x, puur_naa_nka y)
{
	textplain_content *text = (textplain_content *) c;

	ᬬᬾᬦ᭄ (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		puur_naa_nka dir = -1;
		size_t idx;

		ᬬᬾᬦ᭄ (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, NULL);
	}

	switch (browser_window_get_drag_type(bw)) {

	pak_sa DRAGGING_SELECTION: {
		puur_naa_nka dir = -1;
		size_t idx;

		ᬬᬾᬦ᭄ (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		bha_nga;

	default:
		textplain_mouse_action(c, bw, mouse, x, y);
		bha_nga;
	}

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
static bool textplain_keypress(sa_nracanaa content *c, uint32_t key)
{
	textplain_content *text = (textplain_content *) c;
	sa_nracanaa selection *sel = text->sel;

	switch (key) {
	pak_sa NS_KEY_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ᬯᬮᬶ true;

	pak_sa NS_KEY_CLEAR_SELECTION:
		selection_clear(sel, true);
		ᬯᬮᬶ true;

	pak_sa NS_KEY_SELECT_ALL:
		selection_select_all(sel);
		ᬯᬮᬶ true;

	pak_sa NS_KEY_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ᬯᬮᬶ selection_clear(sel, true);
	}

	ᬯᬮᬶ false;
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
static bool
text_draw(const var_na *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    puur_naa_nka x,
	    puur_naa_nka y,
	    const sa_nracanaa rect *clip,
	    puur_naa_nka height,
	    float scale,
	    textplain_content *text,
	    const sa_nracanaa selection *sel,
	    const sa_nracanaa redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ᬬᬾᬦ᭄ (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ᬬᬾᬦ᭄ (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ᬬᬾᬦ᭄ (!highlighted &&
		    (text->base.textsearch.context != NULL) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ᬬᬾᬦ᭄ (highlighted) {
			sa_nracanaa rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			puur_naa_nka startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ᬬᬾᬦ᭄ (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				assert(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				endx = 0;
			}

			ᬬᬾᬦ᭄ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ᬬᬾᬦ᭄ (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (puur_naa_nka)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				ᬬᬾᬦ᭄ (res != NSERROR_OK) {
					ᬯᬮᬶ false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				ᬯᬮᬶ false;
			}

			ᬬᬾᬦ᭄ (start_idx > 0) {
				puur_naa_nka px0 = max(x + startx, clip->x0);
				puur_naa_nka px1 = min(x + endx, clip->x1);

				ᬬᬾᬦ᭄ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ᬬᬾᬦ᭄ (res != NSERROR_OK) {
						ᬯᬮᬶ false;
					}

					clip_changed = true;
				} ᬮᬶᬬ {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ᬬᬾᬦ᭄ (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (puur_naa_nka)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ᬯᬮᬶ false;
			}

			/* draw any text succeeding highlighted portion */
			ᬬᬾᬦ᭄ (endtxt_idx < utf8_len) {
				puur_naa_nka px0 = max(x + endx, clip->x0);
				ᬬᬾᬦ᭄ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ᬬᬾᬦ᭄ (res != NSERROR_OK) {
						ᬯᬮᬶ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (puur_naa_nka)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ᬬᬾᬦ᭄ (res != NSERROR_OK) {
						ᬯᬮᬶ false;
					}
				}
			}

			ᬬᬾᬦ᭄ (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ᬯᬮᬶ false;
			}
		}
	}

	ᬬᬾᬦ᭄ (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (puur_naa_nka) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ false;
		}
	}
	ᬯᬮᬶ true;
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
static bool
textplain_redraw(sa_nracanaa content *c,
		 sa_nracanaa content_redraw_data *data,
		 const sa_nracanaa rect *clip,
		 const sa_nracanaa redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	sa_nracanaa browser_window *bw = text->bw;
	var_na *utf8_data = text->utf8_data;
	long lineno;
	puur_naa_nka x = data->x;
	puur_naa_nka y = data->y;
	unsigned long line_count = text->physical_line_count;
	float line_height = textplain_line_height();
	float scaled_line_height = line_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_line_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_line_height + 1;
	sa_nracanaa textplain_line *line = text->physical_line;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	ᬬᬾᬦ᭄ (line0 < 0)
		line0 = 0;
	ᬬᬾᬦ᭄ (line1 < 0)
		line1 = 0;
	ᬬᬾᬦ᭄ (line_count < (unsigned long) line0)
		line0 = line_count;
	ᬬᬾᬦ᭄ (line_count < (unsigned long) line1)
		line1 = line_count;
	ᬬᬾᬦ᭄ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ false;
	}

	ᬬᬾᬦ᭄ (!line)
		ᬯᬮᬶ true;

	/* choose a suitable background colour for any highlighted text */
	ᬬᬾᬦ᭄ ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ᬮᬶᬬ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	for (lineno = line0; lineno != line1; lineno++) {
		const var_na *text_d = utf8_data + line[lineno].start;
		puur_naa_nka tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		puur_naa_nka tx = x;

		ᬬᬾᬦ᭄ (!tab_width) tab_width = 1;

		length = line[lineno].length;
		ᬬᬾᬦ᭄ (!length)
			continue;

		ᬲᭂᬤᭂᬮ (offset < length) {
			size_t next_offset = offset;
			puur_naa_nka width;
			puur_naa_nka ntx;
			nserror res;

			ᬲᭂᬤᭂᬮ ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			ᬬᬾᬦ᭄ (!text_draw(text_d + offset,
				       next_offset - offset,
				       line[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_line_height),
				       clip,
				       line_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				ᬯᬮᬶ false;
			}

			ᬬᬾᬦ᭄ (next_offset >= length)
				bha_nga;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			ᬬᬾᬦ᭄ (res == NSERROR_OK) {
				tx += (puur_naa_nka)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ᬬᬾᬦ᭄ (bw) {
				unsigned tab_ofst = line[lineno].start + next_offset;
				sa_nracanaa selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				ᬬᬾᬦ᭄ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				ᬬᬾᬦ᭄ (!highlighted &&
				    (c->textsearch.context != NULL)) {
					unsigned start_idx, end_idx;
					ᬬᬾᬦ᭄ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				ᬬᬾᬦ᭄ (highlighted) {
					sa_nracanaa rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_line_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_line_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ᬬᬾᬦ᭄ (res != NSERROR_OK) {
						ᬯᬮᬶ false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	ᬯᬮᬶ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
static nserror
textplain_open(sa_nracanaa content *c,
	       sa_nracanaa browser_window *bw,
	       sa_nracanaa content *page,
	       sa_nracanaa object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
static nserror textplain_close(sa_nracanaa content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = NULL;

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
static var_na *textplain_get_selection(sa_nracanaa content *c)
{
	textplain_content *text = (textplain_content *) c;

	ᬯᬮᬶ selection_get_copy(text->sel);
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
static puur_naa_nka
textplain_coord_from_offset(const var_na *text, size_t offset, size_t length)
{
	puur_naa_nka x = 0;

	ᬲᭂᬤᭂᬮ (offset > 0) {
		size_t next_offset = 0;
		puur_naa_nka tx;

		ᬲᭂᬤᭂᬮ (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		ᬬᬾᬦ᭄ (next_offset >= offset)
			bha_nga;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	ᬯᬮᬶ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
static unsigned long textplain_line_count(sa_nracanaa content *c)
{
	textplain_content *text = (textplain_content *) c;

	assert(c != NULL);

	ᬯᬮᬶ text->physical_line_count;
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
static var_na *
textplain_get_line(sa_nracanaa content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	sa_nracanaa textplain_line *line;

	assert(c != NULL);

	ᬬᬾᬦ᭄ (lineno >= text->physical_line_count)
		ᬯᬮᬶ NULL;
	line = &text->physical_line[lineno];

	*poffset = line->start;
	*plen = line->length;
	ᬯᬮᬶ text->utf8_data + line->start;
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
static puur_naa_nka textplain_find_line(sa_nracanaa content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	sa_nracanaa textplain_line *line;
	puur_naa_nka nlines;
	puur_naa_nka lineno = 0;

	assert(c != NULL);

	line = text->physical_line;
	nlines = text->physical_line_count;

	ᬬᬾᬦ᭄ (offset > text->utf8_data_size) {
		ᬯᬮᬶ -1;
	}

/* \todo - implement binary search here */
	ᬲᭂᬤᭂᬮ (lineno < nlines && line[lineno].start < offset) {
		lineno++;
	}
	ᬬᬾᬦ᭄ (line[lineno].start > offset) {
		lineno--;
	}

	ᬯᬮᬶ lineno;
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
static nserror
textplain_textsearch_find(sa_nracanaa content *c,
			  sa_nracanaa textsearch_context *context,
			  const var_na *pattern,
			  puur_naa_nka p_len,
			  bool case_sens)
{
	puur_naa_nka nlines = textplain_line_count(c);
	puur_naa_nka line;
	nserror res = NSERROR_OK;

	for(line = 0; line < nlines; line++) {
		size_t offset, length;
		const var_na *text;

		text = textplain_get_line(c, line, &offset, &length);
		ᬬᬾᬦ᭄ (text) {
			ᬲᭂᬤᭂᬮ (length > 0) {
				unsigned match_length;
				size_t start_idx;
				const var_na *new_text;
				const var_na *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				ᬬᬾᬦ᭄ (!pos)
					bha_nga;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						NULL,
						NULL);
				ᬬᬾᬦ᭄ (res != NSERROR_OK) {
					ᬯᬮᬶ res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	ᬯᬮᬶ res;
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
static void
textplain_coords_from_range(sa_nracanaa content *c,
			    unsigned start,
			    unsigned end,
			    sa_nracanaa rect *r)
{
	textplain_content *text = (textplain_content *) c;
	float line_height = textplain_line_height();
	var_na *utf8_data;
	sa_nracanaa textplain_line *line;
	unsigned lineno = 0;
	unsigned nlines;

	assert(c != NULL);
	assert(start <= end);
	assert(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_line_count;
	line = text->physical_line;

	/* find start */
	lineno = textplain_find_line(c, start);

	r->y0 = (puur_naa_nka)(MARGIN + lineno * line_height);

	ᬬᬾᬦ᭄ (lineno + 1 <= nlines || line[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_line(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} ᬮᬶᬬ {
		/* single line */
		const var_na *text = utf8_data + line[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - line[lineno].start,
						    line[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - line[lineno].start,
						    line[lineno].length);
	}

	r->y1 = (puur_naa_nka)(MARGIN + (lineno + 1) * line_height);
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
static var_na *
textplain_get_raw_data(sa_nracanaa content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	assert(c != NULL);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	ᬬᬾᬦ᭄ (!utf8_size) ᬯᬮᬶ NULL;

	/* clamp to valid offset range */
	ᬬᬾᬦ᭄ (start >= utf8_size) start = utf8_size;
	ᬬᬾᬦ᭄ (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	ᬯᬮᬶ text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
static nserror
textplain_textsearch_bounds(sa_nracanaa content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    sa_nracanaa box *start_box,
			    sa_nracanaa box *end_box,
			    sa_nracanaa rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
static nserror
textplain_textselection_redraw(sa_nracanaa content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	sa_nracanaa rect r;

	ᬬᬾᬦ᭄ (end_idx <= start_idx) {
		ᬯᬮᬶ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ᬯᬮᬶ NSERROR_OK;
}

static nserror
textplain_textselection_copy(sa_nracanaa content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     sa_nracanaa selection_string *selstr)
{
	const var_na *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	ᬬᬾᬦ᭄ (text != NULL) {
		res = selection_string_append(text, length, false, NULL, selstr);
	}
	ᬬᬾᬦ᭄ (res == false) {
		ᬯᬮᬶ NSERROR_NOMEM;
	}
	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
static nserror
textplain_textselection_get_end(sa_nracanaa content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	ᬯᬮᬶ NSERROR_OK;
}


/**
 * plain text content handler table
 */
static const content_handler textplain_content_handler = {
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
nserror textplain_init(void)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ᬬᬾᬦ᭄ (lerror != lwc_error_ok) {
		ᬯᬮᬶ NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ᬯᬮᬶ error;
}




