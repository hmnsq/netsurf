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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <parserutils/input/inputstream.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utf8.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/keypress.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/handlers/css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "text/textplain.h"

ਬਣਤਰ textplain_ਸਤਰ {
	size_t	ਸ਼ੁਰੂ;
	size_t	ਲੰਮਾਈ;
};

/**
 * plain text content
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ textplain_content {
	ਬਣਤਰ content ਅਧਾਰ;

	lwc_string *encoding;
	ਖਾਲੀ *inputstream;
	ਅੱਖਰ *utf8_ਅੰਕੜੇ;
	size_t utf8_ਅੰਕੜੇ_ਮਾਪ;
	size_t utf8_ਅੰਕੜੇ_allocated;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ physical_ਸਤਰ_ਗਿਣਤੀ;
	ਬਣਤਰ textplain_ਸਤਰ *physical_ਸਤਰ;
	ਪੂਰਨ formatted_ਚੌੜਾਈ;
	ਬਣਤਰ browser_ਖਿੜਕੀ *bw;

	ਬਣਤਰ selection *sel; /** Selection state */

} textplain_content;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 32768 /* Must be a power of 2 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MARGIN 4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAB_ਚੌੜਾਈ 8  /* must be power of 2 currently */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਠ_ਮਾਪ 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ਸਥਿਰ plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.ਮਾਪ = ਪਾਠ_ਮਾਪ,
	.ਭਾਰ = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ਸਥਿਰ ਪੂਰਨ textplain_tab_ਚੌੜਾਈ = 256;  /* try for a sensible default */

ਸਥਿਰ lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ਸਥਿਰ ਖਾਲੀ textplain_fini(ਖਾਲੀ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ਨਿਰਾ;
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
ਸਥਿਰ parserutils_error
textplain_charset_hack(ਅਚਲ uint8_t *ਅੰਕੜੇ,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *ਸਰੋਤ)
{
	ਵਾਪਸ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ਸਥਿਰ nserror
textplain_create_ਅੰਦਰੂਨੀ(textplain_content *c, lwc_string *encoding)
{
	ਅੱਖਰ *utf8_ਅੰਕੜੇ;
	parserutils_inputstream *ਧਾਰਾ;
	parserutils_error error;

	textplain_style.ਮਾਪ = (nsoption_int(font_ਮਾਪ) * PLOT_STYLE_SCALE) / 10;

	utf8_ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
	ਜੇ (utf8_ਅੰਕੜੇ == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	error = parserutils_inputstream_create(lwc_string_ਅੰਕੜੇ(encoding), 0,
					       textplain_charset_hack, &ਧਾਰਾ);
	ਜੇ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &ਧਾਰਾ);
	}
	ਜੇ (error != PARSERUTILS_OK) {
		ਮੁਕਤ(utf8_ਅੰਕੜੇ);
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = ਧਾਰਾ;
	c->utf8_ਅੰਕੜੇ = utf8_ਅੰਕੜੇ;
	c->utf8_ਅੰਕੜੇ_ਮਾਪ = 0;
	c->utf8_ਅੰਕੜੇ_allocated = CHUNK;
	c->physical_ਸਤਰ = 0;
	c->physical_ਸਤਰ_ਗਿਣਤੀ = 0;
	c->formatted_ਚੌੜਾਈ = 0;
	c->bw = ਨਿਰਾ;
	c->sel = selection_create((ਬਣਤਰ content *)c);

	ਵਾਪਸ NSERROR_OK;

no_ਸਮ੍ਰਿਤੀ:
	content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);

	ਵਾਪਸ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ਸਥਿਰ nserror
textplain_create(ਅਚਲ content_handler *handler,
		 lwc_string *imime_type,
		 ਅਚਲ ਬਣਤਰ http_parameter *params,
		 llcache_handle *llcache,
		 ਅਚਲ ਅੱਖਰ *fallback_charset,
		 bool quirks,
		 ਬਣਤਰ content **c)
{
	textplain_content *ਪਾਠ;
	nserror error;
	lwc_string *encoding;

	ਪਾਠ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(textplain_content));
	ਜੇ (ਪਾਠ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	error = content__init(&ਪਾਠ->ਅਧਾਰ, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਪਾਠ);
		ਵਾਪਸ error;
	}

	error = http_parameter_list_ਲੱਭੋ_ਵਸਤੂ(params, corestring_lwc_charset,
					      &encoding);
	ਜੇ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_ਅੰਦਰੂਨੀ(ਪਾਠ, encoding);
	ਜੇ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		ਮੁਕਤ(ਪਾਠ);
		ਵਾਪਸ error;
	}

	lwc_string_unref(encoding);

	*c = (ਬਣਤਰ content *) ਪਾਠ;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ਸਥਿਰ bool
textplain_copy_utf8_ਅੰਕੜੇ(textplain_content *c, ਅਚਲ uint8_t *buf, size_t len)
{
	ਜੇ (c->utf8_ਅੰਕੜੇ_ਮਾਪ + len >= c->utf8_ਅੰਕੜੇ_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		ਅੱਖਰ *utf8_ਅੰਕੜੇ;

		allocated = (c->utf8_ਅੰਕੜੇ_ਮਾਪ + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_ਅੰਕੜੇ = realloc(c->utf8_ਅੰਕੜੇ, allocated);
		ਜੇ (utf8_ਅੰਕੜੇ == ਨਿਰਾ)
			ਵਾਪਸ false;

		c->utf8_ਅੰਕੜੇ = utf8_ਅੰਕੜੇ;
		c->utf8_ਅੰਕੜੇ_allocated = allocated;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(c->utf8_ਅੰਕੜੇ + c->utf8_ਅੰਕੜੇ_ਮਾਪ, buf, len);
	c->utf8_ਅੰਕੜੇ_ਮਾਪ += len;

	ਵਾਪਸ true;
}


/**
 * drain input
 */
ਸਥਿਰ bool
textplain_drain_ਨਿਵੇਸ਼(textplain_content *c,
		      parserutils_inputstream *ਧਾਰਾ,
		      parserutils_error terminator)
{
	ਸਥਿਰ ਅਚਲ uint8_t *u_fffd = (ਅਚਲ uint8_t *) "\xef\xbf\xbd";
	ਅਚਲ uint8_t *ch;
	size_t chlen, ਵਿੱਥ = 0;

	ਜਦੋਂ (parserutils_inputstream_peek(ਧਾਰਾ, ਵਿੱਥ, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ਜੇ (chlen == 1 && *ch == 0) {
			ਜੇ (ਵਿੱਥ > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ਧਾਰਾ, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ਜੇ (textplain_copy_utf8_ਅੰਕੜੇ(c, ch,
							     ਵਿੱਥ) == false)
					ਵਾਪਸ false;
			}

			/* Emit U+FFFD */
			ਜੇ (textplain_copy_utf8_ਅੰਕੜੇ(c, u_fffd, 3) == false)
				ਵਾਪਸ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(ਧਾਰਾ, ਵਿੱਥ + 1);
			/* Reset the read offset */
			ਵਿੱਥ = 0;
		} ਨਹੀਂਤਾਂ {
			/* Accumulate input */
			ਵਿੱਥ += chlen;

			ਜੇ (ਵਿੱਥ > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ਧਾਰਾ, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ਜੇ (textplain_copy_utf8_ਅੰਕੜੇ(c, ch,
							     ਵਿੱਥ) == false)
					ਵਾਪਸ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(ਧਾਰਾ, ਵਿੱਥ);
				/* Reset the read offset */
				ਵਿੱਥ = 0;
			}
		}
	}

	ਜੇ (ਵਿੱਥ > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(ਧਾਰਾ, 0, &ch, &chlen);
		/* Emit any data remaining */
		ਜੇ (textplain_copy_utf8_ਅੰਕੜੇ(c, ch, ਵਿੱਥ) == false)
			ਵਾਪਸ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(ਧਾਰਾ, ਵਿੱਥ);
	}

	ਵਾਪਸ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ਸਥਿਰ bool
textplain_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	parserutils_inputstream *ਧਾਰਾ = ਪਾਠ->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ਧਾਰਾ,
					       (ਅਚਲ uint8_t *) ਅੰਕੜੇ, ਮਾਪ);
	ਜੇ (error != PARSERUTILS_OK) {
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}

	ਜੇ (textplain_drain_ਨਿਵੇਸ਼(ਪਾਠ, ਧਾਰਾ, PARSERUTILS_NEEDDATA) == false)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	ਵਾਪਸ true;

no_ਸਮ੍ਰਿਤੀ:
	content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
	ਵਾਪਸ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ਸਥਿਰ bool textplain_convert(ਬਣਤਰ content *c)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	parserutils_inputstream *ਧਾਰਾ = ਪਾਠ->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ਧਾਰਾ, ਨਿਰਾ, 0);
	ਜੇ (error != PARSERUTILS_OK) {
		ਵਾਪਸ false;
	}

	ਜੇ (textplain_drain_ਨਿਵੇਸ਼(ਪਾਠ, ਧਾਰਾ, PARSERUTILS_EOF) == false)
		ਵਾਪਸ false;

	parserutils_inputstream_destroy(ਧਾਰਾ);
	ਪਾਠ->inputstream = ਨਿਰਾ;

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, messages_get("Done"));

	ਵਾਪਸ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ਸਥਿਰ ਦਸ਼ਮਲਵ textplain_ਸਤਰ_ਉਚਾਈ(ਖਾਲੀ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ਵਾਪਸ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.ਮਾਪ / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ਸਥਿਰ ਖਾਲੀ textplain_reformat(ਬਣਤਰ content *c, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਅੱਖਰ *utf8_ਅੰਕੜੇ = ਪਾਠ->utf8_ਅੰਕੜੇ;
	size_t utf8_ਅੰਕੜੇ_ਮਾਪ = ਪਾਠ->utf8_ਅੰਕੜੇ_ਮਾਪ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਤਰ_ਗਿਣਤੀ = 0;
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ = ਪਾਠ->physical_ਸਤਰ;
	ਬਣਤਰ textplain_ਸਤਰ *line1;
	size_t i, space, ਖਾਨਾ;
	size_t columns = 80;
	ਪੂਰਨ character_ਚੌੜਾਈ;
	size_t ਸਤਰ_ਸ਼ੁਰੂ;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, ਚੌੜਾਈ, ਉਚਾਈ);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->ਚੌੜਾਈ(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_ਚੌੜਾਈ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ;
	}

	columns = (ਚੌੜਾਈ - MARGIN - MARGIN) * 8 / character_ਚੌੜਾਈ;
	textplain_tab_ਚੌੜਾਈ = (TAB_ਚੌੜਾਈ * character_ਚੌੜਾਈ) / 8;

	ਪਾਠ->formatted_ਚੌੜਾਈ = ਚੌੜਾਈ;

	ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ = 0;

	ਜੇ (!ਸਤਰ) {
		ਪਾਠ->physical_ਸਤਰ = ਸਤਰ =
			ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ textplain_ਸਤਰ) * (1024 + 3));
		ਜੇ (!ਸਤਰ)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}

	ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ++].ਸ਼ੁਰੂ = ਸਤਰ_ਸ਼ੁਰੂ = 0;
	space = 0;
	i = 0;
	ਖਾਨਾ = 0;
	ਜਦੋਂ (i < utf8_ਅੰਕੜੇ_ਮਾਪ) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool ਪਦ;
		size_t ਅਗਲਾ_ਖਾਨਾ;
		parserutils_error perror;

		perror = parserutils_charset_utf8_ਮੰਜ਼ਿਲ_ucs4((ਅਚਲ uint8_t *)utf8_ਅੰਕੜੇ + i, utf8_ਅੰਕੜੇ_ਮਾਪ - i, &chr, &csize);
		ਜੇ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		ਪਦ = (chr == '\n' || chr == '\r');

		ਅਗਲਾ_ਖਾਨਾ = ਖਾਨਾ + 1;

		ਜੇ (chr == '\t') {
			ਅਗਲਾ_ਖਾਨਾ = (ਅਗਲਾ_ਖਾਨਾ + TAB_ਚੌੜਾਈ - 1) & ~(TAB_ਚੌੜਾਈ - 1);
		}

		ਜੇ (ਪਦ || ਅਗਲਾ_ਖਾਨਾ >= columns) {
			ਜੇ (ਸਤਰ_ਗਿਣਤੀ % 1024 == 0) {
				line1 = realloc(ਸਤਰ,
						ਆਕਾਰ(ਬਣਤਰ textplain_ਸਤਰ) *
						(ਸਤਰ_ਗਿਣਤੀ + 1024 + 3));
				ਜੇ (!line1)
					ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
				ਪਾਠ->physical_ਸਤਰ = ਸਤਰ = line1;
			}

			ਜੇ (ਪਦ) {
				ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ-1].ਲੰਮਾਈ = i - ਸਤਰ_ਸ਼ੁਰੂ;

				/* skip second char of CR/LF or LF/CR pair */
				ਜੇ (i + 1 < utf8_ਅੰਕੜੇ_ਮਾਪ &&
				    utf8_ਅੰਕੜੇ[i+1] != utf8_ਅੰਕੜੇ[i] &&
				    (utf8_ਅੰਕੜੇ[i+1] == '\n' ||
				     utf8_ਅੰਕੜੇ[i+1] == '\r')) {
					i++;
				}
			} ਨਹੀਂਤਾਂ {
				ਜੇ (space) {
					/* break at last space in line */
					i = space;
					ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ-1].ਲੰਮਾਈ = (i + 1) - ਸਤਰ_ਸ਼ੁਰੂ;
				} ਨਹੀਂਤਾਂ
					ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ-1].ਲੰਮਾਈ = i - ਸਤਰ_ਸ਼ੁਰੂ;
			}

			ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ++].ਸ਼ੁਰੂ = ਸਤਰ_ਸ਼ੁਰੂ = i + 1;
			ਖਾਨਾ = 0;
			space = 0;
		} ਨਹੀਂਤਾਂ {
			ਖਾਨਾ++;
			ਜੇ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ-1].ਲੰਮਾਈ = i - ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ-1].ਸ਼ੁਰੂ;
	ਸਤਰ[ਸਤਰ_ਗਿਣਤੀ].ਸ਼ੁਰੂ = utf8_ਅੰਕੜੇ_ਮਾਪ;

	ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ = ਸਤਰ_ਗਿਣਤੀ;
	c->ਚੌੜਾਈ = ਚੌੜਾਈ;
	c->ਉਚਾਈ = ਸਤਰ_ਗਿਣਤੀ * textplain_ਸਤਰ_ਉਚਾਈ() + MARGIN + MARGIN;

	ਵਾਪਸ;

no_ਸਮ੍ਰਿਤੀ:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", ਸਤਰ_ਗਿਣਤੀ);
	ਵਾਪਸ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ਸਥਿਰ ਖਾਲੀ textplain_destroy(ਬਣਤਰ content *c)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	lwc_string_unref(ਪਾਠ->encoding);

	ਜੇ (ਪਾਠ->inputstream != ਨਿਰਾ) {
		parserutils_inputstream_destroy(ਪਾਠ->inputstream);
	}

	ਜੇ (ਪਾਠ->physical_ਸਤਰ != ਨਿਰਾ) {
		ਮੁਕਤ(ਪਾਠ->physical_ਸਤਰ);
	}

	ਜੇ (ਪਾਠ->utf8_ਅੰਕੜੇ != ਨਿਰਾ) {
		ਮੁਕਤ(ਪਾਠ->utf8_ਅੰਕੜੇ);
	}

	ਜੇ (ਪਾਠ->sel != ਨਿਰਾ) {
		selection_destroy(ਪਾਠ->sel);
	}
}


ਸਥਿਰ nserror textplain_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	ਅਚਲ textplain_content *old_ਪਾਠ = (textplain_content *) old;
	textplain_content *ਪਾਠ;
	nserror error;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;

	ਪਾਠ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(textplain_content));
	ਜੇ (ਪਾਠ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &ਪਾਠ->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&ਪਾਠ->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_ਅੰਦਰੂਨੀ(ਪਾਠ, old_ਪਾਠ->encoding);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&ਪਾਠ->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(&ਪਾਠ->ਅਧਾਰ, &ਮਾਪ);
	ਜੇ (ਮਾਪ > 0) {
		ਜੇ (textplain_process_ਅੰਕੜੇ(&ਪਾਠ->ਅਧਾਰ,
					   (ਅਚਲ ਅੱਖਰ *)ਅੰਕੜੇ,
					   ਮਾਪ) == false) {
			content_destroy(&ਪਾਠ->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
	    old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (textplain_convert(&ਪਾਠ->ਅਧਾਰ) == false) {
			content_destroy(&ਪਾਠ->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ content_type textplain_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_TEXTPLAIN;
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
ਸਥਿਰ size_t
textplain_ਵਿੱਥ_from_coords(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ dir)
{
	textplain_content *textc = (textplain_content *) c;
	ਦਸ਼ਮਲਵ ਸਤਰ_ਉਚਾਈ = textplain_ਸਤਰ_ਉਚਾਈ();
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ;
	ਅਚਲ ਅੱਖਰ *ਪਾਠ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nlines;
	size_t ਲੰਮਾਈ;
	ਪੂਰਨ idx;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	y = (ਪੂਰਨ)((ਦਸ਼ਮਲਵ)(y - MARGIN) / ਸਤਰ_ਉਚਾਈ);
	x -= MARGIN;

	nlines = textc->physical_ਸਤਰ_ਗਿਣਤੀ;
	ਜੇ (!nlines)
		ਵਾਪਸ 0;

	ਜੇ (y <= 0) y = 0;
	ਨਹੀਂਤਾਂ ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)y >= nlines)
		y = nlines - 1;

	ਸਤਰ = &textc->physical_ਸਤਰ[y];
	ਪਾਠ = textc->utf8_ਅੰਕੜੇ + ਸਤਰ->ਸ਼ੁਰੂ;
	ਲੰਮਾਈ = ਸਤਰ->ਲੰਮਾਈ;
	idx = 0;

	ਜਦੋਂ (x > 0) {
		size_t ਅਗਲਾ_ਵਿੱਥ = 0;
		ਪੂਰਨ ਚੌੜਾਈ = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;

		ਜਦੋਂ (ਅਗਲਾ_ਵਿੱਥ < ਲੰਮਾਈ && ਪਾਠ[ਅਗਲਾ_ਵਿੱਥ] != '\t') {
			ਅਗਲਾ_ਵਿੱਥ = utf8_ਅਗਲਾ(ਪਾਠ, ਲੰਮਾਈ, ਅਗਲਾ_ਵਿੱਥ);
		}

		ਜੇ (ਅਗਲਾ_ਵਿੱਥ < ਲੰਮਾਈ) {
			guit->layout->ਚੌੜਾਈ(&textplain_style,
					    ਪਾਠ,
					    ਅਗਲਾ_ਵਿੱਥ,
					    &ਚੌੜਾਈ);
		}

		ਜੇ (x <= ਚੌੜਾਈ) {
			ਪੂਰਨ pixel_ਵਿੱਥ;
			size_t char_ਵਿੱਥ;

			guit->layout->position(&textplain_style,
					       ਪਾਠ, ਅਗਲਾ_ਵਿੱਥ, x,
					       &char_ਵਿੱਥ, &pixel_ਵਿੱਥ);

			idx += char_ਵਿੱਥ;
			ਰੋਕੋ;
		}

		x -= ਚੌੜਾਈ;
		ਲੰਮਾਈ -= ਅਗਲਾ_ਵਿੱਥ;
		ਪਾਠ += ਅਗਲਾ_ਵਿੱਥ;
		idx += ਅਗਲਾ_ਵਿੱਥ;

		/* check if it's within the tab */
		ਚੌੜਾਈ = textplain_tab_ਚੌੜਾਈ - (ਚੌੜਾਈ % textplain_tab_ਚੌੜਾਈ);
		ਜੇ (x <= ਚੌੜਾਈ) ਰੋਕੋ;

		x -= ਚੌੜਾਈ;
		ਲੰਮਾਈ--;
		ਪਾਠ++;
		idx++;
	}

	ਵਾਪਸ ਸਤਰ->ਸ਼ੁਰੂ + idx;
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
ਸਥਿਰ nserror
textplain_mouse_action(ਬਣਤਰ content *c,
		       ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		       browser_mouse_state mouse,
		       ਪੂਰਨ x, ਪੂਰਨ y)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	browser_ਸੂਚਕ_shape ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ = 0;
	size_t idx;
	ਪੂਰਨ dir = 0;

	browser_ਖਿੜਕੀ_ਸੈੱਟ_drag_type(bw, DRAGGING_NONE, ਨਿਰਾ);

	idx = textplain_ਵਿੱਥ_from_coords(c, x, y, dir);
	ਜੇ (selection_click(ਪਾਠ->sel, ਪਾਠ->bw, mouse, idx)) {

		ਜੇ (selection_dragging(ਪਾਠ->sel)) {
			browser_ਖਿੜਕੀ_ਸੈੱਟ_drag_type(bw,
						     DRAGGING_SELECTION, ਨਿਰਾ);
			ਸਥਿਤੀ = messages_get("Selecting");
		}

	} ਨਹੀਂਤਾਂ {
		ਜੇ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_ਖਿੜਕੀ_ਪੰਨਾ_drag_ਸ਼ੁਰੂ(bw, x, y);
			ਸੂਚਕ = BROWSER_ਸੂਚਕ_MOVE;
		}
	}

	ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਸਥਿਤੀ;
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

	ਸੁਨੇਹਾ_ਅੰਕੜੇ.ਸੂਚਕ = ਸੂਚਕ;
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
textplain_mouse_track(ਬਣਤਰ content *c,
		      ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		      browser_mouse_state mouse,
		      ਪੂਰਨ x, ਪੂਰਨ y)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	ਜੇ (browser_ਖਿੜਕੀ_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		ਪੂਰਨ dir = -1;
		size_t idx;

		ਜੇ (selection_dragging_ਸ਼ੁਰੂ(ਪਾਠ->sel))
			dir = 1;

		idx = textplain_ਵਿੱਥ_from_coords(c, x, y, dir);
		selection_track(ਪਾਠ->sel, mouse, idx);

		browser_ਖਿੜਕੀ_ਸੈੱਟ_drag_type(bw, DRAGGING_NONE, ਨਿਰਾ);
	}

	ਚੋਣ (browser_ਖਿੜਕੀ_get_drag_type(bw)) {

	ਹਾਲਤ DRAGGING_SELECTION: {
		ਪੂਰਨ dir = -1;
		size_t idx;

		ਜੇ (selection_dragging_ਸ਼ੁਰੂ(ਪਾਠ->sel)) dir = 1;

		idx = textplain_ਵਿੱਥ_from_coords(c, x, y, dir);
		selection_track(ਪਾਠ->sel, mouse, idx);
	}
		ਰੋਕੋ;

	ਮੂਲ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ਰੋਕੋ;
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ਸਥਿਰ bool textplain_keypress(ਬਣਤਰ content *c, uint32_t ਕੁੰਜੀ)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਬਣਤਰ selection *sel = ਪਾਠ->sel;

	ਚੋਣ (ਕੁੰਜੀ) {
	ਹਾਲਤ NS_ਕੁੰਜੀ_COPY_SELECTION:
		selection_copy_ਮੰਜ਼ਿਲ_clipboard(sel);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_SELECT_ALL:
		selection_select_all(sel);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ਵਾਪਸ selection_clear(sel, true);
	}

	ਵਾਪਸ false;
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
ਸਥਿਰ bool
ਪਾਠ_draw(ਅਚਲ ਅੱਖਰ *utf8_ਪਾਠ,
	    size_t utf8_len,
	    size_t ਵਿੱਥ,
	    ਪੂਰਨ x,
	    ਪੂਰਨ y,
	    ਅਚਲ ਬਣਤਰ rect *clip,
	    ਪੂਰਨ ਉਚਾਈ,
	    ਦਸ਼ਮਲਵ scale,
	    textplain_content *ਪਾਠ,
	    ਅਚਲ ਬਣਤਰ selection *sel,
	    ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.ਮਾਪ *= scale;

	/* is this box part of a selection? */
	ਜੇ (ctx->interactive == true) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ len = utf8_len;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx;

		/* first try the browser window's current selection */
		ਜੇ (selection_highlighted(sel,
					  ਵਿੱਥ,
					  ਵਿੱਥ + len,
					  &ਸ਼ੁਰੂ_idx,
					  &ਸਿਰਾ_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ਜੇ (!highlighted &&
		    (ਪਾਠ->ਅਧਾਰ.textsearch.context != ਨਿਰਾ) &&
		    content_textsearch_ishighlighted(ਪਾਠ->ਅਧਾਰ.textsearch.context,
						     ਵਿੱਥ,
						     ਵਿੱਥ + len,
						     &ਸ਼ੁਰੂ_idx,
						     &ਸਿਰਾ_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ਜੇ (highlighted) {
			ਬਣਤਰ rect r;
			ਬਿਨਾਨਿਸ਼ਾਨੀ endtxt_idx = ਸਿਰਾ_idx;
			bool clip_ਬਦਲਿਆ = false;
			bool ਪਾਠ_visible = true;
			ਪੂਰਨ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ਜੇ (ਸਿਰਾ_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->ਚੌੜਾਈ(&textplain_style,
						  utf8_ਪਾਠ,
						  ਸ਼ੁਰੂ_idx,
						  &startx);
			ਜੇ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->ਚੌੜਾਈ(&textplain_style,
						  utf8_ਪਾਠ,
						  endtxt_idx,
						  &endx);
			ਜੇ (res != NSERROR_OK) {
				endx = 0;
			}

			ਜੇ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ਜੇ (ਸ਼ੁਰੂ_idx > 0) {
				res = ctx->plot->ਪਾਠ(ctx,
					      &plot_fstyle,
					      x,
					      y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
					      utf8_ਪਾਠ,
					      ਸ਼ੁਰੂ_idx);
				ਜੇ (res != NSERROR_OK) {
					ਵਾਪਸ false;
				}
			}

			pstyle_fill_hback.fill_ਰੰਗ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ਉਚਾਈ * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}

			ਜੇ (ਸ਼ੁਰੂ_idx > 0) {
				ਪੂਰਨ px0 = max(x + startx, clip->x0);
				ਪੂਰਨ px1 = min(x + endx, clip->x1);

				ਜੇ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}

					clip_ਬਦਲਿਆ = true;
				} ਨਹੀਂਤਾਂ {
					ਪਾਠ_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_ਰੰਗ;
			fstyle_hback.foreground = ਰੰਗ_ਮੰਜ਼ਿਲ_bw_furthest(
				pstyle_fill_hback.fill_ਰੰਗ);

			ਜੇ (ਪਾਠ_visible &&
			    (ctx->plot->ਪਾਠ(ctx,
					     &fstyle_hback,
					     x,
					     y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
					     utf8_ਪਾਠ,
					     endtxt_idx) != NSERROR_OK)) {
				ਵਾਪਸ false;
			}

			/* draw any text succeeding highlighted portion */
			ਜੇ (endtxt_idx < utf8_len) {
				ਪੂਰਨ px0 = max(x + endx, clip->x0);
				ਜੇ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}

					clip_ਬਦਲਿਆ = true;

					res = ctx->plot->ਪਾਠ(ctx,
							      &plot_fstyle,
							      x,
							      y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
							      utf8_ਪਾਠ,
							      utf8_len);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}
				}
			}

			ਜੇ (clip_ਬਦਲਿਆ &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ਵਾਪਸ false;
			}
		}
	}

	ਜੇ (!highlighted) {
		res = ctx->plot->ਪਾਠ(ctx,
				      &plot_fstyle,
				      x,
				      y + (ਪੂਰਨ) (ਉਚਾਈ * 0.75 * scale),
				      utf8_ਪਾਠ,
				      utf8_len);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}
	ਵਾਪਸ true;
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
ਸਥਿਰ bool
textplain_redraw(ਬਣਤਰ content *c,
		 ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		 ਅਚਲ ਬਣਤਰ rect *clip,
		 ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਬਣਤਰ browser_ਖਿੜਕੀ *bw = ਪਾਠ->bw;
	ਅੱਖਰ *utf8_ਅੰਕੜੇ = ਪਾਠ->utf8_ਅੰਕੜੇ;
	ਲੰਮਾ lineno;
	ਪੂਰਨ x = ਅੰਕੜੇ->x;
	ਪੂਰਨ y = ਅੰਕੜੇ->y;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਤਰ_ਗਿਣਤੀ = ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ;
	ਦਸ਼ਮਲਵ ਸਤਰ_ਉਚਾਈ = textplain_ਸਤਰ_ਉਚਾਈ();
	ਦਸ਼ਮਲਵ scaled_ਸਤਰ_ਉਚਾਈ = ਸਤਰ_ਉਚਾਈ * ਅੰਕੜੇ->scale;
	ਲੰਮਾ line0 = (clip->y0 - y * ਅੰਕੜੇ->scale) / scaled_ਸਤਰ_ਉਚਾਈ - 1;
	ਲੰਮਾ line1 = (clip->y1 - y * ਅੰਕੜੇ->scale) / scaled_ਸਤਰ_ਉਚਾਈ + 1;
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ = ਪਾਠ->physical_ਸਤਰ;
	size_t ਲੰਮਾਈ;
	plot_style_t *plot_style_highlight;
	nserror res;

	ਜੇ (line0 < 0)
		line0 = 0;
	ਜੇ (line1 < 0)
		line1 = 0;
	ਜੇ (ਸਤਰ_ਗਿਣਤੀ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) line0)
		line0 = ਸਤਰ_ਗਿਣਤੀ;
	ਜੇ (ਸਤਰ_ਗਿਣਤੀ < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) line1)
		line1 = ਸਤਰ_ਗਿਣਤੀ;
	ਜੇ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	ਜੇ (!ਸਤਰ)
		ਵਾਪਸ true;

	/* choose a suitable background colour for any highlighted text */
	ਜੇ ((ਅੰਕੜੇ->background_ਰੰਗ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ਨਹੀਂਤਾਂ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = ਅੰਕੜੇ->background_ਰੰਗ;

	x = (x + MARGIN) * ਅੰਕੜੇ->scale;
	y = (y + MARGIN) * ਅੰਕੜੇ->scale;
	ਲਈ (lineno = line0; lineno != line1; lineno++) {
		ਅਚਲ ਅੱਖਰ *ਪਾਠ_d = utf8_ਅੰਕੜੇ + ਸਤਰ[lineno].ਸ਼ੁਰੂ;
		ਪੂਰਨ tab_ਚੌੜਾਈ = textplain_tab_ਚੌੜਾਈ * ਅੰਕੜੇ->scale;
		size_t ਵਿੱਥ = 0;
		ਪੂਰਨ tx = x;

		ਜੇ (!tab_ਚੌੜਾਈ) tab_ਚੌੜਾਈ = 1;

		ਲੰਮਾਈ = ਸਤਰ[lineno].ਲੰਮਾਈ;
		ਜੇ (!ਲੰਮਾਈ)
			ਜਾਰੀ;

		ਜਦੋਂ (ਵਿੱਥ < ਲੰਮਾਈ) {
			size_t ਅਗਲਾ_ਵਿੱਥ = ਵਿੱਥ;
			ਪੂਰਨ ਚੌੜਾਈ;
			ਪੂਰਨ ntx;
			nserror res;

			ਜਦੋਂ ((ਅਗਲਾ_ਵਿੱਥ < ਲੰਮਾਈ) &&
			       (ਪਾਠ_d[ਅਗਲਾ_ਵਿੱਥ] != '\t')) {
				ਅਗਲਾ_ਵਿੱਥ = utf8_ਅਗਲਾ(ਪਾਠ_d,
							ਲੰਮਾਈ,
							ਅਗਲਾ_ਵਿੱਥ);
			}

			ਜੇ (!ਪਾਠ_draw(ਪਾਠ_d + ਵਿੱਥ,
				       ਅਗਲਾ_ਵਿੱਥ - ਵਿੱਥ,
				       ਸਤਰ[lineno].ਸ਼ੁਰੂ + ਵਿੱਥ,
				       tx,
				       y + (lineno * scaled_ਸਤਰ_ਉਚਾਈ),
				       clip,
				       ਸਤਰ_ਉਚਾਈ,
				       ਅੰਕੜੇ->scale,
				       ਪਾਠ,
				       ਪਾਠ->sel,
				       ctx)) {
				ਵਾਪਸ false;
			}

			ਜੇ (ਅਗਲਾ_ਵਿੱਥ >= ਲੰਮਾਈ)
				ਰੋਕੋ;

			res = guit->layout->ਚੌੜਾਈ(&textplain_style,
						  &ਪਾਠ_d[ਵਿੱਥ],
						  ਅਗਲਾ_ਵਿੱਥ - ਵਿੱਥ,
						  &ਚੌੜਾਈ);
			/* locate end of string and align to next tab position */
			ਜੇ (res == NSERROR_OK) {
				tx += (ਪੂਰਨ)(ਚੌੜਾਈ * ਅੰਕੜੇ->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_ਚੌੜਾਈ) * tab_ਚੌੜਾਈ);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ਜੇ (bw) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ tab_ofst = ਸਤਰ[lineno].ਸ਼ੁਰੂ + ਅਗਲਾ_ਵਿੱਥ;
				ਬਣਤਰ selection *sel = ਪਾਠ->sel;
				bool highlighted = false;

				ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx;
				ਜੇ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &ਸ਼ੁਰੂ_idx,
							  &ਸਿਰਾ_idx)) {
					highlighted = true;
				}


				ਜੇ (!highlighted &&
				    (c->textsearch.context != ਨਿਰਾ)) {
					ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx;
					ਜੇ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &ਸ਼ੁਰੂ_idx,
						    &ਸਿਰਾ_idx)) {
						highlighted = true;
					}
				}

				ਜੇ (highlighted) {
					ਬਣਤਰ rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_ਸਤਰ_ਉਚਾਈ);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_ਸਤਰ_ਉਚਾਈ;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}
				}
			}

			ਵਿੱਥ = ਅਗਲਾ_ਵਿੱਥ + 1;
			tx = ntx;
		}
	}

	ਵਾਪਸ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ਸਥਿਰ nserror
textplain_open(ਬਣਤਰ content *c,
	       ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
	       ਬਣਤਰ content *ਪੰਨਾ,
	       ਬਣਤਰ object_params *params)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	ਪਾਠ->bw = bw;

	/* text selection */
	selection_init(ਪਾਠ->sel);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ਸਥਿਰ nserror textplain_close(ਬਣਤਰ content *c)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	ਪਾਠ->bw = ਨਿਰਾ;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ਸਥਿਰ ਅੱਖਰ *textplain_get_selection(ਬਣਤਰ content *c)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	ਵਾਪਸ selection_get_copy(ਪਾਠ->sel);
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
ਸਥਿਰ ਪੂਰਨ
textplain_coord_from_ਵਿੱਥ(ਅਚਲ ਅੱਖਰ *ਪਾਠ, size_t ਵਿੱਥ, size_t ਲੰਮਾਈ)
{
	ਪੂਰਨ x = 0;

	ਜਦੋਂ (ਵਿੱਥ > 0) {
		size_t ਅਗਲਾ_ਵਿੱਥ = 0;
		ਪੂਰਨ tx;

		ਜਦੋਂ (ਅਗਲਾ_ਵਿੱਥ < ਵਿੱਥ && ਪਾਠ[ਅਗਲਾ_ਵਿੱਥ] != '\t') {
			ਅਗਲਾ_ਵਿੱਥ = utf8_ਅਗਲਾ(ਪਾਠ, ਲੰਮਾਈ, ਅਗਲਾ_ਵਿੱਥ);
		}

		guit->layout->ਚੌੜਾਈ(&textplain_style, ਪਾਠ, ਅਗਲਾ_ਵਿੱਥ, &tx);

		x += tx;

		ਜੇ (ਅਗਲਾ_ਵਿੱਥ >= ਵਿੱਥ)
			ਰੋਕੋ;

		/* align to next tab boundary */
		ਅਗਲਾ_ਵਿੱਥ++;
		x = (1 + (x / textplain_tab_ਚੌੜਾਈ)) * textplain_tab_ਚੌੜਾਈ;
		ਵਿੱਥ -= ਅਗਲਾ_ਵਿੱਥ;
		ਪਾਠ += ਅਗਲਾ_ਵਿੱਥ;
		ਲੰਮਾਈ -= ਅਗਲਾ_ਵਿੱਥ;
	}

	ਵਾਪਸ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ textplain_ਸਤਰ_ਗਿਣਤੀ(ਬਣਤਰ content *c)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ;
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
ਸਥਿਰ ਅੱਖਰ *
textplain_get_ਸਤਰ(ਬਣਤਰ content *c,
		   ਬਿਨਾਨਿਸ਼ਾਨੀ lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (lineno >= ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ)
		ਵਾਪਸ ਨਿਰਾ;
	ਸਤਰ = &ਪਾਠ->physical_ਸਤਰ[lineno];

	*poffset = ਸਤਰ->ਸ਼ੁਰੂ;
	*plen = ਸਤਰ->ਲੰਮਾਈ;
	ਵਾਪਸ ਪਾਠ->utf8_ਅੰਕੜੇ + ਸਤਰ->ਸ਼ੁਰੂ;
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
ਸਥਿਰ ਪੂਰਨ textplain_ਲੱਭੋ_ਸਤਰ(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ;
	ਪੂਰਨ nlines;
	ਪੂਰਨ lineno = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਸਤਰ = ਪਾਠ->physical_ਸਤਰ;
	nlines = ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ;

	ਜੇ (ਵਿੱਥ > ਪਾਠ->utf8_ਅੰਕੜੇ_ਮਾਪ) {
		ਵਾਪਸ -1;
	}

/* \todo - implement binary search here */
	ਜਦੋਂ (lineno < nlines && ਸਤਰ[lineno].ਸ਼ੁਰੂ < ਵਿੱਥ) {
		lineno++;
	}
	ਜੇ (ਸਤਰ[lineno].ਸ਼ੁਰੂ > ਵਿੱਥ) {
		lineno--;
	}

	ਵਾਪਸ lineno;
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
ਸਥਿਰ nserror
textplain_textsearch_ਲੱਭੋ(ਬਣਤਰ content *c,
			  ਬਣਤਰ textsearch_context *context,
			  ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ,
			  ਪੂਰਨ p_len,
			  bool case_sens)
{
	ਪੂਰਨ nlines = textplain_ਸਤਰ_ਗਿਣਤੀ(c);
	ਪੂਰਨ ਸਤਰ;
	nserror res = NSERROR_OK;

	ਲਈ(ਸਤਰ = 0; ਸਤਰ < nlines; ਸਤਰ++) {
		size_t ਵਿੱਥ, ਲੰਮਾਈ;
		ਅਚਲ ਅੱਖਰ *ਪਾਠ;

		ਪਾਠ = textplain_get_ਸਤਰ(c, ਸਤਰ, &ਵਿੱਥ, &ਲੰਮਾਈ);
		ਜੇ (ਪਾਠ) {
			ਜਦੋਂ (ਲੰਮਾਈ > 0) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ_ਲੰਮਾਈ;
				size_t ਸ਼ੁਰੂ_idx;
				ਅਚਲ ਅੱਖਰ *new_ਪਾਠ;
				ਅਚਲ ਅੱਖਰ *pos;

				pos = content_textsearch_ਲੱਭੋ_ਨਮੂਨਾ(
						ਪਾਠ,
						ਲੰਮਾਈ,
						ਨਮੂਨਾ,
						p_len,
						case_sens,
						&ਮੇਲ_ਲੰਮਾਈ);
				ਜੇ (!pos)
					ਰੋਕੋ;

				/* found string in line => add to list */
				ਸ਼ੁਰੂ_idx = ਵਿੱਥ + (pos - ਪਾਠ);
				res = content_textsearch_ਜੋੜੋ_ਮੇਲ(context,
						ਸ਼ੁਰੂ_idx,
						ਸ਼ੁਰੂ_idx + ਮੇਲ_ਲੰਮਾਈ,
						ਨਿਰਾ,
						ਨਿਰਾ);
				ਜੇ (res != NSERROR_OK) {
					ਵਾਪਸ res;
				}

				new_ਪਾਠ = pos + ਮੇਲ_ਲੰਮਾਈ;
				ਵਿੱਥ += (new_ਪਾਠ - ਪਾਠ);
				ਲੰਮਾਈ -= (new_ਪਾਠ - ਪਾਠ);
				ਪਾਠ = new_ਪਾਠ;
			}
		}
	}

	ਵਾਪਸ res;
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
ਸਥਿਰ ਖਾਲੀ
textplain_coords_from_range(ਬਣਤਰ content *c,
			    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ,
			    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ,
			    ਬਣਤਰ rect *r)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	ਦਸ਼ਮਲਵ ਸਤਰ_ਉਚਾਈ = textplain_ਸਤਰ_ਉਚਾਈ();
	ਅੱਖਰ *utf8_ਅੰਕੜੇ;
	ਬਣਤਰ textplain_ਸਤਰ *ਸਤਰ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ lineno = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nlines;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਸ਼ੁਰੂ <= ਸਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ <= ਪਾਠ->utf8_ਅੰਕੜੇ_ਮਾਪ);

	utf8_ਅੰਕੜੇ = ਪਾਠ->utf8_ਅੰਕੜੇ;
	nlines = ਪਾਠ->physical_ਸਤਰ_ਗਿਣਤੀ;
	ਸਤਰ = ਪਾਠ->physical_ਸਤਰ;

	/* find start */
	lineno = textplain_ਲੱਭੋ_ਸਤਰ(c, ਸ਼ੁਰੂ);

	r->y0 = (ਪੂਰਨ)(MARGIN + lineno * ਸਤਰ_ਉਚਾਈ);

	ਜੇ (lineno + 1 <= nlines || ਸਤਰ[lineno + 1].ਸ਼ੁਰੂ >= ਸਿਰਾ) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_ਲੱਭੋ_ਸਤਰ(c, ਸਿਰਾ);

		r->x0 = 0;
		r->x1 = ਪਾਠ->formatted_ਚੌੜਾਈ;
	} ਨਹੀਂਤਾਂ {
		/* single line */
		ਅਚਲ ਅੱਖਰ *ਪਾਠ = utf8_ਅੰਕੜੇ + ਸਤਰ[lineno].ਸ਼ੁਰੂ;

		r->x0 = textplain_coord_from_ਵਿੱਥ(ਪਾਠ,
						    ਸ਼ੁਰੂ - ਸਤਰ[lineno].ਸ਼ੁਰੂ,
						    ਸਤਰ[lineno].ਲੰਮਾਈ);

		r->x1 = textplain_coord_from_ਵਿੱਥ(ਪਾਠ,
						    ਸਿਰਾ - ਸਤਰ[lineno].ਸ਼ੁਰੂ,
						    ਸਤਰ[lineno].ਲੰਮਾਈ);
	}

	r->y1 = (ਪੂਰਨ)(MARGIN + (lineno + 1) * ਸਤਰ_ਉਚਾਈ);
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
ਸਥਿਰ ਅੱਖਰ *
textplain_get_raw_ਅੰਕੜੇ(ਬਣਤਰ content *c,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ,
		       size_t *plen)
{
	textplain_content *ਪਾਠ = (textplain_content *) c;
	size_t utf8_ਮਾਪ;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	utf8_ਮਾਪ = ਪਾਠ->utf8_ਅੰਕੜੇ_ਮਾਪ;

	/* any text at all? */
	ਜੇ (!utf8_ਮਾਪ) ਵਾਪਸ ਨਿਰਾ;

	/* clamp to valid offset range */
	ਜੇ (ਸ਼ੁਰੂ >= utf8_ਮਾਪ) ਸ਼ੁਰੂ = utf8_ਮਾਪ;
	ਜੇ (ਸਿਰਾ >= utf8_ਮਾਪ) ਸਿਰਾ = utf8_ਮਾਪ;

	*plen = ਸਿਰਾ - ਸ਼ੁਰੂ;

	ਵਾਪਸ ਪਾਠ->utf8_ਅੰਕੜੇ + ਸ਼ੁਰੂ;
}


/**
 * get bounds of a free text search match
 */
ਸਥਿਰ nserror
textplain_textsearch_bounds(ਬਣਤਰ content *c,
			    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
			    ਬਣਤਰ box *ਸ਼ੁਰੂ_box,
			    ਬਣਤਰ box *ਸਿਰਾ_box,
			    ਬਣਤਰ rect *bounds)
{
	textplain_coords_from_range(c, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, bounds);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ਸਥਿਰ nserror
textplain_textselection_redraw(ਬਣਤਰ content *c,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx)
{
	ਬਣਤਰ rect r;

	ਜੇ (ਸਿਰਾ_idx <= ਸ਼ੁਰੂ_idx) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror
textplain_textselection_copy(ਬਣਤਰ content *c,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
			     ਬਣਤਰ selection_string *selstr)
{
	ਅਚਲ ਅੱਖਰ *ਪਾਠ;
	size_t ਲੰਮਾਈ;
	bool res = false;

	ਪਾਠ = textplain_get_raw_ਅੰਕੜੇ(c, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, &ਲੰਮਾਈ);
	ਜੇ (ਪਾਠ != ਨਿਰਾ) {
		res = selection_string_append(ਪਾਠ, ਲੰਮਾਈ, false, ਨਿਰਾ, selstr);
	}
	ਜੇ (res == false) {
		ਵਾਪਸ NSERROR_NOMEM;
	}
	ਵਾਪਸ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ਸਥਿਰ nserror
textplain_textselection_get_ਸਿਰਾ(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸਿਰਾ_idx)
{
	textplain_content *ਪਾਠ = (textplain_content *)c;

	*ਸਿਰਾ_idx = ਪਾਠ->utf8_ਅੰਕੜੇ_ਮਾਪ;
	ਵਾਪਸ NSERROR_OK;
}


/**
 * plain text content handler table
 */
ਸਥਿਰ ਅਚਲ content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_ਅੰਕੜੇ = textplain_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = textplain_convert,
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
	.textsearch_ਲੱਭੋ = textplain_textsearch_ਲੱਭੋ,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_ਸਿਰਾ = textplain_textselection_get_ਸਿਰਾ,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(ਖਾਲੀ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ਜੇ (lerror != lwc_error_ok) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	error = content_factory_ਰਜਿਸਟਰ_handler("text/plain",
						 &textplain_content_handler);
	ਜੇ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_ਰਜਿਸਟਰ_handler("application/json",
						 &textplain_content_handler);
	ਜੇ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ਵਾਪਸ error;
}




