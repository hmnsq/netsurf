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

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <parserutils/input/inputstream.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/http.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/utf8.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/keypress.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/layout.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "content/textsearch.h"
#અન્તર્ભાવ "content/handlers/css/utils.h"
#અન્તર્ભાવ "desktop/selection.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "text/textplain.h"

સંરચના textplain_લીટી {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
પ્રકારવ્યાખ્યા સંરચના textplain_content {
	સંરચના content base;

	lwc_string *encoding;
	શૂન્ય *inputstream;
	અક્ષર *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_લીટી_ગણતરી;
	સંરચના textplain_લીટી *physical_લીટી;
	પૂર્ણાંક formatted_width;
	સંરચના browser_window *bw;

	સંરચના selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

સ્થિર plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

સ્થિર પૂર્ણાંક textplain_tab_width = 256;  /* try for a sensible default */

સ્થિર lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
સ્થિર શૂન્ય textplain_fini(શૂન્ય)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ખાલી;
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
સ્થિર parserutils_error
textplain_charset_hack(અચલ uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	પાછા PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
સ્થિર nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	અક્ષર *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	જો (utf8_data == ખાલી)
		જાઓ no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	જો (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	જો (error != PARSERUTILS_OK) {
		મુક્ત(utf8_data);
		જાઓ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_લીટી = 0;
	c->physical_લીટી_ગણતરી = 0;
	c->formatted_width = 0;
	c->bw = ખાલી;
	c->sel = selection_create((સંરચના content *)c);

	પાછા NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);

	પાછા NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
સ્થિર nserror
textplain_create(અચલ content_handler *handler,
		 lwc_string *imime_type,
		 અચલ સંરચના http_parameter *params,
		 llcache_handle *llcache,
		 અચલ અક્ષર *fallback_charset,
		 bool quirks,
		 સંરચના content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, કદ(textplain_content));
	જો (text == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(text);
		પાછા error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	જો (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	જો (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		મુક્ત(text);
		પાછા error;
	}

	lwc_string_unref(encoding);

	*c = (સંરચના content *) text;

	પાછા NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
સ્થિર bool
textplain_copy_utf8_data(textplain_content *c, અચલ uint8_t *buf, size_t len)
{
	જો (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		અક્ષર *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		જો (utf8_data == ખાલી)
			પાછા false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	પાછા true;
}


/**
 * drain input
 */
સ્થિર bool
textplain_drain_આગત(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	સ્થિર અચલ uint8_t *u_fffd = (અચલ uint8_t *) "\xef\xbf\xbd";
	અચલ uint8_t *ch;
	size_t chlen, offset = 0;

	જ્યારે (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		જો (chlen == 1 && *ch == 0) {
			જો (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				જો (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					પાછા false;
			}

			/* Emit U+FFFD */
			જો (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				પાછા false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} નહીંતર {
			/* Accumulate input */
			offset += chlen;

			જો (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				જો (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					પાછા false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	જો (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		જો (textplain_copy_utf8_data(c, ch, offset) == false)
			પાછા false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	પાછા true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
સ્થિર bool
textplain_process_data(સંરચના content *c, અચલ અક્ષર *data, unsigned પૂર્ણાંક size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (અચલ uint8_t *) data, size);
	જો (error != PARSERUTILS_OK) {
		જાઓ no_memory;
	}

	જો (textplain_drain_આગત(text, stream, PARSERUTILS_NEEDDATA) == false)
		જાઓ no_memory;

	પાછા true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
	પાછા false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
સ્થિર bool textplain_convert(સંરચના content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, ખાલી, 0);
	જો (error != PARSERUTILS_OK) {
		પાછા false;
	}

	જો (textplain_drain_આગત(text, stream, PARSERUTILS_EOF) == false)
		પાછા false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = ખાલી;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	પાછા true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
સ્થિર દશાંશ textplain_લીટી_height(શૂન્ય)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	પાછા FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
સ્થિર શૂન્ય textplain_reformat(સંરચના content *c, પૂર્ણાંક width, પૂર્ણાંક height)
{
	textplain_content *text = (textplain_content *) c;
	અક્ષર *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long લીટી_ગણતરી = 0;
	સંરચના textplain_લીટી *લીટી = text->physical_લીટી;
	સંરચના textplain_લીટી *line1;
	size_t i, space, col;
	size_t columns = 80;
	પૂર્ણાંક character_width;
	size_t લીટી_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	જો (res != NSERROR_OK) {
		પાછા;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_લીટી_ગણતરી = 0;

	જો (!લીટી) {
		text->physical_લીટી = લીટી =
			malloc(કદ(સંરચના textplain_લીટી) * (1024 + 3));
		જો (!લીટી)
			જાઓ no_memory;
	}

	લીટી[લીટી_ગણતરી++].start = લીટી_start = 0;
	space = 0;
	i = 0;
	col = 0;
	જ્યારે (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((અચલ uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		જો (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		જો (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		જો (term || next_col >= columns) {
			જો (લીટી_ગણતરી % 1024 == 0) {
				line1 = realloc(લીટી,
						કદ(સંરચના textplain_લીટી) *
						(લીટી_ગણતરી + 1024 + 3));
				જો (!line1)
					જાઓ no_memory;
				text->physical_લીટી = લીટી = line1;
			}

			જો (term) {
				લીટી[લીટી_ગણતરી-1].length = i - લીટી_start;

				/* skip second char of CR/LF or LF/CR pair */
				જો (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} નહીંતર {
				જો (space) {
					/* break at last space in line */
					i = space;
					લીટી[લીટી_ગણતરી-1].length = (i + 1) - લીટી_start;
				} નહીંતર
					લીટી[લીટી_ગણતરી-1].length = i - લીટી_start;
			}

			લીટી[લીટી_ગણતરી++].start = લીટી_start = i + 1;
			col = 0;
			space = 0;
		} નહીંતર {
			col++;
			જો (chr == ' ')
				space = i;
		}
		i += csize;
	}
	લીટી[લીટી_ગણતરી-1].length = i - લીટી[લીટી_ગણતરી-1].start;
	લીટી[લીટી_ગણતરી].start = utf8_data_size;

	text->physical_લીટી_ગણતરી = લીટી_ગણતરી;
	c->width = width;
	c->height = લીટી_ગણતરી * textplain_લીટી_height() + MARGIN + MARGIN;

	પાછા;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", લીટી_ગણતરી);
	પાછા;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

સ્થિર શૂન્ય textplain_destroy(સંરચના content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	જો (text->inputstream != ખાલી) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	જો (text->physical_લીટી != ખાલી) {
		મુક્ત(text->physical_લીટી);
	}

	જો (text->utf8_data != ખાલી) {
		મુક્ત(text->utf8_data);
	}

	જો (text->sel != ખાલી) {
		selection_destroy(text->sel);
	}
}


સ્થિર nserror textplain_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	અચલ textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	અચલ uint8_t *data;
	size_t size;

	text = calloc(1, કદ(textplain_content));
	જો (text == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	જો (error != NSERROR_OK) {
		content_destroy(&text->base);
		પાછા error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	જો (error != NSERROR_OK) {
		content_destroy(&text->base);
		પાછા error;
	}

	data = content__get_source_data(&text->base, &size);
	જો (size > 0) {
		જો (textplain_process_data(&text->base,
					   (અચલ અક્ષર *)data,
					   size) == false) {
			content_destroy(&text->base);
			પાછા NSERROR_NOMEM;
		}
	}

	જો (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		જો (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	પાછા NSERROR_OK;
}


સ્થિર content_type textplain_content_type(શૂન્ય)
{
	પાછા CONTENT_TEXTPLAIN;
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
સ્થિર size_t
textplain_offset_from_coords(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક dir)
{
	textplain_content *textc = (textplain_content *) c;
	દશાંશ લીટી_height = textplain_લીટી_height();
	સંરચના textplain_લીટી *લીટી;
	અચલ અક્ષર *text;
	unsigned nlines;
	size_t length;
	પૂર્ણાંક idx;

	પ્રતિજ્ઞા(c != ખાલી);

	y = (પૂર્ણાંક)((દશાંશ)(y - MARGIN) / લીટી_height);
	x -= MARGIN;

	nlines = textc->physical_લીટી_ગણતરી;
	જો (!nlines)
		પાછા 0;

	જો (y <= 0) y = 0;
	નહીંતર જો ((unsigned)y >= nlines)
		y = nlines - 1;

	લીટી = &textc->physical_લીટી[y];
	text = textc->utf8_data + લીટી->start;
	length = લીટી->length;
	idx = 0;

	જ્યારે (x > 0) {
		size_t next_offset = 0;
		પૂર્ણાંક width = INT_MAX;

		જ્યારે (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		જો (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		જો (x <= width) {
			પૂર્ણાંક pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			તોડો;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		જો (x <= width) તોડો;

		x -= width;
		length--;
		text++;
		idx++;
	}

	પાછા લીટી->start + idx;
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
સ્થિર nserror
textplain_mouse_action(સંરચના content *c,
		       સંરચના browser_window *bw,
		       browser_mouse_state mouse,
		       પૂર્ણાંક x, પૂર્ણાંક y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	સંઘ content_સંદેશ_data સંદેશ_data;
	અચલ અક્ષર *status = 0;
	size_t idx;
	પૂર્ણાંક dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, ખાલી);

	idx = textplain_offset_from_coords(c, x, y, dir);
	જો (selection_click(text->sel, text->bw, mouse, idx)) {

		જો (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, ખાલી);
			status = messages_get("Selecting");
		}

	} નહીંતર {
		જો (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	સંદેશ_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_સંદેશ_STATUS, &સંદેશ_data);

	સંદેશ_data.pointer = pointer;
	content_broadcast(c, CONTENT_સંદેશ_POINTER, &સંદેશ_data);

	પાછા NSERROR_OK;
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
સ્થિર nserror
textplain_mouse_track(સંરચના content *c,
		      સંરચના browser_window *bw,
		      browser_mouse_state mouse,
		      પૂર્ણાંક x, પૂર્ણાંક y)
{
	textplain_content *text = (textplain_content *) c;

	જો (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		પૂર્ણાંક dir = -1;
		size_t idx;

		જો (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, ખાલી);
	}

	પસંદગી (browser_window_get_drag_type(bw)) {

	સ્થિતિ DRAGGING_SELECTION: {
		પૂર્ણાંક dir = -1;
		size_t idx;

		જો (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		તોડો;

	મૂળભૂત:
		textplain_mouse_action(c, bw, mouse, x, y);
		તોડો;
	}

	પાછા NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
સ્થિર bool textplain_keypress(સંરચના content *c, uint32_t ચાવી)
{
	textplain_content *text = (textplain_content *) c;
	સંરચના selection *sel = text->sel;

	પસંદગી (ચાવી) {
	સ્થિતિ NS_ચાવી_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		પાછા true;

	સ્થિતિ NS_ચાવી_CLEAR_SELECTION:
		selection_clear(sel, true);
		પાછા true;

	સ્થિતિ NS_ચાવી_SELECT_ALL:
		selection_select_all(sel);
		પાછા true;

	સ્થિતિ NS_ચાવી_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		પાછા selection_clear(sel, true);
	}

	પાછા false;
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
સ્થિર bool
text_draw(અચલ અક્ષર *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    પૂર્ણાંક x,
	    પૂર્ણાંક y,
	    અચલ સંરચના rect *clip,
	    પૂર્ણાંક height,
	    દશાંશ scale,
	    textplain_content *text,
	    અચલ સંરચના selection *sel,
	    અચલ સંરચના redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	જો (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		જો (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		જો (!highlighted &&
		    (text->base.textsearch.context != ખાલી) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		જો (highlighted) {
			સંરચના rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			પૂર્ણાંક startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			જો (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				પ્રતિજ્ઞા(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			જો (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			જો (res != NSERROR_OK) {
				endx = 0;
			}

			જો (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			જો (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (પૂર્ણાંક)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				જો (res != NSERROR_OK) {
					પાછા false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			જો (res != NSERROR_OK) {
				પાછા false;
			}

			જો (start_idx > 0) {
				પૂર્ણાંક px0 = max(x + startx, clip->x0);
				પૂર્ણાંક px1 = min(x + endx, clip->x1);

				જો (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					જો (res != NSERROR_OK) {
						પાછા false;
					}

					clip_changed = true;
				} નહીંતર {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			જો (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (પૂર્ણાંક)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				પાછા false;
			}

			/* draw any text succeeding highlighted portion */
			જો (endtxt_idx < utf8_len) {
				પૂર્ણાંક px0 = max(x + endx, clip->x0);
				જો (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					જો (res != NSERROR_OK) {
						પાછા false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (પૂર્ણાંક)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					જો (res != NSERROR_OK) {
						પાછા false;
					}
				}
			}

			જો (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				પાછા false;
			}
		}
	}

	જો (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (પૂર્ણાંક) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}
	પાછા true;
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
સ્થિર bool
textplain_redraw(સંરચના content *c,
		 સંરચના content_redraw_data *data,
		 અચલ સંરચના rect *clip,
		 અચલ સંરચના redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	સંરચના browser_window *bw = text->bw;
	અક્ષર *utf8_data = text->utf8_data;
	long lineno;
	પૂર્ણાંક x = data->x;
	પૂર્ણાંક y = data->y;
	unsigned long લીટી_ગણતરી = text->physical_લીટી_ગણતરી;
	દશાંશ લીટી_height = textplain_લીટી_height();
	દશાંશ scaled_લીટી_height = લીટી_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_લીટી_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_લીટી_height + 1;
	સંરચના textplain_લીટી *લીટી = text->physical_લીટી;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	જો (line0 < 0)
		line0 = 0;
	જો (line1 < 0)
		line1 = 0;
	જો (લીટી_ગણતરી < (unsigned long) line0)
		line0 = લીટી_ગણતરી;
	જો (લીટી_ગણતરી < (unsigned long) line1)
		line1 = લીટી_ગણતરી;
	જો (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	જો (!લીટી)
		પાછા true;

	/* choose a suitable background colour for any highlighted text */
	જો ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	નહીંતર
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	માટે (lineno = line0; lineno != line1; lineno++) {
		અચલ અક્ષર *text_d = utf8_data + લીટી[lineno].start;
		પૂર્ણાંક tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		પૂર્ણાંક tx = x;

		જો (!tab_width) tab_width = 1;

		length = લીટી[lineno].length;
		જો (!length)
			ચાલુ;

		જ્યારે (offset < length) {
			size_t next_offset = offset;
			પૂર્ણાંક width;
			પૂર્ણાંક ntx;
			nserror res;

			જ્યારે ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			જો (!text_draw(text_d + offset,
				       next_offset - offset,
				       લીટી[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_લીટી_height),
				       clip,
				       લીટી_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				પાછા false;
			}

			જો (next_offset >= length)
				તોડો;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			જો (res == NSERROR_OK) {
				tx += (પૂર્ણાંક)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			જો (bw) {
				unsigned tab_ofst = લીટી[lineno].start + next_offset;
				સંરચના selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				જો (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				જો (!highlighted &&
				    (c->textsearch.context != ખાલી)) {
					unsigned start_idx, end_idx;
					જો (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				જો (highlighted) {
					સંરચના rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_લીટી_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_લીટી_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					જો (res != NSERROR_OK) {
						પાછા false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	પાછા true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
સ્થિર nserror
textplain_open(સંરચના content *c,
	       સંરચના browser_window *bw,
	       સંરચના content *page,
	       સંરચના object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	પાછા NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
સ્થિર nserror textplain_close(સંરચના content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = ખાલી;

	પાછા NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
સ્થિર અક્ષર *textplain_get_selection(સંરચના content *c)
{
	textplain_content *text = (textplain_content *) c;

	પાછા selection_get_copy(text->sel);
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
સ્થિર પૂર્ણાંક
textplain_coord_from_offset(અચલ અક્ષર *text, size_t offset, size_t length)
{
	પૂર્ણાંક x = 0;

	જ્યારે (offset > 0) {
		size_t next_offset = 0;
		પૂર્ણાંક tx;

		જ્યારે (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		જો (next_offset >= offset)
			તોડો;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	પાછા x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
સ્થિર unsigned long textplain_લીટી_ગણતરી(સંરચના content *c)
{
	textplain_content *text = (textplain_content *) c;

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા text->physical_લીટી_ગણતરી;
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
સ્થિર અક્ષર *
textplain_get_લીટી(સંરચના content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	સંરચના textplain_લીટી *લીટી;

	પ્રતિજ્ઞા(c != ખાલી);

	જો (lineno >= text->physical_લીટી_ગણતરી)
		પાછા ખાલી;
	લીટી = &text->physical_લીટી[lineno];

	*poffset = લીટી->start;
	*plen = લીટી->length;
	પાછા text->utf8_data + લીટી->start;
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
સ્થિર પૂર્ણાંક textplain_find_લીટી(સંરચના content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	સંરચના textplain_લીટી *લીટી;
	પૂર્ણાંક nlines;
	પૂર્ણાંક lineno = 0;

	પ્રતિજ્ઞા(c != ખાલી);

	લીટી = text->physical_લીટી;
	nlines = text->physical_લીટી_ગણતરી;

	જો (offset > text->utf8_data_size) {
		પાછા -1;
	}

/* \todo - implement binary search here */
	જ્યારે (lineno < nlines && લીટી[lineno].start < offset) {
		lineno++;
	}
	જો (લીટી[lineno].start > offset) {
		lineno--;
	}

	પાછા lineno;
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
સ્થિર nserror
textplain_textsearch_find(સંરચના content *c,
			  સંરચના textsearch_context *context,
			  અચલ અક્ષર *pattern,
			  પૂર્ણાંક p_len,
			  bool case_sens)
{
	પૂર્ણાંક nlines = textplain_લીટી_ગણતરી(c);
	પૂર્ણાંક લીટી;
	nserror res = NSERROR_OK;

	માટે(લીટી = 0; લીટી < nlines; લીટી++) {
		size_t offset, length;
		અચલ અક્ષર *text;

		text = textplain_get_લીટી(c, લીટી, &offset, &length);
		જો (text) {
			જ્યારે (length > 0) {
				unsigned match_length;
				size_t start_idx;
				અચલ અક્ષર *new_text;
				અચલ અક્ષર *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				જો (!pos)
					તોડો;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						ખાલી,
						ખાલી);
				જો (res != NSERROR_OK) {
					પાછા res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	પાછા res;
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
સ્થિર શૂન્ય
textplain_coords_from_range(સંરચના content *c,
			    unsigned start,
			    unsigned end,
			    સંરચના rect *r)
{
	textplain_content *text = (textplain_content *) c;
	દશાંશ લીટી_height = textplain_લીટી_height();
	અક્ષર *utf8_data;
	સંરચના textplain_લીટી *લીટી;
	unsigned lineno = 0;
	unsigned nlines;

	પ્રતિજ્ઞા(c != ખાલી);
	પ્રતિજ્ઞા(start <= end);
	પ્રતિજ્ઞા(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_લીટી_ગણતરી;
	લીટી = text->physical_લીટી;

	/* find start */
	lineno = textplain_find_લીટી(c, start);

	r->y0 = (પૂર્ણાંક)(MARGIN + lineno * લીટી_height);

	જો (lineno + 1 <= nlines || લીટી[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_લીટી(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} નહીંતર {
		/* single line */
		અચલ અક્ષર *text = utf8_data + લીટી[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - લીટી[lineno].start,
						    લીટી[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - લીટી[lineno].start,
						    લીટી[lineno].length);
	}

	r->y1 = (પૂર્ણાંક)(MARGIN + (lineno + 1) * લીટી_height);
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
સ્થિર અક્ષર *
textplain_get_raw_data(સંરચના content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	પ્રતિજ્ઞા(c != ખાલી);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	જો (!utf8_size) પાછા ખાલી;

	/* clamp to valid offset range */
	જો (start >= utf8_size) start = utf8_size;
	જો (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	પાછા text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
સ્થિર nserror
textplain_textsearch_bounds(સંરચના content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    સંરચના box *start_box,
			    સંરચના box *end_box,
			    સંરચના rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	પાછા NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
સ્થિર nserror
textplain_textselection_redraw(સંરચના content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	સંરચના rect r;

	જો (end_idx <= start_idx) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	પાછા NSERROR_OK;
}

સ્થિર nserror
textplain_textselection_copy(સંરચના content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     સંરચના selection_string *selstr)
{
	અચલ અક્ષર *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	જો (text != ખાલી) {
		res = selection_string_append(text, length, false, ખાલી, selstr);
	}
	જો (res == false) {
		પાછા NSERROR_NOMEM;
	}
	પાછા NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
સ્થિર nserror
textplain_textselection_get_end(સંરચના content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	પાછા NSERROR_OK;
}


/**
 * plain text content handler table
 */
સ્થિર અચલ content_handler textplain_content_handler = {
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
nserror textplain_init(શૂન્ય)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	જો (lerror != lwc_error_ok) {
		પાછા NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	જો (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	જો (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	પાછા error;
}




