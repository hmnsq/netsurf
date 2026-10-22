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

بنية textplain_سٽ {
	size_t	شروعات;
	size_t	ڊيگهائي;
};

/**
 * plain text content
 */
تعريفنوع بنية textplain_content {
	بنية content بنياد;

	lwc_string *encoding;
	فراغ *inputstream;
	حرف *utf8_معلومات;
	size_t utf8_معلومات_سائيز;
	size_t utf8_معلومات_allocated;
	بنانشان ڊگهو physical_سٽ_ڳڻپ;
	بنية textplain_سٽ *physical_سٽ;
	صحيح formatted_ويڪر;
	بنية browser_دري *bw;

	بنية selection *sel; /** Selection state */

} textplain_content;


#وصف_ڪريو CHUNK 32768 /* Must be a power of 2 */
#وصف_ڪريو MARGIN 4

#وصف_ڪريو TAB_ويڪر 8  /* must be power of 2 currently */
#وصف_ڪريو متن_سائيز 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساكن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.سائيز = متن_سائيز,
	.وزن = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساكن صحيح textplain_tab_ويڪر = 256;  /* try for a sensible default */

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
		       uint32_t *ذريعو)
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
textplain_create_اندروني(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_معلومات;
	parserutils_inputstream *وهڪرو;
	parserutils_error error;

	textplain_style.سائيز = (nsoption_int(font_سائيز) * PLOT_STYLE_SCALE) / 10;

	utf8_معلومات = يادگيري_ڏيو(CHUNK);
	إذا (utf8_معلومات == عدم)
		اذهب no_يادگيري;

	error = parserutils_inputstream_create(lwc_string_معلومات(encoding), 0,
					       textplain_charset_hack, &وهڪرو);
	إذا (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &وهڪرو);
	}
	إذا (error != PARSERUTILS_OK) {
		حرر(utf8_معلومات);
		اذهب no_يادگيري;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = وهڪرو;
	c->utf8_معلومات = utf8_معلومات;
	c->utf8_معلومات_سائيز = 0;
	c->utf8_معلومات_allocated = CHUNK;
	c->physical_سٽ = 0;
	c->physical_سٽ_ڳڻپ = 0;
	c->formatted_ويڪر = 0;
	c->bw = عدم;
	c->sel = selection_create((بنية content *)c);

	أرجع NSERROR_OK;

no_يادگيري:
	content_نشر_error(&c->بنياد, NSERROR_NOMEM, عدم);

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

	متن = صفر_يادگيري_ڏيو(1, حجم(textplain_content));
	إذا (متن == عدم) {
		أرجع NSERROR_NOMEM;
	}

	error = content__init(&متن->بنياد, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(متن);
		أرجع error;
	}

	error = http_parameter_list_ڳوليو_شي(params, corestring_lwc_charset,
					      &encoding);
	إذا (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_اندروني(متن, encoding);
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
	إذا (c->utf8_معلومات_سائيز + len >= c->utf8_معلومات_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_معلومات;

		allocated = (c->utf8_معلومات_سائيز + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_معلومات = realloc(c->utf8_معلومات, allocated);
		إذا (utf8_معلومات == عدم)
			أرجع false;

		c->utf8_معلومات = utf8_معلومات;
		c->utf8_معلومات_allocated = allocated;
	}

	يادگيري_نقل(c->utf8_معلومات + c->utf8_معلومات_سائيز, buf, len);
	c->utf8_معلومات_سائيز += len;

	أرجع true;
}


/**
 * drain input
 */
ساكن bool
textplain_drain_داخلا(textplain_content *c,
		      parserutils_inputstream *وهڪرو,
		      parserutils_error terminator)
{
	ساكن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, وٿي = 0;

	طالما (parserutils_inputstream_peek(وهڪرو, وٿي, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		إذا (chlen == 1 && *ch == 0) {
			إذا (وٿي > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(وهڪرو, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				إذا (textplain_copy_utf8_معلومات(c, ch,
							     وٿي) == false)
					أرجع false;
			}

			/* Emit U+FFFD */
			إذا (textplain_copy_utf8_معلومات(c, u_fffd, 3) == false)
				أرجع false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(وهڪرو, وٿي + 1);
			/* Reset the read offset */
			وٿي = 0;
		} وإلا {
			/* Accumulate input */
			وٿي += chlen;

			إذا (وٿي > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(وهڪرو, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				إذا (textplain_copy_utf8_معلومات(c, ch,
							     وٿي) == false)
					أرجع false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(وهڪرو, وٿي);
				/* Reset the read offset */
				وٿي = 0;
			}
		}
	}

	إذا (وٿي > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(وهڪرو, 0, &ch, &chlen);
		/* Emit any data remaining */
		إذا (textplain_copy_utf8_معلومات(c, ch, وٿي) == false)
			أرجع false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(وهڪرو, وٿي);
	}

	أرجع true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساكن bool
textplain_process_معلومات(بنية content *c, ثابت حرف *معلومات, بنانشان صحيح سائيز)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *وهڪرو = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(وهڪرو,
					       (ثابت uint8_t *) معلومات, سائيز);
	إذا (error != PARSERUTILS_OK) {
		اذهب no_يادگيري;
	}

	إذا (textplain_drain_داخلا(متن, وهڪرو, PARSERUTILS_NEEDDATA) == false)
		اذهب no_يادگيري;

	أرجع true;

no_يادگيري:
	content_نشر_error(c, NSERROR_NOMEM, عدم);
	أرجع false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساكن bool textplain_convert(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *وهڪرو = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(وهڪرو, عدم, 0);
	إذا (error != PARSERUTILS_OK) {
		أرجع false;
	}

	إذا (textplain_drain_داخلا(متن, وهڪرو, PARSERUTILS_EOF) == false)
		أرجع false;

	parserutils_inputstream_destroy(وهڪرو);
	متن->inputstream = عدم;

	content_سيٽ_ready(c);
	content_سيٽ_پورو(c);
	content_سيٽ_حالت(c, messages_get("Done"));

	أرجع true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساكن عائم textplain_سٽ_اوچائي(فراغ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	أرجع FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.سائيز / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساكن فراغ textplain_reformat(بنية content *c, صحيح ويڪر, صحيح اوچائي)
{
	textplain_content *متن = (textplain_content *) c;
	حرف *utf8_معلومات = متن->utf8_معلومات;
	size_t utf8_معلومات_سائيز = متن->utf8_معلومات_سائيز;
	بنانشان ڊگهو سٽ_ڳڻپ = 0;
	بنية textplain_سٽ *سٽ = متن->physical_سٽ;
	بنية textplain_سٽ *line1;
	size_t i, space, کانو;
	size_t columns = 80;
	صحيح character_ويڪر;
	size_t سٽ_شروعات;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, ويڪر, اوچائي);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->ويڪر(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_ويڪر);
	إذا (res != NSERROR_OK) {
		أرجع;
	}

	columns = (ويڪر - MARGIN - MARGIN) * 8 / character_ويڪر;
	textplain_tab_ويڪر = (TAB_ويڪر * character_ويڪر) / 8;

	متن->formatted_ويڪر = ويڪر;

	متن->physical_سٽ_ڳڻپ = 0;

	إذا (!سٽ) {
		متن->physical_سٽ = سٽ =
			يادگيري_ڏيو(حجم(بنية textplain_سٽ) * (1024 + 3));
		إذا (!سٽ)
			اذهب no_يادگيري;
	}

	سٽ[سٽ_ڳڻپ++].شروعات = سٽ_شروعات = 0;
	space = 0;
	i = 0;
	کانو = 0;
	طالما (i < utf8_معلومات_سائيز) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool رڪن;
		size_t ايندڙ_کانو;
		parserutils_error perror;

		perror = parserutils_charset_utf8_منزل_ucs4((ثابت uint8_t *)utf8_معلومات + i, utf8_معلومات_سائيز - i, &chr, &csize);
		إذا (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		رڪن = (chr == '\n' || chr == '\r');

		ايندڙ_کانو = کانو + 1;

		إذا (chr == '\t') {
			ايندڙ_کانو = (ايندڙ_کانو + TAB_ويڪر - 1) & ~(TAB_ويڪر - 1);
		}

		إذا (رڪن || ايندڙ_کانو >= columns) {
			إذا (سٽ_ڳڻپ % 1024 == 0) {
				line1 = realloc(سٽ,
						حجم(بنية textplain_سٽ) *
						(سٽ_ڳڻپ + 1024 + 3));
				إذا (!line1)
					اذهب no_يادگيري;
				متن->physical_سٽ = سٽ = line1;
			}

			إذا (رڪن) {
				سٽ[سٽ_ڳڻپ-1].ڊيگهائي = i - سٽ_شروعات;

				/* skip second char of CR/LF or LF/CR pair */
				إذا (i + 1 < utf8_معلومات_سائيز &&
				    utf8_معلومات[i+1] != utf8_معلومات[i] &&
				    (utf8_معلومات[i+1] == '\n' ||
				     utf8_معلومات[i+1] == '\r')) {
					i++;
				}
			} وإلا {
				إذا (space) {
					/* break at last space in line */
					i = space;
					سٽ[سٽ_ڳڻپ-1].ڊيگهائي = (i + 1) - سٽ_شروعات;
				} وإلا
					سٽ[سٽ_ڳڻپ-1].ڊيگهائي = i - سٽ_شروعات;
			}

			سٽ[سٽ_ڳڻپ++].شروعات = سٽ_شروعات = i + 1;
			کانو = 0;
			space = 0;
		} وإلا {
			کانو++;
			إذا (chr == ' ')
				space = i;
		}
		i += csize;
	}
	سٽ[سٽ_ڳڻپ-1].ڊيگهائي = i - سٽ[سٽ_ڳڻپ-1].شروعات;
	سٽ[سٽ_ڳڻپ].شروعات = utf8_معلومات_سائيز;

	متن->physical_سٽ_ڳڻپ = سٽ_ڳڻپ;
	c->ويڪر = ويڪر;
	c->اوچائي = سٽ_ڳڻپ * textplain_سٽ_اوچائي() + MARGIN + MARGIN;

	أرجع;

no_يادگيري:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", سٽ_ڳڻپ);
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

	إذا (متن->physical_سٽ != عدم) {
		حرر(متن->physical_سٽ);
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
	size_t سائيز;

	متن = صفر_يادگيري_ڏيو(1, حجم(textplain_content));
	إذا (متن == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &متن->بنياد);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->بنياد);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_اندروني(متن, old_متن->encoding);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->بنياد);
		أرجع error;
	}

	معلومات = content_get_ذريعو_معلومات_2(&متن->بنياد, &سائيز);
	إذا (سائيز > 0) {
		إذا (textplain_process_معلومات(&متن->بنياد,
					   (ثابت حرف *)معلومات,
					   سائيز) == false) {
			content_destroy(&متن->بنياد);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
	    old->حالت == CONTENT_حالت_پورو) {
		إذا (textplain_convert(&متن->بنياد) == false) {
			content_destroy(&متن->بنياد);
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
textplain_وٿي_from_coords(بنية content *c, صحيح x, صحيح y, صحيح dir)
{
	textplain_content *textc = (textplain_content *) c;
	عائم سٽ_اوچائي = textplain_سٽ_اوچائي();
	بنية textplain_سٽ *سٽ;
	ثابت حرف *متن;
	بنانشان nlines;
	size_t ڊيگهائي;
	صحيح idx;

	أكد(c != عدم);

	y = (صحيح)((عائم)(y - MARGIN) / سٽ_اوچائي);
	x -= MARGIN;

	nlines = textc->physical_سٽ_ڳڻپ;
	إذا (!nlines)
		أرجع 0;

	إذا (y <= 0) y = 0;
	وإلا إذا ((بنانشان)y >= nlines)
		y = nlines - 1;

	سٽ = &textc->physical_سٽ[y];
	متن = textc->utf8_معلومات + سٽ->شروعات;
	ڊيگهائي = سٽ->ڊيگهائي;
	idx = 0;

	طالما (x > 0) {
		size_t ايندڙ_وٿي = 0;
		صحيح ويڪر = وڌ_کان_وڌ_عدد;

		طالما (ايندڙ_وٿي < ڊيگهائي && متن[ايندڙ_وٿي] != '\t') {
			ايندڙ_وٿي = utf8_ايندڙ(متن, ڊيگهائي, ايندڙ_وٿي);
		}

		إذا (ايندڙ_وٿي < ڊيگهائي) {
			guit->layout->ويڪر(&textplain_style,
					    متن,
					    ايندڙ_وٿي,
					    &ويڪر);
		}

		إذا (x <= ويڪر) {
			صحيح pixel_وٿي;
			size_t char_وٿي;

			guit->layout->position(&textplain_style,
					       متن, ايندڙ_وٿي, x,
					       &char_وٿي, &pixel_وٿي);

			idx += char_وٿي;
			اكسر;
		}

		x -= ويڪر;
		ڊيگهائي -= ايندڙ_وٿي;
		متن += ايندڙ_وٿي;
		idx += ايندڙ_وٿي;

		/* check if it's within the tab */
		ويڪر = textplain_tab_ويڪر - (ويڪر % textplain_tab_ويڪر);
		إذا (x <= ويڪر) اكسر;

		x -= ويڪر;
		ڊيگهائي--;
		متن++;
		idx++;
	}

	أرجع سٽ->شروعات + idx;
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
		       بنية browser_دري *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;
	browser_اشارو_shape اشارو = BROWSER_اشارو_DEFAULT;
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	ثابت حرف *حالت = 0;
	size_t idx;
	صحيح dir = 0;

	browser_دري_سيٽ_drag_type(bw, DRAGGING_NONE, عدم);

	idx = textplain_وٿي_from_coords(c, x, y, dir);
	إذا (selection_click(متن->sel, متن->bw, mouse, idx)) {

		إذا (selection_dragging(متن->sel)) {
			browser_دري_سيٽ_drag_type(bw,
						     DRAGGING_SELECTION, عدم);
			حالت = messages_get("Selecting");
		}

	} وإلا {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_دري_صفحو_drag_شروعات(bw, x, y);
			اشارو = BROWSER_اشارو_MOVE;
		}
	}

	نياپو_معلومات.explicit_حالت_متن = حالت;
	content_نشر(c, CONTENT_نياپو_حالت, &نياپو_معلومات);

	نياپو_معلومات.اشارو = اشارو;
	content_نشر(c, CONTENT_نياپو_اشارو, &نياپو_معلومات);

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
		      بنية browser_دري *bw,
		      browser_mouse_state mouse,
		      صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;

	إذا (browser_دري_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروعات(متن->sel))
			dir = 1;

		idx = textplain_وٿي_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);

		browser_دري_سيٽ_drag_type(bw, DRAGGING_NONE, عدم);
	}

	بدل (browser_دري_get_drag_type(bw)) {

	حالة DRAGGING_SELECTION: {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروعات(متن->sel)) dir = 1;

		idx = textplain_وٿي_from_coords(c, x, y, dir);
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
ساكن bool textplain_keypress(بنية content *c, uint32_t ڪنجي)
{
	textplain_content *متن = (textplain_content *) c;
	بنية selection *sel = متن->sel;

	بدل (ڪنجي) {
	حالة NS_ڪنجي_COPY_SELECTION:
		selection_copy_منزل_clipboard(sel);
		أرجع true;

	حالة NS_ڪنجي_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_ڪنجي_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_ڪنجي_ESCAPE:
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
	    size_t وٿي,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح اوچائي,
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
	plot_fstyle.سائيز *= scale;

	/* is this box part of a selection? */
	إذا (ctx->interactive == true) {
		بنانشان len = utf8_len;
		بنانشان شروعات_idx;
		بنانشان پڇاڙي_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  وٿي,
					  وٿي + len,
					  &شروعات_idx,
					  &پڇاڙي_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (متن->بنياد.textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(متن->بنياد.textsearch.context,
						     وٿي,
						     وٿي + len,
						     &شروعات_idx,
						     &پڇاڙي_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بنانشان endtxt_idx = پڇاڙي_idx;
			bool clip_بدليو = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (پڇاڙي_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(پڇاڙي_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->ويڪر(&textplain_style,
						  utf8_متن,
						  شروعات_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->ويڪر(&textplain_style,
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
			إذا (شروعات_idx > 0) {
				res = ctx->plot->متن(ctx,
					      &plot_fstyle,
					      x,
					      y + (صحيح)(اوچائي * 0.75 * scale),
					      utf8_متن,
					      شروعات_idx);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}
			}

			pstyle_fill_hback.fill_رنگ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + اوچائي * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			إذا (شروعات_idx > 0) {
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

					clip_بدليو = true;
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
					     y + (صحيح)(اوچائي * 0.75 * scale),
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

					clip_بدليو = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(اوچائي * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_بدليو &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (اوچائي * 0.75 * scale),
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
	بنية browser_دري *bw = متن->bw;
	حرف *utf8_معلومات = متن->utf8_معلومات;
	ڊگهو lineno;
	صحيح x = معلومات->x;
	صحيح y = معلومات->y;
	بنانشان ڊگهو سٽ_ڳڻپ = متن->physical_سٽ_ڳڻپ;
	عائم سٽ_اوچائي = textplain_سٽ_اوچائي();
	عائم scaled_سٽ_اوچائي = سٽ_اوچائي * معلومات->scale;
	ڊگهو line0 = (clip->y0 - y * معلومات->scale) / scaled_سٽ_اوچائي - 1;
	ڊگهو line1 = (clip->y1 - y * معلومات->scale) / scaled_سٽ_اوچائي + 1;
	بنية textplain_سٽ *سٽ = متن->physical_سٽ;
	size_t ڊيگهائي;
	plot_style_t *plot_style_highlight;
	nserror res;

	إذا (line0 < 0)
		line0 = 0;
	إذا (line1 < 0)
		line1 = 0;
	إذا (سٽ_ڳڻپ < (بنانشان ڊگهو) line0)
		line0 = سٽ_ڳڻپ;
	إذا (سٽ_ڳڻپ < (بنانشان ڊگهو) line1)
		line1 = سٽ_ڳڻپ;
	إذا (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (!سٽ)
		أرجع true;

	/* choose a suitable background colour for any highlighted text */
	إذا ((معلومات->background_رنگ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وإلا
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = معلومات->background_رنگ;

	x = (x + MARGIN) * معلومات->scale;
	y = (y + MARGIN) * معلومات->scale;
	لكل (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *متن_d = utf8_معلومات + سٽ[lineno].شروعات;
		صحيح tab_ويڪر = textplain_tab_ويڪر * معلومات->scale;
		size_t وٿي = 0;
		صحيح tx = x;

		إذا (!tab_ويڪر) tab_ويڪر = 1;

		ڊيگهائي = سٽ[lineno].ڊيگهائي;
		إذا (!ڊيگهائي)
			استمر;

		طالما (وٿي < ڊيگهائي) {
			size_t ايندڙ_وٿي = وٿي;
			صحيح ويڪر;
			صحيح ntx;
			nserror res;

			طالما ((ايندڙ_وٿي < ڊيگهائي) &&
			       (متن_d[ايندڙ_وٿي] != '\t')) {
				ايندڙ_وٿي = utf8_ايندڙ(متن_d,
							ڊيگهائي,
							ايندڙ_وٿي);
			}

			إذا (!متن_draw(متن_d + وٿي,
				       ايندڙ_وٿي - وٿي,
				       سٽ[lineno].شروعات + وٿي,
				       tx,
				       y + (lineno * scaled_سٽ_اوچائي),
				       clip,
				       سٽ_اوچائي,
				       معلومات->scale,
				       متن,
				       متن->sel,
				       ctx)) {
				أرجع false;
			}

			إذا (ايندڙ_وٿي >= ڊيگهائي)
				اكسر;

			res = guit->layout->ويڪر(&textplain_style,
						  &متن_d[وٿي],
						  ايندڙ_وٿي - وٿي,
						  &ويڪر);
			/* locate end of string and align to next tab position */
			إذا (res == NSERROR_OK) {
				tx += (صحيح)(ويڪر * معلومات->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_ويڪر) * tab_ويڪر);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			إذا (bw) {
				بنانشان tab_ofst = سٽ[lineno].شروعات + ايندڙ_وٿي;
				بنية selection *sel = متن->sel;
				bool highlighted = false;

				بنانشان شروعات_idx, پڇاڙي_idx;
				إذا (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &شروعات_idx,
							  &پڇاڙي_idx)) {
					highlighted = true;
				}


				إذا (!highlighted &&
				    (c->textsearch.context != عدم)) {
					بنانشان شروعات_idx, پڇاڙي_idx;
					إذا (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &شروعات_idx,
						    &پڇاڙي_idx)) {
						highlighted = true;
					}
				}

				إذا (highlighted) {
					بنية rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_سٽ_اوچائي);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_سٽ_اوچائي;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			وٿي = ايندڙ_وٿي + 1;
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
	       بنية browser_دري *bw,
	       بنية content *صفحو,
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
textplain_coord_from_وٿي(ثابت حرف *متن, size_t وٿي, size_t ڊيگهائي)
{
	صحيح x = 0;

	طالما (وٿي > 0) {
		size_t ايندڙ_وٿي = 0;
		صحيح tx;

		طالما (ايندڙ_وٿي < وٿي && متن[ايندڙ_وٿي] != '\t') {
			ايندڙ_وٿي = utf8_ايندڙ(متن, ڊيگهائي, ايندڙ_وٿي);
		}

		guit->layout->ويڪر(&textplain_style, متن, ايندڙ_وٿي, &tx);

		x += tx;

		إذا (ايندڙ_وٿي >= وٿي)
			اكسر;

		/* align to next tab boundary */
		ايندڙ_وٿي++;
		x = (1 + (x / textplain_tab_ويڪر)) * textplain_tab_ويڪر;
		وٿي -= ايندڙ_وٿي;
		متن += ايندڙ_وٿي;
		ڊيگهائي -= ايندڙ_وٿي;
	}

	أرجع x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساكن بنانشان ڊگهو textplain_سٽ_ڳڻپ(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أكد(c != عدم);

	أرجع متن->physical_سٽ_ڳڻپ;
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
textplain_get_سٽ(بنية content *c,
		   بنانشان lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سٽ *سٽ;

	أكد(c != عدم);

	إذا (lineno >= متن->physical_سٽ_ڳڻپ)
		أرجع عدم;
	سٽ = &متن->physical_سٽ[lineno];

	*poffset = سٽ->شروعات;
	*plen = سٽ->ڊيگهائي;
	أرجع متن->utf8_معلومات + سٽ->شروعات;
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
ساكن صحيح textplain_ڳوليو_سٽ(بنية content *c, بنانشان وٿي)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سٽ *سٽ;
	صحيح nlines;
	صحيح lineno = 0;

	أكد(c != عدم);

	سٽ = متن->physical_سٽ;
	nlines = متن->physical_سٽ_ڳڻپ;

	إذا (وٿي > متن->utf8_معلومات_سائيز) {
		أرجع -1;
	}

/* \todo - implement binary search here */
	طالما (lineno < nlines && سٽ[lineno].شروعات < وٿي) {
		lineno++;
	}
	إذا (سٽ[lineno].شروعات > وٿي) {
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
textplain_textsearch_ڳوليو(بنية content *c,
			  بنية textsearch_context *context,
			  ثابت حرف *نمونو,
			  صحيح p_len,
			  bool case_sens)
{
	صحيح nlines = textplain_سٽ_ڳڻپ(c);
	صحيح سٽ;
	nserror res = NSERROR_OK;

	لكل(سٽ = 0; سٽ < nlines; سٽ++) {
		size_t وٿي, ڊيگهائي;
		ثابت حرف *متن;

		متن = textplain_get_سٽ(c, سٽ, &وٿي, &ڊيگهائي);
		إذا (متن) {
			طالما (ڊيگهائي > 0) {
				بنانشان ميلاپ_ڊيگهائي;
				size_t شروعات_idx;
				ثابت حرف *new_متن;
				ثابت حرف *pos;

				pos = content_textsearch_ڳوليو_نمونو(
						متن,
						ڊيگهائي,
						نمونو,
						p_len,
						case_sens,
						&ميلاپ_ڊيگهائي);
				إذا (!pos)
					اكسر;

				/* found string in line => add to list */
				شروعات_idx = وٿي + (pos - متن);
				res = content_textsearch_جوڙيو_ميلاپ(context,
						شروعات_idx,
						شروعات_idx + ميلاپ_ڊيگهائي,
						عدم,
						عدم);
				إذا (res != NSERROR_OK) {
					أرجع res;
				}

				new_متن = pos + ميلاپ_ڊيگهائي;
				وٿي += (new_متن - متن);
				ڊيگهائي -= (new_متن - متن);
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
			    بنانشان شروعات,
			    بنانشان پڇاڙي,
			    بنية rect *r)
{
	textplain_content *متن = (textplain_content *) c;
	عائم سٽ_اوچائي = textplain_سٽ_اوچائي();
	حرف *utf8_معلومات;
	بنية textplain_سٽ *سٽ;
	بنانشان lineno = 0;
	بنانشان nlines;

	أكد(c != عدم);
	أكد(شروعات <= پڇاڙي);
	أكد(پڇاڙي <= متن->utf8_معلومات_سائيز);

	utf8_معلومات = متن->utf8_معلومات;
	nlines = متن->physical_سٽ_ڳڻپ;
	سٽ = متن->physical_سٽ;

	/* find start */
	lineno = textplain_ڳوليو_سٽ(c, شروعات);

	r->y0 = (صحيح)(MARGIN + lineno * سٽ_اوچائي);

	إذا (lineno + 1 <= nlines || سٽ[lineno + 1].شروعات >= پڇاڙي) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_ڳوليو_سٽ(c, پڇاڙي);

		r->x0 = 0;
		r->x1 = متن->formatted_ويڪر;
	} وإلا {
		/* single line */
		ثابت حرف *متن = utf8_معلومات + سٽ[lineno].شروعات;

		r->x0 = textplain_coord_from_وٿي(متن,
						    شروعات - سٽ[lineno].شروعات,
						    سٽ[lineno].ڊيگهائي);

		r->x1 = textplain_coord_from_وٿي(متن,
						    پڇاڙي - سٽ[lineno].شروعات,
						    سٽ[lineno].ڊيگهائي);
	}

	r->y1 = (صحيح)(MARGIN + (lineno + 1) * سٽ_اوچائي);
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
		       بنانشان شروعات,
		       بنانشان پڇاڙي,
		       size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	size_t utf8_سائيز;

	أكد(c != عدم);

	utf8_سائيز = متن->utf8_معلومات_سائيز;

	/* any text at all? */
	إذا (!utf8_سائيز) أرجع عدم;

	/* clamp to valid offset range */
	إذا (شروعات >= utf8_سائيز) شروعات = utf8_سائيز;
	إذا (پڇاڙي >= utf8_سائيز) پڇاڙي = utf8_سائيز;

	*plen = پڇاڙي - شروعات;

	أرجع متن->utf8_معلومات + شروعات;
}


/**
 * get bounds of a free text search match
 */
ساكن nserror
textplain_textsearch_bounds(بنية content *c,
			    بنانشان شروعات_idx,
			    بنانشان پڇاڙي_idx,
			    بنية box *شروعات_box,
			    بنية box *پڇاڙي_box,
			    بنية rect *bounds)
{
	textplain_coords_from_range(c, شروعات_idx, پڇاڙي_idx, bounds);

	أرجع NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساكن nserror
textplain_textselection_redraw(بنية content *c,
			       بنانشان شروعات_idx,
			       بنانشان پڇاڙي_idx)
{
	بنية rect r;

	إذا (پڇاڙي_idx <= شروعات_idx) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, شروعات_idx, پڇاڙي_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	أرجع NSERROR_OK;
}

ساكن nserror
textplain_textselection_copy(بنية content *c,
			     بنانشان شروعات_idx,
			     بنانشان پڇاڙي_idx,
			     بنية selection_string *selstr)
{
	ثابت حرف *متن;
	size_t ڊيگهائي;
	bool res = false;

	متن = textplain_get_raw_معلومات(c, شروعات_idx, پڇاڙي_idx, &ڊيگهائي);
	إذا (متن != عدم) {
		res = selection_string_append(متن, ڊيگهائي, false, عدم, selstr);
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
textplain_textselection_get_پڇاڙي(بنية content *c, بنانشان *پڇاڙي_idx)
{
	textplain_content *متن = (textplain_content *)c;

	*پڇاڙي_idx = متن->utf8_معلومات_سائيز;
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
	.textsearch_ڳوليو = textplain_textsearch_ڳوليو,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_پڇاڙي = textplain_textselection_get_پڇاڙي,
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

	error = content_factory_رجسٽر_handler("text/plain",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_رجسٽر_handler("application/json",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	أرجع error;
}




