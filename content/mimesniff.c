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

#شامل <string.h>
#شامل <strings.h>

#شامل "utils/http.h"
#شامل "utils/utils.h"
#شامل "utils/corestrings.h"

#شامل "content/content_factory.h"
#شامل "content/mimesniff.h"

ساخت map_s {
	ثابت uint8_t *sig;
	size_t len;
	bool محفوظ_2;
	lwc_string **type;
};

ساکن bool mimesniff_has_ثنائی_octets(ثابت uint8_t *ڈیٹا, size_t len)
{
	ثابت uint8_t *انتہا = ڈیٹا + len;

	جب (ڈیٹا != انتہا) {
		ثابت uint8_t c = *ڈیٹا;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		اگر (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			توڑو;

		ڈیٹا++;
	}

	واپس ڈیٹا != انتہا;
}

ساکن nserror mimesniff_مطابقت_mp4(ثابت uint8_t *ڈیٹا, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_سائز, i;

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
	اگر (len < 12)
		واپس NSERROR_NOT_ملا;

	/* Box size is big-endian */
	box_سائز = (ڈیٹا[0] << 24) | (ڈیٹا[1] << 16) | (ڈیٹا[2] << 8) | ڈیٹا[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	اگر (len < box_سائز || box_سائز % 4 != 0)
		واپس NSERROR_NOT_ملا;

	/* Ensure this is an 'ftyp' box */
	اگر (ڈیٹا[4] != 'f' || ڈیٹا[5] != 't' ||
			ڈیٹا[6] != 'y' || ڈیٹا[7] != 'p')
		واپس NSERROR_NOT_ملا;

	/* Check if major brand begins with 'mp4' */
	اگر (ڈیٹا[8] == 'm' && ڈیٹا[9] == 'p' && ڈیٹا[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		واپس NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	برائے (i = 16; i <= box_سائز - 4; i += 4) {
		اگر (ڈیٹا[i] == 'm' &&
		    ڈیٹا[i+1] == 'p' &&
		    ڈیٹا[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			واپس NSERROR_OK;
		}
	}

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff_مطابقت_unknown_ws(ثابت uint8_t *ڈیٹا, size_t len,
		lwc_string **effective_type)
{
#تعریف_کرو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساکن ثابت ساخت map_s ws_exact_مطابقت_types[] = {
		SIG(&corestring_lwc_متن_xml, "<?xml", false),
		{ تہی, 0, false, تہی }
	};

	ساکن ثابت ساخت map_s ws_inexact_مطابقت_types[] = {
		SIG(&corestring_lwc_متن_html, "<!DOCTYPE HTML", false),
		SIG(&corestring_lwc_متن_html, "<HTML",          false),
		SIG(&corestring_lwc_متن_html, "<HEAD",          false),
		SIG(&corestring_lwc_متن_html, "<SCRIPT",        false),
		SIG(&corestring_lwc_متن_html, "<IFRAME",        false),
		SIG(&corestring_lwc_متن_html, "<H1",            false),
		SIG(&corestring_lwc_متن_html, "<DIV",           false),
		SIG(&corestring_lwc_متن_html, "<FONT",          false),
		SIG(&corestring_lwc_متن_html, "<TABLE",         false),
		SIG(&corestring_lwc_متن_html, "<A",             false),
		SIG(&corestring_lwc_متن_html, "<STYLE",         false),
		SIG(&corestring_lwc_متن_html, "<TITLE",         false),
		SIG(&corestring_lwc_متن_html, "<B",             false),
		SIG(&corestring_lwc_متن_html, "<BODY",          false),
		SIG(&corestring_lwc_متن_html, "<BR",            false),
		SIG(&corestring_lwc_متن_html, "<P",             false),
		SIG(&corestring_lwc_متن_html, "<!--",           false),
		{ تہی, 0, false, تہی }
	};
#تعریف_منسوخ SIG
	ثابت uint8_t *انتہا = ڈیٹا + len;
	ثابت ساخت map_s *it;

	/* Skip leading whitespace */
	جب (ڈیٹا != انتہا) {
		ثابت uint8_t c = *ڈیٹا;

		اگر (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			توڑو;

		ڈیٹا++;
	}

	اگر (ڈیٹا == انتہا)
		واپس NSERROR_NOT_ملا;

	len = انتہا - ڈیٹا;

	برائے (it = ws_exact_مطابقت_types; it->sig != تہی; it++) {
		اگر (it->len <= len && memcmp(ڈیٹا, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	برائے (it = ws_inexact_مطابقت_types; it->sig != تہی; it++) {
		/* +1 for trailing space or > */
		اگر (len < it->len + 1)
			جاری;

		اگر (strncasecmp((ثابت حرف *) ڈیٹا,
				(ثابت حرف *) it->sig, it->len) == 0 &&
				(ڈیٹا[it->len] == ' ' ||
				ڈیٹا[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff_مطابقت_unknown_bom(ثابت uint8_t *ڈیٹا, size_t len,
		lwc_string **effective_type)
{
#تعریف_کرو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساکن ثابت ساخت map_s bom_مطابقت_types[] = {
		SIG(&corestring_lwc_متن_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_متن_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_متن_plain, "\xef\xbb\xbf", false),
		{ تہی, 0, false, تہی }
	};
#تعریف_منسوخ SIG
	ثابت ساخت map_s *it;

	برائے (it = bom_مطابقت_types; it->sig != تہی; it++) {
		اگر (it->len <= len && memcmp(ڈیٹا, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff_مطابقت_unknown_riff(ثابت uint8_t *ڈیٹا, size_t len,
		lwc_string **effective_type)
{
#تعریف_کرو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساکن ثابت ساخت map_s riff_مطابقت_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ تہی, 0, false, تہی }
	};
#تعریف_منسوخ SIG
	ثابت ساخت map_s *it;

	برائے (it = riff_مطابقت_types; it->sig != تہی; it++) {
		اگر (it->len + SLEN("RIFF????") <= len &&
				memcmp(ڈیٹا, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(ڈیٹا + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff_مطابقت_unknown_exact(ثابت uint8_t *ڈیٹا, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#تعریف_کرو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساکن ثابت ساخت map_s exact_مطابقت_types[] = {
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
			(ثابت uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ تہی, 0, false, تہی }
	};
#تعریف_منسوخ SIG
	ثابت ساخت map_s *it;

	برائے (it = exact_مطابقت_types; it->sig != تہی; it++) {
		اگر (it->len <= len && memcmp(ڈیٹا, it->sig, it->len) == 0 &&
				(allow_unsafe || it->محفوظ_2)) {
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff_مطابقت_unknown(ثابت uint8_t *ڈیٹا, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	اگر (mimesniff_مطابقت_unknown_exact(ڈیٹا, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	اگر (mimesniff_مطابقت_unknown_riff(ڈیٹا, len,
			effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	اگر (allow_unsafe == false)
		واپس NSERROR_NOT_ملا;

	اگر (mimesniff_مطابقت_unknown_bom(ڈیٹا, len,
			effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	اگر (mimesniff_مطابقت_unknown_ws(ڈیٹا, len,
			effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	اگر (mimesniff_مطابقت_mp4(ڈیٹا, len, effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	واپس NSERROR_NOT_ملا;
}

ساکن nserror mimesniff__compute_unknown(ثابت uint8_t *ڈیٹا, size_t len,
		lwc_string **effective_type)
{
	اگر (ڈیٹا == تہی)
		واپس NSERROR_NEED_ڈیٹا;

	len = min(len, 512);

	اگر (mimesniff_مطابقت_unknown(ڈیٹا, len, true,
				     effective_type) == NSERROR_OK) {
		واپس NSERROR_OK;
	}

	اگر (mimesniff_has_ثنائی_octets(ڈیٹا, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		واپس NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_دھارا);

	واپس NSERROR_OK;
}

ساکن nserror mimesniff_compute_متن_or_ثنائی(ثابت uint8_t *ڈیٹا,
		size_t len, lwc_string **effective_type)
{
	اگر (ڈیٹا == تہی) {
		واپس NSERROR_NEED_ڈیٹا;
	}

	len = min(len, 512);

	اگر (len >= 3 && ((ڈیٹا[0] == 0xfe && ڈیٹا[1] == 0xff) ||
			(ڈیٹا[0] == 0xff && ڈیٹا[1] == 0xfe) ||
			(ڈیٹا[0] == 0xef && ڈیٹا[1] == 0xbb &&
				ڈیٹا[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		واپس NSERROR_OK;
	}

	اگر (mimesniff_has_ثنائی_octets(ڈیٹا, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		واپس NSERROR_OK;
	}

	اگر (mimesniff_مطابقت_unknown(ڈیٹا, len, false,
			effective_type) == NSERROR_OK)
		واپس NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_دھارا);

	واپس NSERROR_OK;
}

ساکن nserror mimesniff__compute_image(lwc_string *official_type,
		ثابت uint8_t *ڈیٹا, size_t len, lwc_string **effective_type)
{
#تعریف_کرو SIG(t, s) { (ثابت uint8_t *) s, SLEN(s), t }
	ساکن ثابت ساخت it_s {
		ثابت uint8_t *sig;
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
			(ثابت uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ تہی, 0, تہی }
	};
#تعریف_منسوخ SIG

	ثابت ساخت it_s *it;

	اگر (ڈیٹا == تہی) {
		lwc_string_unref(official_type);
		واپس NSERROR_NEED_ڈیٹا;
	}

	برائے (it = image_types; it->sig != تہی; it++) {
		اگر (it->len <= len && memcmp(ڈیٹا, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			واپس NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	اگر (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(ڈیٹا, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(ڈیٹا + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		واپس NSERROR_OK;
	}

	*effective_type = official_type;

	واپس NSERROR_OK;
}

ساکن nserror mimesniff__compute_feed_or_html(ثابت uint8_t *ڈیٹا,
		size_t len, lwc_string **effective_type)
{
#تعریف_کرو RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#تعریف_کرو RSS_NS "http://purl.org/rss/1.0"

	شمار state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_آغاز,
		COMMENT_OR_DOCTYPE,
		اندر_COMMENT,
		اندر_DOCTYPE,
		اندر_PI,
		اندر_لیبل,
		اندر_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ثابت uint8_t *انتہا;

	اگر (ڈیٹا == تہی)
		واپس NSERROR_NEED_ڈیٹا;

	انتہا = ڈیٹا + min(len, 512);

	جب (ڈیٹا < انتہا) {
		ثابت uint8_t c = *ڈیٹا;

#تعریف_کرو مطابقت_2(s) SLEN(s) <= (size_t) (انتہا - ڈیٹا) && \
			memcmp(ڈیٹا, s, SLEN(s)) == 0

		انتخاب (state) {
		حالت BEFORE_BOM:
			اگر (3 <= انتہا - ڈیٹا && c == 0xef && ڈیٹا[1] == 0xbb &&
					ڈیٹا[2] == 0xbf) {
				ڈیٹا += 3;
			}

			state = BEFORE_MARKUP;
			توڑو;
		حالت BEFORE_MARKUP:
			اگر (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				ڈیٹا++;
			ورنہ اگر (c != '<')
				ڈیٹا = انتہا;
			ورنہ {
				state = MARKUP_آغاز;
				ڈیٹا++;
			}
			توڑو;
		حالت MARKUP_آغاز:
			اگر (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				ڈیٹا++;
			} ورنہ اگر (c == '?') {
				state = اندر_PI;
				ڈیٹا++;
			} ورنہ {
				/* Reconsume input */
				state = اندر_لیبل;
			}
			توڑو;
		حالت COMMENT_OR_DOCTYPE:
			اگر (2 <= انتہا - ڈیٹا && c == '-' && ڈیٹا[1] == '-') {
				state = اندر_COMMENT;
				ڈیٹا += 2;
			} ورنہ {
				/* Reconsume input */
				state = اندر_DOCTYPE;
			}
			توڑو;
		حالت اندر_COMMENT:
			اگر (3 <= انتہا - ڈیٹا && c == '-' && ڈیٹا[1] == '-' &&
					ڈیٹا[2] == '>') {
				state = BEFORE_MARKUP;
				ڈیٹا += 3;
			} ورنہ
				ڈیٹا++;
			توڑو;
		حالت اندر_DOCTYPE:
			اگر (c == '>')
				state = BEFORE_MARKUP;
			ڈیٹا++;
			توڑو;
		حالت اندر_PI:
			اگر (2 <= انتہا - ڈیٹا && c == '?' && ڈیٹا[1] == '>') {
				state = BEFORE_MARKUP;
				ڈیٹا += 2;
			} ورنہ
				ڈیٹا++;
			توڑو;
		حالت اندر_لیبل:
			اگر (مطابقت_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				واپس NSERROR_OK;
			} ورنہ اگر (مطابقت_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				واپس NSERROR_OK;
			} ورنہ اگر (مطابقت_2("rdf:RDF")) {
				state = اندر_RDF;
				ڈیٹا += SLEN("rdf:RDF");
			} ورنہ
				ڈیٹا = انتہا;
			توڑو;
		حالت اندر_RDF:
			اگر (مطابقت_2(RSS_NS)) {
				rss = true;
				ڈیٹا += SLEN(RSS_NS);
			} ورنہ اگر (مطابقت_2(RDF_NS)) {
				rdf = true;
				ڈیٹا += SLEN(RDF_NS);
			} ورنہ
				ڈیٹا++;

			اگر (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				واپس NSERROR_OK;
			}

			توڑو;
		}
#تعریف_منسوخ مطابقت_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_متن_html);

	واپس NSERROR_OK;

#تعریف_منسوخ RSS_NS
#تعریف_منسوخ RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ثابت حرف *content_type_سرخی,
				 ثابت uint8_t *ڈیٹا,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#تعریف_کرو S(s) { s, SLEN(s) }
	ساکن ثابت ساخت tt_s {
		ثابت حرف *ڈیٹا;
		size_t len;
	} متن_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ تہی, 0 }
	};
#تعریف_منسوخ S

	size_t content_type_سرخی_len;
	http_content_type *ct;
	ثابت ساخت tt_s *tt;
	bool مطابقت;
	nserror error;

	اگر (content_type_سرخی == تہی) {
		اگر (sniff_allowed == false) {
			واپس NSERROR_NOT_ملا;
		}

		/* No official type => unknown */
		واپس mimesniff__compute_unknown(ڈیٹا, len, effective_type);
	}

	error = http_parse_content_type(content_type_سرخی, &ct);
	اگر (error != NSERROR_OK) {
		اگر (sniff_allowed == false)
			واپس NSERROR_NOT_ملا;

		/* Unparseable => unknown */
		واپس mimesniff__compute_unknown(ڈیٹا, len, effective_type);
	}

	اگر (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		واپس NSERROR_OK;
	}

	اگر (image_only) {
		lwc_string *official_type;

		اگر (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&مطابقت) == lwc_error_ok && مطابقت) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			واپس NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		واپس mimesniff__compute_image(official_type,
				ڈیٹا, len, effective_type);
	}

	content_type_سرخی_len = لڑی_لمبائی(content_type_سرخی);

	/* Look for text types */
	برائے (tt = متن_types; tt->ڈیٹا != تہی; tt++) {
		اگر (tt->len == content_type_سرخی_len &&
		    memcmp(tt->ڈیٹا,
			   content_type_سرخی,
			   content_type_سرخی_len) == 0) {
			http_content_type_destroy(ct);
			واپس mimesniff_compute_متن_or_ثنائی(ڈیٹا, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	اگر ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &مطابقت) == lwc_error_ok && مطابقت) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &مطابقت) == lwc_error_ok && مطابقت) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &مطابقت) == lwc_error_ok && مطابقت)) {
		http_content_type_destroy(ct);
		واپس mimesniff__compute_unknown(ڈیٹا, len, effective_type);
	}

	/* +xml */
	اگر (lwc_string_لمبائی(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_ڈیٹا(ct->media_type) +
				lwc_string_لمبائی(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		واپس NSERROR_OK;
	}

	/* text/xml, application/xml */
	اگر ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_متن_xml,
					 &مطابقت) == lwc_error_ok && مطابقت) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &مطابقت) == lwc_error_ok && مطابقت)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		واپس NSERROR_OK;
	}

	/* Image types */
	اگر (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		واپس mimesniff__compute_image(official_type,
				ڈیٹا, len, effective_type);
	}

	/* text/html */
	اگر ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_متن_html,
					 &مطابقت) == lwc_error_ok && مطابقت)) {
		http_content_type_destroy(ct);
		واپس mimesniff__compute_feed_or_html(ڈیٹا, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	واپس NSERROR_OK;
}
