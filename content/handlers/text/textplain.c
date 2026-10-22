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

#अन्तर्भाव <string.h>
#अन्तर्भाव <parserutils/input/inputstream.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/utf8.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/keypress.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/layout.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "content/textsearch.h"
#अन्तर्भाव "content/handlers/css/utils.h"
#अन्तर्भाव "desktop/selection.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "text/textplain.h"

संरचना textplain_पंक्ति {
	माप_प्रकार	start;
	माप_प्रकार	length;
};

/**
 * plain text content
 */
प्रकारनिर्देश संरचना textplain_content {
	संरचना content base;

	lwc_string *encoding;
	शून्य *inputstream;
	वर्ण *utf8_data;
	माप_प्रकार utf8_data_size;
	माप_प्रकार utf8_data_allocated;
	अचिन्हित दीर्घ physical_पंक्ति_गिनती;
	संरचना textplain_पंक्ति *physical_पंक्ति;
	पूर्णाङ्क formatted_width;
	संरचना browser_window *bw;

	संरचना selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

स्थिर plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

स्थिर पूर्णाङ्क textplain_tab_width = 256;  /* try for a sensible default */

स्थिर lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
स्थिर शून्य textplain_fini(शून्य)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = रिक्त;
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
स्थिर parserutils_error
textplain_charset_hack(ध्रुव uint8_t *data,
		       माप_प्रकार len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	प्रत्यागम PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
स्थिर nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	वर्ण *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = दो_स्मृति(CHUNK);
	यदि (utf8_data == रिक्त)
		गच्छ no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	यदि (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	यदि (error != PARSERUTILS_OK) {
		मुक्त(utf8_data);
		गच्छ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_पंक्ति = 0;
	c->physical_पंक्ति_गिनती = 0;
	c->formatted_width = 0;
	c->bw = रिक्त;
	c->sel = selection_create((संरचना content *)c);

	प्रत्यागम NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);

	प्रत्यागम NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
स्थिर nserror
textplain_create(ध्रुव content_handler *handler,
		 lwc_string *imime_type,
		 ध्रुव संरचना http_parameter *params,
		 llcache_handle *llcache,
		 ध्रुव वर्ण *fallback_charset,
		 bool quirks,
		 संरचना content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = सुस्मृति(1, परिमाण(textplain_content));
	यदि (text == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(text);
		प्रत्यागम error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	यदि (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		मुक्त(text);
		प्रत्यागम error;
	}

	lwc_string_unref(encoding);

	*c = (संरचना content *) text;

	प्रत्यागम NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
स्थिर bool
textplain_copy_utf8_data(textplain_content *c, ध्रुव uint8_t *buf, माप_प्रकार len)
{
	यदि (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		माप_प्रकार allocated;
		वर्ण *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = पुनः_स्मृति(c->utf8_data, allocated);
		यदि (utf8_data == रिक्त)
			प्रत्यागम false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	स_नकल(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	प्रत्यागम true;
}


/**
 * drain input
 */
स्थिर bool
textplain_drain_निवेश(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	स्थिर ध्रुव uint8_t *u_fffd = (ध्रुव uint8_t *) "\xef\xbf\xbd";
	ध्रुव uint8_t *ch;
	माप_प्रकार chlen, offset = 0;

	यावत् (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		यदि (chlen == 1 && *ch == 0) {
			यदि (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				यदि (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					प्रत्यागम false;
			}

			/* Emit U+FFFD */
			यदि (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				प्रत्यागम false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} अन्यथा {
			/* Accumulate input */
			offset += chlen;

			यदि (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				यदि (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					प्रत्यागम false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	यदि (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		यदि (textplain_copy_utf8_data(c, ch, offset) == false)
			प्रत्यागम false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	प्रत्यागम true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
स्थिर bool
textplain_process_data(संरचना content *c, ध्रुव वर्ण *data, अचिन्हित पूर्णाङ्क size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ध्रुव uint8_t *) data, size);
	यदि (error != PARSERUTILS_OK) {
		गच्छ no_memory;
	}

	यदि (textplain_drain_निवेश(text, stream, PARSERUTILS_NEEDDATA) == false)
		गच्छ no_memory;

	प्रत्यागम true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
	प्रत्यागम false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
स्थिर bool textplain_convert(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, रिक्त, 0);
	यदि (error != PARSERUTILS_OK) {
		प्रत्यागम false;
	}

	यदि (textplain_drain_निवेश(text, stream, PARSERUTILS_EOF) == false)
		प्रत्यागम false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = रिक्त;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	प्रत्यागम true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
स्थिर भिन्न textplain_पंक्ति_height(शून्य)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	प्रत्यागम FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
स्थिर शून्य textplain_reformat(संरचना content *c, पूर्णाङ्क width, पूर्णाङ्क height)
{
	textplain_content *text = (textplain_content *) c;
	वर्ण *utf8_data = text->utf8_data;
	माप_प्रकार utf8_data_size = text->utf8_data_size;
	अचिन्हित दीर्घ पंक्ति_गिनती = 0;
	संरचना textplain_पंक्ति *पंक्ति = text->physical_पंक्ति;
	संरचना textplain_पंक्ति *line1;
	माप_प्रकार i, space, col;
	माप_प्रकार columns = 80;
	पूर्णाङ्क character_width;
	माप_प्रकार पंक्ति_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	यदि (res != NSERROR_OK) {
		प्रत्यागम;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_पंक्ति_गिनती = 0;

	यदि (!पंक्ति) {
		text->physical_पंक्ति = पंक्ति =
			दो_स्मृति(परिमाण(संरचना textplain_पंक्ति) * (1024 + 3));
		यदि (!पंक्ति)
			गच्छ no_memory;
	}

	पंक्ति[पंक्ति_गिनती++].start = पंक्ति_start = 0;
	space = 0;
	i = 0;
	col = 0;
	यावत् (i < utf8_data_size) {
		माप_प्रकार csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		माप_प्रकार next_col;
		parserutils_error लिखो_त्रुटि;

		लिखो_त्रुटि = parserutils_charset_utf8_to_ucs4((ध्रुव uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		यदि (लिखो_त्रुटि != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		यदि (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		यदि (term || next_col >= columns) {
			यदि (पंक्ति_गिनती % 1024 == 0) {
				line1 = पुनः_स्मृति(पंक्ति,
						परिमाण(संरचना textplain_पंक्ति) *
						(पंक्ति_गिनती + 1024 + 3));
				यदि (!line1)
					गच्छ no_memory;
				text->physical_पंक्ति = पंक्ति = line1;
			}

			यदि (term) {
				पंक्ति[पंक्ति_गिनती-1].length = i - पंक्ति_start;

				/* skip second char of CR/LF or LF/CR pair */
				यदि (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} अन्यथा {
				यदि (space) {
					/* break at last space in line */
					i = space;
					पंक्ति[पंक्ति_गिनती-1].length = (i + 1) - पंक्ति_start;
				} अन्यथा
					पंक्ति[पंक्ति_गिनती-1].length = i - पंक्ति_start;
			}

			पंक्ति[पंक्ति_गिनती++].start = पंक्ति_start = i + 1;
			col = 0;
			space = 0;
		} अन्यथा {
			col++;
			यदि (chr == ' ')
				space = i;
		}
		i += csize;
	}
	पंक्ति[पंक्ति_गिनती-1].length = i - पंक्ति[पंक्ति_गिनती-1].start;
	पंक्ति[पंक्ति_गिनती].start = utf8_data_size;

	text->physical_पंक्ति_गिनती = पंक्ति_गिनती;
	c->width = width;
	c->height = पंक्ति_गिनती * textplain_पंक्ति_height() + MARGIN + MARGIN;

	प्रत्यागम;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", पंक्ति_गिनती);
	प्रत्यागम;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

स्थिर शून्य textplain_destroy(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	यदि (text->inputstream != रिक्त) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	यदि (text->physical_पंक्ति != रिक्त) {
		मुक्त(text->physical_पंक्ति);
	}

	यदि (text->utf8_data != रिक्त) {
		मुक्त(text->utf8_data);
	}

	यदि (text->sel != रिक्त) {
		selection_destroy(text->sel);
	}
}


स्थिर nserror textplain_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	ध्रुव textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ध्रुव uint8_t *data;
	माप_प्रकार size;

	text = सुस्मृति(1, परिमाण(textplain_content));
	यदि (text == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&text->base);
		प्रत्यागम error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	यदि (error != NSERROR_OK) {
		content_destroy(&text->base);
		प्रत्यागम error;
	}

	data = content__get_source_data(&text->base, &size);
	यदि (size > 0) {
		यदि (textplain_process_data(&text->base,
					   (ध्रुव वर्ण *)data,
					   size) == false) {
			content_destroy(&text->base);
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	यदि (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		यदि (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	प्रत्यागम NSERROR_OK;
}


स्थिर content_type textplain_content_type(शून्य)
{
	प्रत्यागम CONTENT_TEXTPLAIN;
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
स्थिर माप_प्रकार
textplain_offset_from_coords(संरचना content *c, पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क dir)
{
	textplain_content *textc = (textplain_content *) c;
	भिन्न पंक्ति_height = textplain_पंक्ति_height();
	संरचना textplain_पंक्ति *पंक्ति;
	ध्रुव वर्ण *text;
	अचिन्हित nlines;
	माप_प्रकार length;
	पूर्णाङ्क idx;

	प्रतिज्ञा(c != रिक्त);

	y = (पूर्णाङ्क)((भिन्न)(y - MARGIN) / पंक्ति_height);
	x -= MARGIN;

	nlines = textc->physical_पंक्ति_गिनती;
	यदि (!nlines)
		प्रत्यागम 0;

	यदि (y <= 0) y = 0;
	अन्यथा यदि ((अचिन्हित)y >= nlines)
		y = nlines - 1;

	पंक्ति = &textc->physical_पंक्ति[y];
	text = textc->utf8_data + पंक्ति->start;
	length = पंक्ति->length;
	idx = 0;

	यावत् (x > 0) {
		माप_प्रकार next_offset = 0;
		पूर्णाङ्क width = पूर्णांक_उच्च;

		यावत् (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		यदि (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		यदि (x <= width) {
			पूर्णाङ्क pixel_offset;
			माप_प्रकार char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			भङ्ग;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		यदि (x <= width) भङ्ग;

		x -= width;
		length--;
		text++;
		idx++;
	}

	प्रत्यागम पंक्ति->start + idx;
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
स्थिर nserror
textplain_mouse_action(संरचना content *c,
		       संरचना browser_window *bw,
		       browser_mouse_state mouse,
		       पूर्णाङ्क x, पूर्णाङ्क y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	सङ्घ content_संदेश_data संदेश_data;
	ध्रुव वर्ण *status = 0;
	माप_प्रकार idx;
	पूर्णाङ्क dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, रिक्त);

	idx = textplain_offset_from_coords(c, x, y, dir);
	यदि (selection_click(text->sel, text->bw, mouse, idx)) {

		यदि (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, रिक्त);
			status = messages_get("Selecting");
		}

	} अन्यथा {
		यदि (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	संदेश_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_data);

	संदेश_data.pointer = pointer;
	content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_data);

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror
textplain_mouse_track(संरचना content *c,
		      संरचना browser_window *bw,
		      browser_mouse_state mouse,
		      पूर्णाङ्क x, पूर्णाङ्क y)
{
	textplain_content *text = (textplain_content *) c;

	यदि (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		पूर्णाङ्क dir = -1;
		माप_प्रकार idx;

		यदि (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, रिक्त);
	}

	विकल्प (browser_window_get_drag_type(bw)) {

	पक्ष DRAGGING_SELECTION: {
		पूर्णाङ्क dir = -1;
		माप_प्रकार idx;

		यदि (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		भङ्ग;

	स्वभाव:
		textplain_mouse_action(c, bw, mouse, x, y);
		भङ्ग;
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
स्थिर bool textplain_keypress(संरचना content *c, uint32_t कुंजी)
{
	textplain_content *text = (textplain_content *) c;
	संरचना selection *sel = text->sel;

	विकल्प (कुंजी) {
	पक्ष NS_कुंजी_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		प्रत्यागम true;

	पक्ष NS_कुंजी_CLEAR_SELECTION:
		selection_clear(sel, true);
		प्रत्यागम true;

	पक्ष NS_कुंजी_SELECT_ALL:
		selection_select_all(sel);
		प्रत्यागम true;

	पक्ष NS_कुंजी_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		प्रत्यागम selection_clear(sel, true);
	}

	प्रत्यागम false;
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
स्थिर bool
text_draw(ध्रुव वर्ण *utf8_text,
	    माप_प्रकार utf8_len,
	    माप_प्रकार offset,
	    पूर्णाङ्क x,
	    पूर्णाङ्क y,
	    ध्रुव संरचना rect *clip,
	    पूर्णाङ्क height,
	    भिन्न scale,
	    textplain_content *text,
	    ध्रुव संरचना selection *sel,
	    ध्रुव संरचना redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	यदि (ctx->interactive == true) {
		अचिन्हित len = utf8_len;
		अचिन्हित start_idx;
		अचिन्हित end_idx;

		/* first try the browser window's current selection */
		यदि (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		यदि (!highlighted &&
		    (text->base.textsearch.context != रिक्त) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		यदि (highlighted) {
			संरचना rect r;
			अचिन्हित endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			पूर्णाङ्क startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			यदि (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				प्रतिज्ञा(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			यदि (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			यदि (res != NSERROR_OK) {
				endx = 0;
			}

			यदि (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			यदि (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (पूर्णाङ्क)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				यदि (res != NSERROR_OK) {
					प्रत्यागम false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			यदि (res != NSERROR_OK) {
				प्रत्यागम false;
			}

			यदि (start_idx > 0) {
				पूर्णाङ्क px0 = max(x + startx, clip->x0);
				पूर्णाङ्क px1 = min(x + endx, clip->x1);

				यदि (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					यदि (res != NSERROR_OK) {
						प्रत्यागम false;
					}

					clip_changed = true;
				} अन्यथा {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			यदि (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (पूर्णाङ्क)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				प्रत्यागम false;
			}

			/* draw any text succeeding highlighted portion */
			यदि (endtxt_idx < utf8_len) {
				पूर्णाङ्क px0 = max(x + endx, clip->x0);
				यदि (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					यदि (res != NSERROR_OK) {
						प्रत्यागम false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (पूर्णाङ्क)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					यदि (res != NSERROR_OK) {
						प्रत्यागम false;
					}
				}
			}

			यदि (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				प्रत्यागम false;
			}
		}
	}

	यदि (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (पूर्णाङ्क) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		यदि (res != NSERROR_OK) {
			प्रत्यागम false;
		}
	}
	प्रत्यागम true;
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
स्थिर bool
textplain_redraw(संरचना content *c,
		 संरचना content_redraw_data *data,
		 ध्रुव संरचना rect *clip,
		 ध्रुव संरचना redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	संरचना browser_window *bw = text->bw;
	वर्ण *utf8_data = text->utf8_data;
	दीर्घ lineno;
	पूर्णाङ्क x = data->x;
	पूर्णाङ्क y = data->y;
	अचिन्हित दीर्घ पंक्ति_गिनती = text->physical_पंक्ति_गिनती;
	भिन्न पंक्ति_height = textplain_पंक्ति_height();
	भिन्न scaled_पंक्ति_height = पंक्ति_height * data->scale;
	दीर्घ line0 = (clip->y0 - y * data->scale) / scaled_पंक्ति_height - 1;
	दीर्घ line1 = (clip->y1 - y * data->scale) / scaled_पंक्ति_height + 1;
	संरचना textplain_पंक्ति *पंक्ति = text->physical_पंक्ति;
	माप_प्रकार length;
	plot_style_t *plot_style_highlight;
	nserror res;

	यदि (line0 < 0)
		line0 = 0;
	यदि (line1 < 0)
		line1 = 0;
	यदि (पंक्ति_गिनती < (अचिन्हित दीर्घ) line0)
		line0 = पंक्ति_गिनती;
	यदि (पंक्ति_गिनती < (अचिन्हित दीर्घ) line1)
		line1 = पंक्ति_गिनती;
	यदि (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	यदि (!पंक्ति)
		प्रत्यागम true;

	/* choose a suitable background colour for any highlighted text */
	यदि ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	अन्यथा
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	प्रति (lineno = line0; lineno != line1; lineno++) {
		ध्रुव वर्ण *text_d = utf8_data + पंक्ति[lineno].start;
		पूर्णाङ्क tab_width = textplain_tab_width * data->scale;
		माप_प्रकार offset = 0;
		पूर्णाङ्क tx = x;

		यदि (!tab_width) tab_width = 1;

		length = पंक्ति[lineno].length;
		यदि (!length)
			अनुवर्त;

		यावत् (offset < length) {
			माप_प्रकार next_offset = offset;
			पूर्णाङ्क width;
			पूर्णाङ्क ntx;
			nserror res;

			यावत् ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			यदि (!text_draw(text_d + offset,
				       next_offset - offset,
				       पंक्ति[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_पंक्ति_height),
				       clip,
				       पंक्ति_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				प्रत्यागम false;
			}

			यदि (next_offset >= length)
				भङ्ग;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			यदि (res == NSERROR_OK) {
				tx += (पूर्णाङ्क)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			यदि (bw) {
				अचिन्हित tab_ofst = पंक्ति[lineno].start + next_offset;
				संरचना selection *sel = text->sel;
				bool highlighted = false;

				अचिन्हित start_idx, end_idx;
				यदि (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				यदि (!highlighted &&
				    (c->textsearch.context != रिक्त)) {
					अचिन्हित start_idx, end_idx;
					यदि (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				यदि (highlighted) {
					संरचना rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_पंक्ति_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_पंक्ति_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					यदि (res != NSERROR_OK) {
						प्रत्यागम false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	प्रत्यागम true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
स्थिर nserror
textplain_open(संरचना content *c,
	       संरचना browser_window *bw,
	       संरचना content *page,
	       संरचना object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	प्रत्यागम NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
स्थिर nserror textplain_close(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = रिक्त;

	प्रत्यागम NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
स्थिर वर्ण *textplain_get_selection(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	प्रत्यागम selection_get_copy(text->sel);
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
स्थिर पूर्णाङ्क
textplain_coord_from_offset(ध्रुव वर्ण *text, माप_प्रकार offset, माप_प्रकार length)
{
	पूर्णाङ्क x = 0;

	यावत् (offset > 0) {
		माप_प्रकार next_offset = 0;
		पूर्णाङ्क tx;

		यावत् (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		यदि (next_offset >= offset)
			भङ्ग;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	प्रत्यागम x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
स्थिर अचिन्हित दीर्घ textplain_पंक्ति_गिनती(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	प्रतिज्ञा(c != रिक्त);

	प्रत्यागम text->physical_पंक्ति_गिनती;
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
स्थिर वर्ण *
textplain_get_पंक्ति(संरचना content *c,
		   अचिन्हित lineno,
		   माप_प्रकार *poffset,
		   माप_प्रकार *plen)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_पंक्ति *पंक्ति;

	प्रतिज्ञा(c != रिक्त);

	यदि (lineno >= text->physical_पंक्ति_गिनती)
		प्रत्यागम रिक्त;
	पंक्ति = &text->physical_पंक्ति[lineno];

	*poffset = पंक्ति->start;
	*plen = पंक्ति->length;
	प्रत्यागम text->utf8_data + पंक्ति->start;
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
स्थिर पूर्णाङ्क textplain_find_पंक्ति(संरचना content *c, अचिन्हित offset)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_पंक्ति *पंक्ति;
	पूर्णाङ्क nlines;
	पूर्णाङ्क lineno = 0;

	प्रतिज्ञा(c != रिक्त);

	पंक्ति = text->physical_पंक्ति;
	nlines = text->physical_पंक्ति_गिनती;

	यदि (offset > text->utf8_data_size) {
		प्रत्यागम -1;
	}

/* \todo - implement binary search here */
	यावत् (lineno < nlines && पंक्ति[lineno].start < offset) {
		lineno++;
	}
	यदि (पंक्ति[lineno].start > offset) {
		lineno--;
	}

	प्रत्यागम lineno;
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
स्थिर nserror
textplain_textsearch_find(संरचना content *c,
			  संरचना textsearch_context *context,
			  ध्रुव वर्ण *pattern,
			  पूर्णाङ्क p_len,
			  bool case_sens)
{
	पूर्णाङ्क nlines = textplain_पंक्ति_गिनती(c);
	पूर्णाङ्क पंक्ति;
	nserror res = NSERROR_OK;

	प्रति(पंक्ति = 0; पंक्ति < nlines; पंक्ति++) {
		माप_प्रकार offset, length;
		ध्रुव वर्ण *text;

		text = textplain_get_पंक्ति(c, पंक्ति, &offset, &length);
		यदि (text) {
			यावत् (length > 0) {
				अचिन्हित match_length;
				माप_प्रकार start_idx;
				ध्रुव वर्ण *new_text;
				ध्रुव वर्ण *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				यदि (!pos)
					भङ्ग;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						रिक्त,
						रिक्त);
				यदि (res != NSERROR_OK) {
					प्रत्यागम res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	प्रत्यागम res;
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
स्थिर शून्य
textplain_coords_from_range(संरचना content *c,
			    अचिन्हित start,
			    अचिन्हित end,
			    संरचना rect *r)
{
	textplain_content *text = (textplain_content *) c;
	भिन्न पंक्ति_height = textplain_पंक्ति_height();
	वर्ण *utf8_data;
	संरचना textplain_पंक्ति *पंक्ति;
	अचिन्हित lineno = 0;
	अचिन्हित nlines;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(start <= end);
	प्रतिज्ञा(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_पंक्ति_गिनती;
	पंक्ति = text->physical_पंक्ति;

	/* find start */
	lineno = textplain_find_पंक्ति(c, start);

	r->y0 = (पूर्णाङ्क)(MARGIN + lineno * पंक्ति_height);

	यदि (lineno + 1 <= nlines || पंक्ति[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_पंक्ति(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} अन्यथा {
		/* single line */
		ध्रुव वर्ण *text = utf8_data + पंक्ति[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - पंक्ति[lineno].start,
						    पंक्ति[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - पंक्ति[lineno].start,
						    पंक्ति[lineno].length);
	}

	r->y1 = (पूर्णाङ्क)(MARGIN + (lineno + 1) * पंक्ति_height);
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
स्थिर वर्ण *
textplain_get_raw_data(संरचना content *c,
		       अचिन्हित start,
		       अचिन्हित end,
		       माप_प्रकार *plen)
{
	textplain_content *text = (textplain_content *) c;
	माप_प्रकार utf8_size;

	प्रतिज्ञा(c != रिक्त);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	यदि (!utf8_size) प्रत्यागम रिक्त;

	/* clamp to valid offset range */
	यदि (start >= utf8_size) start = utf8_size;
	यदि (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	प्रत्यागम text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
स्थिर nserror
textplain_textsearch_bounds(संरचना content *c,
			    अचिन्हित start_idx,
			    अचिन्हित end_idx,
			    संरचना box *start_box,
			    संरचना box *end_box,
			    संरचना rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	प्रत्यागम NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
स्थिर nserror
textplain_textselection_redraw(संरचना content *c,
			       अचिन्हित start_idx,
			       अचिन्हित end_idx)
{
	संरचना rect r;

	यदि (end_idx <= start_idx) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
textplain_textselection_copy(संरचना content *c,
			     अचिन्हित start_idx,
			     अचिन्हित end_idx,
			     संरचना selection_string *selstr)
{
	ध्रुव वर्ण *text;
	माप_प्रकार length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	यदि (text != रिक्त) {
		res = selection_string_append(text, length, false, रिक्त, selstr);
	}
	यदि (res == false) {
		प्रत्यागम NSERROR_NOMEM;
	}
	प्रत्यागम NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
स्थिर nserror
textplain_textselection_get_end(संरचना content *c, अचिन्हित *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	प्रत्यागम NSERROR_OK;
}


/**
 * plain text content handler table
 */
स्थिर ध्रुव content_handler textplain_content_handler = {
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
	.खोलो = textplain_open,
	.बंद = textplain_close,
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
nserror textplain_init(शून्य)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	यदि (lerror != lwc_error_ok) {
		प्रत्यागम NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	प्रत्यागम error;
}




