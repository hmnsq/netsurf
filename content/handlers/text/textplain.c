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

संरचना textplain_ओळ {
	माप_प्रकार	start;
	माप_प्रकार	length;
};

/**
 * plain text content
 */
प्रकारव्याख्या संरचना textplain_content {
	संरचना content base;

	lwc_string *encoding;
	शून्य *inputstream;
	अक्षर *utf8_data;
	माप_प्रकार utf8_data_size;
	माप_प्रकार utf8_data_allocated;
	अचिन्हित दीर्घ physical_ओळ_मोजणी;
	संरचना textplain_ओळ *physical_ओळ;
	पूर्णांक formatted_width;
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

स्थिर पूर्णांक textplain_tab_width = 256;  /* try for a sensible default */

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
textplain_charset_hack(स्थिरांक uint8_t *data,
		       माप_प्रकार len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	परत PARSERUTILS_OK;
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
	अक्षर *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = दो_स्मृति(CHUNK);
	जर (utf8_data == रिक्त)
		जा no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	जर (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	जर (error != PARSERUTILS_OK) {
		मुक्त(utf8_data);
		जा no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_ओळ = 0;
	c->physical_ओळ_मोजणी = 0;
	c->formatted_width = 0;
	c->bw = रिक्त;
	c->sel = selection_create((संरचना content *)c);

	परत NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);

	परत NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
स्थिर nserror
textplain_create(स्थिरांक content_handler *handler,
		 lwc_string *imime_type,
		 स्थिरांक संरचना http_parameter *params,
		 llcache_handle *llcache,
		 स्थिरांक अक्षर *fallback_charset,
		 bool quirks,
		 संरचना content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = सुस्मृति(1, आकार(textplain_content));
	जर (text == रिक्त) {
		परत NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	जर (error != NSERROR_OK) {
		मुक्त(text);
		परत error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	जर (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	जर (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		मुक्त(text);
		परत error;
	}

	lwc_string_unref(encoding);

	*c = (संरचना content *) text;

	परत NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
स्थिर bool
textplain_copy_utf8_data(textplain_content *c, स्थिरांक uint8_t *buf, माप_प्रकार len)
{
	जर (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		माप_प्रकार allocated;
		अक्षर *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = पुनः_स्मृति(c->utf8_data, allocated);
		जर (utf8_data == रिक्त)
			परत false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	स_नकल(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	परत true;
}


/**
 * drain input
 */
स्थिर bool
textplain_drain_आदान(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	स्थिर स्थिरांक uint8_t *u_fffd = (स्थिरांक uint8_t *) "\xef\xbf\xbd";
	स्थिरांक uint8_t *ch;
	माप_प्रकार chlen, offset = 0;

	पर्यंत (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		जर (chlen == 1 && *ch == 0) {
			जर (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				जर (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					परत false;
			}

			/* Emit U+FFFD */
			जर (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				परत false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} अन्यथा {
			/* Accumulate input */
			offset += chlen;

			जर (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				जर (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					परत false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	जर (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		जर (textplain_copy_utf8_data(c, ch, offset) == false)
			परत false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	परत true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
स्थिर bool
textplain_process_data(संरचना content *c, स्थिरांक अक्षर *data, अचिन्हित पूर्णांक size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (स्थिरांक uint8_t *) data, size);
	जर (error != PARSERUTILS_OK) {
		जा no_memory;
	}

	जर (textplain_drain_आदान(text, stream, PARSERUTILS_NEEDDATA) == false)
		जा no_memory;

	परत true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
	परत false;
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
	जर (error != PARSERUTILS_OK) {
		परत false;
	}

	जर (textplain_drain_आदान(text, stream, PARSERUTILS_EOF) == false)
		परत false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = रिक्त;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	परत true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
स्थिर दशांश textplain_ओळ_height(शून्य)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	परत FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
स्थिर शून्य textplain_reformat(संरचना content *c, पूर्णांक width, पूर्णांक height)
{
	textplain_content *text = (textplain_content *) c;
	अक्षर *utf8_data = text->utf8_data;
	माप_प्रकार utf8_data_size = text->utf8_data_size;
	अचिन्हित दीर्घ ओळ_मोजणी = 0;
	संरचना textplain_ओळ *ओळ = text->physical_ओळ;
	संरचना textplain_ओळ *line1;
	माप_प्रकार i, space, col;
	माप_प्रकार columns = 80;
	पूर्णांक character_width;
	माप_प्रकार ओळ_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	जर (res != NSERROR_OK) {
		परत;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_ओळ_मोजणी = 0;

	जर (!ओळ) {
		text->physical_ओळ = ओळ =
			दो_स्मृति(आकार(संरचना textplain_ओळ) * (1024 + 3));
		जर (!ओळ)
			जा no_memory;
	}

	ओळ[ओळ_मोजणी++].start = ओळ_start = 0;
	space = 0;
	i = 0;
	col = 0;
	पर्यंत (i < utf8_data_size) {
		माप_प्रकार csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		माप_प्रकार next_col;
		parserutils_error लिखो_त्रुटि;

		लिखो_त्रुटि = parserutils_charset_utf8_to_ucs4((स्थिरांक uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		जर (लिखो_त्रुटि != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		जर (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		जर (term || next_col >= columns) {
			जर (ओळ_मोजणी % 1024 == 0) {
				line1 = पुनः_स्मृति(ओळ,
						आकार(संरचना textplain_ओळ) *
						(ओळ_मोजणी + 1024 + 3));
				जर (!line1)
					जा no_memory;
				text->physical_ओळ = ओळ = line1;
			}

			जर (term) {
				ओळ[ओळ_मोजणी-1].length = i - ओळ_start;

				/* skip second char of CR/LF or LF/CR pair */
				जर (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} अन्यथा {
				जर (space) {
					/* break at last space in line */
					i = space;
					ओळ[ओळ_मोजणी-1].length = (i + 1) - ओळ_start;
				} अन्यथा
					ओळ[ओळ_मोजणी-1].length = i - ओळ_start;
			}

			ओळ[ओळ_मोजणी++].start = ओळ_start = i + 1;
			col = 0;
			space = 0;
		} अन्यथा {
			col++;
			जर (chr == ' ')
				space = i;
		}
		i += csize;
	}
	ओळ[ओळ_मोजणी-1].length = i - ओळ[ओळ_मोजणी-1].start;
	ओळ[ओळ_मोजणी].start = utf8_data_size;

	text->physical_ओळ_मोजणी = ओळ_मोजणी;
	c->width = width;
	c->height = ओळ_मोजणी * textplain_ओळ_height() + MARGIN + MARGIN;

	परत;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", ओळ_मोजणी);
	परत;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

स्थिर शून्य textplain_destroy(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	जर (text->inputstream != रिक्त) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	जर (text->physical_ओळ != रिक्त) {
		मुक्त(text->physical_ओळ);
	}

	जर (text->utf8_data != रिक्त) {
		मुक्त(text->utf8_data);
	}

	जर (text->sel != रिक्त) {
		selection_destroy(text->sel);
	}
}


स्थिर nserror textplain_clone(स्थिरांक संरचना content *old, संरचना content **newc)
{
	स्थिरांक textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	स्थिरांक uint8_t *data;
	माप_प्रकार size;

	text = सुस्मृति(1, आकार(textplain_content));
	जर (text == रिक्त)
		परत NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	जर (error != NSERROR_OK) {
		content_destroy(&text->base);
		परत error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	जर (error != NSERROR_OK) {
		content_destroy(&text->base);
		परत error;
	}

	data = content__get_source_data(&text->base, &size);
	जर (size > 0) {
		जर (textplain_process_data(&text->base,
					   (स्थिरांक अक्षर *)data,
					   size) == false) {
			content_destroy(&text->base);
			परत NSERROR_NOMEM;
		}
	}

	जर (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		जर (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			परत NSERROR_CLONE_FAILED;
		}
	}

	परत NSERROR_OK;
}


स्थिर content_type textplain_content_type(शून्य)
{
	परत CONTENT_TEXTPLAIN;
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
textplain_offset_from_coords(संरचना content *c, पूर्णांक x, पूर्णांक y, पूर्णांक dir)
{
	textplain_content *textc = (textplain_content *) c;
	दशांश ओळ_height = textplain_ओळ_height();
	संरचना textplain_ओळ *ओळ;
	स्थिरांक अक्षर *text;
	अचिन्हित nlines;
	माप_प्रकार length;
	पूर्णांक idx;

	प्रतिज्ञा(c != रिक्त);

	y = (पूर्णांक)((दशांश)(y - MARGIN) / ओळ_height);
	x -= MARGIN;

	nlines = textc->physical_ओळ_मोजणी;
	जर (!nlines)
		परत 0;

	जर (y <= 0) y = 0;
	अन्यथा जर ((अचिन्हित)y >= nlines)
		y = nlines - 1;

	ओळ = &textc->physical_ओळ[y];
	text = textc->utf8_data + ओळ->start;
	length = ओळ->length;
	idx = 0;

	पर्यंत (x > 0) {
		माप_प्रकार next_offset = 0;
		पूर्णांक width = पूर्णांक_उच्च;

		पर्यंत (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		जर (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		जर (x <= width) {
			पूर्णांक pixel_offset;
			माप_प्रकार char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			थांबा;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		जर (x <= width) थांबा;

		x -= width;
		length--;
		text++;
		idx++;
	}

	परत ओळ->start + idx;
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
		       पूर्णांक x, पूर्णांक y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	संघ content_संदेश_data संदेश_data;
	स्थिरांक अक्षर *status = 0;
	माप_प्रकार idx;
	पूर्णांक dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, रिक्त);

	idx = textplain_offset_from_coords(c, x, y, dir);
	जर (selection_click(text->sel, text->bw, mouse, idx)) {

		जर (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, रिक्त);
			status = messages_get("Selecting");
		}

	} अन्यथा {
		जर (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	संदेश_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_data);

	संदेश_data.pointer = pointer;
	content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_data);

	परत NSERROR_OK;
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
		      पूर्णांक x, पूर्णांक y)
{
	textplain_content *text = (textplain_content *) c;

	जर (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		पूर्णांक dir = -1;
		माप_प्रकार idx;

		जर (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, रिक्त);
	}

	निवड (browser_window_get_drag_type(bw)) {

	स्थिती DRAGGING_SELECTION: {
		पूर्णांक dir = -1;
		माप_प्रकार idx;

		जर (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		थांबा;

	पूर्वनिर्धारित:
		textplain_mouse_action(c, bw, mouse, x, y);
		थांबा;
	}

	परत NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
स्थिर bool textplain_keypress(संरचना content *c, uint32_t किल्ली)
{
	textplain_content *text = (textplain_content *) c;
	संरचना selection *sel = text->sel;

	निवड (किल्ली) {
	स्थिती NS_किल्ली_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		परत true;

	स्थिती NS_किल्ली_CLEAR_SELECTION:
		selection_clear(sel, true);
		परत true;

	स्थिती NS_किल्ली_SELECT_ALL:
		selection_select_all(sel);
		परत true;

	स्थिती NS_किल्ली_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		परत selection_clear(sel, true);
	}

	परत false;
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
text_draw(स्थिरांक अक्षर *utf8_text,
	    माप_प्रकार utf8_len,
	    माप_प्रकार offset,
	    पूर्णांक x,
	    पूर्णांक y,
	    स्थिरांक संरचना rect *clip,
	    पूर्णांक height,
	    दशांश scale,
	    textplain_content *text,
	    स्थिरांक संरचना selection *sel,
	    स्थिरांक संरचना redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	जर (ctx->interactive == true) {
		अचिन्हित len = utf8_len;
		अचिन्हित start_idx;
		अचिन्हित end_idx;

		/* first try the browser window's current selection */
		जर (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		जर (!highlighted &&
		    (text->base.textsearch.context != रिक्त) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		जर (highlighted) {
			संरचना rect r;
			अचिन्हित endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			पूर्णांक startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			जर (end_idx > utf8_len) {
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
			जर (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			जर (res != NSERROR_OK) {
				endx = 0;
			}

			जर (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			जर (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (पूर्णांक)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				जर (res != NSERROR_OK) {
					परत false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			जर (res != NSERROR_OK) {
				परत false;
			}

			जर (start_idx > 0) {
				पूर्णांक px0 = max(x + startx, clip->x0);
				पूर्णांक px1 = min(x + endx, clip->x1);

				जर (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					जर (res != NSERROR_OK) {
						परत false;
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

			जर (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (पूर्णांक)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				परत false;
			}

			/* draw any text succeeding highlighted portion */
			जर (endtxt_idx < utf8_len) {
				पूर्णांक px0 = max(x + endx, clip->x0);
				जर (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					जर (res != NSERROR_OK) {
						परत false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (पूर्णांक)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					जर (res != NSERROR_OK) {
						परत false;
					}
				}
			}

			जर (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				परत false;
			}
		}
	}

	जर (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (पूर्णांक) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		जर (res != NSERROR_OK) {
			परत false;
		}
	}
	परत true;
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
		 स्थिरांक संरचना rect *clip,
		 स्थिरांक संरचना redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	संरचना browser_window *bw = text->bw;
	अक्षर *utf8_data = text->utf8_data;
	दीर्घ lineno;
	पूर्णांक x = data->x;
	पूर्णांक y = data->y;
	अचिन्हित दीर्घ ओळ_मोजणी = text->physical_ओळ_मोजणी;
	दशांश ओळ_height = textplain_ओळ_height();
	दशांश scaled_ओळ_height = ओळ_height * data->scale;
	दीर्घ line0 = (clip->y0 - y * data->scale) / scaled_ओळ_height - 1;
	दीर्घ line1 = (clip->y1 - y * data->scale) / scaled_ओळ_height + 1;
	संरचना textplain_ओळ *ओळ = text->physical_ओळ;
	माप_प्रकार length;
	plot_style_t *plot_style_highlight;
	nserror res;

	जर (line0 < 0)
		line0 = 0;
	जर (line1 < 0)
		line1 = 0;
	जर (ओळ_मोजणी < (अचिन्हित दीर्घ) line0)
		line0 = ओळ_मोजणी;
	जर (ओळ_मोजणी < (अचिन्हित दीर्घ) line1)
		line1 = ओळ_मोजणी;
	जर (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	जर (res != NSERROR_OK) {
		परत false;
	}

	जर (!ओळ)
		परत true;

	/* choose a suitable background colour for any highlighted text */
	जर ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	अन्यथा
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	साठी (lineno = line0; lineno != line1; lineno++) {
		स्थिरांक अक्षर *text_d = utf8_data + ओळ[lineno].start;
		पूर्णांक tab_width = textplain_tab_width * data->scale;
		माप_प्रकार offset = 0;
		पूर्णांक tx = x;

		जर (!tab_width) tab_width = 1;

		length = ओळ[lineno].length;
		जर (!length)
			सुरूठेवा;

		पर्यंत (offset < length) {
			माप_प्रकार next_offset = offset;
			पूर्णांक width;
			पूर्णांक ntx;
			nserror res;

			पर्यंत ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			जर (!text_draw(text_d + offset,
				       next_offset - offset,
				       ओळ[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_ओळ_height),
				       clip,
				       ओळ_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				परत false;
			}

			जर (next_offset >= length)
				थांबा;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			जर (res == NSERROR_OK) {
				tx += (पूर्णांक)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			जर (bw) {
				अचिन्हित tab_ofst = ओळ[lineno].start + next_offset;
				संरचना selection *sel = text->sel;
				bool highlighted = false;

				अचिन्हित start_idx, end_idx;
				जर (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				जर (!highlighted &&
				    (c->textsearch.context != रिक्त)) {
					अचिन्हित start_idx, end_idx;
					जर (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				जर (highlighted) {
					संरचना rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_ओळ_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_ओळ_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					जर (res != NSERROR_OK) {
						परत false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	परत true;
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

	परत NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
स्थिर nserror textplain_close(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = रिक्त;

	परत NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
स्थिर अक्षर *textplain_get_selection(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	परत selection_get_copy(text->sel);
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
स्थिर पूर्णांक
textplain_coord_from_offset(स्थिरांक अक्षर *text, माप_प्रकार offset, माप_प्रकार length)
{
	पूर्णांक x = 0;

	पर्यंत (offset > 0) {
		माप_प्रकार next_offset = 0;
		पूर्णांक tx;

		पर्यंत (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		जर (next_offset >= offset)
			थांबा;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	परत x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
स्थिर अचिन्हित दीर्घ textplain_ओळ_मोजणी(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	प्रतिज्ञा(c != रिक्त);

	परत text->physical_ओळ_मोजणी;
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
स्थिर अक्षर *
textplain_get_ओळ(संरचना content *c,
		   अचिन्हित lineno,
		   माप_प्रकार *poffset,
		   माप_प्रकार *plen)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_ओळ *ओळ;

	प्रतिज्ञा(c != रिक्त);

	जर (lineno >= text->physical_ओळ_मोजणी)
		परत रिक्त;
	ओळ = &text->physical_ओळ[lineno];

	*poffset = ओळ->start;
	*plen = ओळ->length;
	परत text->utf8_data + ओळ->start;
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
स्थिर पूर्णांक textplain_find_ओळ(संरचना content *c, अचिन्हित offset)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_ओळ *ओळ;
	पूर्णांक nlines;
	पूर्णांक lineno = 0;

	प्रतिज्ञा(c != रिक्त);

	ओळ = text->physical_ओळ;
	nlines = text->physical_ओळ_मोजणी;

	जर (offset > text->utf8_data_size) {
		परत -1;
	}

/* \todo - implement binary search here */
	पर्यंत (lineno < nlines && ओळ[lineno].start < offset) {
		lineno++;
	}
	जर (ओळ[lineno].start > offset) {
		lineno--;
	}

	परत lineno;
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
			  स्थिरांक अक्षर *pattern,
			  पूर्णांक p_len,
			  bool case_sens)
{
	पूर्णांक nlines = textplain_ओळ_मोजणी(c);
	पूर्णांक ओळ;
	nserror res = NSERROR_OK;

	साठी(ओळ = 0; ओळ < nlines; ओळ++) {
		माप_प्रकार offset, length;
		स्थिरांक अक्षर *text;

		text = textplain_get_ओळ(c, ओळ, &offset, &length);
		जर (text) {
			पर्यंत (length > 0) {
				अचिन्हित match_length;
				माप_प्रकार start_idx;
				स्थिरांक अक्षर *new_text;
				स्थिरांक अक्षर *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				जर (!pos)
					थांबा;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						रिक्त,
						रिक्त);
				जर (res != NSERROR_OK) {
					परत res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	परत res;
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
	दशांश ओळ_height = textplain_ओळ_height();
	अक्षर *utf8_data;
	संरचना textplain_ओळ *ओळ;
	अचिन्हित lineno = 0;
	अचिन्हित nlines;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(start <= end);
	प्रतिज्ञा(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_ओळ_मोजणी;
	ओळ = text->physical_ओळ;

	/* find start */
	lineno = textplain_find_ओळ(c, start);

	r->y0 = (पूर्णांक)(MARGIN + lineno * ओळ_height);

	जर (lineno + 1 <= nlines || ओळ[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_ओळ(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} अन्यथा {
		/* single line */
		स्थिरांक अक्षर *text = utf8_data + ओळ[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - ओळ[lineno].start,
						    ओळ[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - ओळ[lineno].start,
						    ओळ[lineno].length);
	}

	r->y1 = (पूर्णांक)(MARGIN + (lineno + 1) * ओळ_height);
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
स्थिर अक्षर *
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
	जर (!utf8_size) परत रिक्त;

	/* clamp to valid offset range */
	जर (start >= utf8_size) start = utf8_size;
	जर (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	परत text->utf8_data + start;
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

	परत NSERROR_OK;
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

	जर (end_idx <= start_idx) {
		परत NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	परत NSERROR_OK;
}

स्थिर nserror
textplain_textselection_copy(संरचना content *c,
			     अचिन्हित start_idx,
			     अचिन्हित end_idx,
			     संरचना selection_string *selstr)
{
	स्थिरांक अक्षर *text;
	माप_प्रकार length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	जर (text != रिक्त) {
		res = selection_string_append(text, length, false, रिक्त, selstr);
	}
	जर (res == false) {
		परत NSERROR_NOMEM;
	}
	परत NSERROR_OK;
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
	परत NSERROR_OK;
}


/**
 * plain text content handler table
 */
स्थिर स्थिरांक content_handler textplain_content_handler = {
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
	जर (lerror != lwc_error_ok) {
		परत NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	जर (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	जर (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	परत error;
}




