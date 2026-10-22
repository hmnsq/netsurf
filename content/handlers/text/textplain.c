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

#شامل <string.h>
#شامل <parserutils/input/inputstream.h>

#شامل "utils/errors.h"
#شامل "utils/corestrings.h"
#شامل "utils/http.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/utf8.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/content.h"
#شامل "netsurf/keypress.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/layout.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "content/hlcache.h"
#شامل "content/textsearch.h"
#شامل "content/handlers/css/utils.h"
#شامل "desktop/selection.h"
#شامل "desktop/gui_internal.h"

#شامل "text/textplain.h"

ساختار textplain_سطر {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
تعریفنوع ساختار textplain_content {
	ساختار content base;

	lwc_string *encoding;
	تهی *inputstream;
	نویسه *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_سطر_شمار;
	ساختار textplain_سطر *physical_سطر;
	صحیح formatted_width;
	ساختار browser_window *bw;

	ساختار selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ایستا plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ایستا صحیح textplain_tab_width = 256;  /* try for a sensible default */

ایستا lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ایستا تهی textplain_fini(تهی)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = پوچ;
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
ایستا parserutils_error
textplain_charset_hack(ثابت uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	بازگشت PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ایستا nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	نویسه *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	اگر (utf8_data == پوچ)
		برو no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	اگر (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	اگر (error != PARSERUTILS_OK) {
		آزاد(utf8_data);
		برو no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_سطر = 0;
	c->physical_سطر_شمار = 0;
	c->formatted_width = 0;
	c->bw = پوچ;
	c->sel = selection_create((ساختار content *)c);

	بازگشت NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);

	بازگشت NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ایستا nserror
textplain_create(ثابت content_handler *handler,
		 lwc_string *imime_type,
		 ثابت ساختار http_parameter *params,
		 llcache_handle *llcache,
		 ثابت نویسه *fallback_charset,
		 bool quirks,
		 ساختار content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, اندازه(textplain_content));
	اگر (text == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(text);
		بازگشت error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	اگر (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		آزاد(text);
		بازگشت error;
	}

	lwc_string_unref(encoding);

	*c = (ساختار content *) text;

	بازگشت NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ایستا bool
textplain_copy_utf8_data(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	اگر (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		نویسه *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		اگر (utf8_data == پوچ)
			بازگشت false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	بازگشت true;
}


/**
 * drain input
 */
ایستا bool
textplain_drain_درون_داد(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	ایستا ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, offset = 0;

	وقتی (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		اگر (chlen == 1 && *ch == 0) {
			اگر (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				اگر (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					بازگشت false;
			}

			/* Emit U+FFFD */
			اگر (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				بازگشت false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} وگرنه {
			/* Accumulate input */
			offset += chlen;

			اگر (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				اگر (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					بازگشت false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	اگر (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		اگر (textplain_copy_utf8_data(c, ch, offset) == false)
			بازگشت false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	بازگشت true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ایستا bool
textplain_process_data(ساختار content *c, ثابت نویسه *data, unsigned صحیح size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ثابت uint8_t *) data, size);
	اگر (error != PARSERUTILS_OK) {
		برو no_memory;
	}

	اگر (textplain_drain_درون_داد(text, stream, PARSERUTILS_NEEDDATA) == false)
		برو no_memory;

	بازگشت true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, پوچ);
	بازگشت false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ایستا bool textplain_convert(ساختار content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, پوچ, 0);
	اگر (error != PARSERUTILS_OK) {
		بازگشت false;
	}

	اگر (textplain_drain_درون_داد(text, stream, PARSERUTILS_EOF) == false)
		بازگشت false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = پوچ;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	بازگشت true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ایستا اعشاری textplain_سطر_height(تهی)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	بازگشت FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ایستا تهی textplain_reformat(ساختار content *c, صحیح width, صحیح height)
{
	textplain_content *text = (textplain_content *) c;
	نویسه *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long سطر_شمار = 0;
	ساختار textplain_سطر *سطر = text->physical_سطر;
	ساختار textplain_سطر *line1;
	size_t i, space, col;
	size_t columns = 80;
	صحیح character_width;
	size_t سطر_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	اگر (res != NSERROR_OK) {
		بازگشت;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_سطر_شمار = 0;

	اگر (!سطر) {
		text->physical_سطر = سطر =
			malloc(اندازه(ساختار textplain_سطر) * (1024 + 3));
		اگر (!سطر)
			برو no_memory;
	}

	سطر[سطر_شمار++].start = سطر_start = 0;
	space = 0;
	i = 0;
	col = 0;
	وقتی (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ثابت uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		اگر (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		اگر (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		اگر (term || next_col >= columns) {
			اگر (سطر_شمار % 1024 == 0) {
				line1 = realloc(سطر,
						اندازه(ساختار textplain_سطر) *
						(سطر_شمار + 1024 + 3));
				اگر (!line1)
					برو no_memory;
				text->physical_سطر = سطر = line1;
			}

			اگر (term) {
				سطر[سطر_شمار-1].length = i - سطر_start;

				/* skip second char of CR/LF or LF/CR pair */
				اگر (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} وگرنه {
				اگر (space) {
					/* break at last space in line */
					i = space;
					سطر[سطر_شمار-1].length = (i + 1) - سطر_start;
				} وگرنه
					سطر[سطر_شمار-1].length = i - سطر_start;
			}

			سطر[سطر_شمار++].start = سطر_start = i + 1;
			col = 0;
			space = 0;
		} وگرنه {
			col++;
			اگر (chr == ' ')
				space = i;
		}
		i += csize;
	}
	سطر[سطر_شمار-1].length = i - سطر[سطر_شمار-1].start;
	سطر[سطر_شمار].start = utf8_data_size;

	text->physical_سطر_شمار = سطر_شمار;
	c->width = width;
	c->height = سطر_شمار * textplain_سطر_height() + MARGIN + MARGIN;

	بازگشت;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", سطر_شمار);
	بازگشت;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ایستا تهی textplain_destroy(ساختار content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	اگر (text->inputstream != پوچ) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	اگر (text->physical_سطر != پوچ) {
		آزاد(text->physical_سطر);
	}

	اگر (text->utf8_data != پوچ) {
		آزاد(text->utf8_data);
	}

	اگر (text->sel != پوچ) {
		selection_destroy(text->sel);
	}
}


ایستا nserror textplain_clone(ثابت ساختار content *old, ساختار content **newc)
{
	ثابت textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ثابت uint8_t *data;
	size_t size;

	text = calloc(1, اندازه(textplain_content));
	اگر (text == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&text->base);
		بازگشت error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	اگر (error != NSERROR_OK) {
		content_destroy(&text->base);
		بازگشت error;
	}

	data = content__get_source_data(&text->base, &size);
	اگر (size > 0) {
		اگر (textplain_process_data(&text->base,
					   (ثابت نویسه *)data,
					   size) == false) {
			content_destroy(&text->base);
			بازگشت NSERROR_NOMEM;
		}
	}

	اگر (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		اگر (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	بازگشت NSERROR_OK;
}


ایستا content_type textplain_content_type(تهی)
{
	بازگشت CONTENT_TEXTPLAIN;
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
ایستا size_t
textplain_offset_from_coords(ساختار content *c, صحیح x, صحیح y, صحیح dir)
{
	textplain_content *textc = (textplain_content *) c;
	اعشاری سطر_height = textplain_سطر_height();
	ساختار textplain_سطر *سطر;
	ثابت نویسه *text;
	unsigned nlines;
	size_t length;
	صحیح idx;

	أكد(c != پوچ);

	y = (صحیح)((اعشاری)(y - MARGIN) / سطر_height);
	x -= MARGIN;

	nlines = textc->physical_سطر_شمار;
	اگر (!nlines)
		بازگشت 0;

	اگر (y <= 0) y = 0;
	وگرنه اگر ((unsigned)y >= nlines)
		y = nlines - 1;

	سطر = &textc->physical_سطر[y];
	text = textc->utf8_data + سطر->start;
	length = سطر->length;
	idx = 0;

	وقتی (x > 0) {
		size_t next_offset = 0;
		صحیح width = INT_MAX;

		وقتی (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		اگر (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		اگر (x <= width) {
			صحیح pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			بشکن;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		اگر (x <= width) بشکن;

		x -= width;
		length--;
		text++;
		idx++;
	}

	بازگشت سطر->start + idx;
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
ایستا nserror
textplain_mouse_action(ساختار content *c,
		       ساختار browser_window *bw,
		       browser_mouse_state mouse,
		       صحیح x, صحیح y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	اجتماع content_پیام_data پیام_data;
	ثابت نویسه *status = 0;
	size_t idx;
	صحیح dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, پوچ);

	idx = textplain_offset_from_coords(c, x, y, dir);
	اگر (selection_click(text->sel, text->bw, mouse, idx)) {

		اگر (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, پوچ);
			status = messages_get("Selecting");
		}

	} وگرنه {
		اگر (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	پیام_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_پیام_STATUS, &پیام_data);

	پیام_data.pointer = pointer;
	content_broadcast(c, CONTENT_پیام_POINTER, &پیام_data);

	بازگشت NSERROR_OK;
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
ایستا nserror
textplain_mouse_track(ساختار content *c,
		      ساختار browser_window *bw,
		      browser_mouse_state mouse,
		      صحیح x, صحیح y)
{
	textplain_content *text = (textplain_content *) c;

	اگر (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحیح dir = -1;
		size_t idx;

		اگر (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, پوچ);
	}

	انتخاب (browser_window_get_drag_type(bw)) {

	حالت DRAGGING_SELECTION: {
		صحیح dir = -1;
		size_t idx;

		اگر (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		بشکن;

	پیشفرض:
		textplain_mouse_action(c, bw, mouse, x, y);
		بشکن;
	}

	بازگشت NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ایستا bool textplain_keypress(ساختار content *c, uint32_t کلید)
{
	textplain_content *text = (textplain_content *) c;
	ساختار selection *sel = text->sel;

	انتخاب (کلید) {
	حالت NS_کلید_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		بازگشت true;

	حالت NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		بازگشت true;

	حالت NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		بازگشت true;

	حالت NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		بازگشت selection_clear(sel, true);
	}

	بازگشت false;
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
ایستا bool
text_draw(ثابت نویسه *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    صحیح x,
	    صحیح y,
	    ثابت ساختار rect *clip,
	    صحیح height,
	    اعشاری scale,
	    textplain_content *text,
	    ثابت ساختار selection *sel,
	    ثابت ساختار redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	اگر (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		اگر (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		اگر (!highlighted &&
		    (text->base.textsearch.context != پوچ) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		اگر (highlighted) {
			ساختار rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			صحیح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			اگر (end_idx > utf8_len) {
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
			اگر (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			اگر (res != NSERROR_OK) {
				endx = 0;
			}

			اگر (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			اگر (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحیح)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				اگر (res != NSERROR_OK) {
					بازگشت false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}

			اگر (start_idx > 0) {
				صحیح px0 = max(x + startx, clip->x0);
				صحیح px1 = min(x + endx, clip->x1);

				اگر (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}

					clip_changed = true;
				} وگرنه {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			اگر (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحیح)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				بازگشت false;
			}

			/* draw any text succeeding highlighted portion */
			اگر (endtxt_idx < utf8_len) {
				صحیح px0 = max(x + endx, clip->x0);
				اگر (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحیح)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}
				}
			}

			اگر (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				بازگشت false;
			}
		}
	}

	اگر (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحیح) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}
	بازگشت true;
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
ایستا bool
textplain_redraw(ساختار content *c,
		 ساختار content_redraw_data *data,
		 ثابت ساختار rect *clip,
		 ثابت ساختار redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	ساختار browser_window *bw = text->bw;
	نویسه *utf8_data = text->utf8_data;
	long lineno;
	صحیح x = data->x;
	صحیح y = data->y;
	unsigned long سطر_شمار = text->physical_سطر_شمار;
	اعشاری سطر_height = textplain_سطر_height();
	اعشاری scaled_سطر_height = سطر_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_سطر_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_سطر_height + 1;
	ساختار textplain_سطر *سطر = text->physical_سطر;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	اگر (line0 < 0)
		line0 = 0;
	اگر (line1 < 0)
		line1 = 0;
	اگر (سطر_شمار < (unsigned long) line0)
		line0 = سطر_شمار;
	اگر (سطر_شمار < (unsigned long) line1)
		line1 = سطر_شمار;
	اگر (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	اگر (!سطر)
		بازگشت true;

	/* choose a suitable background colour for any highlighted text */
	اگر ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وگرنه
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	برای (lineno = line0; lineno != line1; lineno++) {
		ثابت نویسه *text_d = utf8_data + سطر[lineno].start;
		صحیح tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		صحیح tx = x;

		اگر (!tab_width) tab_width = 1;

		length = سطر[lineno].length;
		اگر (!length)
			ادامه;

		وقتی (offset < length) {
			size_t next_offset = offset;
			صحیح width;
			صحیح ntx;
			nserror res;

			وقتی ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			اگر (!text_draw(text_d + offset,
				       next_offset - offset,
				       سطر[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_سطر_height),
				       clip,
				       سطر_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				بازگشت false;
			}

			اگر (next_offset >= length)
				بشکن;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			اگر (res == NSERROR_OK) {
				tx += (صحیح)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			اگر (bw) {
				unsigned tab_ofst = سطر[lineno].start + next_offset;
				ساختار selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				اگر (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				اگر (!highlighted &&
				    (c->textsearch.context != پوچ)) {
					unsigned start_idx, end_idx;
					اگر (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				اگر (highlighted) {
					ساختار rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_سطر_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_سطر_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	بازگشت true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ایستا nserror
textplain_open(ساختار content *c,
	       ساختار browser_window *bw,
	       ساختار content *page,
	       ساختار object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	بازگشت NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ایستا nserror textplain_close(ساختار content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = پوچ;

	بازگشت NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ایستا نویسه *textplain_get_selection(ساختار content *c)
{
	textplain_content *text = (textplain_content *) c;

	بازگشت selection_get_copy(text->sel);
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
ایستا صحیح
textplain_coord_from_offset(ثابت نویسه *text, size_t offset, size_t length)
{
	صحیح x = 0;

	وقتی (offset > 0) {
		size_t next_offset = 0;
		صحیح tx;

		وقتی (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		اگر (next_offset >= offset)
			بشکن;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	بازگشت x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ایستا unsigned long textplain_سطر_شمار(ساختار content *c)
{
	textplain_content *text = (textplain_content *) c;

	أكد(c != پوچ);

	بازگشت text->physical_سطر_شمار;
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
ایستا نویسه *
textplain_get_سطر(ساختار content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	ساختار textplain_سطر *سطر;

	أكد(c != پوچ);

	اگر (lineno >= text->physical_سطر_شمار)
		بازگشت پوچ;
	سطر = &text->physical_سطر[lineno];

	*poffset = سطر->start;
	*plen = سطر->length;
	بازگشت text->utf8_data + سطر->start;
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
ایستا صحیح textplain_find_سطر(ساختار content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	ساختار textplain_سطر *سطر;
	صحیح nlines;
	صحیح lineno = 0;

	أكد(c != پوچ);

	سطر = text->physical_سطر;
	nlines = text->physical_سطر_شمار;

	اگر (offset > text->utf8_data_size) {
		بازگشت -1;
	}

/* \todo - implement binary search here */
	وقتی (lineno < nlines && سطر[lineno].start < offset) {
		lineno++;
	}
	اگر (سطر[lineno].start > offset) {
		lineno--;
	}

	بازگشت lineno;
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
ایستا nserror
textplain_textsearch_find(ساختار content *c,
			  ساختار textsearch_context *context,
			  ثابت نویسه *pattern,
			  صحیح p_len,
			  bool case_sens)
{
	صحیح nlines = textplain_سطر_شمار(c);
	صحیح سطر;
	nserror res = NSERROR_OK;

	برای(سطر = 0; سطر < nlines; سطر++) {
		size_t offset, length;
		ثابت نویسه *text;

		text = textplain_get_سطر(c, سطر, &offset, &length);
		اگر (text) {
			وقتی (length > 0) {
				unsigned match_length;
				size_t start_idx;
				ثابت نویسه *new_text;
				ثابت نویسه *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				اگر (!pos)
					بشکن;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						پوچ,
						پوچ);
				اگر (res != NSERROR_OK) {
					بازگشت res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	بازگشت res;
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
ایستا تهی
textplain_coords_from_range(ساختار content *c,
			    unsigned start,
			    unsigned end,
			    ساختار rect *r)
{
	textplain_content *text = (textplain_content *) c;
	اعشاری سطر_height = textplain_سطر_height();
	نویسه *utf8_data;
	ساختار textplain_سطر *سطر;
	unsigned lineno = 0;
	unsigned nlines;

	أكد(c != پوچ);
	أكد(start <= end);
	أكد(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_سطر_شمار;
	سطر = text->physical_سطر;

	/* find start */
	lineno = textplain_find_سطر(c, start);

	r->y0 = (صحیح)(MARGIN + lineno * سطر_height);

	اگر (lineno + 1 <= nlines || سطر[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_سطر(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} وگرنه {
		/* single line */
		ثابت نویسه *text = utf8_data + سطر[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - سطر[lineno].start,
						    سطر[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - سطر[lineno].start,
						    سطر[lineno].length);
	}

	r->y1 = (صحیح)(MARGIN + (lineno + 1) * سطر_height);
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
ایستا نویسه *
textplain_get_raw_data(ساختار content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	أكد(c != پوچ);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	اگر (!utf8_size) بازگشت پوچ;

	/* clamp to valid offset range */
	اگر (start >= utf8_size) start = utf8_size;
	اگر (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	بازگشت text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
ایستا nserror
textplain_textsearch_bounds(ساختار content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    ساختار box *start_box,
			    ساختار box *end_box,
			    ساختار rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	بازگشت NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ایستا nserror
textplain_textselection_redraw(ساختار content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	ساختار rect r;

	اگر (end_idx <= start_idx) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	بازگشت NSERROR_OK;
}

ایستا nserror
textplain_textselection_copy(ساختار content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     ساختار selection_string *selstr)
{
	ثابت نویسه *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	اگر (text != پوچ) {
		res = selection_string_append(text, length, false, پوچ, selstr);
	}
	اگر (res == false) {
		بازگشت NSERROR_NOMEM;
	}
	بازگشت NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ایستا nserror
textplain_textselection_get_end(ساختار content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	بازگشت NSERROR_OK;
}


/**
 * plain text content handler table
 */
ایستا ثابت content_handler textplain_content_handler = {
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
nserror textplain_init(تهی)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	اگر (lerror != lwc_error_ok) {
		بازگشت NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	بازگشت error;
}




