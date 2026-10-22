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

#ضمن <string.h>
#ضمن <parserutils/input/inputstream.h>

#ضمن "utils/errors.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/http.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/utf8.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/keypress.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/layout.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "content/hlcache.h"
#ضمن "content/textsearch.h"
#ضمن "content/handlers/css/utils.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/gui_internal.h"

#ضمن "text/textplain.h"

بنية textplain_سطر {
	size_t	شروع;
	size_t	لمائی;
};

/**
 * plain text content
 */
تعريفنوع بنية textplain_content {
	بنية content ادھار;

	lwc_string *encoding;
	فراغ *inputstream;
	حرف *utf8_انکڑے;
	size_t utf8_انکڑے_ناپ;
	size_t utf8_انکڑے_allocated;
	بنانشانی لما physical_سطر_گنتی;
	بنية textplain_سطر *physical_سطر;
	صحيح formatted_چوڑائی;
	بنية browser_کھڑکی *bw;

	بنية selection *sel; /** Selection state */

} textplain_content;


#تعریف_کرو CHUNK 32768 /* Must be a power of 2 */
#تعریف_کرو MARGIN 4

#تعریف_کرو TAB_چوڑائی 8  /* must be power of 2 currently */
#تعریف_کرو متن_ناپ 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساكن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.ناپ = متن_ناپ,
	.بھار = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساكن صحيح textplain_tab_چوڑائی = 256;  /* try for a sensible default */

ساكن lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ساكن فراغ textplain_fini(فراغ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = عدم;
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
ساكن parserutils_error
textplain_charset_hack(ثابت uint8_t *انکڑے,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *سروت)
{
	أرجع PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ساكن nserror
textplain_create_اندرونی(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_انکڑے;
	parserutils_inputstream *دھارا;
	parserutils_error error;

	textplain_style.ناپ = (nsoption_int(font_ناپ) * PLOT_STYLE_SCALE) / 10;

	utf8_انکڑے = حافظہ_دیو(CHUNK);
	إذا (utf8_انکڑے == عدم)
		اذهب no_حافظہ;

	error = parserutils_inputstream_create(lwc_string_انکڑے(encoding), 0,
					       textplain_charset_hack, &دھارا);
	إذا (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &دھارا);
	}
	إذا (error != PARSERUTILS_OK) {
		حرر(utf8_انکڑے);
		اذهب no_حافظہ;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = دھارا;
	c->utf8_انکڑے = utf8_انکڑے;
	c->utf8_انکڑے_ناپ = 0;
	c->utf8_انکڑے_allocated = CHUNK;
	c->physical_سطر = 0;
	c->physical_سطر_گنتی = 0;
	c->formatted_چوڑائی = 0;
	c->bw = عدم;
	c->sel = selection_create((بنية content *)c);

	أرجع NSERROR_OK;

no_حافظہ:
	content_نشر_error(&c->ادھار, NSERROR_NOMEM, عدم);

	أرجع NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ساكن nserror
textplain_create(ثابت content_handler *handler,
		 lwc_string *imime_type,
		 ثابت بنية http_parameter *params,
		 llcache_handle *llcache,
		 ثابت حرف *fallback_charset,
		 bool quirks,
		 بنية content **c)
{
	textplain_content *متن;
	nserror error;
	lwc_string *encoding;

	متن = صفر_حافظہ_دیو(1, حجم(textplain_content));
	إذا (متن == عدم) {
		أرجع NSERROR_NOMEM;
	}

	error = content__init(&متن->ادھار, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(متن);
		أرجع error;
	}

	error = http_parameter_list_لبھو_شے(params, corestring_lwc_charset,
					      &encoding);
	إذا (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_اندرونی(متن, encoding);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		حرر(متن);
		أرجع error;
	}

	lwc_string_unref(encoding);

	*c = (بنية content *) متن;

	أرجع NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ساكن bool
textplain_copy_utf8_انکڑے(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	إذا (c->utf8_انکڑے_ناپ + len >= c->utf8_انکڑے_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_انکڑے;

		allocated = (c->utf8_انکڑے_ناپ + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_انکڑے = realloc(c->utf8_انکڑے, allocated);
		إذا (utf8_انکڑے == عدم)
			أرجع false;

		c->utf8_انکڑے = utf8_انکڑے;
		c->utf8_انکڑے_allocated = allocated;
	}

	حافظہ_نقل(c->utf8_انکڑے + c->utf8_انکڑے_ناپ, buf, len);
	c->utf8_انکڑے_ناپ += len;

	أرجع true;
}


/**
 * drain input
 */
ساكن bool
textplain_drain_داخلہ(textplain_content *c,
		      parserutils_inputstream *دھارا,
		      parserutils_error terminator)
{
	ساكن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, وتھ = 0;

	طالما (parserutils_inputstream_peek(دھارا, وتھ, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		إذا (chlen == 1 && *ch == 0) {
			إذا (وتھ > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(دھارا, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				إذا (textplain_copy_utf8_انکڑے(c, ch,
							     وتھ) == false)
					أرجع false;
			}

			/* Emit U+FFFD */
			إذا (textplain_copy_utf8_انکڑے(c, u_fffd, 3) == false)
				أرجع false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(دھارا, وتھ + 1);
			/* Reset the read offset */
			وتھ = 0;
		} وإلا {
			/* Accumulate input */
			وتھ += chlen;

			إذا (وتھ > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(دھارا, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				إذا (textplain_copy_utf8_انکڑے(c, ch,
							     وتھ) == false)
					أرجع false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(دھارا, وتھ);
				/* Reset the read offset */
				وتھ = 0;
			}
		}
	}

	إذا (وتھ > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(دھارا, 0, &ch, &chlen);
		/* Emit any data remaining */
		إذا (textplain_copy_utf8_انکڑے(c, ch, وتھ) == false)
			أرجع false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(دھارا, وتھ);
	}

	أرجع true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساكن bool
textplain_process_انکڑے(بنية content *c, ثابت حرف *انکڑے, بنانشانی صحيح ناپ)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *دھارا = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(دھارا,
					       (ثابت uint8_t *) انکڑے, ناپ);
	إذا (error != PARSERUTILS_OK) {
		اذهب no_حافظہ;
	}

	إذا (textplain_drain_داخلہ(متن, دھارا, PARSERUTILS_NEEDDATA) == false)
		اذهب no_حافظہ;

	أرجع true;

no_حافظہ:
	content_نشر_error(c, NSERROR_NOMEM, عدم);
	أرجع false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساكن bool textplain_convert(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *دھارا = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(دھارا, عدم, 0);
	إذا (error != PARSERUTILS_OK) {
		أرجع false;
	}

	إذا (textplain_drain_داخلہ(متن, دھارا, PARSERUTILS_EOF) == false)
		أرجع false;

	parserutils_inputstream_destroy(دھارا);
	متن->inputstream = عدم;

	content_سیٹ_ready(c);
	content_سیٹ_پورا(c);
	content_سیٹ_حالت(c, messages_get("Done"));

	أرجع true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساكن عائم textplain_سطر_اچائی(فراغ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	أرجع FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.ناپ / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساكن فراغ textplain_reformat(بنية content *c, صحيح چوڑائی, صحيح اچائی)
{
	textplain_content *متن = (textplain_content *) c;
	حرف *utf8_انکڑے = متن->utf8_انکڑے;
	size_t utf8_انکڑے_ناپ = متن->utf8_انکڑے_ناپ;
	بنانشانی لما سطر_گنتی = 0;
	بنية textplain_سطر *سطر = متن->physical_سطر;
	بنية textplain_سطر *line1;
	size_t i, space, خانہ;
	size_t columns = 80;
	صحيح character_چوڑائی;
	size_t سطر_شروع;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, چوڑائی, اچائی);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->چوڑائی(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_چوڑائی);
	إذا (res != NSERROR_OK) {
		أرجع;
	}

	columns = (چوڑائی - MARGIN - MARGIN) * 8 / character_چوڑائی;
	textplain_tab_چوڑائی = (TAB_چوڑائی * character_چوڑائی) / 8;

	متن->formatted_چوڑائی = چوڑائی;

	متن->physical_سطر_گنتی = 0;

	إذا (!سطر) {
		متن->physical_سطر = سطر =
			حافظہ_دیو(حجم(بنية textplain_سطر) * (1024 + 3));
		إذا (!سطر)
			اذهب no_حافظہ;
	}

	سطر[سطر_گنتی++].شروع = سطر_شروع = 0;
	space = 0;
	i = 0;
	خانہ = 0;
	طالما (i < utf8_انکڑے_ناپ) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool رکن;
		size_t اگلا_خانہ;
		parserutils_error perror;

		perror = parserutils_charset_utf8_منزل_ucs4((ثابت uint8_t *)utf8_انکڑے + i, utf8_انکڑے_ناپ - i, &chr, &csize);
		إذا (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		رکن = (chr == '\n' || chr == '\r');

		اگلا_خانہ = خانہ + 1;

		إذا (chr == '\t') {
			اگلا_خانہ = (اگلا_خانہ + TAB_چوڑائی - 1) & ~(TAB_چوڑائی - 1);
		}

		إذا (رکن || اگلا_خانہ >= columns) {
			إذا (سطر_گنتی % 1024 == 0) {
				line1 = realloc(سطر,
						حجم(بنية textplain_سطر) *
						(سطر_گنتی + 1024 + 3));
				إذا (!line1)
					اذهب no_حافظہ;
				متن->physical_سطر = سطر = line1;
			}

			إذا (رکن) {
				سطر[سطر_گنتی-1].لمائی = i - سطر_شروع;

				/* skip second char of CR/LF or LF/CR pair */
				إذا (i + 1 < utf8_انکڑے_ناپ &&
				    utf8_انکڑے[i+1] != utf8_انکڑے[i] &&
				    (utf8_انکڑے[i+1] == '\n' ||
				     utf8_انکڑے[i+1] == '\r')) {
					i++;
				}
			} وإلا {
				إذا (space) {
					/* break at last space in line */
					i = space;
					سطر[سطر_گنتی-1].لمائی = (i + 1) - سطر_شروع;
				} وإلا
					سطر[سطر_گنتی-1].لمائی = i - سطر_شروع;
			}

			سطر[سطر_گنتی++].شروع = سطر_شروع = i + 1;
			خانہ = 0;
			space = 0;
		} وإلا {
			خانہ++;
			إذا (chr == ' ')
				space = i;
		}
		i += csize;
	}
	سطر[سطر_گنتی-1].لمائی = i - سطر[سطر_گنتی-1].شروع;
	سطر[سطر_گنتی].شروع = utf8_انکڑے_ناپ;

	متن->physical_سطر_گنتی = سطر_گنتی;
	c->چوڑائی = چوڑائی;
	c->اچائی = سطر_گنتی * textplain_سطر_اچائی() + MARGIN + MARGIN;

	أرجع;

no_حافظہ:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", سطر_گنتی);
	أرجع;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ساكن فراغ textplain_destroy(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	lwc_string_unref(متن->encoding);

	إذا (متن->inputstream != عدم) {
		parserutils_inputstream_destroy(متن->inputstream);
	}

	إذا (متن->physical_سطر != عدم) {
		حرر(متن->physical_سطر);
	}

	إذا (متن->utf8_انکڑے != عدم) {
		حرر(متن->utf8_انکڑے);
	}

	إذا (متن->sel != عدم) {
		selection_destroy(متن->sel);
	}
}


ساكن nserror textplain_clone(ثابت بنية content *old, بنية content **newc)
{
	ثابت textplain_content *old_متن = (textplain_content *) old;
	textplain_content *متن;
	nserror error;
	ثابت uint8_t *انکڑے;
	size_t ناپ;

	متن = صفر_حافظہ_دیو(1, حجم(textplain_content));
	إذا (متن == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &متن->ادھار);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->ادھار);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_اندرونی(متن, old_متن->encoding);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->ادھار);
		أرجع error;
	}

	انکڑے = content_get_سروت_انکڑے_2(&متن->ادھار, &ناپ);
	إذا (ناپ > 0) {
		إذا (textplain_process_انکڑے(&متن->ادھار,
					   (ثابت حرف *)انکڑے,
					   ناپ) == false) {
			content_destroy(&متن->ادھار);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
	    old->حالت == CONTENT_حالت_پورا) {
		إذا (textplain_convert(&متن->ادھار) == false) {
			content_destroy(&متن->ادھار);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	أرجع NSERROR_OK;
}


ساكن content_type textplain_content_type(فراغ)
{
	أرجع CONTENT_TEXTPLAIN;
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
ساكن size_t
textplain_وتھ_from_coords(بنية content *c, صحيح x, صحيح y, صحيح dir)
{
	textplain_content *textc = (textplain_content *) c;
	عائم سطر_اچائی = textplain_سطر_اچائی();
	بنية textplain_سطر *سطر;
	ثابت حرف *متن;
	بنانشانی nlines;
	size_t لمائی;
	صحيح idx;

	أكد(c != عدم);

	y = (صحيح)((عائم)(y - MARGIN) / سطر_اچائی);
	x -= MARGIN;

	nlines = textc->physical_سطر_گنتی;
	إذا (!nlines)
		أرجع 0;

	إذا (y <= 0) y = 0;
	وإلا إذا ((بنانشانی)y >= nlines)
		y = nlines - 1;

	سطر = &textc->physical_سطر[y];
	متن = textc->utf8_انکڑے + سطر->شروع;
	لمائی = سطر->لمائی;
	idx = 0;

	طالما (x > 0) {
		size_t اگلا_وتھ = 0;
		صحيح چوڑائی = ودھ_توں_ودھ_عدد;

		طالما (اگلا_وتھ < لمائی && متن[اگلا_وتھ] != '\t') {
			اگلا_وتھ = utf8_اگلا(متن, لمائی, اگلا_وتھ);
		}

		إذا (اگلا_وتھ < لمائی) {
			guit->layout->چوڑائی(&textplain_style,
					    متن,
					    اگلا_وتھ,
					    &چوڑائی);
		}

		إذا (x <= چوڑائی) {
			صحيح pixel_وتھ;
			size_t char_وتھ;

			guit->layout->position(&textplain_style,
					       متن, اگلا_وتھ, x,
					       &char_وتھ, &pixel_وتھ);

			idx += char_وتھ;
			اكسر;
		}

		x -= چوڑائی;
		لمائی -= اگلا_وتھ;
		متن += اگلا_وتھ;
		idx += اگلا_وتھ;

		/* check if it's within the tab */
		چوڑائی = textplain_tab_چوڑائی - (چوڑائی % textplain_tab_چوڑائی);
		إذا (x <= چوڑائی) اكسر;

		x -= چوڑائی;
		لمائی--;
		متن++;
		idx++;
	}

	أرجع سطر->شروع + idx;
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
ساكن nserror
textplain_mouse_action(بنية content *c,
		       بنية browser_کھڑکی *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;
	browser_اشارہ_گر_shape اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	ثابت حرف *حالت = 0;
	size_t idx;
	صحيح dir = 0;

	browser_کھڑکی_سیٹ_drag_type(bw, DRAGGING_NONE, عدم);

	idx = textplain_وتھ_from_coords(c, x, y, dir);
	إذا (selection_click(متن->sel, متن->bw, mouse, idx)) {

		إذا (selection_dragging(متن->sel)) {
			browser_کھڑکی_سیٹ_drag_type(bw,
						     DRAGGING_SELECTION, عدم);
			حالت = messages_get("Selecting");
		}

	} وإلا {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_کھڑکی_ورقہ_drag_شروع(bw, x, y);
			اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	}

	پیغام_انکڑے.explicit_حالت_متن = حالت;
	content_نشر(c, CONTENT_پیغام_حالت, &پیغام_انکڑے);

	پیغام_انکڑے.اشارہ_گر = اشارہ_گر;
	content_نشر(c, CONTENT_پیغام_اشارہ_گر, &پیغام_انکڑے);

	أرجع NSERROR_OK;
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
ساكن nserror
textplain_mouse_track(بنية content *c,
		      بنية browser_کھڑکی *bw,
		      browser_mouse_state mouse,
		      صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;

	إذا (browser_کھڑکی_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروع(متن->sel))
			dir = 1;

		idx = textplain_وتھ_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);

		browser_کھڑکی_سیٹ_drag_type(bw, DRAGGING_NONE, عدم);
	}

	بدل (browser_کھڑکی_get_drag_type(bw)) {

	حالة DRAGGING_SELECTION: {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروع(متن->sel)) dir = 1;

		idx = textplain_وتھ_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);
	}
		اكسر;

	افتراضي:
		textplain_mouse_action(c, bw, mouse, x, y);
		اكسر;
	}

	أرجع NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ساكن bool textplain_keypress(بنية content *c, uint32_t کلید)
{
	textplain_content *متن = (textplain_content *) c;
	بنية selection *sel = متن->sel;

	بدل (کلید) {
	حالة NS_کلید_COPY_SELECTION:
		selection_copy_منزل_clipboard(sel);
		أرجع true;

	حالة NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		أرجع selection_clear(sel, true);
	}

	أرجع false;
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
ساكن bool
متن_draw(ثابت حرف *utf8_متن,
	    size_t utf8_len,
	    size_t وتھ,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح اچائی,
	    عائم scale,
	    textplain_content *متن,
	    ثابت بنية selection *sel,
	    ثابت بنية redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.ناپ *= scale;

	/* is this box part of a selection? */
	إذا (ctx->interactive == true) {
		بنانشانی len = utf8_len;
		بنانشانی شروع_idx;
		بنانشانی سرا_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  وتھ,
					  وتھ + len,
					  &شروع_idx,
					  &سرا_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (متن->ادھار.textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(متن->ادھار.textsearch.context,
						     وتھ,
						     وتھ + len,
						     &شروع_idx,
						     &سرا_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بنانشانی endtxt_idx = سرا_idx;
			bool clip_بدلیا = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (سرا_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(سرا_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->چوڑائی(&textplain_style,
						  utf8_متن,
						  شروع_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->چوڑائی(&textplain_style,
						  utf8_متن,
						  endtxt_idx,
						  &endx);
			إذا (res != NSERROR_OK) {
				endx = 0;
			}

			إذا (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			إذا (شروع_idx > 0) {
				res = ctx->plot->متن(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحيح)(اچائی * 0.75 * scale),
					      utf8_متن,
					      شروع_idx);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}
			}

			pstyle_fill_hback.fill_رنگ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + اچائی * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			إذا (شروع_idx > 0) {
				صحيح px0 = max(x + startx, clip->x0);
				صحيح px1 = min(x + endx, clip->x1);

				إذا (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_بدلیا = true;
				} وإلا {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_منزل_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			إذا (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(اچائی * 0.75 * scale),
					     utf8_متن,
					     endtxt_idx) != NSERROR_OK)) {
				أرجع false;
			}

			/* draw any text succeeding highlighted portion */
			إذا (endtxt_idx < utf8_len) {
				صحيح px0 = max(x + endx, clip->x0);
				إذا (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_بدلیا = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(اچائی * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_بدلیا &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (اچائی * 0.75 * scale),
				      utf8_متن,
				      utf8_len);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
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
ساكن bool
textplain_redraw(بنية content *c,
		 بنية content_redraw_انکڑے *انکڑے,
		 ثابت بنية rect *clip,
		 ثابت بنية redraw_context *ctx)
{
	textplain_content *متن = (textplain_content *) c;
	بنية browser_کھڑکی *bw = متن->bw;
	حرف *utf8_انکڑے = متن->utf8_انکڑے;
	لما lineno;
	صحيح x = انکڑے->x;
	صحيح y = انکڑے->y;
	بنانشانی لما سطر_گنتی = متن->physical_سطر_گنتی;
	عائم سطر_اچائی = textplain_سطر_اچائی();
	عائم scaled_سطر_اچائی = سطر_اچائی * انکڑے->scale;
	لما line0 = (clip->y0 - y * انکڑے->scale) / scaled_سطر_اچائی - 1;
	لما line1 = (clip->y1 - y * انکڑے->scale) / scaled_سطر_اچائی + 1;
	بنية textplain_سطر *سطر = متن->physical_سطر;
	size_t لمائی;
	plot_style_t *plot_style_highlight;
	nserror res;

	إذا (line0 < 0)
		line0 = 0;
	إذا (line1 < 0)
		line1 = 0;
	إذا (سطر_گنتی < (بنانشانی لما) line0)
		line0 = سطر_گنتی;
	إذا (سطر_گنتی < (بنانشانی لما) line1)
		line1 = سطر_گنتی;
	إذا (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (!سطر)
		أرجع true;

	/* choose a suitable background colour for any highlighted text */
	إذا ((انکڑے->background_رنگ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وإلا
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = انکڑے->background_رنگ;

	x = (x + MARGIN) * انکڑے->scale;
	y = (y + MARGIN) * انکڑے->scale;
	لكل (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *متن_d = utf8_انکڑے + سطر[lineno].شروع;
		صحيح tab_چوڑائی = textplain_tab_چوڑائی * انکڑے->scale;
		size_t وتھ = 0;
		صحيح tx = x;

		إذا (!tab_چوڑائی) tab_چوڑائی = 1;

		لمائی = سطر[lineno].لمائی;
		إذا (!لمائی)
			استمر;

		طالما (وتھ < لمائی) {
			size_t اگلا_وتھ = وتھ;
			صحيح چوڑائی;
			صحيح ntx;
			nserror res;

			طالما ((اگلا_وتھ < لمائی) &&
			       (متن_d[اگلا_وتھ] != '\t')) {
				اگلا_وتھ = utf8_اگلا(متن_d,
							لمائی,
							اگلا_وتھ);
			}

			إذا (!متن_draw(متن_d + وتھ,
				       اگلا_وتھ - وتھ,
				       سطر[lineno].شروع + وتھ,
				       tx,
				       y + (lineno * scaled_سطر_اچائی),
				       clip,
				       سطر_اچائی,
				       انکڑے->scale,
				       متن,
				       متن->sel,
				       ctx)) {
				أرجع false;
			}

			إذا (اگلا_وتھ >= لمائی)
				اكسر;

			res = guit->layout->چوڑائی(&textplain_style,
						  &متن_d[وتھ],
						  اگلا_وتھ - وتھ,
						  &چوڑائی);
			/* locate end of string and align to next tab position */
			إذا (res == NSERROR_OK) {
				tx += (صحيح)(چوڑائی * انکڑے->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_چوڑائی) * tab_چوڑائی);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			إذا (bw) {
				بنانشانی tab_ofst = سطر[lineno].شروع + اگلا_وتھ;
				بنية selection *sel = متن->sel;
				bool highlighted = false;

				بنانشانی شروع_idx, سرا_idx;
				إذا (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &شروع_idx,
							  &سرا_idx)) {
					highlighted = true;
				}


				إذا (!highlighted &&
				    (c->textsearch.context != عدم)) {
					بنانشانی شروع_idx, سرا_idx;
					إذا (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &شروع_idx,
						    &سرا_idx)) {
						highlighted = true;
					}
				}

				إذا (highlighted) {
					بنية rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_سطر_اچائی);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_سطر_اچائی;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			وتھ = اگلا_وتھ + 1;
			tx = ntx;
		}
	}

	أرجع true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ساكن nserror
textplain_open(بنية content *c,
	       بنية browser_کھڑکی *bw,
	       بنية content *ورقہ,
	       بنية object_params *params)
{
	textplain_content *متن = (textplain_content *) c;

	متن->bw = bw;

	/* text selection */
	selection_init(متن->sel);

	أرجع NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ساكن nserror textplain_close(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	متن->bw = عدم;

	أرجع NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ساكن حرف *textplain_get_selection(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أرجع selection_get_copy(متن->sel);
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
ساكن صحيح
textplain_coord_from_وتھ(ثابت حرف *متن, size_t وتھ, size_t لمائی)
{
	صحيح x = 0;

	طالما (وتھ > 0) {
		size_t اگلا_وتھ = 0;
		صحيح tx;

		طالما (اگلا_وتھ < وتھ && متن[اگلا_وتھ] != '\t') {
			اگلا_وتھ = utf8_اگلا(متن, لمائی, اگلا_وتھ);
		}

		guit->layout->چوڑائی(&textplain_style, متن, اگلا_وتھ, &tx);

		x += tx;

		إذا (اگلا_وتھ >= وتھ)
			اكسر;

		/* align to next tab boundary */
		اگلا_وتھ++;
		x = (1 + (x / textplain_tab_چوڑائی)) * textplain_tab_چوڑائی;
		وتھ -= اگلا_وتھ;
		متن += اگلا_وتھ;
		لمائی -= اگلا_وتھ;
	}

	أرجع x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساكن بنانشانی لما textplain_سطر_گنتی(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أكد(c != عدم);

	أرجع متن->physical_سطر_گنتی;
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
ساكن حرف *
textplain_get_سطر(بنية content *c,
		   بنانشانی lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سطر *سطر;

	أكد(c != عدم);

	إذا (lineno >= متن->physical_سطر_گنتی)
		أرجع عدم;
	سطر = &متن->physical_سطر[lineno];

	*poffset = سطر->شروع;
	*plen = سطر->لمائی;
	أرجع متن->utf8_انکڑے + سطر->شروع;
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
ساكن صحيح textplain_لبھو_سطر(بنية content *c, بنانشانی وتھ)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سطر *سطر;
	صحيح nlines;
	صحيح lineno = 0;

	أكد(c != عدم);

	سطر = متن->physical_سطر;
	nlines = متن->physical_سطر_گنتی;

	إذا (وتھ > متن->utf8_انکڑے_ناپ) {
		أرجع -1;
	}

/* \todo - implement binary search here */
	طالما (lineno < nlines && سطر[lineno].شروع < وتھ) {
		lineno++;
	}
	إذا (سطر[lineno].شروع > وتھ) {
		lineno--;
	}

	أرجع lineno;
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
ساكن nserror
textplain_textsearch_لبھو(بنية content *c,
			  بنية textsearch_context *context,
			  ثابت حرف *نمونہ,
			  صحيح p_len,
			  bool case_sens)
{
	صحيح nlines = textplain_سطر_گنتی(c);
	صحيح سطر;
	nserror res = NSERROR_OK;

	لكل(سطر = 0; سطر < nlines; سطر++) {
		size_t وتھ, لمائی;
		ثابت حرف *متن;

		متن = textplain_get_سطر(c, سطر, &وتھ, &لمائی);
		إذا (متن) {
			طالما (لمائی > 0) {
				بنانشانی میل_لمائی;
				size_t شروع_idx;
				ثابت حرف *new_متن;
				ثابت حرف *pos;

				pos = content_textsearch_لبھو_نمونہ(
						متن,
						لمائی,
						نمونہ,
						p_len,
						case_sens,
						&میل_لمائی);
				إذا (!pos)
					اكسر;

				/* found string in line => add to list */
				شروع_idx = وتھ + (pos - متن);
				res = content_textsearch_جوڑو_میل(context,
						شروع_idx,
						شروع_idx + میل_لمائی,
						عدم,
						عدم);
				إذا (res != NSERROR_OK) {
					أرجع res;
				}

				new_متن = pos + میل_لمائی;
				وتھ += (new_متن - متن);
				لمائی -= (new_متن - متن);
				متن = new_متن;
			}
		}
	}

	أرجع res;
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
ساكن فراغ
textplain_coords_from_range(بنية content *c,
			    بنانشانی شروع,
			    بنانشانی سرا,
			    بنية rect *r)
{
	textplain_content *متن = (textplain_content *) c;
	عائم سطر_اچائی = textplain_سطر_اچائی();
	حرف *utf8_انکڑے;
	بنية textplain_سطر *سطر;
	بنانشانی lineno = 0;
	بنانشانی nlines;

	أكد(c != عدم);
	أكد(شروع <= سرا);
	أكد(سرا <= متن->utf8_انکڑے_ناپ);

	utf8_انکڑے = متن->utf8_انکڑے;
	nlines = متن->physical_سطر_گنتی;
	سطر = متن->physical_سطر;

	/* find start */
	lineno = textplain_لبھو_سطر(c, شروع);

	r->y0 = (صحيح)(MARGIN + lineno * سطر_اچائی);

	إذا (lineno + 1 <= nlines || سطر[lineno + 1].شروع >= سرا) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_لبھو_سطر(c, سرا);

		r->x0 = 0;
		r->x1 = متن->formatted_چوڑائی;
	} وإلا {
		/* single line */
		ثابت حرف *متن = utf8_انکڑے + سطر[lineno].شروع;

		r->x0 = textplain_coord_from_وتھ(متن,
						    شروع - سطر[lineno].شروع,
						    سطر[lineno].لمائی);

		r->x1 = textplain_coord_from_وتھ(متن,
						    سرا - سطر[lineno].شروع,
						    سطر[lineno].لمائی);
	}

	r->y1 = (صحيح)(MARGIN + (lineno + 1) * سطر_اچائی);
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
ساكن حرف *
textplain_get_raw_انکڑے(بنية content *c,
		       بنانشانی شروع,
		       بنانشانی سرا,
		       size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	size_t utf8_ناپ;

	أكد(c != عدم);

	utf8_ناپ = متن->utf8_انکڑے_ناپ;

	/* any text at all? */
	إذا (!utf8_ناپ) أرجع عدم;

	/* clamp to valid offset range */
	إذا (شروع >= utf8_ناپ) شروع = utf8_ناپ;
	إذا (سرا >= utf8_ناپ) سرا = utf8_ناپ;

	*plen = سرا - شروع;

	أرجع متن->utf8_انکڑے + شروع;
}


/**
 * get bounds of a free text search match
 */
ساكن nserror
textplain_textsearch_bounds(بنية content *c,
			    بنانشانی شروع_idx,
			    بنانشانی سرا_idx,
			    بنية box *شروع_box,
			    بنية box *سرا_box,
			    بنية rect *bounds)
{
	textplain_coords_from_range(c, شروع_idx, سرا_idx, bounds);

	أرجع NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساكن nserror
textplain_textselection_redraw(بنية content *c,
			       بنانشانی شروع_idx,
			       بنانشانی سرا_idx)
{
	بنية rect r;

	إذا (سرا_idx <= شروع_idx) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, شروع_idx, سرا_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	أرجع NSERROR_OK;
}

ساكن nserror
textplain_textselection_copy(بنية content *c,
			     بنانشانی شروع_idx,
			     بنانشانی سرا_idx,
			     بنية selection_string *selstr)
{
	ثابت حرف *متن;
	size_t لمائی;
	bool res = false;

	متن = textplain_get_raw_انکڑے(c, شروع_idx, سرا_idx, &لمائی);
	إذا (متن != عدم) {
		res = selection_string_append(متن, لمائی, false, عدم, selstr);
	}
	إذا (res == false) {
		أرجع NSERROR_NOMEM;
	}
	أرجع NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ساكن nserror
textplain_textselection_get_سرا(بنية content *c, بنانشانی *سرا_idx)
{
	textplain_content *متن = (textplain_content *)c;

	*سرا_idx = متن->utf8_انکڑے_ناپ;
	أرجع NSERROR_OK;
}


/**
 * plain text content handler table
 */
ساكن ثابت content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_انکڑے = textplain_process_انکڑے,
	.انکڑے_complete = textplain_convert,
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
	.textsearch_لبھو = textplain_textsearch_لبھو,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_سرا = textplain_textselection_get_سرا,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(فراغ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	إذا (lerror != lwc_error_ok) {
		أرجع NSERROR_NOMEM;
	}

	error = content_factory_رجسٹر_handler("text/plain",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_رجسٹر_handler("application/json",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	أرجع error;
}




