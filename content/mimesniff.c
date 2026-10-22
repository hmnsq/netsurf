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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/mimesniff.h"

ਬਣਤਰ map_s {
	ਅਚਲ uint8_t *sig;
	size_t len;
	bool ਸੁਰੱਖਿਅਤ_2;
	lwc_string **type;
};

ਸਥਿਰ bool mimesniff_has_ਦੁਆਧਾਰੀ_octets(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len)
{
	ਅਚਲ uint8_t *ਸਿਰਾ = ਅੰਕੜੇ + len;

	ਜਦੋਂ (ਅੰਕੜੇ != ਸਿਰਾ) {
		ਅਚਲ uint8_t c = *ਅੰਕੜੇ;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		ਜੇ (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			ਰੋਕੋ;

		ਅੰਕੜੇ++;
	}

	ਵਾਪਸ ਅੰਕੜੇ != ਸਿਰਾ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_mp4(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_ਮਾਪ, i;

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
	ਜੇ (len < 12)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	/* Box size is big-endian */
	box_ਮਾਪ = (ਅੰਕੜੇ[0] << 24) | (ਅੰਕੜੇ[1] << 16) | (ਅੰਕੜੇ[2] << 8) | ਅੰਕੜੇ[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	ਜੇ (len < box_ਮਾਪ || box_ਮਾਪ % 4 != 0)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	/* Ensure this is an 'ftyp' box */
	ਜੇ (ਅੰਕੜੇ[4] != 'f' || ਅੰਕੜੇ[5] != 't' ||
			ਅੰਕੜੇ[6] != 'y' || ਅੰਕੜੇ[7] != 'p')
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	/* Check if major brand begins with 'mp4' */
	ਜੇ (ਅੰਕੜੇ[8] == 'm' && ਅੰਕੜੇ[9] == 'p' && ਅੰਕੜੇ[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ਵਾਪਸ NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	ਲਈ (i = 16; i <= box_ਮਾਪ - 4; i += 4) {
		ਜੇ (ਅੰਕੜੇ[i] == 'm' &&
		    ਅੰਕੜੇ[i+1] == 'p' &&
		    ਅੰਕੜੇ[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_unknown_ws(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIG(t, s, x) { (ਅਚਲ uint8_t *) s, SLEN(s), x, t }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ map_s ws_exact_ਮੇਲ_types[] = {
		SIG(&corestring_lwc_ਪਾਠ_xml, "<?xml", false),
		{ ਨਿਰਾ, 0, false, ਨਿਰਾ }
	};

	ਸਥਿਰ ਅਚਲ ਬਣਤਰ map_s ws_inexact_ਮੇਲ_types[] = {
		SIG(&corestring_lwc_ਪਾਠ_html, "<!DOCTYPE HTML", false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<HTML",          false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<HEAD",          false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<SCRIPT",        false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<IFRAME",        false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<H1",            false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<DIV",           false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<FONT",          false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<TABLE",         false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<A",             false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<STYLE",         false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<TITLE",         false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<B",             false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<BODY",          false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<BR",            false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<P",             false),
		SIG(&corestring_lwc_ਪਾਠ_html, "<!--",           false),
		{ ਨਿਰਾ, 0, false, ਨਿਰਾ }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SIG
	ਅਚਲ uint8_t *ਸਿਰਾ = ਅੰਕੜੇ + len;
	ਅਚਲ ਬਣਤਰ map_s *it;

	/* Skip leading whitespace */
	ਜਦੋਂ (ਅੰਕੜੇ != ਸਿਰਾ) {
		ਅਚਲ uint8_t c = *ਅੰਕੜੇ;

		ਜੇ (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			ਰੋਕੋ;

		ਅੰਕੜੇ++;
	}

	ਜੇ (ਅੰਕੜੇ == ਸਿਰਾ)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	len = ਸਿਰਾ - ਅੰਕੜੇ;

	ਲਈ (it = ws_exact_ਮੇਲ_types; it->sig != ਨਿਰਾ; it++) {
		ਜੇ (it->len <= len && memcmp(ਅੰਕੜੇ, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਲਈ (it = ws_inexact_ਮੇਲ_types; it->sig != ਨਿਰਾ; it++) {
		/* +1 for trailing space or > */
		ਜੇ (len < it->len + 1)
			ਜਾਰੀ;

		ਜੇ (strncasecmp((ਅਚਲ ਅੱਖਰ *) ਅੰਕੜੇ,
				(ਅਚਲ ਅੱਖਰ *) it->sig, it->len) == 0 &&
				(ਅੰਕੜੇ[it->len] == ' ' ||
				ਅੰਕੜੇ[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_unknown_bom(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIG(t, s, x) { (ਅਚਲ uint8_t *) s, SLEN(s), x, t }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ map_s bom_ਮੇਲ_types[] = {
		SIG(&corestring_lwc_ਪਾਠ_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_ਪਾਠ_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_ਪਾਠ_plain, "\xef\xbb\xbf", false),
		{ ਨਿਰਾ, 0, false, ਨਿਰਾ }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SIG
	ਅਚਲ ਬਣਤਰ map_s *it;

	ਲਈ (it = bom_ਮੇਲ_types; it->sig != ਨਿਰਾ; it++) {
		ਜੇ (it->len <= len && memcmp(ਅੰਕੜੇ, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_unknown_riff(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIG(t, s, x) { (ਅਚਲ uint8_t *) s, SLEN(s), x, t }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ map_s riff_ਮੇਲ_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ਨਿਰਾ, 0, false, ਨਿਰਾ }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SIG
	ਅਚਲ ਬਣਤਰ map_s *it;

	ਲਈ (it = riff_ਮੇਲ_types; it->sig != ਨਿਰਾ; it++) {
		ਜੇ (it->len + SLEN("RIFF????") <= len &&
				memcmp(ਅੰਕੜੇ, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(ਅੰਕੜੇ + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_unknown_exact(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIG(t, s, x) { (ਅਚਲ uint8_t *) s, SLEN(s), x, t }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ map_s exact_ਮੇਲ_types[] = {
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
			(ਅਚਲ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ਨਿਰਾ, 0, false, ਨਿਰਾ }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SIG
	ਅਚਲ ਬਣਤਰ map_s *it;

	ਲਈ (it = exact_ਮੇਲ_types; it->sig != ਨਿਰਾ; it++) {
		ਜੇ (it->len <= len && memcmp(ਅੰਕੜੇ, it->sig, it->len) == 0 &&
				(allow_unsafe || it->ਸੁਰੱਖਿਅਤ_2)) {
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff_ਮੇਲ_unknown(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	ਜੇ (mimesniff_ਮੇਲ_unknown_exact(ਅੰਕੜੇ, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	ਜੇ (mimesniff_ਮੇਲ_unknown_riff(ਅੰਕੜੇ, len,
			effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	ਜੇ (allow_unsafe == false)
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	ਜੇ (mimesniff_ਮੇਲ_unknown_bom(ਅੰਕੜੇ, len,
			effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	ਜੇ (mimesniff_ਮੇਲ_unknown_ws(ਅੰਕੜੇ, len,
			effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	ਜੇ (mimesniff_ਮੇਲ_mp4(ਅੰਕੜੇ, len, effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror mimesniff__compute_unknown(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		lwc_string **effective_type)
{
	ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;

	len = min(len, 512);

	ਜੇ (mimesniff_ਮੇਲ_unknown(ਅੰਕੜੇ, len, true,
				     effective_type) == NSERROR_OK) {
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (mimesniff_has_ਦੁਆਧਾਰੀ_octets(ਅੰਕੜੇ, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ਪਾਠ_plain);
		ਵਾਪਸ NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ਧਾਰਾ);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror mimesniff_compute_ਪਾਠ_or_ਦੁਆਧਾਰੀ(ਅਚਲ uint8_t *ਅੰਕੜੇ,
		size_t len, lwc_string **effective_type)
{
	ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;
	}

	len = min(len, 512);

	ਜੇ (len >= 3 && ((ਅੰਕੜੇ[0] == 0xfe && ਅੰਕੜੇ[1] == 0xff) ||
			(ਅੰਕੜੇ[0] == 0xff && ਅੰਕੜੇ[1] == 0xfe) ||
			(ਅੰਕੜੇ[0] == 0xef && ਅੰਕੜੇ[1] == 0xbb &&
				ਅੰਕੜੇ[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ਪਾਠ_plain);
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (mimesniff_has_ਦੁਆਧਾਰੀ_octets(ਅੰਕੜੇ, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ਪਾਠ_plain);
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (mimesniff_ਮੇਲ_unknown(ਅੰਕੜੇ, len, false,
			effective_type) == NSERROR_OK)
		ਵਾਪਸ NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ਧਾਰਾ);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror mimesniff__compute_image(lwc_string *official_type,
		ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len, lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIG(t, s) { (ਅਚਲ uint8_t *) s, SLEN(s), t }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ it_s {
		ਅਚਲ uint8_t *sig;
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
			(ਅਚਲ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ਨਿਰਾ, 0, ਨਿਰਾ }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SIG

	ਅਚਲ ਬਣਤਰ it_s *it;

	ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ) {
		lwc_string_unref(official_type);
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;
	}

	ਲਈ (it = image_types; it->sig != ਨਿਰਾ; it++) {
		ਜੇ (it->len <= len && memcmp(ਅੰਕੜੇ, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ਵਾਪਸ NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	ਜੇ (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(ਅੰਕੜੇ, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(ਅੰਕੜੇ + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ਵਾਪਸ NSERROR_OK;
	}

	*effective_type = official_type;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror mimesniff__compute_feed_or_html(ਅਚਲ uint8_t *ਅੰਕੜੇ,
		size_t len, lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RSS_NS "http://purl.org/rss/1.0"

	ਗਿਣਤੀ state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_ਸ਼ੁਰੂ,
		COMMENT_OR_DOCTYPE,
		ਵਿੱਚ_COMMENT,
		ਵਿੱਚ_DOCTYPE,
		ਵਿੱਚ_PI,
		ਵਿੱਚ_ਟੈਗ,
		ਵਿੱਚ_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ਅਚਲ uint8_t *ਸਿਰਾ;

	ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;

	ਸਿਰਾ = ਅੰਕੜੇ + min(len, 512);

	ਜਦੋਂ (ਅੰਕੜੇ < ਸਿਰਾ) {
		ਅਚਲ uint8_t c = *ਅੰਕੜੇ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮੇਲ_2(s) SLEN(s) <= (size_t) (ਸਿਰਾ - ਅੰਕੜੇ) && \
			memcmp(ਅੰਕੜੇ, s, SLEN(s)) == 0

		ਚੋਣ (state) {
		ਹਾਲਤ BEFORE_BOM:
			ਜੇ (3 <= ਸਿਰਾ - ਅੰਕੜੇ && c == 0xef && ਅੰਕੜੇ[1] == 0xbb &&
					ਅੰਕੜੇ[2] == 0xbf) {
				ਅੰਕੜੇ += 3;
			}

			state = BEFORE_MARKUP;
			ਰੋਕੋ;
		ਹਾਲਤ BEFORE_MARKUP:
			ਜੇ (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				ਅੰਕੜੇ++;
			ਨਹੀਂਤਾਂ ਜੇ (c != '<')
				ਅੰਕੜੇ = ਸਿਰਾ;
			ਨਹੀਂਤਾਂ {
				state = MARKUP_ਸ਼ੁਰੂ;
				ਅੰਕੜੇ++;
			}
			ਰੋਕੋ;
		ਹਾਲਤ MARKUP_ਸ਼ੁਰੂ:
			ਜੇ (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				ਅੰਕੜੇ++;
			} ਨਹੀਂਤਾਂ ਜੇ (c == '?') {
				state = ਵਿੱਚ_PI;
				ਅੰਕੜੇ++;
			} ਨਹੀਂਤਾਂ {
				/* Reconsume input */
				state = ਵਿੱਚ_ਟੈਗ;
			}
			ਰੋਕੋ;
		ਹਾਲਤ COMMENT_OR_DOCTYPE:
			ਜੇ (2 <= ਸਿਰਾ - ਅੰਕੜੇ && c == '-' && ਅੰਕੜੇ[1] == '-') {
				state = ਵਿੱਚ_COMMENT;
				ਅੰਕੜੇ += 2;
			} ਨਹੀਂਤਾਂ {
				/* Reconsume input */
				state = ਵਿੱਚ_DOCTYPE;
			}
			ਰੋਕੋ;
		ਹਾਲਤ ਵਿੱਚ_COMMENT:
			ਜੇ (3 <= ਸਿਰਾ - ਅੰਕੜੇ && c == '-' && ਅੰਕੜੇ[1] == '-' &&
					ਅੰਕੜੇ[2] == '>') {
				state = BEFORE_MARKUP;
				ਅੰਕੜੇ += 3;
			} ਨਹੀਂਤਾਂ
				ਅੰਕੜੇ++;
			ਰੋਕੋ;
		ਹਾਲਤ ਵਿੱਚ_DOCTYPE:
			ਜੇ (c == '>')
				state = BEFORE_MARKUP;
			ਅੰਕੜੇ++;
			ਰੋਕੋ;
		ਹਾਲਤ ਵਿੱਚ_PI:
			ਜੇ (2 <= ਸਿਰਾ - ਅੰਕੜੇ && c == '?' && ਅੰਕੜੇ[1] == '>') {
				state = BEFORE_MARKUP;
				ਅੰਕੜੇ += 2;
			} ਨਹੀਂਤਾਂ
				ਅੰਕੜੇ++;
			ਰੋਕੋ;
		ਹਾਲਤ ਵਿੱਚ_ਟੈਗ:
			ਜੇ (ਮੇਲ_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ਵਾਪਸ NSERROR_OK;
			} ਨਹੀਂਤਾਂ ਜੇ (ਮੇਲ_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ਵਾਪਸ NSERROR_OK;
			} ਨਹੀਂਤਾਂ ਜੇ (ਮੇਲ_2("rdf:RDF")) {
				state = ਵਿੱਚ_RDF;
				ਅੰਕੜੇ += SLEN("rdf:RDF");
			} ਨਹੀਂਤਾਂ
				ਅੰਕੜੇ = ਸਿਰਾ;
			ਰੋਕੋ;
		ਹਾਲਤ ਵਿੱਚ_RDF:
			ਜੇ (ਮੇਲ_2(RSS_NS)) {
				rss = true;
				ਅੰਕੜੇ += SLEN(RSS_NS);
			} ਨਹੀਂਤਾਂ ਜੇ (ਮੇਲ_2(RDF_NS)) {
				rdf = true;
				ਅੰਕੜੇ += SLEN(RDF_NS);
			} ਨਹੀਂਤਾਂ
				ਅੰਕੜੇ++;

			ਜੇ (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ਵਾਪਸ NSERROR_OK;
			}

			ਰੋਕੋ;
		}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਮੇਲ_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_ਪਾਠ_html);

	ਵਾਪਸ NSERROR_OK;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ RSS_NS
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ਅਚਲ ਅੱਖਰ *content_type_ਸਿਰਲੇਖ,
				 ਅਚਲ uint8_t *ਅੰਕੜੇ,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ S(s) { s, SLEN(s) }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ tt_s {
		ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ;
		size_t len;
	} ਪਾਠ_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ਨਿਰਾ, 0 }
	};
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ S

	size_t content_type_ਸਿਰਲੇਖ_len;
	http_content_type *ct;
	ਅਚਲ ਬਣਤਰ tt_s *tt;
	bool ਮੇਲ;
	nserror error;

	ਜੇ (content_type_ਸਿਰਲੇਖ == ਨਿਰਾ) {
		ਜੇ (sniff_allowed == false) {
			ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
		}

		/* No official type => unknown */
		ਵਾਪਸ mimesniff__compute_unknown(ਅੰਕੜੇ, len, effective_type);
	}

	error = http_parse_content_type(content_type_ਸਿਰਲੇਖ, &ct);
	ਜੇ (error != NSERROR_OK) {
		ਜੇ (sniff_allowed == false)
			ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

		/* Unparseable => unknown */
		ਵਾਪਸ mimesniff__compute_unknown(ਅੰਕੜੇ, len, effective_type);
	}

	ਜੇ (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (image_only) {
		lwc_string *official_type;

		ਜੇ (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&ਮੇਲ) == lwc_error_ok && ਮੇਲ) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ਵਾਪਸ NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ਵਾਪਸ mimesniff__compute_image(official_type,
				ਅੰਕੜੇ, len, effective_type);
	}

	content_type_ਸਿਰਲੇਖ_len = ਲੜੀ_ਲੰਬਾਈ(content_type_ਸਿਰਲੇਖ);

	/* Look for text types */
	ਲਈ (tt = ਪਾਠ_types; tt->ਅੰਕੜੇ != ਨਿਰਾ; tt++) {
		ਜੇ (tt->len == content_type_ਸਿਰਲੇਖ_len &&
		    memcmp(tt->ਅੰਕੜੇ,
			   content_type_ਸਿਰਲੇਖ,
			   content_type_ਸਿਰਲੇਖ_len) == 0) {
			http_content_type_destroy(ct);
			ਵਾਪਸ mimesniff_compute_ਪਾਠ_or_ਦੁਆਧਾਰੀ(ਅੰਕੜੇ, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	ਜੇ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ)) {
		http_content_type_destroy(ct);
		ਵਾਪਸ mimesniff__compute_unknown(ਅੰਕੜੇ, len, effective_type);
	}

	/* +xml */
	ਜੇ (lwc_string_ਲੰਮਾਈ(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_ਅੰਕੜੇ(ct->media_type) +
				lwc_string_ਲੰਮਾਈ(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ਵਾਪਸ NSERROR_OK;
	}

	/* text/xml, application/xml */
	ਜੇ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_ਪਾਠ_xml,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ਵਾਪਸ NSERROR_OK;
	}

	/* Image types */
	ਜੇ (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ਵਾਪਸ mimesniff__compute_image(official_type,
				ਅੰਕੜੇ, len, effective_type);
	}

	/* text/html */
	ਜੇ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_ਪਾਠ_html,
					 &ਮੇਲ) == lwc_error_ok && ਮੇਲ)) {
		http_content_type_destroy(ct);
		ਵਾਪਸ mimesniff__compute_feed_or_html(ਅੰਕੜੇ, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ਵਾਪਸ NSERROR_OK;
}
