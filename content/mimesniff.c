/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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
 * MIME type sniffer implementation
 *
 * Spec version: 2011-11-27
 */

#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/corestrings.h"

#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/mimesniff.h"

संरचना map_s {
	ध्रुव uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

स्थिर bool mimesniff_has_द्विमान_octets(ध्रुव uint8_t *दत्त, size_t len)
{
	ध्रुव uint8_t *अन्तः = दत्त + len;

	यावत् (दत्त != अन्तः) {
		ध्रुव uint8_t c = *दत्त;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		यदि (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			भङ्ग;

		दत्त++;
	}

	प्रत्यागम दत्त != अन्तः;
}

स्थिर nserror mimesniff_मेलनम्_mp4(ध्रुव uint8_t *दत्त, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_आकार, i;

	/* ISO/IEC 14496-12:2008 $4.3 says (effectively):
	 *
	 * struct ftyp_box {
	 *   uint32_t size; (in octets, including size+type words)
	 *   uint32_t type; (== 'ftyp')
	 *   uint32_t major_brand;
	 *   uint32_t minor_version;
	 *   uint32_t compatible_brands[];
	 * }
	 *
	 * Note 1: A size of 0 implies that the length of the box is designated
	 * by the remaining input data (and thus may only occur in the last
	 * box in the input). We'll reject this below, as it's pointless
	 * sniffing input that contains no boxes other than 'ftyp'.
	 *
	 * Note 2: A size of 1 implies an additional uint64_t field after
	 * the type which contains the extended box size. We'll reject this,
	 * too, as it implies a minimum of (2^32 - 24) / 4 compatible brands,
	 * which is decidely unlikely.
	 */

	/* 12 reflects the minimum number of octets needed to sniff useful
	 * information out of an 'ftyp' box (i.e. the size, type,
	 * and major_brand words). */
	यदि (len < 12)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	/* Box size is big-endian */
	box_आकार = (दत्त[0] << 24) | (दत्त[1] << 16) | (दत्त[2] << 8) | दत्त[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	यदि (len < box_आकार || box_आकार % 4 != 0)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	/* Ensure this is an 'ftyp' box */
	यदि (दत्त[4] != 'f' || दत्त[5] != 't' ||
			दत्त[6] != 'y' || दत्त[7] != 'p')
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	/* Check if major brand begins with 'mp4' */
	यदि (दत्त[8] == 'm' && दत्त[9] == 'p' && दत्त[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		प्रत्यागम NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	प्रति (i = 16; i <= box_आकार - 4; i += 4) {
		यदि (दत्त[i] == 'm' &&
		    दत्त[i+1] == 'p' &&
		    दत्त[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff_मेलनम्_unknown_ws(ध्रुव uint8_t *दत्त, size_t len,
		lwc_string **effective_type)
{
#परिभाषय SIG(t, s, x) { (ध्रुव uint8_t *) s, SLEN(s), x, t }
	स्थिर ध्रुव संरचना map_s ws_exact_मेलनम्_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ रिक्त, 0, false, रिक्त }
	};

	स्थिर ध्रुव संरचना map_s ws_inexact_मेलनम्_types[] = {
		SIG(&corestring_lwc_text_html, "<!DOCTYPE HTML", false),
		SIG(&corestring_lwc_text_html, "<HTML",          false),
		SIG(&corestring_lwc_text_html, "<HEAD",          false),
		SIG(&corestring_lwc_text_html, "<SCRIPT",        false),
		SIG(&corestring_lwc_text_html, "<IFRAME",        false),
		SIG(&corestring_lwc_text_html, "<H1",            false),
		SIG(&corestring_lwc_text_html, "<DIV",           false),
		SIG(&corestring_lwc_text_html, "<FONT",          false),
		SIG(&corestring_lwc_text_html, "<TABLE",         false),
		SIG(&corestring_lwc_text_html, "<A",             false),
		SIG(&corestring_lwc_text_html, "<STYLE",         false),
		SIG(&corestring_lwc_text_html, "<TITLE",         false),
		SIG(&corestring_lwc_text_html, "<B",             false),
		SIG(&corestring_lwc_text_html, "<BODY",          false),
		SIG(&corestring_lwc_text_html, "<BR",            false),
		SIG(&corestring_lwc_text_html, "<P",             false),
		SIG(&corestring_lwc_text_html, "<!--",           false),
		{ रिक्त, 0, false, रिक्त }
	};
#अपरिभाषय SIG
	ध्रुव uint8_t *अन्तः = दत्त + len;
	ध्रुव संरचना map_s *it;

	/* Skip leading whitespace */
	यावत् (दत्त != अन्तः) {
		ध्रुव uint8_t c = *दत्त;

		यदि (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			भङ्ग;

		दत्त++;
	}

	यदि (दत्त == अन्तः)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	len = अन्तः - दत्त;

	प्रति (it = ws_exact_मेलनम्_types; it->sig != रिक्त; it++) {
		यदि (it->len <= len && memcmp(दत्त, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रति (it = ws_inexact_मेलनम्_types; it->sig != रिक्त; it++) {
		/* +1 for trailing space or > */
		यदि (len < it->len + 1)
			अनुवर्त;

		यदि (strncasecmp((ध्रुव वर्ण *) दत्त,
				(ध्रुव वर्ण *) it->sig, it->len) == 0 &&
				(दत्त[it->len] == ' ' ||
				दत्त[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff_मेलनम्_unknown_bom(ध्रुव uint8_t *दत्त, size_t len,
		lwc_string **effective_type)
{
#परिभाषय SIG(t, s, x) { (ध्रुव uint8_t *) s, SLEN(s), x, t }
	स्थिर ध्रुव संरचना map_s bom_मेलनम्_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ रिक्त, 0, false, रिक्त }
	};
#अपरिभाषय SIG
	ध्रुव संरचना map_s *it;

	प्रति (it = bom_मेलनम्_types; it->sig != रिक्त; it++) {
		यदि (it->len <= len && memcmp(दत्त, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff_मेलनम्_unknown_riff(ध्रुव uint8_t *दत्त, size_t len,
		lwc_string **effective_type)
{
#परिभाषय SIG(t, s, x) { (ध्रुव uint8_t *) s, SLEN(s), x, t }
	स्थिर ध्रुव संरचना map_s riff_मेलनम्_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ रिक्त, 0, false, रिक्त }
	};
#अपरिभाषय SIG
	ध्रुव संरचना map_s *it;

	प्रति (it = riff_मेलनम्_types; it->sig != रिक्त; it++) {
		यदि (it->len + SLEN("RIFF????") <= len &&
				memcmp(दत्त, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(दत्त + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff_मेलनम्_unknown_exact(ध्रुव uint8_t *दत्त, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#परिभाषय SIG(t, s, x) { (ध्रुव uint8_t *) s, SLEN(s), x, t }
	स्थिर ध्रुव संरचना map_s exact_मेलनम्_types[] = {
		SIG(&corestring_lwc_image_gif, "GIF87a", true),
		SIG(&corestring_lwc_image_gif, "GIF89a", true),
		SIG(&corestring_lwc_image_png, "\x89PNG\r\n\x1a\n", true),
		SIG(&corestring_lwc_image_jpeg, "\xff\xd8\xff", true),
		SIG(&corestring_lwc_image_bmp, "BM", true),
		SIG(&corestring_lwc_image_vnd_microsoft_icon, "\x00\x00\x01\x00", true),
		SIG(&corestring_lwc_application_ogg, "OggS\x00", true),
		SIG(&corestring_lwc_video_webm, "\x1a\x45\xdf\xa3", true),
		SIG(&corestring_lwc_application_x_rar_compressed, "Rar \x1a\x07\x00", true),
		SIG(&corestring_lwc_application_zip, "PK\x03\x04", true),
		SIG(&corestring_lwc_application_x_gzip, "\x1f\x8b\x08", true),
		SIG(&corestring_lwc_application_postscript, "%!PS-Adobe-",true),
		SIG(&corestring_lwc_application_pdf, "%PDF-", false),
		SIG(&corestring_lwc_image_jxl, "\xFF\x0A", true), /* containerless jpeg xl*/
		{
			(ध्रुव uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ रिक्त, 0, false, रिक्त }
	};
#अपरिभाषय SIG
	ध्रुव संरचना map_s *it;

	प्रति (it = exact_मेलनम्_types; it->sig != रिक्त; it++) {
		यदि (it->len <= len && memcmp(दत्त, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff_मेलनम्_unknown(ध्रुव uint8_t *दत्त, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	यदि (mimesniff_मेलनम्_unknown_exact(दत्त, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	यदि (mimesniff_मेलनम्_unknown_riff(दत्त, len,
			effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	यदि (allow_unsafe == false)
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	यदि (mimesniff_मेलनम्_unknown_bom(दत्त, len,
			effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	यदि (mimesniff_मेलनम्_unknown_ws(दत्त, len,
			effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	यदि (mimesniff_मेलनम्_mp4(दत्त, len, effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror mimesniff__compute_unknown(ध्रुव uint8_t *दत्त, size_t len,
		lwc_string **effective_type)
{
	यदि (दत्त == रिक्त)
		प्रत्यागम NSERROR_NEED_दत्त;

	len = min(len, 512);

	यदि (mimesniff_मेलनम्_unknown(दत्त, len, true,
				     effective_type) == NSERROR_OK) {
		प्रत्यागम NSERROR_OK;
	}

	यदि (mimesniff_has_द्विमान_octets(दत्त, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		प्रत्यागम NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_प्रवाहः);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror mimesniff_compute_text_or_द्विमान(ध्रुव uint8_t *दत्त,
		size_t len, lwc_string **effective_type)
{
	यदि (दत्त == रिक्त) {
		प्रत्यागम NSERROR_NEED_दत्त;
	}

	len = min(len, 512);

	यदि (len >= 3 && ((दत्त[0] == 0xfe && दत्त[1] == 0xff) ||
			(दत्त[0] == 0xff && दत्त[1] == 0xfe) ||
			(दत्त[0] == 0xef && दत्त[1] == 0xbb &&
				दत्त[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		प्रत्यागम NSERROR_OK;
	}

	यदि (mimesniff_has_द्विमान_octets(दत्त, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		प्रत्यागम NSERROR_OK;
	}

	यदि (mimesniff_मेलनम्_unknown(दत्त, len, false,
			effective_type) == NSERROR_OK)
		प्रत्यागम NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_प्रवाहः);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror mimesniff__compute_image(lwc_string *official_type,
		ध्रुव uint8_t *दत्त, size_t len, lwc_string **effective_type)
{
#परिभाषय SIG(t, s) { (ध्रुव uint8_t *) s, SLEN(s), t }
	स्थिर ध्रुव संरचना it_s {
		ध्रुव uint8_t *sig;
		size_t len;
		lwc_string **type;
	} image_types[] = {
		SIG(&corestring_lwc_image_gif, "GIF87a"),
		SIG(&corestring_lwc_image_gif, "GIF89a"),
		SIG(&corestring_lwc_image_png, "\x89PNG\r\n\x1a\n"),
		SIG(&corestring_lwc_image_jpeg, "\xff\xd8\xff"),
		SIG(&corestring_lwc_image_bmp, "BM"),
		SIG(&corestring_lwc_image_vnd_microsoft_icon, "\x00\x00\x01\x00"),
		SIG(&corestring_lwc_image_jxl, "\xFF\x0A"), /* containerless jpeg xl*/
		{
			(ध्रुव uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ रिक्त, 0, रिक्त }
	};
#अपरिभाषय SIG

	ध्रुव संरचना it_s *it;

	यदि (दत्त == रिक्त) {
		lwc_string_unref(official_type);
		प्रत्यागम NSERROR_NEED_दत्त;
	}

	प्रति (it = image_types; it->sig != रिक्त; it++) {
		यदि (it->len <= len && memcmp(दत्त, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			प्रत्यागम NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	यदि (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(दत्त, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(दत्त + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		प्रत्यागम NSERROR_OK;
	}

	*effective_type = official_type;

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror mimesniff__compute_feed_or_html(ध्रुव uint8_t *दत्त,
		size_t len, lwc_string **effective_type)
{
#परिभाषय RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#परिभाषय RSS_NS "http://purl.org/rss/1.0"

	गणना state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_START,
		COMMENT_OR_DOCTYPE,
		अन्तर्_COMMENT,
		अन्तर्_DOCTYPE,
		अन्तर्_PI,
		अन्तर्_चिह्नकम्,
		अन्तर्_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ध्रुव uint8_t *अन्तः;

	यदि (दत्त == रिक्त)
		प्रत्यागम NSERROR_NEED_दत्त;

	अन्तः = दत्त + min(len, 512);

	यावत् (दत्त < अन्तः) {
		ध्रुव uint8_t c = *दत्त;

#परिभाषय मेलनम्_2(s) SLEN(s) <= (size_t) (अन्तः - दत्त) && \
			memcmp(दत्त, s, SLEN(s)) == 0

		विकल्प (state) {
		पक्ष BEFORE_BOM:
			यदि (3 <= अन्तः - दत्त && c == 0xef && दत्त[1] == 0xbb &&
					दत्त[2] == 0xbf) {
				दत्त += 3;
			}

			state = BEFORE_MARKUP;
			भङ्ग;
		पक्ष BEFORE_MARKUP:
			यदि (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				दत्त++;
			अन्यथा यदि (c != '<')
				दत्त = अन्तः;
			अन्यथा {
				state = MARKUP_START;
				दत्त++;
			}
			भङ्ग;
		पक्ष MARKUP_START:
			यदि (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				दत्त++;
			} अन्यथा यदि (c == '?') {
				state = अन्तर्_PI;
				दत्त++;
			} अन्यथा {
				/* Reconsume input */
				state = अन्तर्_चिह्नकम्;
			}
			भङ्ग;
		पक्ष COMMENT_OR_DOCTYPE:
			यदि (2 <= अन्तः - दत्त && c == '-' && दत्त[1] == '-') {
				state = अन्तर्_COMMENT;
				दत्त += 2;
			} अन्यथा {
				/* Reconsume input */
				state = अन्तर्_DOCTYPE;
			}
			भङ्ग;
		पक्ष अन्तर्_COMMENT:
			यदि (3 <= अन्तः - दत्त && c == '-' && दत्त[1] == '-' &&
					दत्त[2] == '>') {
				state = BEFORE_MARKUP;
				दत्त += 3;
			} अन्यथा
				दत्त++;
			भङ्ग;
		पक्ष अन्तर्_DOCTYPE:
			यदि (c == '>')
				state = BEFORE_MARKUP;
			दत्त++;
			भङ्ग;
		पक्ष अन्तर्_PI:
			यदि (2 <= अन्तः - दत्त && c == '?' && दत्त[1] == '>') {
				state = BEFORE_MARKUP;
				दत्त += 2;
			} अन्यथा
				दत्त++;
			भङ्ग;
		पक्ष अन्तर्_चिह्नकम्:
			यदि (मेलनम्_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				प्रत्यागम NSERROR_OK;
			} अन्यथा यदि (मेलनम्_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				प्रत्यागम NSERROR_OK;
			} अन्यथा यदि (मेलनम्_2("rdf:RDF")) {
				state = अन्तर्_RDF;
				दत्त += SLEN("rdf:RDF");
			} अन्यथा
				दत्त = अन्तः;
			भङ्ग;
		पक्ष अन्तर्_RDF:
			यदि (मेलनम्_2(RSS_NS)) {
				rss = true;
				दत्त += SLEN(RSS_NS);
			} अन्यथा यदि (मेलनम्_2(RDF_NS)) {
				rdf = true;
				दत्त += SLEN(RDF_NS);
			} अन्यथा
				दत्त++;

			यदि (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				प्रत्यागम NSERROR_OK;
			}

			भङ्ग;
		}
#अपरिभाषय मेलनम्_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	प्रत्यागम NSERROR_OK;

#अपरिभाषय RSS_NS
#अपरिभाषय RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ध्रुव वर्ण *content_type_शीर्षकम्,
				 ध्रुव uint8_t *दत्त,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#परिभाषय S(s) { s, SLEN(s) }
	स्थिर ध्रुव संरचना tt_s {
		ध्रुव वर्ण *दत्त;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ रिक्त, 0 }
	};
#अपरिभाषय S

	size_t content_type_शीर्षकम्_len;
	http_content_type *ct;
	ध्रुव संरचना tt_s *tt;
	bool मेलनम्;
	nserror error;

	यदि (content_type_शीर्षकम् == रिक्त) {
		यदि (sniff_allowed == false) {
			प्रत्यागम NSERROR_NOT_प्राप्तम्;
		}

		/* No official type => unknown */
		प्रत्यागम mimesniff__compute_unknown(दत्त, len, effective_type);
	}

	error = http_parse_content_type(content_type_शीर्षकम्, &ct);
	यदि (error != NSERROR_OK) {
		यदि (sniff_allowed == false)
			प्रत्यागम NSERROR_NOT_प्राप्तम्;

		/* Unparseable => unknown */
		प्रत्यागम mimesniff__compute_unknown(दत्त, len, effective_type);
	}

	यदि (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		प्रत्यागम NSERROR_OK;
	}

	यदि (image_only) {
		lwc_string *official_type;

		यदि (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&मेलनम्) == lwc_error_ok && मेलनम्) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			प्रत्यागम NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		प्रत्यागम mimesniff__compute_image(official_type,
				दत्त, len, effective_type);
	}

	content_type_शीर्षकम्_len = वाक्यदैर्घ्यम्(content_type_शीर्षकम्);

	/* Look for text types */
	प्रति (tt = text_types; tt->दत्त != रिक्त; tt++) {
		यदि (tt->len == content_type_शीर्षकम्_len &&
		    memcmp(tt->दत्त,
			   content_type_शीर्षकम्,
			   content_type_शीर्षकम्_len) == 0) {
			http_content_type_destroy(ct);
			प्रत्यागम mimesniff_compute_text_or_द्विमान(दत्त, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	यदि ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &मेलनम्) == lwc_error_ok && मेलनम्) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &मेलनम्) == lwc_error_ok && मेलनम्) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &मेलनम्) == lwc_error_ok && मेलनम्)) {
		http_content_type_destroy(ct);
		प्रत्यागम mimesniff__compute_unknown(दत्त, len, effective_type);
	}

	/* +xml */
	यदि (lwc_string_दीर्घता(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_दत्त(ct->media_type) +
				lwc_string_दीर्घता(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		प्रत्यागम NSERROR_OK;
	}

	/* text/xml, application/xml */
	यदि ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &मेलनम्) == lwc_error_ok && मेलनम्) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &मेलनम्) == lwc_error_ok && मेलनम्)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		प्रत्यागम NSERROR_OK;
	}

	/* Image types */
	यदि (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		प्रत्यागम mimesniff__compute_image(official_type,
				दत्त, len, effective_type);
	}

	/* text/html */
	यदि ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &मेलनम्) == lwc_error_ok && मेलनम्)) {
		http_content_type_destroy(ct);
		प्रत्यागम mimesniff__compute_feed_or_html(दत्त, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	प्रत्यागम NSERROR_OK;
}
