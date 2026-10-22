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
	size_t	درازی;
};

/**
 * plain text content
 */
تعريفنوع بنية textplain_content {
	بنية content پایه;

	lwc_string *encoding;
	فراغ *inputstream;
	حرف *utf8_داده;
	size_t utf8_داده_اندازه;
	size_t utf8_داده_allocated;
	بیعلامت دراز physical_سطر_شمار;
	بنية textplain_سطر *physical_سطر;
	صحيح formatted_عرض;
	بنية browser_پنجره *bw;

	بنية selection *sel; /** Selection state */

} textplain_content;


#تعریف_کن CHUNK 32768 /* Must be a power of 2 */
#تعریف_کن MARGIN 4

#تعریف_کن TAB_عرض 8  /* must be power of 2 currently */
#تعریف_کن متن_اندازه 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ساكن plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.اندازه = متن_اندازه,
	.وزن = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ساكن صحيح textplain_tab_عرض = 256;  /* try for a sensible default */

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
textplain_charset_hack(ثابت uint8_t *داده,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *منبع)
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
textplain_create_داخلی(textplain_content *c, lwc_string *encoding)
{
	حرف *utf8_داده;
	parserutils_inputstream *جریان;
	parserutils_error error;

	textplain_style.اندازه = (nsoption_int(font_اندازه) * PLOT_STYLE_SCALE) / 10;

	utf8_داده = حافظه_بده(CHUNK);
	إذا (utf8_داده == عدم)
		اذهب no_حافظه;

	error = parserutils_inputstream_create(lwc_string_داده(encoding), 0,
					       textplain_charset_hack, &جریان);
	إذا (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &جریان);
	}
	إذا (error != PARSERUTILS_OK) {
		حرر(utf8_داده);
		اذهب no_حافظه;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = جریان;
	c->utf8_داده = utf8_داده;
	c->utf8_داده_اندازه = 0;
	c->utf8_داده_allocated = CHUNK;
	c->physical_سطر = 0;
	c->physical_سطر_شمار = 0;
	c->formatted_عرض = 0;
	c->bw = عدم;
	c->sel = selection_create((بنية content *)c);

	أرجع NSERROR_OK;

no_حافظه:
	content_پخش_error(&c->پایه, NSERROR_NOMEM, عدم);

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

	متن = حافظه_صفر_بده(1, حجم(textplain_content));
	إذا (متن == عدم) {
		أرجع NSERROR_NOMEM;
	}

	error = content__init(&متن->پایه, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(متن);
		أرجع error;
	}

	error = http_parameter_list_پیدا_کن_قلم(params, corestring_lwc_charset,
					      &encoding);
	إذا (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_داخلی(متن, encoding);
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
textplain_copy_utf8_داده(textplain_content *c, ثابت uint8_t *buf, size_t len)
{
	إذا (c->utf8_داده_اندازه + len >= c->utf8_داده_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		حرف *utf8_داده;

		allocated = (c->utf8_داده_اندازه + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_داده = realloc(c->utf8_داده, allocated);
		إذا (utf8_داده == عدم)
			أرجع false;

		c->utf8_داده = utf8_داده;
		c->utf8_داده_allocated = allocated;
	}

	کپی_حافظه(c->utf8_داده + c->utf8_داده_اندازه, buf, len);
	c->utf8_داده_اندازه += len;

	أرجع true;
}


/**
 * drain input
 */
ساكن bool
textplain_drain_درون_داد(textplain_content *c,
		      parserutils_inputstream *جریان,
		      parserutils_error terminator)
{
	ساكن ثابت uint8_t *u_fffd = (ثابت uint8_t *) "\xef\xbf\xbd";
	ثابت uint8_t *ch;
	size_t chlen, جابه_جایی = 0;

	طالما (parserutils_inputstream_peek(جریان, جابه_جایی, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		إذا (chlen == 1 && *ch == 0) {
			إذا (جابه_جایی > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(جریان, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				إذا (textplain_copy_utf8_داده(c, ch,
							     جابه_جایی) == false)
					أرجع false;
			}

			/* Emit U+FFFD */
			إذا (textplain_copy_utf8_داده(c, u_fffd, 3) == false)
				أرجع false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(جریان, جابه_جایی + 1);
			/* Reset the read offset */
			جابه_جایی = 0;
		} وإلا {
			/* Accumulate input */
			جابه_جایی += chlen;

			إذا (جابه_جایی > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(جریان, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				إذا (textplain_copy_utf8_داده(c, ch,
							     جابه_جایی) == false)
					أرجع false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(جریان, جابه_جایی);
				/* Reset the read offset */
				جابه_جایی = 0;
			}
		}
	}

	إذا (جابه_جایی > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(جریان, 0, &ch, &chlen);
		/* Emit any data remaining */
		إذا (textplain_copy_utf8_داده(c, ch, جابه_جایی) == false)
			أرجع false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(جریان, جابه_جایی);
	}

	أرجع true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ساكن bool
textplain_process_داده(بنية content *c, ثابت حرف *داده, بیعلامت صحيح اندازه)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *جریان = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(جریان,
					       (ثابت uint8_t *) داده, اندازه);
	إذا (error != PARSERUTILS_OK) {
		اذهب no_حافظه;
	}

	إذا (textplain_drain_درون_داد(متن, جریان, PARSERUTILS_NEEDDATA) == false)
		اذهب no_حافظه;

	أرجع true;

no_حافظه:
	content_پخش_error(c, NSERROR_NOMEM, عدم);
	أرجع false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ساكن bool textplain_convert(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;
	parserutils_inputstream *جریان = متن->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(جریان, عدم, 0);
	إذا (error != PARSERUTILS_OK) {
		أرجع false;
	}

	إذا (textplain_drain_درون_داد(متن, جریان, PARSERUTILS_EOF) == false)
		أرجع false;

	parserutils_inputstream_destroy(جریان);
	متن->inputstream = عدم;

	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);
	content_مجموعه_وضعیت(c, messages_get("Done"));

	أرجع true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ساكن عائم textplain_سطر_ارتفاع(فراغ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	أرجع FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.اندازه / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ساكن فراغ textplain_reformat(بنية content *c, صحيح عرض, صحيح ارتفاع)
{
	textplain_content *متن = (textplain_content *) c;
	حرف *utf8_داده = متن->utf8_داده;
	size_t utf8_داده_اندازه = متن->utf8_داده_اندازه;
	بیعلامت دراز سطر_شمار = 0;
	بنية textplain_سطر *سطر = متن->physical_سطر;
	بنية textplain_سطر *line1;
	size_t i, space, ستون;
	size_t columns = 80;
	صحيح character_عرض;
	size_t سطر_شروع;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, عرض, ارتفاع);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->عرض(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_عرض);
	إذا (res != NSERROR_OK) {
		أرجع;
	}

	columns = (عرض - MARGIN - MARGIN) * 8 / character_عرض;
	textplain_tab_عرض = (TAB_عرض * character_عرض) / 8;

	متن->formatted_عرض = عرض;

	متن->physical_سطر_شمار = 0;

	إذا (!سطر) {
		متن->physical_سطر = سطر =
			حافظه_بده(حجم(بنية textplain_سطر) * (1024 + 3));
		إذا (!سطر)
			اذهب no_حافظه;
	}

	سطر[سطر_شمار++].شروع = سطر_شروع = 0;
	space = 0;
	i = 0;
	ستون = 0;
	طالما (i < utf8_داده_اندازه) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool جمله;
		size_t بعدی_ستون;
		parserutils_error perror;

		perror = parserutils_charset_utf8_مقصد_ucs4((ثابت uint8_t *)utf8_داده + i, utf8_داده_اندازه - i, &chr, &csize);
		إذا (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		جمله = (chr == '\n' || chr == '\r');

		بعدی_ستون = ستون + 1;

		إذا (chr == '\t') {
			بعدی_ستون = (بعدی_ستون + TAB_عرض - 1) & ~(TAB_عرض - 1);
		}

		إذا (جمله || بعدی_ستون >= columns) {
			إذا (سطر_شمار % 1024 == 0) {
				line1 = realloc(سطر,
						حجم(بنية textplain_سطر) *
						(سطر_شمار + 1024 + 3));
				إذا (!line1)
					اذهب no_حافظه;
				متن->physical_سطر = سطر = line1;
			}

			إذا (جمله) {
				سطر[سطر_شمار-1].درازی = i - سطر_شروع;

				/* skip second char of CR/LF or LF/CR pair */
				إذا (i + 1 < utf8_داده_اندازه &&
				    utf8_داده[i+1] != utf8_داده[i] &&
				    (utf8_داده[i+1] == '\n' ||
				     utf8_داده[i+1] == '\r')) {
					i++;
				}
			} وإلا {
				إذا (space) {
					/* break at last space in line */
					i = space;
					سطر[سطر_شمار-1].درازی = (i + 1) - سطر_شروع;
				} وإلا
					سطر[سطر_شمار-1].درازی = i - سطر_شروع;
			}

			سطر[سطر_شمار++].شروع = سطر_شروع = i + 1;
			ستون = 0;
			space = 0;
		} وإلا {
			ستون++;
			إذا (chr == ' ')
				space = i;
		}
		i += csize;
	}
	سطر[سطر_شمار-1].درازی = i - سطر[سطر_شمار-1].شروع;
	سطر[سطر_شمار].شروع = utf8_داده_اندازه;

	متن->physical_سطر_شمار = سطر_شمار;
	c->عرض = عرض;
	c->ارتفاع = سطر_شمار * textplain_سطر_ارتفاع() + MARGIN + MARGIN;

	أرجع;

no_حافظه:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", سطر_شمار);
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

	إذا (متن->utf8_داده != عدم) {
		حرر(متن->utf8_داده);
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
	ثابت uint8_t *داده;
	size_t اندازه;

	متن = حافظه_صفر_بده(1, حجم(textplain_content));
	إذا (متن == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &متن->پایه);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->پایه);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_داخلی(متن, old_متن->encoding);
	إذا (error != NSERROR_OK) {
		content_destroy(&متن->پایه);
		أرجع error;
	}

	داده = content_get_منبع_داده_2(&متن->پایه, &اندازه);
	إذا (اندازه > 0) {
		إذا (textplain_process_داده(&متن->پایه,
					   (ثابت حرف *)داده,
					   اندازه) == false) {
			content_destroy(&متن->پایه);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->وضعیت == CONTENT_وضعیت_READY ||
	    old->وضعیت == CONTENT_وضعیت_انجامشده) {
		إذا (textplain_convert(&متن->پایه) == false) {
			content_destroy(&متن->پایه);
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
textplain_جابه_جایی_from_coords(بنية content *c, صحيح x, صحيح y, صحيح dir)
{
	textplain_content *textc = (textplain_content *) c;
	عائم سطر_ارتفاع = textplain_سطر_ارتفاع();
	بنية textplain_سطر *سطر;
	ثابت حرف *متن;
	بیعلامت nlines;
	size_t درازی;
	صحيح idx;

	أكد(c != عدم);

	y = (صحيح)((عائم)(y - MARGIN) / سطر_ارتفاع);
	x -= MARGIN;

	nlines = textc->physical_سطر_شمار;
	إذا (!nlines)
		أرجع 0;

	إذا (y <= 0) y = 0;
	وإلا إذا ((بیعلامت)y >= nlines)
		y = nlines - 1;

	سطر = &textc->physical_سطر[y];
	متن = textc->utf8_داده + سطر->شروع;
	درازی = سطر->درازی;
	idx = 0;

	طالما (x > 0) {
		size_t بعدی_جابه_جایی = 0;
		صحيح عرض = بیشترین_صحیح;

		طالما (بعدی_جابه_جایی < درازی && متن[بعدی_جابه_جایی] != '\t') {
			بعدی_جابه_جایی = utf8_بعدی(متن, درازی, بعدی_جابه_جایی);
		}

		إذا (بعدی_جابه_جایی < درازی) {
			guit->layout->عرض(&textplain_style,
					    متن,
					    بعدی_جابه_جایی,
					    &عرض);
		}

		إذا (x <= عرض) {
			صحيح pixel_جابه_جایی;
			size_t char_جابه_جایی;

			guit->layout->position(&textplain_style,
					       متن, بعدی_جابه_جایی, x,
					       &char_جابه_جایی, &pixel_جابه_جایی);

			idx += char_جابه_جایی;
			اكسر;
		}

		x -= عرض;
		درازی -= بعدی_جابه_جایی;
		متن += بعدی_جابه_جایی;
		idx += بعدی_جابه_جایی;

		/* check if it's within the tab */
		عرض = textplain_tab_عرض - (عرض % textplain_tab_عرض);
		إذا (x <= عرض) اكسر;

		x -= عرض;
		درازی--;
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
		       بنية browser_پنجره *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;
	browser_اشاره_گر_shape اشاره_گر = BROWSER_اشاره_گر_DEFAULT;
	اتحاد content_پیام_داده پیام_داده;
	ثابت حرف *وضعیت = 0;
	size_t idx;
	صحيح dir = 0;

	browser_پنجره_مجموعه_drag_type(bw, DRAGGING_NONE, عدم);

	idx = textplain_جابه_جایی_from_coords(c, x, y, dir);
	إذا (selection_click(متن->sel, متن->bw, mouse, idx)) {

		إذا (selection_dragging(متن->sel)) {
			browser_پنجره_مجموعه_drag_type(bw,
						     DRAGGING_SELECTION, عدم);
			وضعیت = messages_get("Selecting");
		}

	} وإلا {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_پنجره_صفحه_drag_شروع(bw, x, y);
			اشاره_گر = BROWSER_اشاره_گر_MOVE;
		}
	}

	پیام_داده.explicit_وضعیت_متن = وضعیت;
	content_پخش(c, CONTENT_پیام_وضعیت, &پیام_داده);

	پیام_داده.اشاره_گر = اشاره_گر;
	content_پخش(c, CONTENT_پیام_اشاره_گر, &پیام_داده);

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
		      بنية browser_پنجره *bw,
		      browser_mouse_state mouse,
		      صحيح x, صحيح y)
{
	textplain_content *متن = (textplain_content *) c;

	إذا (browser_پنجره_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروع(متن->sel))
			dir = 1;

		idx = textplain_جابه_جایی_from_coords(c, x, y, dir);
		selection_track(متن->sel, mouse, idx);

		browser_پنجره_مجموعه_drag_type(bw, DRAGGING_NONE, عدم);
	}

	بدل (browser_پنجره_get_drag_type(bw)) {

	حالة DRAGGING_SELECTION: {
		صحيح dir = -1;
		size_t idx;

		إذا (selection_dragging_شروع(متن->sel)) dir = 1;

		idx = textplain_جابه_جایی_from_coords(c, x, y, dir);
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
		selection_copy_مقصد_clipboard(sel);
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
	    size_t جابه_جایی,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح ارتفاع,
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
		بیعلامت len = utf8_len;
		بیعلامت شروع_idx;
		بیعلامت انتها_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  جابه_جایی,
					  جابه_جایی + len,
					  &شروع_idx,
					  &انتها_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (متن->پایه.textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(متن->پایه.textsearch.context,
						     جابه_جایی,
						     جابه_جایی + len,
						     &شروع_idx,
						     &انتها_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بیعلامت endtxt_idx = انتها_idx;
			bool clip_تغییریافته = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (انتها_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				أكد(انتها_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->عرض(&textplain_style,
						  utf8_متن,
						  شروع_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->عرض(&textplain_style,
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
					      y + (صحيح)(ارتفاع * 0.75 * scale),
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
			r.y1 = y + ارتفاع * scale;
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

					clip_تغییریافته = true;
				} وإلا {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_مقصد_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			إذا (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(ارتفاع * 0.75 * scale),
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

					clip_تغییریافته = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(ارتفاع * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_تغییریافته &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (ارتفاع * 0.75 * scale),
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
		 بنية content_redraw_داده *داده,
		 ثابت بنية rect *clip,
		 ثابت بنية redraw_context *ctx)
{
	textplain_content *متن = (textplain_content *) c;
	بنية browser_پنجره *bw = متن->bw;
	حرف *utf8_داده = متن->utf8_داده;
	دراز lineno;
	صحيح x = داده->x;
	صحيح y = داده->y;
	بیعلامت دراز سطر_شمار = متن->physical_سطر_شمار;
	عائم سطر_ارتفاع = textplain_سطر_ارتفاع();
	عائم scaled_سطر_ارتفاع = سطر_ارتفاع * داده->scale;
	دراز line0 = (clip->y0 - y * داده->scale) / scaled_سطر_ارتفاع - 1;
	دراز line1 = (clip->y1 - y * داده->scale) / scaled_سطر_ارتفاع + 1;
	بنية textplain_سطر *سطر = متن->physical_سطر;
	size_t درازی;
	plot_style_t *plot_style_highlight;
	nserror res;

	إذا (line0 < 0)
		line0 = 0;
	إذا (line1 < 0)
		line1 = 0;
	إذا (سطر_شمار < (بیعلامت دراز) line0)
		line0 = سطر_شمار;
	إذا (سطر_شمار < (بیعلامت دراز) line1)
		line1 = سطر_شمار;
	إذا (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (!سطر)
		أرجع true;

	/* choose a suitable background colour for any highlighted text */
	إذا ((داده->background_رنگ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	وإلا
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = داده->background_رنگ;

	x = (x + MARGIN) * داده->scale;
	y = (y + MARGIN) * داده->scale;
	لكل (lineno = line0; lineno != line1; lineno++) {
		ثابت حرف *متن_d = utf8_داده + سطر[lineno].شروع;
		صحيح tab_عرض = textplain_tab_عرض * داده->scale;
		size_t جابه_جایی = 0;
		صحيح tx = x;

		إذا (!tab_عرض) tab_عرض = 1;

		درازی = سطر[lineno].درازی;
		إذا (!درازی)
			استمر;

		طالما (جابه_جایی < درازی) {
			size_t بعدی_جابه_جایی = جابه_جایی;
			صحيح عرض;
			صحيح ntx;
			nserror res;

			طالما ((بعدی_جابه_جایی < درازی) &&
			       (متن_d[بعدی_جابه_جایی] != '\t')) {
				بعدی_جابه_جایی = utf8_بعدی(متن_d,
							درازی,
							بعدی_جابه_جایی);
			}

			إذا (!متن_draw(متن_d + جابه_جایی,
				       بعدی_جابه_جایی - جابه_جایی,
				       سطر[lineno].شروع + جابه_جایی,
				       tx,
				       y + (lineno * scaled_سطر_ارتفاع),
				       clip,
				       سطر_ارتفاع,
				       داده->scale,
				       متن,
				       متن->sel,
				       ctx)) {
				أرجع false;
			}

			إذا (بعدی_جابه_جایی >= درازی)
				اكسر;

			res = guit->layout->عرض(&textplain_style,
						  &متن_d[جابه_جایی],
						  بعدی_جابه_جایی - جابه_جایی,
						  &عرض);
			/* locate end of string and align to next tab position */
			إذا (res == NSERROR_OK) {
				tx += (صحيح)(عرض * داده->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_عرض) * tab_عرض);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			إذا (bw) {
				بیعلامت tab_ofst = سطر[lineno].شروع + بعدی_جابه_جایی;
				بنية selection *sel = متن->sel;
				bool highlighted = false;

				بیعلامت شروع_idx, انتها_idx;
				إذا (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &شروع_idx,
							  &انتها_idx)) {
					highlighted = true;
				}


				إذا (!highlighted &&
				    (c->textsearch.context != عدم)) {
					بیعلامت شروع_idx, انتها_idx;
					إذا (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &شروع_idx,
						    &انتها_idx)) {
						highlighted = true;
					}
				}

				إذا (highlighted) {
					بنية rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_سطر_ارتفاع);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_سطر_ارتفاع;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			جابه_جایی = بعدی_جابه_جایی + 1;
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
	       بنية browser_پنجره *bw,
	       بنية content *صفحه,
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
textplain_coord_from_جابه_جایی(ثابت حرف *متن, size_t جابه_جایی, size_t درازی)
{
	صحيح x = 0;

	طالما (جابه_جایی > 0) {
		size_t بعدی_جابه_جایی = 0;
		صحيح tx;

		طالما (بعدی_جابه_جایی < جابه_جایی && متن[بعدی_جابه_جایی] != '\t') {
			بعدی_جابه_جایی = utf8_بعدی(متن, درازی, بعدی_جابه_جایی);
		}

		guit->layout->عرض(&textplain_style, متن, بعدی_جابه_جایی, &tx);

		x += tx;

		إذا (بعدی_جابه_جایی >= جابه_جایی)
			اكسر;

		/* align to next tab boundary */
		بعدی_جابه_جایی++;
		x = (1 + (x / textplain_tab_عرض)) * textplain_tab_عرض;
		جابه_جایی -= بعدی_جابه_جایی;
		متن += بعدی_جابه_جایی;
		درازی -= بعدی_جابه_جایی;
	}

	أرجع x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ساكن بیعلامت دراز textplain_سطر_شمار(بنية content *c)
{
	textplain_content *متن = (textplain_content *) c;

	أكد(c != عدم);

	أرجع متن->physical_سطر_شمار;
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
		   بیعلامت lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سطر *سطر;

	أكد(c != عدم);

	إذا (lineno >= متن->physical_سطر_شمار)
		أرجع عدم;
	سطر = &متن->physical_سطر[lineno];

	*poffset = سطر->شروع;
	*plen = سطر->درازی;
	أرجع متن->utf8_داده + سطر->شروع;
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
ساكن صحيح textplain_پیدا_کن_سطر(بنية content *c, بیعلامت جابه_جایی)
{
	textplain_content *متن = (textplain_content *) c;
	بنية textplain_سطر *سطر;
	صحيح nlines;
	صحيح lineno = 0;

	أكد(c != عدم);

	سطر = متن->physical_سطر;
	nlines = متن->physical_سطر_شمار;

	إذا (جابه_جایی > متن->utf8_داده_اندازه) {
		أرجع -1;
	}

/* \todo - implement binary search here */
	طالما (lineno < nlines && سطر[lineno].شروع < جابه_جایی) {
		lineno++;
	}
	إذا (سطر[lineno].شروع > جابه_جایی) {
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
textplain_textsearch_پیدا_کن(بنية content *c,
			  بنية textsearch_context *context,
			  ثابت حرف *الگو,
			  صحيح p_len,
			  bool case_sens)
{
	صحيح nlines = textplain_سطر_شمار(c);
	صحيح سطر;
	nserror res = NSERROR_OK;

	لكل(سطر = 0; سطر < nlines; سطر++) {
		size_t جابه_جایی, درازی;
		ثابت حرف *متن;

		متن = textplain_get_سطر(c, سطر, &جابه_جایی, &درازی);
		إذا (متن) {
			طالما (درازی > 0) {
				بیعلامت تطابق_درازی;
				size_t شروع_idx;
				ثابت حرف *new_متن;
				ثابت حرف *pos;

				pos = content_textsearch_پیدا_کن_الگو(
						متن,
						درازی,
						الگو,
						p_len,
						case_sens,
						&تطابق_درازی);
				إذا (!pos)
					اكسر;

				/* found string in line => add to list */
				شروع_idx = جابه_جایی + (pos - متن);
				res = content_textsearch_جمع_کن_تطابق(context,
						شروع_idx,
						شروع_idx + تطابق_درازی,
						عدم,
						عدم);
				إذا (res != NSERROR_OK) {
					أرجع res;
				}

				new_متن = pos + تطابق_درازی;
				جابه_جایی += (new_متن - متن);
				درازی -= (new_متن - متن);
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
			    بیعلامت شروع,
			    بیعلامت انتها,
			    بنية rect *r)
{
	textplain_content *متن = (textplain_content *) c;
	عائم سطر_ارتفاع = textplain_سطر_ارتفاع();
	حرف *utf8_داده;
	بنية textplain_سطر *سطر;
	بیعلامت lineno = 0;
	بیعلامت nlines;

	أكد(c != عدم);
	أكد(شروع <= انتها);
	أكد(انتها <= متن->utf8_داده_اندازه);

	utf8_داده = متن->utf8_داده;
	nlines = متن->physical_سطر_شمار;
	سطر = متن->physical_سطر;

	/* find start */
	lineno = textplain_پیدا_کن_سطر(c, شروع);

	r->y0 = (صحيح)(MARGIN + lineno * سطر_ارتفاع);

	إذا (lineno + 1 <= nlines || سطر[lineno + 1].شروع >= انتها) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_پیدا_کن_سطر(c, انتها);

		r->x0 = 0;
		r->x1 = متن->formatted_عرض;
	} وإلا {
		/* single line */
		ثابت حرف *متن = utf8_داده + سطر[lineno].شروع;

		r->x0 = textplain_coord_from_جابه_جایی(متن,
						    شروع - سطر[lineno].شروع,
						    سطر[lineno].درازی);

		r->x1 = textplain_coord_from_جابه_جایی(متن,
						    انتها - سطر[lineno].شروع,
						    سطر[lineno].درازی);
	}

	r->y1 = (صحيح)(MARGIN + (lineno + 1) * سطر_ارتفاع);
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
textplain_get_raw_داده(بنية content *c,
		       بیعلامت شروع,
		       بیعلامت انتها,
		       size_t *plen)
{
	textplain_content *متن = (textplain_content *) c;
	size_t utf8_اندازه;

	أكد(c != عدم);

	utf8_اندازه = متن->utf8_داده_اندازه;

	/* any text at all? */
	إذا (!utf8_اندازه) أرجع عدم;

	/* clamp to valid offset range */
	إذا (شروع >= utf8_اندازه) شروع = utf8_اندازه;
	إذا (انتها >= utf8_اندازه) انتها = utf8_اندازه;

	*plen = انتها - شروع;

	أرجع متن->utf8_داده + شروع;
}


/**
 * get bounds of a free text search match
 */
ساكن nserror
textplain_textsearch_bounds(بنية content *c,
			    بیعلامت شروع_idx,
			    بیعلامت انتها_idx,
			    بنية box *شروع_box,
			    بنية box *انتها_box,
			    بنية rect *bounds)
{
	textplain_coords_from_range(c, شروع_idx, انتها_idx, bounds);

	أرجع NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ساكن nserror
textplain_textselection_redraw(بنية content *c,
			       بیعلامت شروع_idx,
			       بیعلامت انتها_idx)
{
	بنية rect r;

	إذا (انتها_idx <= شروع_idx) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, شروع_idx, انتها_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	أرجع NSERROR_OK;
}

ساكن nserror
textplain_textselection_copy(بنية content *c,
			     بیعلامت شروع_idx,
			     بیعلامت انتها_idx,
			     بنية selection_string *selstr)
{
	ثابت حرف *متن;
	size_t درازی;
	bool res = false;

	متن = textplain_get_raw_داده(c, شروع_idx, انتها_idx, &درازی);
	إذا (متن != عدم) {
		res = selection_string_append(متن, درازی, false, عدم, selstr);
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
textplain_textselection_get_انتها(بنية content *c, بیعلامت *انتها_idx)
{
	textplain_content *متن = (textplain_content *)c;

	*انتها_idx = متن->utf8_داده_اندازه;
	أرجع NSERROR_OK;
}


/**
 * plain text content handler table
 */
ساكن ثابت content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_داده = textplain_process_داده,
	.داده_complete = textplain_convert,
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
	.textsearch_پیدا_کن = textplain_textsearch_پیدا_کن,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_انتها = textplain_textselection_get_انتها,
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

	error = content_factory_ثبات_handler("text/plain",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_ثبات_handler("application/json",
						 &textplain_content_handler);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	أرجع error;
}




