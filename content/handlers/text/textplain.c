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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <parserutils/input/inputstream.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/utf8.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/keypress.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/layout.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "content/textsearch.h"
#അന്തര്ഭാവ "content/handlers/css/utils.h"
#അന്തര്ഭാവ "desktop/selection.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "text/textplain.h"

ഘടന textplain_വരി {
	size_t	start;
	size_t	length;
};

/**
 * plain text content
 */
തരനിർവചനം ഘടന textplain_content {
	ഘടന content base;

	lwc_string *encoding;
	ശൂന്യം *inputstream;
	അക്ഷരം *utf8_data;
	size_t utf8_data_size;
	size_t utf8_data_allocated;
	unsigned long physical_വരി_എണ്ണം;
	ഘടന textplain_വരി *physical_വരി;
	പൂർണ്ണസംഖ്യ formatted_width;
	ഘടന browser_window *bw;

	ഘടന selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_SIZE 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

സ്ഥിതം plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.size = TEXT_SIZE,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

സ്ഥിതം പൂർണ്ണസംഖ്യ textplain_tab_width = 256;  /* try for a sensible default */

സ്ഥിതം lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
സ്ഥിതം ശൂന്യം textplain_fini(ശൂന്യം)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ശൂന്യ;
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
സ്ഥിതം parserutils_error
textplain_charset_hack(സ്ഥിരം uint8_t *data,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	തിരികെ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
സ്ഥിതം nserror
textplain_create_internal(textplain_content *c, lwc_string *encoding)
{
	അക്ഷരം *utf8_data;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.size = (nsoption_int(font_size) * PLOT_STYLE_SCALE) / 10;

	utf8_data = malloc(CHUNK);
	എങ്കിൽ (utf8_data == ശൂന്യ)
		പോകുക no_memory;

	error = parserutils_inputstream_create(lwc_string_data(encoding), 0,
					       textplain_charset_hack, &stream);
	എങ്കിൽ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	എങ്കിൽ (error != PARSERUTILS_OK) {
		സ്വതന്ത്രം(utf8_data);
		പോകുക no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_data = utf8_data;
	c->utf8_data_size = 0;
	c->utf8_data_allocated = CHUNK;
	c->physical_വരി = 0;
	c->physical_വരി_എണ്ണം = 0;
	c->formatted_width = 0;
	c->bw = ശൂന്യ;
	c->sel = selection_create((ഘടന content *)c);

	തിരികെ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);

	തിരികെ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
സ്ഥിതം nserror
textplain_create(സ്ഥിരം content_handler *handler,
		 lwc_string *imime_type,
		 സ്ഥിരം ഘടന http_parameter *params,
		 llcache_handle *llcache,
		 സ്ഥിരം അക്ഷരം *fallback_charset,
		 bool quirks,
		 ഘടന content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, വലുപ്പം(textplain_content));
	എങ്കിൽ (text == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(text);
		തിരികെ error;
	}

	error = http_parameter_list_find_item(params, corestring_lwc_charset,
					      &encoding);
	എങ്കിൽ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_internal(text, encoding);
	എങ്കിൽ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		സ്വതന്ത്രം(text);
		തിരികെ error;
	}

	lwc_string_unref(encoding);

	*c = (ഘടന content *) text;

	തിരികെ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
സ്ഥിതം bool
textplain_copy_utf8_data(textplain_content *c, സ്ഥിരം uint8_t *buf, size_t len)
{
	എങ്കിൽ (c->utf8_data_size + len >= c->utf8_data_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		അക്ഷരം *utf8_data;

		allocated = (c->utf8_data_size + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_data = realloc(c->utf8_data, allocated);
		എങ്കിൽ (utf8_data == ശൂന്യ)
			തിരികെ false;

		c->utf8_data = utf8_data;
		c->utf8_data_allocated = allocated;
	}

	memcpy(c->utf8_data + c->utf8_data_size, buf, len);
	c->utf8_data_size += len;

	തിരികെ true;
}


/**
 * drain input
 */
സ്ഥിതം bool
textplain_drain_നിവേശം(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	സ്ഥിതം സ്ഥിരം uint8_t *u_fffd = (സ്ഥിരം uint8_t *) "\xef\xbf\xbd";
	സ്ഥിരം uint8_t *ch;
	size_t chlen, offset = 0;

	ആകുമ്പോൾ (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		എങ്കിൽ (chlen == 1 && *ch == 0) {
			എങ്കിൽ (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				എങ്കിൽ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					തിരികെ false;
			}

			/* Emit U+FFFD */
			എങ്കിൽ (textplain_copy_utf8_data(c, u_fffd, 3) == false)
				തിരികെ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(stream, offset + 1);
			/* Reset the read offset */
			offset = 0;
		} അല്ലെങ്കിൽ {
			/* Accumulate input */
			offset += chlen;

			എങ്കിൽ (offset > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				എങ്കിൽ (textplain_copy_utf8_data(c, ch,
							     offset) == false)
					തിരികെ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(stream, offset);
				/* Reset the read offset */
				offset = 0;
			}
		}
	}

	എങ്കിൽ (offset > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(stream, 0, &ch, &chlen);
		/* Emit any data remaining */
		എങ്കിൽ (textplain_copy_utf8_data(c, ch, offset) == false)
			തിരികെ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	തിരികെ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
സ്ഥിതം bool
textplain_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data, unsigned പൂർണ്ണസംഖ്യ size)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (സ്ഥിരം uint8_t *) data, size);
	എങ്കിൽ (error != PARSERUTILS_OK) {
		പോകുക no_memory;
	}

	എങ്കിൽ (textplain_drain_നിവേശം(text, stream, PARSERUTILS_NEEDDATA) == false)
		പോകുക no_memory;

	തിരികെ true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
	തിരികെ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
സ്ഥിതം bool textplain_convert(ഘടന content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, ശൂന്യ, 0);
	എങ്കിൽ (error != PARSERUTILS_OK) {
		തിരികെ false;
	}

	എങ്കിൽ (textplain_drain_നിവേശം(text, stream, PARSERUTILS_EOF) == false)
		തിരികെ false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = ശൂന്യ;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	തിരികെ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
സ്ഥിതം ദശാംശം textplain_വരി_height(ശൂന്യം)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	തിരികെ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.size / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
സ്ഥിതം ശൂന്യം textplain_reformat(ഘടന content *c, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	textplain_content *text = (textplain_content *) c;
	അക്ഷരം *utf8_data = text->utf8_data;
	size_t utf8_data_size = text->utf8_data_size;
	unsigned long വരി_എണ്ണം = 0;
	ഘടന textplain_വരി *വരി = text->physical_വരി;
	ഘടന textplain_വരി *line1;
	size_t i, space, col;
	size_t columns = 80;
	പൂർണ്ണസംഖ്യ character_width;
	size_t വരി_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_വരി_എണ്ണം = 0;

	എങ്കിൽ (!വരി) {
		text->physical_വരി = വരി =
			malloc(വലുപ്പം(ഘടന textplain_വരി) * (1024 + 3));
		എങ്കിൽ (!വരി)
			പോകുക no_memory;
	}

	വരി[വരി_എണ്ണം++].start = വരി_start = 0;
	space = 0;
	i = 0;
	col = 0;
	ആകുമ്പോൾ (i < utf8_data_size) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t next_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((സ്ഥിരം uint8_t *)utf8_data + i, utf8_data_size - i, &chr, &csize);
		എങ്കിൽ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		next_col = col + 1;

		എങ്കിൽ (chr == '\t') {
			next_col = (next_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		എങ്കിൽ (term || next_col >= columns) {
			എങ്കിൽ (വരി_എണ്ണം % 1024 == 0) {
				line1 = realloc(വരി,
						വലുപ്പം(ഘടന textplain_വരി) *
						(വരി_എണ്ണം + 1024 + 3));
				എങ്കിൽ (!line1)
					പോകുക no_memory;
				text->physical_വരി = വരി = line1;
			}

			എങ്കിൽ (term) {
				വരി[വരി_എണ്ണം-1].length = i - വരി_start;

				/* skip second char of CR/LF or LF/CR pair */
				എങ്കിൽ (i + 1 < utf8_data_size &&
				    utf8_data[i+1] != utf8_data[i] &&
				    (utf8_data[i+1] == '\n' ||
				     utf8_data[i+1] == '\r')) {
					i++;
				}
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (space) {
					/* break at last space in line */
					i = space;
					വരി[വരി_എണ്ണം-1].length = (i + 1) - വരി_start;
				} അല്ലെങ്കിൽ
					വരി[വരി_എണ്ണം-1].length = i - വരി_start;
			}

			വരി[വരി_എണ്ണം++].start = വരി_start = i + 1;
			col = 0;
			space = 0;
		} അല്ലെങ്കിൽ {
			col++;
			എങ്കിൽ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	വരി[വരി_എണ്ണം-1].length = i - വരി[വരി_എണ്ണം-1].start;
	വരി[വരി_എണ്ണം].start = utf8_data_size;

	text->physical_വരി_എണ്ണം = വരി_എണ്ണം;
	c->width = width;
	c->height = വരി_എണ്ണം * textplain_വരി_height() + MARGIN + MARGIN;

	തിരികെ;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", വരി_എണ്ണം);
	തിരികെ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

സ്ഥിതം ശൂന്യം textplain_destroy(ഘടന content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	എങ്കിൽ (text->inputstream != ശൂന്യ) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	എങ്കിൽ (text->physical_വരി != ശൂന്യ) {
		സ്വതന്ത്രം(text->physical_വരി);
	}

	എങ്കിൽ (text->utf8_data != ശൂന്യ) {
		സ്വതന്ത്രം(text->utf8_data);
	}

	എങ്കിൽ (text->sel != ശൂന്യ) {
		selection_destroy(text->sel);
	}
}


സ്ഥിതം nserror textplain_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	സ്ഥിരം textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	സ്ഥിരം uint8_t *data;
	size_t size;

	text = calloc(1, വലുപ്പം(textplain_content));
	എങ്കിൽ (text == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&text->base);
		തിരികെ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&text->base);
		തിരികെ error;
	}

	data = content__get_source_data(&text->base, &size);
	എങ്കിൽ (size > 0) {
		എങ്കിൽ (textplain_process_data(&text->base,
					   (സ്ഥിരം അക്ഷരം *)data,
					   size) == false) {
			content_destroy(&text->base);
			തിരികെ NSERROR_NOMEM;
		}
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	തിരികെ NSERROR_OK;
}


സ്ഥിതം content_type textplain_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_TEXTPLAIN;
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
സ്ഥിതം size_t
textplain_offset_from_coords(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ dir)
{
	textplain_content *textc = (textplain_content *) c;
	ദശാംശം വരി_height = textplain_വരി_height();
	ഘടന textplain_വരി *വരി;
	സ്ഥിരം അക്ഷരം *text;
	unsigned nlines;
	size_t length;
	പൂർണ്ണസംഖ്യ idx;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	y = (പൂർണ്ണസംഖ്യ)((ദശാംശം)(y - MARGIN) / വരി_height);
	x -= MARGIN;

	nlines = textc->physical_വരി_എണ്ണം;
	എങ്കിൽ (!nlines)
		തിരികെ 0;

	എങ്കിൽ (y <= 0) y = 0;
	അല്ലെങ്കിൽ എങ്കിൽ ((unsigned)y >= nlines)
		y = nlines - 1;

	വരി = &textc->physical_വരി[y];
	text = textc->utf8_data + വരി->start;
	length = വരി->length;
	idx = 0;

	ആകുമ്പോൾ (x > 0) {
		size_t next_offset = 0;
		പൂർണ്ണസംഖ്യ width = INT_MAX;

		ആകുമ്പോൾ (next_offset < length && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		എങ്കിൽ (next_offset < length) {
			guit->layout->width(&textplain_style,
					    text,
					    next_offset,
					    &width);
		}

		എങ്കിൽ (x <= width) {
			പൂർണ്ണസംഖ്യ pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, next_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			നിർത്തുക;
		}

		x -= width;
		length -= next_offset;
		text += next_offset;
		idx += next_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		എങ്കിൽ (x <= width) നിർത്തുക;

		x -= width;
		length--;
		text++;
		idx++;
	}

	തിരികെ വരി->start + idx;
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
സ്ഥിതം nserror
textplain_mouse_action(ഘടന content *c,
		       ഘടന browser_window *bw,
		       browser_mouse_state mouse,
		       പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	സ്ഥിരം അക്ഷരം *status = 0;
	size_t idx;
	പൂർണ്ണസംഖ്യ dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, ശൂന്യ);

	idx = textplain_offset_from_coords(c, x, y, dir);
	എങ്കിൽ (selection_click(text->sel, text->bw, mouse, idx)) {

		എങ്കിൽ (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, ശൂന്യ);
			status = messages_get("Selecting");
		}

	} അല്ലെങ്കിൽ {
		എങ്കിൽ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	സന്ദേശം_data.explicit_status_text = status;
	content_broadcast(c, CONTENT_സന്ദേശം_STATUS, &സന്ദേശം_data);

	സന്ദേശം_data.pointer = pointer;
	content_broadcast(c, CONTENT_സന്ദേശം_POINTER, &സന്ദേശം_data);

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror
textplain_mouse_track(ഘടന content *c,
		      ഘടന browser_window *bw,
		      browser_mouse_state mouse,
		      പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	textplain_content *text = (textplain_content *) c;

	എങ്കിൽ (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		പൂർണ്ണസംഖ്യ dir = -1;
		size_t idx;

		എങ്കിൽ (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, ശൂന്യ);
	}

	തിരഞ്ഞെടുപ്പ് (browser_window_get_drag_type(bw)) {

	അവസ്ഥ DRAGGING_SELECTION: {
		പൂർണ്ണസംഖ്യ dir = -1;
		size_t idx;

		എങ്കിൽ (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		textplain_mouse_action(c, bw, mouse, x, y);
		നിർത്തുക;
	}

	തിരികെ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
സ്ഥിതം bool textplain_keypress(ഘടന content *c, uint32_t താക്കോൽ)
{
	textplain_content *text = (textplain_content *) c;
	ഘടന selection *sel = text->sel;

	തിരഞ്ഞെടുപ്പ് (താക്കോൽ) {
	അവസ്ഥ NS_താക്കോൽ_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_CLEAR_SELECTION:
		selection_clear(sel, true);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_SELECT_ALL:
		selection_select_all(sel);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		തിരികെ selection_clear(sel, true);
	}

	തിരികെ false;
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
സ്ഥിതം bool
text_draw(സ്ഥിരം അക്ഷരം *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    പൂർണ്ണസംഖ്യ x,
	    പൂർണ്ണസംഖ്യ y,
	    സ്ഥിരം ഘടന rect *clip,
	    പൂർണ്ണസംഖ്യ height,
	    ദശാംശം scale,
	    textplain_content *text,
	    സ്ഥിരം ഘടന selection *sel,
	    സ്ഥിരം ഘടന redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	എങ്കിൽ (ctx->interactive == true) {
		unsigned len = utf8_len;
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		എങ്കിൽ (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		എങ്കിൽ (!highlighted &&
		    (text->base.textsearch.context != ശൂന്യ) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		എങ്കിൽ (highlighted) {
			ഘടന rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			പൂർണ്ണസംഖ്യ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			എങ്കിൽ (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				പ്രതിജ്ഞാ(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			എങ്കിൽ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(&textplain_style,
						  utf8_text,
						  endtxt_idx,
						  &endx);
			എങ്കിൽ (res != NSERROR_OK) {
				endx = 0;
			}

			എങ്കിൽ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			എങ്കിൽ (start_idx > 0) {
				res = ctx->plot->text(ctx,
					      &plot_fstyle,
					      x,
					      y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				എങ്കിൽ (res != NSERROR_OK) {
					തിരികെ false;
				}
			}

			pstyle_fill_hback.fill_colour = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}

			എങ്കിൽ (start_idx > 0) {
				പൂർണ്ണസംഖ്യ px0 = max(x + startx, clip->x0);
				പൂർണ്ണസംഖ്യ px1 = min(x + endx, clip->x1);

				എങ്കിൽ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}

					clip_changed = true;
				} അല്ലെങ്കിൽ {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			എങ്കിൽ (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				തിരികെ false;
			}

			/* draw any text succeeding highlighted portion */
			എങ്കിൽ (endtxt_idx < utf8_len) {
				പൂർണ്ണസംഖ്യ px0 = max(x + endx, clip->x0);
				എങ്കിൽ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}
				}
			}

			എങ്കിൽ (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				തിരികെ false;
			}
		}
	}

	എങ്കിൽ (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (പൂർണ്ണസംഖ്യ) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}
	തിരികെ true;
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
സ്ഥിതം bool
textplain_redraw(ഘടന content *c,
		 ഘടന content_redraw_data *data,
		 സ്ഥിരം ഘടന rect *clip,
		 സ്ഥിരം ഘടന redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	ഘടന browser_window *bw = text->bw;
	അക്ഷരം *utf8_data = text->utf8_data;
	long lineno;
	പൂർണ്ണസംഖ്യ x = data->x;
	പൂർണ്ണസംഖ്യ y = data->y;
	unsigned long വരി_എണ്ണം = text->physical_വരി_എണ്ണം;
	ദശാംശം വരി_height = textplain_വരി_height();
	ദശാംശം scaled_വരി_height = വരി_height * data->scale;
	long line0 = (clip->y0 - y * data->scale) / scaled_വരി_height - 1;
	long line1 = (clip->y1 - y * data->scale) / scaled_വരി_height + 1;
	ഘടന textplain_വരി *വരി = text->physical_വരി;
	size_t length;
	plot_style_t *plot_style_highlight;
	nserror res;

	എങ്കിൽ (line0 < 0)
		line0 = 0;
	എങ്കിൽ (line1 < 0)
		line1 = 0;
	എങ്കിൽ (വരി_എണ്ണം < (unsigned long) line0)
		line0 = വരി_എണ്ണം;
	എങ്കിൽ (വരി_എണ്ണം < (unsigned long) line1)
		line1 = വരി_എണ്ണം;
	എങ്കിൽ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	എങ്കിൽ (!വരി)
		തിരികെ true;

	/* choose a suitable background colour for any highlighted text */
	എങ്കിൽ ((data->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	അല്ലെങ്കിൽ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = data->background_colour;

	x = (x + MARGIN) * data->scale;
	y = (y + MARGIN) * data->scale;
	വേണ്ടി (lineno = line0; lineno != line1; lineno++) {
		സ്ഥിരം അക്ഷരം *text_d = utf8_data + വരി[lineno].start;
		പൂർണ്ണസംഖ്യ tab_width = textplain_tab_width * data->scale;
		size_t offset = 0;
		പൂർണ്ണസംഖ്യ tx = x;

		എങ്കിൽ (!tab_width) tab_width = 1;

		length = വരി[lineno].length;
		എങ്കിൽ (!length)
			തുടരുക;

		ആകുമ്പോൾ (offset < length) {
			size_t next_offset = offset;
			പൂർണ്ണസംഖ്യ width;
			പൂർണ്ണസംഖ്യ ntx;
			nserror res;

			ആകുമ്പോൾ ((next_offset < length) &&
			       (text_d[next_offset] != '\t')) {
				next_offset = utf8_next(text_d,
							length,
							next_offset);
			}

			എങ്കിൽ (!text_draw(text_d + offset,
				       next_offset - offset,
				       വരി[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_വരി_height),
				       clip,
				       വരി_height,
				       data->scale,
				       text,
				       text->sel,
				       ctx)) {
				തിരികെ false;
			}

			എങ്കിൽ (next_offset >= length)
				നിർത്തുക;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  next_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			എങ്കിൽ (res == NSERROR_OK) {
				tx += (പൂർണ്ണസംഖ്യ)(width * data->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			എങ്കിൽ (bw) {
				unsigned tab_ofst = വരി[lineno].start + next_offset;
				ഘടന selection *sel = text->sel;
				bool highlighted = false;

				unsigned start_idx, end_idx;
				എങ്കിൽ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &end_idx)) {
					highlighted = true;
				}


				എങ്കിൽ (!highlighted &&
				    (c->textsearch.context != ശൂന്യ)) {
					unsigned start_idx, end_idx;
					എങ്കിൽ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &end_idx)) {
						highlighted = true;
					}
				}

				എങ്കിൽ (highlighted) {
					ഘടന rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_വരി_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_വരി_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}
				}
			}

			offset = next_offset + 1;
			tx = ntx;
		}
	}

	തിരികെ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
സ്ഥിതം nserror
textplain_open(ഘടന content *c,
	       ഘടന browser_window *bw,
	       ഘടന content *page,
	       ഘടന object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	തിരികെ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
സ്ഥിതം nserror textplain_close(ഘടന content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = ശൂന്യ;

	തിരികെ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
സ്ഥിതം അക്ഷരം *textplain_get_selection(ഘടന content *c)
{
	textplain_content *text = (textplain_content *) c;

	തിരികെ selection_get_copy(text->sel);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
textplain_coord_from_offset(സ്ഥിരം അക്ഷരം *text, size_t offset, size_t length)
{
	പൂർണ്ണസംഖ്യ x = 0;

	ആകുമ്പോൾ (offset > 0) {
		size_t next_offset = 0;
		പൂർണ്ണസംഖ്യ tx;

		ആകുമ്പോൾ (next_offset < offset && text[next_offset] != '\t') {
			next_offset = utf8_next(text, length, next_offset);
		}

		guit->layout->width(&textplain_style, text, next_offset, &tx);

		x += tx;

		എങ്കിൽ (next_offset >= offset)
			നിർത്തുക;

		/* align to next tab boundary */
		next_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= next_offset;
		text += next_offset;
		length -= next_offset;
	}

	തിരികെ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
സ്ഥിതം unsigned long textplain_വരി_എണ്ണം(ഘടന content *c)
{
	textplain_content *text = (textplain_content *) c;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ text->physical_വരി_എണ്ണം;
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
സ്ഥിതം അക്ഷരം *
textplain_get_വരി(ഘടന content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	ഘടന textplain_വരി *വരി;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (lineno >= text->physical_വരി_എണ്ണം)
		തിരികെ ശൂന്യ;
	വരി = &text->physical_വരി[lineno];

	*poffset = വരി->start;
	*plen = വരി->length;
	തിരികെ text->utf8_data + വരി->start;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ textplain_find_വരി(ഘടന content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	ഘടന textplain_വരി *വരി;
	പൂർണ്ണസംഖ്യ nlines;
	പൂർണ്ണസംഖ്യ lineno = 0;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	വരി = text->physical_വരി;
	nlines = text->physical_വരി_എണ്ണം;

	എങ്കിൽ (offset > text->utf8_data_size) {
		തിരികെ -1;
	}

/* \todo - implement binary search here */
	ആകുമ്പോൾ (lineno < nlines && വരി[lineno].start < offset) {
		lineno++;
	}
	എങ്കിൽ (വരി[lineno].start > offset) {
		lineno--;
	}

	തിരികെ lineno;
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
സ്ഥിതം nserror
textplain_textsearch_find(ഘടന content *c,
			  ഘടന textsearch_context *context,
			  സ്ഥിരം അക്ഷരം *pattern,
			  പൂർണ്ണസംഖ്യ p_len,
			  bool case_sens)
{
	പൂർണ്ണസംഖ്യ nlines = textplain_വരി_എണ്ണം(c);
	പൂർണ്ണസംഖ്യ വരി;
	nserror res = NSERROR_OK;

	വേണ്ടി(വരി = 0; വരി < nlines; വരി++) {
		size_t offset, length;
		സ്ഥിരം അക്ഷരം *text;

		text = textplain_get_വരി(c, വരി, &offset, &length);
		എങ്കിൽ (text) {
			ആകുമ്പോൾ (length > 0) {
				unsigned match_length;
				size_t start_idx;
				സ്ഥിരം അക്ഷരം *new_text;
				സ്ഥിരം അക്ഷരം *pos;

				pos = content_textsearch_find_pattern(
						text,
						length,
						pattern,
						p_len,
						case_sens,
						&match_length);
				എങ്കിൽ (!pos)
					നിർത്തുക;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_length,
						ശൂന്യ,
						ശൂന്യ);
				എങ്കിൽ (res != NSERROR_OK) {
					തിരികെ res;
				}

				new_text = pos + match_length;
				offset += (new_text - text);
				length -= (new_text - text);
				text = new_text;
			}
		}
	}

	തിരികെ res;
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
സ്ഥിതം ശൂന്യം
textplain_coords_from_range(ഘടന content *c,
			    unsigned start,
			    unsigned end,
			    ഘടന rect *r)
{
	textplain_content *text = (textplain_content *) c;
	ദശാംശം വരി_height = textplain_വരി_height();
	അക്ഷരം *utf8_data;
	ഘടന textplain_വരി *വരി;
	unsigned lineno = 0;
	unsigned nlines;

	പ്രതിജ്ഞാ(c != ശൂന്യ);
	പ്രതിജ്ഞാ(start <= end);
	പ്രതിജ്ഞാ(end <= text->utf8_data_size);

	utf8_data = text->utf8_data;
	nlines = text->physical_വരി_എണ്ണം;
	വരി = text->physical_വരി;

	/* find start */
	lineno = textplain_find_വരി(c, start);

	r->y0 = (പൂർണ്ണസംഖ്യ)(MARGIN + lineno * വരി_height);

	എങ്കിൽ (lineno + 1 <= nlines || വരി[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_വരി(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} അല്ലെങ്കിൽ {
		/* single line */
		സ്ഥിരം അക്ഷരം *text = utf8_data + വരി[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - വരി[lineno].start,
						    വരി[lineno].length);

		r->x1 = textplain_coord_from_offset(text,
						    end - വരി[lineno].start,
						    വരി[lineno].length);
	}

	r->y1 = (പൂർണ്ണസംഖ്യ)(MARGIN + (lineno + 1) * വരി_height);
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
സ്ഥിതം അക്ഷരം *
textplain_get_raw_data(ഘടന content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_size;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	utf8_size = text->utf8_data_size;

	/* any text at all? */
	എങ്കിൽ (!utf8_size) തിരികെ ശൂന്യ;

	/* clamp to valid offset range */
	എങ്കിൽ (start >= utf8_size) start = utf8_size;
	എങ്കിൽ (end >= utf8_size) end = utf8_size;

	*plen = end - start;

	തിരികെ text->utf8_data + start;
}


/**
 * get bounds of a free text search match
 */
സ്ഥിതം nserror
textplain_textsearch_bounds(ഘടന content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    ഘടന box *start_box,
			    ഘടന box *end_box,
			    ഘടന rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	തിരികെ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
സ്ഥിതം nserror
textplain_textselection_redraw(ഘടന content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	ഘടന rect r;

	എങ്കിൽ (end_idx <= start_idx) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror
textplain_textselection_copy(ഘടന content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     ഘടന selection_string *selstr)
{
	സ്ഥിരം അക്ഷരം *text;
	size_t length;
	bool res = false;

	text = textplain_get_raw_data(c, start_idx, end_idx, &length);
	എങ്കിൽ (text != ശൂന്യ) {
		res = selection_string_append(text, length, false, ശൂന്യ, selstr);
	}
	എങ്കിൽ (res == false) {
		തിരികെ NSERROR_NOMEM;
	}
	തിരികെ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
സ്ഥിതം nserror
textplain_textselection_get_end(ഘടന content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_data_size;
	തിരികെ NSERROR_OK;
}


/**
 * plain text content handler table
 */
സ്ഥിതം സ്ഥിരം content_handler textplain_content_handler = {
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
nserror textplain_init(ശൂന്യം)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	എങ്കിൽ (lerror != lwc_error_ok) {
		തിരികെ NSERROR_NOMEM;
	}

	error = content_factory_register_handler("text/plain",
						 &textplain_content_handler);
	എങ്കിൽ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_register_handler("application/json",
						 &textplain_content_handler);
	എങ്കിൽ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	തിരികെ error;
}




