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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>

#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/corestrings.h"

#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/mimesniff.h"

கட்டமைப்பு map_s {
	மாறிலி uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

நிலையான bool mimesniff_has_இருமம்_octets(மாறிலி uint8_t *data, size_t len)
{
	மாறிலி uint8_t *end = data + len;

	போது (data != end) {
		மாறிலி uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		எனில் (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			நிறுத்து;

		data++;
	}

	திரும்பு data != end;
}

நிலையான nserror mimesniff__match_mp4(மாறிலி uint8_t *data, size_t len,
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
	எனில் (len < 12)
		திரும்பு NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	எனில் (len < box_size || box_size % 4 != 0)
		திரும்பு NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	எனில் (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		திரும்பு NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	எனில் (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		திரும்பு NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	க்கு (i = 16; i <= box_size - 4; i += 4) {
		எனில் (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			திரும்பு NSERROR_OK;
		}
	}

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__match_unknown_ws(மாறிலி uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (மாறிலி uint8_t *) s, SLEN(s), x, t }
	நிலையான மாறிலி கட்டமைப்பு map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ வெற்று, 0, false, வெற்று }
	};

	நிலையான மாறிலி கட்டமைப்பு map_s ws_inexact_match_types[] = {
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
		{ வெற்று, 0, false, வெற்று }
	};
#undef SIG
	மாறிலி uint8_t *end = data + len;
	மாறிலி கட்டமைப்பு map_s *it;

	/* Skip leading whitespace */
	போது (data != end) {
		மாறிலி uint8_t c = *data;

		எனில் (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			நிறுத்து;

		data++;
	}

	எனில் (data == end)
		திரும்பு NSERROR_NOT_FOUND;

	len = end - data;

	க்கு (it = ws_exact_match_types; it->sig != வெற்று; it++) {
		எனில் (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	க்கு (it = ws_inexact_match_types; it->sig != வெற்று; it++) {
		/* +1 for trailing space or > */
		எனில் (len < it->len + 1)
			தொடர்;

		எனில் (strncasecmp((மாறிலி எழுத்து *) data,
				(மாறிலி எழுத்து *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__match_unknown_bom(மாறிலி uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (மாறிலி uint8_t *) s, SLEN(s), x, t }
	நிலையான மாறிலி கட்டமைப்பு map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ வெற்று, 0, false, வெற்று }
	};
#undef SIG
	மாறிலி கட்டமைப்பு map_s *it;

	க்கு (it = bom_match_types; it->sig != வெற்று; it++) {
		எனில் (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__match_unknown_riff(மாறிலி uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (மாறிலி uint8_t *) s, SLEN(s), x, t }
	நிலையான மாறிலி கட்டமைப்பு map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ வெற்று, 0, false, வெற்று }
	};
#undef SIG
	மாறிலி கட்டமைப்பு map_s *it;

	க்கு (it = riff_match_types; it->sig != வெற்று; it++) {
		எனில் (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__match_unknown_exact(மாறிலி uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (மாறிலி uint8_t *) s, SLEN(s), x, t }
	நிலையான மாறிலி கட்டமைப்பு map_s exact_match_types[] = {
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
			(மாறிலி uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ வெற்று, 0, false, வெற்று }
	};
#undef SIG
	மாறிலி கட்டமைப்பு map_s *it;

	க்கு (it = exact_match_types; it->sig != வெற்று; it++) {
		எனில் (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__match_unknown(மாறிலி uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	எனில் (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	எனில் (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	எனில் (allow_unsafe == false)
		திரும்பு NSERROR_NOT_FOUND;

	எனில் (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	எனில் (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	எனில் (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	திரும்பு NSERROR_NOT_FOUND;
}

நிலையான nserror mimesniff__compute_unknown(மாறிலி uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	எனில் (data == வெற்று)
		திரும்பு NSERROR_NEED_DATA;

	len = min(len, 512);

	எனில் (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		திரும்பு NSERROR_OK;
	}

	எனில் (mimesniff_has_இருமம்_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		திரும்பு NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	திரும்பு NSERROR_OK;
}

நிலையான nserror mimesniff_compute_text_or_இருமம்(மாறிலி uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	எனில் (data == வெற்று) {
		திரும்பு NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	எனில் (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		திரும்பு NSERROR_OK;
	}

	எனில் (mimesniff_has_இருமம்_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		திரும்பு NSERROR_OK;
	}

	எனில் (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		திரும்பு NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	திரும்பு NSERROR_OK;
}

நிலையான nserror mimesniff__compute_image(lwc_string *official_type,
		மாறிலி uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (மாறிலி uint8_t *) s, SLEN(s), t }
	நிலையான மாறிலி கட்டமைப்பு it_s {
		மாறிலி uint8_t *sig;
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
			(மாறிலி uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ வெற்று, 0, வெற்று }
	};
#undef SIG

	மாறிலி கட்டமைப்பு it_s *it;

	எனில் (data == வெற்று) {
		lwc_string_unref(official_type);
		திரும்பு NSERROR_NEED_DATA;
	}

	க்கு (it = image_types; it->sig != வெற்று; it++) {
		எனில் (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			திரும்பு NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	எனில் (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		திரும்பு NSERROR_OK;
	}

	*effective_type = official_type;

	திரும்பு NSERROR_OK;
}

நிலையான nserror mimesniff__compute_feed_or_html(மாறிலி uint8_t *data,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	எண்ணிக்கை state_e {
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
	மாறிலி uint8_t *end;

	எனில் (data == வெற்று)
		திரும்பு NSERROR_NEED_DATA;

	end = data + min(len, 512);

	போது (data < end) {
		மாறிலி uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		தேர்வு (state) {
		நிலை BEFORE_BOM:
			எனில் (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			நிறுத்து;
		நிலை BEFORE_MARKUP:
			எனில் (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			இல்லையெனில் எனில் (c != '<')
				data = end;
			இல்லையெனில் {
				state = MARKUP_START;
				data++;
			}
			நிறுத்து;
		நிலை MARKUP_START:
			எனில் (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} இல்லையெனில் எனில் (c == '?') {
				state = IN_PI;
				data++;
			} இல்லையெனில் {
				/* Reconsume input */
				state = IN_TAG;
			}
			நிறுத்து;
		நிலை COMMENT_OR_DOCTYPE:
			எனில் (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} இல்லையெனில் {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			நிறுத்து;
		நிலை IN_COMMENT:
			எனில் (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} இல்லையெனில்
				data++;
			நிறுத்து;
		நிலை IN_DOCTYPE:
			எனில் (c == '>')
				state = BEFORE_MARKUP;
			data++;
			நிறுத்து;
		நிலை IN_PI:
			எனில் (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} இல்லையெனில்
				data++;
			நிறுத்து;
		நிலை IN_TAG:
			எனில் (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				திரும்பு NSERROR_OK;
			} இல்லையெனில் எனில் (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				திரும்பு NSERROR_OK;
			} இல்லையெனில் எனில் (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} இல்லையெனில்
				data = end;
			நிறுத்து;
		நிலை IN_RDF:
			எனில் (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} இல்லையெனில் எனில் (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} இல்லையெனில்
				data++;

			எனில் (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				திரும்பு NSERROR_OK;
			}

			நிறுத்து;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	திரும்பு NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(மாறிலி எழுத்து *content_type_header,
				 மாறிலி uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	நிலையான மாறிலி கட்டமைப்பு tt_s {
		மாறிலி எழுத்து *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ வெற்று, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	மாறிலி கட்டமைப்பு tt_s *tt;
	bool match;
	nserror error;

	எனில் (content_type_header == வெற்று) {
		எனில் (sniff_allowed == false) {
			திரும்பு NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		திரும்பு mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	எனில் (error != NSERROR_OK) {
		எனில் (sniff_allowed == false)
			திரும்பு NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		திரும்பு mimesniff__compute_unknown(data, len, effective_type);
	}

	எனில் (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		திரும்பு NSERROR_OK;
	}

	எனில் (image_only) {
		lwc_string *official_type;

		எனில் (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			திரும்பு NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		திரும்பு mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	க்கு (tt = text_types; tt->data != வெற்று; tt++) {
		எனில் (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			திரும்பு mimesniff_compute_text_or_இருமம்(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	எனில் ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		திரும்பு mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	எனில் (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		திரும்பு NSERROR_OK;
	}

	/* text/xml, application/xml */
	எனில் ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		திரும்பு NSERROR_OK;
	}

	/* Image types */
	எனில் (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		திரும்பு mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	எனில் ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		திரும்பு mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	திரும்பு NSERROR_OK;
}
