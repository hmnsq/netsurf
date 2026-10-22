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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <libcss/libcss.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"

#ଅନ୍ତର୍ଭାଵ "css/dump.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
ସ୍ଥିର ଶୂନ୍ୟ dump_css_fixed(FILE *stream, css_fixed f)
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
ସ୍ଥିର ଶୂନ୍ୟ dump_css_ସଂଖ୍ୟା(FILE *stream, css_fixed val)
{
	ଯଦି (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	ଅନ୍ୟଥା
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ସ୍ଥିର ଶୂନ୍ୟ dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_ସଂଖ୍ୟା(stream, val);

	ବାଛ (unit) {
	ସ୍ଥିତି CSS_UNIT_PX:
		fprintf(stream, "px");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_EX:
		fprintf(stream, "ex");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_EM:
		fprintf(stream, "em");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_IN:
		fprintf(stream, "in");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_CM:
		fprintf(stream, "cm");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_MM:
		fprintf(stream, "mm");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_PT:
		fprintf(stream, "pt");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_PC:
		fprintf(stream, "pc");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_PCT:
		fprintf(stream, "%%");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_DEG:
		fprintf(stream, "deg");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_RAD:
		fprintf(stream, "rad");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_MS:
		fprintf(stream, "ms");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_S:
		fprintf(stream, "s");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_CH:
		fprintf(stream, "ch");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_REM:
		fprintf(stream, "rem");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_LH:
		fprintf(stream, "lh");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VH:
		fprintf(stream, "vh");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VW:
		fprintf(stream, "vw");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VI:
		fprintf(stream, "vi");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VB:
		fprintf(stream, "vb");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_Q:
		fprintf(stream, "q");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		ଭାଙ୍ଗ;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
ଶୂନ୍ୟ nscss_dump_computed_style(FILE *stream, ଅଚଳ css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ଖାଲି;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ଅଚଳ css_computed_content_item *content = ଖାଲି;
	ଅଚଳ css_computed_counter *counter = ଖାଲି;
	lwc_string **string_list = ଖାଲି;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	ଯଦି (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ଖାଲି) {
		fprintf(stream, "background-image: url('%.*s') ",
				(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(url),
				lwc_string_data(url));
	} ଅନ୍ୟଥା ଯଦି (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	ଯଦି (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:

		ଭାଙ୍ଗ;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	ଯଦି (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_ଶୀର୍ଷ_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-right-color */
	val = css_computed_border_ଡାହାଣ_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-left-color */
	val = css_computed_border_ବାମ_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-top-style */
	val = css_computed_border_ଶୀର୍ଷ_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-right-style */
	val = css_computed_border_ଡାହାଣ_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-left-style */
	val = css_computed_border_ବାମ_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-top-width */
	val = css_computed_border_ଶୀର୍ଷ_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-right-width */
	val = css_computed_border_ଡାହାଣ_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* border-left-width */
	val = css_computed_border_ବାମ_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_CAPTION_SIDE_ଶୀର୍ଷ:
		fprintf(stream, "caption_side: top ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* clear */
	val = css_computed_clear(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CLEAR_ବାମ:
		fprintf(stream, "clear: left ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CLEAR_ଡାହାଣ:
		fprintf(stream, "clear: right ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		ଯଦି (rect.ଶୀର୍ଷ_auto)
			fprintf(stream, "auto");
		ଅନ୍ୟଥା
			dump_css_unit(stream, rect.ଶୀର୍ଷ, rect.tunit);
		fprintf(stream, ", ");

		ଯଦି (rect.ଡାହାଣ_auto)
			fprintf(stream, "auto");
		ଅନ୍ୟଥା
			dump_css_unit(stream, rect.ଡାହାଣ, rect.runit);
		fprintf(stream, ", ");

		ଯଦି (rect.bottom_auto)
			fprintf(stream, "auto");
		ଅନ୍ୟଥା
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		ଯଦି (rect.ବାମ_auto)
			fprintf(stream, "auto");
		ଅନ୍ୟଥା
			dump_css_unit(stream, rect.ବାମ, rect.lunit);
		fprintf(stream, ") ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* color */
	val = css_computed_color(style, &color);
	ଯଦି (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CONTENT_SET:
		fprintf(stream, "content:");

		ଯେତେବେଳେ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			ବାଛ (content->type) {
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				ଭାଙ୍ଗ;
			}

			content++;
		}

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	ଯଦି ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ଖାଲି)) {
		fprintf(stream, "counter-increment: none ");
	} ଅନ୍ୟଥା {
		fprintf(stream, "counter-increment:");

		ଯେତେବେଳେ (counter->name != ଖାଲି) {
			fprintf(stream, " %.*s ",
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->ମୂଲ୍ୟ);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	ଯଦି ((val == CSS_COUNTER_RESET_NONE) || (counter == ଖାଲି)) {
		fprintf(stream, "counter-reset: none ");
	} ଅନ୍ୟଥା {
		fprintf(stream, "counter-reset:");

		ଯେତେବେଳେ (counter->name != ଖାଲି) {
			fprintf(stream, " %.*s ",
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->ମୂଲ୍ୟ);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	ଯଦି (string_list != ଖାଲି) {
		ଯେତେବେଳେ (*string_list != ଖାଲି) {
			fprintf(stream, " url\"%.*s\")",
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	ବାଛ (val) {
	ସ୍ଥିତି CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* direction */
	val = css_computed_direction(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* display */
	val = ns_computed_display_static(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* float */
	val = css_computed_float(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_FLOAT_ବାମ:
		fprintf(stream, "float: left ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FLOAT_ଡାହାଣ:
		fprintf(stream, "float: right ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	ଯଦି (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		ଯଦି (string_list != ଖାଲି) {
			ଯେତେବେଳେ (*string_list != ଖାଲି) {
				fprintf(stream, " \"%.*s\"",
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		ବାଛ (val) {
		ସ୍ଥିତି CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			ଭାଙ୍ଗ;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* font-style */
	val = css_computed_font_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/*ବାମ*/
	val = css_computed_ବାମ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_ବାମ_AUTO:
		fprintf(stream, "left: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_ବାମ_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* line-height */
	val = css_computed_ଧାଡ଼ି_height(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_ଧାଡ଼ି_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_ଧାଡ଼ି_HEIGHT_ସଂଖ୍ୟା:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_ଧାଡ଼ି_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	ଯଦି (url != ଖାଲି) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(url),
				lwc_string_data(url));
	} ଅନ୍ୟଥା ଯଦି (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* margin-top */
	val = css_computed_margin_ଶୀର୍ଷ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* margin-right */
	val = css_computed_margin_ଡାହାଣ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* margin-left */
	val = css_computed_margin_ବାମ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* padding-top */
	val = css_computed_padding_ଶୀର୍ଷ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* padding-right */
	val = css_computed_padding_ଡାହାଣ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* padding-left */
	val = css_computed_padding_ବାମ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* position */
	val = css_computed_position(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	ଯଦି (val == CSS_QUOTES_STRING && string_list != ଖାଲି) {
		fprintf(stream, "quotes:");

		ଯେତେବେଳେ (*string_list != ଖାଲି) {
			fprintf(stream, " \"%.*s\"",
				(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} ଅନ୍ୟଥା {
		ବାଛ (val) {
		ସ୍ଥିତି CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}
	}

	/*ଡାହାଣ*/
	val = css_computed_ଡାହାଣ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_ଡାହାଣ_AUTO:
		fprintf(stream, "right: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_ଡାହାଣ_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* text-align */
	val = css_computed_text_align(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_TEXT_ALIGN_ବାମ:
		fprintf(stream, "text-align: left ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_ଡାହାଣ:
		fprintf(stream, "text-align: right ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ବାମ:
		fprintf(stream, "text-align: -libcss-left ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ଡାହାଣ:
		fprintf(stream, "text-align: -libcss-right ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	ଯଦି (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} ଅନ୍ୟଥା {
		fprintf(stream, "text-decoration:");

		ଯଦି (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		ଯଦି (val & CSS_TEXT_DECORATION_ଧାଡ଼ି_THROUGH) {
			fprintf(stream, " line-through");
		}
		ଯଦି (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		ଯଦି (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/*ଶୀର୍ଷ*/
	val = css_computed_ଶୀର୍ଷ(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_ଶୀର୍ଷ_AUTO:
		fprintf(stream, "top: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_ଶୀର୍ଷ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_ଶୀର୍ଷ:
		fprintf(stream, "vertical-align: top ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_ଶୀର୍ଷ:
		fprintf(stream, "vertical-align: text-top ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_ମଧ୍ୟ:
		fprintf(stream, "vertical-align: middle ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* visibility */
	val = css_computed_visibility(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* white-space */
	val = css_computed_white_space(style);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WHITE_SPACE_PRE_ଧାଡ଼ି:
		fprintf(stream, "white-space: pre-line ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	ବାଛ (val) {
	ସ୍ଥିତି CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	fprintf(stream, "}");
}
