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

संरचना textplain_पङ्क्तिः {
	size_t	start;
	size_t	दीर्घता;
};

/**
 * plain text content
 */
प्रकारनिर्देश संरचना textplain_content {
	संरचना content आधारः;

	lwc_string *encoding;
	शून्य *inputstream;
	वर्ण *utf8_दत्त;
	size_t utf8_दत्त_आकार;
	size_t utf8_दत्त_allocated;
	अचिह्नित दीर्घ physical_पङ्क्तिः_गणनम्;
	संरचना textplain_पङ्क्तिः *physical_पङ्क्तिः;
	पूर्णाङ्क formatted_विस्तारः;
	संरचना browser_गवाक्षः *bw;

	संरचना selection *sel; /** Selection state */

} textplain_content;


#परिभाषय CHUNK 32768 /* Must be a power of 2 */
#परिभाषय MARGIN 4

#परिभाषय TAB_विस्तारः 8  /* must be power of 2 currently */
#परिभाषय TEXT_आकार 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

स्थिर plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.आकार = TEXT_आकार,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

स्थिर पूर्णाङ्क textplain_tab_विस्तारः = 256;  /* try for a sensible default */

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
textplain_charset_hack(ध्रुव uint8_t *दत्त,
		       size_t len,
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
textplain_create_आन्तरिकम्(textplain_content *c, lwc_string *encoding)
{
	वर्ण *utf8_दत्त;
	parserutils_inputstream *प्रवाहः;
	parserutils_error error;

	textplain_style.आकार = (nsoption_int(font_आकार) * PLOT_STYLE_SCALE) / 10;

	utf8_दत्त = स्मृतिं_देहि(CHUNK);
	यदि (utf8_दत्त == रिक्त)
		गच्छ no_स्मृतिः;

	error = parserutils_inputstream_create(lwc_string_दत्त(encoding), 0,
					       textplain_charset_hack, &प्रवाहः);
	यदि (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &प्रवाहः);
	}
	यदि (error != PARSERUTILS_OK) {
		मुक्त(utf8_दत्त);
		गच्छ no_स्मृतिः;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = प्रवाहः;
	c->utf8_दत्त = utf8_दत्त;
	c->utf8_दत्त_आकार = 0;
	c->utf8_दत्त_allocated = CHUNK;
	c->physical_पङ्क्तिः = 0;
	c->physical_पङ्क्तिः_गणनम् = 0;
	c->formatted_विस्तारः = 0;
	c->bw = रिक्त;
	c->sel = selection_create((संरचना content *)c);

	प्रत्यागम NSERROR_OK;

no_स्मृतिः:
	content_प्रसारणम्_error(&c->आधारः, NSERROR_NOMEM, रिक्त);

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

	text = शून्यस्मृतिं_देहि(1, परिमाण(textplain_content));
	यदि (text == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	error = content__init(&text->आधारः, handler, imime_type, params,
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

	error = textplain_create_आन्तरिकम्(text, encoding);
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
textplain_copy_utf8_दत्त(textplain_content *c, ध्रुव uint8_t *buf, size_t len)
{
	यदि (c->utf8_दत्त_आकार + len >= c->utf8_दत्त_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		वर्ण *utf8_दत्त;

		allocated = (c->utf8_दत्त_आकार + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_दत्त = realloc(c->utf8_दत्त, allocated);
		यदि (utf8_दत्त == रिक्त)
			प्रत्यागम false;

		c->utf8_दत्त = utf8_दत्त;
		c->utf8_दत्त_allocated = allocated;
	}

	स्मृतिं_प्रतिलिख(c->utf8_दत्त + c->utf8_दत्त_आकार, buf, len);
	c->utf8_दत्त_आकार += len;

	प्रत्यागम true;
}


/**
 * drain input
 */
स्थिर bool
textplain_drain_निवेशः(textplain_content *c,
		      parserutils_inputstream *प्रवाहः,
		      parserutils_error terminator)
{
	स्थिर ध्रुव uint8_t *u_fffd = (ध्रुव uint8_t *) "\xef\xbf\xbd";
	ध्रुव uint8_t *ch;
	size_t chlen, विस्थापनम् = 0;

	यावत् (parserutils_inputstream_peek(प्रवाहः, विस्थापनम्, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		यदि (chlen == 1 && *ch == 0) {
			यदि (विस्थापनम् > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(प्रवाहः, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				यदि (textplain_copy_utf8_दत्त(c, ch,
							     विस्थापनम्) == false)
					प्रत्यागम false;
			}

			/* Emit U+FFFD */
			यदि (textplain_copy_utf8_दत्त(c, u_fffd, 3) == false)
				प्रत्यागम false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(प्रवाहः, विस्थापनम् + 1);
			/* Reset the read offset */
			विस्थापनम् = 0;
		} अन्यथा {
			/* Accumulate input */
			विस्थापनम् += chlen;

			यदि (विस्थापनम् > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(प्रवाहः, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				यदि (textplain_copy_utf8_दत्त(c, ch,
							     विस्थापनम्) == false)
					प्रत्यागम false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(प्रवाहः, विस्थापनम्);
				/* Reset the read offset */
				विस्थापनम् = 0;
			}
		}
	}

	यदि (विस्थापनम् > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(प्रवाहः, 0, &ch, &chlen);
		/* Emit any data remaining */
		यदि (textplain_copy_utf8_दत्त(c, ch, विस्थापनम्) == false)
			प्रत्यागम false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(प्रवाहः, विस्थापनम्);
	}

	प्रत्यागम true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
स्थिर bool
textplain_process_दत्त(संरचना content *c, ध्रुव वर्ण *दत्त, अचिह्नित पूर्णाङ्क आकार)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *प्रवाहः = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(प्रवाहः,
					       (ध्रुव uint8_t *) दत्त, आकार);
	यदि (error != PARSERUTILS_OK) {
		गच्छ no_स्मृतिः;
	}

	यदि (textplain_drain_निवेशः(text, प्रवाहः, PARSERUTILS_NEEDDATA) == false)
		गच्छ no_स्मृतिः;

	प्रत्यागम true;

no_स्मृतिः:
	content_प्रसारणम्_error(c, NSERROR_NOMEM, रिक्त);
	प्रत्यागम false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
स्थिर bool textplain_convert(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;
	parserutils_inputstream *प्रवाहः = text->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(प्रवाहः, रिक्त, 0);
	यदि (error != PARSERUTILS_OK) {
		प्रत्यागम false;
	}

	यदि (textplain_drain_निवेशः(text, प्रवाहः, PARSERUTILS_EOF) == false)
		प्रत्यागम false;

	parserutils_inputstream_destroy(प्रवाहः);
	text->inputstream = रिक्त;

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);
	content_समुच्चयः_स्थितिः(c, messages_get("Done"));

	प्रत्यागम true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
स्थिर भिन्न textplain_पङ्क्तिः_height(शून्य)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	प्रत्यागम FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.आकार / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
स्थिर शून्य textplain_reformat(संरचना content *c, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	textplain_content *text = (textplain_content *) c;
	वर्ण *utf8_दत्त = text->utf8_दत्त;
	size_t utf8_दत्त_आकार = text->utf8_दत्त_आकार;
	अचिह्नित दीर्घ पङ्क्तिः_गणनम् = 0;
	संरचना textplain_पङ्क्तिः *पङ्क्तिः = text->physical_पङ्क्तिः;
	संरचना textplain_पङ्क्तिः *line1;
	size_t i, space, col;
	size_t columns = 80;
	पूर्णाङ्क character_विस्तारः;
	size_t पङ्क्तिः_start;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, विस्तारः, height);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->विस्तारः(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_विस्तारः);
	यदि (res != NSERROR_OK) {
		प्रत्यागम;
	}

	columns = (विस्तारः - MARGIN - MARGIN) * 8 / character_विस्तारः;
	textplain_tab_विस्तारः = (TAB_विस्तारः * character_विस्तारः) / 8;

	text->formatted_विस्तारः = विस्तारः;

	text->physical_पङ्क्तिः_गणनम् = 0;

	यदि (!पङ्क्तिः) {
		text->physical_पङ्क्तिः = पङ्क्तिः =
			स्मृतिं_देहि(परिमाण(संरचना textplain_पङ्क्तिः) * (1024 + 3));
		यदि (!पङ्क्तिः)
			गच्छ no_स्मृतिः;
	}

	पङ्क्तिः[पङ्क्तिः_गणनम्++].start = पङ्क्तिः_start = 0;
	space = 0;
	i = 0;
	col = 0;
	यावत् (i < utf8_दत्त_आकार) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool पदांशः;
		size_t अग्र_col;
		parserutils_error perror;

		perror = parserutils_charset_utf8_गन्तव्यम्_ucs4((ध्रुव uint8_t *)utf8_दत्त + i, utf8_दत्त_आकार - i, &chr, &csize);
		यदि (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		पदांशः = (chr == '\n' || chr == '\r');

		अग्र_col = col + 1;

		यदि (chr == '\t') {
			अग्र_col = (अग्र_col + TAB_विस्तारः - 1) & ~(TAB_विस्तारः - 1);
		}

		यदि (पदांशः || अग्र_col >= columns) {
			यदि (पङ्क्तिः_गणनम् % 1024 == 0) {
				line1 = realloc(पङ्क्तिः,
						परिमाण(संरचना textplain_पङ्क्तिः) *
						(पङ्क्तिः_गणनम् + 1024 + 3));
				यदि (!line1)
					गच्छ no_स्मृतिः;
				text->physical_पङ्क्तिः = पङ्क्तिः = line1;
			}

			यदि (पदांशः) {
				पङ्क्तिः[पङ्क्तिः_गणनम्-1].दीर्घता = i - पङ्क्तिः_start;

				/* skip second char of CR/LF or LF/CR pair */
				यदि (i + 1 < utf8_दत्त_आकार &&
				    utf8_दत्त[i+1] != utf8_दत्त[i] &&
				    (utf8_दत्त[i+1] == '\n' ||
				     utf8_दत्त[i+1] == '\r')) {
					i++;
				}
			} अन्यथा {
				यदि (space) {
					/* break at last space in line */
					i = space;
					पङ्क्तिः[पङ्क्तिः_गणनम्-1].दीर्घता = (i + 1) - पङ्क्तिः_start;
				} अन्यथा
					पङ्क्तिः[पङ्क्तिः_गणनम्-1].दीर्घता = i - पङ्क्तिः_start;
			}

			पङ्क्तिः[पङ्क्तिः_गणनम्++].start = पङ्क्तिः_start = i + 1;
			col = 0;
			space = 0;
		} अन्यथा {
			col++;
			यदि (chr == ' ')
				space = i;
		}
		i += csize;
	}
	पङ्क्तिः[पङ्क्तिः_गणनम्-1].दीर्घता = i - पङ्क्तिः[पङ्क्तिः_गणनम्-1].start;
	पङ्क्तिः[पङ्क्तिः_गणनम्].start = utf8_दत्त_आकार;

	text->physical_पङ्क्तिः_गणनम् = पङ्क्तिः_गणनम्;
	c->विस्तारः = विस्तारः;
	c->height = पङ्क्तिः_गणनम् * textplain_पङ्क्तिः_height() + MARGIN + MARGIN;

	प्रत्यागम;

no_स्मृतिः:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", पङ्क्तिः_गणनम्);
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

	यदि (text->physical_पङ्क्तिः != रिक्त) {
		मुक्त(text->physical_पङ्क्तिः);
	}

	यदि (text->utf8_दत्त != रिक्त) {
		मुक्त(text->utf8_दत्त);
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
	ध्रुव uint8_t *दत्त;
	size_t आकार;

	text = शून्यस्मृतिं_देहि(1, परिमाण(textplain_content));
	यदि (text == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &text->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&text->आधारः);
		प्रत्यागम error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_आन्तरिकम्(text, old_text->encoding);
	यदि (error != NSERROR_OK) {
		content_destroy(&text->आधारः);
		प्रत्यागम error;
	}

	दत्त = content_get_source_दत्त_2(&text->आधारः, &आकार);
	यदि (आकार > 0) {
		यदि (textplain_process_दत्त(&text->आधारः,
					   (ध्रुव वर्ण *)दत्त,
					   आकार) == false) {
			content_destroy(&text->आधारः);
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	यदि (old->स्थितिः == CONTENT_स्थितिः_READY ||
	    old->स्थितिः == CONTENT_स्थितिः_समाप्तम्) {
		यदि (textplain_convert(&text->आधारः) == false) {
			content_destroy(&text->आधारः);
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
स्थिर size_t
textplain_विस्थापनम्_from_coords(संरचना content *c, पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क dir)
{
	textplain_content *textc = (textplain_content *) c;
	भिन्न पङ्क्तिः_height = textplain_पङ्क्तिः_height();
	संरचना textplain_पङ्क्तिः *पङ्क्तिः;
	ध्रुव वर्ण *text;
	अचिह्नित nlines;
	size_t दीर्घता;
	पूर्णाङ्क idx;

	प्रतिज्ञा(c != रिक्त);

	y = (पूर्णाङ्क)((भिन्न)(y - MARGIN) / पङ्क्तिः_height);
	x -= MARGIN;

	nlines = textc->physical_पङ्क्तिः_गणनम्;
	यदि (!nlines)
		प्रत्यागम 0;

	यदि (y <= 0) y = 0;
	अन्यथा यदि ((अचिह्नित)y >= nlines)
		y = nlines - 1;

	पङ्क्तिः = &textc->physical_पङ्क्तिः[y];
	text = textc->utf8_दत्त + पङ्क्तिः->start;
	दीर्घता = पङ्क्तिः->दीर्घता;
	idx = 0;

	यावत् (x > 0) {
		size_t अग्र_विस्थापनम् = 0;
		पूर्णाङ्क विस्तारः = अधिकतम_पूर्णाङ्कः;

		यावत् (अग्र_विस्थापनम् < दीर्घता && text[अग्र_विस्थापनम्] != '\t') {
			अग्र_विस्थापनम् = utf8_अग्र(text, दीर्घता, अग्र_विस्थापनम्);
		}

		यदि (अग्र_विस्थापनम् < दीर्घता) {
			guit->layout->विस्तारः(&textplain_style,
					    text,
					    अग्र_विस्थापनम्,
					    &विस्तारः);
		}

		यदि (x <= विस्तारः) {
			पूर्णाङ्क pixel_विस्थापनम्;
			size_t char_विस्थापनम्;

			guit->layout->position(&textplain_style,
					       text, अग्र_विस्थापनम्, x,
					       &char_विस्थापनम्, &pixel_विस्थापनम्);

			idx += char_विस्थापनम्;
			भङ्ग;
		}

		x -= विस्तारः;
		दीर्घता -= अग्र_विस्थापनम्;
		text += अग्र_विस्थापनम्;
		idx += अग्र_विस्थापनम्;

		/* check if it's within the tab */
		विस्तारः = textplain_tab_विस्तारः - (विस्तारः % textplain_tab_विस्तारः);
		यदि (x <= विस्तारः) भङ्ग;

		x -= विस्तारः;
		दीर्घता--;
		text++;
		idx++;
	}

	प्रत्यागम पङ्क्तिः->start + idx;
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
		       संरचना browser_गवाक्षः *bw,
		       browser_mouse_state mouse,
		       पूर्णाङ्क x, पूर्णाङ्क y)
{
	textplain_content *text = (textplain_content *) c;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	ध्रुव वर्ण *स्थितिः = 0;
	size_t idx;
	पूर्णाङ्क dir = 0;

	browser_गवाक्षः_समुच्चयः_drag_type(bw, DRAGGING_NONE, रिक्त);

	idx = textplain_विस्थापनम्_from_coords(c, x, y, dir);
	यदि (selection_click(text->sel, text->bw, mouse, idx)) {

		यदि (selection_dragging(text->sel)) {
			browser_गवाक्षः_समुच्चयः_drag_type(bw,
						     DRAGGING_SELECTION, रिक्त);
			स्थितिः = messages_get("Selecting");
		}

	} अन्यथा {
		यदि (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_गवाक्षः_page_drag_start(bw, x, y);
			pointer = BROWSER_POINTER_MOVE;
		}
	}

	सन्देशः_दत्त.explicit_स्थितिः_text = स्थितिः;
	content_प्रसारणम्(c, CONTENT_सन्देशः_स्थितिः, &सन्देशः_दत्त);

	सन्देशः_दत्त.pointer = pointer;
	content_प्रसारणम्(c, CONTENT_सन्देशः_POINTER, &सन्देशः_दत्त);

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
		      संरचना browser_गवाक्षः *bw,
		      browser_mouse_state mouse,
		      पूर्णाङ्क x, पूर्णाङ्क y)
{
	textplain_content *text = (textplain_content *) c;

	यदि (browser_गवाक्षः_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		पूर्णाङ्क dir = -1;
		size_t idx;

		यदि (selection_dragging_start(text->sel))
			dir = 1;

		idx = textplain_विस्थापनम्_from_coords(c, x, y, dir);
		selection_track(text->sel, mouse, idx);

		browser_गवाक्षः_समुच्चयः_drag_type(bw, DRAGGING_NONE, रिक्त);
	}

	विकल्प (browser_गवाक्षः_get_drag_type(bw)) {

	पक्ष DRAGGING_SELECTION: {
		पूर्णाङ्क dir = -1;
		size_t idx;

		यदि (selection_dragging_start(text->sel)) dir = 1;

		idx = textplain_विस्थापनम्_from_coords(c, x, y, dir);
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
स्थिर bool textplain_keypress(संरचना content *c, uint32_t कुञ्चिका)
{
	textplain_content *text = (textplain_content *) c;
	संरचना selection *sel = text->sel;

	विकल्प (कुञ्चिका) {
	पक्ष NS_कुञ्चिका_COPY_SELECTION:
		selection_copy_गन्तव्यम्_clipboard(sel);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_CLEAR_SELECTION:
		selection_clear(sel, true);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_SELECT_ALL:
		selection_select_all(sel);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_ESCAPE:
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
	    size_t utf8_len,
	    size_t विस्थापनम्,
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
	plot_fstyle.आकार *= scale;

	/* is this box part of a selection? */
	यदि (ctx->interactive == true) {
		अचिह्नित len = utf8_len;
		अचिह्नित start_idx;
		अचिह्नित अन्तः_idx;

		/* first try the browser window's current selection */
		यदि (selection_highlighted(sel,
					  विस्थापनम्,
					  विस्थापनम् + len,
					  &start_idx,
					  &अन्तः_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		यदि (!highlighted &&
		    (text->आधारः.textsearch.context != रिक्त) &&
		    content_textsearch_ishighlighted(text->आधारः.textsearch.context,
						     विस्थापनम्,
						     विस्थापनम् + len,
						     &start_idx,
						     &अन्तः_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		यदि (highlighted) {
			संरचना rect r;
			अचिह्नित endtxt_idx = अन्तः_idx;
			bool clip_परिवर्तितम् = false;
			bool text_visible = true;
			पूर्णाङ्क startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			यदि (अन्तः_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				प्रतिज्ञा(अन्तः_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->विस्तारः(&textplain_style,
						  utf8_text,
						  start_idx,
						  &startx);
			यदि (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->विस्तारः(&textplain_style,
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

			pstyle_fill_hback.fill_रङ्गः = textplain_style.foreground;

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

					clip_परिवर्तितम् = true;
				} अन्यथा {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_रङ्गः;
			fstyle_hback.foreground = रङ्गः_गन्तव्यम्_bw_furthest(
				pstyle_fill_hback.fill_रङ्गः);

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

					clip_परिवर्तितम् = true;

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

			यदि (clip_परिवर्तितम् &&
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
		 संरचना content_redraw_दत्त *दत्त,
		 ध्रुव संरचना rect *clip,
		 ध्रुव संरचना redraw_context *ctx)
{
	textplain_content *text = (textplain_content *) c;
	संरचना browser_गवाक्षः *bw = text->bw;
	वर्ण *utf8_दत्त = text->utf8_दत्त;
	दीर्घ lineno;
	पूर्णाङ्क x = दत्त->x;
	पूर्णाङ्क y = दत्त->y;
	अचिह्नित दीर्घ पङ्क्तिः_गणनम् = text->physical_पङ्क्तिः_गणनम्;
	भिन्न पङ्क्तिः_height = textplain_पङ्क्तिः_height();
	भिन्न scaled_पङ्क्तिः_height = पङ्क्तिः_height * दत्त->scale;
	दीर्घ line0 = (clip->y0 - y * दत्त->scale) / scaled_पङ्क्तिः_height - 1;
	दीर्घ line1 = (clip->y1 - y * दत्त->scale) / scaled_पङ्क्तिः_height + 1;
	संरचना textplain_पङ्क्तिः *पङ्क्तिः = text->physical_पङ्क्तिः;
	size_t दीर्घता;
	plot_style_t *plot_style_highlight;
	nserror res;

	यदि (line0 < 0)
		line0 = 0;
	यदि (line1 < 0)
		line1 = 0;
	यदि (पङ्क्तिः_गणनम् < (अचिह्नित दीर्घ) line0)
		line0 = पङ्क्तिः_गणनम्;
	यदि (पङ्क्तिः_गणनम् < (अचिह्नित दीर्घ) line1)
		line1 = पङ्क्तिः_गणनम्;
	यदि (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	यदि (!पङ्क्तिः)
		प्रत्यागम true;

	/* choose a suitable background colour for any highlighted text */
	यदि ((दत्त->background_रङ्गः & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	अन्यथा
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = दत्त->background_रङ्गः;

	x = (x + MARGIN) * दत्त->scale;
	y = (y + MARGIN) * दत्त->scale;
	प्रति (lineno = line0; lineno != line1; lineno++) {
		ध्रुव वर्ण *text_d = utf8_दत्त + पङ्क्तिः[lineno].start;
		पूर्णाङ्क tab_विस्तारः = textplain_tab_विस्तारः * दत्त->scale;
		size_t विस्थापनम् = 0;
		पूर्णाङ्क tx = x;

		यदि (!tab_विस्तारः) tab_विस्तारः = 1;

		दीर्घता = पङ्क्तिः[lineno].दीर्घता;
		यदि (!दीर्घता)
			अनुवर्त;

		यावत् (विस्थापनम् < दीर्घता) {
			size_t अग्र_विस्थापनम् = विस्थापनम्;
			पूर्णाङ्क विस्तारः;
			पूर्णाङ्क ntx;
			nserror res;

			यावत् ((अग्र_विस्थापनम् < दीर्घता) &&
			       (text_d[अग्र_विस्थापनम्] != '\t')) {
				अग्र_विस्थापनम् = utf8_अग्र(text_d,
							दीर्घता,
							अग्र_विस्थापनम्);
			}

			यदि (!text_draw(text_d + विस्थापनम्,
				       अग्र_विस्थापनम् - विस्थापनम्,
				       पङ्क्तिः[lineno].start + विस्थापनम्,
				       tx,
				       y + (lineno * scaled_पङ्क्तिः_height),
				       clip,
				       पङ्क्तिः_height,
				       दत्त->scale,
				       text,
				       text->sel,
				       ctx)) {
				प्रत्यागम false;
			}

			यदि (अग्र_विस्थापनम् >= दीर्घता)
				भङ्ग;

			res = guit->layout->विस्तारः(&textplain_style,
						  &text_d[विस्थापनम्],
						  अग्र_विस्थापनम् - विस्थापनम्,
						  &विस्तारः);
			/* locate end of string and align to next tab position */
			यदि (res == NSERROR_OK) {
				tx += (पूर्णाङ्क)(विस्तारः * दत्त->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_विस्तारः) * tab_विस्तारः);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			यदि (bw) {
				अचिह्नित tab_ofst = पङ्क्तिः[lineno].start + अग्र_विस्थापनम्;
				संरचना selection *sel = text->sel;
				bool highlighted = false;

				अचिह्नित start_idx, अन्तः_idx;
				यदि (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &start_idx,
							  &अन्तः_idx)) {
					highlighted = true;
				}


				यदि (!highlighted &&
				    (c->textsearch.context != रिक्त)) {
					अचिह्नित start_idx, अन्तः_idx;
					यदि (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &start_idx,
						    &अन्तः_idx)) {
						highlighted = true;
					}
				}

				यदि (highlighted) {
					संरचना rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_पङ्क्तिः_height);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_पङ्क्तिः_height;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					यदि (res != NSERROR_OK) {
						प्रत्यागम false;
					}
				}
			}

			विस्थापनम् = अग्र_विस्थापनम् + 1;
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
	       संरचना browser_गवाक्षः *bw,
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
textplain_coord_from_विस्थापनम्(ध्रुव वर्ण *text, size_t विस्थापनम्, size_t दीर्घता)
{
	पूर्णाङ्क x = 0;

	यावत् (विस्थापनम् > 0) {
		size_t अग्र_विस्थापनम् = 0;
		पूर्णाङ्क tx;

		यावत् (अग्र_विस्थापनम् < विस्थापनम् && text[अग्र_विस्थापनम्] != '\t') {
			अग्र_विस्थापनम् = utf8_अग्र(text, दीर्घता, अग्र_विस्थापनम्);
		}

		guit->layout->विस्तारः(&textplain_style, text, अग्र_विस्थापनम्, &tx);

		x += tx;

		यदि (अग्र_विस्थापनम् >= विस्थापनम्)
			भङ्ग;

		/* align to next tab boundary */
		अग्र_विस्थापनम्++;
		x = (1 + (x / textplain_tab_विस्तारः)) * textplain_tab_विस्तारः;
		विस्थापनम् -= अग्र_विस्थापनम्;
		text += अग्र_विस्थापनम्;
		दीर्घता -= अग्र_विस्थापनम्;
	}

	प्रत्यागम x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
स्थिर अचिह्नित दीर्घ textplain_पङ्क्तिः_गणनम्(संरचना content *c)
{
	textplain_content *text = (textplain_content *) c;

	प्रतिज्ञा(c != रिक्त);

	प्रत्यागम text->physical_पङ्क्तिः_गणनम्;
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
textplain_get_पङ्क्तिः(संरचना content *c,
		   अचिह्नित lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_पङ्क्तिः *पङ्क्तिः;

	प्रतिज्ञा(c != रिक्त);

	यदि (lineno >= text->physical_पङ्क्तिः_गणनम्)
		प्रत्यागम रिक्त;
	पङ्क्तिः = &text->physical_पङ्क्तिः[lineno];

	*poffset = पङ्क्तिः->start;
	*plen = पङ्क्तिः->दीर्घता;
	प्रत्यागम text->utf8_दत्त + पङ्क्तिः->start;
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
स्थिर पूर्णाङ्क textplain_find_पङ्क्तिः(संरचना content *c, अचिह्नित विस्थापनम्)
{
	textplain_content *text = (textplain_content *) c;
	संरचना textplain_पङ्क्तिः *पङ्क्तिः;
	पूर्णाङ्क nlines;
	पूर्णाङ्क lineno = 0;

	प्रतिज्ञा(c != रिक्त);

	पङ्क्तिः = text->physical_पङ्क्तिः;
	nlines = text->physical_पङ्क्तिः_गणनम्;

	यदि (विस्थापनम् > text->utf8_दत्त_आकार) {
		प्रत्यागम -1;
	}

/* \todo - implement binary search here */
	यावत् (lineno < nlines && पङ्क्तिः[lineno].start < विस्थापनम्) {
		lineno++;
	}
	यदि (पङ्क्तिः[lineno].start > विस्थापनम्) {
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
			  ध्रुव वर्ण *आदर्शः,
			  पूर्णाङ्क p_len,
			  bool case_sens)
{
	पूर्णाङ्क nlines = textplain_पङ्क्तिः_गणनम्(c);
	पूर्णाङ्क पङ्क्तिः;
	nserror res = NSERROR_OK;

	प्रति(पङ्क्तिः = 0; पङ्क्तिः < nlines; पङ्क्तिः++) {
		size_t विस्थापनम्, दीर्घता;
		ध्रुव वर्ण *text;

		text = textplain_get_पङ्क्तिः(c, पङ्क्तिः, &विस्थापनम्, &दीर्घता);
		यदि (text) {
			यावत् (दीर्घता > 0) {
				अचिह्नित मेलनम्_दीर्घता;
				size_t start_idx;
				ध्रुव वर्ण *new_text;
				ध्रुव वर्ण *pos;

				pos = content_textsearch_find_आदर्शः(
						text,
						दीर्घता,
						आदर्शः,
						p_len,
						case_sens,
						&मेलनम्_दीर्घता);
				यदि (!pos)
					भङ्ग;

				/* found string in line => add to list */
				start_idx = विस्थापनम् + (pos - text);
				res = content_textsearch_सङ्कलय_मेलनम्(context,
						start_idx,
						start_idx + मेलनम्_दीर्घता,
						रिक्त,
						रिक्त);
				यदि (res != NSERROR_OK) {
					प्रत्यागम res;
				}

				new_text = pos + मेलनम्_दीर्घता;
				विस्थापनम् += (new_text - text);
				दीर्घता -= (new_text - text);
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
			    अचिह्नित start,
			    अचिह्नित अन्तः,
			    संरचना rect *r)
{
	textplain_content *text = (textplain_content *) c;
	भिन्न पङ्क्तिः_height = textplain_पङ्क्तिः_height();
	वर्ण *utf8_दत्त;
	संरचना textplain_पङ्क्तिः *पङ्क्तिः;
	अचिह्नित lineno = 0;
	अचिह्नित nlines;

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(start <= अन्तः);
	प्रतिज्ञा(अन्तः <= text->utf8_दत्त_आकार);

	utf8_दत्त = text->utf8_दत्त;
	nlines = text->physical_पङ्क्तिः_गणनम्;
	पङ्क्तिः = text->physical_पङ्क्तिः;

	/* find start */
	lineno = textplain_find_पङ्क्तिः(c, start);

	r->y0 = (पूर्णाङ्क)(MARGIN + lineno * पङ्क्तिः_height);

	यदि (lineno + 1 <= nlines || पङ्क्तिः[lineno + 1].start >= अन्तः) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_find_पङ्क्तिः(c, अन्तः);

		r->x0 = 0;
		r->x1 = text->formatted_विस्तारः;
	} अन्यथा {
		/* single line */
		ध्रुव वर्ण *text = utf8_दत्त + पङ्क्तिः[lineno].start;

		r->x0 = textplain_coord_from_विस्थापनम्(text,
						    start - पङ्क्तिः[lineno].start,
						    पङ्क्तिः[lineno].दीर्घता);

		r->x1 = textplain_coord_from_विस्थापनम्(text,
						    अन्तः - पङ्क्तिः[lineno].start,
						    पङ्क्तिः[lineno].दीर्घता);
	}

	r->y1 = (पूर्णाङ्क)(MARGIN + (lineno + 1) * पङ्क्तिः_height);
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
textplain_get_raw_दत्त(संरचना content *c,
		       अचिह्नित start,
		       अचिह्नित अन्तः,
		       size_t *plen)
{
	textplain_content *text = (textplain_content *) c;
	size_t utf8_आकार;

	प्रतिज्ञा(c != रिक्त);

	utf8_आकार = text->utf8_दत्त_आकार;

	/* any text at all? */
	यदि (!utf8_आकार) प्रत्यागम रिक्त;

	/* clamp to valid offset range */
	यदि (start >= utf8_आकार) start = utf8_आकार;
	यदि (अन्तः >= utf8_आकार) अन्तः = utf8_आकार;

	*plen = अन्तः - start;

	प्रत्यागम text->utf8_दत्त + start;
}


/**
 * get bounds of a free text search match
 */
स्थिर nserror
textplain_textsearch_bounds(संरचना content *c,
			    अचिह्नित start_idx,
			    अचिह्नित अन्तः_idx,
			    संरचना box *start_box,
			    संरचना box *अन्तः_box,
			    संरचना rect *bounds)
{
	textplain_coords_from_range(c, start_idx, अन्तः_idx, bounds);

	प्रत्यागम NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
स्थिर nserror
textplain_textselection_redraw(संरचना content *c,
			       अचिह्नित start_idx,
			       अचिह्नित अन्तः_idx)
{
	संरचना rect r;

	यदि (अन्तः_idx <= start_idx) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, start_idx, अन्तः_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
textplain_textselection_copy(संरचना content *c,
			     अचिह्नित start_idx,
			     अचिह्नित अन्तः_idx,
			     संरचना selection_string *selstr)
{
	ध्रुव वर्ण *text;
	size_t दीर्घता;
	bool res = false;

	text = textplain_get_raw_दत्त(c, start_idx, अन्तः_idx, &दीर्घता);
	यदि (text != रिक्त) {
		res = selection_string_append(text, दीर्घता, false, रिक्त, selstr);
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
textplain_textselection_get_अन्तः(संरचना content *c, अचिह्नित *अन्तः_idx)
{
	textplain_content *text = (textplain_content *)c;

	*अन्तः_idx = text->utf8_दत्त_आकार;
	प्रत्यागम NSERROR_OK;
}


/**
 * plain text content handler table
 */
स्थिर ध्रुव content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_दत्त = textplain_process_दत्त,
	.दत्त_complete = textplain_convert,
	.reformat = textplain_reformat,
	.destroy = textplain_destroy,
	.mouse_track = textplain_mouse_track,
	.mouse_action = textplain_mouse_action,
	.keypress = textplain_keypress,
	.redraw = textplain_redraw,
	.उद्घाटय = textplain_open,
	.close = textplain_close,
	.get_selection = textplain_get_selection,
	.clone = textplain_clone,
	.type = textplain_content_type,
	.textsearch_find = textplain_textsearch_find,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_अन्तः = textplain_textselection_get_अन्तः,
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

	error = content_factory_पञ्जिकाकोशः_handler("text/plain",
						 &textplain_content_handler);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_पञ्जिकाकोशः_handler("application/json",
						 &textplain_content_handler);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	प्रत्यागम error;
}




