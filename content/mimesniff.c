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

#समावेश <string.h>
#समावेश <strings.h>

#समावेश "utils/http.h"
#समावेश "utils/utils.h"
#समावेश "utils/corestrings.h"

#समावेश "content/content_factory.h"
#समावेश "content/mimesniff.h"

काष्ठा map_s {
	स्थिर uint8_t *sig;
	माप_प्रकार len;
	bool safe;
	lwc_string **type;
};

अटल bool mimesniff_has_द्विआधारी_octets(स्थिर uint8_t *तथ्य, माप_प्रकार len)
{
	स्थिर uint8_t *end = तथ्य + len;

	जबतक (तथ्य != end) {
		स्थिर uint8_t c = *तथ्य;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		अगर (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			अवरोध;

		तथ्य++;
	}

	वापस तथ्य != end;
}

अटल nserror mimesniff__match_mp4(स्थिर uint8_t *तथ्य, माप_प्रकार len,
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
	अगर (len < 12)
		वापस NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_आकार = (तथ्य[0] << 24) | (तथ्य[1] << 16) | (तथ्य[2] << 8) | तथ्य[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	अगर (len < box_आकार || box_आकार % 4 != 0)
		वापस NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	अगर (तथ्य[4] != 'f' || तथ्य[5] != 't' ||
			तथ्य[6] != 'y' || तथ्य[7] != 'p')
		वापस NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	अगर (तथ्य[8] == 'm' && तथ्य[9] == 'p' && तथ्य[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		वापस NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	क्रम (i = 16; i <= box_आकार - 4; i += 4) {
		अगर (तथ्य[i] == 'm' &&
		    तथ्य[i+1] == 'p' &&
		    तथ्य[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			वापस NSERROR_OK;
		}
	}

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__match_unknown_ws(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		lwc_string **effective_type)
{
#घोषणा SIG(t, s, x) { (स्थिर uint8_t *) s, SLEN(s), x, t }
	अटल स्थिर काष्ठा map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ शून्य, 0, false, शून्य }
	};

	अटल स्थिर काष्ठा map_s ws_inexact_match_types[] = {
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
		{ शून्य, 0, false, शून्य }
	};
#अघोषित SIG
	स्थिर uint8_t *end = तथ्य + len;
	स्थिर काष्ठा map_s *it;

	/* Skip leading whitespace */
	जबतक (तथ्य != end) {
		स्थिर uint8_t c = *तथ्य;

		अगर (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			अवरोध;

		तथ्य++;
	}

	अगर (तथ्य == end)
		वापस NSERROR_NOT_FOUND;

	len = end - तथ्य;

	क्रम (it = ws_exact_match_types; it->sig != शून्य; it++) {
		अगर (it->len <= len && स_भेद(तथ्य, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	क्रम (it = ws_inexact_match_types; it->sig != शून्य; it++) {
		/* +1 for trailing space or > */
		अगर (len < it->len + 1)
			जारी;

		अगर (strncasecmp((स्थिर अक्षर *) तथ्य,
				(स्थिर अक्षर *) it->sig, it->len) == 0 &&
				(तथ्य[it->len] == ' ' ||
				तथ्य[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__match_unknown_bom(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		lwc_string **effective_type)
{
#घोषणा SIG(t, s, x) { (स्थिर uint8_t *) s, SLEN(s), x, t }
	अटल स्थिर काष्ठा map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ शून्य, 0, false, शून्य }
	};
#अघोषित SIG
	स्थिर काष्ठा map_s *it;

	क्रम (it = bom_match_types; it->sig != शून्य; it++) {
		अगर (it->len <= len && स_भेद(तथ्य, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__match_unknown_riff(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		lwc_string **effective_type)
{
#घोषणा SIG(t, s, x) { (स्थिर uint8_t *) s, SLEN(s), x, t }
	अटल स्थिर काष्ठा map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ शून्य, 0, false, शून्य }
	};
#अघोषित SIG
	स्थिर काष्ठा map_s *it;

	क्रम (it = riff_match_types; it->sig != शून्य; it++) {
		अगर (it->len + SLEN("RIFF????") <= len &&
				स_भेद(तथ्य, "RIFF", SLEN("RIFF")) == 0 &&
				स_भेद(तथ्य + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__match_unknown_exact(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		bool allow_unsafe, lwc_string **effective_type)
{
#घोषणा SIG(t, s, x) { (स्थिर uint8_t *) s, SLEN(s), x, t }
	अटल स्थिर काष्ठा map_s exact_match_types[] = {
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
			(स्थिर uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ शून्य, 0, false, शून्य }
	};
#अघोषित SIG
	स्थिर काष्ठा map_s *it;

	क्रम (it = exact_match_types; it->sig != शून्य; it++) {
		अगर (it->len <= len && स_भेद(तथ्य, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__match_unknown(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		bool allow_unsafe, lwc_string **effective_type)
{
	अगर (mimesniff__match_unknown_exact(तथ्य, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	अगर (mimesniff__match_unknown_riff(तथ्य, len,
			effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	अगर (allow_unsafe == false)
		वापस NSERROR_NOT_FOUND;

	अगर (mimesniff__match_unknown_bom(तथ्य, len,
			effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	अगर (mimesniff__match_unknown_ws(तथ्य, len,
			effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	अगर (mimesniff__match_mp4(तथ्य, len, effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	वापस NSERROR_NOT_FOUND;
}

अटल nserror mimesniff__compute_unknown(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		lwc_string **effective_type)
{
	अगर (तथ्य == शून्य)
		वापस NSERROR_NEED_तथ्य;

	len = min(len, 512);

	अगर (mimesniff__match_unknown(तथ्य, len, true,
				     effective_type) == NSERROR_OK) {
		वापस NSERROR_OK;
	}

	अगर (mimesniff_has_द्विआधारी_octets(तथ्य, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		वापस NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	वापस NSERROR_OK;
}

अटल nserror mimesniff_compute_text_or_द्विआधारी(स्थिर uint8_t *तथ्य,
		माप_प्रकार len, lwc_string **effective_type)
{
	अगर (तथ्य == शून्य) {
		वापस NSERROR_NEED_तथ्य;
	}

	len = min(len, 512);

	अगर (len >= 3 && ((तथ्य[0] == 0xfe && तथ्य[1] == 0xff) ||
			(तथ्य[0] == 0xff && तथ्य[1] == 0xfe) ||
			(तथ्य[0] == 0xef && तथ्य[1] == 0xbb &&
				तथ्य[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		वापस NSERROR_OK;
	}

	अगर (mimesniff_has_द्विआधारी_octets(तथ्य, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		वापस NSERROR_OK;
	}

	अगर (mimesniff__match_unknown(तथ्य, len, false,
			effective_type) == NSERROR_OK)
		वापस NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	वापस NSERROR_OK;
}

अटल nserror mimesniff__compute_image(lwc_string *official_type,
		स्थिर uint8_t *तथ्य, माप_प्रकार len, lwc_string **effective_type)
{
#घोषणा SIG(t, s) { (स्थिर uint8_t *) s, SLEN(s), t }
	अटल स्थिर काष्ठा it_s {
		स्थिर uint8_t *sig;
		माप_प्रकार len;
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
			(स्थिर uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ शून्य, 0, शून्य }
	};
#अघोषित SIG

	स्थिर काष्ठा it_s *it;

	अगर (तथ्य == शून्य) {
		lwc_string_unref(official_type);
		वापस NSERROR_NEED_तथ्य;
	}

	क्रम (it = image_types; it->sig != शून्य; it++) {
		अगर (it->len <= len && स_भेद(तथ्य, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			वापस NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	अगर (SLEN("RIFF????WEBPVP") <= len &&
			स_भेद(तथ्य, "RIFF", SLEN("RIFF")) == 0 &&
			स_भेद(तथ्य + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		वापस NSERROR_OK;
	}

	*effective_type = official_type;

	वापस NSERROR_OK;
}

अटल nserror mimesniff__compute_feed_or_html(स्थिर uint8_t *तथ्य,
		माप_प्रकार len, lwc_string **effective_type)
{
#घोषणा RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#घोषणा RSS_NS "http://purl.org/rss/1.0"

	क्रमागत state_e {
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
	स्थिर uint8_t *end;

	अगर (तथ्य == शून्य)
		वापस NSERROR_NEED_तथ्य;

	end = तथ्य + min(len, 512);

	जबतक (तथ्य < end) {
		स्थिर uint8_t c = *तथ्य;

#घोषणा MATCH(s) SLEN(s) <= (माप_प्रकार) (end - तथ्य) && \
			स_भेद(तथ्य, s, SLEN(s)) == 0

		चयन (state) {
		हाल BEFORE_BOM:
			अगर (3 <= end - तथ्य && c == 0xef && तथ्य[1] == 0xbb &&
					तथ्य[2] == 0xbf) {
				तथ्य += 3;
			}

			state = BEFORE_MARKUP;
			अवरोध;
		हाल BEFORE_MARKUP:
			अगर (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				तथ्य++;
			अन्यथा अगर (c != '<')
				तथ्य = end;
			अन्यथा {
				state = MARKUP_START;
				तथ्य++;
			}
			अवरोध;
		हाल MARKUP_START:
			अगर (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				तथ्य++;
			} अन्यथा अगर (c == '?') {
				state = IN_PI;
				तथ्य++;
			} अन्यथा {
				/* Reconsume input */
				state = IN_TAG;
			}
			अवरोध;
		हाल COMMENT_OR_DOCTYPE:
			अगर (2 <= end - तथ्य && c == '-' && तथ्य[1] == '-') {
				state = IN_COMMENT;
				तथ्य += 2;
			} अन्यथा {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			अवरोध;
		हाल IN_COMMENT:
			अगर (3 <= end - तथ्य && c == '-' && तथ्य[1] == '-' &&
					तथ्य[2] == '>') {
				state = BEFORE_MARKUP;
				तथ्य += 3;
			} अन्यथा
				तथ्य++;
			अवरोध;
		हाल IN_DOCTYPE:
			अगर (c == '>')
				state = BEFORE_MARKUP;
			तथ्य++;
			अवरोध;
		हाल IN_PI:
			अगर (2 <= end - तथ्य && c == '?' && तथ्य[1] == '>') {
				state = BEFORE_MARKUP;
				तथ्य += 2;
			} अन्यथा
				तथ्य++;
			अवरोध;
		हाल IN_TAG:
			अगर (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				वापस NSERROR_OK;
			} अन्यथा अगर (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				वापस NSERROR_OK;
			} अन्यथा अगर (MATCH("rdf:RDF")) {
				state = IN_RDF;
				तथ्य += SLEN("rdf:RDF");
			} अन्यथा
				तथ्य = end;
			अवरोध;
		हाल IN_RDF:
			अगर (MATCH(RSS_NS)) {
				rss = true;
				तथ्य += SLEN(RSS_NS);
			} अन्यथा अगर (MATCH(RDF_NS)) {
				rdf = true;
				तथ्य += SLEN(RDF_NS);
			} अन्यथा
				तथ्य++;

			अगर (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				वापस NSERROR_OK;
			}

			अवरोध;
		}
#अघोषित MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	वापस NSERROR_OK;

#अघोषित RSS_NS
#अघोषित RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(स्थिर अक्षर *content_type_header,
				 स्थिर uint8_t *तथ्य,
				 माप_प्रकार len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#घोषणा S(s) { s, SLEN(s) }
	अटल स्थिर काष्ठा tt_s {
		स्थिर अक्षर *तथ्य;
		माप_प्रकार len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ शून्य, 0 }
	};
#अघोषित S

	माप_प्रकार content_type_header_len;
	http_content_type *ct;
	स्थिर काष्ठा tt_s *tt;
	bool match;
	nserror error;

	अगर (content_type_header == शून्य) {
		अगर (sniff_allowed == false) {
			वापस NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		वापस mimesniff__compute_unknown(तथ्य, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	अगर (error != NSERROR_OK) {
		अगर (sniff_allowed == false)
			वापस NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		वापस mimesniff__compute_unknown(तथ्य, len, effective_type);
	}

	अगर (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		वापस NSERROR_OK;
	}

	अगर (image_only) {
		lwc_string *official_type;

		अगर (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			वापस NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		वापस mimesniff__compute_image(official_type,
				तथ्य, len, effective_type);
	}

	content_type_header_len = म_माप(content_type_header);

	/* Look for text types */
	क्रम (tt = text_types; tt->तथ्य != शून्य; tt++) {
		अगर (tt->len == content_type_header_len &&
		    स_भेद(tt->तथ्य,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			वापस mimesniff_compute_text_or_द्विआधारी(तथ्य, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	अगर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		वापस mimesniff__compute_unknown(तथ्य, len, effective_type);
	}

	/* +xml */
	अगर (lwc_string_लंबाई(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_तथ्य(ct->media_type) +
				lwc_string_लंबाई(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		वापस NSERROR_OK;
	}

	/* text/xml, application/xml */
	अगर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		वापस NSERROR_OK;
	}

	/* Image types */
	अगर (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		वापस mimesniff__compute_image(official_type,
				तथ्य, len, effective_type);
	}

	/* text/html */
	अगर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		वापस mimesniff__compute_feed_or_html(तथ्य, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	वापस NSERROR_OK;
}
