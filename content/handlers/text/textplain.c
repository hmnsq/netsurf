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

#समावेश <string.h>
#समावेश <parserutils/input/inputstream.h>

#समावेश "utils/errors.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/http.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/utf8.h"
#समावेश "utils/nsoption.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/keypress.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/layout.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "content/hlcache.h"
#समावेश "content/textsearch.h"
#समावेश "content/handlers/css/utils.h"
#समावेश "desktop/selection.h"
#समावेश "desktop/gui_internal.h"

#समावेश "text/textplain.h"

काष्ठा textplain_पंक्ति {
	माप_प्रकार	start;
	माप_प्रकार	लंबाई;
};

/**
 * plain text content
 */
प्रकार काष्ठा textplain_content {
	काष्ठा content base;

	lwc_string *encoding;
	व्योम *inputstream;
	अक्षर *utf8_तथ्य;
	माप_प्रकार utf8_तथ्य_आकार;
	माप_प्रकार utf8_तथ्य_allocated;
	अचिन्हित दीर्घ physical_पंक्ति_गिनती;
	काष्ठा textplain_पंक्ति *physical_पंक्ति;
	पूर्णांक formatted_width;
	काष्ठा browser_window *bw;

	काष्ठा selection *sel; /** Selection state */

} textplain_content;


#घोषणा CHUNK 32768 /* Must be a power of 2 */
#घोषणा MARGIN 4

#घोषणा TAB_WIDTH 8  /* must be power of 2 currently */
#घोषणा TEXT_आकार 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

अटल plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.आकार = TEXT_आकार,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

अटल पूर्णांक textplain_tab_width = 256;  /* try for a sensible default */

अटल lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
अटल व्योम textplain_fini(व्योम)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = शून्य;
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
अटल parserutils_error
textplain_charset_hack(स्थिर uint8_t *तथ्य,
		       माप_प्रकार len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	वापस PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
अटल nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	अक्षर *utf8_तथ्य;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.आकार = (nsoption_int(font_आकार) * PLOT_STYLE_SCALE) / 10;

	utf8_तथ्य = दो_स्मृति(CHUNK);
	अगर (utf8_तथ्य == शून्य)
		जाओ no_memory;

	error = parserutils_inputstream_create(lwc_string_तथ्य(encoding), 0,
					       textplain_charset_hack, &stream);
	अगर (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	अगर (error != PARSERUTILS_OK) {
		मुक्त(utf8_तथ्य);
		जाओ no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_तथ्य = utf8_तथ्य;
	c->utf8_तथ्य_आकार = 0;
	c->utf8_तथ्य_allocated = CHUNK;
	c->physical_पंक्ति = 0;
	c->physical_पंक्ति_गिनती = 0;
	c->formatted_width = 0;
	c->bw = शून्य;
	c->sel = selection_create((काष्ठा content *)c);

	वापस NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);

	वापस NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
अटल nserror
textplain_create(स्थिर content_handler *handler,
		 lwc_string *imime_type,
		 स्थिर काष्ठा http_parameter *params,
		 llcache_handle *llcache,
		 स्थिर अक्षर *fallback_charset,
		 bool quirks,
		 काष्ठा content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = सुस्मृति(1, माप(textplain_content));
	अगर (text == शून्य) {
		वापस NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(text);
		वापस error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	अगर (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	अगर (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		मुक्त(text);
		वापस error;
	}

	lwc_string_unref(encoding);

	*c = (काष्ठा content *) text;

	वापस NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
अटल bool
textplain_copy_utf8_तथ्य(textplain_content *c, स्थिर uint8_t *buf, माप_प्रकार len)
{
	अगर (c->utf8_तथ्य_आकार + len >= c->utf8_तथ्य_allocated) {
		/* Compute next multiple of chunk above the required space */
		माप_प्रकार allocated;
		अक्षर *utf8_तथ्य;

		allocated = (c->utf8_तथ्य_आकार + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_तथ्य = पुनः_स्मृति(c->utf8_तथ्य, allocated);
		अगर (utf8_तथ्य == शून्य)
			वापस false;

		c->utf8_तथ्य = utf8_तथ्य;
		c->utf8_तथ्य_allocated = allocated;
	}

	स_नकल(c->utf8_तथ्य + c->utf8_तथ्य_आकार, buf, len);
	c->utf8_तथ्य_आकार += len;

	वापस true;
}


/**
 * drain input
 */
अटल bool
textplain_drain_निवेश(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	अटल स्थिर uint8_t *u_fffd = (स्थिर uint8_t *) "\xef\xbf\xbd";
	स्थिर uint8_t *ch;
	माप_प्रकार chlen, offset = 0;

	जबतक (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		अगर (chlen == 1 && *ch == 0) {
			अगर (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				अगर (textplain_copy_utf8_तथ्य(c, ch,
							     offset) == false)
					वापस false;
			}

			/* Emit U+FFFD */
			अगर (textplain_copy_utf8_तथ्य(c, u_fffd, 3) == false)
				वापस false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} अन्यथा {
			/* Accumulate input */
			offset += chlen;

			अगर (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				अगर (textplain_copy_utf8_तथ्य(c, ch,
							     offset) == false)
					वापस false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	अगर (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		अगर (textplain_copy_utf8_तथ्य(c, ch, offset) == false)
			वापस false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	वापस true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
अटल bool
textplain_process_तथ्य(काष्ठा content *c, स्थिर अक्षर *तथ्य, अचिन्हित पूर्णांक आकार)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (स्थिर uint8_t *) तथ्य, आकार);
	अगर (error != PARSERUTILS_OK) {
		जाओ no_memory;
	}

	अगर (textplain_drain_निवेश(text, stream, PARSERUTILS_NEEDDATA) == false)
		जाओ no_memory;

	वापस true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, शून्य);
	वापस false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
अटल bool textplain_convert(काष्ठा content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, शून्य, 0);
	अगर (error != PARSERUTILS_OK) {
		वापस false;
	}

	अगर (textplain_drain_निवेश(text, stream, PARSERUTILS_EOF) == false)
		वापस false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = शून्य;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	वापस true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
अटल भग्न textplain_पंक्ति_height(व्योम)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	वापस FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.आकार / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
अटल व्योम textplain_reformat(काष्ठा content *c, पूर्णांक width, पूर्णांक height)
{
	textplain_content *text = (textplain_content *) c;
	अक्षर *utf8_तथ्य = text->utf8_तथ्य;
	माप_प्रकार utf8_तथ्य_आकार = text->utf8_तथ्य_आकार;
	अचिन्हित दीर्घ पंक्ति_गिनती = 0;
	काष्ठा textplain_पंक्ति *पंक्ति = text->physical_पंक्ति;
	काष्ठा textplain_पंक्ति *line1;
	माप_प्रकार i, space, col;
	माप_प्रकार columns = 80;
	पूर्णांक character_width;
	माप_प्रकार पंक्ति_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	अगर (res != NSERROR_OK) {
		वापस;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_पंक्ति_गिनती = 0;

	अगर (!पंक्ति) {
		text->physical_पंक्ति = पंक्ति =
			दो_स्मृति(माप(काष्ठा textplain_पंक्ति) * (1024 + 3));
		अगर (!पंक्ति)
			जाओ no_memory;
	}

	पंक्ति[पंक्ति_गिनती++].start = पंक्ति_start = 0;
	space = 0;
	i = 0;
	col = 0;
	जबतक (i < utf8_तथ्य_आकार) {
		माप_प्रकार csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		माप_प्रकार अग्र_col;
		parserutils_error लिखो_त्रुटि;

		लिखो_त्रुटि = parserutils_charset_utf8_to_ucs4((स्थिर uint8_t *)utf8_तथ्य + i, utf8_तथ्य_आकार - i, &chr, &csize);
		अगर (लिखो_त्रुटि != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		अग्र_col = col + 1;

		अगर (chr == '\t') {
			अग्र_col = (अग्र_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		अगर (term || अग्र_col >= columns) {
			अगर (पंक्ति_गिनती % 1024 == 0) {
				line1 = पुनः_स्मृति(पंक्ति,
						माप(काष्ठा textplain_पंक्ति) *
						(पंक्ति_गिनती + 1024 + 3));
				अगर (!line1)
					जाओ no_memory;
				text->physical_पंक्ति = पंक्ति = line1;
			}

			अगर (term) {
				पंक्ति[पंक्ति_गिनती-1].लंबाई = i - पंक्ति_start;

				/* skip second char of CR/LF or LF/CR pair */
				अगर (i + 1 < utf8_तथ्य_आकार &&
				    utf8_तथ्य[i+1] != utf8_तथ्य[i] &&
				    (utf8_तथ्य[i+1] == '\n' ||
				     utf8_तथ्य[i+1] == '\r')) {
					i++;
				}
			} अन्यथा {
				अगर (space) {
					/* break at last space in line */
					i = space;
					पंक्ति[पंक्ति_गिनती-1].लंबाई = (i + 1) - पंक्ति_start;
				} अन्यथा
					पंक्ति[पंक्ति_गिनती-1].लंबाई = i - पंक्ति_start;
			}

			पंक्ति[पंक्ति_गिनती++].start = पंक्ति_start = i + 1;
			col = 0;
			space = 0;
		} अन्यथा {
			col++;
			अगर (chr == ' ')
				space = i;
		}
		i += csize;
	}
	पंक्ति[पंक्ति_गिनती-1].लंबाई = i - पंक्ति[पंक्ति_गिनती-1].start;
	पंक्ति[पंक्ति_गिनती].start = utf8_तथ्य_आकार;

	text->physical_पंक्ति_गिनती = पंक्ति_गिनती;
	c->width = width;
	c->height = पंक्ति_गिनती * textplain_पंक्ति_height() + MARGIN + MARGIN;

	वापस;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", पंक्ति_गिनती);
	वापस;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

अटल व्योम textplain_destroy(काष्ठा content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	अगर (text->inputstream != शून्य) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	अगर (text->physical_पंक्ति != शून्य) {
		मुक्त(text->physical_पंक्ति);
	}

	अगर (text->utf8_तथ्य != शून्य) {
		मुक्त(text->utf8_तथ्य);
	}

	अगर (text->sel != शून्य) {
		selection_destroy(text->sel);
	}
}


अटल nserror textplain_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	स्थिर textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;

	text = सुस्मृति(1, माप(textplain_content));
	अगर (text == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&text->base);
		वापस error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	अगर (error != NSERROR_OK) {
		content_destroy(&text->base);
		वापस error;
	}

	तथ्य = content_get_source_तथ्य_2(&text->base, &आकार);
	अगर (आकार > 0) {
		अगर (textplain_process_तथ्य(&text->base,
					   (स्थिर अक्षर *)तथ्य,
					   आकार) == false) {
			content_destroy(&text->base);
			वापस NSERROR_NOMEM;
		}
	}

	अगर (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		अगर (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	वापस NSERROR_OK;
}


अटल content_type textplain_content_type(व्योम)
{
	वापस CONTENT_TEXTPLAIN;
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
अटल माप_प्रकार
textplain_offset_from_coords(काष्ठा content *c, पूर्णांक x, पूर्णांक y, पूर्णांक dir)
{
	textplain_content *textc = (textplain_content *) c;
	भग्न पंक्ति_height = textplain_पंक्ति_height();
	काष्ठा textplain_पंक्ति *पंक्ति;
	स्थिर अक्षर *text;
	अचिन्हित nlines;
	माप_प्रकार लंबाई;
	पूर्णांक idx;

	निश्चित(c != शून्य);

	y = (पूर्णांक)((भग्न)(y - MARGIN) / पंक्ति_height);
	x -= MARGIN;

	nlines = textc->physical_पंक्ति_गिनती;
	अगर (!nlines)
		वापस 0;

	अगर (y <= 0) y = 0;
	अन्यथा अगर ((अचिन्हित)y >= nlines)
		y = nlines - 1;

	पंक्ति = &textc->physical_पंक्ति[y];
	text = textc->utf8_तथ्य + पंक्ति->start;
	लंबाई = पंक्ति->लंबाई;
	idx = 0;

	जबतक (x > 0) {
		माप_प्रकार अग्र_offset = 0;
		पूर्णांक width = पूर्णांक_उच्च;

		जबतक (अग्र_offset < लंबाई && text[अग्र_offset] != '\t') {
			अग्र_offset = utf8_अग्र(text, लंबाई, अग्र_offset);
		}

		अगर (अग्र_offset < लंबाई) {
			guit->layout->width(&textplain_style,
					    text,
					    अग्र_offset,
					    &width);
		}

		अगर (x <= width) {
			पूर्णांक pixel_offset;
			माप_प्रकार char_offset;

			guit->layout->position(&textplain_style,
					       text, अग्र_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			अवरोध;
		}

		x -= width;
		लंबाई -= अग्र_offset;
		text += अग्र_offset;
		idx += अग्र_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		अगर (x <= width) अवरोध;

		x -= width;
		लंबाई--;
		text++;
		idx++;
	}

	वापस पंक्ति->start + idx;
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
अटल nserror
textplain_mouse_action(काष्ठा content *c,
		       काष्ठा browser_window *bw,
		       browser_mouse_state mouse,
		       पूर्णांक x, पूर्णांक y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	स्थिर अक्षर *status = 0;
	माप_प्रकार idx;
	पूर्णांक dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, शून्य);

	idx = textplain_offset_from_coords(c, x, y, dir);
	अगर (selection_click(text->sel, text->bw, mouse, idx)) {

		अगर (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, शून्य);
			status = messages_get("Selecting");
		}

	} अन्यथा {
		अगर (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	संदेश_तथ्य.explicit_status_text = status;
	content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_तथ्य);

	संदेश_तथ्य.pointer = pointer;
	content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_तथ्य);

	वापस NSERROR_OK;
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
अटल nserror
textplain_mouse_track(काष्ठा content *c,
		      काष्ठा browser_window *bw,
		      browser_mouse_state mouse,
		      पूर्णांक x, पूर्णांक y)
{
	textplain_content *text = (textplain_content *) c;

	अगर (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		पूर्णांक dir = -1;
		माप_प्रकार idx;

		अगर (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, शून्य);
	}

	चयन (browser_window_get_drag_type(bw)) {

	हाल DRAGGING_SELECTION: {
		पूर्णांक dir = -1;
		माप_प्रकार idx;

		अगर (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		अवरोध;

	शेष:
		textplain_mouse_action(c, bw, mouse, x, y);
		अवरोध;
	}

	वापस NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
अटल bool textplain_keypress(काष्ठा content *c, uint32_t कुंजी)
{
	textplain_content *text = (textplain_content *) c;
	काष्ठा selection *sel = text->sel;

	चयन (कुंजी) {
	हाल NS_कुंजी_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		वापस true;

	हाल NS_कुंजी_CLEAR_SELECTION:
		selection_clear(sel, true);
		वापस true;

	हाल NS_कुंजी_SELECT_ALL:
		selection_select_all(sel);
		वापस true;

	हाल NS_कुंजी_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		वापस selection_clear(sel, true);
	}

	वापस false;
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
अटल bool
text_draw(स्थिर अक्षर *utf8_text,
	    माप_प्रकार utf8_len,
	    माप_प्रकार offset,
	    पूर्णांक x,
	    पूर्णांक y,
	    स्थिर काष्ठा rect *clip,
	    पूर्णांक height,
	    भग्न scale,
	    textplain_content *text,
	    स्थिर काष्ठा selection *sel,
	    स्थिर काष्ठा redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.आकार *= scale;

	/* is this box part of a selection? */
	अगर (ctx->interactive == true) {
		अचिन्हित len = utf8_len;
		अचिन्हित start_idx;
		अचिन्हित end_idx;

		/* first try the browser window's current selection */
		अगर (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		अगर (!highlighted &&
		    (text->base.textsearch.context != शून्य) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		अगर (highlighted) {
			काष्ठा rect r;
			अचिन्हित endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			पूर्णांक startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			अगर (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				निश्चित(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			अगर (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			अगर (res != NSERROR_OK) {
				endx = 0;
			}

			अगर (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			अगर (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (पूर्णांक)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				अगर (res != NSERROR_OK) {
					वापस false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			अगर (res != NSERROR_OK) {
				वापस false;
			}

			अगर (start_idx > 0) {
				पूर्णांक px0 = max(x + startx, clip->x0);
				पूर्णांक px1 = min(x + endx, clip->x1);

				अगर (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					अगर (res != NSERROR_OK) {
						वापस false;
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

			अगर (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (पूर्णांक)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				वापस false;
			}

			/* draw any text succeeding highlighted portion */
			अगर (endtxt_idx < utf8_len) {
				पूर्णांक px0 = max(x + endx, clip->x0);
				अगर (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					अगर (res != NSERROR_OK) {
						वापस false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (पूर्णांक)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					अगर (res != NSERROR_OK) {
						वापस false;
					}
				}
			}

			अगर (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				वापस false;
			}
		}
	}

	अगर (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (पूर्णांक) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}
	वापस true;
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
अटल bool
textplain_redraw(काष्ठा content *c,
		 काष्ठा content_redraw_तथ्य *तथ्य,
		 स्थिर काष्ठा rect *clip,
		 स्थिर काष्ठा redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	काष्ठा browser_window *bw = text->bw;
	अक्षर *utf8_तथ्य = text->utf8_तथ्य;
	दीर्घ lineno;
	पूर्णांक x = तथ्य->x;
	पूर्णांक y = तथ्य->y;
	अचिन्हित दीर्घ पंक्ति_गिनती = text->physical_पंक्ति_गिनती;
	भग्न पंक्ति_height = textplain_पंक्ति_height();
	भग्न scaled_पंक्ति_height = पंक्ति_height * तथ्य->scale;
	दीर्घ line0 = (clip->y0 - y * तथ्य->scale) / scaled_पंक्ति_height - 1;
	दीर्घ line1 = (clip->y1 - y * तथ्य->scale) / scaled_पंक्ति_height + 1;
	काष्ठा textplain_पंक्ति *पंक्ति = text->physical_पंक्ति;
	माप_प्रकार लंबाई;
	plot_style_t *plot_style_highlight;
	nserror res;

	अगर (line0 < 0)
		line0 = 0;
	अगर (line1 < 0)
		line1 = 0;
	अगर (पंक्ति_गिनती < (अचिन्हित दीर्घ) line0)
		line0 = पंक्ति_गिनती;
	अगर (पंक्ति_गिनती < (अचिन्हित दीर्घ) line1)
		line1 = पंक्ति_गिनती;
	अगर (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	अगर (!पंक्ति)
		वापस true;

	/* choose a suitable background colour for any highlighted text */
	अगर ((तथ्य->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	अन्यथा
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = तथ्य->background_colour;

	x = (x + MARGIN) * तथ्य->scale;
	y = (y + MARGIN) * तथ्य->scale;
	क्रम (lineno = line0; lineno != line1; lineno++) {
		स्थिर अक्षर *text_d = utf8_तथ्य + पंक्ति[lineno].start;
		पूर्णांक tab_width = textplain_tab_width * तथ्य->scale;
		माप_प्रकार offset = 0;
		पूर्णांक tx = x;

		अगर (!tab_width) tab_width = 1;

		लंबाई = पंक्ति[lineno].लंबाई;
		अगर (!लंबाई)
			जारी;

		जबतक (offset < लंबाई) {
			माप_प्रकार अग्र_offset = offset;
			पूर्णांक width;
			पूर्णांक ntx;
			nserror res;

			जबतक ((अग्र_offset < लंबाई) &&
			       (text_d[अग्र_offset] != '\t')) {
				अग्र_offset = utf8_अग्र(text_d,
							लंबाई,
							अग्र_offset);
			}

			अगर (!text_draw(text_d + offset,
				       अग्र_offset - offset,
				       पंक्ति[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_पंक्ति_height),
				       clip,
				       पंक्ति_height,
				       तथ्य->scale,
				       text,
				       text->sel,
				       ctx)) {
				वापस false;
			}

			अगर (अग्र_offset >= लंबाई)
				अवरोध;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  अग्र_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			अगर (res == NSERROR_OK) {
				tx += (पूर्णांक)(width * तथ्य->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			अगर (bw) {
				अचिन्हित tab_ofst = पंक्ति[lineno].start + अग्र_offset;
				काष्ठा selection *sel = text->sel;
				bool highlighted = false;

				अचिन्हित start_idx, end_idx;
				अगर (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				अगर (!highlighted &&
				    (c->textsearch.context != शून्य)) {
					अचिन्हित start_idx, end_idx;
					अगर (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				अगर (highlighted) {
					काष्ठा rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_पंक्ति_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_पंक्ति_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					अगर (res != NSERROR_OK) {
						वापस false;
					}
				}
			}

			offset = अग्र_offset + 1;
			tx = ntx;
		}
	}

	वापस true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
अटल nserror
textplain_open(काष्ठा content *c,
	       काष्ठा browser_window *bw,
	       काष्ठा content *page,
	       काष्ठा object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	वापस NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
अटल nserror textplain_close(काष्ठा content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = शून्य;

	वापस NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
अटल अक्षर *textplain_get_selection(काष्ठा content *c)
{
	textplain_content *text = (textplain_content *) c;

	वापस selection_get_copy(text->sel);
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
अटल पूर्णांक
textplain_coord_from_offset(स्थिर अक्षर *text, माप_प्रकार offset, माप_प्रकार लंबाई)
{
	पूर्णांक x = 0;

	जबतक (offset > 0) {
		माप_प्रकार अग्र_offset = 0;
		पूर्णांक tx;

		जबतक (अग्र_offset < offset && text[अग्र_offset] != '\t') {
			अग्र_offset = utf8_अग्र(text, लंबाई, अग्र_offset);
		}

		guit->layout->width(&textplain_style, text, अग्र_offset, &tx);

		x += tx;

		अगर (अग्र_offset >= offset)
			अवरोध;

		/* align to next tab boundary */
		अग्र_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= अग्र_offset;
		text += अग्र_offset;
		लंबाई -= अग्र_offset;
	}

	वापस x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
अटल अचिन्हित दीर्घ textplain_पंक्ति_गिनती(काष्ठा content *c)
{
	textplain_content *text = (textplain_content *) c;

	निश्चित(c != शून्य);

	वापस text->physical_पंक्ति_गिनती;
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
अटल अक्षर *
textplain_get_पंक्ति(काष्ठा content *c,
		   अचिन्हित lineno,
		   माप_प्रकार *poffset,
		   माप_प्रकार *plen)
{
	textplain_content *text = (textplain_content *) c;
	काष्ठा textplain_पंक्ति *पंक्ति;

	निश्चित(c != शून्य);

	अगर (lineno >= text->physical_पंक्ति_गिनती)
		वापस शून्य;
	पंक्ति = &text->physical_पंक्ति[lineno];

	*poffset = पंक्ति->start;
	*plen = पंक्ति->लंबाई;
	वापस text->utf8_तथ्य + पंक्ति->start;
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
अटल पूर्णांक textplain_find_पंक्ति(काष्ठा content *c, अचिन्हित offset)
{
	textplain_content *text = (textplain_content *) c;
	काष्ठा textplain_पंक्ति *पंक्ति;
	पूर्णांक nlines;
	पूर्णांक lineno = 0;

	निश्चित(c != शून्य);

	पंक्ति = text->physical_पंक्ति;
	nlines = text->physical_पंक्ति_गिनती;

	अगर (offset > text->utf8_तथ्य_आकार) {
		वापस -1;
	}

/* \todo - implement binary search here */
	जबतक (lineno < nlines && पंक्ति[lineno].start < offset) {
		lineno++;
	}
	अगर (पंक्ति[lineno].start > offset) {
		lineno--;
	}

	वापस lineno;
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
अटल nserror
textplain_textsearch_find(काष्ठा content *c,
			  काष्ठा textsearch_context *context,
			  स्थिर अक्षर *pattern,
			  पूर्णांक p_len,
			  bool case_sens)
{
	पूर्णांक nlines = textplain_पंक्ति_गिनती(c);
	पूर्णांक पंक्ति;
	nserror res = NSERROR_OK;

	क्रम(पंक्ति = 0; पंक्ति < nlines; पंक्ति++) {
		माप_प्रकार offset, लंबाई;
		स्थिर अक्षर *text;

		text = textplain_get_पंक्ति(c, पंक्ति, &offset, &लंबाई);
		अगर (text) {
			जबतक (लंबाई > 0) {
				अचिन्हित match_लंबाई;
				माप_प्रकार start_idx;
				स्थिर अक्षर *new_text;
				स्थिर अक्षर *pos;

				pos = content_textsearch_find_pattern(
						text,
						लंबाई,
						pattern,
						p_len,
						case_sens,
						&match_लंबाई);
				अगर (!pos)
					अवरोध;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_लंबाई,
						शून्य,
						शून्य);
				अगर (res != NSERROR_OK) {
					वापस res;
				}

				new_text = pos + match_लंबाई;
				offset += (new_text - text);
				लंबाई -= (new_text - text);
				text = new_text;
			}
		}
	}

	वापस res;
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
अटल व्योम
textplain_coords_from_range(काष्ठा content *c,
			    अचिन्हित start,
			    अचिन्हित end,
			    काष्ठा rect *r)
{
	textplain_content *text = (textplain_content *) c;
	भग्न पंक्ति_height = textplain_पंक्ति_height();
	अक्षर *utf8_तथ्य;
	काष्ठा textplain_पंक्ति *पंक्ति;
	अचिन्हित lineno = 0;
	अचिन्हित nlines;

	निश्चित(c != शून्य);
	निश्चित(start <= end);
	निश्चित(end <= text->utf8_तथ्य_आकार);

	utf8_तथ्य = text->utf8_तथ्य;
	nlines = text->physical_पंक्ति_गिनती;
	पंक्ति = text->physical_पंक्ति;

	/* find start */
	lineno = textplain_find_पंक्ति(c, start);

	r->y0 = (पूर्णांक)(MARGIN + lineno * पंक्ति_height);

	अगर (lineno + 1 <= nlines || पंक्ति[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_पंक्ति(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} अन्यथा {
		/* single line */
		स्थिर अक्षर *text = utf8_तथ्य + पंक्ति[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - पंक्ति[lineno].start,
						    पंक्ति[lineno].लंबाई);

		r->x1 = textplain_coord_from_offset(text,
						    end - पंक्ति[lineno].start,
						    पंक्ति[lineno].लंबाई);
	}

	r->y1 = (पूर्णांक)(MARGIN + (lineno + 1) * पंक्ति_height);
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
अटल अक्षर *
textplain_get_raw_तथ्य(काष्ठा content *c,
		       अचिन्हित start,
		       अचिन्हित end,
		       माप_प्रकार *plen)
{
	textplain_content *text = (textplain_content *) c;
	माप_प्रकार utf8_आकार;

	निश्चित(c != शून्य);

	utf8_आकार = text->utf8_तथ्य_आकार;

	/* any text at all? */
	अगर (!utf8_आकार) वापस शून्य;

	/* clamp to valid offset range */
	अगर (start >= utf8_आकार) start = utf8_आकार;
	अगर (end >= utf8_आकार) end = utf8_आकार;

	*plen = end - start;

	वापस text->utf8_तथ्य + start;
}


/**
 * get bounds of a free text search match
 */
अटल nserror
textplain_textsearch_bounds(काष्ठा content *c,
			    अचिन्हित start_idx,
			    अचिन्हित end_idx,
			    काष्ठा box *start_box,
			    काष्ठा box *end_box,
			    काष्ठा rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	वापस NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
अटल nserror
textplain_textselection_redraw(काष्ठा content *c,
			       अचिन्हित start_idx,
			       अचिन्हित end_idx)
{
	काष्ठा rect r;

	अगर (end_idx <= start_idx) {
		वापस NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	वापस NSERROR_OK;
}

अटल nserror
textplain_textselection_copy(काष्ठा content *c,
			     अचिन्हित start_idx,
			     अचिन्हित end_idx,
			     काष्ठा selection_string *selstr)
{
	स्थिर अक्षर *text;
	माप_प्रकार लंबाई;
	bool res = false;

	text = textplain_get_raw_तथ्य(c, start_idx, end_idx, &लंबाई);
	अगर (text != शून्य) {
		res = selection_string_append(text, लंबाई, false, शून्य, selstr);
	}
	अगर (res == false) {
		वापस NSERROR_NOMEM;
	}
	वापस NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
अटल nserror
textplain_textselection_get_end(काष्ठा content *c, अचिन्हित *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_तथ्य_आकार;
	वापस NSERROR_OK;
}


/**
 * plain text content handler table
 */
अटल स्थिर content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_तथ्य = textplain_process_तथ्य,
	.तथ्य_complete = textplain_convert,
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
nserror textplain_init(व्योम)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	अगर (lerror != lwc_error_ok) {
		वापस NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	अगर (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	अगर (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	वापस error;
}




