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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <parserutils/input/inputstream.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "content/handlers/css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "text/textplain.h"

ರಚನೆ textplain_ಸಾಲು {
	size_t	ಆರಂಭ;
	size_t	ಉದ್ದಳತೆ;
};

/**
 * plain text content
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ textplain_content {
	ರಚನೆ content ತಳ;

	lwc_string *encoding;
	ಶೂನ್ಯ *inputstream;
	ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ;
	size_t utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ;
	size_t utf8_ದತ್ತಾಂಶ_allocated;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ physical_ಸಾಲು_ಎಣಿಕೆ;
	ರಚನೆ textplain_ಸಾಲು *physical_ಸಾಲು;
	ಪೂರ್ಣಾಂಕ formatted_ಅಗಲ;
	ರಚನೆ browser_ಕಿಟಕಿ *bw;

	ರಚನೆ selection *sel; /** Selection state */

} textplain_content;


#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 32768 /* Must be a power of 2 */
#ವ್ಯಾಖ್ಯಾನಿಸು MARGIN 4

#ವ್ಯಾಖ್ಯಾನಿಸು TAB_ಅಗಲ 8  /* must be power of 2 currently */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪಠ್ಯ_ಗಾತ್ರ 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

ಸ್ಥಿರ plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.ಗಾತ್ರ_2 = ಪಠ್ಯ_ಗಾತ್ರ,
	.ತೂಕ = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ textplain_tab_ಅಗಲ = 256;  /* try for a sensible default */

ಸ್ಥಿರ lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
ಸ್ಥಿರ ಶೂನ್ಯ textplain_fini(ಶೂನ್ಯ)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ಖಾಲಿ;
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
ಸ್ಥಿರ parserutils_error
textplain_charset_hack(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *ಮೂಲ)
{
	ಹಿಂತಿರುಗಿ PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
ಸ್ಥಿರ nserror
textplain_create_ಆಂತರಿಕ(textplain_content *c, lwc_string *encoding)
{
	ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ;
	parserutils_inputstream *ಹರಿವು;
	parserutils_error error;

	textplain_style.ಗಾತ್ರ_2 = (nsoption_int(font_ಗಾತ್ರ) * PLOT_STYLE_SCALE) / 10;

	utf8_ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
	ಆದರೆ (utf8_ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	error = parserutils_inputstream_create(lwc_string_ದತ್ತಾಂಶ(encoding), 0,
					       textplain_charset_hack, &ಹರಿವು);
	ಆದರೆ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &ಹರಿವು);
	}
	ಆದರೆ (error != PARSERUTILS_OK) {
		ಬಿಡುಗಡೆ(utf8_ದತ್ತಾಂಶ);
		ಹೋಗು no_ಸ್ಮರಣೆ;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = ಹರಿವು;
	c->utf8_ದತ್ತಾಂಶ = utf8_ದತ್ತಾಂಶ;
	c->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ = 0;
	c->utf8_ದತ್ತಾಂಶ_allocated = CHUNK;
	c->physical_ಸಾಲು = 0;
	c->physical_ಸಾಲು_ಎಣಿಕೆ = 0;
	c->formatted_ಅಗಲ = 0;
	c->bw = ಖಾಲಿ;
	c->sel = selection_create((ರಚನೆ content *)c);

	ಹಿಂತಿರುಗಿ NSERROR_OK;

no_ಸ್ಮರಣೆ:
	content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
ಸ್ಥಿರ nserror
textplain_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		 lwc_string *imime_type,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		 llcache_handle *llcache,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		 bool quirks,
		 ರಚನೆ content **c)
{
	textplain_content *ಪಠ್ಯ;
	nserror error;
	lwc_string *encoding;

	ಪಠ್ಯ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(textplain_content));
	ಆದರೆ (ಪಠ್ಯ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	error = content__init(&ಪಠ್ಯ->ತಳ, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ಪಠ್ಯ);
		ಹಿಂತಿರುಗಿ error;
	}

	error = http_parameter_list_ಕಂಡುಹಿಡಿ_ಅಂಶ(params, corestring_lwc_charset,
					      &encoding);
	ಆದರೆ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_ಆಂತರಿಕ(ಪಠ್ಯ, encoding);
	ಆದರೆ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		ಬಿಡುಗಡೆ(ಪಠ್ಯ);
		ಹಿಂತಿರುಗಿ error;
	}

	lwc_string_unref(encoding);

	*c = (ರಚನೆ content *) ಪಠ್ಯ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
ಸ್ಥಿರ bool
textplain_copy_utf8_ದತ್ತಾಂಶ(textplain_content *c, ಸ್ಥಿರಾಂಕ uint8_t *buf, size_t len)
{
	ಆದರೆ (c->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ + len >= c->utf8_ದತ್ತಾಂಶ_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ;

		allocated = (c->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_ದತ್ತಾಂಶ = realloc(c->utf8_ದತ್ತಾಂಶ, allocated);
		ಆದರೆ (utf8_ದತ್ತಾಂಶ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		c->utf8_ದತ್ತಾಂಶ = utf8_ದತ್ತಾಂಶ;
		c->utf8_ದತ್ತಾಂಶ_allocated = allocated;
	}

	ಸ್ಮರಣೆ_ನಕಲು(c->utf8_ದತ್ತಾಂಶ + c->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ, buf, len);
	c->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ += len;

	ಹಿಂತಿರುಗಿ true;
}


/**
 * drain input
 */
ಸ್ಥಿರ bool
textplain_drain_ಒಳಹರಿವು(textplain_content *c,
		      parserutils_inputstream *ಹರಿವು,
		      parserutils_error terminator)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t *u_fffd = (ಸ್ಥಿರಾಂಕ uint8_t *) "\xef\xbf\xbd";
	ಸ್ಥಿರಾಂಕ uint8_t *ch;
	size_t chlen, ಸ್ಥಾನಾಂತರ = 0;

	ಆಗ (parserutils_inputstream_peek(ಹರಿವು, ಸ್ಥಾನಾಂತರ, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ಆದರೆ (chlen == 1 && *ch == 0) {
			ಆದರೆ (ಸ್ಥಾನಾಂತರ > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ಹರಿವು, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ಆದರೆ (textplain_copy_utf8_ದತ್ತಾಂಶ(c, ch,
							     ಸ್ಥಾನಾಂತರ) == false)
					ಹಿಂತಿರುಗಿ false;
			}

			/* Emit U+FFFD */
			ಆದರೆ (textplain_copy_utf8_ದತ್ತಾಂಶ(c, u_fffd, 3) == false)
				ಹಿಂತಿರುಗಿ false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(ಹರಿವು, ಸ್ಥಾನಾಂತರ + 1);
			/* Reset the read offset */
			ಸ್ಥಾನಾಂತರ = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Accumulate input */
			ಸ್ಥಾನಾಂತರ += chlen;

			ಆದರೆ (ಸ್ಥಾನಾಂತರ > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ಹರಿವು, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ಆದರೆ (textplain_copy_utf8_ದತ್ತಾಂಶ(c, ch,
							     ಸ್ಥಾನಾಂತರ) == false)
					ಹಿಂತಿರುಗಿ false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(ಹರಿವು, ಸ್ಥಾನಾಂತರ);
				/* Reset the read offset */
				ಸ್ಥಾನಾಂತರ = 0;
			}
		}
	}

	ಆದರೆ (ಸ್ಥಾನಾಂತರ > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(ಹರಿವು, 0, &ch, &chlen);
		/* Emit any data remaining */
		ಆದರೆ (textplain_copy_utf8_ದತ್ತಾಂಶ(c, ch, ಸ್ಥಾನಾಂತರ) == false)
			ಹಿಂತಿರುಗಿ false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(ಹರಿವು, ಸ್ಥಾನಾಂತರ);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
ಸ್ಥಿರ bool
textplain_process_ದತ್ತಾಂಶ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	parserutils_inputstream *ಹರಿವು = ಪಠ್ಯ->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ಹರಿವು,
					       (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
	ಆದರೆ (error != PARSERUTILS_OK) {
		ಹೋಗು no_ಸ್ಮರಣೆ;
	}

	ಆದರೆ (textplain_drain_ಒಳಹರಿವು(ಪಠ್ಯ, ಹರಿವು, PARSERUTILS_NEEDDATA) == false)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	ಹಿಂತಿರುಗಿ true;

no_ಸ್ಮರಣೆ:
	content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
ಸ್ಥಿರ bool textplain_convert(ರಚನೆ content *c)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	parserutils_inputstream *ಹರಿವು = ಪಠ್ಯ->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ಹರಿವು, ಖಾಲಿ, 0);
	ಆದರೆ (error != PARSERUTILS_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (textplain_drain_ಒಳಹರಿವು(ಪಠ್ಯ, ಹರಿವು, PARSERUTILS_EOF) == false)
		ಹಿಂತಿರುಗಿ false;

	parserutils_inputstream_destroy(ಹರಿವು);
	ಪಠ್ಯ->inputstream = ಖಾಲಿ;

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
	content_ಗಣ_ಸ್ಥಿತಿ(c, messages_get("Done"));

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
ಸ್ಥಿರ ದಶಮಾಂಶ textplain_ಸಾಲು_ಎತ್ತರ(ಶೂನ್ಯ)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	ಹಿಂತಿರುಗಿ FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.ಗಾತ್ರ_2 / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
ಸ್ಥಿರ ಶೂನ್ಯ textplain_reformat(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ;
	size_t utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸಾಲು_ಎಣಿಕೆ = 0;
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು = ಪಠ್ಯ->physical_ಸಾಲು;
	ರಚನೆ textplain_ಸಾಲು *line1;
	size_t i, space, ಲಂಬಸಾಲು;
	size_t columns = 80;
	ಪೂರ್ಣಾಂಕ character_ಅಗಲ;
	size_t ಸಾಲು_ಆರಂಭ;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, ಅಗಲ, ಎತ್ತರ);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->ಅಗಲ(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_ಅಗಲ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ;
	}

	columns = (ಅಗಲ - MARGIN - MARGIN) * 8 / character_ಅಗಲ;
	textplain_tab_ಅಗಲ = (TAB_ಅಗಲ * character_ಅಗಲ) / 8;

	ಪಠ್ಯ->formatted_ಅಗಲ = ಅಗಲ;

	ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ = 0;

	ಆದರೆ (!ಸಾಲು) {
		ಪಠ್ಯ->physical_ಸಾಲು = ಸಾಲು =
			ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ textplain_ಸಾಲು) * (1024 + 3));
		ಆದರೆ (!ಸಾಲು)
			ಹೋಗು no_ಸ್ಮರಣೆ;
	}

	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ++].ಆರಂಭ = ಸಾಲು_ಆರಂಭ = 0;
	space = 0;
	i = 0;
	ಲಂಬಸಾಲು = 0;
	ಆಗ (i < utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool ಪದ;
		size_t ಮುಂದಿನ_ಲಂಬಸಾಲು;
		parserutils_error perror;

		perror = parserutils_charset_utf8_ಗಮ್ಯ_ucs4((ಸ್ಥಿರಾಂಕ uint8_t *)utf8_ದತ್ತಾಂಶ + i, utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ - i, &chr, &csize);
		ಆದರೆ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		ಪದ = (chr == '\n' || chr == '\r');

		ಮುಂದಿನ_ಲಂಬಸಾಲು = ಲಂಬಸಾಲು + 1;

		ಆದರೆ (chr == '\t') {
			ಮುಂದಿನ_ಲಂಬಸಾಲು = (ಮುಂದಿನ_ಲಂಬಸಾಲು + TAB_ಅಗಲ - 1) & ~(TAB_ಅಗಲ - 1);
		}

		ಆದರೆ (ಪದ || ಮುಂದಿನ_ಲಂಬಸಾಲು >= columns) {
			ಆದರೆ (ಸಾಲು_ಎಣಿಕೆ % 1024 == 0) {
				line1 = realloc(ಸಾಲು,
						ಗಾತ್ರ(ರಚನೆ textplain_ಸಾಲು) *
						(ಸಾಲು_ಎಣಿಕೆ + 1024 + 3));
				ಆದರೆ (!line1)
					ಹೋಗು no_ಸ್ಮರಣೆ;
				ಪಠ್ಯ->physical_ಸಾಲು = ಸಾಲು = line1;
			}

			ಆದರೆ (ಪದ) {
				ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].ಉದ್ದಳತೆ = i - ಸಾಲು_ಆರಂಭ;

				/* skip second char of CR/LF or LF/CR pair */
				ಆದರೆ (i + 1 < utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ &&
				    utf8_ದತ್ತಾಂಶ[i+1] != utf8_ದತ್ತಾಂಶ[i] &&
				    (utf8_ದತ್ತಾಂಶ[i+1] == '\n' ||
				     utf8_ದತ್ತಾಂಶ[i+1] == '\r')) {
					i++;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (space) {
					/* break at last space in line */
					i = space;
					ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].ಉದ್ದಳತೆ = (i + 1) - ಸಾಲು_ಆರಂಭ;
				} ಇಲ್ಲದಿದ್ದರೆ
					ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].ಉದ್ದಳತೆ = i - ಸಾಲು_ಆರಂಭ;
			}

			ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ++].ಆರಂಭ = ಸಾಲು_ಆರಂಭ = i + 1;
			ಲಂಬಸಾಲು = 0;
			space = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಲಂಬಸಾಲು++;
			ಆದರೆ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].ಉದ್ದಳತೆ = i - ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ-1].ಆರಂಭ;
	ಸಾಲು[ಸಾಲು_ಎಣಿಕೆ].ಆರಂಭ = utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ;

	ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ = ಸಾಲು_ಎಣಿಕೆ;
	c->ಅಗಲ = ಅಗಲ;
	c->ಎತ್ತರ = ಸಾಲು_ಎಣಿಕೆ * textplain_ಸಾಲು_ಎತ್ತರ() + MARGIN + MARGIN;

	ಹಿಂತಿರುಗಿ;

no_ಸ್ಮರಣೆ:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", ಸಾಲು_ಎಣಿಕೆ);
	ಹಿಂತಿರುಗಿ;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ textplain_destroy(ರಚನೆ content *c)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	lwc_string_unref(ಪಠ್ಯ->encoding);

	ಆದರೆ (ಪಠ್ಯ->inputstream != ಖಾಲಿ) {
		parserutils_inputstream_destroy(ಪಠ್ಯ->inputstream);
	}

	ಆದರೆ (ಪಠ್ಯ->physical_ಸಾಲು != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ಪಠ್ಯ->physical_ಸಾಲು);
	}

	ಆದರೆ (ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ);
	}

	ಆದರೆ (ಪಠ್ಯ->sel != ಖಾಲಿ) {
		selection_destroy(ಪಠ್ಯ->sel);
	}
}


ಸ್ಥಿರ nserror textplain_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	ಸ್ಥಿರಾಂಕ textplain_content *old_ಪಠ್ಯ = (textplain_content *) old;
	textplain_content *ಪಠ್ಯ;
	nserror error;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;

	ಪಠ್ಯ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(textplain_content));
	ಆದರೆ (ಪಠ್ಯ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &ಪಠ್ಯ->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&ಪಠ್ಯ->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_ಆಂತರಿಕ(ಪಠ್ಯ, old_ಪಠ್ಯ->encoding);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&ಪಠ್ಯ->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(&ಪಠ್ಯ->ತಳ, &ಗಾತ್ರ_2);
	ಆದರೆ (ಗಾತ್ರ_2 > 0) {
		ಆದರೆ (textplain_process_ದತ್ತಾಂಶ(&ಪಠ್ಯ->ತಳ,
					   (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ದತ್ತಾಂಶ,
					   ಗಾತ್ರ_2) == false) {
			content_destroy(&ಪಠ್ಯ->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
	    old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (textplain_convert(&ಪಠ್ಯ->ತಳ) == false) {
			content_destroy(&ಪಠ್ಯ->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ content_type textplain_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_TEXTPLAIN;
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
ಸ್ಥಿರ size_t
textplain_ಸ್ಥಾನಾಂತರ_from_coords(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ dir)
{
	textplain_content *textc = (textplain_content *) c;
	ದಶಮಾಂಶ ಸಾಲು_ಎತ್ತರ = textplain_ಸಾಲು_ಎತ್ತರ();
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ;
	ಚಿಹ್ನೆರಹಿತ nlines;
	size_t ಉದ್ದಳತೆ;
	ಪೂರ್ಣಾಂಕ idx;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	y = (ಪೂರ್ಣಾಂಕ)((ದಶಮಾಂಶ)(y - MARGIN) / ಸಾಲು_ಎತ್ತರ);
	x -= MARGIN;

	nlines = textc->physical_ಸಾಲು_ಎಣಿಕೆ;
	ಆದರೆ (!nlines)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (y <= 0) y = 0;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)y >= nlines)
		y = nlines - 1;

	ಸಾಲು = &textc->physical_ಸಾಲು[y];
	ಪಠ್ಯ = textc->utf8_ದತ್ತಾಂಶ + ಸಾಲು->ಆರಂಭ;
	ಉದ್ದಳತೆ = ಸಾಲು->ಉದ್ದಳತೆ;
	idx = 0;

	ಆಗ (x > 0) {
		size_t ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = 0;
		ಪೂರ್ಣಾಂಕ ಅಗಲ = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;

		ಆಗ (ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ < ಉದ್ದಳತೆ && ಪಠ್ಯ[ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ] != '\t') {
			ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = utf8_ಮುಂದಿನ(ಪಠ್ಯ, ಉದ್ದಳತೆ, ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ);
		}

		ಆದರೆ (ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ < ಉದ್ದಳತೆ) {
			guit->layout->ಅಗಲ(&textplain_style,
					    ಪಠ್ಯ,
					    ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ,
					    &ಅಗಲ);
		}

		ಆದರೆ (x <= ಅಗಲ) {
			ಪೂರ್ಣಾಂಕ pixel_ಸ್ಥಾನಾಂತರ;
			size_t char_ಸ್ಥಾನಾಂತರ;

			guit->layout->position(&textplain_style,
					       ಪಠ್ಯ, ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ, x,
					       &char_ಸ್ಥಾನಾಂತರ, &pixel_ಸ್ಥಾನಾಂತರ);

			idx += char_ಸ್ಥಾನಾಂತರ;
			ನಿಲ್ಲಿಸು;
		}

		x -= ಅಗಲ;
		ಉದ್ದಳತೆ -= ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
		ಪಠ್ಯ += ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
		idx += ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;

		/* check if it's within the tab */
		ಅಗಲ = textplain_tab_ಅಗಲ - (ಅಗಲ % textplain_tab_ಅಗಲ);
		ಆದರೆ (x <= ಅಗಲ) ನಿಲ್ಲಿಸು;

		x -= ಅಗಲ;
		ಉದ್ದಳತೆ--;
		ಪಠ್ಯ++;
		idx++;
	}

	ಹಿಂತಿರುಗಿ ಸಾಲು->ಆರಂಭ + idx;
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
ಸ್ಥಿರ nserror
textplain_mouse_action(ರಚನೆ content *c,
		       ರಚನೆ browser_ಕಿಟಕಿ *bw,
		       browser_mouse_state mouse,
		       ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	browser_ಸೂಚಕ_shape ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_2 = 0;
	size_t idx;
	ಪೂರ್ಣಾಂಕ dir = 0;

	browser_ಕಿಟಕಿ_ಗಣ_drag_type(bw, DRAGGING_NONE, ಖಾಲಿ);

	idx = textplain_ಸ್ಥಾನಾಂತರ_from_coords(c, x, y, dir);
	ಆದರೆ (selection_click(ಪಠ್ಯ->sel, ಪಠ್ಯ->bw, mouse, idx)) {

		ಆದರೆ (selection_dragging(ಪಠ್ಯ->sel)) {
			browser_ಕಿಟಕಿ_ಗಣ_drag_type(bw,
						     DRAGGING_SELECTION, ಖಾಲಿ);
			ಸ್ಥಿತಿ_2 = messages_get("Selecting");
		}

	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_ಕಿಟಕಿ_ಪುಟ_drag_ಆರಂಭ(bw, x, y);
			ಸೂಚಕ = BROWSER_ಸೂಚಕ_MOVE;
		}
	}

	ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಸ್ಥಿತಿ_2;
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

	ಸಂದೇಶ_ದತ್ತಾಂಶ.ಸೂಚಕ = ಸೂಚಕ;
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸೂಚಕ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
textplain_mouse_track(ರಚನೆ content *c,
		      ರಚನೆ browser_ಕಿಟಕಿ *bw,
		      browser_mouse_state mouse,
		      ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	ಆದರೆ (browser_ಕಿಟಕಿ_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		ಪೂರ್ಣಾಂಕ dir = -1;
		size_t idx;

		ಆದರೆ (selection_dragging_ಆರಂಭ(ಪಠ್ಯ->sel))
			dir = 1;

		idx = textplain_ಸ್ಥಾನಾಂತರ_from_coords(c, x, y, dir);
		selection_track(ಪಠ್ಯ->sel, mouse, idx);

		browser_ಕಿಟಕಿ_ಗಣ_drag_type(bw, DRAGGING_NONE, ಖಾಲಿ);
	}

	ಆಯ್ಕೆ (browser_ಕಿಟಕಿ_get_drag_type(bw)) {

	ಸ್ಥಿತಿ DRAGGING_SELECTION: {
		ಪೂರ್ಣಾಂಕ dir = -1;
		size_t idx;

		ಆದರೆ (selection_dragging_ಆರಂಭ(ಪಠ್ಯ->sel)) dir = 1;

		idx = textplain_ಸ್ಥಾನಾಂತರ_from_coords(c, x, y, dir);
		selection_track(ಪಠ್ಯ->sel, mouse, idx);
	}
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
ಸ್ಥಿರ bool textplain_keypress(ರಚನೆ content *c, uint32_t ಕೀಲಿ)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ರಚನೆ selection *sel = ಪಠ್ಯ->sel;

	ಆಯ್ಕೆ (ಕೀಲಿ) {
	ಸ್ಥಿತಿ NS_ಕೀಲಿ_COPY_SELECTION:
		selection_copy_ಗಮ್ಯ_clipboard(sel);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_SELECT_ALL:
		selection_select_all(sel);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ಹಿಂತಿರುಗಿ selection_clear(sel, true);
	}

	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ bool
ಪಠ್ಯ_draw(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *utf8_ಪಠ್ಯ,
	    size_t utf8_len,
	    size_t ಸ್ಥಾನಾಂತರ,
	    ಪೂರ್ಣಾಂಕ x,
	    ಪೂರ್ಣಾಂಕ y,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
	    ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
	    ದಶಮಾಂಶ scale,
	    textplain_content *ಪಠ್ಯ,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ selection *sel,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.ಗಾತ್ರ_2 *= scale;

	/* is this box part of a selection? */
	ಆದರೆ (ctx->interactive == true) {
		ಚಿಹ್ನೆರಹಿತ len = utf8_len;
		ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx;
		ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx;

		/* first try the browser window's current selection */
		ಆದರೆ (selection_highlighted(sel,
					  ಸ್ಥಾನಾಂತರ,
					  ಸ್ಥಾನಾಂತರ + len,
					  &ಆರಂಭ_idx,
					  &ಕೊನೆ_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ಆದರೆ (!highlighted &&
		    (ಪಠ್ಯ->ತಳ.textsearch.context != ಖಾಲಿ) &&
		    content_textsearch_ishighlighted(ಪಠ್ಯ->ತಳ.textsearch.context,
						     ಸ್ಥಾನಾಂತರ,
						     ಸ್ಥಾನಾಂತರ + len,
						     &ಆರಂಭ_idx,
						     &ಕೊನೆ_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ಆದರೆ (highlighted) {
			ರಚನೆ rect r;
			ಚಿಹ್ನೆರಹಿತ endtxt_idx = ಕೊನೆ_idx;
			bool clip_ಬದಲಾಯಿತು = false;
			bool ಪಠ್ಯ_visible = true;
			ಪೂರ್ಣಾಂಕ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ಆದರೆ (ಕೊನೆ_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->ಅಗಲ(&textplain_style,
						  utf8_ಪಠ್ಯ,
						  ಆರಂಭ_idx,
						  &startx);
			ಆದರೆ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->ಅಗಲ(&textplain_style,
						  utf8_ಪಠ್ಯ,
						  endtxt_idx,
						  &endx);
			ಆದರೆ (res != NSERROR_OK) {
				endx = 0;
			}

			ಆದರೆ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ಆದರೆ (ಆರಂಭ_idx > 0) {
				res = ctx->plot->ಪಠ್ಯ(ctx,
					      &plot_fstyle,
					      x,
					      y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
					      utf8_ಪಠ್ಯ,
					      ಆರಂಭ_idx);
				ಆದರೆ (res != NSERROR_OK) {
					ಹಿಂತಿರುಗಿ false;
				}
			}

			pstyle_fill_hback.fill_ಬಣ್ಣ = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ಎತ್ತರ * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (ಆರಂಭ_idx > 0) {
				ಪೂರ್ಣಾಂಕ px0 = max(x + startx, clip->x0);
				ಪೂರ್ಣಾಂಕ px1 = min(x + endx, clip->x1);

				ಆದರೆ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}

					clip_ಬದಲಾಯಿತು = true;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಪಠ್ಯ_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_ಬಣ್ಣ;
			fstyle_hback.foreground = ಬಣ್ಣ_ಗಮ್ಯ_bw_furthest(
				pstyle_fill_hback.fill_ಬಣ್ಣ);

			ಆದರೆ (ಪಠ್ಯ_visible &&
			    (ctx->plot->ಪಠ್ಯ(ctx,
					     &fstyle_hback,
					     x,
					     y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
					     utf8_ಪಠ್ಯ,
					     endtxt_idx) != NSERROR_OK)) {
				ಹಿಂತಿರುಗಿ false;
			}

			/* draw any text succeeding highlighted portion */
			ಆದರೆ (endtxt_idx < utf8_len) {
				ಪೂರ್ಣಾಂಕ px0 = max(x + endx, clip->x0);
				ಆದರೆ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}

					clip_ಬದಲಾಯಿತು = true;

					res = ctx->plot->ಪಠ್ಯ(ctx,
							      &plot_fstyle,
							      x,
							      y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
							      utf8_ಪಠ್ಯ,
							      utf8_len);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}
				}
			}

			ಆದರೆ (clip_ಬದಲಾಯಿತು &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}

	ಆದರೆ (!highlighted) {
		res = ctx->plot->ಪಠ್ಯ(ctx,
				      &plot_fstyle,
				      x,
				      y + (ಪೂರ್ಣಾಂಕ) (ಎತ್ತರ * 0.75 * scale),
				      utf8_ಪಠ್ಯ,
				      utf8_len);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}
	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
textplain_redraw(ರಚನೆ content *c,
		 ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ರಚನೆ browser_ಕಿಟಕಿ *bw = ಪಠ್ಯ->bw;
	ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ;
	ದೀರ್ಘ lineno;
	ಪೂರ್ಣಾಂಕ x = ದತ್ತಾಂಶ->x;
	ಪೂರ್ಣಾಂಕ y = ದತ್ತಾಂಶ->y;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸಾಲು_ಎಣಿಕೆ = ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ;
	ದಶಮಾಂಶ ಸಾಲು_ಎತ್ತರ = textplain_ಸಾಲು_ಎತ್ತರ();
	ದಶಮಾಂಶ scaled_ಸಾಲು_ಎತ್ತರ = ಸಾಲು_ಎತ್ತರ * ದತ್ತಾಂಶ->scale;
	ದೀರ್ಘ line0 = (clip->y0 - y * ದತ್ತಾಂಶ->scale) / scaled_ಸಾಲು_ಎತ್ತರ - 1;
	ದೀರ್ಘ line1 = (clip->y1 - y * ದತ್ತಾಂಶ->scale) / scaled_ಸಾಲು_ಎತ್ತರ + 1;
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು = ಪಠ್ಯ->physical_ಸಾಲು;
	size_t ಉದ್ದಳತೆ;
	plot_style_t *plot_style_highlight;
	nserror res;

	ಆದರೆ (line0 < 0)
		line0 = 0;
	ಆದರೆ (line1 < 0)
		line1 = 0;
	ಆದರೆ (ಸಾಲು_ಎಣಿಕೆ < (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) line0)
		line0 = ಸಾಲು_ಎಣಿಕೆ;
	ಆದರೆ (ಸಾಲು_ಎಣಿಕೆ < (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) line1)
		line1 = ಸಾಲು_ಎಣಿಕೆ;
	ಆದರೆ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!ಸಾಲು)
		ಹಿಂತಿರುಗಿ true;

	/* choose a suitable background colour for any highlighted text */
	ಆದರೆ ((ದತ್ತಾಂಶ->background_ಬಣ್ಣ & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	ಇಲ್ಲದಿದ್ದರೆ
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = ದತ್ತಾಂಶ->background_ಬಣ್ಣ;

	x = (x + MARGIN) * ದತ್ತಾಂಶ->scale;
	y = (y + MARGIN) * ದತ್ತಾಂಶ->scale;
	ಗಾಗಿ (lineno = line0; lineno != line1; lineno++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ_d = utf8_ದತ್ತಾಂಶ + ಸಾಲು[lineno].ಆರಂಭ;
		ಪೂರ್ಣಾಂಕ tab_ಅಗಲ = textplain_tab_ಅಗಲ * ದತ್ತಾಂಶ->scale;
		size_t ಸ್ಥಾನಾಂತರ = 0;
		ಪೂರ್ಣಾಂಕ tx = x;

		ಆದರೆ (!tab_ಅಗಲ) tab_ಅಗಲ = 1;

		ಉದ್ದಳತೆ = ಸಾಲು[lineno].ಉದ್ದಳತೆ;
		ಆದರೆ (!ಉದ್ದಳತೆ)
			ಮುಂದುವರಿ;

		ಆಗ (ಸ್ಥಾನಾಂತರ < ಉದ್ದಳತೆ) {
			size_t ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;
			ಪೂರ್ಣಾಂಕ ಅಗಲ;
			ಪೂರ್ಣಾಂಕ ntx;
			nserror res;

			ಆಗ ((ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ < ಉದ್ದಳತೆ) &&
			       (ಪಠ್ಯ_d[ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ] != '\t')) {
				ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = utf8_ಮುಂದಿನ(ಪಠ್ಯ_d,
							ಉದ್ದಳತೆ,
							ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ);
			}

			ಆದರೆ (!ಪಠ್ಯ_draw(ಪಠ್ಯ_d + ಸ್ಥಾನಾಂತರ,
				       ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ - ಸ್ಥಾನಾಂತರ,
				       ಸಾಲು[lineno].ಆರಂಭ + ಸ್ಥಾನಾಂತರ,
				       tx,
				       y + (lineno * scaled_ಸಾಲು_ಎತ್ತರ),
				       clip,
				       ಸಾಲು_ಎತ್ತರ,
				       ದತ್ತಾಂಶ->scale,
				       ಪಠ್ಯ,
				       ಪಠ್ಯ->sel,
				       ctx)) {
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ >= ಉದ್ದಳತೆ)
				ನಿಲ್ಲಿಸು;

			res = guit->layout->ಅಗಲ(&textplain_style,
						  &ಪಠ್ಯ_d[ಸ್ಥಾನಾಂತರ],
						  ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ - ಸ್ಥಾನಾಂತರ,
						  &ಅಗಲ);
			/* locate end of string and align to next tab position */
			ಆದರೆ (res == NSERROR_OK) {
				tx += (ಪೂರ್ಣಾಂಕ)(ಅಗಲ * ದತ್ತಾಂಶ->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_ಅಗಲ) * tab_ಅಗಲ);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ಆದರೆ (bw) {
				ಚಿಹ್ನೆರಹಿತ tab_ofst = ಸಾಲು[lineno].ಆರಂಭ + ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
				ರಚನೆ selection *sel = ಪಠ್ಯ->sel;
				bool highlighted = false;

				ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx, ಕೊನೆ_idx;
				ಆದರೆ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &ಆರಂಭ_idx,
							  &ಕೊನೆ_idx)) {
					highlighted = true;
				}


				ಆದರೆ (!highlighted &&
				    (c->textsearch.context != ಖಾಲಿ)) {
					ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx, ಕೊನೆ_idx;
					ಆದರೆ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &ಆರಂಭ_idx,
						    &ಕೊನೆ_idx)) {
						highlighted = true;
					}
				}

				ಆದರೆ (highlighted) {
					ರಚನೆ rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_ಸಾಲು_ಎತ್ತರ);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_ಸಾಲು_ಎತ್ತರ;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}
				}
			}

			ಸ್ಥಾನಾಂತರ = ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ + 1;
			tx = ntx;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
ಸ್ಥಿರ nserror
textplain_open(ರಚನೆ content *c,
	       ರಚನೆ browser_ಕಿಟಕಿ *bw,
	       ರಚನೆ content *ಪುಟ,
	       ರಚನೆ object_params *params)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	ಪಠ್ಯ->bw = bw;

	/* text selection */
	selection_init(ಪಠ್ಯ->sel);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
ಸ್ಥಿರ nserror textplain_close(ರಚನೆ content *c)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	ಪಠ್ಯ->bw = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
ಸ್ಥಿರ ಅಕ್ಷರ *textplain_get_selection(ರಚನೆ content *c)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	ಹಿಂತಿರುಗಿ selection_get_copy(ಪಠ್ಯ->sel);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
textplain_coord_from_ಸ್ಥಾನಾಂತರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ, size_t ಸ್ಥಾನಾಂತರ, size_t ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ x = 0;

	ಆಗ (ಸ್ಥಾನಾಂತರ > 0) {
		size_t ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = 0;
		ಪೂರ್ಣಾಂಕ tx;

		ಆಗ (ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ < ಸ್ಥಾನಾಂತರ && ಪಠ್ಯ[ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ] != '\t') {
			ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ = utf8_ಮುಂದಿನ(ಪಠ್ಯ, ಉದ್ದಳತೆ, ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ);
		}

		guit->layout->ಅಗಲ(&textplain_style, ಪಠ್ಯ, ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ, &tx);

		x += tx;

		ಆದರೆ (ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ >= ಸ್ಥಾನಾಂತರ)
			ನಿಲ್ಲಿಸು;

		/* align to next tab boundary */
		ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ++;
		x = (1 + (x / textplain_tab_ಅಗಲ)) * textplain_tab_ಅಗಲ;
		ಸ್ಥಾನಾಂತರ -= ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
		ಪಠ್ಯ += ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
		ಉದ್ದಳತೆ -= ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
	}

	ಹಿಂತಿರುಗಿ x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ textplain_ಸಾಲು_ಎಣಿಕೆ(ರಚನೆ content *c)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ;
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
ಸ್ಥಿರ ಅಕ್ಷರ *
textplain_get_ಸಾಲು(ರಚನೆ content *c,
		   ಚಿಹ್ನೆರಹಿತ lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (lineno >= ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಸಾಲು = &ಪಠ್ಯ->physical_ಸಾಲು[lineno];

	*poffset = ಸಾಲು->ಆರಂಭ;
	*plen = ಸಾಲು->ಉದ್ದಳತೆ;
	ಹಿಂತಿರುಗಿ ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ + ಸಾಲು->ಆರಂಭ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ textplain_ಕಂಡುಹಿಡಿ_ಸಾಲು(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು;
	ಪೂರ್ಣಾಂಕ nlines;
	ಪೂರ್ಣಾಂಕ lineno = 0;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಸಾಲು = ಪಠ್ಯ->physical_ಸಾಲು;
	nlines = ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ;

	ಆದರೆ (ಸ್ಥಾನಾಂತರ > ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ) {
		ಹಿಂತಿರುಗಿ -1;
	}

/* \todo - implement binary search here */
	ಆಗ (lineno < nlines && ಸಾಲು[lineno].ಆರಂಭ < ಸ್ಥಾನಾಂತರ) {
		lineno++;
	}
	ಆದರೆ (ಸಾಲು[lineno].ಆರಂಭ > ಸ್ಥಾನಾಂತರ) {
		lineno--;
	}

	ಹಿಂತಿರುಗಿ lineno;
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
ಸ್ಥಿರ nserror
textplain_textsearch_ಕಂಡುಹಿಡಿ(ರಚನೆ content *c,
			  ರಚನೆ textsearch_context *context,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ,
			  ಪೂರ್ಣಾಂಕ p_len,
			  bool case_sens)
{
	ಪೂರ್ಣಾಂಕ nlines = textplain_ಸಾಲು_ಎಣಿಕೆ(c);
	ಪೂರ್ಣಾಂಕ ಸಾಲು;
	nserror res = NSERROR_OK;

	ಗಾಗಿ(ಸಾಲು = 0; ಸಾಲು < nlines; ಸಾಲು++) {
		size_t ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ;

		ಪಠ್ಯ = textplain_get_ಸಾಲು(c, ಸಾಲು, &ಸ್ಥಾನಾಂತರ, &ಉದ್ದಳತೆ);
		ಆದರೆ (ಪಠ್ಯ) {
			ಆಗ (ಉದ್ದಳತೆ > 0) {
				ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
				size_t ಆರಂಭ_idx;
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *new_ಪಠ್ಯ;
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pos;

				pos = content_textsearch_ಕಂಡುಹಿಡಿ_ನಮೂನೆ(
						ಪಠ್ಯ,
						ಉದ್ದಳತೆ,
						ನಮೂನೆ,
						p_len,
						case_sens,
						&ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ);
				ಆದರೆ (!pos)
					ನಿಲ್ಲಿಸು;

				/* found string in line => add to list */
				ಆರಂಭ_idx = ಸ್ಥಾನಾಂತರ + (pos - ಪಠ್ಯ);
				res = content_textsearch_ಕೂಡಿಸು_ಹೊಂದಿಕೆ(context,
						ಆರಂಭ_idx,
						ಆರಂಭ_idx + ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ,
						ಖಾಲಿ,
						ಖಾಲಿ);
				ಆದರೆ (res != NSERROR_OK) {
					ಹಿಂತಿರುಗಿ res;
				}

				new_ಪಠ್ಯ = pos + ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
				ಸ್ಥಾನಾಂತರ += (new_ಪಠ್ಯ - ಪಠ್ಯ);
				ಉದ್ದಳತೆ -= (new_ಪಠ್ಯ - ಪಠ್ಯ);
				ಪಠ್ಯ = new_ಪಠ್ಯ;
			}
		}
	}

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ ಶೂನ್ಯ
textplain_coords_from_range(ರಚನೆ content *c,
			    ಚಿಹ್ನೆರಹಿತ ಆರಂಭ,
			    ಚಿಹ್ನೆರಹಿತ ಕೊನೆ,
			    ರಚನೆ rect *r)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	ದಶಮಾಂಶ ಸಾಲು_ಎತ್ತರ = textplain_ಸಾಲು_ಎತ್ತರ();
	ಅಕ್ಷರ *utf8_ದತ್ತಾಂಶ;
	ರಚನೆ textplain_ಸಾಲು *ಸಾಲು;
	ಚಿಹ್ನೆರಹಿತ lineno = 0;
	ಚಿಹ್ನೆರಹಿತ nlines;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಆರಂಭ <= ಕೊನೆ);
	ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ <= ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ);

	utf8_ದತ್ತಾಂಶ = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ;
	nlines = ಪಠ್ಯ->physical_ಸಾಲು_ಎಣಿಕೆ;
	ಸಾಲು = ಪಠ್ಯ->physical_ಸಾಲು;

	/* find start */
	lineno = textplain_ಕಂಡುಹಿಡಿ_ಸಾಲು(c, ಆರಂಭ);

	r->y0 = (ಪೂರ್ಣಾಂಕ)(MARGIN + lineno * ಸಾಲು_ಎತ್ತರ);

	ಆದರೆ (lineno + 1 <= nlines || ಸಾಲು[lineno + 1].ಆರಂಭ >= ಕೊನೆ) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_ಕಂಡುಹಿಡಿ_ಸಾಲು(c, ಕೊನೆ);

		r->x0 = 0;
		r->x1 = ಪಠ್ಯ->formatted_ಅಗಲ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* single line */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ = utf8_ದತ್ತಾಂಶ + ಸಾಲು[lineno].ಆರಂಭ;

		r->x0 = textplain_coord_from_ಸ್ಥಾನಾಂತರ(ಪಠ್ಯ,
						    ಆರಂಭ - ಸಾಲು[lineno].ಆರಂಭ,
						    ಸಾಲು[lineno].ಉದ್ದಳತೆ);

		r->x1 = textplain_coord_from_ಸ್ಥಾನಾಂತರ(ಪಠ್ಯ,
						    ಕೊನೆ - ಸಾಲು[lineno].ಆರಂಭ,
						    ಸಾಲು[lineno].ಉದ್ದಳತೆ);
	}

	r->y1 = (ಪೂರ್ಣಾಂಕ)(MARGIN + (lineno + 1) * ಸಾಲು_ಎತ್ತರ);
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
ಸ್ಥಿರ ಅಕ್ಷರ *
textplain_get_raw_ದತ್ತಾಂಶ(ರಚನೆ content *c,
		       ಚಿಹ್ನೆರಹಿತ ಆರಂಭ,
		       ಚಿಹ್ನೆರಹಿತ ಕೊನೆ,
		       size_t *plen)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *) c;
	size_t utf8_ಗಾತ್ರ;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	utf8_ಗಾತ್ರ = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ;

	/* any text at all? */
	ಆದರೆ (!utf8_ಗಾತ್ರ) ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* clamp to valid offset range */
	ಆದರೆ (ಆರಂಭ >= utf8_ಗಾತ್ರ) ಆರಂಭ = utf8_ಗಾತ್ರ;
	ಆದರೆ (ಕೊನೆ >= utf8_ಗಾತ್ರ) ಕೊನೆ = utf8_ಗಾತ್ರ;

	*plen = ಕೊನೆ - ಆರಂಭ;

	ಹಿಂತಿರುಗಿ ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ + ಆರಂಭ;
}


/**
 * get bounds of a free text search match
 */
ಸ್ಥಿರ nserror
textplain_textsearch_bounds(ರಚನೆ content *c,
			    ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			    ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
			    ರಚನೆ box *ಆರಂಭ_box,
			    ರಚನೆ box *ಕೊನೆ_box,
			    ರಚನೆ rect *bounds)
{
	textplain_coords_from_range(c, ಆರಂಭ_idx, ಕೊನೆ_idx, bounds);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
ಸ್ಥಿರ nserror
textplain_textselection_redraw(ರಚನೆ content *c,
			       ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			       ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx)
{
	ರಚನೆ rect r;

	ಆದರೆ (ಕೊನೆ_idx <= ಆರಂಭ_idx) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, ಆರಂಭ_idx, ಕೊನೆ_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror
textplain_textselection_copy(ರಚನೆ content *c,
			     ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			     ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
			     ರಚನೆ selection_string *selstr)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ;
	size_t ಉದ್ದಳತೆ;
	bool res = false;

	ಪಠ್ಯ = textplain_get_raw_ದತ್ತಾಂಶ(c, ಆರಂಭ_idx, ಕೊನೆ_idx, &ಉದ್ದಳತೆ);
	ಆದರೆ (ಪಠ್ಯ != ಖಾಲಿ) {
		res = selection_string_append(ಪಠ್ಯ, ಉದ್ದಳತೆ, false, ಖಾಲಿ, selstr);
	}
	ಆದರೆ (res == false) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
ಸ್ಥಿರ nserror
textplain_textselection_get_ಕೊನೆ(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ *ಕೊನೆ_idx)
{
	textplain_content *ಪಠ್ಯ = (textplain_content *)c;

	*ಕೊನೆ_idx = ಪಠ್ಯ->utf8_ದತ್ತಾಂಶ_ಗಾತ್ರ;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * plain text content handler table
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_ದತ್ತಾಂಶ = textplain_process_ದತ್ತಾಂಶ,
	.ದತ್ತಾಂಶ_complete = textplain_convert,
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
	.textsearch_ಕಂಡುಹಿಡಿ = textplain_textsearch_ಕಂಡುಹಿಡಿ,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_ಕೊನೆ = textplain_textselection_get_ಕೊನೆ,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(ಶೂನ್ಯ)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ಆದರೆ (lerror != lwc_error_ok) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	error = content_factory_ರಿಜಿಸ್ಟರ್_handler("text/plain",
						 &textplain_content_handler);
	ಆದರೆ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_ರಿಜಿಸ್ಟರ್_handler("application/json",
						 &textplain_content_handler);
	ಆದರೆ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	ಹಿಂತಿರುಗಿ error;
}




