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

#include <string.h>
#include <strings.h>

#include "utils/http.h"
#include "utils/utils.h"
#include "utils/corestrings.h"

#include "content/content_factory.h"
#include "content/mimesniff.h"

sa_nracanaa map_s {
	const uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

static bool mimesniff__has_binary_octets(const uint8_t *data, size_t len)
{
	const uint8_t *end = data + len;

	སྐབས (data != end) {
		const uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		གལ་ཏེ (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			bha_nga;

		data++;
	}

	ཕྱིར data != end;
}

static nserror mimesniff__match_mp4(const uint8_t *data, size_t len,
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
	གལ་ཏེ (len < 12)
		ཕྱིར NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	གལ་ཏེ (len < box_size || box_size % 4 != 0)
		ཕྱིར NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	གལ་ཏེ (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		ཕྱིར NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	གལ་ཏེ (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ཕྱིར NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	ཆེད་དུ (i = 16; i <= box_size - 4; i += 4) {
		གལ་ཏེ (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ཕྱིར NSERROR_OK;
		}
	}

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__match_unknown_ws(const uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (const uint8_t *) s, SLEN(s), x, t }
	static const sa_nracanaa map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ NULL, 0, false, NULL }
	};

	static const sa_nracanaa map_s ws_inexact_match_types[] = {
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
		{ NULL, 0, false, NULL }
	};
#undef SIG
	const uint8_t *end = data + len;
	const sa_nracanaa map_s *it;

	/* Skip leading whitespace */
	སྐབས (data != end) {
		const uint8_t c = *data;

		གལ་ཏེ (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			bha_nga;

		data++;
	}

	གལ་ཏེ (data == end)
		ཕྱིར NSERROR_NOT_FOUND;

	len = end - data;

	ཆེད་དུ (it = ws_exact_match_types; it->sig != NULL; it++) {
		གལ་ཏེ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	ཆེད་དུ (it = ws_inexact_match_types; it->sig != NULL; it++) {
		/* +1 for trailing space or > */
		གལ་ཏེ (len < it->len + 1)
			continue;

		གལ་ཏེ (strncasecmp((const var_na *) data,
				(const var_na *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__match_unknown_bom(const uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (const uint8_t *) s, SLEN(s), x, t }
	static const sa_nracanaa map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ NULL, 0, false, NULL }
	};
#undef SIG
	const sa_nracanaa map_s *it;

	ཆེད་དུ (it = bom_match_types; it->sig != NULL; it++) {
		གལ་ཏེ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__match_unknown_riff(const uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (const uint8_t *) s, SLEN(s), x, t }
	static const sa_nracanaa map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ NULL, 0, false, NULL }
	};
#undef SIG
	const sa_nracanaa map_s *it;

	ཆེད་དུ (it = riff_match_types; it->sig != NULL; it++) {
		གལ་ཏེ (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__match_unknown_exact(const uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (const uint8_t *) s, SLEN(s), x, t }
	static const sa_nracanaa map_s exact_match_types[] = {
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
			(const uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ NULL, 0, false, NULL }
	};
#undef SIG
	const sa_nracanaa map_s *it;

	ཆེད་དུ (it = exact_match_types; it->sig != NULL; it++) {
		གལ་ཏེ (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__match_unknown(const uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	གལ་ཏེ (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	གལ་ཏེ (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	གལ་ཏེ (allow_unsafe == false)
		ཕྱིར NSERROR_NOT_FOUND;

	གལ་ཏེ (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	གལ་ཏེ (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	གལ་ཏེ (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	ཕྱིར NSERROR_NOT_FOUND;
}

static nserror mimesniff__compute_unknown(const uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	གལ་ཏེ (data == NULL)
		ཕྱིར NSERROR_NEED_DATA;

	len = min(len, 512);

	གལ་ཏེ (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		ཕྱིར NSERROR_OK;
	}

	གལ་ཏེ (mimesniff__has_binary_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ཕྱིར NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ཕྱིར NSERROR_OK;
}

static nserror mimesniff__compute_text_or_binary(const uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	གལ་ཏེ (data == NULL) {
		ཕྱིར NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	གལ་ཏེ (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ཕྱིར NSERROR_OK;
	}

	གལ་ཏེ (mimesniff__has_binary_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ཕྱིར NSERROR_OK;
	}

	གལ་ཏེ (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		ཕྱིར NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ཕྱིར NSERROR_OK;
}

static nserror mimesniff__compute_image(lwc_string *official_type,
		const uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (const uint8_t *) s, SLEN(s), t }
	static const sa_nracanaa it_s {
		const uint8_t *sig;
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
			(const uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ NULL, 0, NULL }
	};
#undef SIG

	const sa_nracanaa it_s *it;

	གལ་ཏེ (data == NULL) {
		lwc_string_unref(official_type);
		ཕྱིར NSERROR_NEED_DATA;
	}

	ཆེད་དུ (it = image_types; it->sig != NULL; it++) {
		གལ་ཏེ (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ཕྱིར NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	གལ་ཏེ (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ཕྱིར NSERROR_OK;
	}

	*effective_type = official_type;

	ཕྱིར NSERROR_OK;
}

static nserror mimesniff__compute_feed_or_html(const uint8_t *data,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	ga_nanaa state_e {
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
	const uint8_t *end;

	གལ་ཏེ (data == NULL)
		ཕྱིར NSERROR_NEED_DATA;

	end = data + min(len, 512);

	སྐབས (data < end) {
		const uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		switch (state) {
		pak_sa BEFORE_BOM:
			གལ་ཏེ (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			bha_nga;
		pak_sa BEFORE_MARKUP:
			གལ་ཏེ (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			དེ་མིན གལ་ཏེ (c != '<')
				data = end;
			དེ་མིན {
				state = MARKUP_START;
				data++;
			}
			bha_nga;
		pak_sa MARKUP_START:
			གལ་ཏེ (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} དེ་མིན གལ་ཏེ (c == '?') {
				state = IN_PI;
				data++;
			} དེ་མིན {
				/* Reconsume input */
				state = IN_TAG;
			}
			bha_nga;
		pak_sa COMMENT_OR_DOCTYPE:
			གལ་ཏེ (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} དེ་མིན {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			bha_nga;
		pak_sa IN_COMMENT:
			གལ་ཏེ (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} དེ་མིན
				data++;
			bha_nga;
		pak_sa IN_DOCTYPE:
			གལ་ཏེ (c == '>')
				state = BEFORE_MARKUP;
			data++;
			bha_nga;
		pak_sa IN_PI:
			གལ་ཏེ (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} དེ་མིན
				data++;
			bha_nga;
		pak_sa IN_TAG:
			གལ་ཏེ (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ཕྱིར NSERROR_OK;
			} དེ་མིན གལ་ཏེ (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ཕྱིར NSERROR_OK;
			} དེ་མིན གལ་ཏེ (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} དེ་མིན
				data = end;
			bha_nga;
		pak_sa IN_RDF:
			གལ་ཏེ (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} དེ་མིན གལ་ཏེ (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} དེ་མིན
				data++;

			གལ་ཏེ (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ཕྱིར NSERROR_OK;
			}

			bha_nga;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	ཕྱིར NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(const var_na *content_type_header,
				 const uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	static const sa_nracanaa tt_s {
		const var_na *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ NULL, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	const sa_nracanaa tt_s *tt;
	bool match;
	nserror error;

	གལ་ཏེ (content_type_header == NULL) {
		གལ་ཏེ (sniff_allowed == false) {
			ཕྱིར NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		ཕྱིར mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	གལ་ཏེ (error != NSERROR_OK) {
		གལ་ཏེ (sniff_allowed == false)
			ཕྱིར NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		ཕྱིར mimesniff__compute_unknown(data, len, effective_type);
	}

	གལ་ཏེ (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ཕྱིར NSERROR_OK;
	}

	གལ་ཏེ (image_only) {
		lwc_string *official_type;

		གལ་ཏེ (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ཕྱིར NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ཕྱིར mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	ཆེད་དུ (tt = text_types; tt->data != NULL; tt++) {
		གལ་ཏེ (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			ཕྱིར mimesniff__compute_text_or_binary(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	གལ་ཏེ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ཕྱིར mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	གལ་ཏེ (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ཕྱིར NSERROR_OK;
	}

	/* text/xml, application/xml */
	གལ་ཏེ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ཕྱིར NSERROR_OK;
	}

	/* Image types */
	གལ་ཏེ (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ཕྱིར mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	གལ་ཏེ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ཕྱིར mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ཕྱིར NSERROR_OK;
}
