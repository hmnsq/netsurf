/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"

#ಅನ್ತರ್ಭಾವ "css/dump.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_fixed(FILE *ಹರಿವು, css_fixed f)
{
#ವ್ಯಾಖ್ಯಾನಿಸು NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು NSCSS_ABS

	fprintf(ಹರಿವು, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_ಸಂಖ್ಯೆ(FILE *ಹರಿವು, css_fixed val)
{
	ಆದರೆ (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(ಹರಿವು, "%"PRId32, FIXTOINT(val));
	ಇಲ್ಲದಿದ್ದರೆ
		dump_css_fixed(ಹರಿವು, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_unit(FILE *ಹರಿವು, css_fixed val, css_unit unit)
{
	dump_css_ಸಂಖ್ಯೆ(ಹರಿವು, val);

	ಆಯ್ಕೆ (unit) {
	ಸ್ಥಿತಿ CSS_UNIT_PX:
		fprintf(ಹರಿವು, "px");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_EX:
		fprintf(ಹರಿವು, "ex");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_EM:
		fprintf(ಹರಿವು, "em");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_ಒಳ:
		fprintf(ಹರಿವು, "in");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_CM:
		fprintf(ಹರಿವು, "cm");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_MM:
		fprintf(ಹರಿವು, "mm");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_PT:
		fprintf(ಹರಿವು, "pt");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_PC:
		fprintf(ಹರಿವು, "pc");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_PCT:
		fprintf(ಹರಿವು, "%%");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_DEG:
		fprintf(ಹರಿವು, "deg");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_GRAD:
		fprintf(ಹರಿವು, "grad");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_RAD:
		fprintf(ಹರಿವು, "rad");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_MS:
		fprintf(ಹರಿವು, "ms");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_S:
		fprintf(ಹರಿವು, "s");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_HZ:
		fprintf(ಹರಿವು, "Hz");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_KHZ:
		fprintf(ಹರಿವು, "kHz");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_CH:
		fprintf(ಹರಿವು, "ch");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_REM:
		fprintf(ಹರಿವು, "rem");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_LH:
		fprintf(ಹರಿವು, "lh");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VH:
		fprintf(ಹರಿವು, "vh");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VW:
		fprintf(ಹರಿವು, "vw");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VI:
		fprintf(ಹರಿವು, "vi");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VB:
		fprintf(ಹರಿವು, "vb");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VMIN:
		fprintf(ಹರಿವು, "vmin");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_VMAX:
		fprintf(ಹರಿವು, "vmax");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_Q:
		fprintf(ಹರಿವು, "q");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNIT_CALC:
		fprintf(ಹರಿವು, "calc()");
		ನಿಲ್ಲಿಸು;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
ಶೂನ್ಯ nscss_dump_computed_style(FILE *ಹರಿವು, ಸ್ಥಿರಾಂಕ css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ಖಾಲಿ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ಸ್ಥಿರಾಂಕ css_computed_content_ಅಂಶ *content = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ css_computed_counter *counter = ಖಾಲಿ;
	lwc_string **string_list = ಖಾಲಿ;
	int32_t zindex = 0;

	fprintf(ಹರಿವು, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(ಹರಿವು, "background-attachment: fixed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(ಹರಿವು, "background-attachment: scroll ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(ಹರಿವು, "background-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	ಆದರೆ (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ಖಾಲಿ) {
		fprintf(ಹರಿವು, "background-image: url('%.*s') ",
				(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(url),
				lwc_string_ದತ್ತಾಂಶ(url));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(ಹರಿವು, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	ಆದರೆ (val == CSS_BACKGROUND_POSITION_ಗಣ) {
		fprintf(ಹರಿವು, "background-position: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		dump_css_unit(ಹರಿವು, len2, unit2);
		fprintf(ಹರಿವು, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(ಹರಿವು, "background-repeat: repeat-x ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(ಹರಿವು, "background-repeat: repeat-y ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(ಹರಿವು, "background-repeat: repeat ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(ಹರಿವು, "background-repeat: no-repeat ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(ಹರಿವು, "border-collapse: separate ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(ಹರಿವು, "border-collapse: collapse ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:

		ನಿಲ್ಲಿಸು;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	ಆದರೆ (val == CSS_BORDER_SPACING_ಗಣ) {
		fprintf(ಹರಿವು, "border-spacing: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		dump_css_unit(ಹರಿವು, len2, unit2);
		fprintf(ಹರಿವು, " ");
	}

	/* border-top-color */
	val = css_computed_border_ತುದಿ_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_COLOR_COLOR:
		fprintf(ಹರಿವು, "border-top-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-right-color */
	val = css_computed_border_ಬಲ_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_COLOR_COLOR:
		fprintf(ಹರಿವು, "border-right-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_COLOR_COLOR:
		fprintf(ಹರಿವು, "border-bottom-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-left-color */
	val = css_computed_border_ಎಡ_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_COLOR_COLOR:
		fprintf(ಹರಿವು, "border-left-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-top-style */
	val = css_computed_border_ತುದಿ_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_NONE:
		fprintf(ಹರಿವು, "border-top-style: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ಹರಿವು, "border-top-style: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED:
		fprintf(ಹರಿವು, "border-top-style: dotted ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED:
		fprintf(ಹರಿವು, "border-top-style: dashed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:
		fprintf(ಹರಿವು, "border-top-style: solid ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ಹರಿವು, "border-top-style: double ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE:
		fprintf(ಹರಿವು, "border-top-style: groove ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:
		fprintf(ಹರಿವು, "border-top-style: ridge ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:
		fprintf(ಹರಿವು, "border-top-style: inset ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET:
		fprintf(ಹರಿವು, "border-top-style: outset ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-right-style */
	val = css_computed_border_ಬಲ_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_NONE:
		fprintf(ಹರಿವು, "border-right-style: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ಹರಿವು, "border-right-style: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED:
		fprintf(ಹರಿವು, "border-right-style: dotted ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED:
		fprintf(ಹರಿವು, "border-right-style: dashed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:
		fprintf(ಹರಿವು, "border-right-style: solid ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ಹರಿವು, "border-right-style: double ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE:
		fprintf(ಹರಿವು, "border-right-style: groove ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:
		fprintf(ಹರಿವು, "border-right-style: ridge ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:
		fprintf(ಹರಿವು, "border-right-style: inset ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET:
		fprintf(ಹರಿವು, "border-right-style: outset ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_NONE:
		fprintf(ಹರಿವು, "border-bottom-style: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ಹರಿವು, "border-bottom-style: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED:
		fprintf(ಹರಿವು, "border-bottom-style: dotted ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED:
		fprintf(ಹರಿವು, "border-bottom-style: dashed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:
		fprintf(ಹರಿವು, "border-bottom-style: solid ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ಹರಿವು, "border-bottom-style: double ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE:
		fprintf(ಹರಿವು, "border-bottom-style: groove ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:
		fprintf(ಹರಿವು, "border-bottom-style: ridge ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:
		fprintf(ಹರಿವು, "border-bottom-style: inset ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET:
		fprintf(ಹರಿವು, "border-bottom-style: outset ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-left-style */
	val = css_computed_border_ಎಡ_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_NONE:
		fprintf(ಹರಿವು, "border-left-style: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ಹರಿವು, "border-left-style: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED:
		fprintf(ಹರಿವು, "border-left-style: dotted ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED:
		fprintf(ಹರಿವು, "border-left-style: dashed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:
		fprintf(ಹರಿವು, "border-left-style: solid ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ಹರಿವು, "border-left-style: double ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE:
		fprintf(ಹರಿವು, "border-left-style: groove ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:
		fprintf(ಹರಿವು, "border-left-style: ridge ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:
		fprintf(ಹರಿವು, "border-left-style: inset ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET:
		fprintf(ಹರಿವು, "border-left-style: outset ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-top-width */
	val = css_computed_border_ತುದಿ_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THIN:
		fprintf(ಹರಿವು, "border-top-width: thin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_MEDIUM:
		fprintf(ಹರಿವು, "border-top-width: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THICK:
		fprintf(ಹರಿವು, "border-top-width: thick ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_ಅಗಲ:
		fprintf(ಹರಿವು, "border-top-width: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-right-width */
	val = css_computed_border_ಬಲ_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THIN:
		fprintf(ಹರಿವು, "border-right-width: thin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_MEDIUM:
		fprintf(ಹರಿವು, "border-right-width: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THICK:
		fprintf(ಹರಿವು, "border-right-width: thick ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_ಅಗಲ:
		fprintf(ಹರಿವು, "border-right-width: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THIN:
		fprintf(ಹರಿವು, "border-bottom-width: thin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_MEDIUM:
		fprintf(ಹರಿವು, "border-bottom-width: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THICK:
		fprintf(ಹರಿವು, "border-bottom-width: thick ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_ಅಗಲ:
		fprintf(ಹರಿವು, "border-bottom-width: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* border-left-width */
	val = css_computed_border_ಎಡ_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THIN:
		fprintf(ಹರಿವು, "border-left-width: thin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_MEDIUM:
		fprintf(ಹರಿವು, "border-left-width: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_THICK:
		fprintf(ಹರಿವು, "border-left-width: thick ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BORDER_ಅಗಲ_ಅಗಲ:
		fprintf(ಹರಿವು, "border-left-width: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_BOTTOM_AUTO:
		fprintf(ಹರಿವು, "bottom: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_BOTTOM_ಗಣ:
		fprintf(ಹರಿವು, "bottom: ");
		dump_css_unit(ಹರಿವು, len1, unit1);
		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_CAPTION_SIDE_ತುದಿ:
		fprintf(ಹರಿವು, "caption_side: top ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(ಹರಿವು, "caption_side: bottom ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* clear */
	val = css_computed_clear(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_CLEAR_NONE:
		fprintf(ಹರಿವು, "clear: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CLEAR_ಎಡ:
		fprintf(ಹರಿವು, "clear: left ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CLEAR_ಬಲ:
		fprintf(ಹರಿವು, "clear: right ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CLEAR_BOTH:
		fprintf(ಹರಿವು, "clear: both ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_CLIP_AUTO:
		fprintf(ಹರಿವು, "clip: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CLIP_RECT:
		fprintf(ಹರಿವು, "clip: rect( ");

		ಆದರೆ (rect.ತುದಿ_auto)
			fprintf(ಹರಿವು, "auto");
		ಇಲ್ಲದಿದ್ದರೆ
			dump_css_unit(ಹರಿವು, rect.ತುದಿ, rect.tunit);
		fprintf(ಹರಿವು, ", ");

		ಆದರೆ (rect.ಬಲ_auto)
			fprintf(ಹರಿವು, "auto");
		ಇಲ್ಲದಿದ್ದರೆ
			dump_css_unit(ಹರಿವು, rect.ಬಲ, rect.runit);
		fprintf(ಹರಿವು, ", ");

		ಆದರೆ (rect.bottom_auto)
			fprintf(ಹರಿವು, "auto");
		ಇಲ್ಲದಿದ್ದರೆ
			dump_css_unit(ಹರಿವು, rect.bottom, rect.bunit);
		fprintf(ಹರಿವು, ", ");

		ಆದರೆ (rect.ಎಡ_auto)
			fprintf(ಹರಿವು, "auto");
		ಇಲ್ಲದಿದ್ದರೆ
			dump_css_unit(ಹರಿವು, rect.ಎಡ, rect.lunit);
		fprintf(ಹರಿವು, ") ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* color */
	val = css_computed_color(style, &color);
	ಆದರೆ (val == CSS_COLOR_COLOR) {
		fprintf(ಹರಿವು, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_CONTENT_NONE:
		fprintf(ಹರಿವು, "content: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CONTENT_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "content: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CONTENT_ಗಣ:
		fprintf(ಹರಿವು, "content:");

		ಆಗ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(ಹರಿವು, " ");

			ಆಯ್ಕೆ (content->type) {
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_STRING:
				fprintf(ಹರಿವು,	"\"%.*s\"",
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.string),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.string));
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_URI:
				fprintf(ಹರಿವು,	"uri(\"%.*s\")",
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.uri),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.uri));
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(ಹರಿವು, "counter(%.*s)",
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.counter.name),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.counter.name));
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(ಹರಿವು, "counters(%.*s, \"%.*s\")",
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.counters.name),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.counters.name),
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.counters.sep),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.counters.sep));
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(ಹರಿವು, "attr(%.*s)",
						(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(
						content->ದತ್ತಾಂಶ.attr),
						lwc_string_ದತ್ತಾಂಶ(
						content->ದತ್ತಾಂಶ.attr));
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(ಹರಿವು, "open-quote");
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(ಹರಿವು, "close-quote");
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(ಹರಿವು, "no-open-quote");
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(ಹರಿವು, "no-close-quote");
				ನಿಲ್ಲಿಸು;
			}

			content++;
		}

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	ಆದರೆ ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ಖಾಲಿ)) {
		fprintf(ಹರಿವು, "counter-increment: none ");
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(ಹರಿವು, "counter-increment:");

		ಆಗ (counter->name != ಖಾಲಿ) {
			fprintf(ಹರಿವು, " %.*s ",
					(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(counter->name),
					lwc_string_ದತ್ತಾಂಶ(counter->name));

			dump_css_fixed(ಹರಿವು, counter->ಮೌಲ್ಯ);

			counter++;
		}

		fprintf(ಹರಿವು, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	ಆದರೆ ((val == CSS_COUNTER_RESET_NONE) || (counter == ಖಾಲಿ)) {
		fprintf(ಹರಿವು, "counter-reset: none ");
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(ಹರಿವು, "counter-reset:");

		ಆಗ (counter->name != ಖಾಲಿ) {
			fprintf(ಹರಿವು, " %.*s ",
					(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(counter->name),
					lwc_string_ದತ್ತಾಂಶ(counter->name));

			dump_css_fixed(ಹರಿವು, counter->ಮೌಲ್ಯ);

			counter++;
		}

		fprintf(ಹರಿವು, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(ಹರಿವು, "cursor:");

	ಆದರೆ (string_list != ಖಾಲಿ) {
		ಆಗ (*string_list != ಖಾಲಿ) {
			fprintf(ಹರಿವು, " url\"%.*s\")",
					(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(*string_list),
					lwc_string_ದತ್ತಾಂಶ(*string_list));

			string_list++;
		}
	}
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_CURSOR_AUTO:
		fprintf(ಹರಿವು, " auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_CROSSHAIR:
		fprintf(ಹರಿವು, " crosshair ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_DEFAULT:
		fprintf(ಹರಿವು, " default ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಸೂಚಕ:
		fprintf(ಹರಿವು, " pointer ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_MOVE:
		fprintf(ಹರಿವು, " move ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_E_RESIZE:
		fprintf(ಹರಿವು, " e-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_NE_RESIZE:
		fprintf(ಹರಿವು, " ne-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_NW_RESIZE:
		fprintf(ಹರಿವು, " nw-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_N_RESIZE:
		fprintf(ಹರಿವು, " n-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_SE_RESIZE:
		fprintf(ಹರಿವು, " se-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_SW_RESIZE:
		fprintf(ಹರಿವು, " sw-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_S_RESIZE:
		fprintf(ಹರಿವು, " s-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_W_RESIZE:
		fprintf(ಹರಿವು, " w-resize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಪಠ್ಯ:
		fprintf(ಹರಿವು, " text ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_WAIT:
		fprintf(ಹರಿವು, " wait ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_HELP:
		fprintf(ಹರಿವು, " help ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಪ್ರಗತಿ:
		fprintf(ಹರಿವು, " progress ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* direction */
	val = css_computed_direction(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_DIRECTION_LTR:
		fprintf(ಹರಿವು, "direction: ltr ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DIRECTION_RTL:
		fprintf(ಹರಿವು, "direction: rtl ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* display */
	val = ns_computed_display_static(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE:
		fprintf(ಹರಿವು, "display: inline ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಖಂಡ:
		fprintf(ಹರಿವು, "display: block ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_LIST_ಅಂಶ:
		fprintf(ಹರಿವು, "display: list-item ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಸತತ_ಒಳ:
		fprintf(ಹರಿವು, "display: run-in ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE_ಖಂಡ:
		fprintf(ಹರಿವು, "display: inline-block ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ:
		fprintf(ಹರಿವು, "display: table ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE_ಕೋಷ್ಟಕ:
		fprintf(ಹರಿವು, "display: inline-table ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		fprintf(ಹರಿವು, "display: table-row-group ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_ಗುಂಪು:
		fprintf(ಹರಿವು, "display: table-header-group ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_FOOTER_ಗುಂಪು:
		fprintf(ಹರಿವು, "display: table-footer-group ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		fprintf(ಹರಿವು, "display: table-row ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗುಂಪು:
		fprintf(ಹರಿವು, "display: table-column-group ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_ಸ್ತಂಭ:
		fprintf(ಹರಿವು, "display: table-column ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_CELL:
		fprintf(ಹರಿವು, "display: table-cell ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_ಕೋಷ್ಟಕ_CAPTION:
		fprintf(ಹರಿವು, "display: table-caption ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_NONE:
		fprintf(ಹರಿವು, "display: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_FLEX:
		fprintf(ಹರಿವು, "display: flex ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE_FLEX:
		fprintf(ಹರಿವು, "display: inline-flex ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* empty-cells */
	val = css_computed_ಖಾಲಿಗಳು_cells(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಖಾಲಿಗಳು_CELLS_SHOW:
		fprintf(ಹರಿವು, "empty-cells: show ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಖಾಲಿಗಳು_CELLS_HIDE:
		fprintf(ಹರಿವು, "empty-cells: hide ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* float */
	val = css_computed_float(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_FLOAT_ಎಡ:
		fprintf(ಹರಿವು, "float: left ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FLOAT_ಬಲ:
		fprintf(ಹರಿವು, "float: right ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FLOAT_NONE:
		fprintf(ಹರಿವು, "float: none ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	ಆದರೆ (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(ಹರಿವು, "font-family:");

		ಆದರೆ (string_list != ಖಾಲಿ) {
			ಆಗ (*string_list != ಖಾಲಿ) {
				fprintf(ಹರಿವು, " \"%.*s\"",
					(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(*string_list),
					lwc_string_ದತ್ತಾಂಶ(*string_list));

				string_list++;
			}
		}
		ಆಯ್ಕೆ (val) {
		ಸ್ಥಿತಿ CSS_FONT_FAMILY_SERIF:
			fprintf(ಹರಿವು, " serif ");
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(ಹರಿವು, " sans-serif ");
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_FONT_FAMILY_CURSIVE:
			fprintf(ಹರಿವು, " cursive ");
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_FONT_FAMILY_FANTASY:
			fprintf(ಹರಿವು, " fantasy ");
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(ಹರಿವು, " monospace ");
			ನಿಲ್ಲಿಸು;
		}
	}

	/* font-size */
	val = css_computed_font_ಗಾತ್ರ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_XX_SMALL:
		fprintf(ಹರಿವು, "font-size: xx-small ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_X_SMALL:
		fprintf(ಹರಿವು, "font-size: x-small ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_SMALL:
		fprintf(ಹರಿವು, "font-size: small ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_MEDIUM:
		fprintf(ಹರಿವು, "font-size: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_LARGE:
		fprintf(ಹರಿವು, "font-size: large ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_X_LARGE:
		fprintf(ಹರಿವು, "font-size: x-large ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_XX_LARGE:
		fprintf(ಹರಿವು, "font-size: xx-large ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_LARGER:
		fprintf(ಹರಿವು, "font-size: larger ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_SMALLER:
		fprintf(ಹರಿವು, "font-size: smaller ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ಗಾತ್ರ_DIMENSION:
		fprintf(ಹರಿವು, "font-size: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* font-style */
	val = css_computed_font_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_FONT_STYLE_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "font-style: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_STYLE_ITALIC:
		fprintf(ಹರಿವು, "font-style: italic ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_STYLE_OBLIQUE:
		fprintf(ಹರಿವು, "font-style: oblique ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_FONT_VARIANT_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "font-variant: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(ಹರಿವು, "font-variant: small-caps ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* font-weight */
	val = css_computed_font_ತೂಕ(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "font-weight: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_BOLD:
		fprintf(ಹರಿವು, "font-weight: bold ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_BOLDER:
		fprintf(ಹರಿವು, "font-weight: bolder ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_LIGHTER:
		fprintf(ಹರಿವು, "font-weight: lighter ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_100:
		fprintf(ಹರಿವು, "font-weight: 100 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_200:
		fprintf(ಹರಿವು, "font-weight: 200 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_300:
		fprintf(ಹರಿವು, "font-weight: 300 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_400:
		fprintf(ಹರಿವು, "font-weight: 400 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_500:
		fprintf(ಹರಿವು, "font-weight: 500 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_600:
		fprintf(ಹರಿವು, "font-weight: 600 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_700:
		fprintf(ಹರಿವು, "font-weight: 700 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_800:
		fprintf(ಹರಿವು, "font-weight: 800 ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_900:
		fprintf(ಹರಿವು, "font-weight: 900 ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಎತ್ತರ*/
	val = css_computed_ಎತ್ತರ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಎತ್ತರ_AUTO:
		fprintf(ಹರಿವು, "height: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಎತ್ತರ_ಗಣ:
		fprintf(ಹರಿವು, "height: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಎಡ*/
	val = css_computed_ಎಡ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಎಡ_AUTO:
		fprintf(ಹರಿವು, "left: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಎಡ_ಗಣ:
		fprintf(ಹರಿವು, "left: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_LETTER_SPACING_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "letter-spacing: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LETTER_SPACING_ಗಣ:
		fprintf(ಹರಿವು, "letter-spacing: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* line-height */
	val = css_computed_ಸಾಲು_ಎತ್ತರ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಸಾಲು_ಎತ್ತರ_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "line-height: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಸಾಲು_ಎತ್ತರ_ಸಂಖ್ಯೆ:
		fprintf(ಹರಿವು, "line-height: ");

		dump_css_fixed(ಹರಿವು, len1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಸಾಲು_ಎತ್ತರ_DIMENSION:
		fprintf(ಹರಿವು, "line-height: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	ಆದರೆ (url != ಖಾಲಿ) {
		fprintf(ಹರಿವು, "list-style-image: url('%.*s') ",
				(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(url),
				lwc_string_ದತ್ತಾಂಶ(url));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(ಹರಿವು, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(ಹರಿವು, "list-style-position: inside ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(ಹರಿವು, "list-style-position: outside ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(ಹರಿವು, "list-style-type: disc ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(ಹರಿವು, "list-style-type: circle ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ವರ್ಗಫಲ:
		fprintf(ಹರಿವು, "list-style-type: square ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(ಹರಿವು, "list-style-type: decimal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(ಹರಿವು, "list-style-type: decimal-leading-zero ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಅಧೋ_ROMAN:
		fprintf(ಹರಿವು, "list-style-type: lower-roman ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಊರ್ಧ್ವ_ROMAN:
		fprintf(ಹರಿವು, "list-style-type: upper-roman ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಅಧೋ_GREEK:
		fprintf(ಹರಿವು, "list-style-type: lower-greek ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಅಧೋ_LATIN:
		fprintf(ಹರಿವು, "list-style-type: lower-latin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಊರ್ಧ್ವ_LATIN:
		fprintf(ಹರಿವು, "list-style-type: upper-latin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(ಹರಿವು, "list-style-type: armenian ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(ಹರಿವು, "list-style-type: georgian ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಅಧೋ_ALPHA:
		fprintf(ಹರಿವು, "list-style-type: lower-alpha ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ಊರ್ಧ್ವ_ALPHA:
		fprintf(ಹರಿವು, "list-style-type: upper-alpha ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(ಹರಿವು, "list-style-type: none ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* margin-top */
	val = css_computed_margin_ತುದಿ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MARGIN_AUTO:
		fprintf(ಹರಿವು, "margin-top: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MARGIN_ಗಣ:
		fprintf(ಹರಿವು, "margin-top: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* margin-right */
	val = css_computed_margin_ಬಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MARGIN_AUTO:
		fprintf(ಹರಿವು, "margin-right: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MARGIN_ಗಣ:
		fprintf(ಹರಿವು, "margin-right: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MARGIN_AUTO:
		fprintf(ಹರಿವು, "margin-bottom: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MARGIN_ಗಣ:
		fprintf(ಹರಿವು, "margin-bottom: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* margin-left */
	val = css_computed_margin_ಎಡ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MARGIN_AUTO:
		fprintf(ಹರಿವು, "margin-left: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MARGIN_ಗಣ:
		fprintf(ಹರಿವು, "margin-left: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* max-height */
	val = css_computed_max_ಎತ್ತರ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MAX_ಎತ್ತರ_NONE:
		fprintf(ಹರಿವು, "max-height: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MAX_ಎತ್ತರ_ಗಣ:
		fprintf(ಹರಿವು, "max-height: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* max-width */
	val = css_computed_max_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MAX_ಅಗಲ_NONE:
		fprintf(ಹರಿವು, "max-width: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_MAX_ಅಗಲ_ಗಣ:
		fprintf(ಹರಿವು, "max-width: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* min-height */
	val = ns_computed_min_ಎತ್ತರ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MIN_ಎತ್ತರ_ಗಣ:
		fprintf(ಹರಿವು, "min-height: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* min-width */
	val = ns_computed_min_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_MIN_ಅಗಲ_ಗಣ:
		fprintf(ಹರಿವು, "min-width: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_OPACITY_ಗಣ:
		fprintf(ಹರಿವು, "opacity: ");

		dump_css_fixed(ಹರಿವು, len1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_OUTLINE_COLOR_INVERT:
		fprintf(ಹರಿವು, "outline-color: invert ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_COLOR_COLOR:
		fprintf(ಹರಿವು, "outline-color: #%08"PRIx32" ", color);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_NONE:
		fprintf(ಹರಿವು, "outline-style: none ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(ಹರಿವು, "outline-style: dotted ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_DASHED:
		fprintf(ಹರಿವು, "outline-style: dashed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_SOLID:
		fprintf(ಹರಿವು, "outline-style: solid ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(ಹರಿವು, "outline-style: double ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(ಹರಿವು, "outline-style: groove ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(ಹರಿವು, "outline-style: ridge ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_INSET:
		fprintf(ಹರಿವು, "outline-style: inset ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(ಹರಿವು, "outline-style: outset ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* outline-width */
	val = css_computed_outline_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_OUTLINE_ಅಗಲ_THIN:
		fprintf(ಹರಿವು, "outline-width: thin ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_ಅಗಲ_MEDIUM:
		fprintf(ಹರಿವು, "outline-width: medium ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_ಅಗಲ_THICK:
		fprintf(ಹರಿವು, "outline-width: thick ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_OUTLINE_ಅಗಲ_ಅಗಲ:
		fprintf(ಹರಿವು, "outline-width: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಅತಿಪ್ರವಾಹ*/
	val = css_computed_ಅತಿಪ್ರವಾಹ_x(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_VISIBLE:
		fprintf(ಹರಿವು, "overflow-x: visible ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_HIDDEN:
		fprintf(ಹರಿವು, "overflow-x: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_SCROLL:
		fprintf(ಹರಿವು, "overflow-x: scroll ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_AUTO:
		fprintf(ಹರಿವು, "overflow-x auto ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಅತಿಪ್ರವಾಹ*/
	val = css_computed_ಅತಿಪ್ರವಾಹ_y(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_VISIBLE:
		fprintf(ಹರಿವು, "overflow-y: visible ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_HIDDEN:
		fprintf(ಹರಿವು, "overflow-y: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_SCROLL:
		fprintf(ಹರಿವು, "overflow-y: scroll ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅತಿಪ್ರವಾಹ_AUTO:
		fprintf(ಹರಿವು, "overflow-y: auto ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* padding-top */
	val = css_computed_padding_ತುದಿ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_PADDING_ಗಣ:
		fprintf(ಹರಿವು, "padding-top: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* padding-right */
	val = css_computed_padding_ಬಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_PADDING_ಗಣ:
		fprintf(ಹರಿವು, "padding-right: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_PADDING_ಗಣ:
		fprintf(ಹರಿವು, "padding-bottom: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* padding-left */
	val = css_computed_padding_ಎಡ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_PADDING_ಗಣ:
		fprintf(ಹರಿವು, "padding-left: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* position */
	val = css_computed_position(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_POSITION_STATIC:
		fprintf(ಹರಿವು, "position: static ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_POSITION_RELATIVE:
		fprintf(ಹರಿವು, "position: relative ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_POSITION_ABSOLUTE:
		fprintf(ಹರಿವು, "position: absolute ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_POSITION_FIXED:
		fprintf(ಹರಿವು, "position: fixed ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	ಆದರೆ (val == CSS_QUOTES_STRING && string_list != ಖಾಲಿ) {
		fprintf(ಹರಿವು, "quotes:");

		ಆಗ (*string_list != ಖಾಲಿ) {
			fprintf(ಹರಿವು, " \"%.*s\"",
				(ಪೂರ್ಣಾಂಕ) lwc_string_ಉದ್ದಳತೆ(*string_list),
				lwc_string_ದತ್ತಾಂಶ(*string_list));

			string_list++;
		}

		fprintf(ಹರಿವು, " ");
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆಯ್ಕೆ (val) {
		ಸ್ಥಿತಿ CSS_QUOTES_NONE:
			fprintf(ಹರಿವು, "quotes: none ");
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}
	}

	/*ಬಲ*/
	val = css_computed_ಬಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಬಲ_AUTO:
		fprintf(ಹರಿವು, "right: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಬಲ_ಗಣ:
		fprintf(ಹರಿವು, "right: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* table-layout */
	val = css_computed_ಕೋಷ್ಟಕ_layout(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಕೋಷ್ಟಕ_LAYOUT_AUTO:
		fprintf(ಹರಿವು, "table-layout: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಕೋಷ್ಟಕ_LAYOUT_FIXED:
		fprintf(ಹರಿವು, "table-layout: fixed ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* text-align */
	val = css_computed_ಪಠ್ಯ_align(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_ಎಡ:
		fprintf(ಹರಿವು, "text-align: left ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_ಬಲ:
		fprintf(ಹರಿವು, "text-align: right ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_CENTER:
		fprintf(ಹರಿವು, "text-align: center ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_JUSTIFY:
		fprintf(ಹರಿವು, "text-align: justify ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_DEFAULT:
		fprintf(ಹರಿವು, "text-align: default ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಎಡ:
		fprintf(ಹರಿವು, "text-align: -libcss-left ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_CENTER:
		fprintf(ಹರಿವು, "text-align: -libcss-center ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಬಲ:
		fprintf(ಹರಿವು, "text-align: -libcss-right ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* text-decoration */
	val = css_computed_ಪಠ್ಯ_decoration(style);
	ಆದರೆ (val == CSS_ಪಠ್ಯ_DECORATION_NONE) {
		fprintf(ಹರಿವು, "text-decoration: none ");
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(ಹರಿವು, "text-decoration:");

		ಆದರೆ (val & CSS_ಪಠ್ಯ_DECORATION_BLINK) {
			fprintf(ಹರಿವು, " blink");
		}
		ಆದರೆ (val & CSS_ಪಠ್ಯ_DECORATION_ಸಾಲು_THROUGH) {
			fprintf(ಹರಿವು, " line-through");
		}
		ಆದರೆ (val & CSS_ಪಠ್ಯ_DECORATION_OVERLINE) {
			fprintf(ಹರಿವು, " overline");
		}
		ಆದರೆ (val & CSS_ಪಠ್ಯ_DECORATION_UNDERLINE) {
			fprintf(ಹರಿವು, " underline");
		}

		fprintf(ಹರಿವು, " ");
	}

	/* text-indent */
	val = css_computed_ಪಠ್ಯ_indent(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_INDENT_ಗಣ:
		fprintf(ಹರಿವು, "text-indent: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* text-transform */
	val = css_computed_ಪಠ್ಯ_transform(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_CAPITALIZE:
		fprintf(ಹರಿವು, "text-transform: capitalize ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_UPPERCASE:
		fprintf(ಹರಿವು, "text-transform: uppercase ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_LOWERCASE:
		fprintf(ಹರಿವು, "text-transform: lowercase ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_TRANSFORM_NONE:
		fprintf(ಹರಿವು, "text-transform: none ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ತುದಿ*/
	val = css_computed_ತುದಿ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ತುದಿ_AUTO:
		fprintf(ಹರಿವು, "top: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ತುದಿ_ಗಣ:
		fprintf(ಹರಿವು, "top: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_UNICODE_BIDI_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "unicode-bidi: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNICODE_BIDI_EMBED:
		fprintf(ಹರಿವು, "unicode-bidi: embed ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(ಹರಿವು, "unicode-bidi: bidi-override ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(ಹರಿವು, "vertical-align: baseline ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUB:
		fprintf(ಹರಿವು, "vertical-align: sub ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(ಹರಿವು, "vertical-align: super ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ತುದಿ:
		fprintf(ಹರಿವು, "vertical-align: top ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_ತುದಿ:
		fprintf(ಹರಿವು, "vertical-align: text-top ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಮಧ್ಯ:
		fprintf(ಹರಿವು, "vertical-align: middle ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(ಹರಿವು, "vertical-align: bottom ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_BOTTOM:
		fprintf(ಹರಿವು, "vertical-align: text-bottom ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಗಣ:
		fprintf(ಹರಿವು, "vertical-align: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* visibility */
	val = css_computed_visibility(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_VISIBILITY_VISIBLE:
		fprintf(ಹರಿವು, "visibility: visible ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VISIBILITY_HIDDEN:
		fprintf(ಹರಿವು, "visibility: hidden ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_VISIBILITY_COLLAPSE:
		fprintf(ಹರಿವು, "visibility: collapse ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* white-space */
	val = css_computed_white_space(style);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_WHITE_SPACE_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "white-space: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_WHITE_SPACE_PRE:
		fprintf(ಹರಿವು, "white-space: pre ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_WHITE_SPACE_NOWRAP:
		fprintf(ಹರಿವು, "white-space: nowrap ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(ಹರಿವು, "white-space: pre-wrap ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_WHITE_SPACE_PRE_ಸಾಲು:
		fprintf(ಹರಿವು, "white-space: pre-line ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/*ಅಗಲ*/
	val = css_computed_ಅಗಲ(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಅಗಲ_AUTO:
		fprintf(ಹರಿವು, "width: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಅಗಲ_ಗಣ:
		fprintf(ಹರಿವು, "width: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* word-spacing */
	val = css_computed_ಶಬ್ದ_spacing(style, &len1, &unit1);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_ಶಬ್ದ_SPACING_ಸಾಮಾನ್ಯ:
		fprintf(ಹರಿವು, "word-spacing: normal ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಶಬ್ದ_SPACING_ಗಣ:
		fprintf(ಹರಿವು, "word-spacing: ");

		dump_css_unit(ಹರಿವು, len1, unit1);

		fprintf(ಹರಿವು, " ");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* z-index */
	val = css_computed_z_ಸೂಚ್ಯಂಕ(style, &zindex);
	ಆಯ್ಕೆ (val) {
	ಸ್ಥಿತಿ CSS_Z_ಸೂಚ್ಯಂಕ_AUTO:
		fprintf(ಹರಿವು, "z-index: auto ");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_Z_ಸೂಚ್ಯಂಕ_ಗಣ:
		fprintf(ಹರಿವು, "z-index: %"PRId32" ", zindex);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	fprintf(ಹರಿವು, "}");
}
