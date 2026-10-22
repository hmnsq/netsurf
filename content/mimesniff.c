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
	bool safe;
	lwc_string **type;
};

ساكن bool mimesniff_has_ئىككىلىك_octets(ثابت uint8_t *data, size_t len)
{
	ثابت uint8_t *end = data + len;

	طالما (data != end) {
		ثابت uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		إذا (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			اكسر;

		data++;
	}

	أرجع data != end;
}

ساكن nserror mimesniff__match_mp4(ثابت uint8_t *data, size_t len,
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
	إذا (len < 12)
		أرجع NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	إذا (len < box_size || box_size % 4 != 0)
		أرجع NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	إذا (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		أرجع NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	إذا (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		أرجع NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	لكل (i = 16; i <= box_size - 4; i += 4) {
		إذا (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__match_unknown_ws(ثابت uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ عدم, 0, false, عدم }
	};

	ساكن ثابت بنية map_s ws_inexact_match_types[] = {
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
		{ عدم, 0, false, عدم }
	};
#undef SIG
	ثابت uint8_t *end = data + len;
	ثابت بنية map_s *it;

	/* Skip leading whitespace */
	طالما (data != end) {
		ثابت uint8_t c = *data;

		إذا (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			اكسر;

		data++;
	}

	إذا (data == end)
		أرجع NSERROR_NOT_FOUND;

	len = end - data;

	لكل (it = ws_exact_match_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	لكل (it = ws_inexact_match_types; it->sig != عدم; it++) {
		/* +1 for trailing space or > */
		إذا (len < it->len + 1)
			استمر;

		إذا (strncasecmp((ثابت حرف *) data,
				(ثابت حرف *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__match_unknown_bom(ثابت uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ عدم, 0, false, عدم }
	};
#undef SIG
	ثابت بنية map_s *it;

	لكل (it = bom_match_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__match_unknown_riff(ثابت uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ عدم, 0, false, عدم }
	};
#undef SIG
	ثابت بنية map_s *it;

	لكل (it = riff_match_types; it->sig != عدم; it++) {
		إذا (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__match_unknown_exact(ثابت uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (ثابت uint8_t *) s, SLEN(s), x, t }
	ساكن ثابت بنية map_s exact_match_types[] = {
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
#undef SIG
	ثابت بنية map_s *it;

	لكل (it = exact_match_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__match_unknown(ثابت uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	إذا (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (allow_unsafe == false)
		أرجع NSERROR_NOT_FOUND;

	إذا (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	إذا (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	أرجع NSERROR_NOT_FOUND;
}

ساكن nserror mimesniff__compute_unknown(ثابت uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	إذا (data == عدم)
		أرجع NSERROR_NEED_DATA;

	len = min(len, 512);

	إذا (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		أرجع NSERROR_OK;
	}

	إذا (mimesniff_has_ئىككىلىك_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		أرجع NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff_compute_text_or_ئىككىلىك(ثابت uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	إذا (data == عدم) {
		أرجع NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	إذا (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		أرجع NSERROR_OK;
	}

	إذا (mimesniff_has_ئىككىلىك_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		أرجع NSERROR_OK;
	}

	إذا (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		أرجع NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff__compute_image(lwc_string *official_type,
		ثابت uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (ثابت uint8_t *) s, SLEN(s), t }
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
#undef SIG

	ثابت بنية it_s *it;

	إذا (data == عدم) {
		lwc_string_unref(official_type);
		أرجع NSERROR_NEED_DATA;
	}

	لكل (it = image_types; it->sig != عدم; it++) {
		إذا (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			أرجع NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	إذا (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		أرجع NSERROR_OK;
	}

	*effective_type = official_type;

	أرجع NSERROR_OK;
}

ساكن nserror mimesniff__compute_feed_or_html(ثابت uint8_t *data,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	تعداد state_e {
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
	ثابت uint8_t *end;

	إذا (data == عدم)
		أرجع NSERROR_NEED_DATA;

	end = data + min(len, 512);

	طالما (data < end) {
		ثابت uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		بدل (state) {
		حالة BEFORE_BOM:
			إذا (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			اكسر;
		حالة BEFORE_MARKUP:
			إذا (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			وإلا إذا (c != '<')
				data = end;
			وإلا {
				state = MARKUP_START;
				data++;
			}
			اكسر;
		حالة MARKUP_START:
			إذا (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} وإلا إذا (c == '?') {
				state = IN_PI;
				data++;
			} وإلا {
				/* Reconsume input */
				state = IN_TAG;
			}
			اكسر;
		حالة COMMENT_OR_DOCTYPE:
			إذا (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} وإلا {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			اكسر;
		حالة IN_COMMENT:
			إذا (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} وإلا
				data++;
			اكسر;
		حالة IN_DOCTYPE:
			إذا (c == '>')
				state = BEFORE_MARKUP;
			data++;
			اكسر;
		حالة IN_PI:
			إذا (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} وإلا
				data++;
			اكسر;
		حالة IN_TAG:
			إذا (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				أرجع NSERROR_OK;
			} وإلا إذا (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				أرجع NSERROR_OK;
			} وإلا إذا (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} وإلا
				data = end;
			اكسر;
		حالة IN_RDF:
			إذا (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} وإلا إذا (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} وإلا
				data++;

			إذا (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				أرجع NSERROR_OK;
			}

			اكسر;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	أرجع NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ثابت حرف *content_type_header,
				 ثابت uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	ساكن ثابت بنية tt_s {
		ثابت حرف *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ عدم, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	ثابت بنية tt_s *tt;
	bool match;
	nserror error;

	إذا (content_type_header == عدم) {
		إذا (sniff_allowed == false) {
			أرجع NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		أرجع mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	إذا (error != NSERROR_OK) {
		إذا (sniff_allowed == false)
			أرجع NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		أرجع mimesniff__compute_unknown(data, len, effective_type);
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
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			أرجع NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	لكل (tt = text_types; tt->data != عدم; tt++) {
		إذا (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			أرجع mimesniff_compute_text_or_ئىككىلىك(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	إذا (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		أرجع NSERROR_OK;
	}

	/* text/xml, application/xml */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
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
				data, len, effective_type);
	}

	/* text/html */
	إذا ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		أرجع mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	أرجع NSERROR_OK;
}
