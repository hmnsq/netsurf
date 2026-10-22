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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>

#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/corestrings.h"

#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/mimesniff.h"

ഘടന map_s {
	സ്ഥിരം uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

സ്ഥിതം bool mimesniff_has_ദ്വിമാന_octets(സ്ഥിരം uint8_t *data, size_t len)
{
	സ്ഥിരം uint8_t *end = data + len;

	ആകുമ്പോൾ (data != end) {
		സ്ഥിരം uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		എങ്കിൽ (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			നിർത്തുക;

		data++;
	}

	തിരികെ data != end;
}

സ്ഥിതം nserror mimesniff__match_mp4(സ്ഥിരം uint8_t *data, size_t len,
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
	എങ്കിൽ (len < 12)
		തിരികെ NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	എങ്കിൽ (len < box_size || box_size % 4 != 0)
		തിരികെ NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	എങ്കിൽ (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		തിരികെ NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	എങ്കിൽ (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		തിരികെ NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	വേണ്ടി (i = 16; i <= box_size - 4; i += 4) {
		എങ്കിൽ (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			തിരികെ NSERROR_OK;
		}
	}

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__match_unknown_ws(സ്ഥിരം uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (സ്ഥിരം uint8_t *) s, SLEN(s), x, t }
	സ്ഥിതം സ്ഥിരം ഘടന map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ ശൂന്യ, 0, false, ശൂന്യ }
	};

	സ്ഥിതം സ്ഥിരം ഘടന map_s ws_inexact_match_types[] = {
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
		{ ശൂന്യ, 0, false, ശൂന്യ }
	};
#undef SIG
	സ്ഥിരം uint8_t *end = data + len;
	സ്ഥിരം ഘടന map_s *it;

	/* Skip leading whitespace */
	ആകുമ്പോൾ (data != end) {
		സ്ഥിരം uint8_t c = *data;

		എങ്കിൽ (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			നിർത്തുക;

		data++;
	}

	എങ്കിൽ (data == end)
		തിരികെ NSERROR_NOT_FOUND;

	len = end - data;

	വേണ്ടി (it = ws_exact_match_types; it->sig != ശൂന്യ; it++) {
		എങ്കിൽ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	വേണ്ടി (it = ws_inexact_match_types; it->sig != ശൂന്യ; it++) {
		/* +1 for trailing space or > */
		എങ്കിൽ (len < it->len + 1)
			തുടരുക;

		എങ്കിൽ (strncasecmp((സ്ഥിരം അക്ഷരം *) data,
				(സ്ഥിരം അക്ഷരം *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__match_unknown_bom(സ്ഥിരം uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (സ്ഥിരം uint8_t *) s, SLEN(s), x, t }
	സ്ഥിതം സ്ഥിരം ഘടന map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ ശൂന്യ, 0, false, ശൂന്യ }
	};
#undef SIG
	സ്ഥിരം ഘടന map_s *it;

	വേണ്ടി (it = bom_match_types; it->sig != ശൂന്യ; it++) {
		എങ്കിൽ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__match_unknown_riff(സ്ഥിരം uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (സ്ഥിരം uint8_t *) s, SLEN(s), x, t }
	സ്ഥിതം സ്ഥിരം ഘടന map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ശൂന്യ, 0, false, ശൂന്യ }
	};
#undef SIG
	സ്ഥിരം ഘടന map_s *it;

	വേണ്ടി (it = riff_match_types; it->sig != ശൂന്യ; it++) {
		എങ്കിൽ (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__match_unknown_exact(സ്ഥിരം uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (സ്ഥിരം uint8_t *) s, SLEN(s), x, t }
	സ്ഥിതം സ്ഥിരം ഘടന map_s exact_match_types[] = {
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
			(സ്ഥിരം uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ശൂന്യ, 0, false, ശൂന്യ }
	};
#undef SIG
	സ്ഥിരം ഘടന map_s *it;

	വേണ്ടി (it = exact_match_types; it->sig != ശൂന്യ; it++) {
		എങ്കിൽ (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__match_unknown(സ്ഥിരം uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	എങ്കിൽ (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	എങ്കിൽ (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	എങ്കിൽ (allow_unsafe == false)
		തിരികെ NSERROR_NOT_FOUND;

	എങ്കിൽ (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	എങ്കിൽ (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	എങ്കിൽ (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror mimesniff__compute_unknown(സ്ഥിരം uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	എങ്കിൽ (data == ശൂന്യ)
		തിരികെ NSERROR_NEED_DATA;

	len = min(len, 512);

	എങ്കിൽ (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (mimesniff_has_ദ്വിമാന_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		തിരികെ NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror mimesniff_compute_text_or_ദ്വിമാന(സ്ഥിരം uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	എങ്കിൽ (data == ശൂന്യ) {
		തിരികെ NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	എങ്കിൽ (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (mimesniff_has_ദ്വിമാന_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		തിരികെ NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror mimesniff__compute_image(lwc_string *official_type,
		സ്ഥിരം uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (സ്ഥിരം uint8_t *) s, SLEN(s), t }
	സ്ഥിതം സ്ഥിരം ഘടന it_s {
		സ്ഥിരം uint8_t *sig;
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
			(സ്ഥിരം uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ശൂന്യ, 0, ശൂന്യ }
	};
#undef SIG

	സ്ഥിരം ഘടന it_s *it;

	എങ്കിൽ (data == ശൂന്യ) {
		lwc_string_unref(official_type);
		തിരികെ NSERROR_NEED_DATA;
	}

	വേണ്ടി (it = image_types; it->sig != ശൂന്യ; it++) {
		എങ്കിൽ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			തിരികെ NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	എങ്കിൽ (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		തിരികെ NSERROR_OK;
	}

	*effective_type = official_type;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror mimesniff__compute_feed_or_html(സ്ഥിരം uint8_t *data,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	എണ്ണൽ state_e {
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
	സ്ഥിരം uint8_t *end;

	എങ്കിൽ (data == ശൂന്യ)
		തിരികെ NSERROR_NEED_DATA;

	end = data + min(len, 512);

	ആകുമ്പോൾ (data < end) {
		സ്ഥിരം uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		തിരഞ്ഞെടുപ്പ് (state) {
		അവസ്ഥ BEFORE_BOM:
			എങ്കിൽ (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			നിർത്തുക;
		അവസ്ഥ BEFORE_MARKUP:
			എങ്കിൽ (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			അല്ലെങ്കിൽ എങ്കിൽ (c != '<')
				data = end;
			അല്ലെങ്കിൽ {
				state = MARKUP_START;
				data++;
			}
			നിർത്തുക;
		അവസ്ഥ MARKUP_START:
			എങ്കിൽ (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} അല്ലെങ്കിൽ എങ്കിൽ (c == '?') {
				state = IN_PI;
				data++;
			} അല്ലെങ്കിൽ {
				/* Reconsume input */
				state = IN_TAG;
			}
			നിർത്തുക;
		അവസ്ഥ COMMENT_OR_DOCTYPE:
			എങ്കിൽ (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} അല്ലെങ്കിൽ {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			നിർത്തുക;
		അവസ്ഥ IN_COMMENT:
			എങ്കിൽ (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} അല്ലെങ്കിൽ
				data++;
			നിർത്തുക;
		അവസ്ഥ IN_DOCTYPE:
			എങ്കിൽ (c == '>')
				state = BEFORE_MARKUP;
			data++;
			നിർത്തുക;
		അവസ്ഥ IN_PI:
			എങ്കിൽ (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} അല്ലെങ്കിൽ
				data++;
			നിർത്തുക;
		അവസ്ഥ IN_TAG:
			എങ്കിൽ (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				തിരികെ NSERROR_OK;
			} അല്ലെങ്കിൽ എങ്കിൽ (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				തിരികെ NSERROR_OK;
			} അല്ലെങ്കിൽ എങ്കിൽ (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} അല്ലെങ്കിൽ
				data = end;
			നിർത്തുക;
		അവസ്ഥ IN_RDF:
			എങ്കിൽ (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} അല്ലെങ്കിൽ എങ്കിൽ (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} അല്ലെങ്കിൽ
				data++;

			എങ്കിൽ (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				തിരികെ NSERROR_OK;
			}

			നിർത്തുക;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	തിരികെ NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(സ്ഥിരം അക്ഷരം *content_type_header,
				 സ്ഥിരം uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	സ്ഥിതം സ്ഥിരം ഘടന tt_s {
		സ്ഥിരം അക്ഷരം *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ശൂന്യ, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	സ്ഥിരം ഘടന tt_s *tt;
	bool match;
	nserror error;

	എങ്കിൽ (content_type_header == ശൂന്യ) {
		എങ്കിൽ (sniff_allowed == false) {
			തിരികെ NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		തിരികെ mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	എങ്കിൽ (error != NSERROR_OK) {
		എങ്കിൽ (sniff_allowed == false)
			തിരികെ NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		തിരികെ mimesniff__compute_unknown(data, len, effective_type);
	}

	എങ്കിൽ (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (image_only) {
		lwc_string *official_type;

		എങ്കിൽ (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			തിരികെ NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		തിരികെ mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	വേണ്ടി (tt = text_types; tt->data != ശൂന്യ; tt++) {
		എങ്കിൽ (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			തിരികെ mimesniff_compute_text_or_ദ്വിമാന(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	എങ്കിൽ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		തിരികെ mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	എങ്കിൽ (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		തിരികെ NSERROR_OK;
	}

	/* text/xml, application/xml */
	എങ്കിൽ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		തിരികെ NSERROR_OK;
	}

	/* Image types */
	എങ്കിൽ (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		തിരികെ mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	എങ്കിൽ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		തിരികെ mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	തിരികെ NSERROR_OK;
}
