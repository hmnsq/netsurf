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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <parserutils/input/inputstream.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/utf8.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/keypress.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/layout.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "content/textsearch.h"
#அந்தர்பாவ "content/handlers/css/utils.h"
#அந்தர்பாவ "desktop/selection.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "text/textplain.h"

கட்டமைப்பு textplain_வரி {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
வகைவரை கட்டமைப்பு textplain_content {
	கட்டமைப்பு content base;

	lwc_string *encoding;
	வெறுமை *inputstream;
	எழுத்து *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_வரி_மொத்தம்;
	கட்டமைப்பு textplain_வரி *physical_வரி;
	முழு formatted_width;
	கட்டமைப்பு browser_window *bw;

	கட்டமைப்பு selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

நிலையான plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

நிலையான முழு textplain_tab_width = 256;  /* try for a sensible default */

நிலையான lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
நிலையான வெறுமை textplain_fini(வெறுமை)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = வெற்று;
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
நிலையான parserutils_error
textplain_charset_hack(மாறிலி uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	திரும்பு PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
நிலையான nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	எழுத்து *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	எனில் (utf8_data == வெற்று)
		செல் no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	எனில் (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	எனில் (error != PARSERUTILS_OK) {
		விடுவி(utf8_data);
		செல் no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_வரி = 0;
	c->physical_வரி_மொத்தம் = 0;
	c->formatted_width = 0;
	c->bw = வெற்று;
	c->sel = selection_create((கட்டமைப்பு content *)c);

	திரும்பு NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);

	திரும்பு NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
நிலையான nserror
textplain_create(மாறிலி content_handler *handler,
		 lwc_string *imime_type,
		 மாறிலி கட்டமைப்பு http_parameter *params,
		 llcache_handle *llcache,
		 மாறிலி எழுத்து *fallback_charset,
		 bool quirks,
		 கட்டமைப்பு content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, அளவு(textplain_content));
	எனில் (text == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(text);
		திரும்பு error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	எனில் (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	எனில் (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		விடுவி(text);
		திரும்பு error;
	}

	lwc_string_unref(encoding);

	*c = (கட்டமைப்பு content *) text;

	திரும்பு NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
நிலையான bool
textplain_copy_utf8_data(textplain_content *c, மாறிலி uint8_t *buf, size_t len)
{
	எனில் (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		எழுத்து *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		எனில் (utf8_data == வெற்று)
			திரும்பு false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	திரும்பு true;
}


/**
 * drain input
 */
நிலையான bool
textplain_drain_உள்ளீடு(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	நிலையான மாறிலி uint8_t *u_fffd = (மாறிலி uint8_t *) "\xef\xbf\xbd";
	மாறிலி uint8_t *ch;
	size_t chlen, offset = 0;

	போது (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		எனில் (chlen == 1 && *ch == 0) {
			எனில் (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				எனில் (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					திரும்பு false;
			}

			/* Emit U+FFFD */
			எனில் (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				திரும்பு false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} இல்லையெனில் {
			/* Accumulate input */
			offset += chlen;

			எனில் (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				எனில் (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					திரும்பு false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	எனில் (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		எனில் (textplain_copy_utf8_data(c, ch, offset) == false)
			திரும்பு false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	திரும்பு true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
நிலையான bool
textplain_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data, unsigned முழு size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (மாறிலி uint8_t *) data, size);
	எனில் (error != PARSERUTILS_OK) {
		செல் no_memory;
	}

	எனில் (textplain_drain_உள்ளீடு(text, stream, PARSERUTILS_NEEDDATA) == false)
		செல் no_memory;

	திரும்பு true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
	திரும்பு false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
நிலையான bool textplain_convert(கட்டமைப்பு content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, வெற்று, 0);
	எனில் (error != PARSERUTILS_OK) {
		திரும்பு false;
	}

	எனில் (textplain_drain_உள்ளீடு(text, stream, PARSERUTILS_EOF) == false)
		திரும்பு false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = வெற்று;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	திரும்பு true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
நிலையான தசம textplain_வரி_height(வெறுமை)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	திரும்பு FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
நிலையான வெறுமை textplain_reformat(கட்டமைப்பு content *c, முழு width, முழு height)
{
	textplain_content *text = (textplain_content *) c;
	எழுத்து *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long வரி_மொத்தம் = 0;
	கட்டமைப்பு textplain_வரி *வரி = text->physical_வரி;
	கட்டமைப்பு textplain_வரி *line1;
	size_t i, space, col;
	size_t columns = 80;
	முழு character_width;
	size_t வரி_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	எனில் (res != NSERROR_OK) {
		திரும்பு;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_வரி_மொத்தம் = 0;

	எனில் (!வரி) {
		text->physical_வரி = வரி =
			malloc(அளவு(கட்டமைப்பு textplain_வரி) * (1024 + 3));
		எனில் (!வரி)
			செல் no_memory;
	}

	வரி[வரி_மொத்தம்++].start = வரி_start = 0;
	space = 0;
	i = 0;
	col = 0;
	போது (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((மாறிலி uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		எனில் (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		எனில் (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		எனில் (term || next_col >= columns) {
			எனில் (வரி_மொத்தம் % 1024 == 0) {
				line1 = realloc(வரி,
						அளவு(கட்டமைப்பு textplain_வரி) *
						(வரி_மொத்தம் + 1024 + 3));
				எனில் (!line1)
					செல் no_memory;
				text->physical_வரி = வரி = line1;
			}

			எனில் (term) {
				வரி[வரி_மொத்தம்-1].length = i - வரி_start;

				/* skip second char of CR/LF or LF/CR pair */
				எனில் (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} இல்லையெனில் {
				எனில் (space) {
					/* break at last space in line */
					i = space;
					வரி[வரி_மொத்தம்-1].length = (i + 1) - வரி_start;
				} இல்லையெனில்
					வரி[வரி_மொத்தம்-1].length = i - வரி_start;
			}

			வரி[வரி_மொத்தம்++].start = வரி_start = i + 1;
			col = 0;
			space = 0;
		} இல்லையெனில் {
			col++;
			எனில் (chr == ' ')
				space = i;
		}
		i += csize;
	}
	வரி[வரி_மொத்தம்-1].length = i - வரி[வரி_மொத்தம்-1].start;
	வரி[வரி_மொத்தம்].start = utf8_data_size;

	text->physical_வரி_மொத்தம் = வரி_மொத்தம்;
	c->width = width;
	c->height = வரி_மொத்தம் * textplain_வரி_height() + MARGIN + MARGIN;

	திரும்பு;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", வரி_மொத்தம்);
	திரும்பு;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

நிலையான வெறுமை textplain_destroy(கட்டமைப்பு content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	எனில் (text->inputstream != வெற்று) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	எனில் (text->physical_வரி != வெற்று) {
		விடுவி(text->physical_வரி);
	}

	எனில் (text->utf8_data != வெற்று) {
		விடுவி(text->utf8_data);
	}

	எனில் (text->sel != வெற்று) {
		selection_destroy(text->sel);
	}
}


நிலையான nserror textplain_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	மாறிலி textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	மாறிலி uint8_t *data;
	size_t size;

	text = calloc(1, அளவு(textplain_content));
	எனில் (text == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&text->base);
		திரும்பு error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	எனில் (error != NSERROR_OK) {
		content_destroy(&text->base);
		திரும்பு error;
	}

	data = content__get_source_data(&text->base, &size);
	எனில் (size > 0) {
		எனில் (textplain_process_data(&text->base,
					   (மாறிலி எழுத்து *)data,
					   size) == false) {
			content_destroy(&text->base);
			திரும்பு NSERROR_NOMEM;
		}
	}

	எனில் (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		எனில் (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	திரும்பு NSERROR_OK;
}


நிலையான content_type textplain_content_type(வெறுமை)
{
	திரும்பு CONTENT_TEXTPLAIN;
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
நிலையான size_t
textplain_offset_from_coords(கட்டமைப்பு content *c, முழு x, முழு y, முழு dir)
{
	textplain_content *textc = (textplain_content *) c;
	தசம வரி_height = textplain_வரி_height();
	கட்டமைப்பு textplain_வரி *வரி;
	மாறிலி எழுத்து *text;
	unsigned nlines;
	size_t length;
	முழு idx;

	ப்ரதிஜ்ஞா(c != வெற்று);

	y = (முழு)((தசம)(y - MARGIN) / வரி_height);
	x -= MARGIN;

	nlines = textc->physical_வரி_மொத்தம்;
	எனில் (!nlines)
		திரும்பு 0;

	எனில் (y <= 0) y = 0;
	இல்லையெனில் எனில் ((unsigned)y >= nlines)
		y = nlines - 1;

	வரி = &textc->physical_வரி[y];
	text = textc->utf8_data + வரி->start;
	length = வரி->length;
	idx = 0;

	போது (x > 0) {
		size_t next_offset = 0;
		முழு width = INT_MAX;

		போது (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		எனில் (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		எனில் (x <= width) {
			முழு pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			நிறுத்து;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		எனில் (x <= width) நிறுத்து;

		x -= width;
		length--;
		text++;
		idx++;
	}

	திரும்பு வரி->start + idx;
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
நிலையான nserror
textplain_mouse_action(கட்டமைப்பு content *c,
		       கட்டமைப்பு browser_window *bw,
		       browser_mouse_state mouse,
		       முழு x, முழு y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	மாறிலி எழுத்து *status = 0;
	size_t idx;
	முழு dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, வெற்று);

	idx = textplain_offset_from_coords(c, x, y, dir);
	எனில் (selection_click(text->sel, text->bw, mouse, idx)) {

		எனில் (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, வெற்று);
			status = messages_get("Selecting");
		}

	} இல்லையெனில் {
		எனில் (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	செய்தி_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_செய்தி_STATUS, &செய்தி_data);

	செய்தி_data.pointer = pointer;
	content_broadcast(c, CONTENT_செய்தி_POINTER, &செய்தி_data);

	திரும்பு NSERROR_OK;
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
நிலையான nserror
textplain_mouse_track(கட்டமைப்பு content *c,
		      கட்டமைப்பு browser_window *bw,
		      browser_mouse_state mouse,
		      முழு x, முழு y)
{
	textplain_content *text = (textplain_content *) c;

	எனில் (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		முழு dir = -1;
		size_t idx;

		எனில் (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, வெற்று);
	}

	தேர்வு (browser_window_get_drag_type(bw)) {

	நிலை DRAGGING_SELECTION: {
		முழு dir = -1;
		size_t idx;

		எனில் (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		நிறுத்து;

	இயல்பு:
		textplain_mouse_action(c, bw, mouse, x, y);
		நிறுத்து;
	}

	திரும்பு NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
நிலையான bool textplain_keypress(கட்டமைப்பு content *c, uint32_t சாவி)
{
	textplain_content *text = (textplain_content *) c;
	கட்டமைப்பு selection *sel = text->sel;

	தேர்வு (சாவி) {
	நிலை NS_சாவி_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		திரும்பு true;

	நிலை NS_சாவி_CLEAR_SELECTION:
		selection_clear(sel, true);
		திரும்பு true;

	நிலை NS_சாவி_SELECT_ALL:
		selection_select_all(sel);
		திரும்பு true;

	நிலை NS_சாவி_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		திரும்பு selection_clear(sel, true);
	}

	திரும்பு false;
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
நிலையான bool
text_draw(மாறிலி எழுத்து *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    முழு x,
	    முழு y,
	    மாறிலி கட்டமைப்பு rect *clip,
	    முழு height,
	    தசம scale,
	    textplain_content *text,
	    மாறிலி கட்டமைப்பு selection *sel,
	    மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	எனில் (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		எனில் (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		எனில் (!highlighted &&
		    (text->base.textsearch.context != வெற்று) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		எனில் (highlighted) {
			கட்டமைப்பு rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			முழு startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			எனில் (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ப்ரதிஜ்ஞா(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			எனில் (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			எனில் (res != NSERROR_OK) {
				endx = 0;
			}

			எனில் (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			எனில் (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (முழு)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				எனில் (res != NSERROR_OK) {
					திரும்பு false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}

			எனில் (start_idx > 0) {
				முழு px0 = max(x + startx, clip->x0);
				முழு px1 = min(x + endx, clip->x1);

				எனில் (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}

					clip_changed = true;
				} இல்லையெனில் {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			எனில் (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (முழு)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				திரும்பு false;
			}

			/* draw any text succeeding highlighted portion */
			எனில் (endtxt_idx < utf8_len) {
				முழு px0 = max(x + endx, clip->x0);
				எனில் (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (முழு)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}
				}
			}

			எனில் (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				திரும்பு false;
			}
		}
	}

	எனில் (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (முழு) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}
	திரும்பு true;
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
நிலையான bool
textplain_redraw(கட்டமைப்பு content *c,
		 கட்டமைப்பு content_redraw_data *data,
		 மாறிலி கட்டமைப்பு rect *clip,
		 மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	கட்டமைப்பு browser_window *bw = text->bw;
	எழுத்து *utf8_data = text->utf8_data;
	long lineno;
	முழு x = data->x;
	முழு y = data->y;
	unsigned long வரி_மொத்தம் = text->physical_வரி_மொத்தம்;
	தசம வரி_height = textplain_வரி_height();
	தசம scaled_வரி_height = வரி_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_வரி_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_வரி_height + 1;
	கட்டமைப்பு textplain_வரி *வரி = text->physical_வரி;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	எனில் (line0 < 0)
		line0 = 0;
	எனில் (line1 < 0)
		line1 = 0;
	எனில் (வரி_மொத்தம் < (unsigned long) line0)
		line0 = வரி_மொத்தம்;
	எனில் (வரி_மொத்தம் < (unsigned long) line1)
		line1 = வரி_மொத்தம்;
	எனில் (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	எனில் (!வரி)
		திரும்பு true;

	/* choose a suitable background colour for any highlighted text */
	எனில் ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	இல்லையெனில்
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	க்கு (lineno = line0; lineno != line1; lineno++) {
		மாறிலி எழுத்து *text_d = utf8_data + வரி[lineno].start;
		முழு tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		முழு tx = x;

		எனில் (!tab_width) tab_width = 1;

		length = வரி[lineno].length;
		எனில் (!length)
			தொடர்;

		போது (offset < length) {
			size_t next_offset = offset;
			முழு width;
			முழு ntx;
			nserror res;

			போது ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			எனில் (!text_draw(text_d + offset,
				       next_offset - offset,
				       வரி[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_வரி_height),
				       clip,
				       வரி_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				திரும்பு false;
			}

			எனில் (next_offset >= length)
				நிறுத்து;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			எனில் (res == NSERROR_OK) {
				tx += (முழு)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			எனில் (bw) {
				unsigned tab_ofst = வரி[lineno].start + next_offset;
				கட்டமைப்பு selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				எனில் (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				எனில் (!highlighted &&
				    (c->textsearch.context != வெற்று)) {
					unsigned start_idx, end_idx;
					எனில் (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				எனில் (highlighted) {
					கட்டமைப்பு rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_வரி_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_வரி_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	திரும்பு true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
நிலையான nserror
textplain_open(கட்டமைப்பு content *c,
	       கட்டமைப்பு browser_window *bw,
	       கட்டமைப்பு content *page,
	       கட்டமைப்பு object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	திரும்பு NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
நிலையான nserror textplain_close(கட்டமைப்பு content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = வெற்று;

	திரும்பு NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
நிலையான எழுத்து *textplain_get_selection(கட்டமைப்பு content *c)
{
	textplain_content *text = (textplain_content *) c;

	திரும்பு selection_get_copy(text->sel);
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
நிலையான முழு
textplain_coord_from_offset(மாறிலி எழுத்து *text, size_t offset, size_t length)
{
	முழு x = 0;

	போது (offset > 0) {
		size_t next_offset = 0;
		முழு tx;

		போது (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		எனில் (next_offset >= offset)
			நிறுத்து;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	திரும்பு x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
நிலையான unsigned long textplain_வரி_மொத்தம்(கட்டமைப்பு content *c)
{
	textplain_content *text = (textplain_content *) c;

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு text->physical_வரி_மொத்தம்;
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
நிலையான எழுத்து *
textplain_get_வரி(கட்டமைப்பு content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	கட்டமைப்பு textplain_வரி *வரி;

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (lineno >= text->physical_வரி_மொத்தம்)
		திரும்பு வெற்று;
	வரி = &text->physical_வரி[lineno];

	*poffset = வரி->start;
	*plen = வரி->length;
	திரும்பு text->utf8_data + வரி->start;
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
நிலையான முழு textplain_find_வரி(கட்டமைப்பு content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	கட்டமைப்பு textplain_வரி *வரி;
	முழு nlines;
	முழு lineno = 0;

	ப்ரதிஜ்ஞா(c != வெற்று);

	வரி = text->physical_வரி;
	nlines = text->physical_வரி_மொத்தம்;

	எனில் (offset > text->utf8_data_size) {
		திரும்பு -1;
	}

/* \todo - implement binary search here */
	போது (lineno < nlines && வரி[lineno].start < offset) {
		lineno++;
	}
	எனில் (வரி[lineno].start > offset) {
		lineno--;
	}

	திரும்பு lineno;
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
நிலையான nserror
textplain_textsearch_find(கட்டமைப்பு content *c,
			  கட்டமைப்பு textsearch_context *context,
			  மாறிலி எழுத்து *pattern,
			  முழு p_len,
			  bool case_sens)
{
	முழு nlines = textplain_வரி_மொத்தம்(c);
	முழு வரி;
	nserror res = NSERROR_OK;

	க்கு(வரி = 0; வரி < nlines; வரி++) {
		size_t offset, length;
		மாறிலி எழுத்து *text;

		text = textplain_get_வரி(c, வரி, &offset, &length);
		எனில் (text) {
			போது (length > 0) {
				unsigned match_length;
				size_t start_idx;
				மாறிலி எழுத்து *new_text;
				மாறிலி எழுத்து *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				எனில் (!pos)
					நிறுத்து;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						வெற்று,
						வெற்று);
				எனில் (res != NSERROR_OK) {
					திரும்பு res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	திரும்பு res;
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
நிலையான வெறுமை
textplain_coords_from_range(கட்டமைப்பு content *c,
			    unsigned start,
			    unsigned end,
			    கட்டமைப்பு rect *r)
{
	textplain_content *text = (textplain_content *) c;
	தசம வரி_height = textplain_வரி_height();
	எழுத்து *utf8_data;
	கட்டமைப்பு textplain_வரி *வரி;
	unsigned lineno = 0;
	unsigned nlines;

	ப்ரதிஜ்ஞா(c != வெற்று);
	ப்ரதிஜ்ஞா(start <= end);
	ப்ரதிஜ்ஞா(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_வரி_மொத்தம்;
	வரி = text->physical_வரி;

	/* find start */
	lineno = textplain_find_வரி(c, start);

	r->y0 = (முழு)(MARGIN + lineno * வரி_height);

	எனில் (lineno + 1 <= nlines || வரி[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_வரி(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} இல்லையெனில் {
		/* single line */
		மாறிலி எழுத்து *text = utf8_data + வரி[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - வரி[lineno].start,
						    வரி[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - வரி[lineno].start,
						    வரி[lineno].length);
	}

	r->y1 = (முழு)(MARGIN + (lineno + 1) * வரி_height);
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
நிலையான எழுத்து *
textplain_get_raw_data(கட்டமைப்பு content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	ப்ரதிஜ்ஞா(c != வெற்று);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	எனில் (!utf8_size) திரும்பு வெற்று;

	/* clamp to valid offset range */
	எனில் (start >= utf8_size) start = utf8_size;
	எனில் (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	திரும்பு text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
நிலையான nserror
textplain_textsearch_bounds(கட்டமைப்பு content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    கட்டமைப்பு box *start_box,
			    கட்டமைப்பு box *end_box,
			    கட்டமைப்பு rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	திரும்பு NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
நிலையான nserror
textplain_textselection_redraw(கட்டமைப்பு content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	கட்டமைப்பு rect r;

	எனில் (end_idx <= start_idx) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	திரும்பு NSERROR_OK;
}

நிலையான nserror
textplain_textselection_copy(கட்டமைப்பு content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     கட்டமைப்பு selection_string *selstr)
{
	மாறிலி எழுத்து *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	எனில் (text != வெற்று) {
		res = selection_string_append(text, length, false, வெற்று, selstr);
	}
	எனில் (res == false) {
		திரும்பு NSERROR_NOMEM;
	}
	திரும்பு NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
நிலையான nserror
textplain_textselection_get_end(கட்டமைப்பு content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	திரும்பு NSERROR_OK;
}


/**
 * plain text content handler table
 */
நிலையான மாறிலி content_handler textplain_content_handler = {
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
nserror textplain_init(வெறுமை)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	எனில் (lerror != lwc_error_ok) {
		திரும்பு NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	எனில் (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	எனில் (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	திரும்பு error;
}




