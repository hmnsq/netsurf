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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>

#অন্তর্ভাব "utils/http.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/corestrings.h"

#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/mimesniff.h"

কাঠামো map_s {
	ধ্রুবক uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

স্থির bool mimesniff_has_দ্বিমিক_octets(ধ্রুবক uint8_t *তথ্য, size_t len)
{
	ধ্রুবক uint8_t *end = তথ্য + len;

	যখন (তথ্য != end) {
		ধ্রুবক uint8_t c = *তথ্য;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		যদি (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			বিরতি;

		তথ্য++;
	}

	ফেরত তথ্য != end;
}

স্থির nserror mimesniff__match_mp4(ধ্রুবক uint8_t *তথ্য, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_আকার, i;

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
	যদি (len < 12)
		ফেরত NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_আকার = (তথ্য[0] << 24) | (তথ্য[1] << 16) | (তথ্য[2] << 8) | তথ্য[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	যদি (len < box_আকার || box_আকার % 4 != 0)
		ফেরত NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	যদি (তথ্য[4] != 'f' || তথ্য[5] != 't' ||
			তথ্য[6] != 'y' || তথ্য[7] != 'p')
		ফেরত NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	যদি (তথ্য[8] == 'm' && তথ্য[9] == 'p' && তথ্য[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ফেরত NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	জন্য (i = 16; i <= box_আকার - 4; i += 4) {
		যদি (তথ্য[i] == 'm' &&
		    তথ্য[i+1] == 'p' &&
		    তথ্য[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ফেরত NSERROR_OK;
		}
	}

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_ws(ধ্রুবক uint8_t *তথ্য, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুবক uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুবক কাঠামো map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ ফাঁকা, 0, false, ফাঁকা }
	};

	স্থির ধ্রুবক কাঠামো map_s ws_inexact_match_types[] = {
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
		{ ফাঁকা, 0, false, ফাঁকা }
	};
#undef SIG
	ধ্রুবক uint8_t *end = তথ্য + len;
	ধ্রুবক কাঠামো map_s *it;

	/* Skip leading whitespace */
	যখন (তথ্য != end) {
		ধ্রুবক uint8_t c = *তথ্য;

		যদি (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			বিরতি;

		তথ্য++;
	}

	যদি (তথ্য == end)
		ফেরত NSERROR_NOT_FOUND;

	len = end - তথ্য;

	জন্য (it = ws_exact_match_types; it->sig != ফাঁকা; it++) {
		যদি (it->len <= len && memcmp(তথ্য, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	জন্য (it = ws_inexact_match_types; it->sig != ফাঁকা; it++) {
		/* +1 for trailing space or > */
		যদি (len < it->len + 1)
			চালু;

		যদি (strncasecmp((ধ্রুবক অক্ষর *) তথ্য,
				(ধ্রুবক অক্ষর *) it->sig, it->len) == 0 &&
				(তথ্য[it->len] == ' ' ||
				তথ্য[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_bom(ধ্রুবক uint8_t *তথ্য, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুবক uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুবক কাঠামো map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ ফাঁকা, 0, false, ফাঁকা }
	};
#undef SIG
	ধ্রুবক কাঠামো map_s *it;

	জন্য (it = bom_match_types; it->sig != ফাঁকা; it++) {
		যদি (it->len <= len && memcmp(তথ্য, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_riff(ধ্রুবক uint8_t *তথ্য, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুবক uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুবক কাঠামো map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ফাঁকা, 0, false, ফাঁকা }
	};
#undef SIG
	ধ্রুবক কাঠামো map_s *it;

	জন্য (it = riff_match_types; it->sig != ফাঁকা; it++) {
		যদি (it->len + SLEN("RIFF????") <= len &&
				memcmp(তথ্য, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(তথ্য + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_exact(ধ্রুবক uint8_t *তথ্য, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুবক uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুবক কাঠামো map_s exact_match_types[] = {
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
			(ধ্রুবক uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ফাঁকা, 0, false, ফাঁকা }
	};
#undef SIG
	ধ্রুবক কাঠামো map_s *it;

	জন্য (it = exact_match_types; it->sig != ফাঁকা; it++) {
		যদি (it->len <= len && memcmp(তথ্য, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown(ধ্রুবক uint8_t *তথ্য, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	যদি (mimesniff__match_unknown_exact(তথ্য, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	যদি (mimesniff__match_unknown_riff(তথ্য, len,
			effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	যদি (allow_unsafe == false)
		ফেরত NSERROR_NOT_FOUND;

	যদি (mimesniff__match_unknown_bom(তথ্য, len,
			effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	যদি (mimesniff__match_unknown_ws(তথ্য, len,
			effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	যদি (mimesniff__match_mp4(তথ্য, len, effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	ফেরত NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__compute_unknown(ধ্রুবক uint8_t *তথ্য, size_t len,
		lwc_string **effective_type)
{
	যদি (তথ্য == ফাঁকা)
		ফেরত NSERROR_NEED_তথ্য;

	len = min(len, 512);

	যদি (mimesniff__match_unknown(তথ্য, len, true,
				     effective_type) == NSERROR_OK) {
		ফেরত NSERROR_OK;
	}

	যদি (mimesniff_has_দ্বিমিক_octets(তথ্য, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ফেরত NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ফেরত NSERROR_OK;
}

স্থির nserror mimesniff_compute_text_or_দ্বিমিক(ধ্রুবক uint8_t *তথ্য,
		size_t len, lwc_string **effective_type)
{
	যদি (তথ্য == ফাঁকা) {
		ফেরত NSERROR_NEED_তথ্য;
	}

	len = min(len, 512);

	যদি (len >= 3 && ((তথ্য[0] == 0xfe && তথ্য[1] == 0xff) ||
			(তথ্য[0] == 0xff && তথ্য[1] == 0xfe) ||
			(তথ্য[0] == 0xef && তথ্য[1] == 0xbb &&
				তথ্য[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ফেরত NSERROR_OK;
	}

	যদি (mimesniff_has_দ্বিমিক_octets(তথ্য, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ফেরত NSERROR_OK;
	}

	যদি (mimesniff__match_unknown(তথ্য, len, false,
			effective_type) == NSERROR_OK)
		ফেরত NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ফেরত NSERROR_OK;
}

স্থির nserror mimesniff__compute_image(lwc_string *official_type,
		ধ্রুবক uint8_t *তথ্য, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (ধ্রুবক uint8_t *) s, SLEN(s), t }
	স্থির ধ্রুবক কাঠামো it_s {
		ধ্রুবক uint8_t *sig;
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
			(ধ্রুবক uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ফাঁকা, 0, ফাঁকা }
	};
#undef SIG

	ধ্রুবক কাঠামো it_s *it;

	যদি (তথ্য == ফাঁকা) {
		lwc_string_unref(official_type);
		ফেরত NSERROR_NEED_তথ্য;
	}

	জন্য (it = image_types; it->sig != ফাঁকা; it++) {
		যদি (it->len <= len && memcmp(তথ্য, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ফেরত NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	যদি (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(তথ্য, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(তথ্য + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ফেরত NSERROR_OK;
	}

	*effective_type = official_type;

	ফেরত NSERROR_OK;
}

স্থির nserror mimesniff__compute_feed_or_html(ধ্রুবক uint8_t *তথ্য,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	গণনা state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_START,
		COMMENT_OR_DOCTYPE,
		IN_COMMENT,
		IN_DOCTYPE,
		IN_PI,
		IN_TAG,
		IN_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ধ্রুবক uint8_t *end;

	যদি (তথ্য == ফাঁকা)
		ফেরত NSERROR_NEED_তথ্য;

	end = তথ্য + min(len, 512);

	যখন (তথ্য < end) {
		ধ্রুবক uint8_t c = *তথ্য;

#define MATCH(s) SLEN(s) <= (size_t) (end - তথ্য) && \
			memcmp(তথ্য, s, SLEN(s)) == 0

		নির্বাচন (state) {
		ক্ষেত্র BEFORE_BOM:
			যদি (3 <= end - তথ্য && c == 0xef && তথ্য[1] == 0xbb &&
					তথ্য[2] == 0xbf) {
				তথ্য += 3;
			}

			state = BEFORE_MARKUP;
			বিরতি;
		ক্ষেত্র BEFORE_MARKUP:
			যদি (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				তথ্য++;
			অন্যথা যদি (c != '<')
				তথ্য = end;
			অন্যথা {
				state = MARKUP_START;
				তথ্য++;
			}
			বিরতি;
		ক্ষেত্র MARKUP_START:
			যদি (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				তথ্য++;
			} অন্যথা যদি (c == '?') {
				state = IN_PI;
				তথ্য++;
			} অন্যথা {
				/* Reconsume input */
				state = IN_TAG;
			}
			বিরতি;
		ক্ষেত্র COMMENT_OR_DOCTYPE:
			যদি (2 <= end - তথ্য && c == '-' && তথ্য[1] == '-') {
				state = IN_COMMENT;
				তথ্য += 2;
			} অন্যথা {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			বিরতি;
		ক্ষেত্র IN_COMMENT:
			যদি (3 <= end - তথ্য && c == '-' && তথ্য[1] == '-' &&
					তথ্য[2] == '>') {
				state = BEFORE_MARKUP;
				তথ্য += 3;
			} অন্যথা
				তথ্য++;
			বিরতি;
		ক্ষেত্র IN_DOCTYPE:
			যদি (c == '>')
				state = BEFORE_MARKUP;
			তথ্য++;
			বিরতি;
		ক্ষেত্র IN_PI:
			যদি (2 <= end - তথ্য && c == '?' && তথ্য[1] == '>') {
				state = BEFORE_MARKUP;
				তথ্য += 2;
			} অন্যথা
				তথ্য++;
			বিরতি;
		ক্ষেত্র IN_TAG:
			যদি (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ফেরত NSERROR_OK;
			} অন্যথা যদি (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ফেরত NSERROR_OK;
			} অন্যথা যদি (MATCH("rdf:RDF")) {
				state = IN_RDF;
				তথ্য += SLEN("rdf:RDF");
			} অন্যথা
				তথ্য = end;
			বিরতি;
		ক্ষেত্র IN_RDF:
			যদি (MATCH(RSS_NS)) {
				rss = true;
				তথ্য += SLEN(RSS_NS);
			} অন্যথা যদি (MATCH(RDF_NS)) {
				rdf = true;
				তথ্য += SLEN(RDF_NS);
			} অন্যথা
				তথ্য++;

			যদি (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ফেরত NSERROR_OK;
			}

			বিরতি;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	ফেরত NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ধ্রুবক অক্ষর *content_type_header,
				 ধ্রুবক uint8_t *তথ্য,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	স্থির ধ্রুবক কাঠামো tt_s {
		ধ্রুবক অক্ষর *তথ্য;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ফাঁকা, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	ধ্রুবক কাঠামো tt_s *tt;
	bool match;
	nserror error;

	যদি (content_type_header == ফাঁকা) {
		যদি (sniff_allowed == false) {
			ফেরত NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		ফেরত mimesniff__compute_unknown(তথ্য, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	যদি (error != NSERROR_OK) {
		যদি (sniff_allowed == false)
			ফেরত NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		ফেরত mimesniff__compute_unknown(তথ্য, len, effective_type);
	}

	যদি (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ফেরত NSERROR_OK;
	}

	যদি (image_only) {
		lwc_string *official_type;

		যদি (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ফেরত NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ফেরত mimesniff__compute_image(official_type,
				তথ্য, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	জন্য (tt = text_types; tt->তথ্য != ফাঁকা; tt++) {
		যদি (tt->len == content_type_header_len &&
		    memcmp(tt->তথ্য,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			ফেরত mimesniff_compute_text_or_দ্বিমিক(তথ্য, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	যদি ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ফেরত mimesniff__compute_unknown(তথ্য, len, effective_type);
	}

	/* +xml */
	যদি (lwc_string_দৈর্ঘ্য(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_তথ্য(ct->media_type) +
				lwc_string_দৈর্ঘ্য(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ফেরত NSERROR_OK;
	}

	/* text/xml, application/xml */
	যদি ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ফেরত NSERROR_OK;
	}

	/* Image types */
	যদি (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ফেরত mimesniff__compute_image(official_type,
				তথ্য, len, effective_type);
	}

	/* text/html */
	যদি ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ফেরত mimesniff__compute_feed_or_html(তথ্য, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ফেরত NSERROR_OK;
}
