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

بنية textplain_کرښه {
	size_t	پیل;
	size_t	اوږدتیا;
};

/**
 * plain text content
 */
تعريفنوع بنية textplain_content {
	بنية content بنسټ;

	lwc_string *encoding;
	فراغ *inputstream;
	حرف *utf8_معلومات;
	size_t utf8_معلومات_اندازه;
	size_t utf8_معلومات_allocated;
	بینښه اوږد physical_کرښه_شمېر;
	بنية textplain_کرښه *physical_کرښه;
	صحيح formatted_پلنوالی;
	بنية browser_کړکۍ *bw;

	بنية selection *sel; /** Selection state */

} textplain_content;


#تعریف_کړه CHUNK 32768 /* Must be a power of 2 */
#تعریف_کړه MARGIN 4

#تعریف_کړه TAB_پلنوالی 8  /* must be power of 2 currently */
#تعریف_کړه متن_اندازه 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساكن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.اندازه = متن_اندازه,
	.وزن = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساكن صحيح textplain_tab_پلنوالی = 256;  /* try for a sensible default */

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
textplain_charset_hack(ثابت uint8_t *معلومات,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *سرچینه)
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
textplain_create_دننی(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_معلومات;
	parserutils_inputstream *بهیر;
	parserutils_error error;

	textplain_style.اندازه = (nsoption_int(font_اندازه) * PLOT_STYLE_SCALE) / 10;

	utf8_معلومات = حافظه_ورکړه(CHUNK);
	إذا (utf8_معلومات == عدم)
		اذهب no_حافظه;

	error = parserutils_inputstream_create(lwc_string_معلومات(encoding), 0,
					       textplain_charset_hack, &بهیر);
	إذا (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &بهیر);
	}
	إذا (error != PARSERUTILS_OK) {
		حرر(utf8_معلومات);
		اذهب no_حافظه;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = بهیر;
	c->utf8_معلومات = utf8_معلومات;
	c->utf8_معلومات_اندازه = 0;
	c->utf8_معلومات_allocated = CHUNK;
	c->physical_کرښه = 0;
	c->physical_کرښه_شمېر = 0;
	c->formatted_پلنوالی = 0;
	c->bw = عدم;
	c->sel = selection_create((بنية content *)c);

	أرجع NSERROR_OK;

no_حافظه:
	content_خپرونه_error(&c->بنسټ, NSERROR_NOMEM, عدم);

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

	متن = حافظه_صفر_ورکړه(1, حجم(textplain_content));
	إذا (متن == عدم) {
		أرجع NSERROR_NOMEM;
	}

	error = content__init(&متن->بنسټ, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(متن);
		أرجع error;
	}

	error = http_parameter_list_پیدا_کړه_توکی(params, corestring_lwc_charset,
					      &encoding);
	إذا (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_دننی(متن, encoding);
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
textplain_copy_utf8_معلومات(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	إذا (c->utf8_معلومات_اندازه + len >= c->utf8_معلومات_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_معلومات;

		allocated = (c->utf8_معلومات_اندازه + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_معلومات = realloc(c->utf8_معلومات, allocated);
		إذا (utf8_معلومات == عدم)
			أرجع false;

		c->utf8_معلومات = utf8_معلومات;
		c->utf8_معلومات_allocated = allocated;
	}

	حافظه_نقل(c->utf8_معلومات + c->utf8_معلومات_اندازه, buf, len);
	c->utf8_معلومات_اندازه += len;

	أرجع true;
}


/**
 * drain input
 */
ساكن bool
textplain_drain_ننوت(textplain_content *c,
		      parserutils_inputstream *بهیر,
		      parserutils_error terminator)
{
	ساكن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, دننه_واټن = 0;

	طالما (parserutils_inputstream_peek(بهیر, دننه_واټن, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		إذا (chlen == 1 && *ch == 0) {
			إذا (دننه_واټن > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(بهیر, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				إذا (textplain_copy_utf8_معلومات(c, ch,
							     دننه_واټن) == false)
					أرجع false;
			}

			/* Emit U+FFFD */
			إذا (textplain_copy_utf8_معلومات(c, u_fffd, 3) == false)
				أرجع false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(بهیر, دننه_واټن + 1);
			/* Reset the read offset */
			دننه_واټن = 0;
		} وإلا {
			/* Accumulate input */
			دننه_واټن += chlen;

			إذا (دننه_واټن > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(بهیر, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				إذا (textplain_copy_utf8_معلومات(c, ch,
							     دننه_واټن) == false)
					أرجع false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(بهیر, دننه_واټن);
				/* Reset the read offset */
				دننه_واټن = 0;
			}
		}
	}

	إذا (دننه_واټن > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(بهیر, 0, &ch, &chlen);
		/* Emit any data remaining */
		إذا (textplain_copy_utf8_معلومات(c, ch, دننه_واټن) == false)
			أرجع false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(بهیر, دننه_واټن);
	}

	أرجع true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساكن bool
textplain_process_معلومات(بنية content *c, ثابت حرف *معلومات, بینښه صحيح اندازه)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *بهیر = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(بهیر,
					       (ثابت uint8_t *) معلومات, اندازه);
	إذا (error != PARSERUTILS_OK) {
		اذهب no_حافظه;
	}

	إذا (textplain_drain_ننوت(متن, بهیر, PARSERUTILS_NEEDDATA) == false)
		اذهب no_حافظه;

	أرجع true;

no_حافظه:
	content_خپرونه_error(c, NSERROR_NOMEM, عدم);
	أرجع false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساكن bool textplain_convert(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *بهیر = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(بهیر, عدم, 0);
	إذا (error != PARSERUTILS_OK) {
		أرجع false;
	}

	إذا (textplain_drain_ننوت(متن, بهیر, PARSERUTILS_EOF) == false)
		أرجع false;

	parserutils_inputstream_destroy(بهیر);
	متن->inputstream = عدم;

	content_سېټ_ready(c);
	content_سېټ_بشپړ(c);
	content_سېټ_حالت(c, messages_get("Done"));

	أرجع true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساكن عائم textplain_کرښه_لوړوالی(فراغ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	أرجع FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.اندازه / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساكن فراغ textplain_reformat(بنية content *c, صحيح پلنوالی, صحيح لوړوالی)
{
	textplain_content *متن = (textplain_content *) c;
	حرف *utf8_معلومات = متن->utf8_معلومات;
	size_t utf8_معلومات_اندازه = متن->utf8_معلومات_اندازه;
	بینښه اوږد کرښه_شمېر = 0;
	بنية textplain_کرښه *کرښه = متن->physical_کرښه;
	بنية textplain_کرښه *line1;
	size_t i, space, ستون;
	size_t columns = 80;
	صحيح character_پلنوالی;
	size_t کرښه_پیل;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, پلنوالی, لوړوالی);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->پلنوالی(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_پلنوالی);
	إذا (res != NSERROR_OK) {
		أرجع;
	}

	columns = (پلنوالی - MARGIN - MARGIN) * 8 / character_پلنوالی;
	textplain_tab_پلنوالی = (TAB_پلنوالی * character_پلنوالی) / 8;

	متن->formatted_پلنوالی = پلنوالی;

	متن->physical_کرښه_شمېر = 0;

	إذا (!کرښه) {
		متن->physical_کرښه = کرښه =
			حافظه_ورکړه(حجم(بنية textplain_کرښه) * (1024 + 3));
		إذا (!کرښه)
			اذهب no_حافظه;
	}

	کرښه[کرښه_شمېر++].پیل = کرښه_پیل = 0;
	space = 0;
	i = 0;
	ستون = 0;
	طالما (i < utf8_معلومات_اندازه) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool غړی;
		size_t بل_ستون;
		parserutils_error perror;

		perror = parserutils_charset_utf8_منزل_ucs4((ثابت uint8_t *)utf8_معلومات + i, utf8_معلومات_اندازه - i, &chr, &csize);
		إذا (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		غړی = (chr == '\n' || chr == '\r');

		بل_ستون = ستون + 1;

		إذا (chr == '\t') {
			بل_ستون = (بل_ستون + TAB_پلنوالی - 1) & ~(TAB_پلنوالی - 1);
		}

		إذا (غړی || بل_ستون >= columns) {
			إذا (کرښه_شمېر % 1024 == 0) {
				line1 = realloc(کرښه,
						حجم(بنية textplain_کرښه) *
						(کرښه_شمېر + 1024 + 3));
				إذا (!line1)
					اذهب no_حافظه;
				متن->physical_کرښه = کرښه = line1;
			}

			إذا (غړی) {
				کرښه[کرښه_شمېر-1].اوږدتیا = i - کرښه_پیل;

				/* skip second char of CR/LF or LF/CR pair */
				إذا (i + 1 < utf8_معلومات_اندازه &&
				    utf8_معلومات[i+1] != utf8_معلومات[i] &&
				    (utf8_معلومات[i+1] == '\n' ||
				     utf8_معلومات[i+1] == '\r')) {
					i++;
				}
			} وإلا {
				إذا (space) {
					/* break at last space in line */
					i = space;
					کرښه[کرښه_شمېر-1].اوږدتیا = (i + 1) - کرښه_پیل;
				} وإلا
					کرښه[کرښه_شمېر-1].اوږدتیا = i - کرښه_پیل;
			}

			کرښه[کرښه_شمېر++].پیل = کرښه_پیل = i + 1;
			ستون = 0;
			space = 0;
		} وإلا {
			ستون++;
			إذا (chr == ' ')
				space = i;
		}
		i += csize;
	}
	کرښه[کرښه_شمېر-1].اوږدتیا = i - کرښه[کرښه_شمېر-1].پیل;
	کرښه[کرښه_شمېر].پیل = utf8_معلومات_اندازه;

	متن->physical_کرښه_شمېر = کرښه_شمېر;
	c->پلنوالی = پلنوالی;
	c->لوړوالی = کرښه_شمېر * textplain_کرښه_لوړوالی() + MARGIN + MARGIN;

	أرجع;

no_حافظه:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", کرښه_شمېر);
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

	إذا (متن->physical_کرښه != عدم) {
		حرر(متن->physical_کرښه);
	}

	إذا (متن->utf8_معلومات != عدم) {
		حرر(متن->utf8_معلومات);
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
	ثابت uint8_t *معلومات;
	size_t اندازه;

	متن = حافظه_صفر_ورکړه(1, حجم(textplain_content));
	إذا (متن == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &متن->بنسټ);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->بنسټ);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_دننی(متن, old_متن->encoding);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->بنسټ);
		أرجع error;
	}

	معلومات = content_get_سرچینه_معلومات_2(&متن->بنسټ, &اندازه);
	إذا (اندازه > 0) {
		إذا (textplain_process_معلومات(&متن->بنسټ,
					   (ثابت حرف *)معلومات,
					   اندازه) == false) {
			content_destroy(&متن->بنسټ);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
	    old->حالت == CONTENT_حالت_بشپړ) {
		إذا (textplain_convert(&متن->بنسټ) == false) {
			content_destroy(&متن->بنسټ);
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
textplain_دننه_واټن_from_coords(بنية content *c, صحيح x, صحيح y, صحيح dir)
{
	textplain_content *textc = (textplain_content *) c;
	عائم کرښه_لوړوالی = textplain_کرښه_لوړوالی();
	بنية textplain_کرښه *کرښه;
	ثابت حرف *متن;
	بینښه nlines;
	size_t اوږدتیا;
	صحيح idx;

	أكد(c != عدم);

	y = (صحيح)((عائم)(y - MARGIN) / کرښه_لوړوالی);
	x -= MARGIN;

	nlines = textc->physical_کرښه_شمېر;
	إذا (!nlines)
		أرجع 0;

	إذا (y <= 0) y = 0;
	وإلا إذا ((بینښه)y >= nlines)
		y = nlines - 1;

	کرښه = &textc->physical_کرښه[y];
	متن = textc->utf8_معلومات + کرښه->پیل;
	اوږدتیا = کرښه->اوږدتیا;
	idx = 0;

	طالما (x > 0) {
		size_t بل_دننه_واټن = 0;
		صحيح پلنوالی = لوی_عدد;

		طالما (بل_دننه_واټن < اوږدتیا && متن[بل_دننه_واټن] != '\t') {
			بل_دننه_واټن = utf8_بل(متن, اوږدتیا, بل_دننه_واټن);
		}

		إذا (بل_دننه_واټن < اوږدتیا) {
			guit->layout->پلنوالی(&textplain_style,
					    متن,
					    بل_دننه_واټن,
					    &پلنوالی);
		}

		إذا (x <= پلنوالی) {
			صحيح pixel_دننه_واټن;
			size_t char_دننه_واټن;

			guit->layout->position(&textplain_style,
					       متن, بل_دننه_واټن, x,
					       &char_دننه_واټن, &pixel_دننه_واټن);

			idx += char_دننه_واټن;
			اكسر;
		}

		x -= پلنوالی;
		اوږدتیا -= بل_دننه_واټن;
		متن += بل_دننه_واټن;
		idx += بل_دننه_واټن;

		/* check if it's within the tab */
		پلنوالی = textplain_tab_پلنوالی - (پلنوالی % textplain_tab_پلنوالی);
		إذا (x <= پلنوالی) اكسر;

		x -= پلنوالی;
		اوږدتیا--;
		متن++;
		idx++;
	}

	أرجع کرښه->پیل + idx;
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
		       بنية browser_کړکۍ *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;
	browser_اشاره_ګر_shape اشاره_ګر = BROWSER_اشاره_ګر_DEFAULT;
	اتحاد content_پیغام_معلومات پیغام_معلومات;
	ثابت حرف *حالت = 0;
	size_t idx;
	صحيح dir = 0;

	browser_کړکۍ_سېټ_drag_type(bw, DRAGGING_NONE, عدم);

	idx = textplain_دننه_واټن_from_coords(c, x, y, dir);
	إذا (selection_click(متن->sel, متن->bw, mouse, idx)) {

		إذا (selection_dragging(متن->sel)) {
			browser_کړکۍ_سېټ_drag_type(bw,
						     DRAGGING_SELECTION, عدم);
			حالت = messages_get("Selecting");
		}

	} وإلا {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_کړکۍ_پاڼه_drag_پیل(bw, x, y);
			اشاره_ګر = BROWSER_اشاره_ګر_MOVE;
		}
	}

	پیغام_معلومات.explicit_حالت_متن = حالت;
	content_خپرونه(c, CONTENT_پیغام_حالت, &پیغام_معلومات);

	پیغام_معلومات.اشاره_ګر = اشاره_ګر;
	content_خپرونه(c, CONTENT_پیغام_اشاره_ګر, &پیغام_معلومات);

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
		      بنية browser_کړکۍ *bw,
		      browser_mouse_state mouse,
		      صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;

	إذا (browser_کړکۍ_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_پیل(متن->sel))
			dir = 1;

		idx = textplain_دننه_واټن_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);

		browser_کړکۍ_سېټ_drag_type(bw, DRAGGING_NONE, عدم);
	}

	بدل (browser_کړکۍ_get_drag_type(bw)) {

	حالة DRAGGING_SELECTION: {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_پیل(متن->sel)) dir = 1;

		idx = textplain_دننه_واټن_from_coords(c, x, y, dir);
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
ساكن bool textplain_keypress(بنية content *c, uint32_t کلی)
{
	textplain_content *متن = (textplain_content *) c;
	بنية selection *sel = متن->sel;

	بدل (کلی) {
	حالة NS_کلی_COPY_SELECTION:
		selection_copy_منزل_clipboard(sel);
		أرجع true;

	حالة NS_کلی_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_کلی_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_کلی_ESCAPE:
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
	    size_t دننه_واټن,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح لوړوالی,
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
	plot_fstyle.اندازه *= scale;

	/* is this box part of a selection? */
	إذا (ctx->interactive == true) {
		بینښه len = utf8_len;
		بینښه پیل_idx;
		بینښه پای_ټکی_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  دننه_واټن,
					  دننه_واټن + len,
					  &پیل_idx,
					  &پای_ټکی_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (متن->بنسټ.textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(متن->بنسټ.textsearch.context,
						     دننه_واټن,
						     دننه_واټن + len,
						     &پیل_idx,
						     &پای_ټکی_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بینښه endtxt_idx = پای_ټکی_idx;
			bool clip_بدل_شوی = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (پای_ټکی_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(پای_ټکی_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->پلنوالی(&textplain_style,
						  utf8_متن,
						  پیل_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->پلنوالی(&textplain_style,
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
			إذا (پیل_idx > 0) {
				res = ctx->plot->متن(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحيح)(لوړوالی * 0.75 * scale),
					      utf8_متن,
					      پیل_idx);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}
			}

			pstyle_fill_hback.fill_رنګ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + لوړوالی * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			إذا (پیل_idx > 0) {
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

					clip_بدل_شوی = true;
				} وإلا {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنګ;
			fstyle_hback.foreground = رنګ_منزل_bw_furthest(
				pstyle_fill_hback.fill_رنګ);

			إذا (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(لوړوالی * 0.75 * scale),
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

					clip_بدل_شوی = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(لوړوالی * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_بدل_شوی &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (لوړوالی * 0.75 * scale),
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
		 بنية content_redraw_معلومات *معلومات,
		 ثابت بنية rect *clip,
		 ثابت بنية redraw_context *ctx)
{
	textplain_content *متن = (textplain_content *) c;
	بنية browser_کړکۍ *bw = متن->bw;
	حرف *utf8_معلومات = متن->utf8_معلومات;
	اوږد lineno;
	صحيح x = معلومات->x;
	صحيح y = معلومات->y;
	بینښه اوږد کرښه_شمېر = متن->physical_کرښه_شمېر;
	عائم کرښه_لوړوالی = textplain_کرښه_لوړوالی();
	عائم scaled_کرښه_لوړوالی = کرښه_لوړوالی * معلومات->scale;
	اوږد line0 = (clip->y0 - y * معلومات->scale) / scaled_کرښه_لوړوالی - 1;
	اوږد line1 = (clip->y1 - y * معلومات->scale) / scaled_کرښه_لوړوالی + 1;
	بنية textplain_کرښه *کرښه = متن->physical_کرښه;
	size_t اوږدتیا;
	plot_style_t *plot_style_highlight;
	nserror res;

	إذا (line0 < 0)
		line0 = 0;
	إذا (line1 < 0)
		line1 = 0;
	إذا (کرښه_شمېر < (بینښه اوږد) line0)
		line0 = کرښه_شمېر;
	إذا (کرښه_شمېر < (بینښه اوږد) line1)
		line1 = کرښه_شمېر;
	إذا (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (!کرښه)
		أرجع true;

	/* choose a suitable background colour for any highlighted text */
	إذا ((معلومات->background_رنګ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وإلا
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = معلومات->background_رنګ;

	x = (x + MARGIN) * معلومات->scale;
	y = (y + MARGIN) * معلومات->scale;
	لكل (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *متن_d = utf8_معلومات + کرښه[lineno].پیل;
		صحيح tab_پلنوالی = textplain_tab_پلنوالی * معلومات->scale;
		size_t دننه_واټن = 0;
		صحيح tx = x;

		إذا (!tab_پلنوالی) tab_پلنوالی = 1;

		اوږدتیا = کرښه[lineno].اوږدتیا;
		إذا (!اوږدتیا)
			استمر;

		طالما (دننه_واټن < اوږدتیا) {
			size_t بل_دننه_واټن = دننه_واټن;
			صحيح پلنوالی;
			صحيح ntx;
			nserror res;

			طالما ((بل_دننه_واټن < اوږدتیا) &&
			       (متن_d[بل_دننه_واټن] != '\t')) {
				بل_دننه_واټن = utf8_بل(متن_d,
							اوږدتیا,
							بل_دننه_واټن);
			}

			إذا (!متن_draw(متن_d + دننه_واټن,
				       بل_دننه_واټن - دننه_واټن,
				       کرښه[lineno].پیل + دننه_واټن,
				       tx,
				       y + (lineno * scaled_کرښه_لوړوالی),
				       clip,
				       کرښه_لوړوالی,
				       معلومات->scale,
				       متن,
				       متن->sel,
				       ctx)) {
				أرجع false;
			}

			إذا (بل_دننه_واټن >= اوږدتیا)
				اكسر;

			res = guit->layout->پلنوالی(&textplain_style,
						  &متن_d[دننه_واټن],
						  بل_دننه_واټن - دننه_واټن,
						  &پلنوالی);
			/* locate end of string and align to next tab position */
			إذا (res == NSERROR_OK) {
				tx += (صحيح)(پلنوالی * معلومات->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_پلنوالی) * tab_پلنوالی);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			إذا (bw) {
				بینښه tab_ofst = کرښه[lineno].پیل + بل_دننه_واټن;
				بنية selection *sel = متن->sel;
				bool highlighted = false;

				بینښه پیل_idx, پای_ټکی_idx;
				إذا (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &پیل_idx,
							  &پای_ټکی_idx)) {
					highlighted = true;
				}


				إذا (!highlighted &&
				    (c->textsearch.context != عدم)) {
					بینښه پیل_idx, پای_ټکی_idx;
					إذا (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &پیل_idx,
						    &پای_ټکی_idx)) {
						highlighted = true;
					}
				}

				إذا (highlighted) {
					بنية rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_کرښه_لوړوالی);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_کرښه_لوړوالی;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			دننه_واټن = بل_دننه_واټن + 1;
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
	       بنية browser_کړکۍ *bw,
	       بنية content *پاڼه,
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
textplain_coord_from_دننه_واټن(ثابت حرف *متن, size_t دننه_واټن, size_t اوږدتیا)
{
	صحيح x = 0;

	طالما (دننه_واټن > 0) {
		size_t بل_دننه_واټن = 0;
		صحيح tx;

		طالما (بل_دننه_واټن < دننه_واټن && متن[بل_دننه_واټن] != '\t') {
			بل_دننه_واټن = utf8_بل(متن, اوږدتیا, بل_دننه_واټن);
		}

		guit->layout->پلنوالی(&textplain_style, متن, بل_دننه_واټن, &tx);

		x += tx;

		إذا (بل_دننه_واټن >= دننه_واټن)
			اكسر;

		/* align to next tab boundary */
		بل_دننه_واټن++;
		x = (1 + (x / textplain_tab_پلنوالی)) * textplain_tab_پلنوالی;
		دننه_واټن -= بل_دننه_واټن;
		متن += بل_دننه_واټن;
		اوږدتیا -= بل_دننه_واټن;
	}

	أرجع x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساكن بینښه اوږد textplain_کرښه_شمېر(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أكد(c != عدم);

	أرجع متن->physical_کرښه_شمېر;
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
textplain_get_کرښه(بنية content *c,
		   بینښه lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_کرښه *کرښه;

	أكد(c != عدم);

	إذا (lineno >= متن->physical_کرښه_شمېر)
		أرجع عدم;
	کرښه = &متن->physical_کرښه[lineno];

	*poffset = کرښه->پیل;
	*plen = کرښه->اوږدتیا;
	أرجع متن->utf8_معلومات + کرښه->پیل;
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
ساكن صحيح textplain_پیدا_کړه_کرښه(بنية content *c, بینښه دننه_واټن)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_کرښه *کرښه;
	صحيح nlines;
	صحيح lineno = 0;

	أكد(c != عدم);

	کرښه = متن->physical_کرښه;
	nlines = متن->physical_کرښه_شمېر;

	إذا (دننه_واټن > متن->utf8_معلومات_اندازه) {
		أرجع -1;
	}

/* \todo - implement binary search here */
	طالما (lineno < nlines && کرښه[lineno].پیل < دننه_واټن) {
		lineno++;
	}
	إذا (کرښه[lineno].پیل > دننه_واټن) {
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
textplain_textsearch_پیدا_کړه(بنية content *c,
			  بنية textsearch_context *context,
			  ثابت حرف *نمونه,
			  صحيح p_len,
			  bool case_sens)
{
	صحيح nlines = textplain_کرښه_شمېر(c);
	صحيح کرښه;
	nserror res = NSERROR_OK;

	لكل(کرښه = 0; کرښه < nlines; کرښه++) {
		size_t دننه_واټن, اوږدتیا;
		ثابت حرف *متن;

		متن = textplain_get_کرښه(c, کرښه, &دننه_واټن, &اوږدتیا);
		إذا (متن) {
			طالما (اوږدتیا > 0) {
				بینښه سمون_اوږدتیا;
				size_t پیل_idx;
				ثابت حرف *new_متن;
				ثابت حرف *pos;

				pos = content_textsearch_پیدا_کړه_نمونه(
						متن,
						اوږدتیا,
						نمونه,
						p_len,
						case_sens,
						&سمون_اوږدتیا);
				إذا (!pos)
					اكسر;

				/* found string in line => add to list */
				پیل_idx = دننه_واټن + (pos - متن);
				res = content_textsearch_جمع_کړه_سمون(context,
						پیل_idx,
						پیل_idx + سمون_اوږدتیا,
						عدم,
						عدم);
				إذا (res != NSERROR_OK) {
					أرجع res;
				}

				new_متن = pos + سمون_اوږدتیا;
				دننه_واټن += (new_متن - متن);
				اوږدتیا -= (new_متن - متن);
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
			    بینښه پیل,
			    بینښه پای_ټکی,
			    بنية rect *r)
{
	textplain_content *متن = (textplain_content *) c;
	عائم کرښه_لوړوالی = textplain_کرښه_لوړوالی();
	حرف *utf8_معلومات;
	بنية textplain_کرښه *کرښه;
	بینښه lineno = 0;
	بینښه nlines;

	أكد(c != عدم);
	أكد(پیل <= پای_ټکی);
	أكد(پای_ټکی <= متن->utf8_معلومات_اندازه);

	utf8_معلومات = متن->utf8_معلومات;
	nlines = متن->physical_کرښه_شمېر;
	کرښه = متن->physical_کرښه;

	/* find start */
	lineno = textplain_پیدا_کړه_کرښه(c, پیل);

	r->y0 = (صحيح)(MARGIN + lineno * کرښه_لوړوالی);

	إذا (lineno + 1 <= nlines || کرښه[lineno + 1].پیل >= پای_ټکی) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_پیدا_کړه_کرښه(c, پای_ټکی);

		r->x0 = 0;
		r->x1 = متن->formatted_پلنوالی;
	} وإلا {
		/* single line */
		ثابت حرف *متن = utf8_معلومات + کرښه[lineno].پیل;

		r->x0 = textplain_coord_from_دننه_واټن(متن,
						    پیل - کرښه[lineno].پیل,
						    کرښه[lineno].اوږدتیا);

		r->x1 = textplain_coord_from_دننه_واټن(متن,
						    پای_ټکی - کرښه[lineno].پیل,
						    کرښه[lineno].اوږدتیا);
	}

	r->y1 = (صحيح)(MARGIN + (lineno + 1) * کرښه_لوړوالی);
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
textplain_get_raw_معلومات(بنية content *c,
		       بینښه پیل,
		       بینښه پای_ټکی,
		       size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	size_t utf8_اندازه;

	أكد(c != عدم);

	utf8_اندازه = متن->utf8_معلومات_اندازه;

	/* any text at all? */
	إذا (!utf8_اندازه) أرجع عدم;

	/* clamp to valid offset range */
	إذا (پیل >= utf8_اندازه) پیل = utf8_اندازه;
	إذا (پای_ټکی >= utf8_اندازه) پای_ټکی = utf8_اندازه;

	*plen = پای_ټکی - پیل;

	أرجع متن->utf8_معلومات + پیل;
}


/**
 * get bounds of a free text search match
 */
ساكن nserror
textplain_textsearch_bounds(بنية content *c,
			    بینښه پیل_idx,
			    بینښه پای_ټکی_idx,
			    بنية box *پیل_box,
			    بنية box *پای_ټکی_box,
			    بنية rect *bounds)
{
	textplain_coords_from_range(c, پیل_idx, پای_ټکی_idx, bounds);

	أرجع NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساكن nserror
textplain_textselection_redraw(بنية content *c,
			       بینښه پیل_idx,
			       بینښه پای_ټکی_idx)
{
	بنية rect r;

	إذا (پای_ټکی_idx <= پیل_idx) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, پیل_idx, پای_ټکی_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	أرجع NSERROR_OK;
}

ساكن nserror
textplain_textselection_copy(بنية content *c,
			     بینښه پیل_idx,
			     بینښه پای_ټکی_idx,
			     بنية selection_string *selstr)
{
	ثابت حرف *متن;
	size_t اوږدتیا;
	bool res = false;

	متن = textplain_get_raw_معلومات(c, پیل_idx, پای_ټکی_idx, &اوږدتیا);
	إذا (متن != عدم) {
		res = selection_string_append(متن, اوږدتیا, false, عدم, selstr);
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
textplain_textselection_get_پای_ټکی(بنية content *c, بینښه *پای_ټکی_idx)
{
	textplain_content *متن = (textplain_content *)c;

	*پای_ټکی_idx = متن->utf8_معلومات_اندازه;
	أرجع NSERROR_OK;
}


/**
 * plain text content handler table
 */
ساكن ثابت content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_معلومات = textplain_process_معلومات,
	.معلومات_complete = textplain_convert,
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
	.textsearch_پیدا_کړه = textplain_textsearch_پیدا_کړه,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_پای_ټکی = textplain_textselection_get_پای_ټکی,
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

	error = content_factory_ثبتوونکی_handler("text/plain",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_ثبتوونکی_handler("application/json",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	أرجع error;
}




