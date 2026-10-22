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

#ضمن <string.h>
#ضمن <strings.h>

#ضمن "utils/http.h"
#ضمن "utils/utils.h"
#ضمن "utils/corestrings.h"

#ضمن "content/content_factory.h"
#ضمن "content/mimesniff.h"

بنية map_s {
	ثابت uint8_t *sig;
	size_t len;
	bool محفوظ;
	lwc_string **type;
};

ساكن bool mimesniff_has_ٻٽو_octets(ثابت uint8_t *معلومات, size_t len)
{
	ثابت uint8_t *پڇاڙي = معلومات + len;

	طالما (معلومات != پڇاڙي) {
		ثابت uint8_t c = *معلومات;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		إذا (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			اكسر;

		معلومات++;
	}

	أرجع معلومات != پڇاڙي;
}

ساكن nserror mimesniff_ميلاپ_mp4(ثابت uint8_t *معلومات, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_سائيز, i;

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
	إذا (len < 12)
		أرجع NSERROR_NOT_مليو;

	/* Box size is big-endian */
	box_سائيز = (معلومات[0] << 24) | (معلومات[1] << 16) | (معلومات[2] << 8) | معلومات[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	إذا (len < box_سائيز || box_سائيز % 4 != 0)
		أرجع NSERROR_NOT_مليو;

	/* Ensure this is an 'ftyp' box */
	إذا (معلومات[4] != 'f' || معلومات[5] != 't' ||
			معلومات[6] != 'y' || معلومات[7] != 'p')
		أرجع NSERROR_NOT_مليو;

	/* Check if major brand begins with 'mp4' */
	إذا (معلومات[8] == 'm' && معلومات[9] == 'p' && معلومات[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		أرجع NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	لكل (i = 16; i <= box_سائيز - 4; i += 4) {
		إذا (معلومات[i] == 'm' &&
		    معلومات[i+1] == 'p' &&
		    معلومات[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff_ميلاپ_unknown_ws(ثابت uint8_t *معلومات, size_t len,
		lwc_string **effective_type)
{
#وصف_ڪريو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s ws_exact_ميلاپ_types[] = {
		SIG(&corestring_lwc_متن_xml, "<?xml", false),
		{ عدم, 0, false, عدم }
	};

	ساكن ثابت بنية map_s ws_inexact_ميلاپ_types[] = {
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
		{ عدم, 0, false, عدم }
	};
#وصف_رد SIG
	ثابت uint8_t *پڇاڙي = معلومات + len;
	ثابت بنية map_s *it;

	/* Skip leading whitespace */
	طالما (معلومات != پڇاڙي) {
		ثابت uint8_t c = *معلومات;

		إذا (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			اكسر;

		معلومات++;
	}

	إذا (معلومات == پڇاڙي)
		أرجع NSERROR_NOT_مليو;

	len = پڇاڙي - معلومات;

	لكل (it = ws_exact_ميلاپ_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(معلومات, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	لكل (it = ws_inexact_ميلاپ_types; it->sig != عدم; it++) {
		/* +1 for trailing space or > */
		إذا (len < it->len + 1)
			استمر;

		إذا (strncasecmp((ثابت حرف *) معلومات,
				(ثابت حرف *) it->sig, it->len) == 0 &&
				(معلومات[it->len] == ' ' ||
				معلومات[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff_ميلاپ_unknown_bom(ثابت uint8_t *معلومات, size_t len,
		lwc_string **effective_type)
{
#وصف_ڪريو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s bom_ميلاپ_types[] = {
		SIG(&corestring_lwc_متن_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_متن_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_متن_plain, "\xef\xbb\xbf", false),
		{ عدم, 0, false, عدم }
	};
#وصف_رد SIG
	ثابت بنية map_s *it;

	لكل (it = bom_ميلاپ_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(معلومات, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff_ميلاپ_unknown_riff(ثابت uint8_t *معلومات, size_t len,
		lwc_string **effective_type)
{
#وصف_ڪريو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s riff_ميلاپ_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ عدم, 0, false, عدم }
	};
#وصف_رد SIG
	ثابت بنية map_s *it;

	لكل (it = riff_ميلاپ_types; it->sig != عدم; it++) {
		إذا (it->len + SLEN("RIFF????") <= len &&
				memcmp(معلومات, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(معلومات + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff_ميلاپ_unknown_exact(ثابت uint8_t *معلومات, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#وصف_ڪريو SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s exact_ميلاپ_types[] = {
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
		{ عدم, 0, false, عدم }
	};
#وصف_رد SIG
	ثابت بنية map_s *it;

	لكل (it = exact_ميلاپ_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(معلومات, it->sig, it->len) == 0 &&
				(allow_unsafe || it->محفوظ)) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff_ميلاپ_unknown(ثابت uint8_t *معلومات, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	إذا (mimesniff_ميلاپ_unknown_exact(معلومات, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff_ميلاپ_unknown_riff(معلومات, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (allow_unsafe == false)
		أرجع NSERROR_NOT_مليو;

	إذا (mimesniff_ميلاپ_unknown_bom(معلومات, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff_ميلاپ_unknown_ws(معلومات, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff_ميلاپ_mp4(معلومات, len, effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	أرجع NSERROR_NOT_مليو;
}

ساكن nserror mimesniff__compute_unknown(ثابت uint8_t *معلومات, size_t len,
		lwc_string **effective_type)
{
	إذا (معلومات == عدم)
		أرجع NSERROR_NEED_معلومات;

	len = min(len, 512);

	إذا (mimesniff_ميلاپ_unknown(معلومات, len, true,
				     effective_type) == NSERROR_OK) {
		أرجع NSERROR_OK;
	}

	إذا (mimesniff_has_ٻٽو_octets(معلومات, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		أرجع NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_وهڪرو);

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff_compute_متن_or_ٻٽو(ثابت uint8_t *معلومات,
		size_t len, lwc_string **effective_type)
{
	إذا (معلومات == عدم) {
		أرجع NSERROR_NEED_معلومات;
	}

	len = min(len, 512);

	إذا (len >= 3 && ((معلومات[0] == 0xfe && معلومات[1] == 0xff) ||
			(معلومات[0] == 0xff && معلومات[1] == 0xfe) ||
			(معلومات[0] == 0xef && معلومات[1] == 0xbb &&
				معلومات[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		أرجع NSERROR_OK;
	}

	إذا (mimesniff_has_ٻٽو_octets(معلومات, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_متن_plain);
		أرجع NSERROR_OK;
	}

	إذا (mimesniff_ميلاپ_unknown(معلومات, len, false,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_وهڪرو);

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff__compute_image(lwc_string *official_type,
		ثابت uint8_t *معلومات, size_t len, lwc_string **effective_type)
{
#وصف_ڪريو SIG(t, s) { (ثابت uint8_t *) s, SLEN(s), t }
	ساكن ثابت بنية it_s {
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
		{ عدم, 0, عدم }
	};
#وصف_رد SIG

	ثابت بنية it_s *it;

	إذا (معلومات == عدم) {
		lwc_string_unref(official_type);
		أرجع NSERROR_NEED_معلومات;
	}

	لكل (it = image_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(معلومات, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	إذا (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(معلومات, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(معلومات + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		أرجع NSERROR_OK;
	}

	*effective_type = official_type;

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff__compute_feed_or_html(ثابت uint8_t *معلومات,
		size_t len, lwc_string **effective_type)
{
#وصف_ڪريو RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#وصف_ڪريو RSS_NS "http://purl.org/rss/1.0"

	تعداد state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_شروعات,
		COMMENT_OR_DOCTYPE,
		اندريون_COMMENT,
		اندريون_DOCTYPE,
		اندريون_PI,
		اندريون_ٽيگ,
		اندريون_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ثابت uint8_t *پڇاڙي;

	إذا (معلومات == عدم)
		أرجع NSERROR_NEED_معلومات;

	پڇاڙي = معلومات + min(len, 512);

	طالما (معلومات < پڇاڙي) {
		ثابت uint8_t c = *معلومات;

#وصف_ڪريو ميلاپ_2(s) SLEN(s) <= (size_t) (پڇاڙي - معلومات) && \
			memcmp(معلومات, s, SLEN(s)) == 0

		بدل (state) {
		حالة BEFORE_BOM:
			إذا (3 <= پڇاڙي - معلومات && c == 0xef && معلومات[1] == 0xbb &&
					معلومات[2] == 0xbf) {
				معلومات += 3;
			}

			state = BEFORE_MARKUP;
			اكسر;
		حالة BEFORE_MARKUP:
			إذا (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				معلومات++;
			وإلا إذا (c != '<')
				معلومات = پڇاڙي;
			وإلا {
				state = MARKUP_شروعات;
				معلومات++;
			}
			اكسر;
		حالة MARKUP_شروعات:
			إذا (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				معلومات++;
			} وإلا إذا (c == '?') {
				state = اندريون_PI;
				معلومات++;
			} وإلا {
				/* Reconsume input */
				state = اندريون_ٽيگ;
			}
			اكسر;
		حالة COMMENT_OR_DOCTYPE:
			إذا (2 <= پڇاڙي - معلومات && c == '-' && معلومات[1] == '-') {
				state = اندريون_COMMENT;
				معلومات += 2;
			} وإلا {
				/* Reconsume input */
				state = اندريون_DOCTYPE;
			}
			اكسر;
		حالة اندريون_COMMENT:
			إذا (3 <= پڇاڙي - معلومات && c == '-' && معلومات[1] == '-' &&
					معلومات[2] == '>') {
				state = BEFORE_MARKUP;
				معلومات += 3;
			} وإلا
				معلومات++;
			اكسر;
		حالة اندريون_DOCTYPE:
			إذا (c == '>')
				state = BEFORE_MARKUP;
			معلومات++;
			اكسر;
		حالة اندريون_PI:
			إذا (2 <= پڇاڙي - معلومات && c == '?' && معلومات[1] == '>') {
				state = BEFORE_MARKUP;
				معلومات += 2;
			} وإلا
				معلومات++;
			اكسر;
		حالة اندريون_ٽيگ:
			إذا (ميلاپ_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				أرجع NSERROR_OK;
			} وإلا إذا (ميلاپ_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				أرجع NSERROR_OK;
			} وإلا إذا (ميلاپ_2("rdf:RDF")) {
				state = اندريون_RDF;
				معلومات += SLEN("rdf:RDF");
			} وإلا
				معلومات = پڇاڙي;
			اكسر;
		حالة اندريون_RDF:
			إذا (ميلاپ_2(RSS_NS)) {
				rss = true;
				معلومات += SLEN(RSS_NS);
			} وإلا إذا (ميلاپ_2(RDF_NS)) {
				rdf = true;
				معلومات += SLEN(RDF_NS);
			} وإلا
				معلومات++;

			إذا (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				أرجع NSERROR_OK;
			}

			اكسر;
		}
#وصف_رد ميلاپ_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_متن_html);

	أرجع NSERROR_OK;

#وصف_رد RSS_NS
#وصف_رد RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ثابت حرف *content_type_سرو,
				 ثابت uint8_t *معلومات,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#وصف_ڪريو S(s) { s, SLEN(s) }
	ساكن ثابت بنية tt_s {
		ثابت حرف *معلومات;
		size_t len;
	} متن_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ عدم, 0 }
	};
#وصف_رد S

	size_t content_type_سرو_len;
	http_content_type *ct;
	ثابت بنية tt_s *tt;
	bool ميلاپ;
	nserror error;

	إذا (content_type_سرو == عدم) {
		إذا (sniff_allowed == false) {
			أرجع NSERROR_NOT_مليو;
		}

		/* No official type => unknown */
		أرجع mimesniff__compute_unknown(معلومات, len, effective_type);
	}

	error = http_parse_content_type(content_type_سرو, &ct);
	إذا (error != NSERROR_OK) {
		إذا (sniff_allowed == false)
			أرجع NSERROR_NOT_مليو;

		/* Unparseable => unknown */
		أرجع mimesniff__compute_unknown(معلومات, len, effective_type);
	}

	إذا (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع NSERROR_OK;
	}

	إذا (image_only) {
		lwc_string *official_type;

		إذا (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&ميلاپ) == lwc_error_ok && ميلاپ) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			أرجع NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_image(official_type,
				معلومات, len, effective_type);
	}

	content_type_سرو_len = لڙي_ڊيگهه(content_type_سرو);

	/* Look for text types */
	لكل (tt = متن_types; tt->معلومات != عدم; tt++) {
		إذا (tt->len == content_type_سرو_len &&
		    memcmp(tt->معلومات,
			   content_type_سرو,
			   content_type_سرو_len) == 0) {
			http_content_type_destroy(ct);
			أرجع mimesniff_compute_متن_or_ٻٽو(معلومات, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &ميلاپ) == lwc_error_ok && ميلاپ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &ميلاپ) == lwc_error_ok && ميلاپ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &ميلاپ) == lwc_error_ok && ميلاپ)) {
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_unknown(معلومات, len, effective_type);
	}

	/* +xml */
	إذا (lwc_string_ڊيگهائي(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_معلومات(ct->media_type) +
				lwc_string_ڊيگهائي(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع NSERROR_OK;
	}

	/* text/xml, application/xml */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_متن_xml,
					 &ميلاپ) == lwc_error_ok && ميلاپ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &ميلاپ) == lwc_error_ok && ميلاپ)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع NSERROR_OK;
	}

	/* Image types */
	إذا (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_image(official_type,
				معلومات, len, effective_type);
	}

	/* text/html */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_متن_html,
					 &ميلاپ) == lwc_error_ok && ميلاپ)) {
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_feed_or_html(معلومات, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	أرجع NSERROR_OK;
}
