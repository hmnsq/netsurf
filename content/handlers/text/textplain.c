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

ساخت textplain_سطر {
	size_t	آغاز;
	size_t	لمبائی;
};

/**
 * plain text content
 */
قسمتعریف ساخت textplain_content {
	ساخت content بنیاد;

	lwc_string *encoding;
	خالی *inputstream;
	حرف *utf8_ڈیٹا;
	size_t utf8_ڈیٹا_سائز;
	size_t utf8_ڈیٹا_allocated;
	غیرعلامتی طویل physical_سطر_گنتی;
	ساخت textplain_سطر *physical_سطر;
	صحیح formatted_چوڑائی;
	ساخت browser_کھڑکی *bw;

	ساخت selection *sel; /** Selection state */

} textplain_content;


#تعریف_کرو CHUNK 32768 /* Must be a power of 2 */
#تعریف_کرو MARGIN 4

#تعریف_کرو TAB_چوڑائی 8  /* must be power of 2 currently */
#تعریف_کرو متن_سائز 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساکن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.سائز = متن_سائز,
	.وزن = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساکن صحیح textplain_tab_چوڑائی = 256;  /* try for a sensible default */

ساکن lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ساکن خالی textplain_fini(خالی)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = تہی;
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
ساکن parserutils_error
textplain_charset_hack(ثابت uint8_t *ڈیٹا,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *ماخذ)
{
	واپس PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ساکن nserror
textplain_create_اندرونی(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_ڈیٹا;
	parserutils_inputstream *دھارا;
	parserutils_error error;

	textplain_style.سائز = (nsoption_int(font_سائز) * PLOT_STYLE_SCALE) / 10;

	utf8_ڈیٹا = حافظہ_دو(CHUNK);
	اگر (utf8_ڈیٹا == تہی)
		جاؤ no_حافظہ;

	error = parserutils_inputstream_create(lwc_string_ڈیٹا(encoding), 0,
					       textplain_charset_hack, &دھارا);
	اگر (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &دھارا);
	}
	اگر (error != PARSERUTILS_OK) {
		آزاد(utf8_ڈیٹا);
		جاؤ no_حافظہ;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = دھارا;
	c->utf8_ڈیٹا = utf8_ڈیٹا;
	c->utf8_ڈیٹا_سائز = 0;
	c->utf8_ڈیٹا_allocated = CHUNK;
	c->physical_سطر = 0;
	c->physical_سطر_گنتی = 0;
	c->formatted_چوڑائی = 0;
	c->bw = تہی;
	c->sel = selection_create((ساخت content *)c);

	واپس NSERROR_OK;

no_حافظہ:
	content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);

	واپس NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ساکن nserror
textplain_create(ثابت content_handler *handler,
		 lwc_string *imime_type,
		 ثابت ساخت http_parameter *params,
		 llcache_handle *llcache,
		 ثابت حرف *fallback_charset,
		 bool quirks,
		 ساخت content **c)
{
	textplain_content *متن;
	nserror error;
	lwc_string *encoding;

	متن = صفر_حافظہ_دو(1, حجم(textplain_content));
	اگر (متن == تہی) {
		واپس NSERROR_NOMEM;
	}

	error = content__init(&متن->بنیاد, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(متن);
		واپس error;
	}

	error = http_parameter_list_ڈھونڈو_شے(params, corestring_lwc_charset,
					      &encoding);
	اگر (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_اندرونی(متن, encoding);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		آزاد(متن);
		واپس error;
	}

	lwc_string_unref(encoding);

	*c = (ساخت content *) متن;

	واپس NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ساکن bool
textplain_copy_utf8_ڈیٹا(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	اگر (c->utf8_ڈیٹا_سائز + len >= c->utf8_ڈیٹا_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_ڈیٹا;

		allocated = (c->utf8_ڈیٹا_سائز + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_ڈیٹا = realloc(c->utf8_ڈیٹا, allocated);
		اگر (utf8_ڈیٹا == تہی)
			واپس false;

		c->utf8_ڈیٹا = utf8_ڈیٹا;
		c->utf8_ڈیٹا_allocated = allocated;
	}

	حافظہ_نقل(c->utf8_ڈیٹا + c->utf8_ڈیٹا_سائز, buf, len);
	c->utf8_ڈیٹا_سائز += len;

	واپس true;
}


/**
 * drain input
 */
ساکن bool
textplain_drain_داخلہ(textplain_content *c,
		      parserutils_inputstream *دھارا,
		      parserutils_error terminator)
{
	ساکن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, انحراف = 0;

	جب (parserutils_inputstream_peek(دھارا, انحراف, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		اگر (chlen == 1 && *ch == 0) {
			اگر (انحراف > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(دھارا, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				اگر (textplain_copy_utf8_ڈیٹا(c, ch,
							     انحراف) == false)
					واپس false;
			}

			/* Emit U+FFFD */
			اگر (textplain_copy_utf8_ڈیٹا(c, u_fffd, 3) == false)
				واپس false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(دھارا, انحراف + 1);
			/* Reset the read offset */
			انحراف = 0;
		} ورنہ {
			/* Accumulate input */
			انحراف += chlen;

			اگر (انحراف > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(دھارا, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				اگر (textplain_copy_utf8_ڈیٹا(c, ch,
							     انحراف) == false)
					واپس false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(دھارا, انحراف);
				/* Reset the read offset */
				انحراف = 0;
			}
		}
	}

	اگر (انحراف > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(دھارا, 0, &ch, &chlen);
		/* Emit any data remaining */
		اگر (textplain_copy_utf8_ڈیٹا(c, ch, انحراف) == false)
			واپس false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(دھارا, انحراف);
	}

	واپس true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساکن bool
textplain_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا, غیرعلامتی صحیح سائز)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *دھارا = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(دھارا,
					       (ثابت uint8_t *) ڈیٹا, سائز);
	اگر (error != PARSERUTILS_OK) {
		جاؤ no_حافظہ;
	}

	اگر (textplain_drain_داخلہ(متن, دھارا, PARSERUTILS_NEEDDATA) == false)
		جاؤ no_حافظہ;

	واپس true;

no_حافظہ:
	content_نشر_error(c, NSERROR_NOMEM, تہی);
	واپس false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساکن bool textplain_convert(ساخت content *c)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *دھارا = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(دھارا, تہی, 0);
	اگر (error != PARSERUTILS_OK) {
		واپس false;
	}

	اگر (textplain_drain_داخلہ(متن, دھارا, PARSERUTILS_EOF) == false)
		واپس false;

	parserutils_inputstream_destroy(دھارا);
	متن->inputstream = تہی;

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
	content_سیٹ_حالت(c, messages_get("Done"));

	واپس true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساکن اعشاری textplain_سطر_اونچائی(خالی)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	واپس FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.سائز / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساکن خالی textplain_reformat(ساخت content *c, صحیح چوڑائی, صحیح اونچائی)
{
	textplain_content *متن = (textplain_content *) c;
	حرف *utf8_ڈیٹا = متن->utf8_ڈیٹا;
	size_t utf8_ڈیٹا_سائز = متن->utf8_ڈیٹا_سائز;
	غیرعلامتی طویل سطر_گنتی = 0;
	ساخت textplain_سطر *سطر = متن->physical_سطر;
	ساخت textplain_سطر *line1;
	size_t i, space, ستون;
	size_t columns = 80;
	صحیح character_چوڑائی;
	size_t سطر_آغاز;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, چوڑائی, اونچائی);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->چوڑائی(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_چوڑائی);
	اگر (res != NSERROR_OK) {
		واپس;
	}

	columns = (چوڑائی - MARGIN - MARGIN) * 8 / character_چوڑائی;
	textplain_tab_چوڑائی = (TAB_چوڑائی * character_چوڑائی) / 8;

	متن->formatted_چوڑائی = چوڑائی;

	متن->physical_سطر_گنتی = 0;

	اگر (!سطر) {
		متن->physical_سطر = سطر =
			حافظہ_دو(حجم(ساخت textplain_سطر) * (1024 + 3));
		اگر (!سطر)
			جاؤ no_حافظہ;
	}

	سطر[سطر_گنتی++].آغاز = سطر_آغاز = 0;
	space = 0;
	i = 0;
	ستون = 0;
	جب (i < utf8_ڈیٹا_سائز) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool رکن;
		size_t اگلا_ستون;
		parserutils_error perror;

		perror = parserutils_charset_utf8_مقصد_ucs4((ثابت uint8_t *)utf8_ڈیٹا + i, utf8_ڈیٹا_سائز - i, &chr, &csize);
		اگر (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		رکن = (chr == '\n' || chr == '\r');

		اگلا_ستون = ستون + 1;

		اگر (chr == '\t') {
			اگلا_ستون = (اگلا_ستون + TAB_چوڑائی - 1) & ~(TAB_چوڑائی - 1);
		}

		اگر (رکن || اگلا_ستون >= columns) {
			اگر (سطر_گنتی % 1024 == 0) {
				line1 = realloc(سطر,
						حجم(ساخت textplain_سطر) *
						(سطر_گنتی + 1024 + 3));
				اگر (!line1)
					جاؤ no_حافظہ;
				متن->physical_سطر = سطر = line1;
			}

			اگر (رکن) {
				سطر[سطر_گنتی-1].لمبائی = i - سطر_آغاز;

				/* skip second char of CR/LF or LF/CR pair */
				اگر (i + 1 < utf8_ڈیٹا_سائز &&
				    utf8_ڈیٹا[i+1] != utf8_ڈیٹا[i] &&
				    (utf8_ڈیٹا[i+1] == '\n' ||
				     utf8_ڈیٹا[i+1] == '\r')) {
					i++;
				}
			} ورنہ {
				اگر (space) {
					/* break at last space in line */
					i = space;
					سطر[سطر_گنتی-1].لمبائی = (i + 1) - سطر_آغاز;
				} ورنہ
					سطر[سطر_گنتی-1].لمبائی = i - سطر_آغاز;
			}

			سطر[سطر_گنتی++].آغاز = سطر_آغاز = i + 1;
			ستون = 0;
			space = 0;
		} ورنہ {
			ستون++;
			اگر (chr == ' ')
				space = i;
		}
		i += csize;
	}
	سطر[سطر_گنتی-1].لمبائی = i - سطر[سطر_گنتی-1].آغاز;
	سطر[سطر_گنتی].آغاز = utf8_ڈیٹا_سائز;

	متن->physical_سطر_گنتی = سطر_گنتی;
	c->چوڑائی = چوڑائی;
	c->اونچائی = سطر_گنتی * textplain_سطر_اونچائی() + MARGIN + MARGIN;

	واپس;

no_حافظہ:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", سطر_گنتی);
	واپس;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ساکن خالی textplain_destroy(ساخت content *c)
{
	textplain_content *متن = (textplain_content *) c;

	lwc_string_unref(متن->encoding);

	اگر (متن->inputstream != تہی) {
		parserutils_inputstream_destroy(متن->inputstream);
	}

	اگر (متن->physical_سطر != تہی) {
		آزاد(متن->physical_سطر);
	}

	اگر (متن->utf8_ڈیٹا != تہی) {
		آزاد(متن->utf8_ڈیٹا);
	}

	اگر (متن->sel != تہی) {
		selection_destroy(متن->sel);
	}
}


ساکن nserror textplain_clone(ثابت ساخت content *old, ساخت content **newc)
{
	ثابت textplain_content *old_متن = (textplain_content *) old;
	textplain_content *متن;
	nserror error;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;

	متن = صفر_حافظہ_دو(1, حجم(textplain_content));
	اگر (متن == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &متن->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&متن->بنیاد);
		واپس error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_اندرونی(متن, old_متن->encoding);
	اگر (error != NSERROR_OK) {
		content_destroy(&متن->بنیاد);
		واپس error;
	}

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(&متن->بنیاد, &سائز);
	اگر (سائز > 0) {
		اگر (textplain_process_ڈیٹا(&متن->بنیاد,
					   (ثابت حرف *)ڈیٹا,
					   سائز) == false) {
			content_destroy(&متن->بنیاد);
			واپس NSERROR_NOMEM;
		}
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY ||
	    old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (textplain_convert(&متن->بنیاد) == false) {
			content_destroy(&متن->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	واپس NSERROR_OK;
}


ساکن content_type textplain_content_type(خالی)
{
	واپس CONTENT_TEXTPLAIN;
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
ساکن size_t
textplain_انحراف_from_coords(ساخت content *c, صحیح x, صحیح y, صحیح dir)
{
	textplain_content *textc = (textplain_content *) c;
	اعشاری سطر_اونچائی = textplain_سطر_اونچائی();
	ساخت textplain_سطر *سطر;
	ثابت حرف *متن;
	غیرعلامتی nlines;
	size_t لمبائی;
	صحیح idx;

	أكد(c != تہی);

	y = (صحیح)((اعشاری)(y - MARGIN) / سطر_اونچائی);
	x -= MARGIN;

	nlines = textc->physical_سطر_گنتی;
	اگر (!nlines)
		واپس 0;

	اگر (y <= 0) y = 0;
	ورنہ اگر ((غیرعلامتی)y >= nlines)
		y = nlines - 1;

	سطر = &textc->physical_سطر[y];
	متن = textc->utf8_ڈیٹا + سطر->آغاز;
	لمبائی = سطر->لمبائی;
	idx = 0;

	جب (x > 0) {
		size_t اگلا_انحراف = 0;
		صحیح چوڑائی = زیادہ_سے_زیادہ_عدد;

		جب (اگلا_انحراف < لمبائی && متن[اگلا_انحراف] != '\t') {
			اگلا_انحراف = utf8_اگلا(متن, لمبائی, اگلا_انحراف);
		}

		اگر (اگلا_انحراف < لمبائی) {
			guit->layout->چوڑائی(&textplain_style,
					    متن,
					    اگلا_انحراف,
					    &چوڑائی);
		}

		اگر (x <= چوڑائی) {
			صحیح pixel_انحراف;
			size_t char_انحراف;

			guit->layout->position(&textplain_style,
					       متن, اگلا_انحراف, x,
					       &char_انحراف, &pixel_انحراف);

			idx += char_انحراف;
			توڑو;
		}

		x -= چوڑائی;
		لمبائی -= اگلا_انحراف;
		متن += اگلا_انحراف;
		idx += اگلا_انحراف;

		/* check if it's within the tab */
		چوڑائی = textplain_tab_چوڑائی - (چوڑائی % textplain_tab_چوڑائی);
		اگر (x <= چوڑائی) توڑو;

		x -= چوڑائی;
		لمبائی--;
		متن++;
		idx++;
	}

	واپس سطر->آغاز + idx;
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
ساکن nserror
textplain_mouse_action(ساخت content *c,
		       ساخت browser_کھڑکی *bw,
		       browser_mouse_state mouse,
		       صحیح x, صحیح y)
{
	textplain_content *متن = (textplain_content *) c;
	browser_اشارہ_گر_shape اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	ثابت حرف *حالت_2 = 0;
	size_t idx;
	صحیح dir = 0;

	browser_کھڑکی_سیٹ_drag_type(bw, DRAGGING_NONE, تہی);

	idx = textplain_انحراف_from_coords(c, x, y, dir);
	اگر (selection_click(متن->sel, متن->bw, mouse, idx)) {

		اگر (selection_dragging(متن->sel)) {
			browser_کھڑکی_سیٹ_drag_type(bw,
						     DRAGGING_SELECTION, تہی);
			حالت_2 = messages_get("Selecting");
		}

	} ورنہ {
		اگر (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_کھڑکی_صفحہ_drag_آغاز(bw, x, y);
			اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	}

	پیغام_ڈیٹا.explicit_حالت_متن = حالت_2;
	content_نشر(c, CONTENT_پیغام_حالت, &پیغام_ڈیٹا);

	پیغام_ڈیٹا.اشارہ_گر = اشارہ_گر;
	content_نشر(c, CONTENT_پیغام_اشارہ_گر, &پیغام_ڈیٹا);

	واپس NSERROR_OK;
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
ساکن nserror
textplain_mouse_track(ساخت content *c,
		      ساخت browser_کھڑکی *bw,
		      browser_mouse_state mouse,
		      صحیح x, صحیح y)
{
	textplain_content *متن = (textplain_content *) c;

	اگر (browser_کھڑکی_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحیح dir = -1;
		size_t idx;

		اگر (selection_dragging_آغاز(متن->sel))
			dir = 1;

		idx = textplain_انحراف_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);

		browser_کھڑکی_سیٹ_drag_type(bw, DRAGGING_NONE, تہی);
	}

	انتخاب (browser_کھڑکی_get_drag_type(bw)) {

	حالت DRAGGING_SELECTION: {
		صحیح dir = -1;
		size_t idx;

		اگر (selection_dragging_آغاز(متن->sel)) dir = 1;

		idx = textplain_انحراف_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);
	}
		توڑو;

	طےشدہ:
		textplain_mouse_action(c, bw, mouse, x, y);
		توڑو;
	}

	واپس NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ساکن bool textplain_keypress(ساخت content *c, uint32_t کلید)
{
	textplain_content *متن = (textplain_content *) c;
	ساخت selection *sel = متن->sel;

	انتخاب (کلید) {
	حالت NS_کلید_COPY_SELECTION:
		selection_copy_مقصد_clipboard(sel);
		واپس true;

	حالت NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		واپس true;

	حالت NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		واپس true;

	حالت NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		واپس selection_clear(sel, true);
	}

	واپس false;
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
ساکن bool
متن_draw(ثابت حرف *utf8_متن,
	    size_t utf8_len,
	    size_t انحراف,
	    صحیح x,
	    صحیح y,
	    ثابت ساخت rect *clip,
	    صحیح اونچائی,
	    اعشاری scale,
	    textplain_content *متن,
	    ثابت ساخت selection *sel,
	    ثابت ساخت redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.سائز *= scale;

	/* is this box part of a selection? */
	اگر (ctx->interactive == true) {
		غیرعلامتی len = utf8_len;
		غیرعلامتی آغاز_idx;
		غیرعلامتی انتہا_idx;

		/* first try the browser window's current selection */
		اگر (selection_highlighted(sel,
					  انحراف,
					  انحراف + len,
					  &آغاز_idx,
					  &انتہا_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		اگر (!highlighted &&
		    (متن->بنیاد.textsearch.context != تہی) &&
		    content_textsearch_ishighlighted(متن->بنیاد.textsearch.context,
						     انحراف,
						     انحراف + len,
						     &آغاز_idx,
						     &انتہا_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		اگر (highlighted) {
			ساخت rect r;
			غیرعلامتی endtxt_idx = انتہا_idx;
			bool clip_بدلا = false;
			bool متن_visible = true;
			صحیح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			اگر (انتہا_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(انتہا_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->چوڑائی(&textplain_style,
						  utf8_متن,
						  آغاز_idx,
						  &startx);
			اگر (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->چوڑائی(&textplain_style,
						  utf8_متن,
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
			اگر (آغاز_idx > 0) {
				res = ctx->plot->متن(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحیح)(اونچائی * 0.75 * scale),
					      utf8_متن,
					      آغاز_idx);
				اگر (res != NSERROR_OK) {
					واپس false;
				}
			}

			pstyle_fill_hback.fill_رنگ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + اونچائی * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			اگر (res != NSERROR_OK) {
				واپس false;
			}

			اگر (آغاز_idx > 0) {
				صحیح px0 = max(x + startx, clip->x0);
				صحیح px1 = min(x + endx, clip->x1);

				اگر (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						واپس false;
					}

					clip_بدلا = true;
				} ورنہ {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_مقصد_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			اگر (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحیح)(اونچائی * 0.75 * scale),
					     utf8_متن,
					     endtxt_idx) != NSERROR_OK)) {
				واپس false;
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
						واپس false;
					}

					clip_بدلا = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحیح)(اونچائی * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					اگر (res != NSERROR_OK) {
						واپس false;
					}
				}
			}

			اگر (clip_بدلا &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				واپس false;
			}
		}
	}

	اگر (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحیح) (اونچائی * 0.75 * scale),
				      utf8_متن,
				      utf8_len);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}
	واپس true;
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
ساکن bool
textplain_redraw(ساخت content *c,
		 ساخت content_redraw_ڈیٹا *ڈیٹا,
		 ثابت ساخت rect *clip,
		 ثابت ساخت redraw_context *ctx)
{
	textplain_content *متن = (textplain_content *) c;
	ساخت browser_کھڑکی *bw = متن->bw;
	حرف *utf8_ڈیٹا = متن->utf8_ڈیٹا;
	طویل lineno;
	صحیح x = ڈیٹا->x;
	صحیح y = ڈیٹا->y;
	غیرعلامتی طویل سطر_گنتی = متن->physical_سطر_گنتی;
	اعشاری سطر_اونچائی = textplain_سطر_اونچائی();
	اعشاری scaled_سطر_اونچائی = سطر_اونچائی * ڈیٹا->scale;
	طویل line0 = (clip->y0 - y * ڈیٹا->scale) / scaled_سطر_اونچائی - 1;
	طویل line1 = (clip->y1 - y * ڈیٹا->scale) / scaled_سطر_اونچائی + 1;
	ساخت textplain_سطر *سطر = متن->physical_سطر;
	size_t لمبائی;
	plot_style_t *plot_style_highlight;
	nserror res;

	اگر (line0 < 0)
		line0 = 0;
	اگر (line1 < 0)
		line1 = 0;
	اگر (سطر_گنتی < (غیرعلامتی طویل) line0)
		line0 = سطر_گنتی;
	اگر (سطر_گنتی < (غیرعلامتی طویل) line1)
		line1 = سطر_گنتی;
	اگر (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	اگر (!سطر)
		واپس true;

	/* choose a suitable background colour for any highlighted text */
	اگر ((ڈیٹا->background_رنگ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ورنہ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = ڈیٹا->background_رنگ;

	x = (x + MARGIN) * ڈیٹا->scale;
	y = (y + MARGIN) * ڈیٹا->scale;
	برائے (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *متن_d = utf8_ڈیٹا + سطر[lineno].آغاز;
		صحیح tab_چوڑائی = textplain_tab_چوڑائی * ڈیٹا->scale;
		size_t انحراف = 0;
		صحیح tx = x;

		اگر (!tab_چوڑائی) tab_چوڑائی = 1;

		لمبائی = سطر[lineno].لمبائی;
		اگر (!لمبائی)
			جاری;

		جب (انحراف < لمبائی) {
			size_t اگلا_انحراف = انحراف;
			صحیح چوڑائی;
			صحیح ntx;
			nserror res;

			جب ((اگلا_انحراف < لمبائی) &&
			       (متن_d[اگلا_انحراف] != '\t')) {
				اگلا_انحراف = utf8_اگلا(متن_d,
							لمبائی,
							اگلا_انحراف);
			}

			اگر (!متن_draw(متن_d + انحراف,
				       اگلا_انحراف - انحراف,
				       سطر[lineno].آغاز + انحراف,
				       tx,
				       y + (lineno * scaled_سطر_اونچائی),
				       clip,
				       سطر_اونچائی,
				       ڈیٹا->scale,
				       متن,
				       متن->sel,
				       ctx)) {
				واپس false;
			}

			اگر (اگلا_انحراف >= لمبائی)
				توڑو;

			res = guit->layout->چوڑائی(&textplain_style,
						  &متن_d[انحراف],
						  اگلا_انحراف - انحراف,
						  &چوڑائی);
			/* locate end of string and align to next tab position */
			اگر (res == NSERROR_OK) {
				tx += (صحیح)(چوڑائی * ڈیٹا->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_چوڑائی) * tab_چوڑائی);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			اگر (bw) {
				غیرعلامتی tab_ofst = سطر[lineno].آغاز + اگلا_انحراف;
				ساخت selection *sel = متن->sel;
				bool highlighted = false;

				غیرعلامتی آغاز_idx, انتہا_idx;
				اگر (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &آغاز_idx,
							  &انتہا_idx)) {
					highlighted = true;
				}


				اگر (!highlighted &&
				    (c->textsearch.context != تہی)) {
					غیرعلامتی آغاز_idx, انتہا_idx;
					اگر (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &آغاز_idx,
						    &انتہا_idx)) {
						highlighted = true;
					}
				}

				اگر (highlighted) {
					ساخت rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_سطر_اونچائی);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_سطر_اونچائی;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					اگر (res != NSERROR_OK) {
						واپس false;
					}
				}
			}

			انحراف = اگلا_انحراف + 1;
			tx = ntx;
		}
	}

	واپس true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ساکن nserror
textplain_open(ساخت content *c,
	       ساخت browser_کھڑکی *bw,
	       ساخت content *صفحہ,
	       ساخت object_params *params)
{
	textplain_content *متن = (textplain_content *) c;

	متن->bw = bw;

	/* text selection */
	selection_init(متن->sel);

	واپس NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ساکن nserror textplain_close(ساخت content *c)
{
	textplain_content *متن = (textplain_content *) c;

	متن->bw = تہی;

	واپس NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ساکن حرف *textplain_get_selection(ساخت content *c)
{
	textplain_content *متن = (textplain_content *) c;

	واپس selection_get_copy(متن->sel);
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
ساکن صحیح
textplain_coord_from_انحراف(ثابت حرف *متن, size_t انحراف, size_t لمبائی)
{
	صحیح x = 0;

	جب (انحراف > 0) {
		size_t اگلا_انحراف = 0;
		صحیح tx;

		جب (اگلا_انحراف < انحراف && متن[اگلا_انحراف] != '\t') {
			اگلا_انحراف = utf8_اگلا(متن, لمبائی, اگلا_انحراف);
		}

		guit->layout->چوڑائی(&textplain_style, متن, اگلا_انحراف, &tx);

		x += tx;

		اگر (اگلا_انحراف >= انحراف)
			توڑو;

		/* align to next tab boundary */
		اگلا_انحراف++;
		x = (1 + (x / textplain_tab_چوڑائی)) * textplain_tab_چوڑائی;
		انحراف -= اگلا_انحراف;
		متن += اگلا_انحراف;
		لمبائی -= اگلا_انحراف;
	}

	واپس x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساکن غیرعلامتی طویل textplain_سطر_گنتی(ساخت content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أكد(c != تہی);

	واپس متن->physical_سطر_گنتی;
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
ساکن حرف *
textplain_get_سطر(ساخت content *c,
		   غیرعلامتی lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	ساخت textplain_سطر *سطر;

	أكد(c != تہی);

	اگر (lineno >= متن->physical_سطر_گنتی)
		واپس تہی;
	سطر = &متن->physical_سطر[lineno];

	*poffset = سطر->آغاز;
	*plen = سطر->لمبائی;
	واپس متن->utf8_ڈیٹا + سطر->آغاز;
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
ساکن صحیح textplain_ڈھونڈو_سطر(ساخت content *c, غیرعلامتی انحراف)
{
	textplain_content *متن = (textplain_content *) c;
	ساخت textplain_سطر *سطر;
	صحیح nlines;
	صحیح lineno = 0;

	أكد(c != تہی);

	سطر = متن->physical_سطر;
	nlines = متن->physical_سطر_گنتی;

	اگر (انحراف > متن->utf8_ڈیٹا_سائز) {
		واپس -1;
	}

/* \todo - implement binary search here */
	جب (lineno < nlines && سطر[lineno].آغاز < انحراف) {
		lineno++;
	}
	اگر (سطر[lineno].آغاز > انحراف) {
		lineno--;
	}

	واپس lineno;
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
ساکن nserror
textplain_textsearch_ڈھونڈو(ساخت content *c,
			  ساخت textsearch_context *context,
			  ثابت حرف *نمونہ,
			  صحیح p_len,
			  bool case_sens)
{
	صحیح nlines = textplain_سطر_گنتی(c);
	صحیح سطر;
	nserror res = NSERROR_OK;

	برائے(سطر = 0; سطر < nlines; سطر++) {
		size_t انحراف, لمبائی;
		ثابت حرف *متن;

		متن = textplain_get_سطر(c, سطر, &انحراف, &لمبائی);
		اگر (متن) {
			جب (لمبائی > 0) {
				غیرعلامتی مطابقت_لمبائی;
				size_t آغاز_idx;
				ثابت حرف *new_متن;
				ثابت حرف *pos;

				pos = content_textsearch_ڈھونڈو_نمونہ(
						متن,
						لمبائی,
						نمونہ,
						p_len,
						case_sens,
						&مطابقت_لمبائی);
				اگر (!pos)
					توڑو;

				/* found string in line => add to list */
				آغاز_idx = انحراف + (pos - متن);
				res = content_textsearch_جوڑو_مطابقت(context,
						آغاز_idx,
						آغاز_idx + مطابقت_لمبائی,
						تہی,
						تہی);
				اگر (res != NSERROR_OK) {
					واپس res;
				}

				new_متن = pos + مطابقت_لمبائی;
				انحراف += (new_متن - متن);
				لمبائی -= (new_متن - متن);
				متن = new_متن;
			}
		}
	}

	واپس res;
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
ساکن خالی
textplain_coords_from_range(ساخت content *c,
			    غیرعلامتی آغاز,
			    غیرعلامتی انتہا,
			    ساخت rect *r)
{
	textplain_content *متن = (textplain_content *) c;
	اعشاری سطر_اونچائی = textplain_سطر_اونچائی();
	حرف *utf8_ڈیٹا;
	ساخت textplain_سطر *سطر;
	غیرعلامتی lineno = 0;
	غیرعلامتی nlines;

	أكد(c != تہی);
	أكد(آغاز <= انتہا);
	أكد(انتہا <= متن->utf8_ڈیٹا_سائز);

	utf8_ڈیٹا = متن->utf8_ڈیٹا;
	nlines = متن->physical_سطر_گنتی;
	سطر = متن->physical_سطر;

	/* find start */
	lineno = textplain_ڈھونڈو_سطر(c, آغاز);

	r->y0 = (صحیح)(MARGIN + lineno * سطر_اونچائی);

	اگر (lineno + 1 <= nlines || سطر[lineno + 1].آغاز >= انتہا) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_ڈھونڈو_سطر(c, انتہا);

		r->x0 = 0;
		r->x1 = متن->formatted_چوڑائی;
	} ورنہ {
		/* single line */
		ثابت حرف *متن = utf8_ڈیٹا + سطر[lineno].آغاز;

		r->x0 = textplain_coord_from_انحراف(متن,
						    آغاز - سطر[lineno].آغاز,
						    سطر[lineno].لمبائی);

		r->x1 = textplain_coord_from_انحراف(متن,
						    انتہا - سطر[lineno].آغاز,
						    سطر[lineno].لمبائی);
	}

	r->y1 = (صحیح)(MARGIN + (lineno + 1) * سطر_اونچائی);
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
ساکن حرف *
textplain_get_raw_ڈیٹا(ساخت content *c,
		       غیرعلامتی آغاز,
		       غیرعلامتی انتہا,
		       size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	size_t utf8_سائز;

	أكد(c != تہی);

	utf8_سائز = متن->utf8_ڈیٹا_سائز;

	/* any text at all? */
	اگر (!utf8_سائز) واپس تہی;

	/* clamp to valid offset range */
	اگر (آغاز >= utf8_سائز) آغاز = utf8_سائز;
	اگر (انتہا >= utf8_سائز) انتہا = utf8_سائز;

	*plen = انتہا - آغاز;

	واپس متن->utf8_ڈیٹا + آغاز;
}


/**
 * get bounds of a free text search match
 */
ساکن nserror
textplain_textsearch_bounds(ساخت content *c,
			    غیرعلامتی آغاز_idx,
			    غیرعلامتی انتہا_idx,
			    ساخت box *آغاز_box,
			    ساخت box *انتہا_box,
			    ساخت rect *bounds)
{
	textplain_coords_from_range(c, آغاز_idx, انتہا_idx, bounds);

	واپس NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساکن nserror
textplain_textselection_redraw(ساخت content *c,
			       غیرعلامتی آغاز_idx,
			       غیرعلامتی انتہا_idx)
{
	ساخت rect r;

	اگر (انتہا_idx <= آغاز_idx) {
		واپس NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, آغاز_idx, انتہا_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	واپس NSERROR_OK;
}

ساکن nserror
textplain_textselection_copy(ساخت content *c,
			     غیرعلامتی آغاز_idx,
			     غیرعلامتی انتہا_idx,
			     ساخت selection_string *selstr)
{
	ثابت حرف *متن;
	size_t لمبائی;
	bool res = false;

	متن = textplain_get_raw_ڈیٹا(c, آغاز_idx, انتہا_idx, &لمبائی);
	اگر (متن != تہی) {
		res = selection_string_append(متن, لمبائی, false, تہی, selstr);
	}
	اگر (res == false) {
		واپس NSERROR_NOMEM;
	}
	واپس NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ساکن nserror
textplain_textselection_get_انتہا(ساخت content *c, غیرعلامتی *انتہا_idx)
{
	textplain_content *متن = (textplain_content *)c;

	*انتہا_idx = متن->utf8_ڈیٹا_سائز;
	واپس NSERROR_OK;
}


/**
 * plain text content handler table
 */
ساکن ثابت content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_ڈیٹا = textplain_process_ڈیٹا,
	.ڈیٹا_complete = textplain_convert,
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
	.textsearch_ڈھونڈو = textplain_textsearch_ڈھونڈو,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_انتہا = textplain_textselection_get_انتہا,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(خالی)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	اگر (lerror != lwc_error_ok) {
		واپس NSERROR_NOMEM;
	}

	error = content_factory_رجسٹر_handler("text/plain",
						 &textplain_content_handler);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_رجسٹر_handler("application/json",
						 &textplain_content_handler);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	واپس error;
}




