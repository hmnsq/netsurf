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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <libcss/libcss.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"

#അന്തര്ഭാവ "css/dump.h"
#അന്തര്ഭാവ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
സ്ഥിതം ശൂന്യം dump_css_fixed(FILE *stream, css_fixed f)
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
സ്ഥിതം ശൂന്യം dump_css_സംഖ്യ(FILE *stream, css_fixed val)
{
	എങ്കിൽ (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	അല്ലെങ്കിൽ
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
സ്ഥിതം ശൂന്യം dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_സംഖ്യ(stream, val);

	തിരഞ്ഞെടുപ്പ് (unit) {
	അവസ്ഥ CSS_UNIT_PX:
		fprintf(stream, "px");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_EX:
		fprintf(stream, "ex");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_EM:
		fprintf(stream, "em");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_IN:
		fprintf(stream, "in");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_CM:
		fprintf(stream, "cm");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_MM:
		fprintf(stream, "mm");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_PT:
		fprintf(stream, "pt");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_PC:
		fprintf(stream, "pc");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_PCT:
		fprintf(stream, "%%");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_DEG:
		fprintf(stream, "deg");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_RAD:
		fprintf(stream, "rad");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_MS:
		fprintf(stream, "ms");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_S:
		fprintf(stream, "s");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_CH:
		fprintf(stream, "ch");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_REM:
		fprintf(stream, "rem");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_LH:
		fprintf(stream, "lh");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VH:
		fprintf(stream, "vh");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VW:
		fprintf(stream, "vw");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VI:
		fprintf(stream, "vi");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VB:
		fprintf(stream, "vb");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_Q:
		fprintf(stream, "q");
		നിർത്തുക;
	അവസ്ഥ CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		നിർത്തുക;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
ശൂന്യം nscss_dump_computed_style(FILE *stream, സ്ഥിരം css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ശൂന്യ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	സ്ഥിരം css_computed_content_item *content = ശൂന്യ;
	സ്ഥിരം css_computed_counter *counter = ശൂന്യ;
	lwc_string **string_list = ശൂന്യ;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		നിർത്തുക;
	അവസ്ഥ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	എങ്കിൽ (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ശൂന്യ) {
		fprintf(stream, "background-image: url('%.*s') ",
				(പൂർണ്ണസംഖ്യ) lwc_string_length(url),
				lwc_string_data(url));
	} അല്ലെങ്കിൽ എങ്കിൽ (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	എങ്കിൽ (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		നിർത്തുക;
	അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		നിർത്തുക;
	അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		നിർത്തുക;
	അവസ്ഥ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:

		നിർത്തുക;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	എങ്കിൽ (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_മുകൾ_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-right-color */
	val = css_computed_border_വലത്_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-left-color */
	val = css_computed_border_ഇടത്_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-top-style */
	val = css_computed_border_മുകൾ_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-right-style */
	val = css_computed_border_വലത്_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-left-style */
	val = css_computed_border_ഇടത്_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-top-width */
	val = css_computed_border_മുകൾ_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-right-width */
	val = css_computed_border_വലത്_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* border-left-width */
	val = css_computed_border_ഇടത്_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		നിർത്തുക;
	അവസ്ഥ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_CAPTION_SIDE_മുകൾ:
		fprintf(stream, "caption_side: top ");
		നിർത്തുക;
	അവസ്ഥ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* clear */
	val = css_computed_clear(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_CLEAR_ഇടത്:
		fprintf(stream, "clear: left ");
		നിർത്തുക;
	അവസ്ഥ CSS_CLEAR_വലത്:
		fprintf(stream, "clear: right ");
		നിർത്തുക;
	അവസ്ഥ CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		എങ്കിൽ (rect.മുകൾ_auto)
			fprintf(stream, "auto");
		അല്ലെങ്കിൽ
			dump_css_unit(stream, rect.മുകൾ, rect.tunit);
		fprintf(stream, ", ");

		എങ്കിൽ (rect.വലത്_auto)
			fprintf(stream, "auto");
		അല്ലെങ്കിൽ
			dump_css_unit(stream, rect.വലത്, rect.runit);
		fprintf(stream, ", ");

		എങ്കിൽ (rect.bottom_auto)
			fprintf(stream, "auto");
		അല്ലെങ്കിൽ
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		എങ്കിൽ (rect.ഇടത്_auto)
			fprintf(stream, "auto");
		അല്ലെങ്കിൽ
			dump_css_unit(stream, rect.ഇടത്, rect.lunit);
		fprintf(stream, ") ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* color */
	val = css_computed_color(style, &color);
	എങ്കിൽ (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_CONTENT_SET:
		fprintf(stream, "content:");

		ആകുമ്പോൾ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			തിരഞ്ഞെടുപ്പ് (content->type) {
			അവസ്ഥ CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(പൂർണ്ണസംഖ്യ) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				നിർത്തുക;
			അവസ്ഥ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				നിർത്തുക;
			}

			content++;
		}

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	എങ്കിൽ ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ശൂന്യ)) {
		fprintf(stream, "counter-increment: none ");
	} അല്ലെങ്കിൽ {
		fprintf(stream, "counter-increment:");

		ആകുമ്പോൾ (counter->name != ശൂന്യ) {
			fprintf(stream, " %.*s ",
					(പൂർണ്ണസംഖ്യ) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->മൂല്യം);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	എങ്കിൽ ((val == CSS_COUNTER_RESET_NONE) || (counter == ശൂന്യ)) {
		fprintf(stream, "counter-reset: none ");
	} അല്ലെങ്കിൽ {
		fprintf(stream, "counter-reset:");

		ആകുമ്പോൾ (counter->name != ശൂന്യ) {
			fprintf(stream, " %.*s ",
					(പൂർണ്ണസംഖ്യ) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->മൂല്യം);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	എങ്കിൽ (string_list != ശൂന്യ) {
		ആകുമ്പോൾ (*string_list != ശൂന്യ) {
			fprintf(stream, " url\"%.*s\")",
					(പൂർണ്ണസംഖ്യ) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* direction */
	val = css_computed_direction(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		നിർത്തുക;
	അവസ്ഥ CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* display */
	val = ns_computed_display_static(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		നിർത്തുക;
	അവസ്ഥ CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		നിർത്തുക;
	അവസ്ഥ CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* float */
	val = css_computed_float(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_FLOAT_ഇടത്:
		fprintf(stream, "float: left ");
		നിർത്തുക;
	അവസ്ഥ CSS_FLOAT_വലത്:
		fprintf(stream, "float: right ");
		നിർത്തുക;
	അവസ്ഥ CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	എങ്കിൽ (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		എങ്കിൽ (string_list != ശൂന്യ) {
			ആകുമ്പോൾ (*string_list != ശൂന്യ) {
				fprintf(stream, " \"%.*s\"",
					(പൂർണ്ണസംഖ്യ) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		തിരഞ്ഞെടുപ്പ് (val) {
		അവസ്ഥ CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			നിർത്തുക;
		അവസ്ഥ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			നിർത്തുക;
		അവസ്ഥ CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			നിർത്തുക;
		അവസ്ഥ CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			നിർത്തുക;
		അവസ്ഥ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			നിർത്തുക;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* font-style */
	val = css_computed_font_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/*ഇടത്*/
	val = css_computed_ഇടത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_ഇടത്_AUTO:
		fprintf(stream, "left: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_ഇടത്_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* line-height */
	val = css_computed_വരി_height(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_വരി_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_വരി_HEIGHT_സംഖ്യ:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		നിർത്തുക;
	അവസ്ഥ CSS_വരി_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	എങ്കിൽ (url != ശൂന്യ) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(പൂർണ്ണസംഖ്യ) lwc_string_length(url),
				lwc_string_data(url));
	} അല്ലെങ്കിൽ എങ്കിൽ (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		നിർത്തുക;
	അവസ്ഥ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* margin-top */
	val = css_computed_margin_മുകൾ(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* margin-right */
	val = css_computed_margin_വലത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* margin-left */
	val = css_computed_margin_ഇടത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		നിർത്തുക;
	അവസ്ഥ CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		നിർത്തുക;
	അവസ്ഥ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* padding-top */
	val = css_computed_padding_മുകൾ(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* padding-right */
	val = css_computed_padding_വലത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* padding-left */
	val = css_computed_padding_ഇടത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* position */
	val = css_computed_position(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		നിർത്തുക;
	അവസ്ഥ CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		നിർത്തുക;
	അവസ്ഥ CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		നിർത്തുക;
	അവസ്ഥ CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	എങ്കിൽ (val == CSS_QUOTES_STRING && string_list != ശൂന്യ) {
		fprintf(stream, "quotes:");

		ആകുമ്പോൾ (*string_list != ശൂന്യ) {
			fprintf(stream, " \"%.*s\"",
				(പൂർണ്ണസംഖ്യ) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} അല്ലെങ്കിൽ {
		തിരഞ്ഞെടുപ്പ് (val) {
		അവസ്ഥ CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}
	}

	/*വലത്*/
	val = css_computed_വലത്(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_വലത്_AUTO:
		fprintf(stream, "right: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_വലത്_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* text-align */
	val = css_computed_text_align(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_TEXT_ALIGN_ഇടത്:
		fprintf(stream, "text-align: left ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_വലത്:
		fprintf(stream, "text-align: right ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_ഇടത്:
		fprintf(stream, "text-align: -libcss-left ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_വലത്:
		fprintf(stream, "text-align: -libcss-right ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	എങ്കിൽ (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} അല്ലെങ്കിൽ {
		fprintf(stream, "text-decoration:");

		എങ്കിൽ (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		എങ്കിൽ (val & CSS_TEXT_DECORATION_വരി_THROUGH) {
			fprintf(stream, " line-through");
		}
		എങ്കിൽ (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		എങ്കിൽ (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/*മുകൾ*/
	val = css_computed_മുകൾ(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_മുകൾ_AUTO:
		fprintf(stream, "top: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_മുകൾ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		നിർത്തുക;
	അവസ്ഥ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_മുകൾ:
		fprintf(stream, "vertical-align: top ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_മുകൾ:
		fprintf(stream, "vertical-align: text-top ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_നടുവ്:
		fprintf(stream, "vertical-align: middle ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		നിർത്തുക;
	അവസ്ഥ CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* visibility */
	val = css_computed_visibility(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		നിർത്തുക;
	അവസ്ഥ CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		നിർത്തുക;
	അവസ്ഥ CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* white-space */
	val = css_computed_white_space(style);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		നിർത്തുക;
	അവസ്ഥ CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		നിർത്തുക;
	അവസ്ഥ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		നിർത്തുക;
	അവസ്ഥ CSS_WHITE_SPACE_PRE_വരി:
		fprintf(stream, "white-space: pre-line ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		നിർത്തുക;
	അവസ്ഥ CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	തിരഞ്ഞെടുപ്പ് (val) {
	അവസ്ഥ CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		നിർത്തുക;
	അവസ്ഥ CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	fprintf(stream, "}");
}
