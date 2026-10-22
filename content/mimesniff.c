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

#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>

#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/corestrings.h"

#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/mimesniff.h"

నిర్మాణం map_s {
	స్థిరరాశి uint8_t *sig;
	size_t len;
	bool సురక్షిత;
	lwc_string **type;
};

స్థిర bool mimesniff_has_ద్విమాన_octets(స్థిరరాశి uint8_t *దత్తాంశం, size_t len)
{
	స్థిరరాశి uint8_t *చివర = దత్తాంశం + len;

	అయితే (దత్తాంశం != చివర) {
		స్థిరరాశి uint8_t c = *దత్తాంశం;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		ఒకవేళ (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			ఆపు;

		దత్తాంశం++;
	}

	తిరిగి దత్తాంశం != చివర;
}

స్థిర nserror mimesniff_సరిపోలిక_mp4(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_కొలత, i;

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
	ఒకవేళ (len < 12)
		తిరిగి NSERROR_NOT_దొరికింది;

	/* Box size is big-endian */
	box_కొలత = (దత్తాంశం[0] << 24) | (దత్తాంశం[1] << 16) | (దత్తాంశం[2] << 8) | దత్తాంశం[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	ఒకవేళ (len < box_కొలత || box_కొలత % 4 != 0)
		తిరిగి NSERROR_NOT_దొరికింది;

	/* Ensure this is an 'ftyp' box */
	ఒకవేళ (దత్తాంశం[4] != 'f' || దత్తాంశం[5] != 't' ||
			దత్తాంశం[6] != 'y' || దత్తాంశం[7] != 'p')
		తిరిగి NSERROR_NOT_దొరికింది;

	/* Check if major brand begins with 'mp4' */
	ఒకవేళ (దత్తాంశం[8] == 'm' && దత్తాంశం[9] == 'p' && దత్తాంశం[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		తిరిగి NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	కొరకు (i = 16; i <= box_కొలత - 4; i += 4) {
		ఒకవేళ (దత్తాంశం[i] == 'm' &&
		    దత్తాంశం[i+1] == 'p' &&
		    దత్తాంశం[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			తిరిగి NSERROR_OK;
		}
	}

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff_సరిపోలిక_unknown_ws(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		lwc_string **effective_type)
{
#నిర్వచించు SIG(t, s, x) { (స్థిరరాశి uint8_t *) s, SLEN(s), x, t }
	స్థిర స్థిరరాశి నిర్మాణం map_s ws_exact_సరిపోలిక_types[] = {
		SIG(&corestring_lwc_పాఠ్యం_xml, "<?xml", false),
		{ ఖాళీ, 0, false, ఖాళీ }
	};

	స్థిర స్థిరరాశి నిర్మాణం map_s ws_inexact_సరిపోలిక_types[] = {
		SIG(&corestring_lwc_పాఠ్యం_html, "<!DOCTYPE HTML", false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<HTML",          false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<HEAD",          false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<SCRIPT",        false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<IFRAME",        false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<H1",            false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<DIV",           false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<FONT",          false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<TABLE",         false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<A",             false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<STYLE",         false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<TITLE",         false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<B",             false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<BODY",          false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<BR",            false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<P",             false),
		SIG(&corestring_lwc_పాఠ్యం_html, "<!--",           false),
		{ ఖాళీ, 0, false, ఖాళీ }
	};
#నిర్వచనం_రద్దు SIG
	స్థిరరాశి uint8_t *చివర = దత్తాంశం + len;
	స్థిరరాశి నిర్మాణం map_s *it;

	/* Skip leading whitespace */
	అయితే (దత్తాంశం != చివర) {
		స్థిరరాశి uint8_t c = *దత్తాంశం;

		ఒకవేళ (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			ఆపు;

		దత్తాంశం++;
	}

	ఒకవేళ (దత్తాంశం == చివర)
		తిరిగి NSERROR_NOT_దొరికింది;

	len = చివర - దత్తాంశం;

	కొరకు (it = ws_exact_సరిపోలిక_types; it->sig != ఖాళీ; it++) {
		ఒకవేళ (it->len <= len && memcmp(దత్తాంశం, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	కొరకు (it = ws_inexact_సరిపోలిక_types; it->sig != ఖాళీ; it++) {
		/* +1 for trailing space or > */
		ఒకవేళ (len < it->len + 1)
			కొనసాగు;

		ఒకవేళ (strncasecmp((స్థిరరాశి అక్షరం *) దత్తాంశం,
				(స్థిరరాశి అక్షరం *) it->sig, it->len) == 0 &&
				(దత్తాంశం[it->len] == ' ' ||
				దత్తాంశం[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff_సరిపోలిక_unknown_bom(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		lwc_string **effective_type)
{
#నిర్వచించు SIG(t, s, x) { (స్థిరరాశి uint8_t *) s, SLEN(s), x, t }
	స్థిర స్థిరరాశి నిర్మాణం map_s bom_సరిపోలిక_types[] = {
		SIG(&corestring_lwc_పాఠ్యం_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_పాఠ్యం_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_పాఠ్యం_plain, "\xef\xbb\xbf", false),
		{ ఖాళీ, 0, false, ఖాళీ }
	};
#నిర్వచనం_రద్దు SIG
	స్థిరరాశి నిర్మాణం map_s *it;

	కొరకు (it = bom_సరిపోలిక_types; it->sig != ఖాళీ; it++) {
		ఒకవేళ (it->len <= len && memcmp(దత్తాంశం, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff_సరిపోలిక_unknown_riff(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		lwc_string **effective_type)
{
#నిర్వచించు SIG(t, s, x) { (స్థిరరాశి uint8_t *) s, SLEN(s), x, t }
	స్థిర స్థిరరాశి నిర్మాణం map_s riff_సరిపోలిక_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ఖాళీ, 0, false, ఖాళీ }
	};
#నిర్వచనం_రద్దు SIG
	స్థిరరాశి నిర్మాణం map_s *it;

	కొరకు (it = riff_సరిపోలిక_types; it->sig != ఖాళీ; it++) {
		ఒకవేళ (it->len + SLEN("RIFF????") <= len &&
				memcmp(దత్తాంశం, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(దత్తాంశం + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff_సరిపోలిక_unknown_exact(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#నిర్వచించు SIG(t, s, x) { (స్థిరరాశి uint8_t *) s, SLEN(s), x, t }
	స్థిర స్థిరరాశి నిర్మాణం map_s exact_సరిపోలిక_types[] = {
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
			(స్థిరరాశి uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ఖాళీ, 0, false, ఖాళీ }
	};
#నిర్వచనం_రద్దు SIG
	స్థిరరాశి నిర్మాణం map_s *it;

	కొరకు (it = exact_సరిపోలిక_types; it->sig != ఖాళీ; it++) {
		ఒకవేళ (it->len <= len && memcmp(దత్తాంశం, it->sig, it->len) == 0 &&
				(allow_unsafe || it->సురక్షిత)) {
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff_సరిపోలిక_unknown(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	ఒకవేళ (mimesniff_సరిపోలిక_unknown_exact(దత్తాంశం, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	ఒకవేళ (mimesniff_సరిపోలిక_unknown_riff(దత్తాంశం, len,
			effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	ఒకవేళ (allow_unsafe == false)
		తిరిగి NSERROR_NOT_దొరికింది;

	ఒకవేళ (mimesniff_సరిపోలిక_unknown_bom(దత్తాంశం, len,
			effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	ఒకవేళ (mimesniff_సరిపోలిక_unknown_ws(దత్తాంశం, len,
			effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	ఒకవేళ (mimesniff_సరిపోలిక_mp4(దత్తాంశం, len, effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror mimesniff__compute_unknown(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		lwc_string **effective_type)
{
	ఒకవేళ (దత్తాంశం == ఖాళీ)
		తిరిగి NSERROR_NEED_దత్తాంశం;

	len = min(len, 512);

	ఒకవేళ (mimesniff_సరిపోలిక_unknown(దత్తాంశం, len, true,
				     effective_type) == NSERROR_OK) {
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (mimesniff_has_ద్విమాన_octets(దత్తాంశం, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_పాఠ్యం_plain);
		తిరిగి NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ప్రవాహం);

	తిరిగి NSERROR_OK;
}

స్థిర nserror mimesniff_compute_పాఠ్యం_or_ద్విమాన(స్థిరరాశి uint8_t *దత్తాంశం,
		size_t len, lwc_string **effective_type)
{
	ఒకవేళ (దత్తాంశం == ఖాళీ) {
		తిరిగి NSERROR_NEED_దత్తాంశం;
	}

	len = min(len, 512);

	ఒకవేళ (len >= 3 && ((దత్తాంశం[0] == 0xfe && దత్తాంశం[1] == 0xff) ||
			(దత్తాంశం[0] == 0xff && దత్తాంశం[1] == 0xfe) ||
			(దత్తాంశం[0] == 0xef && దత్తాంశం[1] == 0xbb &&
				దత్తాంశం[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_పాఠ్యం_plain);
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (mimesniff_has_ద్విమాన_octets(దత్తాంశం, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_పాఠ్యం_plain);
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (mimesniff_సరిపోలిక_unknown(దత్తాంశం, len, false,
			effective_type) == NSERROR_OK)
		తిరిగి NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ప్రవాహం);

	తిరిగి NSERROR_OK;
}

స్థిర nserror mimesniff__compute_image(lwc_string *official_type,
		స్థిరరాశి uint8_t *దత్తాంశం, size_t len, lwc_string **effective_type)
{
#నిర్వచించు SIG(t, s) { (స్థిరరాశి uint8_t *) s, SLEN(s), t }
	స్థిర స్థిరరాశి నిర్మాణం it_s {
		స్థిరరాశి uint8_t *sig;
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
			(స్థిరరాశి uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ఖాళీ, 0, ఖాళీ }
	};
#నిర్వచనం_రద్దు SIG

	స్థిరరాశి నిర్మాణం it_s *it;

	ఒకవేళ (దత్తాంశం == ఖాళీ) {
		lwc_string_unref(official_type);
		తిరిగి NSERROR_NEED_దత్తాంశం;
	}

	కొరకు (it = image_types; it->sig != ఖాళీ; it++) {
		ఒకవేళ (it->len <= len && memcmp(దత్తాంశం, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			తిరిగి NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	ఒకవేళ (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(దత్తాంశం, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(దత్తాంశం + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		తిరిగి NSERROR_OK;
	}

	*effective_type = official_type;

	తిరిగి NSERROR_OK;
}

స్థిర nserror mimesniff__compute_feed_or_html(స్థిరరాశి uint8_t *దత్తాంశం,
		size_t len, lwc_string **effective_type)
{
#నిర్వచించు RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#నిర్వచించు RSS_NS "http://purl.org/rss/1.0"

	గణన state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_ప్రారంభం,
		COMMENT_OR_DOCTYPE,
		లోపల_COMMENT,
		లోపల_DOCTYPE,
		లోపల_PI,
		లోపల_ట్యాగ్,
		లోపల_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	స్థిరరాశి uint8_t *చివర;

	ఒకవేళ (దత్తాంశం == ఖాళీ)
		తిరిగి NSERROR_NEED_దత్తాంశం;

	చివర = దత్తాంశం + min(len, 512);

	అయితే (దత్తాంశం < చివర) {
		స్థిరరాశి uint8_t c = *దత్తాంశం;

#నిర్వచించు సరిపోలిక_2(s) SLEN(s) <= (size_t) (చివర - దత్తాంశం) && \
			memcmp(దత్తాంశం, s, SLEN(s)) == 0

		ఎంపిక (state) {
		స్థితి BEFORE_BOM:
			ఒకవేళ (3 <= చివర - దత్తాంశం && c == 0xef && దత్తాంశం[1] == 0xbb &&
					దత్తాంశం[2] == 0xbf) {
				దత్తాంశం += 3;
			}

			state = BEFORE_MARKUP;
			ఆపు;
		స్థితి BEFORE_MARKUP:
			ఒకవేళ (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				దత్తాంశం++;
			లేకపోతే ఒకవేళ (c != '<')
				దత్తాంశం = చివర;
			లేకపోతే {
				state = MARKUP_ప్రారంభం;
				దత్తాంశం++;
			}
			ఆపు;
		స్థితి MARKUP_ప్రారంభం:
			ఒకవేళ (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				దత్తాంశం++;
			} లేకపోతే ఒకవేళ (c == '?') {
				state = లోపల_PI;
				దత్తాంశం++;
			} లేకపోతే {
				/* Reconsume input */
				state = లోపల_ట్యాగ్;
			}
			ఆపు;
		స్థితి COMMENT_OR_DOCTYPE:
			ఒకవేళ (2 <= చివర - దత్తాంశం && c == '-' && దత్తాంశం[1] == '-') {
				state = లోపల_COMMENT;
				దత్తాంశం += 2;
			} లేకపోతే {
				/* Reconsume input */
				state = లోపల_DOCTYPE;
			}
			ఆపు;
		స్థితి లోపల_COMMENT:
			ఒకవేళ (3 <= చివర - దత్తాంశం && c == '-' && దత్తాంశం[1] == '-' &&
					దత్తాంశం[2] == '>') {
				state = BEFORE_MARKUP;
				దత్తాంశం += 3;
			} లేకపోతే
				దత్తాంశం++;
			ఆపు;
		స్థితి లోపల_DOCTYPE:
			ఒకవేళ (c == '>')
				state = BEFORE_MARKUP;
			దత్తాంశం++;
			ఆపు;
		స్థితి లోపల_PI:
			ఒకవేళ (2 <= చివర - దత్తాంశం && c == '?' && దత్తాంశం[1] == '>') {
				state = BEFORE_MARKUP;
				దత్తాంశం += 2;
			} లేకపోతే
				దత్తాంశం++;
			ఆపు;
		స్థితి లోపల_ట్యాగ్:
			ఒకవేళ (సరిపోలిక_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				తిరిగి NSERROR_OK;
			} లేకపోతే ఒకవేళ (సరిపోలిక_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				తిరిగి NSERROR_OK;
			} లేకపోతే ఒకవేళ (సరిపోలిక_2("rdf:RDF")) {
				state = లోపల_RDF;
				దత్తాంశం += SLEN("rdf:RDF");
			} లేకపోతే
				దత్తాంశం = చివర;
			ఆపు;
		స్థితి లోపల_RDF:
			ఒకవేళ (సరిపోలిక_2(RSS_NS)) {
				rss = true;
				దత్తాంశం += SLEN(RSS_NS);
			} లేకపోతే ఒకవేళ (సరిపోలిక_2(RDF_NS)) {
				rdf = true;
				దత్తాంశం += SLEN(RDF_NS);
			} లేకపోతే
				దత్తాంశం++;

			ఒకవేళ (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				తిరిగి NSERROR_OK;
			}

			ఆపు;
		}
#నిర్వచనం_రద్దు సరిపోలిక_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_పాఠ్యం_html);

	తిరిగి NSERROR_OK;

#నిర్వచనం_రద్దు RSS_NS
#నిర్వచనం_రద్దు RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(స్థిరరాశి అక్షరం *content_type_శీర్షిక,
				 స్థిరరాశి uint8_t *దత్తాంశం,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#నిర్వచించు S(s) { s, SLEN(s) }
	స్థిర స్థిరరాశి నిర్మాణం tt_s {
		స్థిరరాశి అక్షరం *దత్తాంశం;
		size_t len;
	} పాఠ్యం_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ఖాళీ, 0 }
	};
#నిర్వచనం_రద్దు S

	size_t content_type_శీర్షిక_len;
	http_content_type *ct;
	స్థిరరాశి నిర్మాణం tt_s *tt;
	bool సరిపోలిక;
	nserror error;

	ఒకవేళ (content_type_శీర్షిక == ఖాళీ) {
		ఒకవేళ (sniff_allowed == false) {
			తిరిగి NSERROR_NOT_దొరికింది;
		}

		/* No official type => unknown */
		తిరిగి mimesniff__compute_unknown(దత్తాంశం, len, effective_type);
	}

	error = http_parse_content_type(content_type_శీర్షిక, &ct);
	ఒకవేళ (error != NSERROR_OK) {
		ఒకవేళ (sniff_allowed == false)
			తిరిగి NSERROR_NOT_దొరికింది;

		/* Unparseable => unknown */
		తిరిగి mimesniff__compute_unknown(దత్తాంశం, len, effective_type);
	}

	ఒకవేళ (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (image_only) {
		lwc_string *official_type;

		ఒకవేళ (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&సరిపోలిక) == lwc_error_ok && సరిపోలిక) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			తిరిగి NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		తిరిగి mimesniff__compute_image(official_type,
				దత్తాంశం, len, effective_type);
	}

	content_type_శీర్షిక_len = అక్షరశ్రేణి_పొడవు(content_type_శీర్షిక);

	/* Look for text types */
	కొరకు (tt = పాఠ్యం_types; tt->దత్తాంశం != ఖాళీ; tt++) {
		ఒకవేళ (tt->len == content_type_శీర్షిక_len &&
		    memcmp(tt->దత్తాంశం,
			   content_type_శీర్షిక,
			   content_type_శీర్షిక_len) == 0) {
			http_content_type_destroy(ct);
			తిరిగి mimesniff_compute_పాఠ్యం_or_ద్విమాన(దత్తాంశం, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	ఒకవేళ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక)) {
		http_content_type_destroy(ct);
		తిరిగి mimesniff__compute_unknown(దత్తాంశం, len, effective_type);
	}

	/* +xml */
	ఒకవేళ (lwc_string_నిడివి(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_దత్తాంశం(ct->media_type) +
				lwc_string_నిడివి(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		తిరిగి NSERROR_OK;
	}

	/* text/xml, application/xml */
	ఒకవేళ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_పాఠ్యం_xml,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		తిరిగి NSERROR_OK;
	}

	/* Image types */
	ఒకవేళ (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		తిరిగి mimesniff__compute_image(official_type,
				దత్తాంశం, len, effective_type);
	}

	/* text/html */
	ఒకవేళ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_పాఠ్యం_html,
					 &సరిపోలిక) == lwc_error_ok && సరిపోలిక)) {
		http_content_type_destroy(ct);
		తిరిగి mimesniff__compute_feed_or_html(దత్తాంశం, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	తిరిగి NSERROR_OK;
}
