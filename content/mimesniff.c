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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>

#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"

#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/mimesniff.h"

ସଂରଚନା map_s {
	ଅଚଳ uint8_t *sig;
	size_t len;
	bool safe;
	lwc_string **type;
};

ସ୍ଥିର bool mimesniff_has_ଦ୍ୱିମିକ_octets(ଅଚଳ uint8_t *data, size_t len)
{
	ଅଚଳ uint8_t *end = data + len;

	ଯେତେବେଳେ (data != end) {
		ଅଚଳ uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		ଯଦି (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			ଭାଙ୍ଗ;

		data++;
	}

	ଫେରନ୍ତୁ data != end;
}

ସ୍ଥିର nserror mimesniff__match_mp4(ଅଚଳ uint8_t *data, size_t len,
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
	ଯଦି (len < 12)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	ଯଦି (len < box_size || box_size % 4 != 0)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	ଯଦି (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	ଯଦି (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	ପାଇଁ (i = 16; i <= box_size - 4; i += 4) {
		ଯଦି (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__match_unknown_ws(ଅଚଳ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ଅଚଳ uint8_t *) s, SLEN(s), x, t }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ ଖାଲି, 0, false, ଖାଲି }
	};

	ସ୍ଥିର ଅଚଳ ସଂରଚନା map_s ws_inexact_match_types[] = {
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
		{ ଖାଲି, 0, false, ଖାଲି }
	};
#undef SIG
	ଅଚଳ uint8_t *end = data + len;
	ଅଚଳ ସଂରଚନା map_s *it;

	/* Skip leading whitespace */
	ଯେତେବେଳେ (data != end) {
		ଅଚଳ uint8_t c = *data;

		ଯଦି (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			ଭାଙ୍ଗ;

		data++;
	}

	ଯଦି (data == end)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	len = end - data;

	ପାଇଁ (it = ws_exact_match_types; it->sig != ଖାଲି; it++) {
		ଯଦି (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ପାଇଁ (it = ws_inexact_match_types; it->sig != ଖାଲି; it++) {
		/* +1 for trailing space or > */
		ଯଦି (len < it->len + 1)
			ଜାରି;

		ଯଦି (strncasecmp((ଅଚଳ ଅକ୍ଷର *) data,
				(ଅଚଳ ଅକ୍ଷର *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__match_unknown_bom(ଅଚଳ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ଅଚଳ uint8_t *) s, SLEN(s), x, t }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ ଖାଲି, 0, false, ଖାଲି }
	};
#undef SIG
	ଅଚଳ ସଂରଚନା map_s *it;

	ପାଇଁ (it = bom_match_types; it->sig != ଖାଲି; it++) {
		ଯଦି (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__match_unknown_riff(ଅଚଳ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (ଅଚଳ uint8_t *) s, SLEN(s), x, t }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ଖାଲି, 0, false, ଖାଲି }
	};
#undef SIG
	ଅଚଳ ସଂରଚନା map_s *it;

	ପାଇଁ (it = riff_match_types; it->sig != ଖାଲି; it++) {
		ଯଦି (it->len + SLEN("RIFF????") <= len &&
				memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__match_unknown_exact(ଅଚଳ uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (ଅଚଳ uint8_t *) s, SLEN(s), x, t }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା map_s exact_match_types[] = {
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
			(ଅଚଳ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ଖାଲି, 0, false, ଖାଲି }
	};
#undef SIG
	ଅଚଳ ସଂରଚନା map_s *it;

	ପାଇଁ (it = exact_match_types; it->sig != ଖାଲି; it++) {
		ଯଦି (it->len <= len && memcmp(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__match_unknown(ଅଚଳ uint8_t *data, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	ଯଦି (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	ଯଦି (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	ଯଦି (allow_unsafe == false)
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	ଯଦି (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	ଯଦି (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	ଯଦି (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
}

ସ୍ଥିର nserror mimesniff__compute_unknown(ଅଚଳ uint8_t *data, size_t len,
		lwc_string **effective_type)
{
	ଯଦି (data == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;

	len = min(len, 512);

	ଯଦି (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (mimesniff_has_ଦ୍ୱିମିକ_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror mimesniff_compute_text_or_ଦ୍ୱିମିକ(ଅଚଳ uint8_t *data,
		size_t len, lwc_string **effective_type)
{
	ଯଦି (data == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	ଯଦି (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (mimesniff_has_ଦ୍ୱିମିକ_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		ଫେରନ୍ତୁ NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror mimesniff__compute_image(lwc_string *official_type,
		ଅଚଳ uint8_t *data, size_t len, lwc_string **effective_type)
{
#define SIG(t, s) { (ଅଚଳ uint8_t *) s, SLEN(s), t }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା it_s {
		ଅଚଳ uint8_t *sig;
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
			(ଅଚଳ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ଖାଲି, 0, ଖାଲି }
	};
#undef SIG

	ଅଚଳ ସଂରଚନା it_s *it;

	ଯଦି (data == ଖାଲି) {
		lwc_string_unref(official_type);
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;
	}

	ପାଇଁ (it = image_types; it->sig != ଖାଲି; it++) {
		ଯଦି (it->len <= len && memcmp(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	ଯଦି (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(data, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	*effective_type = official_type;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror mimesniff__compute_feed_or_html(ଅଚଳ uint8_t *data,
		size_t len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	ଗଣନା state_e {
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
	ଅଚଳ uint8_t *end;

	ଯଦି (data == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;

	end = data + min(len, 512);

	ଯେତେବେଳେ (data < end) {
		ଅଚଳ uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (size_t) (end - data) && \
			memcmp(data, s, SLEN(s)) == 0

		ବାଛ (state) {
		ସ୍ଥିତି BEFORE_BOM:
			ଯଦି (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BEFORE_MARKUP:
			ଯଦି (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			ଅନ୍ୟଥା ଯଦି (c != '<')
				data = end;
			ଅନ୍ୟଥା {
				state = MARKUP_START;
				data++;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି MARKUP_START:
			ଯଦି (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} ଅନ୍ୟଥା ଯଦି (c == '?') {
				state = IN_PI;
				data++;
			} ଅନ୍ୟଥା {
				/* Reconsume input */
				state = IN_TAG;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି COMMENT_OR_DOCTYPE:
			ଯଦି (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} ଅନ୍ୟଥା {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IN_COMMENT:
			ଯଦି (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} ଅନ୍ୟଥା
				data++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IN_DOCTYPE:
			ଯଦି (c == '>')
				state = BEFORE_MARKUP;
			data++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IN_PI:
			ଯଦି (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} ଅନ୍ୟଥା
				data++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IN_TAG:
			ଯଦି (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ଫେରନ୍ତୁ NSERROR_OK;
			} ଅନ୍ୟଥା ଯଦି (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ଫେରନ୍ତୁ NSERROR_OK;
			} ଅନ୍ୟଥା ଯଦି (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} ଅନ୍ୟଥା
				data = end;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି IN_RDF:
			ଯଦି (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} ଅନ୍ୟଥା ଯଦି (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} ଅନ୍ୟଥା
				data++;

			ଯଦି (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ଫେରନ୍ତୁ NSERROR_OK;
			}

			ଭାଙ୍ଗ;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	ଫେରନ୍ତୁ NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ଅଚଳ ଅକ୍ଷର *content_type_header,
				 ଅଚଳ uint8_t *data,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା tt_s {
		ଅଚଳ ଅକ୍ଷର *data;
		size_t len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ଖାଲି, 0 }
	};
#undef S

	size_t content_type_header_len;
	http_content_type *ct;
	ଅଚଳ ସଂରଚନା tt_s *tt;
	bool match;
	nserror error;

	ଯଦି (content_type_header == ଖାଲି) {
		ଯଦି (sniff_allowed == false) {
			ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		ଫେରନ୍ତୁ mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	ଯଦି (error != NSERROR_OK) {
		ଯଦି (sniff_allowed == false)
			ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		ଫେରନ୍ତୁ mimesniff__compute_unknown(data, len, effective_type);
	}

	ଯଦି (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (image_only) {
		lwc_string *official_type;

		ଯଦି (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ଫେରନ୍ତୁ NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = strlen(content_type_header);

	/* Look for text types */
	ପାଇଁ (tt = text_types; tt->data != ଖାଲି; tt++) {
		ଯଦି (tt->len == content_type_header_len &&
		    memcmp(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			ଫେରନ୍ତୁ mimesniff_compute_text_or_ଦ୍ୱିମିକ(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	ଯଦି ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	ଯଦି (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* text/xml, application/xml */
	ଯଦି ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* Image types */
	ଯଦି (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	ଯଦି ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		ଫେରନ୍ତୁ mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ଫେରନ୍ତୁ NSERROR_OK;
}
