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

#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <parserutils/input/inputstream.h>

#অন্তর্ভাৱ "utils/errors.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/http.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/utf8.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "netsurf/keypress.h"
#অন্তর্ভাৱ "netsurf/browser_window.h"
#অন্তর্ভাৱ "netsurf/plotters.h"
#অন্তর্ভাৱ "netsurf/layout.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "content/hlcache.h"
#অন্তর্ভাৱ "content/textsearch.h"
#অন্তর্ভাৱ "content/handlers/css/utils.h"
#অন্তর্ভাৱ "desktop/selection.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "text/textplain.h"

সংরচনা textplain_সাৰি {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
প্রকারনির্দেশ সংরচনা textplain_content {
	সংরচনা content base;

	lwc_string *encoding;
	শূন্য *inputstream;
	ৱর্ণ *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_সাৰি_গুনতি;
	সংরচনা textplain_সাৰি *physical_সাৰি;
	পূর্ণাঙ্ক formatted_width;
	সংরচনা browser_window *bw;

	সংরচনা selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

স্থির plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

স্থির পূর্ণাঙ্ক textplain_tab_width = 256;  /* try for a sensible default */

স্থির lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
স্থির শূন্য textplain_fini(শূন্য)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = রিক্ত;
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
স্থির parserutils_error
textplain_charset_hack(ধ্রুৱ uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	ঘূৰাই PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
স্থির nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	ৱর্ণ *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	যদি (utf8_data == রিক্ত)
		গচ্ছ no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	যদি (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	যদি (error != PARSERUTILS_OK) {
		মুক্ত(utf8_data);
		গচ্ছ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_সাৰি = 0;
	c->physical_সাৰি_গুনতি = 0;
	c->formatted_width = 0;
	c->bw = রিক্ত;
	c->sel = selection_create((সংরচনা content *)c);

	ঘূৰাই NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);

	ঘূৰাই NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
স্থির nserror
textplain_create(ধ্রুৱ content_handler *handler,
		 lwc_string *imime_type,
		 ধ্রুৱ সংরচনা http_parameter *params,
		 llcache_handle *llcache,
		 ধ্রুৱ ৱর্ণ *fallback_charset,
		 bool quirks,
		 সংরচনা content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, পরিমাণ(textplain_content));
	যদি (text == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(text);
		ঘূৰাই error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	যদি (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		মুক্ত(text);
		ঘূৰাই error;
	}

	lwc_string_unref(encoding);

	*c = (সংরচনা content *) text;

	ঘূৰাই NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
স্থির bool
textplain_copy_utf8_data(textplain_content *c, ধ্রুৱ uint8_t *buf, size_t len)
{
	যদি (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		ৱর্ণ *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		যদি (utf8_data == রিক্ত)
			ঘূৰাই false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	ঘূৰাই true;
}


/**
 * drain input
 */
স্থির bool
textplain_drain_প্ৰবেশ(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	স্থির ধ্রুৱ uint8_t *u_fffd = (ধ্রুৱ uint8_t *) "\xef\xbf\xbd";
	ধ্রুৱ uint8_t *ch;
	size_t chlen, offset = 0;

	যেতিয়ালৈ (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		যদি (chlen == 1 && *ch == 0) {
			যদি (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				যদি (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ঘূৰাই false;
			}

			/* Emit U+FFFD */
			যদি (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				ঘূৰাই false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} অন্যথা {
			/* Accumulate input */
			offset += chlen;

			যদি (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				যদি (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					ঘূৰাই false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	যদি (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		যদি (textplain_copy_utf8_data(c, ch, offset) == false)
			ঘূৰাই false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	ঘূৰাই true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
স্থির bool
textplain_process_data(সংরচনা content *c, ধ্রুৱ ৱর্ণ *data, unsigned পূর্ণাঙ্ক size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ধ্রুৱ uint8_t *) data, size);
	যদি (error != PARSERUTILS_OK) {
		গচ্ছ no_memory;
	}

	যদি (textplain_drain_প্ৰবেশ(text, stream, PARSERUTILS_NEEDDATA) == false)
		গচ্ছ no_memory;

	ঘূৰাই true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
	ঘূৰাই false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
স্থির bool textplain_convert(সংরচনা content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, রিক্ত, 0);
	যদি (error != PARSERUTILS_OK) {
		ঘূৰাই false;
	}

	যদি (textplain_drain_প্ৰবেশ(text, stream, PARSERUTILS_EOF) == false)
		ঘূৰাই false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = রিক্ত;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	ঘূৰাই true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
স্থির ভিন্ন textplain_সাৰি_height(শূন্য)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ঘূৰাই FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
স্থির শূন্য textplain_reformat(সংরচনা content *c, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	textplain_content *text = (textplain_content *) c;
	ৱর্ণ *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long সাৰি_গুনতি = 0;
	সংরচনা textplain_সাৰি *সাৰি = text->physical_সাৰি;
	সংরচনা textplain_সাৰি *line1;
	size_t i, space, col;
	size_t columns = 80;
	পূর্ণাঙ্ক character_width;
	size_t সাৰি_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	যদি (res != NSERROR_OK) {
		ঘূৰাই;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_সাৰি_গুনতি = 0;

	যদি (!সাৰি) {
		text->physical_সাৰি = সাৰি =
			malloc(পরিমাণ(সংরচনা textplain_সাৰি) * (1024 + 3));
		যদি (!সাৰি)
			গচ্ছ no_memory;
	}

	সাৰি[সাৰি_গুনতি++].start = সাৰি_start = 0;
	space = 0;
	i = 0;
	col = 0;
	যেতিয়ালৈ (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ধ্রুৱ uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		যদি (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		যদি (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		যদি (term || next_col >= columns) {
			যদি (সাৰি_গুনতি % 1024 == 0) {
				line1 = realloc(সাৰি,
						পরিমাণ(সংরচনা textplain_সাৰি) *
						(সাৰি_গুনতি + 1024 + 3));
				যদি (!line1)
					গচ্ছ no_memory;
				text->physical_সাৰি = সাৰি = line1;
			}

			যদি (term) {
				সাৰি[সাৰি_গুনতি-1].length = i - সাৰি_start;

				/* skip second char of CR/LF or LF/CR pair */
				যদি (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} অন্যথা {
				যদি (space) {
					/* break at last space in line */
					i = space;
					সাৰি[সাৰি_গুনতি-1].length = (i + 1) - সাৰি_start;
				} অন্যথা
					সাৰি[সাৰি_গুনতি-1].length = i - সাৰি_start;
			}

			সাৰি[সাৰি_গুনতি++].start = সাৰি_start = i + 1;
			col = 0;
			space = 0;
		} অন্যথা {
			col++;
			যদি (chr == ' ')
				space = i;
		}
		i += csize;
	}
	সাৰি[সাৰি_গুনতি-1].length = i - সাৰি[সাৰি_গুনতি-1].start;
	সাৰি[সাৰি_গুনতি].start = utf8_data_size;

	text->physical_সাৰি_গুনতি = সাৰি_গুনতি;
	c->width = width;
	c->height = সাৰি_গুনতি * textplain_সাৰি_height() + MARGIN + MARGIN;

	ঘূৰাই;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", সাৰি_গুনতি);
	ঘূৰাই;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

স্থির শূন্য textplain_destroy(সংরচনা content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	যদি (text->inputstream != রিক্ত) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	যদি (text->physical_সাৰি != রিক্ত) {
		মুক্ত(text->physical_সাৰি);
	}

	যদি (text->utf8_data != রিক্ত) {
		মুক্ত(text->utf8_data);
	}

	যদি (text->sel != রিক্ত) {
		selection_destroy(text->sel);
	}
}


স্থির nserror textplain_clone(ধ্রুৱ সংরচনা content *old, সংরচনা content **newc)
{
	ধ্রুৱ textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ধ্রুৱ uint8_t *data;
	size_t size;

	text = calloc(1, পরিমাণ(textplain_content));
	যদি (text == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&text->base);
		ঘূৰাই error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	যদি (error != NSERROR_OK) {
		content_destroy(&text->base);
		ঘূৰাই error;
	}

	data = content__get_source_data(&text->base, &size);
	যদি (size > 0) {
		যদি (textplain_process_data(&text->base,
					   (ধ্রুৱ ৱর্ণ *)data,
					   size) == false) {
			content_destroy(&text->base);
			ঘূৰাই NSERROR_NOMEM;
		}
	}

	যদি (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		যদি (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			ঘূৰাই NSERROR_CLONE_FAILED;
		}
	}

	ঘূৰাই NSERROR_OK;
}


স্থির content_type textplain_content_type(শূন্য)
{
	ঘূৰাই CONTENT_TEXTPLAIN;
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
স্থির size_t
textplain_offset_from_coords(সংরচনা content *c, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক dir)
{
	textplain_content *textc = (textplain_content *) c;
	ভিন্ন সাৰি_height = textplain_সাৰি_height();
	সংরচনা textplain_সাৰি *সাৰি;
	ধ্রুৱ ৱর্ণ *text;
	unsigned nlines;
	size_t length;
	পূর্ণাঙ্ক idx;

	প্রতিজ্ঞা(c != রিক্ত);

	y = (পূর্ণাঙ্ক)((ভিন্ন)(y - MARGIN) / সাৰি_height);
	x -= MARGIN;

	nlines = textc->physical_সাৰি_গুনতি;
	যদি (!nlines)
		ঘূৰাই 0;

	যদি (y <= 0) y = 0;
	অন্যথা যদি ((unsigned)y >= nlines)
		y = nlines - 1;

	সাৰি = &textc->physical_সাৰি[y];
	text = textc->utf8_data + সাৰি->start;
	length = সাৰি->length;
	idx = 0;

	যেতিয়ালৈ (x > 0) {
		size_t next_offset = 0;
		পূর্ণাঙ্ক width = INT_MAX;

		যেতিয়ালৈ (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		যদি (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		যদি (x <= width) {
			পূর্ণাঙ্ক pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			ভঙ্গ;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		যদি (x <= width) ভঙ্গ;

		x -= width;
		length--;
		text++;
		idx++;
	}

	ঘূৰাই সাৰি->start + idx;
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
স্থির nserror
textplain_mouse_action(সংরচনা content *c,
		       সংরচনা browser_window *bw,
		       browser_mouse_state mouse,
		       পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	ধ্রুৱ ৱর্ণ *status = 0;
	size_t idx;
	পূর্ণাঙ্ক dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, রিক্ত);

	idx = textplain_offset_from_coords(c, x, y, dir);
	যদি (selection_click(text->sel, text->bw, mouse, idx)) {

		যদি (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, রিক্ত);
			status = messages_get("Selecting");
		}

	} অন্যথা {
		যদি (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	বাৰ্তা_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_বাৰ্তা_STATUS, &বাৰ্তা_data);

	বাৰ্তা_data.pointer = pointer;
	content_broadcast(c, CONTENT_বাৰ্তা_POINTER, &বাৰ্তা_data);

	ঘূৰাই NSERROR_OK;
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
স্থির nserror
textplain_mouse_track(সংরচনা content *c,
		      সংরচনা browser_window *bw,
		      browser_mouse_state mouse,
		      পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	textplain_content *text = (textplain_content *) c;

	যদি (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		পূর্ণাঙ্ক dir = -1;
		size_t idx;

		যদি (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, রিক্ত);
	}

	ৱিকল্প (browser_window_get_drag_type(bw)) {

	পক্ষ DRAGGING_SELECTION: {
		পূর্ণাঙ্ক dir = -1;
		size_t idx;

		যদি (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		ভঙ্গ;

	স্ৱভাৱ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ভঙ্গ;
	}

	ঘূৰাই NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
স্থির bool textplain_keypress(সংরচনা content *c, uint32_t চাবি)
{
	textplain_content *text = (textplain_content *) c;
	সংরচনা selection *sel = text->sel;

	ৱিকল্প (চাবি) {
	পক্ষ NS_চাবি_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ঘূৰাই true;

	পক্ষ NS_চাবি_CLEAR_SELECTION:
		selection_clear(sel, true);
		ঘূৰাই true;

	পক্ষ NS_চাবি_SELECT_ALL:
		selection_select_all(sel);
		ঘূৰাই true;

	পক্ষ NS_চাবি_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ঘূৰাই selection_clear(sel, true);
	}

	ঘূৰাই false;
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
স্থির bool
text_draw(ধ্রুৱ ৱর্ণ *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    পূর্ণাঙ্ক x,
	    পূর্ণাঙ্ক y,
	    ধ্রুৱ সংরচনা rect *clip,
	    পূর্ণাঙ্ক height,
	    ভিন্ন scale,
	    textplain_content *text,
	    ধ্রুৱ সংরচনা selection *sel,
	    ধ্রুৱ সংরচনা redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	যদি (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		যদি (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		যদি (!highlighted &&
		    (text->base.textsearch.context != রিক্ত) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		যদি (highlighted) {
			সংরচনা rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			পূর্ণাঙ্ক startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			যদি (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				প্রতিজ্ঞা(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			যদি (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			যদি (res != NSERROR_OK) {
				endx = 0;
			}

			যদি (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			যদি (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (পূর্ণাঙ্ক)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				যদি (res != NSERROR_OK) {
					ঘূৰাই false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			যদি (res != NSERROR_OK) {
				ঘূৰাই false;
			}

			যদি (start_idx > 0) {
				পূর্ণাঙ্ক px0 = max(x + startx, clip->x0);
				পূর্ণাঙ্ক px1 = min(x + endx, clip->x1);

				যদি (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ঘূৰাই false;
					}

					clip_changed = true;
				} অন্যথা {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			যদি (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (পূর্ণাঙ্ক)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ঘূৰাই false;
			}

			/* draw any text succeeding highlighted portion */
			যদি (endtxt_idx < utf8_len) {
				পূর্ণাঙ্ক px0 = max(x + endx, clip->x0);
				যদি (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ঘূৰাই false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (পূর্ণাঙ্ক)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					যদি (res != NSERROR_OK) {
						ঘূৰাই false;
					}
				}
			}

			যদি (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ঘূৰাই false;
			}
		}
	}

	যদি (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (পূর্ণাঙ্ক) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		যদি (res != NSERROR_OK) {
			ঘূৰাই false;
		}
	}
	ঘূৰাই true;
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
স্থির bool
textplain_redraw(সংরচনা content *c,
		 সংরচনা content_redraw_data *data,
		 ধ্রুৱ সংরচনা rect *clip,
		 ধ্রুৱ সংরচনা redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	সংরচনা browser_window *bw = text->bw;
	ৱর্ণ *utf8_data = text->utf8_data;
	long lineno;
	পূর্ণাঙ্ক x = data->x;
	পূর্ণাঙ্ক y = data->y;
	unsigned long সাৰি_গুনতি = text->physical_সাৰি_গুনতি;
	ভিন্ন সাৰি_height = textplain_সাৰি_height();
	ভিন্ন scaled_সাৰি_height = সাৰি_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_সাৰি_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_সাৰি_height + 1;
	সংরচনা textplain_সাৰি *সাৰি = text->physical_সাৰি;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	যদি (line0 < 0)
		line0 = 0;
	যদি (line1 < 0)
		line1 = 0;
	যদি (সাৰি_গুনতি < (unsigned long) line0)
		line0 = সাৰি_গুনতি;
	যদি (সাৰি_গুনতি < (unsigned long) line1)
		line1 = সাৰি_গুনতি;
	যদি (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	যদি (res != NSERROR_OK) {
		ঘূৰাই false;
	}

	যদি (!সাৰি)
		ঘূৰাই true;

	/* choose a suitable background colour for any highlighted text */
	যদি ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	অন্যথা
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	বাবে (lineno = line0; lineno != line1; lineno++) {
		ধ্রুৱ ৱর্ণ *text_d = utf8_data + সাৰি[lineno].start;
		পূর্ণাঙ্ক tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		পূর্ণাঙ্ক tx = x;

		যদি (!tab_width) tab_width = 1;

		length = সাৰি[lineno].length;
		যদি (!length)
			অনুৱর্ত;

		যেতিয়ালৈ (offset < length) {
			size_t next_offset = offset;
			পূর্ণাঙ্ক width;
			পূর্ণাঙ্ক ntx;
			nserror res;

			যেতিয়ালৈ ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			যদি (!text_draw(text_d + offset,
				       next_offset - offset,
				       সাৰি[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_সাৰি_height),
				       clip,
				       সাৰি_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				ঘূৰাই false;
			}

			যদি (next_offset >= length)
				ভঙ্গ;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			যদি (res == NSERROR_OK) {
				tx += (পূর্ণাঙ্ক)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			যদি (bw) {
				unsigned tab_ofst = সাৰি[lineno].start + next_offset;
				সংরচনা selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				যদি (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				যদি (!highlighted &&
				    (c->textsearch.context != রিক্ত)) {
					unsigned start_idx, end_idx;
					যদি (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				যদি (highlighted) {
					সংরচনা rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_সাৰি_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_সাৰি_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					যদি (res != NSERROR_OK) {
						ঘূৰাই false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	ঘূৰাই true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
স্থির nserror
textplain_open(সংরচনা content *c,
	       সংরচনা browser_window *bw,
	       সংরচনা content *page,
	       সংরচনা object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	ঘূৰাই NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
স্থির nserror textplain_close(সংরচনা content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = রিক্ত;

	ঘূৰাই NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
স্থির ৱর্ণ *textplain_get_selection(সংরচনা content *c)
{
	textplain_content *text = (textplain_content *) c;

	ঘূৰাই selection_get_copy(text->sel);
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
স্থির পূর্ণাঙ্ক
textplain_coord_from_offset(ধ্রুৱ ৱর্ণ *text, size_t offset, size_t length)
{
	পূর্ণাঙ্ক x = 0;

	যেতিয়ালৈ (offset > 0) {
		size_t next_offset = 0;
		পূর্ণাঙ্ক tx;

		যেতিয়ালৈ (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		যদি (next_offset >= offset)
			ভঙ্গ;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	ঘূৰাই x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
স্থির unsigned long textplain_সাৰি_গুনতি(সংরচনা content *c)
{
	textplain_content *text = (textplain_content *) c;

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই text->physical_সাৰি_গুনতি;
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
স্থির ৱর্ণ *
textplain_get_সাৰি(সংরচনা content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	সংরচনা textplain_সাৰি *সাৰি;

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (lineno >= text->physical_সাৰি_গুনতি)
		ঘূৰাই রিক্ত;
	সাৰি = &text->physical_সাৰি[lineno];

	*poffset = সাৰি->start;
	*plen = সাৰি->length;
	ঘূৰাই text->utf8_data + সাৰি->start;
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
স্থির পূর্ণাঙ্ক textplain_find_সাৰি(সংরচনা content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	সংরচনা textplain_সাৰি *সাৰি;
	পূর্ণাঙ্ক nlines;
	পূর্ণাঙ্ক lineno = 0;

	প্রতিজ্ঞা(c != রিক্ত);

	সাৰি = text->physical_সাৰি;
	nlines = text->physical_সাৰি_গুনতি;

	যদি (offset > text->utf8_data_size) {
		ঘূৰাই -1;
	}

/* \todo - implement binary search here */
	যেতিয়ালৈ (lineno < nlines && সাৰি[lineno].start < offset) {
		lineno++;
	}
	যদি (সাৰি[lineno].start > offset) {
		lineno--;
	}

	ঘূৰাই lineno;
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
স্থির nserror
textplain_textsearch_find(সংরচনা content *c,
			  সংরচনা textsearch_context *context,
			  ধ্রুৱ ৱর্ণ *pattern,
			  পূর্ণাঙ্ক p_len,
			  bool case_sens)
{
	পূর্ণাঙ্ক nlines = textplain_সাৰি_গুনতি(c);
	পূর্ণাঙ্ক সাৰি;
	nserror res = NSERROR_OK;

	বাবে(সাৰি = 0; সাৰি < nlines; সাৰি++) {
		size_t offset, length;
		ধ্রুৱ ৱর্ণ *text;

		text = textplain_get_সাৰি(c, সাৰি, &offset, &length);
		যদি (text) {
			যেতিয়ালৈ (length > 0) {
				unsigned match_length;
				size_t start_idx;
				ধ্রুৱ ৱর্ণ *new_text;
				ধ্রুৱ ৱর্ণ *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				যদি (!pos)
					ভঙ্গ;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						রিক্ত,
						রিক্ত);
				যদি (res != NSERROR_OK) {
					ঘূৰাই res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	ঘূৰাই res;
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
স্থির শূন্য
textplain_coords_from_range(সংরচনা content *c,
			    unsigned start,
			    unsigned end,
			    সংরচনা rect *r)
{
	textplain_content *text = (textplain_content *) c;
	ভিন্ন সাৰি_height = textplain_সাৰি_height();
	ৱর্ণ *utf8_data;
	সংরচনা textplain_সাৰি *সাৰি;
	unsigned lineno = 0;
	unsigned nlines;

	প্রতিজ্ঞা(c != রিক্ত);
	প্রতিজ্ঞা(start <= end);
	প্রতিজ্ঞা(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_সাৰি_গুনতি;
	সাৰি = text->physical_সাৰি;

	/* find start */
	lineno = textplain_find_সাৰি(c, start);

	r->y0 = (পূর্ণাঙ্ক)(MARGIN + lineno * সাৰি_height);

	যদি (lineno + 1 <= nlines || সাৰি[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_সাৰি(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} অন্যথা {
		/* single line */
		ধ্রুৱ ৱর্ণ *text = utf8_data + সাৰি[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - সাৰি[lineno].start,
						    সাৰি[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - সাৰি[lineno].start,
						    সাৰি[lineno].length);
	}

	r->y1 = (পূর্ণাঙ্ক)(MARGIN + (lineno + 1) * সাৰি_height);
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
স্থির ৱর্ণ *
textplain_get_raw_data(সংরচনা content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	প্রতিজ্ঞা(c != রিক্ত);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	যদি (!utf8_size) ঘূৰাই রিক্ত;

	/* clamp to valid offset range */
	যদি (start >= utf8_size) start = utf8_size;
	যদি (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	ঘূৰাই text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
স্থির nserror
textplain_textsearch_bounds(সংরচনা content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    সংরচনা box *start_box,
			    সংরচনা box *end_box,
			    সংরচনা rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	ঘূৰাই NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
স্থির nserror
textplain_textselection_redraw(সংরচনা content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	সংরচনা rect r;

	যদি (end_idx <= start_idx) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ঘূৰাই NSERROR_OK;
}

স্থির nserror
textplain_textselection_copy(সংরচনা content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     সংরচনা selection_string *selstr)
{
	ধ্রুৱ ৱর্ণ *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	যদি (text != রিক্ত) {
		res = selection_string_append(text, length, false, রিক্ত, selstr);
	}
	যদি (res == false) {
		ঘূৰাই NSERROR_NOMEM;
	}
	ঘূৰাই NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
স্থির nserror
textplain_textselection_get_end(সংরচনা content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	ঘূৰাই NSERROR_OK;
}


/**
 * plain text content handler table
 */
স্থির ধ্রুৱ content_handler textplain_content_handler = {
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
nserror textplain_init(শূন্য)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	যদি (lerror != lwc_error_ok) {
		ঘূৰাই NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ঘূৰাই error;
}




