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

#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/corestrings.h"

#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/mimesniff.h"

संरचना map_s {
	स्थिरांक uint8_t *sig;
	माप_प्रकार len;
	bool safe;
	lwc_string **type;
};

स्थिर bool mimesniff_has_द्विमान_octets(स्थिरांक uint8_t *data, माप_प्रकार len)
{
	स्थिरांक uint8_t *end = data + len;

	पर्यंत (data != end) {
		स्थिरांक uint8_t c = *data;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		जर (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			थांबा;

		data++;
	}

	परत data != end;
}

स्थिर nserror mimesniff__match_mp4(स्थिरांक uint8_t *data, माप_प्रकार len,
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
	जर (len < 12)
		परत NSERROR_NOT_FOUND;

	/* Box size is big-endian */
	box_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	जर (len < box_size || box_size % 4 != 0)
		परत NSERROR_NOT_FOUND;

	/* Ensure this is an 'ftyp' box */
	जर (data[4] != 'f' || data[5] != 't' ||
			data[6] != 'y' || data[7] != 'p')
		परत NSERROR_NOT_FOUND;

	/* Check if major brand begins with 'mp4' */
	जर (data[8] == 'm' && data[9] == 'p' && data[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		परत NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	साठी (i = 16; i <= box_size - 4; i += 4) {
		जर (data[i] == 'm' &&
		    data[i+1] == 'p' &&
		    data[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			परत NSERROR_OK;
		}
	}

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__match_unknown_ws(स्थिरांक uint8_t *data, माप_प्रकार len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (स्थिरांक uint8_t *) s, SLEN(s), x, t }
	स्थिर स्थिरांक संरचना map_s ws_exact_match_types[] = {
		SIG(&corestring_lwc_text_xml, "<?xml", false),
		{ रिक्त, 0, false, रिक्त }
	};

	स्थिर स्थिरांक संरचना map_s ws_inexact_match_types[] = {
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
		{ रिक्त, 0, false, रिक्त }
	};
#undef SIG
	स्थिरांक uint8_t *end = data + len;
	स्थिरांक संरचना map_s *it;

	/* Skip leading whitespace */
	पर्यंत (data != end) {
		स्थिरांक uint8_t c = *data;

		जर (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			थांबा;

		data++;
	}

	जर (data == end)
		परत NSERROR_NOT_FOUND;

	len = end - data;

	साठी (it = ws_exact_match_types; it->sig != रिक्त; it++) {
		जर (it->len <= len && स_भेद(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	साठी (it = ws_inexact_match_types; it->sig != रिक्त; it++) {
		/* +1 for trailing space or > */
		जर (len < it->len + 1)
			सुरूठेवा;

		जर (strncasecmp((स्थिरांक अक्षर *) data,
				(स्थिरांक अक्षर *) it->sig, it->len) == 0 &&
				(data[it->len] == ' ' ||
				data[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__match_unknown_bom(स्थिरांक uint8_t *data, माप_प्रकार len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (स्थिरांक uint8_t *) s, SLEN(s), x, t }
	स्थिर स्थिरांक संरचना map_s bom_match_types[] = {
		SIG(&corestring_lwc_text_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_text_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_text_plain, "\xef\xbb\xbf", false),
		{ रिक्त, 0, false, रिक्त }
	};
#undef SIG
	स्थिरांक संरचना map_s *it;

	साठी (it = bom_match_types; it->sig != रिक्त; it++) {
		जर (it->len <= len && स_भेद(data, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__match_unknown_riff(स्थिरांक uint8_t *data, माप_प्रकार len,
		lwc_string **effective_type)
{
#define SIG(t, s, x) { (स्थिरांक uint8_t *) s, SLEN(s), x, t }
	स्थिर स्थिरांक संरचना map_s riff_match_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ रिक्त, 0, false, रिक्त }
	};
#undef SIG
	स्थिरांक संरचना map_s *it;

	साठी (it = riff_match_types; it->sig != रिक्त; it++) {
		जर (it->len + SLEN("RIFF????") <= len &&
				स_भेद(data, "RIFF", SLEN("RIFF")) == 0 &&
				स_भेद(data + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__match_unknown_exact(स्थिरांक uint8_t *data, माप_प्रकार len,
		bool allow_unsafe, lwc_string **effective_type)
{
#define SIG(t, s, x) { (स्थिरांक uint8_t *) s, SLEN(s), x, t }
	स्थिर स्थिरांक संरचना map_s exact_match_types[] = {
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
			(स्थिरांक uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ रिक्त, 0, false, रिक्त }
	};
#undef SIG
	स्थिरांक संरचना map_s *it;

	साठी (it = exact_match_types; it->sig != रिक्त; it++) {
		जर (it->len <= len && स_भेद(data, it->sig, it->len) == 0 &&
				(allow_unsafe || it->safe)) {
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__match_unknown(स्थिरांक uint8_t *data, माप_प्रकार len,
		bool allow_unsafe, lwc_string **effective_type)
{
	जर (mimesniff__match_unknown_exact(data, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	जर (mimesniff__match_unknown_riff(data, len,
			effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	जर (allow_unsafe == false)
		परत NSERROR_NOT_FOUND;

	जर (mimesniff__match_unknown_bom(data, len,
			effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	जर (mimesniff__match_unknown_ws(data, len,
			effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	जर (mimesniff__match_mp4(data, len, effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	परत NSERROR_NOT_FOUND;
}

स्थिर nserror mimesniff__compute_unknown(स्थिरांक uint8_t *data, माप_प्रकार len,
		lwc_string **effective_type)
{
	जर (data == रिक्त)
		परत NSERROR_NEED_DATA;

	len = min(len, 512);

	जर (mimesniff__match_unknown(data, len, true,
				     effective_type) == NSERROR_OK) {
		परत NSERROR_OK;
	}

	जर (mimesniff_has_द्विमान_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		परत NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	परत NSERROR_OK;
}

स्थिर nserror mimesniff_compute_text_or_द्विमान(स्थिरांक uint8_t *data,
		माप_प्रकार len, lwc_string **effective_type)
{
	जर (data == रिक्त) {
		परत NSERROR_NEED_DATA;
	}

	len = min(len, 512);

	जर (len >= 3 && ((data[0] == 0xfe && data[1] == 0xff) ||
			(data[0] == 0xff && data[1] == 0xfe) ||
			(data[0] == 0xef && data[1] == 0xbb &&
				data[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		परत NSERROR_OK;
	}

	जर (mimesniff_has_द्विमान_octets(data, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_text_plain);
		परत NSERROR_OK;
	}

	जर (mimesniff__match_unknown(data, len, false,
			effective_type) == NSERROR_OK)
		परत NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_stream);

	परत NSERROR_OK;
}

स्थिर nserror mimesniff__compute_image(lwc_string *official_type,
		स्थिरांक uint8_t *data, माप_प्रकार len, lwc_string **effective_type)
{
#define SIG(t, s) { (स्थिरांक uint8_t *) s, SLEN(s), t }
	स्थिर स्थिरांक संरचना it_s {
		स्थिरांक uint8_t *sig;
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
			(स्थिरांक uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ रिक्त, 0, रिक्त }
	};
#undef SIG

	स्थिरांक संरचना it_s *it;

	जर (data == रिक्त) {
		lwc_string_unref(official_type);
		परत NSERROR_NEED_DATA;
	}

	साठी (it = image_types; it->sig != रिक्त; it++) {
		जर (it->len <= len && स_भेद(data, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			परत NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	जर (SLEN("RIFF????WEBPVP") <= len &&
			स_भेद(data, "RIFF", SLEN("RIFF")) == 0 &&
			स_भेद(data + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		परत NSERROR_OK;
	}

	*effective_type = official_type;

	परत NSERROR_OK;
}

स्थिर nserror mimesniff__compute_feed_or_html(स्थिरांक uint8_t *data,
		माप_प्रकार len, lwc_string **effective_type)
{
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RSS_NS "http://purl.org/rss/1.0"

	गणना state_e {
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
	स्थिरांक uint8_t *end;

	जर (data == रिक्त)
		परत NSERROR_NEED_DATA;

	end = data + min(len, 512);

	पर्यंत (data < end) {
		स्थिरांक uint8_t c = *data;

#define MATCH(s) SLEN(s) <= (माप_प्रकार) (end - data) && \
			स_भेद(data, s, SLEN(s)) == 0

		निवड (state) {
		स्थिती BEFORE_BOM:
			जर (3 <= end - data && c == 0xef && data[1] == 0xbb &&
					data[2] == 0xbf) {
				data += 3;
			}

			state = BEFORE_MARKUP;
			थांबा;
		स्थिती BEFORE_MARKUP:
			जर (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				data++;
			अन्यथा जर (c != '<')
				data = end;
			अन्यथा {
				state = MARKUP_START;
				data++;
			}
			थांबा;
		स्थिती MARKUP_START:
			जर (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				data++;
			} अन्यथा जर (c == '?') {
				state = IN_PI;
				data++;
			} अन्यथा {
				/* Reconsume input */
				state = IN_TAG;
			}
			थांबा;
		स्थिती COMMENT_OR_DOCTYPE:
			जर (2 <= end - data && c == '-' && data[1] == '-') {
				state = IN_COMMENT;
				data += 2;
			} अन्यथा {
				/* Reconsume input */
				state = IN_DOCTYPE;
			}
			थांबा;
		स्थिती IN_COMMENT:
			जर (3 <= end - data && c == '-' && data[1] == '-' &&
					data[2] == '>') {
				state = BEFORE_MARKUP;
				data += 3;
			} अन्यथा
				data++;
			थांबा;
		स्थिती IN_DOCTYPE:
			जर (c == '>')
				state = BEFORE_MARKUP;
			data++;
			थांबा;
		स्थिती IN_PI:
			जर (2 <= end - data && c == '?' && data[1] == '>') {
				state = BEFORE_MARKUP;
				data += 2;
			} अन्यथा
				data++;
			थांबा;
		स्थिती IN_TAG:
			जर (MATCH("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				परत NSERROR_OK;
			} अन्यथा जर (MATCH("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				परत NSERROR_OK;
			} अन्यथा जर (MATCH("rdf:RDF")) {
				state = IN_RDF;
				data += SLEN("rdf:RDF");
			} अन्यथा
				data = end;
			थांबा;
		स्थिती IN_RDF:
			जर (MATCH(RSS_NS)) {
				rss = true;
				data += SLEN(RSS_NS);
			} अन्यथा जर (MATCH(RDF_NS)) {
				rdf = true;
				data += SLEN(RDF_NS);
			} अन्यथा
				data++;

			जर (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				परत NSERROR_OK;
			}

			थांबा;
		}
#undef MATCH
	}

	*effective_type = lwc_string_ref(corestring_lwc_text_html);

	परत NSERROR_OK;

#undef RSS_NS
#undef RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(स्थिरांक अक्षर *content_type_header,
				 स्थिरांक uint8_t *data,
				 माप_प्रकार len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#define S(s) { s, SLEN(s) }
	स्थिर स्थिरांक संरचना tt_s {
		स्थिरांक अक्षर *data;
		माप_प्रकार len;
	} text_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ रिक्त, 0 }
	};
#undef S

	माप_प्रकार content_type_header_len;
	http_content_type *ct;
	स्थिरांक संरचना tt_s *tt;
	bool match;
	nserror error;

	जर (content_type_header == रिक्त) {
		जर (sniff_allowed == false) {
			परत NSERROR_NOT_FOUND;
		}

		/* No official type => unknown */
		परत mimesniff__compute_unknown(data, len, effective_type);
	}

	error = http_parse_content_type(content_type_header, &ct);
	जर (error != NSERROR_OK) {
		जर (sniff_allowed == false)
			परत NSERROR_NOT_FOUND;

		/* Unparseable => unknown */
		परत mimesniff__compute_unknown(data, len, effective_type);
	}

	जर (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		परत NSERROR_OK;
	}

	जर (image_only) {
		lwc_string *official_type;

		जर (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&match) == lwc_error_ok && match) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			परत NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		परत mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	content_type_header_len = म_माप(content_type_header);

	/* Look for text types */
	साठी (tt = text_types; tt->data != रिक्त; tt++) {
		जर (tt->len == content_type_header_len &&
		    स_भेद(tt->data,
			   content_type_header,
			   content_type_header_len) == 0) {
			http_content_type_destroy(ct);
			परत mimesniff_compute_text_or_द्विमान(data, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	जर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		परत mimesniff__compute_unknown(data, len, effective_type);
	}

	/* +xml */
	जर (lwc_string_length(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_data(ct->media_type) +
				lwc_string_length(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		परत NSERROR_OK;
	}

	/* text/xml, application/xml */
	जर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_xml,
					 &match) == lwc_error_ok && match) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &match) == lwc_error_ok && match)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		परत NSERROR_OK;
	}

	/* Image types */
	जर (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		परत mimesniff__compute_image(official_type,
				data, len, effective_type);
	}

	/* text/html */
	जर ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_text_html,
					 &match) == lwc_error_ok && match)) {
		http_content_type_destroy(ct);
		परत mimesniff__compute_feed_or_html(data, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	परत NSERROR_OK;
}
