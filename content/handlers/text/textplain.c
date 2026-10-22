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

#అన్తర్భావ <string.h>
#అన్తర్భావ <parserutils/input/inputstream.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/utf8.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/keypress.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/layout.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "content/textsearch.h"
#అన్తర్భావ "content/handlers/css/utils.h"
#అన్తర్భావ "desktop/selection.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "text/textplain.h"

నిర్మాణం textplain_పంక్తి {
	size_t	ప్రారంభం;
	size_t	నిడివి;
};

/**
 * plain text content
 */
రకనిర్వచనం నిర్మాణం textplain_content {
	నిర్మాణం content పునాది;

	lwc_string *encoding;
	శూన్య *inputstream;
	అక్షరం *utf8_దత్తాంశం;
	size_t utf8_దత్తాంశం_కొలత;
	size_t utf8_దత్తాంశం_allocated;
	గుర్తులేని దీర్ఘ physical_పంక్తి_లెక్క;
	నిర్మాణం textplain_పంక్తి *physical_పంక్తి;
	పూర్ణాంకం formatted_వెడల్పు;
	నిర్మాణం browser_కిటికీ *bw;

	నిర్మాణం selection *sel; /** Selection state */

} textplain_content;


#నిర్వచించు CHUNK 32768 /* Must be a power of 2 */
#నిర్వచించు MARGIN 4

#నిర్వచించు TAB_వెడల్పు 8  /* must be power of 2 currently */
#నిర్వచించు పాఠ్యం_కొలత 10 * PLOT_STYLE_SCALE  /* Unscaled text size in pt */

స్థిర plot_font_style_t textplain_style = {
	.family = PLOT_FONT_FAMILY_MONOSPACE,
	.కొలత = పాఠ్యం_కొలత,
	.బరువు = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

స్థిర పూర్ణాంకం textplain_tab_వెడల్పు = 256;  /* try for a sensible default */

స్థిర lwc_string *textplain_default_charset;


/**
 * Clean up after the text content handler
 */
స్థిర శూన్య textplain_fini(శూన్య)
{
	lwc_string_unref(textplain_default_charset);
	textplain_default_charset = ఖాళీ;
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
స్థిర parserutils_error
textplain_charset_hack(స్థిరరాశి uint8_t *దత్తాంశం,
		       size_t len,
		       uint16_t *mibenum,
		       uint32_t *ఆధారం)
{
	తిరిగి PARSERUTILS_OK;
}


/**
 * setup plain text render.
 *
 * \param[in] c content object.
 * \param[in] encoding the encoding of the content.
 * \return NSERROR_OK else appropriate error code.
 */
స్థిర nserror
textplain_create_అంతర్గత(textplain_content *c, lwc_string *encoding)
{
	అక్షరం *utf8_దత్తాంశం;
	parserutils_inputstream *ప్రవాహం;
	parserutils_error error;

	textplain_style.కొలత = (nsoption_int(font_కొలత) * PLOT_STYLE_SCALE) / 10;

	utf8_దత్తాంశం = జ్ఞాపకం_ఇవ్వు(CHUNK);
	ఒకవేళ (utf8_దత్తాంశం == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;

	error = parserutils_inputstream_create(lwc_string_దత్తాంశం(encoding), 0,
					       textplain_charset_hack, &ప్రవాహం);
	ఒకవేళ (error == PARSERUTILS_BADENCODING) {
		/* Fall back to Windows-1252 */
		error = parserutils_inputstream_create("Windows-1252", 0,
					textplain_charset_hack, &ప్రవాహం);
	}
	ఒకవేళ (error != PARSERUTILS_OK) {
		విడుదల(utf8_దత్తాంశం);
		వెళ్ళు no_జ్ఞాపకం;
	}

	c->encoding = lwc_string_ref(encoding);
	c->inputstream = ప్రవాహం;
	c->utf8_దత్తాంశం = utf8_దత్తాంశం;
	c->utf8_దత్తాంశం_కొలత = 0;
	c->utf8_దత్తాంశం_allocated = CHUNK;
	c->physical_పంక్తి = 0;
	c->physical_పంక్తి_లెక్క = 0;
	c->formatted_వెడల్పు = 0;
	c->bw = ఖాళీ;
	c->sel = selection_create((నిర్మాణం content *)c);

	తిరిగి NSERROR_OK;

no_జ్ఞాపకం:
	content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);

	తిరిగి NSERROR_NOMEM;
}


/**
 * Create a CONTENT_TEXTPLAIN.
 */
స్థిర nserror
textplain_create(స్థిరరాశి content_handler *handler,
		 lwc_string *imime_type,
		 స్థిరరాశి నిర్మాణం http_parameter *params,
		 llcache_handle *llcache,
		 స్థిరరాశి అక్షరం *fallback_charset,
		 bool quirks,
		 నిర్మాణం content **c)
{
	textplain_content *పాఠ్యం;
	nserror error;
	lwc_string *encoding;

	పాఠ్యం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(textplain_content));
	ఒకవేళ (పాఠ్యం == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	error = content__init(&పాఠ్యం->పునాది, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(పాఠ్యం);
		తిరిగి error;
	}

	error = http_parameter_list_కనుగొను_అంశం(params, corestring_lwc_charset,
					      &encoding);
	ఒకవేళ (error != NSERROR_OK) {
		encoding = lwc_string_ref(textplain_default_charset);
	}

	error = textplain_create_అంతర్గత(పాఠ్యం, encoding);
	ఒకవేళ (error != NSERROR_OK) {
		lwc_string_unref(encoding);
		విడుదల(పాఠ్యం);
		తిరిగి error;
	}

	lwc_string_unref(encoding);

	*c = (నిర్మాణం content *) పాఠ్యం;

	తిరిగి NSERROR_OK;
}


/**
 * copy utf8 encoded data
 */
స్థిర bool
textplain_copy_utf8_దత్తాంశం(textplain_content *c, స్థిరరాశి uint8_t *buf, size_t len)
{
	ఒకవేళ (c->utf8_దత్తాంశం_కొలత + len >= c->utf8_దత్తాంశం_allocated) {
		/* Compute next multiple of chunk above the required space */
		size_t allocated;
		అక్షరం *utf8_దత్తాంశం;

		allocated = (c->utf8_దత్తాంశం_కొలత + len + CHUNK - 1) & ~(CHUNK - 1);
		utf8_దత్తాంశం = realloc(c->utf8_దత్తాంశం, allocated);
		ఒకవేళ (utf8_దత్తాంశం == ఖాళీ)
			తిరిగి false;

		c->utf8_దత్తాంశం = utf8_దత్తాంశం;
		c->utf8_దత్తాంశం_allocated = allocated;
	}

	జ్ఞాపకం_నకలు(c->utf8_దత్తాంశం + c->utf8_దత్తాంశం_కొలత, buf, len);
	c->utf8_దత్తాంశం_కొలత += len;

	తిరిగి true;
}


/**
 * drain input
 */
స్థిర bool
textplain_drain_ఆదానం(textplain_content *c,
		      parserutils_inputstream *ప్రవాహం,
		      parserutils_error terminator)
{
	స్థిర స్థిరరాశి uint8_t *u_fffd = (స్థిరరాశి uint8_t *) "\xef\xbf\xbd";
	స్థిరరాశి uint8_t *ch;
	size_t chlen, స్థానభ్రంశం = 0;

	అయితే (parserutils_inputstream_peek(ప్రవాహం, స్థానభ్రంశం, &ch, &chlen) !=
	       terminator) {
		/* Replace all instances of NUL with U+FFFD */
		ఒకవేళ (chlen == 1 && *ch == 0) {
			ఒకవేళ (స్థానభ్రంశం > 0) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ప్రవాహం, 0,
							     &ch, &chlen);
				/* Copy from it up to the start of the NUL */
				ఒకవేళ (textplain_copy_utf8_దత్తాంశం(c, ch,
							     స్థానభ్రంశం) == false)
					తిరిగి false;
			}

			/* Emit U+FFFD */
			ఒకవేళ (textplain_copy_utf8_దత్తాంశం(c, u_fffd, 3) == false)
				తిరిగి false;

			/* Advance inputstream past the NUL we just read */
			parserutils_inputstream_advance(ప్రవాహం, స్థానభ్రంశం + 1);
			/* Reset the read offset */
			స్థానభ్రంశం = 0;
		} లేకపోతే {
			/* Accumulate input */
			స్థానభ్రంశం += chlen;

			ఒకవేళ (స్థానభ్రంశం > CHUNK) {
				/* Obtain pointer to start of input data */
				parserutils_inputstream_peek(ప్రవాహం, 0,
							     &ch, &chlen);

				/* Emit the data we've read */
				ఒకవేళ (textplain_copy_utf8_దత్తాంశం(c, ch,
							     స్థానభ్రంశం) == false)
					తిరిగి false;

				/* Advance the inputstream */
				parserutils_inputstream_advance(ప్రవాహం, స్థానభ్రంశం);
				/* Reset the read offset */
				స్థానభ్రంశం = 0;
			}
		}
	}

	ఒకవేళ (స్థానభ్రంశం > 0) {
		/* Obtain pointer to start of input data */
		parserutils_inputstream_peek(ప్రవాహం, 0, &ch, &chlen);
		/* Emit any data remaining */
		ఒకవేళ (textplain_copy_utf8_దత్తాంశం(c, ch, స్థానభ్రంశం) == false)
			తిరిగి false;

		/* Advance the inputstream past the data we've read */
		parserutils_inputstream_advance(ప్రవాహం, స్థానభ్రంశం);
	}

	తిరిగి true;
}


/**
 * Process data for CONTENT_TEXTPLAIN.
 */
స్థిర bool
textplain_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం, గుర్తులేని పూర్ణాంకం కొలత)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	parserutils_inputstream *ప్రవాహం = పాఠ్యం->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ప్రవాహం,
					       (స్థిరరాశి uint8_t *) దత్తాంశం, కొలత);
	ఒకవేళ (error != PARSERUTILS_OK) {
		వెళ్ళు no_జ్ఞాపకం;
	}

	ఒకవేళ (textplain_drain_ఆదానం(పాఠ్యం, ప్రవాహం, PARSERUTILS_NEEDDATA) == false)
		వెళ్ళు no_జ్ఞాపకం;

	తిరిగి true;

no_జ్ఞాపకం:
	content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
	తిరిగి false;
}


/**
 * Convert a CONTENT_TEXTPLAIN for display.
 */
స్థిర bool textplain_convert(నిర్మాణం content *c)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	parserutils_inputstream *ప్రవాహం = పాఠ్యం->inputstream;
	parserutils_error error;

	error = parserutils_inputstream_append(ప్రవాహం, ఖాళీ, 0);
	ఒకవేళ (error != PARSERUTILS_OK) {
		తిరిగి false;
	}

	ఒకవేళ (textplain_drain_ఆదానం(పాఠ్యం, ప్రవాహం, PARSERUTILS_EOF) == false)
		తిరిగి false;

	parserutils_inputstream_destroy(ప్రవాహం);
	పాఠ్యం->inputstream = ఖాళీ;

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
	content_సమితి_స్థితి(c, messages_get("Done"));

	తిరిగి true;
}


/**
 * Calculate the line height, in pixels
 *
 * \return Line height, in pixels
 */
స్థిర దశాంశం textplain_పంక్తి_ఎత్తు(శూన్య)
{
	/* Size is in points, so convert to pixels.
	 * Then use a constant line height of 1.2 x font size.
	 */
	తిరిగి FIXTOFLT(FDIV((FMUL(FLTTOFIX(1.2), FMUL(nscss_screen_dpi, INTTOFIX((textplain_style.కొలత / PLOT_STYLE_SCALE))))), F_72));
}


/**
 * Reformat a CONTENT_TEXTPLAIN to a new width.
 */
స్థిర శూన్య textplain_reformat(నిర్మాణం content *c, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	అక్షరం *utf8_దత్తాంశం = పాఠ్యం->utf8_దత్తాంశం;
	size_t utf8_దత్తాంశం_కొలత = పాఠ్యం->utf8_దత్తాంశం_కొలత;
	గుర్తులేని దీర్ఘ పంక్తి_లెక్క = 0;
	నిర్మాణం textplain_పంక్తి *పంక్తి = పాఠ్యం->physical_పంక్తి;
	నిర్మాణం textplain_పంక్తి *line1;
	size_t i, space, నిలువు;
	size_t columns = 80;
	పూర్ణాంకం character_వెడల్పు;
	size_t పంక్తి_ప్రారంభం;
	nserror res;

	NSLOG(netsurf, INFO, "content %p w:%d h:%d", c, వెడల్పు, ఎత్తు);

	/* compute available columns (assuming monospaced font) - use 8
	 * characters for better accuracy
	 */
	res = guit->layout->వెడల్పు(&textplain_style,
				  "ABCDEFGH", 8,
				  &character_వెడల్పు);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి;
	}

	columns = (వెడల్పు - MARGIN - MARGIN) * 8 / character_వెడల్పు;
	textplain_tab_వెడల్పు = (TAB_వెడల్పు * character_వెడల్పు) / 8;

	పాఠ్యం->formatted_వెడల్పు = వెడల్పు;

	పాఠ్యం->physical_పంక్తి_లెక్క = 0;

	ఒకవేళ (!పంక్తి) {
		పాఠ్యం->physical_పంక్తి = పంక్తి =
			జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం textplain_పంక్తి) * (1024 + 3));
		ఒకవేళ (!పంక్తి)
			వెళ్ళు no_జ్ఞాపకం;
	}

	పంక్తి[పంక్తి_లెక్క++].ప్రారంభం = పంక్తి_ప్రారంభం = 0;
	space = 0;
	i = 0;
	నిలువు = 0;
	అయితే (i < utf8_దత్తాంశం_కొలత) {
		size_t csize; /* number of bytes in character */
		uint32_t chr;
		bool పదం;
		size_t తదుపరి_నిలువు;
		parserutils_error perror;

		perror = parserutils_charset_utf8_గమ్యం_ucs4((స్థిరరాశి uint8_t *)utf8_దత్తాంశం + i, utf8_దత్తాంశం_కొలత - i, &chr, &csize);
		ఒకవేళ (perror != PARSERUTILS_OK) {
			chr = 0xfffd;
		}

		పదం = (chr == '\n' || chr == '\r');

		తదుపరి_నిలువు = నిలువు + 1;

		ఒకవేళ (chr == '\t') {
			తదుపరి_నిలువు = (తదుపరి_నిలువు + TAB_వెడల్పు - 1) & ~(TAB_వెడల్పు - 1);
		}

		ఒకవేళ (పదం || తదుపరి_నిలువు >= columns) {
			ఒకవేళ (పంక్తి_లెక్క % 1024 == 0) {
				line1 = realloc(పంక్తి,
						పరిమాణం(నిర్మాణం textplain_పంక్తి) *
						(పంక్తి_లెక్క + 1024 + 3));
				ఒకవేళ (!line1)
					వెళ్ళు no_జ్ఞాపకం;
				పాఠ్యం->physical_పంక్తి = పంక్తి = line1;
			}

			ఒకవేళ (పదం) {
				పంక్తి[పంక్తి_లెక్క-1].నిడివి = i - పంక్తి_ప్రారంభం;

				/* skip second char of CR/LF or LF/CR pair */
				ఒకవేళ (i + 1 < utf8_దత్తాంశం_కొలత &&
				    utf8_దత్తాంశం[i+1] != utf8_దత్తాంశం[i] &&
				    (utf8_దత్తాంశం[i+1] == '\n' ||
				     utf8_దత్తాంశం[i+1] == '\r')) {
					i++;
				}
			} లేకపోతే {
				ఒకవేళ (space) {
					/* break at last space in line */
					i = space;
					పంక్తి[పంక్తి_లెక్క-1].నిడివి = (i + 1) - పంక్తి_ప్రారంభం;
				} లేకపోతే
					పంక్తి[పంక్తి_లెక్క-1].నిడివి = i - పంక్తి_ప్రారంభం;
			}

			పంక్తి[పంక్తి_లెక్క++].ప్రారంభం = పంక్తి_ప్రారంభం = i + 1;
			నిలువు = 0;
			space = 0;
		} లేకపోతే {
			నిలువు++;
			ఒకవేళ (chr == ' ')
				space = i;
		}
		i += csize;
	}
	పంక్తి[పంక్తి_లెక్క-1].నిడివి = i - పంక్తి[పంక్తి_లెక్క-1].ప్రారంభం;
	పంక్తి[పంక్తి_లెక్క].ప్రారంభం = utf8_దత్తాంశం_కొలత;

	పాఠ్యం->physical_పంక్తి_లెక్క = పంక్తి_లెక్క;
	c->వెడల్పు = వెడల్పు;
	c->ఎత్తు = పంక్తి_లెక్క * textplain_పంక్తి_ఎత్తు() + MARGIN + MARGIN;

	తిరిగి;

no_జ్ఞాపకం:
	NSLOG(netsurf, INFO, "out of memory (line_count %lu)", పంక్తి_లెక్క);
	తిరిగి;
}


/**
 * Destroy a CONTENT_TEXTPLAIN and free all resources it owns.
 */

స్థిర శూన్య textplain_destroy(నిర్మాణం content *c)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	lwc_string_unref(పాఠ్యం->encoding);

	ఒకవేళ (పాఠ్యం->inputstream != ఖాళీ) {
		parserutils_inputstream_destroy(పాఠ్యం->inputstream);
	}

	ఒకవేళ (పాఠ్యం->physical_పంక్తి != ఖాళీ) {
		విడుదల(పాఠ్యం->physical_పంక్తి);
	}

	ఒకవేళ (పాఠ్యం->utf8_దత్తాంశం != ఖాళీ) {
		విడుదల(పాఠ్యం->utf8_దత్తాంశం);
	}

	ఒకవేళ (పాఠ్యం->sel != ఖాళీ) {
		selection_destroy(పాఠ్యం->sel);
	}
}


స్థిర nserror textplain_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	స్థిరరాశి textplain_content *old_పాఠ్యం = (textplain_content *) old;
	textplain_content *పాఠ్యం;
	nserror error;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;

	పాఠ్యం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(textplain_content));
	ఒకవేళ (పాఠ్యం == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &పాఠ్యం->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&పాఠ్యం->పునాది);
		తిరిగి error;
	}

	/* Simply replay create/process/convert */
	error = textplain_create_అంతర్గత(పాఠ్యం, old_పాఠ్యం->encoding);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&పాఠ్యం->పునాది);
		తిరిగి error;
	}

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(&పాఠ్యం->పునాది, &కొలత);
	ఒకవేళ (కొలత > 0) {
		ఒకవేళ (textplain_process_దత్తాంశం(&పాఠ్యం->పునాది,
					   (స్థిరరాశి అక్షరం *)దత్తాంశం,
					   కొలత) == false) {
			content_destroy(&పాఠ్యం->పునాది);
			తిరిగి NSERROR_NOMEM;
		}
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
	    old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (textplain_convert(&పాఠ్యం->పునాది) == false) {
			content_destroy(&పాఠ్యం->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	తిరిగి NSERROR_OK;
}


స్థిర content_type textplain_content_type(శూన్య)
{
	తిరిగి CONTENT_TEXTPLAIN;
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
స్థిర size_t
textplain_స్థానభ్రంశం_from_coords(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం dir)
{
	textplain_content *textc = (textplain_content *) c;
	దశాంశం పంక్తి_ఎత్తు = textplain_పంక్తి_ఎత్తు();
	నిర్మాణం textplain_పంక్తి *పంక్తి;
	స్థిరరాశి అక్షరం *పాఠ్యం;
	గుర్తులేని nlines;
	size_t నిడివి;
	పూర్ణాంకం idx;

	ప్రతిజ్ఞా(c != ఖాళీ);

	y = (పూర్ణాంకం)((దశాంశం)(y - MARGIN) / పంక్తి_ఎత్తు);
	x -= MARGIN;

	nlines = textc->physical_పంక్తి_లెక్క;
	ఒకవేళ (!nlines)
		తిరిగి 0;

	ఒకవేళ (y <= 0) y = 0;
	లేకపోతే ఒకవేళ ((గుర్తులేని)y >= nlines)
		y = nlines - 1;

	పంక్తి = &textc->physical_పంక్తి[y];
	పాఠ్యం = textc->utf8_దత్తాంశం + పంక్తి->ప్రారంభం;
	నిడివి = పంక్తి->నిడివి;
	idx = 0;

	అయితే (x > 0) {
		size_t తదుపరి_స్థానభ్రంశం = 0;
		పూర్ణాంకం వెడల్పు = గరిష్ఠ_పూర్ణాంకం;

		అయితే (తదుపరి_స్థానభ్రంశం < నిడివి && పాఠ్యం[తదుపరి_స్థానభ్రంశం] != '\t') {
			తదుపరి_స్థానభ్రంశం = utf8_తదుపరి(పాఠ్యం, నిడివి, తదుపరి_స్థానభ్రంశం);
		}

		ఒకవేళ (తదుపరి_స్థానభ్రంశం < నిడివి) {
			guit->layout->వెడల్పు(&textplain_style,
					    పాఠ్యం,
					    తదుపరి_స్థానభ్రంశం,
					    &వెడల్పు);
		}

		ఒకవేళ (x <= వెడల్పు) {
			పూర్ణాంకం pixel_స్థానభ్రంశం;
			size_t char_స్థానభ్రంశం;

			guit->layout->position(&textplain_style,
					       పాఠ్యం, తదుపరి_స్థానభ్రంశం, x,
					       &char_స్థానభ్రంశం, &pixel_స్థానభ్రంశం);

			idx += char_స్థానభ్రంశం;
			ఆపు;
		}

		x -= వెడల్పు;
		నిడివి -= తదుపరి_స్థానభ్రంశం;
		పాఠ్యం += తదుపరి_స్థానభ్రంశం;
		idx += తదుపరి_స్థానభ్రంశం;

		/* check if it's within the tab */
		వెడల్పు = textplain_tab_వెడల్పు - (వెడల్పు % textplain_tab_వెడల్పు);
		ఒకవేళ (x <= వెడల్పు) ఆపు;

		x -= వెడల్పు;
		నిడివి--;
		పాఠ్యం++;
		idx++;
	}

	తిరిగి పంక్తి->ప్రారంభం + idx;
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
స్థిర nserror
textplain_mouse_action(నిర్మాణం content *c,
		       నిర్మాణం browser_కిటికీ *bw,
		       browser_mouse_state mouse,
		       పూర్ణాంకం x, పూర్ణాంకం y)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	browser_సూచిక_shape సూచిక = BROWSER_సూచిక_DEFAULT;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	స్థిరరాశి అక్షరం *స్థితి_2 = 0;
	size_t idx;
	పూర్ణాంకం dir = 0;

	browser_కిటికీ_సమితి_drag_type(bw, DRAGGING_NONE, ఖాళీ);

	idx = textplain_స్థానభ్రంశం_from_coords(c, x, y, dir);
	ఒకవేళ (selection_click(పాఠ్యం->sel, పాఠ్యం->bw, mouse, idx)) {

		ఒకవేళ (selection_dragging(పాఠ్యం->sel)) {
			browser_కిటికీ_సమితి_drag_type(bw,
						     DRAGGING_SELECTION, ఖాళీ);
			స్థితి_2 = messages_get("Selecting");
		}

	} లేకపోతే {
		ఒకవేళ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			browser_కిటికీ_పుట_drag_ప్రారంభం(bw, x, y);
			సూచిక = BROWSER_సూచిక_MOVE;
		}
	}

	సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = స్థితి_2;
	content_ప్రసారం(c, CONTENT_సందేశం_స్థితి, &సందేశం_దత్తాంశం);

	సందేశం_దత్తాంశం.సూచిక = సూచిక;
	content_ప్రసారం(c, CONTENT_సందేశం_సూచిక, &సందేశం_దత్తాంశం);

	తిరిగి NSERROR_OK;
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
స్థిర nserror
textplain_mouse_track(నిర్మాణం content *c,
		      నిర్మాణం browser_కిటికీ *bw,
		      browser_mouse_state mouse,
		      పూర్ణాంకం x, పూర్ణాంకం y)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	ఒకవేళ (browser_కిటికీ_get_drag_type(bw) == DRAGGING_SELECTION && !mouse) {
		పూర్ణాంకం dir = -1;
		size_t idx;

		ఒకవేళ (selection_dragging_ప్రారంభం(పాఠ్యం->sel))
			dir = 1;

		idx = textplain_స్థానభ్రంశం_from_coords(c, x, y, dir);
		selection_track(పాఠ్యం->sel, mouse, idx);

		browser_కిటికీ_సమితి_drag_type(bw, DRAGGING_NONE, ఖాళీ);
	}

	ఎంపిక (browser_కిటికీ_get_drag_type(bw)) {

	స్థితి DRAGGING_SELECTION: {
		పూర్ణాంకం dir = -1;
		size_t idx;

		ఒకవేళ (selection_dragging_ప్రారంభం(పాఠ్యం->sel)) dir = 1;

		idx = textplain_స్థానభ్రంశం_from_coords(c, x, y, dir);
		selection_track(పాఠ్యం->sel, mouse, idx);
	}
		ఆపు;

	అప్రమేయ:
		textplain_mouse_action(c, bw, mouse, x, y);
		ఆపు;
	}

	తిరిగి NSERROR_OK;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type CONTENT_TEXTPLAIN
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
స్థిర bool textplain_keypress(నిర్మాణం content *c, uint32_t కీలకం)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	నిర్మాణం selection *sel = పాఠ్యం->sel;

	ఎంపిక (కీలకం) {
	స్థితి NS_కీలకం_COPY_SELECTION:
		selection_copy_గమ్యం_clipboard(sel);
		తిరిగి true;

	స్థితి NS_కీలకం_CLEAR_SELECTION:
		selection_clear(sel, true);
		తిరిగి true;

	స్థితి NS_కీలకం_SELECT_ALL:
		selection_select_all(sel);
		తిరిగి true;

	స్థితి NS_కీలకం_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		తిరిగి selection_clear(sel, true);
	}

	తిరిగి false;
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
స్థిర bool
పాఠ్యం_draw(స్థిరరాశి అక్షరం *utf8_పాఠ్యం,
	    size_t utf8_len,
	    size_t స్థానభ్రంశం,
	    పూర్ణాంకం x,
	    పూర్ణాంకం y,
	    స్థిరరాశి నిర్మాణం rect *clip,
	    పూర్ణాంకం ఎత్తు,
	    దశాంశం scale,
	    textplain_content *పాఠ్యం,
	    స్థిరరాశి నిర్మాణం selection *sel,
	    స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle = textplain_style;
	plot_fstyle.కొలత *= scale;

	/* is this box part of a selection? */
	ఒకవేళ (ctx->interactive == true) {
		గుర్తులేని len = utf8_len;
		గుర్తులేని ప్రారంభం_idx;
		గుర్తులేని చివర_idx;

		/* first try the browser window's current selection */
		ఒకవేళ (selection_highlighted(sel,
					  స్థానభ్రంశం,
					  స్థానభ్రంశం + len,
					  &ప్రారంభం_idx,
					  &చివర_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ఒకవేళ (!highlighted &&
		    (పాఠ్యం->పునాది.textsearch.context != ఖాళీ) &&
		    content_textsearch_ishighlighted(పాఠ్యం->పునాది.textsearch.context,
						     స్థానభ్రంశం,
						     స్థానభ్రంశం + len,
						     &ప్రారంభం_idx,
						     &చివర_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ఒకవేళ (highlighted) {
			నిర్మాణం rect r;
			గుర్తులేని endtxt_idx = చివర_idx;
			bool clip_మారింది = false;
			bool పాఠ్యం_visible = true;
			పూర్ణాంకం startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ఒకవేళ (చివర_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text
				 */
				ప్రతిజ్ఞా(చివర_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->వెడల్పు(&textplain_style,
						  utf8_పాఠ్యం,
						  ప్రారంభం_idx,
						  &startx);
			ఒకవేళ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->వెడల్పు(&textplain_style,
						  utf8_పాఠ్యం,
						  endtxt_idx,
						  &endx);
			ఒకవేళ (res != NSERROR_OK) {
				endx = 0;
			}

			ఒకవేళ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ఒకవేళ (ప్రారంభం_idx > 0) {
				res = ctx->plot->పాఠ్యం(ctx,
					      &plot_fstyle,
					      x,
					      y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
					      utf8_పాఠ్యం,
					      ప్రారంభం_idx);
				ఒకవేళ (res != NSERROR_OK) {
					తిరిగి false;
				}
			}

			pstyle_fill_hback.fill_రంగు = textplain_style.foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ఎత్తు * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}

			ఒకవేళ (ప్రారంభం_idx > 0) {
				పూర్ణాంకం px0 = max(x + startx, clip->x0);
				పూర్ణాంకం px1 = min(x + endx, clip->x1);

				ఒకవేళ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}

					clip_మారింది = true;
				} లేకపోతే {
					పాఠ్యం_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_రంగు;
			fstyle_hback.foreground = రంగు_గమ్యం_bw_furthest(
				pstyle_fill_hback.fill_రంగు);

			ఒకవేళ (పాఠ్యం_visible &&
			    (ctx->plot->పాఠ్యం(ctx,
					     &fstyle_hback,
					     x,
					     y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
					     utf8_పాఠ్యం,
					     endtxt_idx) != NSERROR_OK)) {
				తిరిగి false;
			}

			/* draw any text succeeding highlighted portion */
			ఒకవేళ (endtxt_idx < utf8_len) {
				పూర్ణాంకం px0 = max(x + endx, clip->x0);
				ఒకవేళ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}

					clip_మారింది = true;

					res = ctx->plot->పాఠ్యం(ctx,
							      &plot_fstyle,
							      x,
							      y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
							      utf8_పాఠ్యం,
							      utf8_len);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}
				}
			}

			ఒకవేళ (clip_మారింది &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				తిరిగి false;
			}
		}
	}

	ఒకవేళ (!highlighted) {
		res = ctx->plot->పాఠ్యం(ctx,
				      &plot_fstyle,
				      x,
				      y + (పూర్ణాంకం) (ఎత్తు * 0.75 * scale),
				      utf8_పాఠ్యం,
				      utf8_len);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}
	తిరిగి true;
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
స్థిర bool
textplain_redraw(నిర్మాణం content *c,
		 నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		 స్థిరరాశి నిర్మాణం rect *clip,
		 స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	నిర్మాణం browser_కిటికీ *bw = పాఠ్యం->bw;
	అక్షరం *utf8_దత్తాంశం = పాఠ్యం->utf8_దత్తాంశం;
	దీర్ఘ lineno;
	పూర్ణాంకం x = దత్తాంశం->x;
	పూర్ణాంకం y = దత్తాంశం->y;
	గుర్తులేని దీర్ఘ పంక్తి_లెక్క = పాఠ్యం->physical_పంక్తి_లెక్క;
	దశాంశం పంక్తి_ఎత్తు = textplain_పంక్తి_ఎత్తు();
	దశాంశం scaled_పంక్తి_ఎత్తు = పంక్తి_ఎత్తు * దత్తాంశం->scale;
	దీర్ఘ line0 = (clip->y0 - y * దత్తాంశం->scale) / scaled_పంక్తి_ఎత్తు - 1;
	దీర్ఘ line1 = (clip->y1 - y * దత్తాంశం->scale) / scaled_పంక్తి_ఎత్తు + 1;
	నిర్మాణం textplain_పంక్తి *పంక్తి = పాఠ్యం->physical_పంక్తి;
	size_t నిడివి;
	plot_style_t *plot_style_highlight;
	nserror res;

	ఒకవేళ (line0 < 0)
		line0 = 0;
	ఒకవేళ (line1 < 0)
		line1 = 0;
	ఒకవేళ (పంక్తి_లెక్క < (గుర్తులేని దీర్ఘ) line0)
		line0 = పంక్తి_లెక్క;
	ఒకవేళ (పంక్తి_లెక్క < (గుర్తులేని దీర్ఘ) line1)
		line1 = పంక్తి_లెక్క;
	ఒకవేళ (line1 < line0)
		line1 = line0;

	res = ctx->plot->rectangle(ctx, plot_style_fill_white, clip);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	ఒకవేళ (!పంక్తి)
		తిరిగి true;

	/* choose a suitable background colour for any highlighted text */
	ఒకవేళ ((దత్తాంశం->background_రంగు & 0x808080) == 0x808080)
		plot_style_highlight = plot_style_fill_black;
	లేకపోతే
		plot_style_highlight = plot_style_fill_white;

	/* Set up font plot style */
	textplain_style.background = దత్తాంశం->background_రంగు;

	x = (x + MARGIN) * దత్తాంశం->scale;
	y = (y + MARGIN) * దత్తాంశం->scale;
	కొరకు (lineno = line0; lineno != line1; lineno++) {
		స్థిరరాశి అక్షరం *పాఠ్యం_d = utf8_దత్తాంశం + పంక్తి[lineno].ప్రారంభం;
		పూర్ణాంకం tab_వెడల్పు = textplain_tab_వెడల్పు * దత్తాంశం->scale;
		size_t స్థానభ్రంశం = 0;
		పూర్ణాంకం tx = x;

		ఒకవేళ (!tab_వెడల్పు) tab_వెడల్పు = 1;

		నిడివి = పంక్తి[lineno].నిడివి;
		ఒకవేళ (!నిడివి)
			కొనసాగు;

		అయితే (స్థానభ్రంశం < నిడివి) {
			size_t తదుపరి_స్థానభ్రంశం = స్థానభ్రంశం;
			పూర్ణాంకం వెడల్పు;
			పూర్ణాంకం ntx;
			nserror res;

			అయితే ((తదుపరి_స్థానభ్రంశం < నిడివి) &&
			       (పాఠ్యం_d[తదుపరి_స్థానభ్రంశం] != '\t')) {
				తదుపరి_స్థానభ్రంశం = utf8_తదుపరి(పాఠ్యం_d,
							నిడివి,
							తదుపరి_స్థానభ్రంశం);
			}

			ఒకవేళ (!పాఠ్యం_draw(పాఠ్యం_d + స్థానభ్రంశం,
				       తదుపరి_స్థానభ్రంశం - స్థానభ్రంశం,
				       పంక్తి[lineno].ప్రారంభం + స్థానభ్రంశం,
				       tx,
				       y + (lineno * scaled_పంక్తి_ఎత్తు),
				       clip,
				       పంక్తి_ఎత్తు,
				       దత్తాంశం->scale,
				       పాఠ్యం,
				       పాఠ్యం->sel,
				       ctx)) {
				తిరిగి false;
			}

			ఒకవేళ (తదుపరి_స్థానభ్రంశం >= నిడివి)
				ఆపు;

			res = guit->layout->వెడల్పు(&textplain_style,
						  &పాఠ్యం_d[స్థానభ్రంశం],
						  తదుపరి_స్థానభ్రంశం - స్థానభ్రంశం,
						  &వెడల్పు);
			/* locate end of string and align to next tab position */
			ఒకవేళ (res == NSERROR_OK) {
				tx += (పూర్ణాంకం)(వెడల్పు * దత్తాంశం->scale);
			}

			ntx = x + ((1 + (tx - x) / tab_వెడల్పు) * tab_వెడల్పు);

			/* if the tab character lies within the
			 *  selection, if any, then we must draw it as
			 *  a filled rectangle so that it's consistent
			 *  with background of the selected text
			 */

			ఒకవేళ (bw) {
				గుర్తులేని tab_ofst = పంక్తి[lineno].ప్రారంభం + తదుపరి_స్థానభ్రంశం;
				నిర్మాణం selection *sel = పాఠ్యం->sel;
				bool highlighted = false;

				గుర్తులేని ప్రారంభం_idx, చివర_idx;
				ఒకవేళ (selection_highlighted(sel,
							  tab_ofst,
							  tab_ofst + 1,
							  &ప్రారంభం_idx,
							  &చివర_idx)) {
					highlighted = true;
				}


				ఒకవేళ (!highlighted &&
				    (c->textsearch.context != ఖాళీ)) {
					గుర్తులేని ప్రారంభం_idx, చివర_idx;
					ఒకవేళ (content_textsearch_ishighlighted(
						    c->textsearch.context,
						    tab_ofst,
						    tab_ofst + 1,
						    &ప్రారంభం_idx,
						    &చివర_idx)) {
						highlighted = true;
					}
				}

				ఒకవేళ (highlighted) {
					నిర్మాణం rect rect;
					rect.x0 = tx;
					rect.y0 = y + (lineno * scaled_పంక్తి_ఎత్తు);
					rect.x1 = ntx;
					rect.y1 = rect.y0 + scaled_పంక్తి_ఎత్తు;
					res = ctx->plot->rectangle(ctx,
								   plot_style_highlight,
								   &rect);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}
				}
			}

			స్థానభ్రంశం = తదుపరి_స్థానభ్రంశం + 1;
			tx = ntx;
		}
	}

	తిరిగి true;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being opened.
 */
స్థిర nserror
textplain_open(నిర్మాణం content *c,
	       నిర్మాణం browser_కిటికీ *bw,
	       నిర్మాణం content *పుట,
	       నిర్మాణం object_params *params)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	పాఠ్యం->bw = bw;

	/* text selection */
	selection_init(పాఠ్యం->sel);

	తిరిగి NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_TEXTPLAIN being closed.
 */
స్థిర nserror textplain_close(నిర్మాణం content *c)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	పాఠ్యం->bw = ఖాళీ;

	తిరిగి NSERROR_OK;
}


/**
 * Return an textplain content's selection context
 */
స్థిర అక్షరం *textplain_get_selection(నిర్మాణం content *c)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	తిరిగి selection_get_copy(పాఠ్యం->sel);
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
స్థిర పూర్ణాంకం
textplain_coord_from_స్థానభ్రంశం(స్థిరరాశి అక్షరం *పాఠ్యం, size_t స్థానభ్రంశం, size_t నిడివి)
{
	పూర్ణాంకం x = 0;

	అయితే (స్థానభ్రంశం > 0) {
		size_t తదుపరి_స్థానభ్రంశం = 0;
		పూర్ణాంకం tx;

		అయితే (తదుపరి_స్థానభ్రంశం < స్థానభ్రంశం && పాఠ్యం[తదుపరి_స్థానభ్రంశం] != '\t') {
			తదుపరి_స్థానభ్రంశం = utf8_తదుపరి(పాఠ్యం, నిడివి, తదుపరి_స్థానభ్రంశం);
		}

		guit->layout->వెడల్పు(&textplain_style, పాఠ్యం, తదుపరి_స్థానభ్రంశం, &tx);

		x += tx;

		ఒకవేళ (తదుపరి_స్థానభ్రంశం >= స్థానభ్రంశం)
			ఆపు;

		/* align to next tab boundary */
		తదుపరి_స్థానభ్రంశం++;
		x = (1 + (x / textplain_tab_వెడల్పు)) * textplain_tab_వెడల్పు;
		స్థానభ్రంశం -= తదుపరి_స్థానభ్రంశం;
		పాఠ్యం += తదుపరి_స్థానభ్రంశం;
		నిడివి -= తదుపరి_స్థానభ్రంశం;
	}

	తిరిగి x;
}


/**
 * Retrieve number of lines in content
 *
 * \param[in] c Content to retrieve line count from
 * \return Number of lines
 */
స్థిర గుర్తులేని దీర్ఘ textplain_పంక్తి_లెక్క(నిర్మాణం content *c)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి పాఠ్యం->physical_పంక్తి_లెక్క;
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
స్థిర అక్షరం *
textplain_get_పంక్తి(నిర్మాణం content *c,
		   గుర్తులేని lineno,
		   size_t *poffset,
		   size_t *plen)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	నిర్మాణం textplain_పంక్తి *పంక్తి;

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (lineno >= పాఠ్యం->physical_పంక్తి_లెక్క)
		తిరిగి ఖాళీ;
	పంక్తి = &పాఠ్యం->physical_పంక్తి[lineno];

	*poffset = పంక్తి->ప్రారంభం;
	*plen = పంక్తి->నిడివి;
	తిరిగి పాఠ్యం->utf8_దత్తాంశం + పంక్తి->ప్రారంభం;
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
స్థిర పూర్ణాంకం textplain_కనుగొను_పంక్తి(నిర్మాణం content *c, గుర్తులేని స్థానభ్రంశం)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	నిర్మాణం textplain_పంక్తి *పంక్తి;
	పూర్ణాంకం nlines;
	పూర్ణాంకం lineno = 0;

	ప్రతిజ్ఞా(c != ఖాళీ);

	పంక్తి = పాఠ్యం->physical_పంక్తి;
	nlines = పాఠ్యం->physical_పంక్తి_లెక్క;

	ఒకవేళ (స్థానభ్రంశం > పాఠ్యం->utf8_దత్తాంశం_కొలత) {
		తిరిగి -1;
	}

/* \todo - implement binary search here */
	అయితే (lineno < nlines && పంక్తి[lineno].ప్రారంభం < స్థానభ్రంశం) {
		lineno++;
	}
	ఒకవేళ (పంక్తి[lineno].ప్రారంభం > స్థానభ్రంశం) {
		lineno--;
	}

	తిరిగి lineno;
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
స్థిర nserror
textplain_textsearch_కనుగొను(నిర్మాణం content *c,
			  నిర్మాణం textsearch_context *context,
			  స్థిరరాశి అక్షరం *నమూనా,
			  పూర్ణాంకం p_len,
			  bool case_sens)
{
	పూర్ణాంకం nlines = textplain_పంక్తి_లెక్క(c);
	పూర్ణాంకం పంక్తి;
	nserror res = NSERROR_OK;

	కొరకు(పంక్తి = 0; పంక్తి < nlines; పంక్తి++) {
		size_t స్థానభ్రంశం, నిడివి;
		స్థిరరాశి అక్షరం *పాఠ్యం;

		పాఠ్యం = textplain_get_పంక్తి(c, పంక్తి, &స్థానభ్రంశం, &నిడివి);
		ఒకవేళ (పాఠ్యం) {
			అయితే (నిడివి > 0) {
				గుర్తులేని సరిపోలిక_నిడివి;
				size_t ప్రారంభం_idx;
				స్థిరరాశి అక్షరం *new_పాఠ్యం;
				స్థిరరాశి అక్షరం *pos;

				pos = content_textsearch_కనుగొను_నమూనా(
						పాఠ్యం,
						నిడివి,
						నమూనా,
						p_len,
						case_sens,
						&సరిపోలిక_నిడివి);
				ఒకవేళ (!pos)
					ఆపు;

				/* found string in line => add to list */
				ప్రారంభం_idx = స్థానభ్రంశం + (pos - పాఠ్యం);
				res = content_textsearch_కూడు_సరిపోలిక(context,
						ప్రారంభం_idx,
						ప్రారంభం_idx + సరిపోలిక_నిడివి,
						ఖాళీ,
						ఖాళీ);
				ఒకవేళ (res != NSERROR_OK) {
					తిరిగి res;
				}

				new_పాఠ్యం = pos + సరిపోలిక_నిడివి;
				స్థానభ్రంశం += (new_పాఠ్యం - పాఠ్యం);
				నిడివి -= (new_పాఠ్యం - పాఠ్యం);
				పాఠ్యం = new_పాఠ్యం;
			}
		}
	}

	తిరిగి res;
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
స్థిర శూన్య
textplain_coords_from_range(నిర్మాణం content *c,
			    గుర్తులేని ప్రారంభం,
			    గుర్తులేని చివర,
			    నిర్మాణం rect *r)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	దశాంశం పంక్తి_ఎత్తు = textplain_పంక్తి_ఎత్తు();
	అక్షరం *utf8_దత్తాంశం;
	నిర్మాణం textplain_పంక్తి *పంక్తి;
	గుర్తులేని lineno = 0;
	గుర్తులేని nlines;

	ప్రతిజ్ఞా(c != ఖాళీ);
	ప్రతిజ్ఞా(ప్రారంభం <= చివర);
	ప్రతిజ్ఞా(చివర <= పాఠ్యం->utf8_దత్తాంశం_కొలత);

	utf8_దత్తాంశం = పాఠ్యం->utf8_దత్తాంశం;
	nlines = పాఠ్యం->physical_పంక్తి_లెక్క;
	పంక్తి = పాఠ్యం->physical_పంక్తి;

	/* find start */
	lineno = textplain_కనుగొను_పంక్తి(c, ప్రారంభం);

	r->y0 = (పూర్ణాంకం)(MARGIN + lineno * పంక్తి_ఎత్తు);

	ఒకవేళ (lineno + 1 <= nlines || పంక్తి[lineno + 1].ప్రారంభం >= చివర) {
		/* \todo - it may actually be more efficient just to
		 *   run forwards most of the time
		 */

		/* find end */
		lineno = textplain_కనుగొను_పంక్తి(c, చివర);

		r->x0 = 0;
		r->x1 = పాఠ్యం->formatted_వెడల్పు;
	} లేకపోతే {
		/* single line */
		స్థిరరాశి అక్షరం *పాఠ్యం = utf8_దత్తాంశం + పంక్తి[lineno].ప్రారంభం;

		r->x0 = textplain_coord_from_స్థానభ్రంశం(పాఠ్యం,
						    ప్రారంభం - పంక్తి[lineno].ప్రారంభం,
						    పంక్తి[lineno].నిడివి);

		r->x1 = textplain_coord_from_స్థానభ్రంశం(పాఠ్యం,
						    చివర - పంక్తి[lineno].ప్రారంభం,
						    పంక్తి[lineno].నిడివి);
	}

	r->y1 = (పూర్ణాంకం)(MARGIN + (lineno + 1) * పంక్తి_ఎత్తు);
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
స్థిర అక్షరం *
textplain_get_raw_దత్తాంశం(నిర్మాణం content *c,
		       గుర్తులేని ప్రారంభం,
		       గుర్తులేని చివర,
		       size_t *plen)
{
	textplain_content *పాఠ్యం = (textplain_content *) c;
	size_t utf8_కొలత;

	ప్రతిజ్ఞా(c != ఖాళీ);

	utf8_కొలత = పాఠ్యం->utf8_దత్తాంశం_కొలత;

	/* any text at all? */
	ఒకవేళ (!utf8_కొలత) తిరిగి ఖాళీ;

	/* clamp to valid offset range */
	ఒకవేళ (ప్రారంభం >= utf8_కొలత) ప్రారంభం = utf8_కొలత;
	ఒకవేళ (చివర >= utf8_కొలత) చివర = utf8_కొలత;

	*plen = చివర - ప్రారంభం;

	తిరిగి పాఠ్యం->utf8_దత్తాంశం + ప్రారంభం;
}


/**
 * get bounds of a free text search match
 */
స్థిర nserror
textplain_textsearch_bounds(నిర్మాణం content *c,
			    గుర్తులేని ప్రారంభం_idx,
			    గుర్తులేని చివర_idx,
			    నిర్మాణం box *ప్రారంభం_box,
			    నిర్మాణం box *చివర_box,
			    నిర్మాణం rect *bounds)
{
	textplain_coords_from_range(c, ప్రారంభం_idx, చివర_idx, bounds);

	తిరిగి NSERROR_OK;
}


/**
 * invalidate a region based on offsets into the text cauing a redraw
 */
స్థిర nserror
textplain_textselection_redraw(నిర్మాణం content *c,
			       గుర్తులేని ప్రారంభం_idx,
			       గుర్తులేని చివర_idx)
{
	నిర్మాణం rect r;

	ఒకవేళ (చివర_idx <= ప్రారంభం_idx) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	textplain_coords_from_range(c, ప్రారంభం_idx, చివర_idx, &r);

	content__request_redraw(c, r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);

	తిరిగి NSERROR_OK;
}

స్థిర nserror
textplain_textselection_copy(నిర్మాణం content *c,
			     గుర్తులేని ప్రారంభం_idx,
			     గుర్తులేని చివర_idx,
			     నిర్మాణం selection_string *selstr)
{
	స్థిరరాశి అక్షరం *పాఠ్యం;
	size_t నిడివి;
	bool res = false;

	పాఠ్యం = textplain_get_raw_దత్తాంశం(c, ప్రారంభం_idx, చివర_idx, &నిడివి);
	ఒకవేళ (పాఠ్యం != ఖాళీ) {
		res = selection_string_append(పాఠ్యం, నిడివి, false, ఖాళీ, selstr);
	}
	ఒకవేళ (res == false) {
		తిరిగి NSERROR_NOMEM;
	}
	తిరిగి NSERROR_OK;
}


/**
 * Retrieve the index of the end of the text
 *
 * \param[in] c Content to retrieve size of
 * \return Size, in bytes, of data
 */
స్థిర nserror
textplain_textselection_get_చివర(నిర్మాణం content *c, గుర్తులేని *చివర_idx)
{
	textplain_content *పాఠ్యం = (textplain_content *)c;

	*చివర_idx = పాఠ్యం->utf8_దత్తాంశం_కొలత;
	తిరిగి NSERROR_OK;
}


/**
 * plain text content handler table
 */
స్థిర స్థిరరాశి content_handler textplain_content_handler = {
	.fini = textplain_fini,
	.create = textplain_create,
	.process_దత్తాంశం = textplain_process_దత్తాంశం,
	.దత్తాంశం_complete = textplain_convert,
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
	.textsearch_కనుగొను = textplain_textsearch_కనుగొను,
	.textsearch_bounds = textplain_textsearch_bounds,
	.textselection_redraw = textplain_textselection_redraw,
	.textselection_copy = textplain_textselection_copy,
	.textselection_get_చివర = textplain_textselection_get_చివర,
	.no_share = true,
};


/* exported interface documented in html/textplain.h */
nserror textplain_init(శూన్య)
{
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("Windows-1252",
				   SLEN("Windows-1252"),
				   &textplain_default_charset);
	ఒకవేళ (lerror != lwc_error_ok) {
		తిరిగి NSERROR_NOMEM;
	}

	error = content_factory_రిజిస్టరు_handler("text/plain",
						 &textplain_content_handler);
	ఒకవేళ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	error = content_factory_రిజిస్టరు_handler("application/json",
						 &textplain_content_handler);
	ఒకవేళ (error != NSERROR_OK) {
		lwc_string_unref(textplain_default_charset);
	}

	తిరిగి error;
}




