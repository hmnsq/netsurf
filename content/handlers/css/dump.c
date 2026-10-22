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
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_fixed(FILE *stream, css_fixed f)
{
#define NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#undef NSCSS_ABS

	fprintf(stream, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_ಸಂಖ್ಯೆ(FILE *stream, css_fixed val)
{
	ಯದಿ (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	ಅನ್ಯಥಾ
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ಸ್ಥಿರ ಶೂನ್ಯ dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_ಸಂಖ್ಯೆ(stream, val);

	ವಿಕಲ್ಪ (unit) {
	ಪಕ್ಷ CSS_UNIT_PX:
		fprintf(stream, "px");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_EX:
		fprintf(stream, "ex");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_EM:
		fprintf(stream, "em");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_IN:
		fprintf(stream, "in");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_CM:
		fprintf(stream, "cm");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_MM:
		fprintf(stream, "mm");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_PT:
		fprintf(stream, "pt");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_PC:
		fprintf(stream, "pc");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_PCT:
		fprintf(stream, "%%");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_DEG:
		fprintf(stream, "deg");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_RAD:
		fprintf(stream, "rad");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_MS:
		fprintf(stream, "ms");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_S:
		fprintf(stream, "s");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_CH:
		fprintf(stream, "ch");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_REM:
		fprintf(stream, "rem");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_LH:
		fprintf(stream, "lh");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VH:
		fprintf(stream, "vh");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VW:
		fprintf(stream, "vw");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VI:
		fprintf(stream, "vi");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VB:
		fprintf(stream, "vb");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_Q:
		fprintf(stream, "q");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		ಭಙ್ಗ;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
ಶೂನ್ಯ nscss_dump_computed_style(FILE *stream, ಧ್ರುವ css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ರಿಕ್ತ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ಧ್ರುವ css_computed_content_item *content = ರಿಕ್ತ;
	ಧ್ರುವ css_computed_counter *counter = ರಿಕ್ತ;
	lwc_string **string_list = ರಿಕ್ತ;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	ಯದಿ (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ರಿಕ್ತ) {
		fprintf(stream, "background-image: url('%.*s') ",
				(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(url),
				lwc_string_data(url));
	} ಅನ್ಯಥಾ ಯದಿ (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	ಯದಿ (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:

		ಭಙ್ಗ;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	ಯದಿ (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_ತುದಿ_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-right-color */
	val = css_computed_border_ಬಲ_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-left-color */
	val = css_computed_border_ಎಡ_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-top-style */
	val = css_computed_border_ತುದಿ_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-right-style */
	val = css_computed_border_ಬಲ_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-left-style */
	val = css_computed_border_ಎಡ_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-top-width */
	val = css_computed_border_ತುದಿ_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-right-width */
	val = css_computed_border_ಬಲ_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* border-left-width */
	val = css_computed_border_ಎಡ_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_CAPTION_SIDE_ತುದಿ:
		fprintf(stream, "caption_side: top ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* clear */
	val = css_computed_clear(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CLEAR_ಎಡ:
		fprintf(stream, "clear: left ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CLEAR_ಬಲ:
		fprintf(stream, "clear: right ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		ಯದಿ (rect.ತುದಿ_auto)
			fprintf(stream, "auto");
		ಅನ್ಯಥಾ
			dump_css_unit(stream, rect.ತುದಿ, rect.tunit);
		fprintf(stream, ", ");

		ಯದಿ (rect.ಬಲ_auto)
			fprintf(stream, "auto");
		ಅನ್ಯಥಾ
			dump_css_unit(stream, rect.ಬಲ, rect.runit);
		fprintf(stream, ", ");

		ಯದಿ (rect.bottom_auto)
			fprintf(stream, "auto");
		ಅನ್ಯಥಾ
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		ಯದಿ (rect.ಎಡ_auto)
			fprintf(stream, "auto");
		ಅನ್ಯಥಾ
			dump_css_unit(stream, rect.ಎಡ, rect.lunit);
		fprintf(stream, ") ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* color */
	val = css_computed_color(style, &color);
	ಯದಿ (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CONTENT_SET:
		fprintf(stream, "content:");

		ಯಾವತ್ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			ವಿಕಲ್ಪ (content->type) {
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				ಭಙ್ಗ;
			}

			content++;
		}

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	ಯದಿ ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ರಿಕ್ತ)) {
		fprintf(stream, "counter-increment: none ");
	} ಅನ್ಯಥಾ {
		fprintf(stream, "counter-increment:");

		ಯಾವತ್ (counter->name != ರಿಕ್ತ) {
			fprintf(stream, " %.*s ",
					(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->ಮೌಲ್ಯ);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	ಯದಿ ((val == CSS_COUNTER_RESET_NONE) || (counter == ರಿಕ್ತ)) {
		fprintf(stream, "counter-reset: none ");
	} ಅನ್ಯಥಾ {
		fprintf(stream, "counter-reset:");

		ಯಾವತ್ (counter->name != ರಿಕ್ತ) {
			fprintf(stream, " %.*s ",
					(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->ಮೌಲ್ಯ);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	ಯದಿ (string_list != ರಿಕ್ತ) {
		ಯಾವತ್ (*string_list != ರಿಕ್ತ) {
			fprintf(stream, " url\"%.*s\")",
					(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* direction */
	val = css_computed_direction(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* display */
	val = ns_computed_display_static(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* float */
	val = css_computed_float(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_FLOAT_ಎಡ:
		fprintf(stream, "float: left ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FLOAT_ಬಲ:
		fprintf(stream, "float: right ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	ಯದಿ (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		ಯದಿ (string_list != ರಿಕ್ತ) {
			ಯಾವತ್ (*string_list != ರಿಕ್ತ) {
				fprintf(stream, " \"%.*s\"",
					(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		ವಿಕಲ್ಪ (val) {
		ಪಕ್ಷ CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			ಭಙ್ಗ;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* font-style */
	val = css_computed_font_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/*ಎಡ*/
	val = css_computed_ಎಡ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_ಎಡ_AUTO:
		fprintf(stream, "left: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_ಎಡ_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* line-height */
	val = css_computed_ಸಾಲು_height(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_ಸಾಲು_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_ಸಾಲು_HEIGHT_ಸಂಖ್ಯೆ:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_ಸಾಲು_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	ಯದಿ (url != ರಿಕ್ತ) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(url),
				lwc_string_data(url));
	} ಅನ್ಯಥಾ ಯದಿ (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* margin-top */
	val = css_computed_margin_ತುದಿ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* margin-right */
	val = css_computed_margin_ಬಲ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* margin-left */
	val = css_computed_margin_ಎಡ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* padding-top */
	val = css_computed_padding_ತುದಿ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* padding-right */
	val = css_computed_padding_ಬಲ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* padding-left */
	val = css_computed_padding_ಎಡ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* position */
	val = css_computed_position(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	ಯದಿ (val == CSS_QUOTES_STRING && string_list != ರಿಕ್ತ) {
		fprintf(stream, "quotes:");

		ಯಾವತ್ (*string_list != ರಿಕ್ತ) {
			fprintf(stream, " \"%.*s\"",
				(ಪೂರ್ಣಾಙ್ಕ) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} ಅನ್ಯಥಾ {
		ವಿಕಲ್ಪ (val) {
		ಪಕ್ಷ CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}
	}

	/*ಬಲ*/
	val = css_computed_ಬಲ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_ಬಲ_AUTO:
		fprintf(stream, "right: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_ಬಲ_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* text-align */
	val = css_computed_text_align(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_TEXT_ALIGN_ಎಡ:
		fprintf(stream, "text-align: left ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_ಬಲ:
		fprintf(stream, "text-align: right ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಎಡ:
		fprintf(stream, "text-align: -libcss-left ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಬಲ:
		fprintf(stream, "text-align: -libcss-right ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	ಯದಿ (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} ಅನ್ಯಥಾ {
		fprintf(stream, "text-decoration:");

		ಯದಿ (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		ಯದಿ (val & CSS_TEXT_DECORATION_ಸಾಲು_THROUGH) {
			fprintf(stream, " line-through");
		}
		ಯದಿ (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		ಯದಿ (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/*ತುದಿ*/
	val = css_computed_ತುದಿ(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_ತುದಿ_AUTO:
		fprintf(stream, "top: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_ತುದಿ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_ತುದಿ:
		fprintf(stream, "vertical-align: top ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_ತುದಿ:
		fprintf(stream, "vertical-align: text-top ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_ಮಧ್ಯ:
		fprintf(stream, "vertical-align: middle ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* visibility */
	val = css_computed_visibility(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* white-space */
	val = css_computed_white_space(style);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WHITE_SPACE_PRE_ಸಾಲು:
		fprintf(stream, "white-space: pre-line ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	ವಿಕಲ್ಪ (val) {
	ಪಕ್ಷ CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	fprintf(stream, "}");
}
