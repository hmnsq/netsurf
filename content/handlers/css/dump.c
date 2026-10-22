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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <libcss/libcss.h>

#অন্তর্ভাব "netsurf/inttypes.h"

#অন্তর্ভাব "css/dump.h"
#অন্তর্ভাব "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
স্থির শূন্য dump_css_fixed(FILE *stream, css_fixed f)
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
স্থির শূন্য dump_css_সংখ্যা(FILE *stream, css_fixed val)
{
	যদি (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	অন্যথা
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
স্থির শূন্য dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_সংখ্যা(stream, val);

	বিকল্প (unit) {
	পক্ষ CSS_UNIT_PX:
		fprintf(stream, "px");
		ভঙ্গ;
	পক্ষ CSS_UNIT_EX:
		fprintf(stream, "ex");
		ভঙ্গ;
	পক্ষ CSS_UNIT_EM:
		fprintf(stream, "em");
		ভঙ্গ;
	পক্ষ CSS_UNIT_IN:
		fprintf(stream, "in");
		ভঙ্গ;
	পক্ষ CSS_UNIT_CM:
		fprintf(stream, "cm");
		ভঙ্গ;
	পক্ষ CSS_UNIT_MM:
		fprintf(stream, "mm");
		ভঙ্গ;
	পক্ষ CSS_UNIT_PT:
		fprintf(stream, "pt");
		ভঙ্গ;
	পক্ষ CSS_UNIT_PC:
		fprintf(stream, "pc");
		ভঙ্গ;
	পক্ষ CSS_UNIT_PCT:
		fprintf(stream, "%%");
		ভঙ্গ;
	পক্ষ CSS_UNIT_DEG:
		fprintf(stream, "deg");
		ভঙ্গ;
	পক্ষ CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		ভঙ্গ;
	পক্ষ CSS_UNIT_RAD:
		fprintf(stream, "rad");
		ভঙ্গ;
	পক্ষ CSS_UNIT_MS:
		fprintf(stream, "ms");
		ভঙ্গ;
	পক্ষ CSS_UNIT_S:
		fprintf(stream, "s");
		ভঙ্গ;
	পক্ষ CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		ভঙ্গ;
	পক্ষ CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		ভঙ্গ;
	পক্ষ CSS_UNIT_CH:
		fprintf(stream, "ch");
		ভঙ্গ;
	পক্ষ CSS_UNIT_REM:
		fprintf(stream, "rem");
		ভঙ্গ;
	পক্ষ CSS_UNIT_LH:
		fprintf(stream, "lh");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VH:
		fprintf(stream, "vh");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VW:
		fprintf(stream, "vw");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VI:
		fprintf(stream, "vi");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VB:
		fprintf(stream, "vb");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		ভঙ্গ;
	পক্ষ CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		ভঙ্গ;
	পক্ষ CSS_UNIT_Q:
		fprintf(stream, "q");
		ভঙ্গ;
	পক্ষ CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		ভঙ্গ;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
শূন্য nscss_dump_computed_style(FILE *stream, ধ্রুব css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = রিক্ত;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ধ্রুব css_computed_content_item *content = রিক্ত;
	ধ্রুব css_computed_counter *counter = রিক্ত;
	lwc_string **string_list = রিক্ত;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	বিকল্প (val) {
	পক্ষ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		ভঙ্গ;
	পক্ষ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	যদি (val == CSS_BACKGROUND_IMAGE_IMAGE && url != রিক্ত) {
		fprintf(stream, "background-image: url('%.*s') ",
				(পূর্ণাঙ্ক) lwc_string_length(url),
				lwc_string_data(url));
	} অন্যথা যদি (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	যদি (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	বিকল্প (val) {
	পক্ষ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		ভঙ্গ;
	পক্ষ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		ভঙ্গ;
	পক্ষ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		ভঙ্গ;
	পক্ষ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		ভঙ্গ;
	স্বভাব:

		ভঙ্গ;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	যদি (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_শীর্ষ_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-right-color */
	val = css_computed_border_ডান_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-left-color */
	val = css_computed_border_বাম_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-top-style */
	val = css_computed_border_শীর্ষ_style(style);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-right-style */
	val = css_computed_border_ডান_style(style);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-left-style */
	val = css_computed_border_বাম_style(style);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-top-width */
	val = css_computed_border_শীর্ষ_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-right-width */
	val = css_computed_border_ডান_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* border-left-width */
	val = css_computed_border_বাম_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		ভঙ্গ;
	পক্ষ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		ভঙ্গ;
	পক্ষ CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	বিকল্প (val) {
	পক্ষ CSS_CAPTION_SIDE_শীর্ষ:
		fprintf(stream, "caption_side: top ");
		ভঙ্গ;
	পক্ষ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* clear */
	val = css_computed_clear(style);
	বিকল্প (val) {
	পক্ষ CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		ভঙ্গ;
	পক্ষ CSS_CLEAR_বাম:
		fprintf(stream, "clear: left ");
		ভঙ্গ;
	পক্ষ CSS_CLEAR_ডান:
		fprintf(stream, "clear: right ");
		ভঙ্গ;
	পক্ষ CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	বিকল্প (val) {
	পক্ষ CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		ভঙ্গ;
	পক্ষ CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		যদি (rect.শীর্ষ_auto)
			fprintf(stream, "auto");
		অন্যথা
			dump_css_unit(stream, rect.শীর্ষ, rect.tunit);
		fprintf(stream, ", ");

		যদি (rect.ডান_auto)
			fprintf(stream, "auto");
		অন্যথা
			dump_css_unit(stream, rect.ডান, rect.runit);
		fprintf(stream, ", ");

		যদি (rect.bottom_auto)
			fprintf(stream, "auto");
		অন্যথা
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		যদি (rect.বাম_auto)
			fprintf(stream, "auto");
		অন্যথা
			dump_css_unit(stream, rect.বাম, rect.lunit);
		fprintf(stream, ") ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* color */
	val = css_computed_color(style, &color);
	যদি (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	বিকল্প (val) {
	পক্ষ CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		ভঙ্গ;
	পক্ষ CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		ভঙ্গ;
	পক্ষ CSS_CONTENT_SET:
		fprintf(stream, "content:");

		যাবত্ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			বিকল্প (content->type) {
			পক্ষ CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(পূর্ণাঙ্ক) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				ভঙ্গ;
			পক্ষ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				ভঙ্গ;
			}

			content++;
		}

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	যদি ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == রিক্ত)) {
		fprintf(stream, "counter-increment: none ");
	} অন্যথা {
		fprintf(stream, "counter-increment:");

		যাবত্ (counter->name != রিক্ত) {
			fprintf(stream, " %.*s ",
					(পূর্ণাঙ্ক) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->মান);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	যদি ((val == CSS_COUNTER_RESET_NONE) || (counter == রিক্ত)) {
		fprintf(stream, "counter-reset: none ");
	} অন্যথা {
		fprintf(stream, "counter-reset:");

		যাবত্ (counter->name != রিক্ত) {
			fprintf(stream, " %.*s ",
					(পূর্ণাঙ্ক) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->মান);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	যদি (string_list != রিক্ত) {
		যাবত্ (*string_list != রিক্ত) {
			fprintf(stream, " url\"%.*s\")",
					(পূর্ণাঙ্ক) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	বিকল্প (val) {
	পক্ষ CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		ভঙ্গ;
	পক্ষ CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* direction */
	val = css_computed_direction(style);
	বিকল্প (val) {
	পক্ষ CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		ভঙ্গ;
	পক্ষ CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* display */
	val = ns_computed_display_static(style);
	বিকল্প (val) {
	পক্ষ CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		ভঙ্গ;
	পক্ষ CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	বিকল্প (val) {
	পক্ষ CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		ভঙ্গ;
	পক্ষ CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* float */
	val = css_computed_float(style);
	বিকল্প (val) {
	পক্ষ CSS_FLOAT_বাম:
		fprintf(stream, "float: left ");
		ভঙ্গ;
	পক্ষ CSS_FLOAT_ডান:
		fprintf(stream, "float: right ");
		ভঙ্গ;
	পক্ষ CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	যদি (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		যদি (string_list != রিক্ত) {
			যাবত্ (*string_list != রিক্ত) {
				fprintf(stream, " \"%.*s\"",
					(পূর্ণাঙ্ক) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		বিকল্প (val) {
		পক্ষ CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			ভঙ্গ;
		পক্ষ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			ভঙ্গ;
		পক্ষ CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			ভঙ্গ;
		পক্ষ CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			ভঙ্গ;
		পক্ষ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			ভঙ্গ;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		ভঙ্গ;
	পক্ষ CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* font-style */
	val = css_computed_font_style(style);
	বিকল্প (val) {
	পক্ষ CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		ভঙ্গ;
	পক্ষ CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		ভঙ্গ;
	পক্ষ CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	বিকল্প (val) {
	পক্ষ CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		ভঙ্গ;
	পক্ষ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	বিকল্প (val) {
	পক্ষ CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		ভঙ্গ;
	পক্ষ CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		ভঙ্গ;
	পক্ষ CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/*বাম*/
	val = css_computed_বাম(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_বাম_AUTO:
		fprintf(stream, "left: auto ");
		ভঙ্গ;
	পক্ষ CSS_বাম_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		ভঙ্গ;
	পক্ষ CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* line-height */
	val = css_computed_সারি_height(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_সারি_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		ভঙ্গ;
	পক্ষ CSS_সারি_HEIGHT_সংখ্যা:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ভঙ্গ;
	পক্ষ CSS_সারি_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	যদি (url != রিক্ত) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(পূর্ণাঙ্ক) lwc_string_length(url),
				lwc_string_data(url));
	} অন্যথা যদি (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	বিকল্প (val) {
	পক্ষ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	বিকল্প (val) {
	পক্ষ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		ভঙ্গ;
	পক্ষ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* margin-top */
	val = css_computed_margin_শীর্ষ(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		ভঙ্গ;
	পক্ষ CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* margin-right */
	val = css_computed_margin_ডান(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		ভঙ্গ;
	পক্ষ CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		ভঙ্গ;
	পক্ষ CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* margin-left */
	val = css_computed_margin_বাম(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		ভঙ্গ;
	পক্ষ CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		ভঙ্গ;
	পক্ষ CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		ভঙ্গ;
	পক্ষ CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	বিকল্প (val) {
	পক্ষ CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	বিকল্প (val) {
	পক্ষ CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	বিকল্প (val) {
	পক্ষ CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		ভঙ্গ;
	পক্ষ CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	বিকল্প (val) {
	পক্ষ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	বিকল্প (val) {
	পক্ষ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		ভঙ্গ;
	পক্ষ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* padding-top */
	val = css_computed_padding_শীর্ষ(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* padding-right */
	val = css_computed_padding_ডান(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* padding-left */
	val = css_computed_padding_বাম(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* position */
	val = css_computed_position(style);
	বিকল্প (val) {
	পক্ষ CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		ভঙ্গ;
	পক্ষ CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		ভঙ্গ;
	পক্ষ CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		ভঙ্গ;
	পক্ষ CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	যদি (val == CSS_QUOTES_STRING && string_list != রিক্ত) {
		fprintf(stream, "quotes:");

		যাবত্ (*string_list != রিক্ত) {
			fprintf(stream, " \"%.*s\"",
				(পূর্ণাঙ্ক) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} অন্যথা {
		বিকল্প (val) {
		পক্ষ CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			ভঙ্গ;
		স্বভাব:
			ভঙ্গ;
		}
	}

	/*ডান*/
	val = css_computed_ডান(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_ডান_AUTO:
		fprintf(stream, "right: auto ");
		ভঙ্গ;
	পক্ষ CSS_ডান_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	বিকল্প (val) {
	পক্ষ CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		ভঙ্গ;
	পক্ষ CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* text-align */
	val = css_computed_text_align(style);
	বিকল্প (val) {
	পক্ষ CSS_TEXT_ALIGN_বাম:
		fprintf(stream, "text-align: left ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_ডান:
		fprintf(stream, "text-align: right ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_LIBCSS_বাম:
		fprintf(stream, "text-align: -libcss-left ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_ALIGN_LIBCSS_ডান:
		fprintf(stream, "text-align: -libcss-right ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	যদি (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} অন্যথা {
		fprintf(stream, "text-decoration:");

		যদি (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		যদি (val & CSS_TEXT_DECORATION_সারি_THROUGH) {
			fprintf(stream, " line-through");
		}
		যদি (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		যদি (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	বিকল্প (val) {
	পক্ষ CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		ভঙ্গ;
	পক্ষ CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/*শীর্ষ*/
	val = css_computed_শীর্ষ(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_শীর্ষ_AUTO:
		fprintf(stream, "top: auto ");
		ভঙ্গ;
	পক্ষ CSS_শীর্ষ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	বিকল্প (val) {
	পক্ষ CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		ভঙ্গ;
	পক্ষ CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		ভঙ্গ;
	পক্ষ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_শীর্ষ:
		fprintf(stream, "vertical-align: top ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_TEXT_শীর্ষ:
		fprintf(stream, "vertical-align: text-top ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_মধ্য:
		fprintf(stream, "vertical-align: middle ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		ভঙ্গ;
	পক্ষ CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* visibility */
	val = css_computed_visibility(style);
	বিকল্প (val) {
	পক্ষ CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		ভঙ্গ;
	পক্ষ CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		ভঙ্গ;
	পক্ষ CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* white-space */
	val = css_computed_white_space(style);
	বিকল্প (val) {
	পক্ষ CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		ভঙ্গ;
	পক্ষ CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		ভঙ্গ;
	পক্ষ CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		ভঙ্গ;
	পক্ষ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		ভঙ্গ;
	পক্ষ CSS_WHITE_SPACE_PRE_সারি:
		fprintf(stream, "white-space: pre-line ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		ভঙ্গ;
	পক্ষ CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	বিকল্প (val) {
	পক্ষ CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		ভঙ্গ;
	পক্ষ CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	বিকল্প (val) {
	পক্ষ CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		ভঙ্গ;
	পক্ষ CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	fprintf(stream, "}");
}
