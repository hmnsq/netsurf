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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <parserutils/input/inputstream.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/http.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/utf8.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/keypress.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/layout.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "content/textsearch.h"
#অন্তর্ভাব "content/handlers/css/utils.h"
#অন্তর্ভাব "desktop/selection.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "text/textplain.h"

কাঠামো textplain_সারি {
	size_t	start;
	size_t	দৈর্ঘ্য;
};

/**
 * plain text content
 */
প্রকারলেখ কাঠামো textplain_content {
	কাঠামো content base;

	lwc_string *encoding;
	শূন্য *inputstream;
	অক্ষর *utf8_তথ্য;
	size_t utf8_তথ্য_আকার;
	size_t utf8_তথ্য_allocated;
	unsigned long physical_সারি_গুনতি;
	কাঠামো textplain_সারি *physical_সারি;
	পূর্ণসংখ্যা formatted_width;
	কাঠামো browser_window *bw;

	কাঠামো selection *sel; /** Selection state */

} textplain_content;


#define CHUNK 32768 /* Must be a power of 2 */
#define MARGIN 4

#define TAB_WIDTH 8  /* must be power of 2 currently */
#define TEXT_আকার 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

স্থির plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.আকার_2 = TEXT_আকার,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

স্থির পূর্ণসংখ্যা textplain_tab_width = 256;  /* try for a sensible default */

স্থির lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
স্থির শূন্য textplain_fini(শূন্য)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ফাঁকা;
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
textplain_charset_hack(ধ্রুবক uint8_t *তথ্য,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *source)
{
	ফেরত PARSERUTILS_OK;
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
	অক্ষর *utf8_তথ্য;
	parserutils_inputstream *stream;
	parserutils_error error;

	textplain_style.আকার_2 = (nsoption_int(font_আকার) * PLOT_STYLE_SCALE) / 10;

	utf8_তথ্য = malloc(CHUNK);
	যদি (utf8_তথ্য == ফাঁকা)
		যাও no_memory;

	error = parserutils_inputstream_create(lwc_string_তথ্য(encoding), 0,
					       textplain_charset_hack, &stream);
	যদি (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &stream);
	}
	যদি (error != PARSERUTILS_OK) {
		মুক্ত(utf8_তথ্য);
		যাও no_memory;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = stream;
	c->utf8_তথ্য = utf8_তথ্য;
	c->utf8_তথ্য_আকার = 0;
	c->utf8_তথ্য_allocated = CHUNK;
	c->physical_সারি = 0;
	c->physical_সারি_গুনতি = 0;
	c->formatted_width = 0;
	c->bw = ফাঁকা;
	c->sel = selection_create((কাঠামো content *)c);

	ফেরত NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);

	ফেরত NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
স্থির nserror
textplain_create(ধ্রুবক content_handler *handler,
		 lwc_string *imime_type,
		 ধ্রুবক কাঠামো http_parameter *params,
		 llcache_handle *llcache,
		 ধ্রুবক অক্ষর *fallback_charset,
		 bool quirks,
		 কাঠামো content **c)
{
	textplain_content *text;
	nserror error;
	lwc_string *encoding;

	text = calloc(1, আকার(textplain_content));
	যদি (text == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	error = content__init(&text->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(text);
		ফেরত error;
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
		ফেরত error;
	}

	lwc_string_unref(encoding);

	*c = (কাঠামো content *) text;

	ফেরত NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
স্থির bool
textplain_copy_utf8_তথ্য(textplain_content *c, ধ্রুবক uint8_t *buf, size_t len)
{
	যদি (c->utf8_তথ্য_আকার + len >= c->utf8_তথ্য_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		অক্ষর *utf8_তথ্য;

		allocated = (c->utf8_তথ্য_আকার + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_তথ্য = realloc(c->utf8_তথ্য, allocated);
		যদি (utf8_তথ্য == ফাঁকা)
			ফেরত false;

		c->utf8_তথ্য = utf8_তথ্য;
		c->utf8_তথ্য_allocated = allocated;
	}

	memcpy(c->utf8_তথ্য + c->utf8_তথ্য_আকার, buf, len);
	c->utf8_তথ্য_আকার += len;

	ফেরত true;
}


/**
 * drain input
 */
স্থির bool
textplain_drain_প্রবেশ(textplain_content *c,
		      parserutils_inputstream *stream,
		      parserutils_error terminator)
{
	স্থির ধ্রুবক uint8_t *u_fffd = (ধ্রুবক uint8_t *) "\xef\xbf\xbd";
	ধ্রুবক uint8_t *ch;
	size_t chlen, offset = 0;

	যখন (parserutils_inputstream_peek(stream, offset, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		যদি (chlen == 1 && *ch == 0) {
			যদি (offset > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(stream, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				যদি (textplain_copy_utf8_তথ্য(c, ch,
							     offset) == false)
					ফেরত false;
			}

			/* Emit U+FFFD */
			যদি (textplain_copy_utf8_তথ্য(c, u_fffd, 3) == false)
				ফেরত false;

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
				যদি (textplain_copy_utf8_তথ্য(c, ch,
							     offset) == false)
					ফেরত false;

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
		যদি (textplain_copy_utf8_তথ্য(c, ch, offset) == false)
			ফেরত false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(stream, offset);
	}

	ফেরত true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
স্থির bool
textplain_process_তথ্য(কাঠামো content *c, ধ্রুবক অক্ষর *তথ্য, unsigned পূর্ণসংখ্যা আকার_2)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream,
					       (ধ্রুবক uint8_t *) তথ্য, আকার_2);
	যদি (error != PARSERUTILS_OK) {
		যাও no_memory;
	}

	যদি (textplain_drain_প্রবেশ(text, stream, PARSERUTILS_NEEDDATA) == false)
		যাও no_memory;

	ফেরত true;

no_memory:
	content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
	ফেরত false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
স্থির bool textplain_convert(কাঠামো content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *stream = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(stream, ফাঁকা, 0);
	যদি (error != PARSERUTILS_OK) {
		ফেরত false;
	}

	যদি (textplain_drain_প্রবেশ(text, stream, PARSERUTILS_EOF) == false)
		ফেরত false;

	parserutils_inputstream_destroy(stream);
	text->inputstream = ফাঁকা;

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, messages_get("Done"));

	ফেরত true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
স্থির দশমিক textplain_সারি_height(শূন্য)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ফেরত FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.আকার_2 / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
স্থির শূন্য textplain_reformat(কাঠামো content *c, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	textplain_content *text = (textplain_content *) c;
	অক্ষর *utf8_তথ্য = text->utf8_তথ্য;
	size_t utf8_তথ্য_আকার = text->utf8_তথ্য_আকার;
	unsigned long সারি_গুনতি = 0;
	কাঠামো textplain_সারি *সারি = text->physical_সারি;
	কাঠামো textplain_সারি *line1;
	size_t i, space, col;
	size_t columns = 80;
	পূর্ণসংখ্যা character_width;
	size_t সারি_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, width, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->width(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_width);
	যদি (res != NSERROR_OK) {
		ফেরত;
	}

	columns = (width - MARGIN - MARGIN) * 8 / character_width;
	textplain_tab_width = (TAB_WIDTH * character_width) / 8;

	text->formatted_width = width;

	text->physical_সারি_গুনতি = 0;

	যদি (!সারি) {
		text->physical_সারি = সারি =
			malloc(আকার(কাঠামো textplain_সারি) * (1024 + 3));
		যদি (!সারি)
			যাও no_memory;
	}

	সারি[সারি_গুনতি++].start = সারি_start = 0;
	space = 0;
	i = 0;
	col = 0;
	যখন (i < utf8_তথ্য_আকার) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool term;
		size_t পরবর্তী_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_to_ucs4((ধ্রুবক uint8_t *)utf8_তথ্য + i, utf8_তথ্য_আকার - i, &chr, &csize);
		যদি (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		term = (chr == '\n' || chr == '\r');

		পরবর্তী_col = col + 1;

		যদি (chr == '\t') {
			পরবর্তী_col = (পরবর্তী_col + TAB_WIDTH - 1) & ~(TAB_WIDTH - 1);
		}

		যদি (term || পরবর্তী_col >= columns) {
			যদি (সারি_গুনতি % 1024 == 0) {
				line1 = realloc(সারি,
						আকার(কাঠামো textplain_সারি) *
						(সারি_গুনতি + 1024 + 3));
				যদি (!line1)
					যাও no_memory;
				text->physical_সারি = সারি = line1;
			}

			যদি (term) {
				সারি[সারি_গুনতি-1].দৈর্ঘ্য = i - সারি_start;

				/* skip second char of CR/LF or LF/CR pair */
				যদি (i + 1 < utf8_তথ্য_আকার &&
				    utf8_তথ্য[i+1] != utf8_তথ্য[i] &&
				    (utf8_তথ্য[i+1] == '\n' ||
				     utf8_তথ্য[i+1] == '\r')) {
					i++;
				}
			} অন্যথা {
				যদি (space) {
					/* break at last space in line */
					i = space;
					সারি[সারি_গুনতি-1].দৈর্ঘ্য = (i + 1) - সারি_start;
				} অন্যথা
					সারি[সারি_গুনতি-1].দৈর্ঘ্য = i - সারি_start;
			}

			সারি[সারি_গুনতি++].start = সারি_start = i + 1;
			col = 0;
			space = 0;
		} অন্যথা {
			col++;
			যদি (chr == ' ')
				space = i;
		}
		i += csize;
	}
	সারি[সারি_গুনতি-1].দৈর্ঘ্য = i - সারি[সারি_গুনতি-1].start;
	সারি[সারি_গুনতি].start = utf8_তথ্য_আকার;

	text->physical_সারি_গুনতি = সারি_গুনতি;
	c->width = width;
	c->height = সারি_গুনতি * textplain_সারি_height() + MARGIN + MARGIN;

	ফেরত;

no_memory:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", সারি_গুনতি);
	ফেরত;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

স্থির শূন্য textplain_destroy(কাঠামো content *c)
{
	textplain_content *text = (textplain_content *) c;

	lwc_string_unref(text->encoding);

	যদি (text->inputstream != ফাঁকা) {
		parserutils_inputstream_destroy(text->inputstream);
	}

	যদি (text->physical_সারি != ফাঁকা) {
		মুক্ত(text->physical_সারি);
	}

	যদি (text->utf8_তথ্য != ফাঁকা) {
		মুক্ত(text->utf8_তথ্য);
	}

	যদি (text->sel != ফাঁকা) {
		selection_destroy(text->sel);
	}
}


স্থির nserror textplain_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	ধ্রুবক textplain_content *old_text = (textplain_content *) old;
	textplain_content *text;
	nserror error;
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;

	text = calloc(1, আকার(textplain_content));
	যদি (text == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &text->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&text->base);
		ফেরত error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_internal(text, old_text->encoding);
	যদি (error != NSERROR_OK) {
		content_destroy(&text->base);
		ফেরত error;
	}

	তথ্য = content_get_source_তথ্য_2(&text->base, &আকার_2);
	যদি (আকার_2 > 0) {
		যদি (textplain_process_তথ্য(&text->base,
					   (ধ্রুবক অক্ষর *)তথ্য,
					   আকার_2) == false) {
			content_destroy(&text->base);
			ফেরত NSERROR_NOMEM;
		}
	}

	যদি (old->status == CONTENT_STATUS_READY ||
	    old->status == CONTENT_STATUS_DONE) {
		যদি (textplain_convert(&text->base) == false) {
			content_destroy(&text->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	ফেরত NSERROR_OK;
}


স্থির content_type textplain_content_type(শূন্য)
{
	ফেরত CONTENT_TEXTPLAIN;
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
textplain_offset_from_coords(কাঠামো content *c, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা dir)
{
	textplain_content *textc = (textplain_content *) c;
	দশমিক সারি_height = textplain_সারি_height();
	কাঠামো textplain_সারি *সারি;
	ধ্রুবক অক্ষর *text;
	unsigned nlines;
	size_t দৈর্ঘ্য;
	পূর্ণসংখ্যা idx;

	প্রতিজ্ঞা(c != ফাঁকা);

	y = (পূর্ণসংখ্যা)((দশমিক)(y - MARGIN) / সারি_height);
	x -= MARGIN;

	nlines = textc->physical_সারি_গুনতি;
	যদি (!nlines)
		ফেরত 0;

	যদি (y <= 0) y = 0;
	অন্যথা যদি ((unsigned)y >= nlines)
		y = nlines - 1;

	সারি = &textc->physical_সারি[y];
	text = textc->utf8_তথ্য + সারি->start;
	দৈর্ঘ্য = সারি->দৈর্ঘ্য;
	idx = 0;

	যখন (x > 0) {
		size_t পরবর্তী_offset = 0;
		পূর্ণসংখ্যা width = INT_MAX;

		যখন (পরবর্তী_offset < দৈর্ঘ্য && text[পরবর্তী_offset] != '\t') {
			পরবর্তী_offset = utf8_পরবর্তী(text, দৈর্ঘ্য, পরবর্তী_offset);
		}

		যদি (পরবর্তী_offset < দৈর্ঘ্য) {
			guit->layout->width(&textplain_style,
					    text,
					    পরবর্তী_offset,
					    &width);
		}

		যদি (x <= width) {
			পূর্ণসংখ্যা pixel_offset;
			size_t char_offset;

			guit->layout->position(&textplain_style,
					       text, পরবর্তী_offset, x,
					       &char_offset, &pixel_offset);

			idx += char_offset;
			বিরতি;
		}

		x -= width;
		দৈর্ঘ্য -= পরবর্তী_offset;
		text += পরবর্তী_offset;
		idx += পরবর্তী_offset;

		/* check if it's within the tab */
		width = textplain_tab_width - (width % textplain_tab_width);
		যদি (x <= width) বিরতি;

		x -= width;
		দৈর্ঘ্য--;
		text++;
		idx++;
	}

	ফেরত সারি->start + idx;
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
textplain_mouse_action(কাঠামো content *c,
		       কাঠামো browser_window *bw,
		       browser_mouse_state mouse,
		       পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	ধ্রুবক অক্ষর *status = 0;
	size_t idx;
	পূর্ণসংখ্যা dir = 0;

	browser_window_set_drag_type(bw, DRAGGING_NONE, ফাঁকা);

	idx = textplain_offset_from_coords(c, x, y, dir);
	যদি (selection_click(text->sel, text->bw, mouse, idx)) {

		যদি (selection_dragging(text->sel)) {
			browser_window_set_drag_type(bw,
						     DRAGGING_SELECTION, ফাঁকা);
			status = messages_get("Selecting");
		}

	} অন্যথা {
		যদি (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	বার্তা_তথ্য.explicit_status_text = status;
	content_broadcast(c, CONTENT_বার্তা_STATUS, &বার্তা_তথ্য);

	বার্তা_তথ্য.pointer = pointer;
	content_broadcast(c, CONTENT_বার্তা_POINTER, &বার্তা_তথ্য);

	ফেরত NSERROR_OK;
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
textplain_mouse_track(কাঠামো content *c,
		      কাঠামো browser_window *bw,
		      browser_mouse_state mouse,
		      পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	textplain_content *text = (textplain_content *) c;

	যদি (browser_window_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		পূর্ণসংখ্যা dir = -1;
		size_t idx;

		যদি (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, ফাঁকা);
	}

	নির্বাচন (browser_window_get_drag_type(bw)) {

	ক্ষেত্র DRAGGING_SELECTION: {
		পূর্ণসংখ্যা dir = -1;
		size_t idx;

		যদি (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_offset_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);
	}
		বিরতি;

	পূর্বনির্ধারিত:
		textplain_mouse_action(c, bw, mouse, x, y);
		বিরতি;
	}

	ফেরত NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
স্থির bool textplain_keypress(কাঠামো content *c, uint32_t চাবি)
{
	textplain_content *text = (textplain_content *) c;
	কাঠামো selection *sel = text->sel;

	নির্বাচন (চাবি) {
	ক্ষেত্র NS_চাবি_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ফেরত true;

	ক্ষেত্র NS_চাবি_CLEAR_SELECTION:
		selection_clear(sel, true);
		ফেরত true;

	ক্ষেত্র NS_চাবি_SELECT_ALL:
		selection_select_all(sel);
		ফেরত true;

	ক্ষেত্র NS_চাবি_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ফেরত selection_clear(sel, true);
	}

	ফেরত false;
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
text_draw(ধ্রুবক অক্ষর *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    পূর্ণসংখ্যা x,
	    পূর্ণসংখ্যা y,
	    ধ্রুবক কাঠামো rect *clip,
	    পূর্ণসংখ্যা height,
	    দশমিক scale,
	    textplain_content *text,
	    ধ্রুবক কাঠামো selection *sel,
	    ধ্রুবক কাঠামো redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.আকার_2 *= scale;

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
		    (text->base.textsearch.context != ফাঁকা) &&
		    content_textsearch_ishighlighted(text->base.textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		যদি (highlighted) {
			কাঠামো rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			পূর্ণসংখ্যা startx, endx;
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
					      y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
					      utf8_text,
					      start_idx);
				যদি (res != NSERROR_OK) {
					ফেরত false;
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
				ফেরত false;
			}

			যদি (start_idx > 0) {
				পূর্ণসংখ্যা px0 = max(x + startx, clip->x0);
				পূর্ণসংখ্যা px1 = min(x + endx, clip->x1);

				যদি (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ফেরত false;
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
					     y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ফেরত false;
			}

			/* draw any text succeeding highlighted portion */
			যদি (endtxt_idx < utf8_len) {
				পূর্ণসংখ্যা px0 = max(x + endx, clip->x0);
				যদি (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}
				}
			}

			যদি (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ফেরত false;
			}
		}
	}

	যদি (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (পূর্ণসংখ্যা) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}
	ফেরত true;
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
textplain_redraw(কাঠামো content *c,
		 কাঠামো content_redraw_তথ্য *তথ্য,
		 ধ্রুবক কাঠামো rect *clip,
		 ধ্রুবক কাঠামো redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	কাঠামো browser_window *bw = text->bw;
	অক্ষর *utf8_তথ্য = text->utf8_তথ্য;
	long lineno;
	পূর্ণসংখ্যা x = তথ্য->x;
	পূর্ণসংখ্যা y = তথ্য->y;
	unsigned long সারি_গুনতি = text->physical_সারি_গুনতি;
	দশমিক সারি_height = textplain_সারি_height();
	দশমিক scaled_সারি_height = সারি_height * তথ্য->scale;
	long line0 = (clip->y0 - y * তথ্য->scale) / scaled_সারি_height - 1;
	long line1 = (clip->y1 - y * তথ্য->scale) / scaled_সারি_height + 1;
	কাঠামো textplain_সারি *সারি = text->physical_সারি;
	size_t দৈর্ঘ্য;
	plot_style_t *plot_style_highlight;
	nserror res;

	যদি (line0 < 0)
		line0 = 0;
	যদি (line1 < 0)
		line1 = 0;
	যদি (সারি_গুনতি < (unsigned long) line0)
		line0 = সারি_গুনতি;
	যদি (সারি_গুনতি < (unsigned long) line1)
		line1 = সারি_গুনতি;
	যদি (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	যদি (!সারি)
		ফেরত true;

	/* choose a suitable background colour for any highlighted text */
	যদি ((তথ্য->background_colour & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	অন্যথা
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = তথ্য->background_colour;

	x = (x + MARGIN) * তথ্য->scale;
	y = (y + MARGIN) * তথ্য->scale;
	জন্য (lineno = line0; lineno != line1; lineno++) {
		ধ্রুবক অক্ষর *text_d = utf8_তথ্য + সারি[lineno].start;
		পূর্ণসংখ্যা tab_width = textplain_tab_width * তথ্য->scale;
		size_t offset = 0;
		পূর্ণসংখ্যা tx = x;

		যদি (!tab_width) tab_width = 1;

		দৈর্ঘ্য = সারি[lineno].দৈর্ঘ্য;
		যদি (!দৈর্ঘ্য)
			চালু;

		যখন (offset < দৈর্ঘ্য) {
			size_t পরবর্তী_offset = offset;
			পূর্ণসংখ্যা width;
			পূর্ণসংখ্যা ntx;
			nserror res;

			যখন ((পরবর্তী_offset < দৈর্ঘ্য) &&
			       (text_d[পরবর্তী_offset] != '\t')) {
				পরবর্তী_offset = utf8_পরবর্তী(text_d,
							দৈর্ঘ্য,
							পরবর্তী_offset);
			}

			যদি (!text_draw(text_d + offset,
				       পরবর্তী_offset - offset,
				       সারি[lineno].start + offset,
				       tx,
				       y + (lineno * scaled_সারি_height),
				       clip,
				       সারি_height,
				       তথ্য->scale,
				       text,
				       text->sel,
				       ctx)) {
				ফেরত false;
			}

			যদি (পরবর্তী_offset >= দৈর্ঘ্য)
				বিরতি;

			res = guit->layout->width(&textplain_style,
						  &text_d[offset],
						  পরবর্তী_offset - offset,
						  &width);
			/* locate end of string and align to next tab position */
			যদি (res == NSERROR_OK) {
				tx += (পূর্ণসংখ্যা)(width * তথ্য->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_width) * tab_width);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			যদি (bw) {
				unsigned tab_ofst = সারি[lineno].start + পরবর্তী_offset;
				কাঠামো selection *sel = text->sel;
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
				    (c->textsearch.context != ফাঁকা)) {
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
					কাঠামো rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_সারি_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_সারি_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}
				}
			}

			offset = পরবর্তী_offset + 1;
			tx = ntx;
		}
	}

	ফেরত true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
স্থির nserror
textplain_open(কাঠামো content *c,
	       কাঠামো browser_window *bw,
	       কাঠামো content *page,
	       কাঠামো object_params *params)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = bw;

	/* text selection */
	selection_init(text->sel);

	ফেরত NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
স্থির nserror textplain_close(কাঠামো content *c)
{
	textplain_content *text = (textplain_content *) c;

	text->bw = ফাঁকা;

	ফেরত NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
স্থির অক্ষর *textplain_get_selection(কাঠামো content *c)
{
	textplain_content *text = (textplain_content *) c;

	ফেরত selection_get_copy(text->sel);
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
স্থির পূর্ণসংখ্যা
textplain_coord_from_offset(ধ্রুবক অক্ষর *text, size_t offset, size_t দৈর্ঘ্য)
{
	পূর্ণসংখ্যা x = 0;

	যখন (offset > 0) {
		size_t পরবর্তী_offset = 0;
		পূর্ণসংখ্যা tx;

		যখন (পরবর্তী_offset < offset && text[পরবর্তী_offset] != '\t') {
			পরবর্তী_offset = utf8_পরবর্তী(text, দৈর্ঘ্য, পরবর্তী_offset);
		}

		guit->layout->width(&textplain_style, text, পরবর্তী_offset, &tx);

		x += tx;

		যদি (পরবর্তী_offset >= offset)
			বিরতি;

		/* align to next tab boundary */
		পরবর্তী_offset++;
		x = (1 + (x / textplain_tab_width)) * textplain_tab_width;
		offset -= পরবর্তী_offset;
		text += পরবর্তী_offset;
		দৈর্ঘ্য -= পরবর্তী_offset;
	}

	ফেরত x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
স্থির unsigned long textplain_সারি_গুনতি(কাঠামো content *c)
{
	textplain_content *text = (textplain_content *) c;

	প্রতিজ্ঞা(c != ফাঁকা);

	ফেরত text->physical_সারি_গুনতি;
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
স্থির অক্ষর *
textplain_get_সারি(কাঠামো content *c,
		   unsigned lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	কাঠামো textplain_সারি *সারি;

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (lineno >= text->physical_সারি_গুনতি)
		ফেরত ফাঁকা;
	সারি = &text->physical_সারি[lineno];

	*poffset = সারি->start;
	*plen = সারি->দৈর্ঘ্য;
	ফেরত text->utf8_তথ্য + সারি->start;
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
স্থির পূর্ণসংখ্যা textplain_find_সারি(কাঠামো content *c, unsigned offset)
{
	textplain_content *text = (textplain_content *) c;
	কাঠামো textplain_সারি *সারি;
	পূর্ণসংখ্যা nlines;
	পূর্ণসংখ্যা lineno = 0;

	প্রতিজ্ঞা(c != ফাঁকা);

	সারি = text->physical_সারি;
	nlines = text->physical_সারি_গুনতি;

	যদি (offset > text->utf8_তথ্য_আকার) {
		ফেরত -1;
	}

/* \todo - implement binary search here */
	যখন (lineno < nlines && সারি[lineno].start < offset) {
		lineno++;
	}
	যদি (সারি[lineno].start > offset) {
		lineno--;
	}

	ফেরত lineno;
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
textplain_textsearch_find(কাঠামো content *c,
			  কাঠামো textsearch_context *context,
			  ধ্রুবক অক্ষর *pattern,
			  পূর্ণসংখ্যা p_len,
			  bool case_sens)
{
	পূর্ণসংখ্যা nlines = textplain_সারি_গুনতি(c);
	পূর্ণসংখ্যা সারি;
	nserror res = NSERROR_OK;

	জন্য(সারি = 0; সারি < nlines; সারি++) {
		size_t offset, দৈর্ঘ্য;
		ধ্রুবক অক্ষর *text;

		text = textplain_get_সারি(c, সারি, &offset, &দৈর্ঘ্য);
		যদি (text) {
			যখন (দৈর্ঘ্য > 0) {
				unsigned match_দৈর্ঘ্য;
				size_t start_idx;
				ধ্রুবক অক্ষর *new_text;
				ধ্রুবক অক্ষর *pos;

				pos = content_textsearch_find_pattern(
						text,
						দৈর্ঘ্য,
						pattern,
						p_len,
						case_sens,
						&match_দৈর্ঘ্য);
				যদি (!pos)
					বিরতি;

				/* found string in line => add to list */
				start_idx = offset + (pos - text);
				res = content_textsearch_add_match(context,
						start_idx,
						start_idx + match_দৈর্ঘ্য,
						ফাঁকা,
						ফাঁকা);
				যদি (res != NSERROR_OK) {
					ফেরত res;
				}

				new_text = pos + match_দৈর্ঘ্য;
				offset += (new_text - text);
				দৈর্ঘ্য -= (new_text - text);
				text = new_text;
			}
		}
	}

	ফেরত res;
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
textplain_coords_from_range(কাঠামো content *c,
			    unsigned start,
			    unsigned end,
			    কাঠামো rect *r)
{
	textplain_content *text = (textplain_content *) c;
	দশমিক সারি_height = textplain_সারি_height();
	অক্ষর *utf8_তথ্য;
	কাঠামো textplain_সারি *সারি;
	unsigned lineno = 0;
	unsigned nlines;

	প্রতিজ্ঞা(c != ফাঁকা);
	প্রতিজ্ঞা(start <= end);
	প্রতিজ্ঞা(end <= text->utf8_তথ্য_আকার);

	utf8_তথ্য = text->utf8_তথ্য;
	nlines = text->physical_সারি_গুনতি;
	সারি = text->physical_সারি;

	/* find start */
	lineno = textplain_find_সারি(c, start);

	r->y0 = (পূর্ণসংখ্যা)(MARGIN + lineno * সারি_height);

	যদি (lineno + 1 <= nlines || সারি[lineno + 1].start >= end) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_সারি(c, end);

		r->x0 = 0;
		r->x1 = text->formatted_width;
	} অন্যথা {
		/* single line */
		ধ্রুবক অক্ষর *text = utf8_তথ্য + সারি[lineno].start;

		r->x0 = textplain_coord_from_offset(text,
						    start - সারি[lineno].start,
						    সারি[lineno].দৈর্ঘ্য);

		r->x1 = textplain_coord_from_offset(text,
						    end - সারি[lineno].start,
						    সারি[lineno].দৈর্ঘ্য);
	}

	r->y1 = (পূর্ণসংখ্যা)(MARGIN + (lineno + 1) * সারি_height);
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
স্থির অক্ষর *
textplain_get_raw_তথ্য(কাঠামো content *c,
		       unsigned start,
		       unsigned end,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_আকার;

	প্রতিজ্ঞা(c != ফাঁকা);

	utf8_আকার = text->utf8_তথ্য_আকার;

	/* any text at all? */
	যদি (!utf8_আকার) ফেরত ফাঁকা;

	/* clamp to valid offset range */
	যদি (start >= utf8_আকার) start = utf8_আকার;
	যদি (end >= utf8_আকার) end = utf8_আকার;

	*plen = end - start;

	ফেরত text->utf8_তথ্য + start;
}


/**
 * get bounds of a free text search match
 */
স্থির nserror
textplain_textsearch_bounds(কাঠামো content *c,
			    unsigned start_idx,
			    unsigned end_idx,
			    কাঠামো box *start_box,
			    কাঠামো box *end_box,
			    কাঠামো rect *bounds)
{
	textplain_coords_from_range(c, start_idx, end_idx, bounds);

	ফেরত NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
স্থির nserror
textplain_textselection_redraw(কাঠামো content *c,
			       unsigned start_idx,
			       unsigned end_idx)
{
	কাঠামো rect r;

	যদি (end_idx <= start_idx) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, end_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ফেরত NSERROR_OK;
}

স্থির nserror
textplain_textselection_copy(কাঠামো content *c,
			     unsigned start_idx,
			     unsigned end_idx,
			     কাঠামো selection_string *selstr)
{
	ধ্রুবক অক্ষর *text;
	size_t দৈর্ঘ্য;
	bool res = false;

	text = textplain_get_raw_তথ্য(c, start_idx, end_idx, &দৈর্ঘ্য);
	যদি (text != ফাঁকা) {
		res = selection_string_append(text, দৈর্ঘ্য, false, ফাঁকা, selstr);
	}
	যদি (res == false) {
		ফেরত NSERROR_NOMEM;
	}
	ফেরত NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
স্থির nserror
textplain_textselection_get_end(কাঠামো content *c, unsigned *end_idx)
{
	textplain_content *text = (textplain_content *)c;

	*end_idx = text->utf8_তথ্য_আকার;
	ফেরত NSERROR_OK;
}


/**
 * plain text content handler table
 */
স্থির ধ্রুবক content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_তথ্য = textplain_process_তথ্য,
	.তথ্য_complete = textplain_convert,
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
		ফেরত NSERROR_NOMEM;
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

	ফেরত error;
}




