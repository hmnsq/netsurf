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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>

#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"

#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/mimesniff.h"

ರಚನೆ map_s {
	ಸ್ಥಿರಾಂಕ uint8_t *sig;
	size_t len;
	bool ಸುರಕ್ಷಿತ;
	lwc_string **type;
};

ಸ್ಥಿರ bool mimesniff_has_ದ್ವಿಮಾನ_octets(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len)
{
	ಸ್ಥಿರಾಂಕ uint8_t *ಕೊನೆ = ದತ್ತಾಂಶ + len;

	ಆಗ (ದತ್ತಾಂಶ != ಕೊನೆ) {
		ಸ್ಥಿರಾಂಕ uint8_t c = *ದತ್ತಾಂಶ;

		/* Binary iff in C0 and not ESC, CR, FF, LF, HT */
		ಆದರೆ (c <= 0x1f && c != 0x1b && c != '\r' && c != '\f' &&
				c != '\n' && c != '\t')
			ನಿಲ್ಲಿಸು;

		ದತ್ತಾಂಶ++;
	}

	ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ != ಕೊನೆ;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_mp4(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		lwc_string **effective_type)
{
	uint32_t box_ಗಾತ್ರ, i;

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
	ಆದರೆ (len < 12)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	/* Box size is big-endian */
	box_ಗಾತ್ರ = (ದತ್ತಾಂಶ[0] << 24) | (ದತ್ತಾಂಶ[1] << 16) | (ದತ್ತಾಂಶ[2] << 8) | ದತ್ತಾಂಶ[3];

	/* Require that we can read the entire box, and reject bad box sizes */
	ಆದರೆ (len < box_ಗಾತ್ರ || box_ಗಾತ್ರ % 4 != 0)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	/* Ensure this is an 'ftyp' box */
	ಆದರೆ (ದತ್ತಾಂಶ[4] != 'f' || ದತ್ತಾಂಶ[5] != 't' ||
			ದತ್ತಾಂಶ[6] != 'y' || ದತ್ತಾಂಶ[7] != 'p')
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	/* Check if major brand begins with 'mp4' */
	ಆದರೆ (ದತ್ತಾಂಶ[8] == 'm' && ದತ್ತಾಂಶ[9] == 'p' && ದತ್ತಾಂಶ[10] == '4') {
		*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* Search each compatible brand in the box for "mp4" */
	ಗಾಗಿ (i = 16; i <= box_ಗಾತ್ರ - 4; i += 4) {
		ಆದರೆ (ದತ್ತಾಂಶ[i] == 'm' &&
		    ದತ್ತಾಂಶ[i+1] == 'p' &&
		    ದತ್ತಾಂಶ[i+2] == '4') {
			*effective_type = lwc_string_ref(corestring_lwc_video_mp4);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_unknown_ws(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು SIG(t, s, x) { (ಸ್ಥಿರಾಂಕ uint8_t *) s, SLEN(s), x, t }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ map_s ws_exact_ಹೊಂದಿಕೆ_types[] = {
		SIG(&corestring_lwc_ಪಠ್ಯ_xml, "<?xml", false),
		{ ಖಾಲಿ, 0, false, ಖಾಲಿ }
	};

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ map_s ws_inexact_ಹೊಂದಿಕೆ_types[] = {
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<!DOCTYPE HTML", false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<HTML",          false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<HEAD",          false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<SCRIPT",        false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<IFRAME",        false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<H1",            false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<DIV",           false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<FONT",          false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<TABLE",         false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<A",             false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<STYLE",         false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<TITLE",         false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<B",             false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<BODY",          false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<BR",            false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<P",             false),
		SIG(&corestring_lwc_ಪಠ್ಯ_html, "<!--",           false),
		{ ಖಾಲಿ, 0, false, ಖಾಲಿ }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SIG
	ಸ್ಥಿರಾಂಕ uint8_t *ಕೊನೆ = ದತ್ತಾಂಶ + len;
	ಸ್ಥಿರಾಂಕ ರಚನೆ map_s *it;

	/* Skip leading whitespace */
	ಆಗ (ದತ್ತಾಂಶ != ಕೊನೆ) {
		ಸ್ಥಿರಾಂಕ uint8_t c = *ದತ್ತಾಂಶ;

		ಆದರೆ (c != '\t' && c != '\n' && c != '\f' &&
				c != '\r' && c != ' ')
			ನಿಲ್ಲಿಸು;

		ದತ್ತಾಂಶ++;
	}

	ಆದರೆ (ದತ್ತಾಂಶ == ಕೊನೆ)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	len = ಕೊನೆ - ದತ್ತಾಂಶ;

	ಗಾಗಿ (it = ws_exact_ಹೊಂದಿಕೆ_types; it->sig != ಖಾಲಿ; it++) {
		ಆದರೆ (it->len <= len && memcmp(ದತ್ತಾಂಶ, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಗಾಗಿ (it = ws_inexact_ಹೊಂದಿಕೆ_types; it->sig != ಖಾಲಿ; it++) {
		/* +1 for trailing space or > */
		ಆದರೆ (len < it->len + 1)
			ಮುಂದುವರಿ;

		ಆದರೆ (strncasecmp((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) ದತ್ತಾಂಶ,
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) it->sig, it->len) == 0 &&
				(ದತ್ತಾಂಶ[it->len] == ' ' ||
				ದತ್ತಾಂಶ[it->len] == '>')) {
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_unknown_bom(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು SIG(t, s, x) { (ಸ್ಥಿರಾಂಕ uint8_t *) s, SLEN(s), x, t }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ map_s bom_ಹೊಂದಿಕೆ_types[] = {
		SIG(&corestring_lwc_ಪಠ್ಯ_plain, "\xfe\xff",     false),
		SIG(&corestring_lwc_ಪಠ್ಯ_plain, "\xff\xfe",     false),
		SIG(&corestring_lwc_ಪಠ್ಯ_plain, "\xef\xbb\xbf", false),
		{ ಖಾಲಿ, 0, false, ಖಾಲಿ }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SIG
	ಸ್ಥಿರಾಂಕ ರಚನೆ map_s *it;

	ಗಾಗಿ (it = bom_ಹೊಂದಿಕೆ_types; it->sig != ಖಾಲಿ; it++) {
		ಆದರೆ (it->len <= len && memcmp(ದತ್ತಾಂಶ, it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_unknown_riff(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು SIG(t, s, x) { (ಸ್ಥಿರಾಂಕ uint8_t *) s, SLEN(s), x, t }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ map_s riff_ಹೊಂದಿಕೆ_types[] = {
		SIG(&corestring_lwc_image_webp, "WEBPVP", true),
		SIG(&corestring_lwc_audio_wave, "WAVE",   true),
		{ ಖಾಲಿ, 0, false, ಖಾಲಿ }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SIG
	ಸ್ಥಿರಾಂಕ ರಚನೆ map_s *it;

	ಗಾಗಿ (it = riff_ಹೊಂದಿಕೆ_types; it->sig != ಖಾಲಿ; it++) {
		ಆದರೆ (it->len + SLEN("RIFF????") <= len &&
				memcmp(ದತ್ತಾಂಶ, "RIFF", SLEN("RIFF")) == 0 &&
				memcmp(ದತ್ತಾಂಶ + SLEN("RIFF????"),
						it->sig, it->len) == 0) {
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_unknown_exact(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು SIG(t, s, x) { (ಸ್ಥಿರಾಂಕ uint8_t *) s, SLEN(s), x, t }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ map_s exact_ಹೊಂದಿಕೆ_types[] = {
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
			(ಸ್ಥಿರಾಂಕ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			true,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ಖಾಲಿ, 0, false, ಖಾಲಿ }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SIG
	ಸ್ಥಿರಾಂಕ ರಚನೆ map_s *it;

	ಗಾಗಿ (it = exact_ಹೊಂದಿಕೆ_types; it->sig != ಖಾಲಿ; it++) {
		ಆದರೆ (it->len <= len && memcmp(ದತ್ತಾಂಶ, it->sig, it->len) == 0 &&
				(allow_unsafe || it->ಸುರಕ್ಷಿತ)) {
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff_ಹೊಂದಿಕೆ_unknown(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		bool allow_unsafe, lwc_string **effective_type)
{
	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown_exact(ದತ್ತಾಂಶ, len, allow_unsafe,
			effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown_riff(ದತ್ತಾಂಶ, len,
			effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಆದರೆ (allow_unsafe == false)
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown_bom(ದತ್ತಾಂಶ, len,
			effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown_ws(ದತ್ತಾಂಶ, len,
			effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_mp4(ದತ್ತಾಂಶ, len, effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror mimesniff__compute_unknown(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		lwc_string **effective_type)
{
	ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;

	len = min(len, 512);

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown(ದತ್ತಾಂಶ, len, true,
				     effective_type) == NSERROR_OK) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (mimesniff_has_ದ್ವಿಮಾನ_octets(ದತ್ತಾಂಶ, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ಪಠ್ಯ_plain);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ಹರಿವು);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror mimesniff_compute_ಪಠ್ಯ_or_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ,
		size_t len, lwc_string **effective_type)
{
	ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;
	}

	len = min(len, 512);

	ಆದರೆ (len >= 3 && ((ದತ್ತಾಂಶ[0] == 0xfe && ದತ್ತಾಂಶ[1] == 0xff) ||
			(ದತ್ತಾಂಶ[0] == 0xff && ದತ್ತಾಂಶ[1] == 0xfe) ||
			(ದತ್ತಾಂಶ[0] == 0xef && ದತ್ತಾಂಶ[1] == 0xbb &&
				ದತ್ತಾಂಶ[2] == 0xbf))) {
		/* Found a BOM => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ಪಠ್ಯ_plain);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (mimesniff_has_ದ್ವಿಮಾನ_octets(ದತ್ತಾಂಶ, len) == false) {
		/* No binary octets => text/plain */
		*effective_type = lwc_string_ref(corestring_lwc_ಪಠ್ಯ_plain);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (mimesniff_ಹೊಂದಿಕೆ_unknown(ದತ್ತಾಂಶ, len, false,
			effective_type) == NSERROR_OK)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	*effective_type = lwc_string_ref(corestring_lwc_application_octet_ಹರಿವು);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror mimesniff__compute_image(lwc_string *official_type,
		ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len, lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು SIG(t, s) { (ಸ್ಥಿರಾಂಕ uint8_t *) s, SLEN(s), t }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ it_s {
		ಸ್ಥಿರಾಂಕ uint8_t *sig;
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
			(ಸ್ಥಿರಾಂಕ uint8_t *)"\x00\x00\x00\x0CJXL \x0D\x0A\x87\x0A",
			12,
			&corestring_lwc_image_jxl
		}, /* containered jpeg xl*/
		{ ಖಾಲಿ, 0, ಖಾಲಿ }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SIG

	ಸ್ಥಿರಾಂಕ ರಚನೆ it_s *it;

	ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ) {
		lwc_string_unref(official_type);
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;
	}

	ಗಾಗಿ (it = image_types; it->sig != ಖಾಲಿ; it++) {
		ಆದರೆ (it->len <= len && memcmp(ದತ್ತಾಂಶ, it->sig, it->len) == 0) {
			lwc_string_unref(official_type);
			*effective_type = lwc_string_ref(*it->type);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	}

	/* WebP has a signature that doesn't fit into the above table */
	ಆದರೆ (SLEN("RIFF????WEBPVP") <= len &&
			memcmp(ದತ್ತಾಂಶ, "RIFF", SLEN("RIFF")) == 0 &&
			memcmp(ದತ್ತಾಂಶ + SLEN("RIFF????"),
					"WEBPVP", SLEN("WEBPVP")) == 0 ) {
		lwc_string_unref(official_type);
		*effective_type = lwc_string_ref(corestring_lwc_image_webp);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	*effective_type = official_type;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror mimesniff__compute_feed_or_html(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ,
		size_t len, lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#ವ್ಯಾಖ್ಯಾನಿಸು RSS_NS "http://purl.org/rss/1.0"

	ಎಣಿಕೆ state_e {
		BEFORE_BOM,
		BEFORE_MARKUP,
		MARKUP_ಆರಂಭ,
		COMMENT_OR_DOCTYPE,
		ಒಳ_COMMENT,
		ಒಳ_DOCTYPE,
		ಒಳ_PI,
		ಒಳ_ಟ್ಯಾಗ್,
		ಒಳ_RDF
	} state = BEFORE_BOM;

	bool rdf = false, rss = false;
	ಸ್ಥಿರಾಂಕ uint8_t *ಕೊನೆ;

	ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;

	ಕೊನೆ = ದತ್ತಾಂಶ + min(len, 512);

	ಆಗ (ದತ್ತಾಂಶ < ಕೊನೆ) {
		ಸ್ಥಿರಾಂಕ uint8_t c = *ದತ್ತಾಂಶ;

#ವ್ಯಾಖ್ಯಾನಿಸು ಹೊಂದಿಕೆ_2(s) SLEN(s) <= (size_t) (ಕೊನೆ - ದತ್ತಾಂಶ) && \
			memcmp(ದತ್ತಾಂಶ, s, SLEN(s)) == 0

		ಆಯ್ಕೆ (state) {
		ಸ್ಥಿತಿ BEFORE_BOM:
			ಆದರೆ (3 <= ಕೊನೆ - ದತ್ತಾಂಶ && c == 0xef && ದತ್ತಾಂಶ[1] == 0xbb &&
					ದತ್ತಾಂಶ[2] == 0xbf) {
				ದತ್ತಾಂಶ += 3;
			}

			state = BEFORE_MARKUP;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BEFORE_MARKUP:
			ಆದರೆ (c == '\t' || c == '\n' || c	== '\r' || c == ' ')
				ದತ್ತಾಂಶ++;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c != '<')
				ದತ್ತಾಂಶ = ಕೊನೆ;
			ಇಲ್ಲದಿದ್ದರೆ {
				state = MARKUP_ಆರಂಭ;
				ದತ್ತಾಂಶ++;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ MARKUP_ಆರಂಭ:
			ಆದರೆ (c == '!') {
				state = COMMENT_OR_DOCTYPE;
				ದತ್ತಾಂಶ++;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == '?') {
				state = ಒಳ_PI;
				ದತ್ತಾಂಶ++;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Reconsume input */
				state = ಒಳ_ಟ್ಯಾಗ್;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ COMMENT_OR_DOCTYPE:
			ಆದರೆ (2 <= ಕೊನೆ - ದತ್ತಾಂಶ && c == '-' && ದತ್ತಾಂಶ[1] == '-') {
				state = ಒಳ_COMMENT;
				ದತ್ತಾಂಶ += 2;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Reconsume input */
				state = ಒಳ_DOCTYPE;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ಒಳ_COMMENT:
			ಆದರೆ (3 <= ಕೊನೆ - ದತ್ತಾಂಶ && c == '-' && ದತ್ತಾಂಶ[1] == '-' &&
					ದತ್ತಾಂಶ[2] == '>') {
				state = BEFORE_MARKUP;
				ದತ್ತಾಂಶ += 3;
			} ಇಲ್ಲದಿದ್ದರೆ
				ದತ್ತಾಂಶ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ಒಳ_DOCTYPE:
			ಆದರೆ (c == '>')
				state = BEFORE_MARKUP;
			ದತ್ತಾಂಶ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ಒಳ_PI:
			ಆದರೆ (2 <= ಕೊನೆ - ದತ್ತಾಂಶ && c == '?' && ದತ್ತಾಂಶ[1] == '>') {
				state = BEFORE_MARKUP;
				ದತ್ತಾಂಶ += 2;
			} ಇಲ್ಲದಿದ್ದರೆ
				ದತ್ತಾಂಶ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ಒಳ_ಟ್ಯಾಗ್:
			ಆದರೆ (ಹೊಂದಿಕೆ_2("rss")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_rss_xml);
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹೊಂದಿಕೆ_2("feed")) {
				*effective_type =
					lwc_string_ref(corestring_lwc_application_atom_xml);
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹೊಂದಿಕೆ_2("rdf:RDF")) {
				state = ಒಳ_RDF;
				ದತ್ತಾಂಶ += SLEN("rdf:RDF");
			} ಇಲ್ಲದಿದ್ದರೆ
				ದತ್ತಾಂಶ = ಕೊನೆ;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ಒಳ_RDF:
			ಆದರೆ (ಹೊಂದಿಕೆ_2(RSS_NS)) {
				rss = true;
				ದತ್ತಾಂಶ += SLEN(RSS_NS);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹೊಂದಿಕೆ_2(RDF_NS)) {
				rdf = true;
				ದತ್ತಾಂಶ += SLEN(RDF_NS);
			} ಇಲ್ಲದಿದ್ದರೆ
				ದತ್ತಾಂಶ++;

			ಆದರೆ (rdf && rss) {
				*effective_type = lwc_string_ref(corestring_lwc_application_rss_xml);
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			}

			ನಿಲ್ಲಿಸು;
		}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಹೊಂದಿಕೆ_2
	}

	*effective_type = lwc_string_ref(corestring_lwc_ಪಠ್ಯ_html);

	ಹಿಂತಿರುಗಿ NSERROR_OK;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು RSS_NS
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು RDF_NS
}

/* See mimesniff.h for documentation */
nserror
mimesniff_compute_effective_type(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_type_ಶೀರ್ಷಿಕೆ,
				 ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ,
				 size_t len,
				 bool sniff_allowed,
				 bool image_only,
				 lwc_string **effective_type)
{
#ವ್ಯಾಖ್ಯಾನಿಸು S(s) { s, SLEN(s) }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ tt_s {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ;
		size_t len;
	} ಪಠ್ಯ_types[] = {
		S("text/plain"),
		S("text/plain; charset=ISO-8859-1"),
		S("text/plain; charset=iso-8859-1"),
		S("text/plain; charset=UTF-8"),
		{ ಖಾಲಿ, 0 }
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು S

	size_t content_type_ಶೀರ್ಷಿಕೆ_len;
	http_content_type *ct;
	ಸ್ಥಿರಾಂಕ ರಚನೆ tt_s *tt;
	bool ಹೊಂದಿಕೆ;
	nserror error;

	ಆದರೆ (content_type_ಶೀರ್ಷಿಕೆ == ಖಾಲಿ) {
		ಆದರೆ (sniff_allowed == false) {
			ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
		}

		/* No official type => unknown */
		ಹಿಂತಿರುಗಿ mimesniff__compute_unknown(ದತ್ತಾಂಶ, len, effective_type);
	}

	error = http_parse_content_type(content_type_ಶೀರ್ಷಿಕೆ, &ct);
	ಆದರೆ (error != NSERROR_OK) {
		ಆದರೆ (sniff_allowed == false)
			ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

		/* Unparseable => unknown */
		ಹಿಂತಿರುಗಿ mimesniff__compute_unknown(ದತ್ತಾಂಶ, len, effective_type);
	}

	ಆದರೆ (sniff_allowed == false) {
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (image_only) {
		lwc_string *official_type;

		ಆದರೆ (lwc_string_caseless_isequal(ct->media_type,
						corestring_lwc_image_svg,
						&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) {
			*effective_type = lwc_string_ref(corestring_lwc_image_svg);
			http_content_type_destroy(ct);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}

		official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ mimesniff__compute_image(official_type,
				ದತ್ತಾಂಶ, len, effective_type);
	}

	content_type_ಶೀರ್ಷಿಕೆ_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(content_type_ಶೀರ್ಷಿಕೆ);

	/* Look for text types */
	ಗಾಗಿ (tt = ಪಠ್ಯ_types; tt->ದತ್ತಾಂಶ != ಖಾಲಿ; tt++) {
		ಆದರೆ (tt->len == content_type_ಶೀರ್ಷಿಕೆ_len &&
		    memcmp(tt->ದತ್ತಾಂಶ,
			   content_type_ಶೀರ್ಷಿಕೆ,
			   content_type_ಶೀರ್ಷಿಕೆ_len) == 0) {
			http_content_type_destroy(ct);
			ಹಿಂತಿರುಗಿ mimesniff_compute_ಪಠ್ಯ_or_ದ್ವಿಮಾನ(ದತ್ತಾಂಶ, len,
					effective_type);
		}
	}

	/* unknown/unknown, application/unknown, * / * */
	ಆದರೆ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_unknown_unknown,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_unknown,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_any,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ)) {
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ mimesniff__compute_unknown(ದತ್ತಾಂಶ, len, effective_type);
	}

	/* +xml */
	ಆದರೆ (lwc_string_ಉದ್ದಳತೆ(ct->media_type) > SLEN("+xml") &&
			strncasecmp(lwc_string_ದತ್ತಾಂಶ(ct->media_type) +
				lwc_string_ಉದ್ದಳತೆ(ct->media_type) -
				SLEN("+xml"),
				"+xml", SLEN("+xml")) == 0) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* text/xml, application/xml */
	ಆದರೆ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_ಪಠ್ಯ_xml,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) ||
	    (lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_application_xml,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ)) {
		/* Use official type */
		*effective_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* Image types */
	ಆದರೆ (content_factory_type_from_mime_type(ct->media_type) ==
			CONTENT_IMAGE) {
		lwc_string *official_type = lwc_string_ref(ct->media_type);
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ mimesniff__compute_image(official_type,
				ದತ್ತಾಂಶ, len, effective_type);
	}

	/* text/html */
	ಆದರೆ ((lwc_string_caseless_isequal(ct->media_type,
					 corestring_lwc_ಪಠ್ಯ_html,
					 &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ)) {
		http_content_type_destroy(ct);
		ಹಿಂತಿರುಗಿ mimesniff__compute_feed_or_html(ದತ್ತಾಂಶ, len,
						       effective_type);
	}

	/* Use official type */
	*effective_type = lwc_string_ref(ct->media_type);

	http_content_type_destroy(ct);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}
