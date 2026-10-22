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

#ضمن <string.h>
#ضمن <parserutils/input/inputstream.h>

#ضمن "utils/errors.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/http.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/utf8.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/keypress.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/layout.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "content/hlcache.h"
#ضمن "content/textsearch.h"
#ضمن "content/handlers/css/utils.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/gui_internal.h"

#ضمن "text/textplain.h"

بنية textplain_line {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
تعريفنوع بنية textplain_content {
	بنية content base;

	lwc_string *encoding;
	فراغ *inputstream;
	حرف *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_line_count;
	بنية textplain_line *physical_line;
	صحيح formatted_width;
	بنية browser_window *bw;

	بنية selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساكن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساكن صحيح textplain_tab_width = 256;  /* try for a sensible default */

ساكن lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ساكن فراغ textplain_fini(فراغ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = عدم;
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
ساكن parserutils_error
textplain_charset_hack(ثابت uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	أرجع PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ساكن nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	𐤀𐤌 (utf8_data == عدم)
		اذهب no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	𐤀𐤌 (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	𐤀𐤌 (error != PARSERUTILS_OK) {
		حرر(utf8_data);
		اذهب no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_line = 0;
	c->physical_line_count = 0;
	c->formatted_width = 0;
	c->bw = عدم;
	c->sel = selection_create((بنية content *)c);

	أرجع NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);

	أرجع NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ساكن nserror
textplain_create(ثابت content_handler *handler,
		 lwc_string *imime_type,
		 ثابت بنية http_parameter *params,
		 llcache_handle *llcache,
		 ثابت حرف *fallback_charset,
		 bool quirks,
		 بنية content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, حجم(textplain_content));
	𐤀𐤌 (text == عدم) {
		أرجع NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	𐤀𐤌 (error != NSERROR_OK) {
		حرر(text);
		أرجع error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	𐤀𐤌 (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	𐤀𐤌 (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		حرر(text);
		أرجع error;
	}

	lwc_string_unref(encoding);

	*c = (بنية content *) text;

	أرجع NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ساكن bool
textplain_copy_utf8_data(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	𐤀𐤌 (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		𐤀𐤌 (utf8_data == عدم)
			أرجع false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	أرجع true;
}


/**
 * drain input
 */
ساكن bool
textplain_drain_input(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	ساكن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, offset = 0;

	طالما (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		𐤀𐤌 (chlen == 1 && *ch == 0) {
			𐤀𐤌 (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				𐤀𐤌 (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					أرجع false;
			}

			/* Emit U+FFFD */
			𐤀𐤌 (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				أرجع false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} وإلا {
			/* Accumulate input */
			offset += chlen;

			𐤀𐤌 (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				𐤀𐤌 (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					أرجع false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	𐤀𐤌 (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		𐤀𐤌 (textplain_copy_utf8_data(c, ch, offset) == false)
			أرجع false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	أرجع true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساكن bool
textplain_process_data(بنية content *c, ثابت حرف *data, unsigned صحيح size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ثابت uint8_t *) data, size);
	𐤀𐤌 (error != PARSERUTILS_OK) {
		اذهب no_memory;
	}

	𐤀𐤌 (textplain_drain_input(text, stream, PARSERUTILS_NEEDDATA) == false)
		اذهب no_memory;

	أرجع true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, عدم);
	أرجع false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساكن bool textplain_convert(بنية content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, عدم, 0);
	𐤀𐤌 (error != PARSERUTILS_OK) {
		أرجع false;
	}

	𐤀𐤌 (textplain_drain_input(text, stream, PARSERUTILS_EOF) == false)
		أرجع false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = عدم;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	أرجع true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساكن عائم textplain_line_height(فراغ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	أرجع FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساكن فراغ textplain_reformat(بنية content *c, صحيح width, صحيح height)
{
	textplain_content *text = (textplain_content *) c;
	حرف *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long line_count = 0;
	بنية textplain_line *line = text->physical_line;
	بنية textplain_line *line1;
	size_t i, space, col;
	size_t columns = 80;
	صحيح character_width;
	size_t line_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	𐤀𐤌 (res != NSERROR_OK) {
		أرجع;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_line_count = 0;

	𐤀𐤌 (!line) {
		text->physical_line = line =
			malloc(حجم(بنية textplain_line) * (1024 + 3));
		𐤀𐤌 (!line)
			اذهب no_memory;
	}

	line[line_count++].start = line_start = 0;
	space = 0;
	i = 0;
	col = 0;
	طالما (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ثابت uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		𐤀𐤌 (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		𐤀𐤌 (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		𐤀𐤌 (term || next_col >= columns) {
			𐤀𐤌 (line_count % 1024 == 0) {
				line1 = realloc(line,
						حجم(بنية textplain_line) *
						(line_count + 1024 + 3));
				𐤀𐤌 (!line1)
					اذهب no_memory;
				text->physical_line = line = line1;
			}

			𐤀𐤌 (term) {
				line[line_count-1].length = i - line_start;

				/* skip second char of CR/LF or LF/CR pair */
				𐤀𐤌 (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} وإلا {
				𐤀𐤌 (space) {
					/* break at last space in line */
					i = space;
					line[line_count-1].length = (i + 1) - line_start;
				} وإلا
					line[line_count-1].length = i - line_start;
			}

			line[line_count++].start = line_start = i + 1;
			col = 0;
			space = 0;
		} وإلا {
			col++;
			𐤀𐤌 (chr == ' ')
				space = i;
		}
		i += csize;
	}
	line[line_count-1].length = i - line[line_count-1].start;
	line[line_count].start = utf8_data_size;

	text->physical_line_count = line_count;
	c->width = width;
	c->height = line_count * textplain_line_height() + MARGIN + MARGIN;

	أرجع;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", line_count);
	أرجع;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ساكن فراغ textplain_destroy(بنية content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	𐤀𐤌 (text->inputstream != عدم) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	𐤀𐤌 (text->physical_line != عدم) {
		حرر(text->physical_line);
	}

	𐤀𐤌 (text->utf8_data != عدم) {
		حرر(text->utf8_data);
	}

	𐤀𐤌 (text->sel != عدم) {
		selection_destroy(text->sel);
	}
}


ساكن nserror textplain_clone(ثابت بنية content *old, بنية content **newc)
{
	ثابت textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ثابت uint8_t *data;
	size_t size;

	text = calloc(1, حجم(textplain_content));
	𐤀𐤌 (text == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&text->base);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&text->base);
		أرجع error;
	}

	data = content__get_source_data(&text->base, &size);
	𐤀𐤌 (size > 0) {
		𐤀𐤌 (textplain_process_data(&text->base,
					   (ثابت حرف *)data,
					   size) == false) {
			content_destroy(&text->base);
			أرجع NSERROR_NOMEM;
		}
	}

	𐤀𐤌 (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		𐤀𐤌 (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	أرجع NSERROR_OK;
}


ساكن content_type textplain_content_type(فراغ)
{
	أرجع CONTENT_TEXTPLAIN;
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
ساكن size_t
textplain_offset_from_coords(بنية content *c, صحيح x, صحيح y, صحيح dir)
{
	textplain_content *textc = (textplain_content *) c;
	عائم line_height = textplain_line_height();
	بنية textplain_line *line;
	ثابت حرف *text;
	unsigned nlines;
	size_t length;
	صحيح idx;

	أكد(c != عدم);

	y = (صحيح)((عائم)(y - MARGIN) / line_height);
	x -= MARGIN;

	nlines = textc->physical_line_count;
	𐤀𐤌 (!nlines)
		أرجع 0;

	𐤀𐤌 (y <= 0) y = 0;
	وإلا 𐤀𐤌 ((unsigned)y >= nlines)
		y = nlines - 1;

	line = &textc->physical_line[y];
	text = textc->utf8_data + line->start;
	length = line->length;
	idx = 0;

	طالما (x > 0) {
		size_t next_offset = 0;
		صحيح width = INT_MAX;

		طالما (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		𐤀𐤌 (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		𐤀𐤌 (x <= width) {
			صحيح pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			اكسر;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		𐤀𐤌 (x <= width) اكسر;

		x -= width;
		length--;
		text++;
		idx++;
	}

	أرجع line->start + idx;
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
ساكن nserror
textplain_mouse_action(بنية content *c,
		       بنية browser_window *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	اتحاد content_msg_data msg_data;
	ثابت حرف *status = 0;
	size_t idx;
	صحيح dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, عدم);

	idx = textplain_offset_from_coords(c, x, y, dir);
	𐤀𐤌 (selection_click(text->sel, text->bw, mouse, idx)) {

		𐤀𐤌 (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, عدم);
			status = messages_get("Selecting");
		}

	} وإلا {
		𐤀𐤌 (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	msg_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

	msg_data.pointer = pointer;
	content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);

	أرجع NSERROR_OK;
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
ساكن nserror
textplain_mouse_track(بنية content *c,
		      بنية browser_window *bw,
		      browser_mouse_state mouse,
		      صحيح x, صحيح y)
{
	textplain_content *text = (textplain_content *) c;

	𐤀𐤌 (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحيح dir = -1;
		size_t idx;

		𐤀𐤌 (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, عدم);
	}

	بدل (browser_window_get_drag_type(bw)) {

	حالة DRAGGING_SELECTION: {
		صحيح dir = -1;
		size_t idx;

		𐤀𐤌 (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		اكسر;

	افتراضي:
		textplain_mouse_action(c, bw, mouse, x, y);
		اكسر;
	}

	أرجع NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ساكن bool textplain_keypress(بنية content *c, uint32_t key)
{
	textplain_content *text = (textplain_content *) c;
	بنية selection *sel = text->sel;

	بدل (key) {
	حالة NS_KEY_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		أرجع true;

	حالة NS_KEY_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_KEY_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_KEY_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		أرجع selection_clear(sel, true);
	}

	أرجع false;
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
ساكن bool
text_draw(ثابت حرف *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح height,
	    عائم scale,
	    textplain_content *text,
	    ثابت بنية selection *sel,
	    ثابت بنية redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	𐤀𐤌 (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		𐤀𐤌 (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		𐤀𐤌 (!highlighted &&
		    (text->base.textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		𐤀𐤌 (highlighted) {
			بنية rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			𐤀𐤌 (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			𐤀𐤌 (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			𐤀𐤌 (res != NSERROR_OK) {
				endx = 0;
			}

			𐤀𐤌 (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			𐤀𐤌 (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحيح)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				𐤀𐤌 (res != NSERROR_OK) {
					أرجع false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			𐤀𐤌 (res != NSERROR_OK) {
				أرجع false;
			}

			𐤀𐤌 (start_idx > 0) {
				صحيح px0 = max(x + startx, clip->x0);
				صحيح px1 = min(x + endx, clip->x1);

				𐤀𐤌 (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					𐤀𐤌 (res != NSERROR_OK) {
						أرجع false;
					}

					clip_changed = true;
				} وإلا {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			𐤀𐤌 (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				أرجع false;
			}

			/* draw any text succeeding highlighted portion */
			𐤀𐤌 (endtxt_idx < utf8_len) {
				صحيح px0 = max(x + endx, clip->x0);
				𐤀𐤌 (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					𐤀𐤌 (res != NSERROR_OK) {
						أرجع false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					𐤀𐤌 (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			𐤀𐤌 (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	𐤀𐤌 (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		𐤀𐤌 (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
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
ساكن bool
textplain_redraw(بنية content *c,
		 بنية content_redraw_data *data,
		 ثابت بنية rect *clip,
		 ثابت بنية redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	بنية browser_window *bw = text->bw;
	حرف *utf8_data = text->utf8_data;
	long lineno;
	صحيح x = data->x;
	صحيح y = data->y;
	unsigned long line_count = text->physical_line_count;
	عائم line_height = textplain_line_height();
	عائم scaled_line_height = line_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_line_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_line_height + 1;
	بنية textplain_line *line = text->physical_line;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	𐤀𐤌 (line0 < 0)
		line0 = 0;
	𐤀𐤌 (line1 < 0)
		line1 = 0;
	𐤀𐤌 (line_count < (unsigned long) line0)
		line0 = line_count;
	𐤀𐤌 (line_count < (unsigned long) line1)
		line1 = line_count;
	𐤀𐤌 (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	𐤀𐤌 (res != NSERROR_OK) {
		أرجع false;
	}

	𐤀𐤌 (!line)
		أرجع true;

	/* choose a suitable background colour for any highlighted text */
	𐤀𐤌 ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وإلا
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	لكل (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *text_d = utf8_data + line[lineno].start;
		صحيح tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		صحيح tx = x;

		𐤀𐤌 (!tab_width) tab_width = 1;

		length = line[lineno].length;
		𐤀𐤌 (!length)
			استمر;

		طالما (offset < length) {
			size_t next_offset = offset;
			صحيح width;
			صحيح ntx;
			nserror res;

			طالما ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			𐤀𐤌 (!text_draw(text_d + offset,
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
				أرجع false;
			}

			𐤀𐤌 (next_offset >= length)
				اكسر;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			𐤀𐤌 (res == NSERROR_OK) {
				tx += (صحيح)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			𐤀𐤌 (bw) {
				unsigned tab_ofst = line[lineno].start + next_offset;
				بنية selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				𐤀𐤌 (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				𐤀𐤌 (!highlighted &&
				    (c->textsearch.context != عدم)) {
					unsigned start_idx, end_idx;
					𐤀𐤌 (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				𐤀𐤌 (highlighted) {
					بنية rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_line_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_line_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					𐤀𐤌 (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	أرجع true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ساكن nserror
textplain_open(بنية content *c,
	       بنية browser_window *bw,
	       بنية content *page,
	       بنية object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	أرجع NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ساكن nserror textplain_close(بنية content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = عدم;

	أرجع NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ساكن حرف *textplain_get_selection(بنية content *c)
{
	textplain_content *text = (textplain_content *) c;

	أرجع selection_get_copy(text->sel);
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
ساكن صحيح
textplain_coord_from_offset(ثابت حرف *text, size_t offset, size_t length)
{
	صحيح x = 0;

	طالما (offset > 0) {
		size_t next_offset = 0;
		صحيح tx;

		طالما (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		𐤀𐤌 (next_offset >= offset)
			اكسر;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	أرجع x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساكن unsigned long textplain_line_count(بنية content *c)
{
	textplain_content *text = (textplain_content *) c;

	أكد(c != عدم);

	أرجع text->physical_line_count;
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
ساكن حرف *
textplain_get_line(بنية content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	بنية textplain_line *line;

	أكد(c != عدم);

	𐤀𐤌 (lineno >= text->physical_line_count)
		أرجع عدم;
	line = &text->physical_line[lineno];

	*poffset = line->start;
	*plen = line->length;
	أرجع text->utf8_data + line->start;
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
ساكن صحيح textplain_find_line(بنية content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	بنية textplain_line *line;
	صحيح nlines;
	صحيح lineno = 0;

	أكد(c != عدم);

	line = text->physical_line;
	nlines = text->physical_line_count;

	𐤀𐤌 (offset > text->utf8_data_size) {
		أرجع -1;
	}

/* \todo - implement binary search here */
	طالما (lineno < nlines && line[lineno].start < offset) {
		lineno++;
	}
	𐤀𐤌 (line[lineno].start > offset) {
		lineno--;
	}

	أرجع lineno;
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
ساكن nserror
textplain_textsearch_find(بنية content *c,
			  بنية textsearch_context *context,
			  ثابت حرف *pattern,
			  صحيح p_len,
			  bool case_sens)
{
	صحيح nlines = textplain_line_count(c);
	صحيح line;
	nserror res = NSERROR_OK;

	لكل(line = 0; line < nlines; line++) {
		size_t offset, length;
		ثابت حرف *text;

		text = textplain_get_line(c, line, &offset, &length);
		𐤀𐤌 (text) {
			طالما (length > 0) {
				unsigned match_length;
				size_t start_idx;
				ثابت حرف *new_text;
				ثابت حرف *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				𐤀𐤌 (!pos)
					اكسر;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						عدم,
						عدم);
				𐤀𐤌 (res != NSERROR_OK) {
					أرجع res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	أرجع res;
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
ساكن فراغ
textplain_coords_from_range(بنية content *c,
			    unsigned start,
			    unsigned end,
			    بنية rect *r)
{
	textplain_content *text = (textplain_content *) c;
	عائم line_height = textplain_line_height();
	حرف *utf8_data;
	بنية textplain_line *line;
	unsigned lineno = 0;
	unsigned nlines;

	أكد(c != عدم);
	أكد(start <= end);
	أكد(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_line_count;
	line = text->physical_line;

	/* find start */
	lineno = textplain_find_line(c, start);

	r->y0 = (صحيح)(MARGIN + lineno * line_height);

	𐤀𐤌 (lineno + 1 <= nlines || line[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_line(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} وإلا {
		/* single line */
		ثابت حرف *text = utf8_data + line[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - line[lineno].start,
						    line[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - line[lineno].start,
						    line[lineno].length);
	}

	r->y1 = (صحيح)(MARGIN + (lineno + 1) * line_height);
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
ساكن حرف *
textplain_get_raw_data(بنية content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	أكد(c != عدم);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	𐤀𐤌 (!utf8_size) أرجع عدم;

	/* clamp to valid offset range */
	𐤀𐤌 (start >= utf8_size) start = utf8_size;
	𐤀𐤌 (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	أرجع text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
ساكن nserror
textplain_textsearch_bounds(بنية content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    بنية box *start_box,
			    بنية box *end_box,
			    بنية rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	أرجع NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساكن nserror
textplain_textselection_redraw(بنية content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	بنية rect r;

	𐤀𐤌 (end_idx <= start_idx) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	أرجع NSERROR_OK;
}

ساكن nserror
textplain_textselection_copy(بنية content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     بنية selection_string *selstr)
{
	ثابت حرف *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	𐤀𐤌 (text != عدم) {
		res = selection_string_append(text, length, false, عدم, selstr);
	}
	𐤀𐤌 (res == false) {
		أرجع NSERROR_NOMEM;
	}
	أرجع NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ساكن nserror
textplain_textselection_get_end(بنية content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	أرجع NSERROR_OK;
}


/**
 * plain text content handler table
 */
ساكن ثابت content_handler textplain_content_handler = {
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
nserror textplain_init(فراغ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	𐤀𐤌 (lerror != lwc_error_ok) {
		أرجع NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	𐤀𐤌 (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	𐤀𐤌 (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	أرجع error;
}




