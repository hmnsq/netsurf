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

#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>

#অন্তর্ভাৱ "utils/http.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/corestrings.h"

#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "content/mimesniff.h"

সংরচনা map_s {
	ধ্রুৱ uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

স্থির bool mimesniff_has_দ্বিমিক_octets(ধ্রুৱ uint8_t *data, size_t len)
{
	ধ্রুৱ uint8_t *end = data + len;

	যেতিয়ালৈ (data != end) {
		ধ্রুৱ uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		যদি (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			ভঙ্গ;

		data++;
	}

	ঘূৰাই data != end;
}

স্থির nserror mimesniff__match_mp4(ধ্রুৱ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_size, i;

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
		ঘূৰাই NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	যদি (len < box_size || box_size % 4 != 0)
		ঘূৰাই NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	যদি (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		ঘূৰাই NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	যদি (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ঘূৰাই NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	বাবে (i = 16; i <= box_size - 4; i += 4) {
		যদি (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ঘূৰাই NSERROR_OK;
		}
	}

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_ws(ধ্রুৱ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুৱ uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুৱ সংরচনা map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ রিক্ত, 0, false, রিক্ত }
	};

	স্থির ধ্রুৱ সংরচনা map_s ws_inexact_match_types[] = {
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
		{ রিক্ত, 0, false, রিক্ত }
	};
#undef SIG
	ধ্রুৱ uint8_t *end = data + len;
	ধ্রুৱ সংরচনা map_s *it;

	/* Skip leading whitespace */
	যেতিয়ালৈ (data != end) {
		ধ্রুৱ uint8_t c = *data;

		যদি (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			ভঙ্গ;

		data++;
	}

	যদি (data == end)
		ঘূৰাই NSERROR_NOT_FOUND;

	len = end - data;

	বাবে (it = ws_exact_match_types; it->sig != রিক্ত; it++) {
		যদি (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	বাবে (it = ws_inexact_match_types; it->sig != রিক্ত; it++) {
		/* +1 for trailing space or > */
		যদি (len < it->len + 1)
			অনুৱর্ত;

		যদি (strncasecmp((ধ্রুৱ ৱর্ণ *) data,
				(ধ্রুৱ ৱর্ণ *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_bom(ধ্রুৱ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুৱ uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুৱ সংরচনা map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ রিক্ত, 0, false, রিক্ত }
	};
#undef SIG
	ধ্রুৱ সংরচনা map_s *it;

	বাবে (it = bom_match_types; it->sig != রিক্ত; it++) {
		যদি (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_riff(ধ্রুৱ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুৱ uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুৱ সংরচনা map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ রিক্ত, 0, false, রিক্ত }
	};
#undef SIG
	ধ্রুৱ সংরচনা map_s *it;

	বাবে (it = riff_match_types; it->sig != রিক্ত; it++) {
		যদি (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown_exact(ধ্রুৱ uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (ধ্রুৱ uint8_t *) s, SLEN(s), x, t }
	স্থির ধ্রুৱ সংরচনা map_s exact_match_types[] = {
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
			(ধ্রুৱ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ রিক্ত, 0, false, রিক্ত }
	};
#undef SIG
	ধ্রুৱ সংরচনা map_s *it;

	বাবে (it = exact_match_types; it->sig != রিক্ত; it++) {
		যদি (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__match_unknown(ধ্রুৱ uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	যদি (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	যদি (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	যদি (allow_unsafe == false)
		ঘূৰাই NSERROR_NOT_FOUND;

	যদি (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	যদি (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	যদি (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	ঘূৰাই NSERROR_NOT_FOUND;
}

স্থির nserror mimesniff__compute_unknown(ধ্রুৱ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	যদি (data == রিক্ত)
		ঘূৰাই NSERROR_NEED_DATA;

	len = min(len, 512);

	যদি (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		ঘূৰাই NSERROR_OK;
	}

	যদি (mimesniff_has_দ্বিমিক_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ঘূৰাই NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ঘূৰাই NSERROR_OK;
}

স্থির nserror mimesniff_compute_text_or_দ্বিমিক(ধ্রুৱ uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	যদি (data == রিক্ত) {
		ঘূৰাই NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	যদি (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ঘূৰাই NSERROR_OK;
	}

	যদি (mimesniff_has_দ্বিমিক_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ঘূৰাই NSERROR_OK;
	}

	যদি (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		ঘূৰাই NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ঘূৰাই NSERROR_OK;
}

স্থির nserror mimesniff__compute_image(lwc_string *official_type,
		ধ্রুৱ uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (ধ্রুৱ uint8_t *) s, SLEN(s), t }
	স্থির ধ্রুৱ সংরচনা it_s {
		ধ্রুৱ uint8_t *sig;
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
			(ধ্রুৱ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ রিক্ত, 0, রিক্ত }
	};
#undef SIG

	ধ্রুৱ সংরচনা it_s *it;

	যদি (data == রিক্ত) {
		lwc_string_unref(official_type);
		ঘূৰাই NSERROR_NEED_DATA;
	}

	বাবে (it = image_types; it->sig != রিক্ত; it++) {
		যদি (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ঘূৰাই NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	যদি (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ঘূৰাই NSERROR_OK;
	}

	*effective_type = official_type;

	ঘূৰাই NSERROR_OK;
}

স্থির nserror mimesniff__compute_feed_or_html(ধ্রুৱ uint8_t *data,
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
	ধ্রুৱ uint8_t *end;

	যদি (data == রিক্ত)
		ঘূৰাই NSERROR_NEED_DATA;

	end = data + min(len, 512);

	যেতিয়ালৈ (data < end) {
		ধ্রুৱ uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		ৱিকল্প (state) {
		পক্ষ BEFORE_BOM:
			যদি (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			ভঙ্গ;
		পক্ষ BEFORE_MARKUP:
			যদি (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			অন্যথা যদি (c != '<')
				data = end;
			অন্যথা {
				state = MARKUP_START;
				data++;
			}
			ভঙ্গ;
		পক্ষ MARKUP_START:
			যদি (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} অন্যথা যদি (c == '?') {
				state = IN_PI;
				data++;
			} অন্যথা {
				/* Reconsume input */
				state = IN_TAG;
			}
			ভঙ্গ;
		পক্ষ COMMENT_OR_DOCTYPE:
			যদি (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} অন্যথা {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			ভঙ্গ;
		পক্ষ IN_COMMENT:
			যদি (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} অন্যথা
				data++;
			ভঙ্গ;
		পক্ষ IN_DOCTYPE:
			যদি (c == '>')
				state = BEFORE_MARKUP;
			data++;
			ভঙ্গ;
		পক্ষ IN_PI:
			যদি (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} অন্যথা
				data++;
			ভঙ্গ;
		পক্ষ IN_TAG:
			যদি (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ঘূৰাই NSERROR_OK;
			} অন্যথা যদি (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ঘূৰাই NSERROR_OK;
			} অন্যথা যদি (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} অন্যথা
				data = end;
			ভঙ্গ;
		পক্ষ IN_RDF:
			যদি (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} অন্যথা যদি (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} অন্যথা
				data++;

			যদি (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ঘূৰাই NSERROR_OK;
			}

			ভঙ্গ;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	ঘূৰাই NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ধ্রুৱ ৱর্ণ *content_type_header,
				 ধ্রুৱ uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	স্থির ধ্রুৱ সংরচনা tt_s {
		ধ্রুৱ ৱর্ণ *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ রিক্ত, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	ধ্রুৱ সংরচনা tt_s *tt;
	bool match;
	nserror error;

	যদি (content_type_header == রিক্ত) {
		যদি (sniff_allowed == false) {
			ঘূৰাই NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		ঘূৰাই mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	যদি (error != NSERROR_OK) {
		যদি (sniff_allowed == false)
			ঘূৰাই NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		ঘূৰাই mimesniff__compute_unknown(data, len, effective_type);
	}

	যদি (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ঘূৰাই NSERROR_OK;
	}

	যদি (image_only) {
		lwc_string *official_type;

		যদি (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ঘূৰাই NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ঘূৰাই mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	বাবে (tt = text_types; tt->data != রিক্ত; tt++) {
		যদি (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			ঘূৰাই mimesniff_compute_text_or_দ্বিমিক(data, len,
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
		ঘূৰাই mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	যদি (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ঘূৰাই NSERROR_OK;
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
		ঘূৰাই NSERROR_OK;
	}

	/* Image types */
	যদি (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ঘূৰাই mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	যদি ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ঘূৰাই mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ঘূৰাই NSERROR_OK;
}
